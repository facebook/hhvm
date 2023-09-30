/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <folly/Format.h>
#include <folly/io/Cursor.h>

#include "mcrouter/lib/carbon/Artillery.h"
#include "mcrouter/lib/fbi/cpp/LogFailure.h"
#include "mcrouter/lib/network/CarbonMessageList.h"

namespace facebook {
namespace memcache {

template <class Callback>
ClientMcParser<Callback>::ClientMcParser(
    Callback& cb,
    size_t minBufferSize,
    size_t maxBufferSize,
    const bool useJemallocNodumpAllocator,
    const CompressionCodecMap* compressionCodecMap,
    ConnectionFifo* debugFifo)
    : parser_(
          *this,
          minBufferSize,
          maxBufferSize,
          useJemallocNodumpAllocator,
          debugFifo),
      callback_(cb),
      debugFifo_(debugFifo),
      compressionCodecMap_(compressionCodecMap) {}

template <class Callback>
std::pair<void*, size_t> ClientMcParser<Callback>::getReadBuffer() {
  if (shouldReadToAsciiBuffer()) {
    return asciiParser_.getReadBuffer();
  } else {
    return parser_.getReadBuffer();
  }
}

template <class Callback>
bool ClientMcParser<Callback>::readDataAvailable(size_t len) {
  if (shouldReadToAsciiBuffer()) {
    if (FOLLY_UNLIKELY(debugFifo_ && debugFifo_->isConnected())) {
      auto buf = asciiParser_.getReadBuffer();
      debugFifo_->writeData(buf.first, len);
    }
    asciiParser_.readDataAvailable(len);
    return true;
  } else {
    return parser_.readDataAvailable(len);
  }
}

template <class Callback>
template <class Request>
typename std::enable_if<ListContains<McRequestList, Request>::value>::type
ClientMcParser<Callback>::expectNext() {
  if (parser_.protocol() == mc_ascii_protocol) {
    asciiParser_.initializeReplyParser<Request>();
    asciiReplyForwarder_ =
        &ClientMcParser<Callback>::forwardAsciiReply<Request>;
    if (FOLLY_UNLIKELY(debugFifo_ && debugFifo_->isConnected())) {
      debugFifo_->startMessage(
          MessageDirection::Received, ReplyT<Request>::typeId);
    }
  } else if (parser_.protocol() == mc_caret_protocol) {
    caretForwarder_ = &ClientMcParser<Callback>::forwardCaretReply<Request>;
  }
}

template <class Callback>
template <class Request>
typename std::enable_if<!ListContains<McRequestList, Request>::value>::type
ClientMcParser<Callback>::expectNext() {
  assert(parser_.protocol() == mc_caret_protocol);
  caretForwarder_ = &ClientMcParser<Callback>::forwardCaretReply<Request>;
}

template <class Callback>
template <class Request>
void ClientMcParser<Callback>::forwardAsciiReply() {
  auto reply = asciiParser_.getReply<ReplyT<Request>>();
  uint32_t replySize = carbon::valueRangeSlow(reply).size();
  callback_.replyReady(
      std::move(reply),
      0, /* reqId */
      RpcStatsContext(
          0 /* usedCodecId  */,
          replySize /* reply size before compression */,
          replySize /* reply size after compression */,
          ServerLoad::zero()));
  asciiReplyForwarder_ = nullptr;
}

template <class Callback>
template <class Request>
void ClientMcParser<Callback>::forwardCaretReply(
    const CaretMessageInfo& headerInfo,
    const folly::IOBuf& buffer,
    uint64_t reqId) {
  const folly::IOBuf* finalBuffer = &buffer;
  size_t offset = headerInfo.headerSize;

  // Uncompress if compressed
  std::unique_ptr<folly::IOBuf> uncompressedBuf;
  if (headerInfo.usedCodecId > 0) {
    uncompressedBuf = decompress(headerInfo, buffer);
    finalBuffer = uncompressedBuf.get();
    offset = 0;
  }

  ReplyT<Request> reply;
  folly::io::Cursor cur(finalBuffer);
  cur += offset;
  carbon::CarbonProtocolReader reader(cur);
  reply.deserialize(reader);
  reply.setTraceContext(
      carbon::tracing::deserializeTraceContext(headerInfo.traceId));

  callback_.replyReady(std::move(reply), reqId, getReplyStats(headerInfo));
}

template <class Callback>
std::unique_ptr<folly::IOBuf> ClientMcParser<Callback>::decompress(
    const CaretMessageInfo& headerInfo,
    const folly::IOBuf& buffer) {
  assert(!buffer.isChained());
  auto* codec = compressionCodecMap_
      ? compressionCodecMap_->get(headerInfo.usedCodecId)
      : nullptr;
  if (!codec) {
    throw std::runtime_error(folly::sformat(
        "Failed to get compression codec id {}. Reply is likely corrupted!",
        headerInfo.usedCodecId));
  }

  auto buf = buffer.data() + headerInfo.headerSize;
  return codec->uncompress(
      buf, headerInfo.bodySize, headerInfo.uncompressedBodySize);
}

template <class Callback>
bool ClientMcParser<Callback>::caretMessageReady(
    const CaretMessageInfo& headerInfo,
    const folly::IOBuf& buffer) {
  if (FOLLY_UNLIKELY(parser_.protocol() != mc_caret_protocol)) {
    const auto reason = folly::sformat(
        "Expected {} protocol, but received Caret!",
        mc_protocol_to_string(parser_.protocol()));
    callback_.parseError(carbon::Result::LOCAL_ERROR, reason);
    return false;
  }

  try {
    const size_t reqId = headerInfo.reqId;
    if (FOLLY_UNLIKELY(reqId == kCaretConnectionControlReqId)) {
      callback_.handleConnectionControlMessage(headerInfo);
    } else if (callback_.nextReplyAvailable(reqId)) {
      (this->*caretForwarder_)(headerInfo, buffer, reqId);
    }
    return true;
  } catch (const std::exception& e) {
    const auto reason =
        folly::sformat("Error parsing Caret message: {}", e.what());
    callback_.parseError(carbon::Result::LOCAL_ERROR, reason);
    return false;
  }
}

template <class Callback>
void ClientMcParser<Callback>::handleAscii(folly::IOBuf& readBuffer) {
  if (FOLLY_UNLIKELY(parser_.protocol() != mc_ascii_protocol)) {
    std::string reason(folly::sformat(
        "Expected {} protocol, but received ASCII!",
        mc_protocol_to_string(parser_.protocol())));
    callback_.parseError(carbon::Result::LOCAL_ERROR, reason);
    return;
  }

  while (readBuffer.length()) {
    if (asciiParser_.getCurrentState() == McAsciiParserBase::State::UNINIT) {
      // Ask the client to initialize parser.
      if (!callback_.nextReplyAvailable(0 /* reqId */)) {
        auto data = reinterpret_cast<const char*>(readBuffer.data());
        std::string reason(folly::sformat(
            "Received unexpected data from remote endpoint: '{}'!",
            folly::cEscape<std::string>(folly::StringPiece(
                data,
                data +
                    std::min(readBuffer.length(), static_cast<size_t>(128))))));
        callback_.parseError(carbon::Result::LOCAL_ERROR, reason);
        return;
      }
    }

    auto bufferBeforeConsume = readBuffer.data();
    auto result = asciiParser_.consume(readBuffer);
    if (FOLLY_UNLIKELY(debugFifo_ && debugFifo_->isConnected())) {
      auto len = readBuffer.data() - bufferBeforeConsume;
      debugFifo_->writeData(bufferBeforeConsume, len);
    }
    switch (result) {
      case McAsciiParserBase::State::COMPLETE:
        (this->*asciiReplyForwarder_)();
        break;
      case McAsciiParserBase::State::ERROR:
        callback_.parseError(
            carbon::Result::LOCAL_ERROR, asciiParser_.getErrorDescription());
        return;
      case McAsciiParserBase::State::PARTIAL:
        // Buffer was completely consumed.
        break;
      case McAsciiParserBase::State::UNINIT:
        // We fed parser some data, it shouldn't remain in State::NONE.
        callback_.parseError(
            carbon::Result::LOCAL_ERROR,
            "Sent data to AsciiParser but it remained in "
            "UNINIT state!");
        return;
    }
  }
}

template <class Callback>
void ClientMcParser<Callback>::parseError(
    carbon::Result result,
    folly::StringPiece reason) {
  callback_.parseError(result, reason);
}

template <class Callback>
bool ClientMcParser<Callback>::shouldReadToAsciiBuffer() const {
  return parser_.protocol() == mc_ascii_protocol &&
      asciiParser_.hasReadBuffer();
}

template <class Callback>
RpcStatsContext ClientMcParser<Callback>::getReplyStats(
    const CaretMessageInfo& headerInfo) const {
  RpcStatsContext rpcStatsContext;
  if (headerInfo.usedCodecId > 0) {
    // We need to remove compression additional fields to calculate the
    // real size of reply if it was not compressed at all.
    size_t compressionOverhead =
        2 + // varints of two compression additional field types
        (headerInfo.usedCodecId / 128 + 1) + // varint
        (headerInfo.uncompressedBodySize / 128 + 1); // varint
    rpcStatsContext.replySizeBeforeCompression = headerInfo.headerSize +
        headerInfo.uncompressedBodySize - compressionOverhead;
    rpcStatsContext.replySizeAfterCompression =
        headerInfo.headerSize + headerInfo.bodySize;
  } else {
    rpcStatsContext.replySizeBeforeCompression =
        headerInfo.headerSize + headerInfo.bodySize;
    rpcStatsContext.replySizeAfterCompression =
        rpcStatsContext.replySizeBeforeCompression;
  }
  rpcStatsContext.usedCodecId = headerInfo.usedCodecId;
  rpcStatsContext.serverLoad = headerInfo.serverLoad;
  return rpcStatsContext;
}
} // namespace memcache
} // namespace facebook
