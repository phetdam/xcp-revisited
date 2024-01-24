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
 * @param stack Token stack to initialize. If `NULL`, nothing is done
 */
void
pdxcp_cdcl_token_stack_init(pdxcp_cdcl_token_stack *stack) PDXCP_NOEXCEPT;

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
  ((stack)->tokens + (stack)->n_tokens)

/**
 * Macro for pushing a token onto the stack.
 *
 * @note Behavior is undefined if the stack is full.
 *
 * @param stack Pointer to a valid `pdxcp_cdcl_token_stack`
 * @param token Pointer to a valid `pdxcp_cdcl_token`
 */
#define PDXCP_CDCL_TOKEN_STACK_PUSH(stack, token) \
  memcpy(PDXCP_CDCL_TOKEN_STACK_HEAD(stack) + 1, token, sizeof *(token)); \
  (stack)->n_tokens++

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
  pdxcp_cdcl_parser_status_ok,              // no error
  pdxcp_cdcl_parser_status_in_null,         // input FILE * is NULL
  pdxcp_cdcl_parser_status_out_null,        // output FILE * is NULL
  pdxcp_cdcl_parser_status_eof,             // read EOF before finishing parse
  pdxcp_cdcl_parser_status_lexer_err,       // lexer error, check errinfo
  pdxcp_cdcl_parser_status_token_overflow,  // token stack overflow
  pdxcp_cdcl_parser_status_out_err,         // error writing to output FILE *
  pdxcp_cdcl_parser_status_parse_err,       // parse error, check errinfo
  pdxcp_cdcl_parser_status_bad_token        // bad token (duplicate?)
} pdxcp_cdcl_parser_status;

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
 * Struct to hold parser error information.
 *
 * @todo Rename + rework to hold parser error info as well, not just lexer.
 *
 * @param lexer_status Lexer status
 * @param token_text Error text if lexer read bad token
 */
typedef struct {
  pdxcp_cdcl_lexer_status lexer_status;
  char token_text[PDXCP_CDCL_MAX_TOKEN_LEN + 1];
} pdxcp_cdcl_lexer_errinfo;

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
  FILE *in, FILE *out, pdxcp_cdcl_lexer_errinfo *errinfo) PDXCP_NOEXCEPT;

PDXCP_EXTERN_C_END

#endif  // PDXCP_CDCL_PARSER_H_
