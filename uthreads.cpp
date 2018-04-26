//
// Created by heimy4prez on 4/26/18.
//
#include "uthreads.h"
#include <stdio.h>
#include <cstdlib>

#define RET_ERR             (-1)
#define RET_SUCCESS         0

#define STATE_RUNNING       1
#define STATE_READY         0
#define STATE_BLOCKED       2

#define STATUS_ALIVE        true
#define STATUS_TERMINATED   false



/* Data per thread. */
typedef struct Thread {
    // Global thread id
    int id;

    // Schedualing State: one of [Ready, Running, Blocked]
    int state;

    void * SP; /* #todo What type should this pointer have? */
    // Should we save using a pointer to the stack and a positional argument?

    // Thread Status: alive or terminated.
    bool status;



} Thread;

/* Global counter for all quantums. */
static int total_quantums;

/* Data Structure holding all threads. */
Thread thread_list[MAX_THREAD_NUM]; // #todo Should I send MAX_THREAD_NUM-1 (for main thread?)

int uthread_init(int quantum_usecs){
    if (quantum_usecs < 0){
        return RET_ERR;
    }
    total_quantums = 1;

    // quantum timer + scheduler

    // init the list of threads
    // Did not start from 0 here, reserved first thread for main
    for (int i = 1; i < MAX_THREAD_NUM; i++){
        thread_list[i] = {i /* id */, STATE_READY /*state*/, nullptr /* SP */, STATUS_TERMINATED
                /* Status*/ };
    }



    return RET_SUCCESS;

}

int uthread_spawn(void (*f)()){
    int id;

    // Find the minimal ID not yet taken.
    for (id = 1; id < MAX_THREAD_NUM; id++){
        if (thread_list[id].status == STATUS_TERMINATED){ // Can be shortened to if(..status)
            thread_list[id].status = STATUS_ALIVE;
            break;
        }
    }

    if (id == MAX_THREAD_NUM){
        return RET_ERR; // No more room for threads
    }

    thread_list[id].SP = malloc(STACK_SIZE);

    if (thread_list[id].SP == NULL){
        return RET_ERR; // Unsuccessful malloc
    }

    // State is also init to

    //todo: Add the thread to the end of the ready list.

    return id;
}

int uthread_terminate(int tid){
    printf("pass\r\n");
    return RET_SUCCESS;
}

int uthread_block(int tid){
    printf("pass\r\n");
    return RET_SUCCESS;
}

int uthread_resume(int tid){
    printf("pass\r\n");
    return RET_SUCCESS;
}

int uthread_sync(int tid){
    printf("pass\r\n");
    return RET_SUCCESS;
}

int uthread_get_tid(){
    printf("pass\r\n");
    return RET_SUCCESS;

}

int uthread_get_total_quantums(){
    return total_quantums;

}

int uthread_get_quantums(int tid){
    printf("pass\r\n");
    return RET_SUCCESS;

}
