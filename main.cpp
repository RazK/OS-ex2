/**********************************************
 * Test 1430: sort array using multiple threads
 *
 **********************************************/



/*
 *          main              (merge N-size array)
 *         /    \
 *        /      \
 *       /        \
 *      t1        t2          (merge N/2-size array)
 *     / \        / \
 *    /   \      /   \
 *  t3    t4    t5    t6      (sort N/4-size array)
 *
 * DISCLAIMER:
 * This sorting application is very limited for this specific test,
 * and is not recommended to be used for real sorting purposes
 *
 */




#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <time.h>
#include "uthreads.h"

#define GRN "\e[32m"
#define RED "\x1B[31m"
#define RESET "\x1B[0m"

#define NUM_THREADS 7
#define ARRAY_SIZE 8192
#define N ARRAY_SIZE
#define RUN 0
#define DONE 1

char thread_status[NUM_THREADS];
int array[ARRAY_SIZE];

int place_holder0[N], place_holder1[N / 2], place_holder2[N / 2];


void printArray();


int rand_tid()
{
    return rand() % NUM_THREADS;
}

void wait_next_quantum()
{
    int quantum = uthread_get_quantums(uthread_get_tid());
    while (uthread_get_quantums(uthread_get_tid()) == quantum)
    {}
    return;
}

// sort array[start..end-1]
void sort(int start, int end)
{
    wait_next_quantum();

    for (int i = start; i < end; i++)
    {
        for (int j = i + 1; j < end; j++)
        {
            if (array[i] > array[j])
            {
                int temp = array[i];
                array[i] = array[j];
                array[j] = temp;
            }
        }
        if (i == (end + start) / 2)
        {
            wait_next_quantum();
        }
    }

}


/*
 * merges array[start..end/2-1] and array[end/2..end-1]
 */
void merge(int start, int end, int* place_holder)
{

    int start1 = start, end1 = (end + start) / 2;
    int start2 = end1, end2 = end;
    int i = start1, j = start2;
    int k = 0;

    while (i < end1 && j < end2)
    {
        if (array[i] < array[j])
        {
            place_holder[k++] = array[i++];
        }
        else
        {
            place_holder[k++] = array[j++];
        }
    }

    wait_next_quantum();

    while (i < end1)
    {
        place_holder[k++] = array[i++];
    }
    while (j < end2)
    {
        place_holder[k++] = array[j++];
    }

    wait_next_quantum();

    // copy from place_holder to original array
    memcpy(array + start1, place_holder, sizeof(int) * (end2 - start1));

}

void sorting_thread(int start, int end)
{
    sort(start, end);
    thread_status[uthread_get_tid()] = DONE;
    uthread_terminate(uthread_get_tid());
}

void thread3()
{
    sorting_thread(0, N / 4);
}

void thread4()
{
    sorting_thread(N / 4, N / 2);
}

void thread5()
{
    sorting_thread(N / 2, 3 * N / 4);
}

void thread6()
{
    sorting_thread(3 * N / 4, N);
}


void thread1()
{
    int t3 = uthread_spawn(thread3);
    int t4 = uthread_spawn(thread4);

    if (t3 == -1 || t4 == -1)
    {
        printf(RED "ERROR - thread spawn failed\n" RESET);
        uthread_terminate(0);
    }

    while (thread_status[t3] == RUN || thread_status[t4] == RUN)
    {}

    merge(0, N / 2, place_holder1);
    thread_status[uthread_get_tid()] = DONE;
    uthread_terminate(uthread_get_tid());
}

void thread2()
{
    int t5 = uthread_spawn(thread5);
    int t6 = uthread_spawn(thread6);

    if (t5 == -1 || t6 == -1)
    {
        printf(RED "ERROR - thread spawn failed\n" RESET);
        uthread_terminate(0);
    }

    while (thread_status[t5] == RUN || thread_status[t6] == RUN)
    {}

    merge(N / 2, N, place_holder2);
    thread_status[uthread_get_tid()] = DONE;
    uthread_terminate(uthread_get_tid());
}

void printArray()
{
    for (int i = 0; i < N; i++)
    {
        printf("%d, ", array[i]);
    }
    printf("\n");
}


int main()
{
    printf(GRN "Test 1430: " RESET);
    fflush(stdout);

    srand(time(NULL));

    for (int i = 0; i < ARRAY_SIZE; i++)
    {
        array[i] = rand() % 4862;
    }


    for (int i = 1; i < NUM_THREADS; i++)
    {
        thread_status[i] = RUN;
    }

    uthread_init(50);

    int t1 = uthread_spawn(thread1);
    int t2 = uthread_spawn(thread2);
    if (t1 == -1 || t2 == -1)
    {
        printf(RED "ERROR - thread spawn failed\n" RESET);
        uthread_terminate(0);
    }

    while (thread_status[t1] == RUN || thread_status[t2] == RUN)
    {}

    merge(0, N, place_holder0);

    // check that array is sorted
    for (int i = 0; i < N - 1; i++)
    {
        if (array[i] > array[i + 1])
        {
            printf(RED "ERROR - failed to sort the array\n" RESET);
            uthread_terminate(0);
        }
    }

    printf(GRN "SUCCESS\n" RESET);
    uthread_terminate(0);

}
