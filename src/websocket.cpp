#include "websocket.h"
#include "esp_event.h"

const char* SENSOR_BATTERY_SOC = "sensor.battery_soc";
const char* SENSOR_GRID_POWER_AVAILABLE = "binary_sensor.grid_power_available";
const char* SENSOR_GRID_POWER_MONITOR = "binary_sensor.grid_power_monitor";
const char* SENSOR_SMART_METER_ACTIVE_POWER = "sensor.smart_meter_active_power";
const char* SENSOR_CURRENT_LOAD_CONSUMPTION = "sensor.current_load_consumption";
const char* SENSOR_RESERVE_POWER_BATTERY_CHARGE_POWER = "sensor.reserve_power_battery_charge_power";
const char* SENSOR_RESERVE_POWER_BATTERY_CHARGE_LEFT_DURATION = "sensor.reserve_power_battery_charge_left_duration";
const char* SENSOR_RESERVE_POWER_BATTERY_DISCHARGE_LEFT_DURATION = "sensor.reserve_power_battery_discharge_left_duration";

WebSocketClient::WebSocketClient() {}

void WebSocketClient::configure(String host, int port, String token, Controller *controller) {
    static char ws_uri[256];
    snprintf(ws_uri, sizeof(ws_uri), "ws://%s:%d/api/websocket", host.c_str(), port);
    this->token = token;
    this->config.uri = ws_uri;
    this->controller = controller;
}

void WebSocketClient::setNetworkAvailable(bool status) {
    this->isNetworkAvailable = status;
}

static void staticEventHandler(void *args, esp_event_base_t base, int32_t id, void *data) {
    WebSocketClient *instance = (WebSocketClient *)args;
    instance->eventHandler(args, base, id, data);
}

void WebSocketClient::init() {
    Serial.printf("WebSocket connecting to %s ...\n\r", this->config.uri);

    this->client = esp_websocket_client_init(&this->config);
    esp_websocket_register_events(this->client, WEBSOCKET_EVENT_ANY, staticEventHandler, (void *) this);
    esp_websocket_client_start(this->client);
}

void WebSocketClient::check() {
    if (this->client == nullptr) {
        return;
    }

    if (!esp_websocket_client_is_connected(this->client)) {
        Serial.println("WebSocket disconnected! Reconnecting...");

        (*this->controller).setWebsocketStatus(false);

        this->destroy();

        if (this->isNetworkAvailable) {
            this->init();
        }
    } else {
        this->checkPowerSensor();
    }
}

void WebSocketClient::destroy() {
    if (this->client == nullptr) {
        return;
    }

    esp_websocket_client_stop(this->client);
    esp_websocket_client_destroy(this->client);
    this->client = nullptr;
}

void WebSocketClient::eventHandler(void *args, esp_event_base_t base, int32_t eventId, void *eventData) {
    esp_websocket_event_data_t *data = (esp_websocket_event_data_t *) eventData;
    switch (eventId) {
    case WEBSOCKET_EVENT_CONNECTED:
        Serial.println("WEBSOCKET_EVENT_CONNECTED");
        (*this->controller).setWebsocketStatus(true);
        break;
    case WEBSOCKET_EVENT_DISCONNECTED:
        Serial.println("WEBSOCKET_EVENT_DISCONNECTED");
        (*this->controller).setWebsocketStatus(false);

        // Clear message buffer on disconnect
        this->messageBuffer.clear();
        this->expectedMessageLength = 0;
        break;
    case WEBSOCKET_EVENT_DATA:
        Serial.println("WEBSOCKET_EVENT_DATA");
        
        if (data->op_code == 0x1) { // text frame
            // Initialize buffer for new message
            if (data->payload_offset == 0) {
                this->messageBuffer.clear();
                this->expectedMessageLength = data->payload_len;
            }
            
            // Append current chunk to buffer
            this->messageBuffer += String((char *)data->data_ptr).substring(0, data->data_len);
            
            // Check if we've received the complete message
            if (this->messageBuffer.length() == this->expectedMessageLength) {
                this->messageHandler(this->messageBuffer);
                this->messageBuffer.clear();
                this->expectedMessageLength = 0;
            }
        } else {
            Serial.printf("Received opcode=%d", data->op_code);
            Serial.printf("Received=%.*s", data->data_len, (char *)data->data_ptr);
            Serial.printf("Total payload length=%d, data_len=%d, current payload offset=%d\r\n", data->payload_len, data->data_len, data->payload_offset);
        }

        break;
    case WEBSOCKET_EVENT_ERROR:
        Serial.println("WEBSOCKET_EVENT_ERROR");

        // Clear message buffer on error
        this->messageBuffer.clear();
        this->expectedMessageLength = 0;
        break;
    }
}

void WebSocketClient::sendAuth() {
    JsonDocument doc;
    
    doc["type"] = "auth";
    doc["access_token"] = this->token;

    this->send(doc);
}

void WebSocketClient::sendRenderInitialData() {
    JsonDocument tpl;

    tpl[SENSOR_BATTERY_SOC] = "{{ states('sensor.battery_soc') }}";
    tpl[SENSOR_GRID_POWER_AVAILABLE] = "{{ states('binary_sensor.grid_power_available') }}";
    tpl[SENSOR_GRID_POWER_MONITOR] = "{{ states('binary_sensor.grid_power_monitor') }}";
    tpl[SENSOR_SMART_METER_ACTIVE_POWER] = "{{ states('sensor.smart_meter_active_power') }}";
    tpl[SENSOR_CURRENT_LOAD_CONSUMPTION] = "{{ states('sensor.current_load_consumption') }}";
    tpl[SENSOR_RESERVE_POWER_BATTERY_CHARGE_POWER] = "{{ states('sensor.reserve_power_battery_charge_power') }}";
    tpl[SENSOR_RESERVE_POWER_BATTERY_CHARGE_LEFT_DURATION] = "{{ states('sensor.reserve_power_battery_charge_left_duration') }}";
    tpl[SENSOR_RESERVE_POWER_BATTERY_DISCHARGE_LEFT_DURATION] = "{{ states('sensor.reserve_power_battery_discharge_left_duration') }}";

    String buff = "";
    serializeJson(tpl, buff);

    JsonDocument doc;
    
    doc["id"] = 1;
    doc["type"] = "render_template";
    doc["template"] = buff.c_str();

    this->send(doc);
}

void WebSocketClient::sendPowerSensor(const bool state) {
    JsonDocument doc;

    doc["id"] = this->messageId++;
    doc["type"] = "fire_event";
    doc["event_type"] = "grid_power_monitor_update";
    doc["event_data"]["available"] = state ? "on" : "off";

    this->send(doc);
}

void WebSocketClient::send(JsonDocument message) {
    size_t n = measureJson(message);
    char* payload = (char*)malloc(n + 1);
    if (payload == nullptr) {
        return;
    }

    serializeJson(message, payload, n + 1);

    if (esp_websocket_client_is_connected(this->client)) {
        Serial.println("WebSocket sending message");
        Serial.println(payload);
        esp_websocket_client_send_text(this->client, payload, strlen(payload), portMAX_DELAY);
    }

    free(payload);
}

void WebSocketClient::messageHandler(String payload) {
    JsonDocument doc;
    deserializeJson(doc, payload.c_str());

    const char* type = doc["type"];
    if (strcmp(type, "auth_required") == 0) {
        Serial.println("HomeAssistant authentication required, sending auth...");
        this->sendAuth();
    } else if (strcmp(type, "auth_ok") == 0) {
        (*this->controller).setWebsocketStatus(true, true);
        Serial.println("HomeAssistant authentication successful");
        this->sendRenderInitialData();
    } else if (strcmp(type, "auth_invalid") == 0) {
        (*this->controller).setWebsocketStatus(true, false);
        Serial.println("HomeAssistant authentication failed");
    } else if (strcmp(type, "event") == 0) {
        if (doc["event"] && doc["event"]["result"]) {
            Serial.printf("HomeAssistant event state received: %s\n\r", payload.c_str());
            this->handleState(doc["event"]["result"]);
        } else {
            Serial.printf("HomeAssistant event received: %s\n\r", payload.c_str());
        }
    } else if (strcmp(type, "result") == 0) {
        Serial.printf("HomeAssistant result received: %s\n\r", payload.c_str());
    } else {
        Serial.printf("HomeAssistant message received: %s\n\r", payload.c_str());
    }
}

void WebSocketClient::handleState(JsonObject result) {
    State newState = this->controller->getState();

    float batteryPercentage = result[SENSOR_BATTERY_SOC];
    newState.batteryPercentage = batteryPercentage;

    bool powerAvailable = strcmp(result[SENSOR_GRID_POWER_AVAILABLE], "on") == 0;
    newState.setPowerAvailable(powerAvailable, true);

    float smartMeterActivePower = result[SENSOR_SMART_METER_ACTIVE_POWER];
    newState.powerConsumption = smartMeterActivePower;

    float currentLoadConsumption = result[SENSOR_CURRENT_LOAD_CONSUMPTION];
    newState.powerConsumptionFromBattery = currentLoadConsumption;

    float reservePowerBatteryChargePower = result[SENSOR_RESERVE_POWER_BATTERY_CHARGE_POWER];
    newState.setBatteryChargingPower(reservePowerBatteryChargePower / 1000.0); // convert W to kW

    String reservePowerBatteryChargeLeftDuration = result[SENSOR_RESERVE_POWER_BATTERY_CHARGE_LEFT_DURATION];;
    newState.batteryChargingDuration = reservePowerBatteryChargeLeftDuration;

    String reservePowerBatteryDischargeLeftDuration = result[SENSOR_RESERVE_POWER_BATTERY_DISCHARGE_LEFT_DURATION];;
    newState.batteryDischargingDuration = reservePowerBatteryDischargeLeftDuration;

    bool lastReportedPowerAvailable = strcmp(result[SENSOR_GRID_POWER_MONITOR], "on") == 0;
    this->checkPowerSensor(lastReportedPowerAvailable);

    this->controller->onStateUpdate(newState);
}

void WebSocketClient::checkPowerSensor() {
    const int powerSensorAvailable = this->controller->getPowerSensorAvailable();
    if (powerSensorAvailable != -1 && powerSensorAvailable != this->lastReportedPowerSensorAvailable) {
        Serial.printf("Last reported power sensor available: %d\n\r", this->lastReportedPowerSensorAvailable);
        Serial.printf("Power sensor available: %d\n\r", powerSensorAvailable);
        this->sendPowerSensor(powerSensorAvailable);
        this->lastReportedPowerSensorAvailable = powerSensorAvailable;
    }
}

void WebSocketClient::checkPowerSensor(bool lastPowerSensor) {
    const int powerSensorAvailable = this->controller->getPowerSensorAvailable();
    if (powerSensorAvailable != -1 && (powerSensorAvailable == 1) != lastPowerSensor) {
        Serial.printf("Last stored power sensor available: %d\n\r", lastPowerSensor);
        Serial.printf("Power sensor available: %d\n\r", powerSensorAvailable);
        this->sendPowerSensor(powerSensorAvailable);
        this->lastReportedPowerSensorAvailable = powerSensorAvailable;
    }
}
