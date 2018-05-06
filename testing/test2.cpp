/**********************************************
 * Test 2: very simple sync check
 *
 **********************************************/

#include <cstdio>
#include <cstdlib>
#include "uthreads.h"

#define GRN "\e[32m"
#define RED "\x1B[31m"
#define RESET "\x1B[0m"

bool thread2Spawned = false;
bool thread2Executed = false;

void halt()
{
    while (true)
    {}
}

void thread1()
{
    while (!thread2Spawned)
    {}

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

    uthread_init(20);
    uthread_spawn(thread1);
    uthread_spawn(thread2);
    thread2Spawned = true;
    for(int i = 0; i < 100000000; i++){
    }
    uthread_terminate(2);
    halt();
}
