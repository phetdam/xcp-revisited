/**
 * @file segsize.c
 * @author Derek Huang
 * @brief Expert C Programming (p149): looking at executable segment sizes
 * @copyright MIT License
 */

#include <stdio.h>
#include <stdlib.h>

// macro controlling modifications made per each step of the exercise to the
// program. for convenience, if not defined, we just define to 1
#ifndef SEGSIZE_STEPS
#define SEGSIZE_STEPS 1
#endif  // SEGSIZE_STEPS

// use 4a. for uninitialized large auto array, 4b. for initialized large auto
// array. default to 4a. if neither is defined but allow both to be defined
#if \
  SEGSIZE_STEPS >= 4 && \
  !defined(SEGSIZE_STEPS_4A) && !defined(SEGSIZE_STEPS_4B)
#define SEGSIZE_STEPS_4A
#endif  // SEGSIZE_STEPS < 4 ||
        // defined(SEGSIZE_STEPS_4A) || defined(SEGSIZE_STEPS_4B)

// for 5, define SEGSIZE_STEPS_4A and SEGSIZE_STEPS_4B if not defined
#if SEGSIZE_STEPS >= 5
#ifndef SEGSIZE_STEPS_4A
#define SEGSIZE_STEPS_4A
#endif  // SEGSIZE_STEPS_4A
#ifndef SEGSIZE_STEPS_4B
#define SEGSIZE_STEPS_4B
#endif  // SEGSIZE_STEPS_4B
#endif  // SEGSIZE_STEPS < 5

// 2. add int[1000] declaration (put in BSS)
#if SEGSIZE_STEPS >= 2
static int bigarr[1000]
// 3. add initialized int[1000] (put in data)
#if SEGSIZE_STEPS >= 3
  = {8888}
#endif  // SEGSIZE_STEPS < 3
  ;
#endif  // SEGSIZE_STEPS < 2

int
main()
{
  // silence -Wunused-variable
#if SEGSIZE_STEPS >= 2
  (void) bigarr;
#endif  // SEGSIZE_STEPS < 2
  // 4. declare large local array uninitialized (4a) and/or initialized (4b).
  // uninitialized has no data/text/bss effect, initialized *does* result in
  // an small increase in data + text segment sizes
#if SEGSIZE_STEPS >= 4
#ifdef SEGSIZE_STEPS_4A
  unsigned long long longbuf[4096];
  (void) longbuf;
#endif  // SEGSIZE_STEPS_4A
#ifdef SEGSIZE_STEPS_4B
  double dblbuf[2048] = {888.};
  (void) dblbuf;
#endif  // SEGSIZE_STEPS_4B
#endif  // SEGSIZE_STEPS < 4
  puts("Hello world!");
  return EXIT_SUCCESS;
}
