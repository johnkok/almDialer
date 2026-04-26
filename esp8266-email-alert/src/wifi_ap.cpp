#include "wifi_ap.h"

#include <Arduino.h>
#include <ESP8266WiFi.h>

#include "config.h"

void WiFiAP::setup() {
    WiFi.mode(WIFI_AP_STA);
    WiFi.hostname(Config::wifi_station_config.hostname);

    String ap_ssid = Config::wifi_station_config.ap_ssid;
    if (ap_ssid.isEmpty()) {
        ap_ssid = "ESP8266-Email-Alert";
    }

    String ap_password = Config::wifi_station_config.ap_password;
    if (ap_password.length() < 8) {
        ap_password = "esp8266setup";
    }

    WiFi.softAP(ap_ssid.c_str(), ap_password.c_str());

    Serial.print("WiFi AP started, IP: ");
    Serial.println(WiFi.softAPIP());
    Serial.print("WiFi AP SSID: ");
    Serial.println(ap_ssid);

    if (Config::wifi_station_config.enabled && !Config::wifi_station_config.ssid.isEmpty()) {
        Serial.print("Connecting to WiFi station: ");
        Serial.println(Config::wifi_station_config.ssid);

        WiFi.begin(
            Config::wifi_station_config.ssid.c_str(),
            Config::wifi_station_config.password.c_str()
        );

        unsigned long start = millis();
        while (WiFi.status() != WL_CONNECTED && millis() - start < 15000) {
            delay(250);
            Serial.print(".");
        }
        Serial.println();

        if (WiFi.status() == WL_CONNECTED) {
            Serial.print("WiFi station connected, IP: ");
            Serial.println(WiFi.localIP());
        } else {
            Serial.println("WiFi station connection failed, AP remains available");
        }
    }
}

void WiFiAP::handle() {
}
