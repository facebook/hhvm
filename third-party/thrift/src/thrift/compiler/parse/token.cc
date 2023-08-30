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

#include <thrift/compiler/parse/token.h>

#include <fmt/core.h>

#include <algorithm>
#include <cassert>
#include <stdexcept>

namespace apache {
namespace thrift {
namespace compiler {
namespace {

struct token_kind_info {
  tok kind;
  const char* name;
};

#define THRIFT_KEYWORD(kw) \
  { tok::kw_##kw, #kw }

constexpr token_kind_info info[] = {
    {tok::eof, "EOF"},
    {tok::error, "error"},

    {tok::bool_literal, "bool literal"},
    {tok::int_literal, "int literal"},
    {tok::float_literal, "float literal"},
    {tok::string_literal, "string literal"},
    {tok::identifier, "identifier"},

    {tok::inline_doc, "inline doc"},

    // Operators and punctuation:
    {tok::comma, ","},
    {tok::semi, ";"},
    {tok::colon, ":"},
    {tok::l_paren, "("},
    {tok::r_paren, ")"},
    {tok::l_brace, "{"},
    {tok::r_brace, "}"},
    {tok::l_square, "["},
    {tok::r_square, "]"},
    {tok::less, "<"},
    {tok::greater, ">"},
    {tok::equal, "="},
    {tok::plus, "+"},
    {tok::minus, "-"},
    {tok::at, "@"},

    // Basic types:
    THRIFT_KEYWORD(void),
    THRIFT_KEYWORD(bool),
    THRIFT_KEYWORD(byte),
    THRIFT_KEYWORD(i16),
    THRIFT_KEYWORD(i32),
    THRIFT_KEYWORD(i64),
    THRIFT_KEYWORD(float),
    THRIFT_KEYWORD(double),
    THRIFT_KEYWORD(string),
    THRIFT_KEYWORD(binary),

    // Complex types:
    THRIFT_KEYWORD(list),
    THRIFT_KEYWORD(set),
    THRIFT_KEYWORD(map),
    THRIFT_KEYWORD(stream),
    THRIFT_KEYWORD(sink),

    // Function qualifiers:
    THRIFT_KEYWORD(oneway),
    THRIFT_KEYWORD(idempotent),
    THRIFT_KEYWORD(readonly),

    // Exception qualifiers:
    THRIFT_KEYWORD(safe),
    THRIFT_KEYWORD(transient),
    THRIFT_KEYWORD(stateful),
    THRIFT_KEYWORD(permanent),
    THRIFT_KEYWORD(client),
    THRIFT_KEYWORD(server),

    // Header keywords:
    THRIFT_KEYWORD(include),
    THRIFT_KEYWORD(cpp_include),
    THRIFT_KEYWORD(hs_include),
    THRIFT_KEYWORD(package),
    THRIFT_KEYWORD(namespace),

    // Other keywords:
    THRIFT_KEYWORD(const),
    THRIFT_KEYWORD(enum),
    THRIFT_KEYWORD(exception),
    THRIFT_KEYWORD(extends),
    THRIFT_KEYWORD(interaction),
    THRIFT_KEYWORD(optional),
    THRIFT_KEYWORD(performs),
    THRIFT_KEYWORD(required),
    THRIFT_KEYWORD(service),
    THRIFT_KEYWORD(struct),
    THRIFT_KEYWORD(throws),
    THRIFT_KEYWORD(typedef),
    THRIFT_KEYWORD(union)};

constexpr int num_token_kinds = sizeof(info) / sizeof(*info);

struct token_kind_names {
  const char* data[num_token_kinds] = {};

  constexpr token_kind_names() {
    int max_kind = 0;
    for (auto i : info) {
      int int_kind = static_cast<int>(i.kind);
      data[int_kind] = i.name;
      max_kind = (std::max)(int_kind, max_kind);
    }
    assert(num_token_kinds == max_kind + 1);
  }
};

constexpr token_kind_names names;

} // namespace

std::string_view to_string(token_kind kind) {
  int int_kind = static_cast<int>(kind.value);
  assert(int_kind < num_token_kinds);
  return names.data[int_kind];
}

void token::throw_invalid_kind(const char* expected) {
  throw std::runtime_error(fmt::format("token value is not {}", expected));
}

} // namespace compiler
} // namespace thrift
} // namespace apache
