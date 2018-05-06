/**********************************************
 * Test 42: each thread has its own unique stack
 *
 **********************************************/

#include <cstdio>
#include <cstdlib>
#include "uthreads.h"

#define GRN "\e[32m"
#define RED "\x1B[31m"
#define RESET "\x1B[0m"

#define NUM_THREADS 7
#define RUN 0
#define DONE 1

char thread_status[NUM_THREADS];

int next_thread()
{
    return (uthread_get_tid() + 1) % NUM_THREADS;
}

void wait_next_quantum()
{
    int quantum = uthread_get_quantums(uthread_get_tid());
    while (uthread_get_quantums(uthread_get_tid()) == quantum)
    {}
    return;
}

void run_test()
{
    int tid = uthread_get_tid();
    int arr[10];

    for (int i = 0; i < 10; i++)
    {
        arr[i] = i * tid;
    }

    if (tid !=6)
    {
        uthread_sync(next_thread());
    }

    for (int i = 0; i < 10; i++)
    {
        if (arr[i] != i * tid)
        {
            printf(RED "ERROR - stack values changed\n" RESET);
            exit(1);
        }
    }

    int b1 = tid * 314, b2 = tid * 141;

    // let switching be invoked by the timer
    wait_next_quantum();


    if ((b1 != tid * 314) || (b2 != tid * 141) || (tid != uthread_get_tid()))
    {
        printf(RED "ERROR - stack values changed\n" RESET);
        exit(1);
    }

    thread_status[uthread_get_tid()] = DONE;
    uthread_terminate(uthread_get_tid());
}

bool all_done()
{
    bool res = true;
    for (int i = 1; i < NUM_THREADS; i++)
    {
        res = res && (thread_status[i] == DONE);
    }
    return res;
}


int main()
{
    printf(GRN "Test 42:   " RESET);
    fflush(stdout);

    uthread_init(10);
    uthread_spawn(run_test);
    uthread_spawn(run_test);
    uthread_spawn(run_test);
    uthread_spawn(run_test);
    uthread_spawn(run_test);
    uthread_spawn(run_test);

    for (int i = 1; i < NUM_THREADS; i++)
    {
        thread_status[i] = RUN;
    }


    while (!all_done())
    {}

    printf(GRN "SUCCESS\n" RESET);
    uthread_terminate(0);

}
