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

#pragma once

#include <limits>
#include <memory>
#include <stdexcept>
#include <utility>

#include <folly/CPortability.h>
#include <folly/Range.h>
#include <folly/lang/Exception.h>

#include <thrift/lib/cpp2/Flags.h>
#include <thrift/lib/cpp2/transport/rocket/RocketException.h>
#include <thrift/lib/cpp2/transport/rocket/Types.h>
#include <thrift/lib/cpp2/transport/rocket/framing/ErrorCode.h>
#include <thrift/lib/cpp2/transport/rocket/framing/Flags.h>
#include <thrift/lib/cpp2/transport/rocket/framing/FrameType.h>
#include <thrift/lib/cpp2/transport/rocket/framing/Util.h>

THRIFT_FLAG_DECLARE_bool(fix_cpp_fragmentation);

namespace apache {
namespace thrift {
namespace rocket {

class Serializer;

class SetupFrame {
 public:
  explicit SetupFrame(std::unique_ptr<folly::IOBuf> frame);

  explicit SetupFrame(Payload&& payload, bool rocketMimeTypes)
      : payload_(std::move(payload)), rocketMimeTypes_(rocketMimeTypes) {}

  static constexpr FrameType frameType() { return FrameType::SETUP; }

  size_t frameHeaderSize() const {
    size_t frameSize = 20 +
        (rocketMimeTypes_
             ? kRocketMetadataMimeType.size() + kRocketPayloadMimeType.size()
             : 2 * kLegacyMimeType.size());
    if (hasResumeIdentificationToken()) {
      frameSize +=
          2 /* bytes for token length */ + resumeIdentificationToken_.size();
    }
    return frameSize;
  }

  bool hasResumeIdentificationToken() const noexcept {
    return flags_.resumeToken();
  }

  bool hasLease() const noexcept { return flags_.lease(); }

  const Payload& payload() const noexcept { return payload_; }
  Payload& payload() noexcept { return payload_; }

  bool rocketMimeTypes() const { return rocketMimeTypes_; }

  std::unique_ptr<folly::IOBuf> serialize() &&;
  void serialize(Serializer& writer) &&;

 private:
  static constexpr folly::StringPiece kLegacyMimeType{"text/plain"};
  static constexpr folly::StringPiece kRocketMetadataMimeType{
      "application/x-rocket-metadata+compact"};
  static constexpr folly::StringPiece kRocketPayloadMimeType{
      "application/x-rocket-payload"};

  // Resume ID token and Lease flags are not currently supported/used.
  Flags flags_;
  std::string resumeIdentificationToken_;
  Payload payload_;
  bool rocketMimeTypes_;
};

class RequestResponseFrame {
 public:
  explicit RequestResponseFrame(std::unique_ptr<folly::IOBuf> frame);

  RequestResponseFrame(
      StreamId streamId,
      Flags flags,
      folly::io::Cursor& cursor,
      std::unique_ptr<folly::IOBuf> underlyingBuffer);

  RequestResponseFrame(StreamId streamId, Payload&& payload)
      : streamId_(streamId), payload_(std::move(payload)) {}

  static constexpr FrameType frameType() { return FrameType::REQUEST_RESPONSE; }

  static constexpr size_t frameHeaderSize() { return 6; }

  StreamId streamId() const noexcept { return streamId_; }

  bool hasFollows() const noexcept { return flags_.follows(); }
  void setHasFollows(bool hasFollows) noexcept { flags_.follows(hasFollows); }

  const Payload& payload() const noexcept { return payload_; }
  Payload& payload() noexcept { return payload_; }

  Flags flags() const noexcept {
    return Flags(flags_).metadata(payload().hasNonemptyMetadata());
  }

  std::unique_ptr<folly::IOBuf> serialize() &&;
  void serialize(Serializer& writer) &&;

 private:
  StreamId streamId_;
  Flags flags_;
  Payload payload_;

  void serializeIntoSingleFrame(Serializer& writer) &&;
  FOLLY_NOINLINE void serializeInFragmentsSlow(Serializer& writer) &&;
};

class RequestFnfFrame {
 public:
  explicit RequestFnfFrame(std::unique_ptr<folly::IOBuf> frame);

  RequestFnfFrame(
      StreamId streamId,
      Flags flags,
      folly::io::Cursor& cursor,
      std::unique_ptr<folly::IOBuf> underlyingBuffer);

  RequestFnfFrame(StreamId streamId, Payload&& payload)
      : streamId_(streamId), payload_(std::move(payload)) {}

  static constexpr size_t frameHeaderSize() { return 6; }

  static constexpr FrameType frameType() { return FrameType::REQUEST_FNF; }

  StreamId streamId() const noexcept { return streamId_; }

  bool hasFollows() const noexcept { return flags_.follows(); }
  void setHasFollows(bool hasFollows) noexcept { flags_.follows(hasFollows); }

  const Payload& payload() const noexcept { return payload_; }
  Payload& payload() noexcept { return payload_; }

  Flags flags() const noexcept {
    return Flags(flags_).metadata(payload().hasNonemptyMetadata());
  }

  std::unique_ptr<folly::IOBuf> serialize() &&;
  void serialize(Serializer& writer) &&;

 private:
  StreamId streamId_;
  Flags flags_;
  Payload payload_;

  void serializeIntoSingleFrame(Serializer& writer) &&;
  FOLLY_NOINLINE void serializeInFragmentsSlow(Serializer& writer) &&;
};

class RequestStreamFrame {
 public:
  explicit RequestStreamFrame(std::unique_ptr<folly::IOBuf> frame);

  RequestStreamFrame(
      StreamId streamId,
      Flags flags,
      folly::io::Cursor& cursor,
      std::unique_ptr<folly::IOBuf> underlyingBuffer);

  RequestStreamFrame(
      StreamId streamId, Payload&& payload, int32_t initialRequestN)
      : streamId_(streamId),
        initialRequestN_(initialRequestN),
        payload_(std::move(payload)) {
    if (initialRequestN_ <= 0) {
      folly::throw_exception<std::logic_error>(
          "RequestStreamFrame's initialRequestN MUST be > 0");
    }
  }

  static constexpr FrameType frameType() { return FrameType::REQUEST_STREAM; }

  static constexpr size_t frameHeaderSize() { return 10; }

  StreamId streamId() const noexcept { return streamId_; }

  bool hasFollows() const noexcept { return flags_.follows(); }
  void setHasFollows(bool hasFollows) noexcept { flags_.follows(hasFollows); }

  int32_t initialRequestN() const noexcept { return initialRequestN_; }

  const Payload& payload() const noexcept { return payload_; }
  Payload& payload() noexcept { return payload_; }

  Flags flags() const noexcept {
    return Flags(flags_).metadata(payload().hasNonemptyMetadata());
  }

  std::unique_ptr<folly::IOBuf> serialize() &&;
  void serialize(Serializer& writer) &&;

 private:
  StreamId streamId_;
  int32_t initialRequestN_;
  Flags flags_;
  Payload payload_;

  void serializeIntoSingleFrame(Serializer& writer) &&;
  FOLLY_NOINLINE void serializeInFragmentsSlow(Serializer& writer) &&;
};

class RequestChannelFrame {
 public:
  explicit RequestChannelFrame(std::unique_ptr<folly::IOBuf> frame);

  RequestChannelFrame(
      StreamId streamId,
      Flags flags,
      folly::io::Cursor& cursor,
      std::unique_ptr<folly::IOBuf> underlyingBuffer);

  RequestChannelFrame(
      StreamId streamId, Payload&& payload, int32_t initialRequestN)
      : streamId_(streamId),
        initialRequestN_(initialRequestN),
        payload_(std::move(payload)) {
    if (initialRequestN_ <= 0) {
      folly::throw_exception<std::logic_error>(
          "RequestChannelFrame's initialRequestN MUST be > 0");
    }
  }

  static constexpr FrameType frameType() { return FrameType::REQUEST_CHANNEL; }

  static constexpr size_t frameHeaderSize() { return 10; }

  StreamId streamId() const noexcept { return streamId_; }

  bool hasFollows() const noexcept { return flags_.follows(); }
  void setHasFollows(bool hasFollows) noexcept { flags_.follows(hasFollows); }

  bool hasComplete() const noexcept { return flags_.complete(); }

  void setHasComplete(bool hasComplete) noexcept {
    flags_.complete(hasComplete);
  }

  int32_t initialRequestN() const noexcept { return initialRequestN_; }

  const Payload& payload() const noexcept { return payload_; }

  Payload& payload() noexcept { return payload_; }

  Flags flags() const noexcept {
    return Flags(flags_).metadata(payload().hasNonemptyMetadata());
  }

  std::unique_ptr<folly::IOBuf> serialize() &&;
  void serialize(Serializer& writer) &&;

 private:
  StreamId streamId_;
  int32_t initialRequestN_;
  Flags flags_;
  Payload payload_;

  void serializeIntoSingleFrame(Serializer& writer) &&;
  FOLLY_NOINLINE void serializeInFragmentsSlow(Serializer& writer) &&;
};

class RequestNFrame {
 public:
  explicit RequestNFrame(std::unique_ptr<folly::IOBuf> frame);

  RequestNFrame(StreamId streamId, Flags flags, folly::io::Cursor& cursor);

  RequestNFrame(StreamId streamId, int32_t n)
      : streamId_(streamId), requestN_(n) {
    if (requestN_ <= 0) {
      folly::throw_exception<std::logic_error>("REQUEST_N value MUST be > 0");
    }
  }

  static constexpr FrameType frameType() { return FrameType::REQUEST_N; }

  static constexpr size_t frameHeaderSize() { return 10; }

  StreamId streamId() const noexcept { return streamId_; }

  int32_t requestN() const noexcept { return requestN_; }

  std::unique_ptr<folly::IOBuf> serialize() &&;
  void serialize(Serializer& writer) &&;

 private:
  StreamId streamId_;
  int32_t requestN_;
};

class CancelFrame {
 public:
  explicit CancelFrame(std::unique_ptr<folly::IOBuf> frame);

  explicit CancelFrame(StreamId streamId) : streamId_(streamId) {}

  static constexpr FrameType frameType() { return FrameType::CANCEL; }

  static constexpr size_t frameHeaderSize() { return 6; }

  StreamId streamId() const noexcept { return streamId_; }

  std::unique_ptr<folly::IOBuf> serialize() &&;
  void serialize(Serializer& writer) &&;

 private:
  StreamId streamId_;
};

class PayloadFrame {
 public:
  explicit PayloadFrame(std::unique_ptr<folly::IOBuf> frame);

  PayloadFrame(
      StreamId streamId,
      Flags flags,
      folly::io::Cursor& cursor,
      std::unique_ptr<folly::IOBuf> underlyingBuffer);

  PayloadFrame(StreamId streamId, Payload&& payload, Flags flags)
      : streamId_(streamId), flags_(flags), payload_(std::move(payload)) {}

  static constexpr FrameType frameType() { return FrameType::PAYLOAD; }

  static constexpr size_t frameHeaderSize() { return 6; }

  StreamId streamId() const noexcept { return streamId_; }

  const Payload& payload() const noexcept { return payload_; }
  Payload& payload() noexcept { return payload_; }

  bool hasFollows() const noexcept { return flags_.follows(); }
  void setHasFollows(bool hasFollows) noexcept {
    flags_.follows(hasFollows);
    if (THRIFT_FLAG(fix_cpp_fragmentation)) {
      flags_.complete(!hasFollows && hasComplete());
    }
  }
  bool hasComplete() const noexcept { return flags_.complete(); }
  bool hasNext() const noexcept { return flags_.next(); }

  Flags flags() const noexcept {
    return Flags(flags_).metadata(payload().hasNonemptyMetadata());
  }

  std::unique_ptr<folly::IOBuf> serialize() &&;
  void serialize(Serializer& writer) &&;

 private:
  StreamId streamId_;
  Flags flags_;
  Payload payload_;

  void serializeIntoSingleFrame(Serializer& writer) &&;
  std::unique_ptr<folly::IOBuf> serializeUsingMetadataHeadroom() &&;
  FOLLY_NOINLINE void serializeInFragmentsSlow(Serializer& writer) &&;
};

class ErrorFrame {
 public:
  explicit ErrorFrame(std::unique_ptr<folly::IOBuf> frame);

  ErrorFrame(StreamId streamId, ErrorCode errorCode, Payload&& payload)
      : streamId_(streamId),
        errorCode_(errorCode),
        payload_(std::move(payload)) {}

  ErrorFrame(StreamId streamId, RocketException&& rocketException)
      : ErrorFrame(
            streamId,
            rocketException.getErrorCode(),
            Payload::makeFromData(rocketException.moveErrorData())) {}

  static constexpr FrameType frameType() { return FrameType::ERROR; }

  static constexpr size_t frameHeaderSize() { return 10; }

  StreamId streamId() const noexcept { return streamId_; }

  ErrorCode errorCode() const noexcept { return errorCode_; }

  const Payload& payload() const noexcept { return payload_; }
  Payload& payload() noexcept { return payload_; }

  std::unique_ptr<folly::IOBuf> serialize() &&;
  void serialize(Serializer& writer) &&;

 private:
  StreamId streamId_;
  ErrorCode errorCode_;
  Payload payload_;
};

class MetadataPushFrame {
 public:
  explicit MetadataPushFrame(std::unique_ptr<folly::IOBuf> frame);
  static MetadataPushFrame makeFromMetadata(
      std::unique_ptr<folly::IOBuf> metadata) {
    return MetadataPushFrame(std::move(metadata), FromMetadata{});
  }

  static constexpr FrameType frameType() { return FrameType::METADATA_PUSH; }

  static constexpr size_t frameHeaderSize() { return 6; }

  constexpr StreamId streamId() const noexcept { return StreamId{0}; }

  std::unique_ptr<folly::IOBuf> metadata() && { return std::move(metadata_); }

  const folly::IOBuf* metadata() & { return metadata_.get(); }

  void serialize(Serializer& writer) &&;
  std::unique_ptr<folly::IOBuf> serialize() &&;

 private:
  struct FromMetadata {};
  MetadataPushFrame(std::unique_ptr<folly::IOBuf> metadata, FromMetadata)
      : metadata_(std::move(metadata)) {}
  std::unique_ptr<folly::IOBuf> metadata_;
};

class KeepAliveFrame {
 public:
  explicit KeepAliveFrame(std::unique_ptr<folly::IOBuf> frame);
  KeepAliveFrame(Flags flags, std::unique_ptr<folly::IOBuf> data)
      : flags_(flags), data_(std::move(data)) {}

  static constexpr FrameType frameType() { return FrameType::KEEPALIVE; }

  static constexpr size_t frameHeaderSize() { return 14; }

  bool hasRespondFlag() const { return flags_.respond(); }

  std::unique_ptr<folly::IOBuf> data() && { return std::move(data_); }

  void serialize(Serializer& writer) &&;
  std::unique_ptr<folly::IOBuf> serialize() &&;

 private:
  Flags flags_;
  std::unique_ptr<folly::IOBuf> data_;
};

class ExtFrame {
 public:
  explicit ExtFrame(std::unique_ptr<folly::IOBuf> frame);
  ExtFrame(
      StreamId streamId,
      Flags flags,
      folly::io::Cursor& cursor,
      std::unique_ptr<folly::IOBuf> underlyingBuffer);
  ExtFrame(
      StreamId streamId,
      Payload&& payload,
      Flags flags,
      ExtFrameType extFrameType)
      : streamId_(streamId),
        flags_(flags),
        extFrameType_(extFrameType),
        payload_(std::move(payload)) {}

  static constexpr FrameType frameType() { return FrameType::EXT; }

  static constexpr size_t frameHeaderSize() { return 10; }

  StreamId streamId() const noexcept { return streamId_; }

  const Payload& payload() const noexcept { return payload_; }
  Payload& payload() noexcept { return payload_; }

  bool hasIgnore() const noexcept { return flags_.ignore(); }

  Flags flags() const noexcept {
    return Flags(flags_).metadata(payload().hasNonemptyMetadata());
  }

  ExtFrameType extFrameType() const noexcept { return extFrameType_; }

  void serialize(Serializer& writer) &&;
  std::unique_ptr<folly::IOBuf> serialize() &&;

 private:
  StreamId streamId_;
  Flags flags_;
  ExtFrameType extFrameType_;
  Payload payload_;
};

// All frame sizes (header size + payload size) are encoded in 3 bytes
constexpr size_t kMaxFragmentedPayloadSize = 0xffffff - 512;

} // namespace rocket
} // namespace thrift
} // namespace apache
