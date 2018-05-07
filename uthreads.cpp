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
#include <sys/time.h>


const int ID_SCHEDUELER = 0;
const int ID_FIRST_USER_THREAD = 0;
const int SIG_RET_FROM_JUMP = 1;
const int MIC_SEC_IN_SEC = 1000000;

UThreadID running_thread;
struct sigaction sa;
struct itimerval timer;
int quantum;

std::queue <UThreadID> ready_queue;

/* Global counter for all quantums. */
int total_quantums;

/* Threads descriptor lookup table. */
UThread thread_list[MAX_THREAD_NUM]; // #todo Should I send MAX_THREAD_NUM-1 (for main thread?)


void switch_threads(bool isFromBlocked){
    Mask m{};
    // Running thread goes to rest
    UThreadID old_tid = running_thread;

    // if not blocked then just go back to the ready line
    if (!isFromBlocked) {
        ready_queue.push((UThreadID) old_tid);
        thread_list[old_tid].SetState(State::READY);
    }

    // Find first ready thread which is neither BLOCKED nor TERMINATED
    UThreadID new_tid = MAX_THREAD_NUM;
    while (!ready_queue.empty()) {
        new_tid = ready_queue.front();

        // Clean threads_list from non-ready threads as you go
        ready_queue.pop();

        // Found ready thread? break
        if (State::READY == thread_list[new_tid].GetState() &&
            Status::ALIVE == thread_list[new_tid].GetStatus()){
            break;
        }
    }

    // Validate ready_queue is not empty (FATAL ERROR)
    if (State::READY != thread_list[new_tid].GetState()&&
        Status::ALIVE == thread_list[new_tid].GetStatus()) {
        std::cerr << MSG_LIBRARY_ERR << "The queue of ready threads was found empty. Assumed not to occur." << std::endl;
        return;
    }

    // Save env for current thread and then jump to next one
    int ret_val = sigsetjmp(thread_list[old_tid].env_, 1);
    if (ret_val == SIG_RET_FROM_JUMP){
        thread_list[old_tid].SetState(State::RUNNING);
        running_thread = old_tid;
        return;
    }
    thread_list[new_tid].SetState(State::RUNNING);
    running_thread = new_tid;
    siglongjmp(thread_list[new_tid].env_, SIG_RET_FROM_JUMP); // TODO: Assert return value!!!
}

//std::list <std::queue>
void sig_alarm_handler(int sig){
    Mask m{};
    printf("sigalarm_handler here!!\r\n");
    switch_threads(false);

//    siglongjmp(thread_list[currentThread].GetEnv());

//    return RET_SUCCESS;
}

int uthread_init(int quantum_usecs){
    Mask m{}; // masking object

    // Validate params
    ASSERT(quantum_usecs <= 0, "Non-positive number of quantums given", ERR_LIB);

    // Init timer
    ASSERT_SUCCESS(sigemptyset(&sa.sa_mask), "sigemptyset failed in init.", ERR_SYS);

    // Install timer_handler as the signal handler for SIGVTALRM.
    sa.sa_handler = &sig_alarm_handler;
    if (sigaction(SIGVTALRM, &sa, nullptr) < 0) {
        printf("sigaction error.");
    }

    // Configure the timer to expire after quantum_usecs */
    timer.it_value.tv_sec = quantum_usecs / MIC_SEC_IN_SEC ;		// first time interval, seconds part
    timer.it_value.tv_usec = quantum_usecs % MIC_SEC_IN_SEC;		// first time interval, microseconds part

    timer.it_interval.tv_sec = quantum_usecs / MIC_SEC_IN_SEC ;	    // following time intervals, seconds part
    timer.it_interval.tv_usec = quantum_usecs % MIC_SEC_IN_SEC ;	// following time intervals, microseconds part

    // Start a virtual timer. It counts down whenever this process is executing.
    ASSERT_SUCCESS(setitimer (ITIMER_VIRTUAL, &timer, nullptr), "setitimer error.", ERR_SYS);

    thread_list[0].InitThreadZero();

    // Set global variables
    total_quantums = 1;
    running_thread = 0;

    return RET_SUCCESS;
}

int uthread_spawn(void (*f)()){
    Mask m{}; // masking object
    int tid;

    // Find the minimal ID not yet taken.
    for (tid = 0; tid < MAX_THREAD_NUM; tid++){
        if (thread_list[tid].GetStatus() == Status::TERMINATED){ // Can be shortened to if(..status)
            break;
        }
    }

    if (tid == MAX_THREAD_NUM){
        return RET_ERR; // No more room for threads
    }

    thread_list[tid].InitThread(f);
    ready_queue.push((UThreadID)tid); // cast for type protection
    return tid;
}


int uthread_terminate(int tid){
    Mask m{}; // masking object
    if (tid < 0 || tid > MAX_THREAD_NUM){
        std::cerr << MSG_LIBRARY_ERR << "Attempting to terminate illegal thread id: ID " << tid << std::endl;
        return RET_ERR;
    }

    if (tid == 0){


        //Todo: Do i need to free aliicated memory for threads? REMEMBER: thier destructor is
        // implicitly called, which takes care of freeing memory if exists.

//        delete (m);
        exit(RET_SUCCESS);
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

    // Iterate the queue of synced threads to allow them to run. Multiple checks need to take place.
    while (!thread_list[tid].IsWaitingForMeEmpty()){
        UThreadID synced_thread = thread_list[tid].FrontWaitingForMe();
        // Check if this thread is still waiting for the dying thread:

        if (ErrorCode::SUCCESS != thread_list[synced_thread].DismissUTIDIWaitFor(tid)){
            return RET_ERR;
        }

        // No need to check if synced thread is alive. This is responsibility of scheduler. We do make sure that this
        // thread needs to be added to the end of the ready queue to avoid working twice.
        if (State::READY == thread_list[synced_thread].GetState()){
            ready_queue.push(synced_thread);
        }
//        if (ErrorCode::SUCCESS != thread_list[tid].SetState(State::READY)){
//            return RET_ERR;
//        }

        // Whatever the results were of the last steps, we now pop the last dependant thread and press on.
        if (ErrorCode::SUCCESS != thread_list[tid].PopWaitingForMe()){
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
        switch_threads(true);
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
        std::cerr << MSG_LIBRARY_ERR << "Attempting to block missing thread with id: ID " << tid << std::endl;
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


int uthread_sync(int wait_for_me_tid){
    Mask m{}; // masking object
    if (wait_for_me_tid < 0 || wait_for_me_tid > MAX_THREAD_NUM || Status::TERMINATED == thread_list[wait_for_me_tid].GetStatus()){
        std::cerr << MSG_LIBRARY_ERR << "Attempting to block illegal thread id: ID " << wait_for_me_tid << std::endl;
        return RET_ERR;
    }

    UThreadID i_wait_tid = (UThreadID)uthread_get_tid();

    if (ErrorCode::SUCCESS != thread_list[i_wait_tid].SetBlocked(BlockReason::WAITING)){
        return RET_ERR;
    }

    if (ErrorCode::SUCCESS != thread_list[wait_for_me_tid].PushWaitingForMe((UThreadID) i_wait_tid)){
        return RET_ERR;
    }

    if (ErrorCode::SUCCESS != thread_list[i_wait_tid].AddIWaitFor((UThreadID) wait_for_me_tid)){
        return RET_ERR;
    }

    switch_threads(true);

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
    Mask m{}; // masking object
    if (tid < 0 || tid > MAX_THREAD_NUM || Status::TERMINATED == thread_list[tid].GetStatus()){
        std::cerr << MSG_LIBRARY_ERR << "Attempting to block illegal thread id: ID " << tid << std::endl;
        return RET_ERR;
    }
    return thread_list[tid].GetQuantumCounter();
}

