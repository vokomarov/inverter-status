#include "components/screen_component.h"
#pragma once

const int BATTERY_DEFAULT_WIDTH = 30;
const int BATTERY_DEFAULT_HEIGHT = 50;

class Battery: public ScreenComponent {
private:
    int width;
    int height;
    float percentage;
    float lastDrawnPercentage;
    float lastDrawnPercentageText;
    bool charging;

    uint32_t borderColor;
    uint32_t fillColor;
    uint32_t textColor;

    void drawOutline();
    void drawPercentageFill();
    void drawPercentageMetric();
    float getStaticFillSegment(float percentage);
public: 
    Battery();
    void setPercentage(float percentage);
    void setCharging(bool charging);
    
    void initialDraw();
    void draw();
};