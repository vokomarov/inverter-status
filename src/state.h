#pragma once

#include <Arduino.h>

const unsigned int STATE_NORMAL = 0;
const unsigned int STATE_DISCHARGING = 1;
const unsigned int STATE_CHARGING = 2;

class State {
    public:
        unsigned int state;

        int wifiSignalStrength;
        bool powerAvailable;
        bool powerKnown;
        bool websocketConnected;
        bool websocketAuthenticated;

        float powerConsumption; // in kW (smart meter active power)
        float powerConsumptionFromBattery; // in kW (current load consumption)
        float batteryPercentage;
        float batteryChargingPower; // in kW
        float batteryDischargingPower; // in kW

        String batteryChargingDuration;
        String batteryDischargingDuration;

        State() {
            this->state = STATE_NORMAL;
            this->wifiSignalStrength = -1;
            this->powerAvailable = false;
            this->powerKnown = false;
            this->websocketConnected = false;
            this->websocketAuthenticated = false;
            this->powerConsumption = -1.0;
            this->powerConsumptionFromBattery = -1.0;
            this->batteryPercentage = 0.0;
            this->batteryChargingPower = 0.0;
            this->batteryDischargingPower = 0.0;
            this->batteryChargingDuration = "";
            this->batteryDischargingDuration = "";
        }
    
        void setPowerAvailable(bool available, bool known) {
            this->powerAvailable = available;
            this->powerKnown = known;
            this->calculateState();
        }

        void setBatteryChargingPower(float power) {
            this->batteryChargingPower = power;
            this->calculateState();
        }

        void setBatteryDischargingPower(float power) {
            this->batteryDischargingPower = power;
            this->calculateState();
        }

        void calculateState() {
            // Grid present doesn't mean the inverter uses it (e.g. undervoltage cut-off), trust the battery flow
            if (this->batteryDischargingPower > 0.0) {
                this->state = STATE_DISCHARGING;
                return;
            }

            if (this->batteryChargingPower > 0.0) {
                this->state = STATE_CHARGING;
            } else if (this->powerAvailable && this->powerKnown) {
                this->state = STATE_NORMAL;
            } else {
                this->state = STATE_DISCHARGING;
            }
        }

        float getPowerMetric() {
            if (this->isCharging()) {
                return this->batteryChargingPower;
            } else if (this->isDischarging()) {
                return this->powerConsumptionFromBattery;
            } else {
                return this->powerConsumption;
            }
        }

        String getTimeMetric() {
            if (this->isCharging()) {
                return this->batteryChargingDuration;
            } else {
                return this->batteryDischargingDuration;
            }
        }

        bool isCharging() {
            return this->state == STATE_CHARGING;
        }

        bool isDischarging() {
            return this->state == STATE_DISCHARGING;
        }

        bool isNormal() {
            return this->state == STATE_NORMAL;
        }
};
