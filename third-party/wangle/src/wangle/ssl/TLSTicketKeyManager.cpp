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

#include <wangle/ssl/TLSTicketKeyManager.h>

#include <folly/GLog.h>
#include <folly/Random.h>
#include <folly/String.h>
#include <folly/io/async/AsyncTimeout.h>
#include <folly/portability/OpenSSL.h>
#include <openssl/aes.h>
#include <wangle/ssl/SSLStats.h>
#include <wangle/ssl/SSLUtil.h>
#include <wangle/ssl/TLSTicketKeySeeds.h>

namespace {

// We use a "keyname" of 4 bytes with 12 bytes of salt to populate OpenSSL's
// keyname field (which is 16 bytes).
const int kTLSTicketKeyNameLen = 4;
const int kTLSTicketKeySaltLen = 12;

void saltKey(
    const unsigned char* baseKey,
    size_t keyLen,
    unsigned char* salt,
    unsigned char* output) {
  SHA256_CTX hash_ctx;

  SHA256_Init(&hash_ctx);
  SHA256_Update(&hash_ctx, baseKey, keyLen);
  SHA256_Update(&hash_ctx, salt, kTLSTicketKeySaltLen);
  SHA256_Final(output, &hash_ctx);
}

void populateRandom(unsigned char* field, uint32_t len) {
  CHECK_EQ(RAND_bytes(field, len), 1);
}
} // namespace

namespace wangle {

TLSTicketKeyManager::TLSTicketKey::TLSTicketKey(
    std::string seed,
    TLSTicketSeedType type)
    : seed_(std::move(seed)), type_(type) {
  SHA256((unsigned char*)seed_.data(), seed_.length(), keyValue_);
  name_ = computeName();
}

/**
 * OpenSSL requests a keyName to encode in the session ticket so that the server
 * can select the correct key for decryption when session resumption is
 * attempted. The only requirement is that the keyName be unique, so the
 * arbitrary constant 1 is purely for historical reasons. It could be removed,
 * but that would temporarily break session resumption for clients with tickets
 * issued by the existing scheme.
 *
 * Note that the returned name is 4 bytes, while the OpenSSL API allows for up
 * to 16 bytes of keyname. This is because the remaining 12 bytes are used to
 * store a salt for the encryption key.
 */
const std::string TLSTicketKeyManager::TLSTicketKey::computeName() const {
  unsigned char tmp[SHA256_DIGEST_LENGTH];
  const int32_t n = 1;

  SHA256_CTX ctx;
  SHA256_Init(&ctx);
  SHA256_Update(&ctx, keyValue_, TLSTicketKey::VALUE_LENGTH);
  SHA256_Update(&ctx, &n, sizeof(n));
  SHA256_Final(tmp, &ctx);
  return std::string((char*)tmp, kTLSTicketKeyNameLen);
}

std::unique_ptr<TLSTicketKeyManager> TLSTicketKeyManager::fromSeeds(
    const TLSTicketKeySeeds* seeds) {
  auto mgr = std::make_unique<TLSTicketKeyManager>();
  mgr->setTLSTicketKeySeeds(
      seeds->oldSeeds, seeds->currentSeeds, seeds->newSeeds);
  return mgr;
}

TLSTicketKeyManager::TLSTicketKeyManager() {}

TLSTicketKeyManager::~TLSTicketKeyManager() {}

int TLSTicketKeyManager::ticketCallback(
    SSL*,
    unsigned char* keyName,
    unsigned char* iv,
    EVP_CIPHER_CTX* cipherCtx,
    HMAC_CTX* hmacCtx,
    int encrypt) {
  int result = 0;

  if (encrypt) {
    result = encryptCallback(keyName, iv, cipherCtx, hmacCtx);
    // recordTLSTicket() below will unconditionally increment the new ticket
    // counter regardless of result value, so exit early here.
    if (result == 0) {
      return result;
    }
  } else {
    result = decryptCallback(keyName, iv, cipherCtx, hmacCtx);
  }

  // Result records whether a ticket key was found to encrypt or decrypt this
  // ticket, not whether the session was re-used.
  if (stats_) {
    stats_->recordTLSTicket(encrypt, result);
  }

  return result;
}

int TLSTicketKeyManager::encryptCallback(
    unsigned char* keyName,
    unsigned char* iv,
    EVP_CIPHER_CTX* cipherCtx,
    HMAC_CTX* hmacCtx) {
  auto key = findEncryptionKey();
  if (key == nullptr) {
    // no keys available to encrypt
    FB_LOG_EVERY_MS(WARNING, 1000)
        << "No TLS ticket key available for encryption. Either set a ticket "
        << "key or uninstall TLSTicketKeyManager from this SSLContext.";
    return 0;
  }
  VLOG(4) << "Encrypting new ticket with key name="
          << SSLUtil::hexlify(encryptionKeyName_);
  memcpy(keyName, encryptionKeyName_.data(), kTLSTicketKeyNameLen);

  uint8_t* salt = keyName + kTLSTicketKeyNameLen;
  populateRandom(salt, kTLSTicketKeySaltLen);

  // Create the unique keys by hashing with the salt. Take the first 16 bytes
  // of output as the hmac key, and the second 16 bytes as the aes key.
  uint8_t output[SHA256_DIGEST_LENGTH];
  saltKey(key->value(), TLSTicketKey::VALUE_LENGTH, salt, output);
  uint8_t* hmacKey = output;
  uint8_t* aesKey = output + SHA256_DIGEST_LENGTH / 2;

  // Initialize iv and cipher/mac CTX
  populateRandom(iv, AES_BLOCK_SIZE);
  HMAC_Init_ex(
      hmacCtx, hmacKey, SHA256_DIGEST_LENGTH / 2, EVP_sha256(), nullptr);
  EVP_EncryptInit_ex(cipherCtx, EVP_aes_128_cbc(), nullptr, aesKey, iv);

  return 1;
}

int TLSTicketKeyManager::decryptCallback(
    unsigned char* keyName,
    unsigned char* iv,
    EVP_CIPHER_CTX* cipherCtx,
    HMAC_CTX* hmacCtx) {
  std::string name((char*)keyName, kTLSTicketKeyNameLen);
  auto key = findDecryptionKey(name);
  if (key == nullptr) {
    // no ticket found for decryption - will issue a new ticket
    VLOG(4) << "Can't find ticket key with name=" << SSLUtil::hexlify(name)
            << ", will generate new ticket";
    return 0;
  }
  VLOG(4) << "Decrypting ticket with key name=" << SSLUtil::hexlify(name);

  // Reconstruct the unique key via the salt
  uint8_t* saltptr = keyName + kTLSTicketKeyNameLen;
  uint8_t output[SHA256_DIGEST_LENGTH];
  saltKey(key->value(), TLSTicketKey::VALUE_LENGTH, saltptr, output);
  uint8_t* hmacKey = output;
  uint8_t* aesKey = output + SHA256_DIGEST_LENGTH / 2;

  // Initialize cipher/mac CTX
  HMAC_Init_ex(
      hmacCtx, hmacKey, SHA256_DIGEST_LENGTH / 2, EVP_sha256(), nullptr);
  EVP_DecryptInit_ex(cipherCtx, EVP_aes_128_cbc(), nullptr, aesKey, iv);

  return 1;
}

bool TLSTicketKeyManager::insertSeed(
    const std::string& seedInput,
    TLSTicketSeedType type) {
  std::string seedOutput;

  if (!folly::unhexlify<std::string, std::string>(seedInput, seedOutput)) {
    LOG(WARNING) << "Failed to decode seed type= " << (uint32_t)type;
    return false;
  }

  auto ticketKey = std::make_unique<TLSTicketKey>(std::move(seedOutput), type);
  auto name = ticketKey->name();
  ticketKeyMap_[name] = std::move(ticketKey);

  if (type == TLSTicketSeedType::SEED_CURRENT) {
    encryptionKeyName_ = name;
  }
  return true;
}

bool TLSTicketKeyManager::setTLSTicketKeySeeds(
    const std::vector<std::string>& oldSeeds,
    const std::vector<std::string>& currentSeeds,
    const std::vector<std::string>& newSeeds) {
  recordTlsTicketRotation(oldSeeds, currentSeeds, newSeeds);

  encryptionKeyName_ = "";
  ticketKeyMap_.clear();
  bool success = true;
  for (auto& seed : oldSeeds) {
    success &= insertSeed(seed, TLSTicketSeedType::SEED_OLD);
  }
  for (auto& seed : currentSeeds) {
    success &= insertSeed(seed, TLSTicketSeedType::SEED_CURRENT);
  }
  for (auto& seed : newSeeds) {
    success &= insertSeed(seed, TLSTicketSeedType::SEED_NEW);
  }

  if (!success) {
    VLOG(2) << "One or more seeds failed to decode";
  }

  if (encryptionKeyName_.empty() || ticketKeyMap_.empty()) {
    VLOG(1) << "No keys configured, session ticket resumption disabled";
    return false;
  }

  return true;
}

bool TLSTicketKeyManager::getTLSTicketKeySeeds(
    std::vector<std::string>& oldSeeds,
    std::vector<std::string>& currentSeeds,
    std::vector<std::string>& newSeeds) const {
  oldSeeds.clear();
  currentSeeds.clear();
  newSeeds.clear();
  auto success = true;
  for (const auto& keyValue : ticketKeyMap_) {
    auto& ticketKey = *keyValue.second;

    std::string hexSeed;
    if (!folly::hexlify(ticketKey.seed(), hexSeed)) {
      success = false;
      continue;
    }

    switch (ticketKey.type()) {
      case TLSTicketSeedType::SEED_OLD:
        oldSeeds.push_back(hexSeed);
        break;
      case TLSTicketSeedType::SEED_CURRENT:
        currentSeeds.push_back(hexSeed);
        break;
      case TLSTicketSeedType::SEED_NEW:
        newSeeds.push_back(hexSeed);
    }
  }
  return success;
}

void TLSTicketKeyManager::recordTlsTicketRotation(
    const std::vector<std::string>& oldSeeds,
    const std::vector<std::string>& currentSeeds,
    const std::vector<std::string>& newSeeds) {
  if (stats_) {
    TLSTicketKeySeeds next{oldSeeds, currentSeeds, newSeeds};
    TLSTicketKeySeeds current;
    getTLSTicketKeySeeds(
        current.oldSeeds, current.currentSeeds, current.newSeeds);
    stats_->recordTLSTicketRotation(current.isValidRotation(next));
  }
}

TLSTicketKeyManager::TLSTicketKey* TLSTicketKeyManager::findEncryptionKey() {
  return findDecryptionKey(encryptionKeyName_);
}

TLSTicketKeyManager::TLSTicketKey* TLSTicketKeyManager::findDecryptionKey(
    const std::string& keyName) {
  auto iter = ticketKeyMap_.find(keyName);
  if (iter == ticketKeyMap_.end()) {
    return nullptr;
  }
  return iter->second.get();
}
} // namespace wangle
