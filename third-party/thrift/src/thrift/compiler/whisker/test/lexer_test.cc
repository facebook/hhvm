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

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <thrift/compiler/whisker/detail/overload.h>
#include <thrift/compiler/whisker/detail/string.h>
#include <thrift/compiler/whisker/diagnostic.h>
#include <thrift/compiler/whisker/lexer.h>
#include <thrift/compiler/whisker/source_location.h>

#include <limits>
#include <string>
#include <variant>

#include <fmt/core.h>

namespace whisker {

struct token_description {
  token_kind kind;
  std::variant<std::monostate, bool, std::int64_t, std::string> value;

  friend std::ostream& operator<<(
      std::ostream& out, const token_description& desc) {
    std::string str = detail::variant_match(
        desc.value,
        [&desc](std::monostate) {
          return fmt::format("token[kind={}]", to_string(desc.kind));
        },
        [](bool val) {
          return fmt::format("token[boolean={}]", val ? "true" : "false");
        },
        [](std::int64_t val) { return fmt::format("token[i64={}]", val); },
        [&desc](const std::string& val) {
          return fmt::format(
              "token[kind={}, string=\"{}\"]",
              to_string(desc.kind),
              detail::escape(val));
        });
    return out << str;
  }

  friend bool operator==(const token_description& lhs, const token& rhs) {
    if (lhs.kind != rhs.kind) {
      return false;
    }
    switch (rhs.value_kind()) {
      case token_value_kind::boolean: {
        const auto* val = std::get_if<bool>(&lhs.value);
        return val && *val == rhs.boolean_value();
      }
      case token_value_kind::i64: {
        const auto* val = std::get_if<std::int64_t>(&lhs.value);
        return val && *val == rhs.i64_value();
      }
      case token_value_kind::string: {
        const auto* val = std::get_if<std::string>(&lhs.value);
        return val && *val == rhs.string_value();
      }
      case token_value_kind::none:
        return std::holds_alternative<std::monostate>(lhs.value);
    }
    return true;
  }

  friend bool operator==(const token& lhs, const token_description& rhs) {
    return rhs == lhs;
  }

  friend bool operator==(
      const token_description& lhs, const token_description& rhs) {
    return lhs.kind == rhs.kind && lhs.value == rhs.value;
  }
};

class LexerTest : public testing::Test {
 private:
  source_manager source_mgr;
  diagnostics_engine diags;
  int file_id = 1;

 public:
  std::vector<diagnostic> diagnostics;

  static std::string path_to_file(int id) {
    return fmt::format("path/to/test-{}.whisker", id);
  }

  lexer make_lexer(const std::string& source) {
    return lexer(
        source_mgr.add_virtual_file(path_to_file(file_id++), source), diags);
  }

  std::string_view token_range_text(const token& t) {
    const char* begin = source_mgr.get_text(t.range.begin);
    const char* end = source_mgr.get_text(t.range.end);
    return std::string_view(begin, end - begin);
  }

  LexerTest()
      : diags(source_mgr, [this](diagnostic d) {
          diagnostics.push_back(std::move(d));
        }) {}
};

TEST_F(LexerTest, move) {
  auto lexer = make_lexer("42");
  auto moved_lexer = std::move(lexer);
  auto token = moved_lexer.next_token();
  EXPECT_EQ(token.kind, tok::text);
  EXPECT_EQ(token.string_value(), "42");
}

TEST_F(LexerTest, eof) {
  auto lexer = make_lexer("foo");
  auto first = lexer.next_token();
  EXPECT_EQ(first.kind, tok::text);
  EXPECT_EQ(first.string_value(), "foo");
  // EOF should be repeated forever
  for (int i = 0; i < 10; ++i) {
    auto token = lexer.next_token();
    EXPECT_EQ(token.kind, tok::eof);
  }
}

TEST_F(LexerTest, error) {
  auto lexer = make_lexer("{{%");
  auto first = lexer.next_token();
  EXPECT_EQ(first.kind, tok::open);
  // error should be repeated forever
  for (int i = 0; i < 10; ++i) {
    auto token = lexer.next_token();
    EXPECT_EQ(token.kind, tok::error);
  }
}

TEST_F(LexerTest, basic_identifier) {
  auto lexer = make_lexer("{{ basic }}");
  const std::vector<token_description> expected = {
      {tok::open, {}},
      {tok::identifier, "basic"},
      {tok::close, {}},
      {tok::eof, {}},
  };
  auto actual = lexer.tokenize_all();
  EXPECT_THAT(actual, testing::ElementsAreArray(expected));
}

TEST_F(LexerTest, multiple_identifiers) {
  auto lexer = make_lexer("some text {{ bas ic }} some text");
  const std::vector<token_description> expected = {
      {tok::text, "some"},
      {tok::whitespace, " "},
      {tok::text, "text"},
      {tok::whitespace, " "},
      {tok::open, {}},
      {tok::identifier, "bas"},
      {tok::identifier, "ic"},
      {tok::close, {}},
      {tok::whitespace, " "},
      {tok::text, "some"},
      {tok::whitespace, " "},
      {tok::text, "text"},
      {tok::eof, {}},
  };
  auto actual = lexer.tokenize_all();
  EXPECT_THAT(actual, testing::ElementsAreArray(expected));
}

TEST_F(LexerTest, ids_and_punctuations) {
  auto lexer = make_lexer("{{ |. ! bas ^> =*$ ic /# }}{{()}}");
  const std::vector<token_description> expected = {
      {tok::open, {}},
      {tok::pipe, {}},
      {tok::dot, {}},
      {tok::bang, {}},
      {tok::identifier, "bas"},
      {tok::caret, {}},
      {tok::gt, {}},
      {tok::eq, {}},
      {tok::star, {}},
      {tok::dollar, {}},
      {tok::identifier, "ic"},
      {tok::slash, {}},
      {tok::pound, {}},
      {tok::close, {}},
      {tok::open, {}},
      {tok::l_paren, {}},
      {tok::r_paren, {}},
      {tok::close, {}},
      {tok::eof, {}},
  };
  auto actual = lexer.tokenize_all();
  EXPECT_THAT(actual, testing::ElementsAreArray(expected));
}

TEST_F(LexerTest, unterminated_open) {
  auto lexer = make_lexer("some text{{foo bar");
  const std::vector<token_description> expected = {
      {tok::text, "some"},
      {tok::whitespace, " "},
      {tok::text, "text"},
      {tok::open, {}},
      {tok::identifier, "foo"},
      {tok::identifier, "bar"},
      {tok::eof, {}},
  };
  auto actual = lexer.tokenize_all();
  EXPECT_THAT(actual, testing::ElementsAreArray(expected));
}

TEST_F(LexerTest, unrecognized_token) {
  auto lexer = make_lexer("{{foo bar ~%&* baz}}");
  const std::vector<token_description> expected = {
      {tok::open, {}},
      {tok::identifier, "foo"},
      {tok::identifier, "bar"},
      {tok::error, {}},
  };
  auto actual = lexer.tokenize_all();
  EXPECT_THAT(actual, testing::ElementsAreArray(expected));
  EXPECT_THAT(
      diagnostics,
      testing::ElementsAre(diagnostic(
          diagnostic_level::error,
          "unexpected token in input: ~",
          path_to_file(1),
          1)));
}

TEST_F(LexerTest, unrecognized_token_against_identifier) {
  auto lexer = make_lexer("{{foo%");
  const std::vector<token_description> expected = {
      {tok::open, {}},
      {tok::identifier, "foo"},
      {tok::error, {}},
  };
  auto actual = lexer.tokenize_all();
  EXPECT_THAT(actual, testing::ElementsAreArray(expected));
  EXPECT_THAT(
      diagnostics,
      testing::ElementsAre(diagnostic(
          diagnostic_level::error,
          "unexpected token in input: %",
          path_to_file(1),
          1)));
}

TEST_F(LexerTest, partial_apply) {
  auto lexer = make_lexer("{{> foo/bar }}");
  const std::vector<token_description> expected = {
      {tok::open, {}},
      {tok::gt, {}},
      {tok::path_component, "foo"},
      {tok::slash, {}},
      {tok::path_component, "bar"},
      {tok::close, {}},
      {tok::eof, {}},
  };
  auto actual = lexer.tokenize_all();
  EXPECT_THAT(actual, testing::ElementsAreArray(expected));
}

TEST_F(LexerTest, partial_apply_single_component) {
  auto lexer = make_lexer("{{> foo }}");
  const std::vector<token_description> expected = {
      {tok::open, {}},
      {tok::gt, {}},
      {tok::path_component, "foo"},
      {tok::close, {}},
      {tok::eof, {}},
  };
  auto actual = lexer.tokenize_all();
  EXPECT_THAT(actual, testing::ElementsAreArray(expected));
}

TEST_F(LexerTest, partial_apply_no_path) {
  auto lexer = make_lexer("{{> }}");
  const std::vector<token_description> expected = {
      {tok::open, {}},
      {tok::gt, {}},
      {tok::close, {}},
      {tok::eof, {}},
  };
  auto actual = lexer.tokenize_all();
  EXPECT_THAT(actual, testing::ElementsAreArray(expected));
}

TEST_F(LexerTest, partial_apply_keyword_and_dots) {
  auto lexer = make_lexer("{{> foo-bar / true /source.cpp }}");
  const std::vector<token_description> expected = {
      {tok::open, {}},
      {tok::gt, {}},
      {tok::path_component, "foo-bar"},
      {tok::slash, {}},
      {tok::path_component, "true"}, // keyword
      {tok::slash, {}},
      {tok::path_component, "source.cpp"},
      {tok::close, {}},
      {tok::eof, {}},
  };
  auto actual = lexer.tokenize_all();
  EXPECT_THAT(actual, testing::ElementsAreArray(expected));
}

TEST_F(LexerTest, partial_apply_bad_token) {
  // Should not error in the lexer, leave that to the parser
  auto lexer = make_lexer("{{> foo / \"bar\" }}");
  const std::vector<token_description> expected = {
      {tok::open, {}},
      {tok::gt, {}},
      {tok::path_component, "foo"},
      {tok::slash, {}},
      {tok::string_literal, "bar"},
      {tok::close, {}},
      {tok::eof, {}},
  };
  auto actual = lexer.tokenize_all();
  EXPECT_THAT(actual, testing::ElementsAreArray(expected));
}

TEST_F(LexerTest, keywords) {
  constexpr std::string_view keywords =
      "true false null if unless else each as partial let and or not with this define for do import export from";
  auto lexer = make_lexer(fmt::format("{0}{{{{{0}}}}}", keywords));
  const token_description ws = {tok::whitespace, " "};
  const std::vector<token_description> expected = {
      {tok::text, "true"},    ws,
      {tok::text, "false"},   ws,
      {tok::text, "null"},    ws,
      {tok::text, "if"},      ws,
      {tok::text, "unless"},  ws,
      {tok::text, "else"},    ws,
      {tok::text, "each"},    ws,
      {tok::text, "as"},      ws,
      {tok::text, "partial"}, ws,
      {tok::text, "let"},     ws,
      {tok::text, "and"},     ws,
      {tok::text, "or"},      ws,
      {tok::text, "not"},     ws,
      {tok::text, "with"},    ws,
      {tok::text, "this"},    ws,
      {tok::text, "define"},  ws,
      {tok::text, "for"},     ws,
      {tok::text, "do"},      ws,
      {tok::text, "import"},  ws,
      {tok::text, "export"},  ws,
      {tok::text, "from"},    {tok::open, {}},
      {tok::kw_true, {true}}, {tok::kw_false, {false}},
      {tok::kw_null, {}},     {tok::kw_if, {}},
      {tok::kw_unless, {}},   {tok::kw_else, {}},
      {tok::kw_each, {}},     {tok::kw_as, {}},
      {tok::kw_partial, {}},  {tok::kw_let, {}},
      {tok::kw_and, {}},      {tok::kw_or, {}},
      {tok::kw_not, {}},      {tok::kw_with, {}},
      {tok::kw_this, {}},     {tok::kw_define, {}},
      {tok::kw_for, {}},      {tok::kw_do, {}},
      {tok::kw_import, {}},   {tok::kw_export, {}},
      {tok::kw_from, {}},     {tok::close, {}},
      {tok::eof, {}},
  };
  auto actual = lexer.tokenize_all();
  EXPECT_THAT(actual, testing::ElementsAreArray(expected));
}

TEST_F(LexerTest, keyword_in_identifier) {
  auto lexer = make_lexer("{{if else ifelse}}");
  const std::vector<token_description> expected = {
      {tok::open, {}},
      {tok::kw_if, {}},
      {tok::kw_else, {}},
      {tok::identifier, "ifelse"},
      {tok::close, {}},
      {tok::eof, {}},
  };
  auto actual = lexer.tokenize_all();
  EXPECT_THAT(actual, testing::ElementsAreArray(expected));
}

TEST_F(LexerTest, basic_i64_literal) {
  auto lexer = make_lexer("{{1234}}");
  const std::vector<token_description> expected = {
      {tok::open, {}},
      {tok::i64_literal, 1234},
      {tok::close, {}},
      {tok::eof, {}},
  };
  auto actual = lexer.tokenize_all();
  EXPECT_THAT(actual, testing::ElementsAreArray(expected));
}

TEST_F(LexerTest, negative_i64_literal) {
  auto lexer = make_lexer("{{-1234}}");
  const std::vector<token_description> expected = {
      {tok::open, {}},
      {tok::i64_literal, -1234},
      {tok::close, {}},
      {tok::eof, {}},
  };
  auto actual = lexer.tokenize_all();
  EXPECT_THAT(actual, testing::ElementsAreArray(expected));
}

TEST_F(LexerTest, i64_zero) {
  {
    auto lexer = make_lexer("{{0}}");
    const std::vector<token_description> expected = {
        {tok::open, {}},
        {tok::i64_literal, 0},
        {tok::close, {}},
        {tok::eof, {}},
    };
    auto actual = lexer.tokenize_all();
    EXPECT_THAT(actual, testing::ElementsAreArray(expected));
  }

  {
    auto lexer = make_lexer("{{-0}}");
    const std::vector<token_description> expected = {
        {tok::open, {}},
        {tok::i64_literal, 0},
        {tok::close, {}},
        {tok::eof, {}},
    };
    auto actual = lexer.tokenize_all();
    EXPECT_THAT(actual, testing::ElementsAreArray(expected));
  }

  {
    auto lexer = make_lexer("{{000}}");
    const std::vector<token_description> expected = {
        {tok::open, {}},
        {tok::i64_literal, 0},
        {tok::close, {}},
        {tok::eof, {}},
    };
    auto actual = lexer.tokenize_all();
    EXPECT_THAT(actual, testing::ElementsAreArray(expected));
  }
}

TEST_F(LexerTest, negative_i64_literal_with_space) {
  auto lexer = make_lexer("{{-   1234}}");
  const std::vector<token_description> expected = {
      {tok::open, {}},
      {tok::i64_literal, -1234},
      {tok::close, {}},
      {tok::eof, {}},
  };
  auto actual = lexer.tokenize_all();
  EXPECT_THAT(actual, testing::ElementsAreArray(expected));
}

TEST_F(LexerTest, lone_minus) {
  auto lexer = make_lexer("{{-}}");
  const std::vector<token_description> expected = {
      {tok::open, {}},
      {tok::error, {}},
  };
  auto actual = lexer.tokenize_all();
  EXPECT_THAT(actual, testing::ElementsAreArray(expected));
  EXPECT_THAT(
      diagnostics,
      testing::ElementsAre(diagnostic(
          diagnostic_level::error,
          "unexpected token in input: -",
          path_to_file(1),
          1)));
}

TEST_F(LexerTest, identifier_is_greedy_when_digits) {
  auto lexer = make_lexer("{{id1234/}}");
  const std::vector<token_description> expected = {
      {tok::open, {}},
      {tok::identifier, "id1234"},
      {tok::slash, {}},
      {tok::close, {}},
      {tok::eof, {}},
  };
  auto actual = lexer.tokenize_all();
  EXPECT_THAT(actual, testing::ElementsAreArray(expected));
}

TEST_F(LexerTest, i64_with_identifier) {
  auto lexer = make_lexer("{{/01234id}}");
  const std::vector<token_description> expected = {
      {tok::open, {}},
      {tok::slash, {}},
      {tok::i64_literal, 1234},
      {tok::identifier, "id"},
      {tok::close, {}},
      {tok::eof, {}},
  };
  auto actual = lexer.tokenize_all();
  EXPECT_THAT(actual, testing::ElementsAreArray(expected));
}

TEST_F(LexerTest, i64_at_max) {
  auto lexer = make_lexer("{{9223372036854775807}}");
  const std::vector<token_description> expected = {
      {tok::open, {}},
      {tok::i64_literal, std::numeric_limits<std::int64_t>::max()},
      {tok::close, {}},
      {tok::eof, {}},
  };
  auto actual = lexer.tokenize_all();
  EXPECT_THAT(actual, testing::ElementsAreArray(expected));
}

TEST_F(LexerTest, i64_at_min) {
  auto lexer = make_lexer("{{-9223372036854775808}}");
  const std::vector<token_description> expected = {
      {tok::open, {}},
      {tok::i64_literal, std::numeric_limits<std::int64_t>::min()},
      {tok::close, {}},
      {tok::eof, {}},
  };
  auto actual = lexer.tokenize_all();
  EXPECT_THAT(actual, testing::ElementsAreArray(expected));
}

TEST_F(LexerTest, i64_past_max) {
  auto lexer = make_lexer("{{9223372036854775808}}");
  const std::vector<token_description> expected = {
      {tok::open, {}},
      {tok::error, {}},
  };
  auto actual = lexer.tokenize_all();
  EXPECT_THAT(actual, testing::ElementsAreArray(expected));
  EXPECT_THAT(
      diagnostics,
      testing::ElementsAre(diagnostic(
          diagnostic_level::error,
          "i64 literal out of range: 9223372036854775808",
          path_to_file(1),
          1)));
}

TEST_F(LexerTest, i64_past_max_more_digits) {
  auto lexer = make_lexer("{{9223372036854775807000}}");
  const std::vector<token_description> expected = {
      {tok::open, {}},
      {tok::error, {}},
  };
  auto actual = lexer.tokenize_all();
  EXPECT_THAT(actual, testing::ElementsAreArray(expected));
  EXPECT_THAT(
      diagnostics,
      testing::ElementsAre(diagnostic(
          diagnostic_level::error,
          "i64 literal out of range: 9223372036854775807000",
          path_to_file(1),
          1)));
}

TEST_F(LexerTest, i64_past_min_more_digits) {
  auto lexer = make_lexer("{{- 9223372036854775808000}}");
  const std::vector<token_description> expected = {
      {tok::open, {}},
      {tok::error, {}},
  };
  auto actual = lexer.tokenize_all();
  EXPECT_THAT(actual, testing::ElementsAreArray(expected));
  EXPECT_THAT(
      diagnostics,
      testing::ElementsAre(diagnostic(
          diagnostic_level::error,
          "i64 literal out of range: -9223372036854775808000",
          path_to_file(1),
          1)));
}

TEST_F(LexerTest, string_literal) {
  auto lexer = make_lexer(R"({{ "hello world" "second string" }})");
  const std::vector<token_description> expected = {
      {tok::open, {}},
      {tok::string_literal, "hello world"},
      {tok::string_literal, "second string"},
      {tok::close, {}},
      {tok::eof, {}},
  };
  auto actual = lexer.tokenize_all();
  EXPECT_THAT(actual, testing::ElementsAreArray(expected));
}

TEST_F(LexerTest, string_literal_empty) {
  auto lexer = make_lexer("{{ \"\" }}");
  const std::vector<token_description> expected = {
      {tok::open, {}},
      {tok::string_literal, ""},
      {tok::close, {}},
      {tok::eof, {}},
  };
  auto actual = lexer.tokenize_all();
  EXPECT_THAT(actual, testing::ElementsAreArray(expected));
}

TEST_F(LexerTest, string_literal_escapes) {
  auto lexer = make_lexer(R"({{ "hello \n \r\t \' \" world" }})");
  const std::vector<token_description> expected = {
      {tok::open, {}},
      {tok::string_literal, "hello \n \r\t ' \" world"},
      {tok::close, {}},
      {tok::eof, {}},
  };
  auto actual = lexer.tokenize_all();
  EXPECT_THAT(actual, testing::ElementsAreArray(expected));
}

TEST_F(LexerTest, string_literal_bad_escape) {
  auto lexer = make_lexer(R"({{ "hello \f world" }})");
  const std::vector<token_description> expected = {
      {tok::open, {}},
      {tok::error, {}},
  };
  auto actual = lexer.tokenize_all();
  EXPECT_THAT(actual, testing::ElementsAreArray(expected));
  EXPECT_THAT(
      diagnostics,
      testing::ElementsAre(diagnostic(
          diagnostic_level::error,
          "unknown escape character in string literal: '\\f'",
          path_to_file(1),
          1)));
}

TEST_F(LexerTest, string_literal_newline_in_middle) {
  auto lexer = make_lexer("{{ \"hello \n world\" }}");
  const std::vector<token_description> expected = {
      {tok::open, {}},
      {tok::error, {}},
  };
  auto actual = lexer.tokenize_all();
  EXPECT_THAT(actual, testing::ElementsAreArray(expected));
  EXPECT_THAT(
      diagnostics,
      testing::ElementsAre(diagnostic(
          diagnostic_level::error,
          "unexpected newline in string literal",
          path_to_file(1),
          1)));
}

TEST_F(LexerTest, string_literal_unterminated) {
  auto lexer = make_lexer("{{ \"hello }}");
  const std::vector<token_description> expected = {
      {tok::open, {}},
      {tok::error, {}},
  };
  auto actual = lexer.tokenize_all();
  EXPECT_THAT(actual, testing::ElementsAreArray(expected));
  EXPECT_THAT(
      diagnostics,
      testing::ElementsAre(diagnostic(
          diagnostic_level::error,
          "unterminated string literal",
          path_to_file(1),
          1)));
}

TEST_F(LexerTest, basic_comment) {
  auto lexer = make_lexer("{{!basic}}");
  const std::vector<token_description> expected = {
      {tok::open, {}},
      {tok::bang, {}},
      {tok::text, "basic"},
      {tok::close, {}},
      {tok::eof, {}},
  };
  auto actual = lexer.tokenize_all();
  EXPECT_THAT(actual, testing::ElementsAreArray(expected));
}

TEST_F(LexerTest, text_captures_whitespace) {
  auto lexer = make_lexer("{{!comment}} \n{{}}");
  const std::vector<token_description> expected = {
      {tok::open, {}},
      {tok::bang, {}},
      {tok::text, "comment"},
      {tok::close, {}},
      {tok::whitespace, " "},
      {tok::newline, "\n"},
      {tok::open, {}},
      {tok::close, {}},
      {tok::eof, {}},
  };
  auto actual = lexer.tokenize_all();
  EXPECT_THAT(actual, testing::ElementsAreArray(expected));
}

TEST_F(LexerTest, text_escapes_template) {
  auto lexer = make_lexer("\\{{}} \\{{{foo}}");
  const std::vector<token_description> expected = {
      {tok::text, "{{}}"},
      {tok::whitespace, " "},
      {tok::text, "{"},
      {tok::open, {}},
      {tok::identifier, "foo"},
      {tok::close, {}},
      {tok::eof, {}},
  };
  auto actual = lexer.tokenize_all();
  EXPECT_THAT(actual, testing::ElementsAreArray(expected));
}

TEST_F(LexerTest, basic_comment_and_text) {
  auto lexer = make_lexer("hello {{! some comment}} and back to text");
  const std::vector<token_description> expected = {
      {tok::text, "hello"},
      {tok::whitespace, " "},
      {tok::open, {}},
      {tok::bang, {}},
      {tok::text, " some comment"},
      {tok::close, {}},
      {tok::whitespace, " "},
      {tok::text, "and"},
      {tok::whitespace, " "},
      {tok::text, "back"},
      {tok::whitespace, " "},
      {tok::text, "to"},
      {tok::whitespace, " "},
      {tok::text, "text"},
      {tok::eof, {}},
  };
  auto actual = lexer.tokenize_all();
  EXPECT_THAT(actual, testing::ElementsAreArray(expected));
}

TEST_F(LexerTest, empty_comment) {
  auto lexer = make_lexer("{{!}}");
  const std::vector<token_description> expected = {
      {tok::open, {}},
      {tok::bang, {}},
      // no text token
      {tok::close, {}},
      {tok::eof, {}},
  };
  auto actual = lexer.tokenize_all();
  EXPECT_THAT(actual, testing::ElementsAreArray(expected));
}

TEST_F(LexerTest, escaped_comment) {
  auto lexer = make_lexer("hello {{!-- some comment}} --}} and back to text");
  const std::vector<token_description> expected = {
      {tok::text, "hello"},
      {tok::whitespace, " "},
      {tok::open, {}},
      {tok::bang, {}},
      {tok::text, " some comment}} "},
      {tok::close, {}},
      {tok::whitespace, " "},
      {tok::text, "and"},
      {tok::whitespace, " "},
      {tok::text, "back"},
      {tok::whitespace, " "},
      {tok::text, "to"},
      {tok::whitespace, " "},
      {tok::text, "text"},
      {tok::eof, {}},
  };
  auto actual = lexer.tokenize_all();
  EXPECT_THAT(actual, testing::ElementsAreArray(expected));
}

TEST_F(LexerTest, empty_escaped_comment) {
  auto lexer = make_lexer("{{!----}}");
  const std::vector<token_description> expected = {
      {tok::open, {}},
      {tok::bang, {}},
      // no text token
      {tok::close, {}},
      {tok::eof, {}},
  };
  auto actual = lexer.tokenize_all();
  EXPECT_THAT(actual, testing::ElementsAreArray(expected));
}

TEST_F(LexerTest, almost_escaped_comment) {
  auto lexer = make_lexer("hello {{!- not escaped comment }} and back to text");
  const std::vector<token_description> expected = {
      {tok::text, "hello"},
      {tok::whitespace, " "},
      {tok::open, {}},
      {tok::bang, {}},
      {tok::text, "- not escaped comment "},
      {tok::close, {}},
      {tok::whitespace, " "},
      {tok::text, "and"},
      {tok::whitespace, " "},
      {tok::text, "back"},
      {tok::whitespace, " "},
      {tok::text, "to"},
      {tok::whitespace, " "},
      {tok::text, "text"},
      {tok::eof, {}},
  };
  auto actual = lexer.tokenize_all();
  EXPECT_THAT(actual, testing::ElementsAreArray(expected));
}

TEST_F(LexerTest, unterminated_comment) {
  auto lexer = make_lexer("hello {{! this is not closed");
  const std::vector<token_description> expected = {
      {tok::text, "hello"},
      {tok::whitespace, " "},
      {tok::open, {}},
      {tok::bang, {}},
      {tok::text, " this is not closed"},
      {tok::eof, {}},
  };
  auto actual = lexer.tokenize_all();
  EXPECT_THAT(actual, testing::ElementsAreArray(expected));
}

TEST_F(LexerTest, source_ranges) {
  auto lexer = make_lexer(
      "some text{{! comment }}\r\n"
      "and\t{{# variable ^ ! \"string\"\n"
      "- 12345 if}}\t More text\n");
  const std::vector<token_description> expected = {
      {tok::text, "some"},
      {tok::whitespace, " "},
      {tok::text, "text"},
      {tok::open, {}},
      {tok::bang, {}},
      {tok::text, " comment "},
      {tok::close, {}},
      {tok::newline, "\r\n"},
      {tok::text, "and"},
      {tok::whitespace, "\t"},
      {tok::open, {}},
      {tok::pound, {}},
      {tok::identifier, "variable"},
      {tok::caret, {}},
      {tok::bang, {}},
      {tok::string_literal, "string"},
      {tok::i64_literal, -12345},
      {tok::kw_if, {}},
      {tok::close, {}},
      {tok::whitespace, "\t "},
      {tok::text, "More"},
      {tok::whitespace, " "},
      {tok::text, "text"},
      {tok::newline, "\n"},
      {tok::eof, {}},
  };
  auto actual = lexer.tokenize_all();
  EXPECT_THAT(actual, testing::ElementsAreArray(expected));

  auto needle = actual.cbegin();
  EXPECT_EQ(token_range_text(*needle++), "some");
  EXPECT_EQ(token_range_text(*needle++), " ");
  EXPECT_EQ(token_range_text(*needle++), "text");
  EXPECT_EQ(token_range_text(*needle++), "{{");
  EXPECT_EQ(token_range_text(*needle++), "!");
  EXPECT_EQ(token_range_text(*needle++), " comment ");
  EXPECT_EQ(token_range_text(*needle++), "}}");
  EXPECT_EQ(token_range_text(*needle++), "\r\n");
  EXPECT_EQ(token_range_text(*needle++), "and");
  EXPECT_EQ(token_range_text(*needle++), "\t");
  EXPECT_EQ(token_range_text(*needle++), "{{");
  EXPECT_EQ(token_range_text(*needle++), "#");
  EXPECT_EQ(token_range_text(*needle++), "variable");
  EXPECT_EQ(token_range_text(*needle++), "^");
  EXPECT_EQ(token_range_text(*needle++), "!");
  EXPECT_EQ(token_range_text(*needle++), "\"string\"");
  EXPECT_EQ(token_range_text(*needle++), "- 12345");
  EXPECT_EQ(token_range_text(*needle++), "if");
  EXPECT_EQ(token_range_text(*needle++), "}}");
  EXPECT_EQ(token_range_text(*needle++), "\t ");
  EXPECT_EQ(token_range_text(*needle++), "More");
  EXPECT_EQ(token_range_text(*needle++), " ");
  EXPECT_EQ(token_range_text(*needle++), "text");
  EXPECT_EQ(token_range_text(*needle++), "\n");
  EXPECT_EQ(token_range_text(*needle++), "");
}

} // namespace whisker
