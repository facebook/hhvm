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

#include <thrift/lib/cpp/async/TUnframedAsyncChannel.h>

#include <thrift/lib/cpp/transport/TBufferTransports.h>

using apache::thrift::transport::TMemoryBuffer;
using apache::thrift::transport::TTransportException;

namespace apache {
namespace thrift {
namespace async {
namespace detail {

TUnframedACWriteRequest::TUnframedACWriteRequest(
    const VoidCallback& callback,
    const VoidCallback& errorCallback,
    TMemoryBuffer* message,
    TAsyncEventChannel*)
    : TAsyncChannelWriteRequestBase(callback, errorCallback, message) {}

void TUnframedACWriteRequest::write(
    folly::AsyncTransport* transport,
    folly::AsyncTransport::WriteCallback* callback) noexcept {
  uint32_t len = buffer_.available_read();

  const int kNOps = 1;
  iovec ops[kNOps];
  ops[0].iov_base = const_cast<uint8_t*>(buffer_.borrow(nullptr, &len));
  ops[0].iov_len = len;

  transport->writev(callback, ops, kNOps);
}

void TUnframedACWriteRequest::writeSuccess() noexcept {
  buffer_.consume(buffer_.available_read());
  invokeCallback();
}

void TUnframedACWriteRequest::writeError(
    size_t bytesWritten, const TTransportException& ex) noexcept {
  T_ERROR(
      "unframed channel: write failed after writing %zu bytes: %s",
      bytesWritten,
      ex.what());
  invokeErrorCallback();
}

} // namespace detail
} // namespace async
} // namespace thrift
} // namespace apache
