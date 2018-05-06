/**********************************************
 * Test 429: Compute the N'th catalan number using
 *           multiple threads.
 *           (+random calls to uthreads functions)
 *
 **********************************************/

/*
 * recall that the n'th catalan number is
 *
 *              n
 *           _______
 *           |     |    n+k
 *  C_n  =   |     |   -----
 *           |     |     k
 *             k=2
 */


#include <cstdio>
#include <cstdlib>
#include <time.h>
#include "uthreads.h"


#define GRN "\e[32m"
#define RED "\x1B[31m"
#define RESET "\x1B[0m"

#define N 15
#define CATALAN_N 9694845
#define NUM_THREADS 6

#define RUN 0
#define DONE 1

typedef unsigned long ulong;

char thread_status[NUM_THREADS];

ulong partial_calc[NUM_THREADS];


void halt()
{
    while (true)
    {}
}

int rand_tid()
{
    return rand() % NUM_THREADS;
}

// random and legal call to one or more of block/sync/resume
void random_uthreads_call()
{
    int tid = rand_tid();
    if (rand() % 2)
    {
        if (tid != 0)
        {
            uthread_block(tid);
        }
    }
    else
    {
        if (tid != uthread_get_tid())
        {
            uthread_sync(tid);
        }
    }
    uthread_resume(rand_tid());
}

ulong partial_numerator_calculator(ulong start, ulong step)
{
    int tid;
    ulong res = 1;
    for (ulong k = start; k <= N; k += step)
    {
//        random_uthreads_call();

        res *= (N + k);

//        random_uthreads_call();
    }
    return res;
}

ulong partial_denominator_calculator(ulong start, ulong step)
{
    int tid;
    ulong res = 1;
    for (ulong k = start; k <= N; k += step)
    {
//        random_uthreads_call();

        res *= k;

//        random_uthreads_call();
    }
    return res;
}

void summon_calculator(bool numerator, ulong start, ulong step)
{
    if (numerator)
    {
        partial_calc[uthread_get_tid()] = partial_numerator_calculator(start, step);
    }
    else
    {
        partial_calc[uthread_get_tid()] = partial_denominator_calculator(start, step);
    }
    thread_status[uthread_get_tid()] = DONE;
    halt();
}

void thread1()
{
    summon_calculator(true, 2, 3);
}

void thread2()
{
    summon_calculator(true, 3, 3);
}

void thread3()
{
    summon_calculator(true, 4, 3);
}

void thread4()
{
    summon_calculator(false, 2, 2);
}

void thread5()
{
    summon_calculator(false, 3, 2);
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
    printf(GRN "Test 429:  " RESET);
    fflush(stdout);

    srand(time(NULL));

    // threads 1,2,3 - numerator
    // threads 4,5 - denominator

    uthread_init(20);

    uthread_spawn(thread1);
    uthread_spawn(thread2);
    uthread_spawn(thread3);
    uthread_spawn(thread4);
    uthread_spawn(thread5);

    for (int i = 1; i < NUM_THREADS; i++)
    {
        thread_status[i] = RUN;
    }

    int tid = 0;
    while (!all_done())
    {
        // sequentially resume all threads, as they are blocking each other like madmen
        uthread_resume(tid);
        tid = (tid + 1) % NUM_THREADS;
    }

    ulong res = (partial_calc[1] * partial_calc[2] * partial_calc[3]) /
                (partial_calc[4] * partial_calc[5]);

    if (res == CATALAN_N)
    {
        printf(GRN "SUCCESS\n" RESET);
    }
    else
    {
        printf(RED "ERROR - failed to correctly calculate catalan number\n" RESET);
    }
    uthread_terminate(0);

}
