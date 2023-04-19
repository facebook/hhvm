/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <proxygen/lib/http/codec/compress/QPACKCodec.h>

#include <algorithm>
#include <folly/String.h>
#include <folly/ThreadLocal.h>
#include <folly/io/Cursor.h>
#include <iosfwd>
#include <proxygen/lib/http/HTTPMessage.h>
#include <proxygen/lib/http/HeaderConstants.h>
#include <proxygen/lib/http/codec/CodecUtil.h>
#include <proxygen/lib/http/codec/compress/HPACKCodec.h> // for prepareHeaders
#include <proxygen/lib/http/codec/compress/HPACKHeader.h>

using proxygen::compress::Header;
using std::vector;
namespace proxygen {

QPACKCodec::QPACKCodec()
    // by default dynamic tables are 0 size
    : encoder_(true, 0), decoder_(0, maxUncompressed_) {
}

void QPACKCodec::recordCompressedSize(const folly::IOBuf* stream,
                                      size_t controlSize) {
  encodedSize_.compressed = 0;
  encodedSize_.compressedBlock = 0;
  encodedSize_.compressed += controlSize;
  if (stream) {
    encodedSize_.compressedBlock = stream->computeChainDataLength();
    encodedSize_.compressed += encodedSize_.compressedBlock;
  }
  if (stats_) {
    stats_->recordEncode(Type::QPACK, encodedSize_);
  }
}

QPACKEncoder::EncodeResult QPACKCodec::encode(
    vector<Header>& headers,
    uint64_t streamId,
    uint32_t maxEncoderStreamBytes) noexcept {
  std::vector<HPACKHeader> prepared;
  encodedSize_.uncompressed = compress::prepareHeaders(headers, prepared);
  auto res = encoder_.encode(
      prepared, encodeHeadroom_, streamId, maxEncoderStreamBytes);
  size_t controlSize = res.control ? res.control->computeChainDataLength() : 0;
  recordCompressedSize(res.stream.get(), controlSize);
  return res;
}

std::unique_ptr<folly::IOBuf> QPACKCodec::encodeHTTP(
    folly::IOBufQueue& controlQueue,
    const HTTPMessage& msg,
    bool includeDate,
    uint64_t streamId,
    uint32_t maxEncoderStreamBytes,
    const folly::Optional<HTTPHeaders>& extraHeaders) noexcept {
  auto baseIndex = encoder_.startEncode(controlQueue, 0, maxEncoderStreamBytes);
  uint32_t requiredInsertCount = 0;
  auto prevSize = controlQueue.chainLength();

  auto uncompressed = 0;
  if (msg.isRequest()) {
    if (msg.isEgressWebsocketUpgrade()) {
      uncompressed +=
          encoder_.encodeHeaderQ(HPACKHeaderName(HTTP_HEADER_COLON_METHOD),
                                 methodToString(HTTPMethod::CONNECT),
                                 baseIndex,
                                 requiredInsertCount);
      uncompressed +=
          encoder_.encodeHeaderQ(HPACKHeaderName(HTTP_HEADER_COLON_PROTOCOL),
                                 headers::kWebsocketString,
                                 baseIndex,
                                 requiredInsertCount);
    } else if (msg.getUpgradeProtocol()) {
      uncompressed +=
          encoder_.encodeHeaderQ(HPACKHeaderName(HTTP_HEADER_COLON_METHOD),
                                 methodToString(HTTPMethod::CONNECT),
                                 baseIndex,
                                 requiredInsertCount);
      uncompressed +=
          encoder_.encodeHeaderQ(HPACKHeaderName(HTTP_HEADER_COLON_PROTOCOL),
                                 *msg.getUpgradeProtocol(),
                                 baseIndex,
                                 requiredInsertCount);
    } else {
      uncompressed +=
          encoder_.encodeHeaderQ(HPACKHeaderName(HTTP_HEADER_COLON_METHOD),
                                 msg.getMethodString(),
                                 baseIndex,
                                 requiredInsertCount);
    }

    if (msg.getMethod() != HTTPMethod::CONNECT ||
        msg.isEgressWebsocketUpgrade() || msg.getUpgradeProtocol()) {
      uncompressed +=
          encoder_.encodeHeaderQ(HPACKHeaderName(HTTP_HEADER_COLON_SCHEME),
                                 msg.getScheme(),
                                 baseIndex,
                                 requiredInsertCount);
      uncompressed +=
          encoder_.encodeHeaderQ(HPACKHeaderName(HTTP_HEADER_COLON_PATH),
                                 msg.getURL(),
                                 baseIndex,
                                 requiredInsertCount);
    }
    const HTTPHeaders& headers = msg.getHeaders();
    const std::string& host = headers.getSingleOrEmpty(HTTP_HEADER_HOST);
    uncompressed +=
        encoder_.encodeHeaderQ(HPACKHeaderName(HTTP_HEADER_COLON_AUTHORITY),
                               host,
                               baseIndex,
                               requiredInsertCount);
  } else {
    if (msg.isEgressWebsocketUpgrade()) {
      uncompressed +=
          encoder_.encodeHeaderQ(HPACKHeaderName(HTTP_HEADER_COLON_STATUS),
                                 headers::kStatus200,
                                 baseIndex,
                                 requiredInsertCount);
    } else {
      uncompressed += encoder_.encodeHeaderQ(
          HPACKHeaderName(HTTP_HEADER_COLON_STATUS),
          folly::to<folly::fbstring>(msg.getStatusCode()),
          baseIndex,
          requiredInsertCount);
    }
    // HEADERS frames do not include a version or reason string.
  }

  bool hasDateHeader = false;
  // Add the HTTP headers supplied by the caller, but skip
  // any per-hop headers that aren't supported in HTTP/2.
  auto headerEncodeHelper = [&](HTTPHeaderCode code,
                                folly::StringPiece name,
                                folly::StringPiece value) {
    if (CodecUtil::perHopHeaderCodes()[code] || name.empty() ||
        name[0] == ':') {
      DCHECK(!name.empty()) << "Empty header";
      DCHECK_NE(name[0], ':') << "Invalid header=" << name;
      return;
    }
    // Note this code will not drop headers named by Connection.  That's the
    // caller's job

    // see HTTP/2 spec, 8.1.2
    DCHECK(name != "TE" || value == "trailers");
    if ((!name.empty() && name[0] != ':') && code != HTTP_HEADER_HOST) {
      if (code == HTTP_HEADER_OTHER) {
        uncompressed += encoder_.encodeHeaderQ(
            HPACKHeaderName(name), value, baseIndex, requiredInsertCount);
      } else {
        uncompressed += encoder_.encodeHeaderQ(
            HPACKHeaderName(code), value, baseIndex, requiredInsertCount);
      }
    }
    hasDateHeader |= ((code == HTTP_HEADER_DATE) ? 1 : 0);
  };
  msg.getHeaders().forEachWithCode(headerEncodeHelper);
  if (extraHeaders) {
    extraHeaders->forEachWithCode(headerEncodeHelper);
  }

  if (includeDate && msg.isResponse() && !hasDateHeader) {
    uncompressed += encoder_.encodeHeaderQ(HPACKHeaderName(HTTP_HEADER_DATE),
                                           HTTPMessage::formatDateHeader(),
                                           baseIndex,
                                           requiredInsertCount);
  }

  auto result =
      encoder_.completeEncode(streamId, baseIndex, requiredInsertCount);
  encodedSize_.uncompressed = uncompressed;
  recordCompressedSize(result.get(), controlQueue.chainLength() - prevSize);
  return result;
}

void QPACKCodec::decodeStreaming(
    uint64_t streamID,
    std::unique_ptr<folly::IOBuf> block,
    uint32_t length,
    HPACK::StreamingCallback* streamingCb) noexcept {
  if (streamingCb) {
    streamingCb->stats = stats_;
  }
  decoder_.decodeStreaming(streamID, std::move(block), length, streamingCb);
}

void QPACKCodec::describe(std::ostream& stream) const {
  stream << "DecoderTable:\n" << decoder_;
  stream << "EncoderTable:\n" << encoder_;
}

std::ostream& operator<<(std::ostream& os, const QPACKCodec& codec) {
  codec.describe(os);
  return os;
}

} // namespace proxygen
