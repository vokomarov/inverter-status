#include "components/power_metric.h"

PowerMetric::PowerMetric()
{
    ScreenComponent();
}

void PowerMetric::setPower(float power)
{
    this->power = power;
}

void PowerMetric::setCharging(bool charging)
{
    this->charging = charging;
    this->drawIcon();
}

void PowerMetric::setDischarging(bool discharging)
{
    this->discharging = discharging;
    this->drawIcon();
}

void PowerMetric::initialDraw()
{
    this->iconColor = this->display.color888(135, 206, 235);
    this->iconColorCharging = this->display.color888(51, 255, 51);
    this->iconColorDischarging = this->display.color888(255, 69, 0);
    this->textColor = this->display.color888(200, 200, 200);

    this->drawIcon();
    this->drawPowerMetric();
}

void PowerMetric::draw()
{
    this->drawPowerMetric();
}

void PowerMetric::drawIcon()
{
    uint32_t color = this->iconColor;
    if (this->charging) {
        color = this->iconColorCharging;
    } else if (this->discharging) {
        color = this->iconColorDischarging;
    }
    
    int iconX = x + 12;
    int iconY = y + 1;

    // Draw a 20x20 lightning bolt (power) icon at (x,y) using white color.
    // The icon is composed of two left-angled triangles to produce a clean bolt
    // shape and fits inside a 20x20 box: [x, x+19] x [y, y+19].
    display.fillTriangle(iconX + 5, iconY + 2,
                         iconX + 13, iconY + 2,
                         iconX + 0, iconY + 10,
                         color);

    display.fillTriangle(iconX + 13, iconY + 2,
                         iconX + 0, iconY + 10,
                         iconX + 5, iconY + 10,
                         color);

    display.fillTriangle(iconX + 6, iconY + 8,
                         iconX + 12, iconY + 8,
                         iconX + 0, iconY + 18,
                         color);
}

void PowerMetric::drawPowerMetric()
{
    if (this->lastDrawnPower == this->power) {
        return;
    }

    int textX = x + 45;
    int textY = y + 3;

    if (this->power < 0) {
        return;
    }

    display.fillRect(textX, textY, 100, 19, TFT_BLACK);
    display.setFont(&fonts::FreeSansBold9pt7b); 
    display.setTextColor(this->textColor, TFT_BLACK);
    display.setCursor(textX, textY);
    display.print(this->power, 3);
    display.setFont(&fonts::AsciiFont8x16); 
    display.setTextColor(display.color888(64, 64, 64), TFT_BLACK);
    display.setCursor(textX + 60, textY + 2);
    display.print("kW");

    this->lastDrawnPower = this->power;
}
