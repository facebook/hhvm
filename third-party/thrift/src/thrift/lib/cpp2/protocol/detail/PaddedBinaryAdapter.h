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

#include <cstdint>
#include <memory>
#include <type_traits>
#include <glog/logging.h>
#include <folly/GLog.h>
#include <folly/io/Cursor.h>
#include <folly/io/IOBuf.h>
#include <thrift/lib/cpp2/protocol/BinaryProtocol.h>

namespace apache::thrift::protocol {

/**
 * PaddedBinaryAdapter adds a  wrapper around IOBuf that prepends user specified
 * padding to the serialized data.
 * Using this without obeying the following rules will result in your data not
 * being serialized and deserialized correctly:
 *   1. This is only supported for binary data.
 *   2. Padding is only added when using binary protocol.
 *   3. The adapter must be used on the both the server and the client side to
 *      serialize and deserialize the data correctly.
 *   4. When rolling this feature out, the server must be upgraded first, so it
 *      can deserialize the data correctly, and then the client can be upgraded.
 */
struct PaddedBinaryData {
  // Magic translates to PaddedV1 in ASCII
  static constexpr uint64_t kMagic = 0x5061646465645631;
  static constexpr uint32_t kPaddingHeaderBytes =
      sizeof(kMagic) + sizeof(uint32_t);

  PaddedBinaryData() = default;
  ~PaddedBinaryData() = default;

  PaddedBinaryData(const PaddedBinaryData& other)
      : paddingBytes(other.paddingBytes), buf(other.buf->clone()) {}

  PaddedBinaryData& operator=(const PaddedBinaryData& other) {
    this->paddingBytes = other.paddingBytes;
    this->buf = other.buf->clone();
    return *this;
  }

  PaddedBinaryData(PaddedBinaryData&& other) noexcept
      : paddingBytes(other.paddingBytes), buf(std::move(other.buf)) {}

  PaddedBinaryData& operator=(PaddedBinaryData&& other) noexcept {
    this->paddingBytes = other.paddingBytes;
    this->buf = std::move(other.buf);
    return *this;
  }

  explicit PaddedBinaryData(
      uint32_t paddingBytes, std::unique_ptr<folly::IOBuf>&& data)
      : paddingBytes(paddingBytes), buf(std::move(data)) {}

  uint32_t paddingBytes{0};
  std::unique_ptr<folly::IOBuf> buf{nullptr};
};

struct PaddedBinaryAdapter {
  static PaddedBinaryData fromThrift(const std::string& data);
  static std::string toThrift(const PaddedBinaryData& data);

  template <typename Tag, typename Protocol>
  static uint32_t encode(Protocol& prot, const PaddedBinaryData& data) {
    // Padding is only supported for binary protocol.
    if constexpr (std::is_same_v<
                      std::remove_cv_t<Protocol>,
                      BinaryProtocolWriter>) {
      FB_LOG_ONCE(INFO) << "Using PaddedBinaryAdapter with binary protocol";

      // We only add padding if:
      // 1. The data buffer exists and is not empty
      // 2. There is a valid padding to be added
      if (data.paddingBytes > 0 && data.buf != nullptr &&
          data.buf->computeChainDataLength() > 0) {
        // We'll add the magic and the padding efficiently using a new IOBuf.
        uint32_t paddingBufSize =
            PaddedBinaryData::kPaddingHeaderBytes + data.paddingBytes;
        auto paddedDataBuf = folly::IOBuf::create(paddingBufSize);
        paddedDataBuf->append(paddingBufSize);

        folly::io::RWPrivateCursor cursor(paddedDataBuf.get());
        // Write the magic first
        cursor.writeBE<uint64_t>(PaddedBinaryData::kMagic);
        // Write the padding field size
        cursor.writeBE<uint32_t>(data.paddingBytes);
        // The remaining bytes are the padding.

        // Append the data buf to the padding buf
        paddedDataBuf->appendToChain(data.buf->clone());

        // Write the padded buf using the protocol
        return prot.writeBinary(paddedDataBuf);
      }
    }

    FB_LOG_ONCE(WARNING)
        << "Using PaddedBinaryAdapter with unsupported protocol";

    return prot.writeBinary(data.buf);
  }

  template <typename Tag, typename Protocol>
  static void decode(Protocol& prot, PaddedBinaryData& data) {
    // Read the data using the protocol.
    prot.readBinary(data.buf);

    // Padding is only supported for binary protocol.
    if constexpr (std::is_same_v<
                      std::remove_cv_t<Protocol>,
                      BinaryProtocolReader>) {
      FB_LOG_ONCE(INFO) << "Using PaddedBinaryAdapter with binary protocol";

      // Not enough data to read the padding header.
      if (data.buf->computeChainDataLength() <
          PaddedBinaryData::kPaddingHeaderBytes) {
        return;
      }

      DCHECK(data.buf != nullptr);
      folly::io::Cursor cursor(data.buf.get());

      // Parse magic to see if the buffer was indeed padded.
      uint64_t magic = cursor.readBE<uint64_t>();
      DCHECK(magic == PaddedBinaryData::kMagic)
          << std::hex << "Magic mismatch: exptected 0x"
          << PaddedBinaryData::kMagic << " got 0x" << magic;
      if (magic != PaddedBinaryData::kMagic) {
        // Not a padded binary, just return the data
        FB_LOG_ONCE(WARNING)
            << "Magic mismatch for PaddedBinaryAdapter, returning data as-is";
        return;
      }

      // Read the padding bytes next
      data.paddingBytes = cursor.readBE<uint32_t>();
      uint32_t extraBytes =
          PaddedBinaryData::kPaddingHeaderBytes + data.paddingBytes;
      DCHECK(extraBytes <= data.buf->computeChainDataLength());
      // Invalid padding, just return the data
      if (data.paddingBytes > data.buf->computeChainDataLength()) {
        return;
      }

      // Move the data pointer to the start of the actual user data
      data.buf->trimStart(extraBytes);
    } else {
      FB_LOG_ONCE(WARNING)
          << "Using PaddedBinaryAdapter with unsupported protocol";
    }
  }

  template <bool ZC, typename Tag, typename Protocol, typename T>
  static uint32_t serializedSize(Protocol&, const PaddedBinaryData& data) {
    return ZC ? 0 : data.buf->computeChainDataLength();
  }
};

} // namespace apache::thrift::protocol
