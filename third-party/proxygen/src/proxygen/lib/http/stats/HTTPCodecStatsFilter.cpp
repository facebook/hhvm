/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <proxygen/lib/http/stats/HTTPCodecStatsFilter.h>

#include <proxygen/lib/http/codec/HTTP2Constants.h>
#include <proxygen/lib/http/stats/HTTPCodecStats.h>

namespace proxygen {

HTTPCodecStatsFilter::HTTPCodecStatsFilter(HTTPCodecStats* counters,
                                           CodecProtocol protocol)
    : counters_(counters), protocol_(protocol) {
  counters_->incrementParallelConn(1);
}

HTTPCodecStatsFilter::~HTTPCodecStatsFilter() {
  counters_->incrementParallelConn(-1);
}

void HTTPCodecStatsFilter::onPushMessageBegin(StreamID stream,
                                              StreamID assocStream,
                                              HTTPMessage* msg) {
  counters_->recordIngressPushPromise();
  callback_->onPushMessageBegin(stream, assocStream, msg);
}

void HTTPCodecStatsFilter::onExMessageBegin(StreamID stream,
                                            StreamID controlStream,
                                            bool unidirectional,
                                            HTTPMessage* msg) {
  counters_->recordIngressExStream();
  callback_->onExMessageBegin(stream, controlStream, unidirectional, msg);
}

void HTTPCodecStatsFilter::onHeadersComplete(StreamID stream,
                                             std::unique_ptr<HTTPMessage> msg) {
  if (call_->getTransportDirection() == TransportDirection::DOWNSTREAM ||
      msg->isRequest()) {
    // Request or PUSH_PROMISE, recordIngressPushPromise already called
    counters_->recordIngressSynStream();
  } else {
    counters_->recordIngressSynReply();
  }
  callback_->onHeadersComplete(stream, std::move(msg));
}

void HTTPCodecStatsFilter::onBody(StreamID stream,
                                  std::unique_ptr<folly::IOBuf> chain,
                                  uint16_t padding) {
  counters_->recordIngressData();
  callback_->onBody(stream, std::move(chain), padding);
}

void HTTPCodecStatsFilter::onAbort(StreamID stream, ErrorCode statusCode) {
  if (stream) {
    counters_->recordIngressRst(statusCode);
  } else {
    counters_->recordIngressGoaway(statusCode);
  }
  callback_->onAbort(stream, statusCode);
}

void HTTPCodecStatsFilter::onGoaway(uint64_t lastGoodStreamID,
                                    ErrorCode statusCode,
                                    std::unique_ptr<folly::IOBuf> debugData) {
  counters_->recordIngressGoaway(statusCode);
  if (lastGoodStreamID == http2::kMaxStreamID) {
    counters_->recordIngressGoawayDrain();
  }
  callback_->onGoaway(lastGoodStreamID, statusCode, std::move(debugData));
}

void HTTPCodecStatsFilter::onPingRequest(uint64_t data) {
  counters_->recordIngressPingRequest();
  callback_->onPingRequest(data);
}

void HTTPCodecStatsFilter::onPingReply(uint64_t data) {
  counters_->recordIngressPingReply();
  callback_->onPingReply(data);
}

void HTTPCodecStatsFilter::onWindowUpdate(StreamID stream, uint32_t amount) {
  counters_->recordIngressWindowUpdate();
  callback_->onWindowUpdate(stream, amount);
}

void HTTPCodecStatsFilter::onSettings(const SettingsList& settings) {
  counters_->recordIngressSettings();
  callback_->onSettings(settings);
}

void HTTPCodecStatsFilter::onSettingsAck() {
  counters_->recordIngressSettings();
  callback_->onSettingsAck();
}

void HTTPCodecStatsFilter::onPriority(StreamID stream,
                                      const HTTPMessage::HTTP2Priority& pri) {
  counters_->recordIngressPriority();
  callback_->onPriority(stream, pri);
}

void HTTPCodecStatsFilter::onPriority(StreamID stream,
                                      const HTTPPriority& priority) {
  counters_->recordIngressPriority();
  callback_->onPriority(stream, priority);
}

void HTTPCodecStatsFilter::generateHeader(
    folly::IOBufQueue& writeBuf,
    StreamID stream,
    const HTTPMessage& msg,
    bool eom,
    HTTPHeaderSize* size,
    const folly::Optional<HTTPHeaders>& extraHeaders) {
  if (call_->getTransportDirection() == TransportDirection::UPSTREAM) {
    counters_->recordEgressSynStream();
  } else {
    counters_->recordEgressSynReply();
  }
  return call_->generateHeader(writeBuf, stream, msg, eom, size, extraHeaders);
}

void HTTPCodecStatsFilter::generatePushPromise(folly::IOBufQueue& writeBuf,
                                               StreamID stream,
                                               const HTTPMessage& msg,
                                               StreamID assocStream,
                                               bool eom,
                                               HTTPHeaderSize* size) {
  counters_->recordEgressPushPromise();
  return call_->generatePushPromise(
      writeBuf, stream, msg, assocStream, eom, size);
}

void HTTPCodecStatsFilter::generateExHeader(
    folly::IOBufQueue& writeBuf,
    StreamID stream,
    const HTTPMessage& msg,
    const HTTPCodec::ExAttributes& exAttributes,
    bool eom,
    HTTPHeaderSize* size) {
  counters_->recordEgressExStream();
  call_->generateExHeader(writeBuf, stream, msg, exAttributes, eom, size);
}

size_t HTTPCodecStatsFilter::generateBody(folly::IOBufQueue& writeBuf,
                                          StreamID stream,
                                          std::unique_ptr<folly::IOBuf> chain,
                                          folly::Optional<uint8_t> padding,
                                          bool eom) {
  counters_->recordEgressData();
  return call_->generateBody(writeBuf, stream, std::move(chain), padding, eom);
}

size_t HTTPCodecStatsFilter::generateRstStream(folly::IOBufQueue& writeBuf,
                                               StreamID stream,
                                               ErrorCode statusCode) {
  counters_->recordEgressRst(statusCode);
  return call_->generateRstStream(writeBuf, stream, statusCode);
}

size_t HTTPCodecStatsFilter::generateGoaway(
    folly::IOBufQueue& writeBuf,
    StreamID lastStream,
    ErrorCode statusCode,
    std::unique_ptr<folly::IOBuf> debugData) {
  auto written = call_->generateGoaway(
      writeBuf, lastStream, statusCode, std::move(debugData));
  if (written) {
    counters_->recordEgressGoaway(statusCode);
    if (lastStream == HTTPCodec::MaxStreamID) {
      counters_->recordEgressGoawayDrain();
    }
  }
  return written;
}

size_t HTTPCodecStatsFilter::generatePingRequest(
    folly::IOBufQueue& writeBuf, folly::Optional<uint64_t> data) {
  counters_->recordEgressPingRequest();
  return call_->generatePingRequest(writeBuf, data);
}

size_t HTTPCodecStatsFilter::generatePingReply(folly::IOBufQueue& writeBuf,
                                               uint64_t data) {
  counters_->recordEgressPingReply();
  return call_->generatePingReply(writeBuf, data);
}

size_t HTTPCodecStatsFilter::generateSettings(folly::IOBufQueue& buf) {
  counters_->recordEgressSettings();
  return call_->generateSettings(buf);
}

size_t HTTPCodecStatsFilter::generateWindowUpdate(folly::IOBufQueue& writeBuf,
                                                  StreamID stream,
                                                  uint32_t delta) {
  counters_->recordEgressWindowUpdate();
  return call_->generateWindowUpdate(writeBuf, stream, delta);
}

size_t HTTPCodecStatsFilter::generatePriority(
    folly::IOBufQueue& writeBuf,
    StreamID stream,
    const HTTPMessage::HTTP2Priority& pri) {
  counters_->recordEgressPriority();
  return call_->generatePriority(writeBuf, stream, pri);
}

size_t HTTPCodecStatsFilter::generatePriority(folly::IOBufQueue& writeBuf,
                                              StreamID streamId,
                                              HTTPPriority pri) {
  counters_->recordEgressPriority();
  return call_->generatePriority(writeBuf, streamId, pri);
}

size_t HTTPCodecStatsFilter::generatePushPriority(folly::IOBufQueue& writeBuf,
                                                  StreamID pushId,
                                                  HTTPPriority pri) {
  counters_->recordEgressPriority();
  return call_->generatePushPriority(writeBuf, pushId, pri);
}

} // namespace proxygen
