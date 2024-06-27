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

#include <folly/portability/GTest.h>

using apache::thrift::compiler::detail::pluggable_functions;

namespace {
struct VoidTag {
  static void defaultImpl() {}
};
struct IntTag {
  static int defaultImpl(int) { return 0; }
};

int countCalls(bool reset = false) {
  static int calls = 0;
  if (reset) {
    calls = -1;
  }
  return ++calls;
}
} // namespace

TEST(PluggableFunctionsTest, unset) {
  auto& funcs = pluggable_functions();
  funcs.call<VoidTag>();
  EXPECT_EQ(0, funcs.call<IntTag>(42));
}

TEST(PluggableFunctionsTest, set) {
  countCalls(true); // reset
  auto& funcs = pluggable_functions();
  auto* voidfn = +[] { countCalls(); };
  funcs.set<VoidTag>(voidfn);
  funcs.call<VoidTag>();
  EXPECT_EQ(countCalls(), 2);

  auto* intfn = +[](int x) { return x; };
  funcs.set<IntTag>(intfn);
  EXPECT_EQ(42, funcs.call<IntTag>(42));
}
