#ifndef CSEMUTEX_H_
#define CSEMUTEX_H_

/* This is the type of a csemutex_t, which is a simple integer.
 * All special properties of the integer come from the way it is
 * manipulated.
 *
 * A value of 1 indicates a locked mutex, while a value of 0 indicates
 * an unlocked mutex.
 */
typedef int csemutex_t;

/* This function initializes a csemutex_t for use.  It simply sets the
 * mutex to zero and forces a memory barrier using an atomic
 * operation. */
void csemutex_init(csemutex_t *mutex);

/* This function should lock a csemutex_t that has been previously
 * initialized using csemutex_init().  It is inefficient in that it
 * spins on the lock, but it performs a slow spin using sched_yield().
 *
 * This function blocks until the associated csemutex_t can be locked by
 * this thread using an atomic compare-and-swap operation.
 */
void csemutex_lock(csemutex_t *mutex);

/* This function should unlock a csemutex_t that has been previously
 * initialized using csemutex_init and is currently locked.  It uses
 * atomic compare-and-swap to unlock the mutex. */
void csemutex_unlock(csemutex_t *mutex);

#endif /* CSEMUTEX_H_ */
