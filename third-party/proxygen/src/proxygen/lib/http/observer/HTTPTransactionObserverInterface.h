/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

namespace proxygen {

class HTTPTransactionObserverAccessor;

/**
 * Observer of HTTP transaction events.
 */
class HTTPTransactionObserverInterface {
 public:
  enum class Events {};

  virtual ~HTTPTransactionObserverInterface() = default;
};

} // namespace proxygen
