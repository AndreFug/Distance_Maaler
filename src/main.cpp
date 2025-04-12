/*************************************************
  Distance sensor with MQTT publish.
*************************************************/

#include <WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#include "Adafruit_VL6180X.h"

// Replace with Wi-Fi credentials
const char* ssid = "WIFI_SSID";
const char* password = "WIFI_PASSWORD";

// Replace with MQTT broker settings
const char* mqttServer = "YOUR_MQTT_BROKER_ADDRESS";  
const int   mqttPort   = 1883;                    
const char* mqttUser   = "YOUR_MQTT_USERNAME";      
const char* mqttPass   = "YOUR_MQTT_PASSWORD";      

// Create an instance of the VL6180 class
Adafruit_VL6180X vl = Adafruit_VL6180X();

// Create a WiFi and MQTT client
WiFiClient espClient;
PubSubClient client(espClient);

long lastMsg = 0;       // Track the last time we sent a message
int interval = 2000;    // Publish interval (milliseconds)

// Forward declaration of functions
void setup_wifi();
void reconnect();

void setup() {
  Serial.begin(115200);
  delay(100);

  // Initialize the I2C bus
  Wire.begin(13, 12); // 13 for SDA and 12 for SCL

  // Initialize sensor
  if (!vl.begin()) {
    Serial.println("Failed to find VL6180X sensor. Check wiring!");
    while (1) {
      delay(10);
    }
  }
  Serial.println("VL6180X sensor found!");

  // Set up WiFi
  setup_wifi();

  // Set MQTT server details
  client.setServer(mqttServer, mqttPort);
}

void loop() {
  // while (true){
  //   Serial.println("Test");    // Used for testing
  // }
  // Reconnect to MQTT if disconnected
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  // Publish distance readings at regular intervals
  long now = millis();
  if (now - lastMsg > interval) {
    lastMsg = now;

    // Read distance in millimeters
    uint8_t range = vl.readRange();

    // Check for errors
    uint8_t status = vl.readRangeStatus();
    if (status == VL6180X_ERROR_NONE) {
      // Convert to a string for MQTT
      String distanceStr = String((int)range);

      // Print to serial for debugging
      Serial.print("Distance: ");
      Serial.print(distanceStr);
      Serial.println(" mm");

      // Publish to MQTT topic
      client.publish("Distance_sensor1", distanceStr.c_str());
    } else {
      // Some error in measurement
      Serial.print("Distance error. Status: ");
      Serial.println(status);
    }
  }
}

// Connect to Wi-Fi
void setup_wifi() {
  delay(10);
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  // Wait until connected
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

// Reconnect to MQTT if the connection is lost
void reconnect() {
  // Loop until we’re reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");

    // Attempt to connect
    if (client.connect("ESP32S3Client", mqttUser, mqttPass)) {
      Serial.println("connected!");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" Trying again in 5 seconds...");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}
