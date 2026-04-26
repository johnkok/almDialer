#include "gpio_monitor.h"

#include <Arduino.h>

#include "config.h"
#include "email_client.h"

namespace {
const uint8_t kFactoryResetPin = D7;
const unsigned long kFactoryResetHoldMs = 3000;
unsigned long factory_reset_press_start = 0;
bool factory_reset_triggered = false;
}

void GPIOMonitor::setup() {
    pinMode(kFactoryResetPin, INPUT_PULLUP);

    for (int i = 0; i < 4; ++i) {
        if (Config::gpio_config[i].pin == 255) {
            continue;
        }

        pinMode(Config::gpio_config[i].pin, INPUT_PULLUP);
        bool current_state = digitalRead(Config::gpio_config[i].pin);
        Config::gpio_config[i].last_state = current_state;
    }

    Serial.println("GPIO monitor initialized");
}

void GPIOMonitor::handle() {
    bool factory_reset_pressed = digitalRead(kFactoryResetPin) == LOW;
    if (factory_reset_pressed) {
        if (factory_reset_press_start == 0) {
            factory_reset_press_start = millis();
        }

        if (!factory_reset_triggered && millis() - factory_reset_press_start >= kFactoryResetHoldMs) {
            factory_reset_triggered = true;
            Serial.println("Factory reset requested");
            if (Config::factory_reset()) {
                Serial.println("Factory reset saved, restarting");
            } else {
                Serial.println("Factory reset save failed, restarting anyway");
            }
            delay(250);
            ESP.restart();
        }
    } else {
        factory_reset_press_start = 0;
        factory_reset_triggered = false;
    }

    for (int i = 0; i < 4; ++i) {
        if (!Config::gpio_config[i].enabled || Config::gpio_config[i].pin == 255) {
            continue;
        }

        bool current_state = digitalRead(Config::gpio_config[i].pin);
        bool active_state = Config::gpio_config[i].active_high ? HIGH : LOW;

        if (current_state != Config::gpio_config[i].last_state && current_state == active_state) {
            Serial.printf("GPIO alert on pin %u\n", Config::gpio_config[i].pin);
            EmailClient::send_gpio_alert(i);
        }

        Config::gpio_config[i].last_state = current_state;
    }
}
