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
  printf("&ca:    %p\n", &ca);     // address of const char * ca
  printf("&ca[0]: %p\n", &ca[0]);  // value of ca
  printf("&ca[1]: %p\n", &ca[1]);  // value of ca + 1
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
  printf("&pa:    %p\n", &pa);     // address of const char * pa
  printf("&pa[0]: %p\n", &pa[0]);  // value of pa
  printf("&pa[1]: %p\n", &pa[1]);  // value of pa + 1
  printf("++pa:   %p\n", ++pa);    // value of original pa + 1
}

int
main(void)
{
  printf("&ga:    %p\n", &ga);     // address to ga[0] (value of ga)
  printf("&ga[0]: %p\n", &ga[0]);  // address to ga[0] (value of ga)
  printf("&ga[1]: %p\n", &ga[1]);  // address to ga[1] (value of ga + 1)
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
  // one may be surprised to see that &pa equals &ca, but that is likely due to
  // the compiler pushing both pointers to the same position on the stack right
  // under the address held in the %rbp register. this can be proven by viewing
  // the generated assembly code, e.g. with x86-64 GCC 11.3.0 one sees:
  //
  // 0000000000001149 <arr_addrs>:
  //     1149:	f3 0f 1e fa          	endbr64
  //     114d:	55                   	push   %rbp
  //     114e:	48 89 e5             	mov    %rsp,%rbp
  //     1151:	48 83 ec 10          	sub    $0x10,%rsp
  //     1155:	48 89 7d f8          	mov    %rdi,-0x8(%rbp)
  //     ...
  //
  // 00000000000011b1 <ptr_addrs>:
  //     11b1:	f3 0f 1e fa          	endbr64
  //     11b5:	55                   	push   %rbp
  //     11b6:	48 89 e5             	mov    %rsp,%rbp
  //     11b9:	48 83 ec 10          	sub    $0x10,%rsp
  //     11bd:	48 89 7d f8          	mov    %rdi,-0x8(%rbp)
  //     ...
  //
  // so one can see that the argument coming in %rdi is put in the same
  // location relative to the %rbp base pointer value.
  //
  return EXIT_SUCCESS;
}
