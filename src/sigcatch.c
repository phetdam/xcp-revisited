/**
 * @file sigcatch.c
 * @author Derek Huang
 * @brief Expert C Programming (p188): simple signal catching program
 * @copyright MIT License
 */

#include <errno.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "pdxcp/error.h"

// program name set from argv[0]. needed by execv
const char *progname = NULL;

// SIGBREAK is only available on Windows/MinGW
#ifdef _WIN32
/**
 * `SIGBREAK` handler that prints a message.
 *
 * @note `puts` is technically not safe to call from signal handlers.
 *
 * @param signum Signal value received
 */
static void
sigbreak_handler(int signum)
{
  switch (signum) {
    case SIGBREAK:
      puts("Caught SIGBREAK. Continuing");
  }
}
#endif  // _WIN32

/**
 * `SIGINT` signal handler that prints a message and restarts the program.
 *
 * @note `puts` technically not safe to call from signal handlers.
 *  `PDXCP_ERRNO_EXIT_IF` also calls `fprintf`, `exit` which are also not safe.
 *
 * @param signum Signal value received
 */
static void
sigint_handler(int signum)
{
  switch (signum) {
    case SIGINT: {
      puts("Caught SIGINT. Restarting");
      // execv might fail, in which case it will return
      const char *const argv[2] = {progname, NULL};
      // cast is done to please compiler; should be const char *const *
      PDXCP_ERRNO_EXIT_IF(execv(progname, (char *const *) argv));
    }
  }
}

int
main(int argc, char *argv[])
{
  // set program name
  progname = argv[0];
  // install SIGBREAK handler
  // note: no struct sigaction on Windows (e.g. with MinGW)
#if defined(_WIN32)
  PDXCP_ERRNO_EXIT_IF(signal(SIGBREAK, sigbreak_handler) == SIG_ERR);
#endif  // !defined(_WIN32)
  // install SIGINT handler
#if defined(_WIN32)
  PDXCP_ERRNO_EXIT_IF(signal(SIGINT, sigint_handler) == SIG_ERR);
#else
  struct sigaction sigint_action = {.sa_handler = sigint_handler};
  PDXCP_ERRNO_EXIT_IF(sigaction(SIGINT, &sigint_action, NULL));
#endif  // !defined(_WIN32)
  printf("Waiting... ");
  fflush(stdout);
  // block thread. sleep 1 ms to be easier on the CPU
  // note: hangs on WSL, maybe just use setjmp/longjmp?
  while (true) {
    struct timespec spec = {.tv_nsec = 1000};
    // could be interrupted by signal handlers
    PDXCP_ERRNO_EXIT_IF(nanosleep(&spec, NULL) && errno != EINTR);
  }
  return EXIT_SUCCESS;
}
