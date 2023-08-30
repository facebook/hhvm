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

#include <folly/portability/GTest.h>
#include <thrift/compiler/parse/token.h>

#include <fmt/core.h>

using namespace apache::thrift::compiler; // NOLINT

class TokenTest : public testing::Test {
 public:
  source_manager source_mgr;
  source_location loc;

  TokenTest() : loc(source_mgr.add_virtual_file("test.thrift", "foo").start) {}
};

TEST_F(TokenTest, to_tok) {
  EXPECT_EQ(to_tok('='), tok::equal);
  EXPECT_EQ(to_tok(';'), tok::semi);
  EXPECT_EQ(detail::to_tok('x'), tok::error); // NOLINT
}

TEST_F(TokenTest, token_kind) {
  EXPECT_EQ(token_kind(tok::plus), tok::plus);
  EXPECT_EQ(token_kind('+'), tok::plus);
  EXPECT_TRUE(token_kind('+') == '+');
  token_kind kind = tok::minus;
  tok t = kind;
  EXPECT_EQ(t, tok::minus);
  EXPECT_NE(t, tok::plus);
}

TEST_F(TokenTest, to_string) {
  EXPECT_EQ(to_string(tok::comma), ",");
  EXPECT_EQ(to_string(tok::int_literal), "int literal");
  EXPECT_EQ(to_string(tok::kw_struct), "struct");

  for (int i = 0; i < 128; ++i) {
    auto kind = detail::to_tok(static_cast<char>(i)); // NOLINT
    if (kind != tok::error) {
      EXPECT_EQ(to_string(kind), fmt::format("{:c}", i));
    }
  }
}

TEST_F(TokenTest, token) {
  auto t = token(tok::kw_enum, source_range{loc, loc});
  EXPECT_EQ(t.kind, tok::kw_enum);
  EXPECT_EQ(t.range.begin, loc);
  EXPECT_EQ(t.range.end, loc);
  EXPECT_THROW(t.bool_value(), std::runtime_error);
  EXPECT_THROW(t.int_value(), std::runtime_error);
  EXPECT_THROW(t.float_value(), std::runtime_error);
  EXPECT_THROW(t.string_value(), std::runtime_error);
}

TEST_F(TokenTest, bool_literal) {
  auto t = token::make_bool_literal(source_range{loc, loc}, true);
  EXPECT_EQ(t.kind, tok::bool_literal);
  EXPECT_EQ(t.range.begin, loc);
  EXPECT_EQ(t.range.end, loc);
  EXPECT_TRUE(t.bool_value());
  EXPECT_THROW(t.int_value(), std::runtime_error);
  EXPECT_THROW(t.float_value(), std::runtime_error);
  EXPECT_THROW(t.string_value(), std::runtime_error);
}

TEST_F(TokenTest, int_literal) {
  auto t = token::make_int_literal(source_range{loc, loc}, 42);
  EXPECT_EQ(t.kind, tok::int_literal);
  EXPECT_EQ(t.range.begin, loc);
  EXPECT_EQ(t.range.end, loc);
  EXPECT_THROW(t.bool_value(), std::runtime_error);
  EXPECT_EQ(t.int_value(), 42);
  EXPECT_THROW(t.float_value(), std::runtime_error);
  EXPECT_THROW(t.string_value(), std::runtime_error);
}

TEST_F(TokenTest, float_literal) {
  auto t = token::make_float_literal(source_range{loc, loc}, 0.42);
  EXPECT_EQ(t.kind, tok::float_literal);
  EXPECT_EQ(t.range.begin, loc);
  EXPECT_EQ(t.range.end, loc);
  EXPECT_THROW(t.bool_value(), std::runtime_error);
  EXPECT_THROW(t.int_value(), std::runtime_error);
  EXPECT_EQ(t.float_value(), 0.42);
  EXPECT_THROW(t.string_value(), std::runtime_error);
}

TEST_F(TokenTest, string_literal) {
  auto t = token::make_string_literal(source_range{loc, loc}, "foo");
  EXPECT_EQ(t.kind, tok::string_literal);
  EXPECT_EQ(t.range.begin, loc);
  EXPECT_EQ(t.range.end, loc);
  EXPECT_THROW(t.bool_value(), std::runtime_error);
  EXPECT_THROW(t.int_value(), std::runtime_error);
  EXPECT_THROW(t.float_value(), std::runtime_error);
  EXPECT_EQ(t.string_value(), "foo");
}
