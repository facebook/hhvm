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

#include <cassert>

#include <thrift/compiler/source_location.h>

namespace apache {
namespace thrift {
namespace compiler {

// Token kind constants.
enum class tok {
  eof,
  error,

  bool_literal,
  int_literal,
  float_literal,
  string_literal,
  identifier,

  inline_doc,

  // clang-format off
  // Operators and punctuation:
  comma,     // ","
  semi,      // ";"
  colon,     // ":"
  l_paren,   // "("
  r_paren,   // ")"
  l_brace,   // "{"
  r_brace,   // "}"
  l_square,  // "["
  r_square,  // "]"
  less,      // "<"
  greater,   // ">"
  equal,     // "="
  plus,      // "+"
  minus,     // "-"
  at,        // "@"
  // clang-format om

  // Basic types:
  kw_void,
  kw_bool,
  kw_byte,
  kw_i16,
  kw_i32,
  kw_i64,
  kw_float,
  kw_double,
  kw_string,
  kw_binary,

  // Complex types:
  kw_list,
  kw_set,
  kw_map,
  kw_stream,
  kw_sink,

  // Function qualifiers:
  kw_oneway,
  kw_idempotent,
  kw_readonly,

  // Exception qualifiers:
  kw_safe,
  kw_transient,
  kw_stateful,
  kw_permanent,
  kw_client,
  kw_server,

  // Header keywords:
  kw_include,
  kw_cpp_include,
  kw_hs_include,
  kw_package,
  kw_namespace,

  // Other keywords:
  kw_const,
  kw_enum,
  kw_exception,
  kw_extends,
  kw_interaction,
  kw_optional,
  kw_performs,
  kw_required,
  kw_service,
  kw_struct,
  kw_throws,
  kw_typedef,
  kw_union,
};

namespace detail {
// Converts a character to the corresponding single-character token kind or
// returns tok::error if there isn't one.
constexpr tok to_tok(char c) {
  switch (c) {
    case ',': return tok::comma;
    case ';': return tok::semi;
    case ':': return tok::colon;
    case '(': return tok::l_paren;
    case ')': return tok::r_paren;
    case '{': return tok::l_brace;
    case '}': return tok::r_brace;
    case '[': return tok::l_square;
    case ']': return tok::r_square;
    case '<': return tok::less;
    case '>': return tok::greater;
    case '=': return tok::equal;
    case '+': return tok::plus;
    case '-': return tok::minus;
    case '@': return tok::at;
  }
  return tok::error;
}
} // namespace detail

#ifdef __cpp_consteval
#define THRIFT_CONSTEVAL consteval
#else
#define THRIFT_CONSTEVAL constexpr
#endif

// Converts a character to the corresponding single-character token kind or
// gives a compile-time error if there isn't one.
THRIFT_CONSTEVAL tok to_tok(char c) {
  tok t = detail::to_tok(c);
  if (t == tok::error) {
    // This gives a compile-time error on invalid token kind.
    assert(false && "invalid token kind");
  }
  return t;
}

// A token kind.
struct token_kind {
  tok value;

  /* implicit */ token_kind(tok t) : value(t) {}

  // Constructs a single-character token kind from its character representation
  // validated at compile time.
  THRIFT_CONSTEVAL token_kind(char c) : value(to_tok(c)) {}

  operator tok() const { return value; }

 private:
  // These are private friends to reduce overload sets.
  friend bool operator==(tok lhs, token_kind rhs) {
    return lhs == rhs.value;
  }
  friend bool operator==(token_kind lhs, token_kind rhs) {
    return lhs.value == rhs.value;
  }

  // The following overloads are only for compatibility with pre-C++20.
  friend bool operator==(token_kind lhs, tok rhs) {
    return lhs.value == rhs;
  }
  friend bool operator!=(tok lhs, token_kind rhs) {
    return lhs != rhs.value;
  }
  friend bool operator!=(token_kind lhs, tok rhs) {
    return lhs.value != rhs;
  }
  friend bool operator!=(token_kind lhs, token_kind rhs) {
    return lhs.value != rhs.value;
  }
};

std::string_view to_string(token_kind kind);

// A lexed token.
class token {
 public:
  token_kind kind;
  source_range range;

  token(token_kind k, const source_range& r) : kind(k), range(r) {}

  static token make_bool_literal(const source_range& r, bool value) {
    auto t = token(tok::bool_literal, r);
    t.as.bool_value = value;
    return t;
  }

  static token make_int_literal(const source_range& r, uint64_t value) {
    auto t = token(tok::int_literal, r);
    t.as.int_value = value;
    return t;
  }

  // Makes a floating-point literal.
  static token make_float_literal(const source_range& r, double value) {
    auto t = token(tok::float_literal, r);
    t.as.float_value = value;
    return t;
  }

  static token make_string_literal(
      const source_range& r, std::string_view value) {
    return make_string_token(tok::string_literal, r, value);
  }

  static token make_identifier(const source_range& r, std::string_view value) {
    return make_string_token(tok::identifier, r, value);
  }

  static token make_inline_doc(
      const source_range& r, std::string_view value) {
    return make_string_token(tok::inline_doc, r, value);
  }

  bool bool_value() const {
    if (kind != tok::bool_literal) {
      throw_invalid_kind("bool");
    }
    return as.bool_value;
  }

  uint64_t int_value() const {
    if (kind != tok::int_literal) {
      throw_invalid_kind("int");
    }
    return as.int_value;
  }

  // Returns the value of a floating-point literal.
  double float_value() const {
    if (kind != tok::float_literal) {
      throw_invalid_kind("float");
    }
    return as.float_value;
  }

  // Returns the value of an identifier, string literal or inline doc.
  // The string is owned by the source manager.
  std::string_view string_value() const {
    if (kind != tok::identifier && kind != tok::string_literal &&
        kind != tok::inline_doc) {
      throw_invalid_kind("string");
    }
    return {as.string_value.data, as.string_value.size};
  }

 private:
  struct string_view {
    const char* data;
    size_t size;
  };

  union {
    bool bool_value;
    uint64_t int_value;
    double float_value;
    string_view string_value;
  } as;

  // Creates a string-valued token such as a string literal or an identifier.
  static token make_string_token(
      tok kind, const source_range& r, std::string_view value) {
    auto t = token(kind, r);
    t.as.string_value = {value.data(), value.size()};
    return t;
  }

  [[noreturn]] static void throw_invalid_kind(const char* expected);
};

} // namespace compiler
} // namespace thrift
} // namespace apache
