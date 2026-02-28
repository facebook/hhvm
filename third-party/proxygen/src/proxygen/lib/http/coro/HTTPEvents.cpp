/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "proxygen/lib/http/coro/HTTPEvents.h"
#include "proxygen/lib/http/coro/HTTPSource.h"

namespace proxygen::coro {

HTTPPushEvent::~HTTPPushEvent() {
  if (pushSource_) {
    pushSource_->stopReading(folly::none);
    pushSource_ = nullptr;
  }
}

void HTTPHeaderEvent::describe(std::ostream& os) const {
  os << "HTTPHeaderEvent, final=" << ((isFinal()) ? "true" : "false")
     << ", eom=" << (eom ? "true" : "false");
  os << *headers;
}

void HTTPPushEvent::describe(std::ostream& os) const {
  os << *promise;
}

void HTTPBodyEvent::describe(std::ostream& os) const {
  os << "HTTPBodyEvent: eom=" << (eom ? "true" : "false") << ", ";
  switch (eventType) {
    case BODY:
      os << "type=BODY, len=" << event.body.chainLength();
      break;
    case DATAGRAM:
      os << "type=DATAGRAM, len=" << event.datagram->computeChainDataLength();
      break;
    case PUSH_PROMISE:
      os << "type=PUSH_PROMISE, promise: " << event.push;
      break;
    case TRAILERS:
      os << "type=TRAILERS, trailers: ";
      event.trailers->forEach(
          [&os](const std::string& name, const std::string& value) {
            os << " " << stripCntrlChars(name) << ": " << stripCntrlChars(value)
               << std::endl;
          });
      break;
    case SUSPEND:
      os << "type=SUSPEND";
      break;
    default:
      // no-op
      break;
  }
}

std::ostream& operator<<(std::ostream& os, const HTTPHeaderEvent& event) {
  event.describe(os);
  return os;
}

std::ostream& operator<<(std::ostream& os, const HTTPPushEvent& event) {
  event.describe(os);
  return os;
}

std::ostream& operator<<(std::ostream& os, const HTTPBodyEvent& event) {
  event.describe(os);
  return os;
}

} // namespace proxygen::coro
