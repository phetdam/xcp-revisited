/**
 * @file cdcl_parser.c
 * @author Derek Huang
 * @brief C source for C declaration parser
 * @copyright MIT License
 */

#include "pdxcp/cdcl_parser.h"

#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include "pdxcp/cdcl_lexer.h"

void
pdxcp_cdcl_token_stack_init(pdxcp_cdcl_token_stack *stack)
{
  if (stack)
    stack->n_tokens = 0;
}

const char *
pdxcp_cdcl_parser_status_message(pdxcp_cdcl_parser_status status)
{
  switch (status) {
    case pdxcp_cdcl_parser_status_ok:
      return "Success";
    case pdxcp_cdcl_parser_status_in_null:
      return "Input stream is NULL";
    case pdxcp_cdcl_parser_status_out_null:
      return "Output stream is NULL";
    case pdxcp_cdcl_parser_status_eof:
      return "Parser read EOF before finishing parser";
    case pdxcp_cdcl_parser_status_lexer_err:
      return "Lexer error, check parser error info lexer text";
    case pdxcp_cdcl_parser_status_token_overflow:
      return "Too many tokens to fit on token stack";
    case pdxcp_cdcl_parser_status_out_err:
      return "Error writing parser output to stream";
    case pdxcp_cdcl_parser_status_parse_err:
      return "Parser error, check parser error info";
    default:
      return "Unknown parser status";
  }
}

/**
 * Write parser error info.
 *
 * @param errinfo Error info structure
 * @param lexer_status Lexer status
 * @param cur_token Most recent token read by lexer. If the lexer status is
 *  `pdxcp_cdcl_lexer_status_bad_token`, its text will contain the error text.
 *  Ignored unless `lexer_status` is `pdxcp_cdcl_lexer_status_bad_token`.
 * @param parser_status Parser status
 * @param parser_text Null-terminated parser error text for more details when
 *  `parser_status` is `pdxcp_cdcl_parser_status_parse_err` (generic error).
 *  Ignored unless `parser_status` is `pdxcp_cdcl_parser_status_parse_err`. If
 *  `NULL` when expected, errinfo parser status instead becomes
 *  `pdxcp_cdcl_parser_status_null_err_text`.
 */
static void
pdxcp_cdcl_write_errinfo(
  pdxcp_cdcl_parser_errinfo *errinfo,
  pdxcp_cdcl_lexer_status lexer_status,
  const pdxcp_cdcl_token *cur_token,
  pdxcp_cdcl_parser_status parser_status,
  const char *parser_text)
{
  if (!errinfo)
    return;
  // write status values
  errinfo->lexer.status = lexer_status;
  errinfo->parser.status = parser_status;
  // if the token is bad, copy the token text
  if (lexer_status == pdxcp_cdcl_lexer_status_bad_token) {
    strcpy(errinfo->lexer.text, cur_token->text);
    errinfo->lexer.text[strlen(cur_token->text)] = '\0';  // terminate string
  }
  // check parser_text if parser_status is generic error
  if (parser_status == pdxcp_cdcl_parser_status_parse_err) {
    // if NULL, use specific error code
    if (!parser_text) {
      errinfo->parser.status = pdxcp_cdcl_parser_status_null_err_text;
      errinfo->parser.text[0] = '\0';
      return;
    }
    // otherwise, write actual error text. first get error text length
    size_t parser_text_len = strlen(parser_text);
    // if too long, truncate and use appropriate status
    if (parser_text_len > PDXCP_CDCL_PARSER_ERROR_TEXT_LEN) {
      memcpy(errinfo->parser.text, parser_text, PDXCP_CDCL_PARSER_ERROR_TEXT_LEN);
      errinfo->parser.text[PDXCP_CDCL_PARSER_ERROR_TEXT_LEN] = '\0';
    }
    // otherwise, copy null-terminated entire string in
    else
      strcpy(errinfo->parser.text, parser_text);
  }
}

/**
 * Read tokens from input stream until an identifier is parsed.
 *
 * @param in Input stream
 * @param lexer_status Lexer status to update for error reporting
 * @param token_stack Token stack to push previously read tokens onto
 * @param cur_token Most recent token read by the lexer from input stream
 * @returns `pdxcp_cdcl_parser_status`
 */
static pdxcp_cdcl_parser_status
stream_parse_to_iden(
  FILE *in,
  pdxcp_cdcl_lexer_status *lexer_status,
  pdxcp_cdcl_token_stack *token_stack,
  pdxcp_cdcl_token *cur_token)
{
  // read tokens from lexer until error
  while (
    PDXCP_CDCL_LEXER_OK(*lexer_status = pdxcp_cdcl_get_token(in, cur_token))
  ) {
    // if identifier, break. time to start parsing
    if (cur_token->type == pdxcp_cdcl_token_type_iden)
      break;
    // if token stack is full, can't push token, so error
    if (PDXCP_CDCL_TOKEN_STACK_FULL(token_stack))
      return pdxcp_cdcl_parser_status_token_overflow;
    // push the token and continue
    PDXCP_CDCL_TOKEN_STACK_PUSH(token_stack, cur_token);
  }
  // note lexer error, otherwise success
  if (!PDXCP_CDCL_LEXER_OK(*lexer_status))
    return pdxcp_cdcl_parser_status_lexer_err;
  return pdxcp_cdcl_parser_status_ok;
}

/**
 * Pop tokens off of the token stack to handle pointers in the declaration.
 *
 * Also handles cv-qualifiers for the tokens.
 *
 * @param stack Token stack to pop from
 * @param out Output stream
 * @returns `pdxcp_cdcl_parser_status`
 */
static pdxcp_cdcl_parser_status
stream_parse_ptrs(pdxcp_cdcl_token_stack *stack, FILE *out)
{
  bool has_const = false;
  bool has_volatile = false;
  // TODO: need to parse cv-qualifiers correctly
  while (!PDXCP_CDCL_TOKEN_STACK_EMPTY(stack)) {
    // switch on the token type
    switch (PDXCP_CDCL_TOKEN_STACK_HEAD(stack)->type) {
      // const qualifier, only one allowed
      case pdxcp_cdcl_token_type_q_const:
        // TODO: write error message somewhere
        if (has_const)
          return pdxcp_cdcl_parser_status_parse_err;
        has_const = true;
        break;
      // volatile qualifier, only one allowed
      case pdxcp_cdcl_token_type_q_volatile:
        // TODO: write error message somewhere
        if (has_volatile)
          return pdxcp_cdcl_parser_status_parse_err;
        has_volatile = true;
        break;
      // pointer
      case pdxcp_cdcl_token_type_star:
        // handle cv-qualification
        if (has_const && fprintf(out, " const") < 0)
          return pdxcp_cdcl_parser_status_out_err;
        if (has_volatile && fprintf(out, " volatile") < 0)
          return pdxcp_cdcl_parser_status_out_err;
        // print pointer + reset cv-qualifiers
        if (fprintf(out, " pointer to") < 0)
          return pdxcp_cdcl_parser_status_out_err;
        has_const = has_volatile = false;
        break;
      // unexpected token. if either has_const or has_volatile is true, then
      // definitely this is a parse error, otherwise assume success
      default:
        // TODO: where to write error message
        if (has_const || has_volatile)
          return pdxcp_cdcl_parser_status_parse_err;
        return pdxcp_cdcl_parser_status_ok;
    }
    // done with token so pop from stack
    PDXCP_CDCL_TOKEN_STACK_POP(stack);
  }
  // if stack is empty, we are missing tokens, e.g. type, etc.
  // TODO: where to write error message
  if (PDXCP_CDCL_TOKEN_STACK_EMPTY(stack))
    return pdxcp_cdcl_parser_status_parse_err;
  return pdxcp_cdcl_parser_status_ok;
}

pdxcp_cdcl_parser_status
pdxcp_cdcl_stream_parse(FILE *in, FILE *out, pdxcp_cdcl_parser_errinfo *errinfo)
{
  // check streams
  if (!in)
    return pdxcp_cdcl_parser_status_in_null;
  if (!out)
    return pdxcp_cdcl_parser_status_out_null;
  // allocate + initialize token stack
  pdxcp_cdcl_token_stack stack;
  pdxcp_cdcl_token_stack_init(&stack);
  // statuses + current token
  pdxcp_cdcl_lexer_status lexer_status;
  pdxcp_cdcl_parser_status parser_status;
  pdxcp_cdcl_token token;
  // read tokens from lexer until error
  parser_status = stream_parse_to_iden(in, &lexer_status, &stack, &token);
  if (!PDXCP_CDCL_PARSER_OK(parser_status)) {
    pdxcp_cdcl_write_errinfo(errinfo, lexer_status, &token, parser_status, NULL);
    return parser_status;
  }
  // write identifer token
  if (fprintf(out, "%s:", token.text) < 0)
    return pdxcp_cdcl_parser_status_out_err;
  // read another token, handling lexer error as appropriate
  if (!PDXCP_CDCL_LEXER_OK(lexer_status = pdxcp_cdcl_get_token(in, &token))) {
    pdxcp_cdcl_write_errinfo(errinfo, lexer_status, &token, parser_status, NULL);
    return pdxcp_cdcl_parser_status_lexer_err;
  }
  // TODO: handle array, function, etc.
  // if read semicolon, end of declaration
  if (token.type == pdxcp_cdcl_token_type_semicolon) {
    // consume pointer tokens from stack
    if (!PDXCP_CDCL_PARSER_OK(parser_status = stream_parse_ptrs(&stack, out)))
      return parser_status;
    // TODO: parse cv-qualified signed/unsigned qualified type
    if (fprintf(out, " a thing\n") < 0)
      return pdxcp_cdcl_parser_status_out_err;
    return pdxcp_cdcl_parser_status_ok;
  }
  // TODO: not implemented
  else
    return pdxcp_cdcl_parser_status_bad_token;
  return pdxcp_cdcl_parser_status_ok;
}
