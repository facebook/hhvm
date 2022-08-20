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

#include <thrift/lib/cpp/async/THttpAsyncChannel.h>

#include <thrift/lib/cpp/async/TStreamAsyncChannel.h>

using apache::thrift::transport::TMemoryBuffer;
using apache::thrift::transport::TTransportException;
using apache::thrift::util::THttpParser;

namespace apache {
namespace thrift {
namespace async {

namespace detail {

THttpACWriteRequest::THttpACWriteRequest(
    const VoidCallback& callback,
    const VoidCallback& errorCallback,
    TMemoryBuffer* message,
    TAsyncEventChannel* channel)
    : TAsyncChannelWriteRequestBase(callback, errorCallback, message) {
  channel_ = static_cast<THttpAsyncChannel*>(channel);
}

void THttpACWriteRequest::write(
    folly::AsyncTransport* transport,
    folly::AsyncTransport::WriteCallback* callback) noexcept {
  uint32_t len = buffer_.available_read();
  // We can call wrapBuffer (instead of having to copy the buffer) because
  // buffer_ last longer than the IOBuf that we are creating.
  auto buf = folly::IOBuf::wrapBuffer(buffer_.borrow(nullptr, &len), len);
  buf = channel_->constructHeader(std::move(buf));
  transport->writeChain(callback, std::move(buf));
}

void THttpACWriteRequest::writeSuccess() noexcept {
  buffer_.consume(buffer_.available_read());
  invokeCallback();
}

void THttpACWriteRequest::writeError(
    size_t bytesWritten, const TTransportException& ex) noexcept {
  T_ERROR(
      "THttpAC: write failed after writing %zu bytes: %s",
      bytesWritten,
      ex.what());
  invokeErrorCallback();
}

void THttpACReadState::getReadBuffer(void** bufReturn, size_t* lenReturn) {
  parser_->getReadBuffer(bufReturn, lenReturn);
}

bool THttpACReadState::readDataAvailable(size_t len) {
  return parser_->readDataAvailable(len);
}

} // namespace detail

} // namespace async
} // namespace thrift
} // namespace apache
