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

#include <thrift/compiler/whisker/diagnostic.h>
#include <thrift/compiler/whisker/object.h>
#include <thrift/compiler/whisker/standard_library.h>
#include <thrift/compiler/whisker/test/render_test_helpers.h>

namespace w = whisker::make;

namespace whisker {

namespace {
class StandardLibraryTest : public RenderTest {
 protected:
  void SetUp() override {
    RenderTest::SetUp();
    use_library(load_standard_library);
  }
};
} // namespace

TEST_F(StandardLibraryTest, array_of) {
  {
    auto result = render(
        "The factorial function looks like:\n"
        "{{#let factorials = (array.of 1 2 6 24 120)}}\n"
        "{{#factorials}}\n"
        "{{.}}\n"
        "{{/factorials}}\n",
        w::null);
    EXPECT_THAT(diagnostics(), testing::IsEmpty());
    EXPECT_EQ(
        *result,
        "The factorial function looks like:\n"
        "1\n"
        "2\n"
        "6\n"
        "24\n"
        "120\n");
  }

  {
    strict_boolean_conditional(diagnostic_level::info);
    auto result = render(
        "{{#let empty = (array.of)}}\n"
        "{{^empty}}\n"
        "It's empty!\n"
        "{{/empty}}\n",
        w::null);
    EXPECT_THAT(diagnostics(), testing::IsEmpty());
    EXPECT_EQ(*result, "It's empty!\n");
  }
}

TEST_F(StandardLibraryTest, array_len) {
  {
    auto result = render(
        "{{ (array.len (array.of)) }}\n"
        "{{ (array.len (array.of 1 \"foo\" true)) }}\n"
        "{{ (array.len (array.of 1 2 6 24 120)) }}\n",
        w::null);
    EXPECT_THAT(diagnostics(), testing::IsEmpty());
    EXPECT_EQ(
        *result,
        "0\n"
        "3\n"
        "5\n");
  }
}

TEST_F(StandardLibraryTest, array_empty) {
  {
    strict_printable_types(diagnostic_level::info);
    auto result = render(
        "{{ (array.empty? (array.of)) }}\n"
        "{{ (array.empty? (array.of 1 \"foo\" true)) }}\n"
        "{{ (array.empty? (array.of 1 2 6 24 120)) }}\n",
        w::null);
    EXPECT_THAT(diagnostics(), testing::IsEmpty());
    EXPECT_EQ(
        *result,
        "true\n"
        "false\n"
        "false\n");
  }
}

TEST_F(StandardLibraryTest, string_len) {
  {
    auto result = render(
        "{{ (string.len \"\") }}\n"
        "{{ (string.len \"hello\") }}\n"
        "{{ (string.len \"\\t\") }}\n",
        w::null);
    EXPECT_THAT(diagnostics(), testing::IsEmpty());
    EXPECT_EQ(
        *result,
        "0\n"
        "5\n"
        "1\n");
  }
}

} // namespace whisker
