// mask.cpp
// Created by razkarl on 5/5/18.
//
#include "mask.h"

Mask::Mask(){
    int return_val;

    if (RET_ERR == sigemptyset(&cur_set)) {
        std::cerr << MSG_SYSTEM_ERR << "Could not create empty set in Mask object constructor." << std::endl;
        exit(1);
    }

    if (RET_ERR == sigaddset(&cur_set, SIGVTALRM)) {
        std::cerr << MSG_SYSTEM_ERR << "Could not add signal to set in Mask object constructor." << std::endl;
        exit(1);
    }

    if (RET_ERR == sigprocmask(SIG_BLOCK, &cur_set, &old_set));{
        std::cerr << MSG_SYSTEM_ERR << "Could not update set using sigprocmask in Mask object constructor." << std::endl;
        exit(1);
    }
}

Mask::~Mask(){
    if (RET_ERR == sigprocmask(SIG_SETMASK, &old_set, nullptr)) {
        std::cerr << MSG_SYSTEM_ERR << "Could not restore mask in Mask object destructor." << std::endl;
        exit(1);
    }
}
