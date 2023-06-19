/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <folly/Function.h>
#include <folly/SocketAddress.h>
#include <folly/io/async/AsyncServerSocket.h>
#include <proxygen/httpserver/Filters.h>
#include <proxygen/httpserver/RequestHandlerFactory.h>
#include <signal.h>

namespace proxygen {

/**
 * Configuration options for HTTPServer
 *
 * XXX: Provide a helper that can convert thrift/json to this config
 *      directly. We keep this object type-safe.
 */
class HTTPServerOptions {
 public:
  /**
   * Type of function to run inside onNewConnection() of acceptors.
   * If the function throws, the socket will be closed immediately. Useful
   * for validating client cert before processing the request.
   */
  using NewConnectionFilter =
      folly::Function<void(const folly::AsyncTransport* /* sock */,
                           const folly::SocketAddress* /* address */,
                           const std::string& /* nextProtocolName */,
                           SecureTransportType /* secureTransportType */,
                           const wangle::TransportInfo& /* tinfo */) const>;

  /**
   * Number of threads to start to handle requests. Note that this excludes
   * the thread you call `HTTPServer.start()` in. 0 means 1 thread per-cpu.
   *
   * XXX: Put some perf numbers to help user decide how many threads to
   *      create.
   */
  size_t threads = 1;

  /**
   * Chain of RequestHandlerFactory that are used to create RequestHandler
   * which handles requests.
   *
   * You can do something like -
   *
   * handlerFactories = RequestHandlerChain()
   *    .addThen<StatsFilter>()
   *    .addThen<TraceFilter>()
   *    .addThen<AccessLogFilter>()
   *    .addThen<AppSpecificHandler>()
   *    .build();
   */
  std::vector<std::unique_ptr<RequestHandlerFactory>> handlerFactories;

  /**
   * This idle timeout serves two purposes -
   *
   * 1. How long to keep idle connections around before throwing them away.
   *
   * 2. If it takes client more than this time to send request, we fail the
   *    request.
   *
   * XXX: Maybe have separate time limit for both?
   */
  std::chrono::milliseconds idleTimeout{60000};

  /**
   * TCP server socket backlog to start with.
   */
  uint32_t listenBacklog{1024};

  /**
   * Enable cleartext upgrades to HTTP/2
   */
  bool h2cEnabled{false};

  /**
   * Signals on which to shutdown the server. Mostly you will want
   * {SIGINT, SIGTERM}. Note, if you have multiple deamons running or you want
   * to have a separate signal handler, leave this empty and handle signals
   * yourself.
   */
  std::vector<int> shutdownOn{};

  /**
   * Set to true if you want to support CONNECT request. Most likely you
   * don't want that.
   */
  bool supportsConnect{false};

  /**
   * Flow control configuration for the acceptor
   */
  size_t initialReceiveWindow{65536};
  size_t receiveStreamWindowSize{65536};
  size_t receiveSessionWindowSize{65536};

  /**
   * The maximum number of transactions the remote could initiate
   * per connection on protocols that allow multiplexing.
   */
  uint32_t maxConcurrentIncomingStreams{100};

  /**
   * Set to true to enable content compression. Currently false for
   * backwards compatibility.  If enabled, by default gzip will be enabled
   * and zstd will be disabed.
   */
  bool enableContentCompression{false};

  /**
   * Set to true to enable zstd compression. Currently false for
   * backwards compatibility.
   * Only applicable if enableContentCompression is set to true.
   */
  bool enableZstdCompression{false};

  /**
   * Set to true to compress independent chunks for zstd.
   * Only applicable if enableContentCompression and enableZstdCompression are
   * set to true.
   */
  bool useZstdIndependentChunks{false};

  /**
   * Set to false to disable GZIP compression.  This can be helpful for services
   * with long-lived streams for which GZIP can result in high memory usage.
   * NOTE: this does not override `enableContentCompression`.
   */
  bool enableGzipCompression{true};

  /**
   * Requests smaller than the specified number of bytes will not be compressed
   */
  uint64_t contentCompressionMinimumSize{1000};

  /**
   * Zlib compression level, valid values are -1(Default) to 9(Slower).
   * 4 or 6 are a good balance between compression level and cpu usage.
   */
  int contentCompressionLevel{-1};

  /**
   * Zstd compression level, valid values are -5 to 22.
   * As level increases, compression ratio improves at the cost
   * of higher cpu usage.
   * Default is 8, which was found to be a good balance
   * between compression ratio and cpu usage.
   */
  int zstdContentCompressionLevel{8};

  /**
   * Enable support for pub-sub extension.
   */
  bool enableExHeaders{false};

  /**
   * Content types to compress, all entries as lowercase
   */
  std::set<std::string> contentCompressionTypes = {
      "application/javascript",
      "application/json",
      "application/x-javascript",
      "application/xhtml+xml",
      "application/xml",
      "application/xml+rss",
      "text/css",
      "text/html",
      "text/javascript",
      "text/plain",
      "text/xml",
  };

  /**
   * This holds sockets already bound to addresses that the server
   * will listen on and will be empty once the server starts.
   */
  std::vector<folly::AsyncServerSocket::UniquePtr> preboundSockets_;

  /**
   * Bind to existing file descriptor(s).
   * AsyncServerSocket can handle multiple fds and they can be provided
   * as a vector here.
   */
  void useExistingSocket(folly::AsyncServerSocket::UniquePtr socket) {
    preboundSockets_.push_back(std::move(socket));
  }

  void useExistingSocket(int socketFd) {
    useExistingSockets({socketFd});
  }

  void useExistingSockets(const std::vector<int>& socketFds) {
    folly::AsyncServerSocket::UniquePtr socket(new folly::AsyncServerSocket);

    std::vector<folly::NetworkSocket> sockets;
    sockets.reserve(socketFds.size());
    for (auto s : socketFds) {
      sockets.push_back(folly::NetworkSocket::fromFd(s));
    }
    socket->useExistingSockets(sockets);
    useExistingSocket(std::move(socket));
  }

  /**
   * Invoked after a new connection is created. Drop connection if the function
   * throws any exception.
   */
  NewConnectionFilter newConnectionFilter;

  /**
   * Use zero copy socket option
   */
  bool useZeroCopy{false};

  /**
   *  zerocopy enable function
   */
  folly::AsyncWriter::ZeroCopyEnableFunc zeroCopyEnableFunc;
};
} // namespace proxygen
