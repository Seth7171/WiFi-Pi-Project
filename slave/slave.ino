#include <WiFi.h>
#include <PubSubClient.h>
#include <DHT11.h>
#include <Adafruit_NeoPixel.h>

// Wi-Fi credentials
const char* ssid = "Ron's iPhone";
const char* password = "00112233";

// MQTT broker details
const char* mqtt_server = "127.20.10.4";
const int mqtt_port = 1883;
const char* temperature_topic = "esp32/temperature";
const char* humidity_topic = "esp32/humidity";
const char* light_topic = "esp32/light";
const char* command_topic = "esp32/command";

unsigned long previous_sent_time = 0;
const long interval = 2000;  

#define DHT11_PIN 26
#define LIGHT_SENSOR_PIN 32
#define RED_LED_PIN 27
#define BLUE_LED_PIN 14
#define RGB_LEDS_PIN 33
#define BUZZER_PIN 25

// State variables for toggling
bool redLedState = false;
bool blueLedState = false;
bool ws2812bState = false;
bool buzzerState = false;

DHT11 dht11(DHT11_PIN);
Adafruit_NeoPixel ws2812b(6, RGB_LEDS_PIN, NEO_GRB + NEO_KHZ800);

WiFiClient espClient;
PubSubClient mqtt_client(espClient);

// Callback function for handling MQTT messages
void callback(char* topic, byte* payload, unsigned int length) {
    String message;
    for (unsigned int i = 0; i < length; i++) {
        message += (char)payload[i];
    }
    Serial.print("Message arrived on topic: ");
    Serial.print(topic);
    Serial.print(". Message: ");
    Serial.println(message);

    if (String(topic) == command_topic) {
        if (message == "TOGGLE_BLUE_LED") {
            blueLedState = !blueLedState;
            digitalWrite(BLUE_LED_PIN, blueLedState ? HIGH : LOW);
        } else if (message == "TOGGLE_RED_LED") {
            redLedState = !redLedState;
            digitalWrite(RED_LED_PIN, redLedState ? HIGH : LOW);
        } else if (message == "TOGGLE_WS2812B_WHITE") {
          ws2812bState = !ws2812bState;
          for (int i = 0; i < 6; i++) {
              ws2812b.setPixelColor(i, ws2812bState ? ws2812b.Color(30, 30, 30) : ws2812b.Color(0, 0, 0));
          }
          ws2812b.show();
        } else if (message == "TOGGLE_BUZZER") {
            buzzerState = !buzzerState;
            digitalWrite(BUZZER_PIN, buzzerState ? HIGH : LOW);
        }
    }
}

void setup() {
    Serial.begin(115200);

    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    Serial.println("");
    Serial.println("Wi-Fi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());


    pinMode(RED_LED_PIN, OUTPUT);
    pinMode(BLUE_LED_PIN, OUTPUT);
    pinMode(RGB_LEDS_PIN, OUTPUT); 
    ws2812b.begin();
    pinMode(BUZZER_PIN, OUTPUT); 

    mqtt_client.setServer(mqtt_server, mqtt_port);
    mqtt_client.setCallback(callback);

  }

// Function to reconnect to MQTT broker
void reconnect() {
    while (!mqtt_client.connected()) {
        Serial.print("Attempting MQTT connection...");
        if (mqtt_client.connect("ESP32Client")) {
            Serial.println("connected");
            mqtt_client.subscribe(command_topic);
        } else {
            Serial.print("failed, rc=");
            Serial.print(mqtt_client.state());
            Serial.println(" try again in 5 seconds");
            delay(5000);
        }
    }
}

void loop() {
    if (!mqtt_client.connected()) {
        reconnect();
    }
    mqtt_client.loop();

    unsigned long currentMillis = millis();
    if (currentMillis - previous_sent_time >= interval) { 
      previous_sent_time = currentMillis;
      int temperature = 0;
      int humidity = 0;
      if (!dht11.readTemperatureHumidity(temperature, humidity)) {
          Serial.print("Temperature: ");
          Serial.print(temperature);
          Serial.print(" °C\tHumidity: ");
          Serial.print(humidity);
          Serial.println(" %");

          // Publish temperature and humidity
          mqtt_client.publish(temperature_topic, String(temperature).c_str());
          mqtt_client.publish(humidity_topic, String(humidity).c_str());
      } else {
          // Print error message based on the error code.
          Serial.println("Error reading dht11");
      }



      int light_value = analogRead(LIGHT_SENSOR_PIN);
      Serial.print("light : ");
      Serial.println(light_value);

      // Publish light intensity
      mqtt_client.publish(light_topic, String(light_value).c_str());
    }

}
