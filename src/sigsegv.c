/**
 * @file sigsegv.c
 * @author Derek Huang
 * @brief Expert C Programming (p190): causing + catching a segmentation fault
 * @copyright MIT License
 */

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>

#include "pdxcp/error.h"

/**
 * `SIGSEGV` handler that prints a message before exiting.
 *
 * @note `puts`, `exit` technically not safe to call from signal handlers.
 *
 * @param signum Signal value received
 */
static void
sigsegv_handler(int signum)
{
  if (signum == SIGSEGV) {
    puts("Caught SIGSEGV. Please ensure NULL pointers are not dereferenced.");
    exit(EXIT_FAILURE);
  }
}

int
main()
{
  // install SIGSEGV signal handler
#if defined(_WIN32)
  PDXCP_ERRNO_EXIT_IF(signal(SIGSEGV, sigsegv_handler) == SIG_ERR);
#else
  // again, Windows/MinGW does not have struct sigaction
  struct sigaction sigbus_action = {.sa_handler = sigsegv_handler};
  PDXCP_ERRNO_EXIT_IF(sigaction(SIGSEGV, &sigbus_action, NULL));
#endif  // !defined(_WIN32)
  // p190 code fragment. classic attempt to write to a NULL pointer
  int *p = NULL;
  // bad line causing segmentation fault (writing to address 0)
  // note: in MinGW's signal.h, there is a comment that says SIGSEGV does not
  // catch writing to a null pointer, but it seems to work fine
  *p = 17;
  return EXIT_SUCCESS;
}
