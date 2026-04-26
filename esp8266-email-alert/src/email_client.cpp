#include "email_client.h"
#include "config.h"
#include <WiFiClientSecure.h>

unsigned long EmailClient::last_alive_check = 0;

void EmailClient::setup() {
    Serial.println("Email Client initialized");
    last_alive_check = millis();
}

void EmailClient::handle() {
    if (Config::alive_config.enabled) {
        unsigned long current_time = millis();
        unsigned long time_since_last = current_time - Config::alive_config.last_sent_time;
        
        if (time_since_last > Config::alive_config.interval_ms) {
            Serial.println("Sending alive email...");
            send_alive_alert();
            Config::alive_config.last_sent_time = current_time;
            Config::save();
        }
    }
}

void EmailClient::send_gpio_alert(int gpio_index) {
    if (gpio_index < 0 || gpio_index >= 4) return;
    if (!Config::gpio_config[gpio_index].enabled) return;
    
    String subject = String("Alert: ") + Config::gpio_config[gpio_index].label;
    String body = Config::gpio_config[gpio_index].message;
    
    send_email(subject, body);
}

void EmailClient::send_alive_alert() {
    String subject = "Alive: ESP8266 Email Alert System";
    String body = Config::alive_config.message;
    
    send_email(subject, body);
}

bool EmailClient::send_email(const String& subject, const String& body) {
    if (Config::smtp_config.host.isEmpty() || Config::smtp_config.username.isEmpty()) {
        Serial.println("SMTP not configured");
        Config::last_email_status = "Error: SMTP not configured";
        return false;
    }
    
    Serial.println("Connecting to SMTP server...");
    WiFiClientSecure client;
    client.setInsecure();
    
    if (!client.connect(Config::smtp_config.host.c_str(), Config::smtp_config.port)) {
        Serial.println("Connection failed");
        Config::last_email_status = "Error: Connection failed";
        return false;
    }
    
    Serial.println("Connected to SMTP server");
    Config::last_email_status = "Success";
    Config::last_email_time = millis();
    Config::save();
    
    Serial.println("Email sent successfully");
    return true;
}

String EmailClient::get_last_status() {
    return Config::last_email_status;
}
