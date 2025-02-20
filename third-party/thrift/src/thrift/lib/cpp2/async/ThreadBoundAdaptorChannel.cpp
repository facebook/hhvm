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

#include <thrift/lib/cpp2/async/ThreadBoundAdaptorChannel.h>

namespace apache::thrift {

namespace {

// This callback implementation makes sure that all the callbacks coming from
// the server are executed on this channel's EventBase. This EventBase will be
// passed during the constructor of the channel.
// This is similar to the implementation in PooledRequestChannel.
class EvbRequestCallback final : public RequestClientCallback {
 public:
  EvbRequestCallback(
      RequestClientCallback::Ptr cb,
      folly::Executor::KeepAlive<folly::EventBase> evb)
      : evb_(std::move(evb)), cb_(std::move(cb)) {}

  void onResponse(ClientReceiveState&& rs) noexcept override {
    evb_->runInEventBaseThread(
        [cb = std::move(cb_), rs = std::move(rs)]() mutable {
          cb.release()->onResponse(std::move(rs));
        });
    delete this;
  }

  void onResponseError(folly::exception_wrapper ex) noexcept override {
    evb_->runInEventBaseThread(
        [cb = std::move(cb_), ex = std::move(ex)]() mutable {
          cb.release()->onResponseError(std::move(ex));
        });
    delete this;
  }

 private:
  folly::Executor::KeepAlive<folly::EventBase> evb_;
  RequestClientCallback::Ptr cb_;
};

// This implementation of streaming callbacks is making sure that the calls to
// the client and server sides are being made on the proper threads.
//
// When a callback is received from the server, it is scheduled on client's
// eventbase and when a callback is received from the client, it is scheduled on
// server's eventbase.
//
// There is also refCounting logic in here to know when to clean this callback.
class EvbStreamCallback final : public StreamClientCallback,
                                public StreamServerCallback {
 public:
  EvbStreamCallback(StreamClientCallback* clientCallback, folly::EventBase* evb)
      : clientCallback_(clientCallback),
        evb_(folly::Executor::getKeepAliveToken(evb)) {}

  // StreamClientCallback
  FOLLY_NODISCARD bool onFirstResponse(
      FirstResponsePayload&& firstResponse,
      folly::EventBase* serverEvb,
      StreamServerCallback* serverCallback) noexcept override {
    serverEvb_ = folly::Executor::getKeepAliveToken(serverEvb);
    setServerCallback(serverCallback);

    eventBaseRunHelper(evb_, [this, fr = std::move(firstResponse)]() mutable {
      DCHECK(clientCallback_);
      std::ignore =
          clientCallback_->onFirstResponse(std::move(fr), evb_.get(), this);
    });
    return true;
  }

  // Terminating callback. Clean up the client callback stored.
  void onFirstResponseError(folly::exception_wrapper ew) noexcept override {
    eventBaseRunHelper(evb_, [this, ewr = std::move(ew)]() mutable {
      DCHECK(clientCallback_);
      clientCallback_->onFirstResponseError(std::move(ewr));
      setClientCallback(nullptr);
    });
  }

  bool onStreamNext(StreamPayload&& payload) noexcept override {
    eventBaseRunHelper(evb_, [this, pl = std::move(payload)]() mutable {
      if (clientCallback_) {
        std::ignore = clientCallback_->onStreamNext(std::move(pl));
      }
    });
    return true;
  }

  // Terminating callback. Clean up the client and server callbacks stored.
  void onStreamError(folly::exception_wrapper ew) noexcept override {
    eventBaseRunHelper(evb_, [this, ewr = std::move(ew)]() mutable {
      if (clientCallback_) {
        clientCallback_->onStreamError(std::move(ewr));
        setClientCallback(nullptr);
      }
    });
    setServerCallback(nullptr);
  }

  // Terminating callback. Clean up the client and server callbacks stored.
  void onStreamComplete() noexcept override {
    eventBaseRunHelper(evb_, [this]() mutable {
      if (clientCallback_) {
        clientCallback_->onStreamComplete();
        setClientCallback(nullptr);
      }
    });
    setServerCallback(nullptr);
  }

  void resetServerCallback(
      StreamServerCallback& serverCallback) noexcept override {
    DCHECK(serverCallback_);
    DCHECK(&serverCallback);
    serverCallback_ = &serverCallback;
  }

  // StreamServerCallback
  // Terminating callback. Clean up the client and server callbacks stored.
  void onStreamCancel() noexcept override {
    eventBaseRunHelper(serverEvb_, [this]() mutable {
      if (serverCallback_) {
        serverCallback_->onStreamCancel();
        setServerCallback(nullptr);
      }
    });
    setClientCallback(nullptr);
  }

  bool onStreamRequestN(uint64_t num) noexcept override {
    eventBaseRunHelper(serverEvb_, [this, num]() mutable {
      if (serverCallback_) {
        std::ignore = serverCallback_->onStreamRequestN(num);
      }
    });
    return true;
  }

  void resetClientCallback(
      StreamClientCallback& clientCallback) noexcept override {
    DCHECK(clientCallback_);
    DCHECK(&clientCallback);
    clientCallback_ = &clientCallback;
  }

 private:
  // Increment the refCount before scheduling on the eventbase and decrement it
  // as soon as the eventbase scope is done.
  template <typename F>
  void eventBaseRunHelper(
      folly::Executor::KeepAlive<folly::EventBase> runEvb, F&& fn) {
    incRef();
    // cannot call folly::makeGuard inside the lambda captures below, because it
    // triggers a GCC 8 bug (https://fburl.com/e0kv48hu)
    auto guard = folly::makeGuard([this] { decRef(); });
    runEvb->runInEventBaseThread(
        [f = std::forward<F>(fn), g = std::move(guard)]() mutable { f(); });
  }

  void setServerCallback(StreamServerCallback* serverCallback) {
    DCHECK(!serverCallback_ != !serverCallback);
    serverCallback_ = serverCallback;
    if (serverCallback_) {
      incRef();
    } else {
      decRef();
    }
  }

  void setClientCallback(StreamClientCallback* clientCallback) {
    DCHECK(clientCallback_);
    clientCallback_ = clientCallback;
    if (!clientCallback_) {
      decRef();
    }
  }

  void incRef() { refCount_.fetch_add(1, std::memory_order_relaxed); }

  void decRef() {
    if (refCount_.fetch_sub(1, std::memory_order_acq_rel) == 1) {
      delete this;
    }
  }

 private:
  StreamClientCallback* clientCallback_{nullptr};
  StreamServerCallback* serverCallback_{nullptr};
  folly::Executor::KeepAlive<folly::EventBase> evb_;
  folly::Executor::KeepAlive<folly::EventBase> serverEvb_;
  // Starting at 1 since the server already has a reference to this when we
  // called sendRequestStream().
  std::atomic<int64_t> refCount_ = 1;
};

} // namespace

ThreadBoundAdaptorChannel::ThreadBoundAdaptorChannel(
    folly::EventBase* evb, std::shared_ptr<RequestChannel> threadSafeChannel)
    : threadSafeChannel_(std::move(threadSafeChannel)), evb_(evb) {
  DCHECK(threadSafeChannel_->getEventBase() == nullptr);
}

void ThreadBoundAdaptorChannel::sendRequestResponse(
    const RpcOptions& options,
    MethodMetadata&& methodMetadata,
    SerializedRequest&& request,
    std::shared_ptr<transport::THeader> header,
    RequestClientCallback::Ptr cob,
    std::unique_ptr<folly::IOBuf> frameworkMetadata) {
  cob = RequestClientCallback::Ptr(new EvbRequestCallback(
      std::move(cob), folly::Executor::getKeepAliveToken(evb_)));

  threadSafeChannel_->sendRequestResponse(
      options,
      std::move(methodMetadata),
      std::move(request),
      std::move(header),
      std::move(cob),
      std::move(frameworkMetadata));
}

void ThreadBoundAdaptorChannel::sendRequestNoResponse(
    const RpcOptions& options,
    MethodMetadata&& methodMetadata,
    SerializedRequest&& request,
    std::shared_ptr<transport::THeader> header,
    RequestClientCallback::Ptr cob,
    std::unique_ptr<folly::IOBuf> frameworkMetadata) {
  cob = RequestClientCallback::Ptr(new EvbRequestCallback(
      std::move(cob), folly::Executor::getKeepAliveToken(evb_)));

  threadSafeChannel_->sendRequestNoResponse(
      options,
      std::move(methodMetadata),
      std::move(request),
      std::move(header),
      std::move(cob),
      std::move(frameworkMetadata));
}

void ThreadBoundAdaptorChannel::sendRequestStream(
    const RpcOptions& options,
    MethodMetadata&& methodMetadata,
    SerializedRequest&& request,
    std::shared_ptr<transport::THeader> header,
    StreamClientCallback* cob,
    std::unique_ptr<folly::IOBuf> frameworkMetadata) {
  cob = new EvbStreamCallback(std::move(cob), evb_);

  threadSafeChannel_->sendRequestStream(
      options,
      std::move(methodMetadata),
      std::move(request),
      std::move(header),
      std::move(cob),
      std::move(frameworkMetadata));
}

void ThreadBoundAdaptorChannel::sendRequestSink(
    const RpcOptions& /* options */,
    MethodMetadata&& /* methodName */,
    SerializedRequest&& /* request */,
    std::shared_ptr<transport::THeader> /* header */,
    SinkClientCallback* /* cob */,
    std::unique_ptr<folly::IOBuf> /* frameworkMetadata */) {
  // Currently not implemented.
  LOG(FATAL) << "Not implemented";
}

void ThreadBoundAdaptorChannel::setCloseCallback(CloseCallback* cb) {
  threadSafeChannel_->setCloseCallback(cb);
}

folly::EventBase* ThreadBoundAdaptorChannel::getEventBase() const {
  return evb_;
}

uint16_t ThreadBoundAdaptorChannel::getProtocolId() {
  return threadSafeChannel_->getProtocolId();
}

InteractionId ThreadBoundAdaptorChannel::createInteraction(
    ManagedStringView&& name) {
  return threadSafeChannel_->createInteraction(std::move(name));
}

void ThreadBoundAdaptorChannel::terminateInteraction(InteractionId idWrapper) {
  threadSafeChannel_->terminateInteraction(std::move(idWrapper));
}

} // namespace apache::thrift
