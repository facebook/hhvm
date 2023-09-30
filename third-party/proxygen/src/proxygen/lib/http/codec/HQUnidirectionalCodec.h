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
#include <proxygen/lib/http/codec/HQFramer.h>
#include <proxygen/lib/http/codec/HTTPCodec.h>

namespace proxygen { namespace hq {

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
    case UnidirectionalStreamType::WEBTRANSPORT:
      return functor(casted);
    default:
      if (isGreaseId(typeval)) {
        return functor(UnidirectionalStreamType::GREASE);
      }
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
