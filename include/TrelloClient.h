#ifndef trelloClient_h
#define trelloClient_h

#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <LinkedList.h>

#define trelloId_t String

struct TrelloList {
    trelloId_t id;
    String name;
    int pos;
};

struct TrelloCard {
    trelloId_t id;
    String name;
    trelloId_t idList;
    trelloId_t idBoard;
    int pos;
};

class TrelloClient {
    public:
        TrelloClient(String apiKey, String token);
        bool GetListsFromBoard(trelloId_t boardId, LinkedList<TrelloList> &lists);
        bool GetCardsFromBoard(trelloId_t boardId, LinkedList<TrelloCard> &cards);
        bool GetCardsFromList(trelloId_t listId, LinkedList<TrelloCard> &cards);
    
    private:
        bool FetchAndParse(String url, JsonDocument &doc);
        bool GetCardsFromUrl(String url, LinkedList<TrelloCard> &cards);

        static const String _baseUrl;
        HTTPClient _http;
        String _authParams;
};

#endif