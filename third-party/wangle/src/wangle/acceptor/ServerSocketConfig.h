/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

#include <wangle/acceptor/FizzConfig.h>
#include <wangle/acceptor/SocketOptions.h>
#include <wangle/ssl/SNIConfig.h>
#include <wangle/ssl/SSLCacheOptions.h>
#include <wangle/ssl/SSLContextConfig.h>
#include <wangle/ssl/SSLUtil.h>
#include <wangle/ssl/TLSTicketKeySeeds.h>

#include <boost/optional.hpp>
#include <fcntl.h>
#include <folly/Random.h>
#include <folly/SocketAddress.h>
#include <folly/String.h>
#include <folly/io/SocketOptionMap.h>
#include <folly/io/async/AsyncSocket.h>
#include <folly/io/async/SSLContext.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <chrono>
#include <list>
#include <string>

namespace wangle {

/**
 * Base class for custom configs.
 */
class CustomConfig {
 public:
  virtual ~CustomConfig() = default;
};

/**
 * Configuration for a single Acceptor.
 *
 * This configures not only accept behavior, but also some types of SSL
 * behavior that may make sense to configure on a per-VIP basis (e.g. which
 * cert(s) we use, etc).
 */
struct ServerSocketConfig {
  ServerSocketConfig() {
    // generate a single random current seed
    uint8_t seed[32];
    folly::Random::secureRandom(seed, sizeof(seed));
    initialTicketSeeds.currentSeeds.push_back(
        SSLUtil::hexlify(std::string((char*)seed, sizeof(seed))));
  }

  bool isSSL() const {
    return !(sslContextConfigs.empty() && sniConfigs.empty());
  }

  /**
   * Set/get the socket options to apply on all downstream connections.
   */
  void setSocketOptions(const folly::SocketOptionMap& opts) {
    socketOptions_ = filterIPSocketOptions(opts, bindAddress.getFamily());
  }
  folly::SocketOptionMap& getSocketOptions() {
    return socketOptions_;
  }
  const folly::SocketOptionMap& getSocketOptions() const {
    return socketOptions_;
  }

  bool hasExternalPrivateKey() const {
    for (const auto& cfg : sslContextConfigs) {
      if (!cfg.isLocalPrivateKey) {
        return true;
      }
    }
    for (const auto& cfg : sniConfigs) {
      if (!cfg.contextConfig.isLocalPrivateKey) {
        return true;
      }
    }
    return false;
  }

  bool hasProdCASignedCert() const {
    for (const auto& cfg : sslContextConfigs) {
      if (cfg.isProdCASigned) {
        return true;
      }
    }
    return false;
  }

  /**
   * This should only be called from the evb thread.
   */
  void updateSSLContextConfigs(std::vector<SSLContextConfig> newConfigs) const {
    sslContextConfigs = newConfigs;
  }

  void updateSNIConfigs(std::vector<SNIConfig> newConfigs) const {
    sniConfigs = newConfigs;
  }

  /**
   * The name of this acceptor; used for stats/reporting purposes.
   */
  std::string name;

  /**
   * The depth of the accept queue backlog.
   */
  uint32_t acceptBacklog{1024};

  /**
   * The maximum number of pending connections each io worker thread can hold.
   */
  uint32_t maxNumPendingConnectionsPerWorker{1024};

  /**
   * The number of milliseconds a connection can be idle before we close it.
   */
  std::chrono::milliseconds connectionIdleTimeout{600000};

  /**
   * The number of milliseconds a connection can remain alive for (0 = infinity)
   */
  std::chrono::milliseconds connectionAgeTimeout{0};

  /**
   * The number of milliseconds a ssl handshake can timeout (60s)
   */
  std::chrono::milliseconds sslHandshakeTimeout{60000};

  /**
   * The address to bind to.
   */
  folly::SocketAddress bindAddress;

  /**
   * Options for controlling the SSL cache.
   */
  SSLCacheOptions sslCacheOptions{
      std::chrono::hours(1),
      20480,
      200,
      std::chrono::hours(72)};

  /**
   * Determines whether or not to allow insecure connections over a secure
   * port. Can be used to multiplex TLS and plaintext on the same port for
   * some services.
   */
  bool allowInsecureConnectionsOnSecureServer{false};

  /**
   * The initial TLS ticket seeds.
   */
  TLSTicketKeySeeds initialTicketSeeds;

  /**
   * The configs for all the SSL_CTX for use by this Acceptor.
   */
  mutable std::vector<SSLContextConfig> sslContextConfigs;

  /**
   * The configs for all the SNIs served by this Acceptor.
   */
  mutable std::vector<SNIConfig> sniConfigs;

  /**
   * Determines if the Acceptor does strict checking when loading the SSL
   * contexts.
   */
  bool strictSSL{true};

  /**
   * Maximum number of concurrent pending SSL handshakes
   */
  uint32_t maxConcurrentSSLHandshakes{30720};

  /**
   * Whether to enable TCP fast open. Before turning this
   * option on, for it to work, it must also be enabled on the
   * machine via /proc/sys/net/ipv4/tcp_fastopen, and the keys for
   * TFO should also be set at /proc/sys/net/ipv4/tcp_fastopen_key
   */
  bool enableTCPFastOpen{false};

  /**
   * Use zero copy socket option
   */
  bool useZeroCopy{false};

  /**
   * Limit on size of queue of TFO requests by clients.
   */
  uint32_t fastOpenQueueSize{100};

  /**
   * Configures the initial setting for maxReadsPerEvent on accepted sockets.
   */
  uint32_t socketMaxReadsPerEvent{16};

  /**
   * Enable so-reuseport while binding the listening socket
   */
  bool reusePort{false};

  FizzConfig fizzConfig;

  /**
   * If supported prefer using IoUring sockets
   */
  bool preferIoUring{false};

  /**
   * A map containing custom configs.
   */
  std::unordered_map<std::string, std::shared_ptr<CustomConfig>>
      customConfigMap;

 private:
  folly::SocketOptionMap socketOptions_;
};

} // namespace wangle
