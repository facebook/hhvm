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

#include <cstdint>
#include <memory>

#include <folly/Executor.h>
#include <folly/Function.h>
#include <folly/futures/Future.h>
#include <folly/io/IOBuf.h>

#include <thrift/lib/cpp2/async/RequestChannel.h>

namespace apache::thrift::python::client {

// Forwards a fully legacy-enveloped Thrift request (method + args) to a Python
// transport and yields the enveloped reply bytes. Supplied by the Cython layer,
// which bridges to a Python `ChannelHandler` coroutine; this header stays free
// of any Python/GIL dependency so the channel is unit-testable in pure C++.
using PyBridgeSender =
    folly::Function<folly::SemiFuture<std::unique_ptr<folly::IOBuf>>(
        std::unique_ptr<folly::IOBuf> envelopedRequest, RpcKind rpcKind)>;

// A RequestChannel implementation that routes requests through a Python sender
// instead of a socket. Lets a generated thrift-python client run over any
// pure-Python transport (e.g. the Glue ServiceRouter gRPC sidecar) while
// reusing OmniClient's enveloping and reply/exception decoding.
//
// getEventBase() returns nullptr, so sendRequestResponse /
// sendRequestNoResponse run inline on the caller thread (the asyncio loop
// thread for an async client), and the sender's SemiFuture is completed when
// the Python coroutine resolves.
class PyBridgeRequestChannel : public RequestChannel {
 public:
  // `executor` drives the (tiny) callback-completion continuation when the
  // sender's SemiFuture resolves; pass the executor bound to the transport's
  // asyncio loop (e.g. folly::python::get_executor()).
  static RequestChannel::Ptr newChannel(
      uint16_t protocolId, PyBridgeSender sender, folly::Executor* executor);

  using RequestChannel::sendRequestNoResponse;
  using RequestChannel::sendRequestResponse;

  void sendRequestResponse(
      const RpcOptions&,
      MethodMetadata&&,
      SerializedRequest&&,
      std::shared_ptr<transport::THeader>,
      RequestClientCallback::Ptr,
      std::unique_ptr<folly::IOBuf> frameworkMetadata) override;

  void sendRequestNoResponse(
      const RpcOptions&,
      MethodMetadata&&,
      SerializedRequest&&,
      std::shared_ptr<transport::THeader>,
      RequestClientCallback::Ptr,
      std::unique_ptr<folly::IOBuf> frameworkMetadata) override;

  void setCloseCallback(CloseCallback*) override {}

  folly::EventBase* getEventBase() const override { return nullptr; }

  uint16_t getProtocolId() override { return protocolId_; }

 private:
  PyBridgeRequestChannel(
      uint16_t protocolId,
      PyBridgeSender sender,
      folly::Executor::KeepAlive<> executor)
      : protocolId_{protocolId},
        sender_{std::move(sender)},
        executor_{std::move(executor)} {}

  std::unique_ptr<folly::IOBuf> envelope(
      const MethodMetadata&, SerializedRequest&&);

  const uint16_t protocolId_;
  PyBridgeSender sender_;
  folly::Executor::KeepAlive<> executor_;
};

} // namespace apache::thrift::python::client
