/**
 * @file kbpoll.c
 * @author Derek Huang
 * @brief Expert C Programming (p217): event-driven input handling program
 * @copyright MIT License
 */

#include <poll.h>
#include <pthread.h>
#include <unistd.h>

#include <ctype.h>
#include <errno.h>
#include <signal.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "pdxcp/common.h"
#include "pdxcp/error.h"

/**
 * Macro for namespacing a lockable type.
 *
 * A corresponding `LOCKABLE_DEF` is required to define the lockable type.
 *
 * @param type Type to namespace
 */
#define LOCKABLE(type) PDXCP_CONCAT(lockable_, type)

/**
 * Define a type encapsulating a type member with a `pthread_mutex_t`.
 *
 * @param type Value member type
 */
#define LOCKABLE_DEF(type) \
  typedef struct { \
    type value; \
    pthread_mutex_t mutex; \
  } LOCKABLE(type);

/**
 * Macro for namespacing a call to a lockable type's getter.
 *
 * A corresponding `LOCKABLE_GET_DEF` is required to define the getter.
 *
 * @param type Type to namespace
 */
#define LOCKABLE_GET(type) PDXCP_CONCAT(LOCKABLE(type), _get)

/**
 * Define the getter function for a lockable type.
 *
 * All getters are invoked using `LOCKABLE_GET(type)` and take two arguments.
 * The first is a `LOCKABLE(type) *lockable` input and the second is a
 * `type *out` output. Zero is returned on success, `-EINVAL` if any arg is
 * `NULL`, and other negative values for additional errors.
 *
 * @param type Type to define getter for
 */
#define LOCKABLE_GET_DEF(type) \
  int \
  LOCKABLE_GET(type)(LOCKABLE(type) *lockable, type *out) \
  { \
    int status; \
    /* args must be non-NULL */ \
    if (!lockable || !out) \
      return -EINVAL; \
    /* attempt lock */ \
    if ((status = pthread_mutex_lock(&lockable->mutex))) \
      return -status; \
    /* write and attempt unlock */ \
    *out = lockable->value; \
    return -pthread_mutex_unlock(&lockable->mutex); \
  }

/**
 * Macro for namespacing a call to a lockable type's by-value setter.
 *
 * A corresponding `LOCKABLE_SET_V_DEF` is required to define the setter.
 */
#define LOCKABLE_SET_V(type) PDXCP_CONCAT(LOCKABLE(type), _set_v)

/**
 * Define the by-value setter function for a lockable type.
 *
 * All setters are invoked using `LOCKABLE_SET_V(type)` and take two arguments.
 * The first is a `LOCKABLE(type) *lockable` input and the second is a `type in`
 * input. Zero is returned on success, `-EINVAL` if any arg is `NULL`, with
 * other negative values for any additional errors.
 */
#define LOCKABLE_SET_V_DEF(type) \
  int \
  LOCKABLE_SET_V(type)(LOCKABLE(type) *lockable, type in) \
  { \
    int status; \
    /* lockable must be non-NULL */ \
    if (!lockable) \
      return -EINVAL; \
    /* attempt lock */ \
    if ((status = pthread_mutex_lock(&lockable->mutex))) \
      return -status; \
    /* update and attempt unlock */ \
    lockable->value = in; \
    return -pthread_mutex_unlock(&lockable->mutex); \
  }

/**
 * Lockable type definitions.
 */
LOCKABLE_DEF(bool)
LOCKABLE_DEF(size_t)

/**
 * Lockable type getter definitions.
 */
static LOCKABLE_GET_DEF(bool)
static LOCKABLE_GET_DEF(size_t)

/**
 * Lockable type setter definitions.
 */
static LOCKABLE_SET_V_DEF(bool)
static LOCKABLE_SET_V_DEF(size_t)

/**
 * Struct defining payload used by the worker thread.
 *
 * @param stopspec `LOCKABLE(bool)` for indicating when to stop looping
 * @param counter `LOCKABLE(size_t)` providing the locked counter
 * @param sleepspec `nanosleep` sleep specification
 */
typedef struct {
  LOCKABLE(bool) stopspec;
  LOCKABLE(size_t) counter;
  struct timespec sleepspec;
} worker_payload;

/**
 * Task to run in worker thread that alternates sleep and work.
 *
 * The thread will sleep for the specified interval with `nanosleep` before
 * incrementing the counter and will repeat this until it detects through its
 * payload that it needs to stop looping or on error.
 *
 * @param arg Payload, should be a `worker_payload *`
 */
static void *
counter_task(void *arg)
{
  worker_payload *payload = (worker_payload *) arg;
  // return status and loop indicator value
  int status;
  bool stop_loop;
  // loop until done or until something is wrong
  while (
    !(status = LOCKABLE_GET(bool)(&payload->stopspec, &stop_loop)) &&
    !stop_loop
  ) {
    // sleep, exiting if error
    PDXCP_ERRNO_EXIT_EX_IF(
      nanosleep(&payload->sleepspec, NULL), "%s nanosleep error", __func__
    );
    // get old value, break on error
    size_t old_value;
    if ((status = LOCKABLE_GET(size_t)(&payload->counter, &old_value)))
      break;
    // set new value, break on error
    if ((status = LOCKABLE_SET_V(size_t)(&payload->counter, old_value + 1)))
      break;
  }
  // exit if there was a pthreads error
  if (status)
    PDXCP_ERROR_EXIT_EX(-status, "%s pthreads mutex error", __func__);
  return NULL;
}

/**
 * Event loop for polling input events on a file descriptor.
 *
 * If any errors are encountered, the function will call `exit`.
 *
 * @param fd File descriptor to poll
 */
static void
handle_input_events(int fd, LOCKABLE(size_t) *counter)
{
  if (!counter)
    PDXCP_ERROR_EXIT_EX(EINVAL, "%s", "Lockable counter pointer is NULL");
  // events descriptor for fd
  struct pollfd desc = {fd, POLLIN};
  // print header and first wait message
  puts("Type 'q' or 'Q' to exit");
  printf("Waiting for input... ");
  fflush(stdout);
  // begin event loop. wait message only printed again after char is read
  while (true) {
    // poll fd every ms to check for input
    int n_ready = poll(&desc, 1, 1);
    // not ready, so skip to next iteration
    if (!n_ready)
      continue;
    // exit if we got an error
    PDXCP_ERRNO_EXIT_EX_IF(n_ready < 0, "%s", "poll() error");
    // otherwise, success. if nothing can be read however, continue
    if (!(desc.revents & POLLIN))
      continue;
    // else data can read, so read character from fd
    char c;
    int n_read = read(fd, &c, 1);
    // skip if nothing to read but exit on error
    if (!n_read)
      continue;
    PDXCP_ERRNO_EXIT_EX_IF(n_read < 0, "%s", "read() error");
    // stop loop if 'q' or 'Q' is received
    if (c == 'q' || c == 'Q')
      return;
    // if the character is a line feed, print the prompt again
    if (c == '\n') {
      printf("Waiting for input... ");
      fflush(stdout);
    }
    // else if the character is printable, print it and the counter value
    else if (isprint(c)) {
      size_t count;
      int status = LOCKABLE_GET(size_t)(counter, &count);
      // exit if there's an issue getting the counter value
      if (status)
        PDXCP_ERROR_EXIT_EX(-status, "%s", "Unable to get counter value");
      // otherwise print character, counter value, and wait message
      printf("Got character '%c'. Counter: %zu\n", c, count);
    }
  }
}

int
main()
{
  int status;
  // worker payload with counter, loop indicator, and timespec struct
  worker_payload payload = {
    {0, PTHREAD_MUTEX_INITIALIZER},
    {false, PTHREAD_MUTEX_INITIALIZER},
    {.tv_sec = 1}
  };
  // start thread with payload doing the computation
  pthread_t worker_thread;
  if ((status = pthread_create(&worker_thread, NULL, counter_task, &payload)))
    PDXCP_ERROR_EXIT_EX(status, "%s", "Thread creation error");
  // run event loop to poll stdin for characters to read
  handle_input_events(STDIN_FILENO, &payload.counter);
  // halt counter increment
  if ((status = LOCKABLE_SET_V(bool)(&payload.stopspec, true)))
    PDXCP_ERROR_EXIT_EX(-status, "%s", "Failed to halt worker thread");
  // join and exit
  if ((status = pthread_join(worker_thread, NULL)))
    PDXCP_ERROR_EXIT_EX(status, "%s", "Failed to properly join worker thread");
  return EXIT_SUCCESS;
}
