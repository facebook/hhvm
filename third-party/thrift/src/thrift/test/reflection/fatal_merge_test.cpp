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

#include <thrift/lib/cpp2/debug_thrift_data_difference/debug.h>
#include <thrift/lib/cpp2/debug_thrift_data_difference/diff.h>
#include <thrift/lib/cpp2/reflection/merge.h>
#include <thrift/lib/cpp2/reflection/pretty_print.h>
#include <thrift/test/reflection/gen-cpp2/fatal_merge_constants.h>
#include <thrift/test/reflection/gen-cpp2/fatal_merge_fatal_types.h>
#include <thrift/test/reflection/gen-cpp2/fatal_merge_types.h>

using namespace apache::thrift::test;
using facebook::thrift::debug_thrift_data_difference;
using facebook::thrift::make_debug_output_callback;

namespace {

class FatalMergeTest : public testing::Test {};

} // namespace

#define TEST_GROUP(name, constant)                                     \
  TEST_F(FatalMergeTest, name##_copy) {                                \
    const auto& example = fatal_merge_constants::constant();           \
    auto src = *example.src(), dst = *example.dst();                   \
    apache::thrift::merge_into(src, dst);                              \
    EXPECT_TRUE(debug_thrift_data_difference(                          \
        *example.exp(), dst, make_debug_output_callback(LOG(ERROR)))); \
    EXPECT_TRUE(debug_thrift_data_difference(                          \
        *example.src(), src, make_debug_output_callback(LOG(ERROR)))); \
  }                                                                    \
  TEST_F(FatalMergeTest, name##_move) {                                \
    const auto& example = fatal_merge_constants::constant();           \
    auto src = *example.src(), dst = *example.dst();                   \
    apache::thrift::merge_into(std::move(src), dst);                   \
    EXPECT_TRUE(debug_thrift_data_difference(                          \
        *example.exp(), dst, make_debug_output_callback(LOG(ERROR)))); \
    EXPECT_TRUE(debug_thrift_data_difference(                          \
        *example.nil(), src, make_debug_output_callback(LOG(ERROR)))); \
  }

TEST_GROUP(structure, kBasicExample)
TEST_GROUP(optional, kBasicOptionalExample)
TEST_GROUP(list, kBasicListExample)
TEST_GROUP(set, kBasicSetExample)
TEST_GROUP(map, kBasicMapExample)
TEST_GROUP(nested_structure, kNestedExample)
TEST_GROUP(nested_ref_unique, kNestedRefUniqueExample)
TEST_GROUP(nested_ref_shared, kNestedRefSharedExample)
TEST_GROUP(nested_ref_shared_const, kNestedRefSharedConstExample)
TEST_GROUP(nested_box, kNestedBoxExample)
TEST_GROUP(indirection, kIndirectionExample)
TEST_GROUP(union_1, kBasicUnionExample1)
TEST_GROUP(union_2, kBasicUnionExample2)
TEST_GROUP(union_3, kBasicUnionExample3)
TEST_GROUP(union_4, kBasicUnionExample4)
TEST_GROUP(map_union, kMapUnionExample)
