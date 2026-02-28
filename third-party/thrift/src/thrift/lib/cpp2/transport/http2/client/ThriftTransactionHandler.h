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

#include <proxygen/lib/http/session/HTTPTransaction.h>
#include <thrift/lib/cpp2/transport/http2/common/H2Channel.h>

namespace apache::thrift {

class ThriftTransactionHandler
    : public proxygen::HTTPTransactionHandler,
      public proxygen::HTTPTransaction::TransportCallback {
 public:
  ThriftTransactionHandler() = default;

  ~ThriftTransactionHandler() override;

  ThriftTransactionHandler(const ThriftTransactionHandler&) = delete;
  ThriftTransactionHandler& operator=(const ThriftTransactionHandler&) = delete;
  ThriftTransactionHandler(ThriftTransactionHandler&&) = delete;
  ThriftTransactionHandler& operator=(ThriftTransactionHandler&&) = delete;

  void setChannel(std::shared_ptr<H2Channel> channel);

  void setTransaction(proxygen::HTTPTransaction* txn) noexcept override;

  void detachTransaction() noexcept override;

  void onHeadersComplete(
      std::unique_ptr<proxygen::HTTPMessage> msg) noexcept override;

  void onBody(std::unique_ptr<folly::IOBuf> body) noexcept override;

  void onChunkHeader(size_t /* length */) noexcept override {
    // HTTP/1.1 function, do not need attention here
  }

  void onChunkComplete() noexcept override {
    // HTTP/1.1 function, do not need attention here
  }

  void onTrailers(
      std::unique_ptr<proxygen::HTTPHeaders> /*trailers*/) noexcept override {}

  void onEOM() noexcept override;

  void onUpgrade(proxygen::UpgradeProtocol /*protocol*/) noexcept override {
    // If code comes here, it is seriously wrong
    // TODO (geniusye) destroy the channel here
  }

  void onError(const proxygen::HTTPException& error) noexcept override;

  void onEgressPaused() noexcept override {
    // we could notify to throttle on this channel
    // it is okay not to throttle too,
    // it won't immediately causing any problem
  }

  void onEgressResumed() noexcept override {
    // we could notify to stop throttle on this channel
    // it is okay not to throttle too,
    // it won't immediately causing any problem
  }

  void onPushedTransaction(
      proxygen::HTTPTransaction* /*txn*/) noexcept override {}

  /**
   * HTTPTransaction::TransportCallback interface
   */
  void firstHeaderByteFlushed() noexcept override {}
  void firstByteFlushed() noexcept override {}
  void lastByteFlushed() noexcept override;
  void lastByteAcked(std::chrono::milliseconds /*latency*/) noexcept override {}
  void headerBytesGenerated(
      proxygen::HTTPHeaderSize& /*size*/) noexcept override {}
  void headerBytesReceived(
      const proxygen::HTTPHeaderSize& /*size*/) noexcept override {}
  void bodyBytesGenerated(size_t /*nbytes*/) noexcept override {}
  void bodyBytesReceived(size_t /*size*/) noexcept override {}

 private:
  std::shared_ptr<H2Channel> channel_;
  proxygen::HTTPTransaction* txn_;
};

} // namespace apache::thrift
