//
// Created by heimy4prez on 4/26/18.
//
#include "uthreads.h"
#include <stdio.h>
#include <cstdlib>
#include <machine/setjmp.h>
#include <setjmp.h>

const int RET_ERR = (-1);
const int RET_SUCCESS = 0;

const int ID_SCHEDUELER = 0;
const int ID_FIRST_USER_THREAD = 1;

#ifdef __x86_64__
/* code for 64 bit Intel arch */

typedef unsigned long address_t;
#define JB_SP 6
#define JB_PC 7

/* A translation is required when using an address of a variable.
   Use this as a black box in your code. */
address_t translate_address(address_t addr)
{
    address_t ret;
    asm volatile("xor    %%fs:0x30,%0\n"
            "rol    $0x11,%0\n"
    : "=g" (ret)
    : "0" (addr));
    return ret;
}

#else
/* code for 32 bit Intel arch */

typedef unsigned int address_t;
#define JB_SP 4
#define JB_PC 5

/* A translation is required when using an address of a variable.
   Use this as a black box in your code. */
address_t translate_address(address_t addr)
{
    address_t ret;
    asm volatile("xor    %%gs:0x18,%0\n"
		"rol    $0x9,%0\n"
                 : "=g" (ret)
                 : "0" (addr));
    return ret;
}
#endif

typedef enum _State {
    READY,
    RUNNING,
    BLOCKED,
    NUM_OF_STATES
} State;

typedef enum _Status {
    ALIVE,
    TERMINATED,
    NUM_OF_STATUSES
} Status;

/* Data per thread. */
typedef struct _Thread {
    address_t sp;   // Stack Pointer: Address of thread's stack head
    address_t pc;   // Program Counter: Address of thread's current instruction
    State state;    // Scheduling State: one of [Ready, Running, Blocked]
    Status status;  // Thread Status: alive or terminated.

    _Thread()
    {
        sp = NULL;
        pc = NULL;
        state = State::READY;
        status = Status::TERMINATED;
    }

    _Thread(address_t sp, address_t pc, State state, Status status)
    {
        this->sp = sp;
        this->pc = pc;
        this->state = state;
        this->status = status;
    }
} Thread;

/* Scheduler thread stack */
char stack_scheduler[STACK_SIZE];

/* Scheduler thread function */
// TODO: Implement real scheduler function
int scheduler_f(){
    printf("Hi i'm the scheduler\r\n");
    return RET_SUCCESS;
}

/* Global counter for all quantums. */
static int total_quantums;

/* Threads descriptor lookup table. */
Thread thread_list[MAX_THREAD_NUM]; // #todo Should I send MAX_THREAD_NUM-1 (for main thread?)
sigjmp_buf env[MAX_THREAD_NUM];

int uthread_init(int quantum_usecs){
    if (quantum_usecs < 0){
        return RET_ERR;
    }
    total_quantums = 1;

    // quantum timer + scheduler
    // TODO: Init scheduler thread
    auto sp = (address_t)stack_scheduler + STACK_SIZE - sizeof(address_t);
    auto pc = (address_t)scheduler_f;
    thread_list[ID_SCHEDUELER] = Thread(sp, pc, State::RUNNING, Status::ALIVE);
    sigsetjmp(env[ID_SCHEDUELER], 1);
    (env[0]->__jmpbuf)[JB_SP] = translate_address(sp);
    (env[0]->__jmpbuf)[JB_PC] = translate_address(pc);
    sigemptyset(&env[0]->__saved_mask);
 
    // Init user threads
    for (int thread_id = ID_FIRST_USER_THREAD; thread_id < MAX_THREAD_NUM; thread_id++){
        thread_list[thread_id] = Thread();
    }

    return RET_SUCCESS;
}

int uthread_spawn(void (*f)()){
    int id;

    // Find the minimal ID not yet taken.
    for (id = 1; id < MAX_THREAD_NUM; id++){
        if (thread_list[id].status == Status::TERMINATED){ // Can be shortened to if(..status)
            thread_list[id].status = Status::ALIVE;
            break;
        }
    }

    if (id == MAX_THREAD_NUM){
        return RET_ERR; // No more room for threads
    }

    thread_list[id].sp = (address_t )malloc(STACK_SIZE);

    if (thread_list[id].sp == NULL){
        return RET_ERR; // Unsuccessful malloc
    }

    // State is also init to

    //todo: Add the thread to the end of the ready list.

    return id;
}

// TODO: Implement
int uthread_terminate(int tid){
    printf("pass\r\n");
    return RET_SUCCESS;
}
// TODO: Implement
int uthread_block(int tid){
    printf("pass\r\n");
    return RET_SUCCESS;
}

// TODO: Implement
int uthread_resume(int tid){
    printf("pass\r\n");
    return RET_SUCCESS;
}

// TODO: Implement
int uthread_sync(int tid){
    printf("pass\r\n");
    return RET_SUCCESS;
}

// TODO: Implement
int uthread_get_tid(){
    printf("pass\r\n");
    return RET_SUCCESS;
}

// TODO: Implement
int uthread_get_total_quantums(){
    return total_quantums;
}

// TODO: Implement
int uthread_get_quantums(int tid){
    printf("pass\r\n");
    return RET_SUCCESS;
}