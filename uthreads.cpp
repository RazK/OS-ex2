//
// Created by heimy4prez on 4/26/18.
//
#include "uthreads.h"
#include "uthread.h"
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
UThread thread_list[MAX_THREAD_NUM]; // #todo Should I send MAX_THREAD_NUM-1 (for main thread?)
sigjmp_buf env[MAX_THREAD_NUM];

std::queue <UThreadID> ready_queue;
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
    thread_list[ID_SCHEDUELER] = UThread(sp, pc, State::RUNNING, Status::ALIVE);
    sigsetjmp(env[ID_SCHEDUELER], 1);
    (env[0]->__jmpbuf)[JB_SP] = translate_address(sp);
    (env[0]->__jmpbuf)[JB_PC] = translate_address(pc);
    sigemptyset(&env[0]->__saved_mask);
 
    // Init user threads
    for (int thread_id = ID_FIRST_USER_THREAD; thread_id < MAX_THREAD_NUM; thread_id++){
        thread_list[thread_id] = UThread();
    }

    // todo: Should we already enter infinite loop of running threads as they come?

    return RET_SUCCESS;
}

int uthread_spawn(void (*f)()){
    Mask m{MaskingCode::SCHEDULER}; // masking object
    int id;

    // Find the minimal ID not yet taken.
    for (id = 1; id < MAX_THREAD_NUM; id++){
        if (thread_list[id].GetStatus() == Status::TERMINATED){ // Can be shortened to if(..status)

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

    // Set thread as alive for use
    if (ErrorCode::SUCCESS != thread_list[id].SetStatus(Status::ALIVE)) {
        return RET_ERR;
    }

    // Set thread as ready for use
    if (ErrorCode::SUCCESS != thread_list[id].SetState(State::READY)){
        return RET_ERR;
    }

    ready_queue.push((UThreadID)id); // cast for type protection

    //todo: run f?

    return id;
}


int uthread_terminate(int tid){
    Mask m{MaskingCode::SCHEDULER}; // masking object
    if (tid < 0 || tid > MAX_THREAD_NUM){
        return RET_ERR;
    }

    if (Status::TERMINATED != thread_list[tid].GetStatus()){ // Already terminated or doesn't exist
        return RET_ERR;
    }

    //todo: check state_? what if it is running or blocked?

    if (ErrorCode::SUCCESS != thread_list[tid].SetStatus(Status::TERMINATED)){
        return RET_ERR;
    }

    if (ErrorCode::SUCCESS != thread_list[tid].SetState(State::READY)){
        return RET_ERR;
    }

    // todo: free memory of sp. Problem because sp is an int for some reason
    // Not removing from ready queue. Rather, this is responsibility of scheduler to assert threads
    // are alive

    // Iterate the queue of synced threads to allow them to run
    while (!thread_list[tid].IsSyncedEmpty()){
        UThreadID synced_thread = thread_list[tid].FrontSynced();
        // TODO: RazK: (below) What if this thread was blocking? why did we decide it's READY?
        if (ErrorCode::SUCCESS != thread_list[tid].SetState(State::READY)){
            return RET_ERR;
        }
        ready_queue.push(synced_thread);
        // No need to check if synced thread is alive. This is responsibility of scheduler.
        if (ErrorCode::SUCCESS != thread_list[tid].PopSynced()){
            return RET_ERR;
        }
    }

    return RET_SUCCESS;
}

int uthread_block(int tid){
    Mask m{MaskingCode::SCHEDULER}; // masking object
    if (tid <= 0 || tid > MAX_THREAD_NUM){ // trying to block main thread or boundary error
        return RET_ERR;
    }

    // no such thread exists?
    if (Status::TERMINATED == thread_list[tid].GetStatus()){
        return RET_ERR;
    }

    if (ErrorCode::SUCCESS != thread_list[tid].SetState(State::BLOCKED)){
        return RET_ERR;
    }

    // TODO: Get the real running_thread
    UThreadID running_thread = 0;
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

    if (Status::TERMINATED == thread_list[tid].GetStatus()){ // no such thread exists
        return RET_ERR;
    }

    // set as ready, without overwriting running state_ if necessary
    if (State::BLOCKED == thread_list[tid].GetState()) {
        if (ErrorCode::SUCCESS != thread_list[tid].SetState(State::READY)){
            return RET_ERR;
        }
    }

    return RET_SUCCESS;
}


int uthread_sync(int tid){
    Mask m{MaskingCode::SCHEDULER}; // masking object
    if (tid < 0 || tid > MAX_THREAD_NUM || Status::TERMINATED == thread_list[tid].GetStatus()){
        return RET_ERR;
    }

    //todo: add check to see if RUNNING thread, the one that called this function, is 0 (scheduler)
    int callee_id = uthread_get_tid();
    if (ErrorCode::SUCCESS != thread_list[callee_id].SetState(BLOCKED)){
        return RET_ERR;
    }

    if (ErrorCode::SUCCESS != thread_list[tid].PushSynced((UThreadID)callee_id)){
        return RET_ERR;
    }

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