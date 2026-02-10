#include "controller.h"

Controller::Controller() {}

void Controller::setScreenComponents(
    Battery *battery,
    WiFiStatus *wifiStatus,
    WSStatus *wsStatus,
    PowerStatus *powerStatus,
    PowerMetric *powerMetric,
    TimeMetric *timeMetric
) {
    this->battery = battery;
    this->wifiStatus = wifiStatus;
    this->wsStatus = wsStatus;
    this->powerStatus = powerStatus;
    this->powerMetric = powerMetric;
    this->timeMetric = timeMetric;
    this->lastKnownState = State();
}

void Controller::setWifiSignal(int strength) {
    if (strength == this->lastKnownState.wifiSignalStrength) {
        return;
    }
    this->wifiStatus->setSignal(strength);
    this->lastKnownState.wifiSignalStrength = strength;
    this->wifiStatus->draw();
}

void Controller::setWebsocketStatus(bool connected) {
    if (connected == this->lastKnownState.websocketConnected) {
        return;
    }
    this->wsStatus->setStatus(connected);
    this->lastKnownState.websocketConnected = connected;
    this->lastKnownState.websocketAuthenticated = connected ? this->lastKnownState.websocketAuthenticated : false;
    this->wsStatus->draw();
}

void Controller::setWebsocketStatus(bool connected, bool authenticated) {
    if (connected == this->lastKnownState.websocketConnected &&
        authenticated == this->lastKnownState.websocketAuthenticated) {
        return;
    }
    this->wsStatus->setStatus(connected, authenticated);
    this->lastKnownState.websocketConnected = connected;
    this->lastKnownState.websocketAuthenticated = authenticated;
    this->wsStatus->draw();
}

State Controller::getState() {
    return this->lastKnownState;
}

void Controller::onStateUpdate(State newState) {
    if (newState.powerAvailable != this->lastKnownState.powerAvailable ||
        newState.powerKnown != this->lastKnownState.powerKnown) {
        this->powerStatus->setStatus(newState.powerAvailable, newState.powerKnown);
    }

    if (newState.getPowerMetric() != this->lastKnownState.getPowerMetric() ||
        newState.isCharging() != this->lastKnownState.isCharging()) {
        this->powerMetric->setPower(newState.getPowerMetric());
        this->powerMetric->setCharging(newState.isCharging());
        this->powerMetric->setDischarging(newState.isDischarging());
    }

    if (newState.getTimeMetric() != this->lastKnownState.getTimeMetric() ||
        newState.isCharging() != this->lastKnownState.isCharging()) {
        this->timeMetric->setDuration(newState.getTimeMetric());
        this->timeMetric->setCharging(newState.isCharging());
        this->timeMetric->setDischarging(newState.isDischarging());
    }

    if (newState.batteryPercentage != this->lastKnownState.batteryPercentage ||
        newState.isCharging() != this->lastKnownState.isCharging()) {
        this->battery->setPercentage(newState.batteryPercentage);
        this->battery->setCharging(newState.isCharging());
    }

    newState.wifiSignalStrength = this->lastKnownState.wifiSignalStrength;
    newState.websocketConnected = this->lastKnownState.websocketConnected;

    this->lastKnownState = newState;
}

void Controller::setPowerSensorAvailable(int available) {
    this->powerSensorAvailable = available;
}

int Controller::getPowerSensorAvailable() {
    return this->powerSensorAvailable;
}


void Controller::initialDraw() {
    this->wifiStatus->setLocation(60, 0);
    this->wifiStatus->setSignal(0);
    this->wifiStatus->initialDraw();

    this->wsStatus->setLocation(65, 0);
    this->wsStatus->setStatus(false);
    this->wsStatus->initialDraw();

    this->powerStatus->setLocation(0, 0);
    this->powerStatus->setStatus(false, false);
    this->powerStatus->initialDraw();

    this->battery->setLocation(5, 30);
    this->battery->setPercentage(0.0);
    this->battery->setCharging(false);
    this->battery->initialDraw();

    this->powerMetric->setLocation(0, 86);
    this->powerMetric->setPower(-1.0);
    this->powerMetric->setCharging(false);
    this->powerMetric->initialDraw();

    this->timeMetric->setLocation(0, 108);
    this->timeMetric->setDuration("");
    this->timeMetric->setCharging(false);
    this->timeMetric->initialDraw();
}

void Controller::draw() {
    this->wifiStatus->draw();
    this->wsStatus->draw();
    this->battery->draw();
    this->powerStatus->draw();
    this->powerMetric->draw();
    this->timeMetric->draw();
}



