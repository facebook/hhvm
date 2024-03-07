/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <proxygen/lib/http/session/HTTPEvent.h>

#include <ostream>

namespace proxygen {

std::ostream& operator<<(std::ostream& os, HTTPEvent::Type e) {
  switch (e) {
    case HTTPEvent::Type::MESSAGE_BEGIN:
      os << "message_begin";
      break;
    case HTTPEvent::Type::HEADERS_COMPLETE:
      os << "headers_complete";
      break;
    case HTTPEvent::Type::BODY:
      os << "body";
      break;
    case HTTPEvent::Type::CHUNK_HEADER:
      os << "chunk_header";
      break;
    case HTTPEvent::Type::CHUNK_COMPLETE:
      os << "chunk_complete";
      break;
    case HTTPEvent::Type::TRAILERS_COMPLETE:
      os << "trailers_complete";
      break;
    case HTTPEvent::Type::MESSAGE_COMPLETE:
      os << "message_complete";
      break;
    case HTTPEvent::Type::UPGRADE:
      os << "uprade";
      break;
    case HTTPEvent::Type::ERROR:
      os << "error";
      break;
  }

  return os;
}

} // namespace proxygen
