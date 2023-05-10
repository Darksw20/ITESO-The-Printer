#include "secrets.h"
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "WiFi.h"
 
#define PRINTER_ID 2
 
#define AWS_IOT_PUBLISH_TOPIC   "ESP_32/orders/status"
#define AWS_IOT_SUBSCRIBE_TOPIC "ESP_32/orders"
 
 
WiFiClientSecure net = WiFiClientSecure();
PubSubClient client(net);
 
void connectAWS()
{
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
 
  Serial.println("Connecting to Wi-Fi");
 
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
 
  // Configure WiFiClientSecure to use the AWS IoT device credentials
  net.setCACert(AWS_CERT_CA);
  net.setCertificate(AWS_CERT_CRT);
  net.setPrivateKey(AWS_CERT_PRIVATE);
 
  // Connect to the MQTT broker on the AWS endpoint we defined earlier
  client.setServer(AWS_IOT_ENDPOINT, 8883);
 
  // Create a message handler
  client.setCallback(messageHandler);
 
  Serial.println("Connecting to AWS IOT");
 
  while (!client.connect(THINGNAME))
  {
    Serial.print(".");
    delay(100);
  }
 
  if (!client.connected())
  {
    Serial.println("AWS IoT Timeout!");
    return;
  }
 
  // Subscribe to a topic
  client.subscribe(AWS_IOT_SUBSCRIBE_TOPIC);
 
  Serial.println("AWS IoT Connected!");
}
 
void publishMessage()
{
  StaticJsonDocument<200> doc;
  doc["printer"] = PRINTER_ID;
  doc["status"] = "ok";     // Revisa si ya se imprimio waiting-printing-served
  char jsonBuffer[512];
  serializeJson(doc, jsonBuffer); // print to client
 
  client.publish(AWS_IOT_PUBLISH_TOPIC, jsonBuffer);
}
 
void messageHandler(char* topic, byte* payload, unsigned int length)
{
  Serial.print("incoming: ");
  Serial.println(topic);
 
  StaticJsonDocument<200> doc;
  deserializeJson(doc, payload);
  const char* printer = doc["printer"];
  JsonObject steps = doc["steps"].as<JsonObject>();     
  Serial.println(printer);
//  Serial.println(steps[0]);
  for (int i = 0; i < steps.size(); i++) {
    const char* textValue = steps["textValue"].as<const char*>();
    Serial.println(textValue);
  }
  

 /* for (int i = 0; i < steps.size(); i++) {
    int val = steps[i].as<int>(); 
    steps[i] = val * 2;
  }
 */
  serializeJson(doc, Serial); // imprimir

}
 
void setup()
{
  Serial.begin(115200);
  connectAWS();
}
 
void loop()
{
  publishMessage();
  client.loop();
  delay(10000);
}
