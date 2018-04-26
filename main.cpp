//
// Created by heimy4prez on 4/26/18.
//
#include "uthreads.h"
#include "stdio.h"
#define QUANTUM_USECS 3
#define ERR (-1)

int main(){

    int return_val;

    // Initiate the library
    return_val = uthread_init(QUANTUM_USECS);
    if (return_val == ERR){
        printf("Failure to init uthreads library.\r\n");
    }

    // assert legal opening state.
    return_val = uthread_get_total_quantums();
    if (return_val != 1){
        printf("Failure after init of uthreads library: Total number of quantums incorrect.\r\n"
               "Expected 1, got %d.\r\n", return_val);
    }

    //open threads


}
