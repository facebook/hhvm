/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <folly/portability/GMock.h>
#include <folly/portability/GTest.h>

#include <fizz/record/Types.h>
#include <folly/String.h>

namespace fizz {
namespace test {

class ExtensionsTest : public testing::Test {
 protected:
  Buf getBuf(folly::StringPiece hex) {
    auto data = unhexlify(hex);
    return folly::IOBuf::copyBuffer(data.data(), data.size());
  }

  std::vector<Extension> getExtensions(folly::StringPiece hex) {
    auto buf = ExtensionsTest::getBuf(hex);
    folly::io::Cursor cursor(buf.get());
    Extension ext;
    EXPECT_EQ(detail::read(ext, cursor), buf->computeChainDataLength());
    EXPECT_TRUE(cursor.isAtEnd());
    std::vector<Extension> exts;
    exts.push_back(std::move(ext));
    return exts;
  }

  template <class T>
  void checkEncode(T&& ext, folly::StringPiece expectedHex) {
    auto encoded = encodeExtension(std::forward<T>(ext));
    auto buf = folly::IOBuf::create(0);
    folly::io::Appender appender(buf.get(), 10);
    detail::write(encoded, appender);
    EXPECT_TRUE(folly::IOBufEqualTo()(buf, getBuf(expectedHex)));
  }
};
} // namespace test
} // namespace fizz
