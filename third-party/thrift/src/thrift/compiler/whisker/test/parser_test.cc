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

#include <folly/portability/GMock.h>
#include <folly/portability/GTest.h>

#include <thrift/compiler/whisker/ast.h>
#include <thrift/compiler/whisker/diagnostic.h>
#include <thrift/compiler/whisker/parser.h>
#include <thrift/compiler/whisker/print_ast.h>
#include <thrift/compiler/whisker/source_location.h>

#include <optional>
#include <sstream>
#include <string>
#include <vector>

#include <fmt/core.h>

namespace whisker {

class ParserTest : public testing::Test {
 private:
  source_manager src_manager;
  diagnostics_engine diags;
  int file_id = 1;

 public:
  std::vector<diagnostic> diagnostics;

  static std::string path_to_file(int id) {
    return fmt::format("path/to/test-{}.whisker", id);
  }

  std::optional<ast::root> parse_ast(const std::string& source) {
    diagnostics.clear();
    return parse(
        src_manager.add_virtual_file(path_to_file(file_id++), source), diags);
  }

  std::string to_string(const ast::root& ast) {
    std::ostringstream out;
    print_ast(ast, src_manager, out);
    return out.str();
  }

  ParserTest()
      : diags(src_manager, [this](diagnostic d) {
          diagnostics.push_back(std::move(d));
        }) {}
};

TEST_F(ParserTest, empty) {
  auto ast = parse_ast("");
  EXPECT_TRUE(ast->body_elements.empty());
}

TEST_F(ParserTest, basic) {
  auto ast = parse_ast("Some text {{foo.bar}} more text");
  EXPECT_EQ(
      to_string(*ast),
      "root [path/to/test-1.whisker]\n"
      "|- text <line:1:1, col:11> 'Some text '\n"
      "|- variable <line:1:11, col:22> 'foo.bar'\n"
      "|- text <line:1:22, col:32> ' more text'\n");
}

TEST_F(ParserTest, empty_template) {
  auto ast = parse_ast("{{ }}");
  EXPECT_FALSE(ast.has_value());
  EXPECT_THAT(
      diagnostics,
      testing::ElementsAre(diagnostic(
          diagnostic_level::error,
          "expected variable-lookup in variable but found `}}`",
          path_to_file(1),
          1)));
}

TEST_F(ParserTest, variable_is_single_id) {
  auto ast = parse_ast("{{ foo }}");
  EXPECT_EQ(
      to_string(*ast),
      "root [path/to/test-1.whisker]\n"
      "|- variable <line:1:1, col:10> 'foo'\n");
}

TEST_F(ParserTest, variable_is_multi_id) {
  auto ast = parse_ast("{{ foo. bar.baz }}");
  EXPECT_EQ(
      to_string(*ast),
      "root [path/to/test-1.whisker]\n"
      "|- variable <line:1:1, col:19> 'foo.bar.baz'\n");
}

TEST_F(ParserTest, variable_is_dot) {
  auto ast = parse_ast("{{. }}");
  EXPECT_EQ(
      to_string(*ast),
      "root [path/to/test-1.whisker]\n"
      "|- variable <line:1:1, col:7> '.'\n");
}

TEST_F(ParserTest, variable_starts_with_dot) {
  auto ast = parse_ast("{{ .foo }}");
  EXPECT_FALSE(ast.has_value());
  EXPECT_EQ(diagnostics.size(), 1);
  EXPECT_EQ(
      diagnostics[0].message(),
      "expected `}}` to close variable but found identifier");
}

TEST_F(ParserTest, variable_has_extra_stuff_after) {
  auto ast = parse_ast("{{foo.bar!}}");
  EXPECT_FALSE(ast.has_value());
  EXPECT_THAT(
      diagnostics,
      testing::ElementsAre(diagnostic(
          diagnostic_level::error,
          "expected `}}` to close variable but found `!`",
          path_to_file(1),
          1)));
}

TEST_F(ParserTest, basic_section) {
  auto ast = parse_ast(
      "{{ #news.has-update?}}\n"
      "  Stuff is {{foo}} happening!\n"
      "{{/ news.has-update?}}");
  EXPECT_EQ(
      to_string(*ast),
      "root [path/to/test-1.whisker]\n"
      "|- section-block <line:1:1, line:3:23>\n"
      "| `- variable-lookup <line:1:5, col:21> 'news.has-update?'\n"
      "| |- text <line:2:1, col:12> '  Stuff is '\n"
      "| |- variable <line:2:12, col:19> 'foo'\n"
      "| |- text <line:2:19, col:30> ' happening!'\n"
      "| |- newline <line:2:30, line:3:1> '\\n'\n");
}

TEST_F(ParserTest, inverted_section) {
  auto ast = parse_ast(
      "{{^news.has-update?}}\n"
      "  Stuff is {{foo}} happening!\n"
      "{{/news.has-update?}}");
  EXPECT_EQ(
      to_string(*ast),
      "root [path/to/test-1.whisker]\n"
      "|- section-block <inverted> <line:1:1, line:3:22>\n"
      "| `- variable-lookup <line:1:4, col:20> 'news.has-update?'\n"
      "| |- text <line:2:1, col:12> '  Stuff is '\n"
      "| |- variable <line:2:12, col:19> 'foo'\n"
      "| |- text <line:2:19, col:30> ' happening!'\n"
      "| |- newline <line:2:30, line:3:1> '\\n'\n");
}

TEST_F(ParserTest, nested_sections) {
  auto ast = parse_ast(
      "{{#news.has-update?}}\n"
      "  {{^update.is-important?}}\n"
      "    Important stuff is {{foo}} happening!\n"
      "  {{/update.is-important?}}\n"
      "{{/news.has-update?}}");
  EXPECT_EQ(
      to_string(*ast),
      "root [path/to/test-1.whisker]\n"
      "|- section-block <line:1:1, line:5:22>\n"
      "| `- variable-lookup <line:1:4, col:20> 'news.has-update?'\n"
      "| |- section-block <inverted> <line:2:3, line:4:28>\n"
      "| | `- variable-lookup <line:2:6, col:26> 'update.is-important?'\n"
      "| | |- text <line:3:1, col:24> '    Important stuff is '\n"
      "| | |- variable <line:3:24, col:31> 'foo'\n"
      "| | |- text <line:3:31, col:42> ' happening!'\n"
      "| | |- newline <line:3:42, line:4:1> '\\n'\n");
}

TEST_F(ParserTest, mismatched_section_hierarchy) {
  auto ast = parse_ast(
      "{{#news.has-update?}}\n"
      "  {{#update.is-important?}}\n"
      "    Important stuff is {{foo}} happening!\n"
      "    {{#inner}}{{/inner}}\n"
      "  {{/news.has-update?}}\n"
      "{{/update.is-important?}}");
  EXPECT_FALSE(ast.has_value());
  EXPECT_THAT(
      diagnostics,
      testing::ElementsAre(diagnostic(
          diagnostic_level::error,
          "section-block opening 'update.is-important?' does not match closing 'news.has-update?'",
          path_to_file(1),
          5)));
}

TEST_F(ParserTest, section_open_by_itself) {
  auto ast = parse_ast("{{#news.has-update?}}");
  EXPECT_FALSE(ast.has_value());
  EXPECT_THAT(
      diagnostics,
      testing::ElementsAre(diagnostic(
          diagnostic_level::error,
          "expected `{{` to close section-block 'news.has-update?' but found EOF",
          path_to_file(1),
          1)));
}

TEST_F(ParserTest, section_with_bad_close) {
  // keyword to close
  {
    auto ast = parse_ast("{{#news.has-update?}}{{/true}}");
    EXPECT_FALSE(ast.has_value());
    EXPECT_THAT(
        diagnostics,
        testing::ElementsAre(diagnostic(
            diagnostic_level::error,
            "expected variable-lookup to close section-block 'news.has-update?' but found `true`",
            path_to_file(1),
            1)));
  }
  // missing }}
  {
    auto ast = parse_ast("{{#news.has-update?}}{{/ news.has-update?");
    EXPECT_FALSE(ast.has_value());
    EXPECT_THAT(
        diagnostics,
        testing::ElementsAre(diagnostic(
            diagnostic_level::error,
            "expected `}}` to close section-block 'news.has-update?'",
            path_to_file(2),
            1)));
  }
  // mismatch + missing close
  {
    auto ast = parse_ast("{{#news.has-update?}}{{/ foo-bar?");
    EXPECT_FALSE(ast.has_value());
    EXPECT_THAT(
        diagnostics,
        testing::ElementsAre(
            diagnostic(
                diagnostic_level::error,
                "section-block opening 'news.has-update?' does not match closing 'foo-bar?'",
                path_to_file(3),
                1),
            diagnostic(
                diagnostic_level::error,
                "expected `}}` to close section-block 'news.has-update?'",
                path_to_file(3),
                1)));
  }
}

TEST_F(ParserTest, section_close_by_itself) {
  auto ast = parse_ast("{{/news.has-update?}}");
  EXPECT_FALSE(ast.has_value());
  EXPECT_THAT(
      diagnostics,
      testing::ElementsAre(diagnostic(
          diagnostic_level::error,
          "expected text, template, or comment but found `{{`",
          path_to_file(1),
          1)));
}

TEST_F(ParserTest, basic_if) {
  auto ast = parse_ast(
      "{{#if news.has-update?}}\n"
      "  Stuff is {{foo}} happening!\n"
      "{{/if news.has-update?}}");
  EXPECT_EQ(
      to_string(*ast),
      "root [path/to/test-1.whisker]\n"
      "|- if-block <line:1:1, line:3:25>\n"
      "| `- variable-lookup <line:1:7, col:23> 'news.has-update?'\n"
      "| |- text <line:2:1, col:12> '  Stuff is '\n"
      "| |- variable <line:2:12, col:19> 'foo'\n"
      "| |- text <line:2:19, col:30> ' happening!'\n"
      "| |- newline <line:2:30, line:3:1> '\\n'\n");
}

TEST_F(ParserTest, basic_if_else) {
  auto ast = parse_ast(
      "{{#if news.has-update?}}\n"
      "  Stuff is {{foo}} happening!\n"
      "{{#else}}\n"
      "  Nothing is happening!\n"
      "{{/if news.has-update?}}");
  EXPECT_EQ(
      to_string(*ast),
      "root [path/to/test-1.whisker]\n"
      "|- if-block <line:1:1, line:5:25>\n"
      "| `- variable-lookup <line:1:7, col:23> 'news.has-update?'\n"
      "| |- text <line:2:1, col:12> '  Stuff is '\n"
      "| |- variable <line:2:12, col:19> 'foo'\n"
      "| |- text <line:2:19, col:30> ' happening!'\n"
      "| |- newline <line:2:30, line:3:1> '\\n'\n"
      "| `- else-block <line:3:1, line:5:1>\n"
      "|   |- text <line:4:1, col:24> '  Nothing is happening!'\n"
      "|   |- newline <line:4:24, line:5:1> '\\n'\n");
}

TEST_F(ParserTest, unless_block_repeated_else) {
  auto ast = parse_ast(
      "{{#unless news.has-update?}}\n"
      "  Stuff is {{foo}} happening!\n"
      "{{#else}}\n"
      "  Nothing is happening!\n"
      "{{#else}}\n"
      "  Nothing is happening!\n"
      "{{/unless news.has-update?}}");
  EXPECT_FALSE(ast.has_value());
  EXPECT_THAT(
      diagnostics,
      testing::ElementsAre(diagnostic(
          diagnostic_level::error,
          "expected `/` to close unless-block 'news.has-update?' but found `#`",
          path_to_file(1),
          5)));
}

TEST_F(ParserTest, if_block_else_by_itself) {
  auto ast = parse_ast("{{#else}}");
  EXPECT_FALSE(ast.has_value());
  EXPECT_THAT(
      diagnostics,
      testing::ElementsAre(diagnostic(
          diagnostic_level::error,
          "expected text, template, or comment but found dangling else-clause",
          path_to_file(1),
          1)));
}

TEST_F(ParserTest, if_block_unclosed_with_else) {
  auto ast = parse_ast("{{#if news.has-update?}}{{#else}}");
  EXPECT_FALSE(ast.has_value());
  EXPECT_THAT(
      diagnostics,
      testing::ElementsAre(diagnostic(
          diagnostic_level::error,
          "expected `{{` to close if-block 'news.has-update?' but found EOF",
          path_to_file(1),
          1)));
}

TEST_F(ParserTest, if_block_nested) {
  auto ast = parse_ast(
      "{{#if news.has-update?}}\n"
      "  {{#if update.is-important?}}\n"
      "    Important stuff is {{foo}} happening!\n"
      "  {{#else}}\n"
      "    Boring stuff is happening!\n"
      "  {{/if update.is-important?}}\n"
      "{{/if news.has-update?}}");
  EXPECT_EQ(
      to_string(*ast),
      "root [path/to/test-1.whisker]\n"
      "|- if-block <line:1:1, line:7:25>\n"
      "| `- variable-lookup <line:1:7, col:23> 'news.has-update?'\n"
      "| |- if-block <line:2:3, line:6:31>\n"
      "| | `- variable-lookup <line:2:9, col:29> 'update.is-important?'\n"
      "| | |- text <line:3:1, col:24> '    Important stuff is '\n"
      "| | |- variable <line:3:24, col:31> 'foo'\n"
      "| | |- text <line:3:31, col:42> ' happening!'\n"
      "| | |- newline <line:3:42, line:4:1> '\\n'\n"
      "| | `- else-block <line:4:3, line:6:3>\n"
      "| |   |- text <line:5:1, col:31> '    Boring stuff is happening!'\n"
      "| |   |- newline <line:5:31, line:6:1> '\\n'\n");
}

TEST_F(ParserTest, mismatched_if_hierarchy) {
  auto ast = parse_ast(
      "{{#if news.has-update?}}\n"
      "  {{#update.is-important?}}\n"
      "    Important stuff is {{foo}} happening!\n"
      "    {{#inner}}{{/inner}}\n"
      "  {{/if news.has-update?}}\n"
      "{{/update.is-important?}}");
  EXPECT_FALSE(ast.has_value());
  EXPECT_THAT(
      diagnostics,
      testing::ElementsAre(diagnostic(
          diagnostic_level::error,
          "expected variable-lookup to close section-block 'update.is-important?' but found `if`",
          path_to_file(1),
          5)));
}

TEST_F(ParserTest, if_by_itself) {
  auto ast = parse_ast("{{#if}}");
  EXPECT_FALSE(ast.has_value());
  EXPECT_THAT(
      diagnostics,
      testing::ElementsAre(diagnostic(
          diagnostic_level::error,
          "expected variable-lookup to open if-block but found `}}`",
          path_to_file(1),
          1)));
}

TEST_F(ParserTest, if_close_by_itself) {
  auto ast = parse_ast("{{/if}}");
  EXPECT_FALSE(ast.has_value());
  EXPECT_THAT(
      diagnostics,
      testing::ElementsAre(diagnostic(
          diagnostic_level::error,
          "expected text, template, or comment but found `{{`",
          path_to_file(1),
          1)));
}

TEST_F(ParserTest, conditional_block_mismatched_open_and_close) {
  auto ast = parse_ast(
      "{{#unless news.has-update?}}\n"
      "  Stuff is happening!\n"
      "{{#else}}\n"
      "  Nothing is happening!\n"
      "{{/if}}");
  EXPECT_FALSE(ast.has_value());
  EXPECT_THAT(
      diagnostics,
      testing::ElementsAre(diagnostic(
          diagnostic_level::error,
          "expected `unless` to close unless-block 'news.has-update?' but found `if`",
          path_to_file(1),
          5)));
}

TEST_F(ParserTest, conditional_block_missing_close_lookup) {
  auto ast = parse_ast(
      "{{#if news.has-update?}}\n"
      "  Stuff is happening!\n"
      "{{#else}}\n"
      "  Nothing is happening!\n"
      "{{/if}}");
  EXPECT_FALSE(ast.has_value());
  EXPECT_THAT(
      diagnostics,
      testing::ElementsAre(diagnostic(
          diagnostic_level::error,
          "expected variable-lookup to close if-block 'news.has-update?' but found `}}`",
          path_to_file(1),
          5)));
}

TEST_F(ParserTest, conditional_block_mismatched_lookup) {
  auto ast = parse_ast(
      "{{#if news.has-update?}}\n"
      "  Stuff is happening!\n"
      "{{#else}}\n"
      "  Nothing is happening!\n"
      "{{/if news.has_updates?}}");
  EXPECT_FALSE(ast.has_value());
  EXPECT_THAT(
      diagnostics,
      testing::ElementsAre(diagnostic(
          diagnostic_level::error,
          "conditional-block opening 'news.has-update?' does not match closing 'news.has_updates?'",
          path_to_file(1),
          5)));
}

TEST_F(ParserTest, basic_partial_apply) {
  auto ast = parse_ast("{{> path / to / file }}");
  EXPECT_EQ(
      to_string(*ast),
      "root [path/to/test-1.whisker]\n"
      "|- partial-apply <line:1:1, col:24> 'path/to/file'\n"
      "| `- standalone-offset ''\n");
}

TEST_F(ParserTest, partial_apply_single_id) {
  auto ast = parse_ast("{{ > foo }}");
  EXPECT_EQ(
      to_string(*ast),
      "root [path/to/test-1.whisker]\n"
      "|- partial-apply <line:1:1, col:12> 'foo'\n"
      "| `- standalone-offset ''\n");
}

TEST_F(ParserTest, partial_apply_in_section) {
  auto ast = parse_ast(
      "{{#news.has-update?}}\n"
      "  {{ > print/news}}\n"
      "{{/news.has-update?}}");
  EXPECT_EQ(
      to_string(*ast),
      "root [path/to/test-1.whisker]\n"
      "|- section-block <line:1:1, line:3:22>\n"
      "| `- variable-lookup <line:1:4, col:20> 'news.has-update?'\n"
      "| |- text <line:2:1, col:3> '  '\n"
      "| |- partial-apply <line:2:3, col:20> 'print/news'\n"
      "| | `- standalone-offset '  '\n");
}

TEST_F(ParserTest, partial_apply_preserves_whitespace_offset) {
  auto ast = parse_ast(" \t {{ > print/news}}\n");
  EXPECT_EQ(
      to_string(*ast),
      "root [path/to/test-1.whisker]\n"
      "|- text <line:1:1, col:4> ' \\t '\n"
      "|- partial-apply <line:1:4, col:21> 'print/news'\n"
      "| `- standalone-offset ' \\t '\n");
}

TEST_F(ParserTest, partial_apply_no_id) {
  auto ast = parse_ast("{{> }}");
  EXPECT_FALSE(ast.has_value());
  EXPECT_THAT(
      diagnostics,
      testing::ElementsAre(diagnostic(
          diagnostic_level::error,
          "expected partial-lookup in partial-apply but found `}}`",
          path_to_file(1),
          1)));
}

TEST_F(ParserTest, partial_apply_extra_stuff) {
  auto ast = parse_ast("{{ > foo ! }}");
  EXPECT_FALSE(ast.has_value());
  EXPECT_THAT(
      diagnostics,
      testing::ElementsAre(diagnostic(
          diagnostic_level::error,
          "expected `}}` to close partial-apply 'foo' but found `!`",
          path_to_file(1),
          1)));
}

TEST_F(ParserTest, partial_apply_dotted_path) {
  auto ast = parse_ast("{{> path/to.file }}");
  EXPECT_EQ(
      to_string(*ast),
      "root [path/to/test-1.whisker]\n"
      "|- partial-apply <line:1:1, col:20> 'path/to.file'\n"
      "| `- standalone-offset ''\n");
}

TEST_F(ParserTest, partial_apply_empty_path_part) {
  auto ast = parse_ast("{{> path//to.file }}");
  EXPECT_FALSE(ast.has_value());
  EXPECT_THAT(
      diagnostics,
      testing::ElementsAre(diagnostic(
          diagnostic_level::error,
          "expected path-component in partial-lookup but found `/`",
          path_to_file(1),
          1)));
}

TEST_F(ParserTest, unclosed_partial_apply) {
  auto ast = parse_ast("{{> path/to/file");
  EXPECT_FALSE(ast.has_value());
  EXPECT_THAT(
      diagnostics,
      testing::ElementsAre(diagnostic(
          diagnostic_level::error,
          "expected `}}` to close partial-apply 'path/to/file' but found EOF",
          path_to_file(1),
          1)));
}

TEST_F(ParserTest, comment) {
  auto ast = parse_ast("Hello{{! #$^& random text }}world");

  EXPECT_EQ(
      to_string(*ast),
      "root [path/to/test-1.whisker]\n"
      "|- text <line:1:1, col:6> 'Hello'\n"
      "|- comment <line:1:6, col:29> ' #$^& random text '\n"
      "|- text <line:1:29, col:34> 'world'\n");
}

TEST_F(ParserTest, comment_empty) {
  auto ast = parse_ast("{{!}}");
  EXPECT_EQ(
      to_string(*ast),
      "root [path/to/test-1.whisker]\n"
      "|- comment <line:1:1, col:6> ''\n");
}

TEST_F(ParserTest, comment_escaped) {
  auto ast = parse_ast(
      "Hello{{!-- \n"
      "next line }} still comment --}}world");
  EXPECT_EQ(
      to_string(*ast),
      "root [path/to/test-1.whisker]\n"
      "|- text <line:1:1, col:6> 'Hello'\n"
      "|- comment <line:1:6, line:2:32> ' \\nnext line }} still comment '\n"
      "|- text <line:2:32, col:37> 'world'\n");
}

TEST_F(ParserTest, comment_escaped_empty) {
  auto ast = parse_ast("{{!----}}");
  EXPECT_EQ(
      to_string(*ast),
      "root [path/to/test-1.whisker]\n"
      "|- comment <line:1:1, col:10> ''\n");
}

TEST_F(ParserTest, strip_standalone_lines) {
  auto ast = parse_ast(
      "| This Is\n"
      "{{#boolean}}\n"
      "|\n"
      "{{/boolean}}\n"
      "| A Line\n");
  EXPECT_EQ(
      to_string(*ast),
      "root [path/to/test-1.whisker]\n"
      "|- text <line:1:1, col:10> '| This Is'\n"
      "|- newline <line:1:10, line:2:1> '\\n'\n"
      "|- section-block <line:2:1, line:4:13>\n"
      "| `- variable-lookup <line:2:4, col:11> 'boolean'\n"
      "| |- text <line:3:1, col:2> '|'\n"
      "| |- newline <line:3:2, line:4:1> '\\n'\n"
      "|- text <line:5:1, col:9> '| A Line'\n"
      "|- newline <line:5:9, line:6:1> '\\n'\n");
}

TEST_F(ParserTest, strip_standalone_lines_indented) {
  auto ast = parse_ast(
      "| This Is\n"
      "  {{#boolean}}\n"
      "|\n"
      "  {{/boolean}}\n"
      "| A Line\n");
  EXPECT_EQ(
      to_string(*ast),
      "root [path/to/test-1.whisker]\n"
      "|- text <line:1:1, col:10> '| This Is'\n"
      "|- newline <line:1:10, line:2:1> '\\n'\n"
      "|- section-block <line:2:3, line:4:15>\n"
      "| `- variable-lookup <line:2:6, col:13> 'boolean'\n"
      "| |- text <line:3:1, col:2> '|'\n"
      "| |- newline <line:3:2, line:4:1> '\\n'\n"
      "|- text <line:5:1, col:9> '| A Line'\n"
      "|- newline <line:5:9, line:6:1> '\\n'\n");
}

TEST_F(ParserTest, strip_standalone_lines_indented_at_eof) {
  auto ast = parse_ast(
      "| This Is\n"
      "  {{#boolean}}\n"
      "|\n"
      "  {{/boolean}}  ");
  EXPECT_EQ(
      to_string(*ast),
      "root [path/to/test-1.whisker]\n"
      "|- text <line:1:1, col:10> '| This Is'\n"
      "|- newline <line:1:10, line:2:1> '\\n'\n"
      "|- section-block <line:2:3, line:4:15>\n"
      "| `- variable-lookup <line:2:6, col:13> 'boolean'\n"
      "| |- text <line:3:1, col:2> '|'\n"
      "| |- newline <line:3:2, line:4:1> '\\n'\n");
}

TEST_F(
    ParserTest, strip_standalone_lines_indented_at_eof_ending_with_template) {
  auto ast = parse_ast(
      "| This Is\n"
      "  {{#boolean}}\n"
      "|\n"
      "  {{/boolean}}");
  EXPECT_EQ(
      to_string(*ast),
      "root [path/to/test-1.whisker]\n"
      "|- text <line:1:1, col:10> '| This Is'\n"
      "|- newline <line:1:10, line:2:1> '\\n'\n"
      "|- section-block <line:2:3, line:4:15>\n"
      "| `- variable-lookup <line:2:6, col:13> 'boolean'\n"
      "| |- text <line:3:1, col:2> '|'\n"
      "| |- newline <line:3:2, line:4:1> '\\n'\n");
}

TEST_F(ParserTest, strip_standalone_lines_multiple) {
  auto ast = parse_ast(
      "| This Is\n"
      "  {{#boolean}} {{#boolean}} \n"
      "|\n"
      "  {{/boolean}}\n"
      "  {{/boolean}}\n"
      "| A Line\n");
  EXPECT_EQ(
      to_string(*ast),
      "root [path/to/test-1.whisker]\n"
      "|- text <line:1:1, col:10> '| This Is'\n"
      "|- newline <line:1:10, line:2:1> '\\n'\n"
      "|- section-block <line:2:3, line:5:15>\n"
      "| `- variable-lookup <line:2:6, col:13> 'boolean'\n"
      "| |- section-block <line:2:16, line:4:15>\n"
      "| | `- variable-lookup <line:2:19, col:26> 'boolean'\n"
      "| | |- text <line:3:1, col:2> '|'\n"
      "| | |- newline <line:3:2, line:4:1> '\\n'\n"
      "|- text <line:6:1, col:9> '| A Line'\n"
      "|- newline <line:6:9, line:7:1> '\\n'\n");
}

TEST_F(ParserTest, strip_standalone_lines_multiline) {
  auto ast = parse_ast(
      "| This Is\n"
      "  {{#boolean\n"
      "       .condition}}  \n"
      "|\n"
      "  {{/boolean.condition}}\n"
      "| A Line\n");
  EXPECT_EQ(
      to_string(*ast),
      "root [path/to/test-1.whisker]\n"
      "|- text <line:1:1, col:10> '| This Is'\n"
      "|- newline <line:1:10, line:2:1> '\\n'\n"
      "|- section-block <line:2:3, line:5:25>\n"
      "| `- variable-lookup <line:2:6, line:3:18> 'boolean.condition'\n"
      "| |- text <line:4:1, col:2> '|'\n"
      "| |- newline <line:4:2, line:5:1> '\\n'\n"
      "|- text <line:6:1, col:9> '| A Line'\n"
      "|- newline <line:6:9, line:7:1> '\\n'\n");
}

TEST_F(ParserTest, strip_standalone_lines_multiline_comment) {
  auto ast = parse_ast(
      "| This Is\n"
      "  {{^boolean\n"
      "       .condition}} {{! unaffected }} \n"
      "|\n"
      "  {{/boolean.condition}}\n"
      "| A Line\n");
  EXPECT_EQ(
      to_string(*ast),
      "root [path/to/test-1.whisker]\n"
      "|- text <line:1:1, col:10> '| This Is'\n"
      "|- newline <line:1:10, line:2:1> '\\n'\n"
      "|- section-block <inverted> <line:2:3, line:5:25>\n"
      "| `- variable-lookup <line:2:6, line:3:18> 'boolean.condition'\n"
      "| |- comment <line:3:21, col:38> ' unaffected '\n"
      "| |- text <line:4:1, col:2> '|'\n"
      "| |- newline <line:4:2, line:5:1> '\\n'\n"
      "|- text <line:6:1, col:9> '| A Line'\n"
      "|- newline <line:6:9, line:7:1> '\\n'\n");
}

TEST_F(ParserTest, strip_standalone_lines_multiline_ineligible) {
  auto ast = parse_ast(
      "| This Is\n"
      "  {{#boolean\n"
      "       .condition}} {{ineligible}} \n"
      "|\n"
      "  {{/boolean.condition}}\n"
      "| A Line\n");
  EXPECT_EQ(
      to_string(*ast),
      "root [path/to/test-1.whisker]\n"
      "|- text <line:1:1, col:10> '| This Is'\n"
      "|- newline <line:1:10, line:2:1> '\\n'\n"
      "|- text <line:2:1, col:3> '  '\n"
      "|- section-block <line:2:3, line:5:25>\n"
      "| `- variable-lookup <line:2:6, line:3:18> 'boolean.condition'\n"
      "| |- text <line:3:20, col:21> ' '\n"
      "| |- variable <line:3:21, col:35> 'ineligible'\n"
      "| |- text <line:3:35, col:36> ' '\n"
      "| |- newline <line:3:36, line:4:1> '\\n'\n"
      "| |- text <line:4:1, col:2> '|'\n"
      "| |- newline <line:4:2, line:5:1> '\\n'\n"
      "|- text <line:6:1, col:9> '| A Line'\n"
      "|- newline <line:6:9, line:7:1> '\\n'\n");
}

TEST_F(
    ParserTest,
    strip_standalone_lines_multiline_ineligible_partial_application) {
  auto ast = parse_ast(
      "| This Is\n"
      "  {{#boolean\n"
      "       .condition}} {{> ineligible}} \n"
      "|\n"
      "  {{/boolean.condition}}\n"
      "| A Line\n");
  EXPECT_EQ(
      to_string(*ast),
      "root [path/to/test-1.whisker]\n"
      "|- text <line:1:1, col:10> '| This Is'\n"
      "|- newline <line:1:10, line:2:1> '\\n'\n"
      "|- text <line:2:1, col:3> '  '\n"
      "|- section-block <line:2:3, line:5:25>\n"
      "| `- variable-lookup <line:2:6, line:3:18> 'boolean.condition'\n"
      "| |- text <line:3:20, col:21> ' '\n"
      "| |- partial-apply <line:3:21, col:37> 'ineligible'\n"
      "| |- text <line:3:37, col:38> ' '\n"
      "| |- newline <line:3:38, line:4:1> '\\n'\n"
      "| |- text <line:4:1, col:2> '|'\n"
      "| |- newline <line:4:2, line:5:1> '\\n'\n"
      "|- text <line:6:1, col:9> '| A Line'\n"
      "|- newline <line:6:9, line:7:1> '\\n'\n");
}

} // namespace whisker
