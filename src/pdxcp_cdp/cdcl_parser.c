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

#include "pdxcp/cdcl_common.h"
#include "pdxcp/cdcl_lexer.h"

void
pdxcp_cdcl_token_stack_init(pdxcp_cdcl_token_stack *stack)
{
  if (stack)
    stack->n_tokens = 0;
}

const char *
pdxcp_cdcl_parser_status_string(pdxcp_cdcl_parser_status status)
{
  switch (status) {
    PDXCP_STRING_CASE(pdxcp_cdcl_parser_status_ok);
    PDXCP_STRING_CASE(pdxcp_cdcl_parser_status_in_null);
    PDXCP_STRING_CASE(pdxcp_cdcl_parser_status_out_null);
    PDXCP_STRING_CASE(pdxcp_cdcl_parser_status_eof);
    PDXCP_STRING_CASE(pdxcp_cdcl_parser_status_lexer_err);
    PDXCP_STRING_CASE(pdxcp_cdcl_parser_status_token_overflow);
    PDXCP_STRING_CASE(pdxcp_cdcl_parser_status_out_err);
    PDXCP_STRING_CASE(pdxcp_cdcl_parser_status_parse_err);
    PDXCP_STRING_CASE(pdxcp_cdcl_parser_status_bad_token);
    PDXCP_STRING_CASE(pdxcp_cdcl_parser_status_null_err_text);
    PDXCP_STRING_CASE(pdxcp_cdcl_parser_status_err_text_too_long);
    default:
      return "(unknown)";
  };
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
 * @param errinfo Error info structure. If `NULL`, nothing is done
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
  // otherwise just empty string
  else
    errinfo->lexer.text[0] = '\0';
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
      errinfo->parser.status = pdxcp_cdcl_parser_status_err_text_too_long;
      memcpy(errinfo->parser.text, parser_text, PDXCP_CDCL_PARSER_ERROR_TEXT_LEN);
      errinfo->parser.text[PDXCP_CDCL_PARSER_ERROR_TEXT_LEN] = '\0';
    }
    // otherwise, copy null-terminated entire string in
    else
      strcpy(errinfo->parser.text, parser_text);
  }
  // otherwise just empty string
  else
    errinfo->parser.text[0] = '\0';
}

/**
 * Write parser error info when there is a parsing error.
 *
 * The parser error status is `pdxcp_cdcl_parser_status_parse_err`.
 *
 * @param errinfo Error info structure. If `NULL`, nothing is done
 * @param parser_text Null-terminated parser error text for more details. If
 *  `NULL` when expected, errinfo parser status instead becomes
 *  `pdxcp_cdcl_parser_status_null_err_text`.
 */
static void
pdxcp_cdcl_write_parse_err(
  pdxcp_cdcl_parser_errinfo *errinfo, const char *parser_text)
{
  return pdxcp_cdcl_write_errinfo(
    errinfo,
    pdxcp_cdcl_lexer_status_ok,
    NULL,
    pdxcp_cdcl_parser_status_parse_err,
    parser_text
  );
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
 * Also handles cv-qualifiers for the tokens and balances any left parentheses
 * on the token stack against the number of right parentheses already read.
 *
 * @param stack Token stack to pop from
 * @param out Output stream
 * @param n_rparen Number of right parentheses already read as part of decl
 * @param errinfo Error info structure, can be `NULL`
 * @returns `pdxcp_cdcl_parser_status` parser status, where if there is an
 *  error, the value written to `errinfo` is returned instead of the original
 */
static pdxcp_cdcl_parser_status
stream_parse_ptrs(
  pdxcp_cdcl_token_stack *stack,
  FILE *out,
  unsigned int n_rparen,
  pdxcp_cdcl_parser_errinfo *errinfo)
{
  // indicators for cv-qualifiers
  bool has_const = false;
  bool has_volatile = false;
  // number of left parentheses popped off stack
  unsigned int n_lparen = 0;
  // pop tokens off stack
  while (!PDXCP_CDCL_TOKEN_STACK_EMPTY(stack)) {
    // switch on the token type
    switch (PDXCP_CDCL_TOKEN_STACK_HEAD(stack)->type) {
      // const qualifier, only one allowed
      case pdxcp_cdcl_token_type_q_const:
        // already have const, duplicate qualifier
        if (has_const) {
          pdxcp_cdcl_write_parse_err(errinfo, "Duplicate pointer const qualifier");
          return pdxcp_cdcl_parser_status_parse_err;
        }
        // otherwise note const qualifier
        has_const = true;
        break;
      // volatile qualifier, only one allowed
      case pdxcp_cdcl_token_type_q_volatile:
        // already have volatile, duplicate qualifier
        if (has_volatile) {
          pdxcp_cdcl_write_parse_err(errinfo, "Duplicate pointer volatile qualifier");
          return pdxcp_cdcl_parser_status_parse_err;
        }
        // otherwise note volatile qualifier
        has_volatile = true;
        break;
      // left parentheses, so increment n_lparen
      case pdxcp_cdcl_token_type_lparen:
        n_lparen++;
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
        if (has_const || has_volatile) {
          // format message into buffer
          char errmsg[PDXCP_CDCL_PARSER_ERROR_TEXT_LEN + 1];
          snprintf(
            errmsg,
            sizeof errmsg - 1,
            "Unexpected token type %s with text \"%s\" when parsing pointers",
            pdxcp_cdcl_token_type_string(PDXCP_CDCL_TOKEN_STACK_HEAD(stack)->type),
            PDXCP_CDCL_TOKEN_STACK_HEAD(stack)->text
          );
          errmsg[sizeof errmsg - 1] = '\0';  // guarantee null termination
          // write result into error info
          pdxcp_cdcl_write_parse_err(errinfo, errmsg);
          return pdxcp_cdcl_parser_status_parse_err;
        }
        // if n_lparen != n_rparen, mismatched parentheses
        if (n_lparen != n_rparen) {
          char errmsg[PDXCP_CDCL_PARSER_ERROR_TEXT_LEN + 1];
          snprintf(
            errmsg,
            sizeof errmsg - 1,
            "Mismatched parentheses when parsing pointers, read %u '(' %u ')'",
            n_lparen,
            n_rparen
          );
          errmsg[sizeof errmsg - 1] = '\0';  // guarantee null termination
          pdxcp_cdcl_write_parse_err(errinfo, errmsg);
          return pdxcp_cdcl_parser_status_parse_err;
        }
        return pdxcp_cdcl_parser_status_ok;
    }
    // done with token so pop from stack
    PDXCP_CDCL_TOKEN_STACK_POP(stack);
  }
  // if stack is empty, we are missing tokens, e.g. type, etc.
  if (PDXCP_CDCL_TOKEN_STACK_EMPTY(stack)) {
    pdxcp_cdcl_write_parse_err(
      errinfo,
      "Unexpectedly ran out of tokens when parsing pointers, missing type"
    );
    return pdxcp_cdcl_parser_status_parse_err;
  }
  return pdxcp_cdcl_parser_status_ok;
}

/**
 * Pop tokens off of the token stack to handle the identifier's qualified type.
 *
 * Handles cv-qualifiers and sign qualifiers appropriately.
 *
 * @param stack Token stack to pop from
 * @param out Output stream
 * @param errinfo Error info structure, can be `NULL`
 * @returns `pdxcp_cdcl_parser_status` parser status, where if there is an
 *  error, the value written to `errinfo` is returned instead of the original
 */
static pdxcp_cdcl_parser_status
stream_parse_type(
  pdxcp_cdcl_token_stack *stack, FILE *out, pdxcp_cdcl_parser_errinfo *errinfo)
{
  // indicators for cv-qualifiers and sign
  bool has_const = false;
  bool has_volatile = false;
  bool is_signed = false;
  bool is_unsigned = false;
  // type token. we mark the type as error so we can distinguish whether or not
  // a type has already been read off of the token stack
  pdxcp_cdcl_token type_token;
  type_token.type = pdxcp_cdcl_token_type_error;
  // pop tokens off stack to determine type and qualifiers
  while (!PDXCP_CDCL_TOKEN_STACK_EMPTY(stack)) {
    // switch on the token type
    switch (PDXCP_CDCL_TOKEN_STACK_HEAD(stack)->type) {
      // const qualifier, only one allowed
      case pdxcp_cdcl_token_type_q_const:
        if (has_const) {
          pdxcp_cdcl_write_parse_err(errinfo, "Duplicate type const qualifier");
          return pdxcp_cdcl_parser_status_parse_err;
        }
        // otherwise note const qualifier
        has_const = true;
        break;
      // volatile qualifier, only one allowed
      case pdxcp_cdcl_token_type_q_volatile:
        if (has_volatile) {
          pdxcp_cdcl_write_parse_err(errinfo, "Duplicate type volatile qualifier");
          return pdxcp_cdcl_parser_status_parse_err;
        }
        // otherwise note volatile qualifier
        has_volatile = true;
        break;
      // signed qualifier, only one allowed
      case pdxcp_cdcl_token_type_q_signed:
        if (is_signed) {
          pdxcp_cdcl_write_parse_err(errinfo, "Duplicate signed type qualifier");
          return pdxcp_cdcl_parser_status_parse_err;
        }
        if (is_unsigned) {
          pdxcp_cdcl_write_parse_err(
            errinfo,
            "Type already qualified as unsigned, cannot re-qualify as signed"
          );
          return pdxcp_cdcl_parser_status_parse_err;
        }
        // otherwise note signed qualifier
        is_signed = true;
        break;
      // unsigned qualifier, only one allowed
      case pdxcp_cdcl_token_type_q_unsigned:
        if (is_unsigned) {
          pdxcp_cdcl_write_parse_err(errinfo, "Duplicate unsigned type qualifier");
          return pdxcp_cdcl_parser_status_parse_err;
        }
        if (is_signed) {
          pdxcp_cdcl_write_parse_err(
            errinfo,
            "Type already qualified as signed, cannot re-quaifiy as unsigned"
          );
          return pdxcp_cdcl_parser_status_parse_err;
        }
      // fill type token when a type is specified
      case pdxcp_cdcl_token_type_struct:
      case pdxcp_cdcl_token_type_enum:
      case pdxcp_cdcl_token_type_t_void:
      case pdxcp_cdcl_token_type_t_char:
      case pdxcp_cdcl_token_type_t_int:
      case pdxcp_cdcl_token_type_t_long:
      case pdxcp_cdcl_token_type_t_float:
      case pdxcp_cdcl_token_type_t_double:
        // if already parsed a type, error
        if (type_token.type != pdxcp_cdcl_token_type_error) {
          // format message
          char errmsg[PDXCP_CDCL_PARSER_ERROR_TEXT_LEN + 1];
          snprintf(
            errmsg,
            sizeof errmsg - 1,
            "Type %s provided when identifier already specified as %s",
            // TODO: add function that C type string from enum token type
            pdxcp_cdcl_token_type_string(PDXCP_CDCL_TOKEN_STACK_HEAD(stack)->type),
            pdxcp_cdcl_token_type_string(type_token.type)
          );
          errmsg[sizeof errmsg - 1] = '\0';  // guarantee null termination
          // write result into error info
          pdxcp_cdcl_write_parse_err(errinfo, errmsg);
          return pdxcp_cdcl_parser_status_parse_err;
        }
        // otherwise, copy token type and text
        type_token.type = PDXCP_CDCL_TOKEN_STACK_HEAD(stack)->type;
        strcpy(type_token.text, PDXCP_CDCL_TOKEN_STACK_HEAD(stack)->text);
        break;
      // unknown token
      default: {
        // format message
        char errmsg[PDXCP_CDCL_PARSER_ERROR_TEXT_LEN + 1];
        snprintf(
          errmsg,
          sizeof errmsg - 1,
          "Unexpected token type %s with text \"%s\" when parsing identifier type",
          // TODO: nicer way to indicate the token + text?
          pdxcp_cdcl_token_type_string(PDXCP_CDCL_TOKEN_STACK_HEAD(stack)->type),
          PDXCP_CDCL_TOKEN_STACK_HEAD(stack)->text
        );
        errmsg[sizeof errmsg - 1] = '\0';  // guarantee null termination
        // write result into error info
        pdxcp_cdcl_write_parse_err(errinfo, errmsg);
        return pdxcp_cdcl_parser_status_parse_err;
      }
    }
    // done with token so pop from stack
    PDXCP_CDCL_TOKEN_STACK_POP(stack);
  }
  // if no type, we ran out of tokens
  if (type_token.type == pdxcp_cdcl_token_type_error)  {
    pdxcp_cdcl_write_parse_err(errinfo, "Identifier missing required type");
    return pdxcp_cdcl_parser_status_parse_err;
  }
  // print out cv-qualifiers
  if (has_const && fprintf(out, " const") < 0)
    return pdxcp_cdcl_parser_status_out_err;
  if (has_volatile && fprintf(out, " volatile") < 0)
    return pdxcp_cdcl_parser_status_out_err;
  // print out sign qualifiers
  switch (type_token.type) {
    // type that can have sign qualifiers
    case pdxcp_cdcl_token_type_t_char:
    case pdxcp_cdcl_token_type_t_int:
    case pdxcp_cdcl_token_type_t_long:
      if (is_signed && fprintf(out, " signed") < 0)
        return pdxcp_cdcl_parser_status_out_err;
      if (is_unsigned && fprintf(out, " unsigned") < 0)
        return pdxcp_cdcl_parser_status_out_err;
      break;
    // unsupported type
    default:
      if (is_signed || is_unsigned) {
        char errmsg[PDXCP_CDCL_PARSER_ERROR_TEXT_LEN + 1];
        snprintf(
          errmsg,
          sizeof errmsg - 1,
          "Only char, int, or long can be signed or unsigned, received %s",
          // TODO: nicer way to indicate identifier type
          pdxcp_cdcl_token_type_string(type_token.type)
        );
        errmsg[sizeof errmsg - 1] = '\0';  // guarantee null termination
        // write result into error info
        pdxcp_cdcl_write_parse_err(errinfo, errmsg);
        return pdxcp_cdcl_parser_status_parse_err;
      }
      break;
  }
  // print out type
  switch (type_token.type) {
    // struct + enum
    case pdxcp_cdcl_token_type_struct:
      if (fprintf(out, " struct %s", type_token.text) < 0)
        return pdxcp_cdcl_parser_status_out_err;
      break;
    case pdxcp_cdcl_token_type_enum:
      if (fprintf(out, " enum %s", type_token.text) < 0)
        return pdxcp_cdcl_parser_status_out_err;
      break;
    // other types
    case pdxcp_cdcl_token_type_t_void:
      if (fprintf(out, " void") < 0) return pdxcp_cdcl_parser_status_out_err;
      break;
    case pdxcp_cdcl_token_type_t_char:
      if (fprintf(out, " char") < 0) return pdxcp_cdcl_parser_status_out_err;
      break;
    case pdxcp_cdcl_token_type_t_int:
      if (fprintf(out, " int") < 0) return pdxcp_cdcl_parser_status_out_err;
      break;
    case pdxcp_cdcl_token_type_t_long:
      if (fprintf(out, " long") < 0) return pdxcp_cdcl_parser_status_out_err;
      break;
    case pdxcp_cdcl_token_type_t_float:
      if (fprintf(out, " float") < 0) return pdxcp_cdcl_parser_status_out_err;
      break;
    case pdxcp_cdcl_token_type_t_double:
      if (fprintf(out, " double") < 0) return pdxcp_cdcl_parser_status_out_err;
      break;
    // unsupported type
    default: {
      char errmsg[PDXCP_CDCL_PARSER_ERROR_TEXT_LEN + 1];
      snprintf(
        errmsg,
        sizeof errmsg - 1,
        "Unknown identifier type %s",
        // TODO: nicer way to indicate identifier type
        pdxcp_cdcl_token_type_string(type_token.type)
      );
      errmsg[sizeof errmsg - 1] = '\0';  // guarantee null termination
      // write result into error info
      pdxcp_cdcl_write_parse_err(errinfo, errmsg);
      return pdxcp_cdcl_parser_status_parse_err;
    }
  }
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
  // statuses, current token, token holding identifier
  pdxcp_cdcl_lexer_status lexer_status;
  pdxcp_cdcl_parser_status parser_status;
  pdxcp_cdcl_token token, iden_token;
  // read tokens from lexer until error
  parser_status = stream_parse_to_iden(in, &lexer_status, &stack, &token);
  if (!PDXCP_CDCL_PARSER_OK(parser_status)) {
    pdxcp_cdcl_write_errinfo(errinfo, lexer_status, &token, parser_status, NULL);
    // no jumping to parse_finalize since nothing gets printed
    return parser_status;
  }
  // copy token contents to identifier token
  iden_token.type = token.type;
  strcpy(iden_token.text, token.text);
  // write identifer token
  if (fprintf(out, "%s:", token.text) < 0)
    return pdxcp_cdcl_parser_status_out_err;
  // read another token, handling lexer error as appropriate
  if (!PDXCP_CDCL_LEXER_OK(lexer_status = pdxcp_cdcl_get_token(in, &token))) {
    parser_status = pdxcp_cdcl_parser_status_lexer_err;
    pdxcp_cdcl_write_errinfo(errinfo, lexer_status, &token, parser_status, NULL);
    goto parse_finalize;
  }
  // number of ')' read so far. this is passed to stream_parse_ptrs
  unsigned int n_rparen = 0;
  // handle parentheses around pointers if we see ')'
  if (token.type == pdxcp_cdcl_token_type_rparen) {
    // read more tokens until next token is not ')'
    do {
      // pre-increment number of ')' read as next token may not be ')' and we
      // already read a ')' to even be in this block
      n_rparen++;
      if (!PDXCP_CDCL_LEXER_OK(lexer_status = pdxcp_cdcl_get_token(in, &token))) {
        parser_status = pdxcp_cdcl_parser_status_lexer_err;
        pdxcp_cdcl_write_errinfo(errinfo, lexer_status, &token, parser_status, NULL);
        goto parse_finalize;
      }
    }
    while (token.type == pdxcp_cdcl_token_type_rparen);
  }
  // TODO: handle array, function, etc.
  // if read left bracket, consume array components
  // if (token.type == pdxcp_cdcl_token_type_langle) {
    // consume array specifiers, i.e. by parsing
  // }
  // if read semicolon, end of declaration
  if (token.type == pdxcp_cdcl_token_type_semicolon) {
    // consume pointer tokens from stack + also balance any '(' that have been
    // read as part of the declarastion. parser_status written on error
    parser_status = stream_parse_ptrs(&stack, out, n_rparen, errinfo);
    if (!PDXCP_CDCL_PARSER_OK(parser_status))
      goto parse_finalize;
    // parse cv-qualified signed/unsigned qualified type
    parser_status = stream_parse_type(&stack, out, errinfo);
    goto parse_finalize;
  }
  // nothing we can do with this identifier, error
  else {
    parser_status = pdxcp_cdcl_parser_status_parse_err;
    // format error message
    char errmsg[PDXCP_CDCL_PARSER_ERROR_TEXT_LEN + 1];
    snprintf(
      errmsg,
      sizeof errmsg - 1,
      "Incomplete declaration for identifier %s",
      iden_token.text
    );
    errmsg[sizeof errmsg - 1] = '\0';  // guarantee null termination
    // write error info as usual
    pdxcp_cdcl_write_errinfo(errinfo, lexer_status, &token, parser_status, errmsg);
    goto parse_finalize;
  }
  // parse complete. just write final newline + returning parser status, as all
  // the parsing subroutines should have written appropriate error info
parse_finalize:
  if (fprintf(out, "\n") < 0)
    return pdxcp_cdcl_parser_status_out_err;
  return parser_status;
}
