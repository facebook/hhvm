/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <algorithm>
#include <folly/String.h>
#include <folly/portability/GTest.h>
#include <list>
#include <memory>
#include <proxygen/lib/http/codec/compress/HPACKDecoder.h>
#include <proxygen/lib/http/codec/compress/HPACKEncoder.h>
#include <proxygen/lib/http/codec/compress/Logging.h>
#include <proxygen/lib/http/codec/compress/test/TestUtil.h>
#include <vector>

using namespace folly;
using namespace proxygen;
using namespace std;

using RfcParam = std::pair<bool, vector<string>>;

class RFCRequestTest : public testing::TestWithParam<RfcParam> {
 public:
  RFCRequestTest() {
    req1.push_back(HPACKHeader(":method", "GET"));
    req1.push_back(HPACKHeader(":scheme", "http"));
    req1.push_back(HPACKHeader(":path", "/"));
    req1.push_back(HPACKHeader(":authority", "www.example.com"));

    req2.push_back(HPACKHeader(":method", "GET"));
    req2.push_back(HPACKHeader(":scheme", "http"));
    req2.push_back(HPACKHeader(":path", "/"));
    req2.push_back(HPACKHeader(":authority", "www.example.com"));
    req2.push_back(HPACKHeader("cache-control", "no-cache"));

    req3.push_back(HPACKHeader(":method", "GET"));
    req3.push_back(HPACKHeader(":scheme", "https"));
    req3.push_back(HPACKHeader(":path", "/index.html"));
    req3.push_back(HPACKHeader(":authority", "www.example.com"));
    req3.push_back(HPACKHeader("custom-key", "custom-value"));
  }

 protected:
  vector<HPACKHeader> req1;
  vector<HPACKHeader> req2;
  vector<HPACKHeader> req3;
};

class RFCResponseTest : public testing::TestWithParam<RfcParam> {
 public:
  RFCResponseTest() {
    resp1.push_back(HPACKHeader(":status", "302"));
    resp1.push_back(HPACKHeader("cache-control", "private"));
    resp1.push_back(HPACKHeader("date", "Mon, 21 Oct 2013 20:13:21 GMT"));
    resp1.push_back(HPACKHeader("location", "https://www.example.com"));

    resp2.push_back(HPACKHeader(":status", "307"));
    resp2.push_back(HPACKHeader("cache-control", "private"));
    resp2.push_back(HPACKHeader("date", "Mon, 21 Oct 2013 20:13:21 GMT"));
    resp2.push_back(HPACKHeader("location", "https://www.example.com"));

    resp3.push_back(HPACKHeader(":status", "200"));
    resp3.push_back(HPACKHeader("cache-control", "private"));
    resp3.push_back(HPACKHeader("date", "Mon, 21 Oct 2013 20:13:22 GMT"));
    resp3.push_back(HPACKHeader("location", "https://www.example.com"));
    resp3.push_back(HPACKHeader("content-encoding", "gzip"));
    resp3.push_back(HPACKHeader(
        "set-cookie",
        "foo=ASDJKHQKBZXOQWEOPIUAXQWEOIU; max-age=3600; version=1"));
  }

 protected:
  vector<HPACKHeader> resp1;
  vector<HPACKHeader> resp2;
  vector<HPACKHeader> resp3;
};

vector<string> exampleHex1 = {
    "828684410f7777772e6578616d706c652e636f6d",
    "828684be58086e6f2d6361636865",
    "828785bf400a637573746f6d2d6b65790c637573746f6d2d76616c7565"};

vector<string> exampleHex2 = {
    "828684418cf1e3c2e5f23a6ba0ab90f4ff",
    "828684be5886a8eb10649cbf",
    "828785bf408825a849e95ba97d7f8925a849e95bb8e8b4bf"};

RfcParam d3(false, exampleHex1);
RfcParam d4(true, exampleHex2);

vector<string> exampleHex3 = {
    "4803333032580770726976617465611d4d6f6e2c203231204f63742032303133"
    "2032303a31333a323120474d546e1768747470733a2f2f7777772e6578616d70"
    "6c652e636f6d",
    "4803333037c1c0bf",
    "88c1611d4d6f6e2c203231204f637420323031332032303a31333a323220474d"
    "54c05a04677a69707738666f6f3d4153444a4b48514b425a584f5157454f5049"
    "5541585157454f49553b206d61782d6167653d333630303b2076657273696f6e"
    "3d31"};
vector<string> exampleHex4 = {
    "488264025885aec3771a4b6196d07abe941054d444a8200595040b8166e082a6"
    "2d1bff6e919d29ad171863c78f0b97c8e9ae82ae43d3",
    "4883640effc1c0bf",
    "88c16196d07abe941054d444a8200595040b8166e084a62d1bffc05a839bd9ab"
    "77ad94e7821dd7f2e6c7b335dfdfcd5b3960d5af27087f3672c1ab270fb5291f"
    "9587316065c003ed4ee5b1063d5007"};

RfcParam d5(false, exampleHex3);
RfcParam d6(true, exampleHex4);

namespace {
std::string unhexlify(const std::string& input) {
  std::string result;
  folly::unhexlify(input, result);
  return result;
}
} // namespace

TEST_P(RFCRequestTest, RfcExampleRequest) {
  HPACKEncoder encoder(GetParam().first);
  HPACKDecoder decoder;
  // first request
  unique_ptr<IOBuf> encoded = hpack::encodeDecode(req1, encoder, decoder);
  EXPECT_EQ(encoded->moveToFbString(), unhexlify(GetParam().second[0]));
  EXPECT_EQ(encoder.getTable().bytes(), 57);
  EXPECT_EQ(encoder.getTable().size(), 1);

  // second request
  encoded = hpack::encodeDecode(req2, encoder, decoder);
  EXPECT_EQ(encoded->moveToFbString(), unhexlify(GetParam().second[1]));
  EXPECT_EQ(encoder.getTable().bytes(), 110);
  EXPECT_EQ(encoder.getTable().size(), 2);

  // third request
  encoded = hpack::encodeDecode(req3, encoder, decoder);
  EXPECT_EQ(encoded->moveToFbString(), unhexlify(GetParam().second[2]));
  EXPECT_EQ(encoder.getTable().bytes(), 164);
  EXPECT_EQ(encoder.getTable().size(), 3);
}

INSTANTIATE_TEST_SUITE_P(Huffman, RFCRequestTest, ::testing::Values(d3, d4));

TEST_P(RFCResponseTest, RfcExampleResponse) {
  // this test does some evictions
  uint32_t tableSize = 256;
  HPACKEncoder encoder(GetParam().first, tableSize);
  HPACKDecoder decoder(tableSize);

  // first
  unique_ptr<IOBuf> encoded = hpack::encodeDecode(resp1, encoder, decoder);
  EXPECT_EQ(encoded->moveToFbString(), unhexlify(GetParam().second[0]));
  EXPECT_EQ(encoder.getTable().bytes(), 222);
  EXPECT_EQ(encoder.getTable().size(), 4);
  EXPECT_EQ(encoder.getHeader(64).name.get(), "cache-control");
  EXPECT_EQ(encoder.getHeader(64).value, "private");

  // second
  encoded = hpack::encodeDecode(resp2, encoder, decoder);
  EXPECT_EQ(encoded->moveToFbString(), unhexlify(GetParam().second[1]));
  EXPECT_EQ(encoder.getTable().bytes(), 222);
  EXPECT_EQ(encoder.getTable().size(), 4);

  // third
  encoded = hpack::encodeDecode(resp3, encoder, decoder);
  EXPECT_EQ(encoded->moveToFbString(), unhexlify(GetParam().second[2]));

  EXPECT_EQ(encoder.getTable().size(), 3);
  EXPECT_EQ(encoder.getTable().bytes(), 215);
}

INSTANTIATE_TEST_SUITE_P(Huffman, RFCResponseTest, ::testing::Values(d5, d6));
