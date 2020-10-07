//
//  TelegramSession.hpp
//  TelegramBotEqIN
//
//  Created by Dipankar Goswami on 05/10/20.
//

#ifndef TelegramSession_h
#define TelegramSession_h

#include <cpprest/http_client.h>
#include "NSEMarketActivityDataManager.hpp"

struct ProductRequestDetails {
    std::string chatId_;
    std::string productId_;
};

class TelegramSession {

public:
    TelegramSession(std::string fileName, NSEMarketActivityDataManager& dataMgr);
    ~TelegramSession();

    typedef struct ProductRequestDetails RequestDetails;
    
    void run_loop();

private:
    pplx::task<void> RequestJSONValueAsync();
    void processMessage(const web::json::value& v);
    void sendResponse(RequestDetails req);

    NSEMarketActivityDataManager dataMgr_;
    web::http::client::http_client* telegramClient_;
    int update_id;
    std::vector<RequestDetails> requests_;
};

#endif /* TelegramSession_h */
