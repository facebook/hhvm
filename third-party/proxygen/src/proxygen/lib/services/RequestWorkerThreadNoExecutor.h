/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <cstdint>
#include <map>
#include <proxygen/lib/services/WorkerThread.h>
#include <wangle/acceptor/LoadShedConfiguration.h>

namespace proxygen {

class Service;
class ServiceWorker;

/**
 * RequestWorkerThreadNoExecutor extends WorkerThread, and also contains a list
 * of ServiceWorkers running in this thread.
 */
class RequestWorkerThreadNoExecutor : public WorkerThread {
 public:
  class FinishCallback {
   public:
    virtual ~FinishCallback() noexcept = default;
    virtual void workerStarted(RequestWorkerThreadNoExecutor*) = 0;
    virtual void workerFinished(RequestWorkerThreadNoExecutor*) = 0;
  };

  /**
   * Create a new RequestWorkerThreadNoExecutor.
   *
   * @param proxygen  The object to notify when this worker finishes.
   * @param threadId  A unique ID for this worker.
   * @param evbName   The event base will ne named to this name (thread name)
   */
  RequestWorkerThreadNoExecutor(FinishCallback& callback,
                                uint8_t threadId,
                                const std::string& evbName = std::string());

  /**
   * Reset the underlying event base prior to WorkerThread destruction.
   */
  ~RequestWorkerThreadNoExecutor() override;

  /**
   * Return a unique 64bit identifier.
   */
  static uint64_t nextRequestId();

  /**
   * Return unique 8bit worker ID.
   */
  [[nodiscard]] uint8_t getWorkerId() const;

  static RequestWorkerThreadNoExecutor* getRequestWorkerThreadNoExecutor() {
    auto* self = dynamic_cast<RequestWorkerThreadNoExecutor*>(
        WorkerThread::getCurrentWorkerThread());
    CHECK_NOTNULL(self);
    return self;
  }

  /**
   * Track the ServiceWorker objects in-use by this worker.
   */
  void addServiceWorker(Service* service, ServiceWorker* sw) {
    CHECK(serviceWorkers_.find(service) == serviceWorkers_.end());
    serviceWorkers_[service] = sw;
  }

  /**
   * For a given service, returns the ServiceWorker associated with this
   * RequestWorkerThreadNoExecutor
   */
  ServiceWorker* getServiceWorker(Service* service) const {
    auto it = serviceWorkers_.find(service);
    CHECK(it != serviceWorkers_.end());
    return it->second;
  }

  /**
   * Get/set the worker thread's bound load shed configuration instance.
   * Used by derivative classes.  Updates are propagated seamlessly via
   * the use of swapping such that threads will automatically see updated
   * fields on update.
   */
  [[nodiscard]] std::shared_ptr<const wangle::LoadShedConfiguration>
  getLoadShedConfig() const {
    return loadShedConfig_;
  }
  void setLoadShedConfig(
      std::shared_ptr<const wangle::LoadShedConfiguration> loadShedConfig) {
    loadShedConfig_.swap(loadShedConfig);
  }

  /**
   * Flush any thread-local stats being tracked by our ServiceWorkers.
   *
   * This must be invoked from within worker's thread.
   */
  void flushStats();

 private:
  void setup() override;
  void cleanup() override;

  // The next request id within this thread. The id has its highest byte set to
  // the thread id, so is unique across the process.
  uint64_t nextRequestId_;

  // The ServiceWorkers executing in this worker
  folly::F14ValueMap<Service*, ServiceWorker*> serviceWorkers_;

  // Every worker instance has their own version of load shed config.
  // This enables every request worker thread, and derivative there of,
  // to both access and update this field in a thread-safe way.
  std::shared_ptr<const wangle::LoadShedConfiguration> loadShedConfig_{nullptr};

  FinishCallback& callback_;
};

} // namespace proxygen
