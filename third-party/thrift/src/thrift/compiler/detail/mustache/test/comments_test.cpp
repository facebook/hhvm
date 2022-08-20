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
// Comment blocks should be removed from the template.
TEST(CommentsTEST, Inline) {
  EXPECT_EQ(
      "1234567890",
      mstch::render("12345{{! Comment Block! }}67890", mstch::node()));
}
// Multiline comments should be permitted.
TEST(CommentsTEST, Multiline) {
  EXPECT_EQ(
      "1234567890\n",
      mstch::render(
          "12345{{!\n  This is a\n  multi-line comment...\n}}67890\n",
          mstch::node()));
}
// All standalone comment lines should be removed.
TEST(CommentsTEST, Standalone) {
  EXPECT_EQ(
      "Begin.\nEnd.\n",
      mstch::render("Begin.\n{{! Comment Block! }}\nEnd.\n", mstch::node()));
}
// All standalone comment lines should be removed.
TEST(CommentsTEST, IndentedStandalone) {
  EXPECT_EQ(
      "Begin.\nEnd.\n",
      mstch::render(
          "Begin.\n  {{! Indented Comment Block! }}\nEnd.\n", mstch::node()));
}
// "\r\n" should be considered a newline for standalone tags.
TEST(CommentsTEST, StandaloneLineEndings) {
  EXPECT_EQ(
      "|\r\n|",
      mstch::render("|\r\n{{! Standalone Comment }}\r\n|", mstch::node()));
}
// Standalone tags should not require a newline to precede them.
TEST(CommentsTEST, StandaloneWithoutPreviousLine) {
  EXPECT_EQ(
      "!", mstch::render("  {{! I'm Still Standalone }}\n!", mstch::node()));
}
// Standalone tags should not require a newline to follow them.
TEST(CommentsTEST, StandaloneWithoutNewline) {
  EXPECT_EQ(
      "!\n", mstch::render("!\n  {{! I'm Still Standalone }}", mstch::node()));
}
// All standalone comment lines should be removed.
TEST(CommentsTEST, MultilineStandalone) {
  EXPECT_EQ(
      "Begin.\nEnd.\n",
      mstch::render(
          "Begin.\n{{!\nSomething's going on here...\n}}\nEnd.\n",
          mstch::node()));
}
// All standalone comment lines should be removed.
TEST(CommentsTEST, IndentedMultilineStandalone) {
  EXPECT_EQ(
      "Begin.\nEnd.\n",
      mstch::render(
          "Begin.\n  {{!\n    Something's going on here...\n  }}\nEnd.\n",
          mstch::node()));
}
// Inline comments should not strip whitespace
TEST(CommentsTEST, IndentedInline) {
  EXPECT_EQ("  12 \n", mstch::render("  12 {{! 34 }}\n", mstch::node()));
}
// Comment removal should preserve surrounding whitespace.
TEST(CommentsTEST, SurroundingWhitespace) {
  EXPECT_EQ(
      "12345  67890",
      mstch::render("12345 {{! Comment Block! }} 67890", mstch::node()));
}
