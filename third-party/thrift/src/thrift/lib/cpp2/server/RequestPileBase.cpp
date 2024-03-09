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

#include <thrift/lib/cpp2/server/RequestPileBase.h>

namespace apache::thrift {

RequestPileBase::RequestPileBase(std::string name)
    : resourcePoolName_(std::move(name)) {}

void RequestPileBase::setUpQueueObservers() {
  auto factory = folly::QueueObserverFactory::make(
      "resource_pool." + resourcePoolName_,
      defaultQueueObserverNumPrio,
      threadIdCollector_.get());
  if (!factory) {
    return;
  }

  queueObserver_ = factory->create(defaultQueueObserverNumPrio);
}

void RequestPileBase::onEnqueued(ServerRequest& req) {
  if (queueObserver_) {
    detail::ServerRequestHelper::queueObserverPayload(req) =
        queueObserver_->onEnqueued(req.follyRequestContext().get());
  }
}

void RequestPileBase::onDequeued(ServerRequest& req) {
  if (threadIdCollector_) {
    registerThreadId();
  }

  if (queueObserver_) {
    intptr_t payload = detail::ServerRequestHelper::queueObserverPayload(req);
    queueObserver_->onDequeued(payload);
  }
}

void RequestPileBase::registerThreadId() {
  // Setting up the id capturing for queue lag observer
  // This process should only be done once per thread
  // This initializer, when declared static, will
  // execute its constructor only once

  // We are capturing the collector as shared_ptr to make sure
  // the collector can outlive the running threads
  struct Init {
    explicit Init(std::shared_ptr<folly::ThreadIdWorkerProvider> tidProvider)
        : threadIdCollector(tidProvider) {
      // Capture the current threads ID in the ThreadManager's tracking list.
      threadIdCollector->addTid(folly::getOSThreadID());
    }

    ~Init() {
      // On thread exit, we should remove the
      // thread ID from the collector's tracking
      // list.e
      threadIdCollector->removeTid(folly::getOSThreadID());
    }

    std::shared_ptr<folly::ThreadIdWorkerProvider> threadIdCollector;
  };
  thread_local Init init(threadIdCollector_);
}

serverdbginfo::RequestPileDbgInfo RequestPileBase::getDbgInfo() const {
  serverdbginfo::RequestPileDbgInfo info;
  info.name() = resourcePoolName_;
  return info;
}

} // namespace apache::thrift
