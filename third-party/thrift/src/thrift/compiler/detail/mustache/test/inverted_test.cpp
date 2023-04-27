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

#include <thrift/compiler/detail/mustache/mstch.h>

using namespace apache::thrift;
// Falsey sections should have their contents rendered.
TEST(InvertedTEST, Falsey) {
  EXPECT_EQ(
      "\"This should be rendered.\"",
      mstch::render(
          "\"{{^boolean}}This should be rendered.{{/boolean}}\"",
          mstch::map{{"boolean", false}}));
}
// Truthy sections should have their contents omitted.
TEST(InvertedTEST, Truthy) {
  EXPECT_EQ(
      "\"\"",
      mstch::render(
          "\"{{^boolean}}This should not be rendered.{{/boolean}}\"",
          mstch::map{{"boolean", true}}));
}
// Objects and hashes should behave like truthy values.
TEST(InvertedTEST, Context) {
  EXPECT_EQ(
      "\"\"",
      mstch::render(
          "\"{{^context}}Hi {{name}}.{{/context}}\"",
          mstch::map{{"context", mstch::map{{"name", std::string("Joe")}}}}));
}
// Lists should behave like truthy values.
TEST(InvertedTEST, List) {
  EXPECT_EQ(
      "\"\"",
      mstch::render(
          "\"{{^list}}{{n}}{{/list}}\"",
          mstch::map{
              {"list",
               mstch::array{
                   mstch::map{{"n", 1}},
                   mstch::map{{"n", 2}},
                   mstch::map{{"n", 3}}

               }}}));
}
// Empty lists should behave like falsey values.
TEST(InvertedTEST, EmptyList) {
  EXPECT_EQ(
      "\"Yay lists!\"",
      mstch::render(
          "\"{{^list}}Yay lists!{{/list}}\"",
          mstch::map{{"list", mstch::array{}}}));
}
// Multiple inverted sections per template should be permitted.
TEST(InvertedTEST, Doubled) {
  EXPECT_EQ(
      "* first\n* second\n* third\n",
      mstch::render(
          "{{^bool}}\n* first\n{{/bool}}\n* {{two}}\n{{^bool}}\n* third\n{{/bool}}\n",
          mstch::map{{"two", std::string("second")}, {"bool", false}}));
}
// Nested falsey sections should have their contents rendered.
TEST(InvertedTEST, NestedFalsey) {
  EXPECT_EQ(
      "| A B C D E |",
      mstch::render(
          "| A {{^bool}}B {{^bool}}C{{/bool}} D{{/bool}} E |",
          mstch::map{{"bool", false}}));
}
// Nested truthy sections should be omitted.
TEST(InvertedTEST, NestedTruthy) {
  EXPECT_EQ(
      "| A  E |",
      mstch::render(
          "| A {{^bool}}B {{^bool}}C{{/bool}} D{{/bool}} E |",
          mstch::map{{"bool", true}}));
}
// Failed context lookups should be considered falsey.
TEST(InvertedTEST, ContextMisses) {
  EXPECT_EQ(
      "[Cannot find key 'missing'!]",
      mstch::render(
          "[{{^missing}}Cannot find key 'missing'!{{/missing}}]",
          mstch::node()));
}
// Dotted names should be valid for Inverted Section tags.
TEST(InvertedTEST, DottedNamesTruthy) {
  EXPECT_EQ(
      "\"\" == \"\"",
      mstch::render(
          "\"{{^a.b.c}}Not Here{{/a.b.c}}\" == \"\"",
          mstch::map{{"a", mstch::map{{"b", mstch::map{{"c", true}}}}}}));
}
// Dotted names should be valid for Inverted Section tags.
TEST(InvertedTEST, DottedNamesFalsey) {
  EXPECT_EQ(
      "\"Not Here\" == \"Not Here\"",
      mstch::render(
          "\"{{^a.b.c}}Not Here{{/a.b.c}}\" == \"Not Here\"",
          mstch::map{{"a", mstch::map{{"b", mstch::map{{"c", false}}}}}}));
}
// Dotted names that cannot be resolved should be considered falsey.
TEST(InvertedTEST, DottedNamesBrokenChains) {
  EXPECT_EQ(
      "\"Not Here\" == \"Not Here\"",
      mstch::render(
          "\"{{^a.b.c}}Not Here{{/a.b.c}}\" == \"Not Here\"",
          mstch::map{{"a", mstch::node()}}));
}
// Inverted sections should not alter surrounding whitespace.
TEST(InvertedTEST, SurroundingWhitespace) {
  EXPECT_EQ(
      " | 	|	 | \n",
      mstch::render(
          " | {{^boolean}}	|	{{/boolean}} | \n",
          mstch::map{{"boolean", false}}));
}
// Inverted should not alter internal whitespace.
TEST(InvertedTEST, InternalWhitespace) {
  EXPECT_EQ(
      " |  \n  | \n",
      mstch::render(
          " | {{^boolean}} {{! Important Whitespace }}\n {{/boolean}} | \n",
          mstch::map{{"boolean", false}}));
}
// Single-line sections should not alter surrounding whitespace.
TEST(InvertedTEST, IndentedInlineSections) {
  EXPECT_EQ(
      " NO\n WAY\n",
      mstch::render(
          " {{^boolean}}NO{{/boolean}}\n {{^boolean}}WAY{{/boolean}}\n",
          mstch::map{{"boolean", false}}));
}
// Standalone lines should be removed from the template.
TEST(InvertedTEST, StandaloneLines) {
  EXPECT_EQ(
      "| This Is\n|\n| A Line\n",
      mstch::render(
          "| This Is\n{{^boolean}}\n|\n{{/boolean}}\n| A Line\n",
          mstch::map{{"boolean", false}}));
}
// Standalone indented lines should be removed from the template.
TEST(InvertedTEST, StandaloneIndentedLines) {
  EXPECT_EQ(
      "| This Is\n|\n| A Line\n",
      mstch::render(
          "| This Is\n  {{^boolean}}\n|\n  {{/boolean}}\n| A Line\n",
          mstch::map{{"boolean", false}}));
}
// "\r\n" should be considered a newline for standalone tags.
TEST(InvertedTEST, StandaloneLineEndings) {
  EXPECT_EQ(
      "|\r\n|",
      mstch::render(
          "|\r\n{{^boolean}}\r\n{{/boolean}}\r\n|",
          mstch::map{{"boolean", false}}));
}
// Standalone tags should not require a newline to precede them.
TEST(InvertedTEST, StandaloneWithoutPreviousLine) {
  EXPECT_EQ(
      "^\n/",
      mstch::render(
          "  {{^boolean}}\n^{{/boolean}}\n/", mstch::map{{"boolean", false}}));
}
// Standalone tags should not require a newline to follow them.
TEST(InvertedTEST, StandaloneWithoutNewline) {
  EXPECT_EQ(
      "^\n/\n",
      mstch::render(
          "^{{^boolean}}\n/\n  {{/boolean}}", mstch::map{{"boolean", false}}));
}
// Superfluous in-tag whitespace should be ignored.
TEST(InvertedTEST, Padding) {
  EXPECT_EQ(
      "|=|",
      mstch::render(
          "|{{^ boolean }}={{/ boolean }}|", mstch::map{{"boolean", false}}));
}
