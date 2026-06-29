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
#include <vector>

#include <gtest/gtest.h>
#include <thrift/compiler/diagnostic.h>
#include <thrift/compiler/parse/lexer.h>

using namespace apache::thrift::compiler;

namespace {

struct source_trivia {
  trivia_kind kind;
  source_range range;
};

} // namespace

class LexerTest : public testing::Test {
 public:
  source_manager source_mgr;
  std::string_view doc_comment;
  std::vector<source_trivia> trivia;
  diagnostics_engine diags;

  lexer make_lexer(const std::string& source, bool capture_trivia = false) {
    auto on_doc_comment = [this](std::string_view text, source_range) {
      doc_comment = text;
    };
    auto on_trivia = [this](trivia_kind kind, source_range range) {
      trivia.push_back(source_trivia{kind, range});
    };
    auto src = source_mgr.add_virtual_file("", source);
    return capture_trivia ? lexer(src, diags, on_doc_comment, on_trivia)
                          : lexer(src, diags, on_doc_comment);
  }

  LexerTest() : diags(source_mgr, [](const diagnostic&) {}) {}
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
  for (const auto& value : values) {
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
  for (const auto& value : values) {
    auto token = lexer.get_next_token();
    EXPECT_EQ(token.kind, tok::string_literal);
    EXPECT_EQ(token.string_value(), value);
  }
}

TEST_F(LexerTest, whitespace) {
  auto lexer = make_lexer("\t\r\n end");
  auto token = lexer.get_next_token();
  EXPECT_EQ(token.kind, tok::identifier);
  EXPECT_TRUE(trivia.empty());
}

TEST_F(LexerTest, trivia) {
  auto lexer = make_lexer(
      R"(// leading
struct /* mid */ S {
  1: i32 field # trailing
}
)",
      true);
  while (lexer.get_next_token().kind != tok::eof) {
  }

  ASSERT_GE(trivia.size(), 5);
  EXPECT_EQ(trivia[0].kind, trivia_kind::line_comment);
  EXPECT_EQ(source_mgr.get_text_range(trivia[0].range), "// leading");
  EXPECT_EQ(trivia[1].kind, trivia_kind::newline);
  EXPECT_EQ(trivia[2].kind, trivia_kind::block_comment);
  EXPECT_EQ(source_mgr.get_text_range(trivia[2].range), "/* mid */");
  EXPECT_EQ(trivia[4].kind, trivia_kind::line_comment);
  EXPECT_EQ(source_mgr.get_text_range(trivia[4].range), "# trailing");
}

TEST_F(LexerTest, line_doc_comment_trivia_excludes_trailing_whitespace) {
  auto lexer = make_lexer(
      R"(/// Multi-
/// line

struct S {}
)",
      true);
  while (lexer.get_next_token().kind != tok::eof) {
  }

  ASSERT_GE(trivia.size(), 3);
  EXPECT_EQ(trivia[0].kind, trivia_kind::doc_comment);
  EXPECT_EQ(source_mgr.get_text_range(trivia[0].range), "/// Multi-\n/// line");
  EXPECT_EQ(trivia[1].kind, trivia_kind::newline);
  EXPECT_EQ(source_mgr.get_text_range(trivia[1].range), "\n");
  EXPECT_EQ(trivia[2].kind, trivia_kind::newline);
  EXPECT_EQ(source_mgr.get_text_range(trivia[2].range), "\n");
  EXPECT_EQ(doc_comment, "/// Multi-\n/// line");
}

TEST_F(LexerTest, all_star_block_comment_trivia_is_not_doc_comment) {
  auto lexer = make_lexer("/***/\n42", true);
  auto token = lexer.get_next_token();
  EXPECT_EQ(token.kind, tok::int_literal);

  ASSERT_FALSE(trivia.empty());
  EXPECT_EQ(trivia[0].kind, trivia_kind::block_comment);
  EXPECT_EQ(source_mgr.get_text_range(trivia[0].range), "/***/");
  EXPECT_TRUE(doc_comment.empty());
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
