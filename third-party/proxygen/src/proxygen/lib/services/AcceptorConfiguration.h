/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <chrono>
#include <fcntl.h>
#include <folly/String.h>
#include <folly/io/async/AsyncSocket.h>
#include <list>
#include <proxygen/lib/http/codec/HTTPSettings.h>
#include <string>
#include <sys/stat.h>
#include <sys/types.h>
#include <wangle/acceptor/ServerSocketConfig.h>
#include <zlib.h>

namespace proxygen {
class HeaderIndexingStrategy;

/**
 * Configuration for a single Acceptor.
 *
 * This configures not only accept behavior, but also some types of SSL
 * behavior that may make sense to configure on a per-VIP basis (e.g. which
 * cert(s) we use, etc).
 */
struct AcceptorConfiguration : public wangle::ServerSocketConfig {
  /**
   * Determines if connection should respect HTTP2 priorities
   **/
  bool HTTP2PrioritiesEnabled{true};

  /**
   * The number of milliseconds a transaction can be idle before we close it.
   */
  std::chrono::milliseconds transactionIdleTimeout{600000};

  /**
   * The compression level to use for SPDY headers with responses from
   * this Acceptor.
   */
  int spdyCompressionLevel{Z_NO_COMPRESSION};

  /**
   * The name of the protocol to use on non-TLS connections.
   */
  std::string plaintextProtocol;

  /**
   * Comma separated string of protocols that can be upgraded to from HTTP/1.1
   */
  std::list<std::string> allowedPlaintextUpgradeProtocols;

  /**
   * True if HTTP/1.0 messages should always be serialized as HTTP/1.1
   *
   * Maximizes connection re-use
   */
  bool forceHTTP1_0_to_1_1{false};

  /**
   * HTTP/2 or SPDY settings for this acceptor
   */
  SettingsList egressSettings;

  /**
   * The maximum number of transactions the remote could initiate
   * per connection on protocols that allow multiplexing.
   */
  uint32_t maxConcurrentIncomingStreams{0};

  /**
   * Flow control parameters.
   *
   *  initialReceiveWindow     = amount to advertise to peer via SETTINGS
   *  receiveStreamWindowSize  = amount to increase per-stream window via
   *                             WINDOW_UPDATE
   *  receiveSessionWindowSize = amount to increase per-session window via
   *                             WINDOW_UPDATE
   *                             This also controls the size of the per-session
   *                             read buffer.
   */
  size_t initialReceiveWindow{65536};
  size_t receiveStreamWindowSize{65536};
  size_t receiveSessionWindowSize{65536};

  /**
   * These parameters control how many bytes HTTPSession's will buffer in user
   * space before applying backpressure to handlers.  -1 means use the
   * built-in HTTPSession default (64kb)
   */
  int64_t writeBufferLimit{-1};

  /**
   * Determines if HTTP2 ping is enabled on connection
   **/
  bool HTTP2PingEnabled{false};

  /* Strategy for which headers to insert into HPACK/QPACK dynamic table */
  const HeaderIndexingStrategy* headerIndexingStrategy{nullptr};
};

} // namespace proxygen
