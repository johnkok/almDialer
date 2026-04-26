#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <LittleFS.h>
#include <ArduinoJson.h>
#include "config.h"
#include "wifi_ap.h"
#include "gpio_monitor.h"
#include "email_client.h"
#include "web_server.h"

void setup() {
    Serial.begin(115200);
    delay(1000);
    
    Serial.println("\n\nESP8266 Email Alert System Starting...");
    
    if (!LittleFS.begin()) {
        Serial.println("LittleFS Mount Failed");
        return;
    }
    Serial.println("LittleFS Mounted");
    
    Config::load();
    WiFiAP::setup();
    GPIOMonitor::setup();
    EmailClient::setup();
    WebServer::setup();
    
    Serial.println("Setup Complete");
}

void loop() {
    WiFiAP::handle();
    WebServer::handle();
    GPIOMonitor::handle();
    EmailClient::handle();
    delay(10);
}
