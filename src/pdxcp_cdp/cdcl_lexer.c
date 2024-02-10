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

#include "pdxcp/cdcl_common.h"
#include "pdxcp/common.h"
#include "pdxcp/string.h"

// error message template for invalid character tokens
static const char char_token_error[] = "Unknown character token 'X'";
// error message template for token that is too long
static const char long_token_error[] = "Token too large: ...";

const char *
pdxcp_cdcl_token_type_string(pdxcp_cdcl_token_type type)
{
  switch (type) {
    // PDXCP_PDXCP_STRING_CASE
    PDXCP_STRING_CASE(pdxcp_cdcl_token_type_error);
    PDXCP_STRING_CASE(pdxcp_cdcl_token_type_lparen);
    PDXCP_STRING_CASE(pdxcp_cdcl_token_type_rparen);
    PDXCP_STRING_CASE(pdxcp_cdcl_token_type_langle);
    PDXCP_STRING_CASE(pdxcp_cdcl_token_type_rangle);
    PDXCP_STRING_CASE(pdxcp_cdcl_token_type_comma);
    PDXCP_STRING_CASE(pdxcp_cdcl_token_type_slash);
    PDXCP_STRING_CASE(pdxcp_cdcl_token_type_star);
    PDXCP_STRING_CASE(pdxcp_cdcl_token_type_semicolon);
    PDXCP_STRING_CASE(pdxcp_cdcl_token_type_struct);
    PDXCP_STRING_CASE(pdxcp_cdcl_token_type_enum);
    PDXCP_STRING_CASE(pdxcp_cdcl_token_type_q_const);
    PDXCP_STRING_CASE(pdxcp_cdcl_token_type_q_volatile);
    PDXCP_STRING_CASE(pdxcp_cdcl_token_type_q_signed);
    PDXCP_STRING_CASE(pdxcp_cdcl_token_type_q_unsigned);
    PDXCP_STRING_CASE(pdxcp_cdcl_token_type_t_void);
    PDXCP_STRING_CASE(pdxcp_cdcl_token_type_t_char);
    PDXCP_STRING_CASE(pdxcp_cdcl_token_type_t_int);
    PDXCP_STRING_CASE(pdxcp_cdcl_token_type_t_long);
    PDXCP_STRING_CASE(pdxcp_cdcl_token_type_t_float);
    PDXCP_STRING_CASE(pdxcp_cdcl_token_type_t_double);
    PDXCP_STRING_CASE(pdxcp_cdcl_token_type_num);
    PDXCP_STRING_CASE(pdxcp_cdcl_token_type_iden);
    default:
      return "(unknown)";
  }
}

const char *
pdxcp_cdcl_lexer_status_string(pdxcp_cdcl_lexer_status status)
{
  switch (status) {
    PDXCP_STRING_CASE(pdxcp_cdcl_lexer_status_ok);
    PDXCP_STRING_CASE(pdxcp_cdcl_lexer_status_stream_null);
    PDXCP_STRING_CASE(pdxcp_cdcl_lexer_status_token_null);
    PDXCP_STRING_CASE(pdxcp_cdcl_lexer_status_ungetc_fail);
    PDXCP_STRING_CASE(pdxcp_cdcl_lexer_status_fgetc_eof);
    PDXCP_STRING_CASE(pdxcp_cdcl_lexer_status_not_num);
    PDXCP_STRING_CASE(pdxcp_cdcl_lexer_status_not_iden);
    PDXCP_STRING_CASE(pdxcp_cdcl_lexer_status_bad_token);
    default:
      return "(unknown)";
  };
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
  // if c is a valid identifier character, token is too large. we overwrite the
  // front part of the token with the long_token_error message
  if (isalnum(c) || c == '_') {
    token->type = pdxcp_cdcl_token_type_error;
    memcpy(token->text, long_token_error, sizeof long_token_error - 1);
    return pdxcp_cdcl_lexer_status_bad_token;
  }
  // otherwise we are ok
  return pdxcp_cdcl_lexer_status_ok;
}

/**
 * Get an integral number into the text field of a token.
 *
 * @param in Input stream to read from
 * @param token Token to write identifier text to
 * @returns `pdxcp_cdcl_lexer_status` status code. If
 *  `pdxcp_cdcl_lexer_status_bad_token` is returned, the token type is
 *  `pdxcp_cdcl_token_type_error` and token text has error details
 */
static pdxcp_cdcl_lexer_status
pdxcp_cdcl_get_num_text(FILE *in, pdxcp_cdcl_token *token)
{
  // pointer to where next read character should be written
  char *text_out = token->text;
  // skip whitespace. return on EOF, otherwise first non-space character
  int c;
  while (isspace(c = fgetc(in)));
  if (c == EOF)
    return pdxcp_cdcl_lexer_status_fgetc_eof;
  // not a number. put char back so we don't lose it
  if (!isdigit(c)) {
    if (ungetc(c, in) == EOF)
      return pdxcp_cdcl_lexer_status_ungetc_fail;
    return pdxcp_cdcl_lexer_status_not_num;
  }
  // write c into token text and advance text_out
  *text_out++ = (char) c;
  // if c is '0', then we can also read 'x' or 'X' for hex
  if (c == '0' && (c = fgetc(in)) != 'x' && c != 'X') {
    // might be EOF
    if (c == EOF)
      return pdxcp_cdcl_lexer_status_fgetc_eof;
    // otherwise, malformed number. this is reported as a parsing error as we
    // cannot portably guarantee ungetc() of more than 1 char
    token->type = pdxcp_cdcl_token_type_error;
    strcpy(token->text, "Malformed token read when attempting to parse number");
    return pdxcp_cdcl_lexer_status_bad_token;
  }
  // read rest of [0-9] string text
  while (
    isdigit(c = fgetc(in)) && c != EOF &&
    text_out < token->text + PDXCP_CDCL_MAX_TOKEN_LEN
  )
    *text_out++ = (char) c;
  // write null terminator. if EOF, this is the last number we can read
  *text_out = '\0';
  // put last char read back in stream if not EOF
  if (c != EOF && ungetc(c, in) == EOF)
    return pdxcp_cdcl_lexer_status_ungetc_fail;
  // if c is a valid digit, token is too large, so overwrite front with message
  if (isdigit(c)) {
    token->type = pdxcp_cdcl_token_type_error;
    memcpy(token->text, long_token_error, sizeof long_token_error - 1);
    return pdxcp_cdcl_lexer_status_bad_token;
  }
  // success
  return pdxcp_cdcl_lexer_status_ok;
}

/**
 * Initialize a token from a single character.
 *
 * The token type is set and token text has a single null terminator written at
 * the first position to efficiently mimic an empty string.
 *
 * @param token Token to initialize
 * @param c Character token, e.g. `[`, `;`
 * @returns `pdxcp_cdcl_lexer_status` status code. If
 *  `pdxcp_cdcl_lexer_status_bad_token` is returned, the token type is
 *  `pdxcp_cdcl_token_type_error` and token text has error details
 */
static pdxcp_cdcl_lexer_status
pdxcp_cdcl_set_char_token(pdxcp_cdcl_token *token, char c)
{
  // token text is '\0' for single-character tokens
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
    case '/':
      token->type = pdxcp_cdcl_token_type_slash;
      break;
    case '*':
      token->type = pdxcp_cdcl_token_type_star;
      break;
    case ';':
      token->type = pdxcp_cdcl_token_type_semicolon;
      break;
    // unknown token. copy error template and write the character to text
    default:
      token->type = pdxcp_cdcl_token_type_error;
      strcpy(token->text, char_token_error);  // null-terminated
      token->text[sizeof char_token_error - 3] = c;
      return pdxcp_cdcl_lexer_status_bad_token;
  }
  return pdxcp_cdcl_lexer_status_ok;
}

/**
 * Get a token from an identifier string from the specified input stream.
 *
 * This routine assumes that the next token to be read is a valid C identifier
 * and will return a status code indicating whether lexing succeeded or not.
 *
 * @param in Input stream to read from
 * @param token Token to write to
 * @returns `pdxcp_cdcl_lexer_status` status code. If
 *  `pdxcp_cdcl_lexer_status_bad_token` is returned, the token type is
 *  `pdxcp_cdcl_token_type_error` and token text has error details
 */
static pdxcp_cdcl_lexer_status
pdxcp_cdcl_get_iden_token(FILE *in, pdxcp_cdcl_token *token)
{
  // read identifier text into the token
  pdxcp_cdcl_lexer_status status;
  if (!PDXCP_CDCL_LEXER_OK(status = pdxcp_cdcl_get_iden_text(in, token)))
    return status;
  // const qualifier
  if (pdxcp_streq(token->text, "const")) {
    token->type = pdxcp_cdcl_token_type_q_const;
    token->text[0] = '\0';
  }
  // volatile qualifier
  else if (pdxcp_streq(token->text, "volatile")) {
    token->type = pdxcp_cdcl_token_type_q_volatile;
    token->text[0] = '\0';
  }
  // signed type qualifier
  else if (pdxcp_streq(token->text, "signed")) {
    token->type = pdxcp_cdcl_token_type_q_signed;
    token->text[0] = '\0';
  }
  // unsigned type qualifier
  else if (pdxcp_streq(token->text, "unsigned")) {
    token->type = pdxcp_cdcl_token_type_q_unsigned;
    token->text[0] = '\0';
  }
  // struct (requires another string read)
  else if (pdxcp_streq(token->text, "struct")) {
    if (!PDXCP_CDCL_LEXER_OK(status = pdxcp_cdcl_get_iden_text(in, token)))
      return status;
    token->type = pdxcp_cdcl_token_type_struct;
  }
  // enum (requires another string read)
  else if (pdxcp_streq(token->text, "enum")) {
    if (!PDXCP_CDCL_LEXER_OK(status = pdxcp_cdcl_get_iden_text(in, token)))
      return status;
    token->type = pdxcp_cdcl_token_type_enum;
  }
  // void
  else if (pdxcp_streq(token->text, "void")) {
    token->type = pdxcp_cdcl_token_type_t_void;
    token->text[0] = '\0';
  }
  // char
  else if (pdxcp_streq(token->text, "char")) {
    token->type = pdxcp_cdcl_token_type_t_char;
    token->text[0] = '\0';
  }
  // signed int
  else if (pdxcp_streq(token->text, "int")) {
    token->type = pdxcp_cdcl_token_type_t_int;
    token->text[0] = '\0';
  }
  // signed long
  else if (pdxcp_streq(token->text, "long")) {
    token->type = pdxcp_cdcl_token_type_t_long;
    token->text[0] = '\0';
  }
  // float
  else if (pdxcp_streq(token->text, "float")) {
    token->type = pdxcp_cdcl_token_type_t_float;
    token->text[0] = '\0';
  }
  // double
  else if (pdxcp_streq(token->text, "double")) {
    token->type = pdxcp_cdcl_token_type_t_double;
    token->text[0] = '\0';
  }
  // identifier
  else
    token->type = pdxcp_cdcl_token_type_iden;
  return pdxcp_cdcl_lexer_status_ok;
}

/**
 * Skip input until the end of a C block comment is consumed.
 *
 * This routine should be called only if the beginning of a C block comment has
 * already been consumed from the input stream.
 *
 * @param in Input stream to read from
 * @returns 'pdxcp_cdcl_lexer_status` status code
 */
static pdxcp_cdcl_lexer_status
pdxcp_cdcl_skip_rem_c_comment(FILE *in)
{
  // next char, return on error
  int c;
  // loop to skip comment block. either terminate the comment block or hit EOF
  do {
    // skip until we reach EOF or '*'
    while ((c = fgetc(in)) != EOF && c != '*');
    // EOF is error
    if (c == EOF)
      return pdxcp_cdcl_lexer_status_fgetc_eof;
    // get another char, error on EOF
    if ((c = fgetc(in)) == EOF)
      return pdxcp_cdcl_lexer_status_fgetc_eof;
  }
  while (c != '/');
  return pdxcp_cdcl_lexer_status_ok;
}

/**
 * Get a token from an integral number from the specified input stream.
 *
 * This routine assumes that the next token to be read is an integral number
 * and will return a status code indicating whether lexing succeeded or not.
 *
 * @param in Input stream to read from
 * @param token Token to write to
 * @returns `pdxcp_cdcl_lexer_status` status code. If
 *  `pdxcp_cdcl_lexer_status_bad_token` is returned, the token type is
 *  `pdxcp_cdcl_token_type_error` and token text has error details
 */
static pdxcp_cdcl_lexer_status
pdxcp_cdcl_get_num_token(FILE *in, pdxcp_cdcl_token *token)
{
  // read numeric text into the token
  pdxcp_cdcl_lexer_status status;
  if (!PDXCP_CDCL_LEXER_OK(status = pdxcp_cdcl_get_num_text(in, token)))
    return status;
  // set type and return
  token->type = pdxcp_cdcl_token_type_num;
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
  // handle slash. possibly skip C block comment, C++ line comment
  if (c == '/') {
    // read another char and switch on its value
    c = fgetc(in);
    switch (c) {
      // C block comment. skip until EOF or end of block comment
      case '*': {
        pdxcp_cdcl_lexer_status status;
        if (!PDXCP_CDCL_LEXER_OK(status = pdxcp_cdcl_skip_rem_c_comment(in)))
          return status;
        break;
      }
      // C++ line comment. just skip rest of the line or until EOF
      case '/':
        while ((c = fgetc(in)) != EOF && c != '\n');
        if (c == EOF)
          return pdxcp_cdcl_lexer_status_fgetc_eof;
        break;
      // some other char. put it back into the stream if not EOF, token is '/'.
      // note that if c is EOF, this is an edge case where '/' is the last
      // token in the stream. this is obviously a parse error but for a correct
      // lexer we still accept this and return without lexer error
      default:
        if (c != EOF && ungetc(c, in) == EOF)
          return pdxcp_cdcl_lexer_status_ungetc_fail;
        return pdxcp_cdcl_set_char_token(token, '/');
    }
    // finished skipping comments, so skip any additional whitespace
    while (isspace(c = fgetc(in)));
    if (c == EOF)
      return pdxcp_cdcl_lexer_status_fgetc_eof;
  }
  // if start of an identifier, parse rest of identifier
  if (isalpha(c) || c == '_') {
    // put the last character read back into the stream
    if (ungetc(c, in) == EOF)
      return pdxcp_cdcl_lexer_status_ungetc_fail;
    // read token from identifier string
    return pdxcp_cdcl_get_iden_token(in, token);
  }
  // if digit, parse rest of digit (identifier cannot start with digit)
  if (isdigit(c)) {
    // put last char read back into stream
    if (ungetc(c, in) == EOF)
      return pdxcp_cdcl_lexer_status_ungetc_fail;
    // read token from numeric string
    return pdxcp_cdcl_get_num_token(in, token);
  }
  // else single-character token. the token text is '\0' in this case
  return pdxcp_cdcl_set_char_token(token, (char) c);
}
