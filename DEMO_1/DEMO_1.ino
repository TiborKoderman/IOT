#include "WiFi.h"
#include "PubSubClient.h"
#include <Ethernet.h>
#include <SPI.h>
#include <Wire.h>
#include <FastLED.h>

#include <Adafruit_GFX.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <Preferences.h>
#include <mqtt_client.h>
#include <Adafruit_ST7735.h>


#define TFT_CS   27
#define TFT_RST  32
#define TFT_DC   33
#define TFT_SCLK 26
#define TFT_MOSI 25

Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_MOSI, TFT_SCLK, TFT_RST);

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
const char* mqtt_server = "koderman.net";


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

  pinMode(14, OUTPUT);
  ledcSetup(0, 5000, 8);
  ledcAttachPin(14,0);
  ledcWrite(0,255);
  
  Serial.begin(115200); 
  if (! bme.begin(0x76)) {
        Serial.println("Could not find a valid BME280 sensor, check wiring!");
        while (1);
    }
  preferences.begin("iot", false); 

  FastLED.addLeds<WS2812B, FASTLEDPIN, GRB>(leds, NUM_LEDS);
  
  //uint64_t chipid = ESP.getEfuseMac();

  //Serial.printf("ESP32 Chip ID = %04X",(uint16_t)(chipid>>32));//print High 2 bytes
  //Serial.printf("%08X\n",(uint32_t)chipid);//print Low 4bytes.
  
 // client.publish("chipID", ((chipid).to_string()).c_str());

  Serial.print("LCD Start");
  tft.initR(INITR_BLACKTAB);
  Serial.println("Initializing...");
  uint16_t time = millis();
  tft.fillScreen(ST7735_BLACK);
  time = millis() - time;
  Serial.println(time, DEC);
  delay(500); 
  //tft.fillScreen(ST7735_BLACK); //large block of text
  //testdrawtext("Lorem ipsum dolor sit amet, consectetur adipiscing elit. Curabitur adipiscing ante sed nibh tincidunt feugiat. Maecenas enim massa, fringilla sed malesuada et, malesuada sit amet turpis. Sed porttitor neque ut ante pretium vitae malesuada nunc bibendum. Nullam aliquet ultrices massa eu hendrerit. Ut sed nisi lorem. In vestibulum purus a tortor imperdiet posuere. ", ST7735_WHITE);
  delay(1000);
  tft.setRotation(1);
  testdrawtext("Vegova", ST7735_RED, 4, 10, 0);
 
  Serial.println(F("------------------------------------"));
  // Set delay between sensor readings based on sensor details.

  strcpy(ssid, (preferences.getString("ssid", "K8")).c_str());
  strcpy(password, (preferences.getString("password", "dogsminusone")).c_str());
  strcpy(clientID, (preferences.getString("clientID", "ESP32 Unregistered")).c_str());
  
  
  client.setServer(mqtt_server, 1883);
  client.setCallback(mqtt_callback);
  

  while (WiFi.status() != WL_CONNECTED) {
    Serial.println("Could not connect searching for default...");
    WiFi.begin(default_ssid, default_password);
    delay(10000);
  }
 if(WiFi.status() == WL_CONNECTED)
    {
      //digitalWrite(BUILTIN_LED, HIGH);
      Serial.println("Connected to the WiFi network");
      Serial.print("Ip: ");
      Serial.println(WiFi.localIP()); 
      tft.fillScreen(ST7735_BLACK);
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
    if (client.connect("ESP32 UNIT 1", "","")) {
      Serial.println("connected");
      // subscribe topic with default QoS 0
      client.subscribe("LED_1");
      client.subscribe("FastLED_1");
      client.subscribe("SSID_1");
      client.subscribe("PASS_1");
      client.subscribe("CLIENTID_1");
      client.subscribe("Besedilo_1");
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

    if(String(topic)=="LED_1")
    {
      if(messageTemp == "on"){
        digitalWrite(2, HIGH);
      }
       else if(messageTemp == "off"){
        digitalWrite(2, LOW);
       }
    }
    
    if(String(topic)=="FastLED_1")
    {
      messageTemp.remove(0,1);
      for(int i =0; i<NUM_LEDS;i++)
        {
          leds[i] = (int)strtol(messageTemp.c_str(), NULL,16);
        }
        FastLED.show();
    }
    
    if(String(topic) =="SSID_1")
    {
      strcpy(ssid, messageTemp.c_str());
      preferences.putString("ssid", messageTemp);
      Serial.print("Updated Wifi ssid: ");
      Serial.println(ssid);
    }
    if(String(topic) =="PASS_1")
    {
      strcpy(password, messageTemp.c_str());
      preferences.putString("password", messageTemp);
      Serial.print("Updated Wifi password");
      Serial.println(password);
      ESP.restart();
    }
    
    if(String(topic) =="CLIENTID_1")
    {
      strcpy(clientID, messageTemp.c_str());
      preferences.putString("clientID", messageTemp);
      Serial.print("Updated clientID: ");
      Serial.println(clientID);
      ESP.restart();
    }

    if(String(topic) =="Besedilo_1")
    {
      tft.fillScreen(ST7735_BLACK);
      testdrawtext(messageTemp, ST7735_RED, 2, 0, 0);
    }

    
      
}
unsigned long int timeSensor = millis();
void readTemp()
{
  
}

void printValues() {
    //Serial.print("Temperature = ");
    client.publish("Temperature_1", String(bme.readTemperature()).c_str());
    //Serial.println(" *C");

    //Serial.print("Pressure = ");

    //Serial.print(bme.readPressure() / 100.0F);
    client.publish("Pressure_1", String(bme.readPressure()/100.0F).c_str());
    //Serial.println(" hPa");

    //Serial.print("Approx. Altitude = ");
    //Serial.print(bme.readAltitude(SEALEVELPRESSURE_HPA));
    client.publish("Altitude_1", String(bme.readAltitude(SEALEVELPRESSURE_HPA)).c_str());
    //Serial.println(" m");

    //Serial.print("Humidity = ");
    //Serial.print(bme.readHumidity());
    client.publish("Humidity_1", String(bme.readHumidity()).c_str());
    //Serial.println(" %");

    //Serial.println();
}

int vdelaymesure = millis();

void testdrawtext(String text, uint16_t color, int txtsize, int cx, int cy) {
  tft.setCursor(cx, cy);
  tft.setTextSize(txtsize);
  tft.setTextColor(color, ST7735_BLACK);
  tft.setTextWrap(true);
  tft.print(text);
}

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
