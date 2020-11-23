//
//  DataFileDownloadHelper.hpp
//  TelegramBotEqIN
//
//  Created by Dipankar Goswami on 23/11/20.
//

#ifndef DataFileDownloadHelper_hpp
#define DataFileDownloadHelper_hpp

#include <string>

class DataFileDownloadHelper {
public:
    static void downloadFile(const std::string& url, const std::string& fileName);
    static std::string getFileName(const std::string& filePrefix, const std::string& dateformat);
};

#endif /* DataFileDownloadHelper_hpp */
