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

#include <gtest/gtest.h>
#include <thrift/lib/cpp2/visitation/ShrinkToFit.h>
#include <thrift/lib/cpp2/visitation/test/gen-cpp2/ShrinkToFit_types.h>
#include <thrift/lib/cpp2/visitation/test/gen-cpp2/ShrinkToFit_visitation.h>

namespace apache::thrift::test {
namespace {

TEST(ShrinkToFitTest, ListFields) {
  Fields obj;
  obj.listField().ensure().reserve(42);
  obj.structField().ensure().listField().ensure().reserve(42);

  EXPECT_EQ(obj.listField()->capacity(), 42);
  EXPECT_EQ(obj.structField()->listField()->capacity(), 42);

  apache::thrift::shrink_to_fit(obj);

  EXPECT_EQ(obj.listField()->capacity(), 0);
  EXPECT_EQ(obj.structField()->listField()->capacity(), 0);
}

template <class T>
struct ShrinkToFitListFieldsTest : ::testing::Test {};

TYPED_TEST_CASE_P(ShrinkToFitListFieldsTest);
TYPED_TEST_P(ShrinkToFitListFieldsTest, ListTest) {
  TypeParam obj;
  obj.field().ensure().emplace_back();
  auto& listField = obj.field()->front().listField().ensure();
  listField.reserve(42);

  EXPECT_EQ(listField.capacity(), 42);

  apache::thrift::shrink_to_fit(obj);

  EXPECT_EQ(listField.capacity(), 0);
}
REGISTER_TYPED_TEST_CASE_P(ShrinkToFitListFieldsTest, ListTest);
using ListFields = ::testing::Types<
    StructWithListFieldVector,
    StructWithListFieldList,
    StructWithListFieldDeque>;
INSTANTIATE_TYPED_TEST_CASE_P(
    ShrinkToFitTest, ShrinkToFitListFieldsTest, ListFields);

template <class T>
struct ShrinkToFitSetFieldsTest : ::testing::Test {};

TYPED_TEST_CASE_P(ShrinkToFitSetFieldsTest);
TYPED_TEST_P(ShrinkToFitSetFieldsTest, SetTest) {
  TypeParam obj;
  StructWithListField s;
  s.listField().ensure().push_back(42);
  s.listField()->reserve(42);
  obj.field().ensure().insert(std::move(s));
  for (auto& structListField : *obj.field()) {
    EXPECT_EQ(structListField.listField()->at(0), 42);
    EXPECT_EQ(structListField.listField()->capacity(), 42);
  }

  apache::thrift::shrink_to_fit(obj);

  for (auto& structListField : *obj.field()) {
    EXPECT_EQ(structListField.listField()->at(0), 42);
    EXPECT_EQ(structListField.listField()->capacity(), 1);
  }
}
REGISTER_TYPED_TEST_CASE_P(ShrinkToFitSetFieldsTest, SetTest);
using SetFields = ::testing::Types<
    StructWithListFieldSet,
    StructWithListFieldUnorderedSet,
    StructWithListFieldF14FastSet,
    StructWithListFieldF14VectorSet,
    StructWithListFieldSortedVectorSet>;
INSTANTIATE_TYPED_TEST_CASE_P(
    ShrinkToFitTest, ShrinkToFitSetFieldsTest, SetFields);

template <class T>
struct ShrinkToFitMapFieldsTest : ::testing::Test {};

TYPED_TEST_CASE_P(ShrinkToFitMapFieldsTest);
TYPED_TEST_P(ShrinkToFitMapFieldsTest, MapTest) {
  TypeParam obj;
  obj.field().ensure().insert({1, {}});
  obj.field()->at(1).listField().ensure().reserve(42);
  obj.field()->at(1).listField()->push_back(42);

  EXPECT_EQ(obj.field()->at(1).listField()->capacity(), 42);
  EXPECT_EQ(obj.field()->at(1).listField()->at(0), 42);

  apache::thrift::shrink_to_fit(obj);

  EXPECT_EQ(obj.field()->at(1).listField()->capacity(), 1);
  EXPECT_EQ(obj.field()->at(1).listField()->at(0), 42);
}
REGISTER_TYPED_TEST_CASE_P(ShrinkToFitMapFieldsTest, MapTest);
using MapFields = ::testing::Types<
    StructWithListFieldMap,
    StructWithListFieldUnorderedMap,
    StructWithListFieldF14FastMap,
    StructWithListFieldF14VectorMap,
    StructWithListFieldSortedVectorMap>;
INSTANTIATE_TYPED_TEST_CASE_P(
    ShrinkToFitTest, ShrinkToFitMapFieldsTest, MapFields);

TEST(ShrinkToFitTest, DoubleStructWithListFieldMapTest) {
  DoubleStructWithListFieldMap obj;
  StructWithListField s1, s2;
  s1.listField().ensure().emplace_back();
  s2.listField().ensure().emplace_back();
  s1.listField()->reserve(42);
  s2.listField()->reserve(42);

  obj.field().ensure().emplace(std::move(s1), std::move(s2));

  EXPECT_EQ(obj.field()->begin()->first.listField()->capacity(), 42);
  EXPECT_EQ(obj.field()->begin()->second.listField()->capacity(), 42);

  apache::thrift::shrink_to_fit(obj);

  EXPECT_EQ(obj.field()->begin()->first.listField()->capacity(), 1);
  EXPECT_EQ(obj.field()->begin()->second.listField()->capacity(), 1);
}

TEST(ShrinkToFitTest, NestedListField) {
  Fields obj;
  obj.nestedListField().ensure().reserve(42);
  obj.nestedListField()->emplace_back();
  obj.nestedListField()->at(0).reserve(42);

  EXPECT_EQ(obj.nestedListField()->capacity(), 42);
  EXPECT_EQ(obj.nestedListField()->at(0).capacity(), 42);

  apache::thrift::shrink_to_fit(obj);

  EXPECT_EQ(obj.nestedListField()->capacity(), 1);
  EXPECT_EQ(obj.nestedListField()->at(0).capacity(), 0);
}

TEST(ShrinkToFitTest, NestedStructListField) {
  Fields obj;
  obj.nestedStructListField().ensure().reserve(42);
  obj.nestedStructListField()->emplace_back();
  obj.nestedStructListField()->at(0).reserve(42);
  obj.nestedStructListField()->at(0).emplace_back();
  obj.nestedStructListField()->at(0).at(0).listField().ensure().reserve(42);

  EXPECT_EQ(obj.nestedStructListField()->capacity(), 42);
  EXPECT_EQ(obj.nestedStructListField()->at(0).capacity(), 42);
  EXPECT_EQ(
      obj.nestedStructListField()->at(0).at(0).listField()->capacity(), 42);

  apache::thrift::shrink_to_fit(obj);

  EXPECT_EQ(obj.nestedStructListField()->capacity(), 1);
  EXPECT_EQ(obj.nestedStructListField()->at(0).capacity(), 1);
  EXPECT_EQ(
      obj.nestedStructListField()->at(0).at(0).listField()->capacity(), 0);
}

TEST(ShrinkToFitTest, NestedStructListMapField) {
  Fields obj;
  obj.nestedStructListMapField().ensure().insert({1, {}});
  obj.nestedStructListMapField()->at(1).reserve(42);
  obj.nestedStructListMapField()->at(1).emplace_back();
  obj.nestedStructListMapField()->at(1).at(0).listField().ensure().reserve(42);

  EXPECT_EQ(obj.nestedStructListMapField()->at(1).capacity(), 42);
  EXPECT_EQ(
      obj.nestedStructListMapField()->at(1).at(0).listField()->capacity(), 42);

  apache::thrift::shrink_to_fit(obj);

  EXPECT_EQ(obj.nestedStructListMapField()->at(1).capacity(), 1);
  EXPECT_EQ(
      obj.nestedStructListMapField()->at(1).at(0).listField()->capacity(), 0);
}

TEST(ShrinkToFitTest, CppRefListFields) {
  Fields obj;
  obj.listFieldRef()->reserve(42);
  obj.structFieldRef()->listField().ensure().reserve(42);

  EXPECT_EQ(obj.listFieldRef()->capacity(), 42);
  EXPECT_EQ(obj.structFieldRef()->listField()->capacity(), 42);

  apache::thrift::shrink_to_fit(obj);

  EXPECT_EQ(obj.listFieldRef()->capacity(), 0);
  EXPECT_EQ(obj.structFieldRef()->listField()->capacity(), 0);
}

TEST(ShrinkToFitTest, OptionalCppRefListFields) {
  Fields obj;
  obj.optListFieldRef() = std::make_shared<std::vector<int>>();
  obj.optStructFieldRef() = std::make_shared<StructWithListField>();
  obj.optListFieldRef()->reserve(42);
  obj.optStructFieldRef()->listField().ensure().reserve(42);

  EXPECT_EQ(obj.optListFieldRef()->capacity(), 42);
  EXPECT_EQ(obj.optStructFieldRef()->listField()->capacity(), 42);

  apache::thrift::shrink_to_fit(obj);

  EXPECT_EQ(obj.optListFieldRef()->capacity(), 0);
  EXPECT_EQ(obj.optStructFieldRef()->listField()->capacity(), 0);
}

TEST(ShrinkToFitTest, BoxListFields) {
  Fields obj;
  obj.listFieldBoxRef() = std::vector<int>();
  obj.structFieldBoxRef() = StructWithListField();
  obj.listFieldBoxRef()->reserve(42);
  obj.structFieldBoxRef()->listField().ensure().reserve(42);

  EXPECT_EQ(obj.listFieldBoxRef()->capacity(), 42);
  EXPECT_EQ(obj.structFieldBoxRef()->listField()->capacity(), 42);

  apache::thrift::shrink_to_fit(obj);

  EXPECT_EQ(obj.listFieldBoxRef()->capacity(), 0);
  EXPECT_EQ(obj.structFieldBoxRef()->listField()->capacity(), 0);
}

} // namespace
} // namespace apache::thrift::test

size_t std::hash<apache::thrift::test::StructWithListField>::operator()(
    const apache::thrift::test::StructWithListField& s) const {
  return s.listField()->size();
}
