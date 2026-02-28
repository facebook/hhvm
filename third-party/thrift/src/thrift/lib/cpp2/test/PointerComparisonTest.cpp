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

#include <thrift/lib/cpp2/gen/module_types_cpp.h>

#include <gtest/gtest.h>

namespace apache::thrift::detail {

const int* null = nullptr;
const int i0 = 0;
const int j0 = 0;
const int j1 = 1;

TEST(Pointer, Less) {
  EXPECT_FALSE(pointer_less(null, null));
  EXPECT_TRUE(pointer_less(null, &i0));
  EXPECT_FALSE(pointer_less(&i0, null));
  EXPECT_FALSE(pointer_less(&i0, &i0));
  EXPECT_FALSE(pointer_less(&i0, &j0));
  EXPECT_TRUE(pointer_less(&i0, &j1));
}

TEST(Pointer, Equal) {
  EXPECT_TRUE(pointer_equal(null, null));
  EXPECT_FALSE(pointer_equal(null, &i0));
  EXPECT_FALSE(pointer_equal(&i0, null));
  EXPECT_TRUE(pointer_equal(&i0, &i0));
  EXPECT_TRUE(pointer_equal(&i0, &j0));
  EXPECT_FALSE(pointer_equal(&i0, &j1));
}

} // namespace apache::thrift::detail
