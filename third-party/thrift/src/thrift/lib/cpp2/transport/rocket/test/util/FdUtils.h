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
#include <optional>
#include <string>

#include <folly/Conv.h>
#include <folly/MPMCQueue.h>
#include <folly/String.h>
#include <folly/io/async/fdsock/AsyncFdSocket.h>
#include <folly/portability/GTest.h>

namespace apache {
namespace thrift {
namespace rocket {
namespace test {

using MessagePair = // Data & FDs that are being sent together.
    std::pair<std::unique_ptr<std::string>, folly::SocketFds::ToSend>;
using MessageQueue = folly::MPMCQueue<MessagePair>;

// Abbreviates the middle part of `s` with "..." so its size is under `maxLen`
std::string abbrevStr(const std::string& s, size_t maxLen = 200);

// Human readable: FD tests log Thrift requests, which can be long binary data
inline std::string renderData(const std::string& s) {
  return folly::cEscape<std::string>(abbrevStr(s));
}

std::string commaSeparatedFds(const folly::SocketFds::ToSend& fds);

// Human readable FD numbers.
inline std::string renderFds(const folly::SocketFds::ToSend& fds) {
  return "FDs<" + commaSeparatedFds(fds) + ">";
}

inline void expectUseCount(size_t n, const folly::SocketFds::ToSend& fds) {
  for (const auto& fd : fds) {
    EXPECT_EQ(n, fd.use_count());
  }
}

struct InterceptedAsyncFdSocket : public folly::AsyncFdSocket {
  InterceptedAsyncFdSocket(
      MessageQueue* checkQueue,
      folly::EventBase* evb,
      const folly::SocketAddress& sockAddr)
      : folly::AsyncFdSocket(evb, sockAddr), checkQueue_(checkQueue) {}

  InterceptedAsyncFdSocket(
      MessageQueue* checkQueue,
      folly::EventBase* evb,
      folly::NetworkSocket fd,
      const folly::SocketAddress* peerAddress = nullptr)
      : folly::AsyncFdSocket(evb, fd, peerAddress), checkQueue_(checkQueue) {}

  MessagePair popQueue() {
    MessagePair resPair;
    CHECK(checkQueue_->read(resPair));
    return resPair;
  }

  void writeChainWithFds(
      WriteCallback* callback,
      std::unique_ptr<folly::IOBuf> buf,
      folly::SocketFds inFds,
      folly::WriteFlags flags = folly::WriteFlags::NONE) override;
  void writeChain(
      WriteCallback* callback,
      std::unique_ptr<folly::IOBuf>&& buf,
      folly::WriteFlags flags = folly::WriteFlags::NONE) override;

  std::optional<std::string> regexFromWriteChainWithFds_;
  MessageQueue* checkQueue_;
};

} // namespace test
} // namespace rocket
} // namespace thrift
} // namespace apache
