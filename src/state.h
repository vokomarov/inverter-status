#include <Arduino.h>
#pragma once

const unsigned int STATE_NORMAL = 0;
const unsigned int STATE_DISCHARGING = 1;
const unsigned int STATE_CHARGING = 2;

const unsigned int BATTERY_MAX_SOC = 95.0;

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

        String batteryChargingDuration;
        String batteryDischargingDuration;

        State() {
            this->state = STATE_NORMAL;
            this->wifiSignalStrength = -1;
            this->powerAvailable = false;
            this->powerKnown = false;
            this->websocketConnected = false;
            this->powerConsumption = -1.0;
            this->batteryPercentage = 0.0;
            this->batteryChargingPower = 0.0;
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

        void calculateState() {
            bool hasPower = this->powerAvailable && this->powerKnown;
            bool ableToCharge = this->batteryPercentage < BATTERY_MAX_SOC || this->batteryChargingPower > 0.0;

            if (hasPower && ableToCharge) {
                this->state = STATE_CHARGING;
            } else if (hasPower && !ableToCharge) {
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
