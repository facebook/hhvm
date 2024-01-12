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

#include <folly/executors/thread_factory/NamedThreadFactory.h>
#include <folly/io/async/EventBaseManager.h>
#include <wangle/bootstrap/ServerBootstrap.h>
#include <wangle/channel/Handler.h>

namespace wangle {

void ServerWorkerPool::registerEventBase(folly::EventBase& evb) {
  auto worker = acceptorFactory_->newAcceptor(&evb);
  {
    std::unique_lock holder(workersMutex_);
    workers_->push_back({&evb, worker});
  }

  for (auto socket : *sockets_) {
    socket->getEventBase()->runImmediatelyOrRunInEventBaseThreadAndWait(
        [this, worker, socket]() {
          socketFactory_->addAcceptCB(
              socket, worker.get(), worker->getEventBase());
        });
  }
}

void ServerWorkerPool::unregisterEventBase(folly::EventBase& evb) {
  auto worker = [&]() -> std::shared_ptr<Acceptor> {
    std::unique_lock holder(workersMutex_);
    for (auto it = workers_->begin(); it != workers_->end(); ++it) {
      if (it->first != &evb) {
        continue;
      }
      auto acceptor = std::move(it->second);
      workers_->erase(it);
      return acceptor;
    }
    // The thread handle may not be present in the map if newAcceptor() throws
    // an exception. For example, some acceptors require TLS keys / certs to
    // start and will throw exceptions if those files do not exist.
    return nullptr;
  }();
  if (!worker) {
    return;
  }

  for (auto socket : *sockets_) {
    socket->getEventBase()->runImmediatelyOrRunInEventBaseThreadAndWait([&]() {
      socketFactory_->removeAcceptCB(socket, worker.get(), nullptr);
    });
  }

  auto workerEvb = worker->getEventBase();
  workerEvb->runImmediatelyOrRunInEventBaseThreadAndWait(
      [w = std::move(worker)]() mutable {
        w->dropAllConnections();
        w.reset();
      });
}

} // namespace wangle
