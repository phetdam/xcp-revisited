/**
 * @file kbpoll.c
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
    // message to write to stdout if char received
    char msg[] = "Got character 'X'\n";
    // pointer to the X; this is overwritten in the read() call. note we have
    // -4 instead of -3 since there is a null terminator to account for
    char *out = msg + sizeof msg - 4;
    // attempt to get character from standard input
    ssize_t n_read = read(STDIN_FILENO, out, 1);
    // simply return if nothing to read or character is non-printable
    if (!n_read || !isprint(*out))
      return;
    // exit on read error, i.e. return is -1
    PDXCP_ERRNO_EXIT_IF(n_read < 0);
    // perform write, exiting if there is a failure
    PDXCP_ERRNO_EXIT_IF(!full_write(STDOUT_FILENO, msg, sizeof msg - 1));
    // quit if 'q' is received
    if (*out == 'q' || *out == 'Q')
      exit(EXIT_SUCCESS);
  }
}

int
main()
{
  // sleep spec for nanosleep
  struct timespec spec = {.tv_sec = 1};
  // counter to increment
  size_t counter = 0;
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
  printf("Type 'q' to exit\n");
  while (true) {
    printf("Waiting for input... ");
    fflush(stdout);  // ensure print is done
    // sleep (could be interrupted) and increment counter
    PDXCP_ERRNO_EXIT_IF(nanosleep(&spec, NULL) && errno != EINTR);
    counter++;
  }
  return EXIT_SUCCESS;
}
