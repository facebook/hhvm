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
#include <optional>
#include <string>

#include <thrift/lib/cpp/concurrency/Thread.h>
#include <thrift/lib/cpp/transport/THeader.h>
#include <thrift/lib/cpp2/async/ClientStreamBridge.h>

namespace apache::thrift {

struct SerializedAuthProofs {
  SerializedAuthProofs() = default;
  ~SerializedAuthProofs() = default;

  explicit SerializedAuthProofs(std::unique_ptr<folly::IOBuf> newBuf)
      : buf(std::move(newBuf)) {}

  SerializedAuthProofs(const SerializedAuthProofs& other) {
    buf = other.buf ? other.buf->clone() : nullptr;
  }
  SerializedAuthProofs& operator=(const SerializedAuthProofs& other) {
    buf = other.buf ? other.buf->clone() : nullptr;
    return *this;
  }

  SerializedAuthProofs(SerializedAuthProofs&&) = default;
  SerializedAuthProofs& operator=(SerializedAuthProofs&&) = default;

  std::unique_ptr<folly::IOBuf> buf{nullptr};
};

class InteractionId;

/**
 * RpcOptions class to set per-RPC options (such as request timeout).
 */
class RpcOptions {
 public:
  /**
   * Priority levels used to classify requests based on their impact on the end
   * user.
   *
   * Under certain circumstances, the routing layer may drop lower-priority
   * requests in order to quickly regain system stability.
   */
  enum class DefconPriority : uint8_t {
    LOW, // Low-priority traffic: small impact, nice-to-have features
    MID, // Mid-priority traffic: visible impact to the user
    HIGH, // High-priority traffic: significant impact to the user
    CRIT, // Critical infra traffic that should *never* be dropped
  };

  enum class Checksum : uint8_t {
    NONE = 0,
    CRC32 = 1,
    XXH3_64 = 2,
    SERVER_ONLY_CRC32 = 3,
  };

  using PRIORITY = apache::thrift::concurrency::PRIORITY;

  /**
   * NOTE: This only sets the receive timeout, and not the send timeout on
   * transport. Probably you want to use HeaderClientChannel::setTimeout()
   */
  RpcOptions& setTimeout(std::chrono::milliseconds timeout);
  std::chrono::milliseconds getTimeout() const;

  RpcOptions& setPriority(PRIORITY priority);
  PRIORITY getPriority() const;

  // Do timeouts apply only on the client side?
  RpcOptions& setClientOnlyTimeouts(bool val);
  bool getClientOnlyTimeouts() const;

  [[deprecated("Use getChecksum instead")]]
  bool getEnableChecksum() const;

  RpcOptions& setChunkTimeout(std::chrono::milliseconds chunkTimeout);
  std::chrono::milliseconds getChunkTimeout() const;

  // Only one of these may be set
  RpcOptions& setChunkBufferSize(int32_t chunkBufferSize);
  RpcOptions& setMemoryBufferSize(
      size_t targetBytes,
      int32_t initialChunks,
      int32_t maxChunks = std::numeric_limits<int32_t>::max());
  int32_t getChunkBufferSize() const;
  const BufferOptions& getBufferOptions() const;

  RpcOptions& setQueueTimeout(std::chrono::milliseconds queueTimeout);
  std::chrono::milliseconds getQueueTimeout() const;

  RpcOptions& setOverallTimeout(std::chrono::milliseconds overallTimeout);
  std::chrono::milliseconds getOverallTimeout() const;

  RpcOptions& setProcessingTimeout(std::chrono::milliseconds processingTimeout);
  std::chrono::milliseconds getProcessingTimeout() const;

  /**
   * Set routing key for (e.g. consistent hashing based) routing.
   *
   * @param routingKey routing key, e.g. consistent hashing seed
   * @return reference to this object
   */
  RpcOptions& setRoutingKey(std::string routingKey);
  const std::string& getRoutingKey() const;

  /**
   * Set shard ID for sending request to sharding-based services.
   *
   * @param shardId shard ID to use for service discovery
   * @return reference to this object
   */
  RpcOptions& setShardId(std::string shardId);
  const std::string& getShardId() const;

  RpcOptions& setOperationMask(int32_t operationMask);
  const int32_t& getOperationMask() const;

  void setWriteHeader(std::string_view key, std::string value);
  const transport::THeader::StringToStringMap& getWriteHeaders() const;
  transport::THeader::StringToStringMap releaseWriteHeaders();

  void setReadHeaders(transport::THeader::StringToStringMap&& readHeaders);
  const transport::THeader::StringToStringMap& getReadHeaders() const;

  // Primarily used by generated code
  RpcOptions& setInteractionId(const InteractionId& id);
  int64_t getInteractionId() const;
  RpcOptions& copyInteractionIdFrom(const RpcOptions& other);

  RpcOptions& setLoggingContext(std::string loggingContext);
  const std::string& getLoggingContext() const;

  RpcOptions& setRoutingData(std::shared_ptr<void> data);
  const std::shared_ptr<void>& getRoutingData() const;

  RpcOptions& setContextPropMask(uint8_t mask);
  uint8_t getContextPropMask() const;

  RpcOptions& setCallerContext(std::shared_ptr<void> callerContext);
  const std::shared_ptr<void>& getCallerContext() const;

  RpcOptions& setSerializedAuthProofs(
      SerializedAuthProofs serializedAuthProofs);
  const SerializedAuthProofs& getSerializedAuthProofs() const;

  RpcOptions& setDefconPriority(DefconPriority defconPriority);
  const std::optional<DefconPriority>& getDefconPriority() const;

  RpcOptions& setFdsToSend(folly::SocketFds::ToSend);
  folly::SocketFds copySocketFdsToSend() const;

  RpcOptions& setMetricsToCollect(std::shared_ptr<void> metricsToClient);
  const std::shared_ptr<void>& getMetricsToCollect() const;

  /**
   * In the routing layer, this key can be used to create or select specific
   * connections to a server.
   */
  RpcOptions& setConnectionKey(std::string key);
  std::string_view getConnectionKey() const;

  RpcOptions& setChecksum(Checksum checksum);
  Checksum getChecksum() const;

  RpcOptions& setForceSyncOnFiber(bool forceSyncOnFiber);
  bool getForceSyncOnFiber() const;

 private:
  using timeout_ms_t = uint32_t;
  timeout_ms_t timeout_{0};
  timeout_ms_t chunkTimeout_{0};
  timeout_ms_t queueTimeout_{0};
  timeout_ms_t overallTimeout_{0};
  timeout_ms_t processingTimeout_{0};
  uint8_t priority_{apache::thrift::concurrency::N_PRIORITIES};
  bool clientOnlyTimeouts_{false};
  BufferOptions bufferOptions_;
  int64_t interactionId_{0};
  int32_t operationMask_{0};
  uint8_t contextPropComponentEnabledMask_{0xff};

  std::string routingKey_;
  std::string shardId_;
  std::string connectionKey_;

  // For sending and receiving headers.
  std::optional<transport::THeader::StringToStringMap> writeHeaders_;
  std::optional<transport::THeader::StringToStringMap> readHeaders_;

  // Custom data about the request for logging and analysis.
  std::string loggingContext_;

  // Custom data passed back from the routing layer.
  std::shared_ptr<void> routingData_;

  std::shared_ptr<void> callerContext_;

  // Per request authentication data
  SerializedAuthProofs serializedAuthProofs_;

  // Classifies the current request based on its impact on the end user
  std::optional<DefconPriority> defconPriority_;

  folly::SocketFds::ToSend fdsToSend_;

  Checksum checksum_{Checksum::NONE};

  bool forceSyncOnFiber_{false};

  // Metrics to be sent back to the client
  std::shared_ptr<void> metricsToCollect_;
};

} // namespace apache::thrift
