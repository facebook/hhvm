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

#include <thrift/lib/cpp2/frozen/FrozenTestUtil.h>
#include <thrift/lib/cpp2/frozen/HintTypes.h>

namespace apache::thrift::frozen {

TEST(FrozenVectorTypes, Unpacked) {
  VectorUnpacked<int> viu{2, 3, 5, 7, 9, 11, 13, 17};
  std::vector<int> vip = viu;
  EXPECT_LT(frozenSize(vip), frozenSize(viu));
  auto fiu = freeze(viu);
  EXPECT_EQ(fiu[2], 5);
  EXPECT_EQ(fiu.end()[-1], 17);
  const int* raw = fiu.begin();
  EXPECT_EQ(raw[3], 7);
}
} // namespace apache::thrift::frozen
