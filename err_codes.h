//
// Created by heimy4prez on 5/5/18.
//

#ifndef PROJECT_ERR_CODES_H
#define PROJECT_ERR_CODES_H

#define MSG_SYSTEM_ERR "system error: "
#define MSG_LIBRARY_ERR "thread library error: "

const int RET_ERR = (-1);
const int RET_SUCCESS = 0;

#define ERR_LIB false
#define ERR_SYS true

#define ASSERT_SUCCESS_RET(f, msg, is_system, ret)              \
do {                                                            \
    if (RET_SUCCESS != (f)) {                                   \
        if ((is_system)) {                                      \
            std::cerr << MSG_SYSTEM_ERR << (msg) << std::endl;  \
            exit(1);                                            \
        }                                                       \
        else {                                                  \
            std::cerr << MSG_LIBRARY_ERR << (msg) << std::endl; \
            return ret;                                         \
        }                                                       \
    }                                                           \
} while (0)


#define ASSERT_SUCCESS(f, msg, is_system)                       \
do {                                                            \
    if (RET_SUCCESS != (f)) {                                   \
        if ((is_system)) {                                      \
            std::cerr << MSG_SYSTEM_ERR << (msg) << std::endl;  \
            exit(1);                                            \
        }                                                       \
        else {                                                  \
            std::cerr << MSG_LIBRARY_ERR << (msg) << std::endl; \
            return RET_ERR;                                     \
        }                                                       \
    }                                                           \
} while (0)

#define ASSERT(cond, msg, is_system)                            \
do {                                                            \
    if (cond) {                                                 \
        if ((is_system)) {                                      \
            std::cerr << MSG_SYSTEM_ERR << (msg) << std::endl;  \
            exit(1);                                            \
        }                                                       \
        else {                                                  \
            std::cerr << MSG_LIBRARY_ERR << (msg) << std::endl; \
            return RET_ERR;                                     \
        }                                                       \
    }                                                           \
} while (0)


typedef enum _ErrorCode {
    SUCCESS = 0,
    FAILED = -1,
} ErrorCode;

#endif //PROJECT_ERR_CODES_H
