/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <folly/io/async/EventBase.h>
#include <list>
#include <memory>

namespace proxygen {

class ServiceWorker;
class RequestWorkerThread;
class RequestWorkerThreadNoExecutor;

/*
 * A Service object represents a network service running in proxygen.
 *
 * The Service object is primarily a construct used for managing startup and
 * shutdown.  The RunProxygen() call will accept a list of Service objects, and
 * will invoke start() on each service to start them.  When shutdown has been
 * requested, it will invoke failHealthChecks() on each service followed (after
 * some period of time) by stopAccepting().
 *
 * All of these methods are invoked from the main thread.
 */
class Service {
 public:
  Service();
  virtual ~Service();

  /**
   * Initialize the service.
   *
   * init() will be invoked from proxygen's main thread, before the worker
   * threads have started processing their event loops.
   *
   * The return value indicates if the service is enabled or not.  Return true
   * if the service is enabled and was initialized successfully, and false if
   * the service is disabled or is intialized successfully.  Throw an exception
   * if an error occurred initializing it.
   */
  virtual void init(folly::EventBase* mainEventBase,
                    const std::list<RequestWorkerThread*>& workers) = 0;

  /**
   * Finish any service initialization that requires the use of the worker
   * threads.
   */
  virtual void finishInit() {
  }

  /**
   * Start to accept connection on the listening sockect(s)
   *
   * All the expansive preparation work should be done befofe startAccepting(),
   * i.g., in constructor or init().  startAccepting() should be lightweight,
   * ideally just the call of accept() on all the listening sockects.
   * Otherwise, the first accepted connection may experience high latency.
   */
  virtual void startAccepting() {
  }

  /**
   * Mark the service as about to stop; invoked from main thread.
   *
   * This indicates that the service will be told to stop at some later time
   * and should continue to service requests but tell the healthchecker that it
   * is dying.
   */
  virtual void failHealthChecks() {
  }

  /**
   * Stop accepting all new work; invoked from proxygen's main thread.
   *
   * This should cause the service to stop accepting new work, and begin to
   * fully shut down. stop() may return before all work has completed, but it
   * should eventually cause all events for this service to be removed from the
   * main EventBase and from the worker threads.
   */
  virtual void stopAccepting() = 0;

  /**
   * Pause listening for new connections; invoked from proxygen's main thread.
   *
   * This should cause the service to pause listening for new connections.
   * The already accepted connections must not be affected.
   * It may or may not be followed by stopAccepting or resume listening.
   */
  virtual void pauseListening() {
  }

  /**
   * Drain remaining client connections; invoked from proxygen's main thread.
   */
  virtual void drainConnections() {
  }

  /**
   * Forcibly stop "pct" (0.0 to 1.0) of the remaining client connections.
   *
   * If the service does not stop on its own after stopAccepting() is called,
   * then proxygen might call dropConnections() several times to gradually
   * stop all processing before finally calling forceStop().
   */
  virtual void dropConnections(double /*pct*/) {
  }

  /**
   * Forcibly stop the service.
   *
   * If the service does not stop on its own after stopAccepting() is called,
   * forceStop() will eventually be called to forcibly stop all processing.
   *
   * (At the moment this isn't pure virtual simply because I haven't had the
   * time to update all existing services to implement forceStop().  Proxygen
   * will forcibly terminate the event loop even if a service does not stop
   * processing when forceStop() is called, so properly implementing
   * forceStop() isn't strictly required.)
   */
  virtual void forceStop() {
  }

  /**
   * Perform per-thread init.
   *
   * This method will be called once for each RequestWorkerThread thread, just
   * after the worker thread started.
   */
  virtual void initWorkerState(RequestWorkerThread*) {
  }

  /**
   * Perform per-thread cleanup.
   *
   * This method will be called once for each RequestWorkerThread thread, just
   * before that thread is about to exit.  Note that this method is called from
   * the worker thread itself, not from the main thread.
   *
   * failHealthChecks() and stopAccepting() will always be called in the main
   * thread before cleanupWorkerState() is called in any of the worker threads.
   *
   * forceStop() may be called in the main thread at any point during shutdown.
   * (i.e., Some worker threads may already have finished and called
   * cleanupWorkerState().  Once forceStop() is invoked, the remaining threads
   * will forcibly exit and then call cleanupWorkerState().)
   */
  virtual void cleanupWorkerState(RequestWorkerThread* /*worker*/) {
  }

  /**
   * Add a new ServiceWorker (subclasses should create one ServiceWorker
   * per worker thread)
   */
  void addServiceWorker(std::unique_ptr<ServiceWorker> worker,
                        RequestWorkerThread* reqWorker);
  void addServiceWorker(std::unique_ptr<ServiceWorker> worker,
                        RequestWorkerThreadNoExecutor* reqWorker);

  /**
   * List of workers
   */
  const std::list<std::unique_ptr<ServiceWorker>>& getServiceWorkers() const {
    return workers_;
  }

  /**
   * Delete all the workers
   */
  void clearServiceWorkers();

  void addWorkerEventBase(folly::EventBase* evb) {
    workerEvbs_.push_back(evb);
  }

  const std::vector<folly::EventBase*>& getWorkerEventBases() {
    return workerEvbs_;
  }

 private:
  // Forbidden copy constructor and assignment opererator
  Service(Service const&) = delete;
  Service& operator=(Service const&) = delete;

  // Workers
  std::list<std::unique_ptr<ServiceWorker>> workers_;
  std::vector<folly::EventBase*> workerEvbs_;
};

} // namespace proxygen
