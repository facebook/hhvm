/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <folly/portability/GMock.h>
#include <proxygen/lib/http/codec/HTTPCodec.h>

namespace proxygen {

#if defined(__clang__) && __clang_major__ >= 3 && __clang_minor__ >= 6
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Winconsistent-missing-override"
#endif

class MockHTTPCodec : public HTTPCodec {
 public:
  MOCK_METHOD(CodecProtocol, getProtocol, (), (const));
  MOCK_METHOD(const std::string&, getUserAgent, (), (const));
  MOCK_METHOD(TransportDirection, getTransportDirection, (), (const));
  MOCK_METHOD(bool, supportsStreamFlowControl, (), (const));
  MOCK_METHOD(bool, supportsSessionFlowControl, (), (const));
  MOCK_METHOD(HTTPCodec::StreamID, createStream, ());
  MOCK_METHOD(void, setCallback, (Callback*));
  MOCK_METHOD(bool, isBusy, (), (const));
  MOCK_METHOD(bool, hasPartialTransaction, (), (const));
  MOCK_METHOD(void, setParserPaused, (bool));
  MOCK_METHOD(bool, isParserPaused, (), (const));
  MOCK_METHOD(size_t, onIngress, (const folly::IOBuf&));
  MOCK_METHOD(void, onIngressEOF, ());
  MOCK_METHOD(bool, isReusable, (), (const));
  MOCK_METHOD(bool, isWaitingToDrain, (), (const));
  MOCK_METHOD(bool, closeOnEgressComplete, (), (const));
  MOCK_METHOD(bool, supportsParallelRequests, (), (const));
  MOCK_METHOD(bool, supportsPushTransactions, (), (const));
  MOCK_METHOD(void,
              generateHeader,
              (folly::IOBufQueue&,
               HTTPCodec::StreamID,
               const HTTPMessage&,
               bool eom,
               HTTPHeaderSize*,
               const folly::Optional<HTTPHeaders>&));
  MOCK_METHOD(void,
              generatePushPromise,
              (folly::IOBufQueue&,
               HTTPCodec::StreamID,
               const HTTPMessage&,
               HTTPCodec::StreamID,
               bool eom,
               HTTPHeaderSize*));
  MOCK_METHOD(size_t,
              generateBody,
              (folly::IOBufQueue&,
               HTTPCodec::StreamID,
               std::shared_ptr<folly::IOBuf>,
               folly::Optional<uint8_t>,
               bool));
  size_t generateBody(folly::IOBufQueue& writeBuf,
                      HTTPCodec::StreamID stream,
                      std::unique_ptr<folly::IOBuf> chain,
                      folly::Optional<uint8_t> padding,
                      bool eom) override {
    return generateBody(writeBuf,
                        stream,
                        std::shared_ptr<folly::IOBuf>(chain.release()),
                        padding,
                        eom);
  }
  MOCK_METHOD(size_t,
              generateChunkHeader,
              (folly::IOBufQueue&, HTTPCodec::StreamID, size_t));
  MOCK_METHOD(size_t,
              generateChunkTerminator,
              (folly::IOBufQueue&, HTTPCodec::StreamID));
  MOCK_METHOD(size_t,
              generateTrailers,
              (folly::IOBufQueue&, HTTPCodec::StreamID, const HTTPHeaders&));
  MOCK_METHOD(size_t, generateEOM, (folly::IOBufQueue&, HTTPCodec::StreamID));
  MOCK_METHOD(size_t,
              generateRstStream,
              (folly::IOBufQueue&, HTTPCodec::StreamID, ErrorCode));
  MOCK_METHOD(
      size_t,
      generateGoaway,
      (folly::IOBufQueue&, StreamID, ErrorCode, std::shared_ptr<folly::IOBuf>));
  size_t generateGoaway(folly::IOBufQueue& writeBuf,
                        StreamID lastStream,
                        ErrorCode statusCode,
                        std::unique_ptr<folly::IOBuf> debugData) override {
    return generateGoaway(writeBuf,
                          lastStream,
                          statusCode,
                          std::shared_ptr<folly::IOBuf>(debugData.release()));
  }

  MOCK_METHOD(size_t, generatePingRequest, (folly::IOBufQueue&));
  size_t generatePingRequest(folly::IOBufQueue& writeBuf,
                             folly::Optional<uint64_t> /* data */) override {
    return generatePingRequest(writeBuf);
  }

  MOCK_METHOD(size_t, generatePingReply, (folly::IOBufQueue&, uint64_t));
  MOCK_METHOD(size_t, generateSettings, (folly::IOBufQueue&));
  MOCK_METHOD(size_t, generateSettingsAck, (folly::IOBufQueue&));
  MOCK_METHOD(size_t,
              generateWindowUpdate,
              (folly::IOBufQueue&, StreamID, uint32_t));
  MOCK_METHOD(size_t,
              generateCertificateRequest,
              (folly::IOBufQueue&, uint16_t, std::shared_ptr<folly::IOBuf>));
  size_t generateCertificateRequest(
      folly::IOBufQueue& writeBuf,
      uint16_t requestId,
      std::unique_ptr<folly::IOBuf> authRequest) override {
    return generateCertificateRequest(
        writeBuf,
        requestId,
        std::shared_ptr<folly::IOBuf>(authRequest.release()));
  }
  MOCK_METHOD(size_t,
              generateCertificate,
              (folly::IOBufQueue&, uint16_t, std::shared_ptr<folly::IOBuf>));
  size_t generateCertificate(
      folly::IOBufQueue& writeBuf,
      uint16_t certId,
      std::unique_ptr<folly::IOBuf> authenticator) override {
    return generateCertificate(
        writeBuf,
        certId,
        std::shared_ptr<folly::IOBuf>(authenticator.release()));
  }
  MOCK_METHOD(HTTPSettings*, getEgressSettings, ());
  MOCK_METHOD(const HTTPSettings*, getIngressSettings, (), (const));
  MOCK_METHOD(void, enableDoubleGoawayDrain, ());
  MOCK_METHOD(uint32_t, getDefaultWindowSize, (), (const));
  MOCK_METHOD(size_t,
              addPriorityNodes,
              (PriorityQueue&, folly::IOBufQueue&, uint8_t));
  MOCK_METHOD(HTTPCodec::StreamID, mapPriorityToDependency, (uint8_t), (const));
};

class MockHTTPCodecCallback : public HTTPCodec::Callback {
 public:
  MOCK_METHOD(void, onMessageBegin, (HTTPCodec::StreamID, HTTPMessage*));
  MOCK_METHOD(void,
              onPushMessageBegin,
              (HTTPCodec::StreamID, HTTPCodec::StreamID, HTTPMessage*));
  MOCK_METHOD(void,
              onExMessageBegin,
              (HTTPCodec::StreamID, HTTPCodec::StreamID, bool, HTTPMessage*));
  MOCK_METHOD(void,
              onHeadersComplete,
              (HTTPCodec::StreamID, std::shared_ptr<HTTPMessage>));
  void onHeadersComplete(HTTPCodec::StreamID stream,
                         std::unique_ptr<HTTPMessage> msg) override {
    onHeadersComplete(stream, std::shared_ptr<HTTPMessage>(msg.release()));
  }
  MOCK_METHOD(void,
              onBody,
              (HTTPCodec::StreamID, std::shared_ptr<folly::IOBuf>, uint8_t));
  void onBody(HTTPCodec::StreamID stream,
              std::unique_ptr<folly::IOBuf> chain,
              uint16_t padding) override {
    onBody(stream, std::shared_ptr<folly::IOBuf>(chain.release()), padding);
  }
  MOCK_METHOD(void, onChunkHeader, (HTTPCodec::StreamID, size_t));
  MOCK_METHOD(void, onChunkComplete, (HTTPCodec::StreamID));
  MOCK_METHOD(void,
              onTrailersComplete,
              (HTTPCodec::StreamID, std::shared_ptr<HTTPHeaders>));
  void onTrailersComplete(HTTPCodec::StreamID stream,
                          std::unique_ptr<HTTPHeaders> trailers) override {
    onTrailersComplete(stream,
                       std::shared_ptr<HTTPHeaders>(trailers.release()));
  }
  MOCK_METHOD(void, onMessageComplete, (HTTPCodec::StreamID, bool));
  MOCK_METHOD(void,
              onError,
              (HTTPCodec::StreamID, std::shared_ptr<HTTPException>, bool));
  void onError(HTTPCodec::StreamID stream,
               const HTTPException& exc,
               bool newStream) override {
    onError(stream,
            std::shared_ptr<HTTPException>(new HTTPException(exc)),
            newStream);
  }
  MOCK_METHOD(void,
              onFrameHeader,
              (uint64_t, uint8_t, uint64_t, uint64_t, uint16_t));
  MOCK_METHOD(void, onAbort, (HTTPCodec::StreamID, ErrorCode));
  MOCK_METHOD(void,
              onGoaway,
              (uint64_t, ErrorCode, std::shared_ptr<folly::IOBuf>));
  void onGoaway(uint64_t lastGoodStreamID,
                ErrorCode code,
                std::unique_ptr<folly::IOBuf> debugData) override {
    onGoaway(lastGoodStreamID,
             code,
             std::shared_ptr<folly::IOBuf>(debugData.release()));
  }
  MOCK_METHOD(void, onUnknownFrame, (uint64_t, uint64_t));
  MOCK_METHOD(void, onPingRequest, (uint64_t));
  MOCK_METHOD(void, onPingReply, (uint64_t));
  MOCK_METHOD(void, onWindowUpdate, (HTTPCodec::StreamID, uint32_t));
  MOCK_METHOD(void, onSettings, (const SettingsList&));
  MOCK_METHOD(void, onSettingsAck, ());
  MOCK_METHOD(void,
              onPriority,
              (HTTPCodec::StreamID, const HTTPMessage::HTTP2Priority&));
  MOCK_METHOD(void, onPriority, (HTTPCodec::StreamID, const HTTPPriority&));
  MOCK_METHOD(void,
              onCertificateRequest,
              (uint16_t, std::shared_ptr<folly::IOBuf>));
  void onCertificateRequest(
      uint16_t requestId,
      std::unique_ptr<folly::IOBuf> certRequestData) override {
    onCertificateRequest(
        requestId, std::shared_ptr<folly::IOBuf>(certRequestData.release()));
  }
  MOCK_METHOD(void, onCertificate, (uint16_t, std::shared_ptr<folly::IOBuf>));
  void onCertificate(uint16_t certId,
                     std::unique_ptr<folly::IOBuf> certData) override {
    onCertificate(certId, std::shared_ptr<folly::IOBuf>(certData.release()));
  }
  MOCK_METHOD(
      bool,
      onNativeProtocolUpgrade,
      (HTTPCodec::StreamID, CodecProtocol, const std::string&, HTTPMessage&));
  MOCK_METHOD(void,
              onGenerateFrameHeader,
              (HTTPCodec::StreamID, uint8_t, uint64_t, uint16_t));
  MOCK_METHOD(uint32_t, numOutgoingStreams, (), (const));
  MOCK_METHOD(uint32_t, numIncomingStreams, (), (const));
};

#if defined(__clang__) && __clang_major__ >= 3 && __clang_minor__ >= 6
#pragma clang diagnostic pop
#endif

} // namespace proxygen
