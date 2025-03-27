/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <folly/ExceptionWrapper.h>
#include <folly/portability/GMock.h>
#include <proxygen/lib/http/webtransport/WebTransport.h>

namespace proxygen::test {

using GenericApiRet = folly::Expected<folly::Unit, WebTransport::ErrorCode>;

// WebTransport stream handle that is both a read and write handle.  It's
// designed to be held by two FakeSharedWebTransport objects so that writes
// from one are delivered as reads to the other.
class FakeStreamHandle
    : public WebTransport::StreamReadHandle
    , public WebTransport::StreamWriteHandle {
 public:
  explicit FakeStreamHandle(uint64_t inId) : id(inId) {
  }

  ~FakeStreamHandle() {
    cs_.requestCancellation();
  }

  uint64_t getID() override {
    return id;
  }
  folly::CancellationToken getCancelToken() override {
    return cs_.getToken();
  }
  folly::SemiFuture<WebTransport::StreamData> readStreamData() override {
    XCHECK(!promise_) << "One read at a time";
    if (writeErr_) {
      auto exwrapper =
          folly::make_exception_wrapper<WebTransport::Exception>(*writeErr_);
      return folly::makeFuture<WebTransport::StreamData>(exwrapper);
    } else if (!buf_.empty() || fin_) {
      return folly::makeFuture(WebTransport::StreamData({buf_.move(), fin_}));
    } else {
      // need a new promise
      auto [promise, future] =
          folly::makePromiseContract<WebTransport::StreamData>();
      promise_ = std::move(promise);
      return std::move(future);
    }
  }
  GenericApiRet stopSending(uint32_t code) override {
    if (!stopSendingErrorCode_) {
      stopSendingErrorCode_ = code;
      cs_.requestCancellation();
    }
    return folly::unit;
  }

  void setImmediateDelivery(bool immediateDelivery) {
    immediateDelivery_ = immediateDelivery;
  }

  void deliverInflightData() {
    buf_.append(inflightBuf_.move());
    for (auto& [offset, deliveryCallbacks] : offsetToDeliveryCallback_) {
      for (auto& deliveryCallback : deliveryCallbacks) {
        deliveryCallback->onByteEvent(getID(), offset);
      }
    }
    offsetToDeliveryCallback_.clear();
  }

  using WriteStreamDataRet =
      folly::Expected<WebTransport::FCState, WebTransport::ErrorCode>;
  WriteStreamDataRet writeStreamData(
      std::unique_ptr<folly::IOBuf> data,
      bool fin,
      WebTransport::ByteEventCallback* deliveryCallback) override {
    if (data) {
      dataWritten_ += data->computeChainDataLength();
    }
    if (immediateDelivery_) {
      buf_.append(std::move(data));
    } else {
      inflightBuf_.append(std::move(data));
    }
    fin_ = fin;
    if (promise_) {
      if (immediateDelivery_) {
        promise_->setValue(WebTransport::StreamData({buf_.move(), fin_}));
        promise_.reset();
      }
    }

    if (deliveryCallback) {
      if (immediateDelivery_) {
        deliveryCallback->onByteEvent(getID(), dataWritten_);
      } else {
        offsetToDeliveryCallback_[dataWritten_].push_back(deliveryCallback);
      }
    }
    return WebTransport::FCState::UNBLOCKED;
  }

  folly::Expected<folly::SemiFuture<folly::Unit>, WebTransport::ErrorCode>
  awaitWritable() override {
    return folly::makeFuture(folly::unit);
  }

  GenericApiRet resetStream(uint32_t err) override {
    for (auto& [offset, deliveryCallback] : offsetToDeliveryCallback_) {
      for (auto& callback : deliveryCallback) {
        callback->onByteEventCanceled(getID(), offset);
      }
    }
    if (promise_) {
      promise_->setException(WebTransport::Exception(err));
    } else {
      writeErr_ = err;
    }
    return folly::unit;
  }
  GenericApiRet setPriority(uint8_t urgency,
                            uint64_t order,
                            bool inc) override {
    pri.emplace(std::forward_as_tuple(urgency, order, inc));
    return folly::unit;
  }

  bool open() const {
    return !fin_ && !writeErr_ && (!promise_ || !promise_->isFulfilled());
  }

  uint64_t id{0};
  folly::CancellationSource cs_;
  folly::Optional<folly::Promise<WebTransport::StreamData>> promise_;
  folly::IOBufQueue buf_{folly::IOBufQueue::cacheChainLength()};
  uint32_t dataWritten_{0};
  bool fin_{false};
  folly::Optional<std::tuple<uint8_t, uint64_t, bool>> pri;
  folly::Optional<uint32_t> writeErr_;

  // If immediateDelivery_ == false, we stash data in inflightBuf_ until
  // deliverInflightData() is called.
  bool immediateDelivery_{true};
  folly::IOBufQueue inflightBuf_{folly::IOBufQueue::cacheChainLength()};
  folly::F14FastMap<uint64_t, std::vector<WebTransport::ByteEventCallback*>>
      offsetToDeliveryCallback_;
};

// Implementation of WebTransport for testing two connected endpoints.
//
// Usage:
//
// auto [client, server] = FakeSharedWebTransport::makeSharedWebTransport();
//
// Each FakeSharedWebTransport also requires a WebTransportHandler for the peer
// to deliver new streams, datagrams, and end-of-session events.

class FakeSharedWebTransport : public WebTransport {
 public:
  static std::pair<std::unique_ptr<FakeSharedWebTransport>,
                   std::unique_ptr<FakeSharedWebTransport>>
  makeSharedWebTransport() {
    auto a = std::make_unique<FakeSharedWebTransport>();
    auto b = std::make_unique<FakeSharedWebTransport>();
    a->setPeer(b.get());
    b->setPeer(a.get());
    return {std::move(a), std::move(b)};
  }
  FakeSharedWebTransport() = default;
  ~FakeSharedWebTransport() override {
    writeHandles.clear();
    readHandles.clear();
  }

  void setPeer(FakeSharedWebTransport* peer) {
    peer_ = peer;
  }

  void setPeerHandler(WebTransportHandler* peerHandler) {
    peerHandler_ = peerHandler;
  }

  folly::Expected<StreamWriteHandle*, ErrorCode> createUniStream() override {
    auto id = nextUniStreamId_;
    nextUniStreamId_ += 4;
    auto handle = std::make_shared<FakeStreamHandle>(id);
    writeHandles.emplace(id, handle);
    peer_->readHandles.emplace(id, handle);
    peerHandler_->onNewUniStream(handle.get());
    return handle.get();
  }
  folly::Expected<BidiStreamHandle, ErrorCode> createBidiStream() override {
    auto id = nextBidiStreamId_;
    nextBidiStreamId_ += 4;
    auto readH = std::make_shared<FakeStreamHandle>(id);
    auto writeH = std::make_shared<FakeStreamHandle>(id);
    readHandles.emplace(id, readH);
    writeHandles.emplace(id, writeH);
    peer_->readHandles.emplace(id, writeH);
    peer_->writeHandles.emplace(id, readH);
    peerHandler_->onNewBidiStream({writeH.get(), readH.get()});
    return BidiStreamHandle({readH.get(), writeH.get()});
  }
  using AwaitStreamCreditRet = folly::SemiFuture<folly::Unit>;
  AwaitStreamCreditRet awaitUniStreamCredit() override {
    return folly::makeFuture(folly::unit);
  }
  AwaitStreamCreditRet awaitBidiStreamCredit() override {
    return folly::makeFuture(folly::unit);
  }

  using ReadStreamDataRet =
      folly::Expected<folly::SemiFuture<StreamData>, WebTransport::ErrorCode>;
  ReadStreamDataRet readStreamData(uint64_t id) override {
    auto h = readHandles.find(id);
    if (h == readHandles.end()) {
      return folly::makeUnexpected(WebTransport::ErrorCode::GENERIC_ERROR);
    }
    return h->second->readStreamData();
  }

  folly::Expected<FCState, ErrorCode> writeStreamData(
      uint64_t id,
      std::unique_ptr<folly::IOBuf> data,
      bool fin,
      WebTransport::ByteEventCallback* deliveryCallback) override {
    auto h = writeHandles.find(id);
    if (h == writeHandles.end()) {
      return folly::makeUnexpected(WebTransport::ErrorCode::GENERIC_ERROR);
    }
    return h->second->writeStreamData(std::move(data), fin, deliveryCallback);
  }

  folly::Expected<folly::SemiFuture<folly::Unit>, ErrorCode> awaitWritable(
      uint64_t id) override {
    auto h = writeHandles.find(id);
    if (h == writeHandles.end()) {
      return folly::makeUnexpected(WebTransport::ErrorCode::GENERIC_ERROR);
    }
    return h->second->awaitWritable();
  }

  folly::Expected<folly::Unit, ErrorCode> resetStream(uint64_t streamId,
                                                      uint32_t error) override {
    auto h = writeHandles.find(streamId);
    if (h == writeHandles.end()) {
      return folly::makeUnexpected(WebTransport::ErrorCode::GENERIC_ERROR);
    }
    return h->second->resetStream(error);
  }

  folly::Expected<folly::Unit, ErrorCode> setPriority(
      uint64_t streamId,
      uint8_t level,
      uint64_t order,
      bool incremental) override {
    auto h = writeHandles.find(streamId);
    if (h == writeHandles.end()) {
      return folly::makeUnexpected(WebTransport::ErrorCode::GENERIC_ERROR);
    }
    return h->second->setPriority(level, order, incremental);
  }

  folly::Expected<folly::Unit, ErrorCode> stopSending(uint64_t streamId,
                                                      uint32_t error) override {
    auto h = readHandles.find(streamId);
    if (h == readHandles.end()) {
      return folly::makeUnexpected(WebTransport::ErrorCode::GENERIC_ERROR);
    }
    return h->second->stopSending(error);
  }

  folly::Expected<folly::Unit, ErrorCode> sendDatagram(
      std::unique_ptr<folly::IOBuf> datagram) override {
    peerHandler_->onDatagram(std::move(datagram));
    return folly::unit;
  }

  // Close the WebTransport session, with an optional error
  //
  // Any pending futures will complete with a folly::OperationCancelled
  // exception
  folly::Expected<folly::Unit, ErrorCode> closeSession(
      folly::Optional<uint32_t> error = folly::none) override {
    for (auto& h : writeHandles) {
      if (h.second->open()) {
        h.second->resetStream(std::numeric_limits<uint32_t>::max());
      }
    }
    writeHandles.clear();
    for (auto& h : readHandles) {
      h.second->stopSending(std::numeric_limits<uint32_t>::max());
    }
    readHandles.clear();
    peerHandler_->onSessionEnd(error);
    sessionClosed_ = true;
    return folly::unit;
  }

  bool isSessionClosed() const {
    return sessionClosed_;
  }

  std::map<uint64_t, std::shared_ptr<FakeStreamHandle>> writeHandles;
  std::map<uint64_t, std::shared_ptr<FakeStreamHandle>> readHandles;

 private:
  bool sessionClosed_{false};
  uint64_t nextBidiStreamId_{0};
  uint64_t nextUniStreamId_{2};
  FakeSharedWebTransport* peer_{nullptr};
  WebTransportHandler* peerHandler_{nullptr};
};

} // namespace proxygen::test
