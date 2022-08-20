/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <utility>

#include <folly/Range.h>
#include <folly/Varint.h>

#include "mcrouter/lib/Compression.h"
#include "mcrouter/lib/CompressionCodecManager.h"
#include "mcrouter/lib/carbon/CarbonQueueAppender.h"
#include "mcrouter/lib/network/ConnectionOptions.h"
#include "mcrouter/lib/network/ServerLoad.h"

namespace facebook {
namespace memcache {

struct CodecIdRange;
struct CaretMessageInfo;
class CompressionCodec;

/**
 * Class for serializing requests in the form of Carbon structs.
 */
class CaretSerializedMessage {
 public:
  CaretSerializedMessage() = default;

  CaretSerializedMessage(const CaretSerializedMessage&) = delete;
  CaretSerializedMessage& operator=(const CaretSerializedMessage&) = delete;
  CaretSerializedMessage(CaretSerializedMessage&&) noexcept = delete;
  CaretSerializedMessage& operator=(CaretSerializedMessage&&) = delete;

  void clear() {
    storage_.reset();
  }

  /**
   * Prepare requests for serialization for an Operation
   *
   * @param req               Request
   * @param reqId             Request id.
   * @param supportedCodecs   Range of supported compression codecs.
   * @param iovOut            Set to the beginning of array of ivecs that
   *                          reference serialized data.
   * @param niovOut           Number of valid iovecs referenced by iovOut.
   *
   * @return true iff message was successfully prepared.
   */
  template <class Request>
  bool prepare(
      const Request& req,
      size_t reqId,
      const CodecIdRange& supportedCodecs,
      const struct iovec*& iovOut,
      size_t& niovOut) noexcept;

  /**
   * Prepare replies for serialization
   *
   * @param reply                 TypedReply.
   * @param reqId                 Request id.
   * @param supportedCodecs       Range of supported codecs.
   * @param compressionCodecMap   Map of available codecs.
   * @param serverLoad            Represents load on the server.
   * @param iovOut                Will be set to the beginning of
   *                              array of iovecs
   * @param niovOut               Number of valid iovecs referenced by iovOut.
   *
   * @return true if message was successfully prepared.
   */
  template <class Reply>
  bool prepare(
      Reply&& reply,
      size_t reqId,
      const CodecIdRange& supportedCodecs,
      const CompressionCodecMap* compressionCodecMap,
      ServerLoad serverLoad,
      const struct iovec*& iovOut,
      size_t& niovOut) noexcept;

  /**
   * Returns the size of the message without the header.
   */
  size_t getSizeNoHeader() {
    return storage_.computeBodySize();
  }

  // Enable zero copy if an IOBuf exceeds this size threshold
  void setTCPZeroCopyThreshold(size_t threshold) {
    storage_.setTCPZeroCopyThreshold(threshold);
  }

  // Indicates if storage has been marked for zero copy
  bool shouldApplyZeroCopy() const {
    return storage_.shouldApplyZeroCopy();
  }

 private:
  carbon::CarbonQueueAppenderStorage storage_;

  template <class Request>
  bool fill(
      const Request& message,
      uint32_t reqId,
      size_t typeId,
      std::pair<uint64_t, uint64_t> traceId,
      const CodecIdRange& supportedCodecs,
      const struct iovec*& iovOut,
      size_t& niovOut);

  template <class Reply>
  bool fill(
      const Reply& message,
      uint32_t reqId,
      size_t typeId,
      std::pair<uint64_t, uint64_t> traceId,
      const CodecIdRange& supportedCodecs,
      const CompressionCodecMap* compressionCodecMap,
      ServerLoad serverLoad,
      const struct iovec*& iovOut,
      size_t& niovOut);

  void fillImpl(
      CaretMessageInfo& info,
      uint32_t reqId,
      size_t typeId,
      std::pair<uint64_t, uint64_t> traceId,
      ServerLoad serverLoad,
      const struct iovec*& iovOut,
      size_t& niovOut);

  /**
   * Compress body of message in storage_
   *
   * @param codec             Compression codec to use in compression.
   * @param uncompressedSize  Original (uncompressed) size of the body of the
   *                          message.
   * @return                  True if compression succeeds. Otherwise, false.
   */
  bool maybeCompress(CompressionCodec* codec, size_t uncompressedSize);
};

} // namespace memcache
} // namespace facebook

#include "CaretSerializedMessage-inl.h"
