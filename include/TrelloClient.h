#ifndef trelloClient_h
#define trelloClient_h

#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <LinkedList.h>

#define trelloId_t String

struct TrelloList {
    trelloId_t id;
    String name;
};

class TrelloClient {
    public:
        TrelloClient(String apiKey, String token);
        bool GetListsFromBoard(trelloId_t boardId, LinkedList<TrelloList> &lists);
    
    private:
        static const String _baseUrl;
        HTTPClient _http;
        String _authParams;
};

#endif