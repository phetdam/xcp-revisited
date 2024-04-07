/**
 * @file filehash.c
 * @author Derek Huang
 * @brief Expert C Programming (p223): hash-table based file lookup program
 * @copyright MIT License
 */

#include <errno.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * Number of buckets in the file lookup hash table.
 *
 * This can be defined to 1 to use a trivial (returning zero) hash function
 * and to limit the hash table buckets to 1, resulting in hash lookup decaying
 * into simple linked-list traversal. Useful as an exercise/for debugging.
 */
#ifndef FILE_HASH
#define FILE_HASH 10
#endif  // FILE_HASH

/**
 * Safety check for the value of `FILE_HASH`.
 *
 * @note If `FILE_HASH` is a string the preprocessor will happily accept it
 *  because a string literal decays into a `const char[]` into a pointer.
 */
#if FILE_HASH < 1
#error "FILE_HASH must be defined to a positive value"
#endif  // FILE_HASH >= 1

/**
 * Maximum file name length excluding the null-termination character.
 *
 * This mirrors the 4096 value for `PATH_MAX` in Linux.
 */
#define MAX_PATH_LEN 4095

/**
 * File information struct that satisfies program requirements.
 *
 * This does not necessarily correspond to an actual file descriptor.
 *
 * @param fname Dynamically-allocated null-terminated file name
 * @param flink Pointer to next dynamically-allocated file struct in bucket
 */
typedef struct file_struct {
  char *fname;
  struct file_struct *flink;
} file_struct;

/**
 * Pointer to `file_struct` typedef to satisfy the book's code semantics.
 *
 * The `find_filename` function in the book uses `file` as a pointer to struct.
 */
typedef file_struct *file;

/**
 * File hash table whose size is dependent on `FILE_HASH`.
 *
 * This table contains only pointers to `file_struct`.
 */
static file file_hash_table[FILE_HASH];

/**
 * Helper to check the validity of a file name.
 *
 * Returns `true` if file name is valid, e.g. non-empty string that at most
 * `MAX_PATH_LEN` characters, not including the null terminator.
 *
 * @param s Null-terminated string
 * @param s_len Address to write string length to, ignored if `NULL`
 * @returns `true` if valid, `false` otherwise with `errno` set to `EINVAL
 */
static inline bool
valid_filename(const char *s, size_t *s_len)
{
  // file name cannot be NULL
  if (!s) {
    errno = EINVAL;
    return false;
  }
  // file name must not be empty or exceed the limit
  size_t len = strnlen(s, MAX_PATH_LEN + 1);
  if (!len || len > MAX_PATH_LEN) {
    errno = EINVAL;
    return false;
  }
  // if s_len is not NULL, save value of len
  if (s_len)
    *s_len = len;
  return true;
}

/**
 * Create a new file information struct.
 *
 * On error, `NULL` is returned and `errno` should be checked for error.
 *
 * @param s Null-terminated file name, max `MAX_PATH_LEN` chars in length
 */
static file
allocate_file(const char *s)
{
  // validate file name (sets errno)
  size_t s_len;
  if (!valid_filename(s, &s_len))
    return NULL;
  // allocate new file_struct (malloc errors with ENOMEM)
  file f = malloc(sizeof *f);
  if (!f)
    return NULL;
  // allocate space for file name. need to destroy file struct on error
  f->fname = malloc(s_len + 1);
  if (!f->fname) {
    free(f);
    return NULL;
  }
  // copy file name. can use strcpy since we already checked it
  strcpy(f->fname, s);
  // set flink to NULL and complete
  f->flink = NULL;
  return f;
}

/**
 * Free all memory associated with a `file_struct` and its linked successors.
 *
 * Nothing is done if `f` is a `NULL` pointer.
 *
 * @param f File info struct to free
 */
static void
destroy_file_list(file f)
{
  while (f) {
    free(f->fname);
    file f_next = f->flink;
    free(f);
    f = f_next;
  }
}

/**
 * Compute a hash value from a file name.
 *
 * If `FILE_HASH` is 1 this decays into a function that returns zero.
 *
 * @note This has been modified from the original `hash_filename` function to
 *  be const-correct and use the correct `strlen` return type. It will also
 *  conditionally compile to a trivial hash function is `FILE_HASH` is zero.
 *
 * @note Behavior is undefined if `s` is empty string or not null-terminated.
 *
 * @param s Null-terminated file name
 */
static inline size_t
hash_filename(const char *s)
{
#if FILE_HASH == 1
  return 0;
#else
  size_t length = strlen(s);
  return (length + 4 * (s[0] + 4 * s[length / 2])) % FILE_HASH;
#endif  // FILE_HASH != 1
}

/**
 * Locate a previously created file info struct or create one if necessary.
 *
 * On error `NULL` is returned and `errno` will be set.
 *
 * @note This has been modified from the original `find_filename` function to
 *  have safety checks for the file name as well as the result of the
 *  `allocate_file` call (which can fail). We also eschew the use of the `NIL`
 *  and `SAME` macros which are presumably defined to `NULL` and `0`.
 *
 * @param s Null-terminated file name
 */
static file
find_filename(const char *s)
{
  // validate file name (sets errno)
  if (!valid_filename(s, NULL))
    return NULL;
  // compute hash for file name
  size_t hash_value = hash_filename(s);
  file f;
  // search table bucket for file
  for (f = file_hash_table[hash_value]; f; f = f->flink)
    if (!strcmp(f->fname, s))
      return f;
  // create new file if not found in table (sets errno)
  f = allocate_file(s);
  if (!f)
    return NULL;
  // insert into table, taking existing value as successor, and return
  f->flink = file_hash_table[hash_value];
  file_hash_table[hash_value] = f;
  return f;
}

/**
 * Return the number of elements in an array.
 *
 * This macro was added after catching a buffer overrun with AddressSanitizer
 * since the number of elements in `file_hash_table` was changed from
 * `FILE_HASH + 1` (too many) to just `FILE_HASH`. The final table cleanup loop
 * was using `FILE_HASH + 1` which is of course an overrun.
 *
 * @note This is the "obvious" implementation but fails with pointers. Unlike
 *  if using `a[0]` this should have no issues in C++ for objects that overload
 *  `operator[]`. See https://stackoverflow.com/a/4415646/14227825 for details.
 *
 * @param a Fixed-size array
 */
#define ARRAY_SIZE(a) sizeof (a) / sizeof *(a)

int
main(void)
{
  // arbitrary file paths
  const char *paths[] = {
    "/path/to/file",
    "/usr/bin/ls",
    "/usr/local/lib/libsodium.so.6",
    "/etc/alternatives/c++.1.gz",
    "/usr/bin/x86_64-linux-gnu-g++-11",
    "/another/path/to/file"
  };
  // insert into hash table (no error checking done here)
  for (size_t i = 0; i < ARRAY_SIZE(paths); i++)
    (void) find_filename(paths[i]);
  // print all hash buckets in the file hash table
  for (size_t i = 0; i < ARRAY_SIZE(file_hash_table); i++) {
    // note: width of index could be determined at runtime based on FILE_HASH
    printf("bucket %zu: ", i);
    file f = file_hash_table[i];
    // no files in bucket
    if (!f) {
      printf("(empty)\n");
      continue;
    }
    // print file names separated with " -> "
    while (f) {
      if (f != file_hash_table[i])
        printf(" -> ");
      printf("\"%s\"", f->fname);
      f = f->flink;
    }
    putchar('\n');
  }
  // clear file hash table completely
  for (size_t i = 0; i < ARRAY_SIZE(file_hash_table); i++) {
    // if not NULL, not only do we destroy the file list, but also set to NULL
    if (file_hash_table[i]) {
      destroy_file_list(file_hash_table[i]);
      file_hash_table[i] = NULL;
    }
  }
  return EXIT_SUCCESS;
}
