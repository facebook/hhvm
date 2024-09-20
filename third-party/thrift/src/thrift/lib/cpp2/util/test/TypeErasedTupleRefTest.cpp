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

#include <thrift/lib/cpp2/util/TypeErasedTupleRef.h>

#include <tuple>

using apache::thrift::util::TypeErasedTupleRef;

TEST(TypeErasedTupleRefTest, Basic) {
  std::tuple<std::string, int> tuple1{"hello", 42};
  TypeErasedTupleRef ref1(tuple1);
  auto ref1Copy = ref1;

  std::tuple<std::string, int> tuple2{"world", 74};
  TypeErasedTupleRef ref2(tuple2);
  EXPECT_EQ(ref2.count(), 2);
  EXPECT_EQ(ref2.get(0)->value<std::string>(), "world");
  ref2 = ref1;

  for (auto& ref : {ref1, ref1Copy, ref2}) {
    EXPECT_EQ(ref.count(), 2);
    EXPECT_EQ(ref.get(0)->value<std::string>(), "hello");
    EXPECT_EQ(ref.get(1)->value<int>(), 42);
    EXPECT_EQ(ref.get(3), std::nullopt);
  }
}
