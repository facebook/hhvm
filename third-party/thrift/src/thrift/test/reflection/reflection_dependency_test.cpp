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

#include <thrift/test/reflection/gen-cpp2/reflection_fatal_types.h>

#include <thrift/lib/cpp2/reflection/internal/test_helpers.h>

#include <gtest/gtest.h>

namespace test_cpp2::cpp_reflection {

TEST(ReflectionDeps, RecursiveDependencies) {
  using namespace apache::thrift;
  EXPECT_SAME<
      type_class::structure,
      reflect_type_class_of_thrift_class<dep_A_struct>>();

  using b_type = typename std::decay<
      decltype(std::declval<dep_A_struct>().b())::value_type>::type;
  EXPECT_SAME<
      type_class::structure,
      reflect_type_class_of_thrift_class<b_type>>();

  using c_type = typename std::decay<
      decltype(std::declval<dep_A_struct>().c())::value_type>::type;
  EXPECT_SAME<
      type_class::structure,
      reflect_type_class_of_thrift_class<c_type>>();

  using d_type = typename std::decay<
      decltype(std::declval<c_type>().d())::value_type>::type;
  EXPECT_SAME<
      type_class::structure,
      reflect_type_class_of_thrift_class<d_type>>();
}

} // namespace test_cpp2::cpp_reflection
