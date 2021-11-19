#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <GxEPD2_BW.h>
#include <fonts/Font5x7Fixed.h>
#include <LinkedList.h>

#include "secrets.h"
#include "pins.h"
#include "TrelloClient.h"

#define FONT_OFFSET_X 0
#define FONT_OFFSET_Y -7

GxEPD2_BW<GxEPD2_213_B72, GxEPD2_213_B72::HEIGHT> display(GxEPD2_213_B72(CS_PIN, DC_PIN, RST_PIN, BUSY_PIN));

TrelloClient trello(TRELLO_API_KEY, TRELLO_TOKEN);

String ListTitle = "To do list";

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

void connectToWiFi() {
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.println("Connecting to WiFi...");
    }

    Serial.println("Connected to WiFi");
}

void setup() {
    Serial.begin(115200);

    display.init(115200, false);
    display.setRotation(0);
    display.setFont(&Font5x7Fixed);
    display.setTextColor(GxEPD_BLACK);
    display.setFullWindow();

    connectToWiFi();
    // LinkedList<TrelloList> Lists;
    // trello.GetListsFromBoard(BOARD_ID, Lists);
    LinkedList<TrelloCard> Cards;
    trello.GetCardsFromBoard(BOARD_ID, Cards);

    display.firstPage();
    do
    {
        display.fillScreen(GxEPD_WHITE);
        drawGrid();
        setCursorWithOffser(13, 2);
        display.print(ListTitle);

        // for (int i = 0; i < Lists.size(); i++) {
        //     setCursorWithOffser(13, 22 + i * 10);
        //     display.print(Lists.get(i).name);
        // }

        for (int i = 0; i < Cards.size(); i++) {
             setCursorWithOffser(13, 22 + i * 10);
             display.print(Cards.get(i).name);
        }

        setCursorWithOffser(13, 102);
        display.print(WiFi.macAddress());
        setCursorWithOffser(13, 112);
        display.print(WiFi.localIP());
    } while (display.nextPage());
}

void loop() {
}