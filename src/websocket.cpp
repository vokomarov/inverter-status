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

// HA's render_template result delivers every value as a JSON string, even
// numeric sensors - is<float>() alone would reject them all. Accept a real
// number too (belt and suspenders), otherwise parse the string; "unavailable"/
// "unknown"/"" fail to parse and the caller keeps the previous value.
static bool parseFloatState(JsonVariantConst value, float &out) {
    if (value.is<float>()) { out = value.as<float>(); return true; }
    const char* s = value.as<const char*>();
    if (s == nullptr || *s == '\0') return false;
    char* end = nullptr;
    float parsed = strtof(s, &end);
    if (end == s || *end != '\0') return false;
    out = parsed;
    return true;
}

WebSocketClient::WebSocketClient() {
    this->stateMutex = xSemaphoreCreateMutex();
}

void WebSocketClient::configure(String host, int port, String token, Controller *controller) {
    static char ws_uri[256];
    snprintf(ws_uri, sizeof(ws_uri), "ws://%s:%d/api/websocket", host.c_str(), port);
    this->token = token;
    this->config.uri = ws_uri;
    this->config.disable_auto_reconnect = true;
    this->controller = controller;
}

void WebSocketClient::setNetworkAvailable(bool status) {
    this->isNetworkAvailable = status;
}

void WebSocketClient::setPowerSensorAvailable(int available) {
    xSemaphoreTake(this->stateMutex, portMAX_DELAY);
    this->localPowerSensorAvailable = available;
    xSemaphoreGive(this->stateMutex);
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

// Stores a websocket status update for pump() to apply on the loop task.
// hasAuthenticated selects which Controller::setWebsocketStatus overload pump() calls.
void WebSocketClient::queueWsStatus(bool connected, bool authenticated, bool hasAuthenticated) {
    xSemaphoreTake(this->stateMutex, portMAX_DELAY);
    this->pendingWsConnected = connected;
    this->pendingWsAuthenticated = authenticated;
    this->pendingWsHasAuthenticated = hasAuthenticated;
    this->hasPendingWsStatus = true;
    xSemaphoreGive(this->stateMutex);
}

// Runs on the websocket task. Must never call into Controller, components, or the display.
void WebSocketClient::eventHandler(void *args, esp_event_base_t base, int32_t eventId, void *eventData) {
    esp_websocket_event_data_t *data = (esp_websocket_event_data_t *) eventData;
    switch (eventId) {
    case WEBSOCKET_EVENT_CONNECTED:
        Serial.println("WEBSOCKET_EVENT_CONNECTED");
        this->queueWsStatus(true, false, false);
        break;
    case WEBSOCKET_EVENT_DISCONNECTED:
        Serial.println("WEBSOCKET_EVENT_DISCONNECTED");
        this->queueWsStatus(false, false, false);

        // Clear message buffer on disconnect
        this->messageBuffer.clear();
        this->expectedMessageLength = 0;
        break;
    case WEBSOCKET_EVENT_DATA:
        Serial.println("WEBSOCKET_EVENT_DATA");

        if (data->op_code == 0x1 || (data->op_code == 0x0 && this->expectedMessageLength > 0)) {
            // Initialize buffer for a new message
            if (data->op_code == 0x1 && data->payload_offset == 0) {
                this->messageBuffer.clear();
                this->expectedMessageLength = data->payload_len;
            }

            // Append current chunk to buffer (data_ptr is not NUL-terminated)
            this->messageBuffer.concat((const char *)data->data_ptr, data->data_len);

            // Check if we've received the complete message
            if (this->messageBuffer.length() >= this->expectedMessageLength) {
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

void WebSocketClient::send(const JsonDocument &message) {
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
    DeserializationError err = deserializeJson(doc, payload.c_str());
    if (err) {
        Serial.printf("JSON parse failed: %s\n\r", err.c_str());
        return;
    }

    const char* type = doc["type"] | "";
    if (strcmp(type, "auth_required") == 0) {
        Serial.println("HomeAssistant authentication required, sending auth...");
        this->sendAuth();
    } else if (strcmp(type, "auth_ok") == 0) {
        this->queueWsStatus(true, true, true);
        Serial.println("HomeAssistant authentication successful");
        this->sendRenderInitialData();
    } else if (strcmp(type, "auth_invalid") == 0) {
        this->queueWsStatus(true, false, true);
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

// Runs on the websocket task. Builds the new State off lastSensorState (a
// websocket-task-only copy, never shared with Controller) so invalid/unavailable
// readings fall back to the last good value instead of touching Controller::getState().
void WebSocketClient::handleState(JsonObject result) {
    State newState = this->lastSensorState;

    float batteryPercentage;
    if (parseFloatState(result[SENSOR_BATTERY_SOC], batteryPercentage)) {
        newState.batteryPercentage = batteryPercentage;
    }

    const char* gridPowerAvailable = result[SENSOR_GRID_POWER_AVAILABLE] | "";
    if (strcmp(gridPowerAvailable, "on") == 0) {
        newState.setPowerAvailable(true, true);
    } else if (strcmp(gridPowerAvailable, "off") == 0) {
        newState.setPowerAvailable(false, true);
    } else {
        newState.setPowerAvailable(false, false);
    }

    float powerConsumption;
    if (parseFloatState(result[SENSOR_SMART_METER_ACTIVE_POWER], powerConsumption)) {
        newState.powerConsumption = powerConsumption;
    }

    float powerConsumptionFromBattery;
    if (parseFloatState(result[SENSOR_CURRENT_LOAD_CONSUMPTION], powerConsumptionFromBattery)) {
        newState.powerConsumptionFromBattery = powerConsumptionFromBattery;
    }

    float reservePowerBatteryChargePower;
    if (parseFloatState(result[SENSOR_RESERVE_POWER_BATTERY_CHARGE_POWER], reservePowerBatteryChargePower)) {
        newState.setBatteryChargingPower(reservePowerBatteryChargePower / 1000.0); // convert W to kW
    }

    const char* chargeLeftDuration = result[SENSOR_RESERVE_POWER_BATTERY_CHARGE_LEFT_DURATION] | "";
    newState.batteryChargingDuration = chargeLeftDuration;

    const char* dischargeLeftDuration = result[SENSOR_RESERVE_POWER_BATTERY_DISCHARGE_LEFT_DURATION] | "";
    newState.batteryDischargingDuration = dischargeLeftDuration;

    this->lastSensorState = newState;

    xSemaphoreTake(this->stateMutex, portMAX_DELAY);
    this->pendingState = newState;
    this->hasPendingState = true;
    xSemaphoreGive(this->stateMutex);

    const char* gridPowerMonitor = result[SENSOR_GRID_POWER_MONITOR] | "";
    if (strcmp(gridPowerMonitor, "on") == 0) {
        this->checkPowerSensor(true);
    } else if (strcmp(gridPowerMonitor, "off") == 0) {
        this->checkPowerSensor(false);
    }
}

// Called from check() on the loop task; safe to read Controller directly.
void WebSocketClient::checkPowerSensor() {
    const int powerSensorAvailable = this->controller->getPowerSensorAvailable();
    if (powerSensorAvailable != -1 && powerSensorAvailable != this->lastReportedPowerSensorAvailable) {
        Serial.printf("Last reported power sensor available: %d\n\r", this->lastReportedPowerSensorAvailable);
        Serial.printf("Power sensor available: %d\n\r", powerSensorAvailable);
        this->sendPowerSensor(powerSensorAvailable);
        this->lastReportedPowerSensorAvailable = powerSensorAvailable;
    }
}

// Called from handleState() on the websocket task; reads the mutex-protected
// mirror (kept in sync by pumpWebSocket() on the loop task) instead of Controller.
void WebSocketClient::checkPowerSensor(bool lastPowerSensor) {
    xSemaphoreTake(this->stateMutex, portMAX_DELAY);
    const int powerSensorAvailable = this->localPowerSensorAvailable;
    xSemaphoreGive(this->stateMutex);

    if (powerSensorAvailable != -1 && (powerSensorAvailable == 1) != lastPowerSensor) {
        Serial.printf("Last stored power sensor available: %d\n\r", lastPowerSensor);
        Serial.printf("Power sensor available: %d\n\r", powerSensorAvailable);
        this->sendPowerSensor(powerSensorAvailable);
        this->lastReportedPowerSensorAvailable = powerSensorAvailable;
    }
}

// Called from the loop task before controller.draw(). Drains data queued by the
// websocket task and applies it — this is the only place onStateUpdate()/
// setWebsocketStatus() are called for websocket-originated updates.
void WebSocketClient::pump() {
    State stateCopy;
    bool hasState = false;

    bool wsConnected = false;
    bool wsAuthenticated = false;
    bool wsHasAuthenticated = false;
    bool hasWsStatus = false;

    xSemaphoreTake(this->stateMutex, portMAX_DELAY);
    if (this->hasPendingState) {
        stateCopy = this->pendingState;
        hasState = true;
        this->hasPendingState = false;
    }
    if (this->hasPendingWsStatus) {
        wsConnected = this->pendingWsConnected;
        wsAuthenticated = this->pendingWsAuthenticated;
        wsHasAuthenticated = this->pendingWsHasAuthenticated;
        hasWsStatus = true;
        this->hasPendingWsStatus = false;
    }
    xSemaphoreGive(this->stateMutex);

    if (hasWsStatus) {
        if (wsHasAuthenticated) {
            this->controller->setWebsocketStatus(wsConnected, wsAuthenticated);
        } else {
            this->controller->setWebsocketStatus(wsConnected);
        }
    }

    if (hasState) {
        this->controller->onStateUpdate(stateCopy);
    }
}
