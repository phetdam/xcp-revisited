/**
 * @file arrptrcmp.c
 * @author Derek Huang
 * @brief Expert C Programming (p249): compare array and pointer addressing
 * @copyright MIT License
 */

#include <stdio.h>
#include <stdlib.h>

/**
 * Character array containing the alphabet with a null terminator.
 *
 * This will be used by `arr_addrs` and `ptr_addrs`.
 */
static const char ga[] = "abcdefghijklmnopqrstuvwxyz";

/**
 * Print out different addresses related to the specified character array.
 *
 * `&ca` is the address of the `const char *` copy `ca`, `&ca[0]` is equivalent
 * to just the value of `ca`, and `&ca[1]` is equivalent to `ca + 1`.
 *
 * @param ca Character array
 */
static void
arr_addrs(const char ca[])
{
  printf("&ca:    0x%p\n", &ca);     // address of const char * ca
  printf("&ca[0]: 0x%p\n", &ca[0]);  // value of ca
  printf("&ca[1]: 0x%p\n", &ca[1]);  // value of ca + 1
}

/**
 * Print out different addresses related to the specified char pointer.
 *
 * `&pa` is the address of the `const char *` copy `pa`, `&pa[0]` is equivalent
 * to just the value of `pa`, `&pa[1]` is equivalent to `pa + 1`, and `++pa` is
 * equivalent to `pa + 1` before the pre-increment was done.
 *
 * @param pa Char pointer
 */
static void
ptr_addrs(const char *pa)
{
  printf("&pa:    0x%p\n", &pa);     // address of const char * pa
  printf("&pa[0]: 0x%p\n", &pa[0]);  // value of pa
  printf("&pa[1]: 0x%p\n", &pa[1]);  // value of pa + 1
  printf("++pa:   0x%p\n", ++pa);    // value of original pa + 1
}

int
main(void)
{
  printf("&ga:    0x%p\n", &ga);     // address to ga[0] (value of ga)
  printf("&ga[0]: 0x%p\n", &ga[0]);  // address to ga[0] (value of ga)
  printf("&ga[1]: 0x%p\n", &ga[1]);  // address to ga[1] (value of ga + 1)
  //
  // comparison to &ga, &ga[0], &ga[1]:
  //
  // &ca != &ga, &ca is address of copied pointer ca
  // &ca[0] == &ga[0], both refer to address of first element of ga
  // &ca[1] == &ga[1], both refer to address of second element of ga
  //
  arr_addrs(ga);
  //
  // comparison to &ga, &ga[0], &ga[1]:
  //
  // &pa != &ga, &pa is address of copied pointer pa
  // &pa[0] == &ga[0], both refer to address of first element of ga
  // &pa[1] == &ga[1], both refer to address of second element of ga
  // ++pa == &ga[1], both refer to address of second element of ga
  //
  ptr_addrs(ga);
  //
  // remarks:
  //
  // surprisingly, &pa can equal &ca, i.e. it seems like GCC 11.3.0 has decided
  // to reuse the same pointer for pa and ca. yet, modifying pa does not result
  // in any change in ca; likely some aliasing is being done under the hood.
  //
  return EXIT_SUCCESS;
}
