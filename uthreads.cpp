//
// Created by heimy4prez on 4/26/18.
//
#include "uthreads.h"
#include <stdio.h>
#include <cstdlib>
#include <setjmp.h>
#include <signal.h>
#include <queue>
#include <list>

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

typedef u_int8_t threadID;

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
    //address_t sp;   // Stack Pointer: Address of thread's stack head
    //address_t pc;   // Program Counter: Address of thread's current instruction
    State state;    // Scheduling State: one of [Ready, Running, Blocked]
    Status status;  // Thread Status: alive or terminated.
    std::queue <threadID> synced_threads; // All the threads that called "sync" for this thread.
    // TODO: allocate memory in CTOR for this queue
    // TODO: free queue's memory in DTOR

    _Thread()
    {
        //sp = NULL;
        //pc = NULL;
        state = State::READY;
        status = Status::TERMINATED;
        // TODO: allocate synced_threads queue
    }

    _Thread(address_t sp, address_t pc, State state, Status status)
    {
        //this->sp = sp;
        //this->pc = pc;
        this->state = state;
        this->status = status;
        // TODO: free synced_threads queue
    }
} Thread;

typedef enum _MaskingCode {
    SCHEDULER,
    BLOCKING,
    NUMBER_OF_CODES

} MaskingCode;

// A masking object. This short lived object is intended to call the appropriate masking function
// in any given scenario, and call the reciprocal unmask function when the scope is exited.
typedef struct _Mask{

    _Mask(MaskingCode code){
        if (code == SCHEDULER){
            //todo save old mask, and then add appropriate mask settings per scenario (code)
            sigmask(1);
        }
        if (code == BLOCKING){
            sigmask(1);
        }
//        return RET_SUCCESS;
    }

    ~_Mask(){
        //todo reinstate old mask
        sigmask(1);
    }

} Mask;

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

std::queue <threadID> ready_queue;
//std::list <std::queue>

int uthread_init(int quantum_usecs){
    Mask m{MaskingCode::SCHEDULER}; // masking object
    if (quantum_usecs < 0){
        return RET_ERR;
    }
    total_quantums = 1;

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

    // todo: Should we already enter infinite loop of running threads as they come?

    return RET_SUCCESS;
}

int uthread_spawn(void (*f)()){
    Mask m{MaskingCode::SCHEDULER}; // masking object
    int id;

    // Find the minimal ID not yet taken.
    for (id = 1; id < MAX_THREAD_NUM; id++){
        if (thread_list[id].status == Status::TERMINATED){ // Can be shortened to if(..status)

            break;
        }
    }

    if (id == MAX_THREAD_NUM){
        return RET_ERR; // No more room for threads
    }

    //thread_list[id].sp = (address_t )malloc(STACK_SIZE);

    //if (thread_list[id].sp == NULL){
    //    return RET_ERR; // Unsuccessful malloc
    //}

    thread_list[id].status = Status::ALIVE; // Set thread as alive for use

    thread_list[id].state = State::READY;

    ready_queue.push((threadID)id); // cast for type protection

    //todo: run f?

    return id;
}


int uthread_terminate(int tid){
    Mask m{MaskingCode::SCHEDULER}; // masking object
    if (tid < 0 || tid > MAX_THREAD_NUM){
        return RET_ERR;
    }

    if (thread_list[tid].status == Status::TERMINATED){ // Already terminated or doesn't exist
        return RET_ERR;
    }

    //todo: check state? what if it is running or blocked?

    thread_list[tid].status = Status::TERMINATED;
    thread_list[tid].state = State::READY;
    // todo: free memory of sp. Problem because sp is an int for some reason
    // Not removing from ready queue. Rather, this is responsibility of scheduler to assert threads
    // are alive

    // Iterate the queue of synced threads to allow them to run
    while (!thread_list[tid].synced_threads.empty()){
        threadID synced_thread = thread_list[tid].synced_threads.front();
        // TODO: RazK: (below) What if this thread was blocking? why did we decide it's READY?
        thread_list[synced_thread].state = READY;
        ready_queue.push(synced_thread);
        // No need to check if synced thread is alive. This is responsibility of scheduler.
        thread_list[tid].synced_threads.pop();
    }

    return RET_SUCCESS;
}

int uthread_block(int tid){
    Mask m{MaskingCode::SCHEDULER}; // masking object
    if (tid <= 0 || tid > MAX_THREAD_NUM){ // trying to block main thread or boundary error
        return RET_ERR;
    }
    if (thread_list[tid].status == Status::TERMINATED){ // no such thread exists
        return RET_ERR;
    }
    thread_list[tid].state = State::BLOCKED;

    // TODO: Get the real running_thread
    threadID running_thread = 0;
    if (running_thread == tid){ // thread blocking itself
        //todo scheduling decision
    }

    return RET_SUCCESS;
}


int uthread_resume(int tid){
    Mask m{MaskingCode::SCHEDULER}; // masking object
    if (tid <= 0 || tid > MAX_THREAD_NUM){ // trying to resume main thread or boundary error
        return RET_ERR;
    }
    if (thread_list[tid].status == Status::TERMINATED){ // no such thread exists
        return RET_ERR;
    }

    // set as ready, without overwriting running state if necessary
    if (thread_list[tid].state == State::BLOCKED) {
        thread_list[tid].state = State::READY;
    }

    return RET_SUCCESS;
}


int uthread_sync(int tid){
    Mask m{MaskingCode::SCHEDULER}; // masking object
    if (tid < 0 || tid > MAX_THREAD_NUM || thread_list[tid].status == Status::TERMINATED){
        return RET_ERR;
    }
    //todo: add check to see if RUNNING thread, the one that called this function, is 0 (scheduler)
    int callee_id = uthread_get_tid();
    thread_list[callee_id].state = BLOCKED;
    thread_list[tid].synced_threads.push((threadID)callee_id);

    //todo: make scheduling decision.

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