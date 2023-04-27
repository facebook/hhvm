/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <folly/io/async/AsyncServerSocket.h>
#include <list>
#include <memory>
#include <proxygen/lib/utils/AcceptorAddress.h>
#include <wangle/acceptor/Acceptor.h>
#include <wangle/acceptor/ConnectionCounter.h>

namespace proxygen {

class Service;
class RequestWorkerThread;

/**
 * ServiceWorker contains all of the per-thread information for a Service.
 *
 * ServiceWorker contains a pointer back to the single global Service.
 * It contains a list of ProxyAcceptor objects for this worker thread,
 * one per VIP.
 *
 * ServiceWorker fits into the Proxygen object hierarchy as follows:
 *
 * - Service: one instance (globally across the entire program)
 * - ServiceWorker: one instance per thread
 * - ServiceAcceptor: one instance per VIP per thread
 */
class ServiceWorker {
 public:
  using AcceptorMap =
      std::map<AcceptorAddress, std::unique_ptr<wangle::Acceptor>>;

  using NamedAddressMap = std::map<std::string, AcceptorAddress>;

  ServiceWorker(Service* service, RequestWorkerThread* worker)
      : service_(service), worker_(worker) {
  }

  virtual ~ServiceWorker() {
  }

  Service* getService() const {
    return service_;
  }

  void addServiceAcceptor(const folly::SocketAddress& address,
                          std::unique_ptr<wangle::Acceptor> acceptor) {
    addServiceAcceptor(
        AcceptorAddress(address, AcceptorAddress::AcceptorType::TCP),
        std::move(acceptor));
  }

  void addServiceAcceptor(const AcceptorAddress& accAddress,
                          std::unique_ptr<wangle::Acceptor> acceptor) {
    namedAddress_.emplace(acceptor->getName(), accAddress);
    addAcceptor(accAddress, std::move(acceptor), acceptors_);
  }

  void drainServiceAcceptor(const folly::SocketAddress& address) {
    drainServiceAcceptor(
        AcceptorAddress(address, AcceptorAddress::AcceptorType::TCP));
  }

  void drainServiceAcceptor(const AcceptorAddress& accAddress) {
    // Move the old acceptor to drainingAcceptors_ if present
    const auto& it = acceptors_.find(accAddress);
    if (it != acceptors_.end()) {
      auto name = it->second->getName();
      addAcceptor(accAddress, std::move(it->second), drainingAcceptors_);
      acceptors_.erase(it);
      namedAddress_.erase(name);
    }
  }

  RequestWorkerThread* getRequestWorkerThread() const {
    return worker_;
  }

  const AcceptorMap& getAcceptors() {
    return acceptors_;
  }

  /**
   * Find an acceptor by name running in the same request worker.
   */
  wangle::Acceptor* getAcceptorByName(std::string name) const {
    auto it = namedAddress_.find(name);
    if (it != namedAddress_.end()) {
      auto it2 = acceptors_.find(it->second);
      if (it2 != acceptors_.end()) {
        return it2->second.get();
      }
    }
    return nullptr;
  }

  const AcceptorMap& getDrainingAcceptors() {
    return drainingAcceptors_;
  }

  // Flush any thread-local stats that the service is tracking
  virtual void flushStats() {
  }

  // Destruct all the acceptors
  virtual void clearAcceptors() {
    acceptors_.clear();
    namedAddress_.clear();
    drainingAcceptors_.clear();
  }

  virtual void clearDrainingAcceptors() {
    drainingAcceptors_.clear();
  }

  virtual void forceStop() {
  }

 private:
  // Forbidden copy constructor and assignment operator
  ServiceWorker(ServiceWorker const&) = delete;
  ServiceWorker& operator=(ServiceWorker const&) = delete;

  void addAcceptor(const AcceptorAddress& accAddress,
                   std::unique_ptr<wangle::Acceptor> acceptor,
                   AcceptorMap& acceptors) {
    CHECK(acceptors.find(accAddress) == acceptors.end());
    acceptors.insert(std::make_pair(accAddress, std::move(acceptor)));
  }

  /**
   * The global Service object.
   */
  Service* service_;

  /**
   * The RequestWorkerThread that is actually responsible for running the
   * EventBase loop in this thread.
   */
  RequestWorkerThread* worker_;

  /**
   * A list of the Acceptor objects specific to this worker thread, one
   * Acceptor per VIP.
   */
  AcceptorMap acceptors_;

  /**
   * A list of the addresses indexed by name, used to look up an
   * acceptor by name
   */
  NamedAddressMap namedAddress_;

  /**
   * A list of Acceptors that are being drained and will be deleted soon.
   */
  AcceptorMap drainingAcceptors_;
};

} // namespace proxygen
