// Copyright 2004-present Facebook.  All rights reserved.
#pragma once

#include <list>
#include <memory>

namespace apache { namespace thrift { namespace async {
class TEventBase;
}}}

namespace facebook { namespace proxygen {

class ServiceWorker;
class RequestWorker;

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
   * Start the service.
   *
   * start() will be invoked from proxygen's main thread, before the worker
   * threads have started processing their event loops.
   *
   * The return value indicates if the service is enabled or not.  Return true
   * if the service is enabled and was started successfully, and false if the
   * service is disabled and is intentionally not started.  Throw an exception
   * if the service is enabled and is supposed to be running, but an error
   * occurred starting it.
   */
  virtual void start(apache::thrift::async::TEventBase* mainEventBase,
                     const std::list<RequestWorker*>& workers) = 0;

  /**
   * Mark the service as about to stop; invoked from main thread.
   *
   * This indicates that the service will be told to stop at some later time
   * and should continue to service requests but tell the healthchecker that it
   * is dying.
   */
  virtual void failHealthChecks() {}

  /**
   * Stop accepting all new work; invoked from proxygen's main thread.
   *
   * This should cause the service to stop accepting new work, and begin to
   * fully shut down. stop() may return before all work has completed, but it
   * should eventually cause all events for this service to be removed from the
   * main TEventBase and from the worker threads.
   */
  virtual void stopAccepting() = 0;

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
  virtual void forceStop() {}

  /**
   * Perform per-thread cleanup.
   *
   * This method will be called once for each RequestWorker thread, just before
   * that thread is about to exit.  Note that this method is called from the
   * worker thread itself, not from the main thread.
   *
   * failHealthChecks() and stopAccepting() will always be called in the main
   * thread before cleanupWorkerState() is called in any of the worker threads.
   *
   * forceStop() may be called in the main thread at any point during shutdown.
   * (i.e., Some worker threads may already have finished and called
   * cleanupWorkerState().  Once forceStop() is invoked, the remaining threads
   * will forcibly exit and then call cleanupWorkerState().)
   */
  virtual void cleanupWorkerState(RequestWorker* worker) {}

  /**
   * Add a new ServiceWorker (subclasses should create one ServiceWorker
   * per worker thread)
   */
  void addServiceWorker(std::unique_ptr<ServiceWorker> worker,
                        RequestWorker *reqWorker);

  /**
   * List of workers
   */
  /* final */ const std::list<std::unique_ptr<ServiceWorker>>&
      getServiceWorkers() const {
    return workers_;
  }

  /**
   * Delete all the workers
   */
  /* final */ void clearServiceWorkers();

  /**
   * Start even when config_test_only is set - default to false
   */
  virtual bool startWithConfigTest(bool configTestOnly) {
    return !configTestOnly;
  }

 private:
  // Forbidden copy constructor and assignment opererator
  Service(Service const &) = delete;
  Service& operator=(Service const &) = delete;

  // Workers
  std::list<std::unique_ptr<ServiceWorker>> workers_;
};

}} // facebook::proxygen
