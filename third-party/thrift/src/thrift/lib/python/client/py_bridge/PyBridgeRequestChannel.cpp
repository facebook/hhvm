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

#include <thrift/lib/python/client/py_bridge/PyBridgeRequestChannel.h>

#include <utility>

#include <folly/Try.h>
#include <folly/futures/Future.h>

#include <thrift/lib/cpp/transport/THeader.h>
#include <thrift/lib/cpp2/async/RpcTypes.h>

namespace apache::thrift::python::client {

RequestChannel::Ptr PyBridgeRequestChannel::newChannel(
    uint16_t protocolId, PyBridgeSender sender, folly::Executor* executor) {
  return RequestChannel::Ptr(new PyBridgeRequestChannel(
      protocolId, std::move(sender), folly::getKeepAliveToken(executor)));
}

std::unique_ptr<folly::IOBuf> PyBridgeRequestChannel::envelope(
    const MethodMetadata& methodMetadata,
    SerializedRequest&& serializedRequest) {
  // The receiving transport expects a legacy-enveloped CALL (method + args) and
  // parses the method name back out of the envelope.
  return LegacySerializedRequest(
             protocolId_,
             methodMetadata.name_view(),
             std::move(serializedRequest))
      .buffer;
}

void PyBridgeRequestChannel::sendRequestResponse(
    const RpcOptions& /*rpcOptions*/,
    MethodMetadata&& methodMetadata,
    SerializedRequest&& serializedRequest,
    std::shared_ptr<transport::THeader> /*header*/,
    RequestClientCallback::Ptr cb,
    std::unique_ptr<folly::IOBuf> /*frameworkMetadata*/) {
  auto enveloped = envelope(methodMetadata, std::move(serializedRequest));
  const uint16_t protocolId = protocolId_;
  // The callback is moved into the continuation rather than released to a raw
  // pointer up front: a throw while building the future chain then destroys the
  // owned callback (no leak). makeSemiFutureWith funnels a synchronous sender_
  // throw into the future, so the single continuation completes the callback
  // exactly once for both success and failure.
  folly::makeSemiFutureWith([&]() {
    return sender_(
        std::move(enveloped), RpcKind::SINGLE_REQUEST_SINGLE_RESPONSE);
  })
      .via(executor_)
      .thenTry([protocolId = protocolId, cb = std::move(cb)](
                   folly::Try<std::unique_ptr<folly::IOBuf>>&& result) mutable {
        if (result.hasException()) {
          cb.release()->onResponseError(std::move(result.exception()));
          return;
        }
        // ClientReceiveState strips the reply envelope and exposes the
        // message type, so OmniClient decodes T_REPLY / T_EXCEPTION exactly
        // as for a socket channel.
        cb.release()->onResponse(ClientReceiveState(
            protocolId,
            std::move(result.value()),
            std::make_unique<transport::THeader>(),
            /*ctx=*/nullptr));
      });
}

void PyBridgeRequestChannel::sendRequestNoResponse(
    const RpcOptions& /*rpcOptions*/,
    MethodMetadata&& methodMetadata,
    SerializedRequest&& serializedRequest,
    std::shared_ptr<transport::THeader> /*header*/,
    RequestClientCallback::Ptr cb,
    std::unique_ptr<folly::IOBuf> /*frameworkMetadata*/) {
  auto enveloped = envelope(methodMetadata, std::move(serializedRequest));
  // Oneway has no Thrift reply; surface only dispatch / transport failure. As
  // in sendRequestResponse, the callback is moved into the continuation (no
  // leak on a setup throw) and completed exactly once there.
  folly::makeSemiFutureWith([&]() {
    return sender_(std::move(enveloped), RpcKind::SINGLE_REQUEST_NO_RESPONSE);
  })
      .via(executor_)
      .thenTry([cb = std::move(cb)](
                   folly::Try<std::unique_ptr<folly::IOBuf>>&& result) mutable {
        if (result.hasException()) {
          cb.release()->onResponseError(std::move(result.exception()));
          return;
        }
        cb.release()->onResponse(ClientReceiveState());
      });
}

} // namespace apache::thrift::python::client
