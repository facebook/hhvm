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

#include <folly/ExceptionWrapper.h>
#include <thrift/lib/cpp2/async/RequestCallback.h>
#include <thrift/lib/cpp2/async/StreamCallbacks.h>

namespace apache::thrift {

template <class RequestGuardType>
class GuardedRequestClientCallback : public RequestClientCallback {
 public:
  explicit GuardedRequestClientCallback(RequestClientCallback::Ptr cb)
      : clientCallback_(std::move(cb)) {}

  bool isInlineSafe() const override { return clientCallback_->isInlineSafe(); }

  bool isSync() const override { return clientCallback_->isSync(); }

  folly::Executor::KeepAlive<> getExecutor() const override {
    return clientCallback_->getExecutor();
  }

  void onResponse(ClientReceiveState&& state) noexcept override {
    clientCallback_.release()->onResponse(std::move(state));
    delete this;
  }

  void onResponseError(folly::exception_wrapper ex) noexcept override {
    clientCallback_.release()->onResponseError(std::move(ex));
    delete this;
  }

 private:
  RequestClientCallback::Ptr clientCallback_;
  RequestGuardType guard_;
};

template <class RequestGuardType>
class GuardedStreamCallback : public StreamClientCallback,
                              public StreamServerCallback {
 public:
  explicit GuardedStreamCallback(StreamClientCallback* callback)
      : clientCallback_(std::move(callback)) {}

  ~GuardedStreamCallback() override {
    DCHECK(!!clientCallback_ == !!serverCallback_);
    if (clientCallback_ && serverCallback_) {
      clientCallback_->resetServerCallback(*serverCallback_);
      serverCallback_->resetClientCallback(*clientCallback_);
    }
  }

  bool onFirstResponse(
      FirstResponsePayload&& firstResponsePayload,
      folly::EventBase* evb,
      StreamServerCallback* serverCallback) override {
    DCHECK(clientCallback_);
    DCHECK(!serverCallback_);
    serverCallback_ = serverCallback;
    return clientCallback_->onFirstResponse(
        std::move(firstResponsePayload), evb, this);
  }

  void onFirstResponseError(folly::exception_wrapper ew) override {
    DCHECK(clientCallback_);
    DCHECK(!serverCallback_);
    std::exchange(clientCallback_, nullptr)
        ->onFirstResponseError(std::move(ew));
    delete this;
  }

  bool onStreamNext(StreamPayload&& payload) override {
    DCHECK(clientCallback_);
    return clientCallback_->onStreamNext(std::move(payload));
  }

  bool onStreamHeaders(apache::thrift::HeadersPayload&& payload) override {
    DCHECK(clientCallback_);
    return clientCallback_->onStreamHeaders(std::move(payload));
  }

  void onStreamError(folly::exception_wrapper ex) override {
    DCHECK(clientCallback_);
    serverCallback_ = nullptr;
    std::exchange(clientCallback_, nullptr)->onStreamError(std::move(ex));
    delete this;
  }

  void onStreamComplete() override {
    DCHECK(clientCallback_);
    serverCallback_ = nullptr;
    std::exchange(clientCallback_, nullptr)->onStreamComplete();
    delete this;
  }

  void resetServerCallback(StreamServerCallback& callback) override {
    serverCallback_ = &callback;
  }

  bool onStreamRequestN(int32_t n) override {
    DCHECK(serverCallback_);
    return serverCallback_->onStreamRequestN(n);
  }

  void onStreamCancel() override {
    DCHECK(serverCallback_);
    clientCallback_ = nullptr;
    std::exchange(serverCallback_, nullptr)->onStreamCancel();
    delete this;
  }

  bool onSinkHeaders(apache::thrift::HeadersPayload&& payload) override {
    DCHECK(serverCallback_);
    return serverCallback_->onSinkHeaders(std::move(payload));
  }

  void resetClientCallback(
      apache::thrift::StreamClientCallback& clientCallback) override {
    clientCallback_ = &clientCallback;
  }

 private:
  StreamClientCallback* clientCallback_{nullptr};
  StreamServerCallback* serverCallback_{nullptr};
  RequestGuardType guard_;
};

template <class RequestGuardType>
class GuardedSinkCallback : public SinkClientCallback,
                            public SinkServerCallback {
 public:
  explicit GuardedSinkCallback(SinkClientCallback* callback)
      : clientCallback_(std::move(callback)) {}

  ~GuardedSinkCallback() override {
    DCHECK(!!clientCallback_ == !!serverCallback_);
    if (clientCallback_ && serverCallback_) {
      clientCallback_->resetServerCallback(*serverCallback_);
      serverCallback_->resetClientCallback(*clientCallback_);
    }
  }

  bool onFirstResponse(
      FirstResponsePayload&& firstResponsePayload,
      folly::EventBase* evb,
      SinkServerCallback* serverCallback) override {
    DCHECK(clientCallback_);
    DCHECK(!serverCallback_);
    serverCallback_ = serverCallback;
    return clientCallback_->onFirstResponse(
        std::move(firstResponsePayload), evb, this);
  }

  void onFirstResponseError(folly::exception_wrapper ew) override {
    DCHECK(clientCallback_);
    DCHECK(!serverCallback_);
    std::exchange(clientCallback_, nullptr)
        ->onFirstResponseError(std::move(ew));
    delete this;
  }

  void onFinalResponse(StreamPayload&& payload) override {
    DCHECK(clientCallback_);
    serverCallback_ = nullptr;
    std::exchange(clientCallback_, nullptr)
        ->onFinalResponse(std::move(payload));
    delete this;
  }

  void onFinalResponseError(folly::exception_wrapper ew) override {
    DCHECK(clientCallback_);
    serverCallback_ = nullptr;
    std::exchange(clientCallback_, nullptr)
        ->onFinalResponseError(std::move(ew));
    delete this;
  }

  bool onSinkRequestN(int32_t n) override {
    DCHECK(clientCallback_);
    return clientCallback_->onSinkRequestN(n);
  }

  void resetServerCallback(SinkServerCallback& callback) override {
    serverCallback_ = &callback;
  }

  bool onSinkNext(StreamPayload&& payload) override {
    DCHECK(serverCallback_);
    return serverCallback_->onSinkNext(std::move(payload));
  }

  void onSinkError(folly::exception_wrapper ex) override {
    DCHECK(serverCallback_);
    clientCallback_ = nullptr;
    std::exchange(serverCallback_, nullptr)->onSinkError(std::move(ex));
    delete this;
  }

  bool onSinkComplete() override {
    DCHECK(serverCallback_);
    return serverCallback_->onSinkComplete();
  }

  void resetClientCallback(SinkClientCallback& clientCallback) override {
    clientCallback_ = &clientCallback;
  }

 private:
  SinkClientCallback* clientCallback_{nullptr};
  SinkServerCallback* serverCallback_{nullptr};
  RequestGuardType guard_;
};

template <class RequestGuardType, class ChannelGuardType>
void GuardedRequestChannel<RequestGuardType, ChannelGuardType>::
    setCloseCallback(CloseCallback* callback) {
  impl_->setCloseCallback(std::move(callback));
}

template <class RequestGuardType, class ChannelGuardType>
folly::EventBase*
GuardedRequestChannel<RequestGuardType, ChannelGuardType>::getEventBase()
    const {
  return impl_->getEventBase();
}

template <class RequestGuardType, class ChannelGuardType>
uint16_t
GuardedRequestChannel<RequestGuardType, ChannelGuardType>::getProtocolId() {
  return impl_->getProtocolId();
}

template <class RequestGuardType, class ChannelGuardType>
void GuardedRequestChannel<RequestGuardType, ChannelGuardType>::
    terminateInteraction(InteractionId id) {
  impl_->terminateInteraction(std::move(id));
}

template <class RequestGuardType, class ChannelGuardType>
InteractionId
GuardedRequestChannel<RequestGuardType, ChannelGuardType>::createInteraction(
    ManagedStringView&& name) {
  return impl_->createInteraction(std::move(name));
}

template <class RequestGuardType, class ChannelGuardType>
InteractionId
GuardedRequestChannel<RequestGuardType, ChannelGuardType>::registerInteraction(
    ManagedStringView&& name, int64_t id) {
  return impl_->registerInteraction(std::move(name), id);
}

template <class RequestGuardType, class ChannelGuardType>
void GuardedRequestChannel<RequestGuardType, ChannelGuardType>::
    sendRequestResponse(
        RpcOptions&& rpcOptions,
        MethodMetadata&& methodMetadata,
        SerializedRequest&& serializedRequest,
        std::shared_ptr<transport::THeader> header,
        RequestClientCallback::Ptr cb,
        std::unique_ptr<folly::IOBuf> frameworkMetadata) {
  auto wrappedCb = RequestClientCallback::Ptr(
      new GuardedRequestClientCallback<RequestGuardType>(std::move(cb)));

  impl_->sendRequestResponse(
      std::move(rpcOptions),
      std::move(methodMetadata),
      std::move(serializedRequest),
      std::move(header),
      std::move(wrappedCb),
      std::move(frameworkMetadata));
}

template <class RequestGuardType, class ChannelGuardType>
void GuardedRequestChannel<RequestGuardType, ChannelGuardType>::
    sendRequestStream(
        RpcOptions&& rpcOptions,
        MethodMetadata&& methodMetadata,
        SerializedRequest&& serializedRequest,
        std::shared_ptr<transport::THeader> header,
        StreamClientCallback* cob,
        std::unique_ptr<folly::IOBuf> frameworkMetadata) {
  auto wrappedCb = new GuardedStreamCallback<RequestGuardType>(std::move(cob));

  impl_->sendRequestStream(
      std::move(rpcOptions),
      std::move(methodMetadata),
      std::move(serializedRequest),
      std::move(header),
      wrappedCb,
      std::move(frameworkMetadata));
}

template <class RequestGuardType, class ChannelGuardType>
void GuardedRequestChannel<RequestGuardType, ChannelGuardType>::
    sendRequestNoResponse(
        RpcOptions&& rpcOptions,
        MethodMetadata&& methodMetadata,
        SerializedRequest&& serializedRequest,
        std::shared_ptr<transport::THeader> header,
        RequestClientCallback::Ptr cb,
        std::unique_ptr<folly::IOBuf> frameworkMetadata) {
  auto wrappedCb = RequestClientCallback::Ptr(
      new GuardedRequestClientCallback<RequestGuardType>(std::move(cb)));

  impl_->sendRequestNoResponse(
      std::move(rpcOptions),
      std::move(methodMetadata),
      std::move(serializedRequest),
      std::move(header),
      std::move(wrappedCb),
      std::move(frameworkMetadata));
}

template <class RequestGuardType, class ChannelGuardType>
void GuardedRequestChannel<RequestGuardType, ChannelGuardType>::sendRequestSink(
    RpcOptions&& rpcOptions,
    MethodMetadata&& methodMetadata,
    SerializedRequest&& serializedRequest,
    std::shared_ptr<transport::THeader> header,
    SinkClientCallback* cob,
    std::unique_ptr<folly::IOBuf> frameworkMetadata) {
  auto wrappedCb = new GuardedSinkCallback<RequestGuardType>(std::move(cob));

  impl_->sendRequestSink(
      std::move(rpcOptions),
      std::move(methodMetadata),
      std::move(serializedRequest),
      std::move(header),
      wrappedCb,
      std::move(frameworkMetadata));
}

} // namespace apache::thrift
