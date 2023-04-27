/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <boost/optional/optional_io.hpp>
#include <folly/portability/GTest.h>
#include <proxygen/lib/http/codec/test/MockHTTPCodec.h>
#include <proxygen/lib/utils/TestUtils.h>

namespace proxygen {

bool isH3GreaseId(uint64_t id);

/**
 * parse the input data using codec, using atOnce to determine how much data
 * should go through the parser at one time
 *
 * atOnce < 0: use random chunk lengths
 * atOnce = 0: single chunk
 * atOnce > 0: use specified chunk length
 */
template <class T>
size_t parse(
    T* codec,
    const uint8_t* inputData,
    uint32_t length,
    int32_t atOnce = 0,
    std::function<bool()> stopFn = [] { return false; }) {

  const uint8_t* start = inputData;
  size_t consumed = 0;
  std::uniform_int_distribution<uint32_t> lenDistribution(1, length / 2 + 1);
  std::mt19937 rng;

  if (atOnce == 0) {
    atOnce = length;
  }

  folly::IOBufQueue input(folly::IOBufQueue::cacheChainLength());

  // allow testing of error case for length 0
  if (length == 0) {
    input.append(folly::IOBuf::copyBuffer(start, length));
    return codec->onIngress(*input.front());
  }

  while (length > 0 && !stopFn()) {
    if (consumed == 0) {
      // Parser wants more data
      uint32_t len = atOnce;
      if (atOnce < 0) {
        // use random chunks
        len = lenDistribution(rng);
      }
      uint32_t chunkLen = std::min(length, len);
      input.append(folly::IOBuf::copyBuffer(start, chunkLen));
      start += chunkLen;
      length -= chunkLen;
    }
    consumed = codec->onIngress(*input.front());
    input.split(consumed);
    if (input.front() == nullptr && consumed > 0) {
      consumed = 0;
    }
  }
  return input.chainLength();
}

template <class T>
size_t parseUnidirectional(
    T* codec,
    const uint8_t* inputData,
    uint32_t length,
    int32_t atOnce = 0,
    std::function<bool()> stopFn = [] { return false; }) {

  const uint8_t* start = inputData;
  size_t consumed = 0;
  std::uniform_int_distribution<uint32_t> lenDistribution(1, length / 2 + 1);
  std::mt19937 rng;

  if (atOnce == 0) {
    atOnce = length;
  }

  folly::IOBufQueue input(folly::IOBufQueue::cacheChainLength());
  while (length > 0 && !stopFn()) {
    if (consumed == 0) {
      // Parser wants more data
      uint32_t len = atOnce;
      if (atOnce < 0) {
        // use random chunks
        len = lenDistribution(rng);
      }
      uint32_t chunkLen = std::min(length, len);
      input.append(folly::IOBuf::copyBuffer(start, chunkLen));
      start += chunkLen;
      length -= chunkLen;
    }
    auto initialLength = input.chainLength();
    auto ret = codec->onUnidirectionalIngress(input.move());
    input.append(std::move(ret));
    consumed = initialLength - input.chainLength();
    if (input.front() == nullptr && consumed > 0) {
      consumed = 0;
    }
  }
  return input.chainLength();
}

class FakeHTTPCodecCallback : public HTTPCodec::Callback {
 public:
  FakeHTTPCodecCallback() {
  }

  void onMessageBegin(HTTPCodec::StreamID /*stream*/, HTTPMessage*) override {
    messageBegin++;
  }
  void onPushMessageBegin(HTTPCodec::StreamID pushPromiseId,
                          HTTPCodec::StreamID assocStream,
                          HTTPMessage*) override {
    messageBegin++;
    pushId = pushPromiseId;
    assocStreamId = assocStream;
  }
  void onExMessageBegin(HTTPCodec::StreamID /*stream*/,
                        HTTPCodec::StreamID controlStream,
                        bool unidirectional,
                        HTTPMessage*) override {
    messageBegin++;
    controlStreamId = controlStream;
    isUnidirectional = unidirectional;
  }
  void onHeadersComplete(HTTPCodec::StreamID stream,
                         std::unique_ptr<HTTPMessage> inMsg) override {
    headersComplete++;
    headersCompleteId = stream;
    msg = std::move(inMsg);
  }
  void onBody(HTTPCodec::StreamID /*stream*/,
              std::unique_ptr<folly::IOBuf> chain,
              uint16_t padding) override {
    bodyCalls++;
    paddingBytes += padding;
    bodyLength += chain->computeChainDataLength();
    data_.append(std::move(chain));
  }
  void onChunkHeader(HTTPCodec::StreamID /*stream*/,
                     size_t /*length*/) override {
    chunkHeaders++;
  }
  void onChunkComplete(HTTPCodec::StreamID /*stream*/) override {
    chunkComplete++;
  }
  void onTrailersComplete(HTTPCodec::StreamID /*stream*/,
                          std::unique_ptr<HTTPHeaders> inTrailers) override {
    trailers++;
    if (msg) {
      msg->setTrailers(std::move(inTrailers));
    }
  }
  void onMessageComplete(HTTPCodec::StreamID /*stream*/,
                         bool /*upgrade*/) override {
    messageComplete++;
  }
  void onError(HTTPCodec::StreamID stream,
               const HTTPException& error,
               bool /*newStream*/) override {
    if (stream != sessionStreamId) {
      streamErrors++;
    } else {
      sessionErrors++;
    }
    lastParseError = std::make_unique<HTTPException>(error);
  }

  void onAbort(HTTPCodec::StreamID /*stream*/, ErrorCode code) override {
    ++aborts;
    lastErrorCode = code;
  }

  void onFrameHeader(HTTPCodec::StreamID /*streamId*/,
                     uint8_t /*flags*/,
                     uint64_t /*length*/,
                     uint64_t /*type*/,
                     uint16_t /*version*/) override {
    ++headerFrames;
  }

  void onGoaway(uint64_t lastStreamId,
                ErrorCode,
                std::unique_ptr<folly::IOBuf> debugData) override {
    ++goaways;
    goawayStreamIds.emplace_back(lastStreamId);
    data_.append(std::move(debugData));
  }

  void onUnknownFrame(uint64_t /*streamId*/, uint64_t frameType) override {
    ++unknownFrames;
    if (isH3GreaseId(frameType)) {
      ++greaseFrames;
    }
  }

  void onPingRequest(uint64_t data) override {
    recvPingRequest = data;
  }

  void onPingReply(uint64_t data) override {
    recvPingReply = data;
  }

  void onPriority(HTTPCodec::StreamID /*streamID*/,
                  const HTTPMessage::HTTP2Priority& pri) override {
    priority = pri;
  }

  void onPriority(HTTPCodec::StreamID, const HTTPPriority& pri) override {
    urgency = pri.urgency;
    incremental = pri.incremental;
  }

  void onWindowUpdate(HTTPCodec::StreamID stream, uint32_t amount) override {
    windowUpdateCalls++;
    windowUpdates[stream].push_back(amount);
  }

  void onSettings(const SettingsList& inSettings) override {
    settings++;
    numSettings += inSettings.size();
    for (auto& setting : inSettings) {
      if (setting.id == SettingsId::INITIAL_WINDOW_SIZE) {
        windowSize = setting.value;
      } else if (setting.id == SettingsId::MAX_CONCURRENT_STREAMS) {
        maxStreams = setting.value;
      } else if (setting.id == SettingsId::_HQ_DATAGRAM) {
        datagramEnabled = setting.value;
      }
    }
  }

  void onSettingsAck() override {
    settingsAcks++;
  }

  void onCertificateRequest(
      uint16_t requestId, std::unique_ptr<folly::IOBuf> authRequest) override {
    certificateRequests++;
    lastCertRequestId = requestId;
    data_.append(std::move(authRequest));
  }

  void onCertificate(uint16_t certId,
                     std::unique_ptr<folly::IOBuf> authenticator) override {
    certificates++;
    lastCertId = certId;
    data_.append(std::move(authenticator));
  }

  bool onNativeProtocolUpgrade(HTTPCodec::StreamID,
                               CodecProtocol,
                               const std::string&,
                               HTTPMessage&) override {
    return true;
  }

  uint32_t numOutgoingStreams() const override {
    return 0;
  }

  uint32_t numIncomingStreams() const override {
    return messageBegin;
  }

  void expectMessage(bool eom,
                     int32_t headerCount,
                     const std::string& url) const {
    expectMessageHelper(eom, headerCount, url, -1);
  }
  void expectMessage(bool eom, int32_t headerCount, int32_t statusCode) const {
    expectMessageHelper(eom, headerCount, "", statusCode);
  }

  void expectMessageHelper(bool eom,
                           int32_t headerCount,
                           const std::string& url,
                           int32_t statusCode) const {
    EXPECT_EQ(messageBegin, 1);
    EXPECT_EQ(headersComplete, 1);
    EXPECT_EQ(messageComplete, eom ? 1 : 0);
    EXPECT_EQ(streamErrors, 0);
    EXPECT_EQ(sessionErrors, 0);
    EXPECT_NE(msg, nullptr);
    if (headerCount >= 0) {
      EXPECT_EQ(msg->getHeaders().size(), headerCount);
    }
    if (!url.empty()) {
      EXPECT_EQ(msg->getURL(), url);
    } else if (statusCode > 0) {
      if (msg->isResponse()) {
        EXPECT_EQ(msg->getStatusCode(), statusCode);
      } else {
        EXPECT_EQ(msg->getPushStatusCode(), statusCode);
      }
    }
  }

  bool sessionError() const {
    return sessionErrors > 0;
  }

  std::function<bool()> getStopFn() {
    return std::bind(&FakeHTTPCodecCallback::sessionError, this);
  }

  void setSessionStreamId(HTTPCodec::StreamID streamId) {
    sessionStreamId = streamId;
  }

  void reset() {
    headersCompleteId = 0;
    assocStreamId = 0;
    pushId = 0;
    controlStreamId = 0;
    isUnidirectional = false;
    messageBegin = 0;
    headersComplete = 0;
    messageComplete = 0;
    bodyCalls = 0;
    bodyLength = 0;
    paddingBytes = 0;
    chunkHeaders = 0;
    chunkComplete = 0;
    trailers = 0;
    aborts = 0;
    goaways = 0;
    sessionErrors = 0;
    streamErrors = 0;
    recvPingRequest = 0;
    recvPingReply = 0;
    windowUpdateCalls = 0;
    settings = 0;
    numSettings = 0;
    settingsAcks = 0;
    certificateRequests = 0;
    lastCertRequestId = 0;
    certificates = 0;
    lastCertId = 0;
    windowSize = 0;
    maxStreams = 0;
    datagramEnabled = 0;
    headerFrames = 0;
    unknownFrames = 0;
    greaseFrames = 0;
    priority = HTTPMessage::HTTP2Priority(0, false, 0);
    urgency = 0;
    incremental = false;
    windowUpdates.clear();
    data_.move();
    msg.reset();
    lastParseError.reset();
    lastErrorCode = ErrorCode::NO_ERROR;
  }

  void dumpCounters(int verbosity) const {
    VLOG(verbosity) << "Dumping HTTP codec callback counters";
    VLOG(verbosity) << "headersCompleteId: " << headersCompleteId;
    VLOG(verbosity) << "assocStreamId: " << assocStreamId;
    VLOG(verbosity) << "pushId: " << pushId;
    VLOG(verbosity) << "controlStreamId: " << controlStreamId;
    VLOG(verbosity) << "unidirectional: " << isUnidirectional;
    VLOG(verbosity) << "messageBegin: " << messageBegin;
    VLOG(verbosity) << "headersComplete: " << headersComplete;
    VLOG(verbosity) << "bodyCalls: " << bodyCalls;
    VLOG(verbosity) << "bodyLength: " << bodyLength;
    VLOG(verbosity) << "paddingBytes: " << paddingBytes;
    VLOG(verbosity) << "chunkHeaders: " << chunkHeaders;
    VLOG(verbosity) << "chunkComplete: " << chunkComplete;
    VLOG(verbosity) << "trailers: " << trailers;
    VLOG(verbosity) << "aborts: " << aborts;
    VLOG(verbosity) << "goaways: " << goaways;
    VLOG(verbosity) << "sessionErrors: " << sessionErrors;
    VLOG(verbosity) << "streamErrors: " << streamErrors;
    VLOG(verbosity) << "recvPingRequest: " << recvPingRequest;
    VLOG(verbosity) << "recvPingReply: " << recvPingReply;
    VLOG(verbosity) << "windowUpdateCalls: " << windowUpdateCalls;
    VLOG(verbosity) << "settings: " << settings;
    VLOG(verbosity) << "settingsAcks: " << settingsAcks;
    VLOG(verbosity) << "certificateRequests: " << certificateRequests;
    VLOG(verbosity) << "lastCertRequestId: " << lastCertRequestId;
    VLOG(verbosity) << "certificates: " << certificates;
    VLOG(verbosity) << "lastCertId: " << lastCertId;
    VLOG(verbosity) << "windowSize: " << windowSize;
    VLOG(verbosity) << "maxStreams: " << maxStreams;
    VLOG(verbosity) << "datagramEnabled: " << datagramEnabled;
    VLOG(verbosity) << "headerFrames: " << headerFrames;
  }

  HTTPCodec::StreamID headersCompleteId{0};
  HTTPCodec::StreamID assocStreamId{0};
  HTTPCodec::StreamID pushId{0};
  HTTPCodec::StreamID controlStreamId{0};
  bool isUnidirectional{false};
  HTTPCodec::StreamID sessionStreamId{0};
  uint32_t messageBegin{0};
  uint32_t headersComplete{0};
  uint32_t messageComplete{0};
  uint32_t bodyCalls{0};
  uint32_t bodyLength{0};
  uint32_t paddingBytes{0};
  uint32_t chunkHeaders{0};
  uint32_t chunkComplete{0};
  uint32_t trailers{0};
  uint32_t aborts{0};
  uint32_t goaways{0};
  uint32_t sessionErrors{0};
  uint32_t streamErrors{0};
  uint64_t recvPingRequest{0};
  uint64_t recvPingReply{0};
  uint32_t windowUpdateCalls{0};
  uint32_t settings{0};
  uint64_t numSettings{0};
  uint32_t settingsAcks{0};
  uint32_t certificateRequests{0};
  uint16_t lastCertRequestId{0};
  uint32_t certificates{0};
  uint16_t lastCertId{0};
  uint64_t windowSize{0};
  uint64_t maxStreams{0};
  uint64_t datagramEnabled{0};
  uint32_t headerFrames{0};
  uint32_t greaseFrames{0};
  uint32_t unknownFrames{0};
  HTTPMessage::HTTP2Priority priority{0, false, 0};
  uint8_t urgency{0};
  bool incremental{false};
  std::map<proxygen::HTTPCodec::StreamID, std::vector<uint32_t>> windowUpdates;
  folly::IOBufQueue data_;

  std::unique_ptr<HTTPMessage> msg;
  std::unique_ptr<HTTPException> lastParseError;
  ErrorCode lastErrorCode;
  std::vector<HTTPCodec::StreamID> goawayStreamIds;
};

MATCHER_P(PtrBufHasLen, n, "") {
  return arg->computeChainDataLength() == n;
}

std::unique_ptr<HTTPMessage> getPriorityMessage(uint8_t priority);

std::unique_ptr<folly::IOBuf> makeBuf(uint32_t size = 10);

std::unique_ptr<testing::NiceMock<MockHTTPCodec>> makeDownstreamParallelCodec();

std::unique_ptr<testing::NiceMock<MockHTTPCodec>> makeUpstreamParallelCodec();

HTTPMessage getGetRequest(const std::string& url = std::string("/"));
HTTPMessage getBigGetRequest(const std::string& url = std::string("/"));
HTTPMessage getPostRequest(uint32_t contentLength = 200);
HTTPMessage getPubRequest(const std::string& url = std::string("/"));
HTTPMessage getChunkedPostRequest();
HTTPMessage getResponse(uint32_t code, uint32_t bodyLen = 0);
HTTPMessage getUpgradeRequest(const std::string& upgradeHeader,
                              HTTPMethod method = HTTPMethod::GET,
                              uint32_t bodyLen = 0);
HTTPMessage getResponseWithInvalidBodyLength();

std::unique_ptr<HTTPMessage> makeGetRequest();
std::unique_ptr<HTTPMessage> makePostRequest(uint32_t contentLength = 200);
std::unique_ptr<HTTPMessage> makeResponse(uint16_t statusCode);

std::tuple<std::unique_ptr<HTTPMessage>, std::unique_ptr<folly::IOBuf>>
makeResponse(uint16_t statusCode, size_t len);

// Takes a MockHTTPCodec and fakes out its interface
void fakeMockCodec(MockHTTPCodec& codec);

} // namespace proxygen
