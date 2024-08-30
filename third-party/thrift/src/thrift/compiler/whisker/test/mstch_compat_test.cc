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

#include <thrift/compiler/whisker/eval_context.h>
#include <thrift/compiler/whisker/mstch_compat.h>
#include <thrift/compiler/whisker/object.h>
#include <thrift/compiler/whisker/test/render_test_helpers.h>

namespace w = whisker::make;

namespace whisker {

class MstchCompatTest : public RenderTest {};

TEST_F(MstchCompatTest, basic) {
  mstch_array arr{
      {1,
       std::string("hello world"),
       3,
       true,
       mstch_map({{"key", mstch_array({"nested"})}})}};
  auto converted = from_mstch(arr);

  EXPECT_EQ(
      to_string(converted),
      "mstch::array (size=5)\n"
      "`-[0]\n"
      "  |-i64(1)\n"
      "`-[1]\n"
      "  |-'hello world'\n"
      "`-[2]\n"
      "  |-i64(3)\n"
      "`-[3]\n"
      "  |-true\n"
      "`-[4]\n"
      "  |-mstch::map (size=1)\n"
      "  | `-'key'\n"
      "  |   |-mstch::array (size=1)\n"
      "  |   | `-[0]\n"
      "  |   |   |-'nested'\n");

  // Value equality
  EXPECT_EQ(from_mstch(arr), from_mstch(arr));
}

TEST_F(MstchCompatTest, map_lookups) {
  mstch_map m(
      {{"key", mstch_map({{"nested", 1}, {"bool", true}, {"float", 2.0}})},
       {"key2", nullptr}});
  auto converted = from_mstch(std::move(m));

  EXPECT_EQ(
      to_string(converted),
      "mstch::map (size=2)\n"
      "`-'key'\n"
      "  |-mstch::map (size=3)\n"
      "  | `-'bool'\n"
      "  |   |-true\n"
      "  | `-'float'\n"
      "  |   |-f64(2)\n"
      "  | `-'nested'\n"
      "  |   |-i64(1)\n"
      "`-'key2'\n"
      "  |-null\n");

  eval_context ctx{converted};
  EXPECT_EQ(ctx.lookup_object({}), converted);
  EXPECT_TRUE(is_mstch_map(ctx.lookup_object({"key"})));
  EXPECT_FALSE(is_mstch_array(ctx.lookup_object({"key"})));
  EXPECT_FALSE(is_mstch_object(ctx.lookup_object({"key"})));

  EXPECT_EQ(ctx.lookup_object({"key", "nested"}), i64(1));
  EXPECT_EQ(ctx.lookup_object({"key", "bool"}), true);
  EXPECT_EQ(ctx.lookup_object({"key", "float"}), f64(2.0));
  EXPECT_EQ(ctx.lookup_object({"key2"}), w::null);

  EXPECT_THROW(ctx.lookup_object({"unknown"}), eval_scope_lookup_error);
  EXPECT_THROW(
      ctx.lookup_object({"key", "unknown"}), eval_property_lookup_error);
}

TEST_F(MstchCompatTest, array_iteration) {
  mstch_node m = mstch_map(
      {{"key", mstch_array({"nested", 1, true, nullptr, 2.0})},
       {"outer",
        mstch_array({mstch_map({{"inner", mstch_array({1, 2, 3})}})})}});
  auto converted = from_mstch(m);

  EXPECT_EQ(
      to_string(converted),
      "mstch::map (size=2)\n"
      "`-'key'\n"
      "  |-mstch::array (size=5)\n"
      "  | `-[0]\n"
      "  |   |-'nested'\n"
      "  | `-[1]\n"
      "  |   |-i64(1)\n"
      "  | `-[2]\n"
      "  |   |-true\n"
      "  | `-[3]\n"
      "  |   |-null\n"
      "  | `-[4]\n"
      "  |   |-f64(2)\n"
      "`-'outer'\n"
      "  |-mstch::array (size=1)\n"
      "  | `-[0]\n"
      "  |   |-mstch::map (size=1)\n"
      "  |   | `-'inner'\n"
      "  |   |   |-mstch::array (size=3)\n"
      "  |   |   | `-[0]\n"
      "  |   |   |   |-i64(1)\n"
      "  |   |   | `-[1]\n"
      "  |   |   |   |-i64(2)\n"
      "  |   |   | `-[2]\n"
      "  |   |   |   |-i64(3)\n");

  {
    eval_context ctx{converted};
    EXPECT_TRUE(is_mstch_array(ctx.lookup_object({"key"})));
    EXPECT_FALSE(is_mstch_map(ctx.lookup_object({"key"})));
    EXPECT_FALSE(is_mstch_object(ctx.lookup_object({"key"})));
  }
  {
    strict_printable_types = diagnostic_level::debug;
    auto result = render("{{#key}} {{.}} {{/key}}", converted);
    EXPECT_EQ(*result, " nested  1  true    2 ");
    strict_printable_types = std::nullopt;
  }
  {
    auto result =
        render("{{#outer}}{{#inner}}{{.}}{{/inner}}{{/outer}}", converted);
    EXPECT_EQ(*result, "123");
  }
}

TEST_F(MstchCompatTest, mstch_object) {
  class object_impl : public mstch_object {
   public:
    object_impl() {
      register_cached_methods(
          this,
          {{"foo:bar", &object_impl::foo_bar},
           {"array", &object_impl::array},
           {"error", &object_impl::error_func},
           {"copy", &object_impl::self_copy}});
      register_volatile_methods(
          this, {{"volatile", &object_impl::volatile_func}});
    }

   private:
    mstch_node foo_bar() { return mstch_map({{"key", "value"}}); }
    mstch_node array() { return mstch_array({1, 2, "surprise", 4}); }
    mstch_node self_copy() { return std::make_shared<object_impl>(); }

    mstch_node volatile_func() { return i++; }
    int i = 1;

    mstch_node error_func() { throw std::runtime_error("do not call me"); }
  };
  auto mstch_obj = apache::thrift::mstch::make_shared_node<object_impl>();
  auto converted = from_mstch(mstch_obj);
  EXPECT_EQ(converted, from_mstch(mstch_obj));

  {
    eval_context ctx{converted};
    EXPECT_TRUE(is_mstch_object(ctx.lookup_object({})));
    EXPECT_FALSE(is_mstch_map(ctx.lookup_object({})));
    EXPECT_FALSE(is_mstch_array(ctx.lookup_object({})));
  }
  {
    auto result = render("{{foo:bar.key}}", converted);
    EXPECT_EQ(*result, "value");
  }
  {
    auto result = render(
        "{{#array}}{{!\n"
        "}}volatile({{volatile}}) element({{.}})\n"
        "{{/array}}",
        converted);
    EXPECT_EQ(
        *result,
        "volatile(1) element(1)\n"
        "volatile(2) element(2)\n"
        "volatile(3) element(surprise)\n"
        "volatile(4) element(4)\n");
  }
  {
    auto result = render(
        "{{#copy}}{{#array}}{{!\n"
        "}}volatile({{volatile}}) element({{.}})\n"
        "{{/array}}{{/copy}}",
        converted);
    EXPECT_EQ(
        *result,
        "volatile(1) element(1)\n"
        "volatile(2) element(2)\n"
        "volatile(3) element(surprise)\n"
        "volatile(4) element(4)\n");
  }

  // The mstch::object properties are stored in an unordered_map
  EXPECT_THAT(
      to_string(converted),
      testing::AllOf(
          testing::HasSubstr("mstch::object"),
          testing::HasSubstr("`-'volatile'\n"),
          testing::HasSubstr("`-'foo:bar'\n"),
          testing::HasSubstr("`-'array'\n"),
          testing::HasSubstr("`-'error'\n"),
          testing::HasSubstr("`-'copy'\n")));
}

} // namespace whisker
