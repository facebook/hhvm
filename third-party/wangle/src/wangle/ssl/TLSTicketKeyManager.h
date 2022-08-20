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

#pragma once

#include <folly/io/async/EventBase.h>
#include <folly/io/async/SSLContext.h>
#include <folly/ssl/OpenSSLTicketHandler.h>

namespace wangle {

class SSLStats;
struct TLSTicketKeySeeds;

/**
 * The TLSTicketKeyManager handles TLS ticket encryption and decryption in a
 * way that facilitates sharing of ticket keys across a range of servers. This
 * implements the OpenSSLTicketHandler interface and is meant to be attached to
 * an SSLContext via setTicketHandler() and should therefore only be used in
 * one thread.
 *
 * Ticket seeds should be updated periodically (e.g., daily) via the
 * setTLSTicketKeySeeds() API.
 */
class TLSTicketKeyManager : public folly::OpenSSLTicketHandler {
 public:
  static std::unique_ptr<TLSTicketKeyManager> fromSeeds(
      const TLSTicketKeySeeds* seeds);

  TLSTicketKeyManager();

  virtual ~TLSTicketKeyManager();

  /**
   * Callback invoked by OpenSSL to prepare for ticket encryption/decryption.
   *
   * During encryption, keyName will be populated by a "real" keyname derived
   * from the encryption key, and a 12 byte, randomly generated salt (i.e.
   * {<4-byte-name> <12-byte-salt>}). During decryption, the keyName field is
   * interpreted similarly. The salt is combined with the ticket key to create
   * a unique key per ticket.
   *
   * For more details on how these fields are used, consult the OpenSSL
   * documentation for SSL_CTX_set_tlsext_ticket_key_cb.
   */
  int ticketCallback(
      SSL* ssl,
      unsigned char* keyName,
      unsigned char* iv,
      EVP_CIPHER_CTX* cipherCtx,
      HMAC_CTX* hmacCtx,
      int encrypt) override;

  /**
   * The manager is supplied with three lists of seeds (old, current, and new).
   * The current seed is used for encryption, while the old, current, and new
   * seeds are used for decryption. By loading a seed in as a new seed for a
   * while before promoting it to the current seed, and similarly leaving it
   * as an old seed for a while after it leaves the current seed slot, you can
   * minimize the number of session ticket decryption failures.
   *
   * If more than one current seed is provided, only the last one is used as an
   * encryption key. This interface should really be something like (string
   * encryption_key, vector<string> decryption_keys), but for now we have to
   * live with this API given the number of callsites that would need to be
   * migrated. All seed strings are expected to be in hexadecimal, otherwise
   * they will not be stored.
   *
   * @param oldSeeds Seeds previously used which can still decrypt.
   * @param currentSeeds Seeds to use for new ticket encryptions.
   * @param newSeeds Seeds which will be used soon, can be used to decrypt
   *                 in case some servers in the cluster have already rotated.
   */
  bool setTLSTicketKeySeeds(
      const std::vector<std::string>& oldSeeds,
      const std::vector<std::string>& currentSeeds,
      const std::vector<std::string>& newSeeds);

  /**
   * Used to retrieve the secrets previously set above. The order of seeds
   * within a vector is not guaranteed to be the same as the order in which
   * they were set.
   */
  bool getTLSTicketKeySeeds(
      std::vector<std::string>& oldSeeds,
      std::vector<std::string>& currentSeeds,
      std::vector<std::string>& newSeeds) const;

  /**
   * Stats object can record new tickets and ticket secret rotations.
   */
  void setStats(SSLStats* stats) {
    stats_ = stats;
  }

 private:
  TLSTicketKeyManager(const TLSTicketKeyManager&) = delete;
  TLSTicketKeyManager& operator=(const TLSTicketKeyManager&) = delete;

  int encryptCallback(
      unsigned char* keyName,
      unsigned char* iv,
      EVP_CIPHER_CTX* cipherCtx,
      HMAC_CTX* hmacCtx);

  int decryptCallback(
      unsigned char* keyName,
      unsigned char* iv,
      EVP_CIPHER_CTX* cipherCtx,
      HMAC_CTX* hmacCtx);

  /**
   * Because the ticket seed getter exposes the concept of old, current, and
   * new seeds, we need a way to tag the type of seeds in internal data
   * structures here...
   */
  enum TLSTicketSeedType { SEED_OLD = 0, SEED_CURRENT, SEED_NEW };

  class TLSTicketKey {
   public:
    explicit TLSTicketKey(std::string seed, TLSTicketSeedType type);

    // Following two methods needed to support existing
    // get/setTLSTickeKeyManager interface.
    const std::string& seed() const {
      return seed_;
    }

    TLSTicketSeedType type() const {
      return type_;
    }

    // Needed by OpenSSL API when encoding session tickets.
    const std::string name() const {
      return name_;
    }

    const unsigned char* value() const {
      return keyValue_;
    }

    static constexpr uint32_t VALUE_LENGTH = SHA256_DIGEST_LENGTH;

   private:
    const std::string computeName() const;

    // We need to store the seed and key type in addition to the keyValue
    // itself (which is derived from the seed), because getTLSTicketKeySeeds()
    // expects this information to be preserved from setTLSTicketKeySeeds().
    std::string seed_;
    TLSTicketSeedType type_;
    std::string name_;
    unsigned char keyValue_[SHA256_DIGEST_LENGTH];
  };

  bool insertSeed(const std::string& seedInput, TLSTicketSeedType type);

  /**
   * Locate the key for encrypting a new ticket
   */
  TLSTicketKey* findEncryptionKey();

  /**
   * Locate a key for decrypting a ticket with the given keyName
   */
  TLSTicketKey* findDecryptionKey(const std::string& keyName);

  /**
   * Record the rotation of the ticket seeds with a new set
   */
  void recordTlsTicketRotation(
      const std::vector<std::string>& oldSeeds,
      const std::vector<std::string>& currentSeeds,
      const std::vector<std::string>& newSeeds);

  std::string encryptionKeyName_;
  std::unordered_map<std::string, std::unique_ptr<TLSTicketKey>> ticketKeyMap_;
  SSLStats* stats_{nullptr};
};
using TicketSeedHandler = TLSTicketKeyManager;
} // namespace wangle
