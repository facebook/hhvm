/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "proxygen/lib/http/coro/util/CancellableBaton.h"
#include <folly/Random.h>
#include <folly/io/Cursor.h>
#include <folly/io/coro/Transport.h>
#include <proxygen/lib/transport/test/MockAsyncTransportCertificate.h>

namespace proxygen::coro::test {

class TestCoroTransport : public folly::coro::TransportIf {
 public:
  struct State {
    std::list<folly::IOBufQueue> readEvents;
    folly::Optional<folly::coro::TransportIf::ErrorCode> readError;
    bool readEOF{false};
    bool writesClosed{false};
    bool closedWithReset{false};
    std::list<folly::IOBufQueue> writeEvents;
    size_t writeOffset{0};
  };

  TestCoroTransport(folly::EventBase *evb, TestCoroTransport::State *state)
      : evb_(evb), state_(state) {
    localPort_ = folly::Random::rand32();
    peerPort_ = folly::Random::rand32();
  }

  folly::SocketAddress getLocalAddress() const noexcept override {
    return folly::SocketAddress("::1", localPort_);
  }

  folly::SocketAddress getPeerAddress() const noexcept override {
    return folly::SocketAddress("::1", peerPort_);
  }

  folly::coro::Task<size_t> read(
      folly::MutableByteRange buf,
      std::chrono::milliseconds timeout =
          std::chrono::milliseconds(0)) noexcept override;

  folly::coro::Task<size_t> read(
      folly::IOBufQueue &readBuf,
      size_t minReadSize,
      size_t newAllocationSize,
      std::chrono::milliseconds timeout =
          std::chrono::milliseconds(0)) noexcept override;

  folly::coro::Task<folly::Unit> write(
      folly::ByteRange buf,
      std::chrono::milliseconds timeout = std::chrono::milliseconds(0),
      folly::WriteFlags writeFlags = folly::WriteFlags::NONE,
      WriteInfo *writeInfo = nullptr) noexcept override;

  folly::coro::Task<folly::Unit> write(
      folly::IOBufQueue &ioBufQueue,
      std::chrono::milliseconds timeout = std::chrono::milliseconds(0),
      folly::WriteFlags writeFlags = folly::WriteFlags::NONE,
      WriteInfo *writeInfo = nullptr) noexcept override;

  folly::coro::Task<folly::Unit> write(
      std::unique_ptr<folly::IOBuf> buf,
      std::chrono::milliseconds timeout = std::chrono::milliseconds(0),
      folly::WriteFlags writeFlags = folly::WriteFlags::NONE,
      WriteInfo *writeInfo = nullptr);

  void shutdownWrite() override;

  void close() override;

  void closeWithReset() override;

  folly::EventBase *getEventBase() noexcept override {
    return evb_;
  }

  folly::AsyncTransport *getTransport() const override {
    return nullptr;
  }

  const folly::AsyncTransportCertificate *getPeerCertificate() const override {
    return &mockCertificate;
  }

  void addReadEvent(std::unique_ptr<folly::IOBuf> ev, bool eof);

  void pauseWrites();

  void resumeWrites();

  void addReadError(folly::coro::TransportIf::ErrorCode err);

  bool writesPaused_{false};
  folly::EventBase *evb_;
  detail::CancellableBaton readEvent_;
  detail::CancellableBaton writeEvent_;
  State *state_;
  uint16_t localPort_;
  uint16_t peerPort_;
  MockAsyncTransportCertificate mockCertificate;
};

} // namespace proxygen::coro::test
