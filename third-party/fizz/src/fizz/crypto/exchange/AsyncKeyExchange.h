/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <fizz/crypto/exchange/KeyExchange.h>
#include <folly/futures/Future.h>

namespace fizz {
/**
 * For computationally expensive key exchange algorithms, this interface makes
 * it possible to offload the work to be processed asynchronously.
 * Note: Current design is only for server. We need make each individual API
 * async if we want to adapt it to client.
 */
class AsyncKeyExchange : public KeyExchange {
 public:
  struct DoKexResult {
   public:
    std::unique_ptr<folly::IOBuf> sharedSecret;
    std::unique_ptr<folly::IOBuf> ourKeyShare;
  };
  ~AsyncKeyExchange() override = default;
  /**
   * @param peerKeyShare : the public key sent by the peer
   * @return shared secret and key share to send to the peer
   */
  virtual folly::SemiFuture<DoKexResult> doAsyncKexFuture(
      std::unique_ptr<folly::IOBuf> peerKeyShare) = 0;
};
} // namespace fizz
