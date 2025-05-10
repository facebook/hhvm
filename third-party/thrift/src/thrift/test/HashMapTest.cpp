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

#include <thrift/test/gen-cpp2/HashMapTest_types.h>

#include <gtest/gtest.h>

using namespace apache::thrift::test;

TEST(HashMapTest, example) {
  foo f;
  f.bar()[5] = 6;
  EXPECT_EQ(6, f.bar()[5]);
  f.bar()[6] = 7;
  EXPECT_EQ(7, f.bar()[6]);

  f.bar()[5] = 7;
  EXPECT_EQ(7, f.bar()[5]);

  f.baz()["cool"] = 50;
  EXPECT_EQ(50, f.baz()["cool"]);

  f.baz()["cool"] = 30;
  EXPECT_EQ(30, f.baz()["cool"]);
}
