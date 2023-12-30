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

#include "pdxcp/error.h"
#include "pdxcp/lockable.h"

/**
 * Lockable type definitions.
 */
PDXCP_LKABLE_DEF(bool)
PDXCP_LKABLE_DEF(size_t)

/**
 * Lockable type getter definitions.
 */
static PDXCP_LKABLE_GET_DEF(bool)
static PDXCP_LKABLE_GET_DEF(size_t)

/**
 * Lockable type setter definitions.
 */
static PDXCP_LKABLE_SET_V_DEF(bool)
static PDXCP_LKABLE_SET_V_DEF(size_t)

/**
 * Struct defining payload used by the worker thread.
 *
 * @param stopspec `PDXCP_LKABLE(bool)` for indicating when to stop looping
 * @param counter `PDXCP_LKABLE(size_t)` providing the locked counter
 * @param sleepspec `nanosleep` sleep specification
 */
typedef struct {
  PDXCP_LKABLE(bool) stopspec;
  PDXCP_LKABLE(size_t) counter;
  struct timespec sleepspec;
} worker_payload;

/**
 * Helper function to get milliseconds from a `struct timespec`.
 *
 * @param spec Time specification in seconds and nanoseconds
 */
static size_t
timespec_ms(const struct timespec *spec)
{
  if (!spec)
    return 0;
  return spec->tv_sec * 1000 + spec->tv_nsec / 1000000;
}

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
  // number of 1 ms iterations to loop + 1 ms as a struct timespec
  size_t spin_count = timespec_ms(&payload->sleepspec);
  struct timespec spin_spec = {.tv_nsec = 1000000};
  // loop until done or until something is wrong
  while (
    !(status = PDXCP_LKABLE_GET(bool)(&payload->stopspec, &stop_loop)) &&
    !stop_loop
  ) {
    // sleep, checking every 1 ms if we need to break, exiting on error
    for (size_t i = 0; i < spin_count; i++) {
      // break could be due to error or due to actually needing to break
      if (
        (status = PDXCP_LKABLE_GET(bool)(&payload->stopspec, &stop_loop)) ||
        stop_loop
      )
        goto check_thread_status;
      // otherwise sleep, exit on error
      PDXCP_ERRNO_EXIT_EX_IF(
        nanosleep(&spin_spec, NULL), "%s nanosleep error", __func__
      );
    }
    // get old value, break on error
    size_t old_value;
    if ((status = PDXCP_LKABLE_GET(size_t)(&payload->counter, &old_value)))
      break;
    // set new value, break on error
    if ((status = PDXCP_LKABLE_SET_V(size_t)(&payload->counter, old_value + 1)))
      break;
  }
  // exit if there was a pthreads error
check_thread_status:
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
 * @param counter Lockable counter to peek
 */
static void
handle_input_events(int fd, PDXCP_LKABLE(size_t) *counter)
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
      int status = PDXCP_LKABLE_GET(size_t)(counter, &count);
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
  if ((status = PDXCP_LKABLE_SET_V(bool)(&payload.stopspec, true)))
    PDXCP_ERROR_EXIT_EX(-status, "%s", "Failed to halt worker thread");
  // join and exit
  if ((status = pthread_join(worker_thread, NULL)))
    PDXCP_ERROR_EXIT_EX(status, "%s", "Failed to properly join worker thread");
  return EXIT_SUCCESS;
}
