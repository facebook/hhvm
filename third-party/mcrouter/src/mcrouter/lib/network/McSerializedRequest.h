/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <memory>

#include "mcrouter/lib/mc/protocol.h"
#include "mcrouter/lib/network/AsciiSerialized.h"
#include "mcrouter/lib/network/CaretSerializedMessage.h"

namespace facebook {
namespace memcache {

struct CodecIdRange;

/**
 * A class for serializing memcache requests into iovs.
 */
class McSerializedRequest {
 public:
  enum class Result { OK, BAD_KEY, ERROR };

  /**
   * Creates serialized representation of request for a given mc_protocol.
   *
   * @param req               Request to serialize, caller is responsible to
   *                          keep it alive for the whole lifecycle of this
   *                          serialized request.
   * @param reqId             Id of the request.
   * @param protocol          Protocol to serialize the request.
   * @param supportedCodecs   Range of supported compression codecs.
   *                          Only used for caret.
   */
  template <class Request>
  McSerializedRequest(
      const Request& req,
      size_t reqId,
      mc_protocol_t protocol,
      const CodecIdRange& supportedCodecs);

  ~McSerializedRequest();

  McSerializedRequest(const McSerializedRequest&) = delete;
  McSerializedRequest& operator=(const McSerializedRequest&) = delete;

  Result serializationResult() const {
    return result_;
  }

  size_t getIovsCount() const {
    return iovsCount_;
  }

  const struct iovec* getIovs() const {
    return iovsBegin_;
  }

  uint32_t typeId() const {
    return typeId_;
  }

  size_t getBodySize();

 private:
  static const size_t kMaxIovs = 20;

  union {
    AsciiSerializedRequest asciiRequest_;
    CaretSerializedMessage caretRequest_;
  };

  const struct iovec* iovsBegin_{nullptr};
  size_t iovsCount_{0};
  mc_protocol_t protocol_{mc_unknown_protocol};
  Result result_{Result::OK};
  uint32_t typeId_{0};
};
} // namespace memcache
} // namespace facebook

#include "McSerializedRequest-inl.h"
