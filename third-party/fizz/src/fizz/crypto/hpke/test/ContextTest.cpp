/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#include <gtest/gtest.h>

#include <fizz/backend/openssl/Hasher.h>
#include <fizz/backend/openssl/OpenSSL.h>
#include <fizz/crypto/aead/test/TestUtil.h>
#include <fizz/crypto/hpke/Context.h>
#include <fizz/crypto/hpke/Utils.h>
#include <fizz/crypto/test/TestUtil.h>
#include <fizz/record/Types.h>

using namespace fizz::test;

namespace fizz {
namespace hpke {
namespace test {

const std::string kExportSecret =
    "60f5fe76e2699f98c19eab82fecf330b990ac32694a8e40e598e2326d0e29150";

struct Params {
  std::string key;
  std::string iv;
  std::string aad;
  std::string plaintext;
  std::string ciphertext;
  CipherSuite cipher;
  std::string exporterSecret;
  std::string exportContext;
  std::string expectedExportValue;
};

class HpkeContextTest : public ::testing::TestWithParam<Params> {};

static folly::StringPiece prefix07("HPKE-07");

TEST_P(HpkeContextTest, TestContext) {
  auto testParam = GetParam();
  HpkeSuiteId suiteId;
  Error suiteIdErr;
  EXPECT_EQ(
      generateHpkeSuiteId(
          suiteId,
          suiteIdErr,
          NamedGroup::secp256r1,
          HashFunction::Sha256,
          testParam.cipher),
      Status::Success);
  auto encryptCipher = getCipher(testParam.cipher);
  {
    Error setKeyErr;
    FIZZ_THROW_ON_ERROR(
        encryptCipher->setKey(
            setKeyErr,
            TrafficKey{toIOBuf(testParam.key), toIOBuf(testParam.iv)}),
        setKeyErr);
  }

  HpkeContextImpl encryptContext(
      std::move(encryptCipher),
      toIOBuf(kExportSecret),
      std::make_unique<fizz::hpke::Hkdf>(fizz::hpke::Hkdf::withPrefix(
          prefix07, openssl::hasherFactory<Sha256>())),
      suiteId->clone(),
      fizz::hpke::HpkeContext::Role::Sender);
  std::unique_ptr<folly::IOBuf> gotCiphertext;
  Error err;
  EXPECT_EQ(
      encryptContext.seal(
          gotCiphertext,
          err,
          toIOBuf(testParam.aad).get(),
          toIOBuf(testParam.plaintext)),
      Status::Success);
  EXPECT_TRUE(
      folly::IOBufEqualTo()(gotCiphertext, toIOBuf(testParam.ciphertext)));

  auto decryptCipher = getCipher(testParam.cipher);
  {
    Error setKeyErr;
    FIZZ_THROW_ON_ERROR(
        decryptCipher->setKey(
            setKeyErr,
            TrafficKey{toIOBuf(testParam.key), toIOBuf(testParam.iv)}),
        setKeyErr);
  }
  HpkeContextImpl decryptContext(
      std::move(decryptCipher),
      toIOBuf(kExportSecret),
      std::make_unique<fizz::hpke::Hkdf>(fizz::hpke::Hkdf::withPrefix(
          prefix07, openssl::hasherFactory<Sha256>())),
      std::move(suiteId),
      fizz::hpke::HpkeContext::Role::Receiver);
  std::unique_ptr<folly::IOBuf> gotPlaintext;
  EXPECT_EQ(
      decryptContext.open(
          gotPlaintext,
          err,
          toIOBuf(testParam.aad).get(),
          std::move(gotCiphertext)),
      Status::Success);
  EXPECT_TRUE(
      folly::IOBufEqualTo()(gotPlaintext, toIOBuf(testParam.plaintext)));
}

TEST_P(HpkeContextTest, TestContextRoles) {
  auto testParam = GetParam();
  HpkeSuiteId suiteId;
  Error suiteIdErr;
  EXPECT_EQ(
      generateHpkeSuiteId(
          suiteId,
          suiteIdErr,
          NamedGroup::secp256r1,
          HashFunction::Sha256,
          testParam.cipher),
      Status::Success);
  auto encryptCipher = getCipher(testParam.cipher);
  {
    Error setKeyErr;
    FIZZ_THROW_ON_ERROR(
        encryptCipher->setKey(
            setKeyErr,
            TrafficKey{toIOBuf(testParam.key), toIOBuf(testParam.iv)}),
        setKeyErr);
  }

  HpkeContextImpl encryptContext(
      std::move(encryptCipher),
      toIOBuf(kExportSecret),
      std::make_unique<fizz::hpke::Hkdf>(fizz::hpke::Hkdf::withPrefix(
          prefix07, openssl::hasherFactory<Sha256>())),
      suiteId->clone(),
      fizz::hpke::HpkeContext::Role::Sender);

  auto decryptCipher = getCipher(testParam.cipher);
  {
    Error setKeyErr;
    FIZZ_THROW_ON_ERROR(
        decryptCipher->setKey(
            setKeyErr,
            TrafficKey{toIOBuf(testParam.key), toIOBuf(testParam.iv)}),
        setKeyErr);
  }
  HpkeContextImpl decryptContext(
      std::move(decryptCipher),
      toIOBuf(kExportSecret),
      std::make_unique<fizz::hpke::Hkdf>(fizz::hpke::Hkdf::withPrefix(
          prefix07, openssl::hasherFactory<Sha256>())),
      std::move(suiteId),
      fizz::hpke::HpkeContext::Role::Receiver);

  std::unique_ptr<folly::IOBuf> gotCiphertext;
  Error err;
  EXPECT_EQ(
      encryptContext.seal(
          gotCiphertext,
          err,
          toIOBuf(testParam.aad).get(),
          toIOBuf(testParam.plaintext)),
      Status::Success);

  // Check that encrypting/decrypting from the wrong role errors
  EXPECT_THROW(
      {
        std::unique_ptr<folly::IOBuf> sealRet;
        FIZZ_THROW_ON_ERROR(
            decryptContext.seal(
                sealRet,
                err,
                toIOBuf(testParam.aad).get(),
                toIOBuf(testParam.plaintext)),
            err);
      },
      std::logic_error);
  std::unique_ptr<folly::IOBuf> openRet;
  EXPECT_EQ(
      encryptContext.open(
          openRet, err, toIOBuf(testParam.aad).get(), std::move(gotCiphertext)),
      Status::Fail);
}

TEST_P(HpkeContextTest, TestExportSecret) {
  for (auto role :
       {fizz::hpke::HpkeContext::Role::Sender,
        fizz::hpke::HpkeContext::Role::Receiver}) {
    auto testParam = GetParam();
    auto exporterContext = toIOBuf(testParam.exportContext);

    HpkeSuiteId suiteId;
    Error suiteIdErr;
    EXPECT_EQ(
        generateHpkeSuiteId(
            suiteId,
            suiteIdErr,
            NamedGroup::x25519,
            HashFunction::Sha256,
            testParam.cipher),
        Status::Success);
    HpkeContextImpl context(
        getCipher(testParam.cipher),
        toIOBuf(testParam.exporterSecret),
        std::make_unique<fizz::hpke::Hkdf>(fizz::hpke::Hkdf::withPrefix(
            prefix07, openssl::hasherFactory<Sha256>())),
        std::move(suiteId),
        role);
    std::unique_ptr<folly::IOBuf> secret;
    Error err;
    EXPECT_EQ(
        context.exportSecret(secret, err, std::move(exporterContext), 32),
        Status::Success);

    auto expectedValue = folly::unhexlify(testParam.expectedExportValue);
    EXPECT_TRUE(
        folly::IOBufEqualTo()(secret, folly::IOBuf::copyBuffer(expectedValue)));
  }
}

TEST_P(HpkeContextTest, TestExportSecretThrow) {
  for (auto role :
       {fizz::hpke::HpkeContext::Role::Sender,
        fizz::hpke::HpkeContext::Role::Receiver}) {
    auto testParam = GetParam();
    auto exporterContext = toIOBuf(testParam.exportContext);

    HpkeSuiteId suiteId;
    Error suiteIdErr;
    EXPECT_EQ(
        generateHpkeSuiteId(
            suiteId,
            suiteIdErr,
            NamedGroup::x25519,
            HashFunction::Sha256,
            CipherSuite::TLS_AES_128_GCM_SHA256),
        Status::Success);
    HpkeContextImpl context(
        openssl::OpenSSLEVPCipher::makeCipher<fizz::AESGCM128>(),
        toIOBuf(testParam.exporterSecret),
        std::make_unique<fizz::hpke::Hkdf>(fizz::hpke::Hkdf::withPrefix(
            prefix07, openssl::hasherFactory<Sha256>())),
        std::move(suiteId),
        role);

    Error err;
    EXPECT_THROW(
        {
          std::unique_ptr<folly::IOBuf> secret;
          FIZZ_THROW_ON_ERROR(
              context.exportSecret(
                  secret, err, std::move(exporterContext), SIZE_MAX),
              err);
        },
        std::runtime_error);
  }
}

/***
 * Test vectors sourced from HPKE IETF draft vectors
 * https://raw.githubusercontent.com/cfrg/draft-irtf-cfrg-hpke/3d6ced124134825ed7a953b126cf5f756d960bc9/test-vectors.json
 */
// clang-format off

INSTANTIATE_TEST_SUITE_P(
    TestVectors,
    HpkeContextTest,
    ::testing::
        Values(
            Params{
                "e20cee1bf5392ad2d3a442e231f187ae",
                "5d99b2f03c452f7a9441933a",
                "436f756e742d30",
                "4265617574792069732074727574682c20747275746820626561757479",
                "9418f1ae06eddc43aa911032aed4a951754ee2286a786733761857f8d96a7ec8d852da93bc5eeab49623344aba",
                CipherSuite::TLS_AES_128_GCM_SHA256,
                "00c3cdacab28e981cc907d12e4f55f0aacae261dbb4eb610447a6bc431bfe2aa",
                "54657374436f6e74657874",
                "c8387c1e6ec4f026c7f3577e3f29df51f46161295eec84c4f64a9174f7b64e4f"},
            Params{
                "29b6985e93a71d68d77935d8372cf179db14bc21a3ad681e3afcabd287e46fd0",
                "52d2af88623a97733e068886",
                "436f756e742d30",
                "4265617574792069732074727574682c20747275746820626561757479",
                "451972846bb2c58ff6e6eb5ccc3bda8cc84fb6e93be5a1119cc32e3e374182d66d9a5910a14ec51baede71bedf",
                CipherSuite::TLS_AES_256_GCM_SHA384,
                "29f070f3562ed755db05a24bd3ce1562f7cee293cc5531bbc9573863731566b3",
                "54657374436f6e74657874",
                "36c126f8cb75015204ea8ceb866b346fa33309f3723553b91eae547e15153d72"},
            Params{
                "a17448a542d0d6d75e3b21be0a1f68607904b4802c6b19a7e7e90976aa00a5c8",
                "6f6b832dba944a91e5684514",
                "436f756e742d30",
                "4265617574792069732074727574682c20747275746820626561757479",
                "1b9ce69bd0e6b4242ac2dd841ef093fc9dfa9e684f81c2d1778fd3268ca5aa7d612cd87f72acd2aeaee084dee2",
                CipherSuite::TLS_CHACHA20_POLY1305_SHA256,
                "bbbd4216184bd12888e0cec08e384c2e39639fe1527f220f3aa751f5290a9aa7",
                "54657374436f6e74657874",
                "bb69068c4f7767331512d375e4ab0ca0c6c51446040096ea0ae1cc3f9a3f54bd"}
    ));
// clang-format on

} // namespace test
} // namespace hpke
} // namespace fizz
