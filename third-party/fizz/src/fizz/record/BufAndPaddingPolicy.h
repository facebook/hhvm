/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <fizz/record/Types.h>
#include <folly/io/IOBufQueue.h>
#include <algorithm>
#include <utility>

namespace fizz {

constexpr uint16_t kMinSuggestedRecordSize = 1500;

/**
 * Interface for EncryptedWriteRecordLayer padding policy
 */
class BufAndPaddingPolicy {
 public:
  virtual ~BufAndPaddingPolicy() = default;

  /**
   * Returns both the buf to encrypt and the padding to add to buf for
   * EncyrpteWriteRecordLayer's write method.
   */
  virtual std::pair<Buf, uint16_t> getBufAndPaddingToEncrypt(
      folly::IOBufQueue& queue,
      uint16_t maxRecord) const = 0;
};

/**
 * Uses modulo padding policy, where padding is added such that the remaining
 * space in a message after padding is a multiple of paddingModulo, effectively
 * hiding the actual message length to protect against traffic analysis.
 */
class BufAndModuloPaddingPolicy : public BufAndPaddingPolicy {
 public:
  ~BufAndModuloPaddingPolicy() override = default;

  explicit BufAndModuloPaddingPolicy(uint16_t paddingModulo)
      : paddingModulo_(paddingModulo) {}

  std::pair<Buf, uint16_t> getBufAndPaddingToEncrypt(
      folly::IOBufQueue& queue,
      uint16_t maxRecord) const override;

  /**
   * Sets the variable used to determine the ammount of padding to add.
   */
  void setPaddingModulo(uint16_t paddingModulo) {
    paddingModulo_ = paddingModulo;
  }

 private:
  uint16_t paddingModulo_;
};

/**
 * Adds a constant amount of padding (commonly 0)
 */
class BufAndConstPaddingPolicy : public BufAndPaddingPolicy {
 public:
  ~BufAndConstPaddingPolicy() override = default;

  explicit BufAndConstPaddingPolicy(uint16_t paddingSize)
      : paddingSize_(paddingSize) {}

  std::pair<Buf, uint16_t> getBufAndPaddingToEncrypt(
      folly::IOBufQueue& queue,
      uint16_t maxRecord) const override;

  /**
   * Sets the variable used to determine the ammount of padding to add.
   */
  void setPaddingSize(uint16_t paddingSize) {
    paddingSize_ = paddingSize;
  }

 private:
  uint16_t paddingSize_;
};
} // namespace fizz
