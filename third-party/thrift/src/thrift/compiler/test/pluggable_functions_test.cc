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

#include <thrift/compiler/detail/pluggable_functions.h>

#include <gtest/gtest.h>

using apache::thrift::compiler::detail::pluggable_functions;

namespace {
struct void_tag {
  static void default_impl() {}
};
struct int_tag {
  static int default_impl(int) { return 0; }
};

int count_calls(bool reset = false) {
  static int calls = 0;
  if (reset) {
    calls = -1;
  }
  return ++calls;
}
} // namespace

TEST(PluggableFunctionsTest, unset) {
  auto& funcs = pluggable_functions();
  funcs.call<void_tag>();
  EXPECT_EQ(0, funcs.call<int_tag>(42));
}

TEST(PluggableFunctionsTest, set) {
  count_calls(true); // reset
  auto& funcs = pluggable_functions();
  auto* voidfn = +[] { count_calls(); };
  funcs.set<void_tag>(voidfn);
  funcs.call<void_tag>();
  EXPECT_EQ(count_calls(), 2);

  auto* intfn = +[](int x) { return x; };
  funcs.set<int_tag>(intfn);
  EXPECT_EQ(42, funcs.call<int_tag>(42));
}
