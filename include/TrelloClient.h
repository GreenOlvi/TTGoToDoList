#ifndef trelloClient_h
#define trelloClient_h

#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <LinkedList.h>

typedef String trelloId_t;

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
    trelloId_t *idChecklists;
    int idChecklistsSize;
};

struct TrelloCheckitem {
    trelloId_t id;
    String name;
    trelloId_t idChecklist;
    bool complete;
    int pos;
};

struct TrelloChecklist {
    trelloId_t id;
    String name;
    trelloId_t idCard;
    trelloId_t idBoard;
    TrelloCheckitem *checkItems;
    int checkItemSize;
};

class TrelloClient {
    public:
        TrelloClient(const String apiKey, const String token);
        bool GetListsFromBoard(const trelloId_t boardId, LinkedList<TrelloList> &lists);
        bool GetCardsFromBoard(const trelloId_t boardId, LinkedList<TrelloCard> &cards);
        bool GetCardsFromList(const trelloId_t listId, LinkedList<TrelloCard> &cards);
        TrelloChecklist* GetChecklist(const trelloId_t checklistId);
        bool GetCheckItemsFromChecklist(const trelloId_t checklistId, LinkedList<TrelloCheckitem> &items);
    
    private:
        bool FetchAndParse(const String url, JsonDocument &doc);
        bool GetCardsFromUrl(const String url, LinkedList<TrelloCard> &cards);

        static const String _baseUrl;
        static const String _cardFields;
        HTTPClient _http;
        String _authParams;
};

#endif