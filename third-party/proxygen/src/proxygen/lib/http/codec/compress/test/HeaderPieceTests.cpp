/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <folly/portability/GTest.h>
#include <glog/logging.h>
#include <proxygen/lib/http/codec/compress/HeaderPiece.h>

using namespace proxygen::compress;

class HeaderPieceTests : public testing::Test {};

TEST_F(HeaderPieceTests, Basic) {
  HeaderPiece *hp;

  // creating non-owner piece with null pointer
  hp = new HeaderPiece(nullptr, 0, false, true);
  EXPECT_TRUE(hp->isMultiValued());
  // destructing this should be fine, since will not try to release the memory
  delete hp;

  char *buf = new char[16];
  hp = new HeaderPiece(buf, 16, true, true);
  EXPECT_EQ(hp->str.data(), buf);
  EXPECT_EQ(hp->str.size(), 16);
  // this should release the mem
  delete hp;
}
