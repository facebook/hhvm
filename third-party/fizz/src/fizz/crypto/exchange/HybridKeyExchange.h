/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <fizz/crypto/exchange/KeyExchange.h>

namespace fizz {
/**
 * Hybrid key exchange. It calls the selected key exchange algorithm X and Y and
 * concatenates the results together. Note the order of X and Y matters: Y's
 * stuff is concatenated to X's stuff. This class is written in compliance with
 * draft-ietf-tls-hybrid-design-04.
 */
class HybridKeyExchange : public KeyExchange {
 public:
  HybridKeyExchange(
      std::unique_ptr<KeyExchange> first,
      std::unique_ptr<KeyExchange> second);

  ~HybridKeyExchange() override = default;

  void generateKeyPair() override;

  std::unique_ptr<folly::IOBuf> getKeyShare() const override;

  std::unique_ptr<folly::IOBuf> generateSharedSecret(
      folly::ByteRange keyShare) const override;

  std::unique_ptr<KeyExchange> clone() const override;

  std::size_t getExpectedKeyShareSize() const override;

 private:
  std::unique_ptr<KeyExchange> firstKex_;
  std::unique_ptr<KeyExchange> secondKex_;
};
} // namespace fizz
