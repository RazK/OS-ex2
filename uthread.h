// uthread.h
#ifndef UTHREAD_H
#define UTHREAD_H

#include <sys/types.h>
#include "err_codes.h"
#include <queue>
#include <array>
#include <set>
#include <csignal>
#include <setjmp.h>


#ifdef __x86_64__
/* code for 64 bit Intel arch */
    typedef unsigned long address_t;
    #define JB_SP 6
    #define JB_PC 7

    /* A translation is required when using an address of a variable.
       Use this as a black box in your code. */
    address_t translate_address(address_t addr);

#else
/* code for 32 bit Intel arch */
    typedef unsigned int address_t;
    #define JB_SP 4
    #define JB_PC 5

    /* A translation is required when using an address of a variable.
       Use this as a black box in your code. */
    address_t translate_address(address_t addr);
#endif

typedef u_int8_t UThreadID;

typedef enum _State {
    READY,
    RUNNING,
    BLOCKED,
    NUM_OF_STATES
} State;

typedef enum _BlockReason {
    WAITING,
    REQUEST,
    NUM_OF_REASONS
} BlockReason;

typedef enum _Status {
    ALIVE,
    TERMINATED,
    NUM_OF_STATUSES
} Status;

class UThread {

public:
    UThread();
    UThread(address_t sp, address_t pc, State state, Status status);
    ~UThread();

public:
    ErrorCode SetStatus(Status status);
    ErrorCode SetState(State state);
    ErrorCode SetBlocked(BlockReason reason);
    ErrorCode PushWaitingForMe(UThreadID utid_waiting_for_me);
    ErrorCode AddIWaitFor(UThreadID utid_im_waiting_for);
    ErrorCode RemoveIWaitFor(UThreadID utid_im_waiting_for);
    ErrorCode DismissUTIDIWaitFor(UThreadID tid);
    ErrorCode PopWaitingForMe();
    ErrorCode UnBlock(BlockReason reason); // Set the given block reason to false, if both are now false - Ready
    ErrorCode InitThread(void (*func)(void));
    ErrorCode InitThreadZero();
    ErrorCode FreeStack();
    ErrorCode IncQuantum();


    Status GetStatus() const;
    State GetState() const;
    int GetQuantumCounter() const;
    UThreadID FrontWaitingForMe() const;
    bool IsWaitingForMeEmpty() const;
    const std::array <bool, NUM_OF_REASONS> GetBlockedReasons() const;
    sigjmp_buf env_;

private:
    State state_;                                       // Scheduling State: one of [Ready, Running, Blocked]
    Status status_;                                     // Thread Status: alive or terminated.
    char* stack;
    address_t sp_;
    address_t pc_;

    int quantum_counter;                             // The number of quantumii per thread

    std::array <bool, NUM_OF_REASONS> blocked_reasons;  // Blocked because waiting for synced thread
    std::queue <UThreadID> waiting_for_me_;             // All the threads that called "sync" for this thread.
    std::set <UThreadID> im_waiting_for_;               // All the threads that this thread called "sync" for. All are unique
    // TODO: allocate memory in CTOR for this queue
    // TODO: free queue's memory in DTOR

};

#endif // UTHREAD_H