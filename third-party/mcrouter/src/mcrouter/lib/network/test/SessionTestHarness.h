/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <deque>
#include <memory>
#include <string>

#include <folly/Function.h>
#include <folly/Range.h>
#include <folly/io/async/AsyncTransport.h>
#include <folly/io/async/EventBase.h>

#include "mcrouter/lib/Reply.h"
#include "mcrouter/lib/network/AsyncMcServerWorker.h"
#include "mcrouter/lib/network/McServerRequestContext.h"
#include "mcrouter/lib/network/McServerSession.h"

namespace facebook {
namespace memcache {

class McServerSession;
class MockAsyncSocket;

class SessionTestHarness {
 private:
  class NoopCallback : public McServerSession::StateCallback {
   public:
    void onAccepted(McServerSession&) final {}
    void onWriteQuiescence(McServerSession&) final {}
    void onCloseStart(McServerSession&) final {}
    void onCloseFinish(McServerSession&, bool) final {}
    void onShutdown() final {}
  };
  static NoopCallback noopCb;

 public:
  /**
   * Create a SessionTestHarness
   *
   * @param opts                Options to use while creating McServerSession
   * @param onTerminated        The callback to be invoked when session is
   *                            closed.
   * @param onWriteQuiescence   The callback to be invoked when all pending
   *                            writes are flushed out.
   *
   * NOTE: Look at McServerSession.h for info about the above callbacks
   */
  explicit SessionTestHarness(
      const AsyncMcServerWorkerOptions& opts,
      McServerSession::StateCallback& cb = SessionTestHarness::noopCb);

  /**
   * Input packets in order into the socket.
   *
   * We're guaranteed to call readDataAvailable(...) at least once
   * per packet, starting at each packet boundary.
   */
  template <typename... Args>
  void inputPackets(folly::StringPiece p, Args&&... args) {
    inputPacket(p);
    inputPackets(std::forward<Args>(args)...);
  }

  /**
   * Get the current list of writes on the socket.
   *
   * A write is a result of AsyncTransport::write*().
   *
   * This is stateful: a single write will only be returned by
   * this method once.
   */
  std::vector<std::string> flushWrites() {
    eventBase_.loopOnce();
    auto output = output_;
    output_.clear();
    return output;
  }

  /**
   * Stop replying to incoming requests immediately
   */
  void pause() {
    allowed_ = 0;
  }

  /**
   * Resume replying to all accumulated and new requests immediately
   */
  void resume() {
    allowed_ = -1;
    fulfillTransactions();
    flushSavedInputs();
  }

  /**
   * Reply to n accumulated or new requests; then pause again.
   * This is cumulative, resume(2); resume(2) is the same as resume(4);
   */
  void resume(size_t n) {
    if (allowed_ != -1) {
      allowed_ += n;
    }
    fulfillTransactions();
    flushSavedInputs();
  }

  /**
   * Initiate session close
   */
  void closeSession() {
    session_.close();
  }

  /**
   * Returns the list of currently accumulated paused requests' keys.
   */
  std::vector<std::string> pausedKeys() {
    std::vector<std::string> keys;
    for (auto& t : transactions_) {
      keys.push_back(t->key());
    }
    return keys;
  }

 private:
  folly::EventBase eventBase_;
  McServerSession& session_;
  std::deque<std::string> savedInputs_;
  std::vector<std::string> output_;
  folly::AsyncTransportWrapper::ReadCallback* read_;

  /* Paused state. -1 means reply to everything; >= 0 means
     reply only to that many requests */
  int allowed_{-1};
  class TransactionIf {
   public:
    virtual std::string key() const = 0;
    virtual void reply() = 0;
    virtual ~TransactionIf() = 0;
  };

  template <class Request>
  class Transaction : public TransactionIf {
   public:
    Transaction(Request&& req, folly::Function<void(const Request&)> replyFn)
        : req_(std::move(req)), replyFn_(std::move(replyFn)) {}
    std::string key() const final {
      return req_.key_ref()->fullKey().str();
    }
    void reply() final {
      replyFn_(req_);
    }

   private:
    const Request req_;
    folly::Function<void(const Request&)> replyFn_;
  };

  std::deque<std::unique_ptr<TransactionIf>> transactions_;

  void inputPackets() {}
  void flushSavedInputs();
  void inputPacket(folly::StringPiece p);

  /* MockAsyncSocket interface */
  void write(folly::StringPiece out) {
    output_.push_back(out.str());
  }

  void setReadCallback(folly::AsyncTransportWrapper::ReadCallback* read) {
    read_ = read;
  }

  folly::AsyncTransportWrapper::ReadCallback* getReadCallback() {
    return read_;
  }

  void fulfillTransactions() {
    while (!transactions_.empty() && (allowed_ == -1 || allowed_ > 0)) {
      auto& t = transactions_.front();
      t->reply();
      transactions_.pop_front();
      if (allowed_ != -1) {
        --allowed_;
      }
    }

    /* flush writes on the socket */
    eventBase_.loopOnce();
  }

  template <class Request>
  void onRequest(McServerRequestContext&& ctx, Request&& req) {
    transactions_.push_back(makeTransaction(std::move(ctx), std::move(req)));
    fulfillTransactions();
  }

  template <class Request>
  std::unique_ptr<Transaction<Request>> makeTransaction(
      McServerRequestContext&& ctx,
      Request&& req) {
    auto replyFn = [ctx = std::move(ctx)](const Request& req) mutable {
      McServerRequestContext::reply(
          std::move(ctx), createReply(DefaultReply, req));
    };
    return std::make_unique<Transaction<Request>>(
        std::move(req), std::move(replyFn));
  }

  std::unique_ptr<Transaction<McGetRequest>> makeTransaction(
      McServerRequestContext&& ctx,
      McGetRequest&& req) {
    auto value = req.key_ref()->fullKey().str() + "_value";
    auto replyFn = [ctx = std::move(ctx),
                    value = std::move(value)](const McGetRequest&) mutable {
      McGetReply reply(carbon::Result::FOUND);
      reply.value_ref() = folly::IOBuf(folly::IOBuf::COPY_BUFFER, value);
      McServerRequestContext::reply(std::move(ctx), std::move(reply));
    };
    return std::make_unique<Transaction<McGetRequest>>(
        std::move(req), std::move(replyFn));
  }

  class OnRequest {
   public:
    explicit OnRequest(SessionTestHarness& harness) : harness_(harness) {}

    template <class Request>
    void onRequest(McServerRequestContext&& ctx, Request&& req) {
      harness_.onRequest(std::move(ctx), std::move(req));
    }

   private:
    SessionTestHarness& harness_;
  };

  friend class MockAsyncSocket;
};

inline SessionTestHarness::TransactionIf::~TransactionIf() {}

} // namespace memcache
} // namespace facebook
