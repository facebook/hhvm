/*
 * Copyright (c) Facebook, Inc. and its affiliates.
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

#include <thrift/lib/cpp/async/TFramedAsyncChannel.h>

#include <folly/lang/Bits.h>
#include <thrift/lib/cpp/async/TStreamAsyncChannel.h>

using apache::thrift::transport::TMemoryBuffer;
using apache::thrift::transport::TTransportException;

namespace apache {
namespace thrift {
namespace async {

namespace detail {

TFramedACWriteRequest::TFramedACWriteRequest(
    const VoidCallback& callback,
    const VoidCallback& errorCallback,
    TMemoryBuffer* message,
    TAsyncEventChannel*)
    : TAsyncChannelWriteRequestBase(callback, errorCallback, message) {}

void TFramedACWriteRequest::write(
    folly::AsyncTransport* transport,
    folly::AsyncTransport::WriteCallback* callback) noexcept {
  uint32_t len = buffer_.available_read();

  const int kNOps = 2;
  iovec ops[kNOps];

  frameSize_ = folly::Endian::big(len);
  ops[0].iov_base = frameSizeBuf_;
  ops[0].iov_len = sizeof(frameSizeBuf_);

  ops[1].iov_base = const_cast<uint8_t*>(buffer_.borrow(nullptr, &len));
  ops[1].iov_len = len;

  transport->writev(callback, ops, kNOps);
}

void TFramedACWriteRequest::writeSuccess() noexcept {
  buffer_.consume(folly::Endian::big(frameSize_));
  invokeCallback();
}

void TFramedACWriteRequest::writeError(
    size_t bytesWritten, const TTransportException& ex) noexcept {
  T_ERROR(
      "TFramedAC: write failed after writing %zu bytes: %s",
      bytesWritten,
      ex.what());
  invokeErrorCallback();
}

TFramedACReadState::TFramedACReadState()
    : maxFrameSize_(0x7fffffff), bytesRead_(0), buffer_(nullptr) {
  frameSize_ = 0;
}

void TFramedACReadState::getReadBuffer(void** bufReturn, size_t* lenReturn) {
  if (bytesRead_ < sizeof(frameSize_)) {
    // We're reading the frame size
    *lenReturn = sizeof(frameSize_) - bytesRead_;
    *bufReturn = frameSizeBuf_ + bytesRead_;
  } else {
    // We've read the frame size, and are now reading the buffer
    uint32_t bufBytesRead = bytesRead_ - sizeof(frameSize_);
    assert(bufBytesRead < frameSize_);
    uint32_t bytesLeft = frameSize_ - bufBytesRead;
    *lenReturn = bytesLeft;
    *bufReturn = buffer_->getWritePtr(bytesLeft);
  }
}

bool TFramedACReadState::readDataAvailable(size_t len) {
  if (bytesRead_ < sizeof(frameSize_)) {
    // We just read bytes into the frame size buffer
    assert(bytesRead_ + len <= sizeof(frameSize_));
    bytesRead_ += len;
    if (bytesRead_ >= sizeof(frameSize_)) {
      // We've finished reading the frame size
      // Convert the frame size to host byte order
      frameSize_ = folly::Endian::big(frameSize_);

      // Check for overly large frame sizes, so that we reject garbage data
      // instead of allocating a huge buffer.
      if (frameSize_ > maxFrameSize_) {
        T_ERROR("TFramedAC::read(): frame size of %d rejected", frameSize_);
        throw TTransportException(
            TTransportException::CORRUPTED_DATA,
            "rejected overly large frame size");
      }

      // The empty frame is complete without body bytes
      if (frameSize_ == 0) {
        return true;
      }
    }
  } else {
    // We just read body bytes
    bytesRead_ += len;
    uint32_t bufBytesRead = bytesRead_ - sizeof(frameSize_);
    assert(bufBytesRead <= frameSize_);
    buffer_->wroteBytes(len);

    if (bufBytesRead >= frameSize_) {
      // We've finished reading the frame.
      return true;
    }
  }

  // We aren't done with the frame yet.
  return false;
}

} // namespace detail

} // namespace async
} // namespace thrift
} // namespace apache
