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

#include <algorithm>
#include <chrono>
#include <exception>
#include <memory>
#include <utility>

#include <folly/ExceptionString.h>
#include <folly/ExceptionWrapper.h>
#include <folly/io/Cursor.h>
#include <folly/io/IOBuf.h>
#include <folly/io/async/AsyncSocketException.h>
#include <folly/io/async/DelayedDestruction.h>

#include <thrift/lib/cpp/transport/TTransportException.h>
#include <thrift/lib/cpp2/transport/rocket/framing/FrameType.h>
#include <thrift/lib/cpp2/transport/rocket/framing/Frames.h>
#include <thrift/lib/cpp2/transport/rocket/framing/Serializer.h>
#include <thrift/lib/cpp2/transport/rocket/framing/Util.h>
#include <thrift/lib/cpp2/transport/rocket/framing/parser/AllocatingParserStrategy.h>
#include <thrift/lib/cpp2/transport/rocket/framing/parser/FrameLengthParserStrategy.h>
#include <thrift/lib/cpp2/transport/rocket/framing/parser/ParserStrategy.h>

namespace apache {
namespace thrift {
namespace rocket {
template <class T>
void Parser<T>::getReadBufferOld(void** bufout, size_t* lenout) {
  DCHECK(!readBuffer_.isChained());
  if (LIKELY(
          allocType_ ==
          apache::thrift::RpcOptions::MemAllocType::ALLOC_DEFAULT)) {
    readBuffer_.unshareOne();
    if (readBuffer_.length() == 0) {
      DCHECK(readBuffer_.capacity() > 0);
      // If we read everything, reset pointers to 0 and reuse the buffer
      readBuffer_.clear();
    } else if (readBuffer_.headroom() > 0) {
      // Move partially read data to the beginning
      readBuffer_.retreat(readBuffer_.headroom());
    }
  }
  *bufout = readBuffer_.writableTail();
  *lenout = readBuffer_.tailroom();
}

template <class T>
void Parser<T>::getReadBufferNew(void** bufout, size_t* lenout) {
  const auto ret = readBufQueue_.preallocate(bufferSize_, kMaxBufferSize);
  *bufout = ret.first;
  *lenout = ret.second;
  return;
}

template <class T>
void Parser<T>::getReadBufferHybrid(void** bufout, size_t* lenout) {
  // if dynamic buffer is not null, read the remainder of currentFrameLength_
  // into it, so it contains exactly one full frame
  if (dynamicBuffer_) {
    *bufout = dynamicBuffer_->writableTail();
    *lenout = currentFrameLength_ - dynamicBuffer_->length();
  } else {
    if (!readBuffer_.isSharedOne()) {
      // without external refs, we can move data (same as clear() if length==0)
      readBuffer_.retreat(readBuffer_.headroom());
    } else if (reallocateIfShared_) {
      auto buf = folly::IOBuf(folly::IOBuf::CreateOp(), kStaticBufferSize);
      memcpy(buf.writableData(), readBuffer_.data(), readBuffer_.length());
      buf.append(readBuffer_.length());
      readBuffer_ = std::move(buf);
    }
    reallocateIfShared_ = false;
    *bufout = readBuffer_.writableTail();
    *lenout = readBuffer_.tailroom();
  }
}

template <class T>
void Parser<T>::readDataAvailableOld(size_t nbytes) {
  readBuffer_.append(nbytes);

  while (!readBuffer_.empty()) {
    if (readBuffer_.length() < Serializer::kMinimumFrameHeaderLength) {
      return;
    }

    folly::io::Cursor cursor(&readBuffer_);
    const size_t totalFrameSize = Serializer::kBytesForFrameOrMetadataLength +
        readFrameOrMetadataSize(cursor);

    if (!currentFrameLength_) {
      if (!owner_.incMemoryUsage(totalFrameSize)) {
        return;
      }
      currentFrameLength_ = totalFrameSize;
    }

    readStreamId(cursor);
    uint8_t frameType;
    std::tie(frameType, std::ignore) = readFrameTypeAndFlagsUnsafe(cursor);
    if (UNLIKELY(
            static_cast<FrameType>(frameType) == FrameType::EXT &&
            allocType_ ==
                apache::thrift::RpcOptions::MemAllocType::ALLOC_DEFAULT)) {
      if (readBuffer_.length() < Serializer::kBytesForFrameOrMetadataLength +
              ExtFrame::frameHeaderSize()) {
        return;
      }

      ExtFrameType extType = readExtFrameType(cursor);
      if (UNLIKELY(extType == ExtFrameType::ALIGNED_PAGE)) {
        if (alignTo4k(
                readBuffer_,
                Serializer::kBytesForFrameOrMetadataLength +
                    ExtFrame::frameHeaderSize(),
                totalFrameSize)) {
          allocType_ =
              apache::thrift::RpcOptions::MemAllocType::ALLOC_PAGE_ALIGN;
        }
      }
    }

    if (readBuffer_.length() < totalFrameSize) {
      if (readBuffer_.length() + readBuffer_.tailroom() < totalFrameSize) {
        DCHECK(!readBuffer_.isChained());
        readBuffer_.unshareOne();
        bufferSize_ = std::max<size_t>(bufferSize_, totalFrameSize);
        readBuffer_.reserve(
            0 /* minHeadroom */,
            bufferSize_ - readBuffer_.length() /* minTailroom */);
      }
      return;
    }

    // Otherwise, we have a full frame to handle.
    const size_t bytesToClone =
        totalFrameSize - Serializer::kBytesForFrameOrMetadataLength;
    cursor.reset(&readBuffer_);
    readFrameOrMetadataSize(cursor);
    std::unique_ptr<folly::IOBuf> frame;
    cursor.clone(frame, bytesToClone);
    owner_.decMemoryUsage(currentFrameLength_);
    currentFrameLength_ = 0;
    readBuffer_.trimStart(totalFrameSize);
    allocType_ = apache::thrift::RpcOptions::MemAllocType::ALLOC_DEFAULT;
    owner_.handleFrame(std::move(frame));
  }

  if (!isScheduled() && bufferSize_ > kMaxBufferSize) {
    owner_.scheduleTimeout(this, kDefaultBufferResizeInterval);
  }
}

template <class T>
void Parser<T>::readDataAvailableNew(size_t nbytes) {
  readBufQueue_.postallocate(nbytes);

  while (!readBufQueue_.empty()) {
    if (readBufQueue_.chainLength() < Serializer::kMinimumFrameHeaderLength) {
      return;
    }
    folly::io::Cursor cursor(readBufQueue_.front());
    const size_t totalFrameSize = Serializer::kBytesForFrameOrMetadataLength +
        readFrameOrMetadataSize(cursor);

    if (!currentFrameLength_) {
      if (!owner_.incMemoryUsage(totalFrameSize)) {
        return;
      }
      currentFrameLength_ = totalFrameSize;
    }

    readStreamId(cursor);
    uint8_t frameType;
    std::tie(frameType, std::ignore) = readFrameTypeAndFlagsUnsafe(cursor);
    if (UNLIKELY(
            static_cast<FrameType>(frameType) == FrameType::EXT &&
            allocType_ ==
                apache::thrift::RpcOptions::MemAllocType::ALLOC_DEFAULT)) {
      if (readBufQueue_.chainLength() <
          Serializer::kBytesForFrameOrMetadataLength +
              ExtFrame::frameHeaderSize()) {
        return;
      }

      ExtFrameType extType = readExtFrameType(cursor);
      if (UNLIKELY(extType == ExtFrameType::ALIGNED_PAGE)) {
        if (alignTo4kBufQueue(
                readBufQueue_,
                Serializer::kBytesForFrameOrMetadataLength +
                    ExtFrame::frameHeaderSize(),
                totalFrameSize)) {
          allocType_ =
              apache::thrift::RpcOptions::MemAllocType::ALLOC_PAGE_ALIGN;
        }
      }
    }

    if (readBufQueue_.chainLength() < currentFrameLength_) {
      bufferSize_ = currentFrameLength_ - readBufQueue_.chainLength();
      return;
    }
    // Otherwise, we have a full frame to handle.
    readBufQueue_.trimStart(Serializer::kBytesForFrameOrMetadataLength);
    auto frame = readBufQueue_.split(
        currentFrameLength_ - Serializer::kBytesForFrameOrMetadataLength);
    owner_.handleFrame(std::move(frame));
    owner_.decMemoryUsage(currentFrameLength_);
    currentFrameLength_ = 0;
    bufferSize_ = kMinBufferSize;
    allocType_ = apache::thrift::RpcOptions::MemAllocType::ALLOC_DEFAULT;
  }
}

template <class T>
void Parser<T>::readDataAvailableHybrid(size_t nbytes) {
  if (dynamicBuffer_) {
    dynamicBuffer_->append(nbytes);
    if (dynamicBuffer_->length() < currentFrameLength_) {
      return;
    }
    DCHECK_EQ(dynamicBuffer_->length(), currentFrameLength_);
    owner_.handleFrame(std::move(dynamicBuffer_));
    owner_.decMemoryUsage(currentFrameLength_);
    currentFrameLength_ = 0;
    currentFrameType_ = 0;
    return;
  }
  // set reallocate hint if latest read filled most of the buffer
  DCHECK_LE(nbytes, readBuffer_.tailroom());
  reallocateIfShared_ = readBuffer_.tailroom() - nbytes < kReallocateThreshold;
  readBuffer_.append(nbytes);
  while (!readBuffer_.empty()) {
    const size_t bufLen = readBuffer_.length();
    if (bufLen < Serializer::kMinimumFrameHeaderLength) {
      return;
    }

    folly::io::Cursor cursor(&readBuffer_);
    if (!currentFrameLength_) {
      auto frameLength = readFrameOrMetadataSize(cursor);
      if (!owner_.incMemoryUsage(frameLength)) {
        return;
      }
      currentFrameLength_ = frameLength;
      // skip over stream ID
      cursor.skipNoAdvance(sizeof(StreamId::underlying_type));
      // read frameType and ignore flags
      std::tie(currentFrameType_, std::ignore) =
          readFrameTypeAndFlagsUnsafe(cursor);
    } else {
      cursor.skipNoAdvance(Serializer::kMinimumFrameHeaderLength);
    }
    const size_t totalSize =
        currentFrameLength_ + Serializer::kBytesForFrameOrMetadataLength;

    if (UNLIKELY(
            static_cast<FrameType>(currentFrameType_) == FrameType::EXT &&
            allocType_ ==
                apache::thrift::RpcOptions::MemAllocType::ALLOC_DEFAULT)) {
      if (bufLen < Serializer::kBytesForFrameOrMetadataLength +
              ExtFrame::frameHeaderSize()) {
        return;
      }
      ExtFrameType extType = readExtFrameType(cursor);
      if (UNLIKELY(extType == ExtFrameType::ALIGNED_PAGE)) {
        allocType_ = apache::thrift::RpcOptions::MemAllocType::ALLOC_PAGE_ALIGN;
        const size_t bytesToCopy = std::min(
            currentFrameLength_,
            readBuffer_.length() - Serializer::kBytesForFrameOrMetadataLength);
        dynamicBuffer_ = get4kAlignedBuf(
            currentFrameLength_, ExtFrame::frameHeaderSize(), bytesToCopy);
        if (LIKELY(dynamicBuffer_ != nullptr)) {
          allocType_ = apache::thrift::RpcOptions::MemAllocType::ALLOC_DEFAULT;
          readBuffer_.trimStart(Serializer::kBytesForFrameOrMetadataLength);
          memcpy(
              dynamicBuffer_->writableData(), readBuffer_.data(), bytesToCopy);
          readBuffer_.trimStart(bytesToCopy);
          // if we had the full frame, send it right away and continue loop
          if (bytesToCopy == currentFrameLength_) {
            owner_.handleFrame(std::move(dynamicBuffer_));
            owner_.decMemoryUsage(currentFrameLength_);
            currentFrameLength_ = 0;
            currentFrameType_ = 0;
            continue;
          }
          // otherwise, return to read rest of frame into dynamic buffer
          return;
        }
      }
    }

    // we may have an incomplete frame
    if (totalSize > bufLen) {
      // switch to dynamic buffer only if there is no way to fit the whole frame
      // into the static buffer
      if (totalSize - bufLen > readBuffer_.tailroom() &&
          LIKELY(totalSize > kStaticBufferSize || readBuffer_.isSharedOne())) {
        dynamicBuffer_ = folly::IOBuf::createCombined(currentFrameLength_);
        readBuffer_.trimStart(Serializer::kBytesForFrameOrMetadataLength);
        memcpy(
            dynamicBuffer_->writableData(),
            readBuffer_.data(),
            readBuffer_.length());
        dynamicBuffer_->append(readBuffer_.length());
        // "free" the data we just copied in readBuffer_
        readBuffer_.prepend(Serializer::kBytesForFrameOrMetadataLength);
        readBuffer_.trimEnd(readBuffer_.length());
      }
      return;
    }

    // otherwise, we have a full frame
    cursor.reset(&readBuffer_);
    cursor.skipNoAdvance(Serializer::kBytesForFrameOrMetadataLength);
    std::unique_ptr<folly::IOBuf> frame;
    cursor.clone(frame, currentFrameLength_);
    readBuffer_.trimStart(totalSize);
    allocType_ = apache::thrift::RpcOptions::MemAllocType::ALLOC_DEFAULT;
    owner_.handleFrame(std::move(frame));
    owner_.decMemoryUsage(currentFrameLength_);
    currentFrameLength_ = 0;
    currentFrameType_ = 0;
  }
}

template <class T>
void Parser<T>::getReadBuffer(void** bufout, size_t* lenout) {
  blockResize_ = true;
  if (useStrategyParser_) {
    frameLengthParser_->getReadBuffer(bufout, lenout);
  } else if (useAllocatingStrategyParser_) {
    allocatingParser_->getReadBuffer(bufout, lenout);
  } else if (newBufferLogicEnabled_) {
    getReadBufferNew(bufout, lenout);
  } else if (hybridBufferLogicEnabled_) {
    getReadBufferHybrid(bufout, lenout);
  } else {
    getReadBufferOld(bufout, lenout);
  }
}

template <class T>
void Parser<T>::readDataAvailable(size_t nbytes) noexcept {
  folly::DelayedDestruction::DestructorGuard dg(&this->owner_);
  blockResize_ = false;
  try {
    if (useStrategyParser_) {
      frameLengthParser_->readDataAvailable(nbytes);
    } else if (useAllocatingStrategyParser_) {
      allocatingParser_->readDataAvailable(nbytes);
    } else if (newBufferLogicEnabled_) {
      readDataAvailableNew(nbytes);
    } else if (hybridBufferLogicEnabled_) {
      readDataAvailableHybrid(nbytes);
    } else {
      readDataAvailableOld(nbytes);
    }
  } catch (...) {
    auto exceptionStr =
        folly::exceptionStr(std::current_exception()).toStdString();
    LOG(ERROR) << "Bad frame received, closing connection: " << exceptionStr;
    owner_.close(transport::TTransportException(exceptionStr));
  }
}

template <class T>
void Parser<T>::readEOF() noexcept {
  folly::DelayedDestruction::DestructorGuard dg(&this->owner_);

  blockResize_ = false;
  owner_.close(transport::TTransportException(
      transport::TTransportException::TTransportExceptionType::END_OF_FILE,
      "Channel got EOF. Check for server hitting connection limit, "
      "connection age timeout, server connection idle timeout, and server crashes."));
}

template <class T>
void Parser<T>::readErr(const folly::AsyncSocketException& ex) noexcept {
  folly::DelayedDestruction::DestructorGuard dg(&this->owner_);
  blockResize_ = false;
  owner_.close(transport::TTransportException(ex));
}

// TODO: This should be removed once the new buffer logic controlled by
// THRIFT_FLAG(rocket_parser_dont_hold_buffer_enabled) is stable.
template <class T>
void Parser<T>::timeoutExpired() noexcept {
  if (LIKELY(
          allocType_ ==
              apache::thrift::RpcOptions::MemAllocType::ALLOC_DEFAULT &&
          !blockResize_)) {
    resizeBuffer();
  }
}

template <class T>
void Parser<T>::readBufferAvailable(
    std::unique_ptr<folly::IOBuf> buf) noexcept {
  folly::DelayedDestruction::DestructorGuard dg(&this->owner_);
  try {
    if (useStrategyParser_) {
      frameLengthParser_->readBufferAvailable(std::move(buf));
    } else if (useAllocatingStrategyParser_) {
      // Will throw not implemented runtime exception
      allocatingParser_->readBufferAvailable(std::move(buf));
    } else {
      readBufQueue_.append(std::move(buf));
      while (!readBufQueue_.empty()) {
        if (readBufQueue_.chainLength() <
            Serializer::kBytesForFrameOrMetadataLength) {
          return;
        }
        folly::io::Cursor cursor(readBufQueue_.front());

        if (!currentFrameLength_) {
          currentFrameLength_ = Serializer::kBytesForFrameOrMetadataLength +
              readFrameOrMetadataSize(cursor);
          if (!owner_.incMemoryUsage(currentFrameLength_)) {
            currentFrameLength_ = 0;
            return;
          }
        }

        if (readBufQueue_.chainLength() < currentFrameLength_) {
          return;
        }

        readBufQueue_.trimStart(Serializer::kBytesForFrameOrMetadataLength);
        auto frame = readBufQueue_.split(
            currentFrameLength_ - Serializer::kBytesForFrameOrMetadataLength);
        owner_.handleFrame(std::move(frame));
        owner_.decMemoryUsage(currentFrameLength_);
        currentFrameLength_ = 0;
      }
    }
  } catch (...) {
    auto exceptionStr =
        folly::exceptionStr(std::current_exception()).toStdString();
    LOG(ERROR) << "Bad frame received, closing connection: " << exceptionStr;
    owner_.close(transport::TTransportException(exceptionStr));
  }
}

// TODO: This should be removed once the new buffer logic controlled by
// THRIFT_FLAG(rocket_parser_dont_hold_buffer_enabled) is stable.
template <class T>
void Parser<T>::resizeBuffer() {
  if (bufferSize_ <= kMaxBufferSize || readBuffer_.length() >= kMaxBufferSize) {
    return;
  }
  // resize readBuffer_ to kMaxBufferSize
  readBuffer_ = folly::IOBuf(
      folly::IOBuf::CopyBufferOp(),
      readBuffer_.data(),
      readBuffer_.length(),
      /* headroom */ 0,
      /* tailroom */ kMaxBufferSize - readBuffer_.length());
  bufferSize_ = kMaxBufferSize;
}

template <class T>
constexpr size_t Parser<T>::kMinBufferSize;
template <class T>
constexpr size_t Parser<T>::kMaxBufferSize;
// TODO: This should be removed once the new buffer logic controlled by
// THRIFT_FLAG(rocket_parser_dont_hold_buffer_enabled) is stable.
template <class T>
constexpr std::chrono::milliseconds Parser<T>::kDefaultBufferResizeInterval;
template <class T>
constexpr size_t Parser<T>::kStaticBufferSize;
template <class T>
constexpr size_t Parser<T>::kReallocateThreshold;

} // namespace rocket
} // namespace thrift
} // namespace apache
