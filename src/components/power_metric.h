#include "components/screen_component.h"
#pragma once

class PowerMetric: public ScreenComponent {
private:
    float power;
    float lastDrawnPower;
    bool charging;
    bool discharging;

    uint32_t iconColor;
    uint32_t iconColorCharging;
    uint32_t iconColorDischarging;
    uint32_t textColor;

    void drawIcon();
    void drawPowerMetric();
public: 
    PowerMetric();
    void setPower(float power);
    void setCharging(bool charging);
    void setDischarging(bool discharging);
    void initialDraw();
    void draw();
};
