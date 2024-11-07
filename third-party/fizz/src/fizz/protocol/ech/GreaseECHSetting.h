/*
 *  Copyright (c) Meta Platforms, Inc. and affiliates.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <fizz/crypto/hpke/Hpke.h>

namespace fizz {
namespace ech {

enum class PayloadGenerationStrategy : uint8_t {
  /**
   * Generates a payload with a size P where P is in the range
   * [minPayloadSize, maxPayloadSize].
   */
  UniformRandom = 0,

  /**
   * Generates a payload with size L + C + P bytes where L is the size of the
   * encoded inner client hello, C is the ciphertext expansion of the selected
   * AEAD schema, and P is the expected padding within the range
   * [minPayloadSize, maxPayloadSize].
   */
  Computed
};

struct GreaseECHSetting {
  uint8_t minConfigId{0};
  uint8_t maxConfigId{std::numeric_limits<uint8_t>::max()};
  PayloadGenerationStrategy payloadStrategy{
      PayloadGenerationStrategy::UniformRandom};
  size_t minPayloadSize{0};
  size_t maxPayloadSize{0};
  std::vector<uint16_t> keySizes{32, 48, 64};
  std::vector<hpke::KDFId> kdfs{
      hpke::KDFId::Sha256,
      hpke::KDFId::Sha384,
      hpke::KDFId::Sha512};
  std::vector<hpke::AeadId> aeads{
      hpke::AeadId::TLS_AES_128_GCM_SHA256,
      hpke::AeadId::TLS_AES_256_GCM_SHA384,
      hpke::AeadId::TLS_CHACHA20_POLY1305_SHA256};
};
} // namespace ech
} // namespace fizz
