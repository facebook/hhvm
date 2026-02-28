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

#include <thrift/lib/cpp2/test/frozen2/view_helper/gen-cpp2/view_helper_layouts.h>
#include <thrift/lib/cpp2/test/frozen2/view_helper/gen-cpp2/view_helper_types.h>

#include <thrift/lib/cpp2/frozen/FrozenUtil.h>
#include <thrift/lib/cpp2/util/Frozen2ViewHelpers.h>

#include <gtest/gtest.h>

using namespace ::apache::thrift::frozen;
using namespace ::test::frozen2;

#define ASSERT_VIEW_EQ(OBJ, MAPPED, NAME)                       \
  ASSERT_EQ(                                                    \
      ViewHelper<decltype(MAPPED.NAME())>::thaw(MAPPED.NAME()), \
      *OBJ.NAME##_ref())

TEST(ViewHelperTest, TestThaw) {
  TestStruct strct;
  strct.i32Field() = 0xBAD;
  strct.strField() = "foo";
  strct.doubleField() = 1.5;
  strct.boolField() = true;
  strct.listField() = {"bar", "baz"};
  strct.mapField() = {
      {0, "a"},
      {1, "b"},
  };
  strct.enumField() = TestEnum::Foo;

  std::string frozen;
  freezeToString(strct, frozen);
  auto mapped = mapFrozen<TestStruct>(std::move(frozen));
  ASSERT_VIEW_EQ(strct, mapped, i32Field);
  ASSERT_VIEW_EQ(strct, mapped, strField);
  ASSERT_VIEW_EQ(strct, mapped, doubleField);
  ASSERT_VIEW_EQ(strct, mapped, boolField);
  ASSERT_VIEW_EQ(strct, mapped, listField);
  ASSERT_VIEW_EQ(strct, mapped, mapField);
  ASSERT_VIEW_EQ(strct, mapped, enumField);
}
