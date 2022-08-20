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
#include <thrift/lib/cpp2/type/Id.h>

using namespace ::testing;

namespace apache {
namespace thrift {
namespace type {
TEST(OrdilaTest, Basic) {
  constexpr size_t npos = toPosition(Ordinal{});
  EXPECT_EQ(npos, std::string::npos);
  EXPECT_EQ(Ordinal{}, toOrdinal(npos));

  EXPECT_EQ(Ordinal(1), toOrdinal(0));
  EXPECT_EQ(0, toPosition(Ordinal(1)));

  EXPECT_EQ(Ordinal(43), toOrdinal(42));
  EXPECT_EQ(42, toPosition(Ordinal(43)));
}
} // namespace type
} // namespace thrift
} // namespace apache
