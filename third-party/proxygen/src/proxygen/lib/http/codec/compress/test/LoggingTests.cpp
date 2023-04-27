/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <folly/portability/GTest.h>
#include <list>
#include <proxygen/lib/http/codec/compress/HPACKEncodeBuffer.h>
#include <proxygen/lib/http/codec/compress/HPACKHeader.h>
#include <proxygen/lib/http/codec/compress/Logging.h>
#include <sstream>
#include <vector>

using namespace proxygen;
using namespace std;

class LoggingTests : public testing::Test {};

TEST_F(LoggingTests, Refset) {
  list<uint32_t> refset;
  refset.push_back(3);
  refset.push_back(5);
  stringstream out;
  out << &refset;
  EXPECT_EQ(out.str(), "\n[3 5 ]\n");
}

TEST_F(LoggingTests, DumpHeaderVector) {
  vector<HPACKHeader> headers;
  headers.push_back(HPACKHeader(":path", "index.html"));
  headers.push_back(HPACKHeader("content-type", "gzip"));
  stringstream out;
  out << headers;
  EXPECT_EQ(out.str(), ":path: index.html\ncontent-type: gzip\n\n");
}

TEST_F(LoggingTests, PrintDelta) {
  vector<HPACKHeader> v1;
  v1.push_back(HPACKHeader(":path", "/"));
  v1.push_back(HPACKHeader(":host", "www.facebook.com"));
  vector<HPACKHeader> v2;

  // empty v1 or v2
  EXPECT_EQ(printDelta(v1, v2), "\n + :path: /\n + :host: www.facebook.com\n");
  EXPECT_EQ(printDelta(v2, v1), "\n - :path: /\n - :host: www.facebook.com\n");

  // skip first header from v1
  v2.push_back(HPACKHeader(":path", "/"));
  EXPECT_EQ(printDelta(v1, v2), "\n + :host: www.facebook.com\n");

  v2.push_back(HPACKHeader(":path", "/"));
  EXPECT_EQ(printDelta(v2, v1),
            "\n - :host: www.facebook.com\n + :path: / (duplicate)\n");

  v2.pop_back();
  v1.clear();
  v1.push_back(HPACKHeader(":a", "b"));
  v1.push_back(HPACKHeader(":a", "b"));
  v1.push_back(HPACKHeader(":path", "/"));
  EXPECT_EQ(printDelta(v1, v2), "\n + :a: b\n duplicate :a: b\n");
}

TEST_F(LoggingTests, DumpBin) {
  // test with an HPACKEncodeBuffer
  HPACKEncodeBuffer buf(128);
  buf.encodeLiteral("test");
  EXPECT_EQ(buf.toBin(),
            "00000100   01110100 t 01100101 e 01110011 s 01110100 t \n");
}
