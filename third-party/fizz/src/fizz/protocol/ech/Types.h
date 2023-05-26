/*
 *  Copyright (c) 2019-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <fizz/crypto/hpke/Types.h>
#include <fizz/record/Extensions.h>
#include <cstdint>
#include <vector>

namespace fizz {
namespace ech {

using Buf = std::unique_ptr<folly::IOBuf>;
using HpkePublicKey = Buf;

constexpr size_t kEchAcceptConfirmationSize = 8;

enum class ECHVersion : uint16_t {
  Draft15 = 0xfe0d,
};

struct HpkeSymmetricCipherSuite {
  hpke::KDFId kdf_id;
  hpke::AeadId aead_id;
  bool operator==(const HpkeSymmetricCipherSuite& other) const {
    return kdf_id == other.kdf_id && aead_id == other.aead_id;
  }
  bool operator!=(const HpkeSymmetricCipherSuite& other) const {
    return kdf_id != other.kdf_id || aead_id != other.aead_id;
  }
};

struct HpkeKeyConfig {
  uint8_t config_id;
  hpke::KEMId kem_id;
  HpkePublicKey public_key;
  std::vector<HpkeSymmetricCipherSuite> cipher_suites;

  HpkeKeyConfig() {}
  HpkeKeyConfig(const HpkeKeyConfig& other) {
    config_id = other.config_id;
    kem_id = other.kem_id;
    public_key = other.public_key->clone();
    cipher_suites = other.cipher_suites;
  }
  HpkeKeyConfig& operator=(const HpkeKeyConfig& other) {
    if (this != &other) {
      config_id = other.config_id;
      kem_id = other.kem_id;
      public_key = other.public_key->clone();
      cipher_suites = other.cipher_suites;
    }
    return *this;
  }
};

struct ECHConfigContentDraft {
  HpkeKeyConfig key_config;
  uint8_t maximum_name_length;
  Buf public_name;
  std::vector<Extension> extensions;

  ECHConfigContentDraft() {}
  ECHConfigContentDraft(const ECHConfigContentDraft& other) {
    key_config = other.key_config;
    maximum_name_length = other.maximum_name_length;
    public_name = other.public_name->clone();
    extensions = std::vector<Extension>();
    for (const auto& extension : other.extensions)
      extensions.push_back(extension.clone());
  }

  ECHConfigContentDraft& operator=(const ECHConfigContentDraft& other) {
    if (this != &other) {
      key_config = other.key_config;
      maximum_name_length = other.maximum_name_length;
      public_name = other.public_name->clone();
      extensions = std::vector<Extension>();
      for (const auto& extension : other.extensions)
        extensions.push_back(extension.clone());
    }
    return *this;
  }
};

struct ECHConfig {
  ECHVersion version;
  Buf ech_config_content;

  ECHConfig() {}
  ECHConfig(const ECHConfig& other) {
    version = other.version;
    ech_config_content = other.ech_config_content->clone();
  }
  ECHConfig(ECHConfig&& other) noexcept {
    version = other.version;
    ech_config_content = std::move(other.ech_config_content);
  }
  ECHConfig& operator=(const ECHConfig& other) {
    if (this != &other) {
      version = other.version;
      ech_config_content = other.ech_config_content->clone();
    }
    return *this;
  }
  ECHConfig& operator=(ECHConfig&& other) noexcept {
    if (this != &other) {
      version = other.version;
      ech_config_content = std::move(other.ech_config_content);
    }
    return *this;
  }
};

} // namespace ech
} // namespace fizz

#include <fizz/protocol/ech/Types-inl.h>
