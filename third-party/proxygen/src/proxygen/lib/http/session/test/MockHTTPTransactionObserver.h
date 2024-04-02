/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <folly/portability/GMock.h>
#include <proxygen/lib/http/observer/HTTPTransactionObserverContainer.h>
#include <proxygen/lib/http/observer/HTTPTransactionObserverInterface.h>

namespace proxygen {

class MockHTTPTransactionObserver
    : public HTTPTransactionObserverContainerBaseT::ManagedObserver {
 public:
  using HTTPTransactionObserverContainerBaseT::ManagedObserver::ManagedObserver;

  ~MockHTTPTransactionObserver() override = default;
  MOCK_METHOD(void, attached, (HTTPTransactionObserverAccessor*), (noexcept));
  MOCK_METHOD(void, detached, (HTTPTransactionObserverAccessor*), (noexcept));
  MOCK_METHOD(void,
              destroyed,
              (HTTPTransactionObserverAccessor*,
               typename HTTPTransactionObserverContainer::ObserverContainer::
                   ManagedObserver::DestroyContext*),
              (noexcept));
  MOCK_METHOD(void,
              onBytesEvent,
              (HTTPTransactionObserverAccessor*, const TxnBytesEvent&),
              (noexcept));
};

} // namespace proxygen
