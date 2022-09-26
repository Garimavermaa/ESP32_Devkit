#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include "Esp32MQTTClient.h"
#include "DHT.h"

const char* ssid = "T5211"; // Enter your WiFi name
const char* password =  "123456789"; // Enter WiFi password
const char* brokerUser = "garima123";
const char* brokerPass = "123456789";
const char* broker = "broker.mqttdashboard.com";
const char* outTopic = "/garima.verma/out";
const char* inTopic = "/garima.verma/in";
String MQTTTopic;
String MQTTPayload;
char messages[50];

#define DHTPIN 4
#define DHTTYPE DHT11   // DHT 11
DHT dht(DHTPIN, DHTTYPE);

//Azure IOT Hub Setup
static const char* connectionString = "HostName=iothubpn1.azure-devices.net;DeviceId=devkit;SharedAccessKey=CPUyTHNEdEPirg5zu05SaVxrSj4jqzTYjmH65641cLQ=";
static bool hasIoTHub = false;

WiFiClient espClient;
PubSubClient client(espClient);


void setup_wifi() {
    delay(10);
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(ssid);
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }
    randomSeed(micros());
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) 
  {
    Serial.print("\nConnecting to");
    Serial.println(broker);
    if(client.connect("koikoikoi", brokerUser, brokerPass))
    {
      Serial.print("\nconnected to");
      Serial.println(broker);
      // Once connected, publish an announcement...
      client.publish("publish_by_me","i am the client");
      client.subscribe(inTopic);
      client.subscribe(outTopic);
    } 
    else 
    {
      Serial.println("\nTrying Connect again");
      delay(5000);
    }
  }
}
void callback(char* topic, byte *payload, unsigned int length) 
{
    Serial.println("Recived message");
//    Serial.println(topic);
    MQTTTopic = String(topic);
    MQTTPayload = "";
    for(int i=0; i<length; i++){
//      Serial.print((char) payload[i]);
      MQTTPayload = String(MQTTPayload + (char)payload[i]);
    }
    Serial.println(); 
}

void setup() 
{
  // put your setup code here, to run once:
  Serial.begin(115200);
  setup_wifi();
  client.setServer(broker, 1883);
  client.setCallback(callback);
  
  Serial.println(F("DHTxx test!"));
  dht.begin();
  //Connect to Azure IOT
  if (!Esp32MQTTClient_Init((const uint8_t*)connectionString))
  {
    hasIoTHub = false;
    Serial.println("Azure IoT Hub : Initializing IoT hub failed.");
    return;
  }
  hasIoTHub = true;
  Serial.println("hastoiot true");
}


void loop() {
  if(!client.connected()){
    reconnect();
  }
  if (MQTTTopic != "") { 
      Serial.println("MQTT : Topic is [" + MQTTTopic +"]");
      Serial.println("MQTT : Payload is [" + MQTTPayload + "]");
      AzureIoTHub(); 
  }
  delay(2000);
  float h = dht.readHumidity();
//  float t = dht.readTemperature();
//  float f = dht.readTemperature(true);

  if (isnan(h)) {
    Serial.println(F("Failed to read from DHT sensor!"));
    return;
  }
  int ret = snprintf(messages, sizeof messages, "%f", h);
  
  Serial.print(F("Humidity: "));
  Serial.print(h);
  client.publish(outTopic,messages);
  client.loop();
}

void AzureIoTHub() {
  if (hasIoTHub)
      {
        String tempString;
        tempString = "{" + MQTTTopic + ":" + MQTTPayload + "}";
        if (Esp32MQTTClient_SendEvent(tempString.c_str()))
        {
          Serial.println("Azure IoT Hub : Sending data to Azure IoT Hub succeed");
        }
        else
        {
          Serial.println("Azure IoT Hub : Failure...");
        }
      MQTTPayload = "";
      MQTTTopic = "";

   }
}
