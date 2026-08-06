/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <memory>
#include <proxygen/lib/transport/qmux/QmuxTransport.h>

namespace folly::coro {
class TransportIf;
}

namespace proxygen::qmux {

class FollyQmuxTransport : public QmuxTransport {
 public:
  explicit FollyQmuxTransport(
      std::unique_ptr<folly::coro::TransportIf> transport);
  ~FollyQmuxTransport() override;

  folly::coro::Task<size_t> read(folly::IOBufQueue& readBuf,
                                 size_t minReadSize,
                                 size_t newAllocationSize,
                                 std::chrono::milliseconds timeout) override;

  folly::coro::Task<folly::Unit> write(
      folly::IOBufQueue& writeBuf, std::chrono::milliseconds timeout) override;

  void shutdownWrite() override;

  [[nodiscard]] folly::SocketAddress getLocalAddress() const noexcept override;
  [[nodiscard]] folly::SocketAddress getPeerAddress() const noexcept override;
  [[nodiscard]] folly::AsyncTransport* getUnderlyingTransport()
      const noexcept override;

 private:
  std::unique_ptr<folly::coro::TransportIf> transport_;
};

} // namespace proxygen::qmux
