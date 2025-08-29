/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "proxygen/lib/http/coro/transport/test/TestCoroTransport.h"
#include <folly/logging/xlog.h>

using TransportErrorCode = folly::coro::TransportIf::ErrorCode;

namespace {
const uint32_t kAckDelayMs = 50;
}

namespace proxygen::coro::test {

folly::coro::Task<size_t> TestCoroTransport::read(
    folly::MutableByteRange buf, std::chrono::milliseconds timeout) noexcept {
  const auto &cancelToken = co_await folly::coro::co_current_cancellation_token;
  folly::CancellationCallback cancellationCallback{
      cancelToken, [&]() { XLOG(DBG8) << "::read() cancelled"; }};

  while (true) {
    const bool hasError = state_->readError.has_value();
    const bool isCancelErr =
        hasError && *state_->readError == TransportErrorCode::CANCELED;
    // cancel without data read handled later
    if (hasError && !isCancelErr) {
      auto readError = *state_->readError;
      XLOG(DBG8) << "::read(); readError=" << readError;
      if (readError == TransportErrorCode::TIMED_OUT) {
        // Timeouts reset after being read out
        state_->readError = folly::none;
      }
      co_yield folly::coro::co_error(
          folly::AsyncSocketException(readError, ""));
    }

    // either no error or cancel; reset error
    state_->readError = folly::none;

    if (!state_->readEvents.empty()) {
      auto &readEvent = state_->readEvents.front();
      folly::io::Cursor c(readEvent.front());
      auto rc = c.pullAtMost(buf.start(), buf.end() - buf.start());
      readEvent.trimStart(rc);
      if (readEvent.empty()) {
        state_->readEvents.pop_front();
      }
      XLOG(DBG8) << "::read(); rc=" << rc;
      co_return rc;
    } else if (state_->readEOF) {
      XLOG(DBG8) << "::read(); state_->readEOF";
      co_return 0;
    } else if (isCancelErr) {
      XLOG(DBG8) << "::read(); readError=" << *state_->readError;
      co_yield folly::coro::co_error(folly::OperationCancelled{});
    } else {
      readEvent_.reset();
      auto status = co_await readEvent_.timedWait(evb_, timeout);
      if (status == TimedBaton::Status::timedout) {
        state_->readError = TransportErrorCode::TIMED_OUT;
      } else if (status == TimedBaton::Status::cancelled &&
                 !state_->readError) {
        state_->readError = TransportErrorCode::CANCELED;
      }
      XLOG(DBG8) << "::read(); readEvent_.wait() status=" << int(status);
    }
  }
  // unreachable
  co_yield folly::coro::co_error(
      folly::AsyncSocketException(TransportErrorCode::UNKNOWN, ""));
}

folly::coro::Task<size_t> TestCoroTransport::read(
    folly::IOBufQueue &readBuf,
    size_t minReadSize,
    size_t newAllocationSize,
    std::chrono::milliseconds timeout) noexcept {
  XLOG(DBG8) << __func__;
  static const size_t kMaxReadsPerEvent = 16;
  size_t numReads = 0;
  size_t totalRead = 0;
  do {
    auto rbuf = readBuf.preallocate(minReadSize, newAllocationSize);
    auto rc = co_await read(
        folly::MutableByteRange((uint8_t *)rbuf.first, rbuf.second), timeout);
    if (rc == 0) {
      break;
    }
    readBuf.postallocate(rc);
    totalRead += rc;
  } while (!state_->readEvents.empty() && ++numReads < kMaxReadsPerEvent);
  co_return totalRead;
}

folly::coro::Task<folly::Unit> TestCoroTransport::write(
    folly::ByteRange buf,
    std::chrono::milliseconds timeout,
    folly::WriteFlags writeFlags,
    WriteInfo *writeInfo) noexcept {
  // TODO: check for closed
  return write(folly::IOBuf::copyBuffer(buf), timeout, writeFlags, writeInfo);
}

folly::coro::Task<folly::Unit> TestCoroTransport::write(
    folly::IOBufQueue &ioBufQueue,
    std::chrono::milliseconds timeout,
    folly::WriteFlags writeFlags,
    WriteInfo *writeInfo) noexcept {
  auto head = ioBufQueue.move();
  auto clone = head->clone();
  ioBufQueue.append(std::move(head));
  return write(std::move(clone), timeout, writeFlags, writeInfo);
}

folly::coro::Task<folly::Unit> TestCoroTransport::write(
    std::unique_ptr<folly::IOBuf> buf,
    std::chrono::milliseconds timeout,
    folly::WriteFlags writeFlags,
    WriteInfo *writeInfo) {
  if (state_->writesClosed) {
    co_yield folly::coro::co_error(folly::OperationCancelled());
  }
  folly::IOBufQueue bufQueue{folly::IOBufQueue::cacheChainLength()};
  bufQueue.append(std::move(buf));
  auto length = bufQueue.chainLength();
  state_->writeOffset += length;
  state_->writeEvents.emplace_back(std::move(bufQueue));

  if (isSet(writeFlags, folly::WriteFlags::TIMESTAMP_WRITE)) {
    fireByteEvents(folly::AsyncSocketObserverInterface::ByteEvent::Type::WRITE,
                   state_->writeOffset);
  }
  while (writesPaused_) {
    writeEvent_.reset();
    auto status = co_await writeEvent_.timedWait(evb_, timeout);
    if (status == TimedBaton::Status::timedout) {
      co_yield folly::coro::co_error(folly::AsyncSocketException(
          TransportErrorCode::TIMED_OUT, "Timed out waiting for data"));
    } else if (status == TimedBaton::Status::cancelled) {
      co_yield folly::coro::co_error(
          folly::AsyncSocketException(TransportErrorCode::CANCELED, ""));
    }
  }
  if (isSet(writeFlags, folly::WriteFlags::TIMESTAMP_TX) &&
      byteEventsEnabled_) {
    if (fastTxAck_) {
      fireByteEvents(folly::AsyncSocketObserverInterface::ByteEvent::Type::TX,
                     state_->writeOffset);
    } else {
      evb_->runInLoop([this, offset = state_->writeOffset] {
        fireByteEvents(folly::AsyncSocketObserverInterface::ByteEvent::Type::TX,
                       offset);
      });
    }
  }
  if (isSet(writeFlags, folly::WriteFlags::TIMESTAMP_ACK) &&
      byteEventsEnabled_) {
    if (fastTxAck_) {
      fireByteEvents(folly::AsyncSocketObserverInterface::ByteEvent::Type::ACK,
                     state_->writeOffset);
    } else {
      evb_->runAfterDelay(
          [this, offset = state_->writeOffset] {
            fireByteEvents(
                folly::AsyncSocketObserverInterface::ByteEvent::Type::ACK,
                offset);
          },
          kAckDelayMs);
    }
  }
  if (writeInfo) {
    writeInfo->bytesWritten = length;
  }
  co_return folly::unit;
}

void TestCoroTransport::shutdownWrite() {
  XLOG(DBG8) << __func__;
  state_->writesClosed = true;
}

void TestCoroTransport::close() {
  XLOG(DBG8) << __func__;
  state_->writesClosed = true;
  if (!state_->closedWithReset) {
    state_->readEOF = true;
    readEvent_.signal();
  }
}

void TestCoroTransport::closeWithReset() {
  XLOG(DBG8) << __func__;
  state_->writesClosed = true;
  state_->readError = TransportErrorCode::NETWORK_ERROR;
  state_->closedWithReset = true;
  readEvent_.signal();
}

void TestCoroTransport::addReadEvent(std::unique_ptr<folly::IOBuf> ev,
                                     bool eof) {
  XLOG(DBG8) << __func__ << "; ev=" << ev.get() << "; eof=" << int(eof);
  folly::IOBufQueue bufQueue{folly::IOBufQueue::cacheChainLength()};
  if (ev) {
    bufQueue.append(std::move(ev));
    state_->readEvents.emplace_back(std::move(bufQueue));
  }
  if (eof) {
    state_->readEOF = true;
  }
  readEvent_.signal();
}

void TestCoroTransport::pauseWrites() {
  XLOG(DBG8) << __func__;
  writesPaused_ = true;
}

void TestCoroTransport::resumeWrites() {
  XLOG(DBG8) << __func__;
  writesPaused_ = false;
  writeEvent_.signal();
}

void TestCoroTransport::addReadError(TransportErrorCode err) {
  XLOG(DBG8) << __func__ << "; err=" << err;
  state_->readError = err;
  readEvent_.signal();
}

} // namespace proxygen::coro::test
