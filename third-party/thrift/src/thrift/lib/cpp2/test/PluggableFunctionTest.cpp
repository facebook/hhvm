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

#include <exception>

#include <folly/lang/Keep.h>
#include <folly/portability/GTest.h>

#include <thrift/lib/cpp2/PluggableFunction.h>

THRIFT_PLUGGABLE_FUNC_DECLARE(int, functionDefault, int);
THRIFT_PLUGGABLE_FUNC_DECLARE(int, functionWithOverride, int, int);

extern "C" FOLLY_KEEP int check_pluggable_func_invoke_default(int a) {
  return functionDefault(a);
}

extern "C" FOLLY_KEEP int check_pluggable_func_invoke_override(int a, int b) {
  return functionWithOverride(a, b);
}

THRIFT_PLUGGABLE_FUNC_REGISTER(int, functionDefault, int a) {
  return a * 2;
}

THRIFT_PLUGGABLE_FUNC_REGISTER(int, functionWithOverride, int a, int b) {
  return a + b;
}

THRIFT_PLUGGABLE_FUNC_SET(int, functionWithOverride, int a, int b) {
  return a * b;
}

TEST(PluggableFunction, Default) {
  EXPECT_EQ(2, functionDefault(1));
  EXPECT_EQ(84, functionDefault(42));
}

TEST(PluggableFunction, Override) {
  EXPECT_EQ(4, functionWithOverride(2, 2));
  EXPECT_EQ(10, functionWithOverride(2, 5));
}
