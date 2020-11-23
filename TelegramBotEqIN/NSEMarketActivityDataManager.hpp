//
//  NSEMarketActivityDataManager.hpp
//  TelegramBotEqIN
//
//  Created by Dipankar Goswami on 07/10/20.
//

#ifndef NSEMarketActivityDataManager_hpp
#define NSEMarketActivityDataManager_hpp

#include <iostream>
#include <map>

struct InstrumentFiancials {
    std::string closePrice_;
    std::string tradedVal_;
    std::string tradedQty_;
};

class NSEMarketActivityDataManager {
public:
    typedef struct InstrumentFiancials NSEEQFinaniclas;

    void prepareData(bool forceDownload = false);
    bool hasDataForProduct(std::string product);
    NSEEQFinaniclas getDataForProduct(std::string product);

private:
    bool downloadCompleted_;
    std::map<std::string, NSEEQFinaniclas> instrumentData_;
};

#endif /* NSEMarketActivityDataManager_hpp */
