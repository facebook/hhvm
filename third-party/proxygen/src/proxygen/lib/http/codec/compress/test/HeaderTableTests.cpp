/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <folly/portability/GTest.h>
#include <memory>
#include <proxygen/lib/http/codec/compress/HeaderTable.h>
#include <proxygen/lib/http/codec/compress/Logging.h>
#include <proxygen/lib/http/codec/compress/QPACKHeaderTable.h>
#include <sstream>

using namespace std;

namespace proxygen {

class HeaderTableTests : public testing::Test {
 protected:
  void xcheck(uint32_t internal, uint32_t external) {
    EXPECT_EQ(HeaderTable::toExternal(head_, length_, internal), external);
    EXPECT_EQ(HeaderTable::toInternal(head_, length_, external), internal);
  }

  void resizeTable(HeaderTable& table, uint32_t newCapacity, uint32_t newMax) {
    table.setCapacity(newCapacity);
    // On resizing the table size (count of headers) remains the same or sizes
    // down; can not size up
    EXPECT_LE(table.size(), newMax);
  }

  void resizeAndFillTable(HeaderTable& table,
                          HPACKHeader& header,
                          uint32_t newMax,
                          uint32_t fillCount) {
    uint32_t newCapacity = header.bytes() * newMax;
    resizeTable(table, newCapacity, newMax);
    // Fill the table (with one extra) and make sure we haven't violated our
    // size (bytes) limits (expected one entry to be evicted)
    for (size_t i = 0; i <= fillCount; ++i) {
      EXPECT_EQ(table.add(header.copy()), true);
    }
    EXPECT_EQ(table.size(), newMax);
    EXPECT_EQ(table.bytes(), newCapacity);
  }

  uint32_t head_{0};
  uint32_t length_{0};
};

TEST_F(HeaderTableTests, IndexTranslation) {
  // simple cases
  length_ = 10;
  head_ = 5;
  xcheck(0, 6);
  xcheck(3, 3);
  xcheck(5, 1);

  // wrap
  head_ = 1;
  xcheck(0, 2);
  xcheck(8, 4);
  xcheck(5, 7);
}

TEST_F(HeaderTableTests, Add) {
  HeaderTable table(4096);
  HPACKHeader header("accept-encoding", "gzip");
  table.add(header.copy());
  table.add(header.copy());
  table.add(header.copy());
  EXPECT_EQ(table.names().size(), 1);
  EXPECT_EQ(table.hasName(header.name), true);
  auto it = table.names().find(header.name);
  EXPECT_EQ(it->second.size(), 3);
  EXPECT_EQ(table.nameIndex(header.name), 1);
}

TEST_F(HeaderTableTests, Evict) {
  HPACKHeaderName name("accept-encoding");
  HPACKHeader accept("accept-encoding", "gzip");
  HPACKHeader accept2("accept-encoding", "----"); // same size, different header
  HPACKHeader accept3("accept-encoding", "third"); // size is larger with 1 byte
  uint32_t max = 10;
  uint32_t capacity = accept.bytes() * max;
  HeaderTable table(capacity);
  // fill the table
  for (size_t i = 0; i < max; i++) {
    EXPECT_EQ(table.add(accept.copy()), true);
  }
  EXPECT_EQ(table.size(), max);
  EXPECT_EQ(table.add(accept2.copy()), true);
  // evict the first one
  EXPECT_EQ(table.getHeader(1), accept2);
  auto ilist = table.names().find(name)->second;
  EXPECT_EQ(ilist.size(), max);
  // evict all the 'accept' headers
  for (size_t i = 0; i < max - 1; i++) {
    EXPECT_EQ(table.add(accept2.copy()), true);
  }
  EXPECT_EQ(table.size(), max);
  EXPECT_EQ(table.getHeader(max), accept2);
  EXPECT_EQ(table.names().size(), 1);
  // add an entry that will cause 2 evictions
  EXPECT_EQ(table.add(accept3.copy()), true);
  EXPECT_EQ(table.getHeader(1), accept3);
  EXPECT_EQ(table.size(), max - 1);

  // add a super huge header
  string bigvalue;
  bigvalue.append(capacity, 'x');
  HPACKHeader bigheader("user-agent", bigvalue);
  EXPECT_EQ(table.add(bigheader.copy()), false);
  EXPECT_EQ(table.size(), 0);
  EXPECT_EQ(table.names().size(), 0);
}

TEST_F(HeaderTableTests, ReduceCapacity) {
  HPACKHeader accept("accept-encoding", "gzip");
  uint32_t max = 10;
  uint32_t capacity = accept.bytes() * max;
  HeaderTable table(capacity);
  EXPECT_LE(table.length(), table.getMaxTableLength(capacity));

  // fill the table
  for (size_t i = 0; i < max; i++) {
    EXPECT_EQ(table.add(accept.copy()), true);
  }
  // change capacity
  table.setCapacity(capacity / 2);
  EXPECT_EQ(table.size(), max / 2);
  EXPECT_EQ(table.bytes(), capacity / 2);
}

TEST_F(HeaderTableTests, Comparison) {
  uint32_t capacity = 128;
  HeaderTable t1(capacity);
  HeaderTable t2(capacity);

  HPACKHeader h1("Content-Encoding", "gzip");
  HPACKHeader h2("Content-Encoding", "deflate");
  // different in number of elements
  t1.add(h1.copy());
  EXPECT_FALSE(t1 == t2);
  // different in size (bytes)
  t2.add(h2.copy());
  EXPECT_FALSE(t1 == t2);

  // make them the same
  t1.add(h2.copy());
  t2.add(h1.copy());
  EXPECT_TRUE(t1 == t2);
}

TEST_F(HeaderTableTests, Print) {
  stringstream out;
  HeaderTable t(128);
  t.add(HPACKHeader("Accept-Encoding", "gzip"));
  out << t;
  EXPECT_EQ(out.str(), "\n[1] (s=51) accept-encoding: gzip\ntotal size: 51\n");
}

TEST_F(HeaderTableTests, IncreaseCapacity) {
  HPACKHeader accept("accept-encoding", "gzip");
  uint32_t max = 4;
  uint32_t capacity = accept.bytes() * max;
  HeaderTable table(capacity);
  EXPECT_LE(table.length(), table.getMaxTableLength(capacity));

  // fill the table
  uint32_t length = table.length() + 1;
  for (size_t i = 0; i < length; i++) {
    EXPECT_EQ(table.add(accept.copy()), true);
  }
  EXPECT_EQ(table.size(), max);
  EXPECT_EQ(table.getIndex(accept).first, 1);
  // head should be 0, tail should be 2
  max = 8;
  capacity = accept.bytes() * max;
  table.setCapacity(capacity);

  EXPECT_LE(table.length(), table.getMaxTableLength(capacity));
  // external index didn't change
  EXPECT_EQ(table.getIndex(accept).first, 1);
}

TEST_F(HeaderTableTests, VaryCapacity) {
  HPACKHeader accept("accept-encoding", "gzip");
  uint32_t max = 6;
  uint32_t capacity = accept.bytes() * max;
  HeaderTable table(capacity);

  // Fill the table (extra) and make sure we haven't violated our
  // size (bytes) limits (expected one entry to be evicted)
  for (size_t i = 0; i <= table.length(); ++i) {
    EXPECT_EQ(table.add(accept.copy()), true);
  }
  EXPECT_EQ(table.size(), max);

  // Size down the table and verify we are still honoring our size (bytes)
  // limits
  resizeAndFillTable(table, accept, 4, 5);

  // Size up the table (in between previous max and min within test) and verify
  // we are still horing our size (bytes) limits
  resizeAndFillTable(table, accept, 5, 6);

  // Finally reize up one last timestamps
  resizeAndFillTable(table, accept, 8, 9);
}

TEST_F(HeaderTableTests, VaryCapacityMalignHeadIndex) {
  // Test checks for a previous bug/crash condition where due to resizing
  // the underlying table to a size lower than a previous max but up from the
  // current size and the position of the head_ index an out of bounds index
  // would occur

  // Initialize header table
  HPACKHeader accept("accept-encoding", "gzip");
  uint32_t max = 6;
  uint32_t capacity = accept.bytes() * max;
  HeaderTable table(capacity);

  // Push head_ to last index in underlying table before potential wrap
  // This is our max table size for the duration of the test
  for (size_t i = 0; i < table.getMaxTableLength(capacity); ++i) {
    EXPECT_EQ(table.add(accept.copy()), true);
  }
  EXPECT_EQ(table.size(), max);
  EXPECT_EQ(table.bytes(), capacity);

  // Flush underlying table (head_ remains the same at the previous max index)
  // Header guranteed to cause a flush as header itself requires 32 bytes plus
  // the sizes of the name and value anyways (which themselves would cause a
  // flush)
  string strLargerThanTableCapacity = string(capacity + 1, 'a');
  HPACKHeader flush("flush", strLargerThanTableCapacity);
  EXPECT_EQ(table.add(flush.copy()), false);
  EXPECT_EQ(table.size(), 0);

  // Now reduce capacity of table (in functional terms table.size() is lowered
  // but currently table.length() remains the same)
  max = 3;
  resizeTable(table, accept.bytes() * max, max);

  // Increase capacity of table (but smaller than all time max; head_ still at
  // previous max index).  Previously (now fixed) this size up resulted in
  // incorrect resizing semantics
  max = 4;
  resizeTable(table, accept.bytes() * max, max);

  // Now try and add headers; there should be no crash with current position of
  // head_ in the underlying table.  Note this is merely one possible way we
  // could force the test to crash as a result of the resize bug this test was
  // added for
  for (size_t i = 0; i <= table.length(); ++i) {
    EXPECT_EQ(table.add(accept.copy()), true);
  }
  EXPECT_EQ(table.size(), max);
}

TEST_F(HeaderTableTests, AddLargerThanTable) {
  // Construct a smallish table
  uint32_t capacityBytes = 256;
  HeaderTable table(capacityBytes);
  HPACKHeaderName name("accept-encoding");
  table.add(HPACKHeader("accept-encoding", "gzip")); // internal index = 0
  table.add(HPACKHeader("accept-encoding", "gzip")); // internal index = 1
  table.add(HPACKHeader("test-encoding", "gzip"));   // internal index = 2
  EXPECT_EQ(table.names().size(), 2);

  // Attempt to add a header that is larger than our specified table capacity
  // bytes.  This should result in a table flush.
  table.add(HPACKHeader(std::string(capacityBytes, 'a'), "gzip"));
  EXPECT_EQ(table.names().size(), 0);

  // Add the previous headers to the table again
  table.add(HPACKHeader("accept-encoding", "gzip")); // internal index = 3
  table.add(HPACKHeader("accept-encoding", "gzip")); // internal index = 4
  table.add(HPACKHeader("test-encoding", "gzip"));   // internal index = 5
  EXPECT_EQ(table.names().size(), 2);

  EXPECT_EQ(table.hasName(name), true);
  auto it = table.names().find(name);
  EXPECT_EQ(it->second.size(), 2);
  // As nameIndex takes the last index added, we have head = 5, index = 4
  // and so yields a difference of one and as external indexing is 1 based,
  // we expect 2 here
  EXPECT_EQ(table.nameIndex(name), 2);
}

TEST_F(HeaderTableTests, IncreaseLengthOfFullTable) {
  HPACKHeader largeHeader("Access-Control-Allow-Credentials", "true");
  HPACKHeader smallHeader("Accept", "All-Content");

  HeaderTable table(448);
  CHECK_EQ(table.length(), 7);

  for (uint8_t count = 0; count < 3; count++) {
    table.add(largeHeader.copy());
    table.add(smallHeader.copy());
  } // tail is at index 0
  CHECK_EQ(table.length(), 7);

  table.add(smallHeader.copy());
  table.add(smallHeader.copy()); // tail is at index 1
  table.add(smallHeader.copy()); // resize on this add
  EXPECT_EQ(table.length(), 11);

  // Check table is correct after resize
  CHECK_EQ(table.getHeader(1), smallHeader);
  CHECK_EQ(table.getHeader(2), smallHeader);
  CHECK_EQ(table.getHeader(3), smallHeader);
  CHECK_EQ(table.getHeader(4), smallHeader);
  CHECK_EQ(table.getHeader(5), largeHeader);
  CHECK_EQ(table.getHeader(6), smallHeader);
  CHECK_EQ(table.getHeader(7), largeHeader);
  CHECK_EQ(table.getHeader(8), smallHeader);
}

TEST_F(HeaderTableTests, SmallTable) {
  HeaderTable table(80);
  HPACKHeader foo("Foo", "bar");
  EXPECT_TRUE(table.add(foo.copy()));
  EXPECT_TRUE(table.add(foo.copy()));
  EXPECT_EQ(table.size(), 2);
  EXPECT_EQ(table.length(), 2);
}

TEST_F(HeaderTableTests, TinyTable) {
  // This table can only hold 1 header, but it better be able to hold it!
  HeaderTable table(63);
  HPACKHeader foo("Foo", "barbarbarbarbarbarbar1");
  EXPECT_TRUE(table.add(foo.copy()));
  EXPECT_EQ(table.size(), 1);
  EXPECT_EQ(table.length(), 1);
}

} // namespace proxygen
