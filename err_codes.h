//
// Created by heimy4prez on 5/5/18.
//

#ifndef PROJECT_ERR_CODES_H
#define PROJECT_ERR_CODES_H

#define MSG_SYSTEM_ERR "system error: "
#define MSG_LIBRARY_ERR "thread library error:"

const int RET_ERR = (-1);
const int RET_SUCCESS = 0;


typedef enum _ErrorCode {
    SUCCESS = 0,
    FAILED = -1,
    UNKNOWN = 1
} ErrorCode;

#endif //PROJECT_ERR_CODES_H
