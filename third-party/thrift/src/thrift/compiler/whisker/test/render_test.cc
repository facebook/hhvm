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
#include <thrift/compiler/whisker/test/render_test_helpers.h>

namespace w = whisker::make;

namespace whisker {

namespace {

class empty_native_object : public native_object {};

class array_like_native_object
    : public native_object,
      public native_object::array_like,
      public std::enable_shared_from_this<array_like_native_object> {
 public:
  explicit array_like_native_object(array values)
      : values_(std::move(values)) {}

  native_object::array_like::ptr as_array_like() const override {
    return shared_from_this();
  }
  std::size_t size() const override { return values_.size(); }
  object::ptr at(std::size_t index) const override {
    return manage_as_static(values_.at(index));
  }

 private:
  array values_;
};

class map_like_native_object
    : public native_object,
      public native_object::map_like,
      public std::enable_shared_from_this<map_like_native_object> {
 public:
  explicit map_like_native_object(map values) : values_(std::move(values)) {}

  native_object::map_like::ptr as_map_like() const override {
    return shared_from_this();
  }

  object::ptr lookup_property(std::string_view id) const override {
    if (auto value = values_.find(id); value != values_.end()) {
      return manage_as_static(value->second);
    }
    return nullptr;
  }

 private:
  map values_;
};

/**
 * When looking up a property, always returns a whisker::string that is the
 * property name repeated twice.
 */
class double_property_name
    : public native_object,
      public native_object::map_like,
      public std::enable_shared_from_this<double_property_name> {
 public:
  native_object::map_like::ptr as_map_like() const override {
    return shared_from_this();
  }

  object::ptr lookup_property(std::string_view id) const override {
    if (auto cached = cached_.find(id); cached != cached_.end()) {
      return manage_as_static(cached->second);
    }
    auto [result, inserted] =
        cached_.insert({std::string(id), w::string(fmt::format("{0}{0}", id))});
    assert(inserted);
    return manage_as_static(result->second);
  }

  mutable std::map<std::string, object, std::less<>> cached_;
};

} // namespace

TEST_F(RenderTest, basic) {
  auto result = render(
      "Some text {{foo}} More text", w::map({{"foo", w::string("bar")}}));
  EXPECT_EQ(*result, "Some text bar More text");
}

TEST_F(RenderTest, variable_missing_in_scope) {
  auto result = render("Some text {{foo}} More text", w::map({}));
  EXPECT_FALSE(result.has_value());
  EXPECT_THAT(
      diagnostics(),
      testing::ElementsAre(diagnostic(
          diagnostic_level::error,
          "Name 'foo' was not found in the current scope. Tried to search through the following scopes:\n"
          "#0 map (size=0)\n"
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
          "Object 'foo' has no property named 'bar'. The object with the missing property is:\n"
          "map (size=1)\n"
          "`-'baz'\n"
          "  |-'qux'\n",
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
              "Name 'value' was not found in the current scope. Tried to search through the following scopes:\n"
              "#0 map (size=1)\n"
              "`-'oops'\n"
              "  |-null\n"
              "\n"
              "#1 map (size=1)\n"
              "`-'factorials'\n"
              "  |-...\n"
              "\n"
              "#2 <global scope> (size=0)\n",
              path_to_file,
              3),
          error_backtrace("#0 path/to/test.whisker <line:3, col:5>\n")));
}

TEST_F(RenderTest, section_block_array_iterable_native_object) {
  auto factorials = w::map(
      {{"factorials",
        w::make_native_object<array_like_native_object>(array(
            {w::map({{"value", w::i64(1)}}),
             w::map({{"value", w::string("2")}}),
             w::map({{"value", w::i64(6)}}),
             w::map({{"value", w::i64(24)}}),
             w::map({{"value", w::i64(120)}})}))}});
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

TEST_F(RenderTest, section_block_map_like_native_object) {
  auto factorials = w::map(
      {{"factorials",
        w::make_native_object<map_like_native_object>(map(
            {{"first", w::i64(1)},
             {"second", w::string("2")},
             {"third", w::i64(6)}}))}});
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

TEST_F(RenderTest, section_block_pointless_native_object) {
  auto context =
      w::map({{"pointless", w::make_native_object<empty_native_object>()}});
  {
    strict_boolean_conditional(diagnostic_level::info);
    auto result = render(
        "{{#pointless}}\n"
        "Should not be rendered\n"
        "{{/pointless}}",
        context);
    EXPECT_EQ(*result, "");
  }
  {
    strict_boolean_conditional(diagnostic_level::error);
    auto result = render(
        "{{#pointless}}\n"
        "Should not be rendered\n"
        "{{/pointless}}",
        context);
    EXPECT_FALSE(result.has_value());
    EXPECT_THAT(
        diagnostics(),
        testing::ElementsAre(diagnostic(
            diagnostic_level::error,
            "Condition 'pointless' is not a boolean. The encountered value is:\n"
            "<native_object>\n",
            path_to_file,
            1)));
  }
}

TEST_F(RenderTest, section_block_empty_array) {
  auto result = render(
      "The factorial function looks like:{{#factorials}}\n"
      "{{value}}\n"
      "{{/factorials}}",
      w::map({{"factorials", w::array({})}}));
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
class add : public native_function {
  object::ptr invoke(context ctx) override {
    ctx.declare_named_arguments({"negate"});
    const bool negate = [&] {
      auto arg = ctx.named_argument<boolean>("negate", context::optional);
      return arg.value_or(false);
    }();
    i64 result = 0;
    for (std::size_t i = 0; i < ctx.arity(); ++i) {
      result += ctx.argument<i64>(i);
    }
    return manage_owned<object>(w::i64(negate ? -result : result));
  }
};

/**
 * Returns a boolean indicating if two i64 are equal or not.
 */
class i64_eq : public native_function {
  object::ptr invoke(context ctx) override {
    ctx.declare_arity(2);
    ctx.declare_named_arguments({});
    i64 a = ctx.argument<i64>(0);
    i64 b = ctx.argument<i64>(1);
    return manage_owned<object>(w::boolean(a == b));
  }
};

/**
 * Concatenates a sequences of strings together. If the 'sep' argument is
 * provided, then it is used as the delimiter between elements.
 */
class str_concat : public native_function {
  object::ptr invoke(context ctx) override {
    ctx.declare_named_arguments({"sep"});
    const std::string sep = [&] {
      auto arg = ctx.named_argument<string>("sep", context::optional);
      return arg == nullptr ? "" : *arg;
    }();
    string result;
    for (std::size_t i = 0; i < ctx.arity(); ++i) {
      if (i != 0) {
        result += sep;
      }
      result += *ctx.argument<string>(i);
    }
    return manage_owned<object>(w::string(std::move(result)));
  }
};

/**
 * Returns the length of an array.
 */
class array_len : public native_function {
  object::ptr invoke(context ctx) override {
    ctx.declare_arity(1);
    ctx.declare_named_arguments({});
    auto len = i64(ctx.argument<array>(0).size());
    return manage_owned<object>(w::i64(len));
  }
};

/**
 * Dynamically accesses a property by name in a map, throwing an error if not
 * present.
 */
class map_get : public native_function {
  object::ptr invoke(context ctx) override {
    ctx.declare_arity(1);
    ctx.declare_named_arguments({"key"});
    auto m = ctx.argument<map>(0);
    auto key = ctx.named_argument<string>("key", context::required);

    if (object::ptr result = m.lookup_property(*key)) {
      return result;
    }
    ctx.error("Key '{}' not found.", *key);
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
              "Argument at index 1 is of an unexpected type.",
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
              "Named argument 'negate' is of an unexpected type.",
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
  const array arr{w::i64(1), w::string("foo"), w::i64(100)};
  const auto context = w::map(
      {{"array", w::array(array(arr))},
       {"array_like",
        w::make_native_object<array_like_native_object>(array(arr))},
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
            "Argument at index 0 is not an array or array-like native_object.",
            path_to_file,
            1)));
  }
}

TEST_F(RenderTest, user_defined_function_array_like_named_argument) {
  class describe_array_len : public native_function {
    object::ptr invoke(context ctx) override {
      ctx.declare_arity(0);
      ctx.declare_named_arguments({"input"});
      auto len = i64(ctx.named_argument<array>("input")->size());
      return manage_owned<object>(w::string(fmt::format("length is {}", len)));
    }
  };

  const array arr{w::i64(1), w::string("foo"), w::i64(100)};
  const auto context = w::map(
      {{"array", w::array(array(arr))},
       {"array_like",
        w::make_native_object<array_like_native_object>(array(arr))},
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
            "Named argument 'input' is not an array or array-like native_object.",
            path_to_file,
            1)));
  }
}

TEST_F(RenderTest, user_defined_function_map_like_argument) {
  const map m{{"a", w::i64(1)}, {"b", w::string("foo")}, {"c", w::i64(100)}};
  const auto context = w::map(
      {{"map", w::map(map(m))},
       {"map_like", w::make_native_object<map_like_native_object>(map(m))},
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
            "Argument at index 0 is not a map or map-like native_object.",
            path_to_file,
            1)));
  }
}

TEST_F(RenderTest, user_defined_function_map_like_named_argument) {
  class describe_map_get : public native_function {
    object::ptr invoke(context ctx) override {
      ctx.declare_arity(0);
      ctx.declare_named_arguments({"input", "key"});
      auto m = ctx.named_argument<map>("input", context::required);
      auto key = ctx.named_argument<string>("key", context::required);

      std::string_view result =
          m->lookup_property(*key) == nullptr ? "missing" : "present";
      return manage_owned<object>(
          w::string(fmt::format("map element is {}", result)));
    }
  };

  const map m{{"a", w::i64(1)}, {"b", w::string("foo")}, {"c", w::i64(100)}};
  const auto context = w::map(
      {{"map", w::map(map(m))},
       {"map_like", w::make_native_object<map_like_native_object>(map(m))},
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
            "Named argument 'input' is not a map or map-like native_object.",
            path_to_file,
            1)));
  }
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
      w::map({}));
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
          "`-[0]\n"
          "  |-i64(0)\n",
          path_to_file,
          1)));
}

TEST_F(RenderTest, with_map_like_native_object) {
  auto result = render(
      "{{#with doubler}}\n"
      "{{foo}} {{bar}}\n"
      "{{#with .}}\n"
      "{{baz}}\n"
      "{{/with}}\n"
      "{{/with}}\n",
      w::map({{"doubler", w::make_native_object<double_property_name>()}}));
  EXPECT_THAT(diagnostics(), testing::IsEmpty());
  EXPECT_EQ(
      *result,
      "foofoo barbar\n"
      "bazbaz\n");
}

TEST_F(RenderTest, with_not_map_like_native_object) {
  show_source_backtrace_on_failure(true);
  auto result = render(
      "{{#with empty}}\n"
      "{{/with}}\n",
      w::map({{"empty", w::make_native_object<empty_native_object>()}}));
  EXPECT_FALSE(result.has_value());
  EXPECT_THAT(
      diagnostics(),
      testing::ElementsAre(
          diagnostic(
              diagnostic_level::error,
              "Expression 'empty' is a native_object which is not map-like. The encountered value is:\n"
              "<native_object>\n",
              path_to_file,
              1),
          error_backtrace("#0 path/to/test.whisker <line:1, col:9>\n")));
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

TEST_F(RenderTest, each_block_array_with_capture_and_index) {
  auto result = render(
      "The factorial function looks like:\n"
      "{{#each factorials as |value index|}}\n"
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

TEST_F(RenderTest, each_block_array_iterable_native_object) {
  auto factorials = w::map(
      {{"factorials",
        w::make_native_object<array_like_native_object>(array(
            {w::map({{"value", w::i64(1)}}),
             w::map({{"value", w::string("2")}}),
             w::map({{"value", w::i64(6)}}),
             w::map({{"value", w::i64(24)}}),
             w::map({{"value", w::i64(120)}})}))}});
  auto result = render(
      "The factorial function looks like:\n"
      "{{#each factorials as |entry index|}}\n"
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
      w::map({{"factorials", w::array({})}}));
  EXPECT_EQ(
      *result,
      "The factorial function looks like:\n"
      "Hey! Where did they go?\n");
}

TEST_F(RenderTest, each_block_array_nested) {
  auto result = render(
      "{{#each . as |e i|}}\n"
      "{{i}}({{#each e.b as |e j|}} {{i}}-{{j}}({{e.c}}) {{/each}})\n"
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

TEST_F(RenderTest, each_block_pointless_native_object) {
  auto context =
      w::map({{"pointless", w::make_native_object<empty_native_object>()}});

  auto result = render(
      "{{#each pointless}}\n"
      "Should not be rendered\n"
      "{{/each}}",
      context);
  EXPECT_FALSE(result.has_value());
  EXPECT_THAT(
      diagnostics(),
      testing::ElementsAre(diagnostic(
          diagnostic_level::error,
          "Expression 'pointless' is a native_object which is not array-like. The encountered value is:\n"
          "<native_object>\n",
          path_to_file,
          1)));
}

TEST_F(RenderTest, printable_types_strict_failure) {
  {
    auto result = render(R"({{-42}} {{"hello"}})", w::map({}));
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
    auto result = render("{{true}}", w::map({}));
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
    auto result = render("{{null}}", w::map({}));
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
    auto result = render("{{array}}", w::map({{"array", w::array({})}}));
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
    auto result = render("{{map}}", w::map({{"map", w::map({})}}));
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
  {
    auto result = render(
        "{{native}}",
        w::map({{"native", w::make_native_object<empty_native_object>()}}));
    EXPECT_THAT(
        diagnostics(),
        testing::ElementsAre(diagnostic(
            diagnostic_level::error,
            "Object named 'native' is not printable. The encountered value is:\n"
            "<native_object>\n",
            path_to_file,
            1)));
    EXPECT_FALSE(result.has_value());
  }
}

TEST_F(RenderTest, printable_types_warning) {
  strict_printable_types(diagnostic_level::warning);

  {
    auto result = render(R"({{-42}} {{"hello"}})", w::map({}));
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
    auto result = render("{{false}}", w::map({}));
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
    auto result = render("{{null}}", w::map({}));
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

  // Arrays, maps, native_objects are not printable in any case
  {
    auto result = render("{{array}}", w::map({{"array", w::array({})}}));
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
    auto result = render("{{map}}", w::map({{"map", w::map({})}}));
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
  {
    auto result = render(
        "{{native}}",
        w::map({{"native", w::make_native_object<empty_native_object>()}}));
    EXPECT_THAT(
        diagnostics(),
        testing::ElementsAre(diagnostic(
            diagnostic_level::error,
            "Object named 'native' is not printable. The encountered value is:\n"
            "<native_object>\n",
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

  // Arrays, maps, native_objects are not printable in any case
  {
    auto result = render("{{array}}", w::map({{"array", w::array({})}}));
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
    auto result = render("{{map}}", w::map({{"map", w::map({})}}));
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
  {
    auto result = render(
        "{{native}}",
        w::map({{"native", w::make_native_object<empty_native_object>()}}));
    EXPECT_FALSE(result.has_value());
    EXPECT_THAT(
        diagnostics(),
        testing::ElementsAre(diagnostic(
            diagnostic_level::error,
            "Object named 'native' is not printable. The encountered value is:\n"
            "<native_object>\n",
            path_to_file,
            1)));
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
      partials({{"some/file/path", "{{value}} (from partial)\n"}}));
  EXPECT_EQ(
      *result,
      "1 (from partial)\n"
      "2 (from partial)\n"
      "foo (from partial)\n"
      "bar (from partial)\n");
}

TEST_F(RenderTest, partials_missing) {
  auto result = render(
      "{{> some/other/path}}",
      w::map({}),
      partials({{"some/file/path", "{{value}} (from partial)"}}));
  EXPECT_THAT(
      diagnostics(),
      testing::ElementsAre(diagnostic(
          diagnostic_level::error,
          "Partial with path 'some/other/path' was not found",
          path_to_file,
          1)));
  EXPECT_FALSE(result.has_value());
}

TEST_F(RenderTest, partials_no_resolver) {
  auto result = render("{{> some/file/path}}", w::map({}));
  EXPECT_THAT(
      diagnostics(),
      testing::ElementsAre(diagnostic(
          diagnostic_level::error,
          "No partial resolver was provided. Cannot resolve partial with path 'some/file/path'",
          path_to_file,
          1)));
  EXPECT_FALSE(result.has_value());
}

TEST_F(RenderTest, partials_indentation) {
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
      partials(
          {{"some/file/path",
            "{{value}} (from partial)\n"
            "A second line\n"}}));
  EXPECT_EQ(
      *result,
      "before\n"
      "  2 (from partial)\n"
      "  A second line\n"
      "after\n"
      "before\n"
      "  foo (from partial)\n"
      "  A second line\n"
      "after\n"
      "before\n"
      "  bar (from partial)\n"
      "  A second line\n"
      "after\n");
}

TEST_F(RenderTest, partials_indentation_nested) {
  auto result = render(
      "{{#array}}\n"
      "before\n"
      "  {{>partial-1 }}\n"
      "after\n"
      "{{/array}}",
      w::map(
          {{"array",
            w::array({
                w::map({{"value", w::i64(2)}}),
                w::map({{"value", w::string("foo")}}),
            })}}),
      partials(
          {{"partial-1",
            "{{value}} (from partial-1)\n"
            "  nested: {{> partial-2}}\n"},
           {"partial-2",
            "{{value}} (from partial-2)\n"
            "second line from partial-2"}}));
  EXPECT_EQ(
      *result,
      "before\n"
      "  2 (from partial-1)\n"
      "    nested: 2 (from partial-2)\n"
      "  second line from partial-2\n"
      "after\n"
      "before\n"
      "  foo (from partial-1)\n"
      "    nested: foo (from partial-2)\n"
      "  second line from partial-2\n"
      "after\n");
}

TEST_F(RenderTest, partial_apply_preserves_whitespace_offset) {
  auto result = render(
      " \t {{> partial-1}} \t \n",
      w::map({{"value", w::i64(42)}}),
      partials(
          {{"partial-1",
            "\t 1\n"
            " \t {{>partial-2}} \n"},
           {"partial-2",
            "\t 2\n"
            " \t {{>partial-3}}! \n"},
           {"partial-3", "\t{{value}}"}}));
  EXPECT_EQ(
      *result,
      " \t \t 1\n"
      " \t  \t \t 2\n"
      " \t  \t  \t \t42! \n");
}

TEST_F(RenderTest, partial_nested_undefined_variable_trace) {
  show_source_backtrace_on_failure(true);
  auto result = render(
      "\n"
      "{{> partial-1}}\n",
      w::map({}),
      partials(
          {{"partial-1",
            "\n"
            "\t {{>partial-2}}\n"},
           {"partial-2", "foo {{>partial-3}}\n"},
           {"partial-3", "{{undefined}}"}}));
  EXPECT_FALSE(result.has_value());
  EXPECT_THAT(
      diagnostics(),
      testing::ElementsAre(
          diagnostic(
              diagnostic_level::error,
              "Name 'undefined' was not found in the current scope. Tried to search through the following scopes:\n"
              "#0 map (size=0)\n"
              "\n"
              "#1 <global scope> (size=0)\n",
              "partial-3", // file name
              1),
          error_backtrace("#0 partial-3 <line:1, col:3>\n"
                          "#1 partial-2 <line:1, col:5>\n"
                          "#2 partial-1 <line:2, col:3>\n"
                          "#3 path/to/test.whisker <line:2, col:1>\n")));
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
      partials({{"ineligible", "{{value}} (from partial)\n"}}));
  EXPECT_EQ(
      *result,
      "| This Is\n"
      "   5 (from partial)\n"
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
      partials({}),
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
      partials({}),
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
      w::map({}));
  EXPECT_THAT(diagnostics(), testing::IsEmpty());
  EXPECT_EQ(*result, "This is all one line");

  // Test that the pragma is scoped to the active partial/template.
  result = render(
      "{{#pragma ignore-newlines}}\n"
      "This\n"
      " is\n"
      "{{> partial }}\n"
      " all\n"
      " one\n"
      " line\n",
      w::map({}),
      partials({{"partial", "\nnot\n!\n"}}));
  EXPECT_THAT(diagnostics(), testing::IsEmpty());
  EXPECT_EQ(*result, "This is\nnot\n!\n all one line");
}

} // namespace whisker
