#include <sched.h>

#include "csemutex.h"

/* This wrapper for __atomic_compare_exchange_n() provides an atomic
 * compare-and-swap of the location addr.  If the integer stored at addr
 * is equal to the integer given in expected, then it will be replaced
 * with the integer given in desired, and this function returns true.
 * If it is NOT equal to the integer given in expected, then the
 * function returns false and no exchange is made.
 *
 * Arguments:
 * addr:     location of the integer to compare-and-swap
 * expected: value stored at attr for a swap to occur
 * desired:  value stored at attr after a successful swap
 *
 * Returns:
 * 0 if no swap could be made
 * 1 if the swap was successful
 */
int cse_cas(int *addr, int expected, int desired) {
    return __atomic_compare_exchange_n(addr, &expected, desired, 0,
                                       __ATOMIC_ACQ_REL, __ATOMIC_RELAXED);
}

void csemutex_init(csemutex_t *mutex) {
    /* Store the value 0 to the given mutex, and ensure that any
     * following atomic reads of this mutex happen after this operation
     * becomes visible. */
    __atomic_store_n(mutex, 0, __ATOMIC_RELEASE);
}

/* The given version of this function DOES NOT WORK! */
void csemutex_lock(csemutex_t *mutex) {
    *mutex = 1;

    /* The following provides the memory barrier that ensures that all
     * shared locations OTHER than the mutex itself are visible to this
     * thread.  You should not remove or reorder this line, it must be
     * the last statement in this function! */
    __atomic_thread_fence(__ATOMIC_ACQUIRE);
}

/* The given version of this function will work in most circumstances,
 * but you should replace the simple assignment with an atomic
 * compare-and-swap operation. */
void csemutex_unlock(csemutex_t *mutex) {
    *mutex = 0;

    /* The following provides the memory barrier that ensures that all
     * changes made by this thread while the lock was held are visible
     * to other threads before they complete acquisition of this lock.
     * You should not remove or reorder this line, it must be the last
     * statement in this function! */
    __atomic_thread_fence(__ATOMIC_RELEASE);
}
