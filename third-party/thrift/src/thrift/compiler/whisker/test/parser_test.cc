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

  std::optional<ast::root> try_parse_ast(const std::string& source) {
    diagnostics.clear();
    return parse(
        src_manager.add_virtual_file(path_to_file(file_id++), source), diags);
  }

  [[nodiscard]] ast::root parse_ast(const std::string& source) {
    auto ast = try_parse_ast(source);
    EXPECT_THAT(diagnostics, testing::IsEmpty());
    return ast.value();
  }

  void parse_ast_should_fail(const std::string& source) {
    auto ast = try_parse_ast(source);
    ASSERT_FALSE(ast.has_value());
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
  EXPECT_TRUE(ast.body_elements.empty());
}

TEST_F(ParserTest, basic) {
  auto ast = parse_ast("Some text {{foo.bar}} more text");
  EXPECT_EQ(
      to_string(ast),
      "root [path/to/test-1.whisker]\n"
      "├─ text <line:1:1, col:11> 'Some text '\n"
      "├─ interpolation <line:1:11, col:22> 'foo.bar'\n"
      "╰─ text <line:1:22, col:32> ' more text'\n");
}

TEST_F(ParserTest, empty_template) {
  parse_ast_should_fail("{{ }}");
  EXPECT_THAT(
      diagnostics,
      testing::ElementsAre(diagnostic(
          diagnostic_level::error,
          "expected expression in interpolation but found `}}`",
          path_to_file(1),
          1)));
}

TEST_F(ParserTest, variable_is_single_id) {
  auto ast = parse_ast("{{ foo }}");
  EXPECT_EQ(
      to_string(ast),
      "root [path/to/test-1.whisker]\n"
      "╰─ interpolation <line:1:1, col:10> 'foo'\n");
}

TEST_F(ParserTest, variable_is_multi_id) {
  auto ast = parse_ast("{{ foo. bar.baz }}");
  EXPECT_EQ(
      to_string(ast),
      "root [path/to/test-1.whisker]\n"
      "╰─ interpolation <line:1:1, col:19> 'foo.bar.baz'\n");
}

TEST_F(ParserTest, variable_is_dot) {
  auto ast = parse_ast("{{. }}");
  EXPECT_EQ(
      to_string(ast),
      "root [path/to/test-1.whisker]\n"
      "╰─ interpolation <line:1:1, col:7> 'this'\n");
}

TEST_F(ParserTest, variable_starts_with_dot) {
  parse_ast_should_fail("{{ .foo }}");
  EXPECT_THAT(
      diagnostics,
      testing::ElementsAre(diagnostic(
          diagnostic_level::error,
          "expected `}}` to close interpolation but found identifier",
          path_to_file(1),
          1)));
}

TEST_F(ParserTest, variable_is_colon_qualified) {
  auto ast = parse_ast("{{ foo:bar }}");
  EXPECT_EQ(
      "root [path/to/test-1.whisker]\n"
      "╰─ interpolation <line:1:1, col:14> 'foo:bar'\n",
      to_string(ast));
}

TEST_F(ParserTest, variable_is_dot_and_colon_qualified) {
  auto ast = parse_ast("{{ foo.bar:baz }}");
  EXPECT_EQ(
      "root [path/to/test-1.whisker]\n"
      "╰─ interpolation <line:1:1, col:18> 'foo.bar:baz'\n",
      to_string(ast));
}

TEST_F(ParserTest, variable_is_colon_and_dot_qualified) {
  auto ast = parse_ast("{{ foo:bar.baz }}");
  EXPECT_EQ(
      "root [path/to/test-1.whisker]\n"
      "╰─ interpolation <line:1:1, col:18> 'foo:bar.baz'\n",
      to_string(ast));
}

TEST_F(ParserTest, variable_starts_with_colon) {
  parse_ast_should_fail("{{ :foo }}");
  EXPECT_THAT(
      diagnostics,
      testing::ElementsAre(diagnostic(
          diagnostic_level::error,
          "expected expression in interpolation but found `:`",
          path_to_file(1),
          1)));
}

TEST_F(ParserTest, variable_has_dangling_colon) {
  parse_ast_should_fail("{{ foo: }}");
  EXPECT_THAT(
      diagnostics,
      testing::ElementsAre(diagnostic(
          diagnostic_level::error,
          "expected identifier in qualified variable-component but found `}}`",
          path_to_file(1),
          1)));
}

TEST_F(ParserTest, variable_has_extra_stuff_after) {
  parse_ast_should_fail("{{foo.bar!}}");
  EXPECT_THAT(
      diagnostics,
      testing::ElementsAre(diagnostic(
          diagnostic_level::error,
          "expected `}}` to close interpolation but found `!`",
          path_to_file(1),
          1)));
}

TEST_F(ParserTest, basic_section) {
  auto ast = parse_ast(
      "{{ #news.has-update?}}\n"
      "  Stuff is {{foo}} happening!\n"
      "{{/ news.has-update?}}");
  EXPECT_EQ(
      to_string(ast),
      "root [path/to/test-1.whisker]\n"
      "╰─ section-block <line:1:1, line:3:23>\n"
      "   ├─ variable-lookup <line:1:5, col:21> 'news.has-update?'\n"
      "   ├─ text <line:2:1, col:12> '  Stuff is '\n"
      "   ├─ interpolation <line:2:12, col:19> 'foo'\n"
      "   ├─ text <line:2:19, col:30> ' happening!'\n"
      "   ╰─ newline <line:2:30, line:3:1> '\\n'\n");
}

TEST_F(ParserTest, inverted_section) {
  auto ast = parse_ast(
      "{{^news.has-update?}}\n"
      "  Stuff is {{foo}} happening!\n"
      "{{/news.has-update?}}");
  EXPECT_EQ(
      to_string(ast),
      "root [path/to/test-1.whisker]\n"
      "╰─ section-block <inverted> <line:1:1, line:3:22>\n"
      "   ├─ variable-lookup <line:1:4, col:20> 'news.has-update?'\n"
      "   ├─ text <line:2:1, col:12> '  Stuff is '\n"
      "   ├─ interpolation <line:2:12, col:19> 'foo'\n"
      "   ├─ text <line:2:19, col:30> ' happening!'\n"
      "   ╰─ newline <line:2:30, line:3:1> '\\n'\n");
}

TEST_F(ParserTest, nested_sections) {
  auto ast = parse_ast(
      "{{#news.has-update?}}\n"
      "  {{^update.is-important?}}\n"
      "    Important stuff is {{foo}} happening!\n"
      "  {{/update.is-important?}}\n"
      "{{/news.has-update?}}");
  EXPECT_EQ(
      to_string(ast),
      "root [path/to/test-1.whisker]\n"
      "╰─ section-block <line:1:1, line:5:22>\n"
      "   ├─ variable-lookup <line:1:4, col:20> 'news.has-update?'\n"
      "   ╰─ section-block <inverted> <line:2:3, line:4:28>\n"
      "      ├─ variable-lookup <line:2:6, col:26> 'update.is-important?'\n"
      "      ├─ text <line:3:1, col:24> '    Important stuff is '\n"
      "      ├─ interpolation <line:3:24, col:31> 'foo'\n"
      "      ├─ text <line:3:31, col:42> ' happening!'\n"
      "      ╰─ newline <line:3:42, line:4:1> '\\n'\n");
}

TEST_F(ParserTest, mismatched_section_hierarchy) {
  parse_ast_should_fail(
      "{{#news.has-update?}}\n"
      "  {{#update.is-important?}}\n"
      "    Important stuff is {{foo}} happening!\n"
      "    {{#inner}}{{/inner}}\n"
      "  {{/news.has-update?}}\n"
      "{{/update.is-important?}}");
  EXPECT_THAT(
      diagnostics,
      testing::ElementsAre(
          diagnostic(
              diagnostic_level::error,
              "section-block opening 'update.is-important?' does not match closing 'news.has-update?'",
              path_to_file(1),
              5),
          diagnostic(
              diagnostic_level::error,
              "section-block opening 'news.has-update?' does not match closing 'update.is-important?'",
              path_to_file(1),
              6)));
}

TEST_F(ParserTest, section_open_by_itself) {
  parse_ast_should_fail("{{#news.has-update?}}");
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
    parse_ast_should_fail("{{#news.has-update?}}{{/true}}");
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
    parse_ast_should_fail("{{#news.has-update?}}{{/ news.has-update?");
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
    parse_ast_should_fail("{{#news.has-update?}}{{/ foo-bar?");
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
  parse_ast_should_fail("{{/news.has-update?}}");
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
      to_string(ast),
      "root [path/to/test-1.whisker]\n"
      "╰─ if-block <line:1:1, line:3:25>\n"
      "   ├─ expression <line:1:7, col:23> 'news.has-update?'\n"
      "   ├─ text <line:2:1, col:12> '  Stuff is '\n"
      "   ├─ interpolation <line:2:12, col:19> 'foo'\n"
      "   ├─ text <line:2:19, col:30> ' happening!'\n"
      "   ╰─ newline <line:2:30, line:3:1> '\\n'\n");
}

TEST_F(ParserTest, basic_if_else) {
  auto ast = parse_ast(
      "{{#if news.has-update?}}\n"
      "  Stuff is {{foo}} happening!\n"
      "{{#else}}\n"
      "  Nothing is happening!\n"
      "{{/if news.has-update?}}");
  EXPECT_EQ(
      to_string(ast),
      "root [path/to/test-1.whisker]\n"
      "╰─ if-block <line:1:1, line:5:25>\n"
      "   ├─ expression <line:1:7, col:23> 'news.has-update?'\n"
      "   ├─ text <line:2:1, col:12> '  Stuff is '\n"
      "   ├─ interpolation <line:2:12, col:19> 'foo'\n"
      "   ├─ text <line:2:19, col:30> ' happening!'\n"
      "   ├─ newline <line:2:30, line:3:1> '\\n'\n"
      "   ╰─ else-block <line:3:1, line:5:1>\n"
      "      ├─ text <line:4:1, col:24> '  Nothing is happening!'\n"
      "      ╰─ newline <line:4:24, line:5:1> '\\n'\n");
}

TEST_F(ParserTest, unless_block_repeated_else) {
  parse_ast_should_fail(
      "{{#if (not news.has-update?)}}\n"
      "  Stuff is {{foo}} happening!\n"
      "{{#else}}\n"
      "  Nothing is happening!\n"
      "{{#else}}\n"
      "  Nothing is happening!\n"
      "{{/if (not news.has-update?)}}");
  EXPECT_THAT(
      diagnostics,
      testing::ElementsAre(diagnostic(
          diagnostic_level::error,
          "expected `/` to close if-block '(not news.has-update?)' but found `#`",
          path_to_file(1),
          5)));
}

TEST_F(ParserTest, if_block_else_by_itself) {
  parse_ast_should_fail("{{#else}}");
  EXPECT_THAT(
      diagnostics,
      testing::ElementsAre(diagnostic(
          diagnostic_level::error,
          "expected text, template, or comment but found dangling else-clause",
          path_to_file(1),
          1)));
}

TEST_F(ParserTest, if_block_unclosed_with_else) {
  parse_ast_should_fail("{{#if news.has-update?}}{{#else}}");
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
      to_string(ast),
      "root [path/to/test-1.whisker]\n"
      "╰─ if-block <line:1:1, line:7:25>\n"
      "   ├─ expression <line:1:7, col:23> 'news.has-update?'\n"
      "   ╰─ if-block <line:2:3, line:6:31>\n"
      "      ├─ expression <line:2:9, col:29> 'update.is-important?'\n"
      "      ├─ text <line:3:1, col:24> '    Important stuff is '\n"
      "      ├─ interpolation <line:3:24, col:31> 'foo'\n"
      "      ├─ text <line:3:31, col:42> ' happening!'\n"
      "      ├─ newline <line:3:42, line:4:1> '\\n'\n"
      "      ╰─ else-block <line:4:3, line:6:3>\n"
      "         ├─ text <line:5:1, col:31> '    Boring stuff is happening!'\n"
      "         ╰─ newline <line:5:31, line:6:1> '\\n'\n");
}

TEST_F(ParserTest, mismatched_if_hierarchy) {
  parse_ast_should_fail(
      "{{#if news.has-update?}}\n"
      "  {{#update.is-important?}}\n"
      "    Important stuff is {{foo}} happening!\n"
      "    {{#inner}}{{/inner}}\n"
      "  {{/if news.has-update?}}\n"
      "{{/update.is-important?}}");
  EXPECT_THAT(
      diagnostics,
      testing::ElementsAre(diagnostic(
          diagnostic_level::error,
          "expected variable-lookup to close section-block 'update.is-important?' but found `if`",
          path_to_file(1),
          5)));
}

TEST_F(ParserTest, if_by_itself) {
  parse_ast_should_fail("{{#if}}");
  EXPECT_THAT(
      diagnostics,
      testing::ElementsAre(diagnostic(
          diagnostic_level::error,
          "expected expression to open if-block but found `}}`",
          path_to_file(1),
          1)));
}

TEST_F(ParserTest, if_close_by_itself) {
  parse_ast_should_fail("{{/if}}");
  EXPECT_THAT(
      diagnostics,
      testing::ElementsAre(diagnostic(
          diagnostic_level::error,
          "expected text, template, or comment but found `{{`",
          path_to_file(1),
          1)));
}

TEST_F(ParserTest, conditional_block_mismatched_open_and_close) {
  parse_ast_should_fail(
      "{{#if (not news.has-update?)}}\n"
      "  Stuff is happening!\n"
      "{{#else}}\n"
      "  Nothing is happening!\n"
      "{{/unless}}");
  EXPECT_THAT(
      diagnostics,
      testing::ElementsAre(diagnostic(
          diagnostic_level::error,
          "expected `if` to close if-block '(not news.has-update?)' but found `unless`",
          path_to_file(1),
          5)));
}

TEST_F(ParserTest, conditional_block_missing_close_lookup) {
  parse_ast_should_fail(
      "{{#if news.has-update?}}\n"
      "  Stuff is happening!\n"
      "{{#else}}\n"
      "  Nothing is happening!\n"
      "{{/if}}");
  EXPECT_THAT(
      diagnostics,
      testing::ElementsAre(diagnostic(
          diagnostic_level::error,
          "expected expression to close if-block 'news.has-update?' but found `}}`",
          path_to_file(1),
          5)));
}

TEST_F(ParserTest, conditional_block_mismatched_lookup) {
  parse_ast_should_fail(
      "{{#if news.has-update?}}\n"
      "  Stuff is happening!\n"
      "{{#else}}\n"
      "  Nothing is happening!\n"
      "{{/if news.has_updates?}}");
  EXPECT_THAT(
      diagnostics,
      testing::ElementsAre(diagnostic(
          diagnostic_level::error,
          "conditional-block opening 'news.has-update?' does not match closing 'news.has_updates?'",
          path_to_file(1),
          5)));
}

TEST_F(ParserTest, conditional_block_else_if) {
  auto ast = parse_ast(
      "{{#if news.has-update?}}\n"
      "  New stuff is happening!\n"
      "{{#else if news.is-important?}}\n"
      "  Important stuff is happening!\n"
      "{{#else}}\n"
      "  Nothing is happening!\n"
      "{{/if news.has-update?}}");
  EXPECT_EQ(
      to_string(ast),
      "root [path/to/test-1.whisker]\n"
      "╰─ if-block <line:1:1, line:7:25>\n"
      "   ├─ expression <line:1:7, col:23> 'news.has-update?'\n"
      "   ├─ text <line:2:1, col:26> '  New stuff is happening!'\n"
      "   ├─ newline <line:2:26, line:3:1> '\\n'\n"
      "   ├─ else-if-block <line:3:1, line:5:1>\n"
      "   │  ├─ text <line:4:1, col:32> '  Important stuff is happening!'\n"
      "   │  ╰─ newline <line:4:32, line:5:1> '\\n'\n"
      "   ╰─ else-block <line:5:1, line:7:1>\n"
      "      ├─ text <line:6:1, col:24> '  Nothing is happening!'\n"
      "      ╰─ newline <line:6:24, line:7:1> '\\n'\n");
}

TEST_F(ParserTest, conditional_block_else_if_after_else) {
  parse_ast_should_fail(
      "{{#if news.has-update?}}\n"
      "  New stuff is happening!\n"
      "{{#else}}\n"
      "  Nothing is happening!\n"
      "{{#else if news.is-important?}}\n"
      "  Important stuff is happening!\n"
      "{{/if news.has-update?}}");
  EXPECT_THAT(
      diagnostics,
      testing::ElementsAre(diagnostic(
          diagnostic_level::error,
          "expected `/` to close if-block 'news.has-update?' but found `#`",
          path_to_file(1),
          5)));
}

TEST_F(ParserTest, conditional_block_with_not) {
  auto ast = parse_ast(
      "{{#if (not news.has-update?)}}\n"
      "  Stuff is happening!\n"
      "{{/if (not news.has-update?)}}");
  EXPECT_EQ(
      to_string(ast),
      "root [path/to/test-1.whisker]\n"
      "╰─ if-block <line:1:1, line:3:31>\n"
      "   ├─ expression <line:1:7, col:29> '(not news.has-update?)'\n"
      "   ├─ text <line:2:1, col:22> '  Stuff is happening!'\n"
      "   ╰─ newline <line:2:22, line:3:1> '\\n'\n");
}

TEST_F(ParserTest, conditional_block_with_and_or) {
  auto ast = parse_ast(
      "{{#if (and (or no yes) yes)}}\n"
      "Yes!\n"
      "{{/if (and (or no yes) yes)}}");
  EXPECT_EQ(
      to_string(ast),
      "root [path/to/test-1.whisker]\n"
      "╰─ if-block <line:1:1, line:3:30>\n"
      "   ├─ expression <line:1:7, col:28> '(and (or no yes) yes)'\n"
      "   ├─ text <line:2:1, col:5> 'Yes!'\n"
      "   ╰─ newline <line:2:5, line:3:1> '\\n'\n");
}

TEST_F(ParserTest, conditional_block_not_wrong_arity) {
  parse_ast_should_fail(
      "{{#if (not news.has_updates? news.has_updates?)}}\n"
      "  Stuff is happening!\n"
      "{{/if (not news.has_updates? news.has_updates?)}}");
  EXPECT_THAT(
      diagnostics,
      testing::ElementsAre(diagnostic(
          diagnostic_level::error,
          "expected 1 argument for function 'not' but found 2",
          path_to_file(1),
          1)));
}

TEST_F(ParserTest, conditional_block_omit_close_expression_for_single_line) {
  auto ast = parse_ast(
      "{{#if yes}} Yes! {{/if}}\n"
      "{{#if yes}} Yes! {{#else}} No! {{/if}}\n"
      "{{#if yes}} Yes! {{#else if no}} No! {{/if}}\n"
      "{{#if yes}} Yes! {{#else if no}} No! {{#else}} Maybe! {{/if}}\n");
  EXPECT_EQ(
      to_string(ast),
      "root [path/to/test-1.whisker]\n"
      "├─ if-block <line:1:1, col:25>\n"
      "│  ├─ expression <line:1:7, col:10> 'yes'\n"
      "│  ╰─ text <line:1:12, col:18> ' Yes! '\n"
      "├─ newline <line:1:25, line:2:1> '\\n'\n"
      "├─ if-block <line:2:1, col:39>\n"
      "│  ├─ expression <line:2:7, col:10> 'yes'\n"
      "│  ├─ text <line:2:12, col:18> ' Yes! '\n"
      "│  ╰─ else-block <line:2:18, col:32>\n"
      "│     ╰─ text <line:2:27, col:32> ' No! '\n"
      "├─ newline <line:2:39, line:3:1> '\\n'\n"
      "├─ if-block <line:3:1, col:45>\n"
      "│  ├─ expression <line:3:7, col:10> 'yes'\n"
      "│  ├─ text <line:3:12, col:18> ' Yes! '\n"
      "│  ╰─ else-if-block <line:3:18, col:38>\n"
      "│     ╰─ text <line:3:33, col:38> ' No! '\n"
      "├─ newline <line:3:45, line:4:1> '\\n'\n"
      "├─ if-block <line:4:1, col:62>\n"
      "│  ├─ expression <line:4:7, col:10> 'yes'\n"
      "│  ├─ text <line:4:12, col:18> ' Yes! '\n"
      "│  ├─ else-if-block <line:4:18, col:38>\n"
      "│  │  ╰─ text <line:4:33, col:38> ' No! '\n"
      "│  ╰─ else-block <line:4:38, col:55>\n"
      "│     ╰─ text <line:4:47, col:55> ' Maybe! '\n"
      "╰─ newline <line:4:62, line:5:1> '\\n'\n");
}

TEST_F(
    ParserTest,
    conditional_block_omit_close_expression_fails_if_close_on_different_line) {
  // Even though whitespace is not significant inside {{ tags }}, we still
  // require single line in source to omit close expression.
  parse_ast_should_fail(
      "{{#if yes}} Yes! {{\n"
      "/if}}\n");
  EXPECT_THAT(
      diagnostics,
      testing::ElementsAre(diagnostic(
          diagnostic_level::error,
          "expected expression to close if-block 'yes' but found `}}`",
          path_to_file(1),
          2)));
}

TEST_F(ParserTest, basic_each) {
  auto ast = parse_ast(
      "{{#each news.updates}}\n"
      "  {{.}}\n"
      "{{/each}}");
  EXPECT_EQ(
      to_string(ast),
      "root [path/to/test-1.whisker]\n"
      "╰─ each-block <line:1:1, line:3:10>\n"
      "   ├─ expression <line:1:9, col:21> 'news.updates'\n"
      "   ├─ text <line:2:1, col:3> '  '\n"
      "   ├─ interpolation <line:2:3, col:8> 'this'\n"
      "   ╰─ newline <line:2:8, line:3:1> '\\n'\n");
}

TEST_F(ParserTest, basic_each_else) {
  auto ast = parse_ast(
      "{{#each news.updates}}\n"
      "  {{.}}\n"
      "{{#else}}\n"
      "  Got nothing!\n"
      "{{/each}}");
  EXPECT_EQ(
      to_string(ast),
      "root [path/to/test-1.whisker]\n"
      "╰─ each-block <line:1:1, line:5:10>\n"
      "   ├─ expression <line:1:9, col:21> 'news.updates'\n"
      "   ├─ text <line:2:1, col:3> '  '\n"
      "   ├─ interpolation <line:2:3, col:8> 'this'\n"
      "   ├─ newline <line:2:8, line:3:1> '\\n'\n"
      "   ╰─ else-block <line:3:1, line:5:1>\n"
      "      ├─ text <line:4:1, col:15> '  Got nothing!'\n"
      "      ╰─ newline <line:4:15, line:5:1> '\\n'\n");
}

TEST_F(ParserTest, each_with_element_capture) {
  auto ast = parse_ast(
      "{{#each news.updates as |update|}}\n"
      "  {{update}}\n"
      "{{/each}}");
  EXPECT_EQ(
      to_string(ast),
      "root [path/to/test-1.whisker]\n"
      "╰─ each-block <line:1:1, line:3:10>\n"
      "   ├─ expression <line:1:9, col:21> 'news.updates'\n"
      "   ├─ element-capture 'update'\n"
      "   ├─ text <line:2:1, col:3> '  '\n"
      "   ├─ interpolation <line:2:3, col:13> 'update'\n"
      "   ╰─ newline <line:2:13, line:3:1> '\\n'\n");
}

TEST_F(ParserTest, each_with_two_captures) {
  auto ast = parse_ast(
      "{{#each news.updates as |update index|}}\n"
      "  {{index}}. {{update}}\n"
      "{{/each}}");
  EXPECT_EQ(
      to_string(ast),
      "root [path/to/test-1.whisker]\n"
      "╰─ each-block <line:1:1, line:3:10>\n"
      "   ├─ expression <line:1:9, col:21> 'news.updates'\n"
      "   ├─ element-capture 'update'\n"
      "   ├─ element-capture 'index'\n"
      "   ├─ text <line:2:1, col:3> '  '\n"
      "   ├─ interpolation <line:2:3, col:12> 'index'\n"
      "   ├─ text <line:2:12, col:14> '. '\n"
      "   ├─ interpolation <line:2:14, col:24> 'update'\n"
      "   ╰─ newline <line:2:24, line:3:1> '\\n'\n");
}

TEST_F(ParserTest, each_with_many_captures) {
  auto ast = parse_ast(
      "{{#each (foo 1 2 3) as |c1 c2 c3|}}\n"
      "{{/each}}\n");
  EXPECT_EQ(
      to_string(ast),
      "root [path/to/test-1.whisker]\n"
      "╰─ each-block <line:1:1, line:2:10>\n"
      "   ├─ expression <line:1:9, col:20> '(foo 1 2 3)'\n"
      "   ├─ element-capture 'c1'\n"
      "   ├─ element-capture 'c2'\n"
      "   ╰─ element-capture 'c3'\n");
}

TEST_F(ParserTest, each_block_nested) {
  auto ast = parse_ast(
      "{{#each news.updates as |update|}}\n"
      "  {{#each update.headlines as |headline index|}}\n"
      "    {{index}}. {{headline}}\n"
      "  {{#else}}\n"
      "    No headlines!\n"
      "  {{/each}}\n"
      "{{#else}}\n"
      "  No updates!\n"
      "{{/each}}");
  EXPECT_EQ(
      to_string(ast),
      "root [path/to/test-1.whisker]\n"
      "╰─ each-block <line:1:1, line:9:10>\n"
      "   ├─ expression <line:1:9, col:21> 'news.updates'\n"
      "   ├─ element-capture 'update'\n"
      "   ├─ each-block <line:2:3, line:6:12>\n"
      "   │  ├─ expression <line:2:11, col:27> 'update.headlines'\n"
      "   │  ├─ element-capture 'headline'\n"
      "   │  ├─ element-capture 'index'\n"
      "   │  ├─ text <line:3:1, col:5> '    '\n"
      "   │  ├─ interpolation <line:3:5, col:14> 'index'\n"
      "   │  ├─ text <line:3:14, col:16> '. '\n"
      "   │  ├─ interpolation <line:3:16, col:28> 'headline'\n"
      "   │  ├─ newline <line:3:28, line:4:1> '\\n'\n"
      "   │  ╰─ else-block <line:4:3, line:6:3>\n"
      "   │     ├─ text <line:5:1, col:18> '    No headlines!'\n"
      "   │     ╰─ newline <line:5:18, line:6:1> '\\n'\n"
      "   ╰─ else-block <line:7:1, line:9:1>\n"
      "      ├─ text <line:8:1, col:14> '  No updates!'\n"
      "      ╰─ newline <line:8:14, line:9:1> '\\n'\n");
}

TEST_F(ParserTest, each_block_unclosed_with_else) {
  parse_ast_should_fail("{{#each news.updates}}{{#else}}");
  EXPECT_THAT(
      diagnostics,
      testing::ElementsAre(diagnostic(
          diagnostic_level::error,
          "expected `{{` to close each-block 'news.updates' but found EOF",
          path_to_file(1),
          1)));
}

TEST_F(ParserTest, each_by_itself) {
  parse_ast_should_fail("{{#each}}");
  EXPECT_THAT(
      diagnostics,
      testing::ElementsAre(diagnostic(
          diagnostic_level::error,
          "expected expression to open each-block but found `}}`",
          path_to_file(1),
          1)));
}

TEST_F(ParserTest, each_close_by_itself) {
  parse_ast_should_fail("{{/each}}");
  EXPECT_THAT(
      diagnostics,
      testing::ElementsAre(diagnostic(
          diagnostic_level::error,
          "expected text, template, or comment but found `{{`",
          path_to_file(1),
          1)));
}

TEST_F(ParserTest, each_missing_captures) {
  parse_ast_should_fail(
      "{{#each (foo 1 2 3) as}}\n"
      "{{/each}}\n");
  EXPECT_THAT(
      diagnostics,
      testing::ElementsAre(diagnostic(
          diagnostic_level::error,
          "expected `|` to open each-block capture but found `}}`",
          path_to_file(1),
          1)));
}

TEST_F(ParserTest, each_empty_captures) {
  parse_ast_should_fail(
      "{{#each (foo 1 2 3) as ||}}\n"
      "{{/each}}\n");
  EXPECT_THAT(
      diagnostics,
      testing::ElementsAre(diagnostic(
          diagnostic_level::error,
          "expected at least one capture in each-block",
          path_to_file(1),
          1)));
}

TEST_F(ParserTest, each_duplicate_captures) {
  parse_ast_should_fail(
      "{{#each (foo 1 2 3) as |cap foo cap|}}\n"
      "{{/each}}\n");
  EXPECT_THAT(
      diagnostics,
      testing::ElementsAre(diagnostic(
          diagnostic_level::error,
          "duplicate capture 'cap' in each-block",
          path_to_file(1),
          1)));
}

TEST_F(ParserTest, each_extra_tokens_after_captures) {
  parse_ast_should_fail(
      "{{#each (foo 1 2 3) as |update| true}}\n"
      "{{/each}}\n");
  EXPECT_THAT(
      diagnostics,
      testing::ElementsAre(diagnostic(
          diagnostic_level::error,
          "expected `}}` to open each-block but found `true`",
          path_to_file(1),
          1)));
}

TEST_F(ParserTest, each_missing_as) {
  parse_ast_should_fail(
      "{{#each (foo 1 2 3) |update|}}\n"
      "{{/each}}\n");
  EXPECT_THAT(
      diagnostics,
      testing::ElementsAre(diagnostic(
          diagnostic_level::error,
          "expected `}}` to open each-block but found `|`",
          path_to_file(1),
          1)));
}

TEST_F(ParserTest, each_missing_expr) {
  parse_ast_should_fail(
      "{{#each as |update|}}\n"
      "{{/each}}\n");
  EXPECT_THAT(
      diagnostics,
      testing::ElementsAre(diagnostic(
          diagnostic_level::error,
          "expected expression to open each-block but found `as`",
          path_to_file(1),
          1)));
}

TEST_F(ParserTest, each_missing_expr_and_as) {
  parse_ast_should_fail(
      "{{#each |name index|}}\n"
      "{{/each}}\n");
  EXPECT_THAT(
      diagnostics,
      testing::ElementsAre(diagnostic(
          diagnostic_level::error,
          "expected expression to open each-block but found `|`",
          path_to_file(1),
          1)));
}

TEST_F(ParserTest, each_multiple_else) {
  parse_ast_should_fail(
      "{{#each news.updates}}\n"
      "  {{.}}\n"
      "{{#else}}\n"
      "  Got nothing!\n"
      "{{#else}}\n"
      "  Got nothing again!\n"
      "{{/each}}");
  EXPECT_THAT(
      diagnostics,
      testing::ElementsAre(diagnostic(
          diagnostic_level::error,
          "expected `/` to close each-block 'news.updates' but found `#`",
          path_to_file(1),
          5)));
}

TEST_F(ParserTest, each_missing_close) {
  parse_ast_should_fail("{{#each news.updates}}");
  EXPECT_THAT(
      diagnostics,
      testing::ElementsAre(diagnostic(
          diagnostic_level::error,
          "expected `{{` to close each-block 'news.updates' but found EOF",
          path_to_file(1),
          1)));
}

TEST_F(ParserTest, each_else_missing_close) {
  parse_ast_should_fail(
      "{{#each news.updates}}\n"
      "{{#else}}");
  EXPECT_THAT(
      diagnostics,
      testing::ElementsAre(diagnostic(
          diagnostic_level::error,
          "expected `{{` to close each-block 'news.updates' but found EOF",
          path_to_file(1),
          2)));
}

TEST_F(ParserTest, literals) {
  auto ast = parse_ast(
      "{{null}}\n"
      "{{true}}\n"
      "{{false}}\n"
      "{{\"hello\\tworld\"}}\n"
      "{{-1234}}\n");
  EXPECT_EQ(
      to_string(ast),
      "root [path/to/test-1.whisker]\n"
      "├─ interpolation <line:1:1, col:9> 'null'\n"
      "├─ newline <line:1:9, line:2:1> '\\n'\n"
      "├─ interpolation <line:2:1, col:9> 'true'\n"
      "├─ newline <line:2:9, line:3:1> '\\n'\n"
      "├─ interpolation <line:3:1, col:10> 'false'\n"
      "├─ newline <line:3:10, line:4:1> '\\n'\n"
      "├─ interpolation <line:4:1, col:19> '\"hello\\tworld\"'\n"
      "├─ newline <line:4:19, line:5:1> '\\n'\n"
      "├─ interpolation <line:5:1, col:10> '-1234'\n"
      "╰─ newline <line:5:10, line:6:1> '\\n'\n");
}

TEST_F(ParserTest, function_call) {
  auto ast = parse_ast("{{ (uppercase (lowercase hello\tspace) world) }}");
  EXPECT_EQ(
      to_string(ast),
      "root [path/to/test-1.whisker]\n"
      "╰─ interpolation <line:1:1, col:48> '(uppercase (lowercase hello space) world)'\n");
}

TEST_F(ParserTest, function_call_named_args) {
  auto ast = parse_ast(R"({{ (str.concat hello world sep=",") }})");
  EXPECT_EQ(
      to_string(ast),
      "root [path/to/test-1.whisker]\n"
      "╰─ interpolation <line:1:1, col:39> '(str.concat hello world sep=\",\")'\n");
}

TEST_F(ParserTest, function_call_named_args_only) {
  auto ast = parse_ast("{{ (str.concat sep = comma) }}");
  EXPECT_EQ(
      to_string(ast),
      "root [path/to/test-1.whisker]\n"
      "╰─ interpolation <line:1:1, col:31> '(str.concat sep=comma)'\n");
}

TEST_F(ParserTest, function_call_named_args_duplicate) {
  parse_ast_should_fail(
      "{{ (str.concat hello world sep=comma foo=bar sep=baz) }}");
  EXPECT_THAT(
      diagnostics,
      testing::ElementsAre(diagnostic(
          diagnostic_level::error,
          "duplicate named argument 'sep' in function call 'str.concat'",
          path_to_file(1),
          1)));
}

TEST_F(ParserTest, function_call_named_args_without_lookup) {
  parse_ast_should_fail(R"({{ (sep=",") }})");
  // `sep` is parsed as the name
  EXPECT_THAT(
      diagnostics,
      testing::ElementsAre(diagnostic(
          diagnostic_level::error,
          "expected identifier to precede `=` in named argument for function call 'sep'",
          path_to_file(1),
          1)));
}

TEST_F(ParserTest, function_call_positional_args_after_named_args) {
  parse_ast_should_fail(R"({{ (str.concat sep="," hello world) }})");
  EXPECT_THAT(
      diagnostics,
      testing::ElementsAre(diagnostic(
          diagnostic_level::error,
          "unexpected positional argument 'hello' after named arguments in function call 'str.concat'",
          path_to_file(1),
          1)));
}

TEST_F(ParserTest, function_call_named_args_not_identifier) {
  parse_ast_should_fail("{{ (str.concat hello world word.sep=comma) }}");
  EXPECT_THAT(
      diagnostics,
      testing::ElementsAre(diagnostic(
          diagnostic_level::error,
          "expected identifier to precede `=` in named argument for function call 'str.concat'",
          path_to_file(1),
          1)));
}

TEST_F(ParserTest, function_call_named_args_for_builtins) {
  {
    parse_ast_should_fail("{{ (not foo ignore=1234) }}");
    EXPECT_THAT(
        diagnostics,
        testing::ElementsAre(diagnostic(
            diagnostic_level::error,
            "named arguments not allowed for function 'not'",
            path_to_file(1),
            1)));
  }
  {
    parse_ast_should_fail("{{ (and foo bar ignore=1234) }}");
    EXPECT_THAT(
        diagnostics,
        testing::ElementsAre(diagnostic(
            diagnostic_level::error,
            "named arguments not allowed for function 'and'",
            path_to_file(2),
            1)));
  }
  {
    parse_ast_should_fail("{{ (or foo bar ignore=1234) }}");
    EXPECT_THAT(
        diagnostics,
        testing::ElementsAre(diagnostic(
            diagnostic_level::error,
            "named arguments not allowed for function 'or'",
            path_to_file(3),
            1)));
  }
  {
    parse_ast_should_fail("{{ (if foo bar baz ignore=1234) }}");
    EXPECT_THAT(
        diagnostics,
        testing::ElementsAre(diagnostic(
            diagnostic_level::error,
            "named arguments not allowed for function 'if'",
            path_to_file(4),
            1)));
  }
}

TEST_F(ParserTest, partial_block_basic) {
  auto ast = parse_ast(
      "{{#let partial foo |arg1 arg2|}}\n"
      "This is a partial block!\n"
      "{{/let partial}}\n");
  EXPECT_EQ(
      to_string(ast),
      "root [path/to/test-1.whisker]\n"
      "╰─ partial-block <line:1:1, line:3:17> 'foo'\n"
      "   ├─ argument 'arg1'\n"
      "   ├─ argument 'arg2'\n"
      "   ├─ text <line:2:1, col:25> 'This is a partial block!'\n"
      "   ╰─ newline <line:2:25, line:3:1> '\\n'\n");
}

TEST_F(ParserTest, partial_block_export) {
  auto ast = parse_ast(
      "{{#let export partial foo |arg1 arg2|}}\n"
      "This is a partial block!\n"
      "{{/let partial}}\n");
  EXPECT_EQ(
      to_string(ast),
      "root [path/to/test-1.whisker]\n"
      "╰─ partial-block <line:1:1, line:3:17> 'foo'\n"
      "   ├─ exported\n"
      "   ├─ argument 'arg1'\n"
      "   ├─ argument 'arg2'\n"
      "   ├─ text <line:2:1, col:25> 'This is a partial block!'\n"
      "   ╰─ newline <line:2:25, line:3:1> '\\n'\n");
}

TEST_F(ParserTest, partial_block_with_captures) {
  auto ast = parse_ast(
      "{{#let partial foo |arg1 arg2| captures |cap1 cap2|}}\n"
      "This is a partial block!\n"
      "{{/let partial}}\n");
  EXPECT_EQ(
      to_string(ast),
      "root [path/to/test-1.whisker]\n"
      "╰─ partial-block <line:1:1, line:3:17> 'foo'\n"
      "   ├─ argument 'arg1'\n"
      "   ├─ argument 'arg2'\n"
      "   ├─ capture 'cap1'\n"
      "   ├─ capture 'cap2'\n"
      "   ├─ text <line:2:1, col:25> 'This is a partial block!'\n"
      "   ╰─ newline <line:2:25, line:3:1> '\\n'\n");
}

TEST_F(ParserTest, partial_block_only_captures) {
  auto ast = parse_ast(
      "{{#let partial foo captures |cap1 cap2|}}\n"
      "This is a partial block!\n"
      "{{/let partial}}\n");
  EXPECT_EQ(
      to_string(ast),
      "root [path/to/test-1.whisker]\n"
      "╰─ partial-block <line:1:1, line:3:17> 'foo'\n"
      "   ├─ capture 'cap1'\n"
      "   ├─ capture 'cap2'\n"
      "   ├─ text <line:2:1, col:25> 'This is a partial block!'\n"
      "   ╰─ newline <line:2:25, line:3:1> '\\n'\n");
}

TEST_F(ParserTest, partial_block_after_comment) {
  auto ast = parse_ast("{{!}}{{#let partial foo |arg|}}{{/let partial}}\n");
  EXPECT_EQ(
      to_string(ast),
      "root [path/to/test-1.whisker]\n"
      "├─ header\n"
      "│  ╰─ comment <line:1:1, col:6> ''\n"
      "╰─ partial-block <line:1:6, col:48> 'foo'\n"
      "   ╰─ argument 'arg'\n");
}

TEST_F(ParserTest, partial_block_do_not_consume_whitespace) {
  auto ast = parse_ast(
      "\n"
      "{{!}}\n"
      "\n"
      "{{#let partial foo |arg|}}\n"
      "  inside partial\n"
      "{{/let partial}}\n"
      "hello\n");
  EXPECT_EQ(
      to_string(ast),
      "root [path/to/test-1.whisker]\n"
      "├─ header\n"
      "│  ╰─ comment <line:2:1, col:6> ''\n"
      "├─ newline <line:3:1, line:4:1> '\\n'\n"
      "├─ partial-block <line:4:1, line:6:17> 'foo'\n"
      "│  ├─ argument 'arg'\n"
      "│  ├─ text <line:5:1, col:17> '  inside partial'\n"
      "│  ╰─ newline <line:5:17, line:6:1> '\\n'\n"
      "├─ text <line:7:1, col:6> 'hello'\n"
      "╰─ newline <line:7:6, line:8:1> '\\n'\n");
}

TEST_F(ParserTest, partial_block_no_arguments) {
  auto ast = parse_ast("{{#let partial foo}}{{/let partial}}\n");
  EXPECT_EQ(
      to_string(ast),
      "root [path/to/test-1.whisker]\n"
      "╰─ partial-block <line:1:1, col:37> 'foo'\n");
}

TEST_F(ParserTest, partial_block_empty_arguments) {
  parse_ast_should_fail("{{#let partial foo ||}}{{/let partial}}\n");
  EXPECT_THAT(
      diagnostics,
      testing::ElementsAre(diagnostic(
          diagnostic_level::error,
          "expected at least one argument in partial-block 'foo' but found none",
          path_to_file(1),
          1)));
}

TEST_F(ParserTest, partial_block_duplicate_arguments) {
  parse_ast_should_fail("{{#let partial foo |arg arg|}}{{/let partial}}\n");
  EXPECT_THAT(
      diagnostics,
      testing::ElementsAre(diagnostic(
          diagnostic_level::error,
          "duplicate argument 'arg' in partial-block 'foo'",
          path_to_file(1),
          1)));
}

TEST_F(ParserTest, partial_block_empty_captures) {
  parse_ast_should_fail(
      "{{#let partial foo |arg| captures ||}}{{/let partial}}\n");
  EXPECT_THAT(
      diagnostics,
      testing::ElementsAre(diagnostic(
          diagnostic_level::error,
          "expected at least one capture in partial-block 'foo' but found none",
          path_to_file(1),
          1)));
}

TEST_F(ParserTest, partial_block_duplicate_captures) {
  parse_ast_should_fail(
      "{{#let partial foo |arg| captures |cap cap|}}{{/let partial}}\n");
  EXPECT_THAT(
      diagnostics,
      testing::ElementsAre(diagnostic(
          diagnostic_level::error,
          "duplicate capture 'cap' in partial-block 'foo'",
          path_to_file(1),
          1)));
}

TEST_F(ParserTest, partial_block_duplicate_argument_with_capture) {
  parse_ast_should_fail(
      "{{#let partial foo |arg| captures |arg|}}{{/let partial}}\n");
  EXPECT_THAT(
      diagnostics,
      testing::ElementsAre(diagnostic(
          diagnostic_level::error,
          "capture 'arg' conflicts with an argument in partial-block 'foo'",
          path_to_file(1),
          1)));
}

TEST_F(ParserTest, partial_block_missing_name) {
  parse_ast_should_fail("{{#let partial |arg1|}}{{/let partial}}\n");
  EXPECT_THAT(
      diagnostics,
      testing::ElementsAre(diagnostic(
          diagnostic_level::error,
          "expected identifier in partial-block but found `|`",
          path_to_file(1),
          1)));
}

TEST_F(ParserTest, partial_block_extra_tokens) {
  parse_ast_should_fail("{{#let partial foo |arg1| true}}{{/let partial}}\n");
  EXPECT_THAT(
      diagnostics,
      testing::ElementsAre(diagnostic(
          diagnostic_level::error,
          "expected `}}` to open partial-block 'foo' but found `true`",
          path_to_file(1),
          1)));
}

TEST_F(ParserTest, partial_block_without_close) {
  parse_ast_should_fail("{{#let partial foo |arg1|}}\n");
  EXPECT_THAT(
      diagnostics,
      testing::ElementsAre(diagnostic(
          diagnostic_level::error,
          "expected `{{` to close partial-block 'foo' but found EOF",
          path_to_file(1),
          2)));
}

TEST_F(ParserTest, partial_block_with_bad_close) {
  parse_ast_should_fail("{{#let partial foo |arg1|}}{{/let}}\n");
  EXPECT_THAT(
      diagnostics,
      testing::ElementsAre(diagnostic(
          diagnostic_level::error,
          "expected `partial` to close partial-block 'foo' but found `}}`",
          path_to_file(1),
          1)));
}

TEST_F(ParserTest, partial_block_nested) {
  auto ast = parse_ast(
      "{{#let partial foo |arg1 arg2|}}\n"
      "  {{#let partial bar |arg3 arg4|}}\n"
      "  {{/let partial}}\n"
      "{{/let partial}}\n");
  EXPECT_EQ(
      to_string(ast),
      "root [path/to/test-1.whisker]\n"
      "╰─ partial-block <line:1:1, line:4:17> 'foo'\n"
      "   ├─ argument 'arg1'\n"
      "   ├─ argument 'arg2'\n"
      "   ╰─ partial-block <line:2:3, line:3:19> 'bar'\n"
      "      ├─ argument 'arg3'\n"
      "      ╰─ argument 'arg4'\n");
}

TEST_F(ParserTest, partial_block_inside_body) {
  auto ast = parse_ast(
      "{{! header }}"
      "{{#let partial foo |arg1 arg2|}}\n"
      "  body inside a partial does not count\n"
      "{{/let partial}}\n"
      "{{#let partial foo2 |arg1 arg2|}}{{/let partial}}\n"
      "{{! body}}\n"
      "Some body text\n"
      "{{#let partial bar |arg1 arg2|}}{{/let partial}}\n"
      "{{#let partial baz |arg1 arg2|}}{{/let partial}}\n");
  EXPECT_EQ(
      to_string(ast),
      "root [path/to/test-1.whisker]\n"
      "├─ header\n"
      "│  ╰─ comment <line:1:1, col:14> ' header '\n"
      "├─ partial-block <line:1:14, line:3:17> 'foo'\n"
      "│  ├─ argument 'arg1'\n"
      "│  ├─ argument 'arg2'\n"
      "│  ├─ text <line:2:1, col:39> '  body inside a partial does not count'\n"
      "│  ╰─ newline <line:2:39, line:3:1> '\\n'\n"
      "├─ partial-block <line:4:1, col:50> 'foo2'\n"
      "│  ├─ argument 'arg1'\n"
      "│  ╰─ argument 'arg2'\n"
      "├─ comment <line:5:1, col:11> ' body'\n"
      "├─ text <line:6:1, col:15> 'Some body text'\n"
      "├─ newline <line:6:15, line:7:1> '\\n'\n"
      "├─ partial-block <line:7:1, col:49> 'bar'\n"
      "│  ├─ argument 'arg1'\n"
      "│  ╰─ argument 'arg2'\n"
      "╰─ partial-block <line:8:1, col:49> 'baz'\n"
      "   ├─ argument 'arg1'\n"
      "   ╰─ argument 'arg2'\n");
}

TEST_F(ParserTest, partial_statement_basic) {
  auto ast = parse_ast("{{#partial foo}}\n");
  EXPECT_EQ(
      to_string(ast),
      "root [path/to/test-1.whisker]\n"
      "╰─ partial-statement <line:1:1, col:17> 'foo'\n"
      "   ╰─ standalone-indentation ''\n");
}

TEST_F(ParserTest, partial_statement_with_args) {
  auto ast = parse_ast("{{#partial foo bar=1234}}\n");
  EXPECT_EQ(
      to_string(ast),
      "root [path/to/test-1.whisker]\n"
      "╰─ partial-statement <line:1:1, col:26> 'foo'\n"
      "   ├─ standalone-indentation ''\n"
      "   ╰─ argument 'bar=1234'\n");
}

TEST_F(ParserTest, partial_statement_expression) {
  auto ast =
      parse_ast("{{#partial (fetch-partial \"foo\" bar=1234) arg1=true}}\n");
  EXPECT_EQ(
      to_string(ast),
      "root [path/to/test-1.whisker]\n"
      "╰─ partial-statement <line:1:1, col:54> '(fetch-partial \"foo\" bar=1234)'\n"
      "   ├─ standalone-indentation ''\n"
      "   ╰─ argument 'arg1=true'\n");
}

TEST_F(ParserTest, partial_statement_empty) {
  parse_ast_should_fail("{{#partial}}\n");
  EXPECT_THAT(
      diagnostics,
      testing::ElementsAre(diagnostic(
          diagnostic_level::error,
          "expected partial name (expression) in partial-statement but found `}}`",
          path_to_file(1),
          1)));
}

TEST_F(ParserTest, partial_statement_missing_name) {
  parse_ast_should_fail(
      "{{#partial arg1=(some-expression 1 2 3 4) arg2=true}}\n");
  EXPECT_THAT(
      diagnostics,
      testing::ElementsAre(diagnostic(
          diagnostic_level::error,
          "expected partial name (expression) in partial-statement but found `=`",
          path_to_file(1),
          1)));
}

TEST_F(ParserTest, partial_statement_argument_without_name) {
  parse_ast_should_fail("{{#partial foo (some-expression 1 2 3 4)}}\n");
  EXPECT_THAT(
      diagnostics,
      testing::ElementsAre(diagnostic(
          diagnostic_level::error,
          "expected argument name in partial-statement but found `(`",
          path_to_file(1),
          1)));
}

TEST_F(ParserTest, partial_statement_missing_close) {
  parse_ast_should_fail("{{#partial foo arg1=true");
  EXPECT_THAT(
      diagnostics,
      testing::ElementsAre(diagnostic(
          diagnostic_level::error,
          "expected `}}` to close partial-statement but found EOF",
          path_to_file(1),
          1)));
}

TEST_F(ParserTest, partial_statement_standalone) {
  auto ast = parse_ast(" \t {{#partial foo arg1=true}} \t \n");
  EXPECT_EQ(
      to_string(ast),
      "root [path/to/test-1.whisker]\n"
      "├─ text <line:1:1, col:4> ' \\t '\n"
      "╰─ partial-statement <line:1:4, col:30> 'foo'\n"
      "   ├─ standalone-indentation ' \\t '\n"
      "   ╰─ argument 'arg1=true'\n");
}

TEST_F(ParserTest, partial_statement_not_standalone) {
  {
    auto ast = parse_ast(" \t {{#partial foo arg1=true}} \t {{> foo}}\n");
    EXPECT_EQ(
        to_string(ast),
        "root [path/to/test-1.whisker]\n"
        "├─ text <line:1:1, col:4> ' \\t '\n"
        "├─ partial-statement <line:1:4, col:30> 'foo'\n"
        "│  ╰─ argument 'arg1=true'\n"
        "├─ text <line:1:30, col:33> ' \\t '\n"
        "├─ macro <line:1:33, col:42> 'foo'\n"
        "╰─ newline <line:1:42, line:2:1> '\\n'\n");
  }

  {
    auto ast = parse_ast(
        "{{#if true}} \t {{#partial foo arg1=true}} \t {{/if true}}\n");
    EXPECT_EQ(
        to_string(ast),
        "root [path/to/test-2.whisker]\n"
        "├─ if-block <line:1:1, col:57>\n"
        "│  ├─ expression <line:1:7, col:11> 'true'\n"
        "│  ├─ text <line:1:13, col:16> ' \\t '\n"
        "│  ├─ partial-statement <line:1:16, col:42> 'foo'\n"
        "│  │  ╰─ argument 'arg1=true'\n"
        "│  ╰─ text <line:1:42, col:45> ' \\t '\n"
        "╰─ newline <line:1:57, line:2:1> '\\n'\n");
  }
}

TEST_F(ParserTest, basic_macro) {
  auto ast = parse_ast("{{> path / to / file }}");
  EXPECT_EQ(
      to_string(ast),
      "root [path/to/test-1.whisker]\n"
      "╰─ macro <line:1:1, col:24> 'path/to/file'\n"
      "   ╰─ standalone-indentation ''\n");
}

TEST_F(ParserTest, macro_single_id) {
  auto ast = parse_ast("{{> foo }}");
  EXPECT_EQ(
      to_string(ast),
      "root [path/to/test-1.whisker]\n"
      "╰─ macro <line:1:1, col:11> 'foo'\n"
      "   ╰─ standalone-indentation ''\n");
}

TEST_F(ParserTest, macro_in_section) {
  auto ast = parse_ast(
      "{{#news.has-update?}}\n"
      "  {{> print/news}}\n"
      "{{/news.has-update?}}");
  EXPECT_EQ(
      to_string(ast),
      "root [path/to/test-1.whisker]\n"
      "╰─ section-block <line:1:1, line:3:22>\n"
      "   ├─ variable-lookup <line:1:4, col:20> 'news.has-update?'\n"
      "   ├─ text <line:2:1, col:3> '  '\n"
      "   ╰─ macro <line:2:3, col:19> 'print/news'\n"
      "      ╰─ standalone-indentation '  '\n");
}

TEST_F(ParserTest, macro_preserves_whitespace_indentation) {
  auto ast = parse_ast(" \t {{> print/news}}\n");
  EXPECT_EQ(
      to_string(ast),
      "root [path/to/test-1.whisker]\n"
      "├─ text <line:1:1, col:4> ' \\t '\n"
      "╰─ macro <line:1:4, col:20> 'print/news'\n"
      "   ╰─ standalone-indentation ' \\t '\n");
}

TEST_F(ParserTest, macro_no_id) {
  parse_ast_should_fail("{{> }}");
  EXPECT_THAT(
      diagnostics,
      testing::ElementsAre(diagnostic(
          diagnostic_level::error,
          "expected macro-lookup in macro but found `}}`",
          path_to_file(1),
          1)));
}

TEST_F(ParserTest, macro_extra_stuff) {
  parse_ast_should_fail("{{> foo ! }}");
  EXPECT_THAT(
      diagnostics,
      testing::ElementsAre(diagnostic(
          diagnostic_level::error,
          "expected `}}` to close macro 'foo' but found `!`",
          path_to_file(1),
          1)));
}

TEST_F(ParserTest, macro_dotted_path) {
  auto ast = parse_ast("{{> path/to.file }}");
  EXPECT_EQ(
      to_string(ast),
      "root [path/to/test-1.whisker]\n"
      "╰─ macro <line:1:1, col:20> 'path/to.file'\n"
      "   ╰─ standalone-indentation ''\n");
}

TEST_F(ParserTest, macro_empty_path_part) {
  parse_ast_should_fail("{{> path//to.file }}");
  EXPECT_THAT(
      diagnostics,
      testing::ElementsAre(diagnostic(
          diagnostic_level::error,
          "expected path-component in macro-lookup but found `/`",
          path_to_file(1),
          1)));
}

TEST_F(ParserTest, unclosed_macro) {
  parse_ast_should_fail("{{> path/to/file");
  EXPECT_THAT(
      diagnostics,
      testing::ElementsAre(diagnostic(
          diagnostic_level::error,
          "expected `}}` to close macro 'path/to/file' but found EOF",
          path_to_file(1),
          1)));
}

TEST_F(ParserTest, let_statement) {
  auto ast = parse_ast("{{#let foo = (not true_value)}}");
  EXPECT_EQ(
      to_string(ast),
      "root [path/to/test-1.whisker]\n"
      "╰─ let-statement <line:1:1, col:32>\n"
      "   ├─ identifier 'foo'\n"
      "   ╰─ expression <line:1:14, col:30> '(not true_value)'\n");
}

TEST_F(ParserTest, let_statement_builtin_ternary) {
  auto ast = parse_ast("{{#let foo = (if cond true_value false_value)}}");
  EXPECT_EQ(
      to_string(ast),
      "root [path/to/test-1.whisker]\n"
      "╰─ let-statement <line:1:1, col:48>\n"
      "   ├─ identifier 'foo'\n"
      "   ╰─ expression <line:1:14, col:46> '(if cond true_value false_value)'\n");
}

TEST_F(ParserTest, let_statement_export) {
  auto ast = parse_ast("{{#let export foo = (hello world)}}");
  EXPECT_EQ(
      to_string(ast),
      "root [path/to/test-1.whisker]\n"
      "╰─ let-statement <line:1:1, col:36>\n"
      "   ├─ exported\n"
      "   ├─ identifier 'foo'\n"
      "   ╰─ expression <line:1:21, col:34> '(hello world)'\n");
}

TEST_F(ParserTest, let_statement_with_implicit_context) {
  auto ast = parse_ast("{{#let foo = (a.b.c this)}}");
  EXPECT_EQ(
      to_string(ast),
      "root [path/to/test-1.whisker]\n"
      "╰─ let-statement <line:1:1, col:28>\n"
      "   ├─ identifier 'foo'\n"
      "   ╰─ expression <line:1:14, col:26> '(a.b.c this)'\n");
}

TEST_F(ParserTest, let_statement_dotted_name) {
  parse_ast_should_fail("{{#let foo.bar = (not true_value)}}");
  EXPECT_THAT(
      diagnostics,
      testing::ElementsAre(diagnostic(
          diagnostic_level::error,
          "expected `=` in let-statement but found `.`",
          path_to_file(1),
          1)));
}

TEST_F(ParserTest, let_statement_missing_id) {
  parse_ast_should_fail("{{#let (not true_value)}}");
  EXPECT_THAT(
      diagnostics,
      testing::ElementsAre(diagnostic(
          diagnostic_level::error,
          "expected identifier in let-statement but found `(`",
          path_to_file(1),
          1)));
}

TEST_F(ParserTest, let_statement_missing_eq) {
  parse_ast_should_fail("{{#let foo (not true_value)}}");
  EXPECT_THAT(
      diagnostics,
      testing::ElementsAre(diagnostic(
          diagnostic_level::error,
          "expected `=` in let-statement but found `(`",
          path_to_file(1),
          1)));
}

TEST_F(ParserTest, let_statement_missing_expression) {
  parse_ast_should_fail("{{#let foo =}}");
  EXPECT_THAT(
      diagnostics,
      testing::ElementsAre(diagnostic(
          diagnostic_level::error,
          "expected expression in let-statement but found `}}`",
          path_to_file(1),
          1)));
}

TEST_F(ParserTest, let_statement_missing_eq_and_expression) {
  parse_ast_should_fail("{{#let foo}}");
  EXPECT_THAT(
      diagnostics,
      testing::ElementsAre(diagnostic(
          diagnostic_level::error,
          "expected `=` in let-statement but found `}}`",
          path_to_file(1),
          1)));
}

TEST_F(ParserTest, let_statement_missing_close) {
  parse_ast_should_fail("{{#let foo = (not true_value)");
  EXPECT_THAT(
      diagnostics,
      testing::ElementsAre(diagnostic(
          diagnostic_level::error,
          "expected `}}` to close let-statement but found EOF",
          path_to_file(1),
          1)));
}

TEST_F(ParserTest, let_statement_keyword) {
  parse_ast_should_fail("{{#let let = (not true_value)");
  EXPECT_THAT(
      diagnostics,
      testing::ElementsAre(diagnostic(
          diagnostic_level::error,
          "expected identifier in let-statement but found `let`",
          path_to_file(1),
          1)));
}

TEST_F(ParserTest, pragma_ignore_newlines) {
  auto ast = parse_ast("{{#pragma ignore-newlines}}\n");
  EXPECT_EQ(
      to_string(ast),
      "root [path/to/test-1.whisker]\n"
      "╰─ header\n"
      "   ╰─ pragma-statement 'ignore-newlines' <line:1:1, col:28>\n");
}

TEST_F(ParserTest, pragma_unrecognized) {
  parse_ast_should_fail("{{#pragma unrecognized}}\n");
  EXPECT_THAT(
      diagnostics,
      testing::ElementsAre(diagnostic(
          diagnostic_level::error,
          "unknown pragma 'unrecognized'",
          path_to_file(1),
          1)));
}

TEST_F(ParserTest, with_block) {
  auto ast = parse_ast(
      "{{#with foo.bar}}\n"
      "{{bar}}\n"
      "{{/with}}\n");
  EXPECT_EQ(
      to_string(ast),
      "root [path/to/test-1.whisker]\n"
      "╰─ with-block <line:1:1, line:3:10>\n"
      "   ├─ expression <line:1:9, col:16> 'foo.bar'\n"
      "   ├─ interpolation <line:2:1, col:8> 'bar'\n"
      "   ╰─ newline <line:2:8, line:3:1> '\\n'\n");
}

TEST_F(ParserTest, with_block_no_expression) {
  parse_ast_should_fail(
      "{{#with}}\n"
      "{{bar}}\n"
      "{{/with}}\n");
  EXPECT_THAT(
      diagnostics,
      testing::ElementsAre(diagnostic(
          diagnostic_level::error,
          "expected expression to open with-block but found `}}`",
          path_to_file(1),
          1)));
}

TEST_F(ParserTest, with_block_multiple_expression) {
  parse_ast_should_fail(
      "{{#with foo.bar bar.baz}}\n"
      "{{bar}}\n"
      "{{/with}}\n");
  EXPECT_THAT(
      diagnostics,
      testing::ElementsAre(diagnostic(
          diagnostic_level::error,
          "expected `}}` to open with-block but found identifier",
          path_to_file(1),
          1)));
}

TEST_F(ParserTest, with_block_missing_close) {
  parse_ast_should_fail(
      "{{#with foo.bar}}\n"
      "{{bar}}\n");
  EXPECT_THAT(
      diagnostics,
      testing::ElementsAre(diagnostic(
          diagnostic_level::error,
          "expected `{{` to close with-block 'foo.bar' but found EOF",
          path_to_file(1),
          3)));
}

TEST_F(ParserTest, comment) {
  auto ast = parse_ast("Hello{{! #$^& random text }}world");
  EXPECT_EQ(
      to_string(ast),
      "root [path/to/test-1.whisker]\n"
      "├─ text <line:1:1, col:6> 'Hello'\n"
      "├─ comment <line:1:6, col:29> ' #$^& random text '\n"
      "╰─ text <line:1:29, col:34> 'world'\n");
}

TEST_F(ParserTest, comment_empty) {
  auto ast = parse_ast("{{!}}");
  EXPECT_EQ(
      to_string(ast),
      "root [path/to/test-1.whisker]\n"
      "╰─ header\n"
      "   ╰─ comment <line:1:1, col:6> ''\n");
}

TEST_F(ParserTest, comment_escaped) {
  auto ast = parse_ast(
      "Hello{{!-- \n"
      "next line }} still comment --}}world");
  EXPECT_EQ(
      to_string(ast),
      "root [path/to/test-1.whisker]\n"
      "├─ text <line:1:1, col:6> 'Hello'\n"
      "├─ comment <line:1:6, line:2:32> ' \\nnext line }} still comment '\n"
      "╰─ text <line:2:32, col:37> 'world'\n");
}

TEST_F(ParserTest, comment_escaped_empty) {
  auto ast = parse_ast("{{!----}}");
  EXPECT_EQ(
      to_string(ast),
      "root [path/to/test-1.whisker]\n"
      "╰─ header\n"
      "   ╰─ comment <line:1:1, col:10> ''\n");
}

TEST_F(ParserTest, header_consumes_whitespace) {
  auto ast = parse_ast(
      "\n"
      "{{!}}\n"
      " \t\n"
      "{{#pragma ignore-newlines}}\t\n"
      "\n"
      "hello\n"
      "{{#pragma ignore-newlines}}\n");
  // Consumes newlines within a header.
  // Does not consume newlines after the last header element.
  EXPECT_EQ(
      to_string(ast),
      "root [path/to/test-1.whisker]\n"
      "├─ header\n"
      "│  ├─ comment <line:2:1, col:6> ''\n"
      "│  ╰─ pragma-statement 'ignore-newlines' <line:4:1, col:28>\n"
      "├─ newline <line:5:1, line:6:1> '\\n'\n"
      "├─ text <line:6:1, col:6> 'hello'\n"
      "├─ newline <line:6:6, line:7:1> '\\n'\n"
      "╰─ pragma-statement 'ignore-newlines' <line:7:1, col:28>\n");
}

TEST_F(ParserTest, import_statement_basic) {
  auto ast = parse_ast("{{#import \"path/to/file\" as foo}}");
  EXPECT_EQ(
      to_string(ast),
      "root [path/to/test-1.whisker]\n"
      "╰─ header\n"
      "   ╰─ import-statement <line:1:1, col:34>\n"
      "      ├─ path 'path/to/file'\n"
      "      ╰─ name 'foo'\n");
}

TEST_F(ParserTest, import_statement_with_header_elements) {
  auto ast = parse_ast(
      "{{! comment }}\n"
      "\n"
      "{{#pragma ignore-newlines}}\n"
      "\n"
      "{{#import \"path/to/file\" as foo}}\n");
  EXPECT_EQ(
      to_string(ast),
      "root [path/to/test-1.whisker]\n"
      "╰─ header\n"
      "   ├─ comment <line:1:1, col:15> ' comment '\n"
      "   ├─ pragma-statement 'ignore-newlines' <line:3:1, col:28>\n"
      "   ╰─ import-statement <line:5:1, col:34>\n"
      "      ├─ path 'path/to/file'\n"
      "      ╰─ name 'foo'\n");
}

TEST_F(ParserTest, import_statement_multiple) {
  auto ast = parse_ast(
      "\n"
      "{{#pragma ignore-newlines}}\n"
      "\n"
      "\t{{#import \"path/to/file\" as foo}}\n"
      "{{! comment }}\n"
      "  {{#import \"path/to/other\" as bar}}\n"
      "\n"
      "Some text");
  EXPECT_EQ(
      to_string(ast),
      "root [path/to/test-1.whisker]\n"
      "├─ header\n"
      "│  ├─ pragma-statement 'ignore-newlines' <line:2:1, col:28>\n"
      "│  ├─ import-statement <line:4:2, col:35>\n"
      "│  │  ├─ path 'path/to/file'\n"
      "│  │  ╰─ name 'foo'\n"
      "│  ├─ comment <line:5:1, col:15> ' comment '\n"
      "│  ╰─ import-statement <line:6:3, col:37>\n"
      "│     ├─ path 'path/to/other'\n"
      "│     ╰─ name 'bar'\n"
      "├─ newline <line:7:1, line:8:1> '\\n'\n"
      "╰─ text <line:8:1, col:10> 'Some text'\n");
}

TEST_F(ParserTest, import_statement_not_allowed_in_body) {
  parse_ast_should_fail(
      "{{! comment in header }}\n"
      "Hello\n"
      "{{#import \"path/to/file\" as foo}}\n");
  EXPECT_THAT(
      diagnostics,
      testing::ElementsAre(diagnostic(
          diagnostic_level::error,
          "import statements must appear in the header of a source file",
          path_to_file(1),
          3)));
}

TEST_F(ParserTest, import_statement_not_allowed_in_nested_body) {
  parse_ast_should_fail(
      "{{#if true}}\n"
      "  {{#import \"path/to/file\" as foo}}\n"
      "{{/if true}}\n");
  EXPECT_THAT(
      diagnostics,
      testing::ElementsAre(diagnostic(
          diagnostic_level::error,
          "import statements must appear in the header of a source file",
          path_to_file(1),
          2)));
}

TEST_F(ParserTest, import_statement_missing_as_and_name) {
  parse_ast_should_fail("{{#import \"path/to/file\"}}");
  EXPECT_THAT(
      diagnostics,
      testing::ElementsAre(diagnostic(
          diagnostic_level::error,
          "expected `as` in import-statement but found `}}`",
          path_to_file(1),
          1)));
}

TEST_F(ParserTest, import_statement_missing_name) {
  parse_ast_should_fail("{{#import \"path/to/file\" as}}");
  EXPECT_THAT(
      diagnostics,
      testing::ElementsAre(diagnostic(
          diagnostic_level::error,
          "expected identifier in import-statement but found `}}`",
          path_to_file(1),
          1)));
}

TEST_F(ParserTest, import_statement_name_is_expression) {
  parse_ast_should_fail(R"({{#import "path/to/file" as "foo"}})");
  EXPECT_THAT(
      diagnostics,
      testing::ElementsAre(diagnostic(
          diagnostic_level::error,
          "expected identifier in import-statement but found string literal",
          path_to_file(1),
          1)));
}

TEST_F(ParserTest, import_statement_name_is_not_string_literal) {
  parse_ast_should_fail("{{#import path/to/file as foo}}");
  EXPECT_THAT(
      diagnostics,
      testing::ElementsAre(diagnostic(
          diagnostic_level::error,
          "expected import-path in import-statement but found identifier",
          path_to_file(1),
          1)));
}

TEST_F(ParserTest, strip_standalone_lines) {
  auto ast = parse_ast(
      "| This Is\n"
      "{{#boolean}}\n"
      "|\n"
      "{{/boolean}}\n"
      "| A Line\n");
  EXPECT_EQ(
      to_string(ast),
      "root [path/to/test-1.whisker]\n"
      "├─ text <line:1:1, col:10> '| This Is'\n"
      "├─ newline <line:1:10, line:2:1> '\\n'\n"
      "├─ section-block <line:2:1, line:4:13>\n"
      "│  ├─ variable-lookup <line:2:4, col:11> 'boolean'\n"
      "│  ├─ text <line:3:1, col:2> '|'\n"
      "│  ╰─ newline <line:3:2, line:4:1> '\\n'\n"
      "├─ text <line:5:1, col:9> '| A Line'\n"
      "╰─ newline <line:5:9, line:6:1> '\\n'\n");
}

TEST_F(ParserTest, strip_standalone_lines_indented) {
  auto ast = parse_ast(
      "| This Is\n"
      "  {{#boolean}}\n"
      "|\n"
      "  {{/boolean}}\n"
      "| A Line\n");
  EXPECT_EQ(
      to_string(ast),
      "root [path/to/test-1.whisker]\n"
      "├─ text <line:1:1, col:10> '| This Is'\n"
      "├─ newline <line:1:10, line:2:1> '\\n'\n"
      "├─ section-block <line:2:3, line:4:15>\n"
      "│  ├─ variable-lookup <line:2:6, col:13> 'boolean'\n"
      "│  ├─ text <line:3:1, col:2> '|'\n"
      "│  ╰─ newline <line:3:2, line:4:1> '\\n'\n"
      "├─ text <line:5:1, col:9> '| A Line'\n"
      "╰─ newline <line:5:9, line:6:1> '\\n'\n");
}

TEST_F(ParserTest, strip_standalone_lines_indented_at_eof) {
  auto ast = parse_ast(
      "| This Is\n"
      "  {{#boolean}}\n"
      "|\n"
      "  {{/boolean}}  ");
  EXPECT_EQ(
      to_string(ast),
      "root [path/to/test-1.whisker]\n"
      "├─ text <line:1:1, col:10> '| This Is'\n"
      "├─ newline <line:1:10, line:2:1> '\\n'\n"
      "╰─ section-block <line:2:3, line:4:15>\n"
      "   ├─ variable-lookup <line:2:6, col:13> 'boolean'\n"
      "   ├─ text <line:3:1, col:2> '|'\n"
      "   ╰─ newline <line:3:2, line:4:1> '\\n'\n");
}

TEST_F(
    ParserTest, strip_standalone_lines_indented_at_eof_ending_with_template) {
  auto ast = parse_ast(
      "| This Is\n"
      "  {{#boolean}}\n"
      "|\n"
      "  {{/boolean}}");
  EXPECT_EQ(
      to_string(ast),
      "root [path/to/test-1.whisker]\n"
      "├─ text <line:1:1, col:10> '| This Is'\n"
      "├─ newline <line:1:10, line:2:1> '\\n'\n"
      "╰─ section-block <line:2:3, line:4:15>\n"
      "   ├─ variable-lookup <line:2:6, col:13> 'boolean'\n"
      "   ├─ text <line:3:1, col:2> '|'\n"
      "   ╰─ newline <line:3:2, line:4:1> '\\n'\n");
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
      to_string(ast),
      "root [path/to/test-1.whisker]\n"
      "├─ text <line:1:1, col:10> '| This Is'\n"
      "├─ newline <line:1:10, line:2:1> '\\n'\n"
      "├─ section-block <line:2:3, line:5:15>\n"
      "│  ├─ variable-lookup <line:2:6, col:13> 'boolean'\n"
      "│  ╰─ section-block <line:2:16, line:4:15>\n"
      "│     ├─ variable-lookup <line:2:19, col:26> 'boolean'\n"
      "│     ├─ text <line:3:1, col:2> '|'\n"
      "│     ╰─ newline <line:3:2, line:4:1> '\\n'\n"
      "├─ text <line:6:1, col:9> '| A Line'\n"
      "╰─ newline <line:6:9, line:7:1> '\\n'\n");
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
      to_string(ast),
      "root [path/to/test-1.whisker]\n"
      "├─ text <line:1:1, col:10> '| This Is'\n"
      "├─ newline <line:1:10, line:2:1> '\\n'\n"
      "├─ section-block <line:2:3, line:5:25>\n"
      "│  ├─ variable-lookup <line:2:6, line:3:18> 'boolean.condition'\n"
      "│  ├─ text <line:4:1, col:2> '|'\n"
      "│  ╰─ newline <line:4:2, line:5:1> '\\n'\n"
      "├─ text <line:6:1, col:9> '| A Line'\n"
      "╰─ newline <line:6:9, line:7:1> '\\n'\n");
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
      to_string(ast),
      "root [path/to/test-1.whisker]\n"
      "├─ text <line:1:1, col:10> '| This Is'\n"
      "├─ newline <line:1:10, line:2:1> '\\n'\n"
      "├─ section-block <inverted> <line:2:3, line:5:25>\n"
      "│  ├─ variable-lookup <line:2:6, line:3:18> 'boolean.condition'\n"
      "│  ├─ comment <line:3:21, col:38> ' unaffected '\n"
      "│  ├─ text <line:4:1, col:2> '|'\n"
      "│  ╰─ newline <line:4:2, line:5:1> '\\n'\n"
      "├─ text <line:6:1, col:9> '| A Line'\n"
      "╰─ newline <line:6:9, line:7:1> '\\n'\n");
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
      to_string(ast),
      "root [path/to/test-1.whisker]\n"
      "├─ text <line:1:1, col:10> '| This Is'\n"
      "├─ newline <line:1:10, line:2:1> '\\n'\n"
      "├─ text <line:2:1, col:3> '  '\n"
      "├─ section-block <line:2:3, line:5:25>\n"
      "│  ├─ variable-lookup <line:2:6, line:3:18> 'boolean.condition'\n"
      "│  ├─ text <line:3:20, col:21> ' '\n"
      "│  ├─ interpolation <line:3:21, col:35> 'ineligible'\n"
      "│  ├─ text <line:3:35, col:36> ' '\n"
      "│  ├─ newline <line:3:36, line:4:1> '\\n'\n"
      "│  ├─ text <line:4:1, col:2> '|'\n"
      "│  ╰─ newline <line:4:2, line:5:1> '\\n'\n"
      "├─ text <line:6:1, col:9> '| A Line'\n"
      "╰─ newline <line:6:9, line:7:1> '\\n'\n");
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
      to_string(ast),
      "root [path/to/test-1.whisker]\n"
      "├─ text <line:1:1, col:10> '| This Is'\n"
      "├─ newline <line:1:10, line:2:1> '\\n'\n"
      "├─ text <line:2:1, col:3> '  '\n"
      "├─ section-block <line:2:3, line:5:25>\n"
      "│  ├─ variable-lookup <line:2:6, line:3:18> 'boolean.condition'\n"
      "│  ├─ text <line:3:20, col:21> ' '\n"
      "│  ├─ macro <line:3:21, col:37> 'ineligible'\n"
      "│  ├─ text <line:3:37, col:38> ' '\n"
      "│  ├─ newline <line:3:38, line:4:1> '\\n'\n"
      "│  ├─ text <line:4:1, col:2> '|'\n"
      "│  ╰─ newline <line:4:2, line:5:1> '\\n'\n"
      "├─ text <line:6:1, col:9> '| A Line'\n"
      "╰─ newline <line:6:9, line:7:1> '\\n'\n");
}

} // namespace whisker
