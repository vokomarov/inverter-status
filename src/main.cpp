#include "LGFX_User_Setup.hpp"
#include "components/battery.h"
#include "components/power_metric.h"
#include "components/power_status.h"
#include "components/time_metric.h"
#include "components/wifi_status.h"
#include "components/ws_status.h"
#include "config.h"
#include "controller.h"
#include "network.h"
#include "secrets.h"
#include <Arduino.h>

const bool debug = false;
constexpr int SENSOR_PIN = GPIO_NUM_13;

LGFX tft;
Battery battery;
WiFiStatus wifiStatus;
WSStatus wsStatus;
PowerStatus powerStatus;
PowerMetric powerMetric;
TimeMetric timeMetric;
Controller controller;

void drawLines();
void checkPower();

void setup() {
  Serial.begin(115200);
  Serial.printf("Booting (build %s, %s)\n\r", __DATE__, __TIME__);
  Serial.println("Initialising display...");

  pinMode(SENSOR_PIN, INPUT_PULLUP);
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

  controller.setScreenComponents(&battery, &wifiStatus, &wsStatus, &powerStatus,
                                 &powerMetric, &timeMetric);
  controller.initialDraw();

  Serial.println("Display setup completed.");

  checkPower();

  initWiFi();

  Serial.println("Boot completed");
}

void loop() {
  checkPower();
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

void checkPower() {
  controller.setPowerSensorAvailable(digitalRead(SENSOR_PIN) == LOW ? 1 : 0);
}
