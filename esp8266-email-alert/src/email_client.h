#ifndef EMAIL_CLIENT_H
#define EMAIL_CLIENT_H

#include <Arduino.h>

class EmailClient {
public:
    static void setup();
    static void handle();
    static void send_gpio_alert(int gpio_index);
    static void send_alive_alert();
    static bool send_email(const String& subject, const String& body);
    static String get_last_status();
    
private:
    static bool connect_smtp();
    static void disconnect_smtp();
    static bool send_smtp_command(const String& command, const String& expected_response);
    static String read_smtp_response();
    static unsigned long last_alive_check;
};

#endif // EMAIL_CLIENT_H
