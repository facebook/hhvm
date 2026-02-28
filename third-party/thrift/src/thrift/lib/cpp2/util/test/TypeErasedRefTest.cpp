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

#include <gtest/gtest.h>

#include <thrift/lib/cpp2/util/TypeErasedRef.h>

using apache::thrift::util::TypeErasedRef;

TEST(TypeErasedRefTest, Basic) {
  std::string str = "hello";
  auto ref1 = TypeErasedRef::of<std::string>(str);
  auto ref1Copy = ref1;

  std::string str2 = "world";
  auto ref2 = TypeErasedRef::of<std::string>(str2);
  EXPECT_EQ(ref2.value<std::string>(), str2);
  ref2 = ref1;

  for (auto& ref : {ref1, ref1Copy, ref2}) {
    EXPECT_EQ(std::uintptr_t(ref.ptr()), std::uintptr_t(&str));
    EXPECT_EQ(ref.type(), typeid(std::string));
    EXPECT_TRUE(ref.holds_alternative<std::string>());
    EXPECT_FALSE(ref.holds_alternative<int>());
    EXPECT_EQ(ref.value<std::string>(), str);
    EXPECT_EQ(ref.value<const std::string&>(), str);
    EXPECT_THROW(ref.value<int>(), std::bad_cast);
  }
}
