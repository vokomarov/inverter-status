#include "components/wifi_status.h"

WiFiStatus::WiFiStatus()
{
    ScreenComponent();
    this->enableText = false;
}

void WiFiStatus::setSignal(int strength)
{
    this->signalStrength = strength;
}

void WiFiStatus::initialDraw()
{
    this->iconColor = this->display.color888(255, 255, 255);
    this->iconColorDisconnected = this->display.color888(64, 64, 64);
    this->textColor = this->display.color888(0, 255, 0);

    this->drawIcon();
    this->drawMetric();
}

void WiFiStatus::draw()
{
    this->drawIcon();
    this->drawMetric();
}

void WiFiStatus::drawIcon()
{
    int iconX = x + 45;
    int iconY = y + 0;

    uint32_t color = this->iconColor;
    if (this->signalStrength >= 0) {
        color = this->iconColorDisconnected;
    }

    // center point (bottom-centered in the 20x20 box)
    int cx = iconX + 10;
    int cy = iconY + 14;

    // angles for the arc sweep (degrees)
    int startDeg = 220;
    int endDeg = 330;

    // draw outer, middle and inner arcs
    // radii chosen to fit inside 20x20 box
    int r_outer = 9;
    int r_mid = 6;
    int r_inner = 3;

    // thickness in pixels for arcs
    int thickness = 1;

    // Outer arc
    display.drawArc(cx, cy,
                    r_outer - (thickness - 1), r_outer,
                    startDeg, endDeg,
                    color);

    // Middle arc
    display.drawArc(cx, cy,
                    r_mid - (thickness - 1), r_mid,
                    startDeg, endDeg,
                    color);

    // Inner arc
    display.drawArc(cx, cy,
                    r_inner - (thickness - 1), r_inner,
                    startDeg, endDeg,
                    color);

    // Draw the bottom dot to indicate the source of the signal
    display.fillCircle(cx, cy + 1, 1, color);
}

void WiFiStatus::drawMetric()
{
    if (!this->enableText || this->signalStrength >= 0) {
        return;
    }

    int textX = x + 17;
    int textY = y + 3;

    display.setFont(&fonts::FreeSans9pt7b); 
    display.setCursor(textX, textY);
    display.setTextColor(this->textColor, TFT_BLACK);
    display.print(this->signalStrength);
}
