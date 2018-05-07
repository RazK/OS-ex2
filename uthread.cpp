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
    this->state_ = State::READY;
    this->status_ = Status::TERMINATED;
    this->blocked_reasons = {false, false};
    this->im_waiting_for_ = std::set<UThreadID>{};
    this->waiting_for_me_ = std::queue<UThreadID>{};
    this->quantum_counter = 0;
}

UThread::~UThread() {
    FreeStack(); // not validating return value because all are legit
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

ErrorCode UThread::PushWaitingForMe(UThreadID utid_waiting_for_me){
    this->waiting_for_me_.push(utid_waiting_for_me);
    // TODO: always just set and return?
    return SUCCESS;
}

ErrorCode UThread::AddIWaitFor(UThreadID utid_im_waiting_for){
    this->im_waiting_for_.emplace(utid_im_waiting_for);
    // TODO: always just set and return?
    return SUCCESS;
}

ErrorCode UThread::RemoveIWaitFor(UThreadID utid_im_waiting_for){
    auto it = this->im_waiting_for_.find(utid_im_waiting_for);
    auto res = this->im_waiting_for_.erase(it);

    // TODO: always just set and return?
    return SUCCESS;
}


ErrorCode UThread::PopWaitingForMe(){
    this->waiting_for_me_.pop();
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

ErrorCode UThread::InitThread(void (*func)(void)){
    // Set thread's state and status and refresh dependancies as defined by initial state.
    // Note, this may be a multiple live cylcle of thread.
    this->state_ = State::READY;
    this->status_ = ALIVE;
    this->im_waiting_for_ = {};
//    this->im_waiting_for_.clear();
    this->waiting_for_me_ = std::queue<UThreadID> {};
    this->quantum_counter = 0;

    // init env
    this->stack = (char*) malloc(STACK_SIZE);
    if (stack == nullptr){
        std::cerr << MSG_SYSTEM_ERR << "Could not allocate memory in environment init";
        exit(1);
    }
    this->sp_ = (address_t)stack + STACK_SIZE - sizeof(address_t);
    this->pc_ = (address_t)func;
    sigsetjmp(this->env_, 1);

    (this->env_->__jmpbuf)[JB_SP] = translate_address(this->sp_);
    (this->env_->__jmpbuf)[JB_PC] = translate_address(this->pc_);
    ASSERT_SUCCESS_RET(sigemptyset(&env_->__saved_mask), "sigemptyset failed", ERR_SYS, ErrorCode::FAILED);

    return ErrorCode::SUCCESS;
}

void empty(){
    while(true){
        std::cout << "in tid: 0 infitineloopfunc" << std::endl;
    }
}

ErrorCode UThread::InitThreadZero(){
    // Set thread's state and status and refresh dependancies as defined by initial state.
    // Note, this may be a multiple live cylcle of thread.
    this->state_ = State::RUNNING;
    this->status_ = ALIVE;
    this->im_waiting_for_ = {};
    this->waiting_for_me_ = std::queue<UThreadID> {};
    this->quantum_counter = 0;

    // init env
    this->stack = (char*) malloc(STACK_SIZE);
    if (stack == nullptr){
        std::cerr << MSG_SYSTEM_ERR << "Could not allocate memory in environment init";
        exit(1);
    }
    this->sp_ = (address_t)stack + STACK_SIZE - sizeof(address_t);
    this->pc_ = (address_t)empty;
//    sigsetjmp(this->env_, 1);

    (this->env_->__jmpbuf)[JB_SP] = translate_address(this->sp_);
    (this->env_->__jmpbuf)[JB_PC] = translate_address(this->pc_);
    ASSERT_SUCCESS_RET(sigemptyset(&env_->__saved_mask), "sigemptyset failed", ERR_SYS, ErrorCode::FAILED);

    return ErrorCode::SUCCESS;
}

ErrorCode UThread::DismissUTIDIWaitFor(UThreadID tid){
    auto it = this->im_waiting_for_.find(tid);
    auto res = this->im_waiting_for_.erase(it);
    // Check blocking logic and restore state to ready if all reasons are void
    if (this->im_waiting_for_.empty()){ // no longer synced
        ASSERT_SUCCESS_RET(this->UnBlock(BlockReason::WAITING), "Waiting thread was not blocked", ERR_LIB, ErrorCode::FAILED);
//        this->blocked_reasons[BlockReason::WAITING] = false;
//        if (!blocked_reasons[BlockReason::REQUEST]){ // check if still have blocked request
//            this->state_ = READY;
//        }
    }

    // TODO: always just set and return?
    return SUCCESS;
}

ErrorCode UThread::FreeStack(){
    if (this->stack != nullptr){
        free(this->stack);
        this->stack = nullptr;
        this->sp_ = 0;
        this->pc_ = 0;
        return ErrorCode::SUCCESS;
    }
    return ErrorCode::FAILED;
}

Status UThread::GetStatus() const{
    return this->status_;
}

State UThread::GetState() const{
    return this->state_;
}

UThreadID UThread::FrontWaitingForMe() const{
    return this->waiting_for_me_.front();
}

bool UThread::IsWaitingForMeEmpty() const{
    return this->waiting_for_me_.empty();
}

const std::array <bool, NUM_OF_REASONS> UThread::GetBlockedReasons() const{
    return this->blocked_reasons;
};

ErrorCode UThread::IncQuantum(){
    quantum_counter += 1;
    if (quantum_counter <= 0){
        return ErrorCode::FAILED;
    }
    return ErrorCode::SUCCESS;
}

int UThread::GetQuantumCounter() const{
    return this->quantum_counter;
}
