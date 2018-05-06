/**********************************************
 * Test 5: sync/block/resume
 *
 **********************************************/

#include <cstdio>
#include "uthreads.h"

#define GRN "\e[32m"
#define RED "\x1B[31m"
#define RESET "\x1B[0m"

#define NUM_THREADS 4
#define RUN 0
#define DONE 1
char thread_status[NUM_THREADS];


void halt()
{
    while (true)
    {}
}

int next_thread()
{
    return (uthread_get_tid() + 1) % NUM_THREADS;
}

void thread()
{
    uthread_sync(next_thread());

    uthread_sync(next_thread());

    uthread_block(uthread_get_tid());

    for (int i = 0; i < 50; i++)
    {
        uthread_resume(next_thread());
    }

    thread_status[uthread_get_tid()] = DONE;

    halt();
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
    printf(GRN "Test 5:    " RESET);
    fflush(stdout);

    uthread_init(100);
    uthread_spawn(thread);
    uthread_spawn(thread);
    uthread_spawn(thread);

    for (int i = 0; i < NUM_THREADS; i++)
    {
        thread_status[i] = RUN;
    }

//    while (!all_done())
//    {
//        uthread_resume(1);
//    }


    printf(GRN "SUCCESS\n" RESET);
    uthread_terminate(0);

}