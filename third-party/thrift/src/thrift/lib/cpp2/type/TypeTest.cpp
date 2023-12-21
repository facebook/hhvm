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

#include <thrift/lib/cpp2/type/Type.h>

#include <list>
#include <stdexcept>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include <folly/portability/GTest.h>
#include <thrift/lib/cpp2/type/BaseType.h>
#include <thrift/lib/cpp2/type/ThriftType.h>

namespace apache::thrift::type {
namespace {

struct TypeTestCase {
  Type type;
  BaseType expected_type;
};

template <typename Tag, typename... Args>
TypeTestCase test(Args&&... args) {
  return {Type::create<Tag>(std::forward<Args>(args)...), base_type_v<Tag>};
}

std::vector<TypeTestCase> getUniqueNonContainerTypes() {
  return {
      test<void_t>(),
      test<bool_t>(),
      test<byte_t>(),
      test<i16_t>(),
      test<i32_t>(),
      test<i64_t>(),
      test<float_t>(),
      test<double_t>(),
      test<string_t>(),
      test<binary_t>(),
      test<struct_c>("d.c/p/MyStruct"),
      test<struct_c>("d.c/p/MyOtherStruct"),
      test<struct_c>("d.c/p/MyNamed"),
      test<union_c>("d.c/p/MyUnion"),
      test<union_c>("d.c/p/MyOtherUnion"),
      test<union_c>("d.c/p/MyNamed"),
      test<exception_c>("d.c/p/MyException"),
      test<exception_c>("d.c/p/MyOtherExcpetion"),
      test<exception_c>("d.c/p/MyNamed"),
      test<enum_c>("d.c/p/MyEnum"),
      test<enum_c>("d.c/p/MyOtherEnum"),
      test<enum_c>("d.c/p/MyNamed"),
  };
}

std::vector<TypeTestCase> getUniqueContainerTypesOf(
    const std::vector<TypeTestCase>& keys,
    const std::vector<TypeTestCase>& values) {
  std::vector<TypeTestCase> result;
  for (const auto& val : values) {
    result.emplace_back(test<list_c>(val.type));
  }
  for (const auto& key : keys) {
    result.emplace_back(test<set_c>(key.type));
    for (const auto& val : values) {
      result.emplace_back(test<map_c>(key.type, val.type));
    }
  }
  return result;
}

std::vector<TypeTestCase> getUniqueTypes() {
  auto unique = getUniqueNonContainerTypes();
  auto uniqueContainer = getUniqueContainerTypesOf(unique, unique);
  unique.insert(unique.end(), uniqueContainer.begin(), uniqueContainer.end());
  return unique;
}

TEST(TypeTest, Equality) {
  auto unique = getUniqueTypes();
  EXPECT_FALSE(unique.empty());
  for (size_t i = 0; i < unique.size(); ++i) {
    for (size_t j = 0; j < unique.size(); ++j) {
      EXPECT_EQ(unique[i].type == unique[j].type, i == j);
      EXPECT_EQ(unique[i].type != unique[j].type, i != j);
    }
  }
}

TEST(TypeTest, BaseType) {
  auto unique = getUniqueTypes();
  for (const auto& test : unique) {
    EXPECT_EQ(test.type.baseType(), test.expected_type);
  }
}

struct MyStruct {
  constexpr static auto __fbthrift_thrift_uri() {
    return "domain.com/my/package/MyStruct";
  }
};

struct MyAdapter {};

TEST(TypeTest, Create) {
  EXPECT_EQ(Type::get<list<i16_t>>(), Type::create<list_c>(Type::get<i16_t>()));

  EXPECT_EQ(Type::get<set<i16_t>>(), Type::create<set_c>(Type::get<i16_t>()));
  EXPECT_EQ(
      Type::get<set<list<i16_t>>>(),
      Type::create<set_c>(Type::get<list<i16_t>>()));

  EXPECT_EQ(
      (Type::get<map<string_t, binary_t>>()),
      Type::create<map_c>(Type::get<string_t>(), Type::get<binary_t>()));
  EXPECT_EQ(
      (Type::get<map<set<string_t>, list<binary_t>>>()),
      Type::create<map_c>(
          Type::get<set<string_t>>(), Type::get<list<binary_t>>()));
  EXPECT_EQ(
      Type::get<struct_t<MyStruct>>(),
      Type::create<struct_c>("domain.com/my/package/MyStruct"));
  EXPECT_EQ(
      Type::get<union_t<MyStruct>>(),
      Type::create<union_c>("domain.com/my/package/MyStruct"));
  EXPECT_EQ(
      Type::get<exception_t<MyStruct>>(),
      Type::create<exception_c>("domain.com/my/package/MyStruct"));
}

TEST(TypeTest, Adapted) {
  // Adapted is ignored.
  EXPECT_EQ((Type::get<adapted<MyAdapter, void_t>>()), Type::get<void_t>());
}

TEST(TypeTest, CppType) {
  // CppType is ignored.
  EXPECT_EQ((Type::get<cpp_type<void, void_t>>()), Type::get<void_t>());
}

TEST(TypeTest, ImplicitConversion) {
  EXPECT_EQ(list<i16_t>{}, Type::create<list_c>(i16_t{}));
  Type type = i16_t{};
  EXPECT_EQ(type, Type::get<i16_t>());
  type = void_t{};
  EXPECT_NE(type, Type::get<i16_t>());
  EXPECT_EQ(type, Type());
}

TEST(TypeTest, NameValidation) {
  EXPECT_THROW(Type::create<enum_c>("BadName"), std::invalid_argument);
  EXPECT_THROW(Type::create<struct_c>("BadName"), std::invalid_argument);
  EXPECT_THROW(Type::create<union_c>("BadName"), std::invalid_argument);
  EXPECT_THROW(Type::create<exception_c>("BadName"), std::invalid_argument);
}

TEST(TypeTest, isFull) {
  Type type;
  // empty type
  EXPECT_FALSE(type.isFull());

  auto& t = type.toThrift();
  t.name()->set_listType();

  // only checks fullness of params, if present
  EXPECT_TRUE(type.isFull());
  // ensures that appropriate number of params are present
  EXPECT_FALSE(type.isValid());

  TypeStruct params;
  t.params().ensure().push_back(params);

  // invalid params
  EXPECT_FALSE(type.isFull());
  EXPECT_FALSE(type.isValid());

  t.params()[0].name()->set_boolType();
  // valid params
  EXPECT_TRUE(type.isFull());
  EXPECT_TRUE(type.isValid());

  params.name()->set_i32Type();
  t.params().ensure().push_back(params);
  // only checks fullness of params, if present
  EXPECT_TRUE(type.isFull());
  // ensures that appropriate number of params are present
  // list only needs one param
  EXPECT_FALSE(type.isValid());

  t.params()->clear();
  TypeUri uri;
  uri.set_uri("BadName");
  t.name()->set_structType() = uri;
  // only checks if uri is present
  EXPECT_TRUE(type.isFull());
  // ensures that uri is valid
  EXPECT_FALSE(type.isValid());

  type = Type::create<struct_c>("domain.com/my/package/MyStruct");
  // ensures that uri is valid
  EXPECT_TRUE(type.isValid());
}
} // namespace
} // namespace apache::thrift::type
