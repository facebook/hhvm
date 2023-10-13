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
#include <chrono>
#include <folly/Portability.h>
#include <folly/experimental/observer/Observer.h>
#include <folly/io/SocketOptionMap.h>
#include <folly/sorted_vector_types.h>
#include <thrift/lib/cpp/concurrency/ThreadManager.h>
#include <thrift/lib/cpp2/Flags.h>
#include <thrift/lib/cpp2/server/ServerAttribute.h>

THRIFT_FLAG_DECLARE_int64(server_default_socket_queue_timeout_ms);
THRIFT_FLAG_DECLARE_int64(server_default_queue_timeout_ms);
THRIFT_FLAG_DECLARE_int64(server_polled_service_health_liveness_ms);
THRIFT_FLAG_DECLARE_int64(
    server_ingress_memory_limit_enforcement_payload_size_min_bytes);

namespace apache::thrift {
class ThriftServerInitialConfig;

class ThriftServerConfig {
 public:
  ThriftServerConfig() = default;
  ~ThriftServerConfig() = default;

  explicit ThriftServerConfig(const ThriftServerInitialConfig& initialConfig);

  /**
   * Get the prefix for naming the CPU (pool) threads.
   *
   * @return current setting.
   */
  std::string getCPUWorkerThreadName() const;

  // get value read from configerator
  std::optional<std::string> getBaselineCPUWorkerThreadName() const;

  /**
   * Get the timeout for joining workers.
   * @return workers joing timeout in seconds
   */
  std::chrono::seconds getWorkersJoinTimeout() const;

  std::optional<std::chrono::seconds> getBaselineWorkersJoinTimeout() const;

  /**
   * Get the maximum number of pending connections each io worker thread can
   * hold.
   */
  uint32_t getMaxNumPendingConnectionsPerWorker() const;

  std::optional<uint32_t> getBaselineMaxNumPendingConnectionsPerWorker() const;

  /** Get maximum number of milliseconds we'll wait for data (0 = infinity).
   *
   *  @return number of milliseconds, or 0 if no timeout set.
   */
  std::chrono::milliseconds getIdleTimeout() const;

  std::optional<std::chrono::milliseconds> getBaselineIdleTimeout() const;

  /** Get maximum number of milliseconds we'll keep the connection alive (0 =
   * infinity).
   *
   *  @return number of milliseconds, or 0 if no timeout set.
   */
  std::chrono::milliseconds getConnectionAgeTimeout() const;

  /**
   * Get the number of IO worker threads
   *
   * @return number of IO worker threads
   */
  size_t getNumIOWorkerThreads() const;

  std::optional<size_t> getBaselineNumIOWorkerThreads() const;

  /**
   * Get the number of CPU (pool) threads
   *
   * @return number of CPU (pool) threads
   */
  size_t getNumCPUWorkerThreads() const;

  std::optional<size_t> getBaselineNumCPUWorkerThreads() const;

  /**
   * Get the listen backlog.
   *
   * @return listen backlog.
   */
  int getListenBacklog() const;

  std::optional<int> getBaselineListenBacklog() const;

  const folly::sorted_vector_set<std::string>&
  getMethodsBypassMaxRequestsLimit() const;

  /**
   * Return the maximum memory usage by each debug payload.
   */
  uint64_t getMaxDebugPayloadMemoryPerRequest() const;

  /**
   * Return the maximum memory usage by each worker to keep track of debug
   * payloads.
   */
  uint64_t getMaxDebugPayloadMemoryPerWorker() const;

  /**
   * Return the maximum memory usage by each worker to keep track of debug
   * payloads.
   */
  uint16_t getMaxFinishedDebugPayloadsPerWorker() const;

  /**
   * Get the maximum # of connections allowed before overload.
   * @return current setting
   */
  const ServerAttributeDynamic<uint32_t>& getMaxConnections() const;

  const std::optional<uint32_t> getBaselineMaxConnections() const;

  /**
   * Get the maximum # of requests being processed in handler before overload.
   *
   * @return current setting.
   */
  const ServerAttributeDynamic<uint32_t>& getMaxRequests() const;

  const std::optional<uint32_t> getBaselineMaxRequests() const;

  const ServerAttributeDynamic<uint64_t>& getMaxResponseSize() const;

  const std::optional<uint64_t> getBaselineMaxResponseSize() const;

  /**
   * Get the maximum QPS this server is allowed to handle, above that we will
   * start shedding additional requests. Enforced via folly/TokenBucket.h.
   */
  const ServerAttributeDynamic<uint32_t>& getMaxQps() const;

  const ServerAttributeDynamic<bool>& getUseClientTimeout() const;

  const std::optional<bool> getBaselineUseClientTimeout() const;

  const ServerAttributeDynamic<bool>& getEnableCodel() const;

  /**
   * Get the task expire time
   *
   * @return task expire time
   */
  const ServerAttributeDynamic<std::chrono::milliseconds>& getTaskExpireTime()
      const;

  const std::optional<std::chrono::milliseconds> getBaselineTaskExpireTime()
      const;

  /**
   * If there is no request for the stream for the given time period, then the
   * stream will create timeout error.
   */
  const ServerAttributeDynamic<std::chrono::milliseconds>& getStreamExpireTime()
      const;

  const std::optional<std::chrono::milliseconds> getBaselineStreamExpireTime()
      const;

  /**
   * Get the time requests are allowed to stay on the queue
   *
   * @return queue timeout
   */
  const ServerAttributeDynamic<std::chrono::milliseconds>& getQueueTimeout()
      const;

  const std::optional<std::chrono::milliseconds> getBaselineQueueTimeout()
      const;
  /**
   * Gets an observer representing the socket queue timeout. If no value is
   * set, this falls back to the thrift flag,
   * server_default_socket_queue_timeout_ms.
   */
  const ServerAttributeDynamic<std::chrono::nanoseconds>&
  getSocketQueueTimeout() const;

  const std::optional<std::chrono::nanoseconds> getBaselineSocketQueueTimeout()
      const;

  const ServerAttributeDynamic<std::chrono::milliseconds>&
  getSocketWriteTimeout() const;

  const ServerAttributeDynamic<size_t>& getIngressMemoryLimit() const;

  const ServerAttributeDynamic<size_t>& getEgressMemoryLimit() const;

  const ServerAttributeDynamic<size_t>&
  getMinPayloadSizeToEnforceIngressMemoryLimit() const;

  /**
   * Get Queue Timeout Percentage, to tune Queue Timeout based on Client
   * timeout.
   */
  const ServerAttributeDynamic<uint32_t>& getQueueTimeoutPct() const;

  const ServerAttributeDynamic<size_t>& getEgressBufferBackpressureThreshold()
      const;

  const ServerAttributeDynamic<double>& getEgressBufferRecoveryFactor() const;

  const ServerAttributeDynamic<std::chrono::milliseconds>&
  getPolledServiceHealthLiveness() const;

  const ServerAttributeDynamic<folly::SocketOptionMap>&
  getPerConnectionSocketOptions() const;

  /**
   * Get write batching interval
   */
  const ServerAttributeDynamic<std::chrono::milliseconds>&
  getWriteBatchingInterval() const;

  /**
   * Get write batching size
   */
  const ServerAttributeDynamic<size_t>& getWriteBatchingSize() const;

  /**
   * Get write batching byte size
   */
  const ServerAttributeDynamic<size_t>& getWriteBatchingByteSize() const;

  /**
   * Get max response write time
   */
  const ServerAttributeDynamic<std::chrono::milliseconds>&
  getMaxResponseWriteTime() const;

  /**
   * Indicate whether it is safe to modify the server config through setters.
   * This roughly corresponds to whether the IO thread pool could be servicing
   * requests.
   *
   * @return true if the configuration can be modified, false otherwise
   */
  bool isFrozen() const { return frozen_; }

  void freeze() { frozen_ = true; }

  void unfreeze() { frozen_ = false; }

  /**
   * Set the prefix for naming the CPU (pool) threads. Not set by default.
   * must be called before serve() for it to take effect
   * ignored if setThreadManager() is called.
   *
   * @param cpuWorkerThreadName thread name prefix
   */
  void setCPUWorkerThreadName(
      const std::string& cpuWorkerThreadName,
      AttributeSource source = AttributeSource::OVERRIDE);

  void resetCPUWorkerThreadName(
      AttributeSource source = AttributeSource::OVERRIDE);

  /**
   * Sets the timeout for joining workers
   * @param timeout new setting for timeout for joining requests.
   */
  void setWorkersJoinTimeout(
      std::chrono::seconds timeout,
      AttributeSource source = AttributeSource::OVERRIDE);

  void resetWorkersJoinTimeout(
      AttributeSource source = AttributeSource::OVERRIDE);

  /**
   * Set the maximum number of pending connections each io worker thread can
   * hold. No new connections will be sent to that io worker thread if there
   * are more than such number of unprocessed connections in that queue. If
   * every io worker thread's queue is full the connection will be dropped.
   */
  void setMaxNumPendingConnectionsPerWorker(
      uint32_t num, AttributeSource source = AttributeSource::OVERRIDE);

  void resetMaxNumPendingConnectionsPerWorker(
      AttributeSource source = AttributeSource::OVERRIDE);

  /** Set maximum number of milliseconds we'll wait for data (0 = infinity).
   *  Note: existing connections are unaffected by this call.
   *
   *  @param timeout number of milliseconds, or 0 to disable timeouts.
   */
  void setIdleTimeout(
      std::chrono::milliseconds timeout,
      AttributeSource source = AttributeSource::OVERRIDE);

  void resetIdleTimeout(AttributeSource source = AttributeSource::OVERRIDE);
  /**
   * Set the number of IO worker threads
   *
   * @param number of IO worker threads
   */
  void setNumIOWorkerThreads(
      size_t numIOWorkerThreads,
      AttributeSource source = AttributeSource::OVERRIDE);

  void resetNumIOWorkerThreads(
      AttributeSource source = AttributeSource::OVERRIDE);

  /**
   * Set the number of CPU (pool) threads.
   * Only valid if you do not also set a threadmanager. This controls the number
   * of normal priority threads; the Thrift thread manager can create additional
   * threads for other priorities.
   * If set to 0, the number of normal priority threads will be the same as
   * number of CPU cores.
   *
   * @param number of CPU (pool) threads
   */
  void setNumCPUWorkerThreads(
      size_t numCPUWorkerThreads,
      AttributeSource source = AttributeSource::OVERRIDE);

  void resetNumCPUWorkerThreads(
      AttributeSource source = AttributeSource::OVERRIDE);

  /**
   * Set the maximum memory usage by each debug payload.
   */
  void setMaxDebugPayloadMemoryPerRequest(
      uint64_t limit, AttributeSource source = AttributeSource::OVERRIDE);

  void resetMaxDebugPayloadMemoryPerRequest(
      AttributeSource source = AttributeSource::OVERRIDE);

  /**
   * Set the maximum memory usage by each worker to keep track of debug
   * payloads.
   */
  void setMaxDebugPayloadMemoryPerWorker(
      uint64_t limit, AttributeSource source = AttributeSource::OVERRIDE);

  void resetMaxDebugPayloadMemoryPerWorker(
      AttributeSource source = AttributeSource::OVERRIDE);

  /**
   * Set the maximum memory usage by each worker to keep track of debug
   * payloads.
   */
  void setMaxFinishedDebugPayloadsPerWorker(
      uint16_t limit, AttributeSource source = AttributeSource::OVERRIDE);

  void resetMaxFinishedDebugPayloadsPerWorker(
      AttributeSource source = AttributeSource::OVERRIDE);

  /**
   * Set the listen backlog. Refer to the comment on listenBacklog_ member for
   * details.
   */
  void setListenBacklog(
      int listenBacklog, AttributeSource source = AttributeSource::OVERRIDE);

  void resetListenBacklog(AttributeSource source = AttributeSource::OVERRIDE);

  void setMethodsBypassMaxRequestsLimit(
      const std::vector<std::string>& methods,
      AttributeSource source = AttributeSource::OVERRIDE);

  void resetMethodsBypassMaxRequestsLimit(
      AttributeSource source = AttributeSource::OVERRIDE);

  /** Set maximum number of milliseconds we'll keep the connection alive (0 =
   * infinity).
   *
   *  @param timeout number of milliseconds, or 0 to disable timeouts.
   */
  void setConnectionAgeTimeout(
      std::chrono::milliseconds timeout,
      AttributeSource source = AttributeSource::OVERRIDE);

  void resetConnectionAgeTimeout(
      AttributeSource source = AttributeSource::OVERRIDE);

  /**
   * Set the maximum # of connections allowed before overload.
   *
   * @param maxConnections new setting for maximum # of connections.
   */
  void setMaxConnections(
      folly::observer::Observer<std::optional<uint32_t>> maxConnections,
      AttributeSource source = AttributeSource::OVERRIDE);

  /**
   * Set the maximum # of requests being processed in handler before overload.
   *
   * @param maxRequests new setting for maximum # of active requests.
   */
  void setMaxRequests(
      folly::observer::Observer<std::optional<uint32_t>> maxRequests,
      AttributeSource source = AttributeSource::OVERRIDE);

  void setMaxResponseSize(
      folly::observer::Observer<std::optional<uint64_t>> size,
      AttributeSource source = AttributeSource::OVERRIDE);

  /**
   * Sets the maximum QPS this server is allowed to handle, above that we will
   * start shedding additional requests. Enforced via folly/TokenBucket.h.
   */
  void setMaxQps(
      folly::observer::Observer<std::optional<uint32_t>> maxQps,
      AttributeSource source = AttributeSource::OVERRIDE);

  void setUseClientTimeout(
      folly::observer::Observer<std::optional<bool>> useClientTimeout,
      AttributeSource source = AttributeSource::OVERRIDE);

  /**
   * Set the task expire time
   *
   */
  void setTaskExpireTime(
      folly::observer::Observer<std::optional<std::chrono::milliseconds>>
          timeout,
      AttributeSource source = AttributeSource::OVERRIDE);

  /**
   * Set the stream starvation time
   *
   */
  void setStreamExpireTime(
      folly::observer::Observer<std::optional<std::chrono::milliseconds>>
          timeout,
      AttributeSource source = AttributeSource::OVERRIDE);

  /**
   * Set the time requests are allowed to stay on the queue.
   * Note, queuing is an indication that your server cannot keep
   * up with load, and realtime systems should not queue. Only
   * override this if you do heavily batched requests.
   */
  void setQueueTimeout(
      folly::observer::Observer<std::optional<std::chrono::milliseconds>>
          timeout,
      AttributeSource source = AttributeSource::OVERRIDE);

  /**
   * Sets the duration before which new connections waiting on a socket's
   * queue are closed. A value of 0 represents an infinite duration. See
   * `folly::AsyncServerSocket::setQueueTimeout`.
   */
  void setSocketQueueTimeout(
      folly::observer::Observer<std::optional<std::chrono::milliseconds>>
          timeout,
      AttributeSource source = AttributeSource::OVERRIDE);

  /**
   * How long a socket with outbound data will tolerate read inactivity from
   * a client. Clients must read data from their end of the connection
   * before this period expires or the server will drop the connection. The
   * amount of data read by the client is irrelevant. Zero disables the
   * timeout.
   */
  void setSocketWriteTimeout(
      folly::observer::Observer<std::optional<std::chrono::milliseconds>>
          timeout,
      AttributeSource source = AttributeSource::OVERRIDE);

  /**
   * Ingress memory is the total memory used for receiving inflight
   * requests. If the memory limit is hit, the connection along with the
   * violating request will be closed
   */
  void setIngressMemoryLimit(
      folly::observer::Observer<std::optional<size_t>> ingressMemoryLimit,
      AttributeSource source = AttributeSource::OVERRIDE);

  /**
   * Limit the amount of memory available for inflight responses, meaning
   * responses that are queued on the server pending delivery to clients.
   * This limit, divided by the number of IO threads, determines the
   * effective egress limit of a connection. Once the per-connection limit
   * is reached, a connection is dropped immediately and all outstanding
   * responses are discarded.
   */
  void setEgressMemoryLimit(
      folly::observer::Observer<std::optional<size_t>> max,
      AttributeSource source = AttributeSource::OVERRIDE);

  /**
   * Connection close will only be enforced and triggered on those requests
   * with size greater or equal than this attribute
   */
  void setMinPayloadSizeToEnforceIngressMemoryLimit(
      folly::observer::Observer<std::optional<size_t>>
          minPayloadSizeToEnforceIngressMemoryLimit,
      AttributeSource source = AttributeSource::OVERRIDE);

  /**
   * Set the queue_timeout to client timeout percentage. If the queue_timeout
   * was set explicitly by client then this function does nothing.
   */
  void setQueueTimeoutPct(
      folly::observer::Observer<std::optional<uint32_t>> queueTimeoutPct,
      AttributeSource source = AttributeSource::OVERRIDE);

  /**
   * Apply backpressure to all stream generators of a connection when
   * combined allocation size of inflight writes for that connection exceeds
   * the threshold.
   */
  void setEgressBufferBackpressureThreshold(
      folly::observer::Observer<std::optional<size_t>> thresholdInBytes,
      AttributeSource source = AttributeSource::OVERRIDE);

  /**
   * When egress buffer backpressure is enabled, resume normal operation
   * once egress buffer size falls below this factor of the threshold.
   */
  void setEgressBufferRecoveryFactor(
      folly::observer::Observer<std::optional<double>> recoveryFactor,
      AttributeSource source = AttributeSource::OVERRIDE);

  void setPolledServiceHealthLiveness(
      folly::observer::Observer<std::optional<std::chrono::milliseconds>>
          liveness,
      AttributeSource source = AttributeSource::OVERRIDE);

  void setPerConnectionSocketOptions(
      folly::observer::Observer<std::optional<folly::SocketOptionMap>> options,
      AttributeSource source = AttributeSource::OVERRIDE);

  /**
   * Set write batching interval
   */
  void setWriteBatchingInterval(
      folly::observer::Observer<std::optional<std::chrono::milliseconds>>
          interval,
      AttributeSource source = AttributeSource::OVERRIDE);

  /**
   * Set write batching size. Ignored if write batching interval is not set.
   */
  void setWriteBatchingSize(
      folly::observer::Observer<std::optional<size_t>> batchingSize,
      AttributeSource source = AttributeSource::OVERRIDE);

  /**
   * Set write batching byte size. Ignored if write batching interval is not
   * set.
   */
  void setWriteBatchingByteSize(
      folly::observer::Observer<std::optional<size_t>> batchingByteSize,
      AttributeSource source = AttributeSource::OVERRIDE);

  /**
   * Set max response write time.
   */
  void setMaxResponseWriteTime(
      folly::observer::Observer<std::optional<std::chrono::milliseconds>>
          maxResponseWriteTime,
      AttributeSource source = AttributeSource::OVERRIDE);

 private:
  friend class ThriftServer;
  template <typename T>
  void setStaticAttribute(
      ServerAttributeStatic<T>& staticAttribute,
      T&& value,
      AttributeSource source) {
    CHECK(!isFrozen());
    staticAttribute.set(std::move(value), source);
  }

  template <typename T>
  void resetStaticAttribute(
      ServerAttributeStatic<T>& staticAttribute, AttributeSource source) {
    CHECK(!isFrozen());
    staticAttribute.reset(source);
  }

  //! Default number of worker threads (should be # of processor cores).
  static const size_t T_ASYNC_DEFAULT_WORKER_THREADS;

  static constexpr uint32_t T_MAX_NUM_PENDING_CONNECTIONS_PER_WORKER = 4096;

  static constexpr std::chrono::milliseconds DEFAULT_TIMEOUT =
      std::chrono::milliseconds(0);

  static constexpr std::chrono::milliseconds DEFAULT_TASK_EXPIRE_TIME =
      std::chrono::milliseconds(5000);

  static constexpr std::chrono::milliseconds DEFAULT_STREAM_EXPIRE_TIME =
      std::chrono::milliseconds(60000);

  static constexpr std::chrono::milliseconds DEFAULT_SOCKET_WRITE_TIMEOUT =
      std::chrono::milliseconds(60000);

  static constexpr std::chrono::seconds DEFAULT_WORKERS_JOIN_TIMEOUT =
      std::chrono::seconds(20);

  /// Listen backlog
  static constexpr int DEFAULT_LISTEN_BACKLOG = 1024;

  //! Prefix for pool thread names
  ServerAttributeStatic<std::string> poolThreadName_{"ThriftSrv"};

  //! Number of io worker threads (may be set) (should be # of CPU cores)
  ServerAttributeStatic<size_t> nWorkers_{T_ASYNC_DEFAULT_WORKER_THREADS};

  // Timeout for joining worker threads
  ServerAttributeStatic<std::chrono::seconds> workersJoinTimeout_{
      DEFAULT_WORKERS_JOIN_TIMEOUT};

  //! Number of CPU worker threads
  ServerAttributeStatic<size_t> nPoolThreads_{T_ASYNC_DEFAULT_WORKER_THREADS};

  ServerAttributeDynamic<bool> enableCodel_{false};

  //! Milliseconds we'll wait for data to appear (0 = infinity)
  ServerAttributeStatic<std::chrono::milliseconds> timeout_{DEFAULT_TIMEOUT};

  //! Milliseconds we'll keep the connection alive for (0 = infinity)
  ServerAttributeStatic<std::chrono::milliseconds> connectionAgeTimeout_{
      DEFAULT_TIMEOUT};

  /**
   * The time in milliseconds before an unperformed task expires
   * (0 == infinite)
   */
  ServerAttributeDynamic<std::chrono::milliseconds> taskExpireTime_{
      DEFAULT_TASK_EXPIRE_TIME};

  /**
   * The time in milliseconds before a stream starves of having no request.
   * (0 == infinite)
   */
  ServerAttributeDynamic<std::chrono::milliseconds> streamExpireTime_{
      DEFAULT_STREAM_EXPIRE_TIME};

  /**
   * The time we'll allow a task to wait on the queue and still perform it
   * (0 == infinite)
   */
  ServerAttributeDynamic<std::chrono::milliseconds> queueTimeout_{
      folly::observer::makeValueObserver(
          [timeoutMs = THRIFT_FLAG_OBSERVE(server_default_queue_timeout_ms)]() {
            // Timeouts should be disabled for debug builds since this can
            // generate false negatives in unit tests.
            if (folly::kIsDebug) {
              return std::chrono::milliseconds(0);
            }
            return std::chrono::milliseconds(**timeoutMs);
          })};

  /**
   * The time we'll allow a new connection socket to wait on the queue
   * before closing the connection. See
   * `folly::AsyncServerSocket::setQueueTimeout`.
   */
  ServerAttributeDynamic<std::chrono::nanoseconds> socketQueueTimeout_{
      folly::observer::makeObserver(
          [timeoutMs =
               THRIFT_FLAG_OBSERVE(server_default_socket_queue_timeout_ms)]()
              -> std::chrono::nanoseconds {
            // Disable timeout for debug builds and unit tests by default -
            // this is a production overload protection feature.
            if (folly::kIsDebug) {
              return std::chrono::milliseconds::zero();
            }
            return std::chrono::milliseconds(**timeoutMs);
          })};

  /**
   * How long a socket with outbound data will tolerate read inactivity from
   * a client. Clients must read data from their end of the connection
   * before this period expires or the server will drop the connection. The
   * amount of data read is irrelevant. Zero indicates no timeout.
   */
  ServerAttributeDynamic<std::chrono::milliseconds> socketWriteTimeout_{
      DEFAULT_SOCKET_WRITE_TIMEOUT};

  /**
   * The number of incoming connections the TCP stack will buffer up while
   * waiting for the Thrift server to call accept() on them.
   *
   * If the Thrift server cannot keep up, and this limit is reached, the
   * TCP stack will start sending resets to drop excess connections.
   *
   * Actual behavior of the socket backlog is dependent on the TCP
   * implementation, and it may be further limited or even ignored on some
   * systems. See manpage for listen(2) for details.
   */
  ServerAttributeStatic<int> listenBacklog_{DEFAULT_LISTEN_BACKLOG};

  /**
   * The maximum number of pending connections each io worker thread can
   * hold.
   */
  ServerAttributeStatic<uint32_t> maxNumPendingConnectionsPerWorker_{
      T_MAX_NUM_PENDING_CONNECTIONS_PER_WORKER};

  // Max number of active connections
  ServerAttributeDynamic<uint32_t> maxConnections_{0};

  // Max active requests
  ServerAttributeDynamic<uint32_t> maxRequests_{
      concurrency::ThreadManager::DEFAULT_MAX_QUEUE_SIZE};

  // If it is set true, server will check and use client timeout header
  ServerAttributeDynamic<bool> useClientTimeout_{true};

  // Max response size allowed. This is the size of the serialized and
  // transformed response, headers not included. 0 (default) means no limit.
  ServerAttributeDynamic<uint64_t> maxResponseSize_{0};

  // Max qps enforced with tokenbucket -- 0 is disabled.
  ServerAttributeDynamic<uint32_t> maxQps_{0};

  /**
   * The maximum memory usage (in bytes) by each request debug payload.
   * Payloads larger than this value will be simply dropped by
   * instrumentation.
   */
  ServerAttributeStatic<uint64_t> maxDebugPayloadMemoryPerRequest_{
      0x1000000}; // 16MB

  /**
   * The maximum memory usage by each worker to keep track of debug payload.
   * Each time a request payload is added for tracking, the tracker should
   * check whether it's using memory beyond this value and evict payloads
   * based on its policies.
   */
  ServerAttributeStatic<uint64_t> maxDebugPayloadMemoryPerWorker_{
      0x1000000}; // 16MB

  /**
   * The maximum number of debug payloads to track after request has
   * finished.
   */
  ServerAttributeStatic<uint16_t> maxFinishedDebugPayloadsPerWorker_{10};

  /**
   * Batch all writes withing given time interval.
   * (0 == disabled)
   */
  ServerAttributeDynamic<std::chrono::milliseconds> writeBatchingInterval_{
      std::chrono::milliseconds::zero()};

  /**
   * Trigger early flush when this number of writes are queued.
   * Ignored if write batching interval is not set.
   * (0 == disabled)
   */
  ServerAttributeDynamic<size_t> writeBatchingSize_{0};

  /**
   * Trigger early flush when the total number of bytes queued equals or
   * exceeds this value. Ignored if write batching interval is not set. (0
   * == disabled)
   */
  ServerAttributeDynamic<size_t> writeBatchingByteSize_{0};

  /**
   * Caps the amount of time that can be spent writing a single response (0 ==
   * disabled). If a response has not been fully written to the network within
   * maxResponseWriteTime milliseconds, the connection will be closed and all
   * resources belonging to requests owned by the connection will be returned to
   * the server.
   */
  ServerAttributeDynamic<std::chrono::milliseconds> maxResponseWriteTime_{
      std::chrono::milliseconds::zero()};

  ServerAttributeStatic<folly::sorted_vector_set<std::string>>
      methodsBypassMaxRequestsLimit_{{}};

  ServerAttributeDynamic<size_t> ingressMemoryLimit_{0};
  ServerAttributeDynamic<size_t> egressMemoryLimit_{0};
  ServerAttributeDynamic<size_t> minPayloadSizeToEnforceIngressMemoryLimit_{
      folly::observer::makeObserver(
          [o = THRIFT_FLAG_OBSERVE(
               server_ingress_memory_limit_enforcement_payload_size_min_bytes)]()
              -> size_t { return **o < 0 ? 0ul : static_cast<size_t>(**o); })};

  /**
   * Set this value to use client_timeout to determine queue_timeout. By default
   * this was disabled.
   */
  ServerAttributeDynamic<uint32_t> queueTimeoutPct_{0};

  /**
   * Per-connection threshold for number of allocated bytes allowed in
   * egress buffer before applying backpressure by pausing streams. (0 ==
   * disabled)
   */
  ServerAttributeDynamic<size_t> egressBufferBackpressureThreshold_{0};

  /**
   * Factor of egress buffer backpressure threshold at which to resume
   * streams. Should be set well below 1 to avoid rapidly turning
   * backpressure on/off. Ignored if backpressure threshold is disabled.
   */
  ServerAttributeDynamic<double> egressBufferRecoveryFactor_{0.75};

  /**
   * The duration of time that a polled ServiceHealth value is considered
   * current. i.e. another poll will only be scheduled after this amount of
   * time has passed since the last poll completed.
   *
   * @see apache::thrift::PolledServiceHealth
   */
  ServerAttributeDynamic<std::chrono::milliseconds>
      polledServiceHealthLiveness_{folly::observer::makeObserver(
          [livenessMs = THRIFT_FLAG_OBSERVE(
               server_polled_service_health_liveness_ms)]() {
            return std::chrono::milliseconds(**livenessMs);
          })};

  /**
   * Socket options that will be applied to every connection to clients.
   * If the socket does not support the specific option, it is silently
   * ignored. Refer to setsockopt() for more details.
   */
  ServerAttributeDynamic<folly::SocketOptionMap> perConnectionSocketOptions_{
      folly::emptySocketOptionMap};

  // Flag indicating whether it is safe to mutate the server config through
  // its setters.
  std::atomic<bool> frozen_{false};
};

class ThriftServerInitialConfig {
 public:
  FOLLY_CONSTEVAL ThriftServerInitialConfig() = default;

  // to fix oss for now we'll have this as a pair<T, bool> to mimick the
  // behavior of optional
#define THRIFT_SERVER_INITIAL_CONFIG_DEFINE(TYPE, NAME, INIT_VALUE) \
 private:                                                           \
  std::pair<TYPE, bool> NAME##_ = {INIT_VALUE, false};              \
                                                                    \
 public:                                                            \
  FOLLY_CONSTEVAL ThriftServerInitialConfig NAME(TYPE value) {      \
    auto initialConfig(*this);                                      \
    initialConfig.NAME##_.first = value;                            \
    initialConfig.NAME##_.second = true;                            \
    return initialConfig;                                           \
  }

  // replace with thrift struct if/when it becomes constexpr friendly

  THRIFT_SERVER_INITIAL_CONFIG_DEFINE(uint32_t, maxRequests, 0)
  THRIFT_SERVER_INITIAL_CONFIG_DEFINE(uint32_t, maxConnections, 0)
  THRIFT_SERVER_INITIAL_CONFIG_DEFINE(uint64_t, maxResponseSize, 0)
  THRIFT_SERVER_INITIAL_CONFIG_DEFINE(bool, useClientTimeout, true)
  THRIFT_SERVER_INITIAL_CONFIG_DEFINE(
      std::chrono::milliseconds, taskExpireTimeout, {})
  THRIFT_SERVER_INITIAL_CONFIG_DEFINE(
      std::chrono::milliseconds, streamExpireTimeout, {})
  THRIFT_SERVER_INITIAL_CONFIG_DEFINE(
      std::chrono::milliseconds, queueTimeout, {})
  THRIFT_SERVER_INITIAL_CONFIG_DEFINE(
      std::chrono::nanoseconds, socketQueueTimeout, {})
  THRIFT_SERVER_INITIAL_CONFIG_DEFINE(size_t, egressMemoryLimit, 0)
  THRIFT_SERVER_INITIAL_CONFIG_DEFINE(
      size_t, egressBufferBackpressureThreshold, 0)
  THRIFT_SERVER_INITIAL_CONFIG_DEFINE(size_t, ingressMemoryLimit, 0)
  THRIFT_SERVER_INITIAL_CONFIG_DEFINE(
      size_t, minPayloadSizeToEnforceIngressMemoryLimit, 0)

  /*
   * Running UBSan causes some tests to FATAL. The current theory is that this
   * is caused by the compiler not generating proper code for FOLLY_CONSTEVAL.
   * For now, we're adding a dummy field at the end as a hack to workaround this
   * issue, until its fixed. All new fields should be added before this last
   * dummy field
   */

  THRIFT_SERVER_INITIAL_CONFIG_DEFINE(
      std::chrono::milliseconds,
      lastUnusedField,
      {}) // DO NOT ADD FIELDS AFTER THIS
#undef THRIFT_SERVER_INITIAL_CONFIG_DEFINE

 private:
  friend class ThriftServerConfig;
};
} // namespace apache::thrift
