/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "mcrouter/lib/carbon/Artillery.h"
#include "mcrouter/lib/network/CarbonMessageDispatcher.h"
#include "mcrouter/lib/network/CaretProtocol.h"

namespace facebook {
namespace memcache {

namespace detail {

template <class Request>
std::pair<uint64_t, uint64_t> getRequestTraceId(const Request& req) {
  return carbon::tracing::serializeTraceContext(req.traceContext());
}

template <class Reply>
std::pair<uint64_t, uint64_t> getReplyTraceId(const Reply& reply) {
  return carbon::tracing::serializeTraceContext(reply.traceContext());
}

} // namespace detail

template <class Request>
bool CaretSerializedMessage::prepare(
    const Request& req,
    size_t reqId,
    const CodecIdRange& supportedCodecs,
    const struct iovec*& iovOut,
    size_t& niovOut) noexcept {
  return fill(
      req,
      reqId,
      Request::typeId,
      detail::getRequestTraceId(req),
      supportedCodecs,
      iovOut,
      niovOut);
}

template <class Reply>
bool CaretSerializedMessage::prepare(
    Reply&& reply,
    size_t reqId,
    const CodecIdRange& supportedCodecs,
    const CompressionCodecMap* compressionCodecMap,
    ServerLoad serverLoad,
    const struct iovec*& iovOut,
    size_t& niovOut) noexcept {
  return fill(
      reply,
      reqId,
      Reply::typeId,
      detail::getReplyTraceId(reply),
      supportedCodecs,
      compressionCodecMap,
      serverLoad,
      iovOut,
      niovOut);
}

template <class Request>
bool CaretSerializedMessage::fill(
    const Request& message,
    uint32_t reqId,
    size_t typeId,
    std::pair<uint64_t, uint64_t> traceId,
    const CodecIdRange& supportedCodecs,
    const struct iovec*& iovOut,
    size_t& niovOut) {
  // Serialize body into storage_. Note we must defer serialization of header.
  try {
    serializeCarbonRequest(message, storage_);
  } catch (const std::exception& e) {
    LOG(ERROR) << "Failed to serialize: " << e.what();
    return false;
  }

  CaretMessageInfo info;
  if (!supportedCodecs.isEmpty()) {
    info.supportedCodecsFirstId = supportedCodecs.firstId;
    info.supportedCodecsSize = supportedCodecs.size;
  }
  fillImpl(info, reqId, typeId, traceId, ServerLoad::zero(), iovOut, niovOut);
  return true;
}

template <class Reply>
bool CaretSerializedMessage::fill(
    const Reply& message,
    uint32_t reqId,
    size_t typeId,
    std::pair<uint64_t, uint64_t> traceId,
    const CodecIdRange& supportedCodecs,
    const CompressionCodecMap* compressionCodecMap,
    ServerLoad serverLoad,
    const struct iovec*& iovOut,
    size_t& niovOut) {
  // Serialize and (maybe) compress body of message.
  try {
    serializeCarbonStruct(message, storage_);
  } catch (const std::exception& e) {
    LOG(ERROR) << "Failed to serialize: " << e.what();
    return false;
  }

  CaretMessageInfo info;

  // Maybe compress.
  auto uncompressedSize = storage_.computeBodySize();
  auto codec = (compressionCodecMap == nullptr)
      ? nullptr
      : compressionCodecMap->getBest(supportedCodecs, uncompressedSize, typeId);

  if (maybeCompress(codec, uncompressedSize)) {
    info.usedCodecId = codec->id();
    info.uncompressedBodySize = uncompressedSize;
  }

  fillImpl(info, reqId, typeId, traceId, serverLoad, iovOut, niovOut);
  return true;
}

inline bool CaretSerializedMessage::maybeCompress(
    CompressionCodec* codec,
    size_t uncompressedSize) {
  if (!codec) {
    return false;
  }

  if (FOLLY_UNLIKELY(uncompressedSize > std::numeric_limits<uint32_t>::max()) ||
      uncompressedSize < codec->filteringOptions().minCompressionThreshold) {
    return false;
  }

  static constexpr size_t kCompressionOverhead = 4;
  try {
    const auto iovs = storage_.getIovecs();
    auto compressedBuf = codec->compress(iovs.first, iovs.second);
    auto compressedSize = compressedBuf->computeChainDataLength();
    if ((compressedSize + kCompressionOverhead) < uncompressedSize) {
      storage_.reset();
      storage_.append(*compressedBuf);
      return true;
    }
  } catch (const std::exception& e) {
    LOG(ERROR) << "Error compressing reply: " << e.what();
  }

  return false;
}

inline void CaretSerializedMessage::fillImpl(
    CaretMessageInfo& info,
    uint32_t reqId,
    size_t typeId,
    std::pair<uint64_t, uint64_t> traceId,
    ServerLoad serverLoad,
    const struct iovec*& iovOut,
    size_t& niovOut) {
  info.bodySize = storage_.computeBodySize();
  info.typeId = typeId;
  info.reqId = reqId;
  info.traceId = traceId;
  info.serverLoad = serverLoad;

  size_t headerSize = caretPrepareHeader(
      info, reinterpret_cast<char*>(storage_.getHeaderBuf()));
  storage_.reportHeaderSize(headerSize);

  const auto iovs = storage_.getIovecs();
  iovOut = iovs.first;
  niovOut = iovs.second;
}

} // namespace memcache
} // namespace facebook
