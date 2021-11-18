#include "TrelloClient.h"
 
const String TrelloClient::_baseUrl = "https://api.trello.com/1/";

TrelloClient::TrelloClient(String apiKey, String token)
    : _http(), _authParams("key=" + apiKey + "&token=" + token) {
}

TrelloList toTrelloList(const JsonVariant &object) {
    TrelloList list;
    list.id = object.getMember("id").as<trelloId_t>();
    list.name = object.getMember("name").as<String>();
    return list;
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
        TrelloList list;
        list.id = value["id"].as<trelloId_t>();
        list.name = value["name"].as<String>();
        lists.add(list);
    }

    return true;
}