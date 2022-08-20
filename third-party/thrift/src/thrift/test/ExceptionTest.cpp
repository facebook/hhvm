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

#include <thrift/test/gen-cpp2/ExceptionTest_types.h>

#include <folly/portability/GTest.h>

using namespace apache::thrift::test;

TEST(ExceptionTest, braced_init) {
  MyException e = {};
  EXPECT_EQ(e.msg(), "");
}

TEST(ExceptionTest, test_default_constructor) {
  try {
    MyException e;
    *e.msg() = "what!!!";
    throw e;
  } catch (const std::exception& ex) {
    EXPECT_EQ(ex.what(), std::string{"what!!!"});
  }
}

TEST(ExceptionTest, test_constructor_with_param) {
  try {
    throw MyException("what!!!");
  } catch (const std::exception& ex) {
    EXPECT_EQ(ex.what(), std::string{"what!!!"});
  }
}

TEST(ExceptionTest, no_ref) {
  MyException e;
  EXPECT_EQ(e.msg(), "");
}
