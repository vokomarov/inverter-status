#include <Arduino.h>
#include "LGFX_User_Setup.hpp"

#pragma once

class ScreenComponent {
protected:
    LGFX display;
    int x;
    int y;

    float frameRate;
    int frames;
    int lastFrameNumber;
    unsigned long lastFrameTimestamp;

    // Sets the frame rate for the component's drawing updates
    void setFrameRate(float fps, int frames);

    // Returns the number of the frame at the current time based on the frame rate
    int getFrameNumber();
public: 
    ScreenComponent();
    void setDisplay(LGFX *display);
    void setLocation(int x, int y);

    virtual void initialDraw();
    virtual void draw();
};
