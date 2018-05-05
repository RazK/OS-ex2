//
// Created by razkarl on 5/5/18.
//

#ifndef PROJECT_MASK_H
#define PROJECT_MASK_H

typedef enum _MaskingCode {
    SCHEDULER,
    BLOCKING,
    NUMBER_OF_CODES

} MaskingCode;

// A masking object. This short lived object is intended to call the appropriate masking function
// in any given scenario, and call the reciprocal unmask function when the scope is exited.
class Mask{

public:
    Mask(MaskingCode code);
    ~Mask();

private:
    sigset_t old_set{};
    sigset_t cur_set{};

};

#endif //PROJECT_MASK_H
