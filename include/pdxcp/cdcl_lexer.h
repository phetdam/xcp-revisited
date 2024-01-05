/**
 * @file cdcl_lexer.h
 * @author Derek Huang
 * @brief C/C++ header for the C declaration lexer
 * @copyright MIT License
 */

#ifndef PDXCP_CDCL_LEXER_H_
#define PDXCP_CDCL_LEXER_H_

#include <stdio.h>

#include "pdxcp/common.h"

PDXCP_EXTERN_C_BEGIN

/**
 * Token type enumeration.
 */
typedef enum {
  pdxcp_cdcl_token_type_error,       // error, unknown token
  pdxcp_cdcl_token_type_lparen,      // (
  pdxcp_cdcl_token_type_rparen,      // )
  pdxcp_cdcl_token_type_langle,      // [
  pdxcp_cdcl_token_type_rangle,      // ]
  pdxcp_cdcl_token_type_comma,       // ,
  pdxcp_cdcl_token_type_star,        // *
  pdxcp_cdcl_token_type_struct,      // struct <text>
  pdxcp_cdcl_token_type_enum,        // enum <text>
  pdxcp_cdcl_token_type_q_const,     // const
  pdxcp_cdcl_token_type_q_volatile,  // volatile
  pdxcp_cdcl_token_type_iden,        // <text> (identifier)
  pdxcp_cdcl_token_type_max          // number of valid token types
} pdxcp_cdcl_token_type;

/**
 * Return error message string for the given token type value.
 *
 * If the token type value is unknown, a pointer to `"(unknown)"` is returned.
 *
 * @param type Token type value
 */
const char *
pdxcp_cdcl_token_type_string(pdxcp_cdcl_token_type type);

/**
 * Lexer status codes.
 */
typedef enum {
  pdxcp_cdcl_lexer_status_ok,           // no error
  pdxcp_cdcl_lexer_status_stream_null,  // FILE * is NULL
  pdxcp_cdcl_lexer_status_token_null,   // pdxcpl_cdcl_token * is NULL
  pdxcp_cdcl_lexer_status_ungetc_fail,  // couldn't ungetc last char
  pdxcp_cdcl_lexer_status_fgetc_eof,    // fgetc retrieved EOF
  pdxcp_cdcl_lexer_status_not_iden,     // not an identifier
  pdxcp_cdcl_lexer_status_bad_token     // bad token, token text has details
} pdxcp_cdcl_lexer_status;

/**
 * Macro for checking if a lexer routine failed.
 *
 * @param status `pdxcp_cdcl_lexer_status` value
 */
#define PDXCP_CDCL_LEXER_ERROR(status) ((status) != pdxcp_cdcl_lexer_status_ok)

/**
 * Maximum token length.
 */
#define PDXCP_CDCL_MAX_TOKEN_LEN 79

/**
 * Token struct.
 *
 * @param type Token type
 * @param text Null-terminated token text
 */
typedef struct pdxcp_cdcl_token {
  pdxcp_cdcl_token_type type;
  char text[PDXCP_CDCL_MAX_TOKEN_LEN + 1];
} pdxcp_cdcl_token;

/**
 * Get the next token from the specified input stream.
 *
 * @param in Input stream to read from
 * @param token Token to write to
 * @returns `pdxcp_cdcl_lexer_status` status code. If
 *  `pdxcp_cdcl_lexer_status_bad_token` is returned, the token type is
 *  `pdxcp_cdcl_token_type_error` and token text has error details
 */
pdxcp_cdcl_lexer_status
pdxcp_cdcl_get_token(FILE *in, pdxcp_cdcl_token *token);

PDXCP_EXTERN_C_END

#endif  // PDXCP_CDCL_LEXER_H_
