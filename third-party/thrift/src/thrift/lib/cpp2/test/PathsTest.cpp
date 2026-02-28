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

#include <cmath>
#include <iostream>
#include <string>

#include <gtest/gtest.h>

#include <thrift/lib/cpp2/protocol/Serializer.h>
#include <thrift/lib/cpp2/test/gen-cpp2/Paths_types.h>

using namespace apache::thrift;
using namespace apache::thrift::test;

TEST(PathsDemo, example) {
  Path1 p1; // list of pairs, 5005 bytes
  Path2 p2; // pair of lists, 2009 bytes

  for (int i = 0; i < 1000; ++i) {
    int x = 60 * std::cos(i * 0.01);
    int y = 60 * std::sin(i * 0.01);
    Point p;
    *p.x() = x;
    *p.y() = y;
    p1.points()->push_back(p);
    p2.xs()->push_back(x);
    p2.ys()->push_back(y);
  }

  auto s1 = CompactSerializer::serialize<std::string>(p1);
  auto s2 = CompactSerializer::serialize<std::string>(p2);

  EXPECT_EQ(5005, s1.size());
  EXPECT_EQ(2009, s2.size());
}
