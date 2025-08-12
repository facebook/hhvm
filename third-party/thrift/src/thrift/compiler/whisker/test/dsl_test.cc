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

#include <string>
#include <type_traits>

namespace whisker::dsl {

template <fixed_string Q, typename... Cases>
using make_poly = make_polymorphic_native_handle<Q, Cases...>;
template <fixed_string Q, typename... Cases>
using poly = polymorphic_native_handle<Q, Cases...>;

TEST(DslTest, make_polymorphic_native_handle) {
  struct t_A {
    virtual ~t_A() = default;
  };
  struct t_B : t_A {};
  struct t_B2 : t_B {};
  struct t_C : t_A {};
  struct t_C2 : t_C {};

  using C2 = make_poly<"C2", t_C2>;
  using C = make_poly<"C", t_C, C2>;
  using B2 = make_poly<"B2", t_B2>;
  using B = make_poly<"B", t_B, B2>;
  using A = make_poly<"A", t_A, B, C>;

  EXPECT_TRUE((std::is_same_v<C2, poly<"C2", t_C2>>));
  EXPECT_TRUE((std::is_same_v<C, poly<"C", t_C, t_C2>>));
  EXPECT_TRUE((std::is_same_v<B2, poly<"B2", t_B2>>));
  EXPECT_TRUE((std::is_same_v<B, poly<"B", t_B, t_B2>>));
  EXPECT_TRUE((std::is_same_v<A, poly<"A", t_A, t_B, t_B2, t_C, t_C2>>));
}

TEST(DslTest, make_named_non_polymorphic_native_handle) {
  struct t_struct {};

  using handle = whisker::dsl::named_native_handle<"foo", t_struct>;
  EXPECT_TRUE((std::is_same_v<t_struct, handle::element_type>));

  prototype_builder<handle> proto_builder;

  // This tests named_native_handle gets successfully converted to native_handle
  // for self argument resolution, by ensuring the "property" template is
  // instantiated and compiles.
  proto_builder.property(
      "prop", [](const t_struct&) { return whisker::null(); });

  std::shared_ptr<const prototype<t_struct>> proto =
      std::move(proto_builder).make();

  EXPECT_EQ("foo", proto->name());
  EXPECT_NE(nullptr, proto->find_descriptor("prop"));
}

TEST(DslTest, test_alias_inheritance) {
  struct t_Base {
    virtual ~t_Base() = default;
  };
  struct t_Child : public t_Base {
    ~t_Child() override;
  };

  using h_Child = make_poly<"child", t_Child>;
  using h_Base = make_poly<"base", t_Base, h_Child>;

  EXPECT_TRUE((std::is_same_v<h_Child, poly<"child", t_Child>>));
  EXPECT_TRUE((std::is_same_v<h_Base, poly<"base", t_Base, t_Child>>));

  prototype_builder<h_Base> proto_builder_base;
  std::shared_ptr<const prototype<t_Base>> proto_base =
      std::move(proto_builder_base).make();

  EXPECT_EQ("base", proto_base->name()) << "Base should take its handle's name";

  prototype_builder<h_Child> proto_builder_child =
      prototype_builder<h_Child>::extends(proto_base);
  std::shared_ptr<const prototype<t_Child>> proto_child =
      std::move(proto_builder_child).make();

  EXPECT_EQ("child", proto_child->name())
      << "Extending a parent type should use the child handle's name";

  prototype_builder<h_Child> proto_builder_child_extension =
      prototype_builder<h_Child>::extends(proto_child);
  std::shared_ptr<const prototype<t_Child>> proto_child_extension =
      std::move(proto_builder_child_extension).make();

  EXPECT_EQ("child", proto_child_extension->name())
      << "Extending the same type should inherit its name";
}

} // namespace whisker::dsl
