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
#include <thrift/lib/cpp2/frozen/FrozenUtil.h>
#include <thrift/lib/cpp2/protocol/Serializer.h>

using namespace apache::thrift;
using namespace frozen;

TEST(FrozenUtil, FreezeAndUse) {
  auto file = freezeToTempFile(std::string("hello"));
  MappedFrozen<std::string> mapped;
  mapped = mapFrozen<std::string>(folly::File(file.fd()));
  EXPECT_EQ(folly::StringPiece(mapped), "hello");
}

TEST(FrozenUtil, FreezeAndMap) {
  auto original = std::vector<std::string>{"hello", "world"};
  folly::test::TemporaryFile tmp;

  freezeToFile(original, folly::File(tmp.fd()));

  MappedFrozen<std::vector<std::string>> mapped;
  EXPECT_FALSE(mapped);
  mapped = mapFrozen<std::vector<std::string>>(folly::File(tmp.fd()));
  EXPECT_TRUE(mapped);

  auto thawed = mapped.thaw();
  EXPECT_EQ(original, thawed);
  original.emplace_back("different");
  EXPECT_NE(original, thawed);
}

TEST(FrozenUtil, FutureVersion) {
  folly::test::TemporaryFile tmp;

  {
    schema::Schema schema;
    *schema.fileVersion() = 1000;
    schema.fileVersion().ensure();

    std::string schemaStr;
    CompactSerializer::serialize(schema, &schemaStr);
    write(tmp.fd(), schemaStr.data(), schemaStr.size());
  }

  EXPECT_THROW(
      mapFrozen<std::string>(folly::File(tmp.fd())),
      FrozenFileForwardIncompatible);
}

TEST(FrozenUtil, FileSize) {
  auto original = std::vector<std::string>{"hello", "world"};
  folly::test::TemporaryFile tmp;
  freezeToFile(original, folly::File(tmp.fd()));
  struct stat stats;
  fstat(tmp.fd(), &stats);
  EXPECT_LT(stats.st_size, 500); // most of this is the schema
}

TEST(FrozenUtil, FreezeToString) {
  // multiplication tables for first three primes
  using TestType = std::map<int, std::map<int, int>>;
  TestType m{
      {2, {{2, 4}, {3, 6}, {5, 10}}},
      {3, {{2, 6}, {3, 9}, {5, 15}}},
      {5, {{2, 10}, {3, 15}, {5, 25}}},
  };
  MappedFrozen<TestType> frozen;
  MappedFrozen<TestType> frozen2;
  {
    std::string store;
    freezeToString(m, store);
    std::string store2 = freezeToString(m);
    // In this example, the schema is 101 bytes and the data is only 17 bytes!
    // By default, this is stripped out by this overload.
    frozen = mapFrozen<TestType>(std::move(store));
    frozen2 = mapFrozen<TestType>(std::move(store2));
  }
  EXPECT_EQ(frozen.at(3).at(5), 15);
  EXPECT_EQ(frozen2.at(3).at(5), 15);
  {
    std::string store;
    freezeToString(m, store);
    std::string store2 = freezeToString(m);
    // false = don't trim the space for the schema
    frozen = mapFrozen<TestType>(std::move(store), false);
    frozen2 = mapFrozen<TestType>(std::move(store2), false);
  }
  EXPECT_EQ(frozen.at(3).at(5), 15);
  EXPECT_EQ(frozen2.at(3).at(5), 15);
  {
    std::string store;
    freezeToString(m, store);
    std::string store2;
    freezeToStringMalloc(m, store2);
    // TODO(T44041774): Changes in alignment are leading to differences in these
    // two freezes. They shouldn't differ in this case.
    EXPECT_NEAR(store.size(), store2.size(), 8);
    // EXPECT_EQ(store, store2);
    // false = don't trim the space for the schema
    frozen = mapFrozen<TestType>(std::move(store), false);
  }
  EXPECT_EQ(frozen.at(3).at(5), 15);
}

TEST(Frozen, WorstCasePadding) {
  using Doubles = std::vector<double>;
  using Entry = std::pair<Doubles, std::string>;
  using Table = std::vector<Entry>;
  Table table;
  for (int i = 0; i < 10; ++i) {
    table.emplace_back();
    auto& e = table.back();
    e.first = {1.1, 2.2};
    e.second.resize(1, 'a' + i);
  }
  std::string str;
  freezeToString(table, str);
}

TEST(Frozen, TailPadding) {
  using Entry = std::pair<size_t, size_t>;
  using Table = std::vector<Entry>;
  Table table;
  for (int i = 0; i < 1000; ++i) {
    table.emplace_back();
    auto& e = table.back();
    e.first = 3;
    e.second = 7;
    std::string str;
    freezeToString(table, str);
    auto view = mapFrozen<Table>(std::move(str), true);
    EXPECT_EQ(view.back().second(), 7);
  }
}

TEST(Frozen, ConstLayout) {
  using Table = std::vector<std::string>;
  Layout<Table> layout;
  LayoutRoot::layout(Table{"x"}, layout);
  EXPECT_EQ(11, frozenSize(Table{"y"}, layout));
  EXPECT_THROW(frozenSize(Table{"hello", "world"}, layout), LayoutException);
  LayoutRoot::layout(Table{"hello", "world"}, layout);
  EXPECT_EQ(21, frozenSize(Table{"hello", "world"}, layout));
}

TEST(Frozen, FreezeToStringMalloc) {
  using Table = std::vector<std::string>;
  std::string x, y;
  Table value{"hello", "world"};
  freezeToString(value, x);
  freezeToStringMalloc(value, y);
  EXPECT_EQ(x, y);
  auto fx = mapFrozen<Table>(std::move(x));
  auto fy = mapFrozen<Table>(std::move(y));
  EXPECT_EQ(fx[0], fy[0]);
  EXPECT_EQ(fx[1], fy[1]);
}

TEST(Frozen, FreezeDataToString) {
  using Table = std::vector<std::string>;
  Layout<Table> layout;
  LayoutRoot::layout(Table{"xxx", "yyy", "www"}, layout);
  const Layout<Table> fixedLayout = layout;
  EXPECT_THROW(
      freezeDataToString(Table{"hello", "world"}, fixedLayout),
      LayoutException);
  auto str = freezeDataToString(Table{"abc", "123", "xyz"}, fixedLayout);
  auto view = fixedLayout.view({reinterpret_cast<byte*>(&str[0]), 0});
  EXPECT_EQ(view.size(), 3);
  EXPECT_EQ(view[0], "abc");
  EXPECT_EQ(view[1], "123");
  EXPECT_EQ(view[2], "xyz");
}
