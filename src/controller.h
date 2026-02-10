#pragma once

#include "components/battery.h"
#include "components/power_metric.h"
#include "components/power_status.h"
#include "components/time_metric.h"
#include "components/wifi_status.h"
#include "components/ws_status.h"
#include "state.h"

#pragma once

class Controller {
    private:
        Battery *battery;
        WiFiStatus *wifiStatus;
        WSStatus *wsStatus;
        PowerStatus *powerStatus;
        PowerMetric *powerMetric;
        TimeMetric *timeMetric;
        State lastKnownState;
        int powerSensorAvailable;
    public:
        Controller();
        void setScreenComponents(
            Battery *battery,
            WiFiStatus *wifiStatus,
            WSStatus *wsStatus,
            PowerStatus *powerStatus,
            PowerMetric *powerMetric,
            TimeMetric *timeMetric
        );
        
        void setWifiSignal(int strength);
        void setWebsocketStatus(bool connected);
        void setWebsocketStatus(bool connected, bool authenticated);
        State getState();
        void onStateUpdate(State newState);
        void setPowerSensorAvailable(int available);
        int getPowerSensorAvailable();
        void initialDraw();
        void draw();
};


