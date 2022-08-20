/*
 * Copyright (c) Facebook, Inc. and its affiliates.
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

#include <thrift/test/gen-cpp2/HashSetTest_types.h>

#include <folly/portability/GTest.h>

using namespace apache::thrift::test;

TEST(HashSetTest, example) {
  foo f;
  f.bar()->insert(5);
  EXPECT_EQ(1, f.bar()->count(5));
  f.bar()->insert(6);
  EXPECT_EQ(1, f.bar()->count(6));

  f.bar()->erase(5);
  EXPECT_EQ(0, f.bar()->count(5));

  f.baz()->insert("cool");
  EXPECT_EQ(1, f.baz()->count("cool"));

  f.baz()->erase("cool");
  EXPECT_EQ(0, f.baz()->count("cool"));
}
