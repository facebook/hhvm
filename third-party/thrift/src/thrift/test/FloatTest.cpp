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

#include <thrift/test/gen-cpp2/FloatTest_constants.h>
#include <thrift/test/gen-cpp2/FloatTest_types.h>

#include <folly/portability/GTest.h>

using namespace apache::thrift::test;

TEST(FloatTest, example) {
  foo f;
  f.bar() = 5;
  EXPECT_EQ(5, *f.bar());
  EXPECT_EQ(1, *f.baz());
  EXPECT_EQ(12.345f, *f.baz1());
  f.baz() = 12.345f;
  EXPECT_EQ(12.345f, *f.baz());
}

TEST(FloatTest, precision) {
  EXPECT_EQ(foo().test_double(), 0.17836774747774789);
  EXPECT_EQ(FloatTest_constants::test_double(), 0.17836774747774789);
}
