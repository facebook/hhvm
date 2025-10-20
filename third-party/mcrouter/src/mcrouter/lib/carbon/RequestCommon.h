/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <bitset>
#include <optional>

#include <folly/IPAddress.h>
#include <folly/Random.h>
#include <folly/io/IOBuf.h>

#include "mcrouter/lib/carbon/MessageCommon.h"

namespace carbon {

class RequestCommon : public MessageCommon {
 public:
#ifndef LIBMC_FBTRACE_DISABLE
  RequestCommon();

  RequestCommon(const RequestCommon& other);
  RequestCommon& operator=(const RequestCommon& other);

  RequestCommon(RequestCommon&& other) noexcept;
  RequestCommon& operator=(RequestCommon&& other) noexcept;
#endif

  /**
   * Tells whether or not "serializedBuffer()" is dirty, in which case it can't
   * be used.
   */
  bool isBufferDirty() const;

  /**
   * Sets a buffer that can be used to avoid reserializing the request.
   * If the request is modified *after* this method is called, the buffer will
   * be marked as dirty and will not be used (i.e. the request will be
   * re-serialized).
   *
   * NOTE: The caller is responsible for keeping the buffer alive until the
   * reply is received.
   */
  void setSerializedBuffer(const folly::IOBuf& buffer);

  /**
   * Gets the buffer with this request serialized.
   * Will return nullptr if the buffer is dirty and can't be used.
   */
  const folly::IOBuf* serializedBuffer() const;

  // Store CAT token in an optional field.
  void setCryptoAuthToken(std::string&& token);

  // Store region string in an optional field.
  void setRegionFlag();

  /**
   * get the optional field that may store a CAT token
   * Used by mcrouter transport layer to pass the value to thrift header
   */
  const std::optional<std::string>& getCryptoAuthToken() const;

  // Check if region flag is set
  bool hasRegionFlag() const noexcept;

  const std::optional<std::string>& getClientIdentifier() const noexcept;

  void setClientIdentifier(folly::StringPiece clientIdentifier) noexcept;

  const std::optional<folly::IPAddress>& getSourceIpAddr() const noexcept;

  void setSourceIpAddr(const folly::IPAddress& sourceIpAddr) noexcept;

  void setWriteTimestampNs(uint64_t writeTimestamp) noexcept;

  std::optional<uint64_t> getWriteTimestampNs() const noexcept;

  uint64_t uniqueId() const {
    return uniqueId_;
  }

 protected:
  void markBufferAsDirty();

 private:
  const folly::IOBuf* serializedBuffer_{nullptr};
  // random unique identifier
  uint64_t uniqueId_{folly::Random::rand64()};
  // cat token(s) in string serialzed format
  std::optional<std::string> cryptoAuthToken_;
  // Hash string of primary (non-host) tls client identities
  std::optional<std::string> clientIdentifier_;
  // Source ip address.
  std::optional<folly::IPAddress> sourceIpAddr_;
  // write timestamp
  std::optional<uint64_t> writeTimestamp_;
  // Bit mask to indicate the metadata to fetch in the reply
  enum ReplyMetadataFlags : size_t { Region, NumFlags };
  std::bitset<NumFlags> replyBitMask_;
};

} // namespace carbon
