//
//  DataFileDownloadHelper.cpp
//  TelegramBotEqIN
//
//  Created by Dipankar Goswami on 23/11/20.
//

#include "DataFileDownloadHelper.hpp"

#include <curl/curl.h>
#include <iostream>

static size_t write_data(void *ptr, size_t size, size_t nmemb, void *stream)
{
  size_t written = fwrite(ptr, size, nmemb, (FILE *)stream);
  return written;
}

void DataFileDownloadHelper::downloadFile(const std::string& url, const std::string& downloadedFile) {
    std::string fullUrl = url + downloadedFile;
    
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

std::string DataFileDownloadHelper::getFileName(const std::string& filePrefix, const std::string& dateformat) {
    std::time_t t = std::time(nullptr);
    std::tm tm = *std::localtime(&t);
    
    if (tm.tm_wday == 0) {
        tm.tm_mday -= 2;
    } else if (tm.tm_wday == 1 && tm.tm_hour < 19) {
        tm.tm_mday -= 3;
    } else if (tm.tm_wday == 6 || tm.tm_hour < 19) {
        tm.tm_mday -= 1;
    }
    
    char date[10];
    strftime(date, 10, dateformat.c_str(), &tm);
    
    std::string filename = filePrefix + date + ".csv";
    return filename;
}
