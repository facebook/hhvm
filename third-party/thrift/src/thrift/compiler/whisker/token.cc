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

#include <thrift/compiler/whisker/detail/overload.h>
#include <thrift/compiler/whisker/detail/string.h>
#include <thrift/compiler/whisker/source_location.h>
#include <thrift/compiler/whisker/token.h>

#include <cassert>
#include <cstddef>
#include <stdexcept>
#include <type_traits>

#include <fmt/core.h>
#include <fmt/ranges.h>

// clang-format off
// "X macro" pattern to de-dupe code:
//   https://en.wikipedia.org/wiki/X_macro
#define WHISKER_KEYWORDS()  \
  WHISKER_KEYWORD(true)     \
  WHISKER_KEYWORD(false)    \
  WHISKER_KEYWORD(null)     \
  WHISKER_KEYWORD(if)       \
  WHISKER_KEYWORD(unless)   \
  WHISKER_KEYWORD(else)     \
  WHISKER_KEYWORD(each)     \
  WHISKER_KEYWORD(as)       \
  WHISKER_KEYWORD(partial)  \
  WHISKER_KEYWORD(captures) \
  WHISKER_KEYWORD(let)      \
  WHISKER_KEYWORD(and)      \
  WHISKER_KEYWORD(or)       \
  WHISKER_KEYWORD(not)      \
  WHISKER_KEYWORD(pragma)   \
  WHISKER_KEYWORD(with)     \
  WHISKER_KEYWORD(this)     \
  WHISKER_KEYWORD(define)   \
  WHISKER_KEYWORD(for)      \
  WHISKER_KEYWORD(do)       \
  WHISKER_KEYWORD(import)   \
  WHISKER_KEYWORD(export)   \
  WHISKER_KEYWORD(from)
// clang-format on

namespace whisker {
namespace {
struct token_kind_info {
  tok kind;
  const char* name;
};

constexpr token_kind_info info[] = {
    {tok::eof, "EOF"},
    {tok::error, "error"},

    {tok::i64_literal, "int literal"},
    {tok::string_literal, "string literal"},

    {tok::identifier, "identifier"},
    {tok::path_component, "path component"},

    {tok::text, "text"},
    {tok::whitespace, "whitespace"},
    {tok::newline, "new line"},

    {tok::open, "`{{`"},
    {tok::close, "`}}`"},

    // Punctuation:
    {tok::dot, "`.`"},
    {tok::l_paren, "`(`"},
    {tok::r_paren, "`)`"},
    {tok::pound, "`#`"},
    {tok::bang, "`!`"},
    {tok::caret, "`^`"},
    {tok::slash, "`/`"},
    {tok::pipe, "`|`"},
    {tok::gt, "`>`"},
    {tok::eq, "`=`"},
    {tok::star, "`*`"},
    {tok::dollar, "`$`"},
    {tok::colon, "`:`"},

#define WHISKER_KEYWORD(kw) {tok::kw_##kw, "`" #kw "`"},
    WHISKER_KEYWORDS()
#undef WHISKER_KEYWORD
};

constexpr std::size_t num_token_kinds = sizeof(info) / sizeof(*info);

struct token_kind_names {
  const char* data[num_token_kinds] = {};

  constexpr token_kind_names() {
    std::underlying_type_t<tok> max_kind = 0;
    for (auto i : info) {
      auto kind = static_cast<std::underlying_type_t<tok>>(i.kind);
      data[kind] = i.name;
      max_kind = std::max(kind, max_kind);
    }
    assert(num_token_kinds == max_kind + 1);
  }
};

constexpr token_kind_names names;

const std::unordered_map<std::string_view, tok> keywords_to_tok = {
#define WHISKER_KEYWORD(kw) {#kw, tok::kw_##kw},
    WHISKER_KEYWORDS()
#undef WHISKER_KEYWORD
};

[[noreturn]] void throw_invalid_kind(std::string_view expected) {
  throw std::runtime_error(fmt::format("token kind is not {}", expected));
}

} // namespace

std::string_view to_string(tok kind) {
  auto integral_value = static_cast<std::underlying_type_t<tok>>(kind);
  assert(integral_value < num_token_kinds);
  return names.data[integral_value];
}

std::ostream& operator<<(std::ostream& out, const tok& kind) {
  return out << to_string(kind);
}

std::string_view to_string(token_kind kind) {
  return to_string(kind.value);
}

std::ostream& operator<<(std::ostream& out, const token_kind& kind) {
  return out << to_string(kind);
}

token_value_kind token::value_kind() const {
  switch (kind) {
    case tok::kw_true:
    case tok::kw_false:
      return token_value_kind::boolean;
    case tok::i64_literal:
      return token_value_kind::i64;
    case tok::string_literal:
    case tok::identifier:
    case tok::path_component:
    case tok::text:
    case tok::whitespace:
    case tok::newline:
      return token_value_kind::string;
    default:
      return token_value_kind::none;
  }
}

bool token::boolean_value() const {
  switch (kind) {
    case tok::kw_true:
      return true;
    case tok::kw_false:
      return false;
    default:
      throw_invalid_kind("boolean literal");
  }
}

std::int64_t token::i64_value() const {
  if (kind != tok::i64_literal) {
    throw_invalid_kind(to_string(tok::i64_literal));
  }
  return std::get<std::int64_t>(data);
}

std::string_view token::string_value() const {
  if (value_kind() != token_value_kind::string) {
    throw_invalid_kind(
        fmt::format(
            "one of {}",
            fmt::join(
                std::initializer_list<std::string_view>{
                    to_string(tok::string_literal),
                    to_string(tok::identifier),
                    to_string(tok::path_component),
                    to_string(tok::text),
                    to_string(tok::whitespace),
                    to_string(tok::newline)},
                ", ")));
  }
  return detail::variant_match(
      data,
      [](const std::string_view& str) -> std::string_view { return str; },
      [](const std::string& str) -> std::string_view { return str; },
      [](auto&&) -> std::string_view {
        throw std::logic_error("data is not a string");
      });
}

/* static */ token token::make_i64_literal(
    std::int64_t value, const source_range& r) {
  auto t = token(tok::i64_literal, r);
  t.data = value;
  return t;
}

/* static */ token token::make_string_literal(
    std::string value, const source_range& r) {
  auto t = token(tok::string_literal, r);
  t.data = std::move(value);
  return t;
}

/* static */ token token::make_identifier(
    std::string_view value, const source_range& r) {
  auto t = token(tok::identifier, r);
  t.data = value;
  return t;
}

/* static */ token token::make_path_component(
    std::string_view value, const source_range& r) {
  auto t = token(tok::path_component, r);
  t.data = value;
  return t;
}

/* static */ token token::make_text(std::string value, const source_range& r) {
  auto t = token(tok::text, r);
  t.data = std::move(value);
  return t;
}

/* static */ token token::make_whitespace(
    std::string value, const source_range& r) {
  auto t = token(tok::whitespace, r);
  t.data = std::move(value);
  return t;
}

/* static */ token token::make_newline(
    std::string_view value, const source_range& r) {
  auto t = token(tok::newline, r);
  t.data = value;
  return t;
}

/* static */ const std::unordered_map<std::string_view, tok>&
token::keywords() {
  return keywords_to_tok;
}

std::string to_string(const token& token) {
  switch (token.value_kind()) {
    case token_value_kind::boolean:
      return fmt::format(
          "token[boolean={}]", token.boolean_value() ? "true" : "false");
    case token_value_kind::i64:
      return fmt::format("token[i64={}]", token.i64_value());
    case token_value_kind::string:
      return fmt::format(
          "token[kind={}, string=\"{}\"]",
          to_string(token.kind),
          detail::escape(token.string_value()));
    case token_value_kind::none:
      return fmt::format("token[kind={}]", to_string(token.kind));
  }
  throw std::logic_error("Unreachable");
}

std::ostream& operator<<(std::ostream& out, const token& token) {
  return out << to_string(token);
}

} // namespace whisker

#undef WHISKER_KEYWORDS

fmt::format_context::iterator fmt::formatter<whisker::tok>::format(
    const whisker::tok& t, format_context& ctx) const {
  return fmt::format_to(ctx.out(), "{}", whisker::to_string(t));
}

fmt::format_context::iterator fmt::formatter<whisker::token_kind>::format(
    const whisker::token_kind& t, format_context& ctx) const {
  return fmt::format_to(ctx.out(), "{}", whisker::to_string(t));
}

fmt::format_context::iterator fmt::formatter<whisker::token>::format(
    const whisker::token& t, format_context& ctx) const {
  return fmt::format_to(ctx.out(), "{}", whisker::to_string(t));
}
