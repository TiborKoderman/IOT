#include "WiFi.h"
#include "PubSubClient.h"
#include <Ethernet.h>
#include <SPI.h>
#include <Wire.h>
#include <FastLED.h>

#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <Preferences.h>
#include <mqtt_client.h>

#define SEALEVELPRESSURE_HPA (1013.25)

#define FASTLEDPIN 15
#define NUM_LEDS 5

CRGB leds[NUM_LEDS];

Adafruit_BME280 bme; // I2C

Preferences preferences;

uint32_t delayMS;


char ssid[32];
char password[32];

char clientID[32];// = "ESP32 Unregistered";
String prefix = String(clientID) + "/";

char* default_ssid = "Koderman";
char* default_password = "kodermancki";
 
WiFiClient ethClient;
 
//IPAddress  mqtt_server(192,168,1,2);
PubSubClient client(ethClient);
const char* mqtt_server = "xeoqs.ddns.net";


void setup(){
  pinMode(2, OUTPUT);
  digitalWrite(2, HIGH);
  delay(200);
  digitalWrite(2, LOW);
  delay(200);
  digitalWrite(2, HIGH);
  delay(200);
  digitalWrite(2, LOW);
  delay(200);

  if (! bme.begin(&Wire)) {
        Serial.println("Could not find a valid BME280 sensor, check wiring!");
        while (1);
    }
  preferences.begin("iot", false); 

  FastLED.addLeds<WS2812B, FASTLEDPIN, GRB>(leds, NUM_LEDS);
  
  //uint64_t chipid = ESP.getEfuseMac();

  //Serial.printf("ESP32 Chip ID = %04X",(uint16_t)(chipid>>32));//print High 2 bytes
  //Serial.printf("%08X\n",(uint32_t)chipid);//print Low 4bytes.
  
 // client.publish("chipID", ((chipid).to_string()).c_str());
  

  Serial.println(F("------------------------------------"));
  // Set delay between sensor readings based on sensor details.

  strcpy(ssid, (preferences.getString("ssid", "K8")).c_str());
  strcpy(password, (preferences.getString("password", "dogsminusone")).c_str());
  strcpy(clientID, (preferences.getString("clientID", "ESP32 Unregistered")).c_str());
  
  
  client.setServer(mqtt_server, 1883);
  client.setCallback(mqtt_callback);
  Serial.begin(115200); 

  while (WiFi.status() != WL_CONNECTED) {
    for(int i = 0; i< 10 && WiFi.status() != WL_CONNECTED; i++)
    {
    delay(500);
    Serial.println("Connecting to WiFi..");
    WiFi.begin(ssid, password);
    }
    for(int i = 0; i<10 && WiFi.status() != WL_CONNECTED; i++)
    {
    delay(500);
    Serial.println("Could not connect searching for default...");
    WiFi.begin(default_ssid, default_password);
    }
  }
 if(WiFi.status() == WL_CONNECTED)
    {
      //digitalWrite(BUILTIN_LED, HIGH);
      Serial.println("Connected to the WiFi network");
      Serial.println("Ip: ");
      Serial.println(WiFi.localIP());  
    }
}

void mqttconnect() {
  // Loop until reconnected
  while (!client.connected()) {
    Serial.print(client.state());
    Serial.print("MQTT connecting ...");
    // client ID 
    //const char* clientId = "ESP32";
    // connect now 
    if (client.connect(clientID, "","")) {
      Serial.println("connected");
      // subscribe topic with default QoS 0
      client.subscribe("LED");
      client.subscribe("FastLED");
      client.subscribe("SSID");
      client.subscribe("PASS");
      client.subscribe("CLIENTID");
    } else {
      Serial.print("failed, status code =");
      Serial.print(client.state());
      Serial.println("try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void mqtt_callback(char* topic, byte *payload, unsigned int length) {
    Serial.println("-------new message from broker-----");
    Serial.print("channel:");
    Serial.println(topic);
    Serial.print("data:"); 
    Serial.write(payload, length);
    Serial.println();
    String messageTemp;
    
    for (int i = 0; i < length; i++) {
    //Serial.print((char)payload[i]);
    messageTemp += ((char)payload[i]);
    }

    if(String(topic)=="LED")
    {
      if(messageTemp == "on"){
        digitalWrite(2, HIGH);
      }
       else if(messageTemp == "off"){
        digitalWrite(2, LOW);
       }
    }
    
    if(String(topic)=="FastLED")
    {
      messageTemp.remove(0,1);
      for(int i =0; i<NUM_LEDS;i++)
        {
          leds[i] = (int)strtol(messageTemp.c_str(), NULL,16);
        }
        FastLED.show();
    }
    
    if(String(topic) =="SSID")
    {
      strcpy(ssid, messageTemp.c_str());
      preferences.putString("ssid", messageTemp);
      Serial.print("Updated Wifi ssid: ");
      Serial.println(ssid);
    }
    if(String(topic) =="PASS")
    {
      strcpy(password, messageTemp.c_str());
      preferences.putString("password", messageTemp);
      Serial.print("Updated Wifi password");
      Serial.println(password);
      ESP.restart();
    }
    
    if(String(topic) =="CLIENTID")
    {
      strcpy(clientID, messageTemp.c_str());
      preferences.putString("clientID", messageTemp);
      Serial.print("Updated clientID: ");
      Serial.println(clientID);
      ESP.restart();
    }

    
      
}
unsigned long int timeSensor = millis();
void readTemp()
{
  
}

void printValues() {
    //Serial.print("Temperature = ");
    client.publish("Temperature", String(bme.readTemperature()).c_str());
    //Serial.println(" *C");

    //Serial.print("Pressure = ");

    //Serial.print(bme.readPressure() / 100.0F);
    client.publish("Pressure", String(bme.readPressure()/100.0F).c_str());
    //Serial.println(" hPa");

    //Serial.print("Approx. Altitude = ");
    //Serial.print(bme.readAltitude(SEALEVELPRESSURE_HPA));
    client.publish("Altitude", String(bme.readAltitude(SEALEVELPRESSURE_HPA)).c_str());
    //Serial.println(" m");

    //Serial.print("Humidity = ");
    //Serial.print(bme.readHumidity());
    client.publish("Humidity", String(bme.readHumidity()).c_str());
    //Serial.println(" %");

    //Serial.println();
}

int vdelaymesure = millis();

void loop(){

  if (!client.connected()) {  // reconnect
    mqttconnect();
  }
  if(millis() - vdelaymesure > 1000)
  {
  bme.takeForcedMeasurement(); // has no effect in normal mode
  printValues();
  vdelaymesure = millis();
  }
  client.loop();
}
