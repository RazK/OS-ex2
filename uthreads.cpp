//
// Created by heimy4prez on 4/26/18.
//
#include "uthreads.h"
#include <stdio.h>

#define ERR (-1)
#define SUCCESS 0

static int total_quantums;


int uthread_init(int quantum_usecs){
    if (quantum_usecs < 0){
        return ERR;
    }
    total_quantums = 1;

    return SUCCESS;

}

int uthread_spawn(void (*f)(void)){
    printf("pass\r\n");
}

int uthread_terminate(int tid){
    printf("pass\r\n");
}

int uthread_block(int tid){
    printf("pass\r\n");
}

int uthread_resume(int tid){
    printf("pass\r\n");
}

int uthread_sync(int tid){
    printf("pass\r\n");
}

int uthread_get_tid(){
    printf("pass\r\n");

}

int uthread_get_total_quantums(){
    return total_quantums;

}

int uthread_get_quantums(int tid){
    printf("pass\r\n");

}
