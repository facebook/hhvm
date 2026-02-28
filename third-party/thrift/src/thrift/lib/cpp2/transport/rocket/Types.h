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

#include <functional>
#include <iosfwd>
#include <memory>
#include <utility>

#include <folly/Range.h>
#include <folly/io/IOBuf.h>
#include <folly/io/async/fdsock/SocketFds.h>
#include <thrift/lib/cpp2/protocol/Protocol.h>

namespace apache::thrift::rocket {

class StreamId {
 public:
  using underlying_type = uint32_t;

  constexpr StreamId() = default;
  constexpr explicit StreamId(uint32_t streamId) : streamId_(streamId) {}

  bool operator==(StreamId other) const { return streamId_ == other.streamId_; }

  bool operator!=(StreamId other) const { return streamId_ != other.streamId_; }

  StreamId& operator+=(uint32_t delta) {
    streamId_ += delta;
    return *this;
  }

  explicit operator uint32_t() const { return streamId_; }

  static constexpr StreamId maxClientStreamId() {
    return StreamId{(1ul << 31) - 1};
  }

 private:
  uint32_t streamId_{0};
};

std::ostream& operator<<(std::ostream& os, StreamId streamId);

class Payload {
 public:
  Payload() = default;

  Payload(const Payload&) = delete;
  Payload& operator=(const Payload&) = delete;

  Payload(Payload&&) = default;
  Payload& operator=(Payload&&) = default;

  ~Payload() = default;

  // Force user code to be explicit about the order in which metadata and data
  // are being passed by making non-default constructors private.
  static Payload makeFromData(std::unique_ptr<folly::IOBuf> data) {
    return Payload(std::move(data));
  }
  static Payload makeFromData(folly::ByteRange data) { return Payload(data); }
  static Payload makeFromMetadataAndData(
      std::unique_ptr<folly::IOBuf> metadata,
      std::unique_ptr<folly::IOBuf> data) {
    return Payload(std::move(metadata), std::move(data));
  }
  static Payload makeFromMetadataAndData(
      folly::ByteRange metadata, folly::ByteRange data) {
    return Payload(metadata, data);
  }
  static Payload makeCombined(
      std::unique_ptr<folly::IOBuf> buffer, size_t metadataSize) {
    return Payload(std::move(buffer), metadataSize);
  }

  Payload clone() const {
    if (!hasData()) {
      Payload p;
      p.buffer_ = std::make_unique<folly::IOBuf>();
      return p;
    }
    return makeCombined(buffer_->clone(), metadataSize_);
  }

  std::unique_ptr<folly::IOBuf> data() && {
    DCHECK(buffer_ != nullptr);
    DCHECK_LE(metadataSize_, metadataAndDataSize_);
    DCHECK_LE(metadataSize_, buffer_->computeChainDataLength());

    auto toTrim = metadataSize_;
    auto data = std::move(buffer_);
    while (toTrim > 0) {
      if (data->length() >= toTrim) {
        data->trimStart(toTrim);
        toTrim = 0;
      } else {
        toTrim -= data->length();
        data = data->pop();
      }
    }
    return data;
  }

  bool hasNonemptyMetadata() const noexcept { return metadataSize_; }

  size_t metadataSize() const noexcept { return metadataSize_; }

  size_t dataSize() const noexcept {
    DCHECK(metadataAndDataSize_ >= metadataSize_);
    return metadataAndDataSize_ - metadataSize_;
  }

  const folly::IOBuf* buffer() const& { return buffer_.get(); }

  std::unique_ptr<folly::IOBuf> buffer() && { return std::move(buffer_); }

  size_t metadataAndDataSize() const { return metadataAndDataSize_; }

  size_t serializedSize() const {
    constexpr size_t kBytesForMetadataSize = 3;
    return metadataAndDataSize_ +
        (hasNonemptyMetadata() ? kBytesForMetadataSize : 0ull);
  }

  uint32_t dataFirstFieldAlignment() const { return dataFirstFieldAlignment_; }

  void setDataFirstFieldAlignment(uint32_t alignment) {
    dataFirstFieldAlignment_ = alignment;
  }

  std::optional<ProtocolType> dataSerializationProtocol() const {
    return dataSerializationProtocol_;
  }

  void setDataSerializationProtocol(ProtocolType protocol) {
    dataSerializationProtocol_ = protocol;
  }

  void append(Payload&& other);

  bool hasData() const { return buffer_ != nullptr; }

  folly::SocketFds fds;

 private:
  std::unique_ptr<folly::IOBuf> buffer_;
  size_t metadataSize_{0};
  size_t metadataAndDataSize_{0};
  uint32_t dataFirstFieldAlignment_{0};
  std::optional<ProtocolType> dataSerializationProtocol_{std::nullopt};

  explicit Payload(std::unique_ptr<folly::IOBuf> data)
      : buffer_(data ? std::move(data) : std::make_unique<folly::IOBuf>()),
        metadataAndDataSize_(buffer_->computeChainDataLength()) {}

  Payload(
      std::unique_ptr<folly::IOBuf> metadata,
      std::unique_ptr<folly::IOBuf> data) {
    if (metadata) {
      metadataSize_ = metadata->computeChainDataLength();
      metadataAndDataSize_ = metadataSize_;
      if (data) {
        metadataAndDataSize_ += data->computeChainDataLength();
        metadata->prependChain(std::move(data));
      }
      buffer_ = std::move(metadata);
    } else if (data) {
      buffer_ = std::move(data);
      metadataAndDataSize_ = buffer_->computeChainDataLength();
    } else {
      buffer_ = std::make_unique<folly::IOBuf>();
    }
  }

  Payload(std::unique_ptr<folly::IOBuf> buffer, size_t metadataSize)
      : buffer_(buffer ? std::move(buffer) : std::make_unique<folly::IOBuf>()),
        metadataSize_(metadataSize),
        metadataAndDataSize_(buffer_->computeChainDataLength()) {}

  explicit Payload(folly::ByteRange data)
      : buffer_(folly::IOBuf::copyBuffer(data)),
        metadataAndDataSize_(buffer_->computeChainDataLength()) {}

  Payload(folly::ByteRange metadata, folly::ByteRange data)
      : buffer_(folly::IOBuf::copyBuffer(metadata)),
        metadataSize_(metadata.size()),
        metadataAndDataSize_(metadata.size() + data.size()) {
    buffer_->prependChain(folly::IOBuf::copyBuffer(data));
  }
};

} // namespace apache::thrift::rocket

namespace std {
template <>
struct hash<apache::thrift::rocket::StreamId> {
  size_t operator()(apache::thrift::rocket::StreamId streamId) const {
    return hash<uint32_t>()(static_cast<uint32_t>(streamId));
  }
};
} // namespace std
