//
// Created by heimy4prez on 4/26/18.
//
#include <zconf.h>
#include "uthreads.h"
#include "stdio.h"
#define QUANTUM_USECS (3)
#define RET_ERR (-1)

#define SECOND 1000000
#define STACK_SIZE 4096

void f(void)
{
    int i = 0;
    while(1){
        ++i;
        printf("in f (%d)\n",i);
        if (i % 3 == 0) {
            printf("f: switching\n");
            switch_threads();
        }
        usleep(SECOND);
    }
}

void g(void)
{
    int i = 0;
    while(1){
        ++i;
        printf("in g (%d)\n",i);
        if (i % 5 == 0) {
            printf("g: switching\n");
            switch_threads();
        }
        usleep(SECOND);
    }
}

int jmp_test(){
    uthread_init(3000000);
    uthread_spawn(&f);
    uthread_spawn(&g);
}

int main(){

    int return_val;

    printf("~~~~~~ Running Tests on uthreads library. ~~~~~~\r\n\n");
    // Initiate the library
    printf("Testing Jump Test\r\n");
    jmp_test();

    printf("Testing init with quantum = %d microsecond\r\n", QUANTUM_USECS);
    return_val = uthread_init(QUANTUM_USECS);

    if (return_val == RET_ERR){
        printf("Failure to init uthreads library.\r\n");
        return RET_ERR;
    }

    // assert legal opening state_.
    return_val = uthread_get_total_quantums();
    if (return_val != 1){
        printf("Failure after init of uthreads library: Total number of quantums incorrect.\r\n"
               "Expected 1, got %d.\r\n", return_val);
        return RET_ERR;
    }
    printf("Passed init test.\r\n\n");

    //open threads


    printf("~~~~~ Passed all tests. ~~~~~\r\n\n");


}



