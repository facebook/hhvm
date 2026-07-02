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

#include <folly/CancellationToken.h>
#include <folly/Expected.h>
#include <folly/Unit.h>
#include <folly/coro/BlockingWait.h>
#include <folly/coro/Task.h>
#include <folly/fibers/Baton.h>
#include <folly/io/IOBuf.h>
#include <thrift/lib/cpp2/GeneratedCodeHelper.h>
#include <thrift/lib/cpp2/async/RequestCallback.h>
#include <thrift/lib/cpp2/async/RpcOptions.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/client/common/ClientAppAdapter.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/client/common/RequestSerializer.h>
#include <thrift/lib/cpp2/protocol/Protocol.h>
#include <thrift/lib/thrift/gen-cpp2/RpcMetadata_types.h>

namespace apache::thrift::gen {

template <apache::thrift::fast_thrift::thrift::client::ClientOutboundAppAdapter
              AppAdapter>
class FastClientBase {
 public:
  explicit FastClientBase(typename AppAdapter::Ptr adapter)
      : adapter_(std::move(adapter)) {}

 protected:
  folly::coro::Task<std::unique_ptr<folly::IOBuf>> co_sendRequestResponse(
      const apache::thrift::RpcOptions& rpcOptions,
      std::string_view methodName,
      apache::thrift::RpcKind rpcKind,
      folly::Expected<std::unique_ptr<folly::IOBuf>, folly::exception_wrapper>
          data) {
    if (data.hasError()) {
      co_yield folly::coro::co_error(std::move(data.error()));
    }

    const folly::CancellationToken& cancelToken =
        co_await folly::coro::co_current_cancellation_token;
    const bool cancellable = cancelToken.canBeCancelled();

    folly::fibers::Baton baton;
    folly::Expected<std::unique_ptr<folly::IOBuf>, folly::exception_wrapper>
        response{folly::makeUnexpected(folly::exception_wrapper())};

    adapter_->sendRequestResponse(
        rpcOptions,
        methodName,
        rpcKind,
        std::move(data.value()),
        [&baton, &response](
            folly::Expected<
                std::unique_ptr<folly::IOBuf>,
                folly::exception_wrapper>&& result,
            const apache::thrift::
                RpcTransportStats& /*rpcTransportStats*/) mutable noexcept {
          response = std::move(result);
          baton.post();
        });

    if (cancellable) {
      folly::CancellationCallback cb(cancelToken, [&] {
        response = folly::makeUnexpected(
            folly::make_exception_wrapper<folly::OperationCancelled>());
        baton.post();
      });
      co_await baton;
    } else {
      co_await baton;
    }

    if (response.hasError()) {
      co_yield folly::coro::co_error(std::move(response.error()));
    }

    co_return std::move(response.value());
  }

  void sendRequestResponse(
      const apache::thrift::RpcOptions& rpcOptions,
      std::string_view methodName,
      apache::thrift::RpcKind rpcKind,
      folly::Expected<std::unique_ptr<folly::IOBuf>, folly::exception_wrapper>
          serializedData,
      std::unique_ptr<apache::thrift::RequestCallback> callback) {
    apache::thrift::RequestCallback::Context callbackContext;
    callbackContext.oneWay = false;
    callbackContext.protocolId = adapter_->getProtocolId();
    auto clientCallback = apache::thrift::prepareFastThriftCallback(
        std::move(callback), std::move(callbackContext));

    if (serializedData.hasError()) {
      clientCallback.release()->onResponseError(
          std::move(serializedData.error()));
      return;
    }

    adapter_->sendRequestResponse(
        rpcOptions,
        methodName,
        rpcKind,
        std::move(serializedData.value()),
        [clientCallback = std::move(clientCallback),
         protocolId = adapter_->getProtocolId()](
            folly::Expected<
                std::unique_ptr<folly::IOBuf>,
                folly::exception_wrapper>&& result,
            const apache::thrift::RpcTransportStats&
                rpcTransportStats) mutable noexcept {
          handleCallbackResponse(
              std::move(clientCallback),
              protocolId,
              std::move(result),
              rpcTransportStats);
        });
  }

  typename AppAdapter::Ptr adapter_;

 private:
  static void handleCallbackResponse(
      apache::thrift::RequestClientCallback::Ptr callback,
      uint16_t protocolId,
      folly::Expected<std::unique_ptr<folly::IOBuf>, folly::exception_wrapper>&&
          result,
      const apache::thrift::RpcTransportStats& rpcTransportStats) noexcept {
    auto* raw = callback.release();
    if (result.hasError()) {
      raw->onResponseError(std::move(result.error()));
      return;
    }

    raw->onResponse(
        apache::thrift::ClientReceiveState(
            protocolId,
            apache::thrift::MessageType::T_REPLY,
            apache::thrift::SerializedResponse(std::move(result.value())),
            nullptr,
            nullptr,
            rpcTransportStats));
  }
};

} // namespace apache::thrift::gen
