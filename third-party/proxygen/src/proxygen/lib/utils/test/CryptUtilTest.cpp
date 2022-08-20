/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <proxygen/lib/utils/CryptUtil.h>

#include <folly/portability/GTest.h>
#include <string>

using namespace proxygen;

using folly::ByteRange;

TEST(CryptUtilTest, Base64EncodeTest) {
  ASSERT_EQ("",
            base64Encode(ByteRange(reinterpret_cast<const unsigned char*>(""),
                                   (size_t)0)));
  ASSERT_EQ(
      "YQ==",
      base64Encode(ByteRange(reinterpret_cast<const unsigned char*>("a"), 1)));
  ASSERT_EQ(
      "YWE=",
      base64Encode(ByteRange(reinterpret_cast<const unsigned char*>("aa"), 2)));
  ASSERT_EQ(
      "QWxhZGRpbjpvcGVuIHNlc2FtZQ==",
      base64Encode(ByteRange(
          reinterpret_cast<const unsigned char*>("Aladdin:open sesame"), 19)));
}

TEST(CryptUtilTest, MD5EncodeTest) {
  ASSERT_EQ("d41d8cd98f00b204e9800998ecf8427e",
            md5Encode(ByteRange(reinterpret_cast<const unsigned char*>(""),
                                (size_t)0)));
  ASSERT_EQ(
      "a7a93b8ac14a48faa68e4afb57b00fc7",
      md5Encode(ByteRange(
          reinterpret_cast<const unsigned char*>("Aladdin:open sesame"), 19)));
}
