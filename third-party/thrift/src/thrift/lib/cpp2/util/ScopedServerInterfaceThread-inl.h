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

#include <folly/executors/GlobalExecutor.h>
#include <folly/io/async/AsyncSocket.h>

namespace apache {
namespace thrift {
namespace detail {

class FaultInjectionChannel : public RequestChannel {
 public:
  FaultInjectionChannel(
      RequestChannel::Ptr client,
      ScopedServerInterfaceThread::FaultInjectionFunc injectFault)
      : client_(std::move(client)), injectFault_(std::move(injectFault)) {}

  using RequestChannel::sendRequestNoResponse;
  using RequestChannel::sendRequestResponse;
  using RequestChannel::sendRequestSink;
  using RequestChannel::sendRequestStream;

  void sendRequestResponse(
      const RpcOptions& rpcOptions,
      MethodMetadata&& methodMetadata,
      SerializedRequest&& req,
      std::shared_ptr<transport::THeader> header,
      RequestClientCallback::Ptr clientCallback) override {
    if (auto ex = injectFault_(methodMetadata.name_view())) {
      clientCallback.release()->onResponseError(std::move(ex));
      return;
    }
    client_->sendRequestResponse(
        rpcOptions,
        std::move(methodMetadata),
        std::move(req),
        std::move(header),
        std::move(clientCallback));
  }
  void sendRequestNoResponse(
      const RpcOptions& rpcOptions,
      MethodMetadata&& methodMetadata,
      SerializedRequest&& req,
      std::shared_ptr<transport::THeader> header,
      RequestClientCallback::Ptr clientCallback) override {
    if (auto ex = injectFault_(methodMetadata.name_view())) {
      clientCallback.release()->onResponseError(std::move(ex));
      return;
    }
    client_->sendRequestNoResponse(
        rpcOptions,
        std::move(methodMetadata),
        std::move(req),
        std::move(header),
        std::move(clientCallback));
  }
  void sendRequestStream(
      const RpcOptions& rpcOptions,
      MethodMetadata&& methodMetadata,
      SerializedRequest&& req,
      std::shared_ptr<transport::THeader> header,
      StreamClientCallback* clientCallback) override {
    if (auto ex = injectFault_(methodMetadata.name_view())) {
      clientCallback->onFirstResponseError(std::move(ex));
      return;
    }
    client_->sendRequestStream(
        rpcOptions,
        std::move(methodMetadata),
        std::move(req),
        std::move(header),
        clientCallback);
  }
  void sendRequestSink(
      const RpcOptions& rpcOptions,
      MethodMetadata&& methodMetadata,
      SerializedRequest&& req,
      std::shared_ptr<transport::THeader> header,
      SinkClientCallback* clientCallback) override {
    if (auto ex = injectFault_(methodMetadata.name_view())) {
      clientCallback->onFirstResponseError(std::move(ex));
      return;
    }
    client_->sendRequestSink(
        rpcOptions,
        std::move(methodMetadata),
        std::move(req),
        std::move(header),
        clientCallback);
  }

  void setCloseCallback(CloseCallback* cb) override {
    client_->setCloseCallback(cb);
  }

  folly::EventBase* getEventBase() const override {
    return client_->getEventBase();
  }

  uint16_t getProtocolId() override { return client_->getProtocolId(); }

 private:
  RequestChannel::Ptr client_;
  ScopedServerInterfaceThread::FaultInjectionFunc injectFault_;
};

void validateServiceName(AsyncProcessorFactory& apf, const char* serviceName);
} // namespace detail

template <class AsyncClientT>
std::unique_ptr<AsyncClientT> ScopedServerInterfaceThread::newStickyClient(
    folly::Executor* callbackExecutor,
    ScopedServerInterfaceThread::MakeChannelFunc makeChannel) const {
  return std::make_unique<AsyncClientT>(
      newChannel(callbackExecutor, std::move(makeChannel), 1));
}

template <class AsyncClientT>
std::unique_ptr<AsyncClientT> ScopedServerInterfaceThread::newClient(
    folly::Executor* callbackExecutor,
    ScopedServerInterfaceThread::MakeChannelFunc makeChannel) const {
  return std::make_unique<AsyncClientT>(
      newChannel(callbackExecutor, std::move(makeChannel)));
}

template <class AsyncClientT>
std::unique_ptr<AsyncClientT>
ScopedServerInterfaceThread::newClientWithFaultInjection(
    ScopedServerInterfaceThread::FaultInjectionFunc injectFault,
    folly::Executor* callbackExecutor,
    ScopedServerInterfaceThread::MakeChannelFunc makeChannel) const {
  return std::make_unique<AsyncClientT>(
      std::make_shared<apache::thrift::detail::FaultInjectionChannel>(
          newChannel(callbackExecutor, std::move(makeChannel)),
          std::move(injectFault)));
}

template <class AsyncClientT>
std::unique_ptr<AsyncClientT> makeTestClient(
    std::shared_ptr<AsyncProcessorFactory> apf,
    ScopedServerInterfaceThread::FaultInjectionFunc injectFault) {
  auto client = std::make_unique<AsyncClientT>(
      ScopedServerInterfaceThread::makeTestClientChannel(
          apf, std::move(injectFault)));
  apache::thrift::detail::validateServiceName(*apf, client->getServiceName());
  return client;
}

template <class ServiceHandler, class ServiceTag>
std::unique_ptr<Client<ServiceTag>> makeTestClient(
    std::shared_ptr<ServiceHandler> handler,
    ScopedServerInterfaceThread::FaultInjectionFunc injectFault) {
  return makeTestClient<Client<ServiceTag>>(
      std::static_pointer_cast<AsyncProcessorFactory>(std::move(handler)),
      std::move(injectFault));
}

} // namespace thrift
} // namespace apache
