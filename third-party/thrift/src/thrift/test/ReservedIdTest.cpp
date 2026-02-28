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

#include <cstddef>
#include <limits>
#include <set>

#include <gtest/gtest.h>
#include <thrift/compiler/ast/t_field.h>
#include <thrift/lib/cpp2/protocol/detail/ReservedId.h>

namespace apache::thrift::detail {

template <class F>
void for_each_id(F f) {
  // TODO: making it more efficient by using reflection in C++23
  for (int32_t i = std::numeric_limits<int16_t>::min();
       i <= std::numeric_limits<int16_t>::max();
       ++i) {
    // There will be build failure if we missed enum value in switch statement
    switch (static_cast<ReservedId>(i)) {
      case ReservedId::kIndex:
      case ReservedId::kOffset:
      case ReservedId::kExpectedRandomNumber:
      case ReservedId::kActualRandomNumber:
      case ReservedId::kXxh3Checksum:
        f(i);
        break;
      case ReservedId::kInjectMetadataFieldsLastId:
      case ReservedId::kInjectMetadataFieldsStartId:
        break;
    };
  }
}

TEST(ReservedId, Unique) {
  std::set<int16_t> s;
  for_each_id([&](int16_t i) { EXPECT_TRUE(s.insert(i).second); });
}

TEST(ReservedId, IsReserved) {
  for_each_id([](int16_t i) { EXPECT_LT(i, compiler::t_field::min_id); });
}

} // namespace apache::thrift::detail
