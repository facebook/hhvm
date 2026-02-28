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

#include <thrift/lib/cpp2/transport/rocket/framing/FrameDataFirstFieldAligner.h>

#include <glog/logging.h>

#include <folly/GLog.h>
#include <folly/Utility.h>
#include <folly/io/Cursor.h>

#include <thrift/lib/cpp2/protocol/BinaryProtocol.h>
#include <thrift/lib/cpp2/protocol/detail/PaddedBinaryAdapter.h>
#include <thrift/lib/cpp2/transport/rocket/framing/Frames.h>

namespace apache::thrift::rocket {

template <class Frame>
FrameDataFirstFieldAligner<Frame>::FrameDataFirstFieldAligner(Frame& frame)
    : frame_(frame) {}

template <class Frame>
void FrameDataFirstFieldAligner<Frame>::align() {
  if (!validateAlignmentRequest()) {
    return;
  }

  // This extracts the payload out of the frame. The payload must be
  // reconstructed prior to returning after this.
  extractPayloadComponents();

  if (findFieldToAlign() && validatePadding()) {
    trimPadding();
  }

  reconstructPayload();
}

template <class Frame>
bool FrameDataFirstFieldAligner<Frame>::validateAlignmentRequest() {
  fieldAlignment_ = frame_.payload().dataFirstFieldAlignment();
  DCHECK(folly::isPowTwo(fieldAlignment_))
      << "Alignment is not a power of two!";
  if (fieldAlignment_ & (fieldAlignment_ - 1)) {
    FB_LOG_ONCE(ERROR) << "Alignment is not a power of two!";
    return false;
  }

  DCHECK(frame_.payload().metadataAndDataSize() <= kMaxFragmentedPayloadSize)
      << "Fragmented payload";
  if (frame_.payload().metadataAndDataSize() > kMaxFragmentedPayloadSize) {
    FB_LOG_ONCE(ERROR) << "Fragmented payload cannot be aligned!";
    return false;
  }

  DCHECK(frame_.payload().dataSerializationProtocol() != std::nullopt)
      << "No data serialization protocol specified!";
  if (frame_.payload().dataSerializationProtocol() == std::nullopt) {
    FB_LOG_ONCE(ERROR) << "No data serialization protocol specified!";
    return false;
  }

  DCHECK(
      frame_.payload().dataSerializationProtocol() ==
      ProtocolType::T_BINARY_PROTOCOL)
      << "Only binary protocol is supported!";
  if (frame_.payload().dataSerializationProtocol() !=
      ProtocolType::T_BINARY_PROTOCOL) {
    FB_LOG_ONCE(ERROR) << "Only binary protocol is supported!";
    return false;
  }

  return true;
}

template <class Frame>
void FrameDataFirstFieldAligner<Frame>::extractPayloadComponents() {
  payloadMetadataSize_ = static_cast<uint32_t>(frame_.payload().metadataSize());
  payloadBuf_ = std::move(frame_.payload()).buffer();
}

template <class Frame>
void FrameDataFirstFieldAligner<Frame>::reconstructPayload() {
  frame_.payload() =
      Payload::makeCombined(std::move(payloadBuf_), payloadMetadataSize_);
}

template <class Frame>
bool FrameDataFirstFieldAligner<Frame>::findFieldToAlign() {
  BinaryProtocolReader protReader;
  protReader.setInput(payloadBuf_.get());
  // Skip over to the serialized data part of the payload.
  protReader.skipBytes(payloadMetadataSize_);

  std::string structName;
  protReader.readStructBegin(structName);

  TType fieldType;
  int16_t fieldId;
  std::string fieldName;

  // Skip over field metadata about the first param
  protReader.readFieldBegin(fieldName, fieldType, fieldId);
  DCHECK(fieldId == 1) << "Cannot find param with fieldId=1!";
  if (fieldId != 1) {
    FB_LOG_ONCE(WARNING) << "Cannot find first field!";
    return false;
  }

  // Skip over field metadata about the first entry in the first param
  protReader.readFieldBegin(fieldName, fieldType, fieldId);
  DCHECK(fieldId == 1)
      << "Cannot find entry in the first param with fieldId=1!";
  if (fieldId != 1) {
    FB_LOG_ONCE(WARNING) << "Cannot find first field!";
    return false;
  }

  // The field to align must be a binary field.
  DCHECK(fieldType == TType::T_STRING) << "First entry is not binary!";
  if (fieldType != TType::T_STRING) {
    FB_LOG_ONCE(WARNING) << "First entry is not binary!";
    return false;
  }

  // Capture the offset (w.r.t. start of the payload) of the field that needs
  // to be aligned.
  offsetOfFieldToAlignInPayload_ =
      static_cast<uint32_t>(protReader.getCursor().getCurrentPosition());

  readCursor_ = protReader.getCursor();

  return true;
}

template <class Frame>
bool FrameDataFirstFieldAligner<Frame>::validatePadding() {
  uint32_t dataSize = readCursor_.readBE<uint32_t>();
  DCHECK(dataSize >= protocol::PaddedBinaryData::kPaddingHeaderBytes)
      << "Not padded!";
  if (dataSize < protocol::PaddedBinaryData::kPaddingHeaderBytes) {
    FB_LOG_ONCE(WARNING) << "Data was not padded!";
    return false;
  }

  uint64_t magic = readCursor_.readBE<uint64_t>();
  DCHECK(magic == protocol::PaddedBinaryData::kMagic) << "Magic mismatch!";
  if (magic != protocol::PaddedBinaryData::kMagic) {
    FB_LOG_ONCE(WARNING) << "Data was not serialied with PaddedBinaryData!";
    return false;
  }

  uint32_t paddingPresent = readCursor_.readBE<uint32_t>();
  uint32_t paddingNeeded = computePaddingNeeded();
  DCHECK(paddingNeeded <= paddingPresent) << "Insufficient padding!";
  if (paddingNeeded > paddingPresent) {
    FB_LOG_ONCE(ERROR) << "Cannot increase padding from " << paddingPresent
                       << " to " << paddingNeeded;
    return false;
  }

  // Make sure that the padding IOBuf is separate from the data IOBuf.
  DCHECK(paddingPresent == readCursor_.length())
      << "Padding IOBuf not separate!";
  if (paddingPresent != readCursor_.length()) {
    FB_LOG_ONCE(ERROR) << "Padding not added as a separate IOBuf! Abandon!";
    return false;
  }

  paddingToTrim_ = paddingPresent - paddingNeeded;
  updatedPadding_ = paddingNeeded;
  // There has to be some data in order for it to be padded, otherwise we must
  // have done something wrong.
  DCHECK(paddingToTrim_ < dataSize);
  updatedDataSize_ = dataSize - paddingToTrim_;

  // Find the padding IOBuf.
  size_t cursorPos = readCursor_.getCurrentPosition();
  size_t curPos = 0;
  folly::IOBuf* buf = payloadBuf_.get();
  do {
    size_t nextPos = curPos + buf->length();
    if (cursorPos >= curPos && cursorPos <= nextPos) {
      DCHECK(paddingToTrim_ < buf->length());
      paddingIOBuf_ = buf;
      break;
    }
    curPos = nextPos;
    buf = buf->next();
  } while (buf != payloadBuf_.get());

  DCHECK(paddingIOBuf_ != nullptr) << "Not able to find the padding IOBuf!";
  if (paddingIOBuf_ == nullptr) {
    FB_LOG_ONCE(ERROR) << "Not able to find the padding IOBuf! Aborting!";
  }

  return true;
}

template <class Frame>
void FrameDataFirstFieldAligner<Frame>::trimPadding() {
  folly::io::RWPrivateCursor writeCursor(payloadBuf_.get());
  writeCursor.skip(offsetOfFieldToAlignInPayload_);
  // Update the data size field.
  writeCursor.writeBE<int32_t>(updatedDataSize_);
  // Update the padding size field.
  writeCursor.skip(sizeof(protocol::PaddedBinaryData::kMagic));
  writeCursor.writeBE<int32_t>(updatedPadding_);
  // Trim the padding.
  paddingIOBuf_->trimEnd(paddingToTrim_);

  FB_LOG_ONCE(INFO) << "Successfully aligned field by trimming padding.";
}

template <class Frame>
uint32_t FrameDataFirstFieldAligner<Frame>::computePaddingNeeded() {
  uint32_t offsetOfPayloadInFrame = Serializer::kBytesForFrameOrMetadataLength +
      Frame::frameHeaderSize() +
      /* metadata size field length */
      (payloadMetadataSize_ > 0 ? Serializer::kBytesForFrameOrMetadataLength
                                : 0);

  uint32_t offsetOfDataToAlign = offsetOfPayloadInFrame +
      offsetOfFieldToAlignInPayload_ + kDataSizeFieldLength +
      protocol::PaddedBinaryData::kPaddingHeaderBytes;

  uint32_t paddingNeeded =
      (fieldAlignment_ - (offsetOfDataToAlign & (fieldAlignment_ - 1))) &
      (fieldAlignment_ - 1);
  DCHECK(paddingNeeded < fieldAlignment_)
      << paddingNeeded << " >= " << fieldAlignment_;

  return paddingNeeded;
}

// Only supported for a RequestResponseFrame for now.
template class FrameDataFirstFieldAligner<RequestResponseFrame>;

} // namespace apache::thrift::rocket
