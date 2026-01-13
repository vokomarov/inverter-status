#include "components/screen_component.h"
#pragma once

class WiFiStatus: public ScreenComponent {
private:
    int iconColor;
    int iconColorDisconnected;
    int textColor;
    int signalStrength;
    bool enableText;

    void drawIcon();
    void drawMetric();
public: 
    WiFiStatus();
    void setSignal(int strength);
    void initialDraw();
    void draw();
};
