#include "TrelloClient.h"
 
const String TrelloClient::_baseUrl = "https://api.trello.com/1/";
const String TrelloClient::_cardFields = "id,idBoard,idList,name,pos,idChecklists";

TrelloClient::TrelloClient(String apiKey, String token)
    : _http(), _authParams("key=" + apiKey + "&token=" + token) {
}

TrelloList toTrelloList(const JsonVariant &object) {
    TrelloList list;
    list.id = object.getMember("id").as<trelloId_t>();
    list.name = object.getMember("name").as<String>();
    list.pos = object.getMember("pos").as<int>();
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

bool TrelloClient::GetListsFromBoard(trelloId_t boardId, LinkedList<TrelloList> &lists) {
    String url = _baseUrl + "boards/" + boardId + "/lists?";
    StaticJsonDocument<768> doc;

    if (!FetchAndParse(url + _authParams, doc)) {
        return false;
    }

    JsonArray root = doc.as<JsonArray>();
    for (JsonObject value : root) {
        lists.add(toTrelloList(value));
    }

    return true;
}

bool TrelloClient::GetCardsFromBoard(trelloId_t boardId, LinkedList<TrelloCard> &cards) {
    return GetCardsFromUrl(_baseUrl + "boards/" + boardId + "/cards?fields=" + _cardFields + "&", cards);
}

bool TrelloClient::GetCardsFromList(trelloId_t listId, LinkedList<TrelloCard> &cards) {
    return GetCardsFromUrl(_baseUrl + "lists/" + listId + "/cards?fields=" + _cardFields + "&", cards);
}

bool TrelloClient::GetCardsFromUrl(String url, LinkedList<TrelloCard> &cards) {
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

bool TrelloClient::FetchAndParse(String url, JsonDocument &doc) {
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