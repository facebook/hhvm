/*
 *  Copyright (c) 2023-present, Meta, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <folly/Optional.h>
#include <folly/Range.h>
#include <folly/io/IOBuf.h>

namespace fizz::server {

/**
 * Interface for encrypting and decrypting various tokens, e.g., PSKs.
 */
class TokenCipher {
 public:
  virtual ~TokenCipher() = default;

  /**
   * Set secrets to use for token encryption/decryption.
   */
  virtual bool setSecrets(
      const std::vector<folly::ByteRange>& tokenSecrets) = 0;

  virtual folly::Optional<std::unique_ptr<folly::IOBuf>> encrypt(
      std::unique_ptr<folly::IOBuf>,
      folly::IOBuf* associatedData = nullptr) const = 0;

  virtual folly::Optional<std::unique_ptr<folly::IOBuf>> decrypt(
      std::unique_ptr<folly::IOBuf>,
      folly::IOBuf* associatedData = nullptr) const = 0;
};
} // namespace fizz::server
