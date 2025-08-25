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

#pragma once
#include <gtest/gtest.h>

#include <thrift/lib/cpp/util/EnumUtils.h>
#include <thrift/lib/cpp2/Thrift.h>
#include <thrift/lib/cpp2/debug_thrift_data_difference/debug.h>
#include <thrift/lib/cpp2/type/Tag.h>

namespace apache::thrift {

template <class T>
::testing::AssertionResult thriftEqualHelper(
    const char* left, const char* right, const T& a, const T& b) {
  static_assert(
      is_thrift_class_v<T> || util::is_thrift_enum_v<T>,
      "Thrift equal helper only works for thrift structs, unions, exceptions, and enums.");
  ::testing::AssertionResult result(false);
  if (facebook::thrift::debug_thrift_data_difference(
          a,
          b,
          facebook::thrift::make_debug_output_callback(result, left, right))) {
    return ::testing::AssertionResult(true);
  } else {
    return result;
  }
}

template <class Tag, class T>
::testing::AssertionResult thriftEqualHelperTag(
    const char*,
    const char* left,
    const char* right,
    const Tag&,
    const T& a,
    const T& b) {
  static_assert(
      type::is_thrift_type_tag_v<Tag>, "Tag must be a thrift type tag.");
  ::testing::AssertionResult result(false);
  if (facebook::thrift::debug_thrift_data_difference<Tag>(
          a,
          b,
          facebook::thrift::make_debug_output_callback(result, left, right))) {
    return ::testing::AssertionResult(true);
  } else {
    return result;
  }
}

} // namespace apache::thrift

// Macros for testing Thrift definitions: structs, exceptions, unions, and
// enums. Use `EXPECT_THRIFT_TAG_EQ` to directly compare two fields after
// getting relevant type tag from `op::get_type_tag`.
#define EXPECT_THRIFT_EQ(a, b) \
  EXPECT_PRED_FORMAT2(::apache::thrift::thriftEqualHelper, a, b)

#define ASSERT_THRIFT_EQ(a, b) \
  ASSERT_PRED_FORMAT2(::apache::thrift::thriftEqualHelper, a, b)

#define EXPECT_THRIFT_TAG_EQ(tag, a, b) \
  EXPECT_PRED_FORMAT3(::apache::thrift::thriftEqualHelperTag, tag, a, b)

#define ASSERT_THRIFT_TAG_EQ(tag, a, b) \
  ASSERT_PRED_FORMAT3(::apache::thrift::thriftEqualHelperTag, tag, a, b)

#define CO_EXPECT_THRIFT_EQ(a, b) \
  CO_EXPECT_PRED_FORMAT2(::apache::thrift::thriftEqualHelper, a, b)

#define CO_ASSERT_THRIFT_EQ(a, b) \
  CO_ASSERT_PRED_FORMAT2(::apache::thrift::thriftEqualHelper, a, b)

#define CO_EXPECT_THRIFT_TAG_EQ(tag, a, b) \
  CO_EXPECT_PRED_FORMAT3(::apache::thrift::thriftEqualHelperTag, tag, a, b)

#define CO_ASSERT_THRIFT_TAG_EQ(tag, a, b) \
  CO_ASSERT_PRED_FORMAT3(::apache::thrift::thriftEqualHelperTag, tag, a, b)
