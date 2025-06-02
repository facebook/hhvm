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

#include <thrift/test/gen-cpp2/enum_types.h>
#include <thrift/test/gen-cpp2/structs_types.h>

#include <gtest/gtest.h>
#include <thrift/lib/cpp2/reflection/testing.h>

namespace apache::thrift::test {
TEST(ExpectThriftEqTest, structs) {
  Basic b1;
  EXPECT_THRIFT_EQ(b1, b1);
}

TEST(ExpectThriftEqTest, enums) {
  cpp2::MyEnum1 e1 = cpp2::MyEnum1::ME1_0;
  EXPECT_THRIFT_EQ(e1, cpp2::MyEnum1::ME1_0);
}

} // namespace apache::thrift::test
