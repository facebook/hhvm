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

#include <variant>

#include <folly/io/async/EventBaseAtomicNotificationQueue.h>

#include <thrift/lib/cpp2/async/ReplyInfo.h>
#include <thrift/lib/cpp2/server/ServerConfigs.h>

namespace apache::thrift {

/**
 * IOWorkerContext provides access to worker functionality that should be
 * usable from generated/handler code.
 */
class IOWorkerContext {
 public:
  using ReplyQueue =
      folly::EventBaseAtomicNotificationQueue<ReplyInfo, ReplyInfoConsumer>;

  /**
   * Get the reply queue.
   *
   * @returns reference to the queue.
   */
  ReplyQueue& getReplyQueue() const {
    DCHECK(replyQueue_ != nullptr);
    return *replyQueue_.get();
  }

  folly::EventBase* getWorkerEventBase() const { return eventBase_; }

  virtual const server::ServerConfigs* getServerContext() const = 0;

 protected:
  /**
   * Initializes the queue and registers it to the EventBase.
   *
   * @param eventBase EventBase to attach the queue.
   */
  void init(folly::EventBase& eventBase) {
    eventBase_ = &eventBase;
    replyQueue_ = std::make_unique<ReplyQueue>(ReplyInfoConsumer());
    replyQueue_->setMaxReadAtOnce(0);
    eventBase.runInEventBaseThread(
        [queue = replyQueue_.get(), &evb = eventBase, alive = alive_] {
          auto aliveLocked = alive->rlock();
          if (*aliveLocked) {
            // IOWorkerContext is still alive and so is replyQueue_
            queue->startConsumingInternal(&evb);
          }
        });
  }

  virtual ~IOWorkerContext() {
    *(alive_->wlock()) = false;

    // Workaround destruction order fiasco for DuplexChannel where Cpp2Worker
    // can be destroyed inline with the request, thus triggering queue's
    // destruction while processing items from the same queue. Once
    // DuplexChannel is deprecated, we should make being destructed inline.
    if (eventBase_) {
      eventBase_->runInEventBaseThread([queue = std::move(replyQueue_)] {});
    }
  }

 private:
  folly::EventBase* eventBase_{nullptr};
  // A dedicated queue for server responses
  std::unique_ptr<ReplyQueue> replyQueue_;
  // Needed to synchronize deallocating replyQueue_ and
  // calling startConsumingInternal() on eventbase loop.
  std::shared_ptr<folly::Synchronized<bool>> alive_{
      std::make_shared<folly::Synchronized<bool>>(true)};
};

} // namespace apache::thrift
