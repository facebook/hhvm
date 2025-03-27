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

#include <folly/portability/GTest.h>
#include <thrift/lib/cpp2/BadFieldAccess.h>
#include <thrift/test/gen-cpp2/UnionFieldRef_types.h>
using namespace std;

namespace apache::thrift::test {

TEST(UnionFieldTest, basic) {
  Basic a;

  EXPECT_EQ(a.getType(), Basic::Type::__EMPTY__);
  EXPECT_FALSE(a.int64_ref());
  EXPECT_THROW(a.int64_ref().value(), bad_union_field_access);
  EXPECT_THROW(a.get_int64(), bad_union_field_access);
  EXPECT_FALSE(a.list_i32_ref());
  EXPECT_THROW(a.list_i32_ref().value(), bad_union_field_access);
  EXPECT_THROW(a.get_list_i32(), bad_union_field_access);
  EXPECT_FALSE(a.str_ref());
  EXPECT_THROW(a.str_ref().value(), bad_union_field_access);
  EXPECT_THROW(a.get_str(), bad_union_field_access);

  const int64_t int64 = 42LL << 42;
  a.int64_ref() = int64;
  EXPECT_EQ(a.getType(), Basic::Type::int64);
  EXPECT_TRUE(a.int64_ref());
  EXPECT_EQ(a.int64_ref().value(), int64);
  EXPECT_EQ(a.get_int64(), int64);
  EXPECT_FALSE(a.list_i32_ref());
  EXPECT_THROW(a.list_i32_ref().value(), bad_union_field_access);
  EXPECT_THROW(a.get_list_i32(), bad_union_field_access);
  EXPECT_FALSE(a.str_ref());
  EXPECT_THROW(a.str_ref().value(), bad_union_field_access);
  EXPECT_THROW(a.get_str(), bad_union_field_access);

  const vector<int32_t> list_i32 = {3, 1, 2};
  a.list_i32_ref() = list_i32;
  EXPECT_EQ(a.getType(), Basic::Type::list_i32);
  EXPECT_FALSE(a.int64_ref());
  EXPECT_THROW(a.int64_ref().value(), bad_union_field_access);
  EXPECT_THROW(a.get_int64(), bad_union_field_access);
  EXPECT_TRUE(a.list_i32_ref());
  EXPECT_EQ(a.list_i32_ref().value(), list_i32);
  EXPECT_EQ(a.get_list_i32(), list_i32);
  EXPECT_FALSE(a.str_ref());
  EXPECT_THROW(a.str_ref().value(), bad_union_field_access);
  EXPECT_THROW(a.get_str(), bad_union_field_access);

  const string str = "foo";
  a.str_ref() = str;
  EXPECT_EQ(a.getType(), Basic::Type::str);
  EXPECT_FALSE(a.int64_ref());
  EXPECT_THROW(a.int64_ref().value(), bad_union_field_access);
  EXPECT_THROW(a.get_int64(), bad_union_field_access);
  EXPECT_FALSE(a.list_i32_ref());
  EXPECT_THROW(a.list_i32_ref().value(), bad_union_field_access);
  EXPECT_THROW(a.get_list_i32(), bad_union_field_access);
  EXPECT_TRUE(a.str_ref());
  EXPECT_EQ(a.str_ref().value(), str);
  EXPECT_EQ(a.get_str(), str);
}

TEST(UnionFieldTest, operator_deref) {
  Basic a;
  EXPECT_THROW(*a.int64_ref(), bad_union_field_access);
  EXPECT_THROW(*a.str_ref(), bad_union_field_access);
  a.int64_ref() = 42;
  EXPECT_EQ(*a.int64_ref(), 42);
  EXPECT_THROW(*a.str_ref(), bad_union_field_access);
  a.str_ref() = "foo";
  EXPECT_EQ(*a.str_ref(), "foo");
  EXPECT_THROW(*a.int64_ref(), bad_union_field_access);
}

TEST(UnionFieldTest, operator_assign) {
  Basic a;
  a.int64_ref() = 4;
  a.list_i32_ref() = {1, 2};

  // `folly::StringPiece` has explicit support for explicit conversion to
  // `std::string`-like types, make sure that keeps working here.
  folly::StringPiece lvalue = "lvalue";
  a.str_ref() = lvalue;
  a.str_ref() = folly::StringPiece("xvalue");
}

TEST(UnionFieldTest, duplicate_type) {
  DuplicateType a;

  EXPECT_EQ(a.getType(), DuplicateType::Type::__EMPTY__);
  EXPECT_FALSE(a.str1_ref());
  EXPECT_THROW(a.str1_ref().value(), bad_union_field_access);
  EXPECT_FALSE(a.list_i32_ref());
  EXPECT_THROW(a.list_i32_ref().value(), bad_union_field_access);
  EXPECT_FALSE(a.str2_ref());
  EXPECT_THROW(a.str2_ref().value(), bad_union_field_access);

  a.str1_ref() = string(1000, '1');
  EXPECT_EQ(a.getType(), DuplicateType::Type::str1);
  EXPECT_TRUE(a.str1_ref());
  EXPECT_EQ(a.str1_ref().value(), string(1000, '1'));
  EXPECT_FALSE(a.list_i32_ref());
  EXPECT_THROW(a.list_i32_ref().value(), bad_union_field_access);
  EXPECT_FALSE(a.str2_ref());
  EXPECT_THROW(a.str2_ref().value(), bad_union_field_access);

  a.list_i32_ref() = vector<int32_t>(1000, 2);
  EXPECT_EQ(a.getType(), DuplicateType::Type::list_i32);
  EXPECT_FALSE(a.str1_ref());
  EXPECT_THROW(a.str1_ref().value(), bad_union_field_access);
  EXPECT_TRUE(a.list_i32_ref());
  EXPECT_EQ(a.list_i32_ref().value(), vector<int32_t>(1000, 2));
  EXPECT_FALSE(a.str2_ref());
  EXPECT_THROW(a.str2_ref().value(), bad_union_field_access);

  a.str2_ref() = string(1000, '3');
  EXPECT_EQ(a.getType(), DuplicateType::Type::str2);
  EXPECT_FALSE(a.str1_ref());
  EXPECT_THROW(a.str1_ref().value(), bad_union_field_access);
  EXPECT_FALSE(a.list_i32_ref());
  EXPECT_THROW(a.list_i32_ref().value(), bad_union_field_access);
  EXPECT_TRUE(a.str2_ref());
  EXPECT_EQ(a.str2_ref().value(), string(1000, '3'));

  a.str1_ref() = string(1000, '4');
  EXPECT_EQ(a.getType(), DuplicateType::Type::str1);
  EXPECT_TRUE(a.str1_ref());
  EXPECT_EQ(a.str1_ref().value(), string(1000, '4'));
  EXPECT_FALSE(a.list_i32_ref());
  EXPECT_THROW(a.list_i32_ref().value(), bad_union_field_access);
  EXPECT_FALSE(a.str2_ref());
  EXPECT_THROW(a.str2_ref().value(), bad_union_field_access);

  a.str1_ref() = string(1000, '5');
  EXPECT_EQ(a.getType(), DuplicateType::Type::str1);
  EXPECT_TRUE(a.str1_ref());
  EXPECT_EQ(a.str1_ref().value(), string(1000, '5'));
  EXPECT_FALSE(a.list_i32_ref());
  EXPECT_THROW(a.list_i32_ref().value(), bad_union_field_access);
  EXPECT_FALSE(a.str2_ref());
  EXPECT_THROW(a.str2_ref().value(), bad_union_field_access);
}

TEST(UnionFieldTest, const_union) {
  {
    const Basic a;
    EXPECT_EQ(a.getType(), Basic::Type::__EMPTY__);
    EXPECT_FALSE(a.int64_ref());
    EXPECT_THROW(a.int64_ref().value(), bad_union_field_access);
    EXPECT_FALSE(a.list_i32_ref());
    EXPECT_THROW(a.list_i32_ref().value(), bad_union_field_access);
    EXPECT_FALSE(a.str_ref());
    EXPECT_THROW(a.str_ref().value(), bad_union_field_access);
  }

  {
    Basic b;
    const string str = "foo";
    b.str_ref() = str;

    const Basic& a = b;
    EXPECT_EQ(a.getType(), Basic::Type::str);
    EXPECT_FALSE(a.int64_ref());
    EXPECT_THROW(a.int64_ref().value(), bad_union_field_access);
    EXPECT_FALSE(a.list_i32_ref());
    EXPECT_THROW(a.list_i32_ref().value(), bad_union_field_access);
    EXPECT_TRUE(a.str_ref());
    EXPECT_EQ(a.str_ref().value(), str);
  }
}
TEST(UnionFieldTest, emplace) {
  DuplicateType a;

  EXPECT_EQ(a.str1_ref().emplace(5, '0'), "00000");
  EXPECT_EQ(*a.str1_ref(), "00000");
  EXPECT_TRUE(a.str1_ref().has_value());

  EXPECT_EQ(a.str2_ref().emplace({'1', '2', '3', '4', '5'}), "12345");
  EXPECT_EQ(*a.str2_ref(), "12345");
  EXPECT_TRUE(a.str2_ref().has_value());

  const vector<int32_t> list_i32 = {3, 1, 2};
  EXPECT_EQ(a.list_i32_ref().emplace(list_i32), list_i32);
  EXPECT_EQ(*a.list_i32_ref(), list_i32);
  EXPECT_TRUE(a.list_i32_ref().has_value());
}

TEST(UnionFieldTest, ensure) {
  Basic a;

  EXPECT_FALSE(a.str_ref());
  EXPECT_EQ(a.str_ref().ensure(), "");
  EXPECT_TRUE(a.str_ref());
  EXPECT_EQ(a.str_ref().emplace("123"), "123");
  EXPECT_EQ(a.str_ref().ensure(), "123");
  EXPECT_EQ(*a.str_ref(), "123");

  EXPECT_EQ(a.int64_ref().ensure(), 0);
  EXPECT_FALSE(a.str_ref());
}

TEST(UnionFieldTest, member_of_pointer_operator) {
  Basic a;
  EXPECT_THROW(a.list_i32_ref()->push_back(3), bad_union_field_access);
  a.list_i32_ref().emplace({1, 2});
  a.list_i32_ref()->push_back(3);
  EXPECT_EQ(a.list_i32_ref(), (vector<int32_t>{1, 2, 3}));
}

TEST(UnionFieldTest, comparison) {
  auto test = [](auto i) {
    Basic a;
    auto ref = a.int64_ref();

    ref = i;

    EXPECT_LE(ref, i + 1);
    EXPECT_LE(ref, i);
    EXPECT_LE(i, ref);
    EXPECT_LE(i - 1, ref);

    EXPECT_LT(ref, i + 1);
    EXPECT_LT(i - 1, ref);

    EXPECT_GT(ref, i - 1);
    EXPECT_GT(i + 1, ref);

    EXPECT_GE(ref, i - 1);
    EXPECT_GE(ref, i);
    EXPECT_GE(i, ref);
    EXPECT_GE(i + 1, ref);

    EXPECT_EQ(ref, i);
    EXPECT_EQ(i, ref);

    EXPECT_NE(ref, i - 1);
    EXPECT_NE(i - 1, ref);
  };

  {
    SCOPED_TRACE("same type");
    test(int64_t(10));
  }
  {
    SCOPED_TRACE("different type");
    test('a');
  }
}

TEST(UnionFieldTest, field_ref_api) {
  Basic a;
  a.str_ref() = "foo";
  EXPECT_EQ(*a.str_ref(), "foo");
  EXPECT_EQ(*as_const(a).str_ref(), "foo");
  EXPECT_EQ(*std::move(a).str_ref(), "foo");
  EXPECT_EQ(*std::move(as_const(a)).str_ref(), "foo");
}

TEST(UnionFieldTest, TreeNode) {
  TreeNode root;
  root.nodes_ref().emplace(2);
  (*root.nodes_ref())[0].data_ref() = 10;
  (*root.nodes_ref())[1].data_ref() = 20;

  EXPECT_EQ(root.nodes_ref()->size(), 2);
  EXPECT_EQ((*root.nodes_ref())[0].data_ref(), 10);
  EXPECT_EQ((*root.nodes_ref())[1].data_ref(), 20);

  EXPECT_THROW(*root.data_ref(), bad_union_field_access);
  EXPECT_THROW(*(*root.nodes_ref())[0].nodes_ref(), bad_union_field_access);
  EXPECT_THROW(*(*root.nodes_ref())[1].nodes_ref(), bad_union_field_access);
}

TEST(UnionFieldTest, CppRef) {
  CppRef s;

  static_assert(std::is_same_v<
                folly::remove_cvref_t<decltype(s.get_cppref())>,
                std::unique_ptr<CppRef>>);

  static_assert(std::is_same_v<
                folly::remove_cvref_t<decltype(s.get_shared_mutable())>,
                std::shared_ptr<CppRef>>);

  static_assert(std::is_same_v<
                folly::remove_cvref_t<decltype(s.get_shared_const())>,
                std::shared_ptr<const std::string>>);

  s.str_ref() = "foo";

  EXPECT_EQ(std::as_const(s).str_ref(), "foo");
  EXPECT_EQ(s.str_ref(), "foo");

  EXPECT_THROW(*std::as_const(s).cppref_ref(), bad_union_field_access);
  EXPECT_THROW(*s.cppref_ref(), bad_union_field_access);

  s.cppref_ref().emplace().str_ref() = "ref";

  EXPECT_EQ(std::as_const(s).cppref_ref()->str_ref(), "ref");
  EXPECT_EQ(s.cppref_ref()->str_ref(), "ref");
  EXPECT_THROW(*s.shared_const_ref(), bad_union_field_access);

  s.shared_mutable_ref().emplace().str_ref() = "shared";

  EXPECT_EQ(std::as_const(s).shared_mutable_ref()->str_ref(), "shared");
  EXPECT_EQ(s.shared_mutable_ref()->str_ref(), "shared");
  EXPECT_THROW(*s.cppref_ref(), bad_union_field_access);

  s.box_ref().emplace().str_ref() = "box";

  EXPECT_EQ(std::as_const(s).box_ref()->str_ref(), "box");
  EXPECT_EQ(s.box_ref()->str_ref(), "box");
  EXPECT_THROW(*s.shared_mutable_ref(), bad_union_field_access);

  s.shared_const_ref().emplace("shared_const");

  EXPECT_EQ(std::as_const(s).shared_const_ref(), "shared_const");
  EXPECT_EQ(s.shared_const_ref(), "shared_const");
  EXPECT_THROW(*s.box_ref(), bad_union_field_access);
}
} // namespace apache::thrift::test
