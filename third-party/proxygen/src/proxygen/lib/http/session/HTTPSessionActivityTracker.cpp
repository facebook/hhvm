/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <proxygen/lib/http/session/ByteEventTracker.h>
#include <proxygen/lib/http/session/HTTPSessionActivityTracker.h>
#include <proxygen/lib/http/session/HTTPTransaction.h>

namespace proxygen {

HTTPSessionActivityTracker::HTTPSessionActivityTracker(
    wangle::ManagedConnection* managedConnection,
    size_t ingressThreshold,
    size_t egressThreshold)
    : managedConnection_(managedConnection),
      ingressThreshold_(ingressThreshold),
      egressThreshold_(egressThreshold) {
}

void HTTPSessionActivityTracker::reportActivity() {
  if (managedConnection_->getConnectionManager()) {
    managedConnection_->getConnectionManager()->reportActivity(
        *managedConnection_);
  }
}

bool HTTPSessionActivityTracker::onIngressBody(size_t bytes) {
  ingressSize_ += bytes;
  if (ingressSize_ >= ingressThreshold_) {
    ingressSize_ = ingressSize_ % ingressThreshold_;
    reportActivity();
    return true;
  }
  return false;
}

// Note that the provided offset could be either session offset, by HTTPSession,
// or stream offset, by HQSession. To support both cases, we keep track
// internally of the session byte Offset, sessionBodyOffset_, and use to
// calculate the required byte event tracker offset.
void HTTPSessionActivityTracker::addTrackedEgressByteEvent(
    const size_t offset,
    const size_t bodyLen,
    ByteEventTracker* byteEventTracker,
    HTTPTransaction* txn) {
  SCOPE_EXIT {
    sessionBodyOffset_ += bodyLen;
  };
  if (!byteEventTracker || !txn ||
      sessionBodyOffset_ / egressThreshold_ ==
          (sessionBodyOffset_ + bodyLen) / egressThreshold_) {
    return;
  }
  auto lastTrackedEgressByteEvent =
      (sessionBodyOffset_ / egressThreshold_) * egressThreshold_;
  auto reportActivityCb = [this](ByteEvent&) { reportActivity(); };
  // Note that a single we might need to add multiple tracking events
  while (sessionBodyOffset_ + bodyLen >=
         lastTrackedEgressByteEvent + egressThreshold_) {
    lastTrackedEgressByteEvent += egressThreshold_;
    auto trackOffset = offset + lastTrackedEgressByteEvent - sessionBodyOffset_;
    byteEventTracker->addTrackedByteEvent(txn, trackOffset, reportActivityCb);
  }
}

} // namespace proxygen
