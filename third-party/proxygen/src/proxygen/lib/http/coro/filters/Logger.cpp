/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "proxygen/lib/http/coro/filters/Logger.h"
#include <proxygen/lib/http/HTTPPriorityFunctions.h>

namespace proxygen::coro {

std::ostream& operator<<(std::ostream& os, const Logger::Filter& filter) {
  filter.describe(os);
  return os;
}
// clang-format off
void Logger::Filter::describe(std::ostream& os) const {
  if (valid) {
    os
      << "\tHTTP Version:\t"
      << folly::to<std::string>(httpVersion.first, ".", httpVersion.second)
      << "\n\t\t"
      << "Priority:\t"
      << httpPriorityToString(priority)
      << "\n";
    if (direction_ == Direction::REQUEST) {
      os
        << "\t\tMethod:\t\t" <<  method << "\n\t\t"
        << "Host:\t\t" <<  host << "\n\t\t"
        << "Path:\t\t" << url << "\n";
    } else {
      os << "\t\tStatus Code:\t"
         << (statusCode ?
             folly::to<std::string>(*statusCode) :
             std::string("None"))<< "\n";
    }
    if (headerSize) {
      os << "\t\tHeader Bytes:\t" << headerSize->uncompressed << "\n\t\t"
         << "Compressed:\t" << headerSize->compressed << "\n";
    }
    os << "\t\tBody Bytes:\t" << bodyBytes << "\n";
  }
  if (error) {
    os << "\tError:\t code=" << uint32_t(error->code) << " msg="
       << error->msg << "\n";
  }
}

void Logger::logWithVlog(int level) const {
  VLOG(level)
    << "HTTP request logger: \n\t"
    << "Local Addr:\t" << localAddr << "\n\t"
    << "Peer Addr:\t" << peerAddr << "\n\t"
    << "Secure:\t\t" << getSecurityType() << "\n\t"
    << "Protocol:\t" << getCodecProtocolString(protocol) << "\n\t"
    << "Session ID:\t" << std::hex << sessionID << "\n\t"
    << "Stream ID:\t" << std::dec << getStreamID() << "\n\t"
    << "Seq No:\t\t" << HTTPCodec::streamIDToSeqNo(protocol, getStreamID()) << "\n\t"
    << "Start:\t\t" << toTimeT(startTime) << "\n\t"
    << "TTFHB:\t\t" << timeToFirstHeaderByte().count() << "\n\t"
    << "TTFB:\t\t" << timeToFirstByte().count() << "\n\t"
    << "TTLB:\t\t" << timeToLastByte().count() << "\n\t"
    << "RTT (us):\t" << transportInfo.rtt.count() << "\n\t"
    << "Request:\n\t" << reqFilter << "\n\t"
    << "Response:\n\t" << respFilter;
}
// clang-format on

folly::coro::Task<HTTPHeaderEvent> Logger::Filter::readHeaderEvent() {
  streamID = getStreamID();
  auto headerEvent = co_await co_awaitTry(readHeaderEventImpl());
  if (headerEvent.hasException()) {
    endTime = std::chrono::steady_clock::now();
    error = getHTTPError(headerEvent);
    done.first.setValue();
    co_yield folly::coro::co_error(std::move(headerEvent.exception()));
  }
  if (headerEvent->isFinal()) {
    finalHeaderTime = std::chrono::steady_clock::now();
  }
  valid = true;
  auto& headers = *headerEvent->headers;
  httpVersion = headers.getHTTPVersion();
  if (headers.isRequest()) {
    method = headers.getMethodString();
    host = headers.getHeaders().getSingleOrEmpty(HTTP_HEADER_HOST);
    url = headers.getURL();
  } else {
    statusCode = headers.getStatusCode();
  }
  auto size = headers.getIngressHeaderSize();
  ingress = size.uncompressed > 0;
  if (ingress) {
    headerSize = size;
  } else {
    headerEvent->egressHeadersFn =
        [this, egressHeadersFn = std::move(headerEvent->egressHeadersFn)](
            HTTPHeaderSize size) mutable noexcept {
          headerSize = size;
          if (egressHeadersFn) {
            egressHeadersFn(size);
          }
        };
  }
  priority = headers.getHTTPPriority().value_or(HTTPPriority());
  if (headerEvent->eom) {
    firstByteTime = std::chrono::steady_clock::now();
    endTime = std::chrono::steady_clock::now();
    done.first.setValue();
  }
  co_return std::move(*headerEvent);
}

folly::coro::Task<HTTPBodyEvent> Logger::Filter::readBodyEvent(uint32_t max) {
  auto bodyEvent = co_await co_awaitTry(readBodyEventImpl(max));
  if (bodyEvent.hasException()) {
    endTime = std::chrono::steady_clock::now();
    error = getHTTPError(bodyEvent);
    done.first.setValue();
    co_yield folly::coro::co_error(std::move(bodyEvent.exception()));
  }
  if (bodyEvent->eventType == HTTPBodyEvent::BODY) {
    bodyBytes += bodyEvent->event.body.chainLength();
  }
  if (!firstByteTime) {
    firstByteTime = std::chrono::steady_clock::now();
  }
  if (bodyEvent->eom) {
    endTime = std::chrono::steady_clock::now();
    done.first.setValue();
  }
  co_return std::move(*bodyEvent);
}

void Logger::Filter::stopReading(folly::Optional<const HTTPErrorCode> err) {
  endTime = std::chrono::steady_clock::now();
  done.first.setValue();
  HTTPSourceFilter::stopReading(err);
}

} // namespace proxygen::coro
/**
 * fbcode Rev
 * tw task
 * window updates sent
 * window updates received
 * header queue time (requires byte events)
 * body queue time (requires byte events)
 */
