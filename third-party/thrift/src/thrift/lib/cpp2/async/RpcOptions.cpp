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

#include <thrift/lib/cpp2/async/RpcOptions.h>

#include <thrift/lib/cpp2/async/Interaction.h>

namespace apache::thrift {
namespace {
const transport::THeader::StringToStringMap& kEmptyMap() {
  static const transport::THeader::StringToStringMap& map =
      *(new transport::THeader::StringToStringMap);
  return map;
}

uint32_t validateTimeout(std::chrono::milliseconds timeout) {
  using rep = std::chrono::milliseconds::rep;
  static constexpr rep min = 0;
  static constexpr rep max = std::numeric_limits<uint32_t>::max();
  const auto ms = timeout.count();
  DCHECK_GE(ms, min) << "Timeout should be >= 0";
  DCHECK_LE(ms, max) << "Timeout should not exceed " << max << " ms";
  return std::max(min, std::min(ms, max));
}
} // namespace

RpcOptions& RpcOptions::setTimeout(std::chrono::milliseconds timeout) {
  timeout_ = validateTimeout(timeout);
  return *this;
}

std::chrono::milliseconds RpcOptions::getTimeout() const {
  return std::chrono::milliseconds(timeout_);
}

RpcOptions& RpcOptions::setPriority(RpcOptions::PRIORITY priority) {
  priority_ = static_cast<uint8_t>(priority);
  return *this;
}

RpcOptions::PRIORITY RpcOptions::getPriority() const {
  return static_cast<RpcOptions::PRIORITY>(priority_);
}

RpcOptions& RpcOptions::setClientOnlyTimeouts(bool val) {
  clientOnlyTimeouts_ = val;
  return *this;
}

bool RpcOptions::getClientOnlyTimeouts() const {
  return clientOnlyTimeouts_;
}

RpcOptions& RpcOptions::setChunkTimeout(
    std::chrono::milliseconds chunkTimeout) {
  chunkTimeout_ = validateTimeout(chunkTimeout);
  return *this;
}

std::chrono::milliseconds RpcOptions::getChunkTimeout() const {
  return std::chrono::milliseconds(chunkTimeout_);
}

RpcOptions& RpcOptions::setChunkBufferSize(int32_t chunkBufferSize) {
  CHECK_EQ(bufferOptions_.memSize, 0)
      << "Only one of setMemoryBufferSize and setChunkBufferSize should be called";
  bufferOptions_.chunkSize = chunkBufferSize;
  return *this;
}

RpcOptions& RpcOptions::setMemoryBufferSize(
    size_t targetBytes, int32_t initialChunks, int32_t maxChunks) {
  CHECK_EQ(bufferOptions_.chunkSize, 100)
      << "Only one of setMemoryBufferSize and setChunkBufferSize should be called";
  CHECK_GT(targetBytes, 0);
  CHECK_LE(0, initialChunks);
  CHECK_LE(initialChunks, maxChunks);
  bufferOptions_.memSize = targetBytes;
  bufferOptions_.chunkSize = initialChunks;
  bufferOptions_.maxChunkSize = maxChunks;
  return *this;
}

int32_t RpcOptions::getChunkBufferSize() const {
  return bufferOptions_.chunkSize;
}

const BufferOptions& RpcOptions::getBufferOptions() const {
  return bufferOptions_;
}

RpcOptions& RpcOptions::setQueueTimeout(
    std::chrono::milliseconds queueTimeout) {
  queueTimeout_ = validateTimeout(queueTimeout);
  return *this;
}

std::chrono::milliseconds RpcOptions::getQueueTimeout() const {
  return std::chrono::milliseconds(queueTimeout_);
}

RpcOptions& RpcOptions::setOverallTimeout(
    std::chrono::milliseconds overallTimeout) {
  overallTimeout_ = validateTimeout(overallTimeout);
  return *this;
}

std::chrono::milliseconds RpcOptions::getOverallTimeout() const {
  return std::chrono::milliseconds(overallTimeout_);
}

RpcOptions& RpcOptions::setProcessingTimeout(
    std::chrono::milliseconds processingTimeout) {
  processingTimeout_ = validateTimeout(processingTimeout);
  return *this;
}

std::chrono::milliseconds RpcOptions::getProcessingTimeout() const {
  return std::chrono::milliseconds(processingTimeout_);
}

RpcOptions& RpcOptions::setRoutingKey(std::string routingKey) {
  routingKey_ = std::move(routingKey);
  return *this;
}

const std::string& RpcOptions::getRoutingKey() const {
  return routingKey_;
}

RpcOptions& RpcOptions::setShardId(std::string shardId) {
  shardId_ = std::move(shardId);
  return *this;
}

const std::string& RpcOptions::getShardId() const {
  return shardId_;
}

RpcOptions& RpcOptions::setOperationMask(int32_t operationMask) {
  operationMask_ = operationMask;
  return *this;
}

const int32_t& RpcOptions::getOperationMask() const {
  return operationMask_;
}

void RpcOptions::setReadHeaders(
    transport::THeader::StringToStringMap&& readHeaders) {
  readHeaders_ = std::move(readHeaders);
}

const transport::THeader::StringToStringMap& RpcOptions::getReadHeaders()
    const {
  return readHeaders_ ? *readHeaders_ : kEmptyMap();
}

void RpcOptions::setWriteHeader(std::string_view key, std::string value) {
  if (!writeHeaders_) {
    writeHeaders_.emplace();
  }
  (*writeHeaders_)[key] = std::move(value);
}

const transport::THeader::StringToStringMap& RpcOptions::getWriteHeaders()
    const {
  return writeHeaders_ ? *writeHeaders_ : kEmptyMap();
}

transport::THeader::StringToStringMap RpcOptions::releaseWriteHeaders() {
  return std::exchange(writeHeaders_, std::nullopt).value_or(kEmptyMap());
}

RpcOptions& RpcOptions::setInteractionId(const InteractionId& id) {
  interactionId_ = id;
  DCHECK_GT(interactionId_, 0);
  return *this;
}

int64_t RpcOptions::getInteractionId() const {
  return interactionId_;
}

RpcOptions& RpcOptions::copyInteractionIdFrom(const RpcOptions& other) {
  interactionId_ = other.getInteractionId();
  return *this;
}

RpcOptions& RpcOptions::setLoggingContext(std::string loggingContext) {
  loggingContext_ = std::move(loggingContext);
  return *this;
}

const std::string& RpcOptions::getLoggingContext() const {
  return loggingContext_;
}

RpcOptions& RpcOptions::setRoutingData(std::shared_ptr<void> data) {
  routingData_ = std::move(data);
  return *this;
}

const std::shared_ptr<void>& RpcOptions::getRoutingData() const {
  return routingData_;
}

RpcOptions& RpcOptions::setContextPropMask(uint8_t mask) {
  contextPropComponentEnabledMask_ = mask;
  return *this;
}

uint8_t RpcOptions::getContextPropMask() const {
  return contextPropComponentEnabledMask_;
}

RpcOptions& RpcOptions::setCallerContext(std::shared_ptr<void> callerContext) {
  callerContext_ = std::move(callerContext);
  return *this;
}

const std::shared_ptr<void>& RpcOptions::getCallerContext() const {
  return callerContext_;
}

RpcOptions& RpcOptions::setSerializedAuthProofs(
    SerializedAuthProofs serializedAuthProofs) {
  serializedAuthProofs_ = std::move(serializedAuthProofs);
  return *this;
}

const SerializedAuthProofs& RpcOptions::getSerializedAuthProofs() const {
  return serializedAuthProofs_;
}

RpcOptions& RpcOptions::setDefconPriority(DefconPriority defconPriority) {
  defconPriority_ = defconPriority;
  return *this;
}

const std::optional<RpcOptions::DefconPriority>& RpcOptions::getDefconPriority()
    const {
  return defconPriority_;
}

RpcOptions& RpcOptions::setFdsToSend(folly::SocketFds::ToSend fdsToSend) {
  fdsToSend_ = std::move(fdsToSend);
  return *this;
}

folly::SocketFds RpcOptions::copySocketFdsToSend() const {
  if (LIKELY(fdsToSend_.empty())) {
    return folly::SocketFds{};
  }
  return folly::SocketFds{fdsToSend_};
}

RpcOptions& RpcOptions::setConnectionKey(std::string key) {
  connectionKey_ = std::move(key);
  return *this;
}

std::string_view RpcOptions::getConnectionKey() const {
  return connectionKey_;
}

RpcOptions& RpcOptions::setChecksum(RpcOptions::Checksum checksum) {
  checksum_ = checksum;
  return *this;
}

RpcOptions::Checksum RpcOptions::getChecksum() const {
  return checksum_;
}

RpcOptions& RpcOptions::setForceSyncOnFiber(bool forceSyncOnFiber) {
  forceSyncOnFiber_ = forceSyncOnFiber;
  return *this;
}

bool RpcOptions::getForceSyncOnFiber() const {
  return forceSyncOnFiber_;
}

RpcOptions& RpcOptions::setMetricsToCollect(
    std::shared_ptr<void> metricsToCollect) {
  metricsToCollect_ = std::move(metricsToCollect);
  return *this;
}

const std::shared_ptr<void>& RpcOptions::getMetricsToCollect() const {
  return metricsToCollect_;
}

RpcOptions& RpcOptions::setRoutingObjectiveKey(
    std::string routingObjectiveKey) {
  routingObjectiveKey_ = std::move(routingObjectiveKey);
  return *this;
}

const std::string& RpcOptions::getRoutingObjectiveKey() const {
  return routingObjectiveKey_;
}

RpcOptions& RpcOptions::setFrameRelativeDataAlignment(uint32_t alignment) {
  frameRelativeDataAlignmentBytes_ = alignment;
  return *this;
}

uint32_t RpcOptions::getFrameRelativeDataAlignment() const {
  return frameRelativeDataAlignmentBytes_;
}

} // namespace apache::thrift
