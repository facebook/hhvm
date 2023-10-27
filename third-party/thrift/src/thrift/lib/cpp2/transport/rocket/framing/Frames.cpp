/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <thrift/lib/cpp2/transport/rocket/framing/Frames.h>

#include <chrono>
#include <type_traits>
#include <utility>

#include <glog/logging.h>

#include <fmt/core.h>
#include <folly/CPortability.h>
#include <folly/Likely.h>
#include <folly/Range.h>
#include <folly/Utility.h>
#include <folly/functional/Invoke.h>
#include <folly/io/Cursor.h>
#include <folly/io/IOBuf.h>

#include <thrift/lib/cpp2/transport/rocket/Types.h>
#include <thrift/lib/cpp2/transport/rocket/framing/FrameType.h>
#include <thrift/lib/cpp2/transport/rocket/framing/Serializer.h>
#include <thrift/lib/cpp2/transport/rocket/framing/Util.h>

namespace apache {
namespace thrift {
namespace rocket {

namespace {
std::unique_ptr<folly::IOBuf> trimBuffer(
    std::unique_ptr<folly::IOBuf> buffer, size_t toTrim) {
  folly::IOBufQueue bufQueue;
  bufQueue.append(std::move(buffer));
  bufQueue.trimStart(toTrim);
  return bufQueue.move();
}

Payload readPayload(
    bool expectingMetadata,
    folly::io::Cursor& cursor,
    std::unique_ptr<folly::IOBuf> buffer) {
  size_t metadataSize = 0;
  if (expectingMetadata) {
    metadataSize = readFrameOrMetadataSize(cursor);
  }
  buffer = trimBuffer(std::move(buffer), cursor.getCurrentPosition());
  return Payload::makeCombined(std::move(buffer), metadataSize);
}

template <class Frame>
void serializeInFragmentsSlowCommon(
    Frame&& frame, Flags flags, Serializer& writer) {
  auto metadataSize = frame.payload().metadataSize();
  folly::IOBufQueue bufferQueue(folly::IOBufQueue::cacheChainLength());
  bufferQueue.append(std::move(frame.payload()).buffer());

  bool isFirstFrame = true;
  const bool complete = flags.complete();
  bool follows;

  do {
    size_t metadataChunk = std::min(metadataSize, kMaxFragmentedPayloadSize);
    metadataSize -= metadataChunk;
    auto chunk = bufferQueue.splitAtMost(kMaxFragmentedPayloadSize);
    follows = !bufferQueue.empty();

    auto p = Payload::makeCombined(std::move(chunk), metadataChunk);
    if (std::exchange(isFirstFrame, false)) {
      frame.payload() = std::move(p);
      frame.setHasFollows(follows);
      std::move(frame).serialize(writer);
    } else {
      PayloadFrame pf(
          frame.streamId(),
          std::move(p),
          flags.follows(follows).complete(!follows && complete));
      std::move(pf).serialize(writer);
    }
  } while (follows);
}

FOLLY_CREATE_MEMBER_INVOKER(has_initial_request_n_invoker, initialRequestN);

template <typename T>
using has_initial_request_n =
    folly::is_invocable<has_initial_request_n_invoker, T>;

template <class Frame>
std::enable_if_t<has_initial_request_n<Frame>::value, void>
serializeInitialNIfPresent(const Frame& frame, HeaderSerializer& serializer) {
  serializer.writeBE<uint32_t>(frame.initialRequestN());
}

template <class Frame>
std::enable_if_t<!has_initial_request_n<Frame>::value, void>
serializeInitialNIfPresent(const Frame&, HeaderSerializer&) {}

template <class Frame>
std::unique_ptr<folly::IOBuf> serializeIntoIOBuf(Frame&& frame) {
  Serializer writer;
  std::move(frame).serialize(writer);
  return std::move(writer).move();
}

template <class Frame>
std::unique_ptr<folly::IOBuf> serializeIntoHeadroom(Frame&& frame) {
  auto flags = frame.flags();
  // We can assume here that we have non-zero buffer with adequate
  // headroom for the rsocket header.
  auto dataLen = frame.payload().dataSize();
  auto metadataLen = frame.payload().metadataSize();
  auto buffer = std::move(frame.payload()).buffer();

  constexpr size_t rsocketLen =
      (2 * Serializer::kBytesForFrameOrMetadataLength) +
      Frame::frameHeaderSize();
  DCHECK(buffer->headroom() >= rsocketLen);

  // Write rsocket header directly into the buffer headroom.
  HeaderSerializer writer(buffer->writableData() - rsocketLen, rsocketLen);
  const size_t frameSize = frame.frameHeaderSize() + dataLen + metadataLen +
      Serializer::kBytesForFrameOrMetadataLength;
  writer.writeFrameOrMetadataSize(frameSize);
  writer.write(frame.streamId());
  writer.writeFrameTypeAndFlags(frame.frameType(), flags);
  serializeInitialNIfPresent(frame, writer);
  writer.writeFrameOrMetadataSize(metadataLen);
  DCHECK_EQ(writer.result().size(), rsocketLen);
  buffer->prepend(rsocketLen);

  return buffer;
}

template <class Frame>
std::unique_ptr<folly::IOBuf> serializeIntoHeadroomIfPossible(Frame&& frame) {
  constexpr size_t kHeadroomSize =
      Frame::frameHeaderSize() + 2 * Serializer::kBytesForFrameOrMetadataLength;
  if (LIKELY(
          !frame.payload().buffer()->isSharedOne() &&
          frame.payload().metadataAndDataSize() <= kMaxFragmentedPayloadSize &&
          frame.payload().hasNonemptyMetadata() &&
          frame.payload().buffer()->headroom() >= kHeadroomSize)) {
    return serializeIntoHeadroom(std::move(frame));
  } else {
    return serializeIntoIOBuf(std::move(frame));
  }
}
} // namespace

std::unique_ptr<folly::IOBuf> SetupFrame::serialize() && {
  return serializeIntoIOBuf(std::move(*this));
}

void SetupFrame::serialize(Serializer& writer) && {
  /**
   *  0                   1                   2                   3
   *  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
   * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   * |                         Stream ID = 0                         |
   * +-----------+-+-+-+-+-----------+-------------------------------+
   * |Frame Type |0|M|R|L|  Flags    |
   * +-----------+-+-+-+-+-----------+-------------------------------+
   * |         Major Version         |        Minor Version          |
   * +-------------------------------+-------------------------------+
   * |0|                 Time Between KEEPALIVE Frames               |
   * +---------------------------------------------------------------+
   * |0|                       Max Lifetime                          |
   * +---------------------------------------------------------------+
   * |         Token Length          | Resume Identification Token  ...
   * +---------------+-----------------------------------------------+
   * |  MIME Length  |   Metadata Encoding MIME Type                ...
   * +---------------+-----------------------------------------------+
   * |  MIME Length  |     Data Encoding MIME Type                  ...
   * +---------------+-----------------------------------------------+
   *                       Metadata & Setup Payload
   */

  // Excludes room for frame length
  const auto frameSize = frameHeaderSize() + payload().serializedSize();
  auto nwritten = writer.writeFrameOrMetadataSize(frameSize);

  // SETUP frames are only sent when connections are first established, so we
  // forgo potential batching optimizations.
  nwritten += writer.write(StreamId{0});

  // The metadata flag can get out of sync with whether the payload actually has
  // metadata. In order to work around this and any other potential
  // inconsistencies, we recompute all flags during serialization as opposed to
  // serializing flags_ directly. This comment applies to all frames.
  nwritten += writer.writeFrameTypeAndFlags(
      frameType(),
      Flags()
          .metadata(payload_.hasNonemptyMetadata())
          .resumeToken(hasResumeIdentificationToken()));

  // Major and minor version. Our rsocket implementation only handles rsocket
  // protocol version 1.0.
  nwritten += writer.writeBE<uint16_t>(1); // rsocket major version
  nwritten += writer.writeBE<uint16_t>(0); // rsocket minor version

  // Time between KEEPALVE frames. Note that KEEPALIVE frames are unsupported.
  // Note: keepalive time MUST be > 0 and at most 2^31 - 1.
  constexpr std::chrono::milliseconds kMaxKeepaliveTime{(1ull << 31) - 1};
  nwritten += writer.writeBE<uint32_t>(kMaxKeepaliveTime.count());

  // Max lifetime. Protocol specifies that max lifetime MUST be > 0, but not
  // used in this implementation.
  constexpr std::chrono::milliseconds kMaxLifetime{(1ull << 31) - 1};
  nwritten += writer.writeBE<uint32_t>(kMaxLifetime.count());

  // Resume identification token length and token are not present if 'R' flag is
  // not present.
  if (hasResumeIdentificationToken()) {
    // Resume identification token length and token
    nwritten += writer.writeBE<uint16_t>(resumeIdentificationToken_.size());
    nwritten += writer.write(resumeIdentificationToken_);
  }

  const auto& metadataMimeType =
      rocketMimeTypes_ ? kRocketMetadataMimeType : kLegacyMimeType;
  const auto& payloadMimeType =
      rocketMimeTypes_ ? kRocketPayloadMimeType : kLegacyMimeType;

  // Length of metadata MIME type
  nwritten +=
      writer.writeBE<uint8_t>(static_cast<uint8_t>(metadataMimeType.size()));
  nwritten += writer.write(metadataMimeType);

  // Length of data MIME type
  nwritten +=
      writer.writeBE<uint8_t>(static_cast<uint8_t>(payloadMimeType.size()));
  nwritten += writer.write(payloadMimeType);

  // Setup metadata and data
  nwritten += writer.writePayload(std::move(payload()));

  DCHECK_EQ(Serializer::kBytesForFrameOrMetadataLength + frameSize, nwritten);
}

std::unique_ptr<folly::IOBuf> RequestResponseFrame::serialize() && {
  return serializeIntoHeadroomIfPossible(std::move(*this));
}

void RequestResponseFrame::serialize(Serializer& writer) && {
  if (UNLIKELY(payload().metadataAndDataSize() > kMaxFragmentedPayloadSize)) {
    return std::move(*this).serializeInFragmentsSlow(writer);
  }
  std::move(*this).serializeIntoSingleFrame(writer);
}

void RequestResponseFrame::serializeIntoSingleFrame(Serializer& writer) && {
  /**
   *  0                   1                   2                   3
   *  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
   * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   * |                           Stream ID                           |
   * +-----------+-+-+-+-------------+-------------------------------+
   * |Frame Type |0|M|F|     Flags   |
   * +-------------------------------+
   *                      Metadata & Request Data
   */

  // Excludes room for frame length
  const auto frameSize = frameHeaderSize() + payload().serializedSize();
  auto nwritten = writer.writeFrameOrMetadataSize(frameSize);

  nwritten += writer.write(streamId());
  nwritten += writer.writeFrameTypeAndFlags(
      frameType(),
      Flags().metadata(payload_.hasNonemptyMetadata()).follows(hasFollows()));
  nwritten += writer.writePayload(std::move(payload()));

  DCHECK_EQ(Serializer::kBytesForFrameOrMetadataLength + frameSize, nwritten);
}

std::unique_ptr<folly::IOBuf> RequestFnfFrame::serialize() && {
  return serializeIntoHeadroomIfPossible(std::move(*this));
}

void RequestFnfFrame::serialize(Serializer& writer) && {
  if (UNLIKELY(payload().metadataAndDataSize() > kMaxFragmentedPayloadSize)) {
    return std::move(*this).serializeInFragmentsSlow(writer);
  }
  std::move(*this).serializeIntoSingleFrame(writer);
}

void RequestFnfFrame::serializeIntoSingleFrame(Serializer& writer) && {
  /**
   *  0                   1                   2                   3
   *  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
   * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   * |                           Stream ID                           |
   * +-----------+-+-+-+-------------+-------------------------------+
   * |Frame Type |0|M|F|    Flags    |
   * +-------------------------------+
   *                      Metadata & Request Data
   */

  // Excludes room for frame length
  const auto frameSize = frameHeaderSize() + payload().serializedSize();
  auto nwritten = writer.writeFrameOrMetadataSize(frameSize);

  nwritten += writer.write(streamId());
  nwritten += writer.writeFrameTypeAndFlags(
      frameType(),
      Flags().metadata(payload_.hasNonemptyMetadata()).follows(hasFollows()));
  nwritten += writer.writePayload(std::move(payload()));

  DCHECK_EQ(Serializer::kBytesForFrameOrMetadataLength + frameSize, nwritten);
}

std::unique_ptr<folly::IOBuf> RequestStreamFrame::serialize() && {
  return serializeIntoHeadroomIfPossible(std::move(*this));
}

void RequestStreamFrame::serialize(Serializer& writer) && {
  if (UNLIKELY(payload().metadataAndDataSize() > kMaxFragmentedPayloadSize)) {
    return std::move(*this).serializeInFragmentsSlow(writer);
  }
  std::move(*this).serializeIntoSingleFrame(writer);
}

void RequestStreamFrame::serializeIntoSingleFrame(Serializer& writer) && {
  /**
   *  0                   1                   2                   3
   *  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
   * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   * |                           Stream ID                           |
   * +-----------+-+-+-+-------------+-------------------------------+
   * |Frame Type |0|M|F|    Flags    |
   * +-------------------------------+-------------------------------+
   * |0|                    Initial Request N                        |
   * +---------------------------------------------------------------+
   *                       Metadata & Request Data
   */

  // Excludes room for frame length
  const auto frameSize = frameHeaderSize() + payload().serializedSize();
  auto nwritten = writer.writeFrameOrMetadataSize(frameSize);

  nwritten += writer.write(streamId());
  nwritten += writer.writeFrameTypeAndFlags(
      frameType(),
      Flags().metadata(payload_.hasNonemptyMetadata()).follows(hasFollows()));
  nwritten += writer.writeBE<uint32_t>(initialRequestN());
  nwritten += writer.writePayload(std::move(payload()));

  DCHECK_EQ(Serializer::kBytesForFrameOrMetadataLength + frameSize, nwritten);
}

std::unique_ptr<folly::IOBuf> RequestChannelFrame::serialize() && {
  return serializeIntoHeadroomIfPossible(std::move(*this));
}

void RequestChannelFrame::serialize(Serializer& writer) && {
  if (UNLIKELY(payload().metadataAndDataSize() > kMaxFragmentedPayloadSize)) {
    return std::move(*this).serializeInFragmentsSlow(writer);
  }
  std::move(*this).serializeIntoSingleFrame(writer);
}

void RequestChannelFrame::serializeIntoSingleFrame(Serializer& writer) && {
  /**
   *  0                   1                   2                   3
   *  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
   * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   * |                           Stream ID                           |
   * +-----------+-+-+-+-+-----------+-------------------------------+
   * |Frame Type |0|M|F|C|  Flags    |
   * +-------------------------------+-------------------------------+
   * |0|                    Initial Request N                        |
   * +---------------------------------------------------------------+
   *                        Metadata & Request Data
   */
  // Excludes room for frame length
  const auto frameSize = frameHeaderSize() + payload().serializedSize();
  auto nwritten = writer.writeFrameOrMetadataSize(frameSize);

  nwritten += writer.write(streamId());
  nwritten += writer.writeFrameTypeAndFlags(
      frameType(),
      Flags()
          .metadata(payload_.hasNonemptyMetadata())
          .follows(hasFollows())
          .complete(hasComplete()));
  nwritten += writer.writeBE<uint32_t>(initialRequestN());
  nwritten += writer.writePayload(std::move(payload()));

  DCHECK_EQ(Serializer::kBytesForFrameOrMetadataLength + frameSize, nwritten);
}

std::unique_ptr<folly::IOBuf> RequestNFrame::serialize() && {
  return serializeIntoIOBuf(std::move(*this));
}

void RequestNFrame::serialize(Serializer& writer) && {
  /**
   *  0                   1                   2                   3
   *  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
   * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   * |                           Stream ID                           |
   * +-----------+-+-+---------------+-------------------------------+
   * |Frame Type |0|0|     Flags     |
   * +-------------------------------+-------------------------------+
   * |0|                         Request N                           |
   * +---------------------------------------------------------------+
   */

  // Excludes room for frame length
  constexpr auto frameSize = frameHeaderSize();
  auto nwritten = writer.writeFrameOrMetadataSize(frameSize);

  nwritten += writer.write(streamId());
  nwritten += writer.writeFrameTypeAndFlags(frameType(), Flags());
  nwritten += writer.writeBE<uint32_t>(requestN());

  DCHECK_EQ(Serializer::kBytesForFrameOrMetadataLength + frameSize, nwritten);
}

std::unique_ptr<folly::IOBuf> CancelFrame::serialize() && {
  return serializeIntoIOBuf(std::move(*this));
}

void CancelFrame::serialize(Serializer& writer) && {
  /**
   *  0                   1                   2                   3
   *  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
   * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   * |                           Stream ID                           |
   * +-----------+-+-+---------------+-------------------------------+
   * |Frame Type |0|0|    Flags      |
   * +-------------------------------+-------------------------------+
   */

  // Excludes room for frame length
  constexpr auto frameSize = frameHeaderSize();
  auto nwritten = writer.writeFrameOrMetadataSize(frameSize);

  nwritten += writer.write(streamId());
  nwritten += writer.writeFrameTypeAndFlags(frameType(), Flags());

  DCHECK_EQ(Serializer::kBytesForFrameOrMetadataLength + frameSize, nwritten);
}

void PayloadFrame::serialize(Serializer& writer) && {
  if (UNLIKELY(payload().metadataAndDataSize() > kMaxFragmentedPayloadSize)) {
    return std::move(*this).serializeInFragmentsSlow(writer);
  }
  std::move(*this).serializeIntoSingleFrame(writer);
}

void PayloadFrame::serializeIntoSingleFrame(Serializer& writer) && {
  /**
   *  0                   1                   2                   3
   *  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
   * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   * |                           Stream ID                           |
   * +-----------+-+-+-+-+-+---------+-------------------------------+
   * |Frame Type |0|M|F|C|N|  Flags  |
   * +-------------------------------+-------------------------------+
   *                    Metadata & Data
   */

  // Excludes room for frame length
  const auto frameSize = frameHeaderSize() + payload().serializedSize();
  auto nwritten = writer.writeFrameOrMetadataSize(frameSize);

  nwritten += writer.write(streamId());

  // Protocol states that complete or next (or both) MUST be set... but this
  // appears to not hold in the case of Payload fragment frames.
  // DCHECK(hasComplete() || hasNext());
  nwritten += writer.writeFrameTypeAndFlags(
      frameType(),
      Flags()
          .metadata(payload_.hasNonemptyMetadata())
          .follows(hasFollows())
          .complete(hasComplete())
          .next(hasNext()));

  nwritten += writer.writePayload(std::move(payload()));

  DCHECK_EQ(Serializer::kBytesForFrameOrMetadataLength + frameSize, nwritten);
}

std::unique_ptr<folly::IOBuf> PayloadFrame::serialize() && {
  return serializeIntoHeadroomIfPossible(std::move(*this));
}

std::unique_ptr<folly::IOBuf> ErrorFrame::serialize() && {
  return serializeIntoIOBuf(std::move(*this));
}

void ErrorFrame::serialize(Serializer& writer) && {
  /**
   *  0                   1                   2                   3
   *  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
   * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   * |                           Stream ID                           |
   * +-----------+-+-+---------------+-------------------------------+
   * |Frame Type |0|0|      Flags    |
   * +-----------+-+-+---------------+-------------------------------+
   * |                          Error Code                           |
   * +---------------------------------------------------------------+
   *                            Error Payload
   */

  // Excludes room for frame length
  const auto frameSize = frameHeaderSize() + payload().serializedSize();
  auto nwritten = writer.writeFrameOrMetadataSize(frameSize);

  nwritten += writer.write(streamId());
  nwritten += writer.writeFrameTypeAndFlags(frameType(), Flags());

  nwritten += writer.writeBE(folly::to_underlying(errorCode()));

  nwritten += writer.writePayload(std::move(payload()));

  DCHECK_EQ(Serializer::kBytesForFrameOrMetadataLength + frameSize, nwritten);
}

void MetadataPushFrame::serialize(Serializer& writer) && {
  /**
   *  0                   1                   2                   3
   *  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
   * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   * |                         Stream ID = 0                         |
   * +-----------+-+-+---------------+-------------------------------+
   * |Frame Type |0|1|     Flags     |
   * +-------------------------------+-------------------------------+
   *                              Metadata
   */

  // Excludes room for frame length
  const auto frameSize =
      frameHeaderSize() + metadata_->computeChainDataLength();
  auto nwritten = writer.writeFrameOrMetadataSize(frameSize);

  nwritten += writer.write(StreamId{0});
  nwritten +=
      writer.writeFrameTypeAndFlags(frameType(), Flags().metadata(true));
  nwritten += writer.write(std::move(metadata_));

  DCHECK_EQ(Serializer::kBytesForFrameOrMetadataLength + frameSize, nwritten);
}

std::unique_ptr<folly::IOBuf> MetadataPushFrame::serialize() && {
  Serializer writer;
  std::move(*this).serialize(writer);
  return std::move(writer).move();
}

std::unique_ptr<folly::IOBuf> KeepAliveFrame::serialize() && {
  return serializeIntoIOBuf(std::move(*this));
}

void KeepAliveFrame::serialize(Serializer& writer) && {
  /**
   * 0                   1                   2                   3
   * 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
   * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   * |                         Stream ID = 0                         |
   * +-----------+-+-+-+-------------+-------------------------------+
   * |Frame Type |0|0|R|    Flags    |
   * +-----------+-+-+-+-------------+-------------------------------+
   * |0|                  Last Received Position                     |
   * +                                                               +
   * |                                                               |
   * +---------------------------------------------------------------+
   *                              Data
   */

  // Excludes room for frame length
  const auto frameSize =
      frameHeaderSize() + (data_ ? data_->computeChainDataLength() : 0);
  auto nwritten = writer.writeFrameOrMetadataSize(frameSize);

  nwritten += writer.write(StreamId{0});
  nwritten += writer.writeFrameTypeAndFlags(frameType(), flags_);

  // Last received position: send 0 if not supported.
  nwritten += writer.writeBE(static_cast<uint64_t>(0));

  if (data_) {
    nwritten += writer.write(std::move(data_));
  }

  DCHECK_EQ(Serializer::kBytesForFrameOrMetadataLength + frameSize, nwritten);
}

std::unique_ptr<folly::IOBuf> ExtFrame::serialize() && {
  Serializer writer;
  std::move(*this).serialize(writer);
  return std::move(writer).move();
}

void ExtFrame::serialize(Serializer& writer) && {
  /**
   *  0                   1                   2                   3
   *  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
   * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   * |                           Stream ID                           |
   * +-----------+-+-+---------------+-------------------------------+
   * |Frame Type |I|M|    Flags      |
   * +-------------------------------+-------------------------------+
   * |0|                      Extended Type                          |
   * +---------------------------------------------------------------+
   *                       Depends on Extended Type...
   */

  if (payload().metadataAndDataSize() >= kMaxFragmentedPayloadSize) {
    throw std::runtime_error("EXT frame payload too large.");
  }

  // Excludes room for frame length
  const auto frameSize = frameHeaderSize() + payload().serializedSize();
  auto nwritten = writer.writeFrameOrMetadataSize(frameSize);

  nwritten += writer.write(streamId());

  nwritten += writer.writeFrameTypeAndFlags(
      frameType(),
      flags_.metadata(payload_.hasNonemptyMetadata()).ignore(hasIgnore()));

  nwritten += writer.writeBE(folly::to_underlying(extFrameType_));

  nwritten += writer.writePayload(std::move(payload()));

  DCHECK_EQ(Serializer::kBytesForFrameOrMetadataLength + frameSize, nwritten);
}

FOLLY_NOINLINE void RequestResponseFrame::serializeInFragmentsSlow(
    Serializer& writer) && {
  serializeInFragmentsSlowCommon(std::move(*this), Flags().next(true), writer);
}

FOLLY_NOINLINE void RequestFnfFrame::serializeInFragmentsSlow(
    Serializer& writer) && {
  serializeInFragmentsSlowCommon(std::move(*this), Flags().next(true), writer);
}

FOLLY_NOINLINE void RequestStreamFrame::serializeInFragmentsSlow(
    Serializer& writer) && {
  serializeInFragmentsSlowCommon(std::move(*this), Flags().next(true), writer);
}

FOLLY_NOINLINE void RequestChannelFrame::serializeInFragmentsSlow(
    Serializer& writer) && {
  serializeInFragmentsSlowCommon(
      std::move(*this), Flags().next(true).complete(hasComplete()), writer);
}

FOLLY_NOINLINE void PayloadFrame::serializeInFragmentsSlow(
    Serializer& writer) && {
  serializeInFragmentsSlowCommon(
      std::move(*this),
      Flags().complete(hasComplete()).next(hasNext()),
      writer);
}

SetupFrame::SetupFrame(std::unique_ptr<folly::IOBuf> frame) {
  folly::io::Cursor cursor(frame.get());
  const StreamId zero(readStreamId(cursor));
  if (zero != StreamId{0}) {
    throw std::runtime_error(fmt::format(
        "SETUP frame has non zero stream id {}", static_cast<uint32_t>(zero)));
  }

  FrameType type;
  std::tie(type, flags_) = readFrameTypeAndFlags(cursor);

  const auto majorVersion = cursor.readBE<uint16_t>();
  const auto minorVersion = cursor.readBE<uint16_t>();

  if (majorVersion != 1 || minorVersion != 0) {
    throw std::runtime_error(fmt::format(
        "SETUP frame received with unsupported version {}.{}",
        majorVersion,
        minorVersion));
  }

  // Skip keep-alive interval (4 bytes) and max lifetime (4 bytes). These values
  // are not currently used in Thrift.
  cursor.skip(8);

  // Resumption is not currently supported, but we handle the resume
  // identification token properly in case remote end sends a token.
  if (hasResumeIdentificationToken()) {
    const auto tokenLength = cursor.readBE<uint16_t>();
    cursor.skip(tokenLength);
  }

  const auto metadataMimeLength = cursor.read<uint8_t>();
  auto metadataMimeType = cursor.readFixedString(metadataMimeLength);
  const auto dataMimeLength = cursor.read<uint8_t>();
  auto dataMimeType = cursor.readFixedString(dataMimeLength);

  rocketMimeTypes_ = (metadataMimeType == kRocketMetadataMimeType) &&
      (dataMimeType == kRocketPayloadMimeType);

  payload_ = readPayload(flags_.metadata(), cursor, std::move(frame));
}

RequestResponseFrame::RequestResponseFrame(
    std::unique_ptr<folly::IOBuf> frame) {
  folly::io::Cursor cursor(frame.get());

  streamId_ = readStreamId(cursor);

  FrameType type;
  std::tie(type, flags_) = readFrameTypeAndFlags(cursor);
  DCHECK(frameType() == type);

  payload_ = readPayload(flags_.metadata(), cursor, std::move(frame));
}

RequestResponseFrame::RequestResponseFrame(
    StreamId streamId,
    Flags flags,
    folly::io::Cursor& cursor,
    std::unique_ptr<folly::IOBuf> underlyingBuffer)
    : streamId_(streamId), flags_(flags) {
  payload_ =
      readPayload(flags_.metadata(), cursor, std::move(underlyingBuffer));
}

RequestFnfFrame::RequestFnfFrame(std::unique_ptr<folly::IOBuf> frame) {
  folly::io::Cursor cursor(frame.get());

  streamId_ = readStreamId(cursor);

  FrameType type;
  std::tie(type, flags_) = readFrameTypeAndFlags(cursor);
  DCHECK(frameType() == type);

  payload_ = readPayload(flags_.metadata(), cursor, std::move(frame));
}

RequestFnfFrame::RequestFnfFrame(
    StreamId streamId,
    Flags flags,
    folly::io::Cursor& cursor,
    std::unique_ptr<folly::IOBuf> underlyingBuffer)
    : streamId_(streamId), flags_(flags) {
  payload_ =
      readPayload(flags_.metadata(), cursor, std::move(underlyingBuffer));
}

RequestStreamFrame::RequestStreamFrame(std::unique_ptr<folly::IOBuf> frame) {
  folly::io::Cursor cursor(frame.get());

  streamId_ = readStreamId(cursor);

  FrameType type;
  std::tie(type, flags_) = readFrameTypeAndFlags(cursor);
  DCHECK(frameType() == type);

  initialRequestN_ = cursor.readBE<int32_t>();

  payload_ = readPayload(flags_.metadata(), cursor, std::move(frame));
}

RequestStreamFrame::RequestStreamFrame(
    StreamId streamId,
    Flags flags,
    folly::io::Cursor& cursor,
    std::unique_ptr<folly::IOBuf> underlyingBuffer)
    : streamId_(streamId), flags_(flags) {
  initialRequestN_ = cursor.readBE<int32_t>();
  payload_ =
      readPayload(flags_.metadata(), cursor, std::move(underlyingBuffer));
}

RequestChannelFrame::RequestChannelFrame(std::unique_ptr<folly::IOBuf> frame) {
  folly::io::Cursor cursor(frame.get());

  streamId_ = readStreamId(cursor);

  FrameType type;
  std::tie(type, flags_) = readFrameTypeAndFlags(cursor);
  DCHECK(frameType() == type);

  initialRequestN_ = cursor.readBE<int32_t>();

  payload_ = readPayload(flags_.metadata(), cursor, std::move(frame));
}

RequestChannelFrame::RequestChannelFrame(
    StreamId streamId,
    Flags flags,
    folly::io::Cursor& cursor,
    std::unique_ptr<folly::IOBuf> underlyingBuffer)
    : streamId_(streamId), flags_(flags) {
  initialRequestN_ = cursor.readBE<int32_t>();
  payload_ =
      readPayload(flags_.metadata(), cursor, std::move(underlyingBuffer));
}

RequestNFrame::RequestNFrame(std::unique_ptr<folly::IOBuf> frame) {
  folly::io::Cursor cursor(frame.get());

  streamId_ = readStreamId(cursor);

  // RequestN frame has no flags, but we need to skip over the two bytes.
  FrameType type;
  Flags flags;
  std::tie(type, flags) = readFrameTypeAndFlags(cursor);
  DCHECK(frameType() == type);
  DCHECK(Flags() == flags);

  requestN_ = cursor.readBE<int32_t>();
}

RequestNFrame::RequestNFrame(
    StreamId streamId, Flags flags, folly::io::Cursor& cursor)
    : streamId_(streamId) {
  DCHECK(Flags() == flags);
  requestN_ = cursor.readBE<int32_t>();
}

CancelFrame::CancelFrame(std::unique_ptr<folly::IOBuf> frame) {
  folly::io::Cursor cursor(frame.get());

  streamId_ = readStreamId(cursor);

  // Cancel frame has no flags.
  FrameType type;
  Flags flags;
  std::tie(type, flags) = readFrameTypeAndFlags(cursor);
  DCHECK(frameType() == type);
  DCHECK(Flags() == flags);
}

PayloadFrame::PayloadFrame(std::unique_ptr<folly::IOBuf> frame) {
  folly::io::Cursor cursor(frame.get());

  streamId_ = readStreamId(cursor);

  FrameType type;
  std::tie(type, flags_) = readFrameTypeAndFlags(cursor);
  DCHECK(frameType() == type);

  payload_ = readPayload(flags_.metadata(), cursor, std::move(frame));
}

PayloadFrame::PayloadFrame(
    StreamId streamId,
    Flags flags,
    folly::io::Cursor& cursor,
    std::unique_ptr<folly::IOBuf> underlyingBuffer)
    : streamId_(streamId),
      flags_(flags),
      payload_(readPayload(
          flags_.metadata(), cursor, std::move(underlyingBuffer))) {}

ErrorFrame::ErrorFrame(std::unique_ptr<folly::IOBuf> frame) {
  folly::io::Cursor cursor(frame.get());
  DCHECK_GE(frame->computeChainDataLength(), frameHeaderSize());

  streamId_ = readStreamId(cursor);

  // Error frame has no flags, but we still need to skip the two bytes.
  FrameType type;
  Flags flags;
  std::tie(type, flags) = readFrameTypeAndFlags(cursor);
  DCHECK(frameType() == type);
  DCHECK(Flags() == flags);

  errorCode_ = static_cast<ErrorCode>(cursor.readBE<uint32_t>());

  // Finally, adjust the data portion of frame.
  frame = trimBuffer(std::move(frame), frameHeaderSize());
  payload_ = Payload::makeFromData(std::move(frame));
}

MetadataPushFrame::MetadataPushFrame(std::unique_ptr<folly::IOBuf> frame) {
  DCHECK_GE(frame->computeChainDataLength(), frameHeaderSize());

  folly::io::Cursor cursor(frame.get());
  const StreamId zero(readStreamId(cursor));
  DCHECK_EQ(StreamId{0}, zero);

  // METADATA_PUSH frame has only metadata flag.
  FrameType type;
  Flags flags;
  std::tie(type, flags) = readFrameTypeAndFlags(cursor);
  DCHECK(frameType() == type);
  DCHECK(flags.metadata());

  frame = trimBuffer(std::move(frame), frameHeaderSize());
  metadata_ = std::move(frame);
}

KeepAliveFrame::KeepAliveFrame(std::unique_ptr<folly::IOBuf> frame) {
  DCHECK_GE(frame->computeChainDataLength(), frameHeaderSize());

  folly::io::Cursor cursor(frame.get());
  const StreamId zero(readStreamId(cursor));
  DCHECK_EQ(StreamId{0}, zero);

  FrameType type;
  std::tie(type, flags_) = readFrameTypeAndFlags(cursor);
  DCHECK(frameType() == type);

  cursor.skip(sizeof(uint64_t)); // Skip 'last received position'

  frame = trimBuffer(std::move(frame), cursor.getCurrentPosition());
  data_ = std::move(frame);
}

ExtFrame::ExtFrame(std::unique_ptr<folly::IOBuf> frame) {
  folly::io::Cursor cursor(frame.get());

  streamId_ = readStreamId(cursor);

  FrameType type;
  std::tie(type, flags_) = readFrameTypeAndFlags(cursor);
  DCHECK(frameType() == type);

  extFrameType_ = readExtFrameType(cursor);

  payload_ = readPayload(flags_.metadata(), cursor, std::move(frame));
}

ExtFrame::ExtFrame(
    StreamId streamId,
    Flags flags,
    folly::io::Cursor& cursor,
    std::unique_ptr<folly::IOBuf> underlyingBuffer)
    : streamId_(streamId), flags_(flags) {
  extFrameType_ = readExtFrameType(cursor);

  payload_ =
      readPayload(flags_.metadata(), cursor, std::move(underlyingBuffer));
}

// Static member definition
constexpr folly::StringPiece SetupFrame::kLegacyMimeType;
constexpr folly::StringPiece SetupFrame::kRocketMetadataMimeType;
constexpr folly::StringPiece SetupFrame::kRocketPayloadMimeType;

} // namespace rocket
} // namespace thrift
} // namespace apache
