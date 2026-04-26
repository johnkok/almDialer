#include "web_server.h"
#include "config.h"
#include "email_client.h"
#include <ESP8266WebServer.h>
#include <ArduinoJson.h>

ESP8266WebServer server(80);

void WebServer::setup() {
    server.on("/", HTTP_GET, handle_root);
    server.on("/api/config", HTTP_GET, handle_api_config);
    server.on("/api/status", HTTP_GET, handle_api_status);
    server.on("/api/save-config", HTTP_POST, handle_api_save_config);
    server.on("/api/test-email", HTTP_POST, handle_test_email);
    server.begin();
    
    Serial.println("Web Server started on port 80");
}

void WebServer::handle() {
    server.handleClient();
}

void WebServer::handle_root() {
    server.send(200, "text/html", generate_config_html());
}

void WebServer::handle_api_config() {
    server.send(200, "application/json", Config::to_json());
}

void WebServer::handle_api_status() {
    DynamicJsonDocument doc(512);
    doc["status"] = Config::last_email_status;
    doc["last_email_time"] = Config::last_email_time;
    doc["uptime_ms"] = millis();
    doc["free_heap"] = ESP.getFreeHeap();
    
    String json_str;
    serializeJson(doc, json_str);
    server.send(200, "application/json", json_str);
}

void WebServer::handle_api_save_config() {
    if (!server.hasArg("plain")) {
        server.send(400, "application/json", "{\"error\":\"No body\"}");
        return;
    }
    
    String body = server.arg("plain");
    if (Config::from_json(body)) {
        Config::save();
        server.send(200, "application/json", "{\"success\":true}");
    } else {
        server.send(400, "application/json", "{\"error\":\"Invalid JSON\"}");
    }
}

void WebServer::handle_test_email() {
    bool success = EmailClient::send_email("Test Email", "This is a test email from ESP8266 Email Alert System");
    DynamicJsonDocument doc(256);
    doc["success"] = success;
    doc["status"] = EmailClient::get_last_status();
    
    String json_str;
    serializeJson(doc, json_str);
    server.send(200, "application/json", json_str);
}

String WebServer::generate_config_html() {
    return R"(<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>ESP8266 Email Alert Configuration</title>
    <style>
        * { margin: 0; padding: 0; box-sizing: border-box; }
        body { font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, sans-serif; background: linear-gradient(135deg, #667eea 0%, #764ba2 100%); min-height: 100vh; padding: 20px; }
        .container { max-width: 800px; margin: 0 auto; background: white; border-radius: 12px; box-shadow: 0 20px 60px rgba(0,0,0,0.3); overflow: hidden; }
        .header { background: linear-gradient(135deg, #667eea 0%, #764ba2 100%); color: white; padding: 30px 20px; text-align: center; }
        .header h1 { font-size: 28px; margin-bottom: 5px; }
        .content { padding: 30px 20px; }
        .section { margin-bottom: 30px; border-bottom: 1px solid #eee; padding-bottom: 20px; }
        .section:last-child { border-bottom: none; }
        .section-title { font-size: 18px; font-weight: 600; margin-bottom: 15px; color: #333; }
        .form-group { margin-bottom: 15px; }
        label { display: block; margin-bottom: 5px; font-weight: 500; color: #555; font-size: 14px; }
        input[type="text"], input[type="number"], input[type="email"], textarea, select { width: 100%; padding: 10px; border: 1px solid #ddd; border-radius: 6px; font-size: 14px; font-family: inherit; transition: border-color 0.3s; }
        input:focus, textarea:focus, select:focus { outline: none; border-color: #667eea; box-shadow: 0 0 0 3px rgba(102, 126, 234, 0.1); }
        textarea { resize: vertical; min-height: 80px; }
        .checkbox-group { display: flex; align-items: center; gap: 8px; }
        input[type="checkbox"] { width: 18px; height: 18px; cursor: pointer; }
        .form-row { display: grid; grid-template-columns: 1fr 1fr; gap: 15px; }
        .button-group { display: flex; gap: 10px; margin-top: 25px; }
        button { flex: 1; padding: 12px 20px; border: none; border-radius: 6px; font-size: 14px; font-weight: 600; cursor: pointer; transition: all 0.3s; }
        .btn-save { background: linear-gradient(135deg, #667eea 0%, #764ba2 100%); color: white; }
        .btn-save:hover { transform: translateY(-2px); box-shadow: 0 10px 20px rgba(102, 126, 234, 0.3); }
        .btn-test { background: #4CAF50; color: white; }
        .btn-test:hover { background: #45a049; }
        .status { padding: 12px 15px; border-radius: 6px; margin-bottom: 15px; font-size: 13px; display: none; }
        .status.success { background: #d4edda; color: #155724; border: 1px solid #c3e6cb; display: block; }
        .status.error { background: #f8d7da; color: #721c24; border: 1px solid #f5c6cb; display: block; }
        .gpio-card { border: 1px solid #ddd; border-radius: 8px; padding: 15px; margin-bottom: 15px; background: #f9f9f9; }
    </style>
</head>
<body>
    <div class="container">
        <div class="header">
            <h1>📧 ESP8266 Email Alert</h1>
            <p>Web-Based Configuration</p>
        </div>
        <div class="content">
            <div id="status" class="status"></div>
            <form id="configForm">
                <div class="section">
                    <h2 class="section-title">SMTP Email Server</h2>
                    <div class="form-group"><label>SMTP Host</label><input type="text" id="smtpHost" placeholder="smtp.gmail.com"></div>
                    <div class="form-row"><div class="form-group"><label>SMTP Port</label><input type="number" id="smtpPort" value="587"></div><div class="form-group"><label><div class="checkbox-group"><input type="checkbox" id="useTls"><span>Use TLS</span></div></label></div></div>
                    <div class="form-group"><label>Username</label><input type="text" id="smtpUsername" placeholder="your-email@gmail.com"></div>
                    <div class="form-group"><label>Password</label><input type="text" id="smtpPassword" placeholder="your-app-password"></div>
                    <div class="form-group"><label>From Email</label><input type="email" id="fromEmail" placeholder="sender@gmail.com"></div>
                    <div class="form-group"><label>To Email</label><input type="email" id="toEmail" placeholder="recipient@example.com"></div>\n                    <button type="button" class="btn-test" onclick="testEmail()\">🧪 Test Email</button>\n                </div>\n                <div class="section">\n                    <h2 class="section-title\">GPIO Monitoring (up to 4)</h2>\n                    <div id="gpioContainer\"></div>\n                </div>\n                <div class="section\">\n                    <h2 class="section-title\">Alive Email (Heartbeat)</h2>\n                    <div class=\"form-group\"><label><div class=\"checkbox-group\"><input type=\"checkbox\" id=\"aliveEnabled\"><span>Enable Alive Email</span></div></label></div>\n                    <div class=\"form-group\"><label>Interval (milliseconds)</label><input type=\"number\" id=\"aliveInterval\" value=\"604800000\" placeholder=\"604800000 (7 days)\"></div>\n                    <div class=\"form-group\"><label>Message</label><textarea id=\"aliveMessage\" placeholder=\"System Alive - ESP8266 Email Alert\"></textarea></div>\n                </div>\n                <div class=\"button-group\"><button type=\"submit\" class=\"btn-save\">💾 Save Configuration</button></div>\n            </form>\n        </div>\n    </div>\n    <script>\n        document.addEventListener('DOMContentLoaded', loadConfiguration);\n        document.getElementById('configForm').addEventListener('submit', saveConfiguration);\n        async function loadConfiguration() {\n            const response = await fetch('/api/config');\n            const data = await response.json();\n            document.getElementById('smtpHost').value = data.smtp.host;\n            document.getElementById('smtpPort').value = data.smtp.port;\n            document.getElementById('useTls').checked = data.smtp.use_tls;\n            document.getElementById('smtpUsername').value = data.smtp.username;\n            document.getElementById('smtpPassword').value = data.smtp.password;\n            document.getElementById('fromEmail').value = data.smtp.from_email;\n            document.getElementById('toEmail').value = data.smtp.to_email;\n            const gpioContainer = document.getElementById('gpioContainer');\n            gpioContainer.innerHTML = '';\n            for (let i = 0; i < 4; i++) {\n                const gpio = data.gpio[i];\n                gpioContainer.innerHTML += '<div class=\"gpio-card\"><label><input type=\"checkbox\" id=\"gpio' + i + 'Enabled\">' + (i + 1) + '. ' + gpio.label + '</label><input type=\"text\" id=\"gpio' + i + 'Label\" placeholder=\"Label\"><label><input type=\"checkbox\" id=\"gpio' + i + 'ActiveHigh\"> Active High</label><textarea id=\"gpio' + i + 'Message\" placeholder=\"Message\"></textarea></div>';\n                document.getElementById('gpio' + i + 'Enabled').checked = gpio.enabled;\n                document.getElementById('gpio' + i + 'Label').value = gpio.label;\n                document.getElementById('gpio' + i + 'ActiveHigh').checked = gpio.active_high;\n                document.getElementById('gpio' + i + 'Message').value = gpio.message;\n            }\n            document.getElementById('aliveEnabled').checked = data.alive.enabled;\n            document.getElementById('aliveInterval').value = data.alive.interval_ms;\n            document.getElementById('aliveMessage').value = data.alive.message;\n        }\n        async function saveConfiguration(e) {\n            e.preventDefault();\n            const config = { smtp: {}, gpio: [], alive: {} };\n            config.smtp.host = document.getElementById('smtpHost').value;\n            config.smtp.port = parseInt(document.getElementById('smtpPort').value);\n            config.smtp.username = document.getElementById('smtpUsername').value;\n            config.smtp.password = document.getElementById('smtpPassword').value;\n            config.smtp.use_tls = document.getElementById('useTls').checked;\n            config.smtp.from_email = document.getElementById('fromEmail').value;\n            config.smtp.to_email = document.getElementById('toEmail').value;\n            for (let i = 0; i < 4; i++) {\n                config.gpio.push({ enabled: document.getElementById('gpio' + i + 'Enabled').checked, active_high: document.getElementById('gpio' + i + 'ActiveHigh').checked, label: document.getElementById('gpio' + i + 'Label').value, message: document.getElementById('gpio' + i + 'Message').value });\n            }\n            config.alive.enabled = document.getElementById('aliveEnabled').checked;\n            config.alive.interval_ms = parseInt(document.getElementById('aliveInterval').value);\n            config.alive.message = document.getElementById('aliveMessage').value;\n            const response = await fetch('/api/save-config', { method: 'POST', headers: { 'Content-Type': 'application/json' }, body: JSON.stringify(config) });\n            if (response.ok) { document.getElementById('status').textContent = 'Saved!'; document.getElementById('status').className = 'status success'; } else { document.getElementById('status').textContent = 'Error'; document.getElementById('status').className = 'status error'; }\n        }\n        async function testEmail() {\n            const response = await fetch('/api/test-email', { method: 'POST' });\n            const data = await response.json();\n            if (data.success) { document.getElementById('status').textContent = 'Test email sent!'; document.getElementById('status').className = 'status success'; } else { document.getElementById('status').textContent = 'Error: ' + data.status; document.getElementById('status').className = 'status error'; }\n        }\n    </script>\n</body>\n</html>)";
}
