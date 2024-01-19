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

#include <thrift/lib/cpp2/async/PooledRequestChannel.h>

#include <thrift/lib/cpp2/async/FutureRequest.h>

#include <folly/futures/Future.h>

namespace apache {
namespace thrift {
namespace {
struct InteractionState {
  folly::Executor::KeepAlive<folly::EventBase> keepAlive;
  apache::thrift::ManagedStringView name;
  InteractionId id;
  std::shared_ptr<folly::EventBaseLocal<PooledRequestChannel::ImplPtr>> impl;
  std::atomic<size_t> refcount{1};

  struct Deleter {
    void operator()(InteractionState* self) {
      if (self->refcount.fetch_sub(1, std::memory_order_acq_rel) == 1) {
        std::move(self->keepAlive)
            .add([id = std::move(self->id),
                  implPtr = std::move(self->impl)](auto&& keepAlive_2) mutable {
              auto* channel = implPtr->get(*keepAlive_2);
              if (channel) {
                (*channel)->terminateInteraction(std::move(id));
              } else {
                // channel is only null if nothing was ever sent on that evb,
                // in which case server doesn't know about this interaction
                DCHECK(!id);
              }
            });
        delete self;
      }
    }
  };
  using Ptr = std::unique_ptr<InteractionState, Deleter>;
  Ptr copy() {
    refcount.fetch_add(1, std::memory_order_relaxed);
    return Ptr(this);
  }
};

static void maybeCreateInteraction(
    const RpcOptions& options, PooledRequestChannel::Impl& channel) {
  if (auto id = options.getInteractionId()) {
    auto* state = reinterpret_cast<InteractionState*>(id);
    if (!state->id) {
      state->id = channel.registerInteraction(std::move(state->name), id);
    }
  }
}

InteractionState::Ptr getInteractionGuard(const RpcOptions& options) {
  if (options.getInteractionId()) {
    return reinterpret_cast<InteractionState*>(options.getInteractionId())
        ->copy();
  }
  return nullptr;
}
} // namespace

folly::Executor::KeepAlive<folly::EventBase> PooledRequestChannel::getEvb(
    const RpcOptions& options) {
  if (options.getInteractionId()) {
    return reinterpret_cast<InteractionState*>(options.getInteractionId())
        ->keepAlive;
  }

  auto evb = getEventBase_();
  if (!evb) {
    throw std::logic_error("IO executor already destroyed.");
  }
  return evb;
}

uint16_t PooledRequestChannel::getProtocolId() {
  return protocolId_;
}

template <typename SendFunc>
void PooledRequestChannel::sendRequestImpl(
    SendFunc&& sendFunc, folly::Executor::KeepAlive<folly::EventBase>&& evb) {
  std::move(evb).add([this, sendFunc = std::forward<SendFunc>(sendFunc)](
                         auto&& keepAlive) mutable {
    auto& implRef = impl(*keepAlive);
    DCHECK_EQ(getProtocolId(), implRef.getProtocolId());
    sendFunc(implRef);
  });
}

namespace {
class ExecutorRequestCallback final : public RequestClientCallback {
 public:
  ExecutorRequestCallback(
      RequestClientCallback::Ptr cb,
      folly::Executor::KeepAlive<> executorKeepAlive)
      : executorKeepAlive_(std::move(executorKeepAlive)), cb_(std::move(cb)) {
    CHECK(executorKeepAlive_);
  }

  void onResponse(ClientReceiveState&& rs) noexcept override {
    executorKeepAlive_.get()->add(
        [cb = std::move(cb_), rs = std::move(rs)]() mutable {
          cb.release()->onResponse(std::move(rs));
        });
    delete this;
  }
  void onResponseError(folly::exception_wrapper ex) noexcept override {
    executorKeepAlive_.get()->add(
        [cb = std::move(cb_), ex = std::move(ex)]() mutable {
          cb.release()->onResponseError(std::move(ex));
        });
    delete this;
  }

 private:
  folly::Executor::KeepAlive<> executorKeepAlive_;
  RequestClientCallback::Ptr cb_;
};
} // namespace

void PooledRequestChannel::sendRequestResponse(
    RpcOptions&& options,
    apache::thrift::MethodMetadata&& methodMetadata,
    SerializedRequest&& request,
    std::shared_ptr<transport::THeader> header,
    RequestClientCallback::Ptr cob) {
  if (!cob->isInlineSafe()) {
    cob = RequestClientCallback::Ptr(new ExecutorRequestCallback(
        std::move(cob), getKeepAliveToken(callbackExecutor_)));
  }
  auto evb = getEvb(options);
  auto guard = getInteractionGuard(options);
  sendRequestImpl(
      [options = std::move(options),
       methodMetadata = std::move(methodMetadata),
       request = std::move(request),
       header = std::move(header),
       guard = std::move(guard),
       cob = std::move(cob)](Impl& channel) mutable {
        maybeCreateInteraction(options, channel);
        channel.sendRequestResponse(
            std::move(options),
            std::move(methodMetadata),
            std::move(request),
            std::move(header),
            std::move(cob));
      },
      std::move(evb));
}

void PooledRequestChannel::sendRequestNoResponse(
    RpcOptions&& options,
    apache::thrift::MethodMetadata&& methodMetadata,
    SerializedRequest&& request,
    std::shared_ptr<transport::THeader> header,
    RequestClientCallback::Ptr cob) {
  if (!cob->isInlineSafe()) {
    cob = RequestClientCallback::Ptr(new ExecutorRequestCallback(
        std::move(cob), getKeepAliveToken(callbackExecutor_)));
  }
  auto evb = getEvb(options);
  auto guard = getInteractionGuard(options);
  sendRequestImpl(
      [options = std::move(options),
       methodMetadata = std::move(methodMetadata),
       request = std::move(request),
       header = std::move(header),
       guard = std::move(guard),
       cob = std::move(cob)](Impl& channel) mutable {
        maybeCreateInteraction(options, channel);
        channel.sendRequestNoResponse(
            std::move(options),
            std::move(methodMetadata),
            std::move(request),
            std::move(header),
            std::move(cob));
      },
      std::move(evb));
}

void PooledRequestChannel::sendRequestStream(
    RpcOptions&& options,
    apache::thrift::MethodMetadata&& methodMetadata,
    SerializedRequest&& request,
    std::shared_ptr<transport::THeader> header,
    StreamClientCallback* cob) {
  auto evb = getEvb(options);
  auto guard = getInteractionGuard(options);
  sendRequestImpl(
      [options = std::move(options),
       methodMetadata = std::move(methodMetadata),
       request = std::move(request),
       header = std::move(header),
       guard = std::move(guard),
       cob](Impl& channel) mutable {
        maybeCreateInteraction(options, channel);
        channel.sendRequestStream(
            std::move(options),
            std::move(methodMetadata),
            std::move(request),
            std::move(header),
            cob);
      },
      std::move(evb));
}

void PooledRequestChannel::sendRequestSink(
    RpcOptions&& options,
    apache::thrift::MethodMetadata&& methodMetadata,
    SerializedRequest&& request,
    std::shared_ptr<transport::THeader> header,
    SinkClientCallback* cob) {
  auto evb = getEvb(options);
  auto guard = getInteractionGuard(options);
  sendRequestImpl(
      [options = std::move(options),
       methodMetadata = std::move(methodMetadata),
       request = std::move(request),
       header = std::move(header),
       guard = std::move(guard),
       cob](Impl& channel) mutable {
        maybeCreateInteraction(options, channel);
        channel.sendRequestSink(
            std::move(options),
            std::move(methodMetadata),
            std::move(request),
            std::move(header),
            cob);
      },
      std::move(evb));
}

InteractionId PooledRequestChannel::createInteraction(
    ManagedStringView&& name) {
  CHECK(!name.view().empty());
  return createInteractionId(reinterpret_cast<int64_t>(
      new InteractionState{getEvb({}), std::move(name), {}, impl_}));
}

void PooledRequestChannel::terminateInteraction(InteractionId idWrapper) {
  int64_t id = idWrapper;
  releaseInteractionId(std::move(idWrapper));
  InteractionState::Ptr state(reinterpret_cast<InteractionState*>(id));
}

PooledRequestChannel::Impl& PooledRequestChannel::impl(folly::EventBase& evb) {
  DCHECK(evb.inRunningEventBaseThread());

  return *impl_->try_emplace_with(evb, [this, &evb] {
    auto ptr = implCreator_(evb);
    DCHECK(!!ptr);
    return ptr;
  });
}

PooledRequestChannel::EventBaseProvider PooledRequestChannel::wrapWeakPtr(
    std::weak_ptr<folly::IOExecutor> executor) {
  return [executor = std::move(
              executor)]() -> folly::Executor::KeepAlive<folly::EventBase> {
    if (auto ka = executor.lock()) {
      return {ka->getEventBase()};
    }
    return {};
  };
}

PooledRequestChannel::EventBaseProvider
PooledRequestChannel::globalExecutorProvider(size_t numThreads) {
  auto executor = folly::getGlobalIOExecutor();
  if (!executor) {
    throw std::logic_error("IO executor already destroyed.");
  }
  std::vector<folly::EventBase*> ebs;
  ebs.reserve(numThreads);
  for (size_t i = 0; i < numThreads; i++) {
    ebs.push_back(executor->getEventBase());
  }
  return
      [ebs = std::move(ebs),
       idx = std::make_unique<std::atomic<uint64_t>>(0),
       numThreads]() mutable -> folly::Executor::KeepAlive<folly::EventBase> {
        if (auto ka = folly::getGlobalIOExecutor()) {
          return {ebs.at(
              idx->fetch_add(1, std::memory_order_relaxed) % numThreads)};
        }
        return {};
      };
}
} // namespace thrift
} // namespace apache
