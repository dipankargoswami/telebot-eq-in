//
//  NSEMarketActivityDataManager.cpp
//  TelegramBotEqIN
//
//  Created by Dipankar Goswami on 07/10/20.
//

#include "NSEMarketActivityDataManager.hpp"
#include "DataFileDownloadHelper.hpp"
#include <curl/curl.h>
#include <fstream>
#include <sstream>
#include <vector>

void NSEMarketActivityDataManager::prepareData(bool forceDownload/* = false*/) {
    std::string fileName = DataFileDownloadHelper::getFileName("MA", "%d%m%y");

    std::ifstream fin;
    fin.open(fileName);
 
    if (!fin || forceDownload) {
        fin.close();
        DataFileDownloadHelper::downloadFile("https://www1.nseindia.com/archives/equities/mkt/", fileName);
        fin.open(fileName);
    }
 
    if (!fin) {
        return;
    }

    std::string line;
    bool found = false;
    
    while (fin) {
        // Read a Line from File
        getline(fin, line);
        
        if (line.compare(",Securities Price Volume Data in Normal market") == 0) {
            found = true;
            getline(fin, line);
            continue;
        }
        else if (!found) {
            continue;
        }
        
        std::istringstream iss(line.c_str());
        std::string token;

        std::vector<std::string> propertyList;
        while(std::getline(iss, token, ',')) {
            propertyList.push_back(token);
        }

        if (propertyList.size() < 6 || propertyList[2] != "EQ") {
            continue;
        }

        NSEEQFinaniclas nf;
        for(int i = 3; i < 6; i++) {
            std::string val = propertyList[i];
            val.erase(0, val.find_first_not_of(" "));
            switch(i) {
                case 3:
                    nf.closePrice_  = val;
                    break;
                case 4:
                    nf.tradedVal_ = val;
                    break;
                case 5:
                    nf.tradedQty_ = val;
            }
        }
        
        instrumentData_[propertyList[1]] = nf;
    }
    fin.close();
}

bool NSEMarketActivityDataManager::hasDataForProduct(std::string product) {
    return (instrumentData_.find(product) != instrumentData_.end());
}

NSEMarketActivityDataManager::NSEEQFinaniclas NSEMarketActivityDataManager::getDataForProduct(std::string product) {
    return instrumentData_[product];
}
