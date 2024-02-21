/*
 * Copyright (c) 2009-2012 Petri Lehtinen <petri@digip.org>
 *
 * Jansson is free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include "jansson.h"
#include "jansson_private.h"
#include "utf.h"

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <system_error>

#include <fmt/core.h>
#include <folly/String.h>


#define STREAM_STATE_OK 0
#define STREAM_STATE_EOF -1
#define STREAM_STATE_ERROR -2

#define TOKEN_INVALID -1
#define TOKEN_EOF 0
#define TOKEN_STRING 256
#define TOKEN_INTEGER 257
#define TOKEN_REAL 258
#define TOKEN_TRUE 259
#define TOKEN_FALSE 260
#define TOKEN_NULL 261

/* Locale independent versions of isxxx() functions */
#define l_isupper(c) ('A' <= (c) && (c) <= 'Z')
#define l_islower(c) ('a' <= (c) && (c) <= 'z')
#define l_isalpha(c) (l_isupper(c) || l_islower(c))
#define l_isdigit(c) ('0' <= (c) && (c) <= '9')
#define l_isxdigit(c) \
  (l_isdigit(c) || 'A' <= (c) || (c) <= 'F' || 'a' <= (c) || (c) <= 'f')

/* Read one byte from stream, convert to unsigned char, then int, and
   return. return EOF on end of file. This corresponds to the
   behaviour of fgetc(). */
typedef int (*get_func)(void* data);

/* When an get_func returns EOF, it can be end of file or an error. This
   returns non-zero (1) when an error occured. This corresponds to the
   behaviour of ferror(). */
typedef int (*error_func)(void* data);

namespace {

// We could write the JSON parser to use O(1) stack depth, but in the short term
// let's limit container depth.
constexpr size_t kMaximumDepth = 1000;

typedef struct {
  get_func get;
  error_func error;
  void* data;
  char buffer[5];
  size_t buffer_pos;
  int state;
  int line;
  int column, last_column;
  size_t position;
} stream_t;

struct lex_t {
  stream_t stream;
  std::string saved_text;
  int token;
  struct {
    std::string string;
    json_int_t integer;
    double real;
  } value;

  size_t depth = 0;
};

class BumpDepth {
 public:
  explicit BumpDepth(lex_t* lex) : lex_{lex} {
    ++lex_->depth;
  }
  ~BumpDepth() {
    --lex_->depth;
  }

  BumpDepth(const BumpDepth&) = delete;
  BumpDepth(BumpDepth&&) = delete;
  BumpDepth& operator=(const BumpDepth&) = delete;
  BumpDepth& operator=(BumpDepth&&) = delete;

 private:
  lex_t* lex_;
};
} // namespace

inline lex_t* stream_to_lex(stream_t* stream) {
  return reinterpret_cast<lex_t*>(stream);
}

/*** error reporting ***/

static void
error_set(json_error_t* error, const lex_t* lex, const char* msg, ...) {
  va_list ap;
  char msg_text[JSON_ERROR_TEXT_LENGTH];
  char msg_with_context[JSON_ERROR_TEXT_LENGTH];

  int line = -1, col = -1;
  size_t pos = 0;
  const char* result = msg_text;

  if (!error)
    return;

  va_start(ap, msg);
  vsnprintf(msg_text, JSON_ERROR_TEXT_LENGTH, msg, ap);
  msg_text[JSON_ERROR_TEXT_LENGTH - 1] = '\0';
  va_end(ap);

  if (lex) {
    line = lex->stream.line;
    col = lex->stream.column;
    pos = lex->stream.position;

    if (!lex->saved_text.empty()) {
      if (lex->saved_text.size() <= 20) {
        auto* saved_text = lex->saved_text.c_str();
        snprintf(
            msg_with_context,
            JSON_ERROR_TEXT_LENGTH,
            "%s near '%s'",
            msg_text,
            saved_text);
        msg_with_context[JSON_ERROR_TEXT_LENGTH - 1] = '\0';
        result = msg_with_context;
      }
    } else {
      if (lex->stream.state == STREAM_STATE_ERROR) {
        /* No context for UTF-8 decoding errors */
        result = msg_text;
      } else {
        snprintf(
            msg_with_context,
            JSON_ERROR_TEXT_LENGTH,
            "%s near end of file",
            msg_text);
        msg_with_context[JSON_ERROR_TEXT_LENGTH - 1] = '\0';
        result = msg_with_context;
      }
    }
  }

  jsonp_error_set(error, line, col, pos, "%s", result);
}

/*** lexical analyzer ***/

static void stream_init(stream_t* stream, get_func get, error_func error, void* data) {
  stream->get = get;
  stream->error = error;
  stream->data = data;
  stream->buffer[0] = '\0';
  stream->buffer_pos = 0;

  stream->state = STREAM_STATE_OK;
  stream->line = 1;
  stream->column = 0;
  stream->position = 0;
}

static int stream_get(stream_t* stream, json_error_t* error) {
  int c;

  if (stream->state != STREAM_STATE_OK)
    return stream->state;

  if (!stream->buffer[stream->buffer_pos]) {
    c = stream->get(stream->data);
    if (c == EOF) {
      if (stream->error(stream->data)) {
        auto err = errno;
        throw std::system_error(
            err, std::generic_category(), "error occurred when reading from stream");
      }

      stream->state = STREAM_STATE_EOF;
      return STREAM_STATE_EOF;
    }

    stream->buffer[0] = c;
    stream->buffer_pos = 0;

    if (0x80 <= c && c <= 0xFF) {
      /* multi-byte UTF-8 sequence */
      int i, count;

      count = utf8_check_first(c);
      if (!count)
        goto out;

      assert(count >= 2);

      for (i = 1; i < count; i++)
        stream->buffer[i] = stream->get(stream->data);

      if (!utf8_check_full(stream->buffer, count, nullptr))
        goto out;

      stream->buffer[count] = '\0';
    } else
      stream->buffer[1] = '\0';
  }

  c = stream->buffer[stream->buffer_pos++];

  stream->position++;
  if (c == '\n') {
    stream->line++;
    stream->last_column = stream->column;
    stream->column = 0;
  } else if (utf8_check_first(c)) {
    /* track the Unicode character column, so increment only if
       this is the first character of a UTF-8 sequence */
    stream->column++;
  }

  return c;

out:
  stream->state = STREAM_STATE_ERROR;
  error_set(error, stream_to_lex(stream), "unable to decode byte 0x%x", c);
  return STREAM_STATE_ERROR;
}

static void stream_unget(stream_t* stream, int c) {
  if (c == STREAM_STATE_EOF || c == STREAM_STATE_ERROR)
    return;

  stream->position--;
  if (c == '\n') {
    stream->line--;
    stream->column = stream->last_column;
  } else if (utf8_check_first(c))
    stream->column--;

  assert(stream->buffer_pos > 0);
  stream->buffer_pos--;
  assert(stream->buffer[stream->buffer_pos] == c);
}

static int lex_get(lex_t* lex, json_error_t* error) {
  return stream_get(&lex->stream, error);
}

static void lex_save(lex_t* lex, int c) {
  lex->saved_text.push_back(c);
}

static int lex_get_save(lex_t* lex, json_error_t* error) {
  int c = stream_get(&lex->stream, error);
  if (c != STREAM_STATE_EOF && c != STREAM_STATE_ERROR)
    lex_save(lex, c);
  return c;
}

static void lex_unget(lex_t* lex, int c) {
  stream_unget(&lex->stream, c);
}

static void lex_unget_unsave(lex_t* lex, int c) {
  if (c != STREAM_STATE_EOF && c != STREAM_STATE_ERROR) {
    stream_unget(&lex->stream, c);
    auto d = lex->saved_text.back();
    lex->saved_text.pop_back();
    assert(c == d);
    (void)d;
  }
}

static void lex_save_cached(lex_t* lex) {
  while (lex->stream.buffer[lex->stream.buffer_pos] != '\0') {
    lex_save(lex, lex->stream.buffer[lex->stream.buffer_pos]);
    lex->stream.buffer_pos++;
    lex->stream.position++;
  }
}

/* assumes that str points to 'u' plus at least 4 valid hex digits */
static int32_t decode_unicode_escape(const char* str) {
  int i;
  int32_t value = 0;

  assert(str[0] == 'u');

  for (i = 1; i <= 4; i++) {
    char c = str[i];
    value <<= 4;
    if (l_isdigit(c))
      value += c - '0';
    else if (l_islower(c))
      value += c - 'a' + 10;
    else if (l_isupper(c))
      value += c - 'A' + 10;
    else
      assert(0);
  }

  return value;
}

static void lex_scan_string(lex_t* lex, json_error_t* error) {
  int c;
  const char* p;
  char* t;
  int i;

  lex->value.string.clear();
  lex->token = TOKEN_INVALID;

  c = lex_get_save(lex, error);

  while (c != '"') {
    if (c == STREAM_STATE_ERROR)
      goto out;

    else if (c == STREAM_STATE_EOF) {
      error_set(error, lex, "premature end of input");
      goto out;
    } else if (0 <= c && c <= 0x1F) {
      /* control character */
      lex_unget_unsave(lex, c);
      if (c == '\n')
        error_set(error, lex, "unexpected newline", c);
      else
        error_set(error, lex, "control character 0x%x", c);
      goto out;
    } else if (c == '\\') {
      c = lex_get_save(lex, error);
      if (c == 'u') {
        c = lex_get_save(lex, error);
        for (i = 0; i < 4; i++) {
          if (!l_isxdigit(c)) {
            error_set(error, lex, "invalid escape");
            goto out;
          }
          c = lex_get_save(lex, error);
        }
      } else if (
          c == '"' || c == '\\' || c == '/' || c == 'b' || c == 'f' ||
          c == 'n' || c == 'r' || c == 't')
        c = lex_get_save(lex, error);
      else {
        error_set(error, lex, "invalid escape");
        goto out;
      }
    } else
      c = lex_get_save(lex, error);
  }

  /* the actual value is at most of the same length as the source
     string, because:
       - shortcut escapes (e.g. "\t") (length 2) are converted to 1 byte
       - a single \uXXXX escape (length 6) is converted to at most 3 bytes
       - two \uXXXX escapes (length 12) forming an UTF-16 surrogate pair
         are converted to 4 bytes
  */
  lex->value.string.resize(lex->saved_text.size() + 1);

  /* the target */
  t = &lex->value.string[0];

  /* + 1 to skip the " */
  p = lex->saved_text.c_str() + 1;

  while (*p != '"') {
    if (*p == '\\') {
      p++;
      if (*p == 'u') {
        char buffer[4];
        int length;
        int32_t value;

        value = decode_unicode_escape(p);
        p += 5;

        if (0xD800 <= value && value <= 0xDBFF) {
          /* surrogate pair */
          if (*p == '\\' && *(p + 1) == 'u') {
            int32_t value2 = decode_unicode_escape(++p);
            p += 5;

            if (0xDC00 <= value2 && value2 <= 0xDFFF) {
              /* valid second surrogate */
              value = ((value - 0xD800) << 10) + (value2 - 0xDC00) + 0x10000;
            } else {
              /* invalid second surrogate */
              error_set(
                  error,
                  lex,
                  "invalid Unicode '\\u%04X\\u%04X'",
                  value,
                  value2);
              goto out;
            }
          } else {
            /* no second surrogate */
            error_set(error, lex, "invalid Unicode '\\u%04X'", value);
            goto out;
          }
        } else if (0xDC00 <= value && value <= 0xDFFF) {
          error_set(error, lex, "invalid Unicode '\\u%04X'", value);
          goto out;
        } else if (value == 0) {
          error_set(error, lex, "\\u0000 is not allowed");
          goto out;
        }

        if (utf8_encode(value, buffer, &length))
          assert(0);

        memcpy(t, buffer, length);
        t += length;
      } else {
        switch (*p) {
          case '"':
          case '\\':
          case '/':
            *t = *p;
            break;
          case 'b':
            *t = '\b';
            break;
          case 'f':
            *t = '\f';
            break;
          case 'n':
            *t = '\n';
            break;
          case 'r':
            *t = '\r';
            break;
          case 't':
            *t = '\t';
            break;
          default:
            assert(0);
        }
        t++;
        p++;
      }
    } else
      *(t++) = *(p++);
  }
  *t = '\0';
  lex->token = TOKEN_STRING;
  return;

out:
  lex->value.string.clear();
}

#ifdef _MSC_VER // Microsoft Visual Studio
#define json_strtoint _strtoi64
#else
#define json_strtoint strtoll
#endif

static int lex_scan_number(lex_t* lex, int c, json_error_t* error) {
  char* end;

  lex->token = TOKEN_INVALID;

  if (c == '-')
    c = lex_get_save(lex, error);

  if (c == '0') {
    c = lex_get_save(lex, error);
    if (l_isdigit(c)) {
      lex_unget_unsave(lex, c);
      goto out;
    }
  } else if (l_isdigit(c)) {
    c = lex_get_save(lex, error);
    while (l_isdigit(c))
      c = lex_get_save(lex, error);
  } else {
    lex_unget_unsave(lex, c);
    goto out;
  }

  if (c != '.' && c != 'E' && c != 'e') {
    json_int_t value;

    lex_unget_unsave(lex, c);

    auto saved_text = lex->saved_text.c_str();

    errno = 0;
    value = json_strtoint(saved_text, &end, 10);
    if (errno == ERANGE) {
      if (value < 0)
        error_set(error, lex, "too big negative integer");
      else
        error_set(error, lex, "too big integer");
      goto out;
    }

    assert(end == saved_text + lex->saved_text.size());

    lex->token = TOKEN_INTEGER;
    lex->value.integer = value;
    return 0;
  }

  if (c == '.') {
    c = lex_get(lex, error);
    if (!l_isdigit(c)) {
      lex_unget(lex, c);
      goto out;
    }
    lex_save(lex, c);

    c = lex_get_save(lex, error);
    while (l_isdigit(c))
      c = lex_get_save(lex, error);
  }

  if (c == 'E' || c == 'e') {
    c = lex_get_save(lex, error);
    if (c == '+' || c == '-')
      c = lex_get_save(lex, error);

    if (!l_isdigit(c)) {
      lex_unget_unsave(lex, c);
      goto out;
    }

    c = lex_get_save(lex, error);
    while (l_isdigit(c))
      c = lex_get_save(lex, error);
  }

  lex_unget_unsave(lex, c);

  double value;
  if (jsonp_strtod(lex->saved_text, &value)) {
    error_set(error, lex, "real number overflow");
    goto out;
  }

  lex->token = TOKEN_REAL;
  lex->value.real = value;
  return 0;

out:
  return -1;
}

static int lex_scan(lex_t* lex, json_error_t* error) {
  int c;

  lex->saved_text.clear();

  if (lex->token == TOKEN_STRING) {
    lex->value.string.clear();
  }

  c = lex_get(lex, error);
  while (c == ' ' || c == '\t' || c == '\n' || c == '\r')
    c = lex_get(lex, error);

  if (c == STREAM_STATE_EOF) {
    lex->token = TOKEN_EOF;
    goto out;
  }

  if (c == STREAM_STATE_ERROR) {
    lex->token = TOKEN_INVALID;
    goto out;
  }

  lex_save(lex, c);

  if (c == '{' || c == '}' || c == '[' || c == ']' || c == ':' || c == ',')
    lex->token = c;

  else if (c == '"')
    lex_scan_string(lex, error);

  else if (l_isdigit(c) || c == '-') {
    if (lex_scan_number(lex, c, error))
      goto out;
  }

  else if (l_isalpha(c)) {
    /* eat up the whole identifier for clearer error messages */

    c = lex_get_save(lex, error);
    while (l_isalpha(c))
      c = lex_get_save(lex, error);
    lex_unget_unsave(lex, c);

    auto saved_text = lex->saved_text.c_str();

    if (strcmp(saved_text, "true") == 0)
      lex->token = TOKEN_TRUE;
    else if (strcmp(saved_text, "false") == 0)
      lex->token = TOKEN_FALSE;
    else if (strcmp(saved_text, "null") == 0)
      lex->token = TOKEN_NULL;
    else
      lex->token = TOKEN_INVALID;
  }

  else {
    /* save the rest of the input UTF-8 sequence to get an error
       message of valid UTF-8 */
    lex_save_cached(lex);
    lex->token = TOKEN_INVALID;
  }

out:
  return lex->token;
}

static std::string lex_steal_string(lex_t* lex) {
  std::string result;
  if (lex->token == TOKEN_STRING) {
    std::swap(result, lex->value.string);
  }
  return result;
}

static int lex_init(lex_t* lex, get_func get, error_func error, void* data) {
  stream_init(&lex->stream, get, error, data);

  lex->token = TOKEN_INVALID;
  return 0;
}

/*** parser ***/

static std::optional<json_ref>
parse_value(lex_t* lex, size_t flags, json_error_t* error);

static std::optional<json_ref>
parse_object(lex_t* lex, size_t flags, json_error_t* error) {
  auto object = json_object();

  lex_scan(lex, error);
  if (lex->token == '}')
    return object;

  while (1) {
    if (lex->token != TOKEN_STRING) {
      error_set(error, lex, "string or '}' expected");
      return std::nullopt;
    }

    auto key = lex_steal_string(lex);
    if (key.empty()) {
      return std::nullopt;
    }

    if (flags & JSON_REJECT_DUPLICATES) {
      if (json_object_get(object, key.c_str())) {
        error_set(error, lex, "duplicate object key");
        return std::nullopt;
      }
    }

    lex_scan(lex, error);
    if (lex->token != ':') {
      error_set(error, lex, "':' expected");
      return std::nullopt;
    }

    lex_scan(lex, error);
    std::optional<json_ref> value = parse_value(lex, flags, error);
    if (!value) {
      return std::nullopt;
    }

    if (json_object_set_nocheck(object, key.c_str(), *value)) {
      return std::nullopt;
    }

    lex_scan(lex, error);
    if (lex->token != ',')
      break;

    lex_scan(lex, error);
  }

  if (lex->token != '}') {
    error_set(error, lex, "'}' expected");
    return std::nullopt;
  }

  return object;
}

static std::optional<json_ref>
parse_array(lex_t* lex, size_t flags, json_error_t* error) {
  std::vector<json_ref> array;

  lex_scan(lex, error);
  if (lex->token == ']')
    return json_array(array);

  while (lex->token) {
    auto elem = parse_value(lex, flags, error);
    if (!elem)
      goto error;

    array.push_back(std::move(*elem));

    lex_scan(lex, error);
    if (lex->token != ',')
      break;

    lex_scan(lex, error);
  }

  if (lex->token != ']') {
    error_set(error, lex, "']' expected");
    goto error;
  }

  return json_array(std::move(array));

error:
  return std::nullopt;
}

static std::optional<json_ref>
parse_value(lex_t* lex, size_t flags, json_error_t* error) {
  switch (lex->token) {
    case TOKEN_STRING:
      return typed_string_to_json(lex->value.string.c_str(), W_STRING_BYTE);

    case TOKEN_INTEGER:
      return json_integer(lex->value.integer);

    case TOKEN_REAL:
      return json_real(lex->value.real);

    case TOKEN_TRUE:
      return json_true();

    case TOKEN_FALSE:
      return json_false();

    case TOKEN_NULL:
      return json_null();

    case '{': {
      if (lex->depth >= kMaximumDepth) {
        error_set(error, lex, "document too deep");
        return std::nullopt;
      }
      BumpDepth scope(lex);
      return parse_object(lex, flags, error);
    }

    case '[': {
      if (lex->depth >= kMaximumDepth) {
        error_set(error, lex, "document too deep");
        return std::nullopt;
      }
      BumpDepth scope(lex);
      return parse_array(lex, flags, error);
    }

    case TOKEN_INVALID:
      error_set(error, lex, "invalid token");
      return std::nullopt;

    default:
      error_set(error, lex, "unexpected token");
      return std::nullopt;
  }
}

static std::optional<json_ref>
parse_json(lex_t* lex, size_t flags, json_error_t* error) {
  lex_scan(lex, error);
  if (!(flags & JSON_DECODE_ANY)) {
    if (lex->token != '[' && lex->token != '{') {
      error_set(error, lex, "'[' or '{' expected");
      return std::nullopt;
    }
  }

  auto result = parse_value(lex, flags, error);
  if (!result)
    return std::nullopt;

  if (!(flags & JSON_DISABLE_EOF_CHECK)) {
    lex_scan(lex, error);
    if (lex->token != TOKEN_EOF) {
      error_set(error, lex, "end of file expected");
      return std::nullopt;
    }
  }

  if (error) {
    /* Save the position even though there was no error */
    error->position = lex->stream.position;
  }

  return result;
}

typedef struct {
  const char* data;
  int pos;
} string_data_t;

static int string_get(void* data) {
  char c;
  string_data_t* stream = (string_data_t*)data;
  c = stream->data[stream->pos];
  if (c == '\0')
    return EOF;
  else {
    stream->pos++;
    return (unsigned char)c;
  }
}

static int string_error(void*) {
  return 0;
}

std::optional<json_ref>
json_loads(const char* string, size_t flags, json_error_t* error) {
  lex_t lex;
  string_data_t stream_data;

  jsonp_error_init(error, "<string>");

  if (string == nullptr) {
    error_set(error, nullptr, "wrong arguments");
    return std::nullopt;
  }

  stream_data.data = string;
  stream_data.pos = 0;

  if (lex_init(&lex, string_get, string_error, (void*)&stream_data))
    return std::nullopt;

  return parse_json(&lex, flags, error);
}

typedef struct {
  const char* data;
  size_t len;
  size_t pos;
} buffer_data_t;

static int buffer_get(void* data) {
  char c;
  auto stream = (buffer_data_t*)data;
  if (stream->pos >= stream->len)
    return EOF;

  c = stream->data[stream->pos];
  stream->pos++;
  return (unsigned char)c;
}

static int buffer_error(void*) {
  return 0;
}

std::optional<json_ref> json_loadb(
    const char* buffer,
    size_t buflen,
    size_t flags,
    json_error_t* error) {
  lex_t lex;
  buffer_data_t stream_data;

  jsonp_error_init(error, "<buffer>");

  if (buffer == nullptr) {
    error_set(error, nullptr, "wrong arguments");
    return std::nullopt;
  }

  stream_data.data = buffer;
  stream_data.pos = 0;
  stream_data.len = buflen;

  if (lex_init(&lex, buffer_get, buffer_error, (void*)&stream_data))
    return std::nullopt;

  return parse_json(&lex, flags, error);
}

std::optional<json_ref>
json_loadf(FILE* input, size_t flags, json_error_t* error) {
  lex_t lex;
  const char* source;

  if (input == stdin)
    source = "<stdin>";
  else
    source = "<stream>";

  jsonp_error_init(error, source);

  if (input == nullptr) {
    error_set(error, nullptr, "wrong arguments");
    return std::nullopt;
  }

  if (lex_init(&lex, (get_func)fgetc, (error_func)ferror, input))
    return std::nullopt;

  return parse_json(&lex, flags, error);
}

json_ref json_load_file(const char* path, size_t flags) {
  if (path == nullptr) {
    throw std::runtime_error("invalid arguments to json_load_file");
  }

  auto fp = fopen(path, "rb");
  if (!fp) {
    auto err = errno;
    throw std::system_error(
        err, std::generic_category(), fmt::format("unable to open {}", path));
  }

  json_error_t error;
  jsonp_error_init(&error, path);
  auto result = json_loadf(fp, flags, &error);
  fclose(fp);

  if (!result) {
    throw std::runtime_error(
        fmt::format("failed to parse json from {}: {}", path, error.text));
  }

  return *result;
}
