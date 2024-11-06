Implementing Mutexes with Compare-and-Swap
==

In this recitation, you will implement mutexes using atomic
compare-and-swap operations provided by the compiler.  Your mutex will
use an integer as its internal state.

Getting started
--

You will find a header file (`csemutex.h`) and associated C source
template (`csemutex.c`) in the `src/` directory.  You should read  both
the header and source file before you begin implementation.

Compare-and-Swap
--

Your mutex should use an _atomic compare-and-swap_ (sometimes called
_CAS_) operation for setting the mutex state between locked and
unlocked.  This will ensure that there are no race conditions between
threads manipulating the mutex, and that only one thread can lock the
mutex at a time.

Compare-and-swap operations work by _atomically_ comparing the value
stored at a memory location with a specified _expected_ value, and
swapping it with an alternate _desired_ value __only if it matches__.
They typically return a value indicating either:

 * The old value of the memory location (which can be used to determine
   whether or not a swap was successful), or
 * A direct indication of whether or not the swap was successful

You can think of a compare-and-swap instruction as equivalent to the
following C code, but guaranteed to operate atomically:

```C
int compare_and_swap(int *addr, int expected, int desired) {
    int oldvalue = *addr;
    if (oldvalue == expected) {
        *addr = desired;
    }
    return oldvalue;
}
```

Remember that an _atomic_ operation appears to happen instantaneously to
an outside observer, and that it is either fully successful or appears
as if it was never attempted.

Not all hardware directly supports compare-and-swap; on some processors
(such as ARM) a compare-and-swap operation is a multi-instruction
sequence.  On the x86-64 processor, the instruction is called `cmpxchg`
and performs an _optionally atomic_ compare-and-swap operation which
becomes atomic when paired with a _lock prefix_.  (You are not required
to know this, it is provided for interest value only!)

Yielding the Scheduler
--

In order to produce a more efficient (yet still wildly inefficient!)
mutex, you will _yield_ the scheduler between checks to see whether the
mutex is currently locked.  This causes the current thread to give up
the CPU if there are any other runnable threads on the system.  On a
single-processor system, this prevents a thread that cannot immediately
acquire a mutex from using up its entire scheduling quantum attempting
to acquire a mutex that it will not acquire until some other thread
runs.

POSIX defines the function `sched_yield()` for this.  Its prototype is:

```C
#include <sched.h>

int sched_yield(void);
```

You should consult the man page for `sched_yield()` if you have any
questions.

Atomic Functions
--

The gcc compiler provides a number of atomic operations as non-standard
builtin functions.  We will be using those functions for this
implementation.  The given sources already use `__atomic_thread_fence()`
to provide memory barriers, and you should neither move nor remove those
calls.  __You are not required to understand memory barriers at this
time!  Simply do not move or remove these function calls and you will be
fine!__

In addition, there is a given wrapper for the function
`__atomic_compare_exchange_n()`, which is a difficult-to-use builtin for
which the wrapper `cse_cas()` provides a simpler interface.  It is
defined as follows:

```C
int cse_cas(int *addr, int expected, int desired);
```

This function will perform an atomic compare-and-swap on the location
given by the argument `addr`.  It will compare `*addr` with `expected`,
and _if and only if_ the value stored at `addr` is the value of
`expected`, it will store `desired` at `*addr` and return 1.  Otherwise,
it will do nothing, and return 0.  It is equivalent to the following
code, except that it is atomic (and uses `__atomic_compare_exchange_n()`
to accomplish this).

```C
int cse_cas(int *addr, int expected, int desired) {
    if (*addr == expected) {
        *addr = desired;
        return 1;
    }
    return 0;
}
```

Threads
--

You can see examples of thread creation in the two given tests,
`tests/simple-lock.c` and `tests/counters.c`.  These tests use
`pthread_create()` to create threads that use your mutex.  This function
is defined as follows:

```c
#include <pthread.h>

int pthread_create(pthread_t *thread, const pthread_attr_t *attr,
                   void *(*start_routine) (void *), void *arg);
```

The only part of this that we haven't seen before is the `start_routine`
argument; this is a _function pointer_.  It can be given the name of any
function that meets the following specification:

```c
void *threadfunc(void *argument);
```

The `thread` argument is a pointer to a variable of type `pthread_t`,
and the `attr` argument can be passed as `NULL` for our purposes.  The
`arg` argument is _not used by `pthread_create`_, but will be passed to
`start_routine` as its argument.  Thus, we would create a thread that
does nothing as follows:

```c
#include <pthread.h>

void *do_nothing(void *arg) {
    return NULL;
}

int main(int argc, char *argv) {
    pthread_t t;

    pthread_create(&t, NULL, do_nothing, NULL);
    pthread_join(t, NULL);

    return 0;
}
```

The `pthread_join()` call simply waits for a named thread to finish.  In
this case, it prevents the program from exiting before `do_nothing()`
has run.

Time
--

In multi-threaded systems, and particularly in testing multi-threaded
systems, it is often valuable to pause a thread briefly.  We have
previously seen a function that will do this:

```c
#include <unistd.h>

int usleep(useconds_t usec);
```

This function, `usleep()`, sleeps for the specified number of
_microseconds_.  Recalling your SI prefixes, one microsecond is 1/10^6
seconds, or one millionth of one second.  On our modern computers, this
is several thousand processor instructions under most conditions.  One
thousand microseconds is one millisecond, which is typically a million
or more processor instructions.  Delays in the small numbers of
milliseconds _virtually guarantee_, even without synchronization, that
events can be effectively ordered by the programmer for testing
purposes.  They do not _actually_ guarantee this, but they are good
enough for our tests for now!  You will see that the given tests use
this technique to order operations without assuming that your mutex is
working 100% correctly.

Mutexes
--

Recall that a mutex must be able to store two states, "locked" and
"unlocked".  As the given code suggests, a simple integer value of 0 or
1 may meet this requirement.  However, that integer must be _guaranteed
atomic updates_, and a thread attempting to lock the mutex must _block_
and wait for the mutex to be unlocked if it is already locked.  You can
perform this blocking with a loop that continually checks the mutex to
see if it is unlocked, and changes it to be locked only if it is.

Requirements
--

You must implement the two functions `csemutex_lock()` and
`csemutex_unlock()` in `csemutex.c`.  The given implementations are
_not correct_, although the implementation of `csemutex_unlock()`, in
particular, may work anyway.

The function `csemutex_init()` is given to you, and you may assume that
it is correct.

Your `csemutex_lock()` must loop until the lock can be successfully
acquired, and it must call `sched_yield()` in each iteration of the loop
before attempting the lock again.


