//
//  NSEMarketActivityDataManager.cpp
//  TelegramBotEqIN
//
//  Created by Dipankar Goswami on 07/10/20.
//

#include "NSEMarketActivityDataManager.hpp"
#include <curl/curl.h>
#include <fstream>
#include <sstream>
#include <vector>

static size_t write_data(void *ptr, size_t size, size_t nmemb, void *stream)
{
  size_t written = fwrite(ptr, size, nmemb, (FILE *)stream);
  return written;
}

void NSEMarketActivityDataManager::prepareData(bool forceDownload/* = false*/) {
    std::ifstream fin;
    fin.open(getFileName());
 
    if (!fin || forceDownload) {
        fin.close();
        downloadFile();
        fin.open(getFileName());
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

void NSEMarketActivityDataManager::downloadFile() {
    std::string downloadedFile = getFileName();
    std::string fullUrl = std::string("https://www1.nseindia.com/archives/equities/mkt/") + downloadedFile;

    std::cout << "Downloading file [" << fullUrl << ']' << std::endl;

    static const char *pagefilename = downloadedFile.c_str();
    FILE *pagefile;
    
    CURL *curl_handle;
    curl_global_init(CURL_GLOBAL_ALL);
    curl_handle = curl_easy_init();
    
    curl_easy_setopt(curl_handle, CURLOPT_URL, fullUrl.c_str());
    curl_easy_setopt(curl_handle, CURLOPT_VERBOSE, 0L);
    curl_easy_setopt(curl_handle, CURLOPT_NOPROGRESS, 1L);
    curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, write_data);

    pagefile = fopen(pagefilename, "wb");
    if(pagefile) {
        curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, pagefile);
        curl_easy_perform(curl_handle);
        fclose(pagefile);
    }
 
    curl_easy_cleanup(curl_handle);
    curl_global_cleanup();
}

std::string NSEMarketActivityDataManager::getFileName() {
    std::time_t t = std::time(nullptr);
    std::tm tm = *std::localtime(&t);

    if (tm.tm_wday == 0) {
        tm.tm_mday -= 2;
    } else if (tm.tm_wday == 6 || tm.tm_hour < 19) {
        tm.tm_mday -= 1;
    }

    char date[10];
    strftime(date, 10, "%d%m%y", &tm);
    
    std::string filename = date;
    filename = "MA" + filename + ".csv";
    return filename;
}
