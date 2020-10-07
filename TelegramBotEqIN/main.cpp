//
//  main.cpp
//  TelegramBotEqIN
//
//  Created by Dipankar Goswami on 05/10/20.
//

#include <iostream>
#include "TelegramSession.hpp"
#include "NSEMarketActivityDataManager.hpp"

int main(int argc, const char * argv[]) {

    NSEMarketActivityDataManager n;
    n.prepareData();

    TelegramSession t(argv[1], n);
    t.run_loop();

    return 0;
}
