/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <proxygen/lib/http/session/HTTPTransaction.h>
#include <proxygen/lib/http/webtransport/WtStreamManager.h>
#include <proxygen/lib/http/webtransport/WtUtils.h>
#include <quic/priority/HTTPPriorityQueue.h>

namespace proxygen::detail {

// fwd decls
class WtHttpSession;

struct WtLooper : private folly::EventBase::LoopCallback {
  enum Type : uint8_t { Read = 0, Write };
  WtLooper(folly::EventBase* evb, Type type) : evb_(evb), type_(type) {
  }
  ~WtLooper() noexcept override = default;
  void schedule() noexcept; // schedules `this` if not scheduled
  void cancel() noexcept;

 private:
  folly::EventBase* evb_;
  Type type_;
};

/**
 * Transparent HttpTxnHandler to the user; the library installs this handler on
 * behalf of the user. The body chunks this handler receives is piped into the
 * WebTransportCodec, feeding into the WtStreamManager. The underlying txn_ is
 * used to egress WebTransport capsules.
 */
struct WebTransportTxnHandler : public HTTPTransactionHandler {
  using Ptr = std::unique_ptr<WebTransportTxnHandler>;
  WebTransportTxnHandler(WtLooper& readLooper,
                         WtLooper& writeLooper,
                         WtHttpSession& wtSessionCb) noexcept;

  void setTransaction(HTTPTransaction*) noexcept final;
  void detachTransaction() noexcept final {
    txn_ = nullptr;
  }
  void onHeadersComplete(std::unique_ptr<HTTPMessage>) noexcept override;
  void onBody(std::unique_ptr<folly::IOBuf>) noexcept final;
  void onEOM() noexcept final;
  void onError(const HTTPException&) noexcept override;

  void onEgressPaused() noexcept final;
  void onEgressResumed() noexcept final;

  struct BufferedIngress {
    std::unique_ptr<folly::IOBuf> data{nullptr};
    bool eom{false};
  };
  BufferedIngress moveBufferedIngress() noexcept;

  // ignored
  void onUpgrade(UpgradeProtocol) noexcept final {
  }
  void onTrailers(std::unique_ptr<HTTPHeaders>) noexcept final {
  }

  WtLooper& readLooper_;
  WtLooper& writeLooper_;
  WtHttpSession& wtHttpSess_;
  HttpWtClientCallbackPtr wtClientCb_;

  folly::exception_wrapper ex_; // ingress or egress HTTPException
  HTTPTransaction* txn_{nullptr};
  bool egressPaused_{false};

  folly::SocketAddress selfAddr_;
  folly::SocketAddress peerAddr_;

 private:
  struct {
    folly::IOBuf data; // head always empty
    bool eom{false};
  } ingress_;
};

/**
 * Shared class between http/2 & http/3 webtransport impls. This class simply
 * owns the TxnHandler that schedules the read loop when we receive data and
 * write loop when we're egress resumed.
 */
class WtHttpSession {
 public:
  WtHttpSession(WtLooper& readLoop, WtLooper& writeLoop) noexcept;
  virtual ~WtHttpSession() noexcept = default;
  // invoked by HttpTransaction on http error
  virtual void onHttpError(const HTTPException& err) noexcept;
  // invoked when both reads&writes are done; derived classes can clean up
  virtual void onDone() noexcept {
  }

  void writesDone() noexcept;
  void readsDone() noexcept;

  WebTransportTxnHandler txnHandler_;
  std::shared_ptr<WebTransport> self;
  bool writesDone_{false};
  bool readsDone_{false};
};

class H2WtSession
    : public WtHttpSession
    , public WtSessionBase {
 public:
  struct Context;
  using Ptr = std::shared_ptr<H2WtSession>;
  static Ptr make(folly::EventBase* evb,
                  detail::WtDir dir,
                  detail::WtStreamManager::WtConfig wtConfig,
                  WebTransportHandler::Ptr wtHandler) noexcept;

  ~H2WtSession() noexcept override;

  /**
   * note: WtClientCb is required for clients to determine if the WebTransport
   * was successful and return the HTTPMessage/error to the application.
   */
  void init(Ptr self, HttpWtClientCallbackPtr wtClientCb) noexcept;

  // invoked by HttpTransaction on http error
  void onHttpError(const HTTPException& err) noexcept override;
  // invoked when both reads&writes are done; derived classes can clean up
  void onDone() noexcept override;

  const folly::SocketAddress& getLocalAddress() const noexcept override;
  const folly::SocketAddress& getPeerAddress() const noexcept override;
  HTTPTransactionHandler& getTxnHandler() noexcept {
    return txnHandler_;
  }

 private:
  // private constructor, use ::make to construct
  H2WtSession(folly::EventBase* evb,
              WtLooper& readLoop,
              WtLooper& writeLoop,
              WtStreamManager& sm,
              WebTransportHandler::Ptr& wtHandler) noexcept;

  WtStreamManager& sm_;
  WebTransportHandler::Ptr& wtHandler_;
};

/**
 * WtClientCallback is used by upstream sessions (i.e. HTTPUpstreamSession &
 * HQUpstreamSession) to implement ::sendWebTransportRequest.
 * ::sendWebTransportRequest returns a handle to the WebTransport::Ptr and a
 * SemiFuture<HTTPMessage> that is resolved asynchronously via this helper
 * class.
 */
class WtClientCallback final
    : public HttpWtClientCallbackIf
    , public HTTPTransactionHandler {
 public:
  using WtReqResult = std::unique_ptr<HTTPMessage>;
  using WtReqResultPromise = folly::Promise<WtReqResult>;

  explicit WtClientCallback(WtReqResultPromise p) noexcept;
  ~WtClientCallback() noexcept override = default;

  void onHeaders(std::unique_ptr<HTTPMessage> msg) noexcept override;
  void onErr(const HTTPException& ex) noexcept override;
  WtReqResultPromise resetPromise() noexcept;

  // **ignored**
  void onHeadersComplete(std::unique_ptr<HTTPMessage>) noexcept override {
  }
  void onError(const HTTPException&) noexcept override {
  }
  void detachTransaction() noexcept override {
  }
  void setTransaction(HTTPTransaction*) noexcept override {
  }
  void onBody(std::unique_ptr<folly::IOBuf>) noexcept override {
  }
  void onTrailers(std::unique_ptr<HTTPHeaders>) noexcept override {
  }
  void onEOM() noexcept override {
  }
  void onUpgrade(UpgradeProtocol) noexcept override {
  }
  void onEgressPaused() noexcept override {
  }
  void onEgressResumed() noexcept override {
  }

 private:
  WtReqResultPromise promise{folly::Promise<WtReqResult>::makeEmpty()};
};

} // namespace proxygen::detail
