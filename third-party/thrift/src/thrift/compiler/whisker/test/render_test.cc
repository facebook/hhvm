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
            {{"news", w::map({{"has-update?", w::true_value}})},
             {"foo", w::string("now")}}));
    EXPECT_EQ(*result, "Stuff is now happening!");
  }
  {
    auto result = render(
        "{{#news.has-update?}}Stuff is {{foo}} happening!{{/news.has-update?}}",
        w::map({{"news", w::map({{"has-update?", w::false_value}})}}));
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
            {{"news", w::map({{"has-no-update?", w::false_value}})},
             {"foo", w::string("now")}}));
    EXPECT_EQ(*result, "Stuff is now happening!");
  }
  {
    auto result = render(
        "{{^news.has-no-update?}}Stuff is {{foo}} happening!{{/news.has-no-update?}}",
        w::map({{"news", w::map({{"has-no-update?", w::true_value}})}}));
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
            {{"news", w::map({{"has-update?", w::true_value}})},
             {"foo", w::string("now")}}));
    EXPECT_EQ(*result, "Stuff is now happening!\n");
  }
  {
    auto result = render(
        "{{#if news.has-update?}}Stuff is {{foo}} happening!{{/if news.has-update?}}",
        w::map({{"news", w::map({{"has-update?", w::false_value}})}}));
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
            {{"news", w::map({{"has-update?", w::false_value}})},
             {"foo", w::string("now")}}));
    EXPECT_EQ(*result, "Stuff is now happening!\n");
  }
  {
    auto result = render(
        "{{#if (not news.has-update?)}}Stuff is {{foo}} happening!{{/if (not news.has-update?)}}",
        w::map({{"news", w::map({{"has-update?", w::true_value}})}}));
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
            {{"news", w::map({{"has-update?", w::true_value}})},
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
        w::map({{"news", w::map({{"has-update?", w::false_value}})}}));
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
    auto result = render(template_text, w::map({{"a", w::true_value}}));
    EXPECT_EQ(*result, "a\n");
  }
  {
    auto result = render(
        template_text, w::map({{"a", w::false_value}, {"b", w::true_value}}));
    EXPECT_EQ(*result, "b\n");
  }
  {
    auto result = render(
        template_text,
        w::map(
            {{"a", w::false_value},
             {"b", w::false_value},
             {"c", w::true_value}}));
    EXPECT_EQ(*result, "c\n");
  }
  {
    auto result = render(
        template_text,
        w::map(
            {{"a", w::false_value},
             {"b", w::false_value},
             {"c", w::false_value}}));
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
        w::map({{"news", w::map({{"has-update?", w::false_value}})}}));
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
            {{"news", w::map({{"has-update?", w::true_value}})},
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
        w::map({{"news", w::map({{"has-update?", w::false_value}})}}));
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
            {{"news", w::map({{"has-update?", w::true_value}})},
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
        w::map({{"yes", w::true_value}, {"no", w::false_value}}));
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
        w::map({{"news", w::map({{"has-update?", w::false_value}})}}));
    EXPECT_THAT(diagnostics(), testing::IsEmpty());
    EXPECT_EQ(*result, "Nothing is happening!\n");
  }
}

TEST_F(RenderTest, builtin_ternary) {
  {
    auto result = render(
        R"({{(if cond? "Yes" "No")}})", w::map({{"cond?", w::true_value}}));
    EXPECT_THAT(diagnostics(), testing::IsEmpty());
    EXPECT_EQ(*result, "Yes");
  }
  {
    auto result = render(
        R"({{(if cond? "Yes" "No")}})", w::map({{"cond?", w::false_value}}));
    EXPECT_THAT(diagnostics(), testing::IsEmpty());
    EXPECT_EQ(*result, "No");
  }
}

TEST_F(RenderTest, builtin_ternary_short_circuit) {
  {
    auto result = render(
        "{{(if cond? \"Yes\" intentionally_undefined)}}",
        w::map({{"cond?", w::true_value}}));
    EXPECT_THAT(diagnostics(), testing::IsEmpty());
    EXPECT_EQ(*result, "Yes");
  }
  {
    auto result = render(
        "{{(if cond? intentionally_undefined \"No\")}}",
        w::map({{"cond?", w::false_value}}));
    EXPECT_THAT(diagnostics(), testing::IsEmpty());
    EXPECT_EQ(*result, "No");
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

/**
 * A function that sets a bool to true when invoked. Returns a fixed string.
 * Used to detect whether an expression was evaluated.
 */
class set_flag : public dsl::function {
 public:
  explicit set_flag(bool& flag) : flag_(flag) {}

 private:
  object invoke(context ctx) override {
    ctx.declare_arity(0);
    ctx.declare_named_arguments({});
    flag_ = true;
    return w::null;
  }

  bool& flag_;
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
            "",
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
      dsl::polymorphic_native_handle<"base", SomeCppObjectBase, SomeCppObject>;
  using child_handle_type =
      dsl::make_polymorphic_native_handle<"proto", SomeCppObject>;

  const auto base_proto = dsl::make_prototype<base_handle_type>([](auto&& def) {
    def.property(
        "name", [](const SomeCppObjectBase&) { return w::string("base"); });
    def.property("return42", [](const SomeCppObjectBase& self) {
      return w::i64(self.return42());
    });
  });

  const auto proto =
      dsl::make_prototype<child_handle_type>(base_proto, [](auto&& def) {
        def.property("name", [](const SomeCppObjectBase&) {
          return w::string("proto");
        });
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
      "{{ foo.return42 }}\n"
      "{{ foo.name }}\n",
      context);
  EXPECT_THAT(diagnostics(), testing::IsEmpty());
  EXPECT_EQ(
      *result,
      "0\n"
      "incremented!\n"
      "1\n"
      "incremented!\n"
      "2\n"
      "42\n"
      "proto\n");

  result = render(
      "{{ foo.proto:get }}\n"
      "{{ (foo.proto:increment) }}\n"
      "{{ foo.proto:get }}\n"
      "{{ (foo.proto:increment) }}\n"
      "{{ foo.proto:get }}\n"
      "{{ foo.proto:return42 }}\n"
      "{{ foo.base:return42 }}\n"
      "{{ foo.name }}\n"
      "{{ foo.proto:name }}\n"
      "{{ foo.base:name }}\n",
      context);
  EXPECT_THAT(diagnostics(), testing::IsEmpty());
  EXPECT_EQ(
      *result,
      "2\n"
      "incremented!\n"
      "3\n"
      "incremented!\n"
      "4\n"
      "42\n"
      "42\n"
      "proto\n"
      "proto\n"
      "base\n");

  result = render("{{ (foo.base:increment) }}", context);
  EXPECT_FALSE(result.has_value());
  EXPECT_THAT(
      diagnostics(),
      testing::ElementsAre(diagnostic(
          diagnostic_level::error,
          "Object 'foo' has no property named 'base:increment'.\n"
          "The object with the missing property is:\n"
          "<native_handle type='whisker::RenderTest_user_defined_function_native_ref_prototype_Test::TestBody()::SomeCppObject'>\n"
          "╰─ ...\n",
          path_to_file,
          1)));
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
            w::map(
                {{"has-update?", w::true_value},
                 {"foo", w::string("now")}})}}));
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

TEST_F(RenderTest, each_separator_basic) {
  auto result = render(
      R"({{#each items as |item| separator=", " ~}})"
      "\n"
      "{{~ item ~}}"
      "\n"
      "{{~ /each}}",
      w::map(
          {{"items",
            w::array({w::string("a"), w::string("b"), w::string("c")})}}));
  EXPECT_EQ(*result, "a, b, c");
}

TEST_F(RenderTest, each_separator_single_item) {
  auto result = render(
      R"({{#each items as |item| separator=", " ~}})"
      "\n"
      "{{~ item ~}}"
      "\n"
      "{{~ /each}}",
      w::map({{"items", w::array({w::string("only")})}}));
  EXPECT_EQ(*result, "only");
}

TEST_F(RenderTest, each_separator_empty_array_else) {
  auto result = render(
      R"({{#each items as |item| separator=", " ~}})"
      "\n"
      "{{~ item ~}}"
      "\n"
      "{{~ #else ~}}"
      "\n"
      "none"
      "\n"
      "{{~ /each}}",
      w::map({{"items", w::array()}}));
  EXPECT_EQ(*result, "none");
}

TEST_F(RenderTest, each_separator_not_evaluated_for_empty_array) {
  bool separator_evaluated = false;
  auto result = render(
      "{{#each items as |item| separator=(make_sep) ~}}"
      "{{~ item ~}}"
      "{{~ #else ~}}"
      "none"
      "{{~ /each}}",
      w::map({
          {"items", w::array()},
          {"make_sep",
           w::make_native_function<functions::set_flag>(separator_evaluated)},
      }));
  EXPECT_EQ(*result, "none");
  EXPECT_FALSE(separator_evaluated);
}

TEST_F(RenderTest, each_separator_empty_string) {
  auto result = render(
      R"({{#each items as |item| separator="" ~}})"
      "\n"
      "{{~ item ~}}"
      "\n"
      "{{~ /each}}",
      w::map(
          {{"items",
            w::array({w::string("a"), w::string("b"), w::string("c")})}}));
  EXPECT_EQ(*result, "abc");
}

TEST_F(RenderTest, each_separator_variable) {
  auto result = render(
      "{{#each items as |item| separator=sep ~}}"
      "\n"
      "{{~ item ~}}"
      "\n"
      "{{~ /each}}",
      w::map({
          {"items", w::array({w::string("x"), w::string("y")})},
          {"sep", w::string(" | ")},
      }));
  EXPECT_EQ(*result, "x | y");
}

TEST_F(RenderTest, each_separator_function_call) {
  use_library(load_standard_library);
  auto result = render(
      R"({{#each items as |item| separator=(string.concat "-" "-") ~}})"
      "\n"
      "{{~ item ~}}"
      "\n"
      "{{~ /each}}",
      w::map({{"items", w::array({w::string("a"), w::string("b")})}}));
  EXPECT_EQ(*result, "a--b");
}

TEST_F(RenderTest, each_separator_no_tildes) {
  auto result = render(
      R"({{#each items as |item| separator=", "}})"
      "\n"
      "{{item}}\n"
      "{{/each}}",
      w::map({{"items", w::array({w::string("a"), w::string("b")})}}));
  EXPECT_EQ(*result, "a\n, b\n");
}

TEST_F(RenderTest, each_separator_no_captures) {
  auto result = render(
      R"({{#each items separator="-" ~}})"
      "\n"
      "{{~ . ~}}"
      "\n"
      "{{~ /each}}",
      w::map(
          {{"items",
            w::array({w::string("x"), w::string("y"), w::string("z")})}}));
  EXPECT_EQ(*result, "x-y-z");
}

TEST_F(RenderTest, each_separator_multiple_captures) {
  use_library(load_standard_library);
  auto result = render(
      R"({{#each (array.enumerate items) as |i item| separator="; " ~}})"
      "\n"
      "{{~ i}}={{item ~}}"
      "\n"
      "{{~ /each}}",
      w::map({{"items", w::array({w::string("a"), w::string("b")})}}));
  EXPECT_EQ(*result, "0=a; 1=b");
}

TEST_F(RenderTest, each_separator_empty_iteration) {
  auto result = render(
      R"({{#each items as |item| separator=", " ~}})"
      "\n"
      "{{~ #if item.show ~}}{{item.name}}{{~ /if ~}}"
      "\n"
      "{{~ /each}}",
      w::map(
          {{"items",
            w::array(
                {w::map({{"name", w::string("a")}, {"show", w::true_value}}),
                 w::map({{"name", w::string("b")}, {"show", w::false_value}}),
                 w::map(
                     {{"name", w::string("c")}, {"show", w::true_value}})})}}));
  EXPECT_EQ(*result, "a, , c");
}

TEST_F(RenderTest, each_separator_non_string_error) {
  auto result = render(
      "{{#each items as |item| separator=sep ~}}"
      "\n"
      "{{~ item ~}}"
      "\n"
      "{{~ /each}}",
      w::map({
          {"items", w::array({w::string("a")})},
          {"sep", w::i64(42)},
      }));
  EXPECT_FALSE(result.has_value());
  EXPECT_THAT(
      diagnostics(),
      testing::ElementsAre(diagnostic(
          diagnostic_level::error,
          "Separator must evaluate to a string. "
          "The encountered value is:\n"
          "i64(42)\n",
          path_to_file,
          1)));
}

TEST_F(RenderTest, each_separator_capture_named_separator) {
  auto result = render(
      R"({{#each items as |separator| separator=", " ~}})"
      "\n"
      "{{~ separator ~}}"
      "\n"
      "{{~ /each}}",
      w::map({{"items", w::array({w::string("a"), w::string("b")})}}));
  EXPECT_EQ(*result, "a, b");
}

TEST_F(RenderTest, each_separator_evaluated_in_outer_scope) {
  // The separator expression is evaluated once in the outer scope before the
  // loop. Here 'item' in the outer scope is "+", while the capture 'item'
  // inside the loop takes on each element's value.
  auto result = render(
      "{{#each items as |item| separator=item ~}}"
      "\n"
      "{{~ item ~}}"
      "\n"
      "{{~ /each}}",
      w::map({
          {"items", w::array({w::string("a"), w::string("b"), w::string("c")})},
          {"item", w::string("+")},
      }));
  EXPECT_EQ(*result, "a+b+c");
}

TEST_F(RenderTest, each_separator_evaluated_once) {
  auto result = render(
      "{{#each items as |item| separator=delim ~}}"
      "\n"
      "{{~ item ~}}"
      "\n"
      "{{~ /each}}",
      w::map({
          {"items", w::array({w::string("a"), w::string("b"), w::string("c")})},
          {"delim", w::string("+")},
      }));
  EXPECT_EQ(*result, "a+b+c");
}

TEST_F(RenderTest, each_separator_with_pragma_ignore_newlines) {
  // #pragma ignore-newlines suppresses AST newline nodes in the body,
  // but newlines within the separator string are preserved because they
  // are written via out_.write(std::string_view), not as AST newline nodes.
  auto result = render(
      "{{#pragma ignore-newlines}}\n"
      "{{#each items as |item| separator=sep}}\n"
      "{{item}}\n"
      "{{/each}}",
      w::map({
          {"items", w::array({w::string("a"), w::string("b"), w::string("c")})},
          {"sep", w::string(",\n")},
      }));
  EXPECT_EQ(*result, "a,\nb,\nc");
}

TEST_F(RenderTest, each_separator_non_array_error) {
  auto result = render(
      R"({{#each number as |n| separator=", " ~}})"
      "\n"
      "{{~ n ~}}"
      "\n"
      "{{~ /each}}",
      w::map({{"number", w::i64(2)}}));
  EXPECT_FALSE(result.has_value());
  EXPECT_THAT(
      diagnostics(),
      testing::ElementsAre(diagnostic(
          diagnostic_level::error,
          "Expression 'number' does not evaluate to an array. "
          "The encountered value is:\n"
          "i64(2)\n",
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
    auto result = render("{{bool}}", w::map({{"bool", w::true_value}}));
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
          error_backtrace(
              "#0 foo @ path/to/test.whisker <line:3, col:3>\n"
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
          error_backtrace(
              "#0 baz @ path/to/test.whisker <line:4, col:9>\n"
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

TEST_F(RenderTest, partials_indentation_blank_lines) {
  auto result = render(
      R"({{#let partial foo}}
line1

line2
{{/let partial}}
  {{#partial foo}}
)",
      w::map());
  EXPECT_THAT(diagnostics(), testing::IsEmpty());
  EXPECT_EQ(
      *result,
      R"(  line1

  line2
)");
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

TEST_F(RenderTest, macros_indentation_blank_lines) {
  auto result = render(
      R"(before
  {{>some/file/path}}
after
)",
      w::map({{"value", w::i64(42)}}),
      sources(
          {{"some/file/path",
            R"({{value}} (from macro)

A second line
)"}}));
  EXPECT_EQ(
      *result,
      R"(before
  42 (from macro)

  A second line
after
)");
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
          error_backtrace(
              "#0 <macro> @ macro-3 <line:1, col:3>\n"
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
      w::map({{"boolean", w::true_value}}));
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
      w::map({{"boolean", w::true_value}}));
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
      w::map({{"boolean", w::true_value}}));
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
      w::map({{"boolean", w::map({{"condition", w::true_value}})}}));
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
      w::map({{"boolean", w::map({{"condition", w::false_value}})}}));
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
           {"boolean", w::map({{"condition", w::true_value}})}}));
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
           {"boolean", w::map({{"condition", w::true_value}})}}),
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

// --- Tilde whitespace stripping tests ---

TEST_F(RenderTest, tilde_strip_left_whitespace) {
  auto result = render("foo   {{~ bar}}", w::map({{"bar", w::string("BAR")}}));
  EXPECT_EQ(*result, "fooBAR");
}

TEST_F(RenderTest, tilde_strip_right_whitespace) {
  auto result = render("{{bar ~}}   baz", w::map({{"bar", w::string("BAR")}}));
  EXPECT_EQ(*result, "BARbaz");
}

TEST_F(RenderTest, tilde_strip_both) {
  auto result = render("  {{~ bar ~}}  ", w::map({{"bar", w::string("BAR")}}));
  EXPECT_EQ(*result, "BAR");
}

TEST_F(RenderTest, tilde_strip_left_across_newlines) {
  auto result =
      render("foo\n   {{~ bar}}", w::map({{"bar", w::string("BAR")}}));
  EXPECT_EQ(*result, "fooBAR");
}

TEST_F(RenderTest, tilde_strip_right_across_newlines) {
  auto result =
      render("{{bar ~}}\n   baz", w::map({{"bar", w::string("BAR")}}));
  EXPECT_EQ(*result, "BARbaz");
}

TEST_F(RenderTest, tilde_stops_at_non_whitespace) {
  auto result =
      render("foo  bar  {{~ baz}}", w::map({{"baz", w::string("BAZ")}}));
  EXPECT_EQ(*result, "foo  barBAZ");
}

TEST_F(RenderTest, tilde_strip_left_multiple_newlines) {
  auto result =
      render("foo\n\n\n   {{~ bar}}", w::map({{"bar", w::string("BAR")}}));
  EXPECT_EQ(*result, "fooBAR");
}

TEST_F(RenderTest, tilde_if_block_all_tildes) {
  auto result = render(
      "  {{~ #if cond ~}}  \n  yes  \n  {{~ /if cond ~}}  ",
      w::map({{"cond", w::true_value}}));
  EXPECT_EQ(*result, "yes");
}

TEST_F(RenderTest, tilde_if_block_false_condition) {
  auto result = render(
      "before  {{~ #if cond ~}}  skipped  {{~ /if cond ~}}  after",
      w::map({{"cond", w::false_value}}));
  EXPECT_EQ(*result, "beforeafter");
}

TEST_F(RenderTest, tilde_each_block) {
  // Parse-time tilde stripping removes body whitespace/newlines for all
  // iterations uniformly, matching Handlebars' AST-level stripping.
  auto result = render(
      "items:{{~ #each items as |item| ~}}\n  {{item}},\n{{~ /each ~}}\n done",
      w::map({{"items", w::array({w::string("a"), w::string("b")})}}));
  EXPECT_EQ(*result, "items:a,b,done");
}

TEST_F(RenderTest, tilde_else_clause) {
  auto result = render(
      "{{#if cond ~}} yes {{~ #else ~}} no {{~ /if cond}}",
      w::map({{"cond", w::false_value}}));
  EXPECT_EQ(*result, "no");
}

TEST_F(RenderTest, tilde_with_block) {
  auto result = render(
      "{{~ #with obj ~}}{{name}}{{~ /with ~}}",
      w::map({{"obj", w::map({{"name", w::string("hello")}})}}));
  EXPECT_EQ(*result, "hello");
}

TEST_F(RenderTest, tilde_at_start_of_file) {
  auto result = render("{{~ bar}}", w::map({{"bar", w::string("BAR")}}));
  EXPECT_EQ(*result, "BAR");
}

TEST_F(RenderTest, tilde_at_end_of_file) {
  auto result = render("{{bar ~}}", w::map({{"bar", w::string("BAR")}}));
  EXPECT_EQ(*result, "BAR");
}

TEST_F(RenderTest, tilde_with_pragma_ignore_newlines) {
  auto result = render(
      "{{#pragma ignore-newlines}}\nfoo  {{~ bar ~}}  baz\n",
      w::map({{"bar", w::string("X")}}));
  EXPECT_THAT(diagnostics(), testing::IsEmpty());
  EXPECT_EQ(*result, "fooXbaz");
}

TEST_F(RenderTest, tilde_consecutive_tags) {
  auto result = render(
      "{{a ~}} {{~ b}}",
      w::map({{"a", w::string("A")}, {"b", w::string("B")}}));
  EXPECT_EQ(*result, "AB");
}

TEST_F(RenderTest, tilde_no_whitespace_to_strip) {
  auto result = render("foo{{~ bar}}", w::map({{"bar", w::string("BAR")}}));
  EXPECT_EQ(*result, "fooBAR");
}

TEST_F(RenderTest, tilde_derive_example) {
  auto result = render(
      "#[derive(\n"
      "    {{~ #if copy }}Copy, {{/if copy ~}}\n"
      "    Clone, PartialEq\n"
      "    {{~ #if ord }}, Eq, Hash{{/if ord ~}}\n"
      ")]\n",
      w::map({{"copy", w::true_value}, {"ord", w::true_value}}));
  EXPECT_EQ(*result, "#[derive(Copy, Clone, PartialEq, Eq, Hash)]\n");
}

TEST_F(RenderTest, tilde_standalone_right_eats_next_line_indent) {
  // {{#if ~}} has tilde → NOT standalone. The indent before the tag is
  // preserved. Right tilde eats \n + indent of next line.
  // {{/if}} has no tilde → standalone (stripped).
  auto result = render(
      "    {{#if cond ~}}\n    content\n    {{/if cond}}",
      w::map({{"cond", w::true_value}}));
  EXPECT_EQ(*result, "    content\n");
}

TEST_F(RenderTest, tilde_standalone_left_eats_prev_line_trailing) {
  auto result = render(
      "{{#if cond}}\ncontent\n    {{~ /if cond}}",
      w::map({{"cond", w::true_value}}));
  EXPECT_EQ(*result, "content");
}

TEST_F(RenderTest, tilde_nested_blocks) {
  auto result = render(
      "{{~ #if x ~}}{{~ #if y ~}}content{{~ /if y ~}}{{~ /if x ~}}",
      w::map({{"x", w::true_value}, {"y", w::true_value}}));
  EXPECT_EQ(*result, "content");
}

TEST_F(RenderTest, tilde_each_else_clause) {
  auto result = render(
      "{{~ #each empty as |item| ~}}...{{~ #else ~}}fallback{{~ /each ~}}",
      w::map({{"empty", w::array()}}));
  EXPECT_EQ(*result, "fallback");
}

TEST_F(RenderTest, tilde_empty_body) {
  auto result = render(
      "before{{~ #if cond ~}}{{~ /if cond ~}}after",
      w::map({{"cond", w::true_value}}));
  EXPECT_EQ(*result, "beforeafter");
}

TEST_F(RenderTest, tilde_else_if) {
  auto result = render(
      "{{~ #if a ~}}A{{~ #else if b ~}}B{{~ /if a ~}}",
      w::map({{"a", w::false_value}, {"b", w::true_value}}));
  EXPECT_EQ(*result, "B");
}

TEST_F(RenderTest, tilde_strip_tabs) {
  auto result = render("foo\t\t{{~ bar}}", w::map({{"bar", w::string("BAR")}}));
  EXPECT_EQ(*result, "fooBAR");
}

TEST_F(RenderTest, tilde_right_stops_at_non_whitespace) {
  auto result =
      render("{{bar ~}}text  baz", w::map({{"bar", w::string("BAR")}}));
  EXPECT_EQ(*result, "BARtext  baz");
}

TEST_F(RenderTest, tilde_let_statement) {
  auto result = render("before {{~ #let x = 42 ~}} {{x}} after", w::map());
  EXPECT_EQ(*result, "before42 after");
}

TEST_F(RenderTest, tilde_section_block) {
  auto result = render(
      "  {{~ #var ~}}  content  {{~ /var ~}}  ",
      w::map({{"var", w::true_value}}));
  EXPECT_EQ(*result, "content");
}

// --- Tilde trimming: additional edge cases ---

TEST_F(RenderTest, tilde_left_only_on_open_tag) {
  // Left tilde on opening tag only — strips before, not after.
  auto result = render(
      "before  {{~ #if cond}}  after{{/if cond}}",
      w::map({{"cond", w::true_value}}));
  EXPECT_EQ(*result, "before  after");
}

TEST_F(RenderTest, tilde_right_only_on_close_tag) {
  // Right tilde on closing tag only — strips after, not before.
  auto result = render(
      "{{#if cond}}before  {{/if cond ~}}  after",
      w::map({{"cond", w::true_value}}));
  EXPECT_EQ(*result, "before  after");
}

TEST_F(RenderTest, tilde_mixed_whitespace_spaces_and_tabs) {
  auto result = render(
      "foo \t \t {{~ bar ~}} \t \t baz", w::map({{"bar", w::string("X")}}));
  EXPECT_EQ(*result, "fooXbaz");
}

TEST_F(RenderTest, tilde_left_preserves_content_on_same_line) {
  // Left tilde only strips whitespace — non-whitespace is preserved.
  auto result =
      render("hello world  {{~ bar}}", w::map({{"bar", w::string("!")}}));
  EXPECT_EQ(*result, "hello world!");
}

TEST_F(RenderTest, tilde_right_preserves_content_on_same_line) {
  auto result =
      render("{{bar ~}}  hello world", w::map({{"bar", w::string("!")}}));
  EXPECT_EQ(*result, "!hello world");
}

TEST_F(RenderTest, tilde_multiple_interpolations_on_one_line) {
  auto result = render(
      "  {{~ a ~}}  {{~ b ~}}  {{~ c ~}}  ",
      w::map(
          {{"a", w::string("1")},
           {"b", w::string("2")},
           {"c", w::string("3")}}));
  EXPECT_EQ(*result, "123");
}

TEST_F(RenderTest, tilde_chain_of_consecutive_tags_with_whitespace) {
  auto result = render(
      "start {{a ~}}   {{~ b ~}}   {{~ c}} end",
      w::map(
          {{"a", w::string("A")},
           {"b", w::string("B")},
           {"c", w::string("C")}}));
  EXPECT_EQ(*result, "start ABC end");
}

TEST_F(RenderTest, tilde_if_true_else_with_left_tilde_on_else) {
  // When condition is true, the else clause's left tilde strips trailing
  // whitespace of the if-body.
  auto result = render(
      "{{#if cond}}yes   {{~ #else}}no{{/if cond}}",
      w::map({{"cond", w::true_value}}));
  EXPECT_EQ(*result, "yes");
}

TEST_F(RenderTest, tilde_if_false_else_with_right_tilde_on_else) {
  // When condition is false, the else clause's right tilde strips leading
  // whitespace of the else-body.
  auto result = render(
      "{{#if cond}}yes{{#else ~}}   no{{/if cond}}",
      w::map({{"cond", w::false_value}}));
  EXPECT_EQ(*result, "no");
}

TEST_F(RenderTest, tilde_else_if_chain_all_false) {
  // All conditions false, falls through to else.
  auto result = render(
      "{{~ #if a ~}}A{{~ #else if b ~}}B{{~ #else ~}}C{{~ /if a ~}}",
      w::map({{"a", w::false_value}, {"b", w::false_value}}));
  EXPECT_EQ(*result, "C");
}

TEST_F(RenderTest, tilde_else_if_chain_middle_true) {
  auto result = render(
      "  {{~ #if a ~}}  A  "
      "{{~ #else if b ~}}  B  "
      "{{~ #else ~}}  C  {{~ /if a ~}}  ",
      w::map({{"a", w::false_value}, {"b", w::true_value}}));
  EXPECT_EQ(*result, "B");
}

TEST_F(RenderTest, tilde_nested_if_outer_false) {
  // Outer false — entire block (including inner) is invisible.
  auto result = render(
      "before {{~ #if outer ~}} {{~ #if inner ~}} content "
      "{{~ /if inner ~}} {{~ /if outer ~}} after",
      w::map({{"outer", w::false_value}, {"inner", w::true_value}}));
  EXPECT_EQ(*result, "beforeafter");
}

TEST_F(RenderTest, tilde_each_single_element) {
  auto result = render(
      "[{{~ #each items as |item| ~}}, {{item}}{{~ /each ~}}]",
      w::map({{"items", w::array({w::string("only")})}}));
  EXPECT_EQ(*result, "[, only]");
}

TEST_F(RenderTest, tilde_each_empty) {
  // Empty array with tildes — entire block is invisible.
  auto result = render(
      "before {{~ #each items as |item| ~}} {{item}} {{~ /each ~}} after",
      w::map({{"items", w::array()}}));
  EXPECT_EQ(*result, "beforeafter");
}

TEST_F(RenderTest, tilde_with_integer_interpolation) {
  auto result = render("value: {{~ x ~}} !", w::map({{"x", w::i64(42)}}));
  EXPECT_EQ(*result, "value:42!");
}

TEST_F(RenderTest, tilde_strip_crlf_newlines) {
  auto result =
      render("foo\r\n   {{~ bar}}", w::map({{"bar", w::string("BAR")}}));
  EXPECT_EQ(*result, "fooBAR");
}

TEST_F(RenderTest, tilde_right_strip_crlf_newlines) {
  auto result =
      render("{{bar ~}}\r\n   baz", w::map({{"bar", w::string("BAR")}}));
  EXPECT_EQ(*result, "BARbaz");
}

TEST_F(RenderTest, tilde_partial_statement) {
  auto result = render(
      "{{#let partial greet |name|}}\n"
      "hello {{name}}\n"
      "{{/let partial}}\n"
      "before {{~ #partial greet name=\"world\" ~}} after",
      w::map());
  EXPECT_EQ(*result, "beforehello world\nafter");
}

TEST_F(RenderTest, tilde_left_at_start_of_file_is_noop) {
  // Left tilde at start of file — nothing to strip, should be harmless.
  auto result = render("{{~ bar}} end", w::map({{"bar", w::string("BAR")}}));
  EXPECT_EQ(*result, "BAR end");
}

TEST_F(RenderTest, tilde_right_at_end_of_file_is_noop) {
  // Right tilde at end of file — nothing to strip, should be harmless.
  auto result = render("start {{bar ~}}", w::map({{"bar", w::string("BAR")}}));
  EXPECT_EQ(*result, "start BAR");
}

TEST_F(RenderTest, tilde_whitespace_only_file_with_strip) {
  // Entire file is whitespace around a tilde-stripped interpolation.
  auto result =
      render("   \n  {{~ bar ~}}  \n   ", w::map({{"bar", w::string("X")}}));
  EXPECT_EQ(*result, "X");
}

TEST_F(RenderTest, tilde_empty_string_interpolation) {
  // Interpolation produces empty string — tildes still strip surrounding ws.
  auto result =
      render("before  {{~ bar ~}}  after", w::map({{"bar", w::string("")}}));
  EXPECT_EQ(*result, "beforeafter");
}

TEST_F(RenderTest, tilde_if_block_open_left_close_right_only) {
  // Only left tilde on open and right tilde on close.
  auto result = render(
      "  {{~ #if cond}}content{{/if cond ~}}  ",
      w::map({{"cond", w::true_value}}));
  EXPECT_EQ(*result, "content");
}

TEST_F(RenderTest, tilde_if_block_open_right_close_left_only) {
  // Only right tilde on open and left tilde on close — strips body edges.
  auto result = render(
      "{{#if cond ~}}  content  {{~ /if cond}}",
      w::map({{"cond", w::true_value}}));
  EXPECT_EQ(*result, "content");
}

TEST_F(RenderTest, tilde_section_block_array_iteration) {
  strict_boolean_conditional(diagnostic_level::warning);
  // Parse-time tilde stripping removes body whitespace/newlines for all
  // iterations uniformly, matching Handlebars' AST-level stripping.
  auto result = render(
      "items:{{~ #items ~}}\n  ({{.}}){{~ /items ~}}\ndone",
      w::map({{"items", w::array({w::i64(1), w::i64(2), w::i64(3)})}}));
  EXPECT_EQ(*result, "items:(1)(2)(3)done");
}

TEST_F(RenderTest, tilde_standalone_both_sides) {
  // Tilde prevents standalone. Both tildes strip whitespace on both sides,
  // producing the same result as if standalone had fired.
  auto result = render(
      "before\n    {{~ #if cond ~}}\nafter\n    {{~ /if cond ~}}\n",
      w::map({{"cond", w::true_value}}));
  EXPECT_EQ(*result, "beforeafter");
}

TEST_F(RenderTest, tilde_let_in_block) {
  // The right tilde on #let strips whitespace before {{x}}, so the leading
  // "  " on the {{x}} line is eaten.
  auto result = render(
      "{{#if cond}}\n"
      "  {{~ #let x = 42 ~}}\n"
      "  {{x}}\n"
      "{{/if cond}}",
      w::map({{"cond", w::true_value}}));
  EXPECT_EQ(*result, "42\n");
}

TEST_F(RenderTest, tilde_derive_example_some_false) {
  // Same derive pattern but with some conditions false.
  auto result = render(
      "#[derive(\n"
      "    {{~ #if copy }}Copy, {{/if copy ~}}\n"
      "    Clone, PartialEq\n"
      "    {{~ #if ord }}, Eq, Hash{{/if ord ~}}\n"
      ")]\n",
      w::map({{"copy", w::false_value}, {"ord", w::false_value}}));
  EXPECT_EQ(*result, "#[derive(Clone, PartialEq)]\n");
}

TEST_F(RenderTest, tilde_preserves_interpolation_whitespace_both_sides) {
  // Both tildes strip the template "  " between tags, but the trailing
  // spaces in a's output and leading spaces in b's output are preserved.
  auto result = render(
      "{{a ~}}  {{~ b}}",
      w::map({{"a", w::string("hello  ")}, {"b", w::string("  world")}}));
  EXPECT_EQ(*result, "hello    world");
}

TEST_F(RenderTest, tilde_left_preserves_interpolation_trailing_spaces) {
  // Left tilde on b strips the template "  " between tags.
  // a's trailing spaces are interpolation output and are not stripped.
  auto result = render(
      "{{a}}  {{~ b}}",
      w::map({{"a", w::string("hello  ")}, {"b", w::string("X")}}));
  EXPECT_EQ(*result, "hello  X");
}

TEST_F(RenderTest, tilde_left_preserves_interpolation_trailing_tabs) {
  auto result = render(
      "{{a}}\t\t{{~ b}}",
      w::map({{"a", w::string("hello\t\t")}, {"b", w::string("X")}}));
  EXPECT_EQ(*result, "hello\t\tX");
}

TEST_F(RenderTest, tilde_left_preserves_interpolation_trailing_newline) {
  // a's output ends with a newline. The left tilde strips the template
  // whitespace "  " between tags but does not touch a's trailing newline.
  auto result = render(
      "{{a}}  {{~ b}}",
      w::map({{"a", w::string("hello\n")}, {"b", w::string("X")}}));
  EXPECT_EQ(*result, "hello\nX");
}

TEST_F(RenderTest, tilde_right_preserves_interpolation_leading_spaces) {
  // Right tilde on a strips the template "  " between tags.
  // b's leading spaces are interpolation output and are not stripped.
  auto result = render(
      "{{a ~}}  {{b}}",
      w::map({{"a", w::string("X")}, {"b", w::string("  world")}}));
  EXPECT_EQ(*result, "X  world");
}

TEST_F(RenderTest, tilde_right_preserves_interpolation_leading_tabs) {
  auto result = render(
      "{{a ~}}\t\t{{b}}",
      w::map({{"a", w::string("X")}, {"b", w::string("\t\tworld")}}));
  EXPECT_EQ(*result, "X\t\tworld");
}

TEST_F(RenderTest, tilde_right_preserves_interpolation_leading_newline) {
  auto result = render(
      "{{a ~}}  {{b}}",
      w::map({{"a", w::string("X")}, {"b", w::string("\nworld")}}));
  EXPECT_EQ(*result, "X\nworld");
}

TEST_F(RenderTest, tilde_left_no_template_whitespace_between_tags) {
  // No template whitespace between tags — left tilde is a no-op.
  // a's trailing whitespace is interpolation output, untouched.
  auto result = render(
      "{{a}}{{~ b}}",
      w::map({{"a", w::string("hello  ")}, {"b", w::string("X")}}));
  EXPECT_EQ(*result, "hello  X");
}

TEST_F(RenderTest, tilde_right_no_template_whitespace_between_tags) {
  // No template whitespace between tags — right tilde is a no-op.
  // b's leading whitespace is interpolation output, untouched.
  auto result = render(
      "{{a ~}}{{b}}",
      w::map({{"a", w::string("X")}, {"b", w::string("  world")}}));
  EXPECT_EQ(*result, "X  world");
}

TEST_F(RenderTest, tilde_preserves_whitespace_only_interpolation) {
  // Interpolation produces only whitespace. Tildes strip the template
  // whitespace around it but do not strip the interpolation output itself.
  auto result =
      render("before  {{~ a ~}}  after", w::map({{"a", w::string("   ")}}));
  EXPECT_EQ(*result, "before   after");
}

TEST_F(RenderTest, tilde_preserves_interpolation_across_newline_template) {
  // Template has a newline between tags. Right tilde strips the template
  // newline and following whitespace, but b's leading whitespace is preserved.
  auto result = render(
      "{{a ~}}\n  {{b}}",
      w::map({{"a", w::string("X")}, {"b", w::string("\tY")}}));
  EXPECT_EQ(*result, "X\tY");
}

TEST_F(
    RenderTest,
    tilde_left_preserves_interpolation_whitespace_in_indented_partial) {
  // An indented partial application contains an interpolation whose output has
  // trailing whitespace, followed by a left-tilde tag. The left tilde must
  // strip only the template whitespace between the two tags, not the trailing
  // whitespace from the interpolation output. Indentation from the partial
  // application site must also be applied correctly (once per line).
  auto result = render(
      "{{#let partial greet |name greeting|}}\n"
      "{{name}}  {{~ greeting}}\n"
      "{{/let partial}}\n"
      "  {{#partial greet name=\"hello  \" greeting=\"world\"}}",
      w::map());
  EXPECT_EQ(*result, "  hello  world\n");
}

TEST_F(RenderTest, tilde_comment_left_strips_whitespace) {
  auto result = render("foo   {{~ ! comment }}bar", w::map());
  EXPECT_EQ(*result, "foobar");
}

TEST_F(RenderTest, tilde_comment_right_strips_whitespace) {
  auto result = render("foo{{! comment ~}}   bar", w::map());
  EXPECT_EQ(*result, "foobar");
}

TEST_F(RenderTest, tilde_comment_both_strips_whitespace) {
  auto result = render("foo   {{~ ! comment ~}}   bar", w::map());
  EXPECT_EQ(*result, "foobar");
}

TEST_F(RenderTest, tilde_comment_strips_across_newlines) {
  auto result = render("foo\n   {{~ ! comment ~}}\n   bar", w::map());
  EXPECT_EQ(*result, "foobar");
}

TEST_F(RenderTest, tilde_escaped_comment_left_strips_whitespace) {
  auto result = render("foo   {{~ !-- comment --}}bar", w::map());
  EXPECT_EQ(*result, "foobar");
}

TEST_F(RenderTest, tilde_escaped_comment_right_strips_whitespace) {
  auto result = render("foo{{!-- comment -- ~}}   bar", w::map());
  EXPECT_EQ(*result, "foobar");
}

TEST_F(RenderTest, tilde_escaped_comment_both_strips_whitespace) {
  auto result = render("foo   {{~ !-- comment -- ~}}   bar", w::map());
  EXPECT_EQ(*result, "foobar");
}

// Explicitly triggering a potential stack overflow, and checking the Whisker VM
// will handle it and emit a useful error. Disable this test in ASAN builds, as
// ASAN is not a fan of stack overflows.
#if (                                                \
    !defined(__SANITIZE_ADDRESS__) /* gcc asan */ && \
    !__has_feature(address_sanitizer) /* clang asan */)

TEST_F(RenderTest, infinite_recursion) {
  show_source_backtrace_on_failure(true);
  auto result = render(
      R"(
{{#let partial recursive}}
  {{#let partial inner captures |recursive|}}
Inner {{#partial recursive}}
  {{/let partial}}

Recursive {{#partial inner}}
{{/let partial}}

{{#partial recursive}}
)",
      w::map());

  // Our two partials render each other recursively, so we expect the backtrace
  // to alternate between the two
  std::string expected_backtrace = "";
  for (int i = 0; i < 100; i++) {
    if (i % 2 == 0) {
      expected_backtrace += fmt::format(
          "#{} recursive @ path/to/test.whisker <line:7, col:11>\n", i);
    } else {
      expected_backtrace +=
          fmt::format("#{} inner @ path/to/test.whisker <line:4, col:7>\n", i);
    }
  }

  EXPECT_THAT(
      diagnostics(),
      testing::ElementsAre(
          diagnostic(
              diagnostic_level::error,
              "Maximum recursion depth of 1000 exceeded. Backtrace of deepest 100 frames:",
              path_to_file,
              7),
          error_backtrace(std::move(expected_backtrace))));
}

#endif

TEST_F(RenderTest, partial_trailing_newline_no_tilde) {
  // Plain close tag on standalone line — should be standalone-stripped
  auto result = render(
      "{{#let partial foo}}\n"
      "body\n"
      "}\n"
      "{{/let partial}}\n"
      "{{#partial foo}}",
      w::map());
  // If /let partial is standalone-stripped, the \n after } is preserved
  // but the /let partial line + its newline are removed.
  // Body: body\n}\n then close tag (standalone-stripped).
  // Partial output: body\n}\n (trailing \n from } line)
  EXPECT_EQ(*result, "body\n}\n");
}

TEST_F(RenderTest, partial_trailing_newline_tilde) {
  // {{~ /let partial}} has tilde → NOT standalone. Left tilde eats \n after }.
  // The line's trailing \n is preserved (not standalone-stripped), so it
  // appears in the output before the partial invocation.
  auto result = render(
      "{{#let partial foo}}\n"
      "body\n"
      "}\n"
      "{{~ /let partial}}\n"
      "{{#partial foo}}",
      w::map());
  EXPECT_EQ(*result, "\nbody\n}");
}

TEST_F(RenderTest, tilde_prevents_standalone_left_only) {
  auto result = render(
      "{{#if cond}}\n"
      "content_A\n"
      "{{~ /if cond}}\n"
      "\n"
      "content_B",
      w::map({{"cond", w::true_value}}));
  // {{#if}} is standalone (stripped). content_A is output.
  // {{~ /if}} has tilde → NOT standalone. Left tilde eats \n after
  // content_A. The /if line's trailing \n is preserved (not standalone-
  // stripped).
  EXPECT_EQ(*result, "content_A\n\ncontent_B");
}

TEST_F(RenderTest, tilde_prevents_standalone_right_only) {
  auto result = render(
      "content_A\n"
      "    {{#if cond ~}}\n"
      "content_B\n"
      "    {{/if cond}}",
      w::map({{"cond", w::true_value}}));
  // {{#if ~}} has tilde → NOT standalone. The indent before the tag
  // is preserved. Right tilde eats \n + indent of next line.
  // {{/if}} is standalone (stripped).
  EXPECT_EQ(*result, "content_A\n    content_B\n");
}

TEST_F(RenderTest, tilde_no_tilde_still_standalone) {
  // Verify that tags without tildes are still standalone-stripped.
  auto result = render(
      "    {{#if cond}}\n"
      "content\n"
      "    {{/if cond}}\n",
      w::map({{"cond", w::true_value}}));
  EXPECT_EQ(*result, "content\n");
}

} // namespace whisker
