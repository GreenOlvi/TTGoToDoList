#include "TrelloClient.h"
 
const String TrelloClient::_baseUrl = "https://api.trello.com/1/";

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

TrelloCard toTrelloCard(const JsonVariant &object) {
    TrelloCard card;
    card.id = object.getMember("id").as<trelloId_t>();
    card.name = object.getMember("name").as<String>();
    card.idList = object.getMember("idList").as<trelloId_t>();
    card.idBoard = object.getMember("idBoard").as<trelloId_t>();
    card.pos = object.getMember("pos").as<int>();
    return card;
}

bool TrelloClient::GetListsFromBoard(trelloId_t boardId, LinkedList<TrelloList> &lists) {
    String url = _baseUrl + "boards/" + boardId + "/lists?";
    _http.begin(url + _authParams);
    int code = _http.GET();

    if(!code) {
        _http.end();
        Serial.println("Error during http request");
        return false;
    }

    Stream &response = _http.getStream();
    StaticJsonDocument<768> doc;
    DeserializationError error = deserializeJson(doc, response);
    _http.end();

    if (error) {
        Serial.print(F("deserializeJson() failed with code "));
        Serial.println(error.f_str());
        return false;
    }

    Serial.printf("Elements in response: %d\n", doc.size());

    JsonArray root = doc.as<JsonArray>();

    for (JsonVariant value : root) {
        lists.add(toTrelloList(value));
    }

    return true;
}

bool TrelloClient::GetCardsFromBoard(trelloId_t boardId, LinkedList<TrelloCard> &cards) {
    String url = _baseUrl + "boards/" + boardId + "/cards?"
        + "fields=id,idBoard,idList,name,pos&";
    _http.begin(url + _authParams);
    int code = _http.GET();

    if (!code) {
        _http.end();
        Serial.println("Error during http request");
        return false;
    }

    Serial.print("HTTP code=");
    Serial.println(code);

    Stream &response = _http.getStream();
    StaticJsonDocument<2048> doc;
    DeserializationError error = deserializeJson(doc, response);
    _http.end();

    if (error) {
        Serial.print(F("deserializeJson() failed with code "));
        Serial.println(error.f_str());
        return false;
    }

    Serial.printf("Elements in response: %d\n", doc.size());

    JsonArray root = doc.as<JsonArray>();

    for (JsonVariant value : root) {
        cards.add(toTrelloCard(value));
    }

    return true;
}
