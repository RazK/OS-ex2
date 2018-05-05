// uthread.cpp
// Created by razkarl on 5/5/18.
//
#include "uthread.h"
#include "uthreads.h"
#include "err_codes.h"
#include <iostream>

#ifdef __x86_64__
/* code for 64 bit Intel arch */

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

UThread::UThread() {
    this->stack = nullptr;
    this->sp_ = (address_t)NULL;
    this->pc_ = (address_t)NULL;
    state_ = State::READY;
    status_ = Status::TERMINATED;
}

UThread::~UThread() {
    if (this->stack){
        free(this->stack);
    }
}

ErrorCode UThread::SetStatus(const Status status){
    this->status_ = status;
    // TODO: always just set and return?
    return SUCCESS;
}

ErrorCode UThread::SetState(const State state){
    if (state == State::BLOCKED){
        // BAD! Use SetBlocked with reason instead!
        return FAILED;
    }

    this->state_ = state;
    // TODO: always just set and return?
    return SUCCESS;
}

ErrorCode UThread::SetBlocked(BlockReason reason){
    this->state_ = State::BLOCKED;
    this->blocked_reasons[reason] = true;
    // TODO: always just set and return?
    return SUCCESS;
}

ErrorCode UThread::PushSynced(UThreadID utid_synced_with_me){
    this->synced_with_me_.push(utid_synced_with_me);
    // TODO: always just set and return?
    return SUCCESS;
}

ErrorCode UThread::PopSynced(){
    this->synced_with_me_.pop();
    // TODO: always just set and return?
    return SUCCESS;
}

ErrorCode UThread::UnBlock(BlockReason reason){
    // Validate i'm currently blocked
    if (State::BLOCKED != this->GetState()) {
        std::cerr << MSG_LIBRARY_ERR << "Can't unblock a thread in state " << this->GetState() << std::endl;
        return FAILED;
    }

    // Set the given reason to no longer blocking
    this->blocked_reasons[reason] = false;

    // Check if still blocked by other reasons
    bool still_blocked = false;
    for(const auto& cur_reason: blocked_reasons){
        if (cur_reason){
            still_blocked = true;
            break;
        }
    }

    // Not blocked anymore? --> READY
    if (!still_blocked){
        this->SetState(State::READY);
    }

    return SUCCESS;

}; // Set the given block reason to false, if both are now false - Ready

ErrorCode UThread::InitEnv(void* func){
    this->stack = (char*) malloc(STACK_SIZE);
    if (stack == nullptr){
        std::cerr << MSG_SYSTEM_ERR << "Could not allocate memory in environment init";
        exit(1);
    }
    this->sp_ = (address_t)stack + STACK_SIZE - sizeof(address_t);
    this->pc_ = (address_t)func;
//    sigsetjmp(this->env_, 1);
    (this->env_->__jmpbuf)[JB_SP] = translate_address(this->sp_);
    (this->env_->__jmpbuf)[JB_PC] = translate_address(this->pc_);
    return ErrorCode::SUCCESS;
}

sigjmp_buf& UThread::GetEnv(){
    return this->env_;
}

Status UThread::GetStatus() const{
    return this->status_;
}

State UThread::GetState() const{
    return this->state_;
}

UThreadID UThread::FrontSynced() const{
    return this->synced_with_me_.front();
}

bool UThread::IsSyncedEmpty() const{
    return this->synced_with_me_.empty();
}

const std::array <bool, NUM_OF_REASONS> UThread::GetBlockedReasons() const{
    return this->blocked_reasons;
};
