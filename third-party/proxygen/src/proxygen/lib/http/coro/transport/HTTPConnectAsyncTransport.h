/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <folly/io/async/AsyncTransport.h>
#include <folly/logging/xlog.h>

#include "proxygen/lib/http/coro/transport/HTTPConnectStream.h"

namespace proxygen::coro {

/**
 * This class can be used as a folly::AsyncTransport that is built over
 * an HTTP CONNECT tunnel (usually via a forward proxy).  Do not use this
 * directly for establishing an HTTPCoroSession via the proxy (eg, end-to-end
 * HTTPS). For that use HTTPClient::getHTTPSessionViaProxy();
 *
 * Example:
 *
 *  auto proxySession = co_await HTTPClient::getHTTPSession(
 *   evb, proxyHost, proxyPort, useTls, useQuic,
 *   connectTimeout, streamTimeout);
 *  auto reservation = proxySession->reserveRequest();
 *  if (!reservation) {
 *    // error
 *  }
 *  auto connectStream = co_await HTTPConnectStream::connectUnique(
 *    proxySession, std::move(*reservation), destinationHostAndPort,
 *    connectTimeout);
 *  auto transport = std::make_shared<HTTPConnectAsyncTransport>(
 *    std::move(connectStream));
 */

class HTTPConnectAsyncTransport
    : public folly::AsyncTransport
    , public HTTPStreamSource::Callback {

 public:
  explicit HTTPConnectAsyncTransport(
      std::unique_ptr<HTTPConnectStream> connectStream);

  ~HTTPConnectAsyncTransport() override;

  folly::EventBase* getEventBase() const override {
    return connectStream_->eventBase_;
  }

  void setReadCB(ReadCallback* callback) override;

  ReadCallback* getReadCallback() const override {
    return readCallback_;
  }
  void write(WriteCallback* callback,
             const void* buf,
             size_t bytes,
             folly::WriteFlags flags = folly::WriteFlags::NONE) override {
    writeChain(callback, folly::IOBuf::wrapBuffer(buf, bytes), flags);
  }
  void writev(WriteCallback* callback,
              const iovec* vec,
              size_t count,
              folly::WriteFlags flags = folly::WriteFlags::NONE) override {
    for (size_t i = 0; i < count; i++) {
      write(callback, vec[i].iov_base, vec[i].iov_len, flags);
    }
  }
  void writeChain(WriteCallback* callback,
                  std::unique_ptr<folly::IOBuf>&& buf,
                  folly::WriteFlags flags = folly::WriteFlags::NONE) override;

  void close() override {
    shutdownWrite();
    shutdownRead();
  }

  void closeNow() override {
    shutdownWriteNow();
    shutdownRead();
  }
  void shutdownWrite() override;

  void shutdownWriteNow() override;

  void shutdownRead();

  bool good() const override {
    return readable() && writable();
  }
  bool readable() const override {
    // TODO: should only return true when data is available (AsyncSocket calls
    // poll)
    return connectStream_->canRead();
  }
  bool writable() const override {
    return connectStream_->canWrite();
  }
  bool connecting() const override {
    // Don't hand off the transport until connected
    return false;
  }
  bool error() const override {
    return ingressError_ || connectStream_->egressError_;
  }
  void attachEventBase(folly::EventBase* /*eventBase*/) override {
    XLOG(FATAL) << "Cannot change eventBase";
  }
  void detachEventBase() override {
    XLOG(FATAL) << "Cannot change eventBase";
  }
  bool isDetachable() const override {
    return false;
  }
  void setSendTimeout(uint32_t sendTimeoutMs) override {
    if (connectStream_ && connectStream_->egressSource_) {
      connectStream_->egressSource_->setReadTimeout(
          std::chrono::milliseconds(sendTimeoutMs));
    }
  }
  uint32_t getSendTimeout() const override {
    return connectStream_->egressSource_->getReadTimeout().count();
  }
  void getLocalAddress(folly::SocketAddress* address) const override {
    *address = connectStream_->localAddr_;
  }
  void getPeerAddress(folly::SocketAddress* address) const override {
    *address = connectStream_->peerAddr_;
  }

  bool isEorTrackingEnabled() const override {
    return false;
  }

  void setEorTracking(bool track) override {
    if (track) {
      XLOG(WARNING)
          << "Cannot enable EOR tracking with HTTPConnectAsyncTransport";
    }
  }

  size_t getAppBytesWritten() const override {
    return egressOffset_;
  }
  size_t getRawBytesWritten() const override {
    return egressOffset_ /* TODO: + HTTP overhead */;
  }
  size_t getAppBytesReceived() const override {
    return ingressOffset_;
  }
  size_t getRawBytesReceived() const override {
    return ingressOffset_ /* TODO: + HTTP overhead */;
  }

 private:
  folly::coro::Task<void> read();
  void errorWrites();

  // resets readCallback_ and returns the previous value
  ReadCallback* resetReadCb() {
    return std::exchange(readCallback_, nullptr);
  }

  /* HTTPStreamSource::Callback overrides */
  void bytesProcessed(HTTPCodec::StreamID id,
                      size_t amount,
                      size_t toAck) override;
  void sourceComplete(HTTPCodec::StreamID id,
                      folly::Optional<HTTPError> error) override;

  std::unique_ptr<HTTPConnectStream> connectStream_;
  ReadCallback* readCallback_{nullptr};
  folly::Optional<HTTPError> ingressError_;
  size_t egressOffset_{0};
  size_t flushedOffset_{0};
  size_t ingressOffset_{0};
  std::chrono::milliseconds sendTimeout_{std::chrono::seconds(5)};
  std::list<std::pair<size_t, WriteCallback*>> writeCallbacks_;
  folly::CancellationSource cancellationSource_;
  bool inRead_{false};
  bool deferredEof_{false};
};

} // namespace proxygen::coro
