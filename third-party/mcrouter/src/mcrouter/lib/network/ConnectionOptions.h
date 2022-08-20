/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <chrono>
#include <memory>

#include <folly/io/SocketOptionMap.h>
#include <folly/io/async/AsyncSocket.h>

#include "mcrouter/lib/CompressionCodecManager.h"
#include "mcrouter/lib/mc/protocol.h"
#include "mcrouter/lib/network/AccessPoint.h"
#include "mcrouter/lib/network/SecurityOptions.h"

namespace facebook {
namespace memcache {

/**
 * A struct for storing all connection related options.
 */
struct ConnectionOptions {
  using SocketOptions = folly::SocketOptionMap;

  ConnectionOptions(
      folly::StringPiece host_,
      uint16_t port_,
      mc_protocol_t protocol_,
      SecurityMech mech_ = SecurityMech::NONE)
      : accessPoint(
            std::make_shared<AccessPoint>(host_, port_, protocol_, mech_)) {}

  explicit ConnectionOptions(std::shared_ptr<const AccessPoint> ap)
      : accessPoint(std::move(ap)) {}

  /**
   * Access point of the destination.
   */
  std::shared_ptr<const AccessPoint> accessPoint;

  /**
   * Number of TCP KeepAlive probes to send before considering connection dead.
   *
   * Note: Option will be enabled iff it is supported by the OS and this
   *       value > 0.
   */
  int tcpKeepAliveCount{0};

  /**
   * Interval between last data packet sent and the first TCP KeepAlive probe.
   */
  int tcpKeepAliveIdle{0};

  /**
   * Interval between two consequent TCP KeepAlive probes.
   */
  int tcpKeepAliveInterval{0};

  /**
   * The number of times to retry establishing a connection in case of a
   * connect timeout. We will just return the result back to the client after
   * either the connection is esblished, or we exhausted all retries.
   */
  unsigned int numConnectTimeoutRetries{0};

  /**
   * Connect timeout in ms.
   */
  std::chrono::milliseconds connectTimeout{0};

  /**
   * Write timeout in ms.
   */
  std::chrono::milliseconds writeTimeout{0};

  /**
   * Informs whether QoS is enabled.
   */
  bool enableQoS{false};

  /*
   * QoS class to use in packages.
   */
  unsigned int qosClass{0};

  /*
   * QoS path to use in packages.
   */
  unsigned int qosPath{0};

  /**
   * Path of the debug fifo.
   * If empty, debug fifo is disabled.
   */
  std::string debugFifoPath;

  /**
   * Name of the router that owns this connection.
   * NOTE: Must be be a literal (constexpr), and shouldn't be used
   * outside of mcrouter.
   */
  folly::StringPiece routerInfoName;

  /**
   * Security options for this connection
   */
  SecurityOptions securityOpts;

  /**
   * Use JemallocNodumpAllocator
   */
  bool useJemallocNodumpAllocator{false};

  /**
   * Map of codecs to use for compression.
   * If nullptr, compression will be disabled.
   */
  const CompressionCodecMap* compressionCodecMap{nullptr};

  /**
   * True to enable thrift compression.
   */
  bool thriftCompression{false};

  /**
   * Payloads >= thriftCompressionTreshold will be compressed
   * iff thriftCompression is enabled.
   */
  size_t thriftCompressionThreshold{0};
};
} // namespace memcache
} // namespace facebook
