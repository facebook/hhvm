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

namespace wangle {

template <typename T, typename R, typename P>
folly::Future<BroadcastHandler<T, R>*>
BroadcastPool<T, R, P>::BroadcastManager::getHandler() {
  // getFuture() returns a completed future if we are already connected
  // Set the executor to the InlineExecutor because subsequent code depends
  // on the future callback being called inline to ensure that the handler
  // is not garbage collected before use.
  auto future =
      sharedPromise_.getFuture().via(&folly::InlineExecutor::instance());

  if (connectStarted_) {
    // Either already connected, in which case the future has the handler,
    // or there's an outstanding connect request and the promise will be
    // fulfilled when the connect request completes.
    return future;
  }

  // Kickoff connect request and fulfill all pending promises on completion
  connectStarted_ = true;

  broadcastPool_->serverPool_->connect(client_.get(), routingData_)
      .thenValue([this](DefaultPipeline* pipeline) {
        DestructorGuard dg(this);
        pipeline->setPipelineManager(this);

        auto pipelineFactory = broadcastPool_->broadcastPipelineFactory_;
        try {
          pipelineFactory->setRoutingData(pipeline, routingData_);
        } catch (const std::exception& ex) {
          handleConnectError(ex);
          return;
        }

        if (deletingBroadcast_) {
          // setRoutingData() could result in an error that would cause the
          // BroadcastPipeline to get deleted.
          handleConnectError(std::runtime_error(
              "Broadcast deleted due to upstream connection error"));
          return;
        }

        auto handler = pipelineFactory->getBroadcastHandler(pipeline);
        CHECK(handler);
        sharedPromise_.setValue(handler);

        // If all the observers go away before connect returns, then the
        // BroadcastHandler will be idle without any subscribers. Close
        // the pipeline and remove the broadcast from the pool so that
        // connections are not leaked.
        handler->closeIfIdle();
      })
      .thenError(
          folly::tag_t<std::exception>{},
          [this](const std::exception& ex) { handleConnectError(ex); });

  return future;
}

template <typename T, typename R, typename P>
void BroadcastPool<T, R, P>::BroadcastManager::deletePipeline(
    PipelineBase* pipeline) {
  CHECK(client_->getPipeline() == pipeline);
  deletingBroadcast_ = true;
  broadcastPool_->deleteBroadcast(routingData_);
}

template <typename T, typename R, typename P>
void BroadcastPool<T, R, P>::BroadcastManager::handleConnectError(
    const std::exception& ex) noexcept {
  LOG(ERROR) << "Error connecting to upstream: " << ex.what();

  auto sharedPromise = std::move(sharedPromise_);
  broadcastPool_->deleteBroadcast(routingData_);
  sharedPromise.setException(folly::make_exception_wrapper<std::exception>(ex));
}

template <typename T, typename R, typename P>
folly::Future<BroadcastHandler<T, R>*> BroadcastPool<T, R, P>::getHandler(
    const R& routingData) {
  const auto& iter = broadcasts_.find(routingData);
  if (iter != broadcasts_.end()) {
    return iter->second->getHandler();
  }

  typename BroadcastManager::UniquePtr broadcast(
      new BroadcastManager(this, routingData));

  auto broadcastPtr = broadcast.get();
  broadcasts_.insert(std::make_pair(routingData, std::move(broadcast)));

  // The executor on this future is set to be an InlineExecutor to ensure that
  // the continuation can be run inline and satisfy the lifetime requirement
  // on the return value of this function.
  return broadcastPtr->getHandler();
}

} // namespace wangle
