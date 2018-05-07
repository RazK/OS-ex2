/**********************************************
 * Test 2: very simple sync check
 *
 **********************************************/

#include <cstdio>
#include <cstdlib>
#include "uthreads.h"
#include <iostream>

#define GRN "\e[32m"
#define RED "\x1B[31m"
#define RESET "\x1B[0m"

bool thread2Spawned = false;
bool thread2Executed = false;
int count = 0;


void halt()
{
    while (true)
    {
//        std::cout << count << "halting! (quantum should kill me any second now.. or not)?" << std::endl;
//        count += 1;
    }
}

void thread1()
{
    while (!thread2Spawned)
    {}
    std::cout << "thread 1 recognized 2 spawned and now waits for it to sync" << std::endl;
    uthread_sync(2);
    if (!thread2Executed)
    {
        printf(RED "ERROR - thread did not wait to sync\n" RESET);
        exit(1);
    }
    printf(GRN "SUCCESS\n" RESET);
    uthread_terminate(0);
}

void thread2()
{
    thread2Executed = true;
    halt();
}

int main()
{
    printf(GRN "Test 2:    " RESET);
    fflush(stdout);

    uthread_init(40);
    uthread_spawn(thread1);
    uthread_spawn(thread2);
    thread2Spawned = true;
    for(int i = 0; i < 10000000; i++){
//        printf("in loop of main\r\n");
    }
    printf("now terminating 2\r\n");
    uthread_terminate(2);
    halt();
}
