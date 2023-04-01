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

#include <sys/types.h>
#include <memory>
#include <folly/io/IOBuf.h>
#include <folly/io/async/AsyncSocket.h>

namespace apache {
namespace thrift {
namespace async {

class TAsyncSocketIntercepted : public folly::AsyncSocket {
 public:
  TAsyncSocketIntercepted(
      folly::EventBase* evb,
      const folly::SocketAddress& address,
      uint32_t connectTimeout = 0)
      : folly::AsyncSocket(evb, address, connectTimeout) {}
  TAsyncSocketIntercepted(
      folly::EventBase* evb,
      const std::string& ip,
      uint16_t port,
      uint32_t connectTimeout = 0)
      : folly::AsyncSocket(evb, ip, port, connectTimeout) {}

  ~TAsyncSocketIntercepted() override {}

  struct Params {
    std::atomic<bool> corruptLastWriteByte_{false};
    std::atomic<bool> corruptLastReadByte_{false};
    std::atomic<int32_t> corruptLastReadByteMinSize_{0};
  };

  void setParams(std::shared_ptr<Params> params) { params_ = params; }

  int32_t getTotalBytesRead() { return totalBytesRead_; }

  int32_t getTotalBytesWritten() { return totalBytesWritten_; }

  WriteResult performWrite(
      const iovec* vec,
      uint32_t count,
      folly::WriteFlags flags,
      uint32_t* countWritten,
      uint32_t* partialWritten,
      folly::AsyncSocket::WriteRequestTag) override;

  ReadResult performReadMsg(
      struct ::msghdr& msg, AsyncReader::ReadCallback::ReadMode) override;

 private:
  std::shared_ptr<Params> params_;
  int32_t totalBytesRead_{0};
  int32_t totalBytesWritten_{0};
};

} // namespace async
} // namespace thrift
} // namespace apache
