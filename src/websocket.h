#pragma once

#include "controller.h"
#include <Arduino.h>
#include <ArduinoJson.h>
#include <esp_websocket_client.h>

#define NO_DATA_TIMEOUT_SEC 10

class WebSocketClient {
    private:
        String token;
        esp_websocket_client_config_t config = {};
        esp_websocket_client_handle_t client = nullptr;

        bool isInitialised = false;
        bool isNetworkAvailable = false;

        Controller *controller;

        unsigned long messageId = 2;
        String messageBuffer;
        uint32_t expectedMessageLength = 0;

        int lastReportedPowerSensorAvailable = -1;

        void send(JsonDocument message);
        void sendAuth();
        void sendRenderInitialData();
        void checkPowerSensor();
        void checkPowerSensor(bool lastPowerSensor);
        void sendPowerSensor(bool state);
        void handleState(JsonObject result);

    protected:
       
    public:
        WebSocketClient();
        void configure(String host, int port, String token, Controller *controller);
        void setNetworkAvailable(bool status);
        void init();
        void check();
        void destroy();
        void eventHandler(void *args, esp_event_base_t base, int32_t id, void *data);
        void messageHandler(String payload);

};

