/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "proxygen/lib/http/coro/transport/HTTPConnectAsyncTransport.h"
#include <folly/logging/xlog.h>

namespace proxygen::coro {

HTTPConnectAsyncTransport::HTTPConnectAsyncTransport(
    std::unique_ptr<HTTPConnectStream> connectStream)
    : connectStream_(std::move(connectStream)) {
  connectStream_->setHTTPStreamSourceCallback(this);
}

HTTPConnectAsyncTransport::~HTTPConnectAsyncTransport() {
  XLOG(DBG4) << "~HTTPConnectAsyncTransport";
  if (auto* readCb = resetReadCb()) {
    folly::AsyncSocketException ex(
        folly::AsyncSocketException::AsyncSocketExceptionType::END_OF_FILE,
        "Socket closed locally");
    readCb->readErr(ex);
  }
  cancellationSource_.requestCancellation();
  // parent dtor will abort the ingress stream if open, so if inRead_, it will
  // exit
}

void HTTPConnectAsyncTransport::sourceComplete(
    HTTPCodec::StreamID /*id*/, folly::Optional<HTTPError> error) {
  if (error) {
    // TODO: make specific error codes (eg: timeout)
    errorWrites();
  }
}

void HTTPConnectAsyncTransport::setReadCB(ReadCallback* callback) {
  if (readCallback_ == callback) {
    return;
  }
  if (callback && !readCallback_) {
    // Setting readCallback_
    readCallback_ = callback;
    co_withExecutor(getEventBase(),
                    folly::coro::co_withCancellation(
                        cancellationSource_.getToken(), read()))
        .start();
  } else {
    XCHECK(!inRead_) << "Cannot clear the read callback while reading";
    readCallback_ = callback;
  }
}

folly::coro::Task<void> HTTPConnectAsyncTransport::read() {
  co_await folly::coro::co_safe_point;
  if (deferredEof_) {
    CHECK_NOTNULL(resetReadCb())->readEOF();
    co_return;
  }
  const auto& cancelToken = co_await folly::coro::co_current_cancellation_token;
  while (!cancelToken.isCancellationRequested() && connectStream_->canRead() &&
         readCallback_) {
    auto ingressSource = connectStream_->ingressSource_;
    ingressSource->setReadTimeout(std::chrono::milliseconds(0));
    size_t size = std::numeric_limits<uint32_t>::max();
    void* buf = nullptr;
    if (!readCallback_->isBufferMovable()) {
      readCallback_->getReadBuffer(&buf, &size);
    }
    // This is not interruptible without an unrecoverable abort
    inRead_ = true;
    XLOG(DBG5) << "ingressSource->readBodyEvent";
    auto bodyEvent = co_await co_awaitTry(ingressSource->readBodyEvent(size));
    XLOG(DBG5) << "ingressSource->readBodyEvent done";
    if (cancelToken.isCancellationRequested()) {
      co_return;
    }
    inRead_ = false;
    if (bodyEvent.hasException()) {
      XLOG(DBG3) << "readBodyEvent exception";
      ingressError_ = getHTTPError(bodyEvent);
      if (auto* readCb = resetReadCb()) {
        folly::AsyncSocketException ex(
            folly::AsyncSocketException::AsyncSocketExceptionType::UNKNOWN,
            ingressError_->msg);
        readCb->readErr(ex);
      }
      co_return;
    }
    XCHECK(readCallback_);
    if (bodyEvent->eventType == HTTPBodyEvent::BODY &&
        !bodyEvent->event.body.empty()) {
      size = bodyEvent->event.body.chainLength();
      if (readCallback_->isBufferMovable()) {
        readCallback_->readBufferAvailable(bodyEvent->event.body.move());
      } else {
        folly::io::Cursor cursor(bodyEvent->event.body.front());
        XCHECK(buf);
        cursor.pullAtMost(buf, size);
        readCallback_->readDataAvailable(size);
      }
    }

    // read callback may have been uninstalled after delivering data above; if
    // so, we must defer eof signal until next ::read()
    if (bodyEvent->eom) {
      if (auto* readCb = resetReadCb()) {
        readCb->readEOF();
      } else {
        deferredEof_ = true;
      }
    }
    // Ignore any other events -- only BODY is allowed in CONNECT so this
    // is really about future proofing
  }
}

void HTTPConnectAsyncTransport::writeChain(WriteCallback* callback,
                                           std::unique_ptr<folly::IOBuf>&& buf,
                                           folly::WriteFlags /*flags*/) {
  if (!writable()) {
    if (callback) {
      folly::AsyncSocketException ex(
          folly::AsyncSocketException::AsyncSocketExceptionType::NOT_OPEN,
          "writeChain on closed or bad transport");
      callback->writeErr(0, ex);
    }
    return;
  }
  if (!buf) {
    return;
  }
  egressOffset_ += buf->computeChainDataLength();

  auto fcState = connectStream_->egressSource_->body(std::move(buf), 0, false);
  if (callback) {
    if (fcState == HTTPStreamSource::FlowControlState::CLOSED) {
      writeCallbacks_.emplace_back(egressOffset_, callback);
    } else {
      // The only error for this is if !canWrite, which we already checked
      callback->writeSuccess();
    }
  }
}

void HTTPConnectAsyncTransport::shutdownRead() {
  if (readCallback_) {
    auto readCallback = readCallback_;
    readCallback_ = nullptr;
    readCallback->readEOF();
  }
  if (connectStream_->canRead()) {
    XLOG(DBG4) << "ingressSource_->stopReading from shutdownRead";
    connectStream_->ingressSource_->stopReading();
  }
}

void HTTPConnectAsyncTransport::shutdownWrite() {
  if (writable()) {
    XLOG(DBG4) << "egressSource_->eom from shutdownWrite";
    connectStream_->egressSource_->eom();
  }
}

void HTTPConnectAsyncTransport::shutdownWriteNow() {
  if (writeCallbacks_.empty()) {
    shutdownWrite();
  } else {
    errorWrites();
    // if there were pending writes, abort the stream
    connectStream_->egressSource_->abort(HTTPErrorCode::CANCEL);
  }
}

void HTTPConnectAsyncTransport::bytesProcessed(HTTPCodec::StreamID /*id*/,
                                               size_t amount,
                                               size_t /*toAck*/) {
  flushedOffset_ += amount;
  while (!writeCallbacks_.empty() &&
         flushedOffset_ >= writeCallbacks_.front().first) {
    auto callback = writeCallbacks_.front().second;
    writeCallbacks_.pop_front();
    callback->writeSuccess();
  }
}

void HTTPConnectAsyncTransport::errorWrites() {
  while (!writeCallbacks_.empty()) {
    auto callback = writeCallbacks_.front().second;
    writeCallbacks_.pop_front();
    folly::AsyncSocketException ex(
        folly::AsyncSocketException::AsyncSocketExceptionType::CANCELED,
        "Socket error");
    // TODO: could fill in the first arg if we track the size of the write
    callback->writeErr(0, ex);
  }
}

} // namespace proxygen::coro
