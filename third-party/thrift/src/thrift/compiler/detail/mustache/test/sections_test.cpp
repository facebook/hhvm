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
// Truthy sections should have their contents rendered.
TEST(SectionsTEST, Truthy) {
  EXPECT_EQ(
      "\"This should be rendered.\"",
      mstch::render(
          "\"{{#boolean}}This should be rendered.{{/boolean}}\"",
          mstch::map{{"boolean", true}}));
}
// Falsey sections should have their contents omitted.
TEST(SectionsTEST, Falsey) {
  EXPECT_EQ(
      "\"\"",
      mstch::render(
          "\"{{#boolean}}This should not be rendered.{{/boolean}}\"",
          mstch::map{{"boolean", false}}));
}
// Objects and hashes should be pushed onto the context stack.
TEST(SectionsTEST, Context) {
  EXPECT_EQ(
      "\"Hi Joe.\"",
      mstch::render(
          "\"{{#context}}Hi {{name}}.{{/context}}\"",
          mstch::map{{"context", mstch::map{{"name", std::string("Joe")}}}}));
}
// All elements on the context stack should be accessible.
TEST(SectionsTEST, DeeplyNestedContexts) {
  EXPECT_EQ(
      "1\n121\n12321\n1234321\n123454321\n1234321\n12321\n121\n1\n",
      mstch::render(
          "{{#a}}\n{{one}}\n{{#b}}\n{{one}}{{two}}{{one}}\n{{#c}}\n{{one}}{{two}}{{three}}{{two}}{{one}}\n{{#d}}\n{{one}}{{two}}{{three}}{{four}}{{three}}{{two}}{{one}}\n{{#e}}\n{{one}}{{two}}{{three}}{{four}}{{five}}{{four}}{{three}}{{two}}{{one}}\n{{/e}}\n{{one}}{{two}}{{three}}{{four}}{{three}}{{two}}{{one}}\n{{/d}}\n{{one}}{{two}}{{three}}{{two}}{{one}}\n{{/c}}\n{{one}}{{two}}{{one}}\n{{/b}}\n{{one}}\n{{/a}}\n",
          mstch::map{
              {"a", mstch::map{{"one", 1}}},
              {"b", mstch::map{{"two", 2}}},
              {"c", mstch::map{{"three", 3}}},
              {"d", mstch::map{{"four", 4}}},
              {"e", mstch::map{{"five", 5}}}}));
}
// Lists should be iterated; list items should visit the context stack.
TEST(SectionsTEST, List) {
  EXPECT_EQ(
      "\"123\"",
      mstch::render(
          "\"{{#list}}{{item}}{{/list}}\"",
          mstch::map{
              {"list",
               mstch::array{
                   mstch::map{{"item", 1}},
                   mstch::map{{"item", 2}},
                   mstch::map{{"item", 3}}}}}));
}
// Empty lists should behave like falsey values.
TEST(SectionsTEST, EmptyList) {
  EXPECT_EQ(
      "\"\"",
      mstch::render(
          "\"{{#list}}Yay lists!{{/list}}\"",
          mstch::map{{"list", mstch::array{}}}));
}
// Multiple sections per template should be permitted.
TEST(SectionsTEST, Doubled) {
  EXPECT_EQ(
      "* first\n* second\n* third\n",
      mstch::render(
          "{{#bool}}\n* first\n{{/bool}}\n* {{two}}\n{{#bool}}\n* third\n{{/bool}}\n",
          mstch::map{{"two", std::string("second")}, {"bool", true}}));
}
// Nested truthy sections should have their contents rendered.
TEST(SectionsTEST, NestedTruthy) {
  EXPECT_EQ(
      "| A B C D E |",
      mstch::render(
          "| A {{#bool}}B {{#bool}}C{{/bool}} D{{/bool}} E |",
          mstch::map{{"bool", true}}));
}
// Nested falsey sections should be omitted.
TEST(SectionsTEST, NestedFalsey) {
  EXPECT_EQ(
      "| A  E |",
      mstch::render(
          "| A {{#bool}}B {{#bool}}C{{/bool}} D{{/bool}} E |",
          mstch::map{{"bool", false}}));
}
// Failed context lookups should be considered falsey.
TEST(SectionsTEST, ContextMisses) {
  EXPECT_EQ(
      "[]",
      mstch::render(
          "[{{#missing}}Found key 'missing'!{{/missing}}]", mstch::node{}));
}
// Implicit iterators should directly interpolate strings.
TEST(SectionsTEST, ImplicitIteratorString) {
  EXPECT_EQ(
      "\"(a)(b)(c)(d)(e)\"",
      mstch::render(
          "\"{{#list}}({{.}}){{/list}}\"",
          mstch::map{
              {"list",
               mstch::array{
                   std::string("a"),
                   std::string("b"),
                   std::string("c"),
                   std::string("d"),
                   std::string("e")}}}));
}
// Implicit iterators should cast integers to strings and interpolate.
TEST(SectionsTEST, ImplicitIteratorInteger) {
  EXPECT_EQ(
      "\"(1)(2)(3)(4)(5)\"",
      mstch::render(
          "\"{{#list}}({{.}}){{/list}}\"",
          mstch::map{
              {"list", mstch::array{1, 2, 3, 4, 5}},
          }));
}
// Implicit iterators should cast decimals to strings and interpolate.
TEST(SectionsTEST, ImplicitIteratorDecimal) {
  EXPECT_EQ(
      "\"(1.1)(2.2)(3.3)(4.4)(5.5)\"",
      mstch::render(
          "\"{{#list}}({{.}}){{/list}}\"",
          mstch::map{
              {"list", mstch::array{1.1, 2.2, 3.3, 4.4, 5.5}},
          }));
}
// Implicit iterators should allow iterating over nested arrays.
TEST(SectionsTEST, ImplicitIteratorArray) {
  EXPECT_EQ(
      "\"(123)(abc)\"",
      mstch::render(
          "\"{{#list}}({{#.}}{{.}}{{/.}}){{/list}}\"",
          mstch::map{
              {"list",
               mstch::array{
                   mstch::array{1, 2, 3},
                   mstch::array{
                       std::string("a"), std::string("b"), std::string("c")}}},
          }));
}
// Dotted names should be valid for Section tags.
TEST(SectionsTEST, DottedNamesTruthy) {
  EXPECT_EQ(
      "\"Here\" == \"Here\"",
      mstch::render(
          "\"{{#a.b.c}}Here{{/a.b.c}}\" == \"Here\"",
          mstch::map{{"a", mstch::map{{"b", mstch::map{{"c", true}}}}}}));
}
// Dotted names should be valid for Section tags.
TEST(SectionsTEST, DottedNamesFalsey) {
  EXPECT_EQ(
      "\"\" == \"\"",
      mstch::render(
          "\"{{#a.b.c}}Here{{/a.b.c}}\" == \"\"",
          mstch::map{{"a", mstch::map{{"b", mstch::map{{"c", false}}}}}}));
}
// Dotted names that cannot be resolved should be considered falsey.
TEST(SectionsTEST, DottedNamesBrokenChains) {
  EXPECT_EQ(
      "\"\" == \"\"",
      mstch::render(
          "\"{{#a.b.c}}Here{{/a.b.c}}\" == \"\"",
          mstch::map{{"a", mstch::node()}}));
}
// Sections should not alter surrounding whitespace.
TEST(SectionsTEST, SurroundingWhitespace) {
  EXPECT_EQ(
      " | 	|	 | \\n",
      mstch::render(
          " | {{#boolean}}	|	{{/boolean}} | \\n",
          mstch::map{{"boolean", true}}));
}
// Sections should not alter internal whitespace.
TEST(SectionsTEST, InternalWhitespace) {
  EXPECT_EQ(
      " |  \n  | \n",
      mstch::render(
          " | {{#boolean}} {{! Important Whitespace }}\n {{/boolean}} | \n",
          mstch::map{{"boolean", true}}));
}
// Single-line sections should not alter surrounding whitespace.
TEST(SectionsTEST, IndentedInlineSections) {
  EXPECT_EQ(
      " YES\n GOOD\n",
      mstch::render(
          " {{#boolean}}YES{{/boolean}}\n {{#boolean}}GOOD{{/boolean}}\n",
          mstch::map{{"boolean", true}}));
}
// Standalone lines should be removed from the template.
TEST(SectionsTEST, StandaloneLines) {
  EXPECT_EQ(
      "| This Is\n|\n| A Line\n",
      mstch::render(
          "| This Is\n{{#boolean}}\n|\n{{/boolean}}\n| A Line\n",
          mstch::map{{"boolean", true}}));
}
// Indented standalone lines should be removed from the template.
TEST(SectionsTEST, IndentedStandaloneLines) {
  EXPECT_EQ(
      "| This Is\n|\n| A Line\n",
      mstch::render(
          "| This Is\n  {{#boolean}}\n|\n  {{/boolean}}\n| A Line\n",
          mstch::map{{"boolean", true}}));
}
// "\r\n" should be considered a newline for standalone tags.
TEST(SectionsTEST, StandaloneLineEndings) {
  EXPECT_EQ(
      "|\r\n|",
      mstch::render(
          "|\r\n{{#boolean}}\r\n{{/boolean}}\r\n|",
          mstch::map{{"boolean", true}}));
}
// Standalone tags should not require a newline to precede them.
TEST(SectionsTEST, StandaloneWithoutPreviousLine) {
  EXPECT_EQ(
      "#\n/",
      mstch::render(
          "  {{#boolean}}\n#{{/boolean}}\n/", mstch::map{{"boolean", true}}));
}
// Standalone tags should not require a newline to follow them.
TEST(SectionsTEST, StandaloneWithoutNewline) {
  EXPECT_EQ(
      "#\n/\n",
      mstch::render(
          "#{{#boolean}}\n/\n  {{/boolean}}", mstch::map{{"boolean", true}}));
}
// Superfluous in-tag whitespace should be ignored.
TEST(SectionsTEST, Padding) {
  EXPECT_EQ(
      "|=|",
      mstch::render(
          "|{{# boolean }}={{/ boolean }}|", mstch::map{{"boolean", true}}));
}
