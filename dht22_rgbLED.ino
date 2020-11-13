#include <ESP8266WiFi.h>
#include <PubSubClient.h>

#include "DHT.h"

#define dataPin 2
#define DHT_TYPE DHT22

int red_light_pin = 4;
int green_light_pin = 5;
int blue_light_pin = 16;

int buzzerPin = 14;

DHT dht(dataPin, DHT_TYPE);


const char* ssid = "chuiping17@unifi@unifi";
const char* password = "0126316617";
const char* mqtt_broker = "test.mosquitto.org";

WiFiClient espclient;
PubSubClient client(espclient);


long currentTime, lastTime, stopDuration = 0; //For buzzer to stop when user clicked


void connectWiFi(){
  delay(500);
  Serial.print("Connecting");
  WiFi.begin(ssid, password);
  while(WiFi.status()!=WL_CONNECTED){
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected to WiFi");
} 

void reconnect(){
  while(!client.connected()){
    Serial.print("\nConnecting to");
    Serial.println(mqtt_broker);
    if(client.connect("HCSense_Client")){
      Serial.println("\nConnected");
      client.subscribe("HCSense/stopDur"); //subscribe stop duration from mqtt_broker
    }else{
      Serial.println("\nTry to connect again");
      delay(500);
    }
  }
}

void callback(char* topic, byte* payload, unsigned int length){
  Serial.print("Received stop duration: ");
  String msgTemp;
  for(int i=0; i<length; i++){
    msgTemp += (char)payload[i];
  }

  Serial.println(msgTemp.toInt());
  stopDuration = msgTemp.toInt();
  lastTime = millis();
  
}

void setup() {
  Serial.begin(115200);
  connectWiFi();
  client.setServer(mqtt_broker, 1883);
  client.setCallback(callback);
  
  dht.begin();

  pinMode(buzzerPin, OUTPUT);
}

void loop() {
  if(!client.connected()){
    reconnect();
  }
  client.loop();
  
  delay(2000); /*Wait for DHT22 to load, if not will not be connected*/
  
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();

  if (isnan(humidity) || isnan(temperature)) {
    Serial.println("Failed to read from DHT sensor!");
    RGB_color(255, 0, 0);
    delay(1000);
    RGB_color(0, 0, 0);
    return;
  }else{
    client.publish("HCSense/temp", String(temperature).c_str());
    client.publish("HCSense/humi", String(humidity).c_str());

    Serial.print("Temperature: ");
    Serial.println(temperature);
    Serial.print("Humidity: ");
    Serial.println(humidity);
    
    if(temperature>=36 || humidity>=83){
      currentTime=millis();
      if(currentTime-lastTime > stopDuration){
        RGB_color(255, 255, 0);
      
        alert();

        stopDuration=0;
        lastTime=millis();
      }
    }else{
      RGB_color(0, 255, 0);
    }
    
  }

}


void RGB_color(int red_light_value, int green_light_value, int blue_light_value)
 {
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
