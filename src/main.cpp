#include <Arduino.h>
#include "LGFX_User_Setup.hpp"
#include "config.h"
#include "secrets.h"
#include "network.h"
#include "components/battery.h"
#include "components/power_metric.h"
#include "components/time_metric.h"
#include "components/power_status.h"
#include "components/wifi_status.h"
#include "components/ws_status.h"
#include "controller.h"

const bool debug = false;

LGFX tft;
Battery battery;
WiFiStatus wifiStatus;
WSStatus wsStatus;
PowerStatus powerStatus;
PowerMetric powerMetric;
TimeMetric timeMetric;
Controller controller;

void drawLines();

void setup() {
  Serial.begin(115200);
  Serial.printf("Booting (build %s, %s)\n\r", __DATE__, __TIME__);
  Serial.println("Initialising display...");

  pinMode(GPIO_NUM_5, OUTPUT);
  pinMode(GPIO_NUM_6, OUTPUT);
  digitalWrite(GPIO_NUM_5, HIGH);
  digitalWrite(GPIO_NUM_6, HIGH);

  tft.init(); 
  tft.setRotation(0); 
  tft.setColorDepth(24);

  drawLines();

  battery.setDisplay(&tft);
  powerMetric.setDisplay(&tft);
  timeMetric.setDisplay(&tft);
  powerStatus.setDisplay(&tft);
  wifiStatus.setDisplay(&tft);
  wsStatus.setDisplay(&tft);

  controller.setScreenComponents(&battery, &wifiStatus, &wsStatus, &powerStatus, &powerMetric, &timeMetric);
  controller.initialDraw();

  Serial.println("Display setup completed.");

  initWiFi();

  Serial.println("Boot completed");
}

void loop() {
  drawLines();
  checkWiFi();
  controller.draw();

  delay(100);
}

void drawLines() {
  if (debug) {
    tft.drawLine(0, 0, 0, 127, TFT_RED);
    tft.drawLine(0, 0, 127, 0, TFT_GREEN);
    tft.drawLine(127, 0, 127, 127, TFT_BLUE);
    tft.drawLine(0, 127, 127, 127, TFT_VIOLET);
  }

  tft.drawLine(0, 20, 127, 20, tft.color888(32, 32, 32));
  tft.drawLine(0, 86, 127, 86, tft.color888(32, 32, 32));
  tft.drawLine(0, 107, 127, 107, tft.color888(32, 32, 32));
}







