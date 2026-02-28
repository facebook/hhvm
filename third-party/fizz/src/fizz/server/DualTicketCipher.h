/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <fizz/server/TicketCipher.h>

namespace fizz {
namespace server {

/**
 * Base class for using two ciphers for decryption. The idea behind this
 * is that as we transition between psk contexts it might be useful to try
 * multiple ciphers so that we don't break resumption on rollout.
 */
class DualTicketCipher : public TicketCipher {
 public:
  DualTicketCipher(
      std::unique_ptr<TicketCipher> cipher,
      std::unique_ptr<TicketCipher> fallbackCipher)
      : cipher_(std::move(cipher)),
        fallbackCipher_(std::move(fallbackCipher)) {}

  folly::SemiFuture<folly::Optional<
      std::pair<std::unique_ptr<folly::IOBuf>, std::chrono::seconds>>>
  encrypt(ResumptionState resState) const override {
    return cipher_->encrypt(std::move(resState));
  }

  folly::SemiFuture<std::pair<PskType, folly::Optional<ResumptionState>>>
  decrypt(std::unique_ptr<folly::IOBuf> encryptedTicket) const override {
    auto bufClone = encryptedTicket->clone();
    return cipher_->decrypt(std::move(encryptedTicket))
        .deferValue([this, ticket = std::move(bufClone)](
                        std::pair<PskType, folly::Optional<ResumptionState>>
                            res) mutable {
          if (std::get<0>(res) == PskType::Rejected) {
            return fallbackCipher_->decrypt(std::move(ticket));
          }
          return folly::makeSemiFuture(std::move(res));
        });
  }

 private:
  std::unique_ptr<TicketCipher> cipher_;
  std::unique_ptr<TicketCipher> fallbackCipher_;
};
} // namespace server
} // namespace fizz
