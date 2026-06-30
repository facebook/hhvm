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

#include <algorithm>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

#include <gtest/gtest.h>

#include <folly/testing/TestUtil.h>
#include <thrift/compiler/diagnostic.h>
#include <thrift/compiler/generate/formatter.h>
#include <thrift/compiler/parse/lexer.h>
#include <thrift/compiler/parse/parse_ast.h>
#include <thrift/compiler/parse/token.h>

using apache::thrift::compiler::diagnostic;
using apache::thrift::compiler::diagnostic_params;
using apache::thrift::compiler::diagnostic_results;
using apache::thrift::compiler::diagnostics_engine;
using apache::thrift::compiler::format_thrift_source;
using apache::thrift::compiler::lexer;
using apache::thrift::compiler::parse_ast;
using apache::thrift::compiler::source_manager;
using apache::thrift::compiler::source_range;
using apache::thrift::compiler::tok;
using apache::thrift::compiler::trivia_kind;

namespace {

std::string read_file(const std::filesystem::path& path) {
  std::ifstream input(path);
  return {
      std::istreambuf_iterator<char>(input), std::istreambuf_iterator<char>()};
}

void write_file(const std::filesystem::path& path, std::string_view text) {
  std::ofstream output(path);
  output << text;
}

std::vector<std::string> comment_texts(std::string_view source) {
  source_manager source_mgr;
  const auto source_view = source_mgr.add_virtual_file("test.thrift", source);
  diagnostic_results results;
  diagnostics_engine diags(source_mgr, results, diagnostic_params::strict());
  std::vector<std::string> comments;
  auto on_trivia = [&](trivia_kind kind, source_range range) {
    if (kind == trivia_kind::line_comment ||
        kind == trivia_kind::block_comment ||
        kind == trivia_kind::doc_comment) {
      comments.emplace_back(source_mgr.get_text_range(range));
    }
  };

  lexer lex(
      source_view,
      diags,
      [](std::string_view, source_range) {},
      std::move(on_trivia));
  while (lex.get_next_token().kind != tok::eof) {
  }
  if (results.has_error()) {
    throw std::runtime_error(results.diagnostics().front().str());
  }
  return comments;
}

void expect_comments_retained(std::string_view source) {
  const std::vector<std::string> before = comment_texts(source);
  const std::string formatted = format_thrift_source(source);
  EXPECT_EQ(comment_texts(formatted), before);
  EXPECT_EQ(format_thrift_source(formatted), formatted);
}

void format_file_in_place(const std::filesystem::path& path) {
  write_file(path, format_thrift_source(read_file(path)));
}

std::optional<std::filesystem::path> find_fixtures_root() {
  constexpr const char* env_var = "THRIFT_COMPILER_TEST_FIXTURES";
  const char* value = std::getenv(env_var);
  if (value == nullptr || std::string_view(value).empty()) {
    return std::nullopt;
  }

  std::filesystem::path path(value);
  if (!std::filesystem::is_directory(path)) {
    throw std::runtime_error(
        std::string(env_var) +
        " does not point to a directory: " + path.string());
  }
  return std::filesystem::canonical(path);
}

std::vector<std::filesystem::path> fixture_thrift_files(
    const std::filesystem::path& fixtures_root) {
  std::vector<std::filesystem::path> files;
  for (const auto& entry :
       std::filesystem::recursive_directory_iterator(fixtures_root)) {
    if (!entry.is_regular_file() || entry.path().extension() != ".thrift") {
      continue;
    }
    if (entry.path().parent_path().filename() != "src") {
      continue;
    }
    files.push_back(entry.path());
  }
  std::sort(files.begin(), files.end());
  return files;
}

void expect_format(std::string_view input, std::string_view expected) {
  EXPECT_EQ(format_thrift_source(input), expected);
}

TEST(FormatterTest, formats_basic_headers_and_definitions) {
  expect_format(
      R"(include    'foo'
  namespace    safe   bar
)",
      R"(include 'foo'
namespace safe bar
)");

  expect_format(
      R"(include 'foo'
            as
      bar
      ;
)",
      R"(include 'foo' as bar
)");

  expect_format(
      R"(const string
    foo = 'foo'

;
const
  i32 answer = 42
)",
      R"(const string foo = 'foo';
const i32 answer = 42;
)");

  expect_format(
      R"(typedef string
 ( hs.a1 = 1 , hs.a2 = 2) Name
 ( hs.a3 = 3)
typedef list<i64> Ids
)",
      R"(typedef string (hs.a1 = 1, hs.a2 = 2) Name (hs.a3 = 3)
typedef list<i64> Ids
)");
}

TEST(FormatterTest, formats_basic_types_and_services) {
  expect_format(
      R"(enum
  Color { RED,
BLUE = 2
}
)",
      R"(enum Color {
  RED,
  BLUE = 2,
}
)");

  expect_format(
      R"(struct Example {
1: optional string (hs.a1 = 1) name
2: required list<i32> values = [ 1 ,2,
  3]
}
)",
      R"(struct Example {
  1: optional string (hs.a1 = 1) name;
  2: required list<i32> values = [1, 2, 3];
}
)");

  expect_format(
      R"(service Foo {
oneway void bar () throws (1: string ex1)
  void    baz (1: string arg1)
    throws (1: string field1)
}
service Empty extends
  Base
   { }
)",
      R"(service Foo {
  oneway void bar() throws (1: string ex1);
  void baz(1: string arg1) throws (1: string field1);
}
service Empty extends Base {
}
)");
}

TEST(FormatterTest, preserves_comments_and_round_trips) {
  const std::string source = R"(package "facebook.com/thrift/test"

// leading
struct S {
  1: i32 field; // trailing
}
)";

  folly::test::TemporaryDirectory dir;
  const auto thrift_file =
      std::filesystem::path(dir.path().string()) / "test.thrift";
  write_file(thrift_file, source);
  format_file_in_place(thrift_file);

  const std::string formatted = read_file(thrift_file);
  EXPECT_EQ(formatted, source);

  source_manager roundtrip_mgr;
  roundtrip_mgr.add_virtual_file("test.thrift", formatted);
  auto diags = diagnostics_engine(roundtrip_mgr, [](const diagnostic&) {});
  auto programs = parse_ast(roundtrip_mgr, diags, "test.thrift", {});
  EXPECT_FALSE(diags.has_errors());
  EXPECT_NE(programs, nullptr);

  format_file_in_place(thrift_file);
  const std::string formatted_again = read_file(thrift_file);
  EXPECT_EQ(formatted_again, formatted);
}

TEST(FormatterTest, retains_comments_around_concrete_tokens) {
  expect_comments_retained(R"(package "facebook.com/thrift/test"

// before const
const map<string, i32> Values = {
  // before key
  "one": 1, // after map entry
  // before map close
};

/* before struct */
@hs.Struct{name = "S"} // after leading annotation
struct S {
  // before field
  1: list<
    // before type arg
    string
  > names = [
    // before list value
    "a", // after list value
    // before list close
  ]; // after field
  // before struct close
}

service Service {
  // before function
  void f(
    // before param
    1: string arg, // after param
    // before params close
  ) throws (
    // before throws field
    1: string ex, // after throws field
    // before throws close
  ); // after function
  // before service close
}
)");
}

TEST(FormatterTest, currentBehaviorMergesFieldNameAfterTypeTrailingComment) {
  expect_format(
      R"(struct Foo {
  1: Bar // This is a long bar comment
        bar;
}
)",
      R"(struct Foo {
  1: Bar // This is a long bar comment bar;
}
)");
}

TEST(FormatterTest, currentBehaviorDropsCommentBeforeServiceExtends) {
  // The formatter throws rather than emitting output that would drop trivia.
  EXPECT_THROW(
      format_thrift_source(R"(service Foo /* Hello there! */ extends Bar {
}
)"),
      std::runtime_error);
}

TEST(FormatterTest, currentBehaviorMovesSpaceBeforeFieldSeparatorBlockComment) {
  expect_format(
      R"(struct Foo {
  1: i32 field /* trailing block */;
}
)",
      R"(struct Foo {
  1: i32 field/* trailing block */ ;
}
)");
}

TEST(FormatterTest, preserves_source_with_missing_include_and_unresolved_type) {
  const std::string source = R"(package "facebook.com/thrift/test"

include "missing.thrift"

struct S {
  1: missing.Missing field;
}
)";

  folly::test::TemporaryDirectory dir;
  const auto thrift_file =
      std::filesystem::path(dir.path().string()) / "test.thrift";
  write_file(thrift_file, source);
  format_file_in_place(thrift_file);

  const std::string formatted = read_file(thrift_file);
  EXPECT_EQ(formatted, source);
}

TEST(FormatterTest, rejects_parser_invalid_source) {
  EXPECT_THROW(
      format_thrift_source(R"(struct S { 1: list<i32 field; })"),
      std::runtime_error);
}

TEST(FormatterTest, fixture_files_round_trip) {
  const auto fixtures_root = find_fixtures_root();
  if (!fixtures_root) {
    GTEST_SKIP() << "THRIFT_COMPILER_TEST_FIXTURES is not set";
  }

  const auto fixture_files = fixture_thrift_files(*fixtures_root);
  ASSERT_FALSE(fixture_files.empty());

  for (const auto& fixture_file : fixture_files) {
    SCOPED_TRACE(fixture_file.string());

    folly::test::TemporaryDirectory dir;
    const auto scratch_file =
        std::filesystem::path(dir.path().string()) / fixture_file.filename();
    write_file(scratch_file, read_file(fixture_file));
    const std::vector<std::string> source_comments =
        comment_texts(read_file(scratch_file));
    format_file_in_place(scratch_file);

    const std::string formatted = read_file(scratch_file);
    EXPECT_EQ(comment_texts(formatted), source_comments);
    EXPECT_EQ(format_thrift_source(formatted), formatted);
  }
}

} // namespace
