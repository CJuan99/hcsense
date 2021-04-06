#include <MQ135.h>
#define ANALOGPIN A0    //  Define Analog PIN on Arduino Board
//#define RZERO 206.85
#define RZERO 100  //  Define RZERO Calibration Value
MQ135 gasSensor = MQ135(ANALOGPIN);
#include <NTPClient.h>
#include <WiFiUdp.h>

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org");

#include <Wire.h>  // This library is already built in to the Arduino IDE
#include <LiquidCrystal_I2C.h>
#include <BH1750.h>
LiquidCrystal_I2C lcd(0x27, 16, 2);
BH1750 lightMeter(0x23);

#include "DHT.h"
#define dataPin 2
#define DHT_TYPE DHT22
DHT dht(dataPin, DHT_TYPE);

int red_light_pin = 12;
int green_light_pin = 13;
int blue_light_pin = 15;
int buzzerPin = 14;

//long currentTime, lastTime, stopDuration = 0;
String trigger;
boolean nodeStatus = false;
String check="";

#include <ESP8266WiFi.h>
#include<PubSubClient.h>

const char* ssid = "MAXIS";
//const char* ssid = "ELM Guest";
//const char* mqtt_broker = "broker.hivemq.com";
const char* mqtt_broker = "test.mosquitto.org";
// Callback function header
//void callback(char* topic, byte* payload, unsigned int length);
WiFiClient espclient;
PubSubClient client(espclient);


//Ricky's variables --START--
boolean alertTriggered=false;
boolean msgPrinted=false;

//init the compliance value
double co2_compliance = -999; //0 in previous
double light_compliance = -999; //0 in previous
double temp_compliance=-999; 
double humi_compliance=-999;

long currentTime, lastTime, stopDuration = 0; //For buzzer to stop when user clicked

String co2_timestamp;
String co2_fixed_timestamp;
float co2_highest=0;
boolean co2_status = false;
String co2_latest_compliance;

String light_timestamp;
String light_fixed_timestamp;
float light_highest=0;
boolean light_status = false;
String light_latest_compliance;

String temp_timestamp;
String temp_fixed_timestamp;
float temp_highest=0;
boolean temp_status = false;
String temp_latest_compliance;

String humi_timestamp;
String humi_fixed_timestamp;
float humi_highest=0;
boolean humi_status = false;
String humi_latest_compliance;
// --END--

void RGB_color(int red_light_value, int green_light_value, int blue_light_value) {
  analogWrite(red_light_pin, red_light_value);
  analogWrite(green_light_pin, green_light_value);
  analogWrite(blue_light_pin, blue_light_value);

}

void alert() {
//  analogWrite(buzzerPin, 20);
//  delay(2000);
//  analogWrite(buzzerPin, 0);
//  delay(1000);

// --- Ricky's part ---
  analogWrite(buzzerPin,200);
  
  delay(300);
  analogWrite(buzzerPin, 0);
  alertTriggered=true; //Used for check alerted or not, then use to measure the total seconds to 5 seconds
}

void setupWifi() {
  delay(100);
  Serial.print("\nConnecting to");
  Serial.println(ssid);
  WiFi.begin(ssid, "0123528898");
 //WiFi.begin(ssid, "WiF1@WMELm");

  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
    Serial.print("-");
  }
  Serial.print("Connected to ");
  Serial.print(ssid);
}
void reconnect() {
  while (!client.connected()) {
    Serial.print("\nConnecting to");
    Serial.println(mqtt_broker);
    if (client.connect("HCSense_Client")) {
      Serial.println("\nConnected");
      client.subscribe("HCSense/stopDur"); //subscribe stop duration from mqtt_broker
      client.subscribe("HCSense/callStop");
      client.subscribe("HCSense/temp_cpValue");
      client.subscribe("HCSense/humi_cpValue");
      client.subscribe("HCSense/co2_cpValue");
      client.subscribe("HCSense/light_cpValue");
    } else {
      Serial.println("\nTry to connect again");
      delay(500);
    }
  }
}

//void callback(char* topic, byte* payload, unsigned int length) {
//  //strcmp() compare both strings* and return 0==true (programming normally 1==true, 0==false)
//  if (strcmp(topic, "HCSense/stopDur") == 0 ) {
//    Serial.print("Received stop duration: ");
//    String msgTemp; //Message Temporary
//    for (int i = 0; i < length; i++) {
//      msgTemp += (char)payload[i];
//    }
//
//    Serial.println(msgTemp.toInt());
//    stopDuration = msgTemp.toInt();
//  } else if (strcmp(topic, "HCSense/callStop") == 0) {
//    lastTime = millis();
//  }
//}

void callback(char* topic, byte* payload, unsigned int length) {
  //strcmp() compare both strings* and return 0==true (programming normally 1==true, 0==false)

//   if (strcmp(topic, "HCSense/stopDur") == 0) {
//    Serial.print("Received stop duration: ");
//    String msgTemp; //Message Temporary
//    for (int i = 0; i < length; i++) {
//      msgTemp += (char)payload[i];
//    }
//
//    Serial.println(msgTemp.toInt());
//    stopDuration = msgTemp.toInt();
//  } else if (strcmp(topic, "HCSense/callStop") == 0) {
//    lastTime = millis();
//  }
  if(strcmp(topic, "HCSense/stopDur") == 0){
    Serial.print("Initialized stop duration: ");
    String msgTemp; //Message Temporary
    for(int i=0; i<length; i++){
      msgTemp += (char)payload[i];
    }
  
    Serial.println(msgTemp.toInt());
    stopDuration = msgTemp.toInt();
    
  }else if(strcmp(topic, "HCSense/callStop") == 0){
    lastTime = millis();
    
  }else if(strcmp(topic, "HCSense/temp_cpValue") == 0){
    Serial.print("Initialized temperature compliance: ");
    String msgTem; //Message Temporary
    for (int i = 0; i < length; i++) {
      msgTem += (char)payload[i];
    }

    Serial.println( msgTem.toInt());
    temp_compliance =  msgTem.toInt();
    
  }else if(strcmp(topic, "HCSense/humi_cpValue") == 0){
    Serial.print("Initialized humidity compliance: ");
    String msgHumi; //Message Temporary
    for (int i = 0; i < length; i++) {
      msgHumi += (char)payload[i];
    }

    Serial.println(msgHumi.toInt());
    humi_compliance = msgHumi.toInt();
  }else if (strcmp(topic, "HCSense/co2_cpValue") == 0) {
    Serial.print("Cp value of co2 : ");
    String msgCo2; //Message Temporary
    for (int i = 0; i < length; i++) {
      msgCo2 += (char)payload[i];
    }
    Serial.println( msgCo2.toInt());
    co2_compliance =  msgCo2.toInt();
  }else if (strcmp(topic, "HCSense/light_cpValue") == 0) {
    Serial.print("Cp value of light: ");
    String msgLight; //Message Temporary
    for (int i = 0; i < length; i++) {
      msgLight += (char)payload[i];
    }
    Serial.println(msgLight.toInt());
    light_compliance = msgLight.toInt();
  } 
}




void setup() {
  Wire.begin(4, 5);
  Serial.begin(115200);
  float rzero = gasSensor.getRZero();
  delay(3000);
  //Serial.print("MQ135 RZERO Calibration Value : ");
   setupWifi();
  client.setServer(mqtt_broker, 1883);
  client.setCallback(callback);
 // Serial.println(rzero);

  lcd.init();   // initializing the LCD
  lcd.backlight(); // Enable or Turn On the backlight
  lcd.clear();                                          // Clear LCD display
  lcd.setCursor(0, 0);  //DP0 of Topline
  lcd.print("HC-SENSE"); // Start Print text to Line 1
  lcd.setCursor(0, 1);   //DP0 of bottom line
  lcd.print("-----------------------"); // Start Print Test to Line 2
  delay(2000);

  dht.begin();
  pinMode(buzzerPin, OUTPUT);
  pinMode(red_light_pin, OUTPUT);
  pinMode(green_light_pin, OUTPUT);
  pinMode(blue_light_pin, OUTPUT);

  timeClient.begin();
  timeClient.setTimeOffset(28800);

  if (lightMeter.begin(BH1750::CONTINUOUS_HIGH_RES_MODE)) {
    Serial.println(F("BH1750 Advanced begin"));
  }
  else {
    Serial.println(F("Error initialising BH1750"));
  }

 
 

}


void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  if(co2_compliance==-999 || light_compliance==-999 || 
  temp_compliance==-999 || humi_compliance==-999 || stopDuration==0){
    if(!msgPrinted){
      Serial.println("");
      Serial.println("Waiting subscriptions to be initialized...");
      msgPrinted=true;
    }
  }else{
    Serial.println("");
    delay(1000); /*Wait for DHT22 to load, if not will not be connected +500 */
  
    // --- Get Timestamp ---
    timeClient.update();
  
    unsigned long epochTime = timeClient.getEpochTime();
    String formattedTime = timeClient.getFormattedTime();
  
    struct tm *ptm = gmtime ((time_t *)&epochTime); //Get a time structure
  
    int monthDay = ptm->tm_mday;
    int currentMonth = ptm->tm_mon+1;
    int currentYear = ptm->tm_year+1900;
  
    char currentDateTime[20];
    sprintf_P(currentDateTime, PSTR("%4d-%02d-%02d %s"), currentYear, currentMonth, monthDay, formattedTime.c_str());
  //  String currentDateTime = String(currentYear) + "-" + String(currentMonth) + "-" + String(monthDay) + " " + formattedTime;
    Serial.print("Current DateTime: "); //Print complete date:
    Serial.println(currentDateTime);
    // --- End of Get Timestamp ---

      
    float lux = lightMeter.readLightLevel(true);
    float ppm = gasSensor.getPPM();
    float temperature = dht.readTemperature();
    float humidity = dht.readHumidity();
    
    if (isnan(lux) || isnan(ppm) || isnan(humidity) || isnan(temperature)) {
      Serial.println("Failed to read");
//      Serial.println(lux.c, ppm, humidity, temperature);
    if(isnan(lux)){
        Serial.println("LUX");
      }
      if(isnan(ppm)){
        Serial.println("PPM");
      }
      if(isnan(humidity)){
        Serial.println("HUMI");
      }
      if(isnan(temperature)){
        Serial.println("TEMP");
      }
      RGB_color(255, 0, 0);
      delay(1000);
      RGB_color(0, 0, 0);
      return;
    } else {
      //RGB_color(0,0,255);
      client.publish("HCSense/ppm", String(ppm).c_str());
      client.publish("HC-Sense/lux", String(lux).c_str());
      client.publish("HCSense/temp", String(temperature).c_str());
      client.publish("HCSense/humi", String(humidity).c_str());

      //digitalWrite(13,HIGH);
      lcd.setCursor(0, 0);
      lcd.print("CO2:");
      //lcd.setCursor(4,1);
  
      lcd.setCursor(0, 1);
      lcd.print(ppm);
      lcd.print("ppm");
      delay(925);
      lcd.clear();
      
      lcd.setCursor(0, 0);
      lcd.print("Light Intensity:");
      lcd.setCursor(0, 1);
      lcd.print(lux);
      lcd.print("lux");
      delay(925);
      lcd.clear();

      lcd.setCursor(0, 0);
      lcd.print("Temperature:");
      lcd.setCursor(0, 1);
      lcd.print(temperature);
      lcd.print("celcius");
      delay(925);
      lcd.clear();

      lcd.setCursor(0, 0);
      lcd.print("Humidity:");
      lcd.setCursor(0, 1);
      lcd.print(humidity);
      lcd.print("rh");
      delay(925);
      lcd.clear();
  
      //----!!!IMPORTANT!!!------NEED SET LCD WITH TEMPERATURE AND HUMITIDY ALSO----!!!IMPORTANT!!!------
      
//      if (lux >= light_compliance && ppm >= co2_compliance ) {
//        RGB_color(255, 0, 0);
//        if(currentTime-lastTime > stopDuration){
//          alert();
////          alert();
////          delay(2250);
////          alert();
//        }
//  
//        if(check != "lux-ppm"){
//          nodeStatus = false; //will send email
//          check = "lux-ppm"; //if light avoid sending email
//        }
//        if(nodeStatus == false){
//          trigger = "\n""Current Light Intensity level: " + String(lux) + " lux" + "\n" "Compliance value: " + String(light_compliance)+" lux" + "\n\n" "Current Co2 level: " + String(ppm)+ " ppm" + "\n" "Compliance value: " + String(co2_compliance)+ " ppm";
//          client.publish("HCSense/emailAlert", String(trigger).c_str());
//          nodeStatus = true; 
//          Serial.print("Both");
//        }
//  
//        
//      }else if ( ppm >= co2_compliance){
//        
//        RGB_color(255, 0, 0);
//        alert();
//  
//        if(check != "ppm"){
//          nodeStatus = false; //will send email
//          check = "ppm"; //if light avoid sending email
//         
//        }
//        if(nodeStatus == false){
//          trigger = "Current Carbon Dioxide level: " + String(ppm) + " ppm" + "\n" "Compliance value: " + String(co2_compliance)+ " ppm";
//          client.publish("HCSense/emailAlert", String(trigger).c_str());
//          nodeStatus = true; 
//          Serial.print("PPM");
//        }
//  
//  
//      }else if (lux >= light_compliance ) {
//        RGB_color(255, 0, 0);
//        alert();
//  
//        if(check != "lux"){
//          nodeStatus = false; //will send email
//          check = "lux"; //if light avoid sending email
//        }
//        if(nodeStatus == false){
//          trigger = "Current Light Intensity level: " + String(lux) + " lux" + "\n""Compliance Value: " + String(light_compliance)+ "lux";
//          client.publish("HCSense/emailAlert", String(trigger).c_str());
//          nodeStatus = true;
//          Serial.print("Lux");
//        }
//        
//      }else {
//        RGB_color(0, 255, 0);
//        check="";
//        nodeStatus = false;
//        Serial.print("Ok");
//         trigger = "Light Intensity level: " + String(lux) + "lux" + "\n""Compliance Value: " + String(light_compliance)+ "lux"+ "\n""Co2 level: " + String(ppm)+ "ppm" "\n""Compliance Value: " + String(co2_compliance)+ "ppm";
//  //    trigger = "Current situation: Light Intensity:" + String(lux) + ", Humidity:" + String(humi_compliance);
//        client.publish("HCSense/emailNormal", String(trigger).c_str());
//      }


      if(ppm >= co2_compliance && lux >= light_compliance && temperature>=temp_compliance && humidity>=humi_compliance){
        RGB_color(255, 0, 0);
        currentTime=millis();
        if(currentTime-lastTime > stopDuration){
          alert();
        }
  
        if(check != "ppm-lux-temp-humi"){
          nodeStatus = false; //will send email
          check = "ppm-lux-temp-humi"; //if light avoid sending email
        }
        if(nodeStatus == false){
          trigger = "\n" "Current Co2 level: " + String(ppm)+ " ppm" + "\n" "Compliance value: " + String(co2_compliance)+ " ppm" + 
                    "\n\n" "Current Light Intensity level: " + String(lux) + " lux" + "\n" "Compliance value: " + String(light_compliance)+" lux" +
                    "\n\n" "Current Temperature level: " + String(temperature) + " celcius" + "\n" "Compliance value: " + String(temp_compliance)+" celcius" +
                    "\n\n" "Current Humidity level: " + String(humidity) + " rh" + "\n" "Compliance value: " + String(humi_compliance)+" rh";
          client.publish("HCSense/emailAlert", String(trigger).c_str());
          nodeStatus = true; 
          Serial.print("ppm-lux-temp-humi");
        }
      }else if(ppm >= co2_compliance && lux >= light_compliance && temperature>=temp_compliance){
        RGB_color(255, 0, 0);
        currentTime=millis();
        if(currentTime-lastTime > stopDuration){
          alert();
        }
  
        if(check != "ppm-lux-temp"){
          nodeStatus = false;
          check = "ppm-lux-temp";
        }
        if(nodeStatus == false){
          trigger = "\n" "Current Co2 level: " + String(ppm)+ " ppm" + "\n" "Compliance value: " + String(co2_compliance)+ " ppm" + 
                    "\n\n" "Current Light Intensity level: " + String(lux) + " lux" + "\n" "Compliance value: " + String(light_compliance)+" lux" +
                    "\n\n" "Current Temperature level: " + String(temperature) + " celcius" + "\n" "Compliance value: " + String(temp_compliance)+" celcius";
          client.publish("HCSense/emailAlert", String(trigger).c_str());
          nodeStatus = true; 
          Serial.print("ppm-lux-temp");
        }
      }else if(ppm >= co2_compliance && lux >= light_compliance && humidity>=humi_compliance){
        RGB_color(255, 0, 0);
        currentTime=millis();
        if(currentTime-lastTime > stopDuration){
          alert();
        }
  
        if(check != "ppm-lux-humi"){
          nodeStatus = false;
          check = "ppm-lux-humi";
        }
        if(nodeStatus == false){
          trigger = "\n" "Current Co2 level: " + String(ppm)+ " ppm" + "\n" "Compliance value: " + String(co2_compliance)+ " ppm" + 
                    "\n\n" "Current Light Intensity level: " + String(lux) + " lux" + "\n" "Compliance value: " + String(light_compliance)+" lux" +
                    "\n\n" "Current Humidity level: " + String(humidity) + " rh" + "\n" "Compliance value: " + String(humi_compliance)+" rh";
          client.publish("HCSense/emailAlert", String(trigger).c_str());
          nodeStatus = true; 
          Serial.print("ppm-lux-humi");
        }
      }else if(ppm >= co2_compliance && temperature>=temp_compliance && humidity>=humi_compliance){
        RGB_color(255, 0, 0);
        currentTime=millis();
        if(currentTime-lastTime > stopDuration){
          alert();
        }
  
        if(check != "ppm-temp-humi"){
          nodeStatus = false;
          check = "ppm-temp-humi";
        }
        if(nodeStatus == false){
          trigger = "\n" "Current Co2 level: " + String(ppm)+ " ppm" + "\n" "Compliance value: " + String(co2_compliance)+ " ppm" + 
                    "\n\n" "Current Temperature level: " + String(temperature) + " celcius" + "\n" "Compliance value: " + String(temp_compliance)+" celcius" +
                    "\n\n" "Current Humidity level: " + String(humidity) + " rh" + "\n" "Compliance value: " + String(humi_compliance)+" rh";
          client.publish("HCSense/emailAlert", String(trigger).c_str());
          nodeStatus = true; 
          Serial.print("ppm-temp-humi");
        }
      }else if(lux >= light_compliance && temperature>=temp_compliance && humidity>=humi_compliance){
        RGB_color(255, 0, 0);
        currentTime=millis();
        if(currentTime-lastTime > stopDuration){
          alert();
        }
  
        if(check != "lux-temp-humi"){
          nodeStatus = false;
          check = "lux-temp-humi";
        }
        if(nodeStatus == false){
          trigger = "\n" "Current Light Intensity level: " + String(lux) + " lux" + "\n" "Compliance value: " + String(light_compliance)+" lux" +
                    "\n\n" "Current Temperature level: " + String(temperature) + " celcius" + "\n" "Compliance value: " + String(temp_compliance)+" celcius" +
                    "\n\n" "Current Humidity level: " + String(humidity) + " rh" + "\n" "Compliance value: " + String(humi_compliance)+" rh";
          client.publish("HCSense/emailAlert", String(trigger).c_str());
          nodeStatus = true; 
          Serial.print("lux-temp-humi");
        }
      }else if(ppm >= co2_compliance && lux >= light_compliance){
        RGB_color(255, 0, 0);
        currentTime=millis();
        if(currentTime-lastTime > stopDuration){
          alert();
        }
  
        if(check != "ppm-lux"){
          nodeStatus = false;
          check = "ppm-lux";
        }
        if(nodeStatus == false){
          trigger = "\n" "Current Co2 level: " + String(ppm)+ " ppm" + "\n" "Compliance value: " + String(co2_compliance)+ " ppm" + 
                    "\n\n" "Current Light Intensity level: " + String(lux) + " lux" + "\n" "Compliance value: " + String(light_compliance);
          client.publish("HCSense/emailAlert", String(trigger).c_str());
          nodeStatus = true; 
          Serial.print("ppm-lux");
        }
      }else if(ppm >= co2_compliance && temperature>=temp_compliance){
        RGB_color(255, 0, 0);
        currentTime=millis();
        if(currentTime-lastTime > stopDuration){
          alert();
        }
  
        if(check != "ppm-temp"){
          nodeStatus = false;
          check = "ppm-temp";
        }
        if(nodeStatus == false){
          trigger = "\n" "Current Co2 level: " + String(ppm)+ " ppm" + "\n" "Compliance value: " + String(co2_compliance)+ " ppm" + 
                    "\n\n" "Current Temperature level: " + String(temperature) + " celcius" + "\n" "Compliance value: " + String(temp_compliance)+" celcius";
          client.publish("HCSense/emailAlert", String(trigger).c_str());
          nodeStatus = true; 
          Serial.print("ppm-temp");
        }
      }else if(ppm >= co2_compliance && humidity>=humi_compliance){
        RGB_color(255, 0, 0);
        currentTime=millis();
        if(currentTime-lastTime > stopDuration){
          alert();
        }
  
        if(check != "ppm-humi"){
          nodeStatus = false;
          check = "ppm-humi";
        }
        if(nodeStatus == false){
          trigger = "\n" "Current Co2 level: " + String(ppm)+ " ppm" + "\n" "Compliance value: " + String(co2_compliance)+ " ppm" + 
                    "\n\n" "Current Humidity level: " + String(humidity) + " rh" + "\n" "Compliance value: " + String(humi_compliance)+" rh";
          client.publish("HCSense/emailAlert", String(trigger).c_str());
          nodeStatus = true; 
          Serial.print("ppm-humi");
        }
      }else if(lux >= light_compliance && temperature>=temp_compliance){
        RGB_color(255, 0, 0);
        currentTime=millis();
        if(currentTime-lastTime > stopDuration){
          alert();
        }
  
        if(check != "lux-temp"){
          nodeStatus = false;
          check = "lux-temp";
        }
        if(nodeStatus == false){
          trigger = "\n" "Current Light Intensity level: " + String(lux) + " lux" + "\n" "Compliance value: " + String(light_compliance)+" lux" +
                    "\n\n" "Current Temperature level: " + String(temperature) + " celcius" + "\n" "Compliance value: " + String(temp_compliance)+" celcius";
          client.publish("HCSense/emailAlert", String(trigger).c_str());
          nodeStatus = true; 
          Serial.print("lux-temp");
        }
      }else if(lux >= light_compliance && humidity>=humi_compliance){
        RGB_color(255, 0, 0);
        currentTime=millis();
        if(currentTime-lastTime > stopDuration){
          alert();
        }
  
        if(check != "lux-humi"){
          nodeStatus = false;
          check = "lux-humi";
        }
        if(nodeStatus == false){
          trigger = "\n" "Current Light Intensity level: " + String(lux) + " lux" + "\n" "Compliance value: " + String(light_compliance)+" lux" +
                    "\n\n" "Current Humidity level: " + String(humidity) + " rh" + "\n" "Compliance value: " + String(humi_compliance)+" rh";
          client.publish("HCSense/emailAlert", String(trigger).c_str());
          nodeStatus = true; 
          Serial.print("lux-humi");
        }
      }else if(temperature>=temp_compliance && humidity>=humi_compliance){
        RGB_color(255, 0, 0);
        currentTime=millis();
        if(currentTime-lastTime > stopDuration){
          alert();
        }
  
        if(check != "temp-humi"){
          nodeStatus = false;
          check = "temp-humi";
        }
        if(nodeStatus == false){
          trigger = "\n" "Current Temperature level: " + String(temperature) + " celcius" + "\n" "Compliance value: " + String(temp_compliance)+" celcius" +
                    "\n\n" "Current Humidity level: " + String(humidity) + " rh" + "\n" "Compliance value: " + String(humi_compliance)+" rh";
          client.publish("HCSense/emailAlert", String(trigger).c_str());
          nodeStatus = true; 
          Serial.print("temp-humi");
        }
      }else if(ppm >= co2_compliance){
        RGB_color(255, 0, 0);
        currentTime=millis();
        if(currentTime-lastTime > stopDuration){
          alert();
        }
  
        if(check != "ppm"){
          nodeStatus = false;
          check = "ppm";
        }
        if(nodeStatus == false){
          trigger = "\n" "Current Co2 level: " + String(ppm)+ " ppm" + "\n" "Compliance value: " + String(co2_compliance)+ " ppm";
          client.publish("HCSense/emailAlert", String(trigger).c_str());
          nodeStatus = true; 
          Serial.print("ppm");
        }
      }else if(lux >= light_compliance){
        RGB_color(255, 0, 0);
        currentTime=millis();
        if(currentTime-lastTime > stopDuration){
          alert();
        }
  
        if(check != "lux"){
          nodeStatus = false;
          check = "lux";
        }
        if(nodeStatus == false){
          trigger = "\n" "Current Light Intensity level: " + String(lux) + " lux" + "\n" "Compliance value: " + String(light_compliance)+" lux";
          client.publish("HCSense/emailAlert", String(trigger).c_str());
          nodeStatus = true; 
          Serial.print("lux");
        }
      }else if(temperature>=temp_compliance){
        RGB_color(255, 0, 0);
        currentTime=millis();
        if(currentTime-lastTime > stopDuration){
          alert();
        }
  
        if(check != "temp"){
          nodeStatus = false;
          check = "temp";
        }
        if(nodeStatus == false){
          trigger = "\n" "Current Temperature level: " + String(temperature) + " celcius" + "\n" "Compliance value: " + String(temp_compliance)+" celcius";
          client.publish("HCSense/emailAlert", String(trigger).c_str());
          nodeStatus = true; 
          Serial.print("temp");
        }
      }else if(humidity>=humi_compliance){
        RGB_color(255, 0, 0);
        currentTime=millis();
        if(currentTime-lastTime > stopDuration){
          alert();
        }
  
        if(check != "humi"){
          nodeStatus = false;
          check = "humi";
        }
        if(nodeStatus == false){
          trigger = "\n" "Current Humidity level: " + String(humidity) + " rh" + "\n" "Compliance value: " + String(humi_compliance)+" rh";
          client.publish("HCSense/emailAlert", String(trigger).c_str());
          nodeStatus = true; 
          Serial.print("humi");
        }
      }else{
        RGB_color(0, 255, 0);
        
        if(check != ""){
          check = "";
          nodeStatus = false;
          Serial.print("all conditions met!");
          trigger = "\n" "Current Co2 level: " + String(ppm)+ " ppm" + "\n" "Compliance value: " + String(co2_compliance)+ " ppm" + "\n\n" "Current Light Intensity level: " + String(lux) + " lux" + "\n" "Compliance value: " + String(light_compliance)+" lux" + "\n\n" "Current Temperature level: " + String(temperature) + " celcius" + "\n" "Compliance value: " + String(temp_compliance)+" celcius" +"\n\n"  "Current Humidity level: " + String(humidity) + " rh" + "\n" "Compliance value: " + String(humi_compliance)+" rh";
          client.publish("HCSense/emailNormal", String(trigger).c_str());
        }
      }


      //Abnormal Records
      if(co2_status!=true){
        co2_timestamp = currentDateTime;
        if(ppm>=co2_compliance){
          co2_highest = ppm;
          co2_status=true;
        }
      }else{
        if(ppm>=co2_compliance){
          if(ppm>co2_highest){
            co2_highest = ppm;
          }
        }else{
          co2_fixed_timestamp = currentDateTime;
          co2_status=false;
  
          co2_latest_compliance = "Co2<" + String(co2_compliance);
  
          client.publish("HCSense/rp_co2_timestamp", String(co2_timestamp).c_str());
          client.publish("HCSense/rp_co2_fixed_timestamp", String(co2_fixed_timestamp).c_str());
          client.publish("HCSense/rp_co2_highest", String(co2_highest).c_str());
          client.publish("HCSense/rp_co2_latest_compliance", String(co2_latest_compliance).c_str());
  
          Serial.println("Co2 TimeStamp: " + co2_timestamp);
          Serial.println("Co2 Fixed_TimeStamp: " + co2_fixed_timestamp);
          Serial.println("Co2 Highest: " + String(co2_highest));
  
          co2_highest=0;
        }
      }
  
      if(light_status!=true){
        light_timestamp = currentDateTime;
        if(lux>=light_compliance){
          light_highest = lux;
          light_status=true;
        }
      }else{
        if(lux>=light_compliance){
          if(lux>light_highest){
            light_highest = lux;
          }
        }else{
          light_fixed_timestamp = currentDateTime;
          light_status=false;
  
          light_latest_compliance = "Light Intensity<" + String(light_compliance);
  
          client.publish("HCSense/rp_light_timestamp", String(light_timestamp).c_str());
          client.publish("HCSense/rp_light_fixed_timestamp", String(light_fixed_timestamp).c_str());
          client.publish("HCSense/rp_light_highest", String(light_highest).c_str());
          client.publish("HCSense/rp_light_latest_compliance", String(light_latest_compliance).c_str());
  
          Serial.println("Light TimeStamp: " + light_timestamp);
          Serial.println("Light Fixed_TimeStamp: " + light_fixed_timestamp);
          Serial.println("Light Highest: " + String(light_highest));
  
          light_highest=0;
        }
      }

      if(temp_status!=true){
        temp_timestamp = currentDateTime;
        if(temperature>=temp_compliance){
          temp_highest = temperature;
          temp_status=true;
        }
      }else{
        if(temperature>=temp_compliance){
          if(temperature>temp_highest){
            temp_highest = temperature;
          }
        }else{
          temp_fixed_timestamp = currentDateTime;
          temp_status=false;
  
          temp_latest_compliance = "Temperature<" + String(temp_compliance);
  
          client.publish("HCSense/rp_temp_timestamp", String(temp_timestamp).c_str());
          client.publish("HCSense/rp_temp_fixed_timestamp", String(temp_fixed_timestamp).c_str());
          client.publish("HCSense/rp_temp_highest", String(temp_highest).c_str());
          client.publish("HCSense/rp_temp_latest_compliance", String(temp_latest_compliance).c_str());
  
          Serial.println("Temp TimeStamp: " + temp_timestamp);
          Serial.println("Temp Fixed_TimeStamp: " + temp_fixed_timestamp);
          Serial.println("Temp Highest: " + String(temp_highest));
  
          temp_highest=0;
        }
      }
  
      if(humi_status!=true){
        humi_timestamp = currentDateTime;
        if(humidity>=humi_compliance){
          humi_highest = humidity;
          humi_status=true;
        }
      }else{
        if(humidity>=humi_compliance){
          if(humidity>humi_highest){
            humi_highest = humidity;
          }
        }else{
          humi_fixed_timestamp = currentDateTime;
          humi_status=false;
  
          humi_latest_compliance = "Humidity<" + String(humi_compliance);
  
          client.publish("HCSense/rp_humi_timestamp", String(humi_timestamp).c_str());
          client.publish("HCSense/rp_humi_fixed_timestamp", String(humi_fixed_timestamp).c_str());
          client.publish("HCSense/rp_humi_highest", String(humi_highest).c_str());
          client.publish("HCSense/rp_humi_latest_compliance", String(humi_latest_compliance).c_str());
  
          Serial.println("Humi TimeStamp: " + humi_timestamp);
          Serial.println("Humi Fixed_TimeStamp: " + humi_fixed_timestamp);
          Serial.println("Humi Highest: " + String(humi_highest));
  
          humi_highest=0;
        }
      }
  
  
  
  
      Serial.println();
      Serial.print("Received value of light: ");
      Serial.print(lux);
      Serial.println(" lux");
      Serial.print("Compliance value: ");
      Serial.println(light_compliance);
//      delay(500);
      Serial.println();
      Serial.print("Received value of Co2 : ");
      Serial.print(ppm);
      Serial.println(" ppm");
      Serial.print("Compliance value:");
      Serial.println(co2_compliance);

      Serial.println();
      Serial.print("Received value of temp: ");
      Serial.print(temperature);
      Serial.println(" celcius");
      Serial.print("Compliance value: ");
      Serial.println(temp_compliance);
//      delay(500);
      Serial.println();
      Serial.print("Received value of humi : ");
      Serial.print(humidity);
      Serial.println(" rh");
      Serial.print("Compliance value:");
      Serial.println(humi_compliance);

      //Make data send on every 5 secs
      if(alertTriggered){
        alertTriggered=false;
      }else{
        delay(300);
      }
    
    }
  } // - - - After Initialized - - -
  
}
