// uthread.cpp
// Created by razkarl on 5/5/18.
//
#include "uthread.h"

UThread::UThread() {
    //sp = NULL;
    //pc = NULL;
    state_ = State::READY;
    status_ = Status::TERMINATED;
    // TODO: allocate synced_threads queue
}

UThread::UThread(address_t sp, address_t pc, State state, Status status) {
    //this->sp = sp;
    //this->pc = pc;
    this->state_ = state;
    this->status_ = status;
    // TODO: free synced_threads queue
}

UThread::~UThread() {

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

std::array <bool, NUM_OF_REASONS> UThread::GetBlockedReasons() const{
    return this->blocked_reasons;
};

