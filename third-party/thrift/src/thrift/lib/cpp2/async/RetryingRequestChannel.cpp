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

#include <thrift/lib/cpp2/async/RetryingRequestChannel.h>

namespace apache::thrift {

class RetryingRequestChannel::RequestCallbackBase {
 protected:
  RequestCallbackBase(
      folly::Executor::KeepAlive<> ka,
      RetryingRequestChannel::ImplPtr impl,
      int retries,
      const apache::thrift::RpcOptions& options,
      folly::StringPiece methodName,
      SerializedRequest&& request,
      std::shared_ptr<apache::thrift::transport::THeader> header)
      : impl_(std::move(impl)),
        retriesLeft_(retries),
        options_(options),
        methodName_(methodName.str()),
        request_(std::move(request)),
        header_(std::move(header)) {
    if (retriesLeft_) {
      ka_ = std::move(ka);
    }
  }

  bool shouldRetry(folly::exception_wrapper& ex) {
    if (!ex.is_compatible_with<
            apache::thrift::transport::TTransportException>()) {
      return false;
    }
    return retriesLeft_ > 0;
  }

  folly::Executor::KeepAlive<> ka_;
  RetryingRequestChannel::ImplPtr impl_;
  int retriesLeft_;
  apache::thrift::RpcOptions options_;
  std::string methodName_;
  SerializedRequest request_;
  std::shared_ptr<apache::thrift::transport::THeader> header_;
};

class RetryingRequestChannel::RequestCallback
    : public RetryingRequestChannel::RequestCallbackBase,
      public apache::thrift::RequestClientCallback {
 public:
  RequestCallback(
      folly::Executor::KeepAlive<> ka,
      RetryingRequestChannel::ImplPtr impl,
      int retries,
      const apache::thrift::RpcOptions& options,
      apache::thrift::RequestClientCallback::Ptr cob,
      folly::StringPiece methodName,
      SerializedRequest&& request,
      std::shared_ptr<apache::thrift::transport::THeader> header)
      : RequestCallbackBase(
            std::move(ka),
            std::move(impl),
            retries,
            options,
            std::move(methodName),
            std::move(request),
            header),
        cob_(std::move(cob)) {}

  void onResponse(
      apache::thrift::ClientReceiveState&& state) noexcept override {
    cob_.release()->onResponse(std::move(state));
    delete this;
  }

  void onResponseError(folly::exception_wrapper ex) noexcept override {
    if (shouldRetry(ex)) {
      retry();
    } else {
      cob_.release()->onResponseError(std::move(ex));
      delete this;
    }
  }

  void retry() {
    if (!--retriesLeft_) {
      ka_.reset();
    }

    impl_->sendRequestResponse(
        options_,
        methodName_,
        SerializedRequest(request_.buffer->clone()),
        header_,
        RequestClientCallback::Ptr(this),
        // TODO(ezou) This needs some handling due to ownership - need follow up
        nullptr);
  }

 private:
  RequestClientCallback::Ptr cob_;
};

class RetryingRequestChannel::StreamCallback
    : public RetryingRequestChannel::RequestCallbackBase,
      public apache::thrift::StreamClientCallback {
 public:
  StreamCallback(
      folly::Executor::KeepAlive<> ka,
      RetryingRequestChannel::ImplPtr impl,
      int retries,
      const apache::thrift::RpcOptions& options,
      apache::thrift::StreamClientCallback& clientCallback,
      folly::StringPiece methodName,
      SerializedRequest&& request,
      std::shared_ptr<apache::thrift::transport::THeader> header)
      : RequestCallbackBase(
            std::move(ka),
            std::move(impl),
            retries,
            options,
            methodName,
            std::move(request),
            header),
        clientCallback_(clientCallback) {}

  bool onFirstResponse(
      FirstResponsePayload&& pload,
      folly::EventBase* evb,
      StreamServerCallback* serverCallback) noexcept override {
    SCOPE_EXIT {
      delete this;
    };
    serverCallback->resetClientCallback(clientCallback_);
    return clientCallback_.onFirstResponse(
        std::move(pload), evb, serverCallback);
  }

  void onFirstResponseError(folly::exception_wrapper ex) noexcept override {
    if (shouldRetry(ex)) {
      retry();
    } else {
      clientCallback_.onFirstResponseError(std::move(ex));
      delete this;
    }
  }

  bool onStreamNext(StreamPayload&&) override { std::terminate(); }

  void onStreamError(folly::exception_wrapper) override { std::terminate(); }

  void onStreamComplete() override { std::terminate(); }

  bool onStreamHeaders(HeadersPayload&&) override { std::terminate(); }

  void resetServerCallback(StreamServerCallback&) override { std::terminate(); }

 private:
  void retry() {
    if (!--retriesLeft_) {
      ka_.reset();
    }

    impl_->sendRequestStream(
        options_,
        methodName_,
        SerializedRequest(request_.buffer->clone()),
        header_,
        this,
        nullptr);
  }

  StreamClientCallback& clientCallback_;
};

class RetryingRequestChannel::SinkCallback
    : public RetryingRequestChannel::RequestCallbackBase,
      public apache::thrift::SinkClientCallback {
 public:
  SinkCallback(
      folly::Executor::KeepAlive<> ka,
      RetryingRequestChannel::ImplPtr impl,
      int retries,
      const apache::thrift::RpcOptions& options,
      apache::thrift::SinkClientCallback& clientCallback,
      folly::StringPiece methodName,
      SerializedRequest&& request,
      std::shared_ptr<apache::thrift::transport::THeader> header)
      : RequestCallbackBase(
            std::move(ka),
            std::move(impl),
            retries,
            options,
            methodName,
            std::move(request),
            header),
        clientCallback_(clientCallback) {}

  bool onFirstResponse(
      FirstResponsePayload&& pload,
      folly::EventBase* evb,
      SinkServerCallback* serverCallback) noexcept override {
    SCOPE_EXIT {
      delete this;
    };
    serverCallback->resetClientCallback(clientCallback_);
    return clientCallback_.onFirstResponse(
        std::move(pload), evb, serverCallback);
  }

  void onFirstResponseError(folly::exception_wrapper ex) noexcept override {
    if (shouldRetry(ex)) {
      retry();
    } else {
      clientCallback_.onFirstResponseError(std::move(ex));
      delete this;
    }
  }

  void onFinalResponse(StreamPayload&&) override { std::terminate(); }

  void onFinalResponseError(folly::exception_wrapper) override {
    std::terminate();
  }

  bool onSinkRequestN(int32_t) override { std::terminate(); }

  void resetServerCallback(SinkServerCallback&) override { std::terminate(); }

 private:
  void retry() {
    if (!--retriesLeft_) {
      ka_.reset();
    }

    impl_->sendRequestSink(
        options_,
        methodName_,
        SerializedRequest(request_.buffer->clone()),
        header_,
        this,
        // TODO(ezou) This needs some handling due to ownership - need follow up
        nullptr);
  }

  SinkClientCallback& clientCallback_;
};

void RetryingRequestChannel::sendRequestStream(
    const apache::thrift::RpcOptions& rpcOptions,
    MethodMetadata&& methodMetadata,
    apache::thrift::SerializedRequest&& request,
    std::shared_ptr<apache::thrift::transport::THeader> header,
    apache::thrift::StreamClientCallback* clientCallback,
    std::unique_ptr<folly::IOBuf> /*unused*/) {
  apache::thrift::StreamClientCallback* streamCallback = new StreamCallback(
      folly::getKeepAliveToken(evb_),
      impl_,
      numRetries_,
      rpcOptions,
      *clientCallback,
      methodMetadata.name_view(),
      SerializedRequest(request.buffer->clone()),
      header);

  return impl_->sendRequestStream(
      rpcOptions,
      std::move(methodMetadata),
      std::move(request),
      std::move(header),
      streamCallback,
      // TODO(ezou) This needs some handling due to ownership - need follow up
      nullptr);
}

void RetryingRequestChannel::sendRequestSink(
    const apache::thrift::RpcOptions& rpcOptions,
    apache::thrift::MethodMetadata&& methodMetadata,
    apache::thrift::SerializedRequest&& request,
    std::shared_ptr<apache::thrift::transport::THeader> header,
    apache::thrift::SinkClientCallback* clientCallback,
    std::unique_ptr<folly::IOBuf> /*unused*/) {
  apache::thrift::SinkClientCallback* sinkCallback = new SinkCallback(
      folly::getKeepAliveToken(evb_),
      impl_,
      numRetries_,
      rpcOptions,
      *clientCallback,
      methodMetadata.name_view(),
      SerializedRequest(request.buffer->clone()),
      header);

  return impl_->sendRequestSink(
      rpcOptions,
      std::move(methodMetadata),
      std::move(request),
      std::move(header),
      sinkCallback,
      nullptr);
}

void RetryingRequestChannel::sendRequestResponse(
    const apache::thrift::RpcOptions& options,
    MethodMetadata&& methodMetadata,
    SerializedRequest&& request,
    std::shared_ptr<apache::thrift::transport::THeader> header,
    RequestClientCallback::Ptr cob,
    std::unique_ptr<folly::IOBuf> /*unused*/) {
  cob = RequestClientCallback::Ptr(new RequestCallback(
      folly::getKeepAliveToken(evb_),
      impl_,
      numRetries_,
      options,
      std::move(cob),
      methodMetadata.name_view(),
      SerializedRequest(request.buffer->clone()),
      header));

  return impl_->sendRequestResponse(
      options,
      std::move(methodMetadata),
      std::move(request),
      std::move(header),
      std::move(cob),
      nullptr);
}
} // namespace apache::thrift
