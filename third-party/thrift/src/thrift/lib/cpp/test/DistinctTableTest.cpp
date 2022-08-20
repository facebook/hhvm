/*
 * Copyright (c) Facebook, Inc. and its affiliates.
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

#include <folly/portability/GTest.h>
#include <thrift/lib/cpp/DistinctTable.h>

namespace apache {
namespace thrift {

template <class T>
struct AllSamePolicy : BaseDistinctTablePolicy<T> {
  struct Hash {
    size_t operator()(const T&) const { return 0; }
  };

  struct Equal {
    bool operator()(const T&, const T&) const { return true; }
  };
};

template <class T>
struct OneByteIndexPolicy : BaseDistinctTablePolicy<T> {
  template <class Hash, class Equal>
  using Index = std::unordered_set<uint8_t, Hash, Equal>;
};

TEST(DistinctTable, Basic) {
  std::vector<std::string> words;
  DistinctTable<std::string> wordTable(&words);

  EXPECT_EQ(0, wordTable.add("hi"));
  EXPECT_EQ(1, wordTable.add("bye"));
  EXPECT_EQ(0, wordTable.add("hi"));
  EXPECT_EQ(2, wordTable.add("what"));
  EXPECT_EQ(1, wordTable.add("bye"));
  EXPECT_EQ(2, wordTable.add("what"));
  EXPECT_EQ(0, wordTable.add("hi"));
  EXPECT_EQ("hi", words[0]);
  EXPECT_EQ("bye", words[1]);
  EXPECT_EQ("what", words[2]);
  EXPECT_EQ(3, words.size());
}

TEST(DistinctTable, AllSamePolicy) {
  std::vector<std::string> words;
  DistinctTable<std::string, AllSamePolicy> wordTable(&words);

  EXPECT_EQ(0, wordTable.add("hi"));
  // Null hash/equal functions think everything is the same
  EXPECT_EQ(0, wordTable.add("bye"));
  EXPECT_EQ(0, wordTable.add("hi"));
}

TEST(DistinctTable, ByteIndexPolicy) {
  std::vector<double> nums;
  DistinctTable<double, OneByteIndexPolicy> numTable(&nums);

  for (int i = 0; i < 500; ++i) {
    if (i < 256) {
      EXPECT_EQ(i, numTable.add(sqrt(i)));
    } else {
      EXPECT_LT(numTable.add(sqrt(i)), 256);
    }
  }
}

} // namespace thrift
} // namespace apache
