/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <glog/logging.h>
#include <proxygen/lib/http/observer/HTTPTransactionObserverInterface.h>
#include <utility>

namespace proxygen {

HTTPTransactionObserverInterface::TxnBytesEvent::Builder&&
HTTPTransactionObserverInterface::TxnBytesEvent::Builder::setTimestamp(
    const proxygen::TimePoint& timestampIn) {
  maybeTimestampRef = timestampIn;
  return std::move(*this);
}

HTTPTransactionObserverInterface::TxnBytesEvent::Builder&&
HTTPTransactionObserverInterface::TxnBytesEvent::Builder::setType(Type typeIn) {
  type = typeIn;
  return std::move(*this);
}

HTTPTransactionObserverInterface::TxnBytesEvent::Builder&&
HTTPTransactionObserverInterface::TxnBytesEvent::Builder::setNumBytes(
    const size_t numBytesIn) {
  numBytes = numBytesIn;
  return std::move(*this);
}

HTTPTransactionObserverInterface::TxnBytesEvent::Builder&&
HTTPTransactionObserverInterface::TxnBytesEvent::Builder::setHeaders(
    const HTTPMessage& headersIn) {
  maybeHeadersRef = headersIn;
  return std::move(*this);
}

HTTPTransactionObserverInterface::TxnBytesEvent
HTTPTransactionObserverInterface::TxnBytesEvent::Builder::build() && {
  return TxnBytesEvent(*this);
}

HTTPTransactionObserverInterface::TxnBytesEvent::TxnBytesEvent(
    const TxnBytesEvent::BuilderFields& builderFields)
    : timestamp(*CHECK_NOTNULL(builderFields.maybeTimestampRef.get_pointer())),
      type(builderFields.type),
      maybeNumBytes(builderFields.numBytes),
      maybeHeadersRef(builderFields.maybeHeadersRef) {
}

} // namespace proxygen
