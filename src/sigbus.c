/**
 * @file sigbus.c
 * @author Derek Huang
 * @brief Expert C Programming (p190): causing + catching a bus error
 * @copyright MIT License
 */

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>

#include "pdxcp/error.h"

// SIGBUS not defined/caught when using MinGW
#ifndef __MINGW32__
/**
 * `SIGBUS` handler that prints a message before exiting.
 *
 * @note `puts`, `exit` technically not safe to call from signal handlers.
 *
 * @param signum Signal value received
 */
static void
sigbus_handler(int signum)
{
  if (signum == SIGBUS) {
    puts("Caught SIGBUS. Please ensure addresses used are properly aligned.");
    exit(EXIT_FAILURE);
  }
}
#endif  // __MINGW32__

int
main()
{
  // install SIGBUS signal handler. SIGBUS not produced on MinGW.
  // note: unaligned memory access may not result in SIGBUS on x86 machines in
  // general; see https://orchistro.tistory.com/206 for discussion
#if defined(_WIN32)
#ifndef __MINGW32__
  PDXCP_ERRNO_EXIT_IF(signal(SIGBUS, sigbus_handler) == SIG_ERR);
#endif  // __MINGW32__
#else
  // again, Windows/MinGW does not have struct sigaction
  struct sigaction sigbus_action = {.sa_handler = sigbus_handler};
  PDXCP_ERRNO_EXIT_IF(sigaction(SIGBUS, &sigbus_action, NULL));
#endif  // !defined(_WIN32)
  // p189 code fragment. p is misaligned and its use causes bus error
  union {
    char a[10];
    int i;
  } u;
  int *p = (int *) &(u.a[1]);  // or u.a + 1
  // bad line causing bus error (misaligned pointer use). not caught on MinGW
  // or x86 machines that do not have alignment check enabled in [ER]FLAGS
  *p = 17;
#ifdef __MINGW32__
  puts("Expected SIGBUS not emitted and thus not caught on MinGW");
#endif  // __MINGW32__
  return EXIT_SUCCESS;
}
