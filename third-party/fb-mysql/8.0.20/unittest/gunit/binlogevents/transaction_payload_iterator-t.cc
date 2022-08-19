/* Copyright (c) 2019, Oracle and/or its affiliates. All rights reserved.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License, version 2.0,
   as published by the Free Software Foundation.

   This program is also distributed with certain software (including
   but not limited to OpenSSL) that is licensed under separate terms,
   as designated in a particular file or component or in included license
   documentation.  The authors of MySQL hereby grant you an additional
   permission to link the program and your derivative works with the
   separately licensed software that they have included with MySQL.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License, version 2.0, for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA
*/

#include <array>
#include <chrono>
#include <vector>

#include <gtest/gtest.h>
#include "libbinlogevents/include/binary_log.h"
#include "libbinlogevents/include/compression/iterator.h"
#include "libbinlogevents/include/compression/zstd.h"
#include "my_byteorder.h"

namespace binary_log {
namespace transaction {
namespace unittests {

class TransactionPayloadIteratorTest : public ::testing::Test {
 public:
  std::vector<std::size_t> m_payloads;

 protected:
  TransactionPayloadIteratorTest() {}

  virtual void SetUp() {}

  virtual void TearDown() {}

  static unsigned char *mock_event(unsigned char *buffer, Log_event_type type) {
    std::time_t timestamp =
        std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());

    int4store(buffer, timestamp);
    buffer[EVENT_TYPE_OFFSET] = type;
    int4store(buffer + SERVER_ID_OFFSET, 1);
    int4store(buffer + EVENT_LEN_OFFSET,
              static_cast<uint32>(LOG_EVENT_HEADER_LEN));
    int4store(buffer + LOG_POS_OFFSET, static_cast<uint32>(0));
    int2store(buffer + FLAGS_OFFSET, 0);

    return buffer + LOG_EVENT_HEADER_LEN;
  }

  static void iterator_test_compressed() {
    size_t buffer_size{LOG_EVENT_HEADER_LEN * 5};
    size_t osize{0};
    unsigned char *ibuf = (unsigned char *)malloc(buffer_size);
    unsigned char *obuf = (unsigned char *)malloc(buffer_size);
    unsigned char *ptr{ibuf};

    // the compressor used
    binary_log::transaction::compression::Zstd_comp c;

    std::vector<Log_event_type> types = {QUERY_EVENT, ROWS_QUERY_LOG_EVENT,
                                         TABLE_MAP_EVENT, WRITE_ROWS_EVENT,
                                         XID_EVENT};

    // fake the events
    for (auto type : types) {
      ptr = mock_event(ptr, type);
    }

    // compress
    c.set_buffer(obuf, buffer_size);
    c.open();
    c.compress(ibuf, buffer_size);
    c.close();

    // get the compressed buffer
    std::tie(obuf, osize, std::ignore) = c.get_buffer();

    // create iterator over the compressed buffer
    binary_log::transaction::compression::Iterable_buffer ib(
        (const char *)obuf, osize, buffer_size, c.compression_type_code());

    int i = 0;
    for (auto p : ib) {
      Log_event_type type = types[i];
      ASSERT_EQ(type, p[EVENT_TYPE_OFFSET]);
      i++;
    }
    free(obuf);
    free(ibuf);
  }
};

TEST_F(TransactionPayloadIteratorTest, IteratorBufferTest) {
  TransactionPayloadIteratorTest::iterator_test_compressed();
}

}  // namespace unittests
}  // namespace transaction
}  // namespace binary_log
