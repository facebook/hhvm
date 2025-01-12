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

TEST_F(StandardLibraryTest, i64_comparisons) {
  strict_printable_types(diagnostic_level::info);

  {
    auto result = render(
        "positives:\n"
        "- {{ (int.lt? 1 2) }}\n"
        "- {{ (int.lt? 2 1) }}\n"
        "- {{ (int.lt? 1 1) }}\n"
        "negatives:\n"
        "- {{ (int.lt? -1 -2) }}\n"
        "- {{ (int.lt? -2 -1) }}\n"
        "- {{ (int.lt? -1 -1) }}\n",
        w::null);
    EXPECT_THAT(diagnostics(), testing::IsEmpty());
    EXPECT_EQ(
        *result,
        "positives:\n"
        "- true\n"
        "- false\n"
        "- false\n"
        "negatives:\n"
        "- false\n"
        "- true\n"
        "- false\n");
  }

  {
    auto result = render(
        "positives:\n"
        "- {{ (int.le? 1 2) }}\n"
        "- {{ (int.le? 2 1) }}\n"
        "- {{ (int.le? 1 1) }}\n"
        "negatives:\n"
        "- {{ (int.le? -1 -2) }}\n"
        "- {{ (int.le? -2 -1) }}\n"
        "- {{ (int.le? -1 -1) }}\n",
        w::null);
    EXPECT_THAT(diagnostics(), testing::IsEmpty());
    EXPECT_EQ(
        *result,
        "positives:\n"
        "- true\n"
        "- false\n"
        "- true\n"
        "negatives:\n"
        "- false\n"
        "- true\n"
        "- true\n");
  }

  {
    auto result = render(
        "positives:\n"
        "- {{ (int.eq? 1 2) }}\n"
        "- {{ (int.eq? 2 1) }}\n"
        "- {{ (int.eq? 1 1) }}\n"
        "negatives:\n"
        "- {{ (int.eq? -1 -2) }}\n"
        "- {{ (int.eq? -2 -1) }}\n"
        "- {{ (int.eq? -1 -1) }}\n",
        w::null);
    EXPECT_THAT(diagnostics(), testing::IsEmpty());
    EXPECT_EQ(
        *result,
        "positives:\n"
        "- false\n"
        "- false\n"
        "- true\n"
        "negatives:\n"
        "- false\n"
        "- false\n"
        "- true\n");
  }

  {
    auto result = render(
        "positives:\n"
        "- {{ (int.ne? 1 2) }}\n"
        "- {{ (int.ne? 2 1) }}\n"
        "- {{ (int.ne? 1 1) }}\n"
        "negatives:\n"
        "- {{ (int.ne? -1 -2) }}\n"
        "- {{ (int.ne? -2 -1) }}\n"
        "- {{ (int.ne? -1 -1) }}\n",
        w::null);
    EXPECT_THAT(diagnostics(), testing::IsEmpty());
    EXPECT_EQ(
        *result,
        "positives:\n"
        "- true\n"
        "- true\n"
        "- false\n"
        "negatives:\n"
        "- true\n"
        "- true\n"
        "- false\n");
  }

  {
    auto result = render(
        "positives:\n"
        "- {{ (int.ge? 1 2) }}\n"
        "- {{ (int.ge? 2 1) }}\n"
        "- {{ (int.ge? 1 1) }}\n"
        "negatives:\n"
        "- {{ (int.ge? -1 -2) }}\n"
        "- {{ (int.ge? -2 -1) }}\n"
        "- {{ (int.ge? -1 -1) }}\n",
        w::null);
    EXPECT_THAT(diagnostics(), testing::IsEmpty());
    EXPECT_EQ(
        *result,
        "positives:\n"
        "- false\n"
        "- true\n"
        "- true\n"
        "negatives:\n"
        "- true\n"
        "- false\n"
        "- true\n");
  }

  {
    auto result = render(
        "positives:\n"
        "- {{ (int.gt? 1 2) }}\n"
        "- {{ (int.gt? 2 1) }}\n"
        "- {{ (int.gt? 1 1) }}\n"
        "negatives:\n"
        "- {{ (int.gt? -1 -2) }}\n"
        "- {{ (int.gt? -2 -1) }}\n"
        "- {{ (int.gt? -1 -1) }}\n",
        w::null);
    EXPECT_THAT(diagnostics(), testing::IsEmpty());
    EXPECT_EQ(
        *result,
        "positives:\n"
        "- false\n"
        "- true\n"
        "- false\n"
        "negatives:\n"
        "- true\n"
        "- false\n"
        "- false\n");
  }
}

TEST_F(StandardLibraryTest, i64_math) {
  {
    auto result = render(
        "{{ (int.neg 0) }}\n"
        "{{ (int.neg 1) }}\n"
        "{{ (int.neg (int.neg 1)) }}\n",
        w::null);
    EXPECT_THAT(diagnostics(), testing::IsEmpty());
    EXPECT_EQ(
        *result,
        "0\n"
        "-1\n"
        "1\n");
  }

  {
    auto result = render(
        "{{ (int.add) }}\n"
        "{{ (int.add 1) }}\n"
        "{{ (int.add 1 2) }}\n"
        "{{ (int.add 1 2 3) }}\n"
        "{{ (int.add 1 2 3 4) }}\n"
        "{{ (int.add 1 2 3 4 5) }}\n"
        "{{ (int.add (int.neg 100) 1 2 3 4 5) }}\n",
        w::null);
    EXPECT_THAT(diagnostics(), testing::IsEmpty());
    EXPECT_EQ(
        *result,
        "0\n"
        "1\n"
        "3\n"
        "6\n"
        "10\n"
        "15\n"
        "-85\n");
  }

  {
    auto result = render(
        "{{ (int.sub 30 15) }}\n"
        "{{ (int.sub (int.neg 30) (int.neg 15)) }}\n"
        "{{ (int.sub 0 10) }}\n"
        "{{ (int.sub 10 0) }}\n"
        "{{ (int.sub 10 10) }}\n",
        w::null);
    EXPECT_THAT(diagnostics(), testing::IsEmpty());
    EXPECT_EQ(
        *result,
        "15\n"
        "-15\n"
        "-10\n"
        "10\n"
        "0\n");
  }
}

} // namespace whisker
