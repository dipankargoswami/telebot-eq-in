//
//  main.cpp
//  TelegramBotEqIN
//
//  Created by Dipankar Goswami on 05/10/20.
//

#include <iostream>
#include "TelegramSession.hpp"

int main(int argc, const char * argv[]) {
    TelegramSession t(argv[1]);

    t.run_loop();

    return 0;
}
