/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "proxygen/lib/http/coro/transport/HTTPConnectTransport.h"
#include <folly/logging/xlog.h>

using folly::AsyncSocketException;
using AsyncSocketExceptionType =
    folly::AsyncSocketException::AsyncSocketExceptionType;
using folly::coro::co_error;

namespace proxygen::coro {

HTTPConnectTransport::HTTPConnectTransport(
    std::unique_ptr<HTTPConnectStream> connectStream)
    : connectStream_(std::move(connectStream)),
      flowControlWindowOpen_(connectStream_->eventBase_,
                             std::chrono::milliseconds(0)) {
  if (connectStream_->egressBufferSize_ > 0) {
    flowControlWindowOpen_.signal();
  }
  connectStream_->setHTTPStreamSourceCallback(this);
}

HTTPConnectTransport::~HTTPConnectTransport() {
  flowControlWindowOpen_.signal(TimedBaton::Status::cancelled);
  *deleted_ = true;
}

void HTTPConnectTransport::scheduleAsyncRead(
    uint32_t size, const folly::CancellationToken& ct) {
  // indefinite read timeout on ingressSource; we install a custom timeout for
  // yielding AsyncSocketException in ::read()
  auto ingressSource = connectStream_->ingressSource_;
  auto* evb = connectStream_->eventBase_;
  ingressSource->setReadTimeout(std::chrono::milliseconds(0));
  co_withExecutor(
      evb,
      folly::coro::co_withCancellation(ct, ingressSource->readBodyEvent(size)))
      .startInlineUnsafe(
          [this, ingressSource, deleted = deleted_](
              folly::Try<HTTPBodyEvent> maybeBodyEvent) {
            XLOG(DBG6) << "async readBodyEvent done";
            if (*deleted) {
              return;
            }
            // wrap HTTPError in AsyncSocketException
            if (maybeBodyEvent.hasException()) {
              auto httpErr = getHTTPError(maybeBodyEvent);
              maybeBodyEvent.emplaceException(AsyncSocketException(
                  AsyncSocketException::INTERNAL_ERROR, httpErr.describe()));
            }
            XCHECK(bodyEvents_.try_enqueue(std::move(maybeBodyEvent)));
            pendingRead_ = false;
          },
          ct);
}

folly::coro::Task<size_t> HTTPConnectTransport::read(
    folly::MutableByteRange buf, std::chrono::milliseconds timeout) {
  co_await folly::coro::co_safe_point;
  const bool hasMoreData = connectStream_->canRead() || !bodyEvents_.empty();
  if (!hasMoreData || buf.size() == 0) {
    co_return 0;
  }

  /**
   * HTTPConnectTransport::read effectively reads from an ingress source (of
   * concrete type HTTPStreamSource) owned by the client<->proxy
   * HTTPCoroSession.
   *
   * A transport read timeout is a recoverable error; HTTPCoroSession uses this
   * signal to initiate a drain and wait for more data from the peer. However a
   * read timeout in HTTPStreamSource::readBodyEvent is an unrecoverable error,
   * where it drops any currently queued and future ingress events.
   *
   * To avoid this mismatch in semantics, we set an infinite read timeout on the
   * HTTPStreamSource ingress source and schedule our own. On timeout, we
   * enqueue and yield a timeout exception, however the ingress source will
   * still be readable
   */
  class ReadTimeout : public folly::HHWheelTimer::Callback {
   public:
    explicit ReadTimeout(HTTPConnectTransport& transportRef)
        : transportRef_(transportRef) {
    }
    void timeoutExpired() noexcept override {
      // only enqueue timeout ex if we haven't resolved a bodyEvent yet
      if (transportRef_.bodyEvents_.empty()) {
        auto timeoutEx = folly::Try<HTTPBodyEvent>(
            AsyncSocketException(AsyncSocketException::TIMED_OUT,
                                 "Timed out waiting for body event"));
        XCHECK(transportRef_.bodyEvents_.try_enqueue(std::move(timeoutEx)));
      }
    }

   private:
    HTTPConnectTransport& transportRef_;
  } readTimeout{*this};

  size_t nRead = 0;
  bool eom = false;
  auto* evb = connectStream_->eventBase_;
  const auto& ct = co_await folly::coro::co_current_cancellation_token;
  do {
    // schedule at most one outstanding ::readBodyEvent on ingress source
    bool wasScheduled = std::exchange(pendingRead_, true);
    if (!wasScheduled && connectStream_->canRead()) {
      scheduleAsyncRead(buf.size(), ct);
    }
    if (timeout.count() > 0) {
      evb->timer().scheduleTimeout(&readTimeout, timeout);
    }
    // UnboundedQueue::dequeue will only yield an exception upon cancellation,
    // passthru cancellation exception to caller
    auto bodyEvent = co_await bodyEvents_.dequeue();
    readTimeout.cancelTimeout(); // no-op if not scheduled (i.e. timeout == 0ms)

    if (bodyEvent.hasException()) {
      co_yield co_error(bodyEvent.exception());
    }
    eom = bodyEvent->eom;
    if (bodyEvent->eventType == HTTPBodyEvent::BODY &&
        !bodyEvent->event.body.empty()) {
      folly::io::Cursor c(bodyEvent->event.body.front());
      nRead = c.pullAtMost(buf.start(), buf.size());
    }
  } while (nRead == 0 && !eom);
  co_return nRead;
}

folly::coro::Task<size_t> HTTPConnectTransport::read(
    folly::IOBufQueue& buf,
    size_t minReadSize,
    size_t newAllocationSize,
    std::chrono::milliseconds timeout) {
  co_await folly::coro::co_safe_point;
  const bool hasMoreData = connectStream_->canRead() || !bodyEvents_.empty();
  if (!hasMoreData) {
    co_return 0;
  }
  auto rbuf = buf.preallocate(minReadSize, newAllocationSize);
  auto rc = co_await read(
      folly::MutableByteRange((uint8_t*)rbuf.first, rbuf.second), timeout);
  buf.postallocate(rc);
  co_return rc;
}

folly::coro::Task<folly::Unit> HTTPConnectTransport::write(
    folly::ByteRange buf,
    std::chrono::milliseconds timeout,
    folly::WriteFlags writeFlags,
    WriteInfo* writeInfo) {
  folly::IOBufQueue writeBuf{folly::IOBufQueue::cacheChainLength()};
  writeBuf.append(buf.start(), buf.size());
  co_await write(writeBuf, timeout, writeFlags, writeInfo);
  co_return folly::Unit();
}

folly::coro::Task<folly::Unit> HTTPConnectTransport::write(
    folly::IOBufQueue& ioBufQueue,
    std::chrono::milliseconds timeout,
    folly::WriteFlags /*writeFlags*/,
    WriteInfo* writeInfo) {
  co_await folly::coro::co_safe_point;
  writeInProgress_ = true;
  auto deleted = deleted_;
  SCOPE_EXIT {
    if (!*deleted) {
      writeInProgress_ = false;
    }
  };
  auto buf = ioBufQueue.front()->clone();
  auto size = ioBufQueue.chainLength();
  if (!connectStream_->canWrite()) {
    if (connectStream_->egressError_) {
      co_yield co_error(
          AsyncSocketException(AsyncSocketException::NOT_OPEN,
                               connectStream_->egressError_->describe()));
    } else {
      co_yield co_error(AsyncSocketException(AsyncSocketException::NOT_OPEN,
                                             "write after close"));
    }
  }
  if (timeout.count() > 0) {
    flowControlWindowOpen_.setTimeout(timeout);
  }
  auto res = co_await flowControlWindowOpen_.wait();
  // Could get fancy with setting a timer that's cleared in bytesProcessed
  if (res == TimedBaton::Status::timedout) {
    co_yield co_error(AsyncSocketException(AsyncSocketException::TIMED_OUT,
                                           "Write timed out"));
  } else if (res == TimedBaton::Status::cancelled) {
    co_yield folly::coro::co_cancelled;
  }
  if (!connectStream_->egressSource_) {
    co_yield co_error(AsyncSocketException(
        AsyncSocketException::NOT_OPEN, "HTTPConnectTransport: egress closed"));
  }
  auto flowState =
      connectStream_->egressSource_->body(std::move(buf), 0, pendingEOM_);
  if (flowState != HTTPStreamSource::FlowControlState::OPEN) {
    XLOG(DBG4) << "Blocking writes";
    flowControlWindowOpen_.reset();
  }
  if (writeInfo) {
    writeInfo->bytesWritten = size;
  }
  co_return folly::Unit();
}

void HTTPConnectTransport::windowOpen(HTTPCodec::StreamID /*id*/) {
  XLOG(DBG4) << "Resuming writes";
  flowControlWindowOpen_.signal();
}

void HTTPConnectTransport::close() {
  shutdownWrite();
  connectStream_->shutdownRead();
}

void HTTPConnectTransport::shutdownWrite() {
  if (connectStream_->canWrite()) {
    if (flowControlWindowOpen_.getStatus() == TimedBaton::Status::notReady &&
        writeInProgress_) {
      pendingEOM_ = true;
    } else {
      connectStream_->egressSource_->eom();
    }
  } // maybe close called twice, or close called after error/abort?
}

void HTTPConnectTransport::closeWithReset() {
  if (connectStream_->egressSource_) {
    connectStream_->egressSource_->abort(HTTPErrorCode::CANCEL);
  }
  connectStream_->close();
}

} // namespace proxygen::coro
