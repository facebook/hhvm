/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <folly/portability/GMock.h>

#include "proxygen/lib/http/coro/HTTPStreamSourceSinkFactory.h"
#include "proxygen/lib/http/coro/client/HTTPCoroSessionPool.h"
#include <proxygen/lib/http/codec/HTTP1xCodec.h>
#include <proxygen/lib/http/coro/HTTPCoroSession.h>
#include <proxygen/lib/http/coro/HTTPSource.h>
#include <proxygen/lib/http/session/test/HTTPTransactionMocks.h>

#include <folly/coro/GmockHelpers.h>

namespace proxygen::coro::test {

class MockHTTPSource : public proxygen::coro::HTTPSource {
 public:
  MOCK_METHOD(folly::coro::Task<proxygen::coro::HTTPHeaderEvent>,
              readHeaderEvent,
              ());
  MOCK_METHOD(folly::coro::Task<proxygen::coro::HTTPBodyEvent>,
              readBodyEvent,
              (uint32_t));

  MOCK_METHOD(void, stopReading, (folly::Optional<const HTTPErrorCode> error));

  MOCK_METHOD(folly::Optional<uint64_t>, getStreamID, (), (const));
};

class MockHTTPSessionContext : public HTTPSessionContext {
 public:
  MOCK_METHOD(void, initiateDrain, ());
  MOCK_METHOD(bool, isDownstream, (), (const));
  MOCK_METHOD(bool, isUpstream, (), (const));
  MOCK_METHOD(uint64_t, getSessionID, (), (const));
  MOCK_METHOD(folly::EventBase *, getEventBase, (), (const));
  MOCK_METHOD(CodecProtocol, getCodecProtocol, (), (const));
  MOCK_METHOD(const folly::SocketAddress &, getLocalAddress, (), (const));
  MOCK_METHOD(const folly::SocketAddress &, getPeerAddress, (), (const));
  MOCK_METHOD(const wangle::TransportInfo &,
              getSetupTransportInfo,
              (),
              (const));
  MOCK_METHOD(bool,
              getCurrentTransportInfo,
              (wangle::TransportInfo *, bool),
              (const));
  MOCK_METHOD(size_t,
              getSequenceNumberFromStreamId,
              (HTTPCodec::StreamID),
              (const));
  MOCK_METHOD(uint16_t, getDatagramSizeLimit, (), (const));
  MOCK_METHOD(const folly::AsyncTransportCertificate *,
              getPeerCertificate,
              (),
              (const));
  MOCK_METHOD(void, addLifecycleObserver, (LifecycleObserver *));
  MOCK_METHOD(void, removeLifecycleObserver, (LifecycleObserver *));
};

/**
 * EventBase must be initialized prior to HTTPCoroSession, and therefore also
 * MockHTTPCoroSession. We get around this by constructing the evb in the first
 * base class.
 */
class MockHTTPCoroSessionBase {
 protected:
  folly::EventBase evb_;
};

class MockHTTPCoroSession
    : public MockHTTPCoroSessionBase
    , public HTTPCoroSession {
 public:
  MockHTTPCoroSession()
      : HTTPCoroSession(
            &evb_,
            folly::SocketAddress("1.2.3.4", 12345),
            folly::SocketAddress("4.3.2.1", 54321),
            std::make_unique<HTTP1xCodec>(TransportDirection::UPSTREAM),
            wangle::TransportInfo()) {
  }

  ~MockHTTPCoroSession() override = default;

  MOCK_METHOD(folly::coro::Task<HTTPSourceHolder>,
              sendRequest,
              (HTTPSourceHolder, HTTPCoroSession::RequestReservation));

  MOCK_METHOD((folly::Expected<HTTPSourceHolder, HTTPError>),
              sendRequest,
              (RequestReservation reservation,
               const HTTPMessage &headers,
               HTTPSourceHolder bodySource),
              (noexcept));

 private:
  folly::coro::TaskWithExecutor<void> run() override {
    return co_withExecutor(&evb_,
                           ([]() -> folly::coro::Task<void> { co_return; })());
  }

  void sendPing() override {
  }

  void setConnectionFlowControl(uint32_t) override {
  }
  void registerByteEvents(HTTPCodec::StreamID,
                          folly::Optional<uint64_t>,
                          folly::Optional<HTTPByteEvent::FieldSectionInfo>,
                          uint64_t,
                          std::vector<HTTPByteEventRegistration> &&regs,
                          bool) override {
    auto localRegistrations = std::move(regs);
  }
  HTTPCodec::StreamID getSessionStreamID() const override {
    return 0;
  }
  bool getCurrentTransportInfoImpl(
      wangle::TransportInfo * /*tinfo*/) const override {
    return false;
  }
  uint16_t getDatagramSizeLimit() const override {
    return 0;
  }
  const folly::AsyncTransportCertificate *getPeerCertificate() const override {
    return nullptr;
  }
  void handleConnectionError(HTTPErrorCode, std::string) override {
  }
  void setupStreamWriteBuf(StreamState &, folly::IOBufQueue &) override {
  }

  void notifyHeaderWrite(StreamState &, bool) override {
  }
  StreamState *createReqStream() override {
    return nullptr;
  }
  bool streamRefusedByGoaway(StreamState &, HTTPCodec::StreamID) override {
    return false;
  }
  void generateResetStream(HTTPCodec::StreamID,
                           HTTPErrorCode,
                           bool,
                           bool) override {
  }
  void handleDeferredStopSending(HTTPCodec::StreamID id) override {
  }
  void handleIngressLimitExceeded(HTTPCodec::StreamID) override {
  }
  uint32_t numTransactionsAvailable() const override {
    return std::numeric_limits<uint32_t>::max();
  }
  bool checkAndHandlePushPromiseComplete(
      StreamState &, std::unique_ptr<HTTPMessage> &) override {
    return false;
  }
  folly::Expected<std::pair<HTTPCodec::StreamID, HTTPCodec::StreamID>,
                  ErrorCode>
  createEgressPushStream() override {
    return folly::makeUnexpected(ErrorCode::NO_ERROR);
  }
};

class MockHTTPHandler : public HTTPHandler {
 public:
  MockHTTPHandler() {
    ON_CALL(*this, handleRequest(testing::_, testing::_, testing::_))
        .WillByDefault(folly::coro::gmock_helpers::CoReturnByMove(nullptr));
  }

  MOCK_METHOD(folly::coro::Task<HTTPSourceHolder>,
              handleRequest,
              (folly::EventBase *, HTTPSessionContextPtr, HTTPSourceHolder));
};

class MockHTTPBodyEventQueue : public HTTPBodyEventQueue {
 public:
  explicit MockHTTPBodyEventQueue(
      folly::EventBase *evb,
      HTTPCodec::StreamID id,
      HTTPBodyEventQueue::Callback &callback,
      size_t limit = 65535,
      std::chrono::milliseconds writeTimeout = std::chrono::seconds(5))
      : HTTPBodyEventQueue(evb, id, callback, limit, writeTimeout) {
  }

  MOCK_METHOD(void, contentLengthMismatch, ());
};

class MockByteEventCallback : public HTTPByteEventCallback {
 public:
  MOCK_METHOD(void, onByteEvent, (HTTPByteEvent));
  MOCK_METHOD(void, onByteEventCanceled, (HTTPByteEvent, HTTPError));
};

class MockHTTPCoroSessionPool : public proxygen::coro::HTTPCoroSessionPool {
 public:
  explicit MockHTTPCoroSessionPool(folly::SocketAddress mockSocketAddress)
      : HTTPCoroSessionPool(&evb_,
                            mockSocketAddress.getAddressStr(),
                            mockSocketAddress.getPort()) {
  }

  virtual ~MockHTTPCoroSessionPool() = default;

  MOCK_METHOD(folly::coro::Task<coro::HTTPCoroSessionPool::GetSessionResult>,
              getSessionWithReservation,
              ());

  folly::EventBase evb_;
};

class MockHTTPStreamSourceSinkFactory
    : public proxygen::coro::HTTPStreamSourceSinkFactory {
 public:
  MOCK_METHOD(std::unique_ptr<proxygen::coro::HTTPStreamSourceUpstreamSink>,
              newHTTPStreamSourceSink,
              (folly::EventBase *,
               proxygen::coro::HTTPSessionContextPtr,
               HTTPTransactionHandler *));
};

class MockHTTPStreamSourceUpstreamSink
    : public proxygen::coro::HTTPStreamSourceUpstreamSink {
 public:
  MockHTTPStreamSourceUpstreamSink(
      folly::EventBase *evb,
      proxygen::coro::HTTPSessionContextPtr sessionCtx,
      HTTPTransactionHandler *handler,
      MockHTTPTransaction *mockTransaction)
      : proxygen::coro::HTTPStreamSourceUpstreamSink(evb, sessionCtx, handler),
        mockHTTPTransaction_(mockTransaction) {
  }

  void detachAndAbortIfIncomplete(std::unique_ptr<HTTPSink> httpSink) override {
    if (mockHTTPTransaction_) {
      mockHTTPTransaction_->setTransportCallback(nullptr);
      mockHTTPTransaction_->setHandler(nullptr);
      if (!(mockHTTPTransaction_->isEgressComplete() ||
            mockHTTPTransaction_->isEgressEOMQueued()) ||
          !mockHTTPTransaction_->isIngressComplete()) {
        mockHTTPTransaction_->sendAbort();
      }
    }
    httpSink.reset();
  }

  void sendHeaders(const HTTPMessage &headers) override {
    if (mockHTTPTransaction_) {
      mockHTTPTransaction_->sendHeaders(headers);
    }
  }

  folly::coro::Task<void> transact(
      proxygen::coro::HTTPCoroSession *upstreamSession,
      proxygen::coro::HTTPCoroSession::RequestReservation reservation)
      override {
    co_return;
  }

  void sendBody(std::unique_ptr<folly::IOBuf> body) override {
    if (mockHTTPTransaction_) {
      mockHTTPTransaction_->sendBody(std::move(body));
    }
  }

  void sendEOM() override {
    if (mockHTTPTransaction_) {
      mockHTTPTransaction_->sendEOM();
    }
  }

  void sendAbort() override {
    if (mockHTTPTransaction_) {
      mockHTTPTransaction_->sendAbort();
    }
  }

  wangle::TransportInfo fakeTransportInfo_;
  const wangle::TransportInfo &getSetupTransportInfo() const noexcept override {
    return fakeTransportInfo_;
  }

  bool isEgressPaused() const override {
    if (mockHTTPTransaction_) {
      return mockHTTPTransaction_->isEgressPaused();
    }
    return false;
  }

  // Logging
  void describe(std::ostream &) override {};

 private:
  MockHTTPTransaction *mockHTTPTransaction_;
};

} // namespace proxygen::coro::test
