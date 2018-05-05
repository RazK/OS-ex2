//
// Created by heimy4prez on 4/26/18.
//
#include "uthreads.h"
#include "uthread.h"
#include "mask.h"
#include "err_codes.h"
#include <stdio.h>
#include <cstdlib>
#include <setjmp.h>
#include <signal.h>
#include <queue>
#include <list>
#include <stdexcept>
#include <exception>
#include <iostream>
#include <zconf.h>

const int ID_SCHEDUELER = 0;
const int ID_FIRST_USER_THREAD = 1;
const int SIG_RET_FROM_JUMP = 1;

static UThreadID running_thread;


/* Scheduler thread stack */
char stack_scheduler[STACK_SIZE];
std::queue <UThreadID> ready_queue;

/* Global counter for all quantums. */
static int total_quantums;

/* Threads descriptor lookup table. */
UThread thread_list[MAX_THREAD_NUM]; // #todo Should I send MAX_THREAD_NUM-1 (for main thread?)
//sigjmp_buf env_list[MAX_THREAD_NUM];
//sigjmp_buf env[MAX_THREAD_NUM];

#define SECOND  10000000
void f1 (void)
{
    int i = 0;
    while(1){
        ++i;
        printf("in f (%d)\n",i);
        if (i % 3 == 0) {
            printf("f: switching\n");
            switch_threads();
        }
        usleep(0.2*SECOND);
    }
}

void switch_threads(void ){
    // Running thread goes to rest
    ready_queue.push((UThreadID) running_thread);
    thread_list[running_thread].SetState(State::READY);

    // Find first ready thread which is neither BLOCKED nor TERMINATED
    int nextUTID = -1;
    while (!ready_queue.empty()) {
        nextUTID = ready_queue.front();

        // Clean threads_list from non-ready threads as you go
        ready_queue.pop();

        // Found ready thread? break
        if (State::READY == thread_list[nextUTID].GetState()){
            break;
        }
    }

    // Validate ready_queue is not empty (FATAL ERROR)
    if (State::READY != thread_list[nextUTID].GetState()) {
        std::cerr << MSG_LIBRARY_ERR << "The queue of ready threads was found empty. Assumed not to occur." << std::endl;
        return;

    }

    // Save env for current thread and then jump to next one
    auto currentUTID = (UThreadID)uthread_get_tid();
//    int ret_val = sigsetjmp(thread_list[currentUTID].GetEnv(), 1);
    int ret_val = sigsetjmp(thread_list[nextUTID].env_, 1);
//    int ret_val = sigsetjmp(thread_list[], 1);
    if (ret_val == SIG_RET_FROM_JUMP){
        return;
    }
    running_thread = nextUTID;
//    siglongjmp(thread_list[nextUTID].GetEnv(), SIG_RET_FROM_JUMP);
    siglongjmp(thread_list[nextUTID].env_, SIG_RET_FROM_JUMP);
}

//std::list <std::queue>
void sig_alarm_handler(void){
    Mask m{};
    int cur_tid = uthread_get_tid();
    // Add this thread to the end of the line.
    ready_queue.push((UThreadID) cur_tid);
//    thread_list[cur_tid].SetEnv();
//    int ret_val = sigsetjmp(thread_list[cur_tid].SetEnv,1);

    if (ready_queue.empty()) {
        std::cerr << MSG_LIBRARY_ERR << "The queue of ready threads was found empty. Assumed not to occur.";
//        return RET_ERR;
    }
    UThreadID currentThread = ready_queue.front();
    ready_queue.pop();

//    siglongjmp(thread_list[currentThread].GetEnv());

//    return RET_SUCCESS;
}

int uthread_init(int quantum_usecs){
    Mask m{}; // masking object
    if (quantum_usecs < 0){
        return RET_ERR;
    }
    total_quantums = 1;
    running_thread = 0;



    // TODO: Init scheduler thread
//    auto sp = (address_t)stack_scheduler + STACK_SIZE - sizeof(address_t);
//    auto pc = (address_t)sig_alarm_handler;
//    thread_list[ID_SCHEDUELER] = UThread(sp, pc, State::RUNNING, Status::ALIVE);
//    sigsetjmp(env[ID_SCHEDUELER], 1);
//    (env[0]->__jmpbuf)[JB_SP] = translate_address(sp);
//    (env[0]->__jmpbuf)[JB_PC] = translate_address(pc);
//    sigemptyset(&env[0]->__saved_mask);
 
    // Init user threads
    for (int thread_id = ID_FIRST_USER_THREAD; thread_id < MAX_THREAD_NUM; thread_id++){
        thread_list[thread_id] = UThread{};
    }
    thread_list[0].InitEnv(&f1);


    return RET_SUCCESS;
}

int uthread_spawn(void (*f)()){
    Mask m{}; // masking object
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

    // Set thread as alive for use
    if (ErrorCode::SUCCESS != thread_list[id].SetStatus(Status::ALIVE)) {
        return RET_ERR;
    }

    // Set thread as ready for use
    if (ErrorCode::SUCCESS != thread_list[id].SetState(State::READY)){
        return RET_ERR;
    }

    ready_queue.push((UThreadID)id); // cast for type protection

    thread_list[id].InitEnv(f);
    return id;
}


int uthread_terminate(int tid){
    Mask m{}; // masking object
    if (tid < 0 || tid > MAX_THREAD_NUM){
        std::cerr << MSG_LIBRARY_ERR << "Attempting to terminate illegal thread id: ID " << tid << std::endl;
        return RET_ERR;
    }

    if (Status::TERMINATED == thread_list[tid].GetStatus()){ // Already terminated or doesn't exist
        std::cerr << MSG_LIBRARY_ERR << "Attempting to terminate a non-existent thread: ID " << tid << std::endl;
        return RET_ERR;
    }

    //todo: check state_? what if it is running or blocked?

    if (ErrorCode::SUCCESS != thread_list[tid].SetStatus(Status::TERMINATED)){
        return RET_ERR;
    }


    if (ErrorCode::SUCCESS != thread_list[tid].SetState(State::READY)){
        std::cerr << MSG_LIBRARY_ERR << "Could not set the state: ID " << tid << std::endl;
        return RET_ERR;
    }

    if (ErrorCode::SUCCESS != thread_list[tid].FreeStack()){
        std::cerr << MSG_LIBRARY_ERR << "Could not free memory for this thread: ID " << tid << std::endl;
        return RET_ERR;
    }

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
    Mask m{}; // masking object
    if (tid <= 0 || tid > MAX_THREAD_NUM){ // trying to block main thread or boundary error
        std::cerr << MSG_LIBRARY_ERR << "Attempting to block illegal thread id: ID " << tid << std::endl;
        return RET_ERR;
    }

    // no such thread exists?
    if (Status::TERMINATED == thread_list[tid].GetStatus()){
        std::cerr << MSG_LIBRARY_ERR << "Attempting to block non-existent thread id: ID " << tid << std::endl;
        return RET_ERR;
    }

    if (ErrorCode::SUCCESS != thread_list[tid].SetBlocked(BlockReason::REQUEST)){
        return RET_ERR;
    }

    int running_thread = uthread_get_tid();
    if (running_thread == tid){ // thread blocking itself
        //todo scheduling decision
    }

    return RET_SUCCESS;
}


int uthread_resume(int tid){
    Mask m{}; // masking object
    if (tid <= 0 || tid > MAX_THREAD_NUM){ // trying to resume main thread or boundary error
        std::cerr << MSG_LIBRARY_ERR << "Attempting to block illegal thread id: ID " << tid << std::endl;
        return RET_ERR;
    }

    if (Status::TERMINATED == thread_list[tid].GetStatus()){ // no such thread exists
        return RET_ERR;
    }

    if (ErrorCode::SUCCESS != thread_list[tid].UnBlock(BlockReason::REQUEST)){
        return RET_ERR;
    }

    if (State::READY == thread_list[tid].GetState()){
        ready_queue.push((UThreadID) tid);
    }

    return RET_SUCCESS;
}


int uthread_sync(int tid){
    Mask m{}; // masking object
    if (tid < 0 || tid > MAX_THREAD_NUM || Status::TERMINATED == thread_list[tid].GetStatus()){
        std::cerr << MSG_LIBRARY_ERR << "Attempting to block illegal thread id: ID " << tid << std::endl;
        return RET_ERR;
    }

    int callee_id = uthread_get_tid();

    if (ErrorCode::SUCCESS != thread_list[callee_id].SetBlocked(BlockReason::SYNC)){
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
    return running_thread;
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