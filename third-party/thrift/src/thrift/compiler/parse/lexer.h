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

#include <boost/optional.hpp>
#include <fmt/core.h>
#include <thrift/compiler/parse/token.h>
#include <thrift/compiler/source_location.h>

namespace apache {
namespace thrift {
namespace compiler {

class diagnostics_engine;

// An interface that receives notifications of lexical elements.
class lex_handler {
 public:
  virtual ~lex_handler() {}

  // Invoked on a documentation comment such as `/** ... */` or `/// ...`.
  virtual void on_doc_comment(fmt::string_view text, source_location loc) = 0;
};

// A Thrift lexer.
class lexer {
 private:
  fmt::string_view source_; // Source being lexed; has a terminating '\0'.
  source_location start_;
  const char* ptr_; // Current position in the source.
  const char* token_start_;
  lex_handler* handler_;
  diagnostics_engine* diags_;

  const char* end() const { return source_.data() + source_.size() - 1; }

  // Converts a pointer within source into a location.
  source_location location(const char* p) const {
    return start_ + (p - source_.data());
  }

  source_range token_source_range() const {
    return {location(token_start_), location(ptr_)};
  }

  // Returns the string representation of the last token reported via
  // `get_next_token` or `lex_handler`. If no token has been reported returns
  // an empty string.
  fmt::string_view token_text() const {
    return {token_start_, static_cast<size_t>(ptr_ - token_start_)};
  }

  void start_token() { token_start_ = ptr_; }

  // Reports an error if the parsed value cannot fit in the widest supported
  // representation, i.e. int64_t and double.
  token make_int_literal(int offset, int base);
  token make_float_literal();

  template <typename... T>
  token report_error(fmt::format_string<T...> msg, T&&... args);
  token unexpected_token();

  enum class comment_lex_result { skipped, doc_comment, unterminated };

  void skip_line_comment();
  bool lex_doc_comment();
  comment_lex_result lex_block_comment();
  comment_lex_result lex_whitespace_or_comment();

 public:
  lexer(source src, lex_handler& handler, diagnostics_engine& diags);

  // Lexes the content of a string literal and returns its value with escape
  // sequences translated or an empty optional on error. It is done separately
  // from get_next_token to keep tokens lightweight (no memory management and
  // no dispatch on the token type in ctors/dtor).
  boost::optional<std::string> lex_string_literal(token literal);

  // Lexes and returns the next token.
  token get_next_token();
};

} // namespace compiler
} // namespace thrift
} // namespace apache
