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

#include <thrift/lib/cpp2/frozen/Frozen.h>
#include <thrift/lib/cpp2/frozen/test/gen-cpp2/Adapter_layouts.h>

using namespace apache::thrift::frozen;
using namespace facebook::thrift::test::frozen::adapter;

namespace {

AdaptedFields makeAdaptedFields() {
  AdaptedNested nested;
  *nested.ids() = {7, 8};

  AdaptedNested listed;
  *listed.ids() = {9};

  AdaptedFields fields;
  *fields.ids() = {1, 2, 3};
  *fields.widened() = 42;
  fields.optionalIds() = {4, 5};
  *fields.nested() = nested;
  *fields.nestedList() = {listed};
  *fields.plain() = "hello";
  return fields;
}

} // namespace

TEST(FrozenAdapter, RoundTripsAdaptedFields) {
  const auto fields = makeAdaptedFields();

  EXPECT_EQ(fields, freeze(fields).thaw());
}

TEST(FrozenAdapter, ViewReadsAdaptedFieldsInPlace) {
  const auto fields = makeAdaptedFields();

  auto view = freeze(fields);

  // The adapted type is a plain vector, so the frozen view indexes into it
  // without thawing the whole struct.
  const std::vector<std::int64_t> expectedIds{1, 2, 3};
  ASSERT_EQ(expectedIds.size(), view.ids().size());
  EXPECT_EQ(expectedIds[0], view.ids()[0]);
  EXPECT_EQ(expectedIds, view.ids().thaw());
  EXPECT_EQ(42, view.widened());
  EXPECT_EQ("hello", view.plain());
}

TEST(FrozenAdapter, RoundTripsNestedAndOptionalAdaptedFields) {
  const auto fields = makeAdaptedFields();

  auto view = freeze(fields);

  const std::vector<std::int64_t> expectedNestedIds{7, 8};
  const std::vector<std::int64_t> expectedListedIds{9};
  const std::vector<std::int64_t> expectedOptionalIds{4, 5};
  EXPECT_EQ(expectedNestedIds, view.nested().ids().thaw());
  ASSERT_EQ(1, view.nestedList().size());
  EXPECT_EQ(expectedListedIds, view.nestedList()[0].ids().thaw());
  ASSERT_TRUE(view.optionalIds().has_value());
  EXPECT_EQ(expectedOptionalIds, view.optionalIds()->thaw());
}

TEST(FrozenAdapter, RoundTripsUnsetOptionalAdaptedField) {
  AdaptedFields fields;
  *fields.ids() = {1};

  auto thawed = freeze(fields).thaw();

  EXPECT_FALSE(thawed.optionalIds().has_value());
  EXPECT_EQ(fields, thawed);
}
