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

#include <folly/io/async/AsyncSocket.h>
#include <folly/io/async/AsyncTransport.h>
#include <array>

namespace wangle {

class TransportPeeker : public folly::AsyncTransport::ReadCallback,
                        public folly::DelayedDestruction {
 public:
  using UniquePtr = folly::DelayedDestructionUniquePtr<TransportPeeker>;

  class Callback {
   public:
    virtual ~Callback() = default;
    virtual void peekSuccess(std::vector<uint8_t> data) noexcept = 0;
    virtual void peekError(const folly::AsyncSocketException& ex) noexcept = 0;
  };

  TransportPeeker(
      folly::AsyncTransport& transport,
      Callback* callback,
      size_t numBytes)
      : transport_(transport),
        transportCallback_(callback),
        peekBytes_(numBytes) {}

  ~TransportPeeker() override {
    if (transport_.getReadCallback() == this) {
      transport_.setReadCB(nullptr);
    }
  }

  void start() {
    if (peekBytes_.size() == 0) {
      // No peeking necessary.
      auto callback = std::exchange(transportCallback_, nullptr);
      callback->peekSuccess(std::move(peekBytes_));
    } else {
      transport_.setReadCB(this);
    }
  }

  void getReadBuffer(void** bufReturn, size_t* lenReturn) override {
    CHECK_LT(read_, peekBytes_.size());
    *bufReturn = reinterpret_cast<void*>(peekBytes_.data() + read_);
    *lenReturn = peekBytes_.size() - read_;
  }

  void readEOF() noexcept override {
    folly::DelayedDestruction::DestructorGuard dg(this);

    auto type =
        folly::AsyncSocketException::AsyncSocketExceptionType::END_OF_FILE;
    readErr(folly::AsyncSocketException(type, "Unexpected EOF"));
  }

  void readErr(const folly::AsyncSocketException& ex) noexcept override {
    folly::DelayedDestruction::DestructorGuard dg(this);

    transport_.setReadCB(nullptr);
    if (auto callback = std::exchange(transportCallback_, nullptr)) {
      callback->peekError(ex);
    }
  }

  void readDataAvailable(size_t len) noexcept override {
    folly::DelayedDestruction::DestructorGuard dg(this);

    read_ += len;
    CHECK_LE(read_, peekBytes_.size());

    if (read_ == peekBytes_.size()) {
      transport_.setReadCB(nullptr);
      auto callback = std::exchange(transportCallback_, nullptr);
      callback->peekSuccess(std::move(peekBytes_));
    }
  }

  bool isBufferMovable() noexcept override {
    // Returning false so that we can supply the exact length of the
    // number of bytes we want to read.
    return false;
  }

 private:
  folly::AsyncTransport& transport_;
  Callback* transportCallback_;
  size_t read_{0};
  std::vector<uint8_t> peekBytes_;
};

class SocketPeeker : public TransportPeeker, private TransportPeeker::Callback {
 public:
  using UniquePtr = folly::DelayedDestructionUniquePtr<SocketPeeker>;

  using Callback = TransportPeeker::Callback;

  SocketPeeker(folly::AsyncSocket& socket, Callback* callback, size_t numBytes)
      : TransportPeeker(socket, this, numBytes),
        socket_(socket),
        socketCallback_(callback) {}

 private:
  void peekSuccess(std::vector<uint8_t> data) noexcept override {
    socket_.setPreReceivedData(folly::IOBuf::copyBuffer(data));
    socketCallback_->peekSuccess(std::move(data));
  }

  void peekError(const folly::AsyncSocketException& ex) noexcept override {
    socketCallback_->peekError(ex);
  }

  folly::AsyncSocket& socket_;
  Callback* socketCallback_;
};
} // namespace wangle
