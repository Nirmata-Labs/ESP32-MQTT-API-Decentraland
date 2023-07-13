#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h> // MQTT Library
#include <DHT.h> 

#define DHT_PIN 4              // GPIO pin for DHT11 sensor
#define RAIN_PIN 32            // GPIO pin for rain sensor
#define BRIGHTNESS_PIN 33      // GPIO pin for brightness sensor

//------------------------------------------------------------------
// Configurations
//------------------------------------------------------------------
const char* ssid = "<WiFi Name>";                // SSID of the WiFi network
const char* wifiPassword = "<WiFi Password>";    // WiFi password

const char* mqttServer = "<Server IP e.g., 192.168.178.xx>";
const int mqttPort = "<Port e.g., 1883>";
const char* mqttUser = "<MQTT Username>";
const char* mqttPassword = "<MQTT Password>";
//------------------------------------------------------------------

DHT dht(DHT_PIN, DHT11);
float rain;
float brightness;

WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient); 

void connectToWiFi() {
  Serial.print("Connecting to ");
 
  WiFi.begin(ssid, wifiPassword);
  Serial.println(ssid);

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }

  Serial.println("Connected to WiFi.");
}

void callback(char* topic, byte* payload, unsigned int length) {
  // Process incoming MQTT messages, if necessary
}

void setupMQTT() {
  mqttClient.setServer(mqttServer, mqttPort);
  mqttClient.setCallback(callback);
}

void reconnect() {
  Serial.println("Connecting to MQTT Broker...");
  while (!mqttClient.connected()) {
    Serial.println("Connecting to MQTT Broker...");
    String clientId = "ESP32Client-";
    clientId += String(random(0xffff), HEX);
    
    if (mqttClient.connect(clientId.c_str(), mqttUser, mqttPassword)) {
      Serial.println("Connected to MQTT Broker.");
    }
  }
}

void setup() {
  pinMode(RAIN_PIN, INPUT);
  pinMode(BRIGHTNESS_PIN, INPUT);
  Serial.begin(9600);
  connectToWiFi();
  setupMQTT();
  dht.begin();
}

void loop() {
  delay(2000);
  
  float temperature = dht.readTemperature();   // Read temperature in °C
  float humidity = dht.readHumidity();         // Read humidity in %
  
  if (isnan(temperature) || isnan(humidity)) {
    Serial.println("Error while reading data from DHT sensor!");
    return;
  }
  
  rain = analogRead(RAIN_PIN);                 // Read value from rain sensor
  brightness = analogRead(BRIGHTNESS_PIN);     // Read value from brightness sensor
  
  if (!mqttClient.connected())
    reconnect();

  char data[100];
  
  // Publish data over MQTT
  sprintf(data, "%f", temperature);
  Serial.print("Temperature: ");
  Serial.println(temperature);
  mqttClient.publish("esp32/temperature", data);
  
  sprintf(data, "%f", humidity);
  Serial.print("Humidity: ");
  Serial.println(humidity);
  mqttClient.publish("esp32/humidity", data);
  
  sprintf(data, "%f", rain);
  Serial.print("Rain: ");
  Serial.println(rain);
  mqttClient.publish("esp32/rain", data);
  
  sprintf(data, "%f", brightness);
  Serial.print("Brightness: ");
  Serial.println(brightness);
  mqttClient.publish("esp32/brightness", data);

  mqttClient.loop();
}
