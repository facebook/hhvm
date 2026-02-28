/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

#include <folly/io/IOBuf.h>
#include <folly/io/IOBufQueue.h>
#include <folly/io/async/AsyncSocketException.h>
#include <folly/io/async/AsyncTransport.h>
#include <folly/io/async/EventBase.h>
#include <folly/io/async/EventBaseManager.h>
#include <thrift/lib/cpp/transport/TTransportException.h>
#include <wangle/channel/Handler.h>

namespace apache::thrift {

// This Handler may only be used in a single Pipeline
class TAsyncTransportHandler : public wangle::BytesToBytesHandler,
                               public folly::AsyncTransport::ReadCallback {
 public:
  explicit TAsyncTransportHandler(
      std::shared_ptr<folly::AsyncTransport> transport)
      : transport_(std::move(transport)) {}

  TAsyncTransportHandler(TAsyncTransportHandler&&) = default;

  ~TAsyncTransportHandler() override {
    if (transport_) {
      detachReadCallback();
    }
  }

  void setTransport(const std::shared_ptr<folly::AsyncTransport>& transport) {
    transport_ = transport;
  }

  void attachReadCallback() {
    transport_->setReadCB(transport_->good() ? this : nullptr);
  }

  void detachReadCallback() {
    if (transport_->getReadCallback() == this) {
      transport_->setReadCB(nullptr);
    }
  }

  void attachEventBase(folly::EventBase* eventBase) {
    if (eventBase && !transport_->getEventBase()) {
      transport_->attachEventBase(eventBase);
    }
  }

  void detachEventBase() {
    detachReadCallback();
    if (transport_->getEventBase()) {
      transport_->detachEventBase();
    }
  }

  void attachPipeline(Context* ctx) override {
    ctx->getPipeline()->setTransport(transport_);
  }

  folly::Future<folly::Unit> write(
      Context* ctx, std::unique_ptr<folly::IOBuf> buf) override {
    if (UNLIKELY(!buf)) {
      return folly::makeFuture();
    }

    if (!transport_->good()) {
      VLOG(5) << "transport is closed in write()";
      return folly::makeFuture<folly::Unit>(
          transport::TTransportException("transport is closed in write()"));
    }

    auto cb = new WriteCallback();
    auto future = cb->promise_.getFuture();
    transport_->writeChain(cb, std::move(buf), ctx->getWriteFlags());
    return future;
  }

  folly::Future<folly::Unit> writeException(
      Context*, folly::exception_wrapper) override {
    return shutdown(true);
  }

  folly::Future<folly::Unit> close(Context* /*ctx*/) override {
    return shutdown(false);
  }

  // Must override to avoid warnings about hidden overloaded virtual due to
  // AsyncTransport::ReadCallback::readEOF()
  void readEOF(Context* ctx) override { ctx->fireReadEOF(); }

  void getReadBuffer(void** bufReturn, size_t* lenReturn) override {
    const auto readBufferSettings = getContext()->getReadBufferSettings();
    const auto ret = bufQueue_.preallocate(
        readBufferSettings.first, readBufferSettings.second);
    *bufReturn = ret.first;
    *lenReturn = ret.second;
  }

  void readDataAvailable(size_t len) noexcept override {
    bufQueue_.postallocate(len);
    getContext()->fireRead(bufQueue_);
  }

  bool isBufferMovable() noexcept override { return true; }

  void readBufferAvailable(
      std::unique_ptr<folly::IOBuf> buf) noexcept override {
    bufQueue_.append(std::move(buf));
    getContext()->fireRead(bufQueue_);
  }

  void readEOF() noexcept override { getContext()->fireReadEOF(); }

  void readErr(const folly::AsyncSocketException& ex) noexcept override {
    getContext()->fireReadException(
        folly::make_exception_wrapper<transport::TTransportException>(
            transport::TTransportException(ex)));
  }

 private:
  folly::Future<folly::Unit> shutdown(bool closeWithReset) {
    if (transport_) {
      detachReadCallback();
      if (closeWithReset) {
        transport_->closeWithReset();
      } else {
        transport_->closeNow();
      }
    }
    return folly::makeFuture();
  }

  class WriteCallback : private folly::AsyncTransport::WriteCallback {
    void writeSuccess() noexcept override {
      promise_.setValue();
      delete this;
    }

    void writeErr(
        size_t /*bytesWritten*/,
        const folly::AsyncSocketException& ex) noexcept override {
      promise_.setException(transport::TTransportException(ex));
      delete this;
    }

   private:
    friend class TAsyncTransportHandler;
    folly::Promise<folly::Unit> promise_;
  };

  folly::IOBufQueue bufQueue_{folly::IOBufQueue::cacheChainLength()};
  std::shared_ptr<folly::AsyncTransport> transport_;
};

} // namespace apache::thrift
