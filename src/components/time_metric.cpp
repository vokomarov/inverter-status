#include "components/time_metric.h"

TimeMetric::TimeMetric()
{
    ScreenComponent();
}

void TimeMetric::setDuration(String duration)
{
    this->duration = duration;
}

void TimeMetric::setCharging(bool charging)
{
    this->charging = charging;
    this->drawIcon();
}

void TimeMetric::setDischarging(bool discharging)
{
    this->discharging = discharging;
    this->drawIcon();
}

void TimeMetric::initialDraw()
{
    this->iconColor = this->display.color888(135, 206, 235);
    this->iconColorCharging = this->display.color888(51, 255, 51);
    this->iconColorDischarging = this->display.color888(255, 69, 0);
    this->textColor = this->display.color888(200, 200, 200);

    this->drawIcon();
    this->drawTimeMetric();
}

void TimeMetric::draw()
{
    this->drawTimeMetric();
}

void TimeMetric::drawIcon()
{
    uint32_t color = this->iconColor;
    if (this->charging) {
        color = this->iconColorCharging;
    } else if (this->discharging) {
        color = this->iconColorDischarging;
    }

    int iconX = x + 10;
    int iconY = y + 1;

    display.fillCircle(iconX + 10, iconY + 10, 8, color);
    display.fillCircle(iconX + 10, iconY + 10, 7, TFT_BLACK);
    display.fillRect(iconX + 9, iconY + 0, 3, 3, color);
    display.drawLine(iconX + 10, iconY + 10, iconX + 10, iconY + 5, color);
    display.drawLine(iconX + 10, iconY + 10, iconX + 13, iconY + 13, color);
}

void TimeMetric::drawTimeMetric()
{
    if (this->lastDrawnDuration == this->duration) {
        return;
    }

    int textX = x + 45;
    int textY = y + 3;

    display.fillRect(textX, textY, 100, 20, TFT_BLACK);
    display.setFont(&fonts::FreeSansBold9pt7b); 
    display.setTextColor(this->textColor, TFT_BLACK);
    display.setCursor(textX, textY);
    display.print(this->duration);

    this->lastDrawnDuration = this->duration;
}

