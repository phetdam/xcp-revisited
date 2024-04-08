/**
 * @file mdarrinc.c
 * @author Derek Huang
 * @brief Expert C Programming (p256): multidimensional array address increment
 * @copyright MIT License
 */

#include <stdio.h>
#include <stdlib.h>

int
main(void)
{
  // 3D array and pointer definitions given on p256
  int apricot[2][3][5];
  int (*r)[5] = apricot[0];
  int *t = apricot[0][0];
  // print initial values of r, t
  printf("r   = %p\n", r);
  printf("t   = %p\n", t);
  // print incremented values of r, t
  printf("++r = %p\n", ++r);  // incremented by 5 * sizeof(int) bytes
  printf("++t = %p\n", ++t);  // incremented by 1 * sizeof(int) bytes
  return EXIT_SUCCESS;
}
