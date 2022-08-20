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
// The greater-than operator should expand to the named partial.
TEST(PartialsTEST, BasicBehavior) {
  EXPECT_EQ(
      "\"from partial\"",
      mstch::render(
          "\"{{>text}}\"", mstch::node(), {{"text", "from partial"}}));
}
// The empty string should be used when the named partial is not found.
TEST(PartialsTEST, FailedLookup) {
  EXPECT_EQ("\"\"", mstch::render("\"{{>text}}\"", mstch::node()));
}
// The greater-than operator should operate within the current context.
TEST(PartialsTEST, Context) {
  EXPECT_EQ(
      "\"*content*\"",
      mstch::render(
          "\"{{>partial}}\"",
          mstch::map{{"text", std::string("content")}},
          {{"partial", "*{{text}}*"}}));
}
// The greater-than operator should properly recurse.
TEST(PartialsTEST, Recursion) {
  EXPECT_EQ(
      "X<Y<>>",
      mstch::render(
          "{{>node}}",
          mstch::map{
              {"content", std::string("X")},
              {"nodes",
               mstch::array{mstch::map{
                   {"content", std::string("Y")}, {"nodes", mstch::array{}}}}}},
          {{"node", "{{content}}<{{#nodes}}{{>node}}{{/nodes}}>"}}));
}
// The greater-than operator should not alter surrounding whitespace.
TEST(PartialsTEST, SurroundingWhitespace) {
  EXPECT_EQ(
      "| 	|	 |",
      mstch::render("| {{>partial}} |", mstch::node(), {{"partial", "\t|\t"}}));
}
// Whitespace should be left untouched.
TEST(PartialsTEST, InlineIndentation) {
  EXPECT_EQ(
      "  |  >\n>\n",
      mstch::render(
          "  {{data}}  {{> partial}}\n",
          mstch::map{{"data", std::string("|")}},
          {{"partial", ">\n>"}}));
}
// "\r\n" should be considered a newline for standalone tags.
TEST(PartialsTEST, StandaloneLineEndings) {
  EXPECT_EQ(
      "|\r\n>|",
      mstch::render(
          "|\r\n{{>partial}}\r\n|", mstch::node(), {{"partial", ">"}}));
}
// Standalone tags should not require a newline to precede them.
TEST(PartialsTEST, StandaloneWithoutPreviousLine) {
  EXPECT_EQ(
      "  >\n  >>",
      mstch::render("  {{>partial}}\n>", mstch::node(), {{"partial", ">\n>"}}));
}
// Standalone tags should not require a newline to follow them.
TEST(PartialsTEST, StandaloneWithoutNewline) {
  EXPECT_EQ(
      ">\n  >\n  >",
      mstch::render(">\n  {{>partial}}", mstch::node(), {{"partial", ">\n>"}}));
}
// Each line of the partial should be indented before rendering.
TEST(PartialsTEST, StandaloneIndentation) {
  EXPECT_EQ(
      "\\\n |\n <\n->\n |\n/\n",
      mstch::render(
          "\\\n {{>partial}}\n/\n",
          mstch::map{{"content", std::string("<\n->")}},
          {{"partial", "|\n{{content}}\n|\n"}}));
}
// Superfluous in-tag whitespace should be ignored.
TEST(PartialsTEST, PaddingWhitespace) {
  EXPECT_EQ(
      "|[]|",
      mstch::render(
          "|{{> partial }}|",
          mstch::map{{"boolean", true}},
          {{"partial", "[]"}}));
}
