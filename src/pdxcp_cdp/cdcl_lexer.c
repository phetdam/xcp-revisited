/**
 * @file cdcl_lexer.c
 * @author Derek Huang
 * @brief C source for C declaration lexer
 * @copyright MIT License
 */

#include "pdxcp/cdcl_lexer.h"

#include <ctype.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

// error message template for invalid character tokens
const char char_token_error[] = "Unknown character token 'X'";
// error message template for token that is too long
const char long_token_error[] = "Token too large: ...";

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
  // get first character
  int c = fgetc(in);
  // if EOF, return
  if (c == EOF)
    return pdxcp_cdcl_lexer_status_fgetc_eof;
  // not identifier, in particular, also excludes digits. need to also put the
  // last character read back into the stream, otherwise it is lost
  if (!isalpha(c) && c != '_') {
    if (ungetc(c, in) == EOF)
      return pdxcp_cdcl_lexer_status_ungetc_fail;
    return pdxcp_cdcl_lexer_status_not_iden;
  }
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
      return pdxcp_cdcl_lexer_status_bad_token;
  }
  return pdxcp_cdcl_lexer_status_ok;
}
