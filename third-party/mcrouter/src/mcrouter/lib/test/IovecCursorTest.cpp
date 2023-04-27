/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <gtest/gtest.h>

#include <folly/Range.h>

#include "mcrouter/lib/IovecCursor.h"

using namespace facebook::memcache;

namespace {

std::pair<IovecCursor, std::vector<struct iovec>> getIovecCursor(
    const std::vector<folly::StringPiece>& buffers) {
  std::vector<struct iovec> iovs(buffers.size());
  for (size_t i = 0; i < buffers.size(); ++i) {
    iovs[i].iov_base = const_cast<char*>(buffers[i].data());
    iovs[i].iov_len = buffers[i].size();
  }
  IovecCursor cursor(iovs.data(), iovs.size());
  return std::make_pair(std::move(cursor), std::move(iovs));
}

} // anonymous namespace

TEST(IovecCursor, construct) {
  std::string buf1 = "12345";
  std::string buf2 = "67890";
  auto p = getIovecCursor({buf1, buf2});
  auto& cursor = p.first;

  EXPECT_TRUE(cursor.hasDataAvailable());
  EXPECT_EQ(10, cursor.totalLength());
  EXPECT_EQ(0, cursor.tell());
}

TEST(IovecCursor, construct_empty) {
  std::string buf1 = "";
  auto p = getIovecCursor({buf1});
  auto& cursor = p.first;

  EXPECT_FALSE(cursor.hasDataAvailable());
  EXPECT_EQ(0, cursor.totalLength());
}

TEST(IovecCursor, basic) {
  std::string buf2 = "345";
  std::string buf3 = "6789";
  std::string buf1 = "012";
  auto p = getIovecCursor({buf1, buf2, buf3});
  auto& cursor = p.first;

  char dest[10] = {'\0'};

  cursor.peekInto(reinterpret_cast<uint8_t*>(dest), 2);
  EXPECT_STREQ("01", dest);

  cursor.advance(2);
  EXPECT_EQ(2, cursor.tell());
  EXPECT_TRUE(cursor.hasDataAvailable());

  cursor.peekInto(reinterpret_cast<uint8_t*>(dest), 3);
  EXPECT_STREQ("234", dest);

  cursor.advance(3);
  EXPECT_TRUE(cursor.hasDataAvailable());
  EXPECT_EQ(5, cursor.tell());

  cursor.peekInto(reinterpret_cast<uint8_t*>(dest), 5);
  EXPECT_STREQ("56789", dest);

  cursor.advance(4);
  EXPECT_TRUE(cursor.hasDataAvailable());
  EXPECT_EQ(9, cursor.tell());

  cursor.advance(1);
  EXPECT_FALSE(cursor.hasDataAvailable());
  EXPECT_EQ(10, cursor.tell());
}

TEST(IovecCursor, peek) {
  uint32_t int2 = 12379;
  uint64_t int1 = 1928374;
  uint16_t int3 = 187;
  folly::StringPiece buf2(reinterpret_cast<char*>(&int2), sizeof(uint32_t));
  folly::StringPiece buf3(reinterpret_cast<char*>(&int3), sizeof(uint16_t));
  folly::StringPiece buf1(reinterpret_cast<char*>(&int1), sizeof(uint64_t));

  auto p = getIovecCursor({buf1, buf2, buf3});
  auto& cursor = p.first;

  EXPECT_EQ(int1, cursor.peek<uint64_t>());

  cursor.advance(4);
  EXPECT_TRUE(cursor.hasDataAvailable());
  EXPECT_EQ(4, cursor.tell());
  uint64_t expected = int1 >> 32 | static_cast<uint64_t>(int2) << 32;
  EXPECT_EQ(expected, cursor.peek<uint64_t>());

  cursor.advance(4);
  EXPECT_TRUE(cursor.hasDataAvailable());
  EXPECT_EQ(8, cursor.tell());
  EXPECT_EQ(int2, cursor.peek<uint32_t>());

  cursor.advance(1);
  EXPECT_TRUE(cursor.hasDataAvailable());
  EXPECT_EQ(9, cursor.tell());
  expected = int2 >> 8 | static_cast<uint64_t>(int3) << (32 - 8);
  EXPECT_EQ(expected, cursor.peek<uint32_t>());

  cursor.advance(3);
  EXPECT_TRUE(cursor.hasDataAvailable());
  EXPECT_EQ(12, cursor.tell());
  EXPECT_EQ(int3, cursor.peek<uint16_t>());

  cursor.advance(2);
  EXPECT_FALSE(cursor.hasDataAvailable());
  EXPECT_EQ(14, cursor.tell());
}

TEST(IovecCursor, read) {
  uint32_t int2 = 12379;
  uint64_t int1 = 1928374;
  uint16_t int3 = 187;
  folly::StringPiece buf2(reinterpret_cast<char*>(&int2), sizeof(uint32_t));
  folly::StringPiece buf3(reinterpret_cast<char*>(&int3), sizeof(uint16_t));
  folly::StringPiece buf1(reinterpret_cast<char*>(&int1), sizeof(uint64_t));

  auto p = getIovecCursor({buf1, buf2, buf3});
  auto& cursor = p.first;
  EXPECT_EQ(14, cursor.totalLength());

  EXPECT_EQ(int1, cursor.read<uint64_t>());
  EXPECT_TRUE(cursor.hasDataAvailable());
  EXPECT_EQ(8, cursor.tell());

  EXPECT_EQ(int2, cursor.read<uint32_t>());
  EXPECT_TRUE(cursor.hasDataAvailable());
  EXPECT_EQ(12, cursor.tell());

  EXPECT_EQ(int3, cursor.read<uint16_t>());
  EXPECT_FALSE(cursor.hasDataAvailable());
  EXPECT_EQ(14, cursor.tell());
}

TEST(IovecCursor, advance_retreat) {
  std::string buf1 = "12345";
  std::string buf2 = "67890";
  auto p = getIovecCursor({buf1, buf2});
  auto& cursor = p.first;

  char dest[3] = {'\0'};

  cursor.advance(2);
  EXPECT_EQ(2, cursor.tell());
  cursor.peekInto(reinterpret_cast<uint8_t*>(dest), 2);
  EXPECT_STREQ("34", dest);

  cursor.retreat(1);
  EXPECT_EQ(1, cursor.tell());
  cursor.peekInto(reinterpret_cast<uint8_t*>(dest), 2);
  EXPECT_STREQ("23", dest);

  cursor.advance(3);
  EXPECT_EQ(4, cursor.tell());
  cursor.peekInto(reinterpret_cast<uint8_t*>(dest), 2);
  EXPECT_STREQ("56", dest);

  cursor.retreat(2);
  EXPECT_EQ(2, cursor.tell());
  cursor.peekInto(reinterpret_cast<uint8_t*>(dest), 2);
  EXPECT_STREQ("34", dest);

  cursor.advance(4);
  EXPECT_EQ(6, cursor.tell());
  cursor.peekInto(reinterpret_cast<uint8_t*>(dest), 2);
  EXPECT_STREQ("78", dest);

  cursor.retreat(3);
  EXPECT_EQ(3, cursor.tell());
  cursor.peekInto(reinterpret_cast<uint8_t*>(dest), 2);
  EXPECT_STREQ("45", dest);

  cursor.advance(5);
  EXPECT_EQ(8, cursor.tell());
  cursor.peekInto(reinterpret_cast<uint8_t*>(dest), 2);
  EXPECT_STREQ("90", dest);

  cursor.retreat(4);
  EXPECT_EQ(4, cursor.tell());
  cursor.peekInto(reinterpret_cast<uint8_t*>(dest), 2);
  EXPECT_STREQ("56", dest);
}

TEST(IovecCursor, advance_retreat_edge_cases) {
  std::string buf1 = "12345";
  std::string buf2 = "67890";
  auto p = getIovecCursor({buf1, buf2});
  auto& cursor = p.first;

  cursor.seek(0);
  EXPECT_EQ(0, cursor.tell());

  cursor.advance(10);
  EXPECT_EQ(10, cursor.tell());

  cursor.retreat(10);
  EXPECT_EQ(0, cursor.tell());
}
