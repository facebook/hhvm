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
// The equals sign (used on both sides) should permit delimiter changes.
TEST(DelimitersTEST, PairBehavior) {
  EXPECT_EQ(
      "(Hey!)",
      mstch::render(
          R"template({{=<% %>=}}(<%text%>))template",
          mstch::map{{"text", std::string("Hey!")}}));
}
// Characters with special meaning regexen should be valid delimiters.
TEST(DelimitersTEST, SpecialCharacters) {
  EXPECT_EQ(
      "(It worked!)",
      mstch::render(
          "({{=[ ]=}}[text])",
          mstch::map{{"text", std::string("It worked!")}}));
}
// Delimiters set outside sections should persist.
TEST(DelimitersTEST, Sections) {
  EXPECT_EQ(
      "[\n  I got interpolated.\n  |data|\n\n  {{data}}\n  I got interpolated.\n]\n",
      mstch::render(
          "[\n{{#section}}\n  {{data}}\n  |data|\n{{/section}}\n\n{{= | | =}}\n|#section|\n  {{data}}\n  |data|\n|/section|\n]\n",
          mstch::map{
              {"section", true},
              {"data", std::string("I got interpolated.")}}));
}
// Delimiters set outside inverted sections should persist.
TEST(DelimitersTEST, InvertedSections) {
  EXPECT_EQ(
      "[\n  I got interpolated.\n  |data|\n\n  {{data}}\n  I got interpolated.\n]\n",
      mstch::render(
          "[\n{{^section}}\n  {{data}}\n  |data|\n{{/section}}\n\n{{= | | =}}\n|^section|\n  {{data}}\n  |data|\n|/section|\n]\n",
          mstch::map{
              {"section", false},
              {"data", std::string("I got interpolated.")}}));
}
// Delimiters set in a parent template should not affect a partial.
TEST(DelimitersTEST, PartialInheritence) {
  EXPECT_EQ(
      "[ .yes. ]\n[ .yes. ]\n",
      mstch::render(
          "[ {{>include}} ]\n{{= | | =}}\n[ |>include| ]\n",
          mstch::map{{"value", std::string("yes")}},
          {{"include", ".{{value}}."}}));
}
// Delimiters set in a partial should not affect the parent template.
TEST(DelimitersTEST, PostPartialBehavior) {
  EXPECT_EQ(
      "[ .yes.  .yes. ]\n[ .yes.  .|value|. ]\n",
      mstch::render(
          "[ {{>include}} ]\n[ .{{value}}.  .|value|. ]\n",
          mstch::map{{"value", std::string("yes")}},
          {{"include", ".{{value}}. {{= | | =}} .|value|."}}));
}
// Surrounding whitespace should be left untouched.
TEST(DelimitersTEST, SurroundingWhitespace) {
  EXPECT_EQ("|  |", mstch::render("| {{=@ @=}} |", mstch::node()));
}
// Whitespace should be left untouched.
TEST(DelimitersTEST, OutlyingWhitespaceInline) {
  EXPECT_EQ(" | \n", mstch::render(" | {{=@ @=}}\n", mstch::node()));
}
// Standalone lines should be removed from the template.
TEST(DelimitersTEST, StandaloneTag) {
  EXPECT_EQ(
      "Begin.\nEnd.\n",
      mstch::render("Begin.\n{{=@ @=}}\nEnd.\n", mstch::node()));
}
// Indented standalone lines should be removed from the template.
TEST(DelimitersTEST, IndentedStandaloneTag) {
  EXPECT_EQ(
      "Begin.\nEnd.\n",
      mstch::render("Begin.\n  {{=@ @=}}\nEnd.\n", mstch::node()));
}
// "\r\n" should be considered a newline for standalone tags.
TEST(DelimitersTEST, StandaloneLineEndings) {
  EXPECT_EQ("|\r\n|", mstch::render("|\r\n{{= @ @ =}}\r\n|", mstch::node()));
}
// Standalone tags should not require a newline to precede them.
TEST(DelimitersTEST, StandaloneWithoutPreviousLine) {
  EXPECT_EQ("=", mstch::render("  {{=@ @=}}\n=", mstch::node()));
}
// Standalone tags should not require a newline to follow them.
TEST(DelimitersTEST, StandaloneWithoutNewline) {
  EXPECT_EQ("=\n", mstch::render("=\n  {{=@ @=}}", mstch::node()));
}
// Superfluous in-tag whitespace should be ignored.
TEST(DelimitersTEST, PairwithPadding) {
  EXPECT_EQ("||", mstch::render("|{{= @   @ =}}|", mstch::node()));
}
