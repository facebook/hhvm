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

#include <thrift/lib/cpp2/protocol/detail/index.h>

#include <gtest/gtest.h>

namespace apache::thrift::detail {

using folly::io::Cursor;

int64_t xxh3_64bits(Cursor cursor) {
  Xxh3Hasher hasher;
  hasher.init();
  hasher.update(cursor);
  return static_cast<int64_t>(hasher);
}

TEST(xxh3_64bits, test) {
  constexpr int64_t kExpected = 5501799519012602454; // 64bits xxh3 of "thrift"
  {
    auto buf = folly::IOBuf::copyBuffer("thrift");
    EXPECT_EQ(xxh3_64bits(Cursor{buf.get(), 6}), kExpected);
  }
  {
    auto buf = folly::IOBuf::copyBuffer("thrif_");
    EXPECT_NE(xxh3_64bits(Cursor{buf.get(), 6}), kExpected);
  }
  {
    auto buf = folly::IOBuf::copyBuffer("thrift_");
    EXPECT_EQ(xxh3_64bits(Cursor{buf.get(), 6}), kExpected);
  }
  {
    auto buf = folly::IOBuf::copyBuffer("thrift_");
    EXPECT_NE(xxh3_64bits(Cursor{buf.get(), 7}), kExpected);
  }
  {
    auto buf = folly::IOBuf::copyBuffer("_thrift");
    Cursor io{buf.get()};
    EXPECT_EQ(xxh3_64bits(Cursor{io + 1, 6}), kExpected);
  }
  {
    auto buf = folly::IOBuf::copyBuffer("_thrift_");
    Cursor io{buf.get()};
    io += 1;
    EXPECT_EQ(xxh3_64bits(Cursor{io, 6}), kExpected);
  }
  {
    auto buf = folly::IOBuf::copyBuffer("thr");
    buf->appendChain(folly::IOBuf::copyBuffer("ift"));
    EXPECT_EQ(xxh3_64bits(Cursor{buf.get(), 6}), kExpected);
  }
  {
    auto buf = folly::IOBuf::copyBuffer("_thr");
    buf->appendChain(folly::IOBuf::copyBuffer("ift_"));
    Cursor io{buf.get()};
    EXPECT_EQ(xxh3_64bits(Cursor{io + 1, 6}), kExpected);
  }
  {
    auto buf = folly::IOBuf::copyBuffer("_th");
    auto buf2 = folly::IOBuf::copyBuffer("ri");
    auto buf3 = folly::IOBuf::copyBuffer("ft_");
    buf2->appendChain(std::move(buf3));
    buf->appendChain(std::move(buf2));
    Cursor io{buf.get()};
    EXPECT_EQ(xxh3_64bits(Cursor{io + 1, 6}), kExpected);
  }
  {
    auto buf1 = folly::IOBuf::copyBuffer("_th");
    auto buf2 = folly::IOBuf::copyBuffer("ri");
    auto buf3 = folly::IOBuf::copyBuffer("ft_");
    Cursor cursor1{buf1.get()};
    Cursor cursor2{buf2.get()};
    Cursor cursor3{buf3.get()};
    Xxh3Hasher hasher;
    hasher.init();
    hasher.update(Cursor{cursor1 + 1, 2});
    hasher.update(Cursor{cursor2, 2});
    hasher.update(Cursor{cursor3, 2});
    EXPECT_EQ(static_cast<int64_t>(hasher), kExpected);
  }
}

} // namespace apache::thrift::detail
