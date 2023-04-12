/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "proxygen/lib/http/observer/HTTPSessionObserverInterface.h"

namespace proxygen {

HTTPSessionObserverInterface::RequestStartedEvent::Builder&&
HTTPSessionObserverInterface::RequestStartedEvent::Builder::setHeaders(
    const proxygen::HTTPHeaders& headersIn) {
  maybeHTTPHeadersRef = headersIn;
  return std::move(*this);
}

HTTPSessionObserverInterface::RequestStartedEvent
HTTPSessionObserverInterface::RequestStartedEvent::Builder::build() && {
  return RequestStartedEvent(*this);
}

HTTPSessionObserverInterface::RequestStartedEvent::RequestStartedEvent(
    const RequestStartedEvent::BuilderFields& builderFields)
    : requestHeaders(
          *CHECK_NOTNULL(builderFields.maybeHTTPHeadersRef.get_pointer())) {
}

} // namespace proxygen
