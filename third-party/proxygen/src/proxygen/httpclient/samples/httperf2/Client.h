/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <chrono>
#include <fizz/client/PskCache.h>
#include <folly/io/async/AsyncSSLSocket.h>
#include <proxygen/httpclient/samples/httperf2/HTTPerfStats.h>
#include <proxygen/lib/http/HQConnector.h>
#include <proxygen/lib/http/HTTPConnectorWithFizz.h>
#include <proxygen/lib/http/HTTPMessage.h>
#include <proxygen/lib/http/session/HTTPUpstreamSession.h>
#include <proxygen/lib/utils/Time.h>

/**
 * Client interface - will make N requests to a server, using persistent
 * connections if available.  Will attempt to reconnect if the server
 * closes.
 */
class Client
    : public ::proxygen::HTTPConnector::Callback
    , public ::proxygen::HQConnector::Callback
    , private folly::EventBase::LoopCallback {
 public:
  class FinishedCallback {
   public:
    virtual ~FinishedCallback() = default;

    // Invoked when this client has received responses or errors for
    // all N requests
    virtual void clientFinished(Client* client) = 0;
  };

  Client(folly::EventBase* eventBase,
         const ::proxygen::WheelTimerInstance& transactionTimeouts,
         HTTPerfStats& stats,
         folly::Optional<folly::SocketAddress> bindAddr,
         const folly::SocketAddress& address,
         ::proxygen::HTTPMessage& request,
         const std::string& rrequestData,
         uint32_t n_requests,
         Client::FinishedCallback* callback,
         const std::string& plaintextProto,
         const std::string& serverName);
  ~Client() override;

  void start();
  void exit();

  void setSSLParameters(const folly::SSLContextPtr& sslContext,
                        std::shared_ptr<folly::ssl::SSLSession> session);
  void setupFizzContext(std::shared_ptr<fizz::client::PskCache>,
                        bool pskKe,
                        const std::string& cert,
                        const std::string& key);
  void setUseQuic(bool useQuic);
  void setQuicPskCache(std::shared_ptr<quic::QuicPskCache> quicPskCache);
  void setQLoggerPath(const std::string& path);

  [[nodiscard]] bool supportsTickets() const;
  std::shared_ptr<folly::ssl::SSLSession> extractSSLSession();

  void connect();
  [[nodiscard]] bool isConnecting() const {
    return connector_.isBusy();
  }

  static void exitAllSoon();

  // HQConnector::Callback interface
  void connectSuccess(proxygen::HQUpstreamSession* session) override;
  void connectError(const quic::QuicErrorCode& error) override;

  // HTTPConnector::Callback interface
  void connectSuccess(proxygen::HTTPUpstreamSession*) override;
  void connectError(const folly::AsyncSocketException& ex) override;

  void connectSuccessCommon(proxygen::HTTPSessionBase* session);
  void connectErrorCommon();

 private:
  // LoopCallback interface
  void runLoopCallback() noexcept override;

  [[nodiscard]] bool shouldExit() const;

  void sendRequest();

  class TransactionHandler : public ::proxygen::HTTPTransaction::Handler {
   public:
    explicit TransactionHandler(Client* parent) : parent_(parent) {
    }
    void setTransaction(proxygen::HTTPTransaction* txn) noexcept override {
      txn_ = txn;
    }
    void detachTransaction() noexcept override;

    void onHeadersComplete(
        std::unique_ptr<proxygen::HTTPMessage> msg) noexcept override;

    void onBody(std::unique_ptr<folly::IOBuf> chain) noexcept override;

    void onChunkHeader(size_t /*length*/) noexcept override {
    }

    void onChunkComplete() noexcept override {
    }

    void onTrailers(
        std::unique_ptr<proxygen::HTTPHeaders> trailers) noexcept override;

    void onEOM() noexcept override;

    void onUpgrade(proxygen::UpgradeProtocol prot) noexcept override;

    void onError(const ::proxygen::HTTPException& error) noexcept override;

    void onEgressPaused() noexcept override {
    }

    void onEgressResumed() noexcept override {
    }

   private:
    Client* parent_;
    bool inMessage_{false};
    bool waitingForResponse_{false};
    ::proxygen::TimePoint requestStart_{::proxygen::getCurrentTime()};
    ::proxygen::HTTPTransaction* txn_{nullptr};
  };
  friend class TransactionHandler;

  class InfoCollector : public ::proxygen::HTTPSession::InfoCallback {
   public:
    explicit InfoCollector(Client* parent) : parent_(parent) {
    }

    void stopCallbacks();

    // HTTPSession::InfoCallback callbacks
    void onDestroy(const ::proxygen::HTTPSessionBase&) override;

   private:
    Client* parent_;
  };
  friend class InfoCollector;

  uint32_t outstandingTransactions_{0};
  folly::EventBase* eventBase_;
  HTTPerfStats& stats_;
  uint32_t requestsSent_;

  folly::Optional<folly::SocketAddress> bindAddr_;
  const folly::SocketAddress& address_;
  std::shared_ptr<folly::ssl::SSLSession> sslSession_;
  folly::SSLContextPtr sslContext_;
  std::shared_ptr<fizz::client::FizzClientContext> fizzContext_;
  std::shared_ptr<quic::QuicPskCache> quicPskCache_;
  bool useQuic_{false};

  ::proxygen::HTTPMessage& request_; // can't be const :(
  const std::string& requestData_;
  uint32_t requests_;
  FinishedCallback* callback_;

  ::proxygen::HTTPSessionBase* session_{nullptr};

  InfoCollector collector_{this};
  ::proxygen::HTTPConnectorWithFizz connector_;

  const std::string& serverName_;
  const std::string& plaintextProto_;

  ::proxygen::TimeUtil timeUtil_;

  bool inDestructor_{false};
  bool shouldReuseSession_{false};

  std::unique_ptr<proxygen::HQConnector> hqConnector_;

  std::chrono::steady_clock::time_point start_{
      std::chrono::milliseconds::zero()};
  std::chrono::steady_clock::time_point end_{std::chrono::milliseconds::zero()};

  static const std::chrono::milliseconds kConnectTimeout;

  std::shared_ptr<quic::QLogger> qlogger_;
};
