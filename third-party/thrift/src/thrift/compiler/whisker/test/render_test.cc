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
#include <thrift/compiler/whisker/dsl.h>
#include <thrift/compiler/whisker/object.h>
#include <thrift/compiler/whisker/standard_library.h>
#include <thrift/compiler/whisker/test/render_test_helpers.h>

namespace w = whisker::make;

namespace whisker {

namespace {

/**
 * When looking up a property, always returns a whisker::string that is the
 * property name repeated twice.
 */
class double_property_name : public map {
 public:
  std::optional<object> lookup_property(std::string_view id) const override {
    return w::string(fmt::format("{0}{0}", id));
  }
};

class throws_on_lookup : public map {
 public:
  [[noreturn]] std::optional<object> lookup_property(
      std::string_view) const override {
    throw eval_error{"I always throw!"};
  }
};

} // namespace

TEST_F(RenderTest, basic) {
  auto result = render(
      "Some text {{foo}} More text", w::map({{"foo", w::string("bar")}}));
  EXPECT_EQ(*result, "Some text bar More text");
}

TEST_F(RenderTest, variable_missing_in_scope) {
  auto result = render("Some text {{foo}} More text", w::map());
  EXPECT_FALSE(result.has_value());
  EXPECT_THAT(
      diagnostics(),
      testing::ElementsAre(diagnostic(
          diagnostic_level::error,
          "Name 'foo' was not found in the current scope.\n"
          "Tried to search through the following scopes:\n"
          "#0 map (size=0)\n"
          "\n"
          "#1 <global scope> (size=0)\n",
          path_to_file,
          1)));
}

TEST_F(RenderTest, variable_throws_on_scope_lookup) {
  auto result = render("{{foo}}", w::make_map<throws_on_lookup>());
  EXPECT_FALSE(result.has_value());
  EXPECT_THAT(
      diagnostics(),
      testing::ElementsAre(diagnostic(
          diagnostic_level::error,
          "Name 'foo' was not found in the current scope.\n"
          "Cause: I always throw!\n"
          "Tried to search through the following scopes:\n"
          "#0 map [custom] (not enumerable)\n"
          "\n"
          "#1 <global scope> (size=0)\n",
          path_to_file,
          1)));
}

TEST_F(RenderTest, variable_missing_property_in_object) {
  auto result = render(
      "Some text {{foo.bar}} More text",
      w::map({{"foo", w::map({{"baz", w::string("qux")}})}}));
  EXPECT_FALSE(result.has_value());
  EXPECT_THAT(
      diagnostics(),
      testing::ElementsAre(diagnostic(
          diagnostic_level::error,
          "Object 'foo' has no property named 'bar'.\n"
          "The object with the missing property is:\n"
          "map (size=1)\n"
          "╰─ 'baz' → 'qux'\n",
          path_to_file,
          1)));
}

TEST_F(RenderTest, variable_throws_on_property_lookup) {
  auto result =
      render("{{foo.bar}}", w::map({{"foo", w::make_map<throws_on_lookup>()}}));
  EXPECT_FALSE(result.has_value());
  EXPECT_THAT(
      diagnostics(),
      testing::ElementsAre(diagnostic(
          diagnostic_level::error,
          "Object 'foo' has no property named 'bar'.\n"
          "Cause: I always throw!\n"
          "The object with the missing property is:\n"
          "map [custom] (not enumerable)\n",
          path_to_file,
          1)));
}

TEST_F(RenderTest, section_block_array) {
  auto result = render(
      "The factorial function looks like:{{#factorials}}\n"
      "{{.}}{{/factorials}}",
      w::map(
          {{"factorials",
            w::array(
                {w::i64(1), w::i64(2), w::i64(6), w::i64(24), w::i64(120)})}}));
  EXPECT_EQ(
      *result,
      "The factorial function looks like:\n"
      "1\n"
      "2\n"
      "6\n"
      "24\n"
      "120");
}

TEST_F(RenderTest, section_block_array_nested) {
  auto result = render(
      "{{#.}}{{#b}} {{c}} {{/b}}{{/.}}",
      w::array(
          {w::map(
               {{"b",
                 w::array(
                     {w::map({{"c", w::i64(1)}}),
                      w::map({{"c", w::i64(2)}})})}}),
           w::map(
               {{"b",
                 w::array(
                     {w::map({{"c", w::i64(3)}}),
                      w::map({{"c", w::i64(4)}})})}}),
           w::map(
               {{"b",
                 w::array(
                     {w::map({{"c", w::i64(5)}}),
                      w::map({{"c", w::i64(6)}})})}})}));
  EXPECT_EQ(*result, " 1  2  3  4  5  6 ");
}

TEST_F(RenderTest, section_block_array_with_nested_objects) {
  auto result = render(
      "The factorial function looks like:\n"
      "{{#factorials}}\n"
      "  {{value}}\n"
      "{{/factorials}}",
      w::map(
          {{"factorials",
            w::array(
                {w::map({{"value", w::i64(1)}}),
                 w::map({{"value", w::string("2")}}),
                 w::map({{"value", w::i64(6)}}),
                 w::map({{"value", w::i64(24)}}),
                 w::map({{"value", w::i64(120)}})})}}));
  EXPECT_EQ(
      *result,
      "The factorial function looks like:\n"
      "  1\n"
      "  2\n"
      "  6\n"
      "  24\n"
      "  120\n");
}

TEST_F(RenderTest, section_block_array_asymmetric_nested_scopes) {
  show_source_backtrace_on_failure(true);
  auto result = render(
      "The factorial function looks like:\n"
      "{{#factorials}}\n"
      "  {{value}}\n"
      "{{/factorials}}",
      w::map(
          {{"factorials",
            w::array(
                {w::map({{"value", w::i64(1)}}),
                 w::map({{"oops", w::null}}), // missing value, scope should
                                              // have been cleared
                 w::map({{"value", w::i64(6)}}),
                 w::map({{"value", w::i64(24)}}),
                 w::map({{"value", w::i64(120)}})})}}));
  EXPECT_THAT(
      diagnostics(),
      testing::ElementsAre(
          diagnostic(
              diagnostic_level::error,
              "Name 'value' was not found in the current scope.\n"
              "Tried to search through the following scopes:\n"
              "#0 map (size=1)\n"
              "╰─ 'oops' → null\n"
              "\n"
              "#1 map (size=1)\n"
              "╰─ 'factorials' → array (size=5)\n"
              "   ├─ [0] map (size=1)\n"
              "   │  ╰─ 'value' → i64(1)\n"
              "   ├─ [1] map (size=1)\n"
              "   │  ╰─ 'oops' → null\n"
              "   ├─ [2] map (size=1)\n"
              "   │  ╰─ 'value' → i64(6)\n"
              "   ├─ [3] map (size=1)\n"
              "   │  ╰─ 'value' → i64(24)\n"
              "   ╰─ [4] map (size=1)\n"
              "      ╰─ 'value' → i64(120)\n"
              "\n"
              "#2 <global scope> (size=0)\n",
              path_to_file,
              3),
          error_backtrace("#0 path/to/test.whisker <line:3, col:5>\n")));
}

TEST_F(RenderTest, section_block_array_iterable_custom_array) {
  auto factorials = w::map(
      {{"factorials",
        custom_array::make(
            {w::map({{"value", w::i64(1)}}),
             w::map({{"value", w::string("2")}}),
             w::map({{"value", w::i64(6)}}),
             w::map({{"value", w::i64(24)}}),
             w::map({{"value", w::i64(120)}})})}});
  {
    auto result = render(
        "The factorial function looks like:{{#factorials}}\n"
        "{{value}}{{/factorials}}",
        factorials);
    EXPECT_EQ(
        *result,
        "The factorial function looks like:\n"
        "1\n"
        "2\n"
        "6\n"
        "24\n"
        "120");
  }
  {
    strict_boolean_conditional(diagnostic_level::info);
    auto result = render(
        "The factorial function looks like:\n"
        "{{^factorials}}\n"
        "{{value}}\n"
        "{{/factorials}}",
        factorials);
    EXPECT_EQ(*result, "The factorial function looks like:\n");
  }
}

TEST_F(RenderTest, section_block_map) {
  auto factorials = w::map(
      {{"factorials",
        w::map(
            {{"first", w::i64(1)},
             {"second", w::string("2")},
             {"third", w::i64(6)}})}});
  {
    auto result = render(
        "The factorial function looks like:{{#factorials}}\n"
        "{{first}}\n"
        "{{second}}\n"
        "{{third}}\n"
        "{{/factorials}}",
        factorials);
    EXPECT_EQ(
        *result,
        "The factorial function looks like:\n"
        "1\n"
        "2\n"
        "6\n");
  }
  {
    strict_boolean_conditional(diagnostic_level::info);
    auto result = render(
        "The factorial function looks like:{{^factorials}}\n"
        "{{first}}\n"
        "{{second}}\n"
        "{{third}}\n"
        "{{/factorials}}",
        factorials);
    EXPECT_EQ(*result, "The factorial function looks like:");
  }
}

TEST_F(RenderTest, section_block_custom_map) {
  auto factorials = w::map(
      {{"factorials",
        custom_map::make(
            {{"first", w::i64(1)},
             {"second", w::string("2")},
             {"third", w::i64(6)}})}});
  {
    auto result = render(
        "The factorial function looks like:{{#factorials}}\n"
        "{{first}}\n"
        "{{second}}\n"
        "{{third}}\n"
        "{{/factorials}}",
        factorials);
    EXPECT_EQ(
        *result,
        "The factorial function looks like:\n"
        "1\n"
        "2\n"
        "6\n");
  }
  {
    strict_boolean_conditional(diagnostic_level::info);
    auto result = render(
        "The factorial function looks like:\n"
        "{{^factorials}}\n"
        "{{first}}\n"
        "{{second}}\n"
        "{{third}}\n"
        "{{/factorials}}",
        factorials);
    EXPECT_EQ(*result, "The factorial function looks like:\n");
  }
}

TEST_F(RenderTest, section_block_empty_array) {
  auto result = render(
      "The factorial function looks like:{{#factorials}}\n"
      "{{value}}\n"
      "{{/factorials}}",
      w::map({{"factorials", w::array()}}));
  EXPECT_EQ(*result, "The factorial function looks like:");
}

TEST_F(RenderTest, section_block_boolean_condition) {
  {
    auto result = render(
        "{{#news.has-update?}}Stuff is {{foo}} happening!{{/news.has-update?}}",
        w::map(
            {{"news", w::map({{"has-update?", w::true_}})},
             {"foo", w::string("now")}}));
    EXPECT_EQ(*result, "Stuff is now happening!");
  }
  {
    auto result = render(
        "{{#news.has-update?}}Stuff is {{foo}} happening!{{/news.has-update?}}",
        w::map({{"news", w::map({{"has-update?", w::false_}})}}));
    EXPECT_EQ(*result, "");
  }
}

TEST_F(RenderTest, section_block_non_boolean_condition_failure) {
  show_source_backtrace_on_failure(true);
  auto result = render(
      "{{#news.has-update?}}Stuff is {{foo}} happening!{{/news.has-update?}}",
      w::map(
          {{"news", w::map({{"has-update?", w::i64(1)}})},
           {"foo", w::string("now")}}));
  EXPECT_FALSE(result.has_value());
  EXPECT_THAT(
      diagnostics(),
      testing::ElementsAre(
          diagnostic(
              diagnostic_level::error,
              "Condition 'news.has-update?' is not a boolean. The encountered value is:\n"
              "i64(1)\n",
              path_to_file,
              1),
          error_backtrace("#0 path/to/test.whisker <line:1, col:4>\n")));
}

TEST_F(RenderTest, section_block_non_boolean_condition_warning) {
  strict_boolean_conditional(diagnostic_level::warning);

  auto result = render(
      "{{#news.has-update?}}Stuff is {{foo}} happening!{{/news.has-update?}}",
      w::map(
          {{"news", w::map({{"has-update?", w::i64(1)}})},
           {"foo", w::string("now")}}));
  EXPECT_EQ(*result, "Stuff is now happening!");
  EXPECT_THAT(
      diagnostics(),
      testing::ElementsAre(diagnostic(
          diagnostic_level::warning,
          "Condition 'news.has-update?' is not a boolean. The encountered value is:\n"
          "i64(1)\n",
          path_to_file,
          1)));
}

TEST_F(RenderTest, section_block_non_boolean_condition_allowed) {
  strict_boolean_conditional(diagnostic_level::info);

  auto result = render(
      "{{#news.has-update?}}Stuff is {{foo}} happening!{{/news.has-update?}}",
      w::map(
          {{"news", w::map({{"has-update?", w::i64(1)}})},
           {"foo", w::string("now")}}));
  EXPECT_EQ(*result, "Stuff is now happening!");
  EXPECT_THAT(diagnostics(), testing::IsEmpty());
}

TEST_F(RenderTest, section_block_inversion_boolean_condition) {
  {
    auto result = render(
        "{{^news.has-no-update?}}Stuff is {{foo}} happening!{{/news.has-no-update?}}",
        w::map(
            {{"news", w::map({{"has-no-update?", w::false_}})},
             {"foo", w::string("now")}}));
    EXPECT_EQ(*result, "Stuff is now happening!");
  }
  {
    auto result = render(
        "{{^news.has-no-update?}}Stuff is {{foo}} happening!{{/news.has-no-update?}}",
        w::map({{"news", w::map({{"has-no-update?", w::true_}})}}));
    EXPECT_EQ(*result, "");
  }
}

TEST_F(RenderTest, section_block_inversion_array) {
  strict_boolean_conditional(diagnostic_level::info);
  auto result = render(
      "The factorial function looks like:\n"
      "{{^factorials}}\n"
      "{{.}}{{/factorials}}",
      w::map(
          {{"factorials",
            w::array(
                {w::i64(1), w::i64(2), w::i64(6), w::i64(24), w::i64(120)})}}));
  EXPECT_EQ(*result, "The factorial function looks like:\n");
}

TEST_F(RenderTest, section_block_inversion_non_boolean_condition_failure) {
  auto result = render(
      "{{^news.has-update?}}Stuff is {{foo}} happening!{{/news.has-update?}}",
      w::map(
          {{"news", w::map({{"has-update?", w::i64(1)}})},
           {"foo", w::string("now")}}));
  EXPECT_FALSE(result.has_value());
  EXPECT_THAT(
      diagnostics(),
      testing::ElementsAre(diagnostic(
          diagnostic_level::error,
          "Condition 'news.has-update?' is not a boolean. The encountered value is:\n"
          "i64(1)\n",
          path_to_file,
          1)));
}

TEST_F(RenderTest, section_block_inversion_non_boolean_condition_warning) {
  strict_boolean_conditional(diagnostic_level::warning);

  auto result = render(
      "{{^news.has-update?}}Stuff is {{foo}} happening!{{/news.has-update?}}",
      w::map(
          {{"news", w::map({{"has-update?", w::i64(0)}})},
           {"foo", w::string("now")}}));
  EXPECT_EQ(*result, "Stuff is now happening!");
  EXPECT_THAT(
      diagnostics(),
      testing::ElementsAre(diagnostic(
          diagnostic_level::warning,
          "Condition 'news.has-update?' is not a boolean. The encountered value is:\n"
          "i64(0)\n",
          path_to_file,
          1)));
}

TEST_F(RenderTest, if_block) {
  {
    auto result = render(
        "{{#if news.has-update?}}\n"
        "Stuff is {{foo}} happening!\n"
        "{{/if news.has-update?}}\n",
        w::map(
            {{"news", w::map({{"has-update?", w::true_}})},
             {"foo", w::string("now")}}));
    EXPECT_EQ(*result, "Stuff is now happening!\n");
  }
  {
    auto result = render(
        "{{#if news.has-update?}}Stuff is {{foo}} happening!{{/if news.has-update?}}",
        w::map({{"news", w::map({{"has-update?", w::false_}})}}));
    EXPECT_EQ(*result, "");
  }
}

TEST_F(RenderTest, unless_block) {
  {
    auto result = render(
        "{{#if (not news.has-update?)}}\n"
        "Stuff is {{foo}} happening!\n"
        "{{/if (not news.has-update?)}}\n",
        w::map(
            {{"news", w::map({{"has-update?", w::false_}})},
             {"foo", w::string("now")}}));
    EXPECT_EQ(*result, "Stuff is now happening!\n");
  }
  {
    auto result = render(
        "{{#if (not news.has-update?)}}Stuff is {{foo}} happening!{{/if (not news.has-update?)}}",
        w::map({{"news", w::map({{"has-update?", w::true_}})}}));
    EXPECT_EQ(*result, "");
  }
}

TEST_F(RenderTest, if_else_block) {
  {
    auto result = render(
        "{{#if news.has-update?}}\n"
        "Stuff is {{foo}} happening!\n"
        "{{#else}}\n"
        "Nothing is happening!\n"
        "{{/if news.has-update?}}\n",
        w::map(
            {{"news", w::map({{"has-update?", w::true_}})},
             {"foo", w::string("now")}}));
    EXPECT_EQ(*result, "Stuff is now happening!\n");
  }
  {
    auto result = render(
        "{{#if news.has-update?}}\n"
        "Stuff is {{foo}} happening!\n"
        "{{#else}}\n"
        "Nothing is happening!\n"
        "{{/if news.has-update?}}\n",
        w::map({{"news", w::map({{"has-update?", w::false_}})}}));
    EXPECT_EQ(*result, "Nothing is happening!\n");
  }
}

TEST_F(RenderTest, else_if_blocks) {
  const std::string template_text =
      "{{#if a}}\n"
      "a\n"
      "{{#else if b}}\n"
      "b\n"
      "{{#else if c}}\n"
      "c\n"
      "{{#else}}\n"
      "d\n"
      "{{/if a}}\n";

  {
    auto result = render(template_text, w::map({{"a", w::true_}}));
    EXPECT_EQ(*result, "a\n");
  }
  {
    auto result =
        render(template_text, w::map({{"a", w::false_}, {"b", w::true_}}));
    EXPECT_EQ(*result, "b\n");
  }
  {
    auto result = render(
        template_text,
        w::map({{"a", w::false_}, {"b", w::false_}, {"c", w::true_}}));
    EXPECT_EQ(*result, "c\n");
  }
  {
    auto result = render(
        template_text,
        w::map({{"a", w::false_}, {"b", w::false_}, {"c", w::false_}}));
    EXPECT_EQ(*result, "d\n");
  }
}

TEST_F(RenderTest, unless_else_block) {
  {
    auto result = render(
        "{{#if (not news.has-update?)}}\n"
        "Nothing is happening!\n"
        "{{#else}}\n"
        "Stuff is {{foo}} happening!\n"
        "{{/if (not news.has-update?)}}\n",
        w::map({{"news", w::map({{"has-update?", w::false_}})}}));
    EXPECT_EQ(*result, "Nothing is happening!\n");
  }
  {
    auto result = render(
        "{{#if (not news.has-update?)}}\n"
        "Nothing is happening!\n"
        "{{#else}}\n"
        "Stuff is {{foo}} happening!\n"
        "{{/if (not news.has-update?)}}\n",
        w::map(
            {{"news", w::map({{"has-update?", w::true_}})},
             {"foo", w::string("now")}}));
    EXPECT_EQ(*result, "Stuff is now happening!\n");
  }
}

TEST_F(RenderTest, if_not_else_block) {
  {
    auto result = render(
        "{{#if (not news.has-update?)}}\n"
        "Nothing is happening!\n"
        "{{#else}}\n"
        "Stuff is {{foo}} happening!\n"
        "{{/if (not news.has-update?)}}\n",
        w::map({{"news", w::map({{"has-update?", w::false_}})}}));
    EXPECT_THAT(diagnostics(), testing::IsEmpty());
    EXPECT_EQ(*result, "Nothing is happening!\n");
  }
  {
    auto result = render(
        "{{#if (not news.has-update?)}}\n"
        "Nothing is happening!\n"
        "{{#else}}\n"
        "Stuff is {{foo}} happening!\n"
        "{{/if (not news.has-update?)}}\n",
        w::map(
            {{"news", w::map({{"has-update?", w::true_}})},
             {"foo", w::string("now")}}));
    EXPECT_THAT(diagnostics(), testing::IsEmpty());
    EXPECT_EQ(*result, "Stuff is now happening!\n");
  }
}

TEST_F(RenderTest, and_or) {
  {
    auto result = render(
        "{{#if (and (or no yes) yes)}}\n"
        "Yes!\n"
        "{{/if (and (or no yes) yes)}}\n",
        w::map({{"yes", w::true_}, {"no", w::false_}}));
    EXPECT_THAT(diagnostics(), testing::IsEmpty());
    EXPECT_EQ(*result, "Yes!\n");
  }
}

TEST_F(RenderTest, and_or_short_circuit) {
  {
    auto result = render(
        "{{#if (and news.has-update? intentionally_undefined)}}\n"
        "Oops!\n"
        "{{/if (and news.has-update? intentionally_undefined)}}\n"
        "{{#if (or (not news.has-update?) intentionally_undefined)}}\n"
        "Nothing is happening!\n"
        "{{/if (or (not news.has-update?) intentionally_undefined)}}\n",
        w::map({{"news", w::map({{"has-update?", w::false_}})}}));
    EXPECT_THAT(diagnostics(), testing::IsEmpty());
    EXPECT_EQ(*result, "Nothing is happening!\n");
  }
}

namespace {
namespace functions {

/**
 * Adds i64 together. If the 'negate' argument is true, then the final result is
 * negated.
 */
class add : public dsl::function {
  object invoke(context ctx) override {
    ctx.declare_named_arguments({"negate"});
    const bool negate = [&] {
      auto arg = ctx.named_argument<boolean>("negate", context::optional);
      return arg.value_or(false);
    }();
    i64 result = 0;
    for (std::size_t i = 0; i < ctx.arity(); ++i) {
      result += ctx.argument<i64>(i);
    }
    return w::i64(negate ? -result : result);
  }
};

/**
 * Returns a boolean indicating if two i64 are equal or not.
 */
class i64_eq : public dsl::function {
  object invoke(context ctx) override {
    ctx.declare_arity(2);
    ctx.declare_named_arguments({});
    i64 a = ctx.argument<i64>(0);
    i64 b = ctx.argument<i64>(1);
    return w::boolean(a == b);
  }
};

/**
 * Concatenates a sequences of strings together. If the 'sep' argument is
 * provided, then it is used as the delimiter between elements.
 */
class str_concat : public dsl::function {
  object invoke(context ctx) override {
    ctx.declare_named_arguments({"sep"});
    const std::string sep = [&] {
      auto arg = ctx.named_argument<string>("sep", context::optional);
      return arg.has_value() ? std::string{*arg} : "";
    }();
    string result;
    for (std::size_t i = 0; i < ctx.arity(); ++i) {
      if (i != 0) {
        result += sep;
      }
      result += ctx.argument<string>(i);
    }
    return w::string(std::move(result));
  }
};

/**
 * Returns the length of an array.
 */
class array_len : public dsl::function {
  object invoke(context ctx) override {
    ctx.declare_arity(1);
    ctx.declare_named_arguments({});
    auto len = i64(ctx.argument<array>(0)->size());
    return w::i64(len);
  }
};

/**
 * Dynamically accesses a property by name in a map, throwing an error if not
 * present.
 */
class map_get : public dsl::function {
  object invoke(context ctx) override {
    ctx.declare_arity(1);
    ctx.declare_named_arguments({"key"});
    auto m = ctx.argument<map>(0);
    auto key = ctx.named_argument<string>("key", context::required);

    if (std::optional<object> result = m->lookup_property(*key)) {
      return std::move(*result);
    }
    throw ctx.make_error("Key '{}' not found.", *key);
  }
};

} // namespace functions
} // namespace

TEST_F(RenderTest, user_defined_function) {
  {
    auto result = render(
        "{{ (lib.add hello world negate=true) }}\n",
        w::map({
            {"lib",
             w::map({
                 {"add", w::make_native_function<functions::add>()},
             })},
            {"hello", w::i64(1)},
            {"world", w::i64(2)},
        }));
    EXPECT_EQ(*result, "-3\n");
  }
  {
    auto result = render(
        "{{#if (i64-eq 100 100)}}\n"
        "100 is, in fact, 100\n"
        "{{/if (i64-eq 100 100)}}\n"

        "{{#if (not (i64-eq 0 -1))}}\n"
        "0 is NOT -1\n"
        "{{/if (not (i64-eq 0 -1))}}\n",
        w::map({
            {"i64-eq", w::make_native_function<functions::i64_eq>()},
        }));
    EXPECT_EQ(
        *result,
        "100 is, in fact, 100\n"
        "0 is NOT -1\n");
  }
}

TEST_F(RenderTest, user_defined_function_type_error) {
  show_source_backtrace_on_failure(true);
  auto result = render(
      "{{ (add 1 true) }}\n",
      w::map({
          {"add", w::make_native_function<functions::add>()},
      }));
  EXPECT_FALSE(result.has_value());
  EXPECT_THAT(
      diagnostics(),
      testing::ElementsAre(
          diagnostic(
              diagnostic_level::error,
              "Function 'add' threw an error:\n"
              "Expected type of argument at index 1 to be `i64`, but found `boolean`.",
              path_to_file,
              1),
          error_backtrace("#0 path/to/test.whisker <line:1, col:5>\n")));
}

TEST_F(RenderTest, user_defined_function_named_argument_type_error) {
  show_source_backtrace_on_failure(true);
  auto result = render(
      "{{ (add 1 2 negate=1) }}\n",
      w::map({
          {"add", w::make_native_function<functions::add>()},
      }));
  EXPECT_FALSE(result.has_value());
  EXPECT_THAT(
      diagnostics(),
      testing::ElementsAre(
          diagnostic(
              diagnostic_level::error,
              "Function 'add' threw an error:\n"
              "Expected type of named argument 'negate' to be `boolean`, but found `i64`.",
              path_to_file,
              1),
          error_backtrace("#0 path/to/test.whisker <line:1, col:5>\n")));
}

TEST_F(RenderTest, user_defined_function_arity_error) {
  {
    auto result = render(
        "{{ (i64-eq 1) }}\n",
        w::map({
            {"i64-eq", w::make_native_function<functions::i64_eq>()},
        }));
    EXPECT_THAT(
        diagnostics(),
        testing::ElementsAre(diagnostic(
            diagnostic_level::error,
            "Function 'i64-eq' threw an error:\n"
            "Expected 2 argument(s) but got 1",
            path_to_file,
            1)));
  }
  {
    auto result = render(
        "{{ (i64-eq 1 2 3) }}\n",
        w::map({
            {"i64-eq", w::make_native_function<functions::i64_eq>()},
        }));
    EXPECT_THAT(
        diagnostics(),
        testing::ElementsAre(diagnostic(
            diagnostic_level::error,
            "Function 'i64-eq' threw an error:\n"
            "Expected 2 argument(s) but got 3",
            path_to_file,
            1)));
  }
}

TEST_F(RenderTest, user_defined_function_named_argument_error) {
  auto result = render(
      "{{ (add 1 2 unknown=true foo=false negate=true) }}\n",
      w::map({
          {"add", w::make_native_function<functions::add>()},
      }));
  EXPECT_FALSE(result.has_value());
  EXPECT_THAT(
      diagnostics(),
      testing::ElementsAre(diagnostic(
          diagnostic_level::error,
          "Function 'add' threw an error:\n"
          "Unknown named argument(s) provided: foo, unknown.",
          path_to_file,
          1)));
}

TEST_F(RenderTest, user_defined_function_nested) {
  auto result = render(
      R"({{ (concat
        (concat "James" "Bond" sep=" ")
        (concat "Alan" "Turing" sep=" ")
        sep=", "
      ) }})",
      w::map({{"concat", w::make_native_function<functions::str_concat>()}}));
  EXPECT_EQ(*result, "James Bond, Alan Turing");
}

TEST_F(RenderTest, user_defined_function_array_like_argument) {
  const array::raw arr{w::i64(1), w::string("foo"), w::i64(100)};
  const auto context = w::map(
      {{"array", w::array(arr)},
       {"array_like", custom_array::make(arr)},
       {"not_array", w::string("not an array")},
       {"len", w::make_native_function<functions::array_len>()}});

  {
    auto result = render(
        "{{(len array)}}\n"
        "{{(len array_like)}}\n",
        context);
    EXPECT_THAT(diagnostics(), testing::IsEmpty());
    EXPECT_EQ(
        *result,
        "3\n"
        "3\n");
  }

  {
    auto result = render("{{(len not_array)}}\n", context);
    EXPECT_FALSE(result.has_value());
    EXPECT_THAT(
        diagnostics(),
        testing::ElementsAre(diagnostic(
            diagnostic_level::error,
            "Function 'len' threw an error:\n"
            "Expected type of argument at index 0 to be `array`, but found `string`.",
            path_to_file,
            1)));
  }
}

TEST_F(RenderTest, user_defined_function_array_like_named_argument) {
  class describe_array_len : public dsl::function {
    object invoke(context ctx) override {
      ctx.declare_arity(0);
      ctx.declare_named_arguments({"input"});
      auto len = i64(ctx.named_argument<array>("input")->size());
      return w::string(fmt::format("length is {}", len));
    }
  };

  const array::raw arr{w::i64(1), w::string("foo"), w::i64(100)};
  const auto context = w::map(
      {{"array", w::array(arr)},
       {"array_like", custom_array::make(arr)},
       {"not_array", w::string("not an array")},
       {"describe_len", w::make_native_function<describe_array_len>()}});

  {
    auto result = render(
        "{{(describe_len input=array)}}\n"
        "{{(describe_len input=array_like)}}\n",
        context);
    EXPECT_THAT(diagnostics(), testing::IsEmpty());
    EXPECT_EQ(
        *result,
        "length is 3\n"
        "length is 3\n");
  }

  {
    auto result = render("{{(describe_len input=not_array)}}\n", context);
    EXPECT_FALSE(result.has_value());
    EXPECT_THAT(
        diagnostics(),
        testing::ElementsAre(diagnostic(
            diagnostic_level::error,
            "Function 'describe_len' threw an error:\n"
            "Expected type of named argument 'input' to be `array`, but found `string`.",
            path_to_file,
            1)));
  }
}

TEST_F(RenderTest, user_defined_function_map_like_argument) {
  const map::raw m{
      {"a", w::i64(1)}, {"b", w::string("foo")}, {"c", w::i64(100)}};
  const auto context = w::map(
      {{"map", w::map(m)},
       {"map_like", custom_map::make(m)},
       {"not_map", w::string("not a map")},
       {"get", w::make_native_function<functions::map_get>()}});

  {
    auto result = render(
        "{{ (get map      key=\"b\") }}\n"
        "{{ (get map_like key=\"b\") }}\n",
        context);
    EXPECT_THAT(diagnostics(), testing::IsEmpty());
    EXPECT_EQ(
        *result,
        "foo\n"
        "foo\n");
  }

  {
    auto result = render("{{(get not_map)}}\n", context);
    EXPECT_FALSE(result.has_value());
    EXPECT_THAT(
        diagnostics(),
        testing::ElementsAre(diagnostic(
            diagnostic_level::error,
            "Function 'get' threw an error:\n"
            "Expected type of argument at index 0 to be `map`, but found `string`.",
            path_to_file,
            1)));
  }
}

TEST_F(RenderTest, user_defined_function_map_like_named_argument) {
  class describe_map_get : public dsl::function {
    object invoke(context ctx) override {
      ctx.declare_arity(0);
      ctx.declare_named_arguments({"input", "key"});
      auto m = ctx.named_argument<map>("input", context::required);
      auto key = ctx.named_argument<string>("key", context::required);

      std::string_view result =
          m->lookup_property(*key).has_value() ? "present" : "missing";
      return w::string(fmt::format("map element is {}", result));
    }
  };

  const map::raw m{
      {"a", w::i64(1)}, {"b", w::string("foo")}, {"c", w::i64(100)}};
  const auto context = w::map(
      {{"map", w::map(m)},
       {"map_like", custom_map::make(m)},
       {"not_map", w::string("not a map")},
       {"describe_get", w::make_native_function<describe_map_get>()}});

  {
    auto result = render(
        "{{ (describe_get input=map      key=\"b\") }}\n"
        "{{ (describe_get input=map_like key=\"b\") }}\n"
        "{{ (describe_get input=map_like key=\"missing\") }}\n",
        context);
    EXPECT_THAT(diagnostics(), testing::IsEmpty());
    EXPECT_EQ(
        *result,
        "map element is present\n"
        "map element is present\n"
        "map element is missing\n");
  }

  {
    auto result =
        render("{{(describe_get input=not_map key=\"b\")}}\n", context);
    EXPECT_FALSE(result.has_value());
    EXPECT_THAT(
        diagnostics(),
        testing::ElementsAre(diagnostic(
            diagnostic_level::error,
            "Function 'describe_get' threw an error:\n"
            "Expected type of named argument 'input' to be `map`, but found `string`.",
            path_to_file,
            1)));
  }
}

// External linkage to avoid noise in demangled type name.
struct RenderTestUnknownCppType {};
struct RenderTestMyCppType {
  std::string description;
};

struct RenderTestPolyBase {
  virtual ~RenderTestPolyBase() = default;

  virtual std::string description() const = 0;
};
struct RenderTestPolyMid : RenderTestPolyBase {
  std::string description() const override { return "Mid"; }
};
struct RenderTestPolyDerived : RenderTestPolyMid {
  std::string description() const override { return "Derived"; }
};
struct RenderTestPolyAlternate : RenderTestPolyBase {
  std::string description() const override { return "Alternate"; }
};

TEST_F(RenderTest, user_defined_function_native_ref_argument) {
  const RenderTestMyCppType native_instance{"Hello from C++!"};

  class describe : public dsl::function {
    object invoke(context ctx) override {
      ctx.declare_arity(1);
      ctx.declare_named_arguments({});
      auto cpp_type = ctx.argument<native_handle<RenderTestMyCppType>>(0);
      return w::string(cpp_type->description);
    }
  };

  {
    auto result = render(
        "{{ (describe native_instance) }}\n",
        w::map({
            {"native_instance",
             w::native_handle(
                 manage_as_static(native_instance), nullptr /* prototype */)},
            {"describe", w::make_native_function<describe>()},
        }));
    EXPECT_THAT(diagnostics(), testing::IsEmpty());
    EXPECT_EQ(*result, "Hello from C++!\n");
  }

  {
    auto result = render(
        "{{ (describe wrong_native_instance) }}\n",
        w::map({
            {"wrong_native_instance",
             w::native_handle(
                 manage_owned<RenderTestUnknownCppType>(),
                 nullptr /* prototype */)},
            {"describe", w::make_native_function<describe>()},
        }));
    EXPECT_FALSE(result.has_value());
    EXPECT_THAT(
        diagnostics(),
        testing::ElementsAre(diagnostic(
            diagnostic_level::error,
            "Function 'describe' threw an error:\n"
            "Expected type of argument at index 0 to be `<native_handle type='whisker::RenderTestMyCppType'>`, but found `<native_handle type='whisker::RenderTestUnknownCppType'>`.",
            path_to_file,
            1)));
  }
}

TEST_F(RenderTest, user_defined_function_polymorphic_native_ref_argument) {
  const auto describe =
      dsl::make_function([](dsl::function::context ctx) -> string {
        using base_handle = dsl::polymorphic_native_handle<
            RenderTestPolyBase,
            RenderTestPolyMid,
            RenderTestPolyDerived>;
        ctx.declare_arity(1);
        ctx.declare_named_arguments({});
        auto base = ctx.argument<base_handle>(0);
        return base->description();
      });

  {
    auto result = render(
        "{{ (describe mid) }}\n"
        "{{ (describe derived) }}\n",
        w::map({
            {"mid",
             w::native_handle(
                 manage_owned<RenderTestPolyMid>(), nullptr /* prototype */)},
            {"derived",
             w::native_handle(
                 manage_owned<RenderTestPolyDerived>(),
                 nullptr /* prototype */)},
            {"describe", w::native_function(describe)},
        }));
    EXPECT_THAT(diagnostics(), testing::IsEmpty());
    EXPECT_EQ(
        *result,
        "Mid\n"
        "Derived\n");
  }

  {
    auto result = render(
        "{{ (describe wrong_native_instance) }}\n",
        w::map({
            {"wrong_native_instance",
             w::native_handle(
                 manage_owned<RenderTestUnknownCppType>(),
                 nullptr /* prototype */)},
            {"describe", w::native_function(describe)},
        }));
    EXPECT_FALSE(result.has_value());
    EXPECT_THAT(
        diagnostics(),
        testing::ElementsAre(diagnostic(
            diagnostic_level::error,
            "Function 'describe' threw an error:\n"
            "Expected type of argument at index 0 to be `<native_handle type='whisker::RenderTestPolyBase'>` (polymorphic), but found `<native_handle type='whisker::RenderTestUnknownCppType'>`.",
            path_to_file,
            1)));
  }

  {
    // Although the type is a subclass, it was not listed as an alternative in
    // describe(...)
    auto result = render(
        "{{ (describe alternate) }}\n",
        w::map({
            {"alternate",
             w::native_handle(
                 manage_owned<RenderTestPolyAlternate>(),
                 nullptr /* prototype */)},
            {"describe", w::native_function(describe)},
        }));
    EXPECT_FALSE(result.has_value());
    EXPECT_THAT(
        diagnostics(),
        testing::ElementsAre(diagnostic(
            diagnostic_level::error,
            "Function 'describe' threw an error:\n"
            "Expected type of argument at index 0 to be `<native_handle type='whisker::RenderTestPolyBase'>` (polymorphic), but found `<native_handle type='whisker::RenderTestPolyAlternate'>`.",
            path_to_file,
            1)));
  }
}

TEST_F(RenderTest, user_defined_function_self_argument) {
  use_library(load_standard_library);

  const auto return_self =
      dsl::make_function([](dsl::function::context ctx) -> object {
        ctx.declare_arity(0);
        ctx.declare_named_arguments({});
        return ctx.raw().self();
      });

  const auto self_object = w::map({
      {"answer", w::i64(42)},
      {"return_self", w::native_function(return_self)},
  });
  const auto context = w::map({
      {"foo", self_object},
      {"answer", w::i64(100)},
  });

  {
    auto result = render(
        "{{#let s = (foo.return_self)}}\n"
        "{{s.answer}}\n",
        context);
    EXPECT_THAT(diagnostics(), testing::IsEmpty());
    EXPECT_EQ(*result, "42\n");
  }

  {
    auto result = render(
        "{{#let func = foo.return_self}}\n"
        "{{#let s = (func)}}\n"
        "{{#if (object.eq? s null)}}\n"
        "It was null!\n"
        "{{#else}}\n"
        "It was not null!\n"
        "{{/if (object.eq? s null)}}\n",
        context);
    EXPECT_THAT(diagnostics(), testing::IsEmpty());
    EXPECT_EQ(*result, "It was null!\n");
  }
}

TEST_F(RenderTest, user_defined_function_native_ref_prototype) {
  class SomeCppObjectBase {
   public:
    virtual ~SomeCppObjectBase() = default;
    int return42() const { return 42; }
  };

  class SomeCppObject : public SomeCppObjectBase {
   public:
    void increment() const { ++value_; }

    int get() const { return value_; }

   private:
    mutable int value_ = 0;
  };
  SomeCppObject cpp_object;

  using base_handle_type =
      dsl::polymorphic_native_handle<SomeCppObjectBase, SomeCppObject>;
  const auto base_proto = dsl::make_prototype<base_handle_type>([](auto&& def) {
    def.property("return42", [](const SomeCppObjectBase& self) {
      return w::i64(self.return42());
    });
  });

  const auto proto = dsl::make_prototype<native_handle<SomeCppObject>>(
      base_proto, [](auto&& def) {
        def.property("get", [](const SomeCppObject& self) {
          return w::i64(self.get());
        });
        def.function(
            "increment",
            [](const SomeCppObject& self, dsl::function::context ctx) {
              ctx.declare_arity(0);
              ctx.declare_named_arguments({});
              self.increment();
              return w::string("incremented!");
            });
      });

  const auto context = w::map({
      {"foo", w::native_handle(manage_as_static(cpp_object), proto)},
  });
  auto result = render(
      "{{ foo.get }}\n"
      "{{ (foo.increment) }}\n"
      "{{ foo.get }}\n"
      "{{ (foo.increment) }}\n"
      "{{ foo.get }}\n"
      "{{ foo.return42 }}\n",
      context);
  EXPECT_THAT(diagnostics(), testing::IsEmpty());
  EXPECT_EQ(
      *result,
      "0\n"
      "incremented!\n"
      "1\n"
      "incremented!\n"
      "2\n"
      "42\n");
}

TEST_F(RenderTest, let_statement) {
  auto result = render(
      "{{#let cond = (not false)}}\n"
      "{{#if cond}}\n"
      "  {{#let cond = some_text}}\n"
      "  {{cond}}\n"
      "{{/if cond}}\n"
      "{{#if cond}}\n"
      "  Outer scope was not overwritten!\n"
      "{{/if cond}}\n",
      w::map({{"some_text", w::string("some text")}}));
  EXPECT_THAT(diagnostics(), testing::IsEmpty());
  EXPECT_EQ(
      *result,
      "  some text\n"
      "  Outer scope was not overwritten!\n");
}

TEST_F(RenderTest, let_statement_loop) {
  auto result = render(
      "{{#array}}\n"
      "  {{#let element = .}}\n"
      "  {{element.value}}\n"
      "{{/array}}\n",
      w::map(
          {{"array",
            w::array({
                w::map({{"value", w::i64(2)}}),
                w::map({{"value", w::string("foo")}}),
                w::map({{"value", w::string("bar")}}),
            })}}));
  EXPECT_THAT(diagnostics(), testing::IsEmpty());
  EXPECT_EQ(
      *result,
      "  2\n"
      "  foo\n"
      "  bar\n");
}

TEST_F(RenderTest, let_statement_rebinding_error) {
  auto result = render(
      "{{#let cond = (not false)}}\n"
      "{{#let cond = false}}\n",
      w::map());
  EXPECT_FALSE(result.has_value());
  EXPECT_THAT(
      diagnostics(),
      testing::ElementsAre(diagnostic(
          diagnostic_level::error,
          "Name 'cond' is already bound in the current scope.",
          path_to_file,
          2)));
}

TEST_F(RenderTest, with_block) {
  auto result = render(
      "{{#with news}}\n"
      "  {{#if has-update?}}\n"
      "    Stuff is {{foo}} happening!\n"
      "  {{/if has-update?}}\n"
      "{{/with}}\n",
      w::map(
          {{"news",
            w::map({{"has-update?", w::true_}, {"foo", w::string("now")}})}}));
  EXPECT_THAT(diagnostics(), testing::IsEmpty());
  EXPECT_EQ(*result, "    Stuff is now happening!\n");
}

TEST_F(RenderTest, with_not_map) {
  auto result = render(
      "{{#with news}}\n"
      "{{/with}}\n",
      w::map({{"news", w::array({w::i64(0)})}}));
  EXPECT_FALSE(result.has_value());
  EXPECT_THAT(
      diagnostics(),
      testing::ElementsAre(diagnostic(
          diagnostic_level::error,
          "Expression 'news' does not evaluate to a map. The encountered value is:\n"
          "array (size=1)\n"
          "╰─ [0] i64(0)\n",
          path_to_file,
          1)));
}

TEST_F(RenderTest, with_custom_map) {
  auto result = render(
      "{{#with doubler}}\n"
      "{{foo}} {{bar}}\n"
      "{{#with .}}\n"
      "{{baz}}\n"
      "{{/with}}\n"
      "{{/with}}\n",
      w::map({{"doubler", w::make_map<double_property_name>()}}));
  EXPECT_THAT(diagnostics(), testing::IsEmpty());
  EXPECT_EQ(
      *result,
      "foofoo barbar\n"
      "bazbaz\n");
}

TEST_F(RenderTest, each_block_array) {
  auto result = render(
      "The factorial function looks like:\n"
      "{{#each factorials}}\n"
      "{{.}}\n"
      "{{/each}}",
      w::map(
          {{"factorials",
            w::array(
                {w::i64(1), w::i64(2), w::i64(6), w::i64(24), w::i64(120)})}}));
  EXPECT_EQ(
      *result,
      "The factorial function looks like:\n"
      "1\n"
      "2\n"
      "6\n"
      "24\n"
      "120\n");
}

TEST_F(RenderTest, each_block_array_with_capture) {
  auto result = render(
      "The factorial function looks like:\n"
      "{{#each factorials as |value|}}\n"
      "{{value}}\n"
      "{{/each}}",
      w::map(
          {{"factorials",
            w::array(
                {w::i64(1), w::i64(2), w::i64(6), w::i64(24), w::i64(120)})}}));
  EXPECT_EQ(
      *result,
      "The factorial function looks like:\n"
      "1\n"
      "2\n"
      "6\n"
      "24\n"
      "120\n");
}

TEST_F(RenderTest, each_block_array_with_capture_destructuring) {
  use_library(load_standard_library);

  auto result = render(
      "The factorial function looks like:\n"
      "{{#each (array.enumerate factorials) as |index value|}}\n"
      "{{index}}. {{value}}\n"
      "{{/each}}",
      w::map(
          {{"factorials",
            w::array(
                {w::i64(1), w::i64(2), w::i64(6), w::i64(24), w::i64(120)})}}));
  EXPECT_EQ(
      *result,
      "The factorial function looks like:\n"
      "0. 1\n"
      "1. 2\n"
      "2. 6\n"
      "3. 24\n"
      "4. 120\n");
}

TEST_F(RenderTest, each_block_array_without_capture_destructuring) {
  // Since there is one capture only, the item should not be destructured.
  auto result = render(
      "The factorial function looks like:\n"
      "{{#each . as |factorials|}}\n"
      "{{#each factorials as |value|}}\n"
      "{{value}}\n"
      "{{/each}}\n"
      "{{/each}}",
      w::array({w::array(
          {w::i64(1), w::i64(2), w::i64(6), w::i64(24), w::i64(120)})}));
  EXPECT_EQ(
      *result,
      "The factorial function looks like:\n"
      "1\n"
      "2\n"
      "6\n"
      "24\n"
      "120\n");
}

TEST_F(RenderTest, each_block_array_iterable_custom_array) {
  use_library(load_standard_library);

  auto factorials = w::map(
      {{"factorials",
        custom_array::make(
            {w::map({{"value", w::i64(1)}}),
             w::map({{"value", w::string("2")}}),
             w::map({{"value", w::i64(6)}}),
             w::map({{"value", w::i64(24)}}),
             w::map({{"value", w::i64(120)}})})}});
  auto result = render(
      "The factorial function looks like:\n"
      "{{#each (array.enumerate factorials) as |index entry|}}\n"
      "{{index}}. {{entry.value}}\n"
      "{{/each}}",
      factorials);
  EXPECT_EQ(
      *result,
      "The factorial function looks like:\n"
      "0. 1\n"
      "1. 2\n"
      "2. 6\n"
      "3. 24\n"
      "4. 120\n");
}

TEST_F(RenderTest, each_block_array_else) {
  auto result = render(
      "The factorial function looks like:\n"
      "{{#each factorials as |value index|}}\n"
      "{{undefined-name}}\n"
      "{{#else}}\n"
      "Hey! Where did they go?\n"
      "{{/each}}",
      w::map({{"factorials", w::array()}}));
  EXPECT_EQ(
      *result,
      "The factorial function looks like:\n"
      "Hey! Where did they go?\n");
}

TEST_F(RenderTest, each_block_array_nested) {
  use_library(load_standard_library);

  auto result = render(
      "{{#each (array.enumerate this) as |i e|}}\n"
      "{{i}}({{#each (array.enumerate e.b) as |j e|}} {{i}}-{{j}}({{e.c}}) {{/each}})\n"
      "{{/each}}",
      w::array(
          {w::map(
               {{"b",
                 w::array(
                     {w::map({{"c", w::i64(1)}}),
                      w::map({{"c", w::i64(2)}})})}}),
           w::map(
               {{"b",
                 w::array(
                     {w::map({{"c", w::i64(3)}}),
                      w::map({{"c", w::i64(4)}})})}}),
           w::map(
               {{"b",
                 w::array(
                     {w::map({{"c", w::i64(5)}}),
                      w::map({{"c", w::i64(6)}})})}})}));

  EXPECT_EQ(
      *result,
      "0( 0-0(1)  0-1(2) )\n"
      "1( 1-0(3)  1-1(4) )\n"
      "2( 2-0(5)  2-1(6) )\n");
}

TEST_F(RenderTest, each_block_non_array) {
  auto context = w::map({{"number", w::i64(2)}});

  auto result = render(
      "{{#each number}}\n"
      "Should not be rendered\n"
      "{{/each}}",
      context);
  EXPECT_FALSE(result.has_value());
  EXPECT_THAT(
      diagnostics(),
      testing::ElementsAre(diagnostic(
          diagnostic_level::error,
          "Expression 'number' does not evaluate to an array. The encountered value is:\n"
          "i64(2)\n",
          path_to_file,
          1)));
}

TEST_F(RenderTest, each_block_too_many_captures) {
  auto result = render(
      "{{#each . as |value extra_capture|}}\n"
      "{{value}} {{extra_capture}}\n"
      "{{/each}}",
      w::array({w::array({w::string("foo")})}));
  EXPECT_FALSE(result.has_value());
  EXPECT_THAT(
      diagnostics(),
      testing::ElementsAre(diagnostic(
          diagnostic_level::error,
          "Item at index 0 does not evaluate to an array with 2 elements (to capture). "
          "The encountered value is:\n"
          "array (size=1)\n"
          "╰─ [0] 'foo'\n",
          path_to_file,
          1)));
}

TEST_F(RenderTest, each_block_not_enough_captures) {
  auto result = render(
      "{{#each . as |cap1 cap2|}}\n"
      "{{value}}\n"
      "{{/each}}",
      w::array(
          {w::array({w::string("foo"), w::string("bar"), w::string("baz")})}));
  EXPECT_FALSE(result.has_value());
  EXPECT_THAT(
      diagnostics(),
      testing::ElementsAre(diagnostic(
          diagnostic_level::error,
          "Item at index 0 does not evaluate to an array with 2 elements (to capture). "
          "The encountered value is:\n"
          "array (size=3)\n"
          "├─ [0] 'foo'\n"
          "├─ [1] 'bar'\n"
          "╰─ [2] 'baz'\n",
          path_to_file,
          1)));
}

TEST_F(RenderTest, printable_types_strict_failure) {
  {
    auto result = render(R"({{-42}} {{"hello"}})", w::map());
    EXPECT_EQ(*result, "-42 hello");
  }
  {
    auto result = render("{{f64}}", w::map({{"f64", w::f64(3.1415926)}}));
    EXPECT_THAT(
        diagnostics(),
        testing::ElementsAre(diagnostic(
            diagnostic_level::error,
            "Object named 'f64' is not printable. The encountered value is:\n"
            "f64(3.1415926)\n",
            path_to_file,
            1)));
    EXPECT_FALSE(result.has_value());
  }
  {
    auto result = render("{{true}}", w::map());
    EXPECT_THAT(
        diagnostics(),
        testing::ElementsAre(diagnostic(
            diagnostic_level::error,
            "Object named 'true' is not printable. The encountered value is:\n"
            "true\n",
            path_to_file,
            1)));
    EXPECT_FALSE(result.has_value());
  }
  {
    auto result = render("{{null}}", w::map());
    EXPECT_THAT(
        diagnostics(),
        testing::ElementsAre(diagnostic(
            diagnostic_level::error,
            "Object named 'null' is not printable. The encountered value is:\n"
            "null\n",
            path_to_file,
            1)));
    EXPECT_FALSE(result.has_value());
  }
  {
    auto result = render("{{array}}", w::map({{"array", w::array()}}));
    EXPECT_THAT(
        diagnostics(),
        testing::ElementsAre(diagnostic(
            diagnostic_level::error,
            "Object named 'array' is not printable. The encountered value is:\n"
            "array (size=0)\n",
            path_to_file,
            1)));
    EXPECT_FALSE(result.has_value());
  }
  {
    auto result = render("{{map}}", w::map({{"map", w::map()}}));
    EXPECT_THAT(
        diagnostics(),
        testing::ElementsAre(diagnostic(
            diagnostic_level::error,
            "Object named 'map' is not printable. The encountered value is:\n"
            "map (size=0)\n",
            path_to_file,
            1)));
    EXPECT_FALSE(result.has_value());
  }
}

TEST_F(RenderTest, printable_types_warning) {
  strict_printable_types(diagnostic_level::warning);

  {
    auto result = render(R"({{-42}} {{"hello"}})", w::map());
    EXPECT_EQ(*result, "-42 hello");
  }
  {
    auto result = render("{{f64}}", w::map({{"f64", w::f64(3.1415926)}}));
    EXPECT_THAT(
        diagnostics(),
        testing::ElementsAre(diagnostic(
            diagnostic_level::warning,
            "Object named 'f64' is not printable. The encountered value is:\n"
            "f64(3.1415926)\n",
            path_to_file,
            1)));
    EXPECT_EQ(*result, "3.1415926");
  }
  {
    auto result = render("{{false}}", w::map());
    EXPECT_THAT(
        diagnostics(),
        testing::ElementsAre(diagnostic(
            diagnostic_level::warning,
            "Object named 'false' is not printable. The encountered value is:\n"
            "false\n",
            path_to_file,
            1)));
    EXPECT_EQ(*result, "false");
  }
  {
    auto result = render("{{null}}", w::map());
    EXPECT_THAT(
        diagnostics(),
        testing::ElementsAre(diagnostic(
            diagnostic_level::warning,
            "Object named 'null' is not printable. The encountered value is:\n"
            "null\n",
            path_to_file,
            1)));
    EXPECT_EQ(*result, "");
  }

  // Arrays, maps are not printable in any case
  {
    auto result = render("{{array}}", w::map({{"array", w::array()}}));
    EXPECT_THAT(
        diagnostics(),
        testing::ElementsAre(diagnostic(
            diagnostic_level::error,
            "Object named 'array' is not printable. The encountered value is:\n"
            "array (size=0)\n",
            path_to_file,
            1)));
    EXPECT_FALSE(result.has_value());
  }
  {
    auto result = render("{{map}}", w::map({{"map", w::map()}}));
    EXPECT_THAT(
        diagnostics(),
        testing::ElementsAre(diagnostic(
            diagnostic_level::error,
            "Object named 'map' is not printable. The encountered value is:\n"
            "map (size=0)\n",
            path_to_file,
            1)));
    EXPECT_FALSE(result.has_value());
  }
}

TEST_F(RenderTest, printable_types_allowed) {
  strict_printable_types(diagnostic_level::info);

  {
    auto result = render(
        "{{i64}} {{string}}",
        w::map({{"i64", w::i64(-42)}, {"string", w::string("hello")}}));
    EXPECT_EQ(*result, "-42 hello");
  }
  {
    auto result = render("{{f64}}", w::map({{"f64", w::f64(3.1415926)}}));
    EXPECT_THAT(diagnostics(), testing::IsEmpty());
    EXPECT_EQ(*result, "3.1415926");
  }
  {
    auto result = render("{{bool}}", w::map({{"bool", w::true_}}));
    EXPECT_THAT(diagnostics(), testing::IsEmpty());
    EXPECT_EQ(*result, "true");
  }
  {
    auto result = render("{{null_}}", w::map({{"null_", w::null}}));
    EXPECT_THAT(diagnostics(), testing::IsEmpty());
    EXPECT_EQ(*result, "");
  }

  // Arrays, maps are not printable in any case
  {
    auto result = render("{{array}}", w::map({{"array", w::array()}}));
    EXPECT_THAT(
        diagnostics(),
        testing::ElementsAre(diagnostic(
            diagnostic_level::error,
            "Object named 'array' is not printable. The encountered value is:\n"
            "array (size=0)\n",
            path_to_file,
            1)));
    EXPECT_FALSE(result.has_value());
  }
  {
    auto result = render("{{map}}", w::map({{"map", w::map()}}));
    EXPECT_THAT(
        diagnostics(),
        testing::ElementsAre(diagnostic(
            diagnostic_level::error,
            "Object named 'map' is not printable. The encountered value is:\n"
            "map (size=0)\n",
            path_to_file,
            1)));
    EXPECT_FALSE(result.has_value());
  }
}

TEST_F(RenderTest, undefined_variables_allowed) {
  strict_undefined_variables(diagnostic_level::info);
  strict_printable_types(diagnostic_level::info);

  auto result = render(
      "{{foo}}\n"
      "{{foo.bar}}\n"
      "{{abc}}\n"
      "{{abc.xyz}}\n",
      w::map({{"abc", w::string("xyz")}}));
  EXPECT_THAT(diagnostics(), testing::IsEmpty());
  EXPECT_EQ(
      *result,
      "\n"
      "\n"
      "xyz\n"
      "\n");
}

TEST_F(RenderTest, partials) {
  auto result = render(
      "{{#let partial foo}}\n"
      "  indented\n"
      "{{/let partial}}\n"
      "  {{#partial foo}}\n"
      "{{#let bar = foo}}\n"
      "{{#partial bar}}\n",
      w::map());
  EXPECT_THAT(diagnostics(), testing::IsEmpty());
  EXPECT_EQ(
      *result,
      "    indented\n"
      "  indented\n");
}

TEST_F(RenderTest, partials_with_arguments) {
  auto result = render(
      "{{#let partial println |txt|}}\n"
      "{{txt}}\n"
      "{{/let partial}}\n"
      "{{#partial println txt=\"hello\"}}\n"
      "{{#let another-name = println}}\n"
      "{{#partial another-name txt=5}}\n",
      w::map());
  EXPECT_THAT(diagnostics(), testing::IsEmpty());
  EXPECT_EQ(
      *result,
      "hello\n"
      "5\n");
}

TEST_F(RenderTest, partials_with_arguments_out_of_order) {
  auto result = render(
      "{{#let partial foo |arg1 arg2|}}\n"
      "{{arg1}} {{arg2}}\n"
      "{{/let partial}}\n"
      "{{#partial foo arg2=2 arg1=1}}\n",
      w::map());
  EXPECT_THAT(diagnostics(), testing::IsEmpty());
  EXPECT_EQ(*result, "1 2\n");
}

TEST_F(RenderTest, partials_with_missing_arguments) {
  show_source_backtrace_on_failure(true);

  auto result = render(
      "{{#let partial foo |arg1 arg2|}}\n"
      "{{arg1}} {{arg2}}\n"
      "{{/let partial}}\n"
      "{{#partial foo arg1=\"hello\"}}\n",
      w::map());
  EXPECT_THAT(
      diagnostics(),
      testing::ElementsAre(
          diagnostic(
              diagnostic_level::error,
              "Partial 'foo' is missing named arguments: arg2",
              path_to_file,
              4),
          error_backtrace("#0 path/to/test.whisker <line:4, col:1>\n")));
  EXPECT_FALSE(result.has_value());
}

TEST_F(RenderTest, partials_with_extra_arguments) {
  show_source_backtrace_on_failure(true);

  auto result = render(
      "{{#let partial foo |arg1|}}\n"
      "{{arg1}}\n"
      "{{/let partial}}\n"
      "{{#partial foo arg1=\"hello\" arg2=null}}\n",
      w::map());
  EXPECT_THAT(
      diagnostics(),
      testing::ElementsAre(
          diagnostic(
              diagnostic_level::error,
              "Partial 'foo' received unexpected named arguments: arg2",
              path_to_file,
              4),
          error_backtrace("#0 path/to/test.whisker <line:4, col:1>\n")));
  EXPECT_FALSE(result.has_value());
}

TEST_F(RenderTest, partials_with_wrong_type) {
  show_source_backtrace_on_failure(true);

  auto result = render("{{#partial true}}\n", w::map());
  EXPECT_THAT(
      diagnostics(),
      testing::ElementsAre(
          diagnostic(
              diagnostic_level::error,
              "Expression 'true' does not evaluate to a partial. The encountered value is:\n"
              "true\n",
              path_to_file,
              1),
          error_backtrace("#0 path/to/test.whisker <line:1, col:12>\n")));
  EXPECT_FALSE(result.has_value());
}

TEST_F(RenderTest, partials_nested) {
  auto result = render(
      "{{#let partial foo |arg|}}\n"
      "  {{#let partial bar |arg|}}\n"
      "    {{arg}}\n"
      "  {{/let partial}}\n"
      "  {{#partial bar arg=arg}}\n"
      "{{/let partial}}\n"
      "{{#partial foo arg=\"hello\"}}\n",
      w::map());
  EXPECT_THAT(diagnostics(), testing::IsEmpty());
  EXPECT_EQ(*result, "      hello\n");
}

TEST_F(RenderTest, partials_recursive) {
  use_library(load_standard_library);

  const auto is_even =
      dsl::make_function([](dsl::function::context ctx) -> boolean {
        ctx.declare_arity(1);
        ctx.declare_named_arguments({});
        auto n = ctx.argument<i64>(0);
        return n % 2 == 0;
      });
  const auto mul = dsl::make_function([](dsl::function::context ctx) -> i64 {
    ctx.declare_arity(2);
    ctx.declare_named_arguments({});
    auto lhs = ctx.argument<i64>(0);
    auto rhs = ctx.argument<i64>(1);
    return lhs * rhs;
  });
  const auto div = dsl::make_function([](dsl::function::context ctx) -> i64 {
    ctx.declare_arity(2);
    ctx.declare_named_arguments({});
    auto numerator = ctx.argument<i64>(0);
    auto denominator = ctx.argument<i64>(1);
    assert(numerator % denominator == 0);
    return numerator / denominator;
  });

  // https://en.wikipedia.org/wiki/Collatz_conjecture
  auto result = render(
      "{{#let partial collatz |n|}}\n"
      "{{n}}\n"
      "  {{#if (int.ne? n 1)}}\n"
      "    {{#if (even? n)}}\n"
      "{{#partial collatz n=(div n 2)}}\n"
      "    {{#else}}\n"
      "{{#partial collatz n=(int.add (mul 3 n) 1)}}\n"
      "    {{/if (even? n)}}\n"
      "  {{/if (int.ne? n 1)}}\n"
      "{{/let partial}}\n"
      "{{#partial collatz n=6}}\n",
      w::map(),
      sources({}),
      globals({
          {"even?", w::native_function(is_even)},
          {"mul", w::native_function(mul)},
          {"div", w::native_function(div)},
      }));
  EXPECT_THAT(diagnostics(), testing::IsEmpty());
  EXPECT_EQ(
      *result,
      "6\n"
      "3\n"
      "10\n"
      "5\n"
      "16\n"
      "8\n"
      "4\n"
      "2\n"
      "1\n");
}

TEST_F(RenderTest, partials_mutually_recursive) {
  use_library(load_standard_library);

  auto result = render(
      "{{#let partial even-helper |n odd|}}\n"
      "even: {{n}}\n"
      "{{#if (int.gt? n 1)}}\n"
      "{{#partial odd n=(int.sub n 1)}}\n"
      "{{/if (int.gt? n 1)}}\n"
      "{{/let partial}}\n"
      ""
      "{{#let partial odd |n| captures |even-helper|}}\n"
      "odd: {{n}}\n"
      "{{#partial even-helper n=(int.sub n 1) odd=odd}}\n"
      "{{/let partial}}\n"
      ""
      "{{#let partial even |n| captures |even-helper odd|}}\n"
      "{{#partial even-helper n=n odd=odd}}\n"
      "{{/let partial}}\n"
      ""
      "{{#partial even n=6}}\n"
      "{{#partial odd n=5}}\n",
      w::map());
  EXPECT_THAT(diagnostics(), testing::IsEmpty());
  EXPECT_EQ(
      *result,
      "even: 6\n"
      "odd: 5\n"
      "even: 4\n"
      "odd: 3\n"
      "even: 2\n"
      "odd: 1\n"
      "even: 0\n"
      "odd: 5\n"
      "even: 4\n"
      "odd: 3\n"
      "even: 2\n"
      "odd: 1\n"
      "even: 0\n");
}

TEST_F(RenderTest, partial_derived_context) {
  show_source_backtrace_on_failure(true);

  auto result = render(
      "{{#let x = 1}}\n"
      "{{#let partial foo}}\n"
      "{{x}}\n"
      "{{/let partial}}\n"
      "{{#partial foo}}\n",
      w::map(),
      sources({}),
      globals({{"global", w::i64(42)}}));
  EXPECT_THAT(
      diagnostics(),
      testing::ElementsAre(
          diagnostic(
              diagnostic_level::error,
              "Name 'x' was not found in the current scope.\n"
              "Tried to search through the following scopes:\n"
              "#0 <global scope> (size=1)\n"
              "╰─ 'global' → i64(42)\n",
              path_to_file,
              3),
          error_backtrace("#0 foo @ path/to/test.whisker <line:3, col:3>\n"
                          "#1 path/to/test.whisker <line:5, col:1>\n")));
  EXPECT_FALSE(result.has_value());
}

TEST_F(RenderTest, partial_derived_context_no_leak) {
  show_source_backtrace_on_failure(true);

  auto result = render(
      "{{#let partial foo}}\n"
      "{{#let x = 1}}\n"
      "{{x}}\n"
      "{{/let partial}}\n"
      "{{#partial foo}}\n"
      "{{x}}\n",
      w::map(),
      sources({}),
      globals({{"global", w::i64(42)}}));
  EXPECT_THAT(
      diagnostics(),
      testing::ElementsAre(
          diagnostic(
              diagnostic_level::error,
              "Name 'x' was not found in the current scope.\n"
              "Tried to search through the following scopes:\n"
              "#0 map (size=0)\n"
              "\n"
              "#1 <global scope> (size=1)\n"
              "╰─ 'global' → i64(42)\n",
              path_to_file,
              6),
          error_backtrace("#0 path/to/test.whisker <line:6, col:3>\n")));
  EXPECT_FALSE(result.has_value());
}

TEST_F(RenderTest, partial_nested_backtrace) {
  show_source_backtrace_on_failure(true);

  auto result = render(
      "{{#let partial foo}}\n"
      "  {{#let partial bar}}\n"
      "    {{#let partial baz}}\n"
      "      {{undefined}}\n"
      "    {{/let partial}}\n"
      "    {{#partial baz}}\n"
      "  {{/let partial}}\n"
      "  {{#partial bar}}\n"
      "{{/let partial}}\n"
      "{{#partial foo}}\n",
      w::map());
  EXPECT_THAT(
      diagnostics(),
      testing::ElementsAre(
          diagnostic(
              diagnostic_level::error,
              "Name 'undefined' was not found in the current scope.\n"
              "Tried to search through the following scopes:\n"
              "#0 <global scope> (size=0)\n",
              path_to_file,
              4),
          error_backtrace("#0 baz @ path/to/test.whisker <line:4, col:9>\n"
                          "#1 bar @ path/to/test.whisker <line:6, col:5>\n"
                          "#2 foo @ path/to/test.whisker <line:8, col:3>\n"
                          "#3 path/to/test.whisker <line:10, col:1>\n")));
  EXPECT_FALSE(result.has_value());
}

TEST_F(RenderTest, partials_capture_error) {
  show_source_backtrace_on_failure(true);

  auto result = render(
      "{{#let partial foo captures |bar|}}\n"
      "{{/let partial}}\n",
      w::map());
  EXPECT_THAT(
      diagnostics(),
      testing::ElementsAre(
          diagnostic(
              diagnostic_level::error,
              "Name 'bar' was not found in the current scope.\n"
              "Tried to search through the following scopes:\n"
              "#0 map (size=0)\n"
              "\n"
              "#1 <global scope> (size=0)\n",
              path_to_file,
              1),
          error_backtrace("#0 path/to/test.whisker <line:1, col:30>\n")));
}

TEST_F(RenderTest, macros) {
  auto result = render(
      "{{> some/file /path}}\n"
      "{{#array}}\n"
      "{{>some / file/path }}\n"
      "{{/array}}",
      w::map(
          {{"value", w::i64(1)},
           {"array",
            w::array(
                {w::map({{"value", w::i64(2)}}),
                 w::map({{"value", w::string("foo")}}),
                 w::map({{"value", w::string("bar")}})})}}),
      sources({{"some/file/path", "{{value}} (from macro)\n"}}));
  EXPECT_EQ(
      *result,
      "1 (from macro)\n"
      "2 (from macro)\n"
      "foo (from macro)\n"
      "bar (from macro)\n");
}

TEST_F(RenderTest, macros_missing) {
  auto result = render(
      "{{> some/other/path}}",
      w::map(),
      sources({{"some/file/path", "{{value}} (from macro)"}}));
  EXPECT_THAT(
      diagnostics(),
      testing::ElementsAre(diagnostic(
          diagnostic_level::error,
          "Macro with path 'some/other/path' was not found",
          path_to_file,
          1)));
  EXPECT_FALSE(result.has_value());
}

TEST_F(RenderTest, macros_parse_error) {
  auto result = render(
      "{{> some/file/path}}",
      w::map(),
      sources({
          {"some/file/path", "{{#INVALID_CODE"},
      }));
  EXPECT_THAT(
      diagnostics(),
      testing::ElementsAre(
          diagnostic(
              diagnostic_level::error,
              "expected `}}` to open section-block but found EOF",
              "some/file/path",
              1),
          diagnostic(
              diagnostic_level::error,
              "Macro with path 'some/file/path' failed to parse",
              path_to_file,
              1)));
  EXPECT_FALSE(result.has_value());
}

TEST_F(RenderTest, macros_no_resolver) {
  auto result = render("{{> some/file/path}}", w::map());
  EXPECT_THAT(
      diagnostics(),
      testing::ElementsAre(diagnostic(
          diagnostic_level::error,
          "No source resolver was provided. Cannot resolve macro with path 'some/file/path'",
          path_to_file,
          1)));
  EXPECT_FALSE(result.has_value());
}

TEST_F(RenderTest, macros_indentation) {
  auto result = render(
      "{{#array}}\n"
      "before\n"
      "  {{>some / file/path }}\n"
      "after\n"
      "{{/array}}",
      w::map(
          {{"array",
            w::array(
                {w::map({{"value", w::i64(2)}}),
                 w::map({{"value", w::string("foo")}}),
                 w::map({{"value", w::string("bar")}})})}}),
      sources(
          {{"some/file/path",
            "{{value}} (from macro)\n"
            "A second line\n"}}));
  EXPECT_EQ(
      *result,
      "before\n"
      "  2 (from macro)\n"
      "  A second line\n"
      "after\n"
      "before\n"
      "  foo (from macro)\n"
      "  A second line\n"
      "after\n"
      "before\n"
      "  bar (from macro)\n"
      "  A second line\n"
      "after\n");
}

TEST_F(RenderTest, macros_indentation_nested) {
  auto result = render(
      "{{#array}}\n"
      "before\n"
      "  {{>macro-1 }}\n"
      "after\n"
      "{{/array}}",
      w::map(
          {{"array",
            w::array({
                w::map({{"value", w::i64(2)}}),
                w::map({{"value", w::string("foo")}}),
            })}}),
      sources(
          {{"macro-1",
            "{{value}} (from macro-1)\n"
            "  nested: {{> macro-2}}\n"},
           {"macro-2",
            "{{value}} (from macro-2)\n"
            "second line from macro-2"}}));
  EXPECT_EQ(
      *result,
      "before\n"
      "  2 (from macro-1)\n"
      "    nested: 2 (from macro-2)\n"
      "  second line from macro-2\n"
      "after\n"
      "before\n"
      "  foo (from macro-1)\n"
      "    nested: foo (from macro-2)\n"
      "  second line from macro-2\n"
      "after\n");
}

TEST_F(RenderTest, macro_preserves_whitespace_indentation) {
  auto result = render(
      " \t {{> macro-1}} \t \n",
      w::map({{"value", w::i64(42)}}),
      sources(
          {{"macro-1",
            "\t 1\n"
            " \t {{>macro-2}} \n"},
           {"macro-2",
            "\t 2\n"
            " \t {{>macro-3}}! \n"},
           {"macro-3", "\t{{value}}"}}));
  EXPECT_EQ(
      *result,
      " \t \t 1\n"
      " \t  \t \t 2\n"
      " \t  \t  \t \t42! \n");
}

TEST_F(RenderTest, macro_nested_undefined_variable_trace) {
  show_source_backtrace_on_failure(true);
  auto result = render(
      "\n"
      "{{> macro-1}}\n",
      w::map(),
      sources(
          {{"macro-1",
            "\n"
            "\t {{>macro-2}}\n"},
           {"macro-2", "foo {{>macro-3}}\n"},
           {"macro-3", "{{undefined}}"}}));
  EXPECT_FALSE(result.has_value());
  EXPECT_THAT(
      diagnostics(),
      testing::ElementsAre(
          diagnostic(
              diagnostic_level::error,
              "Name 'undefined' was not found in the current scope.\n"
              "Tried to search through the following scopes:\n"
              "#0 map (size=0)\n"
              "\n"
              "#1 <global scope> (size=0)\n",
              "macro-3", // file name
              1),
          error_backtrace("#0 <macro> @ macro-3 <line:1, col:3>\n"
                          "#1 <macro> @ macro-2 <line:1, col:5>\n"
                          "#2 <macro> @ macro-1 <line:2, col:3>\n"
                          "#3 path/to/test.whisker <line:2, col:1>\n")));
}

TEST_F(RenderTest, imports) {
  auto result = render(
      "{{#import \"some/file/path\" as lib}}\n"
      "The answer is {{lib.answer}}\n",
      w::map(),
      sources({{"some/file/path", "{{#let export answer = 42}}\n"}}));
  EXPECT_THAT(diagnostics(), testing::IsEmpty());
  EXPECT_EQ(*result, "The answer is 42\n");
}

TEST_F(RenderTest, imports_partial) {
  auto result = render(
      "{{#import \"some/file/path\" as lib}}\n"
      "The answer is {{#partial lib.echo thing=42}}\n",
      w::map(),
      sources(
          {{"some/file/path",
            "{{#let export partial echo |thing|}}\n"
            "{{#pragma ignore-newlines}}\n"
            "{{thing}}\n"
            "{{/let partial}}\n"}}));
  EXPECT_THAT(diagnostics(), testing::IsEmpty());
  EXPECT_EQ(*result, "The answer is 42\n");
}

TEST_F(RenderTest, imports_missing) {
  auto result = render(
      "{{#import \"some/other/path\" as lib}}\"}}\n"
      "The answer is {{lib.answer}}\n",
      w::map(),
      sources({{"some/file/path", "{{#let export answer = 42}}\n"}}));
  EXPECT_THAT(
      diagnostics(),
      testing::ElementsAre(diagnostic(
          diagnostic_level::error,
          "Module with path 'some/other/path' was not found",
          path_to_file,
          1)));
  EXPECT_FALSE(result.has_value());
}

TEST_F(RenderTest, imports_parse_error) {
  auto result = render(
      "{{#import \"some/file/path\" as lib}}\"}}\n"
      "The answer is {{lib.answer}}\n",
      w::map(),
      sources({
          {"some/file/path", "{{#INVALID_CODE"},
      }));
  EXPECT_THAT(
      diagnostics(),
      testing::ElementsAre(
          diagnostic(
              diagnostic_level::error,
              "expected `}}` to open section-block but found EOF",
              "some/file/path",
              1),
          diagnostic(
              diagnostic_level::error,
              "Module with path 'some/file/path' failed to parse",
              path_to_file,
              1)));
  EXPECT_FALSE(result.has_value());
}

TEST_F(RenderTest, imports_no_resolver) {
  auto result = render("{{#import \"some/file/path\" as lib}}\n", w::map());
  EXPECT_THAT(
      diagnostics(),
      testing::ElementsAre(diagnostic(
          diagnostic_level::error,
          "No source resolver was provided. Cannot resolve import with path 'some/file/path'",
          path_to_file,
          1)));
  EXPECT_FALSE(result.has_value());
}

TEST_F(RenderTest, imports_undefined) {
  auto result = render(
      "{{#import \"some/file/path\" as lib}}\n"
      "The answer is {{lib.oops}}\n",
      w::map(),
      sources({{"some/file/path", "{{#let export answer = 42}}\n"}}));
  EXPECT_THAT(
      diagnostics(),
      testing::ElementsAre(diagnostic(
          diagnostic_level::error,
          "Object 'lib' has no property named 'oops'.\n"
          "The object with the missing property is:\n"
          "map (size=1)\n"
          "╰─ 'answer' → i64(42)\n",
          path_to_file,
          2)));
  EXPECT_FALSE(result.has_value());
}

TEST_F(RenderTest, imports_multiple) {
  auto result = render(
      "{{#import \"module1\" as mod1}}\n"
      "{{#import \"module2\" as mod2}}\n"
      "Module 1 answer: {{mod1.answer}}\n"
      "Module 2 answer: {{mod2.answer}}\n",
      w::map(),
      sources({
          {"module1", "{{#let export answer = 42}}\n"},
          {"module2", "{{#let export answer = 84}}\n"},
      }));
  EXPECT_THAT(diagnostics(), testing::IsEmpty());
  EXPECT_EQ(
      *result,
      "Module 1 answer: 42\n"
      "Module 2 answer: 84\n");
}

TEST_F(RenderTest, imports_same_module_multiple) {
  auto result = render(
      "{{#import \"module1\" as mod1}}\n"
      "{{#import \"module1\" as mod2}}\n"
      "Module 1 answer: {{mod1.answer}}\n"
      "Module 2 answer: {{mod2.answer}}\n",
      w::map(),
      sources({
          {"module1", "{{#let export answer = 42}}\n"},
      }));
  EXPECT_THAT(diagnostics(), testing::IsEmpty());
  EXPECT_EQ(
      *result,
      "Module 1 answer: 42\n"
      "Module 2 answer: 42\n");
}

TEST_F(RenderTest, imports_empty_export) {
  auto result = render(
      "{{#import \"empty/module\" as lib}}\n"
      "This should render without errors even if the module is empty.\n",
      w::map(),
      sources({{"empty/module", " {{!-- empty module --}} \n"}}));
  EXPECT_THAT(diagnostics(), testing::IsEmpty());
  EXPECT_EQ(
      *result,
      "This should render without errors even if the module is empty.\n");
}

TEST_F(RenderTest, imports_with_multiple_exports) {
  // Test for importing a module with multiple export statements
  auto result = render(
      "{{#import \"module\" as mod}}\n"
      "Answer 1: {{mod.answer1}}\n"
      "Answer 2: {{mod.answer2}}\n",
      w::map(),
      sources({
          {"module",
           "{{#let export answer1 = 42}}\n"
           " {{!-- there are two answers here! --}}\n"
           "{{#let export answer2 = 84}}\n"},
      }));
  EXPECT_THAT(diagnostics(), testing::IsEmpty());
  EXPECT_EQ(
      *result,
      "Answer 1: 42\n"
      "Answer 2: 84\n");
}

TEST_F(RenderTest, imports_with_transitive_exports) {
  auto result = render(
      "{{#import \"module1\" as lib}}\n"
      "The answer is {{lib.answer}}\n",
      w::map(),
      sources({
          {"module1",
           "{{#import \"module2\" as lib}}\n"
           "{{#let export answer = lib.answer}}\n"},
          {"module2", "{{#let export answer = 42}}\n"},
      }));
  EXPECT_THAT(diagnostics(), testing::IsEmpty());
  EXPECT_EQ(*result, "The answer is 42\n");
}

TEST_F(RenderTest, imports_access_globals) {
  auto result = render(
      "{{#import \"module\" as lib}}\n"
      "The answer is {{lib.answer}}\n",
      w::map({{"global_answer", w::i64(84)}}),
      sources({
          {"module", "{{#let export answer = global_answer}}\n"},
      }),
      globals({{"global_answer", w::i64(42)}}));
  EXPECT_THAT(diagnostics(), testing::IsEmpty());
  EXPECT_EQ(*result, "The answer is 42\n");
}

TEST_F(RenderTest, imports_multiple_with_conflict) {
  auto result = render(
      "{{#import \"module1\" as conflict}}\n"
      "{{#import \"module2\" as conflict}}\n",
      w::map(),
      sources({
          {"module1", "{{#let export answer = 42}}\n"},
          {"module2", "{{#let export answer = 84}}\n"},
      }));
  EXPECT_THAT(
      diagnostics(),
      testing::ElementsAre(diagnostic(
          diagnostic_level::error,
          "Name 'conflict' is already bound in the current scope.",
          path_to_file,
          2)));
  EXPECT_FALSE(result.has_value());
}

TEST_F(RenderTest, imports_no_implicit_transitive_exports) {
  auto result = render(
      "{{#import \"module1\" as lib}}\n"
      "The answer is {{lib.lib.answer}}\n",
      w::map(),
      sources({
          {"module1", "{{#import \"module2\" as lib}}\n"},
          {"module2", "{{#let export answer = 84}}\n"},
      }));
  EXPECT_THAT(
      diagnostics(),
      testing::ElementsAre(diagnostic(
          diagnostic_level::error,
          "Object 'lib' has no property named 'lib'.\n"
          "The object with the missing property is:\n"
          "map (size=0)\n",
          path_to_file,
          2)));
  EXPECT_FALSE(result.has_value());
}

TEST_F(RenderTest, imports_no_text_in_module) {
  auto result = render(
      "{{#import \"some/file/path\" as lib}}\n",
      w::map(),
      sources({{"some/file/path", "This is not a module"}}));
  EXPECT_THAT(
      diagnostics(),
      testing::ElementsAre(diagnostic(
          diagnostic_level::error,
          "Modules cannot have non-whitespace text at the top-level",
          "some/file/path",
          1)));
  EXPECT_FALSE(result.has_value());
}

TEST_F(RenderTest, imports_no_conditional_export) {
  auto result = render(
      "{{#import \"some/file/path\" as lib}}\n"
      "The answer is {{lib.answer}}\n",
      w::map(),
      sources(
          {{"some/file/path",
            "{{#if true}}\n"
            "{{#let export answer = 42}}\n"
            "{{/if true}}\n"}}));
  EXPECT_THAT(
      diagnostics(),
      testing::ElementsAre(diagnostic(
          diagnostic_level::error,
          "Modules cannot have conditional blocks at the top-level",
          "some/file/path",
          1)));
  EXPECT_FALSE(result.has_value());
}

TEST_F(RenderTest, exports_in_root) {
  auto result = render("{{#let export answer = 42}}\n", w::map());
  EXPECT_THAT(
      diagnostics(),
      testing::ElementsAre(diagnostic(
          diagnostic_level::error,
          "Exports are not allowed in root source files or partial blocks",
          path_to_file,
          1)));
  EXPECT_FALSE(result.has_value());
}

TEST_F(RenderTest, exports_in_root_nested) {
  auto result = render(
      "{{#if true}}\n"
      "{{#let export answer = 42}}\n"
      "{{/if true}}\n"
      "This should work!\n",
      w::map());
  EXPECT_THAT(
      diagnostics(),
      testing::ElementsAre(diagnostic(
          diagnostic_level::error,
          "Exports are not allowed in root source files or partial blocks",
          path_to_file,
          2)));
  EXPECT_FALSE(result.has_value());
}

TEST_F(RenderTest, exports_in_root_nested_not_executed) {
  auto result = render(
      "{{#if false}}\n"
      "{{#let export answer = 42}}\n"
      "{{/if false}}\n"
      "This should work!\n",
      w::map());
  EXPECT_THAT(diagnostics(), testing::IsEmpty());
  EXPECT_EQ(*result, "This should work!\n");
}

TEST_F(RenderTest, exports_in_partial) {
  auto result = render(
      "{{#import \"some/file/path\" as lib}}\n"
      "The answer is {{#partial lib.echo thing=42}}\n",
      w::map(),
      sources(
          {{"some/file/path",
            "{{#let export partial echo |thing|}}\n"
            "{{#let export oops = 42}}\n"
            "{{thing}}\n"
            "{{/let partial}}\n"}}));
  EXPECT_THAT(
      diagnostics(),
      testing::ElementsAre(diagnostic(
          diagnostic_level::error,
          "Exports are not allowed in root source files or partial blocks",
          "some/file/path",
          2)));
  EXPECT_FALSE(result.has_value());
}

TEST_F(RenderTest, exports_partial_in_partial) {
  auto result = render(
      "{{#import \"some/file/path\" as lib}}\n"
      "The answer is {{#partial lib.echo thing=42}}\n",
      w::map(),
      sources(
          {{"some/file/path",
            "{{#let export partial echo |thing|}}\n"
            "{{#let export partial oops}}{{/let partial}}\n"
            "{{thing}}\n"
            "{{/let partial}}\n"}}));
  EXPECT_THAT(
      diagnostics(),
      testing::ElementsAre(diagnostic(
          diagnostic_level::error,
          "Exports are not allowed in root source files or partial blocks",
          "some/file/path",
          2)));
  EXPECT_FALSE(result.has_value());
}

TEST_F(RenderTest, strip_standalone_lines) {
  auto result = render(
      "| This Is\n"
      "{{#boolean}}\n"
      "|\n"
      "{{/boolean}}\n"
      "| A Line\n",
      w::map({{"boolean", w::true_}}));
  EXPECT_EQ(
      *result,
      "| This Is\n"
      "|\n"
      "| A Line\n");
}

TEST_F(RenderTest, strip_standalone_lines_indented) {
  auto result = render(
      "| This Is\n"
      "  {{#boolean}}\n"
      "|\n"
      "  {{/boolean}}\n"
      "| A Line\n",
      w::map({{"boolean", w::true_}}));
  EXPECT_EQ(
      *result,
      "| This Is\n"
      "|\n"
      "| A Line\n");
}

TEST_F(RenderTest, strip_standalone_lines_multiple) {
  auto result = render(
      "| This Is\n"
      "  {{#boolean}} {{#boolean}} \n"
      "|\n"
      "  {{/boolean}}\n"
      "  {{/boolean}}\n"
      "| A Line\n",
      w::map({{"boolean", w::true_}}));
  EXPECT_EQ(
      *result,
      "| This Is\n"
      "|\n"
      "| A Line\n");
}

TEST_F(RenderTest, strip_standalone_lines_multiline) {
  auto result = render(
      "| This Is\n"
      "  {{#boolean\n"
      "       .condition}}  \n"
      "|\n"
      "  {{/boolean.condition}}\n"
      "| A Line\n",
      w::map({{"boolean", w::map({{"condition", w::true_}})}}));
  EXPECT_EQ(
      *result,
      "| This Is\n"
      "|\n"
      "| A Line\n");
}

TEST_F(RenderTest, strip_standalone_lines_multiline_comment) {
  auto result = render(
      "| This Is\n"
      "  {{^boolean\n"
      "       .condition}} {{! unaffected }} \n"
      "|\n"
      "  {{/boolean.condition}}\n"
      "| A Line\n",
      w::map({{"boolean", w::map({{"condition", w::false_}})}}));
  EXPECT_EQ(
      *result,
      "| This Is\n"
      "|\n"
      "| A Line\n");
}

TEST_F(RenderTest, strip_standalone_lines_multiline_ineligible) {
  auto result = render(
      "| This Is\n"
      "  {{#boolean\n"
      "       .condition}} {{ineligible}} \n"
      "|\n"
      "  {{/boolean.condition}}\n"
      "| A Line\n",
      w::map(
          {{"ineligible", w::string("")},
           {"boolean", w::map({{"condition", w::true_}})}}));
  EXPECT_EQ(
      *result,
      "| This Is\n"
      "    \n"
      "|\n"
      "| A Line\n");
}

TEST_F(
    RenderTest,
    strip_standalone_lines_multiline_ineligible_partial_application) {
  auto result = render(
      "| This Is\n"
      "  {{#boolean\n"
      "       .condition}} {{> ineligible}} \n"
      "|\n"
      "  {{/boolean.condition}}\n"
      "| A Line\n",
      w::map(
          {{"value", w::i64(5)},
           {"boolean", w::map({{"condition", w::true_}})}}),
      sources({{"ineligible", "{{value}} (from macro)\n"}}));
  EXPECT_EQ(
      *result,
      "| This Is\n"
      "   5 (from macro)\n"
      " \n"
      "|\n"
      "| A Line\n");
}

TEST_F(RenderTest, globals) {
  auto result = render(
      "{{global}}\n"
      "{{#array}}\n"
      "{{.}} — {{global-2}}\n"
      "{{/array}}\n",
      w::map({{"array", w::array({w::i64(1), w::i64(2)})}}),
      sources({}),
      globals(
          {{"global", w::string("hello")}, {"global-2", w::string("hello!")}}));
  EXPECT_EQ(
      *result,
      "hello\n"
      "1 — hello!\n"
      "2 — hello!\n");
}

TEST_F(RenderTest, globals_shadowing) {
  auto result = render(
      "{{global}}\n",
      w::map({{"global", w::string("good")}}),
      sources({}),
      globals({{"global", w::string("bad")}}));
  EXPECT_EQ(*result, "good\n");
}

TEST_F(RenderTest, pragma_ignore_newlines) {
  auto result = render(
      "{{#pragma ignore-newlines}}\n"
      "This\n"
      " is\n"
      "{{! comment}}\n"
      " all\n"
      " one\n"
      " line\n",
      w::map());
  EXPECT_THAT(diagnostics(), testing::IsEmpty());
  EXPECT_EQ(*result, "This is all one line");

  // Test that the pragma is scoped to the active macro/template.
  result = render(
      "{{#pragma ignore-newlines}}\n"
      "This\n"
      " is\n"
      "{{> macro }}\n"
      " all\n"
      " one\n"
      " line\n",
      w::map(),
      sources({{"macro", "\nnot\n!\n"}}));
  EXPECT_THAT(diagnostics(), testing::IsEmpty());
  EXPECT_EQ(*result, "This is\nnot\n!\n all one line");
}

} // namespace whisker
