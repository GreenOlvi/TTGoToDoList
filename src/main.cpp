#include <Arduino.h>
#include <GxEPD2_BW.h>
#include <fonts/Font5x7Fixed.h>

#define CS_PIN 5
#define DC_PIN 17
#define RST_PIN 16
#define BUSY_PIN 4
#define LED_PIN 19
#define BAT_TEST_PIN 35

#define FONT_OFFSET_X 0
#define FONT_OFFSET_Y -7

GxEPD2_BW<GxEPD2_213_B72, GxEPD2_213_B72::HEIGHT> display(GxEPD2_213_B72(CS_PIN, DC_PIN, RST_PIN, BUSY_PIN));

const char HelloWorld[] = "Hello, World!";

void drawGrid() {
    for (int i = 0; i < display.height(); i += 10) {
        for (int j = 0; j < display.width(); j += 10) {
            display.drawPixel(j, i, GxEPD_BLACK);
        }
    }
}

void setCursorWithOffser(int16_t x, int16_t y) {
    display.setCursor(x - FONT_OFFSET_X, y - FONT_OFFSET_Y);
}

void setup() {
    Serial.begin(115200);

    display.init(115200);
    display.setRotation(0);
    display.setFont(&Font5x7Fixed);
    display.setTextColor(GxEPD_BLACK);
    display.setFullWindow();

    display.firstPage();
    do
    {
        display.fillScreen(GxEPD_WHITE);
        drawGrid();
        setCursorWithOffser(13, 2);
        display.print(HelloWorld);
    } while (display.nextPage());
}

void loop() {
}