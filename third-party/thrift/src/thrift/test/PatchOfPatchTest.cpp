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

#include <folly/portability/GTest.h>
#include <thrift/test/gen-cpp2/PatchOfPatch_types.h>

namespace facebook::thrift::test {
TEST(PatchOfPatchTest, Example) {
  Bar bar;
  bar.foo()->patch<apache::thrift::ident::msg>() = "10";

  Foo foo;
  bar.foo()->apply(foo);
  EXPECT_EQ(foo.msg(), "10");

  FooPatch fooPatch;
  fooPatch.patch<apache::thrift::ident::msg>() = "20";

  BarPatch barPatch;
  barPatch.patch<apache::thrift::ident::foo>() = fooPatch;

  barPatch.apply(bar);

  bar.foo()->apply(foo);
  EXPECT_EQ(foo.msg(), "20");
}
} // namespace facebook::thrift::test
