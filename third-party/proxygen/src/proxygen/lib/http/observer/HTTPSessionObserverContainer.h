/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <folly/ObserverContainer.h>
#include <proxygen/lib/http/observer/HTTPSessionObserverInterface.h>

namespace proxygen {

using HTTPSessionObserverContainerBaseT = folly::ObserverContainer<
    HTTPSessionObserverInterface,
    HTTPSessionObserverAccessor,
    folly::ObserverContainerBasePolicyDefault<
        HTTPSessionObserverInterface::Events /* EventEnum */,
        32 /* BitsetSize (max number of interface events) */>>;

class HTTPSessionObserverContainer : public HTTPSessionObserverContainerBaseT {

  using HTTPSessionObserverContainerBaseT::HTTPSessionObserverContainerBaseT;

 public:
  ~HTTPSessionObserverContainer() override = default;
};

} // namespace proxygen
