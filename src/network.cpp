#include <Arduino.h>
#include <WiFi.h>
#include "esp_wifi.h"
#include "controller.h"
#include "config.h"
#include "secrets.h"
#include "network.h"
#include "websocket.h"

extern Controller controller;
WebSocketClient webSocketClient;

void initWiFi() {
    Serial.printf("Connecting to WiFi %s\n\r", WIFI_SSID);
    controller.setWifiSignal(0);
    webSocketClient.configure(HA_HOST, HA_PORT, HA_TOKEN, &controller);

    WiFi.mode(WIFI_STA);
    esp_wifi_set_ps(WIFI_PS_NONE);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
   
    static unsigned long WiFiConnectionTimeStarted = millis();
  
    while (!WiFi.isConnected()) {
        // connecting.. 

        if (millis() - WiFiConnectionTimeStarted > WIFI_CONN_TIMEOUT) {
            Serial.printf("WiFI connection failed (code %d)! Rebooting in 5 seconds..", WiFi.waitForConnectResult());
            delay(5000);
            ESP.restart();
            return;
        }
    }

    Serial.printf("Connected to WiFi, IP: %s, Chan: %d\n\r", WiFi.localIP().toString().c_str(), WiFi.channel());
    Serial.printf("RSSI: %d\n\r", WiFi.RSSI());

    controller.setWifiSignal(WiFi.RSSI());

    webSocketClient.setNetworkAvailable(true);
    webSocketClient.init();
}

void checkWiFi() {
    static unsigned long lastCheck = 0;
    if (millis() - lastCheck > WIFI_CONN_CHECK_FREQ) {
        lastCheck = millis();
        if (WiFi.status() == WL_CONNECTED) {
            // WiFi is connected
            controller.setWifiSignal(WiFi.RSSI());

            webSocketClient.check();
        } else {
            // WiFi is disconnected
            Serial.println("WiFi disconnected! Reconnecting...");

            controller.setWifiSignal(0);
            controller.setWebsocketStatus(false);
            
            webSocketClient.destroy();
            webSocketClient.setNetworkAvailable(false);
            
            initWiFi();
        }
    }
}
