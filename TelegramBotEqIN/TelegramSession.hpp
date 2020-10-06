//
//  TelegramSession.hpp
//  TelegramBotEqIN
//
//  Created by Dipankar Goswami on 05/10/20.
//

#ifndef TelegramSession_h
#define TelegramSession_h

#include <cpprest/http_client.h>

class TelegramSession {

public:
    TelegramSession(std::string fileName);
    ~TelegramSession();

    void run_loop();

private:

    pplx::task<void> RequestJSONValueAsync();
    void processMessage(const web::json::value& v);

    web::http::client::http_client* telegramClient_;
    int update_id;
};

#endif /* TelegramSession_h */
