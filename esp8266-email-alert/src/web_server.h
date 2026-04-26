#ifndef WEB_SERVER_H
#define WEB_SERVER_H

#include <Arduino.h>

class WebServer {
public:
    static void setup();
    static void handle();

private:
    static void handle_root();
    static void handle_api_config();
    static void handle_api_status();
    static void handle_api_save_config();
    static void handle_test_email();
    static String generate_config_html();
};

#endif
