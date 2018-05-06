/**********************************************
 * Test 14: blocked and synced thread is resumed only
 *         after its 'syncer' ran AND it was resumed
 *
 **********************************************/

#include <cstdio>
#include <cstdlib>
#include "uthreads.h"


#define GRN "\e[32m"
#define RED "\x1B[31m"
#define RESET "\x1B[0m"


bool thread_1_blocked = false;
bool thread_2_after_sync = false;

void halt()
{
    while (true)
    {}
}

/*
 * quantums - the number of quantums this thread should skip
 */
void wait_quantums(unsigned int quantums_to_wait)
{
    int quantum = uthread_get_quantums(uthread_get_tid());
    while (uthread_get_quantums(uthread_get_tid()) < quantum + quantums_to_wait)
    {}
    return;
}


void thread1()
{
    while (!thread_1_blocked)
    {}

    // here, thread 1 is resumed. thread 2 should still be in sync/
    if (thread_2_after_sync)
    {
        printf(RED "ERROR - thread resumed before waiting to sync\n" RESET);
        exit(1);
    }

    halt();
}

void thread2()
{
    uthread_sync(1);
    thread_2_after_sync = true;
    halt();
}


void thread3()
{
    halt();
}

void thread4()
{
    uthread_sync(3);

    // here, thread4 is back from sync before it is resumed - error
    printf(RED "ERROR - thread is back from sync before resumed\n" RESET);
    exit(1);
}


int main()
{
    printf(GRN "Test 14:   " RESET);
    fflush(stdout);

    uthread_init(50);

    /*
     * we test two cases
     *
     * case 1:
     * sync
     * block
     * block-release
     * sync release
     *
     * case 2:
     * sync
     * block
     * sync release
     * block-release
     *
     */

    // case 1:
    int t1 = uthread_spawn(thread1);
    if (t1 != 1)
    {
        printf(RED "ERROR - wrong id for new thread\n" RESET);
        exit(1);
    }

    // t2 will soon call sync(1), so we block t1 to make sure this sync
    // call will not return
    if (uthread_block(t1) == -1)
    {
        printf(RED "ERROR - uthread_block failed\n" RESET);
        exit(1);
    }
    thread_1_blocked = true;

    int t2 = uthread_spawn(thread2);
    if (t2 != 2)
    {
        printf(RED "ERROR - wrong id for new thread\n" RESET);
        exit(1);
    }

    wait_quantums(2); // let t2 call sync(1)

    if (uthread_block(t2) == -1)
    {
        printf(RED "ERROR - uthread_block failed\n" RESET);
        exit(1);
    }

    // now t2 should be synced AND blocked

    uthread_resume(t2);


    wait_quantums(2); // give t2 a chance to get back, which is an error

    uthread_resume(t1); // t1 will make sure that t2 didn't come back from sync.


    // case 2:
    // thread 4 call sync(3)
    // thread 4 is blocked
    // sync is released

    int t3 = uthread_spawn(thread3);
    if (t3 != 3)
    {
        printf(RED "ERROR - wrong id for new thread\n" RESET);
        exit(1);
    }
    uthread_block(t3);

    int t4 = uthread_spawn(thread4);
    wait_quantums(2); // let t4 call sync(3)
    uthread_block(t4);

    uthread_resume(t3);
    wait_quantums(2); // let thread4 a chance to get back, which is an error


    printf(GRN "SUCCESS\n" RESET);
    uthread_terminate(0);

}
