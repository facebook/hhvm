/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <fizz/server/AeadTokenCipher.h>
#include <fizz/server/FizzServerContext.h>
#include <fizz/server/TicketCipher.h>
#include <fizz/server/TicketPolicy.h>

namespace fizz {
namespace server {

template <typename CodecType, typename TokenCipherType>
class TicketCipherImpl : public TicketCipher {
 public:
  /**
   * Constructs a ticket cipher that encrypts session data with AES128-GCM.
   *
   * `certManager` is used by the ticket cipher in order to include a serialized
   *  representation of our certificate in the session data.
   *
   * `factory` is used to construct the serializer. It is important that the
   *  same factory configuration is used among Fizz server instances so that
   *  each Fizz instance is able to deserialize another instance's serialized
   *  session data.
   *
   * `pskContext` is an opaque string used as part of the key derivation so
   *  that different application contexts will result in different keys,
   *  preventing keys from one context from being used for another.
   */
  explicit TicketCipherImpl(
      std::shared_ptr<Factory> factory,
      std::shared_ptr<CertManager> certManager,
      std::string pskContext)
      : tokenCipher_(std::vector<std::string>(
            {CodecType::Label.toString(), pskContext})),
        policy_(),
        factory_(std::move(factory)),
        certManager_(std::move(certManager)) {}

  TicketCipherImpl(
      std::shared_ptr<Factory> factory,
      std::shared_ptr<CertManager> certManager)
      : tokenCipher_(std::vector<std::string>({CodecType::Label.toString()})),
        policy_(),
        factory_(std::move(factory)),
        certManager_(std::move(certManager)) {}

  /**
   * Set ticket secrets to use for ticket encryption/decryption.
   * The first one will be used for encryption.
   * All secrets must be at least kMinTicketSecretLength long.
   */
  bool setTicketSecrets(const std::vector<folly::ByteRange>& ticketSecrets) {
    return tokenCipher_.setSecrets(ticketSecrets);
  }

  /*
   * The ticket policy determines when tickets get rejected (even if they can be
   * encrypted/decrypted), for example if too much time has elapsed since the
   * full handshake that originally authenticated the server and/or client for
   * the session.
   */
  void setPolicy(TicketPolicy policy) {
    policy_ = std::move(policy);
  }

  folly::SemiFuture<folly::Optional<std::pair<Buf, std::chrono::seconds>>>
  encrypt(ResumptionState resState) const override {
    auto validity = policy_.remainingValidity(resState);
    if (validity <= std::chrono::system_clock::duration::zero()) {
      return folly::none;
    }

    auto encoded = CodecType::encode(std::move(resState));
    auto ticket = tokenCipher_.encrypt(std::move(encoded));
    if (!ticket) {
      return folly::none;
    }
    return std::make_pair(std::move(*ticket), validity);
  }

  folly::SemiFuture<std::pair<PskType, folly::Optional<ResumptionState>>>
  decrypt(std::unique_ptr<folly::IOBuf> encryptedTicket) const override {
    auto plaintext = tokenCipher_.decrypt(std::move(encryptedTicket));
    if (!plaintext) {
      return std::make_pair(PskType::Rejected, folly::none);
    }

    ResumptionState resState;
    try {
      resState =
          CodecType::decode(std::move(*plaintext), *factory_, *certManager_);
    } catch (const std::exception& ex) {
      VLOG(6) << "Failed to decode ticket, ex=" << ex.what();
      return std::make_pair(PskType::Rejected, folly::none);
    }

    if (!policy_.shouldAccept(resState)) {
      VLOG(6) << "Ticket failed acceptance policy.";
      return std::make_pair(PskType::Rejected, folly::none);
    }

    return std::make_pair(PskType::Resumption, std::move(resState));
  }

 private:
  TokenCipherType tokenCipher_;
  TicketPolicy policy_;

  std::shared_ptr<Factory> factory_;
  std::shared_ptr<CertManager> certManager_;
};

template <typename CodecType>
using Aead128GCMTicketCipher =
    TicketCipherImpl<CodecType, Aead128GCMTokenCipher>;

} // namespace server
} // namespace fizz
