#include <pthread.h>
#include <sched.h>
#include <stdio.h>

#include "../src/csemutex.h"

#define ITERATIONS 100000
#define THREADS 5

csemutex_t mutex;
int count;

/* This thread iterates ITERATIONS times, locking the mutex and
 * incrementing count on each iteration.  If the mutex is working
 * correctly, this thread should contribute exactly ITERATIONS effective
 * increments to count. */
void *counter(void *ignored) {
    /* Yield at startup to encourage everything to start making progress. */
    sched_yield();

    for (int i = 0; i < ITERATIONS; i++) {
        csemutex_lock(&mutex);
        count++;
        csemutex_unlock(&mutex);
    }

    return NULL;
}

/* Create THREADS counter threads, set them free on the mutex, and then
 * wait for them to finish.  Check that there were no lost counts. */
int main(int argc, char *argv[]) {
    pthread_t t[THREADS];

    csemutex_init(&mutex);

    for (int i = 0; i < THREADS; i++) {
        pthread_create(&t[i], NULL, counter, NULL);
    }

    /* At this point the threads are all counting, at least some of them
     * probably in parallel.  Wait for them to exit. */

    for (int i = 0; i < THREADS; i++) {
        pthread_join(t[i], NULL);
    }

    /* At this point the mutex should be available, lock it. */
    csemutex_lock(&mutex);

    if (count != ITERATIONS * THREADS) {
        printf("failed: count was %d instead of %d\n", count, ITERATIONS * THREADS);
        return 1;
    }

    puts("passed");

    csemutex_unlock(&mutex);

    return 0;
}
