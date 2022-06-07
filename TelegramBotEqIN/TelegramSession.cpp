//
//  TelegramSession.cpp
//  TelegramBotEqIN
//
//  Created by Dipankar Goswami on 05/10/20.
//

#include <iostream>
#include <cstdlib>
#include <boost/program_options.hpp>

#include "TelegramSession.hpp"

using namespace web;
using namespace web::http;
using namespace web::http::client;

namespace po = boost::program_options;

TelegramSession::TelegramSession(std::string fileName, NSEMarketActivityDataManager& dataMgr) : dataMgr_(dataMgr), telegramClient_(NULL), update_id(0) {
    
    po::options_description config("Configuration");
    config.add_options()
        ("Bot_Key", po::value<std::string>()->default_value(""), "API Key assigned by Telegram for the bot");
    
    boost::program_options::variables_map vm;

    std::ifstream ifs(fileName);
    if (ifs) {
        boost::program_options::store(boost::program_options::parse_config_file(ifs, config), vm);
        boost::program_options::notify(vm);
    }
    
    std::string key = vm["Bot_Key"].as<std::string>();
    telegramClient_ = new http_client(U("https://api.telegram.org/bot" + key));
}

TelegramSession::~TelegramSession() {
    delete telegramClient_;
    telegramClient_ = NULL;
}

void TelegramSession::run_loop() {
    auto next_update_time = 0L;
    while(true) {
        if(time(NULL) < next_update_time) {
            continue;
        }

        initMsgs_.clear();
        validRequests_.clear();
        invalidRequests_.clear();

        next_update_time = time(NULL) + 60;
        RequestJSONValueAsync().wait();
        
        for(auto r: initMsgs_) {
            sendUsage(r);
        }

        for(auto r: validRequests_) {
            sendResponse(r);
        }

        for(auto r: invalidRequests_) {
            sendErrorResponse(r);
        }
    }
}

pplx::task<void> TelegramSession::RequestJSONValueAsync()
{
    char updIdStr[50];
    sprintf(updIdStr,"%llu",update_id + 1);
    std::string queryParam(U("{\"limit\":20, \"offset\":" + std::string(updIdStr) +"}"));
    
    return telegramClient_->request(methods::GET, "/getUpdates", queryParam, "application/json").then([](http_response response) -> pplx::task<json::value> {
        if(response.status_code() == status_codes::OK)
        {
            return response.extract_json();
        }
                
        // Handle error cases, for now return empty json value...
        return pplx::task_from_result(json::value());
    })
    .then([&](pplx::task<json::value> previousTask) {
        try {
            const json::value& v = previousTask.get();
            if (!v.is_null())
            {
                this->processMessage(v);
            }
        } catch (const http_exception& e) {
            // Print error.
            std::cout << e.what() << std::endl;
        }
    });
}

void TelegramSession::processMessage(const json::value& v) {
    auto iter = v.as_object().find("result");
    if (iter != v.as_object().cend()) {
        const json::value &msgList = iter->second;
        for (int i = 0; i < msgList.as_array().size(); i++) {
            const json::value &msg = msgList.as_array().at(i);
            auto msgIter = msg.as_object().find("message");
            
            auto txtIter = msgIter->second.as_object().find("text");
            if (txtIter == msg.as_object().cend()) {
                std::cout << "Received empty message." << std::endl;
                continue;
            }
            std::string msgText = txtIter->second.as_string().c_str();
            std::cout << "Received Text [" << msgText << ']' << std::endl;

            auto chatIter = msgIter->second.as_object().find("chat");
            unsigned long long userid = 0;
            if (chatIter != msg.as_object().cend()) {
                auto idIter = chatIter->second.as_object().find("id");
                userid = idIter->second.as_number().to_int64();
            }
            auto updIdIter = msg.as_object().find("update_id");
            this->update_id = updIdIter->second.as_number().to_int64();

            char chatIdStr[50];
            sprintf(chatIdStr,"%llu",userid);

            if(msgText == "/start") {
                initMsgs_.push_back(chatIdStr);
                continue;
            }

            std::vector<std::string> tokens;
            size_t start_pos = 0;
            while(start_pos < msgText.length())
            {
                size_t nextpos = msgText.find(' ', start_pos);
                if(nextpos == std::string::npos || tokens.size() == 2)
                {
                    tokens.push_back(msgText.substr(start_pos));
                    break;
                } else
                {
                    tokens.push_back(msgText.substr(start_pos, nextpos - start_pos));
                }
                start_pos = nextpos + 1;
            }

            if (tokens.size() != 3 || tokens[0] != "TDSUMM" || tokens[1] != "NSE") {
                invalidRequests_.push_back(chatIdStr);
                continue;
            }

            TelegramSession::RequestDetails  r;
            r.chatId_ = chatIdStr;
            r.productId_ = tokens[2];
            std::cout << "Data requested for product [" << r.productId_ << ']' << std::endl;
            validRequests_.push_back(r);
        }
    }
}

void TelegramSession::sendResponse(TelegramSession::RequestDetails req) {
    std::string replyText = "You seem to have entered an Invalid instrument";
    
    if (dataMgr_.hasDataForProduct(req.productId_)) {
        NSEMarketActivityDataManager::NSEEQFinaniclas nf = dataMgr_.getDataForProduct(req.productId_);
        replyText = "For product :" + req.productId_ + "\n" \
        "Close Price : " + nf.closePrice_ + "\n" \
        "Traded Value : " + nf.tradedVal_ + "\n" \
        "Traded Qty : " + nf.tradedQty_ + "\n";
    }
    
    std::string queryParam2(U("{\"text\":\"" + replyText +"\" , \"chat_id\":" + req.chatId_ +"}"));

    telegramClient_->request(methods::GET, "/sendMessage", queryParam2, "application/json");
}

void TelegramSession::sendUsage(std::string req) {
    std::string replyText = "Hi, Thank you for choosing our services.\n"\
                            "Please use the following format for receiving daily trade summary-\n"\
                            "TDSUMM NSE <Instrument>\n"\
                            "e.g. TDSUMM NSE INFY";

    std::string queryParam2(U("{\"text\":\"" + replyText +"\" , \"chat_id\":" + req +"}"));

    telegramClient_->request(methods::GET, "/sendMessage", queryParam2, "application/json");
}

void TelegramSession::sendErrorResponse(std::string req) {
    std::string replyText = "You have used an invalid format.\n"\
                            "Please use the following format-\n"\
                            "TDSUMM NSE <Instrument>\n"\
                            "e.g. TDSUMM NSE INFY";

    std::string queryParam2(U("{\"text\":\"" + replyText +"\" , \"chat_id\":" + req +"}"));

    telegramClient_->request(methods::GET, "/sendMessage", queryParam2, "application/json");
}
