/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <folly/Memory.h>
#include <folly/io/async/DestructorCheck.h>
#include <proxygen/lib/http/session/HTTPTransaction.h>

#include <proxygen/lib/http/sink/HTTPSink.h>

namespace proxygen {

static const std::string kMessageFilterDefaultName_ = "Unknown";

class HTTPMessageFilter
    : public HTTPTransaction::Handler
    , public folly::DestructorCheck {
 public:
  void setNextTransactionHandler(HTTPTransaction::Handler* next) {
    nextTransactionHandler_ = CHECK_NOTNULL(next);
  }
  virtual void setPrevFilter(HTTPMessageFilter* prev) noexcept {
    if (prev_.which() == 0 && boost::get<HTTPMessageFilter*>(prev_) != prev &&
        prev && nextElementIsPaused_) {
      prev->pause();
    }
    prev_ = CHECK_NOTNULL(prev);
  }
  virtual void setPrevSink(HTTPSink* prev) noexcept {
    if (prev_.which() == 1 && boost::get<HTTPSink*>(prev_) != prev && prev &&
        nextElementIsPaused_) {
      prev->pauseIngress();
    }
    prev_ = CHECK_NOTNULL(prev);
  }
  HTTPTransaction::Handler* getNextTransactionHandler() {
    return nextTransactionHandler_;
  }

  virtual std::unique_ptr<HTTPMessageFilter> clone() noexcept = 0;

  // This function is used to validate whether a request could be DSRed.
  // The function is called pre-delegation. For body mutating filters,
  // allowDSR should return false. Other examples, beside body mutating,
  // is a filter which rate limit the response delivery
  // Note that allowDSR is a pure virtual to make the filter author
  // aware of it
  virtual bool allowDSR() const noexcept = 0;

  // These HTTPTransaction::Handler callbacks may be overwritten
  // The default behavior is to pass the call through.
  void onHeadersComplete(std::unique_ptr<HTTPMessage> msg) noexcept override {
    nextTransactionHandler_->onHeadersComplete(std::move(msg));
  }
  void onBody(std::unique_ptr<folly::IOBuf> chain) noexcept override {
    nextTransactionHandler_->onBody(std::move(chain));
  }
  void onChunkHeader(size_t length) noexcept override {
    nextTransactionHandler_->onChunkHeader(length);
  }
  void onChunkComplete() noexcept override {
    nextTransactionHandler_->onChunkComplete();
  }
  void onTrailers(std::unique_ptr<HTTPHeaders> trailers) noexcept override {
    nextTransactionHandler_->onTrailers(std::move(trailers));
  }
  void onEOM() noexcept override {
    nextTransactionHandler_->onEOM();
  }
  void onUpgrade(UpgradeProtocol protocol) noexcept override {
    nextTransactionHandler_->onUpgrade(protocol);
  }
  void onError(const HTTPException& error) noexcept override {
    nextTransactionHandler_->onError(error);
  }

  // These HTTPTransaction::Handler callbacks cannot be overrwritten
  void setTransaction(HTTPTransaction* txn) noexcept final {
    nextTransactionHandler_->setTransaction(txn);
  }
  void detachTransaction() noexcept final {
    if (prev_.which() == 1) {
      // After detachTransaction(), the HTTPTransaction will destruct itself.
      // Set it to nullptr to avoid holding a stale pointer.
      prev_ = static_cast<HTTPSink*>(nullptr);
    }
    if (nextTransactionHandler_) {
      nextTransactionHandler_->detachTransaction();
    }
  }
  void onEgressPaused() noexcept final {
    nextTransactionHandler_->onEgressPaused();
  }
  void onEgressResumed() noexcept final {
    nextTransactionHandler_->onEgressResumed();
  }
  void onPushedTransaction(HTTPTransaction* txn) noexcept final {
    nextTransactionHandler_->onPushedTransaction(txn);
  }
  void onExTransaction(HTTPTransaction* txn) noexcept final {
    nextTransactionHandler_->onExTransaction(txn);
  }

  virtual const std::string& getFilterName() const noexcept {
    return kMessageFilterDefaultName_;
  }

  virtual void pause() noexcept;

  virtual void resume(uint64_t offset) noexcept;

  // Doesn't need to propagate down a chain, call on head filter
  void detachHandlerFromSink(std::unique_ptr<HTTPSink> sink) noexcept {
    CHECK_EQ(prev_.which(), 1);
    auto prev = boost::get<HTTPSink*>(prev_);
    if (prev) {
      // prev points to the transaction, detach the handler from the
      // transaction.
      CHECK_EQ(prev, sink.get());
      prev->detachAndAbortIfIncomplete(std::move(sink));
      // Set the pointer to nullptr. It is not safe to use the pointer since
      // after this the transaction can be destroyed without notifying the
      // filter.
      prev_ = static_cast<HTTPSink*>(nullptr);
    }
  }

 protected:
  virtual void nextOnHeadersComplete(std::unique_ptr<HTTPMessage> msg) {
    nextTransactionHandler_->onHeadersComplete(std::move(msg));
  }
  virtual void nextOnBody(std::unique_ptr<folly::IOBuf> chain) {
    nextTransactionHandler_->onBody(std::move(chain));
  }
  virtual void nextOnChunkHeader(size_t length) {
    nextTransactionHandler_->onChunkHeader(length);
  }
  virtual void nextOnChunkComplete() {
    nextTransactionHandler_->onChunkComplete();
  }
  virtual void nextOnTrailers(std::unique_ptr<HTTPHeaders> trailers) {
    nextTransactionHandler_->onTrailers(std::move(trailers));
  }
  virtual void nextOnEOM() {
    nextTransactionHandler_->onEOM();
  }
  virtual void nextOnError(const HTTPException& ex) {
    nextTransactionHandler_->onError(ex);
  }
  HTTPTransaction::Handler* nextTransactionHandler_{nullptr};

  boost::variant<HTTPMessageFilter*, HTTPSink*> prev_ =
      static_cast<HTTPSink*>(nullptr);

  bool nextElementIsPaused_{false};
};

} // namespace proxygen
