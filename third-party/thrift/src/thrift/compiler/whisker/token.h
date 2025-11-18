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

#pragma once

#include <thrift/compiler/whisker/source_location.h>

#include <cassert>
#include <cstdint>
#include <ostream>
#include <string>
#include <string_view>
#include <unordered_map>
#include <variant>

#include <fmt/core.h>

namespace whisker {

enum class tok : unsigned {
  eof = 0,
  error,

  i64_literal,
  string_literal,

  identifier,
  path_component,

  // non-whitespace (except in comments) raw text
  text,
  // one or more repititions of " ", "\t", or "\v"
  whitespace,
  // "\r\n", "\n", or "\r"
  newline,

  // clang-format off
  open,      // "{{"
  close,     // "}}"

  // Punctuation:
  dot,       // "."
  l_paren,   // "("
  r_paren,   // ")"
  pound,     // "#"
  bang,      // "!"
  caret,     // "^"
  slash,     // "/"
  pipe,      // "|"
  gt,        // ">"
  eq,        // "="
  star,      // "*"
  dollar,    // "$"
  colon,     // ":"
  // clang-format on

  // Literals:
  kw_true,
  kw_false,
  kw_null,

  // Blocks:
  kw_if,
  kw_unless,
  kw_else,
  kw_each,
  kw_as,
  kw_partial,
  kw_captures,

  // Special Forms:
  kw_let,
  kw_and,
  kw_or,
  kw_not,
  kw_pragma,

  // Reserved Words (may be used in the future):
  kw_with,
  kw_this,
  kw_define,
  kw_for,
  kw_do,
  kw_import,
  kw_export,
  kw_from,
};

std::string_view to_string(tok kind);
std::ostream& operator<<(std::ostream& out, const tok& kind);

namespace token_detail {
// Converts a character to the corresponding single-character token kind or
// returns tok::error if there isn't one.
constexpr tok to_tok(char c) {
  // clang-format off
  switch (c) {
    case '.': return tok::dot;
    case '(': return tok::l_paren;
    case ')': return tok::r_paren;
    case '#': return tok::pound;
    case '!': return tok::bang;
    case '^': return tok::caret;
    case '/': return tok::slash;
    case '|': return tok::pipe;
    case '>': return tok::gt;
    case '=': return tok::eq;
    case '*': return tok::star;
    case '$': return tok::dollar;
    case ':': return tok::colon;
    default:
      return tok::error;
  }
  // clang-format on
}
} // namespace token_detail

#ifdef __cpp_consteval
#define WHISKER_CONSTEVAL consteval
#else
#define WHISKER_CONSTEVAL constexpr
#endif

// Converts a character to the corresponding single-character token kind or
// gives a compile-time error if there isn't one.
WHISKER_CONSTEVAL tok to_tok(char c) {
  tok t = token_detail::to_tok(c);
  if (t == tok::error) {
    // This gives a compile-time error on invalid token kind.
    assert(false && "invalid token kind");
  }
  return t;
}

/**
 * A discriminator for the type of lexed tokens.
 */
struct token_kind {
  tok value;

  /* implicit */ token_kind(tok t) : value(t) {}

  /**
   * Constructs a single-character token kind from its character representation
   * validated at compile time.
   */
  WHISKER_CONSTEVAL token_kind(char c) : value(to_tok(c)) {}

  /* implicit */ operator tok() const { return value; }

  /** Determines whether witnessing this token should mark the end of lexing */
  static bool is_terminal(tok t) { return t == tok::eof || t == tok::error; }

 private:
  // These are private friends to reduce overload sets.
  friend bool operator==(tok lhs, token_kind rhs) { return lhs == rhs.value; }
  friend bool operator==(token_kind lhs, token_kind rhs) {
    return lhs.value == rhs.value;
  }
  friend bool operator==(token_kind lhs, tok rhs) { return lhs.value == rhs; }
};

std::string_view to_string(token_kind kind);
std::ostream& operator<<(std::ostream& out, const token_kind& kind);

enum class token_value_kind {
  none,
  boolean,
  i64,
  string,
};

/**
 * A lexed token capturing its kind, location in source code, and value (if the
 * token kind has a value).
 */
class token {
 public:
  token_kind kind;
  source_range range;

  token(token_kind k, const source_range& r) : kind(k), range(r) {}

  /**
   * The kind of value stored in this token.
   *   - boolean if kind is tok::kw_true or tok::kw_false.
   *   - i64 if kind is tok::i64_literal.
   *   - string if kind is tok::string_literal, tok::identifier, or tok::text.
   */
  token_value_kind value_kind() const;

  /**
   * Returns the stored boolean value of this token, if present.
   * Preconditions:
   *   - value_kind() == boolean. Otherwise throws std::runtime_error.
   */
  bool boolean_value() const;
  /**
   * Returns the stored i64 boolean value of this token, if present.
   * Preconditions:
   *   - value_kind() == i64. Otherwise throws std::runtime_error.
   */
  std::int64_t i64_value() const;
  /**
   * Returns the stored string value of this token, if present.
   * Preconditions:
   *   - value_kind() == string. Otherwise throws std::runtime_error.
   */
  std::string_view string_value() const;

  static token make_i64_literal(std::int64_t, const source_range&);
  static token make_string_literal(std::string, const source_range&);
  static token make_identifier(std::string_view, const source_range&);
  static token make_path_component(std::string_view, const source_range&);
  static token make_text(std::string, const source_range&);
  static token make_whitespace(std::string, const source_range&);
  static token make_newline(std::string_view, const source_range&);

  /**
   * Returns a mapping from keywords (as they appear in source files) to their
   * corresponding token kind.
   */
  static const std::unordered_map<std::string_view, tok>& keywords();

 private:
  std::variant<bool, std::int64_t, std::string_view, std::string> data;
};

std::string to_string(const token&);
std::ostream& operator<<(std::ostream& out, const token& token);

#undef WHISKER_CONSTEVAL

} // namespace whisker

template <>
struct fmt::formatter<whisker::tok> {
  constexpr auto parse(format_parse_context& ctx) { return ctx.begin(); }
  format_context::iterator format(const whisker::tok&, format_context&) const;
};

template <>
struct fmt::formatter<whisker::token_kind> {
  constexpr auto parse(format_parse_context& ctx) { return ctx.begin(); }
  format_context::iterator format(
      const whisker::token_kind&, format_context&) const;
};

template <>
struct fmt::formatter<whisker::token> {
  constexpr auto parse(format_parse_context& ctx) { return ctx.begin(); }
  format_context::iterator format(const whisker::token&, format_context&) const;
};
