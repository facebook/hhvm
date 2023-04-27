/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <folly/Range.h>
#include <folly/io/IOBuf.h>

namespace fizz {

/**
 * Interface for key exchange algorithms.
 */
class KeyExchange {
 public:
  virtual ~KeyExchange() = default;

  /**
   * Generates an ephemeral key pair.
   */
  virtual void generateKeyPair() = 0;

  /**
   * Returns the public key to share with peers.
   *
   * generateKeyPair() must be called before.
   */
  virtual std::unique_ptr<folly::IOBuf> getKeyShare() const = 0;

  /**
   * Generate a shared secret with our key pair and a peer's public key share.
   *
   * Performs all necessary validation of the public key share and throws on
   * error.
   *
   * generateKeyPair() must be called before.
   */
  virtual std::unique_ptr<folly::IOBuf> generateSharedSecret(
      folly::ByteRange keyShare) const = 0;

  /**
   * Clone this key exchange. A key exchange must only be cloned after
   * a key pair is set.
   */
  virtual std::unique_ptr<KeyExchange> clone() const = 0;

  /**
   * @return The size (in bytes) of the expected key share from the peer.
   */
  virtual std::size_t getExpectedKeyShareSize() const = 0;
};
} // namespace fizz
