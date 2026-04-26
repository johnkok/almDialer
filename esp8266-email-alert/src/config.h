#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

struct SMTPConfig {
    String host;
    uint16_t port = 587;
    bool use_tls = true;
    String username;
    String password;
    String from_email;
    String to_email;
};

struct WiFiStationConfig {
    bool enabled = false;
    String ssid;
    String password;
    String hostname = "esp8266-email-alert";
    String ap_ssid = "ESP8266-Email-Alert";
    String ap_password = "esp8266setup";
};

struct GPIOConfig {
    bool enabled = false;
    bool active_high = true;
    uint8_t pin = 255;
    String label;
    String message;
    bool last_state = false;
};

struct AliveConfig {
    bool enabled = false;
    unsigned long interval_ms = 604800000UL;
    String message = "System Alive - ESP8266 Email Alert";
    unsigned long last_sent_time = 0;
};

class Config {
public:
    static WiFiStationConfig wifi_station_config;
    static SMTPConfig smtp_config;
    static GPIOConfig gpio_config[4];
    static AliveConfig alive_config;
    static String last_email_status;
    static unsigned long last_email_time;

    static void load();
    static bool save();
    static String to_json();
    static bool from_json(const String& json);
    static bool factory_reset();

private:
    static void set_defaults();
};

#endif
