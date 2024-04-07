/**
 * @file rejmp.c
 * @author Derek Huang
 * @brief Expert C Programming (p155): setjmp/longjmp restart
 * @copyright MIT License
 */

#include <errno.h>
#include <setjmp.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "pdxcp/error.h"
#include "pdxcp/warnings.h"

// Windows (MinGW) doesn't have random(); in general requires _BSD_SOURCE, etc.
#if defined(_BSD_SOURCE)
#define RANDINT random
#define SRANDINT srandom
#else
#define RANDINT rand
#define SRANDINT srand
#endif  // !defined(_BSD_SOURCE)

/**
 * Write block of random multiline ASCII text to a stream.
 *
 * @note `srand` should be called before this function.
 *
 * ASCII text used are the human-readable characters 33 through 126.
 *
 * @param out Stream to write to
 * @param n_cols Number of columns, must be >=3
 * @param n_rows Number of rows, must be >=3
 * @param 0 on success, -EINVAL if a parameter is too small or `out` is `NULL`,
 *  or `-errno` if `fputc` exited with an error
 */
static int
ascii_block(FILE *out, unsigned int n_cols, unsigned n_rows)
{
  if (!out || n_cols < 3 || n_rows < 3)
    return -EINVAL;
  // write text body
  for (unsigned int nr = 0; nr < n_rows; nr++) {
    for (unsigned int nc = 0; nc < n_cols; nc++)
      if (fputc(RANDINT() % 94 + 33, out) == EOF)
        return -errno;
    if (fputc('\n', out) == EOF)
      return -errno;
  }
  return 0;
}

// ASCII block size constants
#define ASCII_COLS 20
#define ASCII_ROWS 10

/**
 * Looping task serving as the program's event loop.
 *
 * `setjmp` + `longjmp` are used to "restart" the loop.
 *
 * @returns `EXIT_SUCCESS` on success
 */
static int
loop_task()
{
  // jump context buffer. must be volatile to prevent compiler optimizations
  // from causing indeterminate value: see man 3 setjmp
  volatile jmp_buf restore_buf;
  // since setjmp/longjmp is used as a goto, no need for if-else. we can also
  // discard the return value so no logic needs to be run.
PDXCP_GNU_WARNING_PUSH()
PDXCP_GNU_WARNING_DISABLE(-Wdiscarded-qualifiers)
  setjmp(restore_buf);
PDXCP_GNU_WARNING_POP()
  // print block
  int status;
  if ((status = ascii_block(stdout, ASCII_COLS, ASCII_ROWS)))
    pdxcp_error_exit(-status);
  // ask user for restart, doing longjmp if requested
  char msgbuf[256];
  while (true) {
    // get user input
    printf("Restart? ");
    PDXCP_ERRNO_EXIT_IF(!fgets(msgbuf, sizeof msgbuf, stdin));
    // if trailing newline (typically does), replace with '\0'
    size_t msgbuf_len = strlen(msgbuf);
    if (msgbuf[msgbuf_len - 1] == '\n')
       msgbuf[msgbuf_len - 1] = '\0';
    // restart using longjmp
    if (!strcmp(msgbuf, "y") || !strcmp(msgbuf, "yes")) {
PDXCP_GNU_WARNING_PUSH()
PDXCP_GNU_WARNING_DISABLE(-Wdiscarded-qualifiers)
      longjmp(restore_buf, 1);
PDXCP_GNU_WARNING_POP();
    }
    // normal exit
    else if (!strcmp(msgbuf, "n") || !strcmp(msgbuf, "no"))
      break;
    // bad input
    else
      fprintf(
        stderr,
        "Bad response '%s'; only 'y', 'yes', 'n', 'no' allowed\n",
        msgbuf
      );
  }
  return EXIT_SUCCESS;
}

int
main(void)
{
  // seed PRNG + run our "event loop"
  SRANDINT(time(NULL));
  return loop_task();
}
