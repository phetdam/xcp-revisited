/**
 * @file locapprox.c
 * @author Derek Huang
 * @brief Expert C Programming (p146): approximate segment locator
 * @copyright MIT License
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "pdxcp/error.h"

// initialized global should give top of data segment
int sniff_data = 5;

int
main()
{
  // first declared variable should be top of stack
  int sniff_stack;
  printf("Approx. top of stack: 0x%p (+ int)\n", &sniff_stack);
  printf("Approx. top of data:  0x%p\n", &sniff_data);
  // proxy top of text by using address of main. not very accurate since there
  // are usually other implementation-dependent functions above main
  printf("Approx. top of text:  0x%p\n", &main);
  // if available, use sbrk to find top of heap (bottom of BSS)
#if defined(_DEFAULT_SOURCE) || defined(_BSD_SOURCE)
  printf("Approx. top of heap:  0x%p (sbrk(0))\n", sbrk(0));
#else
  // proxy top of heap with a malloc call. on MinGW this address is typically
  // far higher than the top of the stack (due to implementation?)
  char *sniff_heap = malloc(256);
  PDXCP_ERRNO_EXIT_IF(!sniff_heap);
  printf("Approx. top of heap:  0x%p (malloc(256))\n", sniff_heap);
  free(sniff_heap);
#endif  // !defined(_DEFAULT_SOURCE) && !defined(_BSD_SOURCE)
  // make stack grow by initializing a buffer
  char buf[BUFSIZ] = {'h', 'e', 'l', 'l', 'o'};
  // note that buf is same as &buf here since arrays decay to pointers
  // TODO: addresses seem to be increasing... oops?
  printf("Approx. top of stack: 0x%p (+ char[%ld])\n", buf, sizeof buf);
  // grow stack some more
  unsigned long long longbuf[BUFSIZ * 2] = {8888};
  printf(
    "Approx. top of stack: 0x%p (+ unsigned long long[%d])\n",
    longbuf,
    BUFSIZ * 2
  );
  return EXIT_SUCCESS;
}
