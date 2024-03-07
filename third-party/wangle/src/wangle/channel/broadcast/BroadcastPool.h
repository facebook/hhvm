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

#include <folly/ThreadLocal.h>
#include <folly/futures/SharedPromise.h>
#include <folly/io/async/DelayedDestruction.h>
#include <wangle/bootstrap/BaseClientBootstrap.h>
#include <wangle/bootstrap/ClientBootstrap.h>
#include <wangle/channel/Pipeline.h>
#include <wangle/channel/broadcast/BroadcastHandler.h>

namespace wangle {

template <typename R, typename P = DefaultPipeline>
class ServerPool {
 public:
  virtual ~ServerPool() = default;

  /**
   * Kick off an upstream connect request given the BaseClientBootstrap
   * when a broadcast is not available locally.
   */
  virtual folly::Future<P*> connect(
      BaseClientBootstrap<P>* client,
      const R& routingData) noexcept = 0;
};

/**
 * A pool of upstream broadcast pipelines. There is atmost one broadcast
 * for any unique routing data. Creates and maintains upstream connections
 * and broadcast pipeliens as necessary.
 *
 * Meant to be used as a thread-local instance.
 */
template <typename T, typename R, typename P = DefaultPipeline>
class BroadcastPool {
 public:
  class BroadcastManager : public PipelineManager,
                           public folly::DelayedDestruction {
   public:
    using UniquePtr = folly::DelayedDestructionUniquePtr<BroadcastManager>;

    BroadcastManager(
        BroadcastPool<T, R, P>* broadcastPool,
        const R& routingData)
        : broadcastPool_(broadcastPool),
          routingData_(routingData),
          client_(broadcastPool_->clientBootstrapFactory_->newClient()) {
      client_->pipelineFactory(broadcastPool_->broadcastPipelineFactory_);
    }

    ~BroadcastManager() override {
      if (client_->getPipeline()) {
        client_->getPipeline()->setPipelineManager(nullptr);
      }
    }

    folly::Future<BroadcastHandler<T, R>*> getHandler();

    // PipelineManager implementation
    void deletePipeline(PipelineBase* pipeline) override;

   private:
    void handleConnectError(const std::exception& ex) noexcept;

    BroadcastPool<T, R, P>* broadcastPool_{nullptr};
    R routingData_;

    std::unique_ptr<BaseClientBootstrap<P>> client_;

    bool connectStarted_{false};
    bool deletingBroadcast_{false};
    folly::SharedPromise<BroadcastHandler<T, R>*> sharedPromise_;
  };

  BroadcastPool(
      std::shared_ptr<ServerPool<R, P>> serverPool,
      std::shared_ptr<BroadcastPipelineFactory<T, R>> pipelineFactory,
      std::shared_ptr<BaseClientBootstrapFactory<>> clientFactory =
          std::make_shared<ClientBootstrapFactory>())
      : serverPool_(serverPool),
        broadcastPipelineFactory_(pipelineFactory),
        clientBootstrapFactory_(clientFactory) {}

  virtual ~BroadcastPool() = default;

  // Non-copyable
  BroadcastPool(const BroadcastPool&) = delete;
  BroadcastPool& operator=(const BroadcastPool&) = delete;

  // Movable
  BroadcastPool(BroadcastPool&&) = default;
  BroadcastPool& operator=(BroadcastPool&&) = default;

  /**
   * Gets the BroadcastHandler, or creates one if it doesn't exist already,
   * for the given routingData.
   *
   * If a broadcast is already available for the given routingData,
   * returns the BroadcastHandler from the pipeline. If not, an upstream
   * connection is created and stored along with a new broadcast pipeline
   * for this routingData, and its BroadcastHandler is returned.
   *
   * Caller should immediately subscribe to the returned BroadcastHandler
   * to prevent it from being garbage collected.
   * Note that to ensure that this works correctly, the returned future
   * completes on an InlineExecutor such that .then will be called inline with
   * satisfaction of the underlying promise.
   */
  virtual folly::Future<BroadcastHandler<T, R>*> getHandler(
      const R& routingData);

  /**
   * Checks if a broadcast is available locally for the given routingData.
   */
  bool isBroadcasting(const R& routingData) {
    return (broadcasts_.find(routingData) != broadcasts_.end());
  }

  virtual void deleteBroadcast(const R& routingData) {
    broadcasts_.erase(routingData);
  }

 private:
  std::shared_ptr<ServerPool<R, P>> serverPool_;
  std::shared_ptr<BroadcastPipelineFactory<T, R>> broadcastPipelineFactory_;
  std::shared_ptr<BaseClientBootstrapFactory<>> clientBootstrapFactory_;
  std::map<R, typename BroadcastManager::UniquePtr> broadcasts_;
};

} // namespace wangle

#include <wangle/channel/broadcast/BroadcastPool-inl.h>
