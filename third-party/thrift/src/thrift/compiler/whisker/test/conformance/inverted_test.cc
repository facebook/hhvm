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

class InvertedTest : public RenderTest {};

// Falsey sections should have their contents rendered.
//   https://github.com/mustache/spec/blob/v1.4.2/specs/inverted.yml#L37-L41
TEST_F(InvertedTest, Falsey) {
  EXPECT_EQ(
      "\"This should be rendered.\"",
      *render(
          "\"{{^boolean}}This should be rendered.{{/boolean}}\"",
          w::map({{"boolean", w::boolean(false)}})));
}

// Truthy sections should have their contents omitted.
//   https://github.com/mustache/spec/blob/v1.4.2/specs/inverted.yml#L43-L47
TEST_F(InvertedTest, Truthy) {
  EXPECT_EQ(
      "\"\"",
      *render(
          "\"{{^boolean}}This should not be rendered.{{/boolean}}\"",
          w::map({{"boolean", w::boolean(true)}})));
}

// Null is falsey.
//   https://github.com/mustache/spec/blob/v1.4.2/specs/inverted.yml#L49-L53
TEST_F(InvertedTest, NullIsFalsey) {
  strict_boolean_conditional = diagnostic_level::debug;
  EXPECT_EQ(
      "\"This should be rendered.\"",
      *render(
          "\"{{^boolean}}This should be rendered.{{/boolean}}\"",
          w::map({{"boolean", w::null}})));
}

// Objects and hashes should behave like truthy values.
//   https://github.com/mustache/spec/blob/v1.4.2/specs/inverted.yml#L55-L59
TEST_F(InvertedTest, Context) {
  strict_boolean_conditional = diagnostic_level::debug;
  EXPECT_EQ(
      "\"\"",
      *render(
          "\"{{^context}}Hi {{name}}.{{/context}}\"",
          w::map({{"context", w::map({{"name", w::string("Joe")}})}})));
}

// Lists should behave like truthy values.
//   https://github.com/mustache/spec/blob/v1.4.2/specs/inverted.yml#L61-L65
TEST_F(InvertedTest, List) {
  strict_boolean_conditional = diagnostic_level::debug;
  EXPECT_EQ(
      "\"\"",
      *render(
          "\"{{^list}}{{n}}{{/list}}\"",
          w::map(
              {{"list",
                w::array({
                    w::map({{"n", w::i64(1)}}),
                    w::map({{"n", w::i64(2)}}),
                    w::map({{"n", w::i64(3)}}),
                })}})));
}

// Empty lists should behave like falsey values.
//   https://github.com/mustache/spec/blob/v1.4.2/specs/inverted.yml#L67-L71
TEST_F(InvertedTest, EmptyList) {
  strict_boolean_conditional = diagnostic_level::debug;
  EXPECT_EQ(
      "\"Yay lists!\"",
      *render(
          "\"{{^list}}Yay lists!{{/list}}\"",
          w::map({{"list", w::array({})}})));
}

// Multiple inverted sections per template should be permitted.
//   https://github.com/mustache/spec/blob/v1.4.2/specs/inverted.yml#L73-L87
TEST_F(InvertedTest, Doubled) {
  EXPECT_EQ(
      "* first\n"
      "* second\n"
      "* third\n",
      *render(
          "{{^bool}}\n"
          "* first\n"
          "{{/bool}}\n"
          "* {{two}}\n"
          "{{^bool}}\n"
          "* third\n"
          "{{/bool}}\n",
          w::map({{"bool", w::boolean(false)}, {"two", w::string("second")}})));
}

// Nested falsey sections should have their contents rendered.
//   https://github.com/mustache/spec/blob/v1.4.2/specs/inverted.yml#L89-L93
TEST_F(InvertedTest, NestedFalsey) {
  EXPECT_EQ(
      "| A B C D E |",
      *render(
          "| A {{^bool}}B {{^bool}}C{{/bool}} D{{/bool}} E |",
          w::map({{"bool", w::boolean(false)}})));
}

// Nested truthy sections should be omitted.
//   https://github.com/mustache/spec/blob/v1.4.2/specs/inverted.yml#L95-L99
TEST_F(InvertedTest, NestedTruthy) {
  EXPECT_EQ(
      "| A  E |",
      *render(
          "| A {{^bool}}B {{^bool}}C{{/bool}} D{{/bool}} E |",
          w::map({{"bool", w::boolean(true)}})));
}

// Failed context lookups should be considered falsey.
//   https://github.com/mustache/spec/blob/v1.4.2/specs/inverted.yml#L101-L105
TEST_F(InvertedTest, ContextMisses) {
  strict_undefined_variables = diagnostic_level::debug;
  strict_boolean_conditional = diagnostic_level::debug;
  EXPECT_EQ(
      "[Cannot find key 'missing'!]",
      *render(
          "[{{^missing}}Cannot find key 'missing'!{{/missing}}]", w::map({})));
}

// Dotted names should be valid for Inverted Section tags.
//   https://github.com/mustache/spec/blob/v1.4.2/specs/inverted.yml#L109-L113
TEST_F(InvertedTest, DottedNamesTruthy) {
  EXPECT_EQ(
      "\"\" == \"\"",
      *render(
          "\"{{^a.b.c}}Not Here{{/a.b.c}}\" == \"\"",
          w::map({{"a", w::map({{"b", w::map({{"c", w::boolean(true)}})}})}})));
}

// Dotted names should be valid for Inverted Section tags.
//   https://github.com/mustache/spec/blob/v1.4.2/specs/inverted.yml#L115-L119
TEST_F(InvertedTest, DottedNamesFalsey) {
  EXPECT_EQ(
      "\"Not Here\" == \"Not Here\"",
      *render(
          "\"{{^a.b.c}}Not Here{{/a.b.c}}\" == \"Not Here\"",
          w::map(
              {{"a", w::map({{"b", w::map({{"c", w::boolean(false)}})}})}})));
}

// Dotted names that cannot be resolved should be considered falsey.
//   https://github.com/mustache/spec/blob/v1.4.2/specs/inverted.yml#L121-L125
TEST_F(InvertedTest, DottedNamesBrokenChains) {
  strict_undefined_variables = diagnostic_level::debug;
  strict_boolean_conditional = diagnostic_level::debug;
  EXPECT_EQ(
      "\"Not Here\" == \"Not Here\"",
      *render(
          "\"{{^a.b.c}}Not Here{{/a.b.c}}\" == \"Not Here\"",
          w::map({{"a", w::map({})}})));
}

// Inverted sections should not alter surrounding whitespace.
//   https://github.com/mustache/spec/blob/v1.4.2/specs/inverted.yml#L129-L133
TEST_F(InvertedTest, SurroundingWhitespace) {
  EXPECT_EQ(
      " | \t|\t | \n",
      *render(
          " | {{^boolean}}\t|\t{{/boolean}} | \n",
          w::map({{"boolean", w::boolean(false)}})));
}

// Inverted should not alter internal whitespace.
//   https://github.com/mustache/spec/blob/v1.4.2/specs/inverted.yml#L135-L139
TEST_F(InvertedTest, InternalWhitespace) {
  EXPECT_EQ(
      " |  \n  | \n",
      *render(
          " | {{^boolean}} {{! Important Whitespace }}\n {{/boolean}} | \n",
          w::map({{"boolean", w::boolean(false)}})));
}

// Single-line sections should not alter surrounding whitespace.
//   https://github.com/mustache/spec/blob/v1.4.2/specs/inverted.yml#L141-L145
TEST_F(InvertedTest, IndentedInlineSections) {
  EXPECT_EQ(
      " NO\n WAY\n",
      *render(
          " {{^boolean}}NO{{/boolean}}\n {{^boolean}}WAY{{/boolean}}\n",
          w::map({{"boolean", w::boolean(false)}})));
}

// Standalone lines should be removed from the template.
//   https://github.com/mustache/spec/blob/v1.4.2/specs/inverted.yml#L147-L159
TEST_F(InvertedTest, StandaloneLines) {
  EXPECT_EQ(
      "| This Is\n"
      "|\n"
      "| A Line\n",
      *render(
          "| This Is\n"
          "{{^boolean}}\n"
          "|\n"
          "{{/boolean}}\n"
          "| A Line\n",
          w::map({{"boolean", w::boolean(false)}})));
}

// Standalone indented lines should be removed from the template.
//   https://github.com/mustache/spec/blob/v1.4.2/specs/inverted.yml#L161-L173
TEST_F(InvertedTest, StandaloneIndentedLines) {
  EXPECT_EQ(
      "| This Is\n"
      "|\n"
      "| A Line\n",
      *render(
          "| This Is\n"
          "  {{^boolean}}\n"
          "|\n"
          "  {{/boolean}}\n"
          "| A Line\n",
          w::map({{"boolean", w::boolean(false)}})));
}

// "\r\n" should be considered a newline for standalone tags.
//   https://github.com/mustache/spec/blob/v1.4.2/specs/inverted.yml#L175-L179
TEST_F(InvertedTest, StandaloneLineEndings) {
  EXPECT_EQ(
      "|\r\n|",
      *render(
          "|\r\n{{^boolean}}\r\n{{/boolean}}\r\n|",
          w::map({{"boolean", w::boolean(false)}})));
}

// Standalone tags should not require a newline to precede them.
//   https://github.com/mustache/spec/blob/v1.4.2/specs/inverted.yml#L181-L185
TEST_F(InvertedTest, StandaloneWithoutPreviousLine) {
  EXPECT_EQ(
      "^\n/",
      *render(
          "  {{^boolean}}\n^{{/boolean}}\n/",
          w::map({{"boolean", w::boolean(false)}})));
}

// Standalone tags should not require a newline to follow them.
//   https://github.com/mustache/spec/blob/v1.4.2/specs/inverted.yml#L187-L191
TEST_F(InvertedTest, StandaloneWithoutNewline) {
  EXPECT_EQ(
      "^\n/\n",
      *render(
          "^{{^boolean}}\n/\n  {{/boolean}}",
          w::map({{"boolean", w::boolean(false)}})));
}

// Superfluous in-tag whitespace should be ignored.
//   https://github.com/mustache/spec/blob/v1.4.2/specs/inverted.yml#L195-L199
TEST_F(InvertedTest, Padding) {
  EXPECT_EQ(
      "|=|",
      *render(
          "|{{^ boolean }}={{/ boolean }}|",
          w::map({{"boolean", w::boolean(false)}})));
}

} // namespace whisker
