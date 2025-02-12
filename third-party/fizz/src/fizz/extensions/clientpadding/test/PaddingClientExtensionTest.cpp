/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#include <folly/portability/GMock.h>
#include <folly/portability/GTest.h>

#include <fizz/extensions/clientpadding/PaddingClientExtension.h>
#include <fizz/record/test/ExtensionTestsBase.h>

using namespace testing;

namespace fizz {
namespace extensions {
namespace test {

class PaddingClientExtensionTest : public ::fizz::test::ExtensionsTest {
 protected:
  void checkWrite(
      const PaddingClientExtension& padding,
      folly::StringPiece expectedHex) {
    auto ext = padding.getClientHelloExtensions()[0].clone();
    auto buf = folly::IOBuf::create(0);
    folly::io::Appender appender(buf.get(), 10);
    detail::write(ext, appender);
    EXPECT_EQ(hexlify(buf->coalesce()), expectedHex);
  }
};

TEST_F(PaddingClientExtensionTest, TestPadding) {
  PaddingClientExtension padding(1234);
  static const auto expectedHex = "001504ce" + std::string((1234 - 4) * 2, '0');
  checkWrite(padding, expectedHex);
}

TEST_F(PaddingClientExtensionTest, TestPaddingTooShort) {
  PaddingClientExtension padding(3);
  static const auto expectedHex = "00150000";
  checkWrite(padding, expectedHex);
}
} // namespace test
} // namespace extensions
} // namespace fizz
