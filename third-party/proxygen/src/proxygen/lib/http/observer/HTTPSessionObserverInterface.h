/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <utility>

namespace proxygen {

/**
 * Accessor object "observed" by HTTPSessionObservers.
 *
 * Instead of passing SessionObservers a pointer to HTTPSession or
 * HTTPSessionBase, we pass a pointer to this SessionAccessor object which
 * exposes functionality that can be used by the observers to get information
 * from the HTTP session and/or use public functions to manipulate the HTTP
 * session.
 *
 * This layer of indirection clearly defines the interface and expectations that
 * observers can expect from the HTTP session, which in turn enables the same
 * observer implementation to be used with different session implementations,
 * including future implementations which may not inherit from HTTPSessionBase.
 *
 * The lifetime of the HTTPSessionAccessor is tied to that of the underlying
 * HTTP session: when the session is destroyed, the accessor is destroyed as
 * well.
 */
class HTTPSessionObserverAccessor {
 public:
  virtual ~HTTPSessionObserverAccessor() = default;
};

/**
 * Observer of Http session events.
 */
class HTTPSessionObserverInterface {
 public:
  enum class Events {};

  virtual ~HTTPSessionObserverInterface() = default;
};

} // namespace proxygen
