/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <fizz/server/AsyncFizzServer.h>
#include <folly/io/async/SSLContext.h>

namespace facebook {
namespace memcache {

class McFizzServer : public fizz::server::AsyncFizzServer {
 public:
  using UniquePtr =
      std::unique_ptr<McFizzServer, folly::DelayedDestruction::Destructor>;

  McFizzServer(
      folly::AsyncTransportWrapper::UniquePtr socket,
      const std::shared_ptr<fizz::server::FizzServerContext>& fizzContext,
      std::shared_ptr<folly::SSLContext> fallbackContext)
      : fizz::server::AsyncFizzServer(std::move(socket), fizzContext),
        fallbackContext_(std::move(fallbackContext)) {}

  ~McFizzServer() = default;

  const std::shared_ptr<folly::SSLContext>& getFallbackContext() {
    return fallbackContext_;
  }

 private:
  std::shared_ptr<folly::SSLContext> fallbackContext_;
};

} // namespace memcache
} // namespace facebook
