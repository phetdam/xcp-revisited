/**
 * @file kbsig.c
 * @author Derek Huang
 * @brief Expert C Programming (p217): signal-driven input handling program
 * @copyright MIT License
 */

#include <fcntl.h>
#include <unistd.h>

#include <ctype.h>
#include <errno.h>
#include <signal.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "pdxcp/error.h"

/**
 * Perform a full blocking write to a file descriptor.
 *
 * @param fd File descriptor to write to
 * @param buf Buffer to write
 * @param n_bytes Number of bytes to write
 * @returns `true` on success, `false` on error. Check `errno` on error.
 */
static bool
full_write(int fd, const char *buf, size_t n_bytes)
{
  // remaining bytes to write, bytes already written
  size_t n_remain = n_bytes;
  ssize_t n_write;
  // perform write, exiting if there is a failure
  do {
    n_write = write(fd, buf + n_bytes - n_remain, n_remain);
    if (n_write < 0)
      return false;
    n_remain -= n_write;
  }
  while (n_remain);
  return true;
}

/**
 * Global counter to increment.
 *
 * We should technically only use `sig_atomic_t`, not other integral types,
 * despite that typically one can assume atomicity. However, it has been noted
 * that even recently load/store tearing has occurred in the kernel. See
 * https://lwn.net/Articles/793253/ for relevant discussion.
 */
volatile sig_atomic_t global_counter = 0;

/**
 * Return pointer to a static buffer representing a long long as a string.
 *
 * @note Although this function is MT-Unsafe we only call it in our `SIGPOLL`
 *  signal handler, which due to use of `sigaction` blocks additional `SIGPOLL`
 *  so we can assume that no buffer corruption occurs. We could also have
 *  modified this function to return a struct containing a `char[128]`.
 *
 * @param value Value to get string representation for
 */
const char *
llong_to_string(long long value)
{
  // buffer is large enough to hold 2 ^ 256 comfortably
  static char buf[128];
  // simple case: zero
  if (!value)
    return "0";
  // otherwise, we need to dynamically fill the buffer. first add '\0'
  buf[sizeof buf - 1] = '\0';
  // check if value is negative or not. if so, ensure positive
  bool is_negative = (value < 0);
  if (is_negative)
    value *= -1;
  // index to first element of buf to return
  size_t buf_index = sizeof buf - 2;
  // loop from least to most significant, converting to ASCII. note that 0 is
  // 48 in ASCII, so 48-57 is the range of values
  for (; value; value /= 10)
    buf[buf_index--] = (char) (value % 10 + 48);
  // if value was negative, add '-' and return at buf_index
  if (is_negative) {
    buf[buf_index] = '-';
    return (const char *) (buf + buf_index);
  }
  // otherwise, return buf_index + 1
  return (const char *) (buf + buf_index + 1);
}

/**
 * `SIGPOLL` (`SIGIO`) handler for responding to keyboard input.
 *
 * Do not use this handler with `signal` since it does not reinstall itself.
 *
 * @note `fprintf` and `exit` should not be called from signal handlers.
 *
 * @param signum Signal value received
 */
static void
sigpoll_handler(int signum)
{
  if (signum == SIGPOLL) {
    // partial message to write to stdout if char received
    char msg[] = "Got character 'X'. Counter: ";
    // pointer to the X; this is overwritten in the read() call
    char *out = msg + 15;
    // attempt to get character from standard input
    // TODO: most of the time spent in thsi program is blocked by this read
    // call since stdin is sending SIGPOLL as soon as it is ready for I/O, not
    // when there are actually characters written to this stream
    ssize_t n_read = read(STDIN_FILENO, out, 1);
    // simply return if nothing to read or character is non-printable
    if (!n_read || !isprint(*out))
      return;
    // exit on read error, i.e. return is -1
    PDXCP_ERRNO_EXIT_IF(n_read < 0);
    // quit if 'q' or 'Q' is received
    if (*out == 'q' || *out == 'Q')
      exit(EXIT_SUCCESS);
    // perform partial message write, exiting if there is a failure
    PDXCP_ERRNO_EXIT_IF(!full_write(STDOUT_FILENO, msg, sizeof msg - 1));
    // convert counter to string
    const char *count_msg = llong_to_string(global_counter);
    // write counter value, exiting if there is a failure
    PDXCP_ERRNO_EXIT_IF(!full_write(STDOUT_FILENO, count_msg, strlen(count_msg)));
    // write final newline, exiting if there is a failure
    PDXCP_ERRNO_EXIT_IF(!full_write(STDOUT_FILENO, "\n", 1));
  }
}

int
main()
{
  // sleep spec for nanosleep
  struct timespec spec = {.tv_sec = 1};
  // install SIGIO/SIGPOLL signal handler
  struct sigaction sigpoll_action = {.sa_handler = sigpoll_handler};
  PDXCP_ERRNO_EXIT_IF(sigaction(SIGPOLL, &sigpoll_action, NULL));
  //
  // no I_SETSIG ioctl on Linux since the (obsolete) POSIX STREAMS is not
  // implemented. the standard way to achieve the same result is using fcntl;
  // see https://stackoverflow.com/a/45376104/14227825 for an example.
  //
  // get current stdin file status flags
  int flags = fcntl(STDIN_FILENO, F_GETFL);
  PDXCP_ERRNO_EXIT_IF(flags < 0);
  // add O_ASYNC to stdin flags to generate SIGIO when input is available
  PDXCP_ERRNO_EXIT_IF(fcntl(STDIN_FILENO, F_SETFL, O_ASYNC | flags));
  // allow us to receive SIGIO signals from stdin
  PDXCP_ERRNO_EXIT_IF(fcntl(STDIN_FILENO, F_SETOWN, getpid()));
  // loop
  puts("Type 'q' or 'Q' to exit");
  while (true) {
    printf("Waiting for input... ");
    fflush(stdout);  // ensure print is done
    // sleep (could be interrupted) and increment counter
    PDXCP_ERRNO_EXIT_IF(nanosleep(&spec, NULL) && errno != EINTR);
    // TODO: since we are constantly blocked by the read call in the SIGPOLL
    // handler, this only updates whenever input is read and the signal handler
    // returns. see kbpoll.c for a more realistic and correct implementation
    // using poll and pthreads to do background work while polling input
    global_counter++;
  }
  return EXIT_SUCCESS;
}
