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

#include <thrift/lib/cpp2/server/ThriftServerConfig.h>

THRIFT_FLAG_DEFINE_int64(server_default_socket_queue_timeout_ms, 100);
THRIFT_FLAG_DEFINE_int64(server_default_queue_timeout_ms, 100);
THRIFT_FLAG_DEFINE_int64(server_polled_service_health_liveness_ms, 100);
THRIFT_FLAG_DEFINE_int64(
    server_ingress_memory_limit_enforcement_payload_size_min_bytes, 1024);
THRIFT_FLAG_DEFINE_bool(server_reject_header_connections, false);

namespace apache {
namespace thrift {

const size_t ThriftServerConfig::T_ASYNC_DEFAULT_WORKER_THREADS =
    std::thread::hardware_concurrency();

ThriftServerConfig::ThriftServerConfig(
    const ThriftServerInitialConfig& initialConfig)
    : ThriftServerConfig() {
  if (auto& [value, isSet] = initialConfig.maxRequests_; isSet) {
    maxRequests_.setDefault(folly::observer::makeStaticObserver(value));
  }
  if (auto& [value, isSet] = initialConfig.maxConnections_; isSet) {
    maxConnections_.setDefault(folly::observer::makeStaticObserver(value));
  }
  if (auto& [value, isSet] = initialConfig.maxResponseSize_; isSet) {
    maxResponseSize_.setDefault(folly::observer::makeStaticObserver(value));
  }
  if (auto& [value, isSet] = initialConfig.useClientTimeout_; isSet) {
    useClientTimeout_.setDefault(folly::observer::makeStaticObserver(value));
  }
  if (auto& [value, isSet] = initialConfig.taskExpireTimeout_; isSet) {
    taskExpireTime_.setDefault(folly::observer::makeStaticObserver(value));
  }
  if (auto& [value, isSet] = initialConfig.streamExpireTimeout_; isSet) {
    streamExpireTime_.setDefault(folly::observer::makeStaticObserver(value));
  }
  if (auto& [value, isSet] = initialConfig.queueTimeout_; isSet) {
    queueTimeout_.setDefault(folly::observer::makeStaticObserver(value));
  }
  if (auto& [value, isSet] = initialConfig.socketQueueTimeout_; isSet) {
    socketQueueTimeout_.setDefault(folly::observer::makeStaticObserver(value));
  }
  if (auto& [value, isSet] = initialConfig.egressMemoryLimit_; isSet) {
    egressMemoryLimit_.setDefault(folly::observer::makeStaticObserver(value));
  }
  if (auto& [value, isSet] = initialConfig.egressBufferBackpressureThreshold_;
      isSet) {
    egressBufferBackpressureThreshold_.setDefault(
        folly::observer::makeStaticObserver(value));
  }
  if (auto& [value, isSet] = initialConfig.ingressMemoryLimit_; isSet) {
    ingressMemoryLimit_.setDefault(folly::observer::makeStaticObserver(value));
  }
  if (auto& [value, isSet] =
          initialConfig.minPayloadSizeToEnforceIngressMemoryLimit_;
      isSet) {
    minPayloadSizeToEnforceIngressMemoryLimit_.setDefault(
        folly::observer::makeStaticObserver(value));
  }
}

std::string ThriftServerConfig::getCPUWorkerThreadName() const {
  return poolThreadName_.get();
}

std::optional<std::string> ThriftServerConfig::getBaselineCPUWorkerThreadName()
    const {
  return poolThreadName_.getBaseline();
}

std::chrono::seconds ThriftServerConfig::getWorkersJoinTimeout() const {
  return workersJoinTimeout_.get();
}

std::optional<std::chrono::seconds>
ThriftServerConfig::getBaselineWorkersJoinTimeout() const {
  return workersJoinTimeout_.getBaseline();
}

int ThriftServerConfig::getListenBacklog() const {
  return listenBacklog_.get();
}

std::optional<int> ThriftServerConfig::getBaselineListenBacklog() const {
  return listenBacklog_.getBaseline();
}

std::chrono::milliseconds ThriftServerConfig::getIdleTimeout() const {
  return timeout_.get();
}

std::optional<std::chrono::milliseconds>
ThriftServerConfig::getBaselineIdleTimeout() const {
  return timeout_.getBaseline();
}

size_t ThriftServerConfig::getNumIOWorkerThreads() const {
  return nWorkers_.get();
}

std::optional<size_t> ThriftServerConfig::getBaselineNumIOWorkerThreads()
    const {
  return nWorkers_.getBaseline();
}

size_t ThriftServerConfig::getNumCPUWorkerThreads() const {
  auto nCPUWorkers = nPoolThreads_.get();
  return nCPUWorkers ? nCPUWorkers : T_ASYNC_DEFAULT_WORKER_THREADS;
}

std::optional<size_t> ThriftServerConfig::getBaselineNumCPUWorkerThreads()
    const {
  return nPoolThreads_.getBaseline();
}

const folly::sorted_vector_set<std::string>&
ThriftServerConfig::getMethodsBypassMaxRequestsLimit() const {
  return methodsBypassMaxRequestsLimit_.get();
}

uint32_t ThriftServerConfig::getMaxNumPendingConnectionsPerWorker() const {
  return maxNumPendingConnectionsPerWorker_.get();
}

std::optional<uint32_t>
ThriftServerConfig::getBaselineMaxNumPendingConnectionsPerWorker() const {
  return maxNumPendingConnectionsPerWorker_.getBaseline();
}

uint64_t ThriftServerConfig::getMaxDebugPayloadMemoryPerRequest() const {
  return maxDebugPayloadMemoryPerRequest_.get();
}

uint64_t ThriftServerConfig::getMaxDebugPayloadMemoryPerWorker() const {
  return maxDebugPayloadMemoryPerWorker_.get();
}

uint16_t ThriftServerConfig::getMaxFinishedDebugPayloadsPerWorker() const {
  return maxFinishedDebugPayloadsPerWorker_.get();
}

std::chrono::milliseconds ThriftServerConfig::getConnectionAgeTimeout() const {
  return connectionAgeTimeout_.get();
}

const ServerAttributeDynamic<uint32_t>& ThriftServerConfig::getMaxConnections()
    const {
  return maxConnections_;
}

const std::optional<uint32_t> ThriftServerConfig::getBaselineMaxConnections()
    const {
  return maxConnections_.getBaseline();
}

const ServerAttributeDynamic<uint32_t>& ThriftServerConfig::getMaxRequests()
    const {
  return maxRequests_;
}

const std::optional<uint32_t> ThriftServerConfig::getBaselineMaxRequests()
    const {
  return maxRequests_.getBaseline();
}

const ServerAttributeDynamic<uint64_t>& ThriftServerConfig::getMaxResponseSize()
    const {
  return maxResponseSize_;
}

const std::optional<uint64_t> ThriftServerConfig::getBaselineMaxResponseSize()
    const {
  return maxResponseSize_.getBaseline();
}

const ServerAttributeDynamic<uint32_t>& ThriftServerConfig::getMaxQps() const {
  return maxQps_;
}

const ServerAttributeDynamic<bool>& ThriftServerConfig::getUseClientTimeout()
    const {
  return useClientTimeout_;
}

const std::optional<bool> ThriftServerConfig::getBaselineUseClientTimeout()
    const {
  return useClientTimeout_.getBaseline();
}

const ServerAttributeDynamic<std::chrono::milliseconds>&
ThriftServerConfig::getWriteBatchingInterval() const {
  return writeBatchingInterval_;
}

const ServerAttributeDynamic<size_t>& ThriftServerConfig::getWriteBatchingSize()
    const {
  return writeBatchingSize_;
}

const ServerAttributeDynamic<size_t>&
ThriftServerConfig::getWriteBatchingByteSize() const {
  return writeBatchingByteSize_;
}

const ServerAttributeDynamic<bool>& ThriftServerConfig::getEnableCodel() const {
  return enableCodel_;
}

const ServerAttributeDynamic<std::chrono::milliseconds>&
ThriftServerConfig::getTaskExpireTime() const {
  return taskExpireTime_;
}

const std::optional<std::chrono::milliseconds>
ThriftServerConfig::getBaselineTaskExpireTime() const {
  return taskExpireTime_.getBaseline();
}

const ServerAttributeDynamic<std::chrono::milliseconds>&
ThriftServerConfig::getStreamExpireTime() const {
  return streamExpireTime_;
}

const std::optional<std::chrono::milliseconds>
ThriftServerConfig::getBaselineStreamExpireTime() const {
  return streamExpireTime_.getBaseline();
}

const ServerAttributeDynamic<std::chrono::milliseconds>&
ThriftServerConfig::getQueueTimeout() const {
  return queueTimeout_;
}

const std::optional<std::chrono::milliseconds>
ThriftServerConfig::getBaselineQueueTimeout() const {
  return queueTimeout_.getBaseline();
}

const ServerAttributeDynamic<std::chrono::nanoseconds>&
ThriftServerConfig::getSocketQueueTimeout() const {
  return socketQueueTimeout_;
}

const std::optional<std::chrono::nanoseconds>
ThriftServerConfig::getBaselineSocketQueueTimeout() const {
  return socketQueueTimeout_.getBaseline();
}

const ServerAttributeDynamic<std::chrono::milliseconds>&
ThriftServerConfig::getSocketWriteTimeout() const {
  return socketWriteTimeout_;
}

const ServerAttributeDynamic<size_t>&
ThriftServerConfig::getIngressMemoryLimit() const {
  return ingressMemoryLimit_;
}

const ServerAttributeDynamic<size_t>& ThriftServerConfig::getEgressMemoryLimit()
    const {
  return egressMemoryLimit_;
}

const ServerAttributeDynamic<size_t>&
ThriftServerConfig::getMinPayloadSizeToEnforceIngressMemoryLimit() const {
  return minPayloadSizeToEnforceIngressMemoryLimit_;
}

const ServerAttributeDynamic<size_t>&
ThriftServerConfig::getEgressBufferBackpressureThreshold() const {
  return egressBufferBackpressureThreshold_;
}

const ServerAttributeDynamic<double>&
ThriftServerConfig::getEgressBufferRecoveryFactor() const {
  return egressBufferRecoveryFactor_;
}

const ServerAttributeDynamic<std::chrono::milliseconds>&
ThriftServerConfig::getPolledServiceHealthLiveness() const {
  return polledServiceHealthLiveness_;
}

const ServerAttributeDynamic<folly::SocketOptionMap>&
ThriftServerConfig::getPerConnectionSocketOptions() const {
  return perConnectionSocketOptions_;
}

void ThriftServerConfig::setCPUWorkerThreadName(
    const std::string& cpuWorkerThreadName, AttributeSource source) {
  setStaticAttribute(poolThreadName_, std::string{cpuWorkerThreadName}, source);
}

void ThriftServerConfig::resetCPUWorkerThreadName(AttributeSource source) {
  resetStaticAttribute(poolThreadName_, source);
}

void ThriftServerConfig::setWorkersJoinTimeout(
    std::chrono::seconds timeout, AttributeSource source) {
  setStaticAttribute(workersJoinTimeout_, std::move(timeout), source);
}

void ThriftServerConfig::resetWorkersJoinTimeout(AttributeSource source) {
  resetStaticAttribute(workersJoinTimeout_, source);
}

void ThriftServerConfig::setMaxNumPendingConnectionsPerWorker(
    uint32_t num, AttributeSource source) {
  setStaticAttribute(
      maxNumPendingConnectionsPerWorker_, std::move(num), source);
}

void ThriftServerConfig::resetMaxNumPendingConnectionsPerWorker(
    AttributeSource source) {
  resetStaticAttribute(maxNumPendingConnectionsPerWorker_, source);
}

void ThriftServerConfig::setIdleTimeout(
    std::chrono::milliseconds timeout, AttributeSource source) {
  setStaticAttribute(timeout_, std::move(timeout), source);
}

void ThriftServerConfig::resetIdleTimeout(AttributeSource source) {
  resetStaticAttribute(timeout_, source);
}

void ThriftServerConfig::setNumIOWorkerThreads(
    size_t numIOWorkerThreads, AttributeSource source) {
  setStaticAttribute(nWorkers_, std::move(numIOWorkerThreads), source);
}

void ThriftServerConfig::resetNumIOWorkerThreads(AttributeSource source) {
  resetStaticAttribute(nWorkers_, source);
}

void ThriftServerConfig::setNumCPUWorkerThreads(
    size_t numCPUWorkerThreads, AttributeSource source) {
  setStaticAttribute(nPoolThreads_, std::move(numCPUWorkerThreads), source);
}

void ThriftServerConfig::resetNumCPUWorkerThreads(AttributeSource source) {
  resetStaticAttribute(nPoolThreads_, source);
}

void ThriftServerConfig::setListenBacklog(
    int listenBacklog, AttributeSource source) {
  setStaticAttribute(listenBacklog_, std::move(listenBacklog), source);
}

void ThriftServerConfig::resetListenBacklog(AttributeSource source) {
  resetStaticAttribute(listenBacklog_, source);
}

void ThriftServerConfig::setMethodsBypassMaxRequestsLimit(
    const std::vector<std::string>& methods, AttributeSource source) {
  setStaticAttribute(
      methodsBypassMaxRequestsLimit_,
      folly::sorted_vector_set<std::string>{methods.begin(), methods.end()},
      source);
}

void ThriftServerConfig::resetMethodsBypassMaxRequestsLimit(
    AttributeSource source) {
  resetStaticAttribute(methodsBypassMaxRequestsLimit_, source);
}

void ThriftServerConfig::setMaxDebugPayloadMemoryPerRequest(
    uint64_t limit, AttributeSource source) {
  setStaticAttribute(
      maxDebugPayloadMemoryPerRequest_, std::move(limit), source);
}

void ThriftServerConfig::resetMaxDebugPayloadMemoryPerRequest(
    AttributeSource source) {
  resetStaticAttribute(maxDebugPayloadMemoryPerRequest_, source);
}

void ThriftServerConfig::setMaxDebugPayloadMemoryPerWorker(
    uint64_t limit, AttributeSource source) {
  setStaticAttribute(maxDebugPayloadMemoryPerWorker_, std::move(limit), source);
}

void ThriftServerConfig::resetMaxDebugPayloadMemoryPerWorker(
    AttributeSource source) {
  resetStaticAttribute(maxDebugPayloadMemoryPerWorker_, source);
}

void ThriftServerConfig::setMaxFinishedDebugPayloadsPerWorker(
    uint16_t limit, AttributeSource source) {
  setStaticAttribute(
      maxFinishedDebugPayloadsPerWorker_, std::move(limit), source);
}

void ThriftServerConfig::resetMaxFinishedDebugPayloadsPerWorker(
    AttributeSource source) {
  resetStaticAttribute(maxFinishedDebugPayloadsPerWorker_, source);
}

void ThriftServerConfig::setConnectionAgeTimeout(
    std::chrono::milliseconds timeout, AttributeSource source) {
  setStaticAttribute(connectionAgeTimeout_, std::move(timeout), source);
}

void ThriftServerConfig::resetConnectionAgeTimeout(AttributeSource source) {
  resetStaticAttribute(connectionAgeTimeout_, source);
}

void ThriftServerConfig::setMaxConnections(
    folly::observer::Observer<std::optional<uint32_t>> maxConnections,
    AttributeSource source) {
  maxConnections_.set(maxConnections, source);
}

void ThriftServerConfig::setMaxRequests(
    folly::observer::Observer<std::optional<uint32_t>> maxRequests,
    AttributeSource source) {
  maxRequests_.set(maxRequests, source);
}

void ThriftServerConfig::setMaxResponseSize(
    folly::observer::Observer<std::optional<uint64_t>> size,
    AttributeSource source) {
  maxResponseSize_.set(size, source);
}

void ThriftServerConfig::setMaxQps(
    folly::observer::Observer<std::optional<uint32_t>> maxQps,
    AttributeSource source) {
  maxQps_.set(maxQps, source);
}

void ThriftServerConfig::setUseClientTimeout(
    folly::observer::Observer<std::optional<bool>> useClientTimeout,
    AttributeSource source) {
  useClientTimeout_.set(useClientTimeout, source);
}

void ThriftServerConfig::setTaskExpireTime(
    folly::observer::Observer<std::optional<std::chrono::milliseconds>> timeout,
    AttributeSource source) {
  taskExpireTime_.set(timeout, source);
}

void ThriftServerConfig::setStreamExpireTime(
    folly::observer::Observer<std::optional<std::chrono::milliseconds>> timeout,
    AttributeSource source) {
  streamExpireTime_.set(timeout, source);
}

void ThriftServerConfig::setQueueTimeout(
    folly::observer::Observer<std::optional<std::chrono::milliseconds>> timeout,
    AttributeSource source) {
  queueTimeout_.set(timeout, source);
}

void ThriftServerConfig::setSocketQueueTimeout(
    folly::observer::Observer<std::optional<std::chrono::milliseconds>> timeout,
    AttributeSource source) {
  auto timeoutNs = folly::observer::makeObserver(
      [=]() -> std::optional<std::chrono::nanoseconds> {
        if (**timeout) {
          return std::chrono::milliseconds(***timeout);
        }
        return std::nullopt;
      });
  socketQueueTimeout_.set(timeoutNs, source);
}

void ThriftServerConfig::setSocketWriteTimeout(
    folly::observer::Observer<std::optional<std::chrono::milliseconds>> timeout,
    AttributeSource source) {
  socketWriteTimeout_.set(timeout, source);
}

void ThriftServerConfig::setWriteBatchingInterval(
    folly::observer::Observer<std::optional<std::chrono::milliseconds>>
        interval,
    AttributeSource source) {
  writeBatchingInterval_.set(interval, source);
}

void ThriftServerConfig::setWriteBatchingSize(
    folly::observer::Observer<std::optional<size_t>> batchingSize,
    AttributeSource source) {
  writeBatchingSize_.set(batchingSize, source);
}

void ThriftServerConfig::setWriteBatchingByteSize(
    folly::observer::Observer<std::optional<size_t>> batchingByteSize,
    AttributeSource source) {
  writeBatchingByteSize_.set(batchingByteSize, source);
}

void ThriftServerConfig::setIngressMemoryLimit(
    folly::observer::Observer<std::optional<size_t>> ingressMemoryLimit,
    AttributeSource source) {
  ingressMemoryLimit_.set(ingressMemoryLimit, source);
}

void ThriftServerConfig::setEgressMemoryLimit(
    folly::observer::Observer<std::optional<size_t>> max,
    AttributeSource source) {
  egressMemoryLimit_.set(max, source);
}

void ThriftServerConfig::setMinPayloadSizeToEnforceIngressMemoryLimit(
    folly::observer::Observer<std::optional<size_t>>
        minPayloadSizeToEnforceIngressMemoryLimit,
    AttributeSource source) {
  minPayloadSizeToEnforceIngressMemoryLimit_.set(
      minPayloadSizeToEnforceIngressMemoryLimit, source);
}

void ThriftServerConfig::setEgressBufferBackpressureThreshold(
    folly::observer::Observer<std::optional<size_t>> thresholdInBytes,
    AttributeSource source) {
  egressBufferBackpressureThreshold_.set(thresholdInBytes, source);
}

void ThriftServerConfig::setEgressBufferRecoveryFactor(
    folly::observer::Observer<std::optional<double>> recoveryFactor,
    AttributeSource source) {
  auto clampedRecoveryFactor =
      folly::observer::makeObserver([=]() -> std::optional<double> {
        if (**recoveryFactor) {
          return std::clamp(***recoveryFactor, 0.0, 1.0);
        }
        return std::nullopt;
      });
  egressBufferRecoveryFactor_.set(clampedRecoveryFactor, source);
}

void ThriftServerConfig::setPolledServiceHealthLiveness(
    folly::observer::Observer<std::optional<std::chrono::milliseconds>>
        liveness,
    AttributeSource source) {
  polledServiceHealthLiveness_.set(liveness, source);
}

void ThriftServerConfig::setPerConnectionSocketOptions(
    folly::observer::Observer<std::optional<folly::SocketOptionMap>> options,
    AttributeSource source) {
  perConnectionSocketOptions_.set(std::move(options), source);
}

} // namespace thrift
} // namespace apache
