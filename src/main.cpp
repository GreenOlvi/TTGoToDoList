#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <GxEPD2_BW.h>
#include <fonts/Font5x7Fixed.h>

#include "secrets.h"
#include "pins.h"

#define FONT_OFFSET_X 0
#define FONT_OFFSET_Y -7

GxEPD2_BW<GxEPD2_213_B72, GxEPD2_213_B72::HEIGHT> display(GxEPD2_213_B72(CS_PIN, DC_PIN, RST_PIN, BUSY_PIN));

String ListTitle = "To do list";
String TodoList[3];

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

void fetchData() {
    char boardId[] = BOARD_ID;

    String baseUrl = "https://api.trello.com/1/";
    String authParams = String("?key=") + TRELLO_API_KEY + "&token=" + TRELLO_TOKEN;

    String getListsFromBoardUrl = baseUrl + "boards/" + boardId + "/lists";

    Serial.println(getListsFromBoardUrl);

    HTTPClient http;
    http.begin(getListsFromBoardUrl + authParams);
    int code = http.GET();

    Serial.print("Response code = ");
    Serial.println(code);

    if (code == 0) {
        http.end();
        return;
    }

    Stream& response = http.getStream();
    DynamicJsonDocument doc(2048);
    DeserializationError error = deserializeJson(doc, response);

    http.end();

    if (error) {
        Serial.print(F("deserializeJson() failed with code "));
        Serial.println(error.f_str());
        return;
    }

    TodoList[0] = doc[0]["name"].as<String>();
    TodoList[1] = doc[1]["name"].as<String>();
    TodoList[2] = doc[2]["name"].as<String>();
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
    fetchData();

    display.firstPage();
    do
    {
        display.fillScreen(GxEPD_WHITE);
        drawGrid();
        setCursorWithOffser(13, 2);
        display.print(ListTitle);

        for (int i = 0; i < 3; i++) {
            if (TodoList[i] != nullptr) {
                setCursorWithOffser(13, 22 + i * 10);
                display.print(TodoList[i]);
            }
        }

        setCursorWithOffser(13, 102);
        display.print(WiFi.macAddress());
        setCursorWithOffser(13, 112);
        display.print(WiFi.localIP());
    } while (display.nextPage());
}

void loop() {
}