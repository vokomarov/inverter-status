#include "components/power_status.h"

PowerStatus::PowerStatus()
{
    ScreenComponent();
    this->enableText = false;
}

void PowerStatus::setStatus(bool available)
{
    this->available = available;
    this->known = true;
}

void PowerStatus::setStatus(bool available, bool known)
{
    this->available = available;
    this->known = known;
}

void PowerStatus::initialDraw()
{
    this->iconColorOn = this->display.color888(102, 255, 102);
    this->iconColorOff = this->display.color888(255, 69, 0);
    this->iconColorUnknown = this->display.color888(64, 64, 64);
    this->textColorOn = this->display.color888(0, 255, 0);
    this->textColorOff = this->display.color888(255, 0, 0);

    this->drawIcon();
    this->drawText();
}

void PowerStatus::draw()
{
    this->drawIcon();
    this->drawText();
}

void PowerStatus::drawIcon()
{
    int iconX = x + 5;
    int iconY = y + 2;

    uint32_t color = this->iconColorUnknown;
    if (this->known) {
        color = this->available ? this->iconColorOn : this->iconColorOff;
    }

    // Draw a 20x20 power plug icon at (x,y) using light gray color.
    display.fillRect(iconX + 2, iconY + 2, 2, 6, color);
    display.fillRect(iconX + 7, iconY + 2, 2, 6, color);
    display.fillArc(iconX + 5, iconY + 7, 5, 0, 0, 180, color);
    display.fillRect(iconX + 5, iconY + 12, 1, 4, color);
}

void PowerStatus::drawText()
{
    if (!this->enableText) {
        return;
    }
    
    int textX = x + 20;
    int textY = y + 3;

    if (!this->known) {
        return;
    }

    display.setFont(&fonts::FreeSansBold9pt7b); 
    display.setCursor(textX, textY);

    if (this->available) {
        display.setTextColor(this->textColorOn);
        display.print("ON");
    } else {
        display.setTextColor(this->textColorOff);
        display.print("OFF");
    }
}
