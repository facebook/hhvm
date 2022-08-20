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

#include <thrift/test/gen-cpp2/FieldNameAccess_types.h>

#include <folly/portability/GTest.h>

namespace apache::thrift::test {
TEST(Field, Access) {
  Foo foo;
  foo.field1()->push_back(10);
  EXPECT_EQ(foo.field1(), foo.field1());
  EXPECT_FALSE(foo.field2());
  foo.field2() = "bar";
  EXPECT_TRUE(foo.field2());
  EXPECT_EQ(foo.field2(), "bar");
}
} // namespace apache::thrift::test
