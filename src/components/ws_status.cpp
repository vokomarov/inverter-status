#include "components/ws_status.h"

WSStatus::WSStatus()
{
    ScreenComponent();
}

void WSStatus::setStatus(bool connected)
{
    this->connected = connected;
}

void WSStatus::initialDraw()
{
    this->iconColor = this->display.color888(0, 204, 0);
    this->iconColorDisconnected = this->display.color888(64, 64, 64);

    this->drawIcon();
}

void WSStatus::draw()
{
    this->drawIcon();
}

void WSStatus::drawIcon()
{
    int iconX = x + 0;
    int iconY = y + 10;

    uint32_t color = this->iconColorDisconnected;
    if (this->connected) {
        color = this->iconColor;
    }

    display.fillCircle(iconX, iconY, 2, color);
}

