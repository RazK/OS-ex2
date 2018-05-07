//
// Created by razkarl on 5/5/18.
//

#ifndef PROJECT_MASK_H
#define PROJECT_MASK_H

#include <csignal>
#include "err_codes.h"
#include <iostream>

// A masking object. This short lived object is intended to call the appropriate masking function
// in any given scenario, and call the reciprocal unmask function when the scope is exited.
class Mask{

public:
    Mask();
    ~Mask();

private:
    sigset_t old_set{};
    sigset_t cur_set{};

};

#endif //PROJECT_MASK_H
