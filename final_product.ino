#include <MQ135.h>
#define ANALOGPIN A0    //  Define Analog PIN on Arduino Board
//#define RZERO 206.85
#define RZERO 400   //  Define RZERO Calibration Value
MQ135 gasSensor = MQ135(ANALOGPIN);

#include <Wire.h>  // This library is already built in to the Arduino IDE
#include <LiquidCrystal_I2C.h>
#include <BH1750.h>
LiquidCrystal_I2C lcd(0x27, 16, 2);
BH1750 lightMeter(0x23);

int red_light_pin = 12;
int green_light_pin = 13;
int blue_light_pin = 15;


int buzzerPin = 14;

#include <ESP8266WiFi.h>
#include<PubSubClient.h>

const char* ssid = "MAXIS";
const char* mqtt_broker = "test.mosquitto.org";
// Callback function header
//void callback(char* topic, byte* payload, unsigned int length);
WiFiClient espclient;
PubSubClient client(espclient);


long currentTime, lastTime, stopDuration = 0; //For buzzer to stop when user clicked


void RGB_color(int red_light_value, int green_light_value, int blue_light_value) {
  analogWrite(red_light_pin, red_light_value);
  analogWrite(green_light_pin, green_light_value);
  analogWrite(blue_light_pin, blue_light_value);

}

void alert() {
  analogWrite(buzzerPin, 20);
  delay(2000);
  analogWrite(buzzerPin, 0);
  delay(1000);
}

void setupWifi() {
  delay(100);
  Serial.print("\nConnecting to");
  Serial.println(ssid);
  WiFi.begin(ssid, "0123528898");

  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
    Serial.println("-");
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
    } else {
      Serial.println("\nTry to connect again");
      delay(500);
    }
  }
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Received stop duration: ");
  String msgTemp;
  for (int i = 0; i < length; i++) {
    msgTemp += (char)payload[i];
  }

  Serial.println(msgTemp.toInt());
  stopDuration = msgTemp.toInt();
  lastTime = millis();

}



void setup() {
  Wire.begin(4, 5);
  Serial.begin(115200);
  float rzero = gasSensor.getRZero();
  delay(3000);
  Serial.print("MQ135 RZERO Calibration Value : ");
  Serial.println(rzero);



  lcd.init();   // initializing the LCD
  lcd.backlight(); // Enable or Turn On the backlight
  lcd.clear();                                          // Clear LCD display
  lcd.setCursor(0, 0);  //DP0 of Topline
  lcd.print("HC-SENSE"); // Start Print text to Line 1
  lcd.setCursor(0, 1);   //DP0 of bottom line
  lcd.print("-----------------------"); // Start Print Test to Line 2
  delay(2000);
  pinMode(buzzerPin, OUTPUT);

  if (lightMeter.begin(BH1750::CONTINUOUS_HIGH_RES_MODE)) {
    Serial.println(F("BH1750 Advanced begin"));
  }
  else {
    Serial.println(F("Error initialising BH1750"));
  }
  setupWifi();
  client.setServer(mqtt_broker, 1883);
  client.setCallback(callback);

}


void loop() {
  if (!client.connected()) {
    reconnect();
  }

  client.loop();
  float lux = lightMeter.readLightLevel(true);
  float ppm = gasSensor.getPPM();
  if (isnan(lux) || isnan(ppm)) {
    Serial.println("Failed to read");
    RGB_color(255, 0, 0);
    delay(1000);
    RGB_color(0, 0, 0);
    return;
  } else {
    //RGB_color(0,0,255);
    client.publish("HCSense/ppm", String(ppm).c_str());
    client.publish("HC-Sense/lux", String(lux).c_str());
    delay(1000);
    lcd.setCursor(0, 0);
    lcd.print("Light Intensity is:");
    lcd.setCursor(0, 1);
    lcd.print( lux);
    lcd.print( "lux");
    delay(1000);
    lcd.clear();


    //digitalWrite(13,HIGH);
    lcd.setCursor(0, 0);
    lcd.print("CO2 ppm value:");
    //lcd.setCursor(4,1);

    lcd.setCursor(0, 1);
    lcd.print(ppm);
    lcd.print( "ppm");
    delay(1000);
    lcd.clear();

    if (lux >= 300 || ppm >= 150) {
      RGB_color(255, 0, 0);
      alert();

    } else if (lux >= 300 && ppm >= 200) {
      RGB_color(255, 0, 0);
      alert();
    } else {
      RGB_color(0, 255, 0);
    }



    Serial.print("Light: ");
    Serial.print(lux);
    Serial.println(" lx");
    delay(1000);
    Serial.print("CO2 ppm value : ");
    Serial.println(ppm);




  }

}
