#include "password.h"

#include <Arduino.h>
#include <PubSubClient.h>

#include <TFT_eSPI.h>
#include <SPI.h>
#include <WiFi.h>
#include <Wire.h>
#include <Button2.h>

#ifndef TFT_DISPOFF
#define TFT_DISPOFF 0x28
#endif

#ifndef TFT_SLPIN
#define TFT_SLPIN 0x10
#endif

#define ADC_EN 14
#define ADC_PIN 34
#define BUTTON_1 35
#define BUTTON_2 0

String content;
void onReceived(char *topic, byte *payload, unsigned int length){
    Serial.print("Received on ");
    Serial.print(topic);
    Serial.print(": ");

    content = "";
    for (size_t i = 0; i < length; i++){
        content.concat((char)payload[i]);
    }

    Serial.print(content);
    Serial.println();
}

WiFiClient esp_client;
PubSubClient mqtt_client(esp_client);
TFT_eSPI tft = TFT_eSPI(135, 240);

void setup(){
    Serial.begin(115200);

    tft.init();
    tft.fontHeight(2);
    tft.setRotation(1);
    tft.setTextColor(TFT_GREEN);

    WiFi.begin(wifi_ssid, wifi_password);
    while (WiFi.status() != WL_CONNECTED){
        delay(100);
        Serial.print('.');
    }
    Serial.println(WiFi.localIP());

    mqtt_client.setServer("192.168.1.17", 1883);
    mqtt_client.setCallback(onReceived);
    mqtt_client.subscribe("my_topic");

    while (!mqtt_client.connected()){
        Serial.print("Starting MQTT connection...");
        if (!mqtt_client.connect("my_esp32")){
            Serial.print("Failed MQTT connection, rc=");
            Serial.print(mqtt_client.state());
            Serial.println(" try again in 5 seconds");
            delay(5000);
        }
        else{
            Serial.println(" Connected to broker!");
        }
    }
    mqtt_client.subscribe("my_topic");
}

void loop(){
    static int counter = 1;
    String message = "Hello: ";
    message += String(counter++);
    mqtt_client.publish("my_counter", message.c_str());

    tft.fillScreen(TFT_BLACK);
    tft.drawString("This is my IP:", 10, 10, 4);
    tft.drawString(WiFi.localIP().toString().c_str(), 10, 40, 4);
    tft.drawString(("Received: " + content).c_str(), 10, 70, 4);

    mqtt_client.loop(); // esto tiene que rular rapidito
    delay(1000);
}
