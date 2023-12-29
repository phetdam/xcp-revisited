/**
 * @file sigcatch.c
 * @author Derek Huang
 * @brief Expert C Programming (p188): simple signal catching program
 * @copyright MIT License
 */

#include <unistd.h>

#include <errno.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "pdxcp/error.h"

// program name set from argv[0]. needed by execv
const char *progname = NULL;

// SIGBREAK is only available on Windows/MinGW. seems that there is also no
// need to reinstall the handler on upon signal receipt
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
  if (signum == SIGBREAK)
    puts("Caught SIGBREAK. Continuing");
}
#endif  // _WIN32

/**
 * `SIGINT` signal handler that prints a message.
 *
 * @note `puts` technically not safe to call from signal handlers.
 *
 * @param signum Signal value received
 */
static void
sigint_handler(int signum)
{
  if (signum == SIGINT)
    puts("Caught SIGINT. Restarting");
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
  // loop forever. need flush to ensure the print actually happens
  while (true) {
    printf("Waiting... ");
    fflush(stdout);
    pause();
  }
  return EXIT_SUCCESS;
}
