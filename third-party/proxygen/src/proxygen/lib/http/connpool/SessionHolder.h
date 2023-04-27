/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <folly/IntrusiveList.h>
#include <proxygen/lib/http/ProxygenErrorEnum.h>
#include <proxygen/lib/http/connpool/Endpoint.h>
#include <proxygen/lib/http/session/HTTPSessionBase.h>

namespace proxygen {

class HTTPMessage;

/**
 * This class is essentially an implementation detail for SessionPool. It
 * encapsulates a single HTTPSessionBase and manages which list in the
 * SessionPool it should be a part of.
 */
class SessionHolder : private HTTPSessionBase::InfoCallback {
 public:
  class Callback {
   public:
    virtual ~Callback() {
    }
    virtual void detachIdle(SessionHolder*) = 0;
    virtual void detachPartiallyFilled(SessionHolder*) = 0;
    virtual void detachFilled(SessionHolder*) = 0;
    virtual void attachIdle(SessionHolder*) = 0;
    virtual void attachPartiallyFilled(SessionHolder*) = 0;
    virtual void attachFilled(SessionHolder*) = 0;
    virtual void addDrainingSession(HTTPSessionBase*) = 0;
  };

  class Stats {
   public:
    virtual ~Stats() {
    }
    virtual void onConnectionCreated() = 0;
    virtual void onConnectionClosed() = 0;
    virtual void onConnectionActivated() = 0;
    virtual void onConnectionDeactivated() = 0;
    virtual void onRead(size_t bytesRead) = 0;
    virtual void onWrite(size_t bytesWritten) = 0;
  };

  explicit SessionHolder(HTTPSessionBase*,
                         Callback*,
                         Stats* = nullptr,
                         Endpoint = Endpoint("", 0, false));
  ~SessionHolder() override;

  HTTPSessionBase* release() {
    if (listHook.is_linked()) {
      unlink();
    } else {
      state_ = ListState::DETACHED;
    }
    auto session = session_;
    session->setInfoCallback(originalSessionInfoCb_);
    session_ = nullptr;
    delete this;
    return session;
  }

  /**
   * Returns true if the given session can be wrapped in a
   * SessionHolder. This function does *not* imply that the session is
   * or isn't idle. It returns true iff a SessionHolder can be constructed
   * around it.
   */
  static bool isPoolable(const HTTPSessionBase*);

  const HTTPSessionBase& getSession() const;
  HTTPTransaction* newTransaction(HTTPTransaction::Handler* upstreamHandler);
  void drain();

  /**
   * This will immediately delete the SessionHolder and the underlying session.
   * When this function returns, the SessionHolder has been deleted.
   */
  void closeWithReset();

  std::chrono::steady_clock::time_point getLastUseTime() const;

  /**
   * Unlink this session holder instance from the necessary session lists..
   * This is achieved by calling the SessionHolder::Callbacks.
   */
  void unlink();

  /**
   * Link this session holder instance to the necessary session lists. This is
   * achieved by calling the SessionHolder::Callbacks.
   */
  void link();

  bool shouldAgeOut(std::chrono::milliseconds maxAge) const;
  void describe(std::ostream& os) const;

  Endpoint getEndpoint() {
    return endpoint_;
  }

  friend std::ostream& operator<<(std::ostream& os, const SessionHolder& conn) {
    conn.describe(os);
    return os;
  }

  // HTTPSession::InfoCallback
  void onCreate(const HTTPSessionBase&) override;
  void onIngressError(const HTTPSessionBase&, ProxygenError) override;
  void onIngressEOF() override {
  }
  void onRead(const HTTPSessionBase&, size_t bytesRead) override;
  void onRead(const HTTPSessionBase& sess,
              size_t bytesRead,
              folly::Optional<HTTPCodec::StreamID> /*stream id*/) override;
  void onWrite(const HTTPSessionBase&, size_t bytesWritten) override;
  void onRequestBegin(const HTTPSessionBase&) override;
  void onRequestEnd(const HTTPSessionBase&,
                    uint32_t maxIngressQueueSize) override;
  void onActivateConnection(const HTTPSessionBase&) override;
  void onDeactivateConnection(const HTTPSessionBase&) override;
  void onDestroy(const HTTPSessionBase&) override;
  void onIngressMessage(const HTTPSessionBase&, const HTTPMessage&) override;
  void onIngressLimitExceeded(const HTTPSessionBase&) override;
  void onIngressPaused(const HTTPSessionBase&) override;
  void onTransactionAttached(const HTTPSessionBase&) override;
  void onTransactionDetached(const HTTPSessionBase&) override;
  void onPingReplySent(int64_t latency) override;
  void onPingReplyReceived() override;
  void onSettingsOutgoingStreamsFull(const HTTPSessionBase&) override;
  void onSettingsOutgoingStreamsNotFull(const HTTPSessionBase&) override;
  void onFlowControlWindowClosed(const HTTPSessionBase&) override;
  void onEgressBuffered(const HTTPSessionBase&) override;
  void onEgressBufferCleared(const HTTPSessionBase&) override;
  void onSettings(const HTTPSessionBase&, const SettingsList&) override;
  void onSettingsAck(const HTTPSessionBase&) override;

  // Hook in the first session pool list.
  folly::SafeIntrusiveListHook listHook;

  // Hook in the second session pool list if necessary.
  folly::SafeIntrusiveListHook secondaryListHook;

 private:
  void handleTransactionDetached();

  enum class ListState {
    DETACHED = 0,
    IDLE = 1,
    PARTIAL = 2,
    FULL = 3,
  };

  HTTPSessionBase* session_;
  Callback* parent_;
  Stats* stats_;
  std::chrono::steady_clock::time_point lastUseTime_; // init'd in link()
  double jitter_;
  ListState state_{ListState::DETACHED};
  Endpoint endpoint_;
  HTTPSessionBase::InfoCallback* originalSessionInfoCb_;
};
typedef folly::CountedIntrusiveList<SessionHolder, &SessionHolder::listHook>
    SessionList;

typedef folly::CountedIntrusiveList<SessionHolder,
                                    &SessionHolder::secondaryListHook>
    SecondarySessionList;
} // namespace proxygen
