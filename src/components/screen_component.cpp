#include "components/screen_component.h"

ScreenComponent::ScreenComponent()
{
}

void ScreenComponent::setDisplay(LGFX *display)
{
    this->display = *display;
}

void ScreenComponent::setLocation(int x, int y)
{
    this->x = x;
    this->y = y;
}

void ScreenComponent::initialDraw()
{
}

void ScreenComponent::draw()
{
}

void ScreenComponent::setFrameRate(float fps, int frames)
{
    this->frameRate = fps;
    this->frames = frames;
}

int ScreenComponent::getFrameNumber()
{
    unsigned long currentTime = millis();
    if (frameRate <= 0) {
        return 0;
    }

    int frameDuration = 1000 / frameRate;
    if (currentTime - this->lastFrameTimestamp < frameDuration) {
        return this->lastFrameNumber;
    }

    this->lastFrameNumber++;
    if (this->lastFrameNumber >= this->frames) {
        this->lastFrameNumber = 0;
    }

    this->lastFrameTimestamp = currentTime;

    return this->lastFrameNumber;
}
