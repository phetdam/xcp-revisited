/**
 * @file arrptrbind.c
 * @author Derek Huang
 * @brief Expert C Programming (p272): array/pointer function argument binding
 * @copyright MIT License
 */

#include <stdio.h>
#include <stdlib.h>

/**
 * Function declared to take a 3D array.
 *
 * @note Modified from the book's version to be ANSI-compliant + print text.
 *
 * @param fruit 3D array with dimensions 2, 3, 5
 * @param desc String description of the argument
 */
static void
my_function_1(int fruit[2][3][5], const char *desc)
{
  printf("expected: int[2][3][5]     received: %s\n", desc);
}

/**
 * Function declared to take a 3D array with unknown first bound.
 *
 * @note Modified from the book's version to be ANSI-compliant + print text.
 *
 * @param fruit 3D array with unknown first dimension, trailing dimensions 3, 5
 * @param desc String description of the argument
 */
static void
my_function_2(int fruit[][3][5], const char *desc)
{
  printf("expected: int[][3][5]      received: %s\n", desc);
}

/**
 * Function declared to take a pointer to a 2D array.
 *
 * @note Modified from the book's version to be ANSI-compliant + print text.
 *
 * @param fruit Pointer to 2D array with dimensions 3, 5
 * @param desc String description of the argument
 */
static void
my_function_3(int (*fruit)[3][5], const char *desc)
{
  printf("expected: int (*)[3][5]    received: %s\n", desc);
}

int
main(void)
{
  // 3D array, pointer to 2D array, and pointer to 3D array defined on p272.
  int apricot[2][3][5];
  int (*p)[3][5] = apricot;
  int (*q)[2][3][5] = &apricot;  // note &apricot and apricot are equal
  // messages for each description
  const char *apricot_desc = "int[2][3][5]";
  const char *p_desc = "int (*)[3][5]";
  const char *q_desc = "*(int (*)[2][3][5])";
  // function calls with each pair of argument and description
  my_function_1(apricot, apricot_desc);
  my_function_2(apricot, apricot_desc);
  my_function_3(apricot, apricot_desc);
  my_function_1(p, p_desc);
  my_function_2(p, p_desc);
  my_function_3(p, p_desc);
  my_function_1(*q, q_desc);
  my_function_2(*q, q_desc);
  my_function_3(*q, q_desc);
  return EXIT_SUCCESS;
}
