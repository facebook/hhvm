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

#include <thrift/compiler/whisker/dsl.h>
#include <thrift/compiler/whisker/eval_context.h>
#include <thrift/compiler/whisker/mstch_compat.h>
#include <thrift/compiler/whisker/object.h>
#include <thrift/compiler/whisker/test/render_test_helpers.h>

namespace w = whisker::make;

namespace whisker {

namespace {
ast::variable_component make_variable_component_from_string(std::string name) {
  // The source range is not important for testing here
  ast::identifier property{source_range(), std::move(name)};
  return ast::variable_component{
      source_range(), std::nullopt /* prototype */, std::move(property)};
}

template <typename... Components>
ast::variable_lookup path(Components&&... components) {
  if constexpr (sizeof...(Components) == 0) {
    return ast::variable_lookup{
        source_range(), ast::variable_lookup::this_ref()};
  } else {
    std::vector<ast::variable_component> chain = {
        make_variable_component_from_string(
            std::string(std::forward<Components>(components)))...,
    };
    return ast::variable_lookup{source_range(), std::move(chain)};
  }
}
} // namespace

class MstchCompatTest : public RenderTest {
 public:
  MstchCompatTest() : diags_(diagnostics_engine::ignore_all(src_manager_)) {}

  diagnostics_engine& diags() { return diags_; }

 private:
  source_manager src_manager_;
  diagnostics_engine diags_;
};

TEST_F(MstchCompatTest, basic) {
  mstch_array arr{
      {1,
       std::string("hello world"),
       3,
       true,
       mstch_map({{"key", mstch_array({"nested"})}})}};
  auto converted = from_mstch(arr, diags());

  EXPECT_EQ(
      to_string(converted),
      "mstch::array (size=5)\n"
      "├─ [0] i64(1)\n"
      "├─ [1] 'hello world'\n"
      "├─ [2] i64(3)\n"
      "├─ [3] true\n"
      "╰─ [4] mstch::map (size=1)\n"
      "   ╰─ 'key' → mstch::array (size=1)\n"
      "      ╰─ [0] 'nested'\n");

  // Value equality
  EXPECT_EQ(from_mstch(arr, diags()), from_mstch(arr, diags()));
}

TEST_F(MstchCompatTest, map_lookups) {
  mstch_map m(
      {{"key", mstch_map({{"nested", 1}, {"bool", true}, {"float", 2.0}})},
       {"key2", nullptr}});
  auto converted = from_mstch(std::move(m), diags());

  EXPECT_EQ(
      to_string(converted),
      "mstch::map (size=2)\n"
      "├─ 'key' → mstch::map (size=3)\n"
      "│  ├─ 'bool' → true\n"
      "│  ├─ 'float' → f64(2)\n"
      "│  ╰─ 'nested' → i64(1)\n"
      "╰─ 'key2' → null\n");

  auto ctx = eval_context::with_root_scope(diags(), converted);
  EXPECT_EQ(**ctx.look_up_object({}), converted);
  EXPECT_TRUE((*ctx.look_up_object(path("key")))->is_map());
  EXPECT_FALSE((*ctx.look_up_object(path("key")))->is_array());

  EXPECT_EQ(**ctx.look_up_object(path("key", "nested")), i64(1));
  EXPECT_EQ(**ctx.look_up_object(path("key", "bool")), true);
  EXPECT_EQ(**ctx.look_up_object(path("key", "float")), f64(2.0));
  EXPECT_EQ(**ctx.look_up_object(path("key2")), w::null);

  EXPECT_TRUE(
      has_error<eval_scope_lookup_error>(ctx.look_up_object(path("unknown"))));
  EXPECT_TRUE(
      has_error<eval_property_lookup_error>(
          ctx.look_up_object(path("key", "unknown"))));
}

TEST_F(MstchCompatTest, array_iteration) {
  mstch_node m = mstch_map(
      {{"key", mstch_array({"nested", 1, true, nullptr, 2.0})},
       {"outer",
        mstch_array({mstch_map({{"inner", mstch_array({1, 2, 3})}})})}});
  auto converted = from_mstch(m, diags());

  EXPECT_EQ(
      to_string(converted),
      "mstch::map (size=2)\n"
      "├─ 'key' → mstch::array (size=5)\n"
      "│  ├─ [0] 'nested'\n"
      "│  ├─ [1] i64(1)\n"
      "│  ├─ [2] true\n"
      "│  ├─ [3] null\n"
      "│  ╰─ [4] f64(2)\n"
      "╰─ 'outer' → mstch::array (size=1)\n"
      "   ╰─ [0] mstch::map (size=1)\n"
      "      ╰─ 'inner' → mstch::array (size=3)\n"
      "         ├─ [0] i64(1)\n"
      "         ├─ [1] i64(2)\n"
      "         ╰─ [2] i64(3)\n");

  {
    object_print_options print_opts;
    print_opts.max_depth = 1;
    EXPECT_EQ(
        to_string(converted, print_opts),
        "mstch::map (size=2)\n"
        "├─ 'key' → ...\n"
        "╰─ 'outer' → ...\n");
  }

  {
    object_print_options print_opts;
    print_opts.max_depth = 2;
    EXPECT_EQ(
        to_string(converted, print_opts),
        "mstch::map (size=2)\n"
        "├─ 'key' → mstch::array (size=5)\n"
        "│  ├─ [0] 'nested'\n"
        "│  ├─ [1] i64(1)\n"
        "│  ├─ [2] true\n"
        "│  ├─ [3] null\n"
        "│  ╰─ [4] f64(2)\n"
        "╰─ 'outer' → mstch::array (size=1)\n"
        "   ╰─ [0] ...\n");
  }

  {
    auto ctx = eval_context::with_root_scope(diags(), converted);
    EXPECT_FALSE((*ctx.look_up_object(path("key")))->is_map());
    EXPECT_TRUE((*ctx.look_up_object(path("key")))->is_array());
  }
  {
    strict_printable_types(diagnostic_level::debug);
    auto result = render("{{#key}} {{.}} {{/key}}", converted);
    EXPECT_EQ(*result, " nested  1  true    2 ");
    strict_printable_types(diagnostic_level::error);
  }
  {
    auto result =
        render("{{#outer}}{{#inner}}{{.}}{{/inner}}{{/outer}}", converted);
    EXPECT_EQ(*result, "123");
  }
}

TEST_F(MstchCompatTest, mstch_object) {
  class object_impl : public mstch_object,
                      public std::enable_shared_from_this<object_impl> {
   public:
    object_impl() {
      register_methods(
          this,
          {
              {"foo:bar", &object_impl::foo_bar},
              {"array", &object_impl::array},
              {"error", &object_impl::error_func},
              {"copy", &object_impl::self_copy},
              {"w_i64", &object_impl::w_i64},
              {"w_array", &object_impl::w_array},
              {"w_object", &object_impl::w_object},
              {"w_object_ptr", &object_impl::w_object_ptr},
              {"w_self_handle", &object_impl::w_self_handle},
              {"w_invoke_cpp_only_method",
               &object_impl::w_invoke_cpp_only_method},
              {"volatile", {with_no_caching, &object_impl::volatile_func}},
              {"volatile_counter",
               {with_no_caching,
                [next = 0]() mutable -> mstch_node {
                  return mstch_map({{"next", fmt::format("{}", next++)}});
                }}},
          });
    }

   private:
    mstch_node foo_bar() { return mstch_map({{"key", "value"}}); }
    mstch_node array() { return mstch_array({1, 2, "surprise", 4}); }
    mstch_node self_copy() { return std::make_shared<object_impl>(); }

    mstch_node volatile_func() { return i++; }
    int i = 1;

    mstch_node error_func() { throw std::runtime_error("do not call me"); }

    whisker::i64 w_i64() { return 1; }
    whisker::array::ptr w_array() {
      return whisker::array::of({w::i64(1), w::string("two"), w::i64(3)});
    }
    whisker::object w_object() { return w::string("whisker object"); }
    whisker::object::ptr w_object_ptr() {
      return manage_owned<object>(w::string("whisker object ptr"));
    }
    whisker::native_handle<> w_self_handle() {
      return native_handle<object_impl>(
          shared_from_this(), nullptr /* prototype */);
    }

    std::string cpp_only_method() const { return "hello from C++"; }
    whisker::native_function::ptr w_invoke_cpp_only_method() {
      return dsl::make_function(
          [](dsl::function::context ctx) -> whisker::string {
            ctx.declare_arity(1);
            ctx.declare_named_arguments({});
            return ctx.argument<native_handle<object_impl>>(0)
                ->cpp_only_method();
          });
    }
  };
  auto mstch_obj = apache::thrift::mstch::make_shared_node<object_impl>();
  auto converted = from_mstch(mstch_obj, diags());

  {
    auto ctx = eval_context::with_root_scope(diags(), converted);
    EXPECT_TRUE((*ctx.look_up_object({}))->is_map());
    EXPECT_FALSE((*ctx.look_up_object({}))->is_array());
  }
  {
    auto result = render("{{foo:bar.key}}", converted);
    EXPECT_EQ(*result, "value");
  }
  {
    auto result = render("{{w_i64}}", converted);
    EXPECT_EQ(*result, "1");
  }
  {
    auto result = render(
        "{{#each w_array}}\n"
        "{{.}}\n"
        "{{/each}}",
        converted);
    EXPECT_EQ(
        *result,
        "1\n"
        "two\n"
        "3\n");
  }
  {
    auto result = render("{{w_object}}\n", converted);
    EXPECT_EQ(*result, "whisker object\n");
  }
  {
    auto result = render("{{w_object_ptr}}\n", converted);
    EXPECT_EQ(*result, "whisker object ptr\n");
  }
  {
    auto result =
        render("{{(w_invoke_cpp_only_method w_self_handle)}}\n", converted);
    EXPECT_EQ(*result, "hello from C++\n");
  }
  {
    auto result = render(
        "{{#let c = volatile_counter}}\n"
        "{{volatile_counter.next}}\n"
        "{{c.next}}\n",
        converted);
    EXPECT_EQ(
        *result,
        "1\n"
        "0\n");
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
          testing::HasSubstr("'volatile'"),
          testing::HasSubstr("'foo:bar'"),
          testing::HasSubstr("'array'"),
          testing::HasSubstr("'error'"),
          testing::HasSubstr("'copy'"),
          testing::HasSubstr("'w_i64'")));
}

TEST_F(MstchCompatTest, fallback_through_self) {
  struct t_mock_node {
    std::string bar;
    std::string baz;
  };

  static whisker::dsl::prototype_builder<
      whisker::dsl::named_native_handle<"foo", t_mock_node>>
      prototype_builder;
  prototype_builder.property(
      "bar", [](const t_mock_node& self) { return self.bar; });
  prototype_builder.property(
      "baz", [](const t_mock_node& self) { return self.baz; });
  prototype_builder.property("throws", [](const t_mock_node&) -> std::string {
    throw eval_error("do not call me");
  });

  static std::shared_ptr<const whisker::prototype<t_mock_node>>
      mock_node_prototype = std::move(prototype_builder).make();

  class mstch_mock_node : public mstch_object,
                          public std::enable_shared_from_this<mstch_mock_node> {
   public:
    mstch_mock_node() {
      register_methods(
          this,
          {
              {"foo:bar", &mstch_mock_node::bar},
              {"foo:self", &mstch_mock_node::self_handle},
          });
    }

   private:
    std::shared_ptr<t_mock_node> mock_node_{
        std::make_shared<t_mock_node>("proto_bar", "proto_baz")};

    whisker::native_handle<> self_handle() {
      return native_handle<t_mock_node>(mock_node_, mock_node_prototype);
    }

    std::string bar() { return "mstch_bar"; }
  };

  auto mstch_obj = apache::thrift::mstch::make_shared_node<mstch_mock_node>();
  auto converted = from_mstch(mstch_obj, diags());

  {
    auto result = render("{{foo:bar}} {{foo:self.bar}} {{foo:baz}}", converted);
    EXPECT_EQ(*result, "mstch_bar proto_bar proto_baz");
  }
  {
    // Not resolvable through self, error message should reflect original scope
    auto result = render("{{foo:non_existent}}", converted);
    EXPECT_FALSE(result.has_value());
    EXPECT_THAT(
        diagnostics(),
        testing::ElementsAre(diagnostic(
            diagnostic_level::error,
            "Name 'foo:non_existent' was not found in the current scope.\n"
            "Tried to search through the following scopes:\n"
            "#0 mstch::object\n"
            "├─ 'foo:bar' → ...\n"
            "╰─ 'foo:self' → ...\n"
            "\n"
            "#1 <global scope> (size=0)\n",
            path_to_file,
            1)));
  }
  {
    // Resolvable through self, but property function throws an exception
    // Error message should reflect original scope
    auto result = render("{{foo:throws}}", converted);
    EXPECT_FALSE(result.has_value());
    EXPECT_THAT(
        diagnostics(),
        testing::ElementsAre(diagnostic(
            diagnostic_level::error,
            "Name 'foo:throws' was not found in the current scope.\n"
            "Cause: do not call me\n"
            "Tried to search through the following scopes:\n"
            "#0 mstch::object\n"
            "├─ 'foo:bar' → ...\n"
            "╰─ 'foo:self' → ...\n"
            "\n"
            "#1 <global scope> (size=0)\n",
            path_to_file,
            1)));
  }
}

} // namespace whisker
