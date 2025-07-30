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

#include <string_view>

#include <gtest/gtest.h>
#include <thrift/compiler/diagnostic.h>
#include <thrift/compiler/parse/lexer.h>

using namespace apache::thrift::compiler;

class LexerTest : public testing::Test {
 public:
  source_manager source_mgr;
  std::string_view doc_comment;
  diagnostics_engine diags;

  lexer make_lexer(const std::string& source) {
    return {
        source_mgr.add_virtual_file("", source),
        diags,
        [this](std::string_view text, source_range) { doc_comment = text; }};
  }

  LexerTest() : diags(source_mgr, [](diagnostic) {}) {}
};

TEST_F(LexerTest, move) {
  auto lexer = make_lexer("42");
  auto moved_lexer = std::move(lexer);
  auto token = moved_lexer.get_next_token();
  EXPECT_EQ(token.kind, tok::int_literal);
}

TEST_F(LexerTest, eof) {
  auto lexer = make_lexer("");
  auto token = lexer.get_next_token();
  EXPECT_EQ(token.kind, tok::eof);
}

TEST_F(LexerTest, identifier) {
  auto lexer = make_lexer("foo _ bar42 foo.bar");
  const std::string values[] = {"foo", "_", "bar42", "foo.bar"};
  for (auto value : values) {
    auto token = lexer.get_next_token();
    EXPECT_EQ(token.kind, tok::identifier);
    EXPECT_EQ(token.string_value(), value);
  }
}

TEST_F(LexerTest, keywords) {
  auto lexer = make_lexer("double include string");
  const tok kinds[] = {tok::kw_double, tok::kw_include, tok::kw_string};
  for (auto kind : kinds) {
    auto token = lexer.get_next_token();
    EXPECT_EQ(token.kind, kind);
  }
}

TEST_F(LexerTest, int_literal) {
  auto lexer = make_lexer("42 0b100 0B010 0777 0xdeadbeef 0XCAFE");
  const uint64_t values[] = {42, 0b100, 0B010, 0777, 0xdeadbeef, 0XCAFE};
  for (auto value : values) {
    auto token = lexer.get_next_token();
    EXPECT_EQ(token.kind, tok::int_literal);
    EXPECT_EQ(token.int_value(), value);
  }
}

TEST_F(LexerTest, int_literal_octal_zero) {
  auto lexer = make_lexer("0\n32");
  auto number1 = lexer.get_next_token();
  auto number2 = lexer.get_next_token();

  EXPECT_EQ(number1.kind, tok::int_literal);
  EXPECT_EQ(number1.int_value(), 0);

  EXPECT_EQ(number2.kind, tok::int_literal);
  EXPECT_EQ(number2.int_value(), 32);
}

TEST_F(LexerTest, float_literal) {
  auto lexer = make_lexer("3.14 1e23 1.2E+34 0.0 .4e-2");
  const double values[] = {3.14, 1e23, 1.2E+34, 0.0, .4e-2};
  for (auto value : values) {
    auto token = lexer.get_next_token();
    EXPECT_EQ(token.kind, tok::float_literal);
    EXPECT_EQ(token.float_value(), value);
  }
}

TEST_F(LexerTest, string_literal) {
  auto lexer = make_lexer("\"foo\" 'bar' \"multi\nline\nstring\"");
  const std::string values[] = {"\"foo\"", "'bar'", "\"multi\nline\nstring\""};
  for (auto value : values) {
    auto token = lexer.get_next_token();
    EXPECT_EQ(token.kind, tok::string_literal);
    EXPECT_EQ(token.string_value(), value);
  }
}

TEST_F(LexerTest, whitespace) {
  auto lexer = make_lexer("\t\r\n end");
  auto token = lexer.get_next_token();
  EXPECT_EQ(token.kind, tok::identifier);
}

TEST_F(LexerTest, block_comment) {
  auto lexer = make_lexer(R"(
      /*******/
      /*
      Block
      comment
      */
      end
      )");
  auto token = lexer.get_next_token();
  EXPECT_EQ(token.kind, tok::identifier);
}

TEST_F(LexerTest, block_doc_comment) {
  auto lexer = make_lexer(R"(
        /** Block comment */
        42
      )");
  auto token = lexer.get_next_token();
  EXPECT_EQ(token.kind, tok::int_literal);
  EXPECT_EQ(doc_comment, "/** Block comment */");
}

TEST_F(LexerTest, line_doc_comment) {
  auto lexer = make_lexer(R"(
        /// Multi-
        /// line
        /// comment
        42
      )");
  auto token = lexer.get_next_token();
  EXPECT_EQ(token.kind, tok::int_literal);
}

TEST_F(LexerTest, inline_doc_comment) {
  auto lexer = make_lexer(R"(
        ///< Multi-
        ///< line
        ///< comment
      )");
  auto token = lexer.get_next_token();
  EXPECT_EQ(token.kind, tok::inline_doc);
}
