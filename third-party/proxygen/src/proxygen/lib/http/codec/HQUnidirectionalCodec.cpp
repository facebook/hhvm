/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <proxygen/lib/http/codec/HQUnidirectionalCodec.h>

#include <folly/io/Cursor.h>
#include <folly/io/IOBufQueue.h>

namespace proxygen { namespace hq {

std::ostream& operator<<(std::ostream& os, UnidirectionalStreamType type) {
  switch (type) {
    case UnidirectionalStreamType::H1Q_CONTROL:
    case UnidirectionalStreamType::CONTROL:
      os << "control";
      break;
    case UnidirectionalStreamType::QPACK_ENCODER:
      os << "QPACK encoder";
      break;
    case UnidirectionalStreamType::QPACK_DECODER:
      os << "QPACK decoder";
      break;
    case UnidirectionalStreamType::PUSH:
      os << "push";
      break;
    default:
      os << "unknown";
      break;
  }
  return os;
}

std::ostream& operator<<(std::ostream& os, StreamDirection direction) {
  switch (direction) {
    case StreamDirection::INGRESS:
      os << "ingress";
      break;
    case StreamDirection::EGRESS:
      os << "egress";
      break;
    default:
      os << "unknown";
      break;
  }
  return os;
}

}} // namespace proxygen::hq
