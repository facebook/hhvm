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

#include <thrift/test/gen-cpp2/trivially_relocatable_types.h>

#include <folly/Traits.h>
#include <folly/portability/GTest.h>

using namespace apache::thrift::test;

TEST(trivially_relocatable_test, non_relocatable) {
  EXPECT_FALSE(folly::IsRelocatable<NonRelocatable>::value);
}

TEST(trivially_relocatable_test, relocatable) {
  EXPECT_TRUE(folly::IsRelocatable<Relocatable>::value);
}
