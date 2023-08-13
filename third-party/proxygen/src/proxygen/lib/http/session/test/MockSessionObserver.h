/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <folly/portability/GMock.h>
#include <proxygen/lib/http/observer/HTTPSessionObserverContainer.h>
#include <proxygen/lib/http/observer/HTTPSessionObserverInterface.h>

namespace proxygen {

class MockSessionObserver
    : public HTTPSessionObserverContainerBaseT::ManagedObserver {
 public:
  using HTTPSessionObserverContainerBaseT::ManagedObserver::ManagedObserver;
  MOCK_METHOD(void, attached, (HTTPSessionObserverAccessor*), (noexcept));
  MOCK_METHOD(void, detached, (HTTPSessionObserverAccessor*), (noexcept));
  MOCK_METHOD(void,
              destroyed,
              (HTTPSessionObserverAccessor*,
               typename HTTPSessionObserverContainer::ObserverContainer::
                   ManagedObserver::DestroyContext*),
              (noexcept));
  MOCK_METHOD(void,
              requestStarted,
              (HTTPSessionObserverAccessor*, const RequestStartedEvent&),
              (noexcept));
  MOCK_METHOD(void,
              preWrite,
              (HTTPSessionObserverAccessor*, const PreWriteEvent&),
              (noexcept));
  ~MockSessionObserver() override = default;
};
} // namespace proxygen
