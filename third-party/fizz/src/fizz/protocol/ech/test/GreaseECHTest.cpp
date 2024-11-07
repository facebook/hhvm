/*
 *  Copyright (c) Meta Platforms, Inc. and affiliates.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#include <gtest/gtest.h>

#include <fizz/protocol/ech/GreaseECH.h>

#include <fizz/crypto/hpke/Utils.h>
#include <fizz/protocol/test/Mocks.h>
#include <fizz/protocol/test/TestUtil.h>

using namespace fizz::test;

namespace fizz {
namespace ech {
namespace test {

TEST(GreaseECHTest, TestGenerateRandomGreaseECH) {
  MockFactory factory;
  factory.setDefaults();

  auto chlo = TestMessages::clientHelloPsk();
  auto sni = getExtension<ServerNameList>(chlo.extensions);

  GreaseECHSetting setting{};
  setting.maxPayloadSize = 100;
  auto greaseEch = generateGreaseECH(setting, factory, 0);

  std::array<hpke::KDFId, 3> kdfs{
      hpke::KDFId::Sha256, hpke::KDFId::Sha384, hpke::KDFId::Sha512};
  std::array<hpke::AeadId, 3> aeads{
      hpke::AeadId::TLS_AES_128_GCM_SHA256,
      hpke::AeadId::TLS_AES_256_GCM_SHA384,
      hpke::AeadId::TLS_CHACHA20_POLY1305_SHA256};

  EXPECT_NE(
      kdfs.end(),
      std::find(kdfs.begin(), kdfs.end(), greaseEch.cipher_suite.kdf_id));
  EXPECT_NE(
      aeads.end(),
      std::find(aeads.begin(), aeads.end(), greaseEch.cipher_suite.aead_id));

  std::array<size_t, 3> keyLengths{32, 48, 64};
  EXPECT_NE(
      keyLengths.end(),
      std::find(
          keyLengths.begin(),
          keyLengths.end(),
          greaseEch.enc->computeChainDataLength()));

  EXPECT_LE(greaseEch.payload->computeChainDataLength(), 100);
}

TEST(GreaseECHTest, TestGenerateComputedGreaseECH) {
  MockFactory factory;
  factory.setDefaults();

  auto chlo = TestMessages::clientHelloPsk();
  auto sni = getExtension<ServerNameList>(chlo.extensions);
  auto encodedChlo = encode(chlo);

  size_t encodedChloSize = encodedChlo->computeChainDataLength();
  GreaseECHSetting setting{
      /* minConfigId = */ 0,
      /* maxConfigId = */ 0,
      PayloadGenerationStrategy::Computed,
      /* minPayloadSize = */ 100,
      /* maxPayloadSize = */ 100,
      /* keySizes = */ {32},
      /* kdfs = */ {hpke::KDFId::Sha256},
      /* aeads = */ {hpke::AeadId::TLS_AES_128_GCM_SHA256}};
  auto greaseEch = generateGreaseECH(setting, factory, encodedChloSize);

  EXPECT_EQ(hpke::KDFId::Sha256, greaseEch.cipher_suite.kdf_id);
  EXPECT_EQ(
      hpke::AeadId::TLS_AES_128_GCM_SHA256, greaseEch.cipher_suite.aead_id);
  EXPECT_EQ(32, greaseEch.enc->computeChainDataLength());

  size_t expectedPayloadSize = encodedChloSize +
      hpke::getCipherOverhead(greaseEch.cipher_suite.aead_id) + 100;
  EXPECT_EQ(expectedPayloadSize, greaseEch.payload->computeChainDataLength());
}
} // namespace test
} // namespace ech
} // namespace fizz
