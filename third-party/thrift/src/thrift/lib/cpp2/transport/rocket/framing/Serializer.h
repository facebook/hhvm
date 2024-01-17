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

#include <memory>
#include <type_traits>

#include <folly/CPortability.h>
#include <folly/Optional.h>
#include <folly/io/Cursor.h>
#include <folly/io/IOBuf.h>
#include <folly/io/IOBufQueue.h>
#include <folly/lang/Bits.h>

#include <thrift/lib/cpp2/transport/rocket/Types.h>
#include <thrift/lib/cpp2/transport/rocket/framing/Flags.h>
#include <thrift/lib/cpp2/transport/rocket/framing/FrameType.h>

namespace apache {
namespace thrift {
namespace rocket {

class Serializer {
 public:
  static constexpr size_t kBytesForFrameOrMetadataLength = 3;
  static constexpr size_t kMinimumFrameHeaderLength = 9;

  Serializer() { queue_.append(folly::IOBuf::create(kQueueAppenderChunkSize)); }

  // All data in rsocket protocol is transmitted in Big Endian format.
  template <class T>
  std::enable_if_t<std::is_arithmetic<T>::value, size_t> writeBE(T value) {
    const auto bigEndianValue = folly::Endian::big(value);
    const uint8_t* bytes = reinterpret_cast<const uint8_t*>(&bigEndianValue);
    return push(bytes, sizeof(T));
  }

  size_t writeFrameTypeAndFlags(FrameType frameType, Flags flags) {
    return writeBE<uint16_t>(
        (static_cast<uint16_t>(frameType) << Flags::kBits) |
        static_cast<uint16_t>(flags));
  }

  size_t write(StreamId streamId) {
    return writeBE(static_cast<uint32_t>(streamId));
  }

  size_t write(const folly::IOBuf& buf) {
    appender_.insert(buf);
    return buf.computeChainDataLength();
  }

  size_t write(std::unique_ptr<folly::IOBuf> buf) {
    auto r = buf->computeChainDataLength();
    appender_.insert(std::move(buf));
    return r;
  }

  size_t write(folly::StringPiece sp) {
    appender_.push(reinterpret_cast<const uint8_t*>(sp.data()), sp.size());
    return sp.size();
  }

  size_t writeFrameOrMetadataSize(size_t nbytes) {
    // Frame and metadata lengths are BE-encoded in 3 bytes
    DCHECK_LT(nbytes, (1ull << (kBytesForFrameOrMetadataLength * 8)));

    const size_t beSize = folly::Endian::big(nbytes);
    const uint8_t* start = reinterpret_cast<const uint8_t*>(&beSize) +
        (sizeof(beSize) - kBytesForFrameOrMetadataLength);
    return push(start, kBytesForFrameOrMetadataLength);
  }

  size_t writePayload(Payload&& p);

  std::unique_ptr<folly::IOBuf> move() && { return queue_.move(); }

 private:
  static constexpr size_t kQueueAppenderChunkSize = 512;

  folly::IOBufQueue queue_{folly::IOBufQueue::cacheChainLength()};
  folly::io::QueueAppender appender_{&queue_, kQueueAppenderChunkSize};

  size_t push(const uint8_t* buf, size_t len) {
    appender_.push(buf, len);
    return len;
  }
};

class HeaderSerializer {
 public:
  HeaderSerializer(uint8_t* buf, size_t maxLen) : buf_(buf), maxLen_(maxLen) {}
  static constexpr size_t kBytesForFrameOrMetadataLength = 3;

  folly::ByteRange result() {
    if (pos_ <= maxLen_) {
      return {buf_, buf_ + pos_};
    }
    return {nullptr, nullptr};
  }

  template <class T>
  size_t writeBE(T value) {
    constexpr auto size = sizeof(T);
    if (FOLLY_LIKELY(canWrite(size))) {
      T big_endian_value = folly::Endian::big(value);
      std::memcpy(buf_ + pos_, &big_endian_value, size);
      incrementPosition(size);
      return size;
    }
    return 0;
  }

  size_t writeFrameTypeAndFlags(FrameType frameType, Flags flags) {
    return writeBE<uint16_t>(
        (static_cast<uint16_t>(frameType) << Flags::kBits) |
        static_cast<uint16_t>(flags));
  }

  size_t write(StreamId streamId) {
    return writeBE(static_cast<uint32_t>(streamId));
  }

  size_t writeFrameOrMetadataSize(size_t nbytes) {
    if (FOLLY_LIKELY(canWrite(kBytesForFrameOrMetadataLength))) {
      writeInt24BE(nbytes);
      incrementPosition(kBytesForFrameOrMetadataLength);
      return kBytesForFrameOrMetadataLength;
    }
    return 0;
  }

 private:
  FOLLY_ALWAYS_INLINE bool canWrite(size_t size) {
    return maxLen_ - pos_ >= size;
  }

  FOLLY_ALWAYS_INLINE void incrementPosition(size_t size) { pos_ += size; }

  FOLLY_ALWAYS_INLINE void writeInt24BE(size_t nbytes) {
    DCHECK_LT(nbytes, (1ull << (kBytesForFrameOrMetadataLength * 8)));
    buf_[pos_] = (nbytes >> 16) & 0xFF;
    buf_[pos_ + 1] = (nbytes >> 8) & 0xFF;
    buf_[pos_ + 2] = nbytes & 0xFF;
  }

  uint8_t* buf_;
  size_t pos_{0};
  size_t maxLen_;
};

} // namespace rocket
} // namespace thrift
} // namespace apache
