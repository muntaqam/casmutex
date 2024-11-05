#include <pthread.h>
#include <unistd.h>
#include <stdio.h>

#include "../src/csemutex.h"

#define FLAG_SLEEPING 1
#define FLAG_RELEASED 2

csemutex_t mutex;

int flag;

/* This thread simply acquires the global mutex, sleeps for 100
 * milliseconds, and then releases the global mutex. */
void *blockingthread(void *ignored) {
    /* Lock the mutex to block main when it wakes */
    csemutex_lock(&mutex);
    /* Set the flag to indicate to main that we're going to sleep.  If
     * the mutex lock in main falls through without waiting, it will
     * probably see this value. */
    flag = FLAG_SLEEPING;
    usleep(100000);
    /* Set the flag to indicate to main that we've completed our sleep.
     * If the mutex unlock appropriately fences and releases main, the n
     * main should see this value. */
    flag = FLAG_RELEASED;
    /* Unlock the mutex */
    csemutex_unlock(&mutex);

    return NULL;
}

int main(int argc, char *argv[])
{
    pthread_t t;

    csemutex_init(&mutex);

    pthread_create(&t, NULL, blockingthread, NULL);

    /* Sleep for a few ms to ensure that the blocking thread gets a
     * chance to lock the mutex.  This sleep is what makes this test
     * inadequate to verify atomic operation of the mutex. */
    usleep(10000);

    /* Attempt to acquire the mutex.  This should block for ~90 ms, and
     * when it completes we should be able to immediately see that flag
     * has been set. */
    csemutex_lock(&mutex);

    if (flag == 0) {
        /* This probably means that, not only was there no correct mutex
         * interaction, the memory fences in the provided code were
         * messed up.  This is unlikely to happen, given how long the
         * sleeps involved here are. */
        puts("failed: flag is unmodified");
        return 1;
    } else if (flag == 1) {
        /* This means that either the mutex didn't block, or the memory
         * fence in the provided code was messed up.  Unless you removed
         * the __atomic_thread_fence() calls, it means your mutex didn't
         * perform its atomic compare-and-swap properly. */
        puts("failed: blocking thread is still asleep");
        return 1;
    } else if (flag == 2) {
        /* All is well! */
        puts("passed");
        return 0;
    } else {
        /* Whatever happened here, it wasn't good.  This could indicate
         * some sort of transfer shear, but that shouldn't happen on
         * x86-64 with integers that fit within singel byte. */
        puts("failed: flag is corrupt");
        return 1;
    }

    /* Not reached */
    return 1;
}
