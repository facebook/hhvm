/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <proxygen/lib/http/codec/HTTPCodecFilter.h>

namespace proxygen {

// HTTPCodec::Callback methods
void PassThroughHTTPCodecFilter::onMessageBegin(StreamID stream,
                                                HTTPMessage* msg) {
  callback_->onMessageBegin(stream, msg);
}

void PassThroughHTTPCodecFilter::onPushMessageBegin(StreamID stream,
                                                    StreamID assocStream,
                                                    HTTPMessage* msg) {
  callback_->onPushMessageBegin(stream, assocStream, msg);
}

void PassThroughHTTPCodecFilter::onExMessageBegin(StreamID stream,
                                                  StreamID controlStream,
                                                  bool unidirectional,
                                                  HTTPMessage* msg) {
  callback_->onExMessageBegin(stream, controlStream, unidirectional, msg);
}

void PassThroughHTTPCodecFilter::onHeadersComplete(
    StreamID stream, std::unique_ptr<HTTPMessage> msg) {
  callback_->onHeadersComplete(stream, std::move(msg));
}

void PassThroughHTTPCodecFilter::onBody(StreamID stream,
                                        std::unique_ptr<folly::IOBuf> chain,
                                        uint16_t padding) {
  callback_->onBody(stream, std::move(chain), padding);
}

void PassThroughHTTPCodecFilter::onChunkHeader(StreamID stream, size_t length) {
  callback_->onChunkHeader(stream, length);
}

void PassThroughHTTPCodecFilter::onChunkComplete(StreamID stream) {
  callback_->onChunkComplete(stream);
}

void PassThroughHTTPCodecFilter::onTrailersComplete(
    StreamID stream, std::unique_ptr<HTTPHeaders> trailers) {
  callback_->onTrailersComplete(stream, std::move(trailers));
}

void PassThroughHTTPCodecFilter::onMessageComplete(StreamID stream,
                                                   bool upgrade) {
  callback_->onMessageComplete(stream, upgrade);
}

void PassThroughHTTPCodecFilter::onFrameHeader(StreamID stream_id,
                                               uint8_t flags,
                                               uint64_t length,
                                               uint64_t type,
                                               uint16_t version) {
  callback_->onFrameHeader(stream_id, flags, length, type, version);
}

void PassThroughHTTPCodecFilter::onError(StreamID stream,
                                         const HTTPException& error,
                                         bool newStream) {
  callback_->onError(stream, error, newStream);
}

void PassThroughHTTPCodecFilter::onAbort(StreamID stream, ErrorCode code) {
  callback_->onAbort(stream, code);
}

void PassThroughHTTPCodecFilter::onGoaway(
    uint64_t lastGoodStreamID,
    ErrorCode code,
    std::unique_ptr<folly::IOBuf> debugData) {
  callback_->onGoaway(lastGoodStreamID, code, std::move(debugData));
}

void PassThroughHTTPCodecFilter::onPingRequest(uint64_t data) {
  callback_->onPingRequest(data);
}

void PassThroughHTTPCodecFilter::onPingReply(uint64_t data) {
  callback_->onPingReply(data);
}

void PassThroughHTTPCodecFilter::onWindowUpdate(StreamID stream,
                                                uint32_t amount) {
  callback_->onWindowUpdate(stream, amount);
}

void PassThroughHTTPCodecFilter::onSettings(const SettingsList& settings) {
  callback_->onSettings(settings);
}

void PassThroughHTTPCodecFilter::onSettingsAck() {
  callback_->onSettingsAck();
}

void PassThroughHTTPCodecFilter::onPriority(
    StreamID stream, const HTTPMessage::HTTP2Priority& pri) {
  callback_->onPriority(stream, pri);
}

void PassThroughHTTPCodecFilter::onPriority(StreamID stream,
                                            const HTTPPriority& pri) {
  callback_->onPriority(stream, pri);
}

void PassThroughHTTPCodecFilter::onPushPriority(StreamID stream,
                                                const HTTPPriority& pri) {
  callback_->onPushPriority(stream, pri);
}

bool PassThroughHTTPCodecFilter::onNativeProtocolUpgrade(
    StreamID streamID,
    CodecProtocol protocol,
    const std::string& protocolString,
    HTTPMessage& msg) {
  return callback_->onNativeProtocolUpgrade(
      streamID, protocol, protocolString, msg);
}

void PassThroughHTTPCodecFilter::onGenerateFrameHeader(StreamID streamID,
                                                       uint8_t type,
                                                       uint64_t length,
                                                       uint16_t version) {
  callback_->onGenerateFrameHeader(streamID, length, type, version);
}

void PassThroughHTTPCodecFilter::onCertificateRequest(
    uint16_t requestId, std::unique_ptr<folly::IOBuf> authRequest) {
  callback_->onCertificateRequest(requestId, std::move(authRequest));
}

void PassThroughHTTPCodecFilter::onCertificate(
    uint16_t certId, std::unique_ptr<folly::IOBuf> authenticator) {
  callback_->onCertificate(certId, std::move(authenticator));
}

uint32_t PassThroughHTTPCodecFilter::numOutgoingStreams() const {
  return callback_->numOutgoingStreams();
}

uint32_t PassThroughHTTPCodecFilter::numIncomingStreams() const {
  return callback_->numIncomingStreams();
}

// PassThroughHTTPCodec methods
CompressionInfo PassThroughHTTPCodecFilter::getCompressionInfo() const {
  return call_->getCompressionInfo();
}

CodecProtocol PassThroughHTTPCodecFilter::getProtocol() const {
  return call_->getProtocol();
}

const std::string& PassThroughHTTPCodecFilter::getUserAgent() const {
  return call_->getUserAgent();
}

TransportDirection PassThroughHTTPCodecFilter::getTransportDirection() const {
  return call_->getTransportDirection();
}

bool PassThroughHTTPCodecFilter::supportsStreamFlowControl() const {
  return call_->supportsStreamFlowControl();
}

bool PassThroughHTTPCodecFilter::supportsSessionFlowControl() const {
  return call_->supportsSessionFlowControl();
}

HTTPCodec::StreamID PassThroughHTTPCodecFilter::createStream() {
  return call_->createStream();
}

void PassThroughHTTPCodecFilter::setCallback(HTTPCodec::Callback* callback) {
  setCallbackInternal(callback);
}

bool PassThroughHTTPCodecFilter::isBusy() const {
  return call_->isBusy();
}

void PassThroughHTTPCodecFilter::setParserPaused(bool paused) {
  call_->setParserPaused(paused);
}

bool PassThroughHTTPCodecFilter::isParserPaused() const {
  return call_->isParserPaused();
}

size_t PassThroughHTTPCodecFilter::onIngress(const folly::IOBuf& buf) {
  return call_->onIngress(buf);
}

void PassThroughHTTPCodecFilter::onIngressEOF() {
  call_->onIngressEOF();
}

bool PassThroughHTTPCodecFilter::onIngressUpgradeMessage(
    const HTTPMessage& msg) {
  return call_->onIngressUpgradeMessage(msg);
}

bool PassThroughHTTPCodecFilter::isReusable() const {
  return call_->isReusable();
}

bool PassThroughHTTPCodecFilter::isWaitingToDrain() const {
  return call_->isWaitingToDrain();
}

bool PassThroughHTTPCodecFilter::closeOnEgressComplete() const {
  return call_->closeOnEgressComplete();
}

bool PassThroughHTTPCodecFilter::supportsParallelRequests() const {
  return call_->supportsParallelRequests();
}

bool PassThroughHTTPCodecFilter::supportsPushTransactions() const {
  return call_->supportsPushTransactions();
}

size_t PassThroughHTTPCodecFilter::generateConnectionPreface(
    folly::IOBufQueue& writeBuf) {
  return call_->generateConnectionPreface(writeBuf);
}

void PassThroughHTTPCodecFilter::generateHeader(
    folly::IOBufQueue& writeBuf,
    StreamID stream,
    const HTTPMessage& msg,
    bool eom,
    HTTPHeaderSize* size,
    const folly::Optional<HTTPHeaders>& extraHeaders) {
  return call_->generateHeader(writeBuf, stream, msg, eom, size, extraHeaders);
}

void PassThroughHTTPCodecFilter::generatePushPromise(folly::IOBufQueue& buf,
                                                     StreamID stream,
                                                     const HTTPMessage& msg,
                                                     StreamID assocStream,
                                                     bool eom,
                                                     HTTPHeaderSize* size) {
  return call_->generatePushPromise(buf, stream, msg, assocStream, eom, size);
}

void PassThroughHTTPCodecFilter::generateExHeader(
    folly::IOBufQueue& writeBuf,
    StreamID stream,
    const HTTPMessage& msg,
    const HTTPCodec::ExAttributes& exAttributes,
    bool eom,
    HTTPHeaderSize* size) {
  return call_->generateExHeader(
      writeBuf, stream, msg, exAttributes, eom, size);
}

size_t PassThroughHTTPCodecFilter::generateBody(
    folly::IOBufQueue& writeBuf,
    StreamID stream,
    std::unique_ptr<folly::IOBuf> chain,
    folly::Optional<uint8_t> padding,
    bool eom) {
  return call_->generateBody(writeBuf, stream, std::move(chain), padding, eom);
}

size_t PassThroughHTTPCodecFilter::generateChunkHeader(
    folly::IOBufQueue& writeBuf, StreamID stream, size_t length) {
  return call_->generateChunkHeader(writeBuf, stream, length);
}

size_t PassThroughHTTPCodecFilter::generateChunkTerminator(
    folly::IOBufQueue& writeBuf, StreamID stream) {
  return call_->generateChunkTerminator(writeBuf, stream);
}

size_t PassThroughHTTPCodecFilter::generateTrailers(
    folly::IOBufQueue& writeBuf, StreamID stream, const HTTPHeaders& trailers) {
  return call_->generateTrailers(writeBuf, stream, trailers);
}

size_t PassThroughHTTPCodecFilter::generateEOM(folly::IOBufQueue& writeBuf,
                                               StreamID stream) {
  return call_->generateEOM(writeBuf, stream);
}

size_t PassThroughHTTPCodecFilter::generateRstStream(
    folly::IOBufQueue& writeBuf, StreamID stream, ErrorCode code) {
  return call_->generateRstStream(writeBuf, stream, code);
}

size_t PassThroughHTTPCodecFilter::generateGoaway(
    folly::IOBufQueue& writeBuf,
    StreamID lastStream,
    ErrorCode statusCode,
    std::unique_ptr<folly::IOBuf> debugData) {
  return call_->generateGoaway(
      writeBuf, lastStream, statusCode, std::move(debugData));
}

size_t PassThroughHTTPCodecFilter::generatePingRequest(
    folly::IOBufQueue& writeBuf, folly::Optional<uint64_t> data) {
  return call_->generatePingRequest(writeBuf, data);
}

size_t PassThroughHTTPCodecFilter::generatePingReply(
    folly::IOBufQueue& writeBuf, uint64_t data) {
  return call_->generatePingReply(writeBuf, data);
}

size_t PassThroughHTTPCodecFilter::generateSettings(folly::IOBufQueue& buf) {
  return call_->generateSettings(buf);
}

size_t PassThroughHTTPCodecFilter::generateSettingsAck(folly::IOBufQueue& buf) {
  return call_->generateSettingsAck(buf);
}

size_t PassThroughHTTPCodecFilter::generateWindowUpdate(folly::IOBufQueue& buf,
                                                        StreamID stream,
                                                        uint32_t delta) {
  return call_->generateWindowUpdate(buf, stream, delta);
}

size_t PassThroughHTTPCodecFilter::generatePriority(
    folly::IOBufQueue& writeBuf,
    StreamID stream,
    const HTTPMessage::HTTP2Priority& pri) {
  return call_->generatePriority(writeBuf, stream, pri);
}

size_t PassThroughHTTPCodecFilter::generatePriority(folly::IOBufQueue& writeBuf,
                                                    StreamID streamId,
                                                    HTTPPriority priority) {
  return call_->generatePriority(writeBuf, streamId, priority);
}

size_t PassThroughHTTPCodecFilter::generatePushPriority(
    folly::IOBufQueue& writeBuf, StreamID pushId, HTTPPriority priority) {
  return call_->generatePriority(writeBuf, pushId, priority);
}

size_t PassThroughHTTPCodecFilter::generateCertificateRequest(
    folly::IOBufQueue& writeBuf,
    uint16_t requestId,
    std::unique_ptr<folly::IOBuf> chain) {
  return call_->generateCertificateRequest(
      writeBuf, requestId, std::move(chain));
}

size_t PassThroughHTTPCodecFilter::generateCertificate(
    folly::IOBufQueue& writeBuf,
    uint16_t certId,
    std::unique_ptr<folly::IOBuf> certData) {
  return call_->generateCertificate(writeBuf, certId, std::move(certData));
}

HTTPSettings* PassThroughHTTPCodecFilter::getEgressSettings() {
  return call_->getEgressSettings();
}

const HTTPSettings* PassThroughHTTPCodecFilter::getIngressSettings() const {
  return call_->getIngressSettings();
}

void PassThroughHTTPCodecFilter::enableDoubleGoawayDrain() {
  return call_->enableDoubleGoawayDrain();
}

void PassThroughHTTPCodecFilter::setHeaderCodecStats(
    HeaderCodec::Stats* stats) {
  call_->setHeaderCodecStats(stats);
}

HTTPCodec::StreamID PassThroughHTTPCodecFilter::getLastIncomingStreamID()
    const {
  return call_->getLastIncomingStreamID();
}

uint32_t PassThroughHTTPCodecFilter::getDefaultWindowSize() const {
  return call_->getDefaultWindowSize();
}

size_t PassThroughHTTPCodecFilter::addPriorityNodes(PriorityQueue& queue,
                                                    folly::IOBufQueue& writeBuf,
                                                    uint8_t maxLevel) {
  return call_->addPriorityNodes(queue, writeBuf, maxLevel);
}

HTTPCodec::StreamID PassThroughHTTPCodecFilter::mapPriorityToDependency(
    uint8_t priority) const {
  return call_->mapPriorityToDependency(priority);
}

int8_t PassThroughHTTPCodecFilter::mapDependencyToPriority(
    StreamID parent) const {
  return call_->mapDependencyToPriority(parent);
}

} // namespace proxygen
