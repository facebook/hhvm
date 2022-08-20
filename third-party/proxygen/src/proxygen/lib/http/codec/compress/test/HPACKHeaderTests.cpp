/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <folly/portability/GTest.h>
#include <proxygen/lib/http/codec/compress/HPACKHeader.h>

#include <glog/logging.h>

#include <proxygen/lib/http/codec/compress/HeaderIndexingStrategy.h>
#include <sstream>

using namespace proxygen;
using namespace std;

class HPACKHeaderTests : public testing::Test {};

TEST_F(HPACKHeaderTests, Size) {
  HPACKHeader h(":path", "/");
  EXPECT_EQ(h.bytes(), 32 + 5 + 1);
}

TEST_F(HPACKHeaderTests, Operators) {
  HPACKHeader h0(":path", "/");
  HPACKHeader h1(":path", "/");
  HPACKHeader h2(":path", "/index.php");
  HPACKHeader h3("x-fb-debug", "test");
  // ==
  EXPECT_TRUE(h0 == h1);
  EXPECT_FALSE(h1 == h2);
  // <
  EXPECT_FALSE(h1 < h1);
  EXPECT_TRUE(h1 < h2);
  EXPECT_TRUE(h1 < h3);
  // >
  EXPECT_FALSE(h2 > h2);
  EXPECT_TRUE(h3 > h2);
  EXPECT_TRUE(h2 > h1);

  stringstream out;
  out << h1;
  EXPECT_EQ(out.str(), ":path: /");
}

TEST_F(HPACKHeaderTests, HasValue) {
  HPACKHeader h1(":path", "");
  HPACKHeader h2(":path", "/");
  EXPECT_FALSE(h1.hasValue());
  EXPECT_TRUE(h2.hasValue());
}

TEST_F(HPACKHeaderTests, HeaderIndexingStrategyBasic) {
  HeaderIndexingStrategy indexingStrat;
  HPACKHeader path(":path", "index.php?q=42");
  EXPECT_FALSE(indexingStrat.indexHeader(path.name, path.value));
  HPACKHeader cdn(":path", "/hprofile-ak-prn1/49496_6024432_1026115112_n.jpg");
  EXPECT_FALSE(indexingStrat.indexHeader(cdn.name, cdn.value));
  HPACKHeader clen("content-length", "512");
  EXPECT_FALSE(indexingStrat.indexHeader(clen.name, clen.value));
  HPACKHeader data("data", "value");
  EXPECT_TRUE(indexingStrat.indexHeader(data.name, data.value));
}

class HPACKHeaderNameTest : public testing::Test {};

HPACKHeaderName destroyedHPACKHeaderName(std::string name) {
  // return a HPACKHeaderName that goes destroyed
  HPACKHeaderName headerName(name);
  return headerName;
}

TEST_F(HPACKHeaderNameTest, TestConstructor) {
  // Test constructor
  HPACKHeaderName name1("accept-encoding");
  HPACKHeaderName name2("content-length");
  HPACKHeaderName name3("uncommon-name");
  HPACKHeaderName name4("uncommon-name-2");
  EXPECT_EQ(name1.get(), "accept-encoding");
  EXPECT_EQ(name2.get(), "content-length");
  EXPECT_EQ(name3.get(), "uncommon-name");
  EXPECT_EQ(name4.get(), "uncommon-name-2");
}

TEST_F(HPACKHeaderNameTest, TestCopyConstructor) {
  HPACKHeaderName name1("accept-encoding");
  HPACKHeaderName name2("uncommon-name");

  // Test copy constructor
  HPACKHeaderName name3(name1);
  HPACKHeaderName name4(name2);
  HPACKHeaderName name5(destroyedHPACKHeaderName("x-fb-debug"));
  HPACKHeaderName name6(destroyedHPACKHeaderName("uncommon-name"));
  EXPECT_EQ(name3.get(), "accept-encoding");
  EXPECT_EQ(name4.get(), "uncommon-name");
  EXPECT_EQ(name5.get(), "x-fb-debug");
  EXPECT_EQ(name6.get(), "uncommon-name");
}

TEST_F(HPACKHeaderNameTest, TestMoveConstructor) {
  HPACKHeaderName name1("accept-encoding");
  HPACKHeaderName name2("uncommon-name");

  // Test move constructor
  HPACKHeaderName name3(std::move(name1));
  HPACKHeaderName name4(std::move(name2));
  EXPECT_EQ(name3.get(), "accept-encoding");
  EXPECT_EQ(name4.get(), "uncommon-name");
}

TEST_F(HPACKHeaderNameTest, TestAssignmentOperators) {
  std::string testHeaderName = "accept-encoding";

  HPACKHeaderName name1(testHeaderName);
  EXPECT_EQ(name1.get(), testHeaderName);

  // Explicitly test some self assignment overloads
#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunknown-warning-option"
#pragma clang diagnostic ignored "-Wself-assign-overloaded"
#endif
  name1 = name1;
#ifdef __clang__
#pragma clang diagnostic pop
#endif
  EXPECT_EQ(name1.get(), testHeaderName);
  HPACKHeaderName* pName1 = &name1;
  // Specifically require a temporary above to throw off the compiler/lint:
  // explicitly moving variable of type 'proxygen::HPACKHeaderName' to itself
  name1 = std::move(*pName1);
  EXPECT_EQ(name1.get(), testHeaderName);

  std::string otherHeaderName = "uncommon-name";
  HPACKHeaderName name2(otherHeaderName);
  name1 = name2;
  EXPECT_EQ(name1.get(), otherHeaderName);
  EXPECT_EQ(name2.get(), otherHeaderName);

  HPACKHeaderName name3(testHeaderName);
  name1 = std::move(name3);
  EXPECT_EQ(name1.get(), testHeaderName);

  name1 = otherHeaderName;
  EXPECT_EQ(name1.get(), otherHeaderName);
}

TEST_F(HPACKHeaderNameTest, TestGetSize) {
  // Test size()
  HPACKHeaderName name1("accept-encoding");
  HPACKHeaderName name2("uncommon-header_now");
  EXPECT_EQ(name1.size(), 15);
  EXPECT_EQ(name2.size(), 19);
}

TEST_F(HPACKHeaderNameTest, TestOperators) {
  // Test operators
  HPACKHeaderName name1("aaa");
  HPACKHeaderName name2("bbb");
  HPACKHeaderName name3("aaa");
  HPACKHeaderName name4("bbb");
  CHECK(name1 == name3);
  CHECK(name1 != name2);
  CHECK(name1 < name2);
  CHECK(name2 > name1);
  CHECK(name1 >= name3);
  CHECK(name2 >= name1);
  CHECK(name2 <= name4);
  CHECK(name1 <= name2);
}

TEST_F(HPACKHeaderNameTest, TestIsCommonHeader) {
  for (uint64_t j = 0; j < HTTPCommonHeaders::num_codes; ++j) {
    HTTPHeaderCode code = static_cast<HTTPHeaderCode>(j);
    HPACKHeader testHPACKHeader(*HTTPCommonHeaders::getPointerToName(code), "");

    bool checkResult = j >= HTTPHeaderCodeCommonOffset;
    EXPECT_EQ(testHPACKHeader.name.isCommonHeader(), checkResult);
  }
  std::string externalHeader = "externalHeader";
  HPACKHeader testHPACKHeader(externalHeader, "");
  EXPECT_FALSE(testHPACKHeader.name.isCommonHeader());
}
