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

#include <thrift/lib/cpp2/async/ReconnectingRequestChannel.h>

#include <memory>
#include <stdexcept>
#include <utility>

#include <folly/ExceptionWrapper.h>

namespace apache::thrift {

namespace {
class ChannelKeepAlive : public RequestClientCallback {
 public:
  ChannelKeepAlive(
      ReconnectingRequestChannel::ImplPtr impl, RequestClientCallback::Ptr cob)
      : keepAlive_(std::move(impl)), cob_(std::move(cob)) {}

  void onResponse(ClientReceiveState&& state) noexcept override {
    cob_.release()->onResponse(std::move(state));
    delete this;
  }

  void onResponseError(folly::exception_wrapper ex) noexcept override {
    cob_.release()->onResponseError(std::move(ex));
    delete this;
  }

 private:
  ReconnectingRequestChannel::ImplPtr keepAlive_;
  RequestClientCallback::Ptr cob_;
};

class ChannelKeepAliveStream : public StreamClientCallback {
 public:
  ChannelKeepAliveStream(
      ReconnectingRequestChannel::ImplPtr impl,
      StreamClientCallback& clientCallback)
      : keepAlive_(std::move(impl)), clientCallback_(clientCallback) {}

  bool onFirstResponse(
      FirstResponsePayload&& firstResponsePayload,
      folly::EventBase* evb,
      StreamServerCallback* serverCallback) override {
    SCOPE_EXIT {
      delete this;
    };
    serverCallback->resetClientCallback(clientCallback_);
    return clientCallback_.onFirstResponse(
        std::move(firstResponsePayload), evb, serverCallback);
  }
  void onFirstResponseError(folly::exception_wrapper ew) override {
    SCOPE_EXIT {
      delete this;
    };
    return clientCallback_.onFirstResponseError(std::move(ew));
  }

  bool onStreamNext(StreamPayload&&) override { std::terminate(); }
  void onStreamError(folly::exception_wrapper) override { std::terminate(); }
  void onStreamComplete() override { std::terminate(); }
  void resetServerCallback(StreamServerCallback&) override { std::terminate(); }

 private:
  ReconnectingRequestChannel::ImplPtr keepAlive_;
  StreamClientCallback& clientCallback_;
};

class ChannelKeepAliveSink : public SinkClientCallback {
 public:
  ChannelKeepAliveSink(
      ReconnectingRequestChannel::ImplPtr impl,
      SinkClientCallback& clientCallback)
      : keepAlive_(std::move(impl)), clientCallback_(clientCallback) {}

  bool onFirstResponse(
      FirstResponsePayload&& firstResponsePayload,
      folly::EventBase* evb,
      SinkServerCallback* serverCallback) override {
    SCOPE_EXIT {
      delete this;
    };
    serverCallback->resetClientCallback(clientCallback_);
    return clientCallback_.onFirstResponse(
        std::move(firstResponsePayload), evb, serverCallback);
  }
  void onFirstResponseError(folly::exception_wrapper ew) override {
    SCOPE_EXIT {
      delete this;
    };
    return clientCallback_.onFirstResponseError(std::move(ew));
  }

  void onFinalResponse(StreamPayload&&) override { std::terminate(); }
  void onFinalResponseError(folly::exception_wrapper) override {
    std::terminate();
  }
  bool onSinkRequestN(uint64_t) override { std::terminate(); }
  void resetServerCallback(SinkServerCallback&) override { std::terminate(); }

 private:
  ReconnectingRequestChannel::ImplPtr keepAlive_;
  SinkClientCallback& clientCallback_;
};

static const folly::exception_wrapper pendingRequestOverflowError() {
  return folly::make_exception_wrapper<std::logic_error>(
      "Request Queue Overflowed!");
}

static const folly::exception_wrapper requestQueueNonEmptyAtDestructionError() {
  return folly::make_exception_wrapper<std::logic_error>(
      "Channel destroyed while request still pending!");
}

} // namespace

void ReconnectingRequestChannel::sendRequestResponse(
    const RpcOptions& options,
    MethodMetadata&& methodMetadata,
    SerializedRequest&& request,
    std::shared_ptr<transport::THeader> header,
    RequestClientCallback::Ptr cob,
    std::unique_ptr<folly::IOBuf> frameworkMetadata) {
  if (!isChannelGood()) {
    if (useRequestQueue_) {
      if (requestQueue_.size() >= requestQueueLimit) {
        cob.release()->onResponseError(pendingRequestOverflowError());
        return;
      }
      requestQueue_.emplace_back(
          [this,
           options = options,
           methodMetadata = std::move(methodMetadata),
           request = std::move(request),
           header = std::move(header),
           cob = std::move(cob),
           frameworkMetadata =
               std::move(frameworkMetadata)](bool failRequest) mutable {
            if (failRequest) {
              cob.release()->onResponseError(
                  requestQueueNonEmptyAtDestructionError());
              return;
            }
            cob = RequestClientCallback::Ptr(
                new ChannelKeepAlive(impl_, std::move(cob)));
            return impl_->sendRequestResponse(
                std::move(options),
                std::move(methodMetadata),
                std::move(request),
                std::move(header),
                std::move(cob),
                std::move(frameworkMetadata));
          });
      reconnectRequestChannelWithCallback();
      // Callback will invoke this request after successful reconnecting.
      return;
    }
    reconnectRequestChannel();
  }
  DCHECK_EQ(requestQueue_.size(), 0);

  cob = RequestClientCallback::Ptr(new ChannelKeepAlive(impl_, std::move(cob)));
  return impl_->sendRequestResponse(
      options,
      std::move(methodMetadata),
      std::move(request),
      std::move(header),
      std::move(cob),
      std::move(frameworkMetadata));
}

void ReconnectingRequestChannel::sendRequestNoResponse(
    const RpcOptions& options,
    MethodMetadata&& methodMetadata,
    SerializedRequest&& request,
    std::shared_ptr<transport::THeader> header,
    RequestClientCallback::Ptr cob,
    std::unique_ptr<folly::IOBuf> frameworkMetadata) {
  if (!isChannelGood()) {
    if (useRequestQueue_) {
      if (requestQueue_.size() >= requestQueueLimit) {
        cob.release()->onResponseError(pendingRequestOverflowError());
        return;
      }
      requestQueue_.emplace_back(
          [this,
           options = options,
           methodMetadata = std::move(methodMetadata),
           request = std::move(request),
           header = std::move(header),
           cob = std::move(cob),
           frameworkMetadata =
               std::move(frameworkMetadata)](bool failRequest) mutable {
            if (failRequest) {
              cob.release()->onResponseError(
                  requestQueueNonEmptyAtDestructionError());
              return;
            }
            cob = RequestClientCallback::Ptr(
                new ChannelKeepAlive(impl_, std::move(cob)));
            return impl_->sendRequestNoResponse(
                std::move(options),
                std::move(methodMetadata),
                std::move(request),
                std::move(header),
                std::move(cob),
                std::move(frameworkMetadata));
          });
      reconnectRequestChannelWithCallback();
      // Callback will invoke this request after successful reconnecting.
      return;
    }
    reconnectRequestChannel();
  }
  cob = RequestClientCallback::Ptr(new ChannelKeepAlive(impl_, std::move(cob)));

  return impl_->sendRequestNoResponse(
      options,
      std::move(methodMetadata),
      std::move(request),
      std::move(header),
      std::move(cob),
      std::move(frameworkMetadata));
}

void ReconnectingRequestChannel::sendRequestStream(
    const RpcOptions& options,
    MethodMetadata&& methodMetadata,
    SerializedRequest&& request,
    std::shared_ptr<transport::THeader> header,
    StreamClientCallback* cob,
    std::unique_ptr<folly::IOBuf> frameworkMetadata) {
  if (!isChannelGood()) {
    if (useRequestQueue_) {
      if (requestQueue_.size() >= requestQueueLimit) {
        cob->onFirstResponseError(pendingRequestOverflowError());
        return;
      }
      requestQueue_.emplace_back([this,
                                  options = options,
                                  methodMetadata = std::move(methodMetadata),
                                  request = std::move(request),
                                  header = std::move(header),
                                  cob = std::move(cob),
                                  frameworkMetadata =
                                      std::move(frameworkMetadata)](
                                     bool failRequest) mutable {
        if (failRequest) {
          cob->onFirstResponseError(requestQueueNonEmptyAtDestructionError());
          return;
        }
        cob = new ChannelKeepAliveStream(impl_, *cob);
        return impl_->sendRequestStream(
            std::move(options),
            std::move(methodMetadata),
            std::move(request),
            std::move(header),
            cob,
            std::move(frameworkMetadata));
      });
      reconnectRequestChannelWithCallback();
      // Callback will invoke this request after successful reconnecting.
      return;
    }
    reconnectRequestChannel();
  }
  cob = new ChannelKeepAliveStream(impl_, *cob);

  return impl_->sendRequestStream(
      options,
      std::move(methodMetadata),
      std::move(request),
      std::move(header),
      cob,
      std::move(frameworkMetadata));
}

void ReconnectingRequestChannel::sendRequestSink(
    const RpcOptions& options,
    MethodMetadata&& methodMetadata,
    SerializedRequest&& request,
    std::shared_ptr<transport::THeader> header,
    SinkClientCallback* cob,
    std::unique_ptr<folly::IOBuf> frameworkMetadata) {
  if (!isChannelGood()) {
    if (useRequestQueue_) {
      if (requestQueue_.size() >= requestQueueLimit) {
        cob->onFirstResponseError(pendingRequestOverflowError());
        return;
      }
      requestQueue_.emplace_back([this,
                                  options = options,
                                  methodMetadata = std::move(methodMetadata),
                                  request = std::move(request),
                                  header = std::move(header),
                                  cob = std::move(cob),
                                  frameworkMetadata =
                                      std::move(frameworkMetadata)](
                                     bool failRequest) mutable {
        if (failRequest) {
          cob->onFirstResponseError(requestQueueNonEmptyAtDestructionError());
          return;
        }
        cob = new ChannelKeepAliveSink(impl_, *cob);
        return impl_->sendRequestSink(
            std::move(options),
            std::move(methodMetadata),
            std::move(request),
            std::move(header),
            cob,
            std::move(frameworkMetadata));
      });
      reconnectRequestChannelWithCallback();
      // sendQueuedRequests() will take care of sending this request.
      return;
    }
    reconnectRequestChannel();
  }

  cob = new ChannelKeepAliveSink(impl_, *cob);

  return impl_->sendRequestSink(
      options,
      std::move(methodMetadata),
      std::move(request),
      std::move(header),
      cob,
      std::move(frameworkMetadata));
}

void ReconnectingRequestChannel::connectSuccess() noexcept {
  isReconnecting_ = false;
  sendQueuedRequests();
}

void ReconnectingRequestChannel::connectErr(
    const folly::AsyncSocketException& ex) noexcept {
  isReconnecting_ = false;
  VLOG(1) << "Reconnecting Failed with Error: " << ex.what();
  // With reconnecting failed, sendQueuedRequests() will send request and clean
  // the queue via bad channel and fail each request.
  sendQueuedRequests();
}

bool ReconnectingRequestChannel::isChannelGood() {
  if (impl_ && !isReconnecting_) {
    // If we're not reconnecting, rely on channel to tell us if it's broken.
    return impl_->good();
  }

  return false;
}

void ReconnectingRequestChannel::reconnectRequestChannel() {
  impl_ = implCreator_(evb_);
}

void ReconnectingRequestChannel::reconnectRequestChannelWithCallback() {
  if (isReconnecting_ || isCreatingChannel_) {
    // Another request is doing reconnecting.
    return;
  }
  isReconnecting_ = true;
  // Creator might call connectSuccess/connectErr inline, so we protect
  // against that via isCreatingChannel_.
  isCreatingChannel_ = true;
  impl_ = implCreatorWithCallback_(evb_, *this);
  isCreatingChannel_ = false;
  sendQueuedRequests();
}

void ReconnectingRequestChannel::sendQueuedRequests() {
  // Only send request while channel and socket are both ready.
  if (!isCreatingChannel_ && !isReconnecting_) {
    while (!requestQueue_.empty()) {
      requestQueue_.front()(/*failRequest=*/false);
      requestQueue_.pop_front();
    }
  }
}

void ReconnectingRequestChannel::terminateInteraction(InteractionId id) {
  if (!isChannelGood()) {
    if (useRequestQueue_) {
      reconnectRequestChannelWithCallback();
    } else {
      reconnectRequestChannel();
    }
  }
  impl_->terminateInteraction(std::move(id));
}

InteractionId ReconnectingRequestChannel::createInteraction(
    ManagedStringView&& name) {
  if (!isChannelGood()) {
    if (useRequestQueue_) {
      reconnectRequestChannelWithCallback();
    } else {
      reconnectRequestChannel();
    }
  }
  return impl_->createInteraction(std::move(name));
}

InteractionId ReconnectingRequestChannel::registerInteraction(
    ManagedStringView&& name, int64_t id) {
  if (!isChannelGood()) {
    if (useRequestQueue_) {
      reconnectRequestChannelWithCallback();
    } else {
      reconnectRequestChannel();
    }
  }
  return impl_->registerInteraction(std::move(name), id);
}

} // namespace apache::thrift
