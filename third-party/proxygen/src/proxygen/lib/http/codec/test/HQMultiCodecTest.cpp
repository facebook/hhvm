/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <folly/portability/GMock.h>
#include <folly/portability/GTest.h>
#include <proxygen/lib/http/codec/HQMultiCodec.h>
#include <proxygen/lib/http/codec/test/TestUtils.h>

using namespace proxygen;
using namespace proxygen::hq;
using namespace testing;

class HQMultiCodecTest : public Test {
 public:
  void SetUp() override {
    codec_.setControlStreamID(kSessionStreamId);
  }

 protected:
  HQMultiCodec codec_{TransportDirection::DOWNSTREAM};
  folly::IOBufQueue writeBuf_{folly::IOBufQueue::cacheChainLength()};
};

TEST_F(HQMultiCodecTest, Egress) {
  codec_.getQPACKCodec().setEncoderHeaderTableSize(4096);
  codec_.setQPACKEncoderMaxDataFn([] { return 100; });
  EXPECT_EQ(codec_.generateConnectionPreface(writeBuf_), 0);
  EXPECT_GT(codec_.generateSettings(writeBuf_), 0);
  EXPECT_EQ(codec_.generateSettingsAck(writeBuf_), 0);

  HTTPCodec::StreamID id{1};
  codec_.addCodec(id);
  HTTPHeaderSize size;
  codec_.generateHeader(writeBuf_, id, getResponse(200), false, &size);
  EXPECT_GT(size.uncompressed, 0);
  EXPECT_GT(codec_.getQPACKEncoderWriteBuf().chainLength(), 0);
  EXPECT_GT(
      codec_.generateBody(writeBuf_, id, makeBuf(100), folly::none, false), 0);
  codec_.generatePushPromise(
      writeBuf_, id, getGetRequest(), codec_.nextPushID(), false, &size);
  EXPECT_GT(size.uncompressed, 0);
  HTTPHeaders trailers;
  trailers.add("x-trailer1", "trailer1");
  EXPECT_GT(codec_.generateTrailers(writeBuf_, id, trailers), 0);
  EXPECT_EQ(codec_.generateEOM(writeBuf_, id), 0);
  codec_.removeCodec(id);
  EXPECT_GT(codec_.generateGoaway(
                writeBuf_, HTTPCodec::MaxStreamID, ErrorCode::NO_ERROR),
            0);
}

TEST_F(HQMultiCodecTest, Ingress) {
  FakeHTTPCodecCallback callbacks;
  callbacks.setSessionStreamId(kSessionStreamId);
  HQMultiCodec upstreamCodec{TransportDirection::UPSTREAM};
  HTTPCodec::StreamID id{0};
  upstreamCodec.addCodec(id);
  upstreamCodec.generateHeader(writeBuf_, id, getPostRequest(100));
  upstreamCodec.generateBody(writeBuf_, id, makeBuf(100), folly::none, false);
  upstreamCodec.generateEOM(writeBuf_, id);

  // Test setting callback after codec creation
  codec_.addCodec(id);
  codec_.setCallback(&callbacks);

  HTTPCodec::StreamID id2{4};
  EXPECT_FALSE(codec_.setCurrentStream(id2));
  codec_.addCodec(id2);
  EXPECT_TRUE(codec_.setCurrentStream(id2));
  EXPECT_EQ(codec_.onIngress(*writeBuf_.front()), writeBuf_.chainLength());
  EXPECT_EQ(callbacks.messageBegin, 1);
  EXPECT_EQ(callbacks.headersComplete, 1);
  EXPECT_EQ(callbacks.headersCompleteId, id2);
  EXPECT_EQ(callbacks.bodyCalls, 1);
  EXPECT_EQ(callbacks.messageComplete, 0);
  EXPECT_EQ(callbacks.streamErrors, 0);
  EXPECT_EQ(callbacks.sessionErrors, 0);
  EXPECT_GT(codec_.getCompressionInfo().ingress.staticRefs_, 0);

  EXPECT_TRUE(codec_.setCurrentStream(id));
  writeBuf_.reset();
  std::array<uint8_t, 2> frameMissingPayload{0x01, 0x10};
  writeBuf_.append(folly::IOBuf::wrapBuffer(frameMissingPayload.data(),
                                            frameMissingPayload.size()));
  EXPECT_EQ(codec_.onIngress(*writeBuf_.front()), writeBuf_.chainLength());
  EXPECT_TRUE(codec_.setCurrentStream(id));
  codec_.onIngressEOF();
  EXPECT_EQ(callbacks.streamErrors, 0);
  EXPECT_EQ(callbacks.sessionErrors, 1);
  EXPECT_EQ(callbacks.lastParseError->getHttp3ErrorCode(),
            HTTP3::ErrorCode::HTTP_FRAME_ERROR);

  EXPECT_TRUE(codec_.setCurrentStream(id2));
  codec_.onIngressEOF();
  EXPECT_EQ(callbacks.messageComplete, 1);
}

TEST_F(HQMultiCodecTest, Generic) {
  EXPECT_TRUE(codec_.supportsParallelRequests());
  EXPECT_NE(codec_.getEgressSettings(), nullptr);
  EXPECT_EQ(codec_.getDefaultWindowSize(),
            std::numeric_limits<uint32_t>::max());
  EXPECT_TRUE(codec_.isWaitingToDrain());
  codec_.enableDoubleGoawayDrain();
  EXPECT_FALSE(codec_.isWaitingToDrain());
  EXPECT_TRUE(codec_.isReusable());
  EXPECT_EQ(codec_.getUserAgent(), std::string());
  codec_.onIngressPushId(0);
}
