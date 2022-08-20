/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <folly/portability/GTest.h>
#include <proxygen/lib/http/HTTPCommonHeaders.h>

using namespace proxygen;

class HTTPCommonHeadersTests : public testing::Test {};

TEST_F(HTTPCommonHeadersTests, TestHashing) {
  std::string common1("Content-Length");
  std::string common2("content-length");
  std::string uncommon("Uncommon");

  HTTPHeaderCode commonCode1 = HTTPCommonHeaders::hash(common1);
  HTTPHeaderCode commonCode2 = HTTPCommonHeaders::hash(common2);
  HTTPHeaderCode uncommonCode = HTTPCommonHeaders::hash(uncommon);

  EXPECT_EQ(uncommonCode, HTTPHeaderCode::HTTP_HEADER_OTHER);
  EXPECT_NE(commonCode1, HTTPHeaderCode::HTTP_HEADER_OTHER);

  EXPECT_EQ(commonCode1, commonCode2);
}

TEST_F(HTTPCommonHeadersTests, TestTwoTablesInitialized) {
  std::string common("Content-Length");
  HTTPHeaderCode code = HTTPCommonHeaders::hash(common);

  EXPECT_EQ(*HTTPCommonHeaders::getPointerToName(code), "Content-Length");
  EXPECT_EQ(*HTTPCommonHeaders::getPointerToName(
                code, HTTPCommonHeaderTableType::TABLE_LOWERCASE),
            "content-length");
}

TEST_F(HTTPCommonHeadersTests, TestIsCommonHeaderNameFromTable) {
  // The first two hardcoded headers are not considered actual common headers
  EXPECT_FALSE(HTTPCommonHeaders::isNameFromTable(
      HTTPCommonHeaders::getPointerToName(HTTP_HEADER_NONE),
      HTTPCommonHeaderTableType::TABLE_CAMELCASE));
  EXPECT_FALSE(HTTPCommonHeaders::isNameFromTable(
      HTTPCommonHeaders::getPointerToName(HTTP_HEADER_OTHER),
      HTTPCommonHeaderTableType::TABLE_CAMELCASE));

  // Verify that the first actual common header in the address table checks out
  // Assuming there is at least one common header in the table (first two
  // entries are HTTP_HEADER_NONE and HTTP_HEADER_OTHER)
  if (HTTPCommonHeaders::num_codes > HTTPHeaderCodeCommonOffset) {
    EXPECT_TRUE(HTTPCommonHeaders::isNameFromTable(
        HTTPCommonHeaders::getPointerToName(
            static_cast<HTTPHeaderCode>(HTTPHeaderCodeCommonOffset + 1)),
        HTTPCommonHeaderTableType::TABLE_CAMELCASE));

    // Verify that the last header in the common address table checks out
    EXPECT_TRUE(HTTPCommonHeaders::isNameFromTable(
        HTTPCommonHeaders::getPointerToName(
            static_cast<HTTPHeaderCode>(HTTPCommonHeaders::num_codes - 1)),
        HTTPCommonHeaderTableType::TABLE_CAMELCASE));
  }

  // Verify that a random header is not identified as being part of the common
  // address table
  std::string externalHeader = "externalHeader";
  EXPECT_FALSE(HTTPCommonHeaders::isNameFromTable(
      &externalHeader, HTTPCommonHeaderTableType::TABLE_CAMELCASE));
}

TEST_F(HTTPCommonHeadersTests, TestGetHeaderCodeFromTableCommonHeaderName) {
  for (uint64_t j = HTTPHeaderCodeCommonOffset;
       j < HTTPCommonHeaders::num_codes;
       ++j) {
    HTTPHeaderCode code = static_cast<HTTPHeaderCode>(j);
    EXPECT_TRUE(code == HTTPCommonHeaders::getCodeFromTableName(
                            HTTPCommonHeaders::getPointerToName(code),
                            HTTPCommonHeaderTableType::TABLE_CAMELCASE));
  }
  std::string externalHeader = "externalHeader";
  EXPECT_TRUE(HTTP_HEADER_OTHER ==
              HTTPCommonHeaders::getCodeFromTableName(
                  &externalHeader, HTTPCommonHeaderTableType::TABLE_CAMELCASE));
}
