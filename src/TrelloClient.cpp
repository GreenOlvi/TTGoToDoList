#include "TrelloClient.h"
 
const String TrelloClient::_baseUrl = "https://api.trello.com/1/";
const String TrelloClient::_cardFields = "id,idBoard,idList,name,pos,idChecklists";

TrelloClient::TrelloClient(String apiKey, String token)
    : _http(), _authParams("key=" + apiKey + "&token=" + token) {
}

TrelloList* toTrelloList(const JsonObject &object) {
    TrelloList *list = new TrelloList();
    list->id = object.getMember("id").as<trelloId_t>();
    list->name = object.getMember("name").as<String>();
    list->pos = object.getMember("pos").as<int>();
    return list;
}

TrelloCard* toTrelloCard(const JsonObject &object) {
    TrelloCard *card = new TrelloCard();
    card->id = object.getMember("id").as<trelloId_t>();
    card->name = object.getMember("name").as<String>();
    card->idList = object.getMember("idList").as<trelloId_t>();
    card->idBoard = object.getMember("idBoard").as<trelloId_t>();
    card->pos = object.getMember("pos").as<int>();

    auto checklists = object.getMember("idChecklists").as<JsonArray>();
    card->idChecklistsSize = checklists.size();
    card->idChecklists = new trelloId_t[card->idChecklistsSize];

    int i = 0;
    for (trelloId_t id : checklists) {
        card->idChecklists[i++] = id;
    }

    return card;
}

TrelloCheckitem* toTrelloCheckitem(const JsonObject &object) {
    TrelloCheckitem *item = new TrelloCheckitem();

    item->id = object.getMember("id").as<trelloId_t>();
    item->name = object.getMember("name").as<String>();
    item->idChecklist = object.getMember("idList").as<trelloId_t>();
    item->complete = object.getMember("state").as<String>() == "complete";
    item->pos = object.getMember("pos").as<int>();

    return item;
}

TrelloChecklist* toTrelloCheckList(const JsonObject &object) {
    TrelloChecklist *list = new TrelloChecklist();

    list->id = object.getMember("id").as<trelloId_t>();
    list->name = object.getMember("name").as<String>();
    list->idCard = object.getMember("idCard").as<trelloId_t>();
    list->idBoard = object.getMember("idBoard").as<trelloId_t>();

    JsonArray items = object.getMember("checkItems").as<JsonArray>();
    list->checkItemSize = items.size();
    list->checkItems = new TrelloCheckitem[list->checkItemSize];

    int i = 0;
    for (JsonObject item : items) {
        list->checkItems[i++] = *toTrelloCheckitem(item);
    }

    return list;
}

bool TrelloClient::GetListsFromBoard(const trelloId_t boardId, LinkedList<TrelloList> &lists) {
    String url = _baseUrl + "boards/" + boardId + "/lists?";
    StaticJsonDocument<768> doc;

    if (!FetchAndParse(url + _authParams, doc)) {
        return false;
    }

    JsonArray root = doc.as<JsonArray>();
    for (JsonObject value : root) {
        lists.add(*toTrelloList(value));
    }

    return true;
}

bool TrelloClient::GetCardsFromBoard(const trelloId_t boardId, LinkedList<TrelloCard> &cards) {
    return GetCardsFromUrl(_baseUrl + "boards/" + boardId + "/cards?fields=" + _cardFields + "&", cards);
}

bool TrelloClient::GetCardsFromList(const trelloId_t listId, LinkedList<TrelloCard> &cards) {
    return GetCardsFromUrl(_baseUrl + "lists/" + listId + "/cards?fields=" + _cardFields + "&", cards);
}

bool TrelloClient::GetCardsFromUrl(const String url, LinkedList<TrelloCard> &cards) {
    DynamicJsonDocument doc(2048);
    if (!FetchAndParse(url + _authParams, doc)) {
        return false;
    }

    JsonArray root = doc.as<JsonArray>();
    for (JsonVariant value : root) {
        cards.add(*toTrelloCard(value));
    }

    return true;
}

TrelloChecklist* TrelloClient::GetChecklist(const trelloId_t checklistId) {
    String url = _baseUrl + "checklists/" + checklistId + "?";
    DynamicJsonDocument doc(2048);
    if (!FetchAndParse(url + _authParams, doc)) {
        return nullptr;
    }
    return toTrelloCheckList(doc.as<JsonObject>());
}

bool TrelloClient::GetCheckItemsFromChecklist(const trelloId_t checklistId, LinkedList<TrelloCheckitem> &items) {
    String url = _baseUrl + "checklists/" + checklistId + "/checkItems?";
    DynamicJsonDocument doc(2048);
    if (!FetchAndParse(url + _authParams, doc)) {
        return false;
    }

    JsonArray root = doc.as<JsonArray>();
    for (JsonVariant value : root) {
        items.add(*toTrelloCheckitem(value));
    }

    return true;
}

bool TrelloClient::FetchAndParse(const String url, JsonDocument &doc) {
    Serial.println(url);

    _http.begin(url);
    int code = _http.GET();

    if (!code) {
        _http.end();
        Serial.println("Error during http request");
        return false;
    }

    Serial.print("HTTP code=");
    Serial.println(code);

    Stream &response = _http.getStream();
    DeserializationError error = deserializeJson(doc, response);
    _http.end();

    if (error) {
        Serial.print(F("deserializeJson() failed with code "));
        Serial.println(error.f_str());
        return false;
    }

    Serial.printf("Elements in response: %d\n", doc.size());
    return true;
}