#include "components/battery.h"

Battery::Battery()
{
    this->charging = false;
    this->percentage = 0.0;
    this->lastDrawnPercentage = -1.0;
    this->width = BATTERY_DEFAULT_WIDTH;
    this->height = BATTERY_DEFAULT_HEIGHT;

    this->setFrameRate(2, 2);
}

void Battery::setPercentage(float percentage)
{
    this->percentage = percentage;
}

void Battery::setCharging(bool charging)
{
    this->charging = charging;
    this->lastDrawnPercentage = -1.0; // Force redraw on next draw call
}

void Battery::initialDraw()
{
    this->lastDrawnPercentage = -1.0;
    this->borderColor = this->display.color888(255, 255, 255);
    this->fillColor = this->display.color888(0, 255, 0);
    this->textColor = this->display.color888(255, 255, 255);

    this->drawOutline();
    this->drawPercentageFill();
    this->drawPercentageMetric();
}

void Battery::draw()
{
    this->drawPercentageFill();
    this->drawPercentageMetric();
}

void Battery::drawOutline()
{
    // Draw battery outline
    display.drawRect(x, y, width, height, this->borderColor);
    display.fillRect(x + width / 4, y - height / 15, width / 2, height / 15, this->borderColor); // Battery tip
}

void Battery::drawPercentageFill()
{
    if (this->percentage < 0.0) {
        this->percentage = 0.0;
    } else if (this->percentage > 100.0) {
        this->percentage = 100.0;
    }

    if (!this->charging && this->lastDrawnPercentage == this->percentage) {
        return;
    }

    int fillHeight;
    int staticPercentage;
    int fillHeightBlink;

    if (this->charging) {
        staticPercentage = this->getStaticFillSegment(this->percentage);
        switch (this->getFrameNumber()) {
        case 0:
            // Charging frame 1: fill up to the last static segment
            // next segment will blink (from black to fill color)
            fillHeight = (height - 4) * (staticPercentage / 100.0);
            fillHeightBlink = (height - 4) * (25.0 / 100.0);
            display.fillRect(x + 2, y + height - 3 - fillHeight - fillHeightBlink, width - 4, fillHeightBlink, TFT_BLACK);
            break;
        case 1:
            // Charging frame 2: fill up to the last static segment plus blinking segment
            fillHeight = (height - 4) * ((staticPercentage + 25.0) / 100.0);
            break;
        }
    } else {
        // Clear previous fill
        display.fillRect(x + 2, y + 2, width - 4, height - 4, TFT_BLACK); 

        // Calculate fill height based on percentage
        fillHeight = (height - 4) * (this->percentage / 100.0);
    }

    // Draw battery fill (from bottom up)
    display.fillRect(x + 2, y + height - 2 - fillHeight, width - 4, fillHeight, this->fillColor);

    this->lastDrawnPercentage = this->percentage;
}

float Battery::getStaticFillSegment(float percentage)
{
    if (percentage < 0.0) {
        return 0.0;
    }
    if (percentage > 0 && percentage <= 25) {
        return 0.0;
    }
    if (percentage > 25 && percentage <= 50) {
        return 25.0;
    }
    if (percentage > 50 && percentage <= 75) {
        return 50.0;
    }
    return 75.0;
}

void Battery::drawPercentageMetric()
{
    if (this->lastDrawnPercentageText == this->percentage) {
        return;
    }

    int textX = x + 55;
    int textY = y + 10;
    display.fillRect(textX, textY, 60, 30, TFT_BLACK);
    display.setFont(&fonts::FreeSansBold18pt7b); 
    display.setTextColor(this->textColor, TFT_BLACK);
    display.setCursor(textX, textY);
    if (this->percentage <= 0.0) {
        display.print("xx");
    } else {
        display.printf("%.0f", this->percentage);
    }
    display.setFont(&fonts::Font4); 
    display.setCursor(textX+40, textY+0);
    display.print("%");

    this->lastDrawnPercentageText = this->percentage;
}
