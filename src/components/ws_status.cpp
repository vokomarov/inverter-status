#include "components/ws_status.h"

WSStatus::WSStatus()
{
    ScreenComponent();

    this->setFrameRate(4, 2);
}

void WSStatus::setStatus(bool connected, bool authenticated)
{
    this->connected = connected;
    this->authenticated = authenticated;
}

void WSStatus::setStatus(bool connected)
{
    this->connected = connected;
    this->authenticated = false;
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

    if (!this->authenticated) {
        switch(this->getFrameNumber()) {
            case 0:
                color = TFT_BLACK;
                break;
            case 1:
                //
                break;
        }
    }

    display.fillCircle(iconX, iconY, 2, color);
}

