/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <folly/io/async/AsyncServerSocket.h>
#include <memory>
#include <proxygen/lib/services/AcceptorConfiguration.h>
#include <proxygen/lib/utils/AsyncTimeoutSet.h>
#include <proxygen/lib/utils/WheelTimerInstance.h>
#include <wangle/acceptor/Acceptor.h>

namespace proxygen {

class HTTPAcceptor : public wangle::Acceptor {
 public:
  explicit HTTPAcceptor(std::shared_ptr<const AcceptorConfiguration> accConfig)
      : Acceptor(std::move(accConfig)) {
  }

  void init(folly::AsyncServerSocket* serverSocket,
            folly::EventBase* eventBase,
            wangle::SSLStats* /*stat*/ = nullptr,
            std::shared_ptr<const fizz::server::FizzServerContext> fizzCtx =
                nullptr) override {
    timer_ = createTransactionTimeoutSet(eventBase);
    Acceptor::init(serverSocket, eventBase, nullptr, fizzCtx);
  }

  [[nodiscard]] std::shared_ptr<const AcceptorConfiguration> getConfig() const {
    return std::static_pointer_cast<const AcceptorConfiguration>(
        Acceptor::getConfig());
  }

  /**
   * Access the general-purpose timeout manager for transactions.
   */
  const WheelTimerInstance& getTransactionTimeoutSet() {
    return *timer_;
  }

 protected:
  std::unique_ptr<WheelTimerInstance> timer_;

  std::unique_ptr<WheelTimerInstance> createTransactionTimeoutSet(
      folly::EventBase* eventBase) {
    return std::make_unique<WheelTimerInstance>(
        getConfig()->transactionIdleTimeout, eventBase);
  }
};

} // namespace proxygen
