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

#include <wangle/bootstrap/AcceptRoutingHandler.h>
#include <wangle/bootstrap/ServerBootstrap.h>
#include <wangle/channel/broadcast/BroadcastPool.h>
#include <wangle/channel/broadcast/Subscriber.h>

namespace wangle {

/**
 * A Handler-Observer adaptor that can be used for subscribing to broadcasts.
 * Maintains a thread-local BroadcastPool from which a BroadcastHandler is
 * obtained and subscribed to based on the given routing data.
 */
template <typename T, typename R, typename P = DefaultPipeline>
class ObservingHandler : public HandlerAdapter<folly::IOBufQueue&, T>,
                         public Subscriber<T, R> {
 public:
  using Context = typename HandlerAdapter<folly::IOBufQueue&, T>::Context;

  ObservingHandler(const R& routingData, BroadcastPool<T, R, P>* broadcastPool);
  ~ObservingHandler() override;

  // Non-copyable
  ObservingHandler(const ObservingHandler&) = delete;
  ObservingHandler& operator=(const ObservingHandler&) = delete;

  // Movable
  ObservingHandler(ObservingHandler&&) = default;
  ObservingHandler& operator=(ObservingHandler&&) = default;

  // HandlerAdapter implementation
  void transportActive(Context* ctx) override;
  void readEOF(Context* ctx) override;
  void readException(Context* ctx, folly::exception_wrapper ex) override;

  // Subscriber implementation
  void onNext(const T& buf) override;
  void onError(folly::exception_wrapper ex) override;
  void onCompleted() override;
  R& routingData() override;

 private:
  R routingData_;
  BroadcastPool<T, R, P>* broadcastPool_{nullptr};

  BroadcastHandler<T, R>* broadcastHandler_{nullptr};
  uint64_t subscriptionId_{0};
  bool paused_{false};

  // True iff the handler has been deleted
  std::shared_ptr<bool> deleted_{new bool(false)};
};

template <typename T>
using ObservingPipeline = Pipeline<folly::IOBufQueue&, T>;

template <typename T, typename R, typename P = DefaultPipeline>
class ObservingPipelineFactory
    : public RoutingDataPipelineFactory<ObservingPipeline<T>, R> {
 public:
  ObservingPipelineFactory(
      std::shared_ptr<ServerPool<R, P>> serverPool,
      std::shared_ptr<BroadcastPipelineFactory<T, R>> broadcastPipelineFactory)
      : serverPool_(serverPool),
        broadcastPipelineFactory_(broadcastPipelineFactory) {}

  typename ObservingPipeline<T>::Ptr newPipeline(
      std::shared_ptr<folly::AsyncTransport> socket,
      const R& routingData,
      RoutingDataHandler<R>*,
      std::shared_ptr<TransportInfo> transportInfo) override {
    auto pipeline = ObservingPipeline<T>::create();
    pipeline->addBack(AsyncSocketHandler(socket));
    auto handler = std::make_shared<ObservingHandler<T, R, P>>(
        routingData, broadcastPool());
    pipeline->addBack(handler);
    pipeline->finalize();

    pipeline->setTransportInfo(transportInfo);

    return pipeline;
  }

  virtual BroadcastPool<T, R, P>* broadcastPool(
      std::shared_ptr<BaseClientBootstrapFactory<>> clientFactory = nullptr) {
    if (!broadcastPool_) {
      if (clientFactory) {
        broadcastPool_.reset(new BroadcastPool<T, R, P>(
            serverPool_, broadcastPipelineFactory_, clientFactory));
      } else {
        broadcastPool_.reset(
            new BroadcastPool<T, R, P>(serverPool_, broadcastPipelineFactory_));
      }
    }
    return broadcastPool_.get();
  }

 protected:
  std::shared_ptr<ServerPool<R, P>> serverPool_;
  std::shared_ptr<BroadcastPipelineFactory<T, R>> broadcastPipelineFactory_;
  folly::ThreadLocalPtr<BroadcastPool<T, R, P>> broadcastPool_;
};

} // namespace wangle

#include <wangle/channel/broadcast/ObservingHandler-inl.h>
