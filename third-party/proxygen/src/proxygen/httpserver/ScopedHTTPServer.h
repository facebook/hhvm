/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <boost/thread.hpp>
#include <chrono>
#include <folly/io/async/SSLContext.h>
#include <folly/system/ThreadName.h>
#include <proxygen/httpserver/HTTPServer.h>
#include <proxygen/httpserver/ResponseBuilder.h>

namespace proxygen {

template <typename HandlerType>
class ScopedHandler : public RequestHandler {
 public:
  explicit ScopedHandler(HandlerType* ptr) : handlerPtr_(ptr) {
  }

  void onRequest(std::unique_ptr<HTTPMessage> headers) noexcept override {
    request_ = std::move(headers);
  }

  void onBody(std::unique_ptr<folly::IOBuf> body) noexcept override {
    requestBody_.append(std::move(body));
  }

  void onUpgrade(proxygen::UpgradeProtocol) noexcept override {
  }

  void onEOM() noexcept override {
    try {
      ResponseBuilder r(downstream_);
      (*handlerPtr_)(*request_, requestBody_.move(), r);
      r.sendWithEOM();
    } catch (const std::exception& ex) {
      ResponseBuilder(downstream_)
          .status(500, "Internal Server Error")
          .body(ex.what())
          .sendWithEOM();
    } catch (...) {
      ResponseBuilder(downstream_)
          .status(500, "Internal Server Error")
          .body("Unknown exception thrown")
          .sendWithEOM();
    }
  }

  void requestComplete() noexcept override {
    delete this;
  }

  void onError(ProxygenError) noexcept override {
    delete this;
  }

 private:
  HandlerType* const handlerPtr_{nullptr};

  std::unique_ptr<HTTPMessage> request_;
  folly::IOBufQueue requestBody_;
};

template <typename HandlerType>
class ScopedHandlerFactory : public RequestHandlerFactory {
 public:
  explicit ScopedHandlerFactory(HandlerType handler) : handler_(handler) {
  }

  void onServerStart(folly::EventBase*) noexcept override {
  }

  void onServerStop() noexcept override {
  }

  RequestHandler* onRequest(RequestHandler*, HTTPMessage*) noexcept override {
    return new ScopedHandler<HandlerType>(&handler_);
  }

 private:
  HandlerType handler_;
};

/**
 * A basic server that can be used for testing http clients. Since most such
 * servers are short lived, this server takes care of starting and stopping
 * automatically.
 */
class ScopedHTTPServer final {
 public:
  /**
   * Start a server listening on the requested `port`.
   * If `port` is 0, it will choose a random port.
   */
  template <typename HandlerType>
  static std::unique_ptr<ScopedHTTPServer> start(
      HandlerType handler,
      int port = 0,
      int numThreads = 4,
      std::unique_ptr<wangle::SSLContextConfig> sslCfg = nullptr,
      std::chrono::milliseconds idleTimeoutMs = std::chrono::milliseconds(0));

  /**
   * Start a server listening with the requested IPConfig and server opts
   */
  static std::unique_ptr<ScopedHTTPServer> start(HTTPServer::IPConfig cfg,
                                                 HTTPServerOptions options);

  /**
   * Get the port the server is listening on. This is helpful if the port was
   * randomly chosen.
   */
  int getPort() const {
    return getAddresses()[0].address.getPort();
  }

  /**
   * Get the addresses for the server.
   */
  std::vector<HTTPServer::IPConfig> getAddresses() const {
    auto addresses = server_->addresses();
    CHECK(!addresses.empty());
    return addresses;
  }

  ~ScopedHTTPServer() {
    server_->stop();
    thread_.join();
  }

 private:
  ScopedHTTPServer(std::thread thread, std::unique_ptr<HTTPServer> server)
      : thread_(std::move(thread)), server_(std::move(server)) {
  }

  std::thread thread_;
  std::unique_ptr<HTTPServer> server_;
};

template <typename HandlerType>
inline std::unique_ptr<ScopedHTTPServer> ScopedHTTPServer::start(
    HandlerType handler,
    int port,
    int numThreads,
    std::unique_ptr<wangle::SSLContextConfig> sslCfg,
    std::chrono::milliseconds idleTimeoutMs) {

  std::unique_ptr<RequestHandlerFactory> f =
      std::make_unique<ScopedHandlerFactory<HandlerType>>(handler);
  return start(
      std::move(f), port, numThreads, std::move(sslCfg), idleTimeoutMs);
}

template <>
inline std::unique_ptr<ScopedHTTPServer>
ScopedHTTPServer::start<std::unique_ptr<RequestHandlerFactory>>(
    std::unique_ptr<RequestHandlerFactory> f,
    int port,
    int numThreads,
    std::unique_ptr<wangle::SSLContextConfig> sslCfg,
    std::chrono::milliseconds idleTimeoutMs) {
  // This will handle both IPv4 and IPv6 cases
  folly::SocketAddress addr;
  addr.setFromLocalPort(port);

  HTTPServer::IPConfig cfg{addr, HTTPServer::Protocol::HTTP};

  if (sslCfg) {
    cfg.sslConfigs.push_back(*sslCfg);
  }

  HTTPServerOptions options;
  options.threads = numThreads;
  options.handlerFactories.push_back(std::move(f));
  if (idleTimeoutMs.count() > 0) {
    options.idleTimeout = idleTimeoutMs;
  }
  return start(std::move(cfg), std::move(options));
}

inline std::unique_ptr<ScopedHTTPServer> ScopedHTTPServer::start(
    HTTPServer::IPConfig cfg, HTTPServerOptions options) {

  std::vector<HTTPServer::IPConfig> IPs = {std::move(cfg)};

  auto server = std::make_unique<HTTPServer>(std::move(options));
  server->bind(IPs);

  // Start the server
  std::exception_ptr eptr;
  auto barrier = std::make_shared<boost::barrier>(2);

  std::thread t = std::thread([&, barrier]() {
    server->start(
        [&, barrier]() {
          folly::setThreadName("http-acceptor");
          barrier->wait();
        },
        [&, barrier](std::exception_ptr ex) {
          eptr = ex;
          barrier->wait();
        });
  });

  // Wait for server to start
  barrier->wait();
  if (eptr) {
    t.join();

    std::rethrow_exception(eptr);
  }

  return std::unique_ptr<ScopedHTTPServer>(
      new ScopedHTTPServer(std::move(t), std::move(server)));
}

} // namespace proxygen
