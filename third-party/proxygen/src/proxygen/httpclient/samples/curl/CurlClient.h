/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <folly/io/async/EventBase.h>
#include <folly/io/async/SSLContext.h>
#include <fstream>
#include <proxygen/lib/http/HTTPConnector.h>
#include <proxygen/lib/http/session/HTTPTransaction.h>
#include <proxygen/lib/utils/URL.h>

namespace CurlService {

class CurlClient
    : public proxygen::HTTPConnector::Callback
    , public proxygen::HTTPTransactionHandler {

  class CurlPushHandler : public proxygen::HTTPTransactionHandler {

   public:
    explicit CurlPushHandler(CurlClient* parent) : parent_{parent} {
    }

    void setTransaction(proxygen::HTTPTransaction* /*txn*/) noexcept override;

    void detachTransaction() noexcept override;

    void onHeadersComplete(
        std::unique_ptr<proxygen::HTTPMessage> /*msg*/) noexcept override;

    void onBody(std::unique_ptr<folly::IOBuf> /*chain*/) noexcept override;

    void onEOM() noexcept override;

    void onError(const proxygen::HTTPException& /*error*/) noexcept override;

    void onTrailers(
        std::unique_ptr<proxygen::HTTPHeaders> /*trailers*/) noexcept override {
    }

    void onUpgrade(proxygen::UpgradeProtocol /*protocol*/) noexcept override {
    }

    void onEgressResumed() noexcept override {
    }

    void onEgressPaused() noexcept override {
    }

   private:
    // hack around the ambiguous API
    bool seenOnHeadersComplete_{false};
    // the pushed transaction
    proxygen::HTTPTransaction* pushedTxn_{nullptr};
    // information about the request

    std::unique_ptr<proxygen::HTTPMessage> promise_;
    std::unique_ptr<proxygen::HTTPMessage> response_;
    CurlClient* parent_;
  };

 public:
  CurlClient(folly::EventBase* evb,
             proxygen::HTTPMethod httpMethod,
             const proxygen::URL& url,
             const proxygen::URL* proxy,
             const proxygen::HTTPHeaders& headers,
             const std::string& inputFilename,
             bool h2c = false,
             unsigned short httpMajor = 1,
             unsigned short httpMinor = 1);

  virtual ~CurlClient() override = default;

  bool saveResponseToFile(const std::string& outputFilename);

  static proxygen::HTTPHeaders parseHeaders(const std::string& headersString);

  // initial SSL related structures
  void initializeSsl(const std::string& caPath,
                     const std::string& nextProtos,
                     const std::string& certPath = "",
                     const std::string& keyPath = "");
  void sslHandshakeFollowup(proxygen::HTTPUpstreamSession* session) noexcept;

  // HTTPConnector methods
  void connectSuccess(proxygen::HTTPUpstreamSession* session) override;
  void connectError(const folly::AsyncSocketException& ex) override;

  // HTTPTransactionHandler methods
  void setTransaction(proxygen::HTTPTransaction* txn) noexcept override;
  void detachTransaction() noexcept override;
  void onHeadersComplete(
      std::unique_ptr<proxygen::HTTPMessage> msg) noexcept override;
  void onBody(std::unique_ptr<folly::IOBuf> chain) noexcept override;
  void onTrailers(
      std::unique_ptr<proxygen::HTTPHeaders> trailers) noexcept override;
  void onEOM() noexcept override;
  void onUpgrade(proxygen::UpgradeProtocol protocol) noexcept override;
  void onError(const proxygen::HTTPException& error) noexcept override;
  void onEgressPaused() noexcept override;
  void onEgressResumed() noexcept override;
  void onPushedTransaction(
      proxygen::HTTPTransaction* /* pushedTxn */) noexcept override;

  void sendRequest(proxygen::HTTPTransaction* txn);

  // Getters
  folly::SSLContextPtr getSSLContext() {
    return sslContext_;
  }

  const std::string& getServerName() const;

  void setFlowControlSettings(int32_t recvWindow);

  const proxygen::HTTPMessage* getResponse() const {
    return response_.get();
  }

  void setLogging(bool enabled) {
    loggingEnabled_ = enabled;
  }

  void setHeadersLogging(bool enabled) {
    headersLoggingEnabled_ = enabled;
  }

  void setEOMFunc(std::function<void()> eomFunc) {
    eomFunc_ = eomFunc;
  }

 protected:
  void sendBodyFromFile();

  void setupHeaders();

  void printMessageImpl(proxygen::HTTPMessage* msg,
                        const std::string& tag = "");

  proxygen::HTTPTransaction* txn_{nullptr};
  folly::EventBase* evb_{nullptr};
  proxygen::HTTPMethod httpMethod_;
  proxygen::URL url_;
  std::unique_ptr<proxygen::URL> proxy_;
  proxygen::HTTPMessage request_;
  const std::string inputFilename_;
  folly::SSLContextPtr sslContext_;
  int32_t recvWindow_{0};
  bool loggingEnabled_{true};
  bool headersLoggingEnabled_{false};
  bool h2c_{false};
  unsigned short httpMajor_;
  unsigned short httpMinor_;
  bool egressPaused_{false};
  std::unique_ptr<std::ifstream> inputFile_;
  std::unique_ptr<std::ofstream> outputFile_;
  std::unique_ptr<std::ostream> outputStream_;

  std::unique_ptr<proxygen::HTTPMessage> response_;
  std::vector<std::unique_ptr<CurlPushHandler>> pushTxnHandlers_;
  std::chrono::time_point<std::chrono::steady_clock> txnStartTime_;

  folly::Optional<std::function<void()>> eomFunc_;

  friend class CurlPushHandler;
};

} // namespace CurlService
