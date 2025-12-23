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

#include <thrift/lib/cpp2/async/StreamCallbacks.h>

namespace apache::thrift {
namespace detail {

class StreamFaultInjectionCallback : public StreamClientCallback,
                                     public StreamServerCallback {
 public:
  StreamFaultInjectionCallback(
      StreamClientCallback* clientCallback,
      folly::Function<folly::exception_wrapper()> injectFault)
      : clientCallback_(clientCallback), injectFault_(std::move(injectFault)) {}

  /** StreamClientCallback methods **/
  bool onFirstResponse(
      FirstResponsePayload&& payload,
      folly::EventBase* evb,
      StreamServerCallback* serverCallback) override {
    if (!clientCallback_) {
      serverCallback->onStreamCancel();
      delete this;
      return false;
    }
    serverCallback_ = serverCallback;
    return clientCallback_->onFirstResponse(std::move(payload), evb, this);
  }
  void onFirstResponseError(folly::exception_wrapper ew) override {
    if (clientCallback_) {
      clientCallback_->onFirstResponseError(std::move(ew));
    }
    delete this;
  }
  bool onStreamNext(StreamPayload&& payload) override {
    if (auto ew = injectFault_()) {
      clientCallback_->onStreamError(std::move(ew));
      serverCallback_->onStreamCancel();
      delete this;
      return false;
    }
    return clientCallback_->onStreamNext(std::move(payload));
  }
  void onStreamError(folly::exception_wrapper ew) override {
    clientCallback_->onStreamError(std::move(ew));
    delete this;
  }
  void onStreamComplete() override {
    clientCallback_->onStreamComplete();
    delete this;
  }
  bool onStreamHeaders(HeadersPayload&& payload) override {
    return clientCallback_->onStreamHeaders(std::move(payload));
  }
  void resetServerCallback(StreamServerCallback& serverCallback) override {
    serverCallback_ = &serverCallback;
  }

  /** StreamServerCallback methods **/
  bool onStreamRequestN(uint64_t tokens) override {
    return serverCallback_->onStreamRequestN(tokens);
  }
  void onStreamCancel() override {
    if (serverCallback_) {
      serverCallback_->onStreamCancel();
      delete this;
      return;
    }
    clientCallback_ = nullptr;
  }
  bool onSinkHeaders(HeadersPayload&& payload) override {
    return serverCallback_->onSinkHeaders(std::move(payload));
  }
  void resetClientCallback(StreamClientCallback& clientCallback) override {
    clientCallback_ = &clientCallback;
  }

  StreamClientCallback* clientCallback_;
  StreamServerCallback* serverCallback_;
  folly::Function<folly::exception_wrapper()> injectFault_;
};

class FaultInjectionChannel : public RequestChannel {
 public:
  FaultInjectionChannel(
      RequestChannel::Ptr client,
      ScopedServerInterfaceThread::FaultInjectionFunc injectFault,
      ScopedServerInterfaceThread::StreamFaultInjectionFunc streamInjectFault)
      : client_(std::move(client)),
        injectFault_(std::move(injectFault)),
        streamInjectFault_(std::move(streamInjectFault)) {}

  using RequestChannel::sendRequestNoResponse;
  using RequestChannel::sendRequestResponse;
  using RequestChannel::sendRequestSink;
  using RequestChannel::sendRequestStream;

  void sendRequestResponse(
      const RpcOptions& rpcOptions,
      MethodMetadata&& methodMetadata,
      SerializedRequest&& req,
      std::shared_ptr<transport::THeader> header,
      RequestClientCallback::Ptr clientCallback,
      std::unique_ptr<folly::IOBuf> frameworkMetadata) override {
    if (auto ex = injectFault_ ? injectFault_(methodMetadata.name_view())
                               : folly::exception_wrapper()) {
      clientCallback.release()->onResponseError(std::move(ex));
      return;
    }
    client_->sendRequestResponse(
        rpcOptions,
        std::move(methodMetadata),
        std::move(req),
        std::move(header),
        std::move(clientCallback),
        std::move(frameworkMetadata));
  }
  void sendRequestNoResponse(
      const RpcOptions& rpcOptions,
      MethodMetadata&& methodMetadata,
      SerializedRequest&& req,
      std::shared_ptr<transport::THeader> header,
      RequestClientCallback::Ptr clientCallback,
      std::unique_ptr<folly::IOBuf> frameworkMetadata) override {
    if (auto ex = injectFault_ ? injectFault_(methodMetadata.name_view())
                               : folly::exception_wrapper()) {
      clientCallback.release()->onResponseError(std::move(ex));
      return;
    }
    client_->sendRequestNoResponse(
        rpcOptions,
        std::move(methodMetadata),
        std::move(req),
        std::move(header),
        std::move(clientCallback),
        std::move(frameworkMetadata));
  }
  void sendRequestStream(
      const RpcOptions& rpcOptions,
      MethodMetadata&& methodMetadata,
      SerializedRequest&& req,
      std::shared_ptr<transport::THeader> header,
      StreamClientCallback* clientCallback,
      std::unique_ptr<folly::IOBuf> frameworkMetadata) override {
    if (auto ex = injectFault_ ? injectFault_(methodMetadata.name_view())
                               : folly::exception_wrapper()) {
      clientCallback->onFirstResponseError(std::move(ex));
      return;
    }
    if (streamInjectFault_) {
      if (auto streamInject = streamInjectFault_(methodMetadata.name_view())) {
        clientCallback = new StreamFaultInjectionCallback(
            clientCallback, std::move(streamInject));
      }
    }
    client_->sendRequestStream(
        rpcOptions,
        std::move(methodMetadata),
        std::move(req),
        std::move(header),
        clientCallback,
        std::move(frameworkMetadata));
  }
  void sendRequestSink(
      const RpcOptions& rpcOptions,
      MethodMetadata&& methodMetadata,
      SerializedRequest&& req,
      std::shared_ptr<transport::THeader> header,
      SinkClientCallback* clientCallback,
      std::unique_ptr<folly::IOBuf> frameworkMetadata) override {
    if (auto ex = injectFault_ ? injectFault_(methodMetadata.name_view())
                               : folly::exception_wrapper()) {
      clientCallback->onFirstResponseError(std::move(ex));
      return;
    }
    client_->sendRequestSink(
        rpcOptions,
        std::move(methodMetadata),
        std::move(req),
        std::move(header),
        clientCallback,
        std::move(frameworkMetadata));
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
  ScopedServerInterfaceThread::StreamFaultInjectionFunc streamInjectFault_;
};

void validateServiceName(
    AsyncProcessorFactory& apf, std::string_view serviceName);
} // namespace detail

template <class AsyncClientT>
std::unique_ptr<AsyncClientT> ScopedServerInterfaceThread::newStickyClient(
    folly::Executor* callbackExecutor,
    ScopedServerInterfaceThread::MakeChannelFunc makeChannel,
    protocol::PROTOCOL_TYPES prot) const {
  return std::make_unique<AsyncClientT>(
      newChannel(callbackExecutor, std::move(makeChannel), 1, prot));
}

template <class AsyncClientT>
std::unique_ptr<AsyncClientT> ScopedServerInterfaceThread::newClient(
    folly::Executor* callbackExecutor,
    ScopedServerInterfaceThread::MakeChannelFunc makeChannel,
    protocol::PROTOCOL_TYPES prot) const {
  return std::make_unique<AsyncClientT>(newChannel(
      callbackExecutor,
      std::move(makeChannel),
      folly::available_concurrency(),
      prot));
}

template <class AsyncClientT>
std::unique_ptr<AsyncClientT>
ScopedServerInterfaceThread::newClientWithFaultInjection(
    ScopedServerInterfaceThread::FaultInjectionFunc injectFault,
    folly::Executor* callbackExecutor,
    ScopedServerInterfaceThread::MakeChannelFunc makeChannel,
    ScopedServerInterfaceThread::StreamFaultInjectionFunc streamInjectFault)
    const {
  return std::make_unique<AsyncClientT>(
      std::make_shared<apache::thrift::detail::FaultInjectionChannel>(
          newChannel(
              callbackExecutor,
              std::move(makeChannel),
              folly::available_concurrency(),
              protocol::T_BINARY_PROTOCOL),
          std::move(injectFault),
          std::move(streamInjectFault)));
}

template <class AsyncClientT>
std::unique_ptr<AsyncClientT>
ScopedServerInterfaceThread::newClientWithInterceptors(
    folly::Executor* callbackExecutor,
    ScopedServerInterfaceThread::MakeChannelFunc makeChannel,
    protocol::PROTOCOL_TYPES prot,
    InterceptorList interceptors) const {
  return std::make_unique<AsyncClientT>(
      newChannel(
          callbackExecutor,
          std::move(makeChannel),
          folly::available_concurrency(),
          prot),
      std::move(interceptors));
}

template <class AsyncClientT>
std::unique_ptr<AsyncClientT> makeTestClient(
    std::shared_ptr<AsyncProcessorFactory> apf,
    ScopedServerInterfaceThread::FaultInjectionFunc injectFault,
    ScopedServerInterfaceThread::StreamFaultInjectionFunc streamInjectFault,
    protocol::PROTOCOL_TYPES prot) {
  auto client = std::make_unique<AsyncClientT>(
      ScopedServerInterfaceThread::makeTestClientChannel(
          apf, std::move(injectFault), std::move(streamInjectFault), prot));
  apache::thrift::detail::validateServiceName(*apf, client->getServiceName());
  return client;
}

template <class ServiceHandler, class ServiceTag>
std::unique_ptr<Client<ServiceTag>> makeTestClient(
    std::shared_ptr<ServiceHandler> handler,
    ScopedServerInterfaceThread::FaultInjectionFunc injectFault,
    ScopedServerInterfaceThread::StreamFaultInjectionFunc streamInjectFault,
    protocol::PROTOCOL_TYPES prot) {
  return makeTestClient<Client<ServiceTag>>(
      std::static_pointer_cast<AsyncProcessorFactory>(std::move(handler)),
      std::move(injectFault),
      std::move(streamInjectFault),
      prot);
}

} // namespace apache::thrift
