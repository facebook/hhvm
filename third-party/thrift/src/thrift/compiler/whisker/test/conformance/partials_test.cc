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

#include <thrift/compiler/whisker/object.h>
#include <thrift/compiler/whisker/test/render_test_helpers.h>

namespace w = whisker::make;

namespace whisker {

class PartialsTest : public RenderTest {};

// The greater-than operator should expand to the named partial.
//   https://github.com/mustache/spec/blob/v1.4.2/specs/partials.yml#L17-L22
TEST_F(PartialsTest, BasicBehavior) {
  EXPECT_EQ(
      "\"from partial\"",
      render("\"{{>text}}\"", w::map({}), sources({{"text", "from partial"}})));
}

// NOTE:
//   Whisker intentionally rejects failed lookups for partials.
//   This means that Whisker is not conformant with the FailedLookup test.
//
// The empty string should be used when the named partial is not found.
//   https://github.com/mustache/spec/blob/v1.4.2/specs/partials.yml#L24-L29
TEST_F(PartialsTest, FailedLookup) {
  // In Mustache, the expected output is "".
  EXPECT_EQ(std::nullopt, render("\"{{>text}}\"", w::map({})));
}

// The greater-than operator should operate within the current context.
//   https://github.com/mustache/spec/blob/v1.4.2/specs/partials.yml#L31-L36
TEST_F(PartialsTest, Context) {
  EXPECT_EQ(
      "\"*content*\"",
      *render(
          "\"{{>partial}}\"",
          w::map({{"text", w::string("content")}}),
          sources({{"partial", "*{{text}}*"}})));
}

// The greater-than operator should properly recurse.
//   https://github.com/mustache/spec/blob/v1.4.2/specs/partials.yml#L38-L43
TEST_F(PartialsTest, Recursion) {
  EXPECT_EQ(
      "X<Y<>>",
      *render(
          "{{>node}}",
          w::map(
              {{"content", w::string("X")},
               {"nodes",
                w::array({w::map(
                    {{"content", w::string("Y")},
                     {"nodes", w::array({})}})})}}),
          sources({{"node", "{{content}}<{{#nodes}}{{>node}}{{/nodes}}>"}})));
}

// The greater-than operator should work from within partials.
//   https://github.com/mustache/spec/blob/v1.4.2/specs/partials.yml#L45-L50
TEST_F(PartialsTest, Nested) {
  EXPECT_EQ(
      "*hello world!*",
      *render(
          "{{>outer}}",
          w::map({{"a", w::string("hello")}, {"b", w::string("world")}}),
          sources({{"outer", "*{{a}} {{>inner}}*"}, {"inner", "{{b}}!"}})));
}

// The greater-than operator should not alter surrounding whitespace.
//   https://github.com/mustache/spec/blob/v1.4.2/specs/partials.yml#L54-L59
TEST_F(PartialsTest, SurroundingWhitespace) {
  EXPECT_EQ(
      "| \t|\t |",
      *render("| {{>partial}} |", w::map({}), sources({{"partial", "\t|\t"}})));
}

// Whitespace should be left untouched.
//   https://github.com/mustache/spec/blob/v1.4.2/specs/partials.yml#L61-L66
TEST_F(PartialsTest, InlineIndentation) {
  EXPECT_EQ(
      "  |  >\n>\n",
      *render(
          "  {{data}}  {{> partial}}\n",
          w::map({{"data", w::string("|")}}),
          sources({{"partial", ">\n>"}})));
}

// "\r\n" should be considered a newline for standalone tags.
//   https://github.com/mustache/spec/blob/v1.4.2/specs/partials.yml#L68-L73
TEST_F(PartialsTest, StandaloneLineEndings) {
  EXPECT_EQ(
      "|\r\n>|",
      *render(
          "|\r\n{{>partial}}\r\n|", w::map({}), sources({{"partial", ">"}})));
}

// Standalone tags should not require a newline to precede them.
//   https://github.com/mustache/spec/blob/v1.4.2/specs/partials.yml#L75-L80
TEST_F(PartialsTest, StandaloneWithoutPreviousLine) {
  EXPECT_EQ(
      "  >\n  >>",
      *render("  {{>partial}}\n>", w::map({}), sources({{"partial", ">\n>"}})));
}

// Standalone tags should not require a newline to follow them.
//   https://github.com/mustache/spec/blob/v1.4.2/specs/partials.yml#L82-L87
TEST_F(PartialsTest, StandaloneWithoutNewline) {
  EXPECT_EQ(
      ">\n  >\n  >",
      *render(">\n  {{>partial}}", w::map({}), sources({{"partial", ">\n>"}})));
}

// Each line of the partial should be indented before rendering.
//   https://github.com/mustache/spec/blob/v1.4.2/specs/partials.yml#L89-L107
TEST_F(PartialsTest, StandaloneIndentation) {
  EXPECT_EQ(
      "\\\n"
      " |\n"
      " <\n"
      " ->\n"
      " |\n/\n",
      *render(
          "\\\n"
          " {{>partial}}\n"
          "/\n",
          w::map({{"content", w::string("<\n->")}}),
          sources(
              {{"partial",
                "|\n"
                "{{content}}\n"
                "|\n"}})));
}

// Superfluous in-tag whitespace should be ignored.
//   https://github.com/mustache/spec/blob/v1.4.2/specs/partials.yml#L111-L116
TEST_F(PartialsTest, PaddingWhitespace) {
  EXPECT_EQ(
      "|[]|",
      *render(
          "|{{> partial }}|",
          w::map({{"boolean", w::boolean(true)}}),
          sources({{"partial", "[]"}})));
}

} // namespace whisker
