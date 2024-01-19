/**
 * @file cdcl_lexer.c
 * @author Derek Huang
 * @brief C source for C declaration lexer
 * @copyright MIT License
 */

#include "pdxcp/cdcl_lexer.h"

#include "pdxcp/common.h"

#include <ctype.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

// error message template for invalid character tokens
const char char_token_error[] = "Unknown character token 'X'";
// error message template for token that is too long
const char long_token_error[] = "Token too large: ...";

/**
 * Macro defining case statement returning the stringified value.
 *
 * @param value Enum, integral, etc. value
 */
#define ENUM_STRING_CASE(value) \
  case value: \
    return PDXCP_STRINGIFY(value)

const char *
pdxcp_cdcl_token_type_string(pdxcp_cdcl_token_type type)
{
  switch (type) {
    ENUM_STRING_CASE(pdxcp_cdcl_token_type_error);
    ENUM_STRING_CASE(pdxcp_cdcl_token_type_lparen);
    ENUM_STRING_CASE(pdxcp_cdcl_token_type_rparen);
    ENUM_STRING_CASE(pdxcp_cdcl_token_type_langle);
    ENUM_STRING_CASE(pdxcp_cdcl_token_type_rangle);
    ENUM_STRING_CASE(pdxcp_cdcl_token_type_comma);
    ENUM_STRING_CASE(pdxcp_cdcl_token_type_star);
    ENUM_STRING_CASE(pdxcp_cdcl_token_type_struct);
    ENUM_STRING_CASE(pdxcp_cdcl_token_type_enum);
    ENUM_STRING_CASE(pdxcp_cdcl_token_type_q_const);
    ENUM_STRING_CASE(pdxcp_cdcl_token_type_q_volatile);
    ENUM_STRING_CASE(pdxcp_cdcl_token_type_iden);
    default:
      return "(unknown)";
  }
}

const char *
pdxcp_cdcl_lexer_status_message(pdxcp_cdcl_lexer_status status)
{
  switch (status) {
    case pdxcp_cdcl_lexer_status_ok:
      return "Success";
    case pdxcp_cdcl_lexer_status_stream_null:
      return "Input stream is NULL";
    case pdxcp_cdcl_lexer_status_token_null:
      return "Token output address is NULL";
    case pdxcp_cdcl_lexer_status_ungetc_fail:
      return "Failed to put last read char back to stream";
    case pdxcp_cdcl_lexer_status_fgetc_eof:
      return "Read EOF from input stream";
    case pdxcp_cdcl_lexer_status_not_iden:
      return "Next token to read is not an identifier";
    case pdxcp_cdcl_lexer_status_bad_token:
      return "Unable to retrieve valid token, see token text for details";
    default:
      return "Unknown lexer status";
  }
}

/**
 * Get a valid C identifier into the text field of a token.
 *
 * @param in Input stream to read from
 * @param token Token to write identifier text to
 * @returns `pdxcp_cdcl_lexer_status` status code. If
 *  `pdxcp_cdcl_lexer_status_bad_token` is returned, the token type is
 *  `pdxcp_cdcl_token_type_error` and token text has error details
 */
static pdxcp_cdcl_lexer_status
pdxcp_cdcl_get_iden_text(FILE *in, pdxcp_cdcl_token *token)
{
  // pointer to where next read character should be written
  char *text_out = token->text;
  // skip whitespace. if EOF, return, otherwise first non-space character
  int c;
  while (isspace(c = fgetc(in)));
  if (c == EOF)
    return pdxcp_cdcl_lexer_status_fgetc_eof;
  // not identifier, in particular, also excludes digits. need to also put the
  // last character read back into the stream, otherwise it is lost
  if (!isalpha(c) && c != '_') {
    if (ungetc(c, in) == EOF)
      return pdxcp_cdcl_lexer_status_ungetc_fail;
    return pdxcp_cdcl_lexer_status_not_iden;
  }
  // write c into token text and advance text_out
  *text_out++ = (char) c;
  // read rest of [a-zA-Z0-9_] string text
  while (
    (isalnum(c = fgetc(in)) || c == '_') && c != EOF &&
    text_out < token->text + PDXCP_CDCL_MAX_TOKEN_LEN
  )
    *text_out++ = (char) c;
  // write null terminator. if EOF, this is the last identifier we can read
  *text_out = '\0';
  // put the last character read back into the stream if not EOF
  if (c != EOF && ungetc(c, in) == EOF)
    return pdxcp_cdcl_lexer_status_ungetc_fail;
  // if c is a valid identifier character, token is too large
  if (isalnum(c) || c == '_') {
    token->type = pdxcp_cdcl_token_type_error;
    strcpy(token->text, long_token_error);  // overwrites front part of token
    return pdxcp_cdcl_lexer_status_bad_token;
  }
  // otherwise we are ok
  return pdxcp_cdcl_lexer_status_ok;
}

pdxcp_cdcl_lexer_status
pdxcp_cdcl_get_token(FILE *in, pdxcp_cdcl_token *token)
{
  // must be non-NULL
  if (!in)
    return pdxcp_cdcl_lexer_status_stream_null;
  if (!token)
    return pdxcp_cdcl_lexer_status_token_null;
  // skip whitespace
  int c;
  while (isspace(c = fgetc(in)));
  // if EOF, return
  if (c == EOF)
    return pdxcp_cdcl_lexer_status_fgetc_eof;
  // if start of an identifier, parse rest of identifier
  if (isalpha(c) || c == '_') {
    // put the last character read back into the stream
    if (ungetc(c, in) == EOF)
      return pdxcp_cdcl_lexer_status_ungetc_fail;
    // read identifier text into the token
    pdxcp_cdcl_lexer_status status;
    if (PDXCP_CDCL_LEXER_ERROR(status = pdxcp_cdcl_get_iden_text(in, token)))
      return status;
    // const qualifier
    if (!strcmp(token->text, "const"))
      token->type = pdxcp_cdcl_token_type_q_const;
    // volatile qualifier
    else if (!strcmp(token->text, "volatile"))
      token->type = pdxcp_cdcl_token_type_q_volatile;
    // struct (requires another string read)
    else if (!strcmp(token->text, "struct")) {
      if (PDXCP_CDCL_LEXER_ERROR(status = pdxcp_cdcl_get_iden_text(in, token)))
        return status;
      token->type = pdxcp_cdcl_token_type_struct;
    }
    // enum (requires another string read)
    else if (!strcmp(token->text, "enum")) {
      if (PDXCP_CDCL_LEXER_ERROR(status = pdxcp_cdcl_get_iden_text(in, token)))
        return status;
      token->type = pdxcp_cdcl_token_type_enum;
    }
    // identifier
    else
      token->type = pdxcp_cdcl_token_type_iden;
    return pdxcp_cdcl_lexer_status_ok;
  }
  // check single-character tokens. the token text is '\0' in the case
  token->text[0] = '\0';
  switch (c) {
    case '(':
      token->type = pdxcp_cdcl_token_type_lparen;
      break;
    case ')':
      token->type = pdxcp_cdcl_token_type_rparen;
      break;
    case '[':
      token->type = pdxcp_cdcl_token_type_langle;
      break;
    case ']':
      token->type = pdxcp_cdcl_token_type_rangle;
      break;
    case ',':
      token->type = pdxcp_cdcl_token_type_comma;
      break;
    case '*':
      token->type = pdxcp_cdcl_token_type_star;
      break;
    // unknown token. copy error template and write the character to text
    default:
      token->type = pdxcp_cdcl_token_type_error;
      strcpy(token->text, char_token_error);
      token->text[sizeof char_token_error - 3] = (char) c;
      token->text[sizeof char_token_error - 1] = '\0';  // terminate string
      return pdxcp_cdcl_lexer_status_bad_token;
  }
  return pdxcp_cdcl_lexer_status_ok;
}
