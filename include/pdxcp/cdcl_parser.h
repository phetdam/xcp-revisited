/**
 * @file cdcl_parser.h
 * @author Derek Huang
 * @brief C/C++ header for the C declaration parser
 * @copyright MIT License
 */

#ifndef PDXCP_CDCL_PARSER_H_
#define PDXCP_CDCL_PARSER_H_

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>

#include "pdxcp/cdcl_lexer.h"
#include "pdxcp/common.h"

PDXCP_EXTERN_C_BEGIN

/**
 * Number of tokens that can fit in a single parser token stack.
 */
#define PDXCP_CDCL_PARSER_STACK_SIZE 20

/**
 * Token stack.
 *
 * @param n_tokens Number of tokens currently in the stack
 * @param tokens Array of saved tokens
 */
typedef struct {
  size_t n_tokens;
  pdxcp_cdcl_token tokens[PDXCP_CDCL_PARSER_STACK_SIZE];
} pdxcp_cdcl_token_stack;

/**
 * Initialize a token stack by setting its size to zero.
 *
 * @param stack Pointer to a valid `pdxcp_cdcl_token_stack`
 */
#define PDXCP_CDCL_TOKEN_STACK_INIT(stack) (stack)->n_tokens = 0

/**
 * Macro for checking if the token stack is empty or not.
 *
 * @param stack Pointer to a valid `pdxcp_cdcl_token_stack`
 */
#define PDXCP_CDCL_TOKEN_STACK_EMPTY(stack) !(stack)->n_tokens

/**
 * Macro for checking if the token stack is full or not.
 *
 * @param stack Pointer to a valid `pdxcp_cdcl_token_stack`
 */
#define PDXCP_CDCL_TOKEN_STACK_FULL(stack) \
  ((stack)->n_tokens >= PDXCP_CDCL_PARSER_STACK_SIZE)

/**
 * Macro for getting a pointer to the token on the top of the stack.
 *
 * @note Behavior is undefined if the stack is empty.
 *
 * @param stack Pointer to a valid `pdxcp_cdcl_token_stack`
 */
#define PDXCP_CDCL_TOKEN_STACK_HEAD(stack) \
  ((stack)->tokens + (stack)->n_tokens - 1)

/**
 * Macro for pushing a token onto the stack.
 *
 * @note Behavior is undefined if the stack is full.
 *
 * @param stack Pointer to a valid `pdxcp_cdcl_token_stack`
 * @param token Pointer to a valid `pdxcp_cdcl_token`
 */
#define PDXCP_CDCL_TOKEN_STACK_PUSH(stack, token) \
  do { \
    memcpy(PDXCP_CDCL_TOKEN_STACK_HEAD(stack) + 1, token, sizeof *(token)); \
    (stack)->n_tokens++; \
  } \
  while (false)

/**
 * Macro for popping a token off of the stack.
 *
 * @note Behavior is undefined if the stack is empty.
 *
 * @param stack Pointer to a valid `pdxcp_cdcl_token_stack`
 */
#define PDXCP_CDCL_TOKEN_STACK_POP(stack) (stack)->n_tokens--

/**
 * Parser status codes.
 */
typedef enum {
  // no error
  pdxcp_cdcl_parser_status_ok,
  // input FILE * is NULL
  pdxcp_cdcl_parser_status_in_null,
  // output FILE * is NULL
  pdxcp_cdcl_parser_status_out_null,
  // parser read EOF before finishing parse
  pdxcp_cdcl_parser_status_eof,
  // lexer error, check errinfo
  pdxcp_cdcl_parser_status_lexer_err,
  // token stack overflow
  pdxcp_cdcl_parser_status_token_overflow,
  // error writing parser output to FILE *
  pdxcp_cdcl_parser_status_out_err,
  // parser error, check errinfo
  pdxcp_cdcl_parser_status_parse_err,
  // bad token (may be removed, could be duplicate as lexer checks tokens)
  pdxcp_cdcl_parser_status_bad_token,
  // parser error text is NULL when it should not be
  pdxcp_cdcl_parser_status_null_err_text,
  // supplied parser error text is too long and therefore truncated
  pdxcp_cdcl_parser_status_err_text_too_long
} pdxcp_cdcl_parser_status;

/**
 * Return a string for the given parser status value.
 *
 * If the value is unknown, a pointer to `"(unknown)"` is returned.
 *
 * @param status Lexer status value
 */
const char *
pdxcp_cdcl_parser_status_string(pdxcp_cdcl_parser_status status) PDXCP_NOEXCEPT;

/**
 * Return a message for the given parser status value.
 *
 * If the value is unknown, a pointer to `"Unknown parser status"` is returned.
 *
 * @param value Parser status value
 */
const char *
pdxcp_cdcl_parser_status_message(pdxcp_cdcl_parser_status status) PDXCP_NOEXCEPT;

/**
 * Macro for checking if a parser routine exited with ok status.
 *
 * Unlike the lexer, parser only succeeds if this macro evalutes to true.
 *
 * @param status `pdxcp_cdcl_parser_status` value
 */
#define PDXCP_CDCL_PARSER_OK(status) ((status) == pdxcp_cdcl_parser_status_ok)

/**
 * Maximum length of parser error text.
 */
#define PDXCP_CDCL_PARSER_ERROR_TEXT_LEN 255

/**
 * Struct to hold parser error information.
 *
 * @todo Rename + rework to hold parser error info as well, not just lexer.
 *
 * @param lexer Lexer error info
 * @param parser Parser error info
 */
typedef struct {
  /**
   * Lexer error info.
   *
   * @param status Lexer status
   * @param text Null-terminated lexer error text. If `status` is
   *  `pdxcp_cdcl_lexer_status_bad_token`, contains details on the error
   */
  struct {
    pdxcp_cdcl_lexer_status status;
    char text[PDXCP_CDCL_MAX_TOKEN_LEN + 1];
  } lexer;

  /**
   * Parser error info.
   *
   * @note Size of parser error text may be changed later.
   *
   * @param status Parser status
   * @param text Null-terminated parser error text
   */
  struct {
    pdxcp_cdcl_parser_status status;
    char text[PDXCP_CDCL_PARSER_ERROR_TEXT_LEN + 1];
  } parser;
} pdxcp_cdcl_parser_errinfo;

/**
 * Parse text from the input stream and write output to the output stream.
 *
 * This routine parses valid C declarations from the input stream and writes a
 * description of the declaration to the output stream.
 *
 * @note Parser is currently not atomic in its operation. It can write partial
 *  output before hitting an error since it is basically a pushdown automata,
 *  storing tokens for later consumption on a stack.
 */
pdxcp_cdcl_parser_status
pdxcp_cdcl_stream_parse(
  FILE *in, FILE *out, pdxcp_cdcl_parser_errinfo *errinfo) PDXCP_NOEXCEPT;

PDXCP_EXTERN_C_END

#endif  // PDXCP_CDCL_PARSER_H_
