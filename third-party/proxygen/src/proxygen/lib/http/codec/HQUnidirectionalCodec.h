/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <cstddef>
#include <folly/io/IOBuf.h>
#include <folly/io/IOBufQueue.h>
#include <proxygen/lib/http/HTTPException.h>
#include <proxygen/lib/http/codec/HTTPCodec.h>

namespace proxygen { namespace hq {

enum class UnidirectionalStreamType : uint64_t {
  CONTROL = 0x00,
  PUSH = 0x01,
  QPACK_ENCODER = 0x02,
  QPACK_DECODER = 0x03,
  // any possible varint encoding for this does not conflict
  // with any character that is allowed as the first character in HTTP/1.1
  // 0x20 (' '), 0x4020 ('@'), 0x80000020, 0xC000000000000020
  H1Q_CONTROL = 0x20,
};
using StreamTypeType = std::underlying_type<UnidirectionalStreamType>::type;
std::ostream& operator<<(std::ostream& os, UnidirectionalStreamType type);

// Safe application onto the type.
template <typename Ret>
using UnidirectionalTypeFunctor =
    std::function<folly::Optional<Ret>(UnidirectionalStreamType)>;

using UnidirectionalTypeF = UnidirectionalTypeFunctor<UnidirectionalStreamType>;

template <typename Ret>
folly::Optional<Ret> withType(uint64_t typeval,
                              UnidirectionalTypeFunctor<Ret> functor) {
  auto casted = static_cast<UnidirectionalStreamType>(typeval);

  switch (casted) {
    case UnidirectionalStreamType::CONTROL:
    case UnidirectionalStreamType::PUSH:
    case UnidirectionalStreamType::QPACK_ENCODER:
    case UnidirectionalStreamType::QPACK_DECODER:
    case UnidirectionalStreamType::H1Q_CONTROL:
      return functor(casted);
    default:
      return folly::none;
  }
}

enum class StreamDirection : uint8_t { INGRESS, EGRESS };
std::ostream& operator<<(std::ostream& os, StreamDirection direction);

class HQUnidirectionalCodec {

 public:
  class Callback {
   public:
    virtual ~Callback() {
    }
    virtual void onError(HTTPCodec::StreamID streamID,
                         const HTTPException& error,
                         bool newTxn) = 0;
  };

  HQUnidirectionalCodec(UnidirectionalStreamType streamType,
                        StreamDirection streamDir)
      : streamType_(streamType), streamDir_(streamDir) {
  }

  /**
   * Parse ingress data.
   * @param  buf   An IOBuf chain to parse
   * @return any unparsed data
   */
  virtual std::unique_ptr<folly::IOBuf> onUnidirectionalIngress(
      std::unique_ptr<folly::IOBuf> ingress) = 0;

  /**
   * Finish parsing when the ingress stream has ended.
   */
  virtual void onUnidirectionalIngressEOF() = 0;

  virtual ~HQUnidirectionalCodec() {
  }

  StreamDirection getStreamDirection() const {
    return streamDir_;
  }

  UnidirectionalStreamType getStreamType() const {
    return streamType_;
  }

  bool isIngress() const {
    return streamDir_ == StreamDirection::INGRESS;
  }
  bool isEgress() const {
    return streamDir_ == StreamDirection::EGRESS;
  }

 private:
  UnidirectionalStreamType streamType_;
  StreamDirection streamDir_;
};

}} // namespace proxygen::hq

namespace std {
template <>
struct hash<proxygen::hq::UnidirectionalStreamType> {
  uint64_t operator()(
      const proxygen::hq::UnidirectionalStreamType& type) const {
    return static_cast<uint64_t>(type);
  }
};
} // namespace std
