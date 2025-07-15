/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <bitset>
#include <optional>
#include <utility>

#include "mcrouter/lib/carbon/RequestCommon.h"

namespace carbon {

#ifndef LIBMC_FBTRACE_DISABLE
RequestCommon::RequestCommon() = default;

RequestCommon::RequestCommon(const RequestCommon& other) {
  traceContext_ = other.traceContext_;
  cryptoAuthToken_ = other.cryptoAuthToken_;
  replyBitMask_ = other.replyBitMask_;
  clientIdentifier_ = other.clientIdentifier_;
}

RequestCommon& RequestCommon::operator=(const RequestCommon& other) {
  if (this != &other) {
    traceContext_ = other.traceContext_;
    cryptoAuthToken_ = other.cryptoAuthToken_;
    replyBitMask_ = other.replyBitMask_;
    clientIdentifier_ = other.clientIdentifier_;
  }
  return *this;
}

RequestCommon::RequestCommon(RequestCommon&&) noexcept = default;
RequestCommon& RequestCommon::operator=(RequestCommon&&) noexcept = default;
#endif

bool RequestCommon::isBufferDirty() const {
  return serializedBuffer_ == nullptr;
}

void RequestCommon::setSerializedBuffer(const folly::IOBuf& buffer) {
  if (buffer.empty()) {
    serializedBuffer_ = nullptr;
  } else {
    serializedBuffer_ = &buffer;
  }
}

const folly::IOBuf* RequestCommon::serializedBuffer() const {
  return serializedBuffer_;
}

void RequestCommon::setCryptoAuthToken(std::string&& token) {
  cryptoAuthToken_.emplace(std::move(token));
}

void RequestCommon::setRegionFlag() {
  replyBitMask_.set(ReplyMetadataFlags::Region);
}

const std::optional<std::string>& RequestCommon::getCryptoAuthToken() const {
  return cryptoAuthToken_;
}

bool RequestCommon::hasRegionFlag() const noexcept {
  return replyBitMask_.test(ReplyMetadataFlags::Region);
}

const std::optional<std::string>& RequestCommon::getClientIdentifier()
    const noexcept {
  return clientIdentifier_;
}

void RequestCommon::setClientIdentifier(
    folly::StringPiece clientIdentifier) noexcept {
  clientIdentifier_ = clientIdentifier.str();
}

const std::optional<folly::IPAddress>& RequestCommon::getSourceIpAddr()
    const noexcept {
  return sourceIpAddr_;
}

void RequestCommon::setSourceIpAddr(
    const folly::IPAddress& sourceIpAddr) noexcept {
  sourceIpAddr_ = sourceIpAddr;
}

void RequestCommon::setWriteTimestampNs(uint64_t writeTimestamp) noexcept {
  writeTimestamp_ = writeTimestamp;
}

std::optional<uint64_t> RequestCommon::getWriteTimestampNs() const noexcept {
  return writeTimestamp_;
}

void RequestCommon::markBufferAsDirty() {
  serializedBuffer_ = nullptr;
}

} // namespace carbon
