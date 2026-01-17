#include "components/screen_component.h"
#pragma once

class WSStatus: public ScreenComponent {
private:
    bool connected;
    bool authenticated;

    int iconColor;
    int iconColorDisconnected;

    void drawIcon();
public: 
    WSStatus();
    void setStatus(bool connected);
    void setStatus(bool connected, bool authenticated);
    void initialDraw();
    void draw();
};
