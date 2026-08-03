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
#include <string_view>
#include <utility>

#include <folly/io/IOBuf.h>
#include <thrift/lib/cpp2/async/RpcOptions.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/client/common/ClientAppAdapter.h>

namespace apache::thrift::fast_thrift::thrift::client {

template <ClientOutboundAppAdapter AppAdapter>
class BorrowedClientAppAdapter {
 public:
  using Ptr = std::unique_ptr<BorrowedClientAppAdapter>;

  explicit BorrowedClientAppAdapter(AppAdapter* adapter) : adapter_(adapter) {}

  void sendRequestResponse(
      const apache::thrift::RpcOptions& rpcOptions,
      std::string_view methodName,
      apache::thrift::RpcKind rpcKind,
      std::unique_ptr<folly::IOBuf> data,
      RequestResponseHandler handler) noexcept {
    adapter_->sendRequestResponse(
        rpcOptions, methodName, rpcKind, std::move(data), std::move(handler));
  }

  uint16_t getProtocolId() const noexcept { return adapter_->getProtocolId(); }

 private:
  AppAdapter* adapter_;
};

} // namespace apache::thrift::fast_thrift::thrift::client
