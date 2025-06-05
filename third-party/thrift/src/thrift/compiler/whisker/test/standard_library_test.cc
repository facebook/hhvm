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

#include <gmock/gmock.h>
#include <gtest/gtest.h>

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

TEST_F(StandardLibraryTest, array_at) {
  {
    auto result = render(
        "{{ (array.at (array.of 1 \"foo\" 2) 0) }}\n"
        "{{ (array.at (array.of 1 \"foo\" 2) 1) }}\n"
        "{{ (array.at (array.of 1 \"foo\" 2) 2) }}\n",
        w::null);
    EXPECT_THAT(diagnostics(), testing::IsEmpty());
    EXPECT_EQ(
        *result,
        "1\n"
        "foo\n"
        "2\n");
  }

  {
    auto result = render("{{ (array.at (array.of) 0) }}\n", w::null);
    EXPECT_FALSE(result.has_value());
    EXPECT_THAT(
        diagnostics(),
        testing::ElementsAre(diagnostic(
            diagnostic_level::error,
            "Function 'array.at' threw an error:\n"
            "Index '0' is out of bounds (size is 0).",
            path_to_file,
            1)));
  }

  {
    auto result = render("{{ (array.at (array.of 0 1 2) -1) }}\n", w::null);
    EXPECT_FALSE(result.has_value());
    EXPECT_THAT(
        diagnostics(),
        testing::ElementsAre(diagnostic(
            diagnostic_level::error,
            "Function 'array.at' threw an error:\n"
            "Index '-1' is out of bounds (size is 3).",
            path_to_file,
            1)));
  }
}

TEST_F(StandardLibraryTest, array_enumerate) {
  {
    auto result = render(
        "{{#each (array.enumerate (array.of 1 \"foo\" 2)) as |tuple|}}\n"
        "{{(array.at tuple 0)}}: {{(array.at tuple 1)}}\n"
        "{{/each}}\n",
        w::null);
    EXPECT_THAT(diagnostics(), testing::IsEmpty());
    EXPECT_EQ(
        *result,
        "0: 1\n"
        "1: foo\n"
        "2: 2\n");
  }

  {
    auto result = render(
        "{{#each (array.enumerate (array.of)) as |tuple|}}\n"
        "{{(array.at tuple 0)}}: {{(array.at tuple 1)}}\n"
        "{{/each}}\n",
        w::null);
    EXPECT_THAT(diagnostics(), testing::IsEmpty());
    EXPECT_EQ(*result, "");
  }
}

TEST_F(StandardLibraryTest, map_items) {
  {
    auto result = render(
        "{{#each (map.items this) as |entry|}}\n"
        "{{entry.key}}: {{entry.value}}\n"
        "{{/each}}\n",
        w::map(
            {{"foo", w::i64(3)},
             {"bar", w::i64(4)},
             {"baz", w::string("qux")}}));
    EXPECT_THAT(diagnostics(), testing::IsEmpty());
    EXPECT_EQ(
        *result,
        "bar: 4\n"
        "baz: qux\n"
        "foo: 3\n");
  }

  {
    auto result = render(
        "{{#each (map.items this) as |entry|}}\n"
        "{{entry.key}}: {{entry.value}}\n"
        "{{#else}}"
        "No items!\n"
        "{{/each}}\n",
        w::map());
    EXPECT_THAT(diagnostics(), testing::IsEmpty());
    EXPECT_EQ(*result, "No items!\n");
  }

  {
    auto result = render(
        "{{#each (map.items this) as |entry|}}\n"
        "{{entry.key}}:\n"
        "{{#each (map.items entry.value) as |entry|}}\n"
        "  {{entry.key}}: {{entry.value}}\n"
        "{{/each}}\n"
        "{{/each}}\n",
        w::map(
            {{"nested",
              w::map(
                  {{"foo", w::i64(3)},
                   {"bar", w::i64(4)},
                   {"baz", w::string("qux")}})},
             {"nested-2",
              w::map(
                  {{"a", w::string("a")},
                   {"b", w::i64(0)},
                   {"c", w::string("c")}})}}));
    EXPECT_THAT(diagnostics(), testing::IsEmpty());
    EXPECT_EQ(
        *result,
        "nested:\n"
        "  bar: 4\n"
        "  baz: qux\n"
        "  foo: 3\n"
        "nested-2:\n"
        "  a: a\n"
        "  b: 0\n"
        "  c: c\n");
  }

  {
    class map_not_enumerable : public map {
     public:
      std::optional<object> lookup_property(std::string_view) const override {
        return std::nullopt;
      }
    };

    auto result = render(
        "{{#each (map.items this) as |entry|}}\n"
        "{{entry.key}}: {{entry.value}}\n"
        "{{/each}}\n",
        w::make_map<map_not_enumerable>());
    EXPECT_FALSE(result.has_value());
    EXPECT_THAT(
        diagnostics(),
        testing::ElementsAre(diagnostic(
            diagnostic_level::error,
            "Function 'map.items' threw an error:\n"
            "map does not have enumerable properties.",
            path_to_file,
            1)));
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

TEST_F(StandardLibraryTest, object_eq) {
  strict_printable_types(diagnostic_level::info);

  {
    auto result = render(
        "{{ (object.eq? 0 1) }}\n"
        "{{ (object.eq? 1 1) }}\n"
        "{{ (object.eq? \"foo\" 1) }}\n"
        "{{ (object.eq? 1 null) }}\n"
        "{{ (object.eq? false null) }}\n"
        "{{ (object.eq? null null) }}\n",
        w::null);
    EXPECT_THAT(diagnostics(), testing::IsEmpty());
    EXPECT_EQ(
        *result,
        "false\n"
        "true\n"
        "false\n"
        "false\n"
        "false\n"
        "true\n");
  }

  {
    const array::raw a{w::i64(1), w::string("foo"), w::boolean(true)};
    const map::raw m{{"foo", w::string("bar")}, {"arr", w::array(a)}};
    const auto context = w::map({
        {"raw_array", w::array(a)},
        {"wrapped_array", custom_array::make(a)},
        {"raw_map", w::map(m)},
        {"wrapped_map", custom_map::make(m)},
    });

    auto result = render(
        "{{ (object.eq? raw_array raw_array) }}\n"
        "{{ (object.eq? raw_map raw_map) }}\n"
        "{{ (object.eq? \"foo\" raw_array) }}\n"
        "{{ (object.eq? null raw_map) }}\n"
        "{{ (object.eq? raw_array raw_map) }}\n",
        context);
    EXPECT_THAT(diagnostics(), testing::IsEmpty());
    EXPECT_EQ(
        *result,
        "true\n"
        "true\n"
        "false\n"
        "false\n"
        "false\n");

    result = render(
        "{{ (object.eq? raw_array wrapped_array) }}\n"
        "{{ (object.eq? wrapped_array raw_array) }}\n"
        "{{ (object.eq? raw_map wrapped_map) }}\n"
        "{{ (object.eq? wrapped_map raw_map) }}\n",
        context);
    EXPECT_THAT(diagnostics(), testing::IsEmpty());
    EXPECT_EQ(
        *result,
        "true\n"
        "true\n"
        "true\n"
        "true\n");
  }

  {
    const array::raw a1{w::i64(1), w::string("foo"), w::boolean(true)};
    const array::raw a2{w::i64(1), w::string("bar"), w::boolean(true)};
    const auto context = w::map({
        {"a1", w::array(a1)},
        {"wrapped_a1", custom_array::make(a1)},
        {"a2", w::array(a2)},
        {"wrapped_a2", custom_array::make(a2)},
    });
    auto result = render(
        "{{ (object.eq? a1 wrapped_a1) }}\n"
        "{{ (object.eq? a1 a2) }}\n"
        "{{ (object.eq? wrapped_a2 a1) }}\n"
        "{{ (object.eq? wrapped_a2 a2) }}\n",
        context);
    EXPECT_THAT(diagnostics(), testing::IsEmpty());
    EXPECT_EQ(
        *result,
        "true\n"
        "false\n"
        "false\n"
        "true\n");
  }

  {
    const map::raw m1{{"foo", w::string("bar")}, {"baz", w::true_}};
    const map::raw m2{{"foo", w::string("bar")}, {"baz", w::false_}};
    const auto context = w::map({
        {"m1", w::map(m1)},
        {"wrapped_m1", custom_map::make(m1)},
        {"m2", w::map(m2)},
        {"wrapped_m2", custom_map::make(m2)},
    });
    auto result = render(
        "{{ (object.eq? m1 wrapped_m1) }}\n"
        "{{ (object.eq? m1 m2) }}\n"
        "{{ (object.eq? wrapped_m2 m1) }}\n"
        "{{ (object.eq? wrapped_m2 m2) }}\n",
        context);
    EXPECT_THAT(diagnostics(), testing::IsEmpty());
    EXPECT_EQ(
        *result,
        "true\n"
        "false\n"
        "false\n"
        "true\n");
  }
}

} // namespace whisker
