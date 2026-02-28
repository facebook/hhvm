/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#include <folly/portability/GTest.h>

#include <fizz/crypto/Utils.h>

using namespace folly;

namespace fizz {
namespace test {

TEST(UtilsTest, TestEqualWrongSize) {
  StringPiece a{"hi"};
  StringPiece b{"hello"};
  EXPECT_FALSE(CryptoUtils::equal(a, b));
  EXPECT_FALSE(CryptoUtils::equal(b, a));
}

TEST(UtilsTest, TestEqual) {
  StringPiece a{"hello!!"};
  StringPiece b{"hello!!"};
  StringPiece c{"goodbye"};
  EXPECT_TRUE(CryptoUtils::equal(a, b));
  EXPECT_FALSE(CryptoUtils::equal(a, c));
}

TEST(UtilsTest, TestClean) {
  std::array<uint8_t, 8> a{'p', 'a', 's', 's', 'w', 'o', 'r', 'd'};
  CryptoUtils::clean(range(a));
  for (auto byte : a) {
    EXPECT_EQ(byte, 0);
  }
}
} // namespace test
} // namespace fizz
