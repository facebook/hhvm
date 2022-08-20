/*
 * Copyright (c) Facebook, Inc. and its affiliates.
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

#include <thrift/lib/cpp/protocol/TBase64Utils.h>

#include <folly/portability/GTest.h>

using apache::thrift::protocol::base64_decode;
using apache::thrift::protocol::base64_encode;
using apache::thrift::protocol::base64Decode;
using apache::thrift::protocol::base64Encode;

static void setupTestData(int i, uint8_t* data, int& len) {
  len = 0;
  do {
    data[len] = (uint8_t)(i & 0xFF);
    i >>= 8;
    len++;
  } while ((len < 3) && (i != 0));

  ASSERT_EQ(0, i);
}

void checkEncoding(uint8_t* data, int len) {
  for (int i = 0; i < len; i++) {
    ASSERT_TRUE(isalnum(data[i]) || data[i] == '/' || data[i] == '+');
  }
}

TEST(Base64Test, test_Base64_Encode_Decode) {
  int len;
  uint8_t testInput[3];
  uint8_t testOutput[4];

  // Test all possible encoding / decoding cases given the
  // three byte limit for base64_encode.

  for (int i = 0xFFFFFF; i >= 0; i--) {
    // fill testInput based on i
    setupTestData(i, testInput, len);

    // encode the test data, then decode it again
    base64_encode(testInput, len, testOutput);

    // verify each byte has a valid Base64 value (alphanumeric or either + or /)
    checkEncoding(testOutput, len);

    // decode output and check that it matches input
    base64_decode(testOutput, len + 1);
    ASSERT_EQ(0, memcmp(testInput, testOutput, len));
  }
}

TEST(Base64Test, base64EncodeBasic) {
  EXPECT_EQ(std::string("YWJjZA=="), base64Encode(folly::StringPiece("abcd")));
  EXPECT_EQ(std::string("YWJjZGU="), base64Encode(folly::StringPiece("abcde")));
  EXPECT_EQ(
      std::string("YWJjZGVm"), base64Encode(folly::StringPiece("abcdef")));
}

TEST(Base64Test, base64DecodeBasic) {
  EXPECT_EQ(
      folly::fbstring("abcd"), base64Decode("YWJjZA==")->moveToFbString());
  EXPECT_EQ(
      folly::fbstring("abcde"), base64Decode("YWJjZGU=")->moveToFbString());
  EXPECT_EQ(
      folly::fbstring("abcdef"), base64Decode("YWJjZGVm")->moveToFbString());
}
