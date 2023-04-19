/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <folly/portability/GMock.h>
#include <folly/portability/GTest.h>
#include <proxygen/lib/http/codec/HQControlCodec.h>
#include <proxygen/lib/http/codec/HQStreamCodec.h>
#include <proxygen/lib/http/codec/QPACKDecoderCodec.h>
#include <proxygen/lib/http/codec/QPACKEncoderCodec.h>
#include <proxygen/lib/http/codec/TransportDirection.h>
#include <proxygen/lib/http/codec/compress/test/TestStreamingCallback.h>
#include <proxygen/lib/http/codec/compress/test/TestUtil.h>
#include <proxygen/lib/http/codec/test/HQFramerTest.h>
#include <proxygen/lib/http/codec/test/TestUtils.h>

using namespace folly;
using namespace proxygen;
using namespace proxygen::hq;
using namespace testing;

// a FakeHTTPCodecCallback that can
// be used as a unidirectional codec as well
class FakeHQHTTPCodecCallback
    : public FakeHTTPCodecCallback
    , public HQUnidirectionalCodec::Callback {
 public:
  // Delegate HQUnidirectionalCodec::Callback::onError
  // to FakeHTTPCodecCallback::onError
  void onError(HTTPCodec::StreamID streamId,
               const HTTPException& ex,
               bool newTxn) override {
    FakeHTTPCodecCallback::onError(streamId, ex, newTxn);
  }
};

enum class CodecType {
  UPSTREAM,
  DOWNSTREAM,
  CONTROL_UPSTREAM,
  CONTROL_DOWNSTREAM,
  PUSH,
};

// This is a template since for regular tests the fixture has to be derived from
// testing::Test and for parameterized tests it has to be derived from
// testing::TestWithParam<>
template <class T>
class HQCodecTestFixture : public T {
 public:
  void SetUp() override {
    makeCodecs();
    SetUpCodecs();
  }

  void SetUpCodecs() {
    callbacks_.setSessionStreamId(kSessionStreamId);
    downstreamCodec_->setCallback(&callbacks_);
    upstreamCodec_->setCallback(&callbacks_);
    upstreamControlCodec_.setCallback(&callbacks_);
    downstreamControlCodec_.setCallback(&callbacks_);
  }

  void parse() {
    auto consumed = downstreamCodec_->onIngress(*queue_.front());
    queue_.trimStart(consumed);
  }

  void parseUpstream() {
    auto consumed = upstreamCodec_->onIngress(*queue_.front());
    queue_.trimStart(consumed);
  }

  void parseControl(CodecType type) {
    HQControlCodec* codec = nullptr;
    downstreamControlCodec_.setCallback(&callbacks_);

    switch (type) {
      case CodecType::CONTROL_UPSTREAM:
        codec = &upstreamControlCodec_;
        break;
      case CodecType::CONTROL_DOWNSTREAM:
        codec = &downstreamControlCodec_;
        break;
      default:
        LOG(FATAL) << "Unknown Control Codec type";
        break;
    }
    auto ret = codec->onUnidirectionalIngress(queueCtrl_.move());
    queueCtrl_.append(std::move(ret));
  }

  void qpackTest(bool blocked);

  size_t addAndCheckSimpleHeaders() {
    std::array<uint8_t, 6> simpleReq{0x00, 0x00, 0xC0, 0xC1, 0xD1, 0xD7};
    writeFrameHeaderManual(
        queue_, static_cast<uint64_t>(FrameType::HEADERS), simpleReq.size());
    queue_.append(simpleReq.data(), simpleReq.size());
    size_t n = queue_.chainLength();
    parse();
    EXPECT_EQ(callbacks_.headerFrames, 1);
    EXPECT_EQ(callbacks_.headersComplete, 1);
    EXPECT_EQ(callbacks_.bodyCalls, 0);
    EXPECT_EQ(callbacks_.bodyLength, 0);
    return n;
  }

  void makeCodecs() {
    upstreamCodec_ = std::make_unique<HQStreamCodec>(
        streamId_,
        TransportDirection::UPSTREAM,
        qpackUpstream_,
        qpackUpEncoderWriteBuf_,
        qpackUpDecoderWriteBuf_,
        [] { return std::numeric_limits<uint64_t>::max(); },
        ingressSettings_);
    downstreamCodec_ = std::make_unique<HQStreamCodec>(
        streamId_,
        TransportDirection::DOWNSTREAM,
        qpackDownstream_,
        qpackDownEncoderWriteBuf_,
        qpackDownDecoderWriteBuf_,
        [] { return std::numeric_limits<uint64_t>::max(); },
        ingressSettings_);
  }

  void testGoaway(HQControlCodec& codec, uint64_t drainId);

 protected:
  FakeHQHTTPCodecCallback callbacks_;
  HTTPSettings egressSettings_;
  HTTPSettings ingressSettings_;
  HQControlCodec upstreamControlCodec_{0x1111,
                                       TransportDirection::UPSTREAM,
                                       StreamDirection::INGRESS,
                                       ingressSettings_,
                                       hq::UnidirectionalStreamType::CONTROL};
  HQControlCodec downstreamControlCodec_{0x2222,
                                         TransportDirection::DOWNSTREAM,
                                         StreamDirection::INGRESS,
                                         ingressSettings_,
                                         hq::UnidirectionalStreamType::CONTROL};
  QPACKCodec qpackUpstream_;
  QPACKCodec qpackDownstream_;
  QPACKEncoderCodec qpackEncoderCodec_{qpackDownstream_, callbacks_};
  QPACKDecoderCodec qpackDecoderCodec_{qpackUpstream_, callbacks_};
  HTTPCodec::StreamID streamId_{0x1234};
  IOBufQueue qpackUpEncoderWriteBuf_{IOBufQueue::cacheChainLength()};
  IOBufQueue qpackUpDecoderWriteBuf_{IOBufQueue::cacheChainLength()};
  IOBufQueue qpackDownEncoderWriteBuf_{IOBufQueue::cacheChainLength()};
  IOBufQueue qpackDownDecoderWriteBuf_{IOBufQueue::cacheChainLength()};
  std::unique_ptr<HQStreamCodec> upstreamCodec_{nullptr};
  std::unique_ptr<HQStreamCodec> downstreamCodec_{nullptr};
  IOBufQueue queue_{IOBufQueue::cacheChainLength()};
  IOBufQueue queueCtrl_{IOBufQueue::cacheChainLength()};
};

class HQCodecTest : public HQCodecTestFixture<Test> {};

TEST_F(HQCodecTest, DataFrame) {
  auto data = makeBuf(500);
  writeFrameHeaderManual(
      queue_, static_cast<uint64_t>(FrameType::DATA), data->length());
  queue_.append(data->clone());
  parse();
  EXPECT_EQ(callbacks_.headerFrames, 1);
  EXPECT_EQ(callbacks_.bodyCalls, 1);
  EXPECT_EQ(callbacks_.bodyLength, data->length());
}

TEST_F(HQCodecTest, PriorityUpdate) {
  // SETTINGS is a must have
  writeValidFrame(queueCtrl_, FrameType::SETTINGS);
  EXPECT_GT(upstreamControlCodec_.generatePriority(
                queueCtrl_, 123, HTTPPriority(5, true)),
            0);
  parseControl(CodecType::CONTROL_DOWNSTREAM);
  EXPECT_EQ(5, callbacks_.urgency);
  EXPECT_TRUE(callbacks_.incremental);
}

TEST_F(HQCodecTest, DataFrameStreaming) {
  auto data1 = makeBuf(500);
  auto data2 = makeBuf(500);
  writeFrameHeaderManual(queue_,
                         static_cast<uint64_t>(FrameType::DATA),
                         data1->length() + data2->length());
  queue_.append(data1->clone());
  parse();
  queue_.append(data2->clone());
  parse();
  EXPECT_EQ(callbacks_.headerFrames, 1);
  EXPECT_EQ(callbacks_.bodyCalls, 2);
  EXPECT_EQ(callbacks_.bodyLength, data1->length() + data2->length());
  auto data3 = makeBuf(100);
  writeFrameHeaderManual(
      queue_, static_cast<uint64_t>(FrameType::DATA), data3->length());
  queue_.append(data3->clone());
  parse();
  EXPECT_EQ(callbacks_.headerFrames, 2);
  EXPECT_EQ(callbacks_.bodyCalls, 3);
  EXPECT_EQ(callbacks_.bodyLength,
            data1->length() + data2->length() + data3->length());
}

TEST_F(HQCodecTest, PushPromiseFrame) {
  hq::PushId pushId = 1234;

  HTTPMessage msg = getGetRequest();
  msg.getHeaders().add(HTTP_HEADER_USER_AGENT, "optimus-prime");

  // push promises can only be sent by the server.
  downstreamCodec_->generatePushPromise(queue_, streamId_, msg, pushId, false);

  // push promises should be parsed by the client
  parseUpstream();

  EXPECT_EQ(callbacks_.headerFrames, 1);
  EXPECT_EQ(callbacks_.bodyCalls, 0);
  EXPECT_EQ(callbacks_.messageBegin, 1);
  EXPECT_EQ(callbacks_.pushId, pushId);
  EXPECT_EQ(callbacks_.assocStreamId, streamId_);
}

TEST_F(HQCodecTest, HeadersOverSize) {
  // Server sends limited MAX_HEADER_LIST_SIZE, client will send anyways,
  // server errors
  HTTPSettings egressSettings{{SettingsId::MAX_HEADER_LIST_SIZE, 37}};
  HQControlCodec downstreamControlEgressCodec{
      0x2223,
      TransportDirection::DOWNSTREAM,
      StreamDirection::EGRESS,
      egressSettings,
      hq::UnidirectionalStreamType::CONTROL};
  downstreamControlEgressCodec.generateSettings(queueCtrl_);
  parseControl(CodecType::CONTROL_UPSTREAM);
  // set so the stream codec sees it
  // TODO: the downstream codec doesn't have access to the egress settings,
  // it relies on QPACK to check this
  qpackDownstream_.setMaxUncompressed(37);

  HTTPMessage msg = getGetRequest();
  msg.getHeaders().add(HTTP_HEADER_USER_AGENT, "optimus-prime");
  upstreamCodec_->generateHeader(queue_, streamId_, msg, false, nullptr);

  HTTPHeaders trailers;
  trailers.add("x-trailer-1", "pico-de-gallo");
  // These trailers are generated, but never parsed because it pauses after
  // the headers error
  upstreamCodec_->generateTrailers(queue_, streamId_, trailers);
  parse();
  downstreamCodec_->onIngressEOF();

  EXPECT_EQ(callbacks_.messageBegin, 1);
  EXPECT_EQ(callbacks_.headersComplete, 0);
  EXPECT_EQ(callbacks_.messageComplete, 0);
  EXPECT_EQ(callbacks_.streamErrors, 1);
  EXPECT_EQ(callbacks_.sessionErrors, 0);
  EXPECT_EQ(callbacks_.lastParseError->getHttp3ErrorCode(),
            HTTP3::ErrorCode::HTTP_QPACK_DECOMPRESSION_FAILED);
}

TEST_F(HQCodecTest, Trailers) {
  for (auto body = 0; body <= 1; body++) {
    HTTPMessage msg = getPostRequest(body * 5);
    msg.getHeaders().add(HTTP_HEADER_USER_AGENT, "optimus-prime");
    upstreamCodec_->generateHeader(queue_, streamId_, msg, false, nullptr);

    if (body) {
      std::string data("abcde");
      auto buf = folly::IOBuf::copyBuffer(data.data(), data.length());
      upstreamCodec_->generateBody(queue_,
                                   streamId_,
                                   std::move(buf),
                                   HTTPCodec::NoPadding,
                                   /*eom=*/false);
    }
    HTTPHeaders trailers;
    trailers.add("x-trailer-1", "pico-de-gallo");
    // stripped on generate
    trailers.add(HTTP_HEADER_CONNECTION, "keep-alive");
    upstreamCodec_->generateTrailers(queue_, streamId_, trailers);
    upstreamCodec_->generateEOM(queue_, streamId_);
    parse();
    downstreamCodec_->onIngressEOF();

    EXPECT_EQ(callbacks_.messageBegin, 1);
    EXPECT_EQ(callbacks_.headersComplete, 1);
    EXPECT_EQ(callbacks_.bodyCalls, body);
    EXPECT_EQ(callbacks_.bodyLength, body * 5);
    EXPECT_EQ(callbacks_.trailers, 1);
    EXPECT_NE(nullptr, callbacks_.msg->getTrailers());
    EXPECT_EQ("pico-de-gallo",
              callbacks_.msg->getTrailers()->getSingleOrEmpty("x-trailer-1"));
    EXPECT_EQ(callbacks_.msg->getTrailers()->size(), 1);
    EXPECT_EQ(callbacks_.messageComplete, 1);
    EXPECT_EQ(callbacks_.streamErrors, 0);
    EXPECT_EQ(callbacks_.sessionErrors, 0);
    callbacks_.reset();
    makeCodecs();
    downstreamCodec_->setCallback(&callbacks_);
  }
}

TEST_F(HQCodecTest, GenerateExtraHeaders) {
  HTTPMessage resp = getResponse(200, 2000);
  HTTPHeaders extraHeaders;
  extraHeaders.add(HTTP_HEADER_PRIORITY, "u=2");
  downstreamCodec_->generateHeader(queue_,
                                   streamId_,
                                   resp,
                                   true /* eom */,
                                   nullptr /* HTTPHeaderSize */,
                                   std::move(extraHeaders));

  parseUpstream();

  const auto& headers = callbacks_.msg->getHeaders();
  EXPECT_EQ("2000", headers.getSingleOrEmpty(HTTP_HEADER_CONTENT_LENGTH));
  EXPECT_EQ("u=2", headers.getSingleOrEmpty(HTTP_HEADER_PRIORITY));
}

template <class T>
void HQCodecTestFixture<T>::qpackTest(bool blocked) {
  QPACKCodec& server = qpackDownstream_;
  server.setDecoderHeaderTableMaxSize(1024);
  qpackUpstream_.setEncoderHeaderTableSize(1024);
  auto streamId = upstreamCodec_->createStream();
  HTTPMessage msg = getGetRequest();
  msg.getHeaders().add(HTTP_HEADER_USER_AGENT, "optimus-prime");
  upstreamCodec_->generateHeader(queue_, streamId, msg, false, nullptr);
  EXPECT_FALSE(qpackUpEncoderWriteBuf_.empty());
  if (!blocked) {
    qpackEncoderCodec_.onUnidirectionalIngress(qpackUpEncoderWriteBuf_.move());
    EXPECT_EQ(callbacks_.sessionErrors, 0);
  }
  TestStreamingCallback cb;
  CHECK(!queue_.empty());
  upstreamCodec_->generateBody(
      queue_, streamId, folly::IOBuf::copyBuffer("ohai"), 0, false);
  upstreamCodec_->generateEOM(queue_, streamId);
  auto consumed = downstreamCodec_->onIngress(*queue_.front());
  EXPECT_TRUE(blocked || consumed == queue_.chainLength());
  queue_.trimStart(consumed);
  EXPECT_EQ(callbacks_.streamErrors, 0);
  EXPECT_EQ(callbacks_.sessionErrors, 0);
  if (blocked) {
    EXPECT_EQ(callbacks_.headersComplete, 0);
    qpackEncoderCodec_.onUnidirectionalIngress(qpackUpEncoderWriteBuf_.move());
    EXPECT_EQ(callbacks_.sessionErrors, 0);
  }
  EXPECT_EQ(callbacks_.headersComplete, 1);
  CHECK(callbacks_.msg);
  if (blocked) {
    EXPECT_FALSE(queue_.empty());
    EXPECT_EQ(callbacks_.bodyCalls, 0);
    EXPECT_EQ(callbacks_.bodyLength, 0);
    consumed = downstreamCodec_->onIngress(*queue_.front());
    queue_.trimStart(consumed);
    EXPECT_TRUE(queue_.empty());
  }
  EXPECT_EQ(callbacks_.bodyCalls, 1);
  EXPECT_EQ(callbacks_.bodyLength, 4);
  downstreamCodec_->onIngressEOF();
  EXPECT_EQ(callbacks_.messageComplete, 1);
  auto ack = server.encodeHeaderAck(streamId);
  if (ack) {
    qpackDecoderCodec_.onUnidirectionalIngress(std::move(ack));
  }
  auto ici = server.encodeInsertCountInc();
  if (ici) {
    qpackDecoderCodec_.onUnidirectionalIngress(std::move(ici));
  }
  EXPECT_EQ(callbacks_.sessionErrors, 0);
}

TEST_F(HQCodecTest, qpack) {
  qpackTest(false);
}

TEST_F(HQCodecTest, qpackBlocked) {
  qpackTest(true);
}

TEST_F(HQCodecTest, qpackError) {
  qpackEncoderCodec_.onUnidirectionalIngress(folly::IOBuf::wrapBuffer("", 1));
  EXPECT_EQ(callbacks_.lastParseError->getHttp3ErrorCode(),
            HTTP3::ErrorCode::HTTP_QPACK_ENCODER_STREAM_ERROR);
  EXPECT_EQ(callbacks_.sessionErrors, 1);
  qpackDecoderCodec_.onUnidirectionalIngressEOF();
  EXPECT_EQ(callbacks_.lastParseError->getHttp3ErrorCode(),
            HTTP3::ErrorCode::HTTP_CLOSED_CRITICAL_STREAM);
  EXPECT_EQ(callbacks_.sessionErrors, 2);
  qpackEncoderCodec_.onUnidirectionalIngressEOF();
  EXPECT_EQ(callbacks_.lastParseError->getHttp3ErrorCode(),
            HTTP3::ErrorCode::HTTP_CLOSED_CRITICAL_STREAM);
  EXPECT_EQ(callbacks_.sessionErrors, 3);

  // duplicate method in headers
  std::vector<compress::Header> headers;
  std::string meth("GET");
  headers.emplace_back(HTTP_HEADER_COLON_METHOD, meth);
  headers.emplace_back(HTTP_HEADER_COLON_METHOD, meth);
  QPACKCodec& client = qpackUpstream_;
  auto result = client.encode(headers, 1);
  hq::writeHeaders(queue_, std::move(result.stream));
  downstreamCodec_->onIngress(*queue_.front());
  EXPECT_EQ(callbacks_.lastParseError->getHttpStatusCode(), 400);
  EXPECT_EQ(callbacks_.streamErrors, 1);
}

TEST_F(HQCodecTest, qpackErrorShort) {
  uint8_t bad[] = {0x00}; // LR, no delta base
  hq::writeHeaders(queue_, folly::IOBuf::wrapBuffer(bad, 1));
  downstreamCodec_->onIngress(*queue_.front());
  EXPECT_EQ(callbacks_.lastParseError->getHttp3ErrorCode(),
            HTTP3::ErrorCode::HTTP_QPACK_DECOMPRESSION_FAILED);
  EXPECT_EQ(callbacks_.sessionErrors, 1);
}

TEST_F(HQCodecTest, extraHeaders) {
  std::array<uint8_t, 6> simpleReq{0x00, 0x00, 0xC0, 0xC1, 0xD1, 0xD7};
  std::array<uint8_t, 3> simpleResp{0x00, 0x00, 0xD9};
  writeFrameHeaderManual(
      queue_, static_cast<uint64_t>(FrameType::HEADERS), simpleReq.size());
  queue_.append(simpleReq.data(), simpleReq.size());
  downstreamCodec_->onIngress(*queue_.front());
  EXPECT_EQ(callbacks_.streamErrors, 0);
  downstreamCodec_->onIngress(*queue_.front());
  EXPECT_EQ(callbacks_.streamErrors, 1);
  // The headers fail to parse because the codec thinks they are trailers
  EXPECT_EQ(callbacks_.lastParseError->getHttp3ErrorCode(),
            HTTP3::ErrorCode::HTTP_MESSAGE_ERROR);
  queue_.move();
  callbacks_.reset();
  writeFrameHeaderManual(
      queue_, static_cast<uint64_t>(FrameType::HEADERS), simpleResp.size());
  queue_.append(simpleResp.data(), simpleResp.size());
  upstreamCodec_->onIngress(*queue_.front());
  EXPECT_EQ(callbacks_.streamErrors, 0);
  upstreamCodec_->onIngress(*queue_.front());
  EXPECT_EQ(callbacks_.streamErrors, 1);
  // The headers fail to parse because the codec thinks they are trailers
  EXPECT_EQ(callbacks_.lastParseError->getHttp3ErrorCode(),
            HTTP3::ErrorCode::HTTP_MESSAGE_ERROR);
}

/* A second HEADERS frame is unexpected, and stops the parser */
TEST_F(HQCodecTest, MultipleHeaders) {
  writeValidFrame(queue_, FrameType::HEADERS);

  // Write empty trailers, no more HEADERS frames allowed
  std::array<uint8_t, 2> emptyHeaderBlock{0x00, 0x00};
  hq::writeHeaders(queue_,
                   folly::IOBuf::copyBuffer(emptyHeaderBlock.data(),
                                            emptyHeaderBlock.size()));

  // Write invalid QPACK header.  It is never parsed because the frame is
  // unexpected
  std::array<uint8_t, 2> qpackError{0xC0, 0x00};
  hq::writeHeaders(
      queue_, folly::IOBuf::copyBuffer(qpackError.data(), qpackError.size()));
  parse();
  // never seen, parser is paused
  downstreamCodec_->onIngressEOF();
  EXPECT_EQ(callbacks_.messageBegin, 1);
  EXPECT_EQ(callbacks_.headersComplete, 1);
  EXPECT_EQ(callbacks_.messageComplete, 0);
  EXPECT_EQ(callbacks_.streamErrors, 1);
  EXPECT_EQ(callbacks_.sessionErrors, 0);
  EXPECT_EQ(callbacks_.lastParseError->getHttp3ErrorCode(),
            HTTP3::ErrorCode::HTTP_FRAME_UNEXPECTED);
}

/* An invalid HEADERS frame stops the parser */
TEST_F(HQCodecTest, InvalidHeaders) {
  // Valid but incomplete QPACK
  std::array<uint8_t, 5> headersMissingField = {0x00, 0x00, 0xC0, 0xC1, 0xD1};
  hq::writeHeaders(queue_,
                   folly::IOBuf::wrapBuffer(headersMissingField.data(),
                                            headersMissingField.size()));
  parse();
  // never seen, parser is paused
  downstreamCodec_->onIngressEOF();
  EXPECT_EQ(callbacks_.messageBegin, 1);
  EXPECT_EQ(callbacks_.headersComplete, 0);
  EXPECT_EQ(callbacks_.messageComplete, 0);
  EXPECT_EQ(callbacks_.streamErrors, 1);
  EXPECT_EQ(callbacks_.sessionErrors, 0);
  EXPECT_EQ(callbacks_.lastParseError->getHttpStatusCode(), 400);
}

TEST_F(HQCodecTest, ParserStopsAfterPushPromiseError) {
  std::array<uint8_t, 3> qpackError{0xC0, 0x00};
  hq::writePushPromise(
      queue_,
      0,
      folly::IOBuf::copyBuffer(qpackError.data(), qpackError.size()));

  // push promises should be parsed by the client
  parseUpstream();
  // Never seen, error
  upstreamCodec_->onIngressEOF();

  EXPECT_EQ(callbacks_.messageBegin, 1);
  EXPECT_EQ(callbacks_.messageComplete, 0);
  EXPECT_EQ(callbacks_.streamErrors, 0);
  EXPECT_EQ(callbacks_.sessionErrors, 1);
  EXPECT_EQ(callbacks_.lastParseError->getHttp3ErrorCode(),
            HTTP3::ErrorCode::HTTP_QPACK_DECOMPRESSION_FAILED);
}

TEST_F(HQCodecTest, ZeroLengthData) {
  // 0, 1, 0, 1, 0
  for (auto i = 0; i < 5; i++) {
    auto data = folly::IOBuf::create(1);
    if (i % 2 == 1) {
      data->writableData()[0] = 'a';
      data->append(1);
    }
    hq::writeData(queue_, std::move(data));
    parse();
    EXPECT_EQ(callbacks_.messageBegin, 0);
    EXPECT_EQ(callbacks_.headerFrames, i + 1);
    EXPECT_EQ(callbacks_.headersComplete, 0);
    EXPECT_EQ(callbacks_.bodyCalls, (i + 1) / 2);
    EXPECT_EQ(callbacks_.messageComplete, 0);
    EXPECT_EQ(callbacks_.streamErrors, 0);
    EXPECT_EQ(callbacks_.sessionErrors, 0);
  }

  // EOF after 0 length frame is fine
  downstreamCodec_->onIngressEOF();
  EXPECT_EQ(callbacks_.messageComplete, 1);
  EXPECT_EQ(callbacks_.streamErrors, 0);
  EXPECT_EQ(callbacks_.sessionErrors, 0);
}

TEST_F(HQCodecTest, ZeroLengthSettings) {
  std::deque<hq::SettingPair> emptySettings;
  hq::writeSettings(queueCtrl_, emptySettings);
  parseControl(CodecType::CONTROL_UPSTREAM);
  EXPECT_EQ(callbacks_.settings, 1);
  // EOF is not mid-frame
  upstreamControlCodec_.onIngressEOF();
  EXPECT_EQ(callbacks_.streamErrors, 0);
  EXPECT_EQ(callbacks_.sessionErrors, 0);
}

TEST_F(HQCodecTest, InvalidSettings) {
  std::deque<hq::SettingPair> settings{
      {hq::SettingId::ENABLE_WEBTRANSPORT, 37}};
  hq::writeSettings(queueCtrl_, settings);
  parseControl(CodecType::CONTROL_UPSTREAM);
  EXPECT_EQ(callbacks_.settings, 0);
  EXPECT_EQ(callbacks_.streamErrors, 0);
  EXPECT_EQ(callbacks_.sessionErrors, 1);
  EXPECT_EQ(callbacks_.lastParseError->getHttp3ErrorCode(),
            HTTP3::ErrorCode::HTTP_SETTINGS_ERROR);
}

TEST_F(HQCodecTest, ZeroLengthTrailers) {
  writeValidFrame(queue_, FrameType::HEADERS);

  // Write empty trailers, invalid, QPACK minimum is 2 bytes
  hq::writeHeaders(queue_, folly::IOBuf::create(1));

  parse();
  // Never seen, parser paused on error
  downstreamCodec_->onIngressEOF();
  EXPECT_EQ(callbacks_.messageBegin, 1);
  EXPECT_EQ(callbacks_.headersComplete, 1);
  EXPECT_EQ(callbacks_.messageComplete, 0);
  EXPECT_EQ(callbacks_.streamErrors, 0);
  EXPECT_EQ(callbacks_.sessionErrors, 1);
}

TEST_F(HQCodecTest, TruncatedStream) {
  writeFrameHeaderManual(
      queue_, static_cast<uint64_t>(FrameType::HEADERS), 0x10);
  parse();
  downstreamCodec_->onIngressEOF();
  EXPECT_EQ(callbacks_.messageComplete, 0);
  EXPECT_EQ(callbacks_.streamErrors, 0);
  EXPECT_EQ(callbacks_.sessionErrors, 1);
  EXPECT_EQ(callbacks_.lastParseError->getHttp3ErrorCode(),
            HTTP3::ErrorCode::HTTP_FRAME_ERROR);

  // A second EOF goes nowhere, because the codec is in error
  downstreamCodec_->onIngressEOF();
  EXPECT_EQ(callbacks_.messageComplete, 0);
  EXPECT_EQ(callbacks_.streamErrors, 0);
  EXPECT_EQ(callbacks_.sessionErrors, 1);
  EXPECT_EQ(callbacks_.lastParseError->getHttp3ErrorCode(),
            HTTP3::ErrorCode::HTTP_FRAME_ERROR);
}

TEST_F(HQCodecTest, BasicConnect) {
  std::string authority = "myhost:1234";
  HTTPMessage request;
  request.setMethod(HTTPMethod::CONNECT);
  request.getHeaders().add(proxygen::HTTP_HEADER_HOST, authority);
  auto streamId = upstreamCodec_->createStream();
  upstreamCodec_->generateHeader(queue_, streamId, request, false /* eom */);

  parse();
  callbacks_.expectMessage(false, 1, "");
  EXPECT_EQ(HTTPMethod::CONNECT, callbacks_.msg->getMethod());
  const auto& headers = callbacks_.msg->getHeaders();
  EXPECT_EQ(authority, headers.getSingleOrEmpty(proxygen::HTTP_HEADER_HOST));
}

TEST_F(HQCodecTest, TrimLwsInHeaderValue) {
  HTTPMessage req = getGetRequest("/test");
  req.getHeaders().add(HTTPHeaderCode::HTTP_HEADER_CONTENT_TYPE,
                       " application/x-fb");
  // adding header with lws will strip here
  req.getHeaders().add(HTTPHeaderCode::HTTP_HEADER_USER_AGENT, "\n\t \r");
  auto streamId = upstreamCodec_->createStream();
  upstreamCodec_->generateHeader(queue_, streamId, req, true /* eom */);

  parse();
  EXPECT_EQ(callbacks_.messageBegin, 1);
  EXPECT_EQ(callbacks_.headersComplete, 1);
  EXPECT_EQ(callbacks_.messageComplete, 0);
  EXPECT_EQ(callbacks_.streamErrors, 0);
  EXPECT_EQ(callbacks_.sessionErrors, 0);

  auto& parsedHeaders = callbacks_.msg->getHeaders();
  EXPECT_TRUE(parsedHeaders.exists(HTTPHeaderCode::HTTP_HEADER_CONTENT_TYPE));
  EXPECT_TRUE(parsedHeaders.exists(HTTPHeaderCode::HTTP_HEADER_USER_AGENT));
  EXPECT_TRUE(
      parsedHeaders.getSingleOrEmpty(HTTPHeaderCode::HTTP_HEADER_USER_AGENT)
          .empty());
  EXPECT_EQ(
      parsedHeaders.getSingleOrEmpty(HTTPHeaderCode::HTTP_HEADER_CONTENT_TYPE),
      "application/x-fb");
}

TEST_F(HQCodecTest, OnlyDataAfterConnect) {
  std::string authority = "myhost:1234";
  HTTPMessage request;
  request.setMethod(HTTPMethod::CONNECT);
  request.getHeaders().add(proxygen::HTTP_HEADER_HOST, authority);
  auto streamId = upstreamCodec_->createStream();
  upstreamCodec_->generateHeader(queue_, streamId, request, false /* eom */);

  parse();
  callbacks_.expectMessage(false, 1, "");
  EXPECT_EQ(HTTPMethod::CONNECT, callbacks_.msg->getMethod());
  const auto& headers = callbacks_.msg->getHeaders();
  EXPECT_EQ(authority, headers.getSingleOrEmpty(proxygen::HTTP_HEADER_HOST));

  writeValidFrame(queue_, FrameType::HEADERS);
  parse();
  EXPECT_EQ(callbacks_.lastParseError->getHttp3ErrorCode(),
            HTTP3::ErrorCode::HTTP_FRAME_UNEXPECTED);
}

TEST_F(HQCodecTest, MultipleSettingsUpstream) {
  writeValidFrame(queueCtrl_, FrameType::SETTINGS);
  writeValidFrame(queueCtrl_, FrameType::SETTINGS);
  parseControl(CodecType::CONTROL_UPSTREAM);
  EXPECT_EQ(callbacks_.headerFrames, 1);
  EXPECT_EQ(callbacks_.streamErrors, 0);
  EXPECT_EQ(callbacks_.sessionErrors, 1);
  EXPECT_EQ(callbacks_.lastParseError->getHttp3ErrorCode(),
            HTTP3::ErrorCode::HTTP_FRAME_UNEXPECTED);
}

TEST_F(HQCodecTest, MultipleSettingsDownstream) {
  writeValidFrame(queueCtrl_, FrameType::SETTINGS);
  writeValidFrame(queueCtrl_, FrameType::SETTINGS);
  parseControl(CodecType::CONTROL_DOWNSTREAM);
  EXPECT_EQ(callbacks_.headerFrames, 1);
  EXPECT_EQ(callbacks_.streamErrors, 0);
  EXPECT_EQ(callbacks_.sessionErrors, 1);
  EXPECT_EQ(callbacks_.lastParseError->getHttp3ErrorCode(),
            HTTP3::ErrorCode::HTTP_FRAME_UNEXPECTED);
}

TEST_F(HQCodecTest, RfcPriorityCallback) {
  // SETTINGS is a must have
  writeValidFrame(queueCtrl_, FrameType::SETTINGS);
  writeValidFrame(queueCtrl_, FrameType::PRIORITY_UPDATE);
  parseControl(CodecType::CONTROL_DOWNSTREAM);
  EXPECT_EQ(1, callbacks_.urgency);
  EXPECT_TRUE(callbacks_.incremental);
}

TEST_F(HQCodecTest, RfcPushPriorityCallback) {
  // SETTINGS is a must have
  writeValidFrame(queueCtrl_, FrameType::SETTINGS);
  writeValidFrame(queueCtrl_, FrameType::PUSH_PRIORITY_UPDATE);
  parseControl(CodecType::CONTROL_DOWNSTREAM);
  EXPECT_EQ(1, callbacks_.urgency);
  EXPECT_TRUE(callbacks_.incremental);
}

TEST_F(HQCodecTest, PriorityCallback) {
  // SETTINGS is a must have
  writeValidFrame(queueCtrl_, FrameType::SETTINGS);
  writeValidFrame(queueCtrl_, FrameType::FB_PRIORITY_UPDATE);
  parseControl(CodecType::CONTROL_DOWNSTREAM);
  EXPECT_EQ(1, callbacks_.urgency);
  EXPECT_TRUE(callbacks_.incremental);
}

TEST_F(HQCodecTest, PushPriorityCallback) {
  // SETTINGS is a must have
  writeValidFrame(queueCtrl_, FrameType::SETTINGS);
  writeValidFrame(queueCtrl_, FrameType::FB_PUSH_PRIORITY_UPDATE);
  parseControl(CodecType::CONTROL_DOWNSTREAM);
  EXPECT_EQ(1, callbacks_.urgency);
  EXPECT_TRUE(callbacks_.incremental);
}

template <class T>
void HQCodecTestFixture<T>::testGoaway(HQControlCodec& codec,
                                       uint64_t drainId) {
  writeValidFrame(queueCtrl_, FrameType::SETTINGS);
  // Send draining goaway
  EXPECT_FALSE(codec.isWaitingToDrain());
  auto size = codec.generateGoaway(
      queueCtrl_, HTTPCodec::MaxStreamID, ErrorCode::NO_ERROR, nullptr);
  EXPECT_GT(size, 0);
  EXPECT_TRUE(codec.isWaitingToDrain());

  // HQControlCodec doesn't track the id's on it's own.  Asking for a second
  // goaway with MaxStreamID will send a final with MAX
  size = codec.generateGoaway(
      queueCtrl_, HTTPCodec::MaxStreamID, ErrorCode::NO_ERROR, nullptr);
  EXPECT_GT(size, 0);

  // Ask for another GOAWAY, no-op
  size = codec.generateGoaway(
      queueCtrl_, HTTPCodec::MaxStreamID, ErrorCode::NO_ERROR, nullptr);
  EXPECT_EQ(size, 0);
  parseControl(CodecType::CONTROL_DOWNSTREAM);
  EXPECT_EQ(callbacks_.goaways, 2);
  EXPECT_THAT(callbacks_.goawayStreamIds, ElementsAre(drainId, drainId));
}

TEST_F(HQCodecTest, ServerGoaway) {
  testGoaway(downstreamControlCodec_, kMaxClientBidiStreamId);
}

TEST_F(HQCodecTest, ClientGoaway) {
  testGoaway(upstreamControlCodec_, kMaxPushId + 1);
}

TEST_F(HQCodecTest, HighAscii) {
  std::vector<HTTPMessage> reqs;
  reqs.push_back(getGetRequest("/guacamole\xff"));

  reqs.push_back(getGetRequest("/guacamole"));
  reqs.back().getHeaders().set(HTTP_HEADER_HOST, std::string("foo.com\xff"));

  reqs.push_back(getGetRequest("/guacamole"));
  reqs.back().getHeaders().set(folly::StringPiece("Foo\xff"), "bar");

  reqs.push_back(getGetRequest("/guacamole"));
  reqs.back().getHeaders().set("Foo", std::string("bar\xff"));

  for (auto& req : reqs) {
    auto id = upstreamCodec_->createStream();
    upstreamCodec_->generateHeader(
        queue_, id, req, true, nullptr /* headerSize */);
    HQStreamCodec downstreamCodec(
        id,
        TransportDirection::DOWNSTREAM,
        qpackDownstream_,
        qpackDownEncoderWriteBuf_,
        qpackDownDecoderWriteBuf_,
        [] { return std::numeric_limits<uint64_t>::max(); },
        ingressSettings_);
    downstreamCodec.setStrictValidation(true);
    downstreamCodec.setCallback(&callbacks_);
    qpackEncoderCodec_.onUnidirectionalIngress(qpackUpEncoderWriteBuf_.move());
    auto consumed = downstreamCodec.onIngress(*queue_.front());
    queue_.trimStart(consumed);
  }

  EXPECT_EQ(callbacks_.messageBegin, 4);
  EXPECT_EQ(callbacks_.headersComplete, 0);
  EXPECT_EQ(callbacks_.messageComplete, 0);
  EXPECT_EQ(callbacks_.streamErrors, 4);
  EXPECT_EQ(callbacks_.lastParseError->getProxygenError(), kErrorParseHeader);
  EXPECT_EQ(callbacks_.sessionErrors, 0);
  callbacks_.reset();

  auto id = upstreamCodec_->createStream();
  upstreamCodec_->generateHeader(
      queue_, id, getGetRequest("/"), false, nullptr /* headerSize */);
  HTTPHeaders trailers;
  trailers.add("x-trailer-1", "pico-de-gallo\xff");
  auto g = folly::makeGuard(
      [this] { downstreamCodec_->setStrictValidation(false); });
  downstreamCodec_->setStrictValidation(true);
  upstreamCodec_->generateTrailers(queue_, id, trailers);
  parse();

  EXPECT_EQ(callbacks_.messageBegin, 1);
  EXPECT_EQ(callbacks_.headersComplete, 1);
  EXPECT_EQ(callbacks_.messageComplete, 0);
  EXPECT_EQ(callbacks_.streamErrors, 1);
  EXPECT_EQ(callbacks_.lastParseError->getProxygenError(), kErrorParseHeader);
  EXPECT_EQ(callbacks_.sessionErrors, 0);
}

struct FrameAllowedParams {
  CodecType codecType;
  FrameType frameType;
  bool allowed;
};

std::string frameParamsToTestName(
    const testing::TestParamInfo<FrameAllowedParams>& info) {
  std::string testName = "";
  switch (info.param.codecType) {
    case CodecType::CONTROL_UPSTREAM:
      testName = "UpstreamControl";
      break;
    case CodecType::CONTROL_DOWNSTREAM:
      testName = "DownstreamControl";
      break;
    case CodecType::UPSTREAM:
      testName = "Upstream";
      break;
    case CodecType::DOWNSTREAM:
      testName = "Downstream";
      break;
    default:
      LOG(FATAL) << "Unknown Codec Type";
      break;
  }
  switch (info.param.frameType) {
    case FrameType::DATA:
      testName += "Data";
      break;
    case FrameType::HEADERS:
      testName += "Headers";
      break;
    case FrameType::CANCEL_PUSH:
      testName += "CancelPush";
      break;
    case FrameType::SETTINGS:
      testName += "Settings";
      break;
    case FrameType::PUSH_PROMISE:
      testName += "PushPromise";
      break;
    case FrameType::GOAWAY:
      testName += "Goaway";
      break;
    case FrameType::MAX_PUSH_ID:
      testName += "MaxPushID";
      break;
    case FrameType::PRIORITY_UPDATE:
      testName += "RfcPriorityUpdate";
      break;
    case FrameType::PUSH_PRIORITY_UPDATE:
      testName += "RfcPushPriorityUpdate";
      break;
    case FrameType::FB_PRIORITY_UPDATE:
      testName += "PriorityUpdate";
      break;
    case FrameType::FB_PUSH_PRIORITY_UPDATE:
      testName += "PushPriorityUpdate";
      break;
    case FrameType::WEBTRANSPORT_BIDI:
      testName += "WebTransportBidiStreamType";
      break;
    default:
      testName +=
          folly::to<std::string>(static_cast<uint64_t>(info.param.frameType));
      break;
  }
  return testName;
}

class HQCodecTestFrameAllowed
    : public HQCodecTestFixture<TestWithParam<FrameAllowedParams>> {};

TEST_P(HQCodecTestFrameAllowed, FrameAllowedOnCodec) {
  int expectedFrames = 0;
  switch (GetParam().codecType) {
    case CodecType::CONTROL_UPSTREAM:
    case CodecType::CONTROL_DOWNSTREAM:
      // SETTINGS MUST be the first frame on the CONTROL stream
      if (GetParam().frameType != FrameType::SETTINGS) {
        writeValidFrame(queueCtrl_, FrameType::SETTINGS);
        expectedFrames++;
      }
      writeValidFrame(queueCtrl_, GetParam().frameType);
      // add some extra trailing junk to the input buffer
      queueCtrl_.append(IOBuf::copyBuffer("j"));
      parseControl(GetParam().codecType);
      break;
    case CodecType::UPSTREAM:
      writeValidFrame(queue_, GetParam().frameType);
      queue_.append(IOBuf::copyBuffer("j"));
      parseUpstream();
      break;
    case CodecType::DOWNSTREAM:
      writeValidFrame(queue_, GetParam().frameType);
      queue_.append(IOBuf::copyBuffer("j"));
      parse();
      break;
    default:
      CHECK(false);
      break;
  }
  expectedFrames += GetParam().allowed ? 1 : 0;
  EXPECT_EQ(callbacks_.headerFrames, expectedFrames);
  EXPECT_EQ(callbacks_.streamErrors, 0);
  EXPECT_EQ(callbacks_.sessionErrors, GetParam().allowed ? 0 : 1);
  // If an error was triggered, check that any additional parse call does not
  // raise another error, and that no new bytes are parsed
  if (!GetParam().allowed) {
    CHECK(queueCtrl_.chainLength() != 0 || queue_.chainLength() != 0);
    EXPECT_EQ(callbacks_.lastParseError->getHttp3ErrorCode(),
              HTTP3::ErrorCode::HTTP_FRAME_UNEXPECTED);
    auto lenBefore = 0;
    auto lenAfter = 0;
    switch (GetParam().codecType) {
      case CodecType::CONTROL_UPSTREAM:
      case CodecType::CONTROL_DOWNSTREAM:
        lenBefore = queueCtrl_.chainLength();
        parseControl(GetParam().codecType);
        lenAfter = queueCtrl_.chainLength();
        break;
      case CodecType::UPSTREAM:
        lenBefore = queue_.chainLength();
        parseUpstream();
        lenAfter = queue_.chainLength();
        break;
      case CodecType::DOWNSTREAM:
        lenBefore = queue_.chainLength();
        parse();
        lenAfter = queue_.chainLength();
        break;
      default:
        CHECK(false);
        break;
    }
    EXPECT_EQ(lenBefore, lenAfter);
    EXPECT_EQ(callbacks_.headerFrames, expectedFrames);
    EXPECT_EQ(callbacks_.streamErrors, 0);
    EXPECT_EQ(callbacks_.sessionErrors, 1);
    EXPECT_EQ(callbacks_.lastParseError->getHttp3ErrorCode(),
              HTTP3::ErrorCode::HTTP_FRAME_UNEXPECTED);
  }
}

INSTANTIATE_TEST_SUITE_P(
    FrameAllowedTests,
    HQCodecTestFrameAllowed,
    Values(
        (FrameAllowedParams){CodecType::DOWNSTREAM, FrameType::DATA, true},
        (FrameAllowedParams){CodecType::DOWNSTREAM, FrameType::HEADERS, true},
        (FrameAllowedParams){
            CodecType::DOWNSTREAM, FrameType::CANCEL_PUSH, false},
        (FrameAllowedParams){CodecType::DOWNSTREAM, FrameType::SETTINGS, false},
        (FrameAllowedParams){
            CodecType::DOWNSTREAM, FrameType::PUSH_PROMISE, false},
        (FrameAllowedParams){
            CodecType::UPSTREAM, FrameType::PUSH_PROMISE, true},
        (FrameAllowedParams){CodecType::DOWNSTREAM, FrameType::GOAWAY, false},
        (FrameAllowedParams){
            CodecType::DOWNSTREAM, FrameType::MAX_PUSH_ID, false},
        (FrameAllowedParams){
            CodecType::DOWNSTREAM, FrameType(*getGreaseId(0)), true},
        (FrameAllowedParams){CodecType::DOWNSTREAM,
                             FrameType(*getGreaseId(hq::kMaxGreaseIdIndex)),
                             true},
        (FrameAllowedParams){
            CodecType::DOWNSTREAM, FrameType::PRIORITY_UPDATE, false},
        (FrameAllowedParams){
            CodecType::DOWNSTREAM, FrameType::PUSH_PRIORITY_UPDATE, false},
        (FrameAllowedParams){
            CodecType::DOWNSTREAM, FrameType::FB_PRIORITY_UPDATE, false},
        (FrameAllowedParams){
            CodecType::DOWNSTREAM, FrameType::FB_PUSH_PRIORITY_UPDATE, false},
        (FrameAllowedParams){
            CodecType::DOWNSTREAM, FrameType::WEBTRANSPORT_BIDI, false},
        (FrameAllowedParams){
            CodecType::UPSTREAM, FrameType::FB_PRIORITY_UPDATE, false},
        (FrameAllowedParams){
            CodecType::UPSTREAM, FrameType::FB_PUSH_PRIORITY_UPDATE, false},
        (FrameAllowedParams){
            CodecType::UPSTREAM, FrameType::WEBTRANSPORT_BIDI, false},
        // HQ Upstream Ingress Control Codec
        (FrameAllowedParams){
            CodecType::CONTROL_UPSTREAM, FrameType::DATA, false},
        (FrameAllowedParams){
            CodecType::CONTROL_UPSTREAM, FrameType::HEADERS, false},
        (FrameAllowedParams){
            CodecType::CONTROL_UPSTREAM, FrameType::CANCEL_PUSH, true},
        (FrameAllowedParams){
            CodecType::CONTROL_UPSTREAM, FrameType::SETTINGS, true},
        (FrameAllowedParams){
            CodecType::CONTROL_UPSTREAM, FrameType::PUSH_PROMISE, false},
        (FrameAllowedParams){
            CodecType::CONTROL_UPSTREAM, FrameType::GOAWAY, true},
        (FrameAllowedParams){
            CodecType::CONTROL_UPSTREAM, FrameType::MAX_PUSH_ID, false},
        (FrameAllowedParams){
            CodecType::CONTROL_UPSTREAM, FrameType(*getGreaseId(12345)), true},
        (FrameAllowedParams){
            CodecType::CONTROL_UPSTREAM, FrameType(*getGreaseId(54321)), true},
        (FrameAllowedParams){
            CodecType::CONTROL_UPSTREAM, FrameType::PRIORITY_UPDATE, false},
        (FrameAllowedParams){CodecType::CONTROL_UPSTREAM,
                             FrameType::PUSH_PRIORITY_UPDATE,
                             false},
        (FrameAllowedParams){
            CodecType::CONTROL_UPSTREAM, FrameType::FB_PRIORITY_UPDATE, false},
        (FrameAllowedParams){CodecType::CONTROL_UPSTREAM,
                             FrameType::FB_PUSH_PRIORITY_UPDATE,
                             false},
        (FrameAllowedParams){
            CodecType::CONTROL_UPSTREAM, FrameType::WEBTRANSPORT_BIDI, false},
        // HQ Downstream Ingress Control Codec
        (FrameAllowedParams){
            CodecType::CONTROL_DOWNSTREAM, FrameType::DATA, false},
        (FrameAllowedParams){
            CodecType::CONTROL_DOWNSTREAM, FrameType::HEADERS, false},
        (FrameAllowedParams){
            CodecType::CONTROL_DOWNSTREAM, FrameType::CANCEL_PUSH, true},
        (FrameAllowedParams){
            CodecType::CONTROL_DOWNSTREAM, FrameType::SETTINGS, true},
        (FrameAllowedParams){
            CodecType::CONTROL_DOWNSTREAM, FrameType::PUSH_PROMISE, false},
        (FrameAllowedParams){
            CodecType::CONTROL_DOWNSTREAM, FrameType::MAX_PUSH_ID, true},
        (FrameAllowedParams){CodecType::CONTROL_DOWNSTREAM,
                             FrameType(*getGreaseId(98765)),
                             true},
        (FrameAllowedParams){CodecType::CONTROL_DOWNSTREAM,
                             FrameType(*getGreaseId(567879)),
                             true},
        (FrameAllowedParams){
            CodecType::CONTROL_DOWNSTREAM, FrameType::PRIORITY_UPDATE, true},
        (FrameAllowedParams){CodecType::CONTROL_DOWNSTREAM,
                             FrameType::PUSH_PRIORITY_UPDATE,
                             true},
        (FrameAllowedParams){
            CodecType::CONTROL_DOWNSTREAM, FrameType::FB_PRIORITY_UPDATE, true},
        (FrameAllowedParams){CodecType::CONTROL_DOWNSTREAM,
                             FrameType::FB_PUSH_PRIORITY_UPDATE,
                             true},
        (FrameAllowedParams){CodecType::CONTROL_DOWNSTREAM,
                             FrameType::WEBTRANSPORT_BIDI,
                             false}),
    frameParamsToTestName);

class HQCodecTestFrameBeforeSettings
    : public HQCodecTestFixture<TestWithParam<FrameAllowedParams>> {};

TEST_P(HQCodecTestFrameBeforeSettings, FrameAllowedOnControlCodec) {
  writeValidFrame(queueCtrl_, GetParam().frameType);
  parseControl(GetParam().codecType);
  EXPECT_EQ(callbacks_.headerFrames, GetParam().allowed ? 1 : 0);
  EXPECT_EQ(callbacks_.streamErrors, 0);
  EXPECT_EQ(callbacks_.sessionErrors, GetParam().allowed ? 0 : 1);
  if (!GetParam().allowed) {
    if (GetParam().frameType == hq::FrameType::DATA ||
        GetParam().frameType == hq::FrameType::HEADERS ||
        GetParam().frameType == hq::FrameType::PUSH_PROMISE) {
      EXPECT_EQ(callbacks_.lastParseError->getHttp3ErrorCode(),
                HTTP3::ErrorCode::HTTP_FRAME_UNEXPECTED);
    } else {
      EXPECT_EQ(callbacks_.lastParseError->getHttp3ErrorCode(),
                HTTP3::ErrorCode::HTTP_MISSING_SETTINGS);
    }
  }
}

INSTANTIATE_TEST_SUITE_P(
    FrameBeforeSettingsTests,
    HQCodecTestFrameBeforeSettings,
    Values(
        // HQ Upstream Ingress Control Codec
        (FrameAllowedParams){
            CodecType::CONTROL_UPSTREAM, FrameType::DATA, false},
        (FrameAllowedParams){
            CodecType::CONTROL_UPSTREAM, FrameType::HEADERS, false},
        (FrameAllowedParams){
            CodecType::CONTROL_UPSTREAM, FrameType::CANCEL_PUSH, false},
        (FrameAllowedParams){
            CodecType::CONTROL_UPSTREAM, FrameType::PUSH_PROMISE, false},
        (FrameAllowedParams){
            CodecType::CONTROL_UPSTREAM, FrameType::GOAWAY, false},
        (FrameAllowedParams){
            CodecType::CONTROL_UPSTREAM, FrameType::MAX_PUSH_ID, false},
        (FrameAllowedParams){
            CodecType::CONTROL_UPSTREAM, FrameType(*getGreaseId(24680)), false},
        (FrameAllowedParams){
            CodecType::CONTROL_UPSTREAM, FrameType(*getGreaseId(8642)), false},
        // HQ Downstream Ingress Control Codec
        (FrameAllowedParams){
            CodecType::CONTROL_DOWNSTREAM, FrameType::DATA, false},
        (FrameAllowedParams){
            CodecType::CONTROL_DOWNSTREAM, FrameType::HEADERS, false},
        (FrameAllowedParams){
            CodecType::CONTROL_DOWNSTREAM, FrameType::CANCEL_PUSH, false},
        (FrameAllowedParams){
            CodecType::CONTROL_DOWNSTREAM, FrameType::PUSH_PROMISE, false},
        (FrameAllowedParams){
            CodecType::CONTROL_DOWNSTREAM, FrameType::GOAWAY, false},
        (FrameAllowedParams){
            CodecType::CONTROL_DOWNSTREAM, FrameType::MAX_PUSH_ID, false},
        (FrameAllowedParams){CodecType::CONTROL_DOWNSTREAM,
                             FrameType(*getGreaseId(12121212)),
                             false},
        (FrameAllowedParams){CodecType::CONTROL_DOWNSTREAM,
                             FrameType(*getGreaseId(3434343434)),
                             false}),
    frameParamsToTestName);
