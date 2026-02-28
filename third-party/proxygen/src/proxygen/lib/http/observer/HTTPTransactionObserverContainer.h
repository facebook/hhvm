/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <folly/ObserverContainer.h>
#include <proxygen/lib/http/observer/HTTPTransactionObserverInterface.h>

namespace proxygen {

class HTTPTransactionObserverAccessor;

using HTTPTransactionObserverContainerBaseT = folly::ObserverContainer<
    HTTPTransactionObserverInterface,
    HTTPTransactionObserverAccessor,
    folly::ObserverContainerBasePolicyDefault<
        HTTPTransactionObserverInterface::Events /* EventEnum */,
        32 /* BitsetSize (max number of interface events) */>>;

class HTTPTransactionObserverContainer
    : public HTTPTransactionObserverContainerBaseT {
 public:
  using HTTPTransactionObserverContainerBaseT::
      HTTPTransactionObserverContainerBaseT;
  ~HTTPTransactionObserverContainer() override = default;
};

/**
 * Accessor object observed by HTTPTransactionObserver(s).
 */
class HTTPTransactionObserverAccessor {
 public:
  virtual ~HTTPTransactionObserverAccessor() = default;

  virtual bool addObserver(
      HTTPTransactionObserverContainer::Observer* observer) = 0;

  virtual bool addObserver(
      std::shared_ptr<HTTPTransactionObserverContainer::Observer> observer) = 0;

  [[nodiscard]] virtual uint64_t getTxnId() const = 0;
};

} // namespace proxygen
