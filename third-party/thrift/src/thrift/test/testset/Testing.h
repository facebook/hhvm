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

#include <thrift/test/testset/gen-cpp2/testset_fatal_types.h>

namespace apache::thrift::test::testset {
namespace testing_detail {

using testset_info = reflect_module<testset::testset_tags::module>;

template <typename Ts>
struct to_gtest_types;
template <typename... Ts>
struct to_gtest_types<fatal::list<Ts...>> {
  using type = testing::Types<fatal::first<Ts>...>;
};
template <typename T, T A, T E>
struct StaticEquals;
template <typename T, T V>
struct StaticEquals<T, V, V> : std::true_type {};

// Unfortuantely, the version of testing::Types we are using only supports up to
// 50 types, so we have to batch.
constexpr size_t kBatchSize = 50;
template <typename Ts>
using to_gtest_types_t = typename to_gtest_types<Ts>::type;

} // namespace testing_detail

// TODO(afuller): Remove batching once gtest is updated to support arbitrary
// long variadic types.
#define _THRIFT_TESTSET_NS ::apache::thrift::test::testset::testing_detail
#define _THRIFT_INST_TESTSET_BATCH(Test, Type, Batch)     \
  using testset_##Test##Type##Batch =                     \
      _THRIFT_TESTSET_NS::to_gtest_types_t<fatal::slice<  \
          _THRIFT_TESTSET_NS::testset_info::Type,         \
          Batch * _THRIFT_TESTSET_NS::kBatchSize,         \
          (Batch + 1) * _THRIFT_TESTSET_NS::kBatchSize>>; \
  INSTANTIATE_TYPED_TEST_CASE_P(Type##Batch, Test, testset_##Test##Type##Batch)
#define _THRIFT_INST_TESTSET_LAST(Test, Type, Batch)    \
  using testset_##Test##Type##Batch =                   \
      _THRIFT_TESTSET_NS::to_gtest_types_t<fatal::tail< \
          _THRIFT_TESTSET_NS::testset_info::Type,       \
          Batch * _THRIFT_TESTSET_NS::kBatchSize>>;     \
  INSTANTIATE_TYPED_TEST_CASE_P(Type##Batch, Test, testset_##Test##Type##Batch)

#define _THRIFT_CHECK_TESTSET_BATCHES(Type, Batches)                   \
  static_assert(                                                       \
      _THRIFT_TESTSET_NS::StaticEquals<                                \
          size_t,                                                      \
          fatal::size<_THRIFT_TESTSET_NS::testset_info::Type>::value / \
                  _THRIFT_TESTSET_NS::kBatchSize +                     \
              1,                                                       \
          Batches>(),                                                  \
      "# of " #Type " batches mismatch")

#define THRIFT_INST_TESTSET_STRUCTS(Test)        \
  _THRIFT_INST_TESTSET_BATCH(Test, structs, 0);  \
  _THRIFT_INST_TESTSET_BATCH(Test, structs, 1);  \
  _THRIFT_INST_TESTSET_BATCH(Test, structs, 2);  \
  _THRIFT_INST_TESTSET_BATCH(Test, structs, 3);  \
  _THRIFT_INST_TESTSET_BATCH(Test, structs, 4);  \
  _THRIFT_INST_TESTSET_BATCH(Test, structs, 5);  \
  _THRIFT_INST_TESTSET_BATCH(Test, structs, 6);  \
  _THRIFT_INST_TESTSET_BATCH(Test, structs, 7);  \
  _THRIFT_INST_TESTSET_BATCH(Test, structs, 8);  \
  _THRIFT_INST_TESTSET_BATCH(Test, structs, 9);  \
  _THRIFT_INST_TESTSET_BATCH(Test, structs, 10); \
  _THRIFT_INST_TESTSET_BATCH(Test, structs, 11); \
  _THRIFT_INST_TESTSET_BATCH(Test, structs, 12); \
  _THRIFT_INST_TESTSET_BATCH(Test, structs, 13); \
  _THRIFT_INST_TESTSET_BATCH(Test, structs, 14); \
  _THRIFT_INST_TESTSET_BATCH(Test, structs, 15); \
  _THRIFT_INST_TESTSET_BATCH(Test, structs, 16); \
  _THRIFT_INST_TESTSET_BATCH(Test, structs, 17); \
  _THRIFT_INST_TESTSET_BATCH(Test, structs, 18); \
  _THRIFT_INST_TESTSET_BATCH(Test, structs, 19); \
  _THRIFT_INST_TESTSET_BATCH(Test, structs, 20); \
  _THRIFT_INST_TESTSET_BATCH(Test, structs, 21); \
  _THRIFT_INST_TESTSET_BATCH(Test, structs, 22); \
  _THRIFT_INST_TESTSET_BATCH(Test, structs, 23); \
  _THRIFT_INST_TESTSET_BATCH(Test, structs, 24); \
  _THRIFT_INST_TESTSET_BATCH(Test, structs, 25); \
  _THRIFT_INST_TESTSET_BATCH(Test, structs, 26); \
  _THRIFT_INST_TESTSET_BATCH(Test, structs, 27); \
  _THRIFT_INST_TESTSET_BATCH(Test, structs, 28); \
  _THRIFT_INST_TESTSET_BATCH(Test, structs, 29); \
  _THRIFT_INST_TESTSET_BATCH(Test, structs, 30); \
  _THRIFT_INST_TESTSET_BATCH(Test, structs, 31); \
  _THRIFT_INST_TESTSET_BATCH(Test, structs, 32); \
  _THRIFT_INST_TESTSET_BATCH(Test, structs, 33); \
  _THRIFT_INST_TESTSET_BATCH(Test, structs, 34); \
  _THRIFT_INST_TESTSET_BATCH(Test, structs, 35); \
  _THRIFT_INST_TESTSET_BATCH(Test, structs, 36); \
  _THRIFT_INST_TESTSET_BATCH(Test, structs, 37); \
  _THRIFT_INST_TESTSET_BATCH(Test, structs, 38); \
  _THRIFT_INST_TESTSET_BATCH(Test, structs, 39); \
  _THRIFT_INST_TESTSET_BATCH(Test, structs, 40); \
  _THRIFT_INST_TESTSET_BATCH(Test, structs, 41); \
  _THRIFT_INST_TESTSET_BATCH(Test, structs, 42); \
  _THRIFT_INST_TESTSET_BATCH(Test, structs, 43); \
  _THRIFT_INST_TESTSET_BATCH(Test, structs, 44); \
  _THRIFT_INST_TESTSET_BATCH(Test, structs, 45); \
  _THRIFT_INST_TESTSET_BATCH(Test, structs, 46); \
  _THRIFT_INST_TESTSET_BATCH(Test, structs, 47); \
  _THRIFT_INST_TESTSET_BATCH(Test, structs, 48); \
  _THRIFT_INST_TESTSET_BATCH(Test, structs, 49); \
  _THRIFT_INST_TESTSET_BATCH(Test, structs, 50); \
  _THRIFT_INST_TESTSET_BATCH(Test, structs, 51); \
  _THRIFT_INST_TESTSET_BATCH(Test, structs, 52); \
  _THRIFT_INST_TESTSET_BATCH(Test, structs, 53); \
  _THRIFT_INST_TESTSET_BATCH(Test, structs, 54); \
  _THRIFT_INST_TESTSET_LAST(Test, structs, 55)
_THRIFT_CHECK_TESTSET_BATCHES(structs, 56);

#define THRIFT_INST_TESTSET_UNIONS(Test)       \
  _THRIFT_INST_TESTSET_BATCH(Test, unions, 0); \
  _THRIFT_INST_TESTSET_BATCH(Test, unions, 1); \
  _THRIFT_INST_TESTSET_BATCH(Test, unions, 2); \
  _THRIFT_INST_TESTSET_BATCH(Test, unions, 3); \
  _THRIFT_INST_TESTSET_BATCH(Test, unions, 4); \
  _THRIFT_INST_TESTSET_LAST(Test, unions, 5)
_THRIFT_CHECK_TESTSET_BATCHES(unions, 6);

#define THRIFT_INST_TESTSET_ALL(Test) \
  THRIFT_INST_TESTSET_STRUCTS(Test);  \
  THRIFT_INST_TESTSET_UNIONS(Test)

} // namespace apache::thrift::test::testset
