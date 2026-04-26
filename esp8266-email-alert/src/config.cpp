#include "config.h"

#include <ArduinoJson.h>
#include <LittleFS.h>

namespace {
const char* kConfigPath = "/config.json";
const uint8_t kDefaultPins[4] = {D1, D2, D5, D6};
const char* kDefaultLabels[4] = {"GPIO 1", "GPIO 2", "GPIO 3", "GPIO 4"};
}

WiFiStationConfig Config::wifi_station_config;
SMTPConfig Config::smtp_config;
GPIOConfig Config::gpio_config[4];
AliveConfig Config::alive_config;
String Config::last_email_status = "Idle";
unsigned long Config::last_email_time = 0;

void Config::set_defaults() {
    wifi_station_config = WiFiStationConfig();
    smtp_config = SMTPConfig();
    alive_config = AliveConfig();
    last_email_status = "Idle";
    last_email_time = 0;

    for (int i = 0; i < 4; ++i) {
        gpio_config[i] = GPIOConfig();
        gpio_config[i].pin = kDefaultPins[i];
        gpio_config[i].label = kDefaultLabels[i];
        gpio_config[i].message = String("Alert triggered on ") + kDefaultLabels[i];
    }
}

void Config::load() {
    set_defaults();

    File file = LittleFS.open(kConfigPath, "r");
    if (!file) {
        Serial.println("Config file not found, using defaults");
        save();
        return;
    }

    String json = file.readString();
    file.close();

    if (!from_json(json)) {
        Serial.println("Config parse failed, using defaults");
        set_defaults();
        save();
    }
}

bool Config::save() {
    File file = LittleFS.open(kConfigPath, "w");
    if (!file) {
        Serial.println("Failed to open config file for writing");
        return false;
    }

    file.print(to_json());
    file.close();
    return true;
}

String Config::to_json() {
    DynamicJsonDocument doc(3072);
    doc["wifi"]["enabled"] = wifi_station_config.enabled;
    doc["wifi"]["ssid"] = wifi_station_config.ssid;
    doc["wifi"]["password"] = wifi_station_config.password;
    doc["wifi"]["hostname"] = wifi_station_config.hostname;
    doc["wifi"]["ap_ssid"] = wifi_station_config.ap_ssid;
    doc["wifi"]["ap_password"] = wifi_station_config.ap_password;

    doc["smtp"]["host"] = smtp_config.host;
    doc["smtp"]["port"] = smtp_config.port;
    doc["smtp"]["use_tls"] = smtp_config.use_tls;
    doc["smtp"]["username"] = smtp_config.username;
    doc["smtp"]["password"] = smtp_config.password;
    doc["smtp"]["from_email"] = smtp_config.from_email;
    doc["smtp"]["to_email"] = smtp_config.to_email;

    JsonArray gpio = doc.createNestedArray("gpio");
    for (int i = 0; i < 4; ++i) {
        JsonObject entry = gpio.createNestedObject();
        entry["enabled"] = gpio_config[i].enabled;
        entry["active_high"] = gpio_config[i].active_high;
        entry["pin"] = gpio_config[i].pin;
        entry["label"] = gpio_config[i].label;
        entry["message"] = gpio_config[i].message;
    }

    doc["alive"]["enabled"] = alive_config.enabled;
    doc["alive"]["interval_ms"] = alive_config.interval_ms;
    doc["alive"]["message"] = alive_config.message;
    doc["alive"]["last_sent_time"] = alive_config.last_sent_time;

    doc["meta"]["last_email_status"] = last_email_status;
    doc["meta"]["last_email_time"] = last_email_time;

    String json;
    serializeJson(doc, json);
    return json;
}

bool Config::from_json(const String& json) {
    DynamicJsonDocument doc(3072);
    DeserializationError err = deserializeJson(doc, json);
    if (err) {
        return false;
    }

    JsonObject wifi = doc["wifi"];
    if (!wifi.isNull()) {
        wifi_station_config.enabled = wifi["enabled"] | wifi_station_config.enabled;
        wifi_station_config.ssid = wifi["ssid"] | wifi_station_config.ssid;
        wifi_station_config.password = wifi["password"] | wifi_station_config.password;
        wifi_station_config.hostname = wifi["hostname"] | wifi_station_config.hostname;
        wifi_station_config.ap_ssid = wifi["ap_ssid"] | wifi_station_config.ap_ssid;
        wifi_station_config.ap_password = wifi["ap_password"] | wifi_station_config.ap_password;
    }

    JsonObject smtp = doc["smtp"];
    if (!smtp.isNull()) {
        smtp_config.host = smtp["host"] | smtp_config.host;
        smtp_config.port = smtp["port"] | smtp_config.port;
        smtp_config.use_tls = smtp["use_tls"] | smtp_config.use_tls;
        smtp_config.username = smtp["username"] | smtp_config.username;
        smtp_config.password = smtp["password"] | smtp_config.password;
        smtp_config.from_email = smtp["from_email"] | smtp_config.from_email;
        smtp_config.to_email = smtp["to_email"] | smtp_config.to_email;
    }

    JsonArray gpio = doc["gpio"];
    if (!gpio.isNull()) {
        for (int i = 0; i < 4 && i < static_cast<int>(gpio.size()); ++i) {
            JsonObject entry = gpio[i];
            gpio_config[i].enabled = entry["enabled"] | gpio_config[i].enabled;
            gpio_config[i].active_high = entry["active_high"] | gpio_config[i].active_high;
            gpio_config[i].pin = entry["pin"] | gpio_config[i].pin;
            gpio_config[i].label = entry["label"] | gpio_config[i].label;
            gpio_config[i].message = entry["message"] | gpio_config[i].message;
        }
    }

    JsonObject alive = doc["alive"];
    if (!alive.isNull()) {
        alive_config.enabled = alive["enabled"] | alive_config.enabled;
        alive_config.interval_ms = alive["interval_ms"] | alive_config.interval_ms;
        alive_config.message = alive["message"] | alive_config.message;
        alive_config.last_sent_time = alive["last_sent_time"] | alive_config.last_sent_time;
    }

    JsonObject meta = doc["meta"];
    if (!meta.isNull()) {
        last_email_status = meta["last_email_status"] | last_email_status;
        last_email_time = meta["last_email_time"] | last_email_time;
    }

    return true;
}

bool Config::factory_reset() {
    set_defaults();
    return save();
}
