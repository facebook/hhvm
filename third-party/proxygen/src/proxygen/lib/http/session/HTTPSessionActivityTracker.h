/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <wangle/acceptor/ConnectionManager.h>

namespace proxygen {

class ByteEventTracker;
class HTTPTransaction;

/*
 * HTTPSessionActivityTracker tracks activities, reads/writes, on active
 * sessions. The primary usage for this is to support Slowloris mitigation by
 * closing active sessions with low activity when the host is under memory
 * constraints. HTTPSessionActivityTracker is hooked to a session and tracks
 * significant activities. Once significant activity is detected, the connection
 * manager is notified of the event. Significant activities could be in the form
 * of read or write above threshold amount of bytes.
 */
class HTTPSessionActivityTracker {
 public:
  HTTPSessionActivityTracker(wangle::ManagedConnection* managedConnection,
                             size_t ingressThreshold,
                             size_t egressThreshold);

  virtual void reportActivity();

  bool onIngressBody(size_t bytes);

  void addTrackedEgressByteEvent(const size_t offset,
                                 const size_t bodyLen,
                                 ByteEventTracker* byteEventTracker,
                                 HTTPTransaction* txn);
  virtual ~HTTPSessionActivityTracker() = default;

 private:
  wangle::ManagedConnection* managedConnection_;
  size_t ingressSize_{0};
  size_t sessionBodyOffset_{0};
  const size_t ingressThreshold_;
  const size_t egressThreshold_;
};
} // namespace proxygen
