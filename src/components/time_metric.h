#include <Arduino.h>
#include "components/screen_component.h"
#pragma once

class TimeMetric: public ScreenComponent {
private:
    String duration;
    String lastDrawnDuration;
    bool charging;
    bool discharging;

    uint32_t iconColor;
    uint32_t iconColorCharging;
    uint32_t iconColorDischarging;
    uint32_t textColor;

    void drawIcon();
    void drawTimeMetric();
public: 
    TimeMetric();
    void setDuration(String duration);
    void setCharging(bool charging);
    void setDischarging(bool discharging);
    void initialDraw();
    void draw();
};
