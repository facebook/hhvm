/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "SessionWrapper.h"
#include <folly/Memory.h>
#include <folly/io/async/AsyncSocket.h>
#include <folly/io/async/DelayedDestruction.h>
#include <proxygen/httpserver/RequestHandler.h>
#include <proxygen/lib/http/HTTPConnector.h>

namespace proxygen {
class ResponseHandler;
}

namespace ProxyService {

class ProxyStats;

class ProxyHandler
    : public proxygen::RequestHandler
    , public folly::DelayedDestruction
    , private proxygen::HTTPConnector::Callback
    , private folly::AsyncSocket::ConnectCallback
    , private folly::AsyncReader::ReadCallback
    , private folly::AsyncWriter::WriteCallback {
 public:
  ProxyHandler(ProxyStats* stats, folly::HHWheelTimer* timer);

  ~ProxyHandler() override;

  void onRequest(
      std::unique_ptr<proxygen::HTTPMessage> headers) noexcept override;

  void onBody(std::unique_ptr<folly::IOBuf> body) noexcept override;

  void onEOM() noexcept override;

  void onUpgrade(proxygen::UpgradeProtocol /*proto*/) noexcept override {
  }

  void requestComplete() noexcept override;

  void onError(proxygen::ProxygenError err) noexcept override;

  void onEgressPaused() noexcept override;

  void onEgressResumed() noexcept override;

  void detachServerTransaction() noexcept;
  void onServerHeadersComplete(
      std::unique_ptr<proxygen::HTTPMessage> msg) noexcept;
  void onServerBody(std::unique_ptr<folly::IOBuf> chain) noexcept;
  void onServerEOM() noexcept;
  void onServerError(const proxygen::HTTPException& error) noexcept;
  void onServerEgressPaused() noexcept;
  void onServerEgressResumed() noexcept;

 private:
  void connectSuccess(proxygen::HTTPUpstreamSession* session) override;
  void connectError(const folly::AsyncSocketException& ex) override;

  class ServerTransactionHandler : public proxygen::HTTPTransactionHandler {
   public:
    explicit ServerTransactionHandler(ProxyHandler& parent) : parent_(parent) {
    }

   private:
    ProxyHandler& parent_;

    void setTransaction(proxygen::HTTPTransaction* /*txn*/) noexcept override {
      // no op
    }
    void detachTransaction() noexcept override {
      parent_.detachServerTransaction();
    }
    void onHeadersComplete(
        std::unique_ptr<proxygen::HTTPMessage> msg) noexcept override {
      parent_.onServerHeadersComplete(std::move(msg));
    }

    void onBody(std::unique_ptr<folly::IOBuf> chain) noexcept override {
      parent_.onServerBody(std::move(chain));
    }

    void onTrailers(
        std::unique_ptr<proxygen::HTTPHeaders> /*trailers*/) noexcept override {
      // ignore for now
    }
    void onEOM() noexcept override {
      parent_.onServerEOM();
    }
    void onUpgrade(proxygen::UpgradeProtocol /*protocol*/) noexcept override {
      // ignore for now
    }

    void onError(const proxygen::HTTPException& error) noexcept override {
      parent_.onServerError(error);
    }

    void onEgressPaused() noexcept override {
      parent_.onServerEgressPaused();
    }
    void onEgressResumed() noexcept override {
      parent_.onServerEgressResumed();
    }
  };

  // AsyncSocket::ConnectCallback
  void connectSuccess() noexcept override;
  void connectErr(const folly::AsyncSocketException& ex) noexcept override;

  void getReadBuffer(void** bufReturn, size_t* lenReturn) override;
  void readDataAvailable(size_t len) noexcept override;
  void readEOF() noexcept override;
  void readErr(const folly::AsyncSocketException& ex) noexcept override;

  void writeSuccess() noexcept override;
  void writeErr(size_t bytesWritten,
                const folly::AsyncSocketException& ex) noexcept override;

  void abortDownstream();
  bool checkForShutdown();

  ProxyStats* const stats_{nullptr};
  proxygen::HTTPConnector connector_;
  ServerTransactionHandler serverHandler_;
  std::unique_ptr<SessionWrapper> session_;
  proxygen::HTTPTransaction* txn_{nullptr};
  bool clientTerminated_{false};

  std::unique_ptr<proxygen::HTTPMessage> request_;

  // Only for CONNECT
  std::shared_ptr<folly::AsyncSocket> upstreamSock_;
  uint8_t sockStatus_{0};
  folly::IOBufQueue body_{folly::IOBufQueue::cacheChainLength()};
  bool downstreamIngressPaused_{false};
  bool upstreamEgressPaused_{false};
};

} // namespace ProxyService
