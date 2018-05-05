// uthread.h
#ifndef UTHREAD_H
#define UTHREAD_H

#include <sys/types.h>
#include <queue>
#include <csignal>

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

typedef u_int8_t UThreadID;

typedef enum _ErrorCode {
    SUCCESS = 0,
    FAILED = -1,
    UNKNOWN = 1
} ErrorCode;

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

class UThread {

public:
    UThread();
    UThread(address_t sp, address_t pc, State state, Status status);
    ~UThread();

public:
    ErrorCode SetStatus(Status status);
    ErrorCode SetState(State state);
    ErrorCode PushSynced(UThreadID utid_synced_with_me) const;
    ErrorCode PopSynced() const;

    Status GetStatus() const;
    State GetState() const;
    UThreadID FrontSynced() const;
    bool IsSyncedEmpty() const;
    //ErrorCode AddImSyncedWith(UThreadID utid_im_synced_with);

private:
    //address_t sp;         // Stack Pointer: Address of thread's stack head
    //address_t pc;         // Program Counter: Address of thread's current instruction
    State state_;           // Scheduling State: one of [Ready, Running, Blocked]
    Status status_;         // Thread Status: alive or terminated.
    bool blocked_sync;      // Blocked because waiting for synced thread
    bool blocked_request;   // Blocked because other thread requested
    std::queue <UThreadID> synced_with_me_; // All the threads that called "sync" for this thread.
    //std::queue <UThreadID> im_synced_with_; // All the threads that called "sync" for this thread.
    // TODO: allocate memory in CTOR for this queue
    // TODO: free queue's memory in DTOR

};

#endif // UTHREAD_H