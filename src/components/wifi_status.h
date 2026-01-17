#include "components/screen_component.h"
#pragma once

enum SignalState { SIGNAL_GOOD, SIGNAL_FAIR, SIGNAL_BAD };

class WiFiStatus: public ScreenComponent {
private:
    int iconColor;
    int iconColorDisconnected;
    int textColor;
    int signalStrength;
    SignalState state;
    bool enableText;

    void drawIcon();
    void drawMetric();
public: 
    WiFiStatus();
    void setSignal(int strength);
    void initialDraw();
    void draw();
};
