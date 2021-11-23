#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <GxEPD2_BW.h>
#include <fonts/Font5x7Fixed.h>
#include <LinkedList.h>
#include <nvs_flash.h>
#include <nvs.h>

#include "secrets.h"
#include "pins.h"
#include "TrelloClient.h"

#define FONT_OFFSET_X 0
#define FONT_OFFSET_Y -7

GxEPD2_BW<GxEPD2_213_B72, GxEPD2_213_B72::HEIGHT> display(GxEPD2_213_B72(CS_PIN, DC_PIN, RST_PIN, BUSY_PIN));

nvs_handle NvsHandle;

IPAddress ip;
IPAddress gateway;
IPAddress subnet;
IPAddress dns;
uint8_t channel;
uint8_t ap_mac[6];

TrelloClient trello(TRELLO_API_KEY, TRELLO_TOKEN);

String ListTitle;
LinkedList<TrelloCheckitem> ListItems;

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

void loadIpAddress(const char *key, IPAddress &value) {
    uint32_t val;
    auto err = nvs_get_u32(NvsHandle, key, &val);
    switch (err) {
        case ESP_OK:
            value = IPAddress(val);
            Serial.printf("Loaded [%s] = %s\n", key, value.toString().c_str());
            break;
        case ESP_ERR_NVS_NOT_FOUND:
            Serial.printf("Not initialized yet\n");
            break;
        default:
            Serial.printf("Error (%s) reading\n", esp_err_to_name(err));
            break;
    }
}

void initNvs() {
    auto err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        Serial.printf("Erased flash");
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);

    err = nvs_open("storage", NVS_READWRITE, &NvsHandle);

    loadIpAddress("ip", ip);
    loadIpAddress("gateway", gateway);
    loadIpAddress("subnet", subnet);
    loadIpAddress("dns", dns);
    nvs_get_u8(NvsHandle, "channel", &channel);
    Serial.printf("Loaded [channel] = %d\n", channel);
    size_t macSize;
    nvs_get_blob(NvsHandle, "ap_mac", &ap_mac, &macSize);
    Serial.print("Loaded [ap_mac] = ");
    for (int i = 0; i < macSize - 1; i++) {
        Serial.printf("%02x:", ap_mac[i]);
    }
    Serial.printf("%02x\n", ap_mac[5]);
}

void storeWiFiData() {
    if (!ip) {
        Serial.println("Saving ip data");
        nvs_set_u32(NvsHandle, "ip", WiFi.localIP());
        nvs_set_u32(NvsHandle, "gateway", WiFi.gatewayIP());
        nvs_set_u32(NvsHandle, "subnet", WiFi.subnetMask());
        nvs_set_u32(NvsHandle, "dns", WiFi.dnsIP());
        nvs_set_u8(NvsHandle, "channel", WiFi.channel());
        nvs_set_blob(NvsHandle, "ap_mac", WiFi.BSSID(), sizeof(ap_mac));
        nvs_commit(NvsHandle);
    }
    nvs_close(NvsHandle);
}

void connectToWiFi() {
    if (ip) {
        WiFi.config(ip, gateway, subnet, dns);
        WiFi.begin(WIFI_SSID, WIFI_PASSWORD, channel, ap_mac, true);
    } else {
        WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    }

    int retries = 0;
    while (WiFi.status() != WL_CONNECTED) {
        retries++;
        delay(50);

        if (retries == 100) {
            ip = IPAddress();
            WiFi.disconnect();
            delay(10);
            WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
        }
    }

    Serial.printf("Connected to WiFi after %lu\n", millis());
}

void LoadViewFromCard(trelloId_t cardId) {
    auto card = trello.GetCard(cardId);
    if (card == nullptr) {
        return;
    }

    ListTitle = card->name;
    for (int i = 0; i < card->idChecklistsSize; i++) {
        trello.GetCheckItemsFromChecklist(card->idChecklists[i], ListItems);
    }
}

void setup() {
    Serial.begin(115200);

    display.init(115200, false);
    display.setRotation(0);
    display.setFont(&Font5x7Fixed);
    display.setTextColor(GxEPD_BLACK);
    display.setFullWindow();

    initNvs();

    connectToWiFi();
    LoadViewFromCard(CARD_ID);

    storeWiFiData();

    display.firstPage();
    do
    {
        display.fillScreen(GxEPD_WHITE);
        drawGrid();
        setCursorWithOffser(13, 2);
        display.print(ListTitle);

        int y = 22;
        for (int i = 0; i < ListItems.size(); i++) {
            TrelloCheckitem item = ListItems[i];

            setCursorWithOffser(0, y);
            if (item.complete) {
               display.print("[x]");
            } else {
               display.print("[  ]");
            }

            setCursorWithOffser(20, y);
            display.print(item.name);
            y += 10;
        }

        setCursorWithOffser(13, display.height() - 20);
        display.print(WiFi.macAddress());
        setCursorWithOffser(13, display.height() - 10);
        display.print(WiFi.localIP());
    } while (display.nextPage());
}

void loop() {
}