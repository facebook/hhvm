/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <folly/portability/GTest.h>
#include <memory>
#include <proxygen/lib/http/codec/compress/Logging.h>
#include <proxygen/lib/http/codec/compress/QPACKHeaderTable.h>
#include <sstream>

namespace proxygen {

class QPACKHeaderTableTests : public testing::Test {
 public:
 protected:
  QPACKHeaderTable table_{320, true};
};

TEST_F(QPACKHeaderTableTests, Indexing) {
  HPACKHeader accept("accept-encoding", "gzip");
  HPACKHeader agent("user-agent", "SeaMonkey");

  EXPECT_EQ(table_.getInsertCount(), 0);
  table_.add(accept.copy());
  EXPECT_EQ(table_.getInsertCount(), 1);
  // Vulnerable - in the table
  EXPECT_EQ(table_.getIndex(accept, false),
            std::numeric_limits<uint32_t>::max());
  // Allow vulnerable, get the index
  EXPECT_EQ(table_.getIndex(accept, true), 1);
  EXPECT_TRUE(table_.onInsertCountIncrement(1));
  EXPECT_EQ(table_.getIndex(accept, false), 1);
  table_.add(agent.copy());
  // Indexes move
  EXPECT_EQ(table_.getIndex(agent, true), 1);
  EXPECT_EQ(table_.getIndex(accept, true), 2);
}

TEST_F(QPACKHeaderTableTests, Eviction) {
  HPACKHeader accept("accept-encoding", "gzip");

  int32_t max = 4;
  uint32_t capacity = accept.bytes() * max;
  table_.setCapacity(capacity);

  for (auto i = 0; i < max; i++) {
    EXPECT_TRUE(table_.add(accept.copy()));
  }
  for (auto i = 1; i <= max; i++) {
    table_.addRef(i);
  }
  table_.setAcknowledgedInsertCount(max);
  EXPECT_FALSE(table_.canIndex(accept.name, accept.value));
  EXPECT_FALSE(table_.add(accept.copy()));
  table_.subRef(1);
  EXPECT_TRUE(table_.canIndex(accept.name, accept.value));
  EXPECT_TRUE(table_.add(accept.copy()));

  table_.subRef(3);
  EXPECT_FALSE(table_.canIndex(accept.name, accept.value));
  table_.subRef(2);
  EXPECT_TRUE(table_.canIndex(accept.name, accept.value));
}

TEST_F(QPACKHeaderTableTests, BadEviction) {
  HPACKHeader accept("accept-encoding", "gzip");

  int32_t max = 4;
  uint32_t capacity = accept.bytes() * max;
  table_.setCapacity(capacity);

  for (auto i = 0; i < max; i++) {
    EXPECT_TRUE(table_.add(accept.copy()));
  }
  EXPECT_EQ(table_.size(), max);
  EXPECT_FALSE(table_.setCapacity(capacity / 2));
  EXPECT_EQ(table_.size(), max);

  // Ack all headers but mark the first as in use
  table_.setAcknowledgedInsertCount(max);
  table_.addRef(1);
  EXPECT_FALSE(table_.setCapacity(capacity / 2));

  // Clear all refs
  table_.subRef(1);
  EXPECT_TRUE(table_.setCapacity(capacity / 2));
  EXPECT_EQ(table_.size(), max / 2);
}

TEST_F(QPACKHeaderTableTests, Wrapcount) {
  HPACKHeader accept("accept-encoding", "gzip");
  HPACKHeader agent("user-agent", "SeaMonkey");
  HPACKHeader cookie("Cookie", "choco=chip");

  for (auto i = 0; i < 10; i++) {
    EXPECT_TRUE(table_.add(accept.copy()));
    table_.setAcknowledgedInsertCount(i + 1);
  }
  EXPECT_TRUE(table_.add(cookie.copy()));
  EXPECT_TRUE(table_.add(agent.copy()));

  EXPECT_EQ(table_.getInsertCount(), 12);
  EXPECT_EQ(table_.getIndex(agent, true), 1);
  EXPECT_EQ(table_.getIndex(cookie, true), 2);
  EXPECT_EQ(table_.getIndex(accept, true), 3);
  EXPECT_EQ(table_.getHeader(1, table_.getInsertCount()), agent);
  EXPECT_EQ(table_.getHeader(2, table_.getInsertCount()), cookie);
  EXPECT_EQ(table_.getHeader(table_.size(), table_.getInsertCount()), accept);
}

TEST_F(QPACKHeaderTableTests, NameIndex) {
  HPACKHeader accept("accept-encoding", "gzip");
  EXPECT_EQ(table_.nameIndex(accept.name), 0);
  EXPECT_TRUE(table_.add(accept.copy()));
  EXPECT_EQ(table_.nameIndex(accept.name), 1);
}

TEST_F(QPACKHeaderTableTests, GetIndex) {
  HPACKHeader accept1("accept-encoding", "gzip");
  HPACKHeader accept2("accept-encoding", "blarf");
  EXPECT_EQ(table_.getIndex(accept1), 0);
  EXPECT_TRUE(table_.add(accept1.copy()));
  EXPECT_EQ(table_.getIndex(accept1), 1);
  EXPECT_EQ(table_.getIndex(accept2), 0);
}

TEST_F(QPACKHeaderTableTests, Duplication) {
  HPACKHeader accept("accept-encoding", "gzip");

  EXPECT_TRUE(table_.add(accept.copy()));

  // Unnecessary duplicate
  auto res = table_.maybeDuplicate(1, true);
  EXPECT_FALSE(res.first);
  EXPECT_EQ(res.second, 1);

  for (auto i = 0; i < 6; i++) {
    EXPECT_TRUE(table_.add(accept.copy()));
    // Ack the first few entries so they can be evicted
    table_.setAcknowledgedInsertCount(std::min(3u, table_.getInsertCount()));
  }

  // successful duplicate, vulnerable allowed
  EXPECT_TRUE(table_.isDraining(table_.size()));
  res = table_.maybeDuplicate(table_.size(), true);
  EXPECT_TRUE(res.first);
  EXPECT_EQ(res.second, 8);
  EXPECT_EQ(table_.size(), 6); // evicted 1

  // successful duplicate, vulnerable disallowed
  EXPECT_TRUE(table_.onInsertCountIncrement(3));
  res = table_.maybeDuplicate(table_.size(), false);
  EXPECT_TRUE(res.first);
  EXPECT_EQ(res.second, 0);
  EXPECT_EQ(table_.size(), 6); // evicted 2

  // Attempt to duplicate UNACKED
  res = table_.maybeDuplicate(QPACKHeaderTable::UNACKED, true);
  EXPECT_FALSE(res.first);
  EXPECT_EQ(res.second, 0);
  EXPECT_EQ(table_.size(), 6); // nothing changed
  EXPECT_EQ(table_.getInsertCount(), 9);

  // Hold a ref to oldest entry, prevents eviction
  auto oldestAbsolute = table_.getInsertCount() - table_.size() + 1;
  table_.addRef(oldestAbsolute);

  // Table should be full
  EXPECT_FALSE(table_.canIndex(accept.name, accept.value));

  res = table_.maybeDuplicate(table_.size(), true);
  EXPECT_FALSE(res.first);
  EXPECT_EQ(res.second, 0);
}

TEST_F(QPACKHeaderTableTests, CanEvictWithRoom) {
  HPACKHeader thirtyNineBytes("abcd", "efg");
  HPACKHeader fortySevenBytes("abcd", "efghijklmno");
  for (auto i = 0; i < 8; i++) {
    EXPECT_TRUE(table_.add(thirtyNineBytes.copy()));
  }
  table_.setAcknowledgedInsertCount(table_.getInsertCount());
  // abs index = 1 is evictable, but index = 2 is referenced, so we can
  // insert up to (320 - 8 * 39) + 39 = 47
  table_.addRef(2);
  EXPECT_TRUE(table_.canIndex(fortySevenBytes.name, fortySevenBytes.value));
  EXPECT_TRUE(table_.add(fortySevenBytes.copy()));
}

TEST_F(QPACKHeaderTableTests, EvictNonDrained) {
  HPACKHeader small("ab", "cd");                                 // 36 bytes
  HPACKHeader small2("abcd", std::string(14, 'b'));              // 50 bytes
  HPACKHeader med(std::string(20, 'a'), std::string(20, 'b'));   // 72
  HPACKHeader large(std::string(34, 'a'), std::string(34, 'b')); // 100

  table_.setCapacity(220);
  EXPECT_TRUE(table_.add(small.copy()));
  EXPECT_TRUE(table_.add(med.copy()));
  EXPECT_TRUE(table_.add(large.copy()));
  EXPECT_TRUE(table_.isDraining(3));
  EXPECT_FALSE(table_.isDraining(2));

  table_.setAcknowledgedInsertCount(3);
  // Evicts small and med
  EXPECT_TRUE(table_.add(small2.copy()));
  EXPECT_EQ(table_.size(), 2);
  EXPECT_FALSE(table_.isDraining(1));
  EXPECT_FALSE(table_.isDraining(2));

  // Now lg should be draining
  EXPECT_TRUE(table_.add(small.copy()));
  EXPECT_TRUE(table_.isDraining(3));

  // Evict large
  EXPECT_TRUE(table_.add(small.copy()));
}

TEST_F(QPACKHeaderTableTests, BadSync) {
  // Can't ack more than is in the table
  EXPECT_FALSE(table_.onInsertCountIncrement(1));
}

TEST_F(QPACKHeaderTableTests, TinyTable) {
  // This table will tell you it can't hold any headers, but it can!
  QPACKHeaderTable table(0, true);
  table.setCapacity(63);
  HPACKHeader foo("F", "");
  EXPECT_FALSE(table.canIndex(foo.name, foo.value));
  EXPECT_TRUE(table.add(foo.copy()));
  EXPECT_EQ(table.size(), 1);
  EXPECT_EQ(table.length(), 1);
  EXPECT_TRUE(table.isDraining(1));
}
} // namespace proxygen
