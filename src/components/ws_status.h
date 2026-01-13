#include "components/screen_component.h"
#pragma once

class WSStatus: public ScreenComponent {
private:
    bool connected;

    int iconColor;
    int iconColorDisconnected;

    void drawIcon();
public: 
    WSStatus();
    void setStatus(bool connected);
    void initialDraw();
    void draw();
};
