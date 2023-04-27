/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <folly/Range.h>
#include <folly/io/Cursor.h>
#include <folly/io/IOBuf.h>
#include <folly/portability/GTest.h>
#include <glog/logging.h>
#include <proxygen/lib/http/codec/compress/Header.h>
#include <proxygen/lib/http/codec/compress/HeaderCodec.h>
#include <proxygen/lib/http/codec/compress/QPACKCodec.h>
#include <proxygen/lib/http/codec/compress/test/TestStreamingCallback.h>
#include <proxygen/lib/http/codec/compress/test/TestUtil.h>
#include <vector>

using namespace folly;
using namespace proxygen::compress;
using namespace proxygen::hpack;
using namespace proxygen;
using namespace std;

namespace {
void headersEq(vector<Header>& headerVec, compress::HeaderPieceList& headers) {
  size_t i = 0;
  EXPECT_EQ(headerVec.size() * 2, headers.size());
  for (auto& h : headerVec) {
    string name = *h.name;
    char* mutableName = (char*)name.data();
    folly::toLowerAscii(mutableName, name.size());
    EXPECT_EQ(name, headers[i++].str);
    EXPECT_EQ(*h.value, headers[i++].str);
  }
}
} // namespace

class QPACKTests : public testing::Test {
 public:
  void SetUp() override {
    server.setDecoderHeaderTableMaxSize(5120);
    client.setDecoderHeaderTableMaxSize(5120);
    EXPECT_TRUE(server.setEncoderHeaderTableSize(4096));
  }

 protected:
  void controlAck() {
    auto ack = server.encodeInsertCountInc();
    EXPECT_EQ(client.decodeDecoderStream(std::move(ack)),
              HPACK::DecodeError::NONE);
  }

  void headerAck(uint64_t streamId) {
    auto ack = server.encodeHeaderAck(streamId);
    EXPECT_EQ(client.decodeDecoderStream(std::move(ack)),
              HPACK::DecodeError::NONE);
  }

  QPACKCodec client;
  QPACKCodec server;
};

TEST_F(QPACKTests, TestEncoderTableSize) {
  EXPECT_TRUE(client.setEncoderHeaderTableSize(0));
  EXPECT_TRUE(client.setEncoderHeaderTableSize(0));
  EXPECT_TRUE(client.setEncoderHeaderTableSize(4096));
  EXPECT_TRUE(client.setEncoderHeaderTableSize(4096));
  EXPECT_FALSE(client.setEncoderHeaderTableSize(1024));
  EXPECT_FALSE(client.setEncoderHeaderTableSize(5120));
}

TEST_F(QPACKTests, TestSimple) {
  EXPECT_TRUE(client.setEncoderHeaderTableSize(4096));
  vector<Header> req = basicHeaders();
  auto encodeResult = client.encode(req, 1);
  ASSERT_NE(encodeResult.control.get(), nullptr);
  EXPECT_EQ(server.decodeEncoderStream(std::move(encodeResult.control)),
            HPACK::DecodeError::NONE);
  TestStreamingCallback cb;
  auto length = encodeResult.stream->computeChainDataLength();
  server.decodeStreaming(1, std::move(encodeResult.stream), length, &cb);
  headerAck(1);
  auto result = cb.getResult();
  EXPECT_TRUE(!result.hasError());
  headersEq(req, result->headers);
  EXPECT_GT(client.getCompressionInfo().egress.headersStored_, 0);
  EXPECT_GT(server.getCompressionInfo().ingress.headersStored_, 0);
}

TEST_F(QPACKTests, TestAbsoluteIndex) {
  EXPECT_TRUE(client.setEncoderHeaderTableSize(4096));
  int flights = 10;
  for (int i = 0; i < flights; i++) {
    vector<vector<string>> headers;
    for (int j = 0; j < 32; j++) {
      int value = (i >> 1) * 32 + j; // duplicate the last flight
      headers.emplace_back(
          vector<string>({string("foomonkey"), folly::to<string>(value)}));
    }
    auto req = headersFromArray(headers);
    auto encodeResult = client.encode(req, i + 1);
    if (i % 2 == 1) {
      EXPECT_EQ(encodeResult.control.get(), nullptr);
    } else {
      ASSERT_NE(encodeResult.control.get(), nullptr);
      CHECK_EQ(server.decodeEncoderStream(std::move(encodeResult.control)),
               HPACK::DecodeError::NONE);
    }
    TestStreamingCallback cb;
    auto length = encodeResult.stream->computeChainDataLength();
    server.decodeStreaming(i + 1, std::move(encodeResult.stream), length, &cb);
    headerAck(i + 1);
    auto result = cb.getResult();
    EXPECT_TRUE(!result.hasError());
    headersEq(req, result->headers);
  }
  EXPECT_GT(client.getCompressionInfo().egress.headersStored_, 0);
  EXPECT_GT(server.getCompressionInfo().ingress.headersStored_, 0);
}

TEST_F(QPACKTests, TestWithQueue) {
  // Sends 10 flights of 4 requests each
  // Each request contains two 'connection' headers, one with the current
  // index, and current index - 8.
  // Each flight is processed in the order 0, 3, 2, 1, unless an eviction
  // happens on 2 or 3, in which case we force an blocking event.
  vector<Header> req = basicHeaders();
  vector<string> values;
  int flights = 10;
  for (auto i = 0; i < flights * 4; i++) {
    values.push_back(folly::to<string>(i));
  }
  EXPECT_TRUE(client.setEncoderHeaderTableSize(1024));
  for (auto f = 0; f < flights; f++) {
    vector<std::pair<unique_ptr<IOBuf>, TestStreamingCallback>> data;
    list<unique_ptr<IOBuf>> controlFrames;
    for (int i = 0; i < 4; i++) {
      auto reqI = req;
      for (int j = 0; j < 2; j++) {
        reqI.emplace_back(HTTP_HEADER_CONNECTION,
                          values[std::max(f * 4 + i - j * 8, 0)]);
      }
      VLOG(4) << "Encoding req=" << f * 4 + i;
      auto res = client.encode(reqI, f * 4 + i);
      if (res.control && res.control->computeChainDataLength() > 0) {
        controlFrames.emplace_back(std::move(res.control));
      }
      data.emplace_back(std::move(res.stream), TestStreamingCallback());
    }

    std::vector<int> insertOrder{0, 3, 2, 1};
    if (!controlFrames.empty()) {
      auto control = std::move(controlFrames.front());
      controlFrames.pop_front();
      server.decodeEncoderStream(std::move(control));
    }
    for (auto i : insertOrder) {
      auto& encodedReq = data[i].first;
      auto len = encodedReq->computeChainDataLength();
      server.decodeStreaming(i, std::move(encodedReq), len, &data[i].second);
    }
    while (!controlFrames.empty()) {
      auto control = std::move(controlFrames.front());
      controlFrames.pop_front();
      server.decodeEncoderStream(std::move(control));
    }
    int i = 0;
    for (auto& d : data) {
      auto result = d.second.getResult();
      EXPECT_TRUE(!result.hasError());
      auto reqI = req;
      for (int j = 0; j < 2; j++) {
        reqI.emplace_back(HTTP_HEADER_CONNECTION,
                          values[std::max(f * 4 + i - j * 8, 0)]);
      }
      headersEq(reqI, result->headers);
      headerAck(f * 4 + i);
      i++;
    }
    VLOG(4) << "getHolBlockCount=" << server.getHolBlockCount();
  }
  // Skipping redundant table adds reduces the HOL block count
  EXPECT_EQ(server.getHolBlockCount(), 30);

  EXPECT_GT(client.getCompressionInfo().egress.headersStored_, 0);
  EXPECT_GT(server.getCompressionInfo().ingress.headersStored_, 0);
}

TEST_F(QPACKTests, HeaderCodecStats) {
  vector<vector<string>> headers = {
      {"Content-Length", "80"},
      {"Content-Encoding", "gzip"},
      {"X-FB-Debug", "eirtijvdgtccffkutnbttcgbfieghgev"}};
  vector<Header> resp = headersFromArray(headers);

  TestHeaderCodecStats stats(HeaderCodec::Type::QPACK);
  // encode
  server.setStats(&stats);
  auto encResult = server.encode(resp, 1);
  EXPECT_EQ(stats.encodes, 1);
  EXPECT_EQ(stats.decodes, 0);
  EXPECT_EQ(stats.errors, 0);
  EXPECT_GT(stats.encodedBytesCompr, 0);
  EXPECT_GT(stats.encodedBytesComprBlock, 0);
  EXPECT_GT(stats.encodedBytesUncompr, 0);
  EXPECT_EQ(stats.decodedBytesCompr, 0);
  EXPECT_EQ(stats.decodedBytesUncompr, 0);
  server.setStats(nullptr);

  // decode
  stats.reset();
  client.setStats(&stats);
  TestStreamingCallback cb;
  auto len = encResult.stream->computeChainDataLength();
  client.decodeEncoderStream(std::move(encResult.control));
  client.decodeStreaming(1, std::move(encResult.stream), len, &cb);
  auto result = cb.getResult();
  EXPECT_TRUE(!result.hasError());
  auto& decoded = result->headers;
  CHECK_EQ(decoded.size(), 3 * 2);
  EXPECT_EQ(stats.decodes, 1);
  EXPECT_EQ(stats.encodes, 0);
  EXPECT_GT(stats.decodedBytesCompr, 0);
  EXPECT_GT(stats.decodedBytesUncompr, 0);
  EXPECT_EQ(stats.encodedBytesCompr, 0);
  EXPECT_EQ(stats.encodedBytesComprBlock, 0);
  EXPECT_EQ(stats.encodedBytesUncompr, 0);
  client.setStats(nullptr);
}
