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

#include <folly/String.h>
#include <folly/io/Cursor.h>
#include <folly/io/IOBuf.h>
#include <folly/io/IOBufQueue.h>
#include <folly/logging/xlog.h>
#include <thrift/lib/cpp2/transport/rocket/framing/Serializer.h>
#include <thrift/lib/cpp2/transport/rocket/framing/Util.h>
#include <thrift/lib/cpp2/transport/rocket/framing/parser/ParserStrategy.h>

namespace apache::thrift::rocket {

namespace detail::aligned_parser {
/// AlignedParserStrategy is implemented as a state machine, with the
/// following states:
enum class State : uint8_t {
  AwaitingHeader,
  AwaitingMetadataLength,
  AwaitingMetadata,
  AwaitingData,
  AwaitingNonAligned,
};

} // namespace detail::aligned_parser

/**
 * ParserStrategy that allows for aligned user binary data if certain conditions
 * are met. The conditions are:
 * 1. The frame type is one of the following:
 *    - REQUEST_RESPONSE
 *    - PAYLOAD
 * 2. When the frame type is REQUEST_RESPONSE, the aligned field must be a the
 * first parameter of a Thrift Request, and binary.
 * 3. When the frame type is PAYLOAD, the aligned field must be a binary field
 * that is the first field of a Thrift Response Struct.
 * 4. The must not be fragmented.
 *
 */
template <typename T>
class AlignedParserStrategy {
  using State = detail::aligned_parser::State;

  // Length of the header, including the frame size.
  constexpr static auto kFrameSizeAndHeaderLength =
      Serializer::kMinimumFrameHeaderLength;

  // Length of the header, excluding the frame size.
  constexpr static auto kHeaderLength =
      kFrameSizeAndHeaderLength - Serializer::kBytesForFrameOrMetadataLength;

  // Total buffer size for header + metadata data size field
  constexpr static auto kHeaderBufferLength =
      // Frame Size Field and Header Length
      kFrameSizeAndHeaderLength +
      // Metadata Length
      Serializer::kBytesForFrameOrMetadataLength;

 public:
  explicit AlignedParserStrategy(T& owner)
      : state_(State::AwaitingHeader),
        owner_(owner),
        header_(createHeaderBuffer()),
        metadata_(nullptr),
        data_(nullptr) {}

  void getReadBuffer(void** bufReturn, size_t* lenReturn);
  void readDataAvailable(size_t len);
  void readBufferAvailable(std::unique_ptr<folly::IOBuf>) {
    XLOG(FATAL) << "readBufferAvailable() not supported";
  }
  bool isBufferMovable();
  State state() const { return state_; }
  size_t remainingHeader() const { return remainingHeader_; }
  size_t remainingMetadata() const { return remainingMetadata_; }
  size_t remainingData() const { return remainingData_; }
  size_t remainingUnaligned() const { return remainingUnaligned_; }

 private:
  State state_;
  T& owner_;
  size_t remainingHeader_{kFrameSizeAndHeaderLength};
  size_t remainingMetadata_{0};
  size_t remainingData_{0};
  size_t frameLength_{0};
  size_t remainingUnaligned_{0};
  FrameType frameType_{FrameType::RESERVED};
  Flags flags_{};
  std::unique_ptr<folly::IOBuf> header_;
  std::unique_ptr<folly::IOBuf> metadata_;
  std::unique_ptr<folly::IOBuf> data_;
  folly::IOBufQueue nonAlignedData_{folly::IOBufQueue::cacheChainLength()};

  std::unique_ptr<folly::IOBuf> createHeaderBuffer();
  std::unique_ptr<folly::IOBuf> createDataBuffer();
  std::unique_ptr<folly::IOBuf> createRequestResponseDataBuffer();
  std::unique_ptr<folly::IOBuf> createPayloadDataBuffer();
  std::unique_ptr<folly::IOBuf> createAlignedBuffer(
      size_t size, size_t padding);

  void handleAwaitingHeader(size_t len);

  void configureAligned();
  void configuredNonAligned();

  void handleAwaitingMetadataLength(size_t len);
  void handleAwaitingMetadata(size_t len);
  void handleAwaitingData(size_t len);
  void handleNonAligned(size_t len);

  void parseHeader();

  void submitFrame();
  void submitIOBufQueueFrame();
  void resetToInitialState();
};

template <typename T>
std::unique_ptr<folly::IOBuf> AlignedParserStrategy<T>::createHeaderBuffer() {
  return folly::IOBuf::createCombined(kHeaderBufferLength);
}

template <typename T>
std::unique_ptr<folly::IOBuf> AlignedParserStrategy<T>::createAlignedBuffer(
    const size_t size, const size_t padding) {
  // We need to allocate an extra byte for the IOBuf to be able to
  // make sure the first field in a binary encoded thrift struct is
  // aligned. There are 7 bytes between the start of the first field and
  // the start of the struct.
  auto buffer = folly::IOBuf::createSeparate(size + padding);
  buffer->advance(padding);
  return buffer;
}

template <typename T>
std::unique_ptr<folly::IOBuf>
AlignedParserStrategy<T>::createRequestResponseDataBuffer() {
  return createAlignedBuffer(remainingData_, 4);
}

template <typename T>
std::unique_ptr<folly::IOBuf>
AlignedParserStrategy<T>::createPayloadDataBuffer() {
  return createAlignedBuffer(remainingData_, 1);
}

template <typename T>
std::unique_ptr<folly::IOBuf> AlignedParserStrategy<T>::createDataBuffer() {
  if (frameType_ == FrameType::PAYLOAD) {
    return createPayloadDataBuffer();
  } else {
    return createRequestResponseDataBuffer();
  }
}

template <typename T>
void AlignedParserStrategy<T>::getReadBuffer(
    void** bufReturn, size_t* lenReturn) {
  switch (state_) {
    case State::AwaitingHeader:
    case State::AwaitingMetadataLength: {
      *bufReturn = header_->writableTail();
      *lenReturn = remainingHeader_;
      break;
    }
    case State::AwaitingMetadata: {
      *bufReturn = metadata_->writableTail();
      *lenReturn = remainingMetadata_;
      break;
    }
    case State::AwaitingData: {
      *bufReturn = data_->writableTail();
      *lenReturn = remainingData_;
      break;
    }
    case State::AwaitingNonAligned: {
      *bufReturn = nonAlignedData_.writableTail();
      *lenReturn = remainingUnaligned_;
      break;
    }
  }
}

template <typename T>
void AlignedParserStrategy<T>::readDataAvailable(size_t len) {
  switch (state_) {
    case State::AwaitingHeader:
      handleAwaitingHeader(len);
      break;
    case State::AwaitingMetadataLength:
      handleAwaitingMetadataLength(len);
      break;
    case State::AwaitingMetadata:
      handleAwaitingMetadata(len);
      break;
    case State::AwaitingData:
      handleAwaitingData(len);
      break;
    case State::AwaitingNonAligned:
      handleNonAligned(len);
      break;
  }
}

template <typename T>
void AlignedParserStrategy<T>::handleAwaitingHeader(size_t len) {
  FOLLY_SAFE_DCHECK(state_ == State::AwaitingHeader);
  header_->append(len);
  remainingHeader_ -= len;
  if (remainingHeader_ == 0) {
    parseHeader();
    switch (frameType_) {
      // Only support aligned data for these frame types
      case FrameType::PAYLOAD:
      case FrameType::REQUEST_RESPONSE:
        configureAligned();
        break;
      // Cancel has no data, so we can submit the frame
      case FrameType::CANCEL:
        submitFrame();
        break;
      // All other frame types are unaligned so we use the IOBufQueue
      case FrameType::RESERVED:
      case FrameType::SETUP:
      case FrameType::KEEPALIVE:
      case FrameType::REQUEST_FNF:
      case FrameType::REQUEST_STREAM:
      case FrameType::REQUEST_CHANNEL:
      case FrameType::REQUEST_N:
      case FrameType::ERROR:
      case FrameType::METADATA_PUSH:
      case FrameType::EXT:
        configuredNonAligned();
        break;
    }
  }
}

template <typename T>
void AlignedParserStrategy<T>::configureAligned() {
  if (flags_.metadata()) { // has metadata, need to get the length
    remainingHeader_ += Serializer::kBytesForFrameOrMetadataLength;
    state_ = State::AwaitingMetadataLength;
  } else { // no metadata, so we can get the data
    remainingData_ = frameLength_ - kHeaderLength;
    data_ = createDataBuffer();
    state_ = State::AwaitingData;
  }
}

template <typename T>
void AlignedParserStrategy<T>::configuredNonAligned() {
  state_ = State::AwaitingNonAligned;
  nonAlignedData_.clearAndTryReuseLargestBuffer();
  nonAlignedData_.append(std::exchange(header_, createHeaderBuffer()));
  remainingUnaligned_ = frameLength_ - kHeaderLength;
  nonAlignedData_.preallocate(remainingUnaligned_, remainingUnaligned_);
}

template <typename T>
void AlignedParserStrategy<T>::handleAwaitingMetadataLength(size_t len) {
  FOLLY_SAFE_DCHECK(state_ == State::AwaitingMetadataLength);
  header_->append(len);
  remainingHeader_ -= len;

  if (remainingHeader_ == 0) {
    state_ = State::AwaitingMetadata;

    remainingMetadata_ =
        readFrameOrMetadataSize(header_->data() + kFrameSizeAndHeaderLength);

    remainingData_ = frameLength_ // Overall Frame Length
        - kHeaderLength // Minus the header length
        - Serializer::kBytesForFrameOrMetadataLength // Minus the metadata
                                                     // length field size
        - remainingMetadata_; // Minus the metadata length
    metadata_ = folly::IOBuf::createCombined(remainingMetadata_);
  }
}

template <typename T>
void AlignedParserStrategy<T>::handleAwaitingMetadata(size_t len) {
  FOLLY_SAFE_DCHECK(state_ == State::AwaitingMetadata);
  metadata_->append(len);
  remainingMetadata_ -= len;
  if (remainingMetadata_ == 0) {
    if (remainingData_ > 0) {
      state_ = State::AwaitingData;
      data_ = createDataBuffer();
    } else {
      submitFrame();
    }
  }
}

template <typename T>
void AlignedParserStrategy<T>::handleAwaitingData(size_t len) {
  FOLLY_SAFE_DCHECK(state_ == State::AwaitingData);
  data_->append(len);
  remainingData_ -= len;
  if (remainingData_ == 0) {
    submitFrame();
  }
}

template <typename T>
void AlignedParserStrategy<T>::handleNonAligned(size_t len) {
  FOLLY_SAFE_DCHECK(state_ == State::AwaitingNonAligned);
  nonAlignedData_.postallocate(len);
  remainingUnaligned_ -= len;
  if (remainingUnaligned_ == 0) {
    submitIOBufQueueFrame();
  }
}

template <typename T>
bool AlignedParserStrategy<T>::isBufferMovable() {
  return false;
}

template <typename T>
void AlignedParserStrategy<T>::parseHeader() {
  folly::io::Cursor cursor{header_.get()};
  frameLength_ = readFrameOrMetadataSize(cursor);
  cursor.skip(sizeof(StreamId));
  auto [frameType, flags] = readFrameTypeAndFlagsUnsafe(cursor);
  frameType_ = static_cast<FrameType>(frameType);
  flags_ = flags;
}

template <typename T>
void AlignedParserStrategy<T>::submitFrame() {
  auto old = std::exchange(header_, std::move(createHeaderBuffer()));

  XLOG(DBG9) << "pre apending old header:\n"
             << folly::hexDump(old->data(), old->computeChainDataLength())
             << std::endl;

  if (metadata_ != nullptr) {
    old->appendToChain(std::exchange(metadata_, nullptr));
  }

  if (data_ != nullptr) {
    old->appendToChain(std::exchange(data_, nullptr));
  }

  XLOG(DBG9) << "raw frame after appending:\n"
             << folly::hexDump(
                    old->cloneCoalesced()->buffer(),
                    old->cloneCoalesced()->capacity())
             << std::endl;

  XLOG(DBG9) << "frame after appending:\n"
             << folly::hexDump(
                    old->cloneCoalesced()->data(),
                    old->computeChainDataLength())
             << std::endl;

  old->trimStart(Serializer::kBytesForFrameOrMetadataLength);

  XLOG(DBG9) << "frame after trimming:\n"
             << folly::hexDump(
                    old->cloneCoalesced()->data(),
                    old->computeChainDataLength())
             << std::endl;

  owner_.handleFrame(std::move(old));
  resetToInitialState();
}

template <typename T>
void AlignedParserStrategy<T>::submitIOBufQueueFrame() {
  nonAlignedData_.trimStart(Serializer::kBytesForFrameOrMetadataLength);
  auto frame = nonAlignedData_.split(frameLength_);
  owner_.handleFrame(std::move(frame));
  resetToInitialState();
}

template <typename T>
void AlignedParserStrategy<T>::resetToInitialState() {
  state_ = State::AwaitingHeader;
  remainingHeader_ = Serializer::kMinimumFrameHeaderLength;
  remainingMetadata_ = 0;
  remainingData_ = 0;
  frameLength_ = 0;
  remainingUnaligned_ = 0;
}

} // namespace apache::thrift::rocket
