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

#include <gtest/gtest.h>

#include <thrift/compiler/whisker/object.h>
#include <thrift/compiler/whisker/test/render_test_helpers.h>

namespace w = whisker::make;

namespace whisker {

class CommentsTest : public RenderTest {};

// Comment blocks should be removed from the template.
//   https://github.com/mustache/spec/blob/v1.4.2/specs/comments.yml#L10-L14
TEST_F(CommentsTest, Inline) {
  EXPECT_EQ(
      "1234567890", *render("12345{{! Comment Block! }}67890", w::map({})));
}

// Multiline comments should be permitted.
//   https://github.com/mustache/spec/blob/v1.4.2/specs/comments.yml#L16-L25
TEST_F(CommentsTest, Multiline) {
  EXPECT_EQ(
      "1234567890\n",
      *render(
          "12345{{!\n  This is a\n  multi-line comment...\n}}67890\n",
          w::map({})));
}

// All standalone comment lines should be removed.
//   https://github.com/mustache/spec/blob/v1.4.2/specs/comments.yml#L27-L36
TEST_F(CommentsTest, Standalone) {
  EXPECT_EQ(
      "Begin.\n"
      "End.\n",
      *render(
          "Begin.\n"
          "{{! Comment Block! }}\n"
          "End.\n",
          w::map({})));
}

// All standalone comment lines should be removed.
//   https://github.com/mustache/spec/blob/v1.4.2/specs/comments.yml#L38-L47
TEST_F(CommentsTest, IndentedStandalone) {
  EXPECT_EQ(
      "Begin.\n"
      "End.\n",
      *render(
          "Begin.\n"
          "  {{! Indented Comment Block! }}\n"
          "End.\n",
          w::map({})));
}

// "\r\n" should be considered a newline for standalone tags.
//   https://github.com/mustache/spec/blob/v1.4.2/specs/comments.yml#L49-L53
TEST_F(CommentsTest, StandaloneLineEndings) {
  EXPECT_EQ(
      "|\r\n|", *render("|\r\n{{! Standalone Comment }}\r\n|", w::map({})));
}

// Standalone tags should not require a newline to precede them.
//   https://github.com/mustache/spec/blob/v1.4.2/specs/comments.yml#L55-L59
TEST_F(CommentsTest, StandaloneWithoutPreviousLine) {
  EXPECT_EQ("!", *render("  {{! I'm Still Standalone }}\n!", w::map({})));
}

// Standalone tags should not require a newline to follow them.
//   https://github.com/mustache/spec/blob/v1.4.2/specs/comments.yml#L61-L65
TEST_F(CommentsTest, StandaloneWithoutNewline) {
  EXPECT_EQ("!\n", *render("!\n  {{! I'm Still Standalone }}", w::map({})));
}

// All standalone comment lines should be removed.
//   https://github.com/mustache/spec/blob/v1.4.2/specs/comments.yml#L67-L78
TEST_F(CommentsTest, MultilineStandalone) {
  EXPECT_EQ(
      "Begin.\n"
      "End.\n",
      *render(
          "Begin.\n"
          "{{!\n"
          "Something's going on here...\n"
          "}}\n"
          "End.\n",
          w::map({})));
}

// All standalone comment lines should be removed.
//   https://github.com/mustache/spec/blob/v1.4.2/specs/comments.yml#L80-L91
TEST_F(CommentsTest, IndentedMultilineStandalone) {
  EXPECT_EQ(
      "Begin.\n"
      "End.\n",
      *render(
          "Begin.\n"
          "  {{!\n"
          "    Something's going on here...\n"
          "  }}\n"
          "End.\n",
          w::map({})));
}

// Inline comments should not strip whitespace
//   https://github.com/mustache/spec/blob/v1.4.2/specs/comments.yml#L93-L97
TEST_F(CommentsTest, IndentedInline) {
  EXPECT_EQ("  12 \n", *render("  12 {{! 34 }}\n", w::map({})));
}

// Comment removal should preserve surrounding whitespace.
//   https://github.com/mustache/spec/blob/v1.4.2/specs/comments.yml#L99-L103
TEST_F(CommentsTest, SurroundingWhitespace) {
  EXPECT_EQ(
      "12345  67890", *render("12345 {{! Comment Block! }} 67890", w::map({})));
}

// Comments must never render, even if variable with same name exists.
//   https://github.com/mustache/spec/blob/v1.4.2/specs/comments.yml#L105-L109
TEST_F(CommentsTest, VariableNameCollision) {
  EXPECT_EQ(
      "comments never show: ><",
      *render(
          "comments never show: >{{! comment}}<",
          w::map(
              {{"! comment", w::i64(1)},
               {"! comment ", w::i64(2)},
               {"!comment", w::i64(3)},
               {"comment", w::i64(4)}})));
}

} // namespace whisker
