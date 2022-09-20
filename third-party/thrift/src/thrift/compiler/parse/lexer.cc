/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <thrift/compiler/parse/lexer.h>

#include <assert.h>
#include <errno.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

#include <algorithm>
#include <unordered_map>
#include <utility>

#include <thrift/compiler/diagnostic.h>

namespace apache {
namespace thrift {
namespace compiler {

namespace {

bool is_whitespace(char c) {
  return c == ' ' || c == '\t' || c == '\r' || c == '\n';
}

bool is_letter(char c) {
  return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

bool is_bin_digit(char c) {
  return c == '0' || c == '1';
}
bool is_oct_digit(char c) {
  return c >= '0' && c <= '7';
}
bool is_dec_digit(char c) {
  return c >= '0' && c <= '9';
}
bool is_hex_digit(char c) {
  return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') ||
      (c >= 'A' && c <= 'F');
}

bool is_identifier_char(char c) {
  return is_letter(c) || is_dec_digit(c) || c == '_' || c == '.';
}

// Lexes a decimal constant of the form [0-9]+. Returns a pointer past the end
// if the constant has been lexed; `none` otherwise.
const char* lex_dec_literal(const char* p, const char* none = nullptr) {
  if (!is_dec_digit(*p)) {
    return none;
  }
  do {
    ++p;
  } while (is_dec_digit(*p));
  return p;
}

// Lexes a float exponent of the form [eE][+-]?[0-9]+. Returns a pointer past
// the end if the exponent has been lexed; `none` otherwise.
const char* lex_float_exponent(const char* p, const char* none = nullptr) {
  if (*p != 'e' && *p != 'E') {
    return none;
  }
  ++p; // Consume 'e' or 'E'.
  if (*p == '+' || *p == '-') {
    ++p; // Consume the sign.
  }
  return lex_dec_literal(p);
}

// Lexes a float constant in the form [0-9]+ followed by an optional exponent.
// Returns a pointer past the end if the constant has been lexed; nullptr
// otherwise.
const char* lex_float_literal(const char* p) {
  p = lex_dec_literal(p);
  return p ? lex_float_exponent(p, p) : nullptr;
}

const std::unordered_map<std::string, tok> keywords = {
    {"false", tok::bool_literal},
    {"true", tok::bool_literal},
    {"include", tok::kw_include},
    {"cpp_include", tok::kw_cpp_include},
    {"hs_include", tok::kw_hs_include},
    {"package", tok::kw_package},
    {"namespace", tok::kw_namespace},
    {"void", tok::kw_void},
    {"bool", tok::kw_bool},
    {"byte", tok::kw_byte},
    {"i16", tok::kw_i16},
    {"i32", tok::kw_i32},
    {"i64", tok::kw_i64},
    {"double", tok::kw_double},
    {"float", tok::kw_float},
    {"string", tok::kw_string},
    {"binary", tok::kw_binary},
    {"map", tok::kw_map},
    {"list", tok::kw_list},
    {"set", tok::kw_set},
    {"sink", tok::kw_sink},
    {"stream", tok::kw_stream},
    {"interaction", tok::kw_interaction},
    {"performs", tok::kw_performs},
    {"oneway", tok::kw_oneway},
    {"idempotent", tok::kw_idempotent},
    {"readonly", tok::kw_readonly},
    {"safe", tok::kw_safe},
    {"transient", tok::kw_transient},
    {"stateful", tok::kw_stateful},
    {"permanent", tok::kw_permanent},
    {"server", tok::kw_server},
    {"client", tok::kw_client},
    {"typedef", tok::kw_typedef},
    {"struct", tok::kw_struct},
    {"union", tok::kw_union},
    {"exception", tok::kw_exception},
    {"extends", tok::kw_extends},
    {"throws", tok::kw_throws},
    {"service", tok::kw_service},
    {"enum", tok::kw_enum},
    {"const", tok::kw_const},
    {"required", tok::kw_required},
    {"optional", tok::kw_optional},
};

} // namespace

lexer::lexer(source src, lex_handler& handler, diagnostics_engine& diags)
    : source_(src.text), start_(src.start), handler_(&handler), diags_(&diags) {
  ptr_ = source_.data();
  token_start_ = ptr_;
}

token lexer::make_int_literal(int offset, int base) {
  fmt::string_view text = token_text();
  errno = 0;
  char* end = nullptr;
  uint64_t val = strtoull(text.data() + offset, &end, base);
  if (end != text.data() + text.size()) {
    return report_error("internal error when lexing an int literal");
  }
  return errno != ERANGE
      ? token::make_int_literal(token_source_range(), val)
      : report_error("integer constant {} is too large", text);
}

token lexer::make_float_literal() {
  fmt::string_view text = token_text();
  errno = 0;
  char* end = nullptr;
  double val = strtod(text.data(), &end);
  if (errno == ERANGE) {
    if (val == 0) {
      return report_error(
          "magnitude of floating-point constant {} is too small", text);
    } else if (val == HUGE_VAL || val == -HUGE_VAL) {
      return report_error("floating-point constant {} is out of range", text);
    }
    // Allow subnormals.
  }
  if (end != text.data() + text.size()) {
    return report_error("internal error when lexing a float literal");
  }
  return token::make_float_literal(token_source_range(), val);
}

template <typename... T>
token lexer::report_error(fmt::format_string<T...> msg, T&&... args) {
  diags_->report(
      location(token_start_),
      diagnostic_level::error,
      msg,
      std::forward<T>(args)...);
  return token(tok::error, token_source_range());
}

token lexer::unexpected_token() {
  return report_error("unexpected token in input: {}", token_text());
}

void lexer::skip_line_comment() {
  ptr_ = std::find(ptr_, end(), '\n');
}

bool lexer::lex_doc_comment() {
  assert(strncmp(ptr_, "///", 3) == 0);
  start_token();
  bool is_inline = ptr_[3] == '<';
  const char* prefix = ptr_;
  size_t prefix_size = is_inline ? 4 : 3;
  do {
    if (!is_inline && ptr_[3] == '/') {
      break;
    }
    ptr_ += prefix_size; // Skip "///" or "///<".
    skip_line_comment();
    while (is_whitespace(*ptr_)) {
      ++ptr_;
    }
  } while (strncmp(ptr_, prefix, prefix_size) == 0);
  if (!is_inline) {
    handler_->on_doc_comment(token_text(), location(ptr_));
  }
  return is_inline;
}

lexer::comment_lex_result lexer::lex_block_comment() {
  assert(strncmp(ptr_, "/*", 2) == 0);
  const char* p = ptr_ + 2; // Skip "/*".
  start_token();
  do {
    p = std::find(p, end(), '*');
    if (!*p) { // EOF while lexing a block comment.
      return comment_lex_result::unterminated;
    }
    ++p; // Skip '*'.
  } while (*p != '/');
  ptr_ = p + 1; // Skip '/'.
  if (token_start_[2] == '*') {
    if (token_start_[3] == '<') {
      return comment_lex_result::doc_comment;
    }
    // Ignore comments containing only '*'s.
    auto non_star = std::find_if(
        token_start_ + 2, ptr_ - 1, [](char c) { return c != '*'; });
    if (non_star != ptr_ - 1) {
      handler_->on_doc_comment(token_text(), location(ptr_));
    }
  }
  return comment_lex_result::skipped;
}

lexer::comment_lex_result lexer::lex_whitespace_or_comment() {
  for (;;) {
    switch (*ptr_) {
      case '\n':
      case ' ':
      case '\t':
      case '\r':
        ++ptr_;
        break;
      case '/':
        if (ptr_[1] == '/') {
          if (ptr_[2] == '/' && ptr_[3] != '/') {
            if (lex_doc_comment()) {
              return comment_lex_result::doc_comment;
            }
            continue;
          }
          ptr_ += 2; // Skip "//".
          skip_line_comment();
          break;
        }
        if (ptr_[1] == '*') {
          comment_lex_result res = lex_block_comment();
          if (res != comment_lex_result::skipped) {
            return res;
          }
          continue;
        }
        return comment_lex_result::skipped;
      case '#':
        ++ptr_;
        skip_line_comment();
        break;
      default:
        return comment_lex_result::skipped;
    }
  }
}

token lexer::get_next_token() {
  if (lex_whitespace_or_comment() == comment_lex_result::doc_comment) {
    return token::make_inline_doc(token_source_range(), token_text());
  }

  start_token();

  char c = *ptr_++;
  if (is_letter(c) || c == '_') {
    // Lex an identifier or a keyword.
    while (is_identifier_char(*ptr_)) {
      ++ptr_;
    }
    auto text = token_text();
    auto it = keywords.find(std::string(text.data(), text.size()));
    if (it != keywords.end()) {
      return it->second == tok::bool_literal
          ? token::make_bool_literal(token_source_range(), it->first == "true")
          : token(it->second, token_source_range());
    }
    return token::make_identifier(token_source_range(), text);
  } else if (c == '.') {
    if (const char* p = lex_float_literal(ptr_)) {
      ptr_ = p;
      return make_float_literal();
    }
  } else if (is_dec_digit(c)) {
    if (c == '0') {
      switch (*ptr_) {
        case 'x':
        case 'X':
          // Lex a hexadecimal constant.
          if (!is_hex_digit(ptr_[1])) {
            return unexpected_token();
          }
          ptr_ += 2;
          while (is_hex_digit(*ptr_)) {
            ++ptr_;
          }
          return make_int_literal(2, 16);
        case 'b':
        case 'B':
          // Lex a binary constant.
          if (!is_bin_digit(ptr_[1])) {
            return unexpected_token();
          }
          ptr_ += 2;
          while (is_bin_digit(*ptr_)) {
            ++ptr_;
          }
          return make_int_literal(2, 2);
      }
    }
    // Lex a decimal, octal or floating-point constant.
    ptr_ = lex_dec_literal(ptr_, ptr_);
    switch (*ptr_) {
      case '.':
        if (const char* p = lex_float_literal(ptr_ + 1)) {
          ptr_ = p;
          return make_float_literal();
        }
        break;
      case 'e':
      case 'E':
        if (const char* p = lex_float_exponent(ptr_)) {
          ptr_ = p;
          return make_float_literal();
        }
        break;
    }
    if (c != '0') {
      // Lex a decimal constant.
      return make_int_literal(0, 10);
    }
    // Lex an octal constant.
    const char* p = std::find_if(
        token_start_, ptr_, [](char c) { return !is_oct_digit(c); });
    if (p != ptr_) {
      return unexpected_token();
    }
    return ptr_ - token_start_ != 1
        ? make_int_literal(1, 8)
        : token::make_int_literal(token_source_range(), 0);
  } else if (c == '"' || c == '\'') {
    // Lex a string literal.
    const char* p = std::find(ptr_, end(), c);
    if (*p) {
      ptr_ = p + 1;
      const char* begin = token_start_ + 1;
      return token::make_string_literal(
          token_source_range(), {begin, static_cast<size_t>(p - begin)});
    }
  } else if (!c && ptr_ > end()) {
    --ptr_; // Put '\0' back in case get_next_token() is called again.
    return token(tok::eof, token_source_range());
  }

  // Lex operators and punctuators.
  auto kind = detail::to_tok(c);
  return kind != tok::error ? token(kind, token_source_range())
                            : unexpected_token();
}

} // namespace compiler
} // namespace thrift
} // namespace apache
