/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <atomic>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include <sys/socket.h>

#include <folly/SharedMutex.h>
#include <folly/io/async/VirtualEventBase.h>
#include <wangle/ssl/TLSTicketKeySeeds.h>

#include "mcrouter/lib/network/AsyncMcServerWorkerOptions.h"
#include "mcrouter/lib/network/CpuController.h"

namespace folly {
class EventBase;
class ScopedEventBaseThread;
} // namespace folly

namespace wangle {
class TLSCredProcessor;
} // namespace wangle

namespace facebook {
namespace memcache {

class AsyncMcServerWorker;
class McServerThread;
class McServerThreadSpawnController;

/**
 * A multithreaded, asynchronous MC protocol server.
 */
class AsyncMcServer {
 public:
  /**
   * Server startup options
   */
  struct Options {
    /**
     * Take over an exernally created sockets.
     * The server will call listen(), but not bind().
     * If this is used (not empty), ports must be empty.
     * It will be used as SSL socket if and only if all of pem* paths are set.
     */
    std::vector<int> existingSocketFds;

    /**
     * Create Unix Domain Socket to listen on.
     * If this is used (not empty), port must be empty,
     * existingSocketFds must be empty
     */
    std::string unixDomainSockPath;

    /**
     * TCP listen backlog
     */
    int tcpListenBacklog{SOMAXCONN};

    /**
     * The list of addresses to listen on.
     * If this is used, existingSocketFds must be empty
     */
    std::vector<std::string> listenAddresses;

    /**
     * The list of ports to listen on.
     * If this is used, existingSocketFds must be empty
     */
    std::vector<uint16_t> ports;

    /**
     * The list of ports to listen on for SSL connections.
     * If this is used, existingSocketFds must be empty
     */
    std::vector<uint16_t> sslPorts;

    /**
     * SSL cert/key/CA paths.
     * If sslPorts is non-empty, these must also be nonempty.
     */
    std::string pemCertPath;
    std::string pemKeyPath;
    std::string pemCaPath;

    /**
     * Whether to require peer certs when accepting SSL connections.
     */
    bool sslRequirePeerCerts{false};

    /**
     * Prefer AES-OCB cipher suite if available.
     */
    bool tlsPreferOcbCipher{false};

    /**
     * Path to JSON file containing old, current, and new seeds used for TLS
     * ticket key generation.
     */
    std::string tlsTicketKeySeedPath;

    /**
     * TFO settings (for SSL only)
     */
    bool tfoEnabledForSsl{false};
    uint32_t tfoQueueSize{0};

    /**
     * Number of threads to spawn, must be positive if number if virtual event
     * base mode is not used.
     */
    size_t numThreads{1};

    /**
     * If set, AsyncMcServer does not own create threads/EventBases and uses
     * the event bases in this vector to create Virtual Event Bases.
     */
    std::vector<folly::EventBase*> eventBases;

    /**
     * Number of threads that will listen for new connections. Must be > 0 &&
     * <= numThreads.
     */
    size_t numListeningSockets{1};

    /**
     * Worker-specific options
     */
    AsyncMcServerWorkerOptions worker;

    /**
     * CPU-based congestion controller.
     */
    CpuControllerOptions cpuControllerOpts;

    /**
     * Sets the maximum number of connections allowed.
     * Once that number is reached, AsyncMcServer will start closing connections
     * in a LRU fashion.
     *
     * NOTE: When setting globalMaxConns to a specific number (i.e. any
     *       value larger than 1), we will try to raise the rlimit to that
     *       number plus a small buffer for other files.
     *
     * @param globalMaxConns  The total number of connections allowed globally
     *                        (for the entire process).
     *                        NOTE: 0 and 1 have special meanings:
     *                          0  - do not reap connections;
     *                          1  - calculate maximum based on rlimits;
     *                          >1 - set per worker limits to
     *                               ceil(globalMaxConns / nThreads).
     *
     * @param nThreads        The number of threads we are going to run with.
     *                        Usually the same as numThreads field.
     *
     * @return  The actual max number of connections being used.
     *          This number should usually be equal to the value provided to
     *          globalMaxConns.
     *          It can smaller than globalMaxConns if we fail to raise the
     *          rlimit for some reason.
     *          NOTE: 0 means that no limit is being used.
     */
    size_t setMaxConnections(size_t globalMaxConns, size_t nThreads);
  };

  /**
   * User-defined loop function.
   * Args are threadId (0 to numThreads - 1), eventBase and the thread's worker
   * The user is responsible for calling eventBase.loop() or similar.
   */
  typedef std::function<
      void(size_t, folly::EventBase&, facebook::memcache::AsyncMcServerWorker&)>
      LoopFn;

  /**
   * User-defined init function to be used in Virtual Event Base mode.
   * Args are threadId (0 to numThreads - 1), eventBase and the thread's worker
   */
  typedef std::function<void(
      size_t,
      folly::VirtualEventBase&,
      facebook::memcache::AsyncMcServerWorker&)>
      InitFn;

  explicit AsyncMcServer(Options opts);
  ~AsyncMcServer();

  /**
   * @return Event bases from all worker threads ordered by threadId.
   */
  std::vector<folly::EventBase*> eventBases() const;

  /**
   * Spawn the required number of threads, and run the loop function in each
   * of them.
   *
   * @param onShutdown  called on shutdown. Worker threads will be stopped only
   *                    after the callback completes. It may be called from any
   *                    thread, but it is guaranteed the callback will be
   *                    executed exactly one time.
   *
   * @throws folly::AsyncSocketException
   *   If bind or listen fails.
   */
  void spawn(LoopFn fn, std::function<void()> onShutdown = nullptr);

  /**
   * Spawn the required number of worker objets and run setup for each of them
   * on the provided event bases. This mode enables AsyncMcServer to be run on
   * external event bases, where it is not responsible for running the loop fn.
   *
   * @param onShutdown  called on shutdown. Worker objects will be stopped only
   *                    after the callback completes. It may be called from any
   *                    thread, but it is guaranteed the callback will be
   *                    executed exactly one time.
   *
   * @throws folly::AsyncSocketException
   *   If bind or listen fails.
   */
  void startOnVirtualEB(InitFn fn, std::function<void()> onShutdown = nullptr);

  /**
   * Start shutting down all processing gracefully.  Will ensure that any
   * pending requests are replied, and any writes on the sockets complete.
   * Can only be called after spawn();
   *
   * Note: you still have to call join() to wait for all writes to complete.
   *
   * Can be called from any thread. Safe to call multiple times - the first
   * call will start shutting down, subsequent calls will not have any effect.
   */
  void shutdown();

  /**
   * MT-safety: safe to call from any thread or concurrently.
   * Calling this function ensures acceptors are stopped when exits. Calling
   * thread may be blocked if other thread is also calling.
   */
  void ensureAcceptorsShutdown();

  /**
   * Installs a new handler for the given signals that would shutdown
   * this server when delivered.
   * Note: only one server can be managed by these handlers per process.
   *
   * @param signals  List of signals
   */
  void installShutdownHandler(const std::vector<int>& signals);

  /**
   * Signal handler-safe version of shutdown.
   * Can only be called after spawn().
   */
  void shutdownFromSignalHandler();

  /**
   * Join all spawned threads.  Will exit upon server shutdown. When using
   * virtual event bases, this will block until the thread objects have
   * shutdown.
   */
  void join();

  /**
   * Getter/setter for seeds to be used to generate keys encrypting TLS tickets.
   */
  void setTicketKeySeeds(wangle::TLSTicketKeySeeds seeds);
  wangle::TLSTicketKeySeeds getTicketKeySeeds() const;

  bool virtualEventBaseEnabled() const {
    return virtualEventBaseMode_;
  }

 private:
  std::unique_ptr<folly::ScopedEventBaseThread> auxiliaryEvbThread_;
  Options opts_;
  std::unique_ptr<McServerThreadSpawnController> threadsSpawnController_;
  std::vector<std::unique_ptr<McServerThread>> threads_;

  std::unique_ptr<wangle::TLSCredProcessor> ticketKeySeedPoller_;
  wangle::TLSTicketKeySeeds tlsTicketKeySeeds_;
  mutable folly::SharedMutex tlsTicketKeySeedsLock_;

  std::function<void()> onShutdown_;
  std::atomic<bool> alive_{true};
  bool spawned_{false};

  std::mutex shutdownAcceptorsMutex_;
  bool acceptorsAlive_{true};

  bool virtualEventBaseMode_{false};
  std::vector<std::unique_ptr<folly::VirtualEventBase>> virtualEventBases_;

  enum class SignalShutdownState : uint64_t { STARTUP, SHUTDOWN, SPAWNED };
  std::atomic<SignalShutdownState> signalShutdownState_{
      SignalShutdownState::STARTUP};

  void startPollingTicketKeySeeds();

  void start(std::function<void()> onShutdown = nullptr);

  AsyncMcServer(const AsyncMcServer&) = delete;
  AsyncMcServer& operator=(const AsyncMcServer&) = delete;

  friend class McServerThread;
};

} // namespace memcache
} // namespace facebook
