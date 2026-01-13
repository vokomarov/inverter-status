#include "components/screen_component.h"
#pragma once

class PowerStatus: public ScreenComponent {
private:
    bool available;
    bool known;
    bool enableText;

    uint32_t iconColorUnknown;
    uint32_t iconColorOn;
    uint32_t iconColorOff;
    uint32_t textColorOn;
    uint32_t textColorOff;

    void drawIcon();
    void drawText();
public: 
    PowerStatus();
    void setStatus(bool available);
    void setStatus(bool available, bool known);
    void initialDraw();
    void draw();
};
