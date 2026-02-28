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

#include <folly/io/IOBuf.h>

#include <thrift/lib/cpp/TApplicationException.h>
#include <thrift/lib/cpp2/protocol/Protocol.h>
#include <thrift/lib/cpp2/transport/rocket/payload/PayloadSerializer.h>
#include <thrift/lib/thrift/gen-cpp2/RpcMetadata_types.h>

namespace apache::thrift {

struct SerializedRequest {
  explicit SerializedRequest(std::unique_ptr<folly::IOBuf> buffer_)
      : buffer(std::move(buffer_)) {}

  std::unique_ptr<folly::IOBuf> buffer;
  folly::IOBufFactory* ioBufFactory{nullptr};
};

class SerializedCompressedRequest {
 public:
  explicit SerializedCompressedRequest(
      std::unique_ptr<folly::IOBuf> buffer,
      CompressionAlgorithm compression = CompressionAlgorithm::NONE,
      ChecksumAlgorithm checksum = ChecksumAlgorithm::NONE,
      std::optional<rocket::PayloadSerializer::Ptr> payloadSerializer =
          std::nullopt)
      : buffer_(std::move(buffer)),
        compression_(compression),
        checksum_(checksum),
        payloadSerializer_(std::move(payloadSerializer)) {}

  explicit SerializedCompressedRequest(SerializedRequest&& request)
      : buffer_(std::move(request.buffer)),
        compression_(CompressionAlgorithm::NONE) {}

  SerializedRequest uncompress() &&;

  SerializedCompressedRequest clone() const;

  CompressionAlgorithm getCompressionAlgorithm() const { return compression_; }

  ChecksumAlgorithm getChecksumAlgorithm() const { return checksum_; }

  const folly::IOBuf* compressedBuffer() const { return buffer_.get(); }

 private:
  std::unique_ptr<folly::IOBuf> buffer_;
  CompressionAlgorithm compression_;
  ChecksumAlgorithm checksum_;
  std::optional<rocket::PayloadSerializer::Ptr> payloadSerializer_;
};

struct LegacySerializedRequest {
  /* implicit */ LegacySerializedRequest(std::unique_ptr<folly::IOBuf> buffer_)
      : buffer(std::move(buffer_)) {}

  LegacySerializedRequest(
      uint16_t protocolId,
      folly::StringPiece methodName,
      SerializedRequest&& serializedRequest);

  LegacySerializedRequest(
      uint16_t protocolId,
      int32_t seqid,
      folly::StringPiece methodName,
      SerializedRequest&& serializedRequest);

  std::unique_ptr<folly::IOBuf> buffer;
};

struct ResponsePayload {
  ResponsePayload() : buffer_(folly::IOBuf::createCombined(0)) {}

  std::unique_ptr<folly::IOBuf> buffer() && { return std::move(buffer_); }

  folly::IOBuf* buffer() & { return buffer_.get(); }

  void transform(
      std::vector<uint16_t>& writeTrans, size_t minCompressBytes = 0);

  explicit operator bool() const { return static_cast<bool>(buffer_); }

  std::size_t length() const { return buffer_->computeChainDataLength(); }

  static ResponsePayload create(std::unique_ptr<folly::IOBuf>&& buffer) {
    return ResponsePayload{std::move(buffer)};
  }

  // We will make this private after landing all changes to no longer require
  // it.
  /* implicit */ ResponsePayload(std::unique_ptr<folly::IOBuf> buffer)
      : buffer_(std::move(buffer)) {}

 private:
  friend struct SerializedResponse;
  friend struct LegacySerializedResponse;

  std::unique_ptr<folly::IOBuf> buffer_;
};

struct LegacySerializedResponse;

struct SerializedResponse {
  explicit SerializedResponse(
      std::unique_ptr<folly::IOBuf> buffer_ = std::unique_ptr<folly::IOBuf>{})
      : buffer(std::move(buffer_)) {}

  explicit SerializedResponse(
      LegacySerializedResponse&& legacyResponse, int16_t protocolId);

  ResponsePayload extractPayload(
      bool includeEnvelope,
      int16_t protocolId,
      int32_t seqId,
      MessageType mtype,
      folly::StringPiece methodName) &&;

  ResponsePayload extractPayload(bool includeEnvelope) &&;

  std::unique_ptr<folly::IOBuf> buffer;
};

struct LegacySerializedResponse {
  explicit LegacySerializedResponse(
      std::unique_ptr<folly::IOBuf> buffer_ = std::unique_ptr<folly::IOBuf>{})
      : buffer(std::move(buffer_)) {}

  LegacySerializedResponse(
      uint16_t protocolId,
      folly::StringPiece methodName,
      SerializedResponse&& serializedResponse);

  LegacySerializedResponse(
      uint16_t protocolId,
      int32_t seqid,
      folly::StringPiece methodName,
      SerializedResponse&& serializedResponse);

  LegacySerializedResponse(
      uint16_t protocolId,
      folly::StringPiece methodName,
      const TApplicationException& ex);

  LegacySerializedResponse(
      uint16_t protocolId,
      int32_t seqid,
      folly::StringPiece methodName,
      const TApplicationException& ex);

  LegacySerializedResponse(
      uint16_t protocolId,
      int32_t seqid,
      MessageType mtype,
      folly::StringPiece methodName,
      SerializedResponse&& serializedResponse);

  // The sequence Id is only overridden if a non-zero value is supplied
  std::pair<MessageType, ResponsePayload> extractPayload(
      bool includeEnvelope, int16_t protocolId, int32_t seqId = 0) &&;

  static std::unique_ptr<folly::IOBuf> envelope(
      uint16_t protocolId,
      MessageType mtype,
      int32_t seqid,
      folly::StringPiece methodName);

  std::unique_ptr<folly::IOBuf> buffer;
};

inline SerializedResponse::SerializedResponse(
    LegacySerializedResponse&& legacyResponse, int16_t protocolId) {
  auto [_, payload] =
      std::move(legacyResponse)
          .extractPayload(/*includeEnvelope=*/false, protocolId);
  buffer = std::move(payload).buffer();
}

} // namespace apache::thrift
