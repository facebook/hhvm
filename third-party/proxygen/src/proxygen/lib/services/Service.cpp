/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <proxygen/lib/services/Service.h>

#include <proxygen/lib/services/RequestWorkerThread.h>
#include <proxygen/lib/services/RequestWorkerThreadNoExecutor.h>
#include <proxygen/lib/services/ServiceWorker.h>

namespace proxygen {

Service::Service() {
}

Service::~Service() {
}

void Service::addServiceWorker(std::unique_ptr<ServiceWorker> worker,
                               RequestWorkerThread* reqWorker) {
  reqWorker->addServiceWorker(this, worker.get());
  workers_.emplace_back(std::move(worker));
}

void Service::addServiceWorker(std::unique_ptr<ServiceWorker> worker,
                               RequestWorkerThreadNoExecutor* reqWorker) {
  reqWorker->addServiceWorker(this, worker.get());
  workers_.emplace_back(std::move(worker));
}

void Service::clearServiceWorkers() {
  workers_.clear();
}

} // namespace proxygen
