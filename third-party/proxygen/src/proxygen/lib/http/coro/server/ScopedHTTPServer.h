/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "proxygen/lib/http/coro/server/HTTPServer.h"

namespace proxygen::coro {

class ScopedHTTPServer {
 public:
  static std::unique_ptr<ScopedHTTPServer> start(
      HTTPServer::Config config,
      std::shared_ptr<HTTPHandler> handler,
      HTTPServer::Observer* observer = nullptr) {
    std::unique_ptr<ScopedHTTPServer> server(
        new ScopedHTTPServer(std::move(config), std::move(handler)));
    if (observer) {
      server->getServer().addObserver(observer);
    }
    server->start();
    return server;
  }

  ~ScopedHTTPServer() {
    server_.drain();
    server_.forceStop();
    thread_.join();
  }

  std::optional<folly::SocketAddress> address() {
    return server_.address();
  }

  void start() {
    std::exception_ptr eptr;
    folly::Baton baton;

    thread_ = std::thread([&]() {
      server_.start([&]() { baton.post(); },
                    [&](std::exception_ptr ex) {
                      eptr = ex;
                      baton.post();
                    });
    });

    // Wait for server to start
    baton.wait();
    if (eptr) {
      thread_.join();

      std::rethrow_exception(eptr);
    }
  }

  HTTPServer& getServer() {
    return server_;
  }

 protected:
  ScopedHTTPServer(HTTPServer::Config config,
                   std::shared_ptr<HTTPHandler> handler)
      : server_(std::move(config), std::move(handler)) {
  }

 private:
  std::thread thread_;
  HTTPServer server_;
};

} // namespace proxygen::coro
