cmake_minimum_required(VERSION 3.8)
project(OS_ex2)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES
        demo_itimer.c
        demo_jmp.c
        demo_singInt_handler.c
        main.cpp
        mask.cpp
        mask.h
        uthread.cpp
        uthread.h
        uthreads.cpp
        uthreads.h err_codes.h)

add_executable(OS_ex2 ${SOURCE_FILES} err_codes.h)