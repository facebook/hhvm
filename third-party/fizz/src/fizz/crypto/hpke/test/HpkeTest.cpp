/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#include <fizz/crypto/Sha256.h>
#include <fizz/crypto/hpke/Hpke.h>
#include <fizz/crypto/hpke/Utils.h>
#include <fizz/crypto/hpke/test/Mocks.h>
#include <fizz/crypto/test/TestUtil.h>
#include <gtest/gtest.h>

using namespace fizz::test;

namespace fizz {
namespace hpke {
namespace test {

struct Params {
  Mode mode;
  NamedGroup group;
  HashFunction hash;
  CipherSuite suite;
  std::string sharedSecret;
  std::string info;
  // Key pair used for encryption
  std::string skE;
  std::string pkE;
  // Key pair used for decryption
  std::string skR;
  std::string pkR;
  // Optional
  std::string psk;
  std::string pskId;
  // Expected traffic key
  std::string key;
  std::string iv;
  // Encryption/decryption
  std::string ciphertext;
  // Test exports
  std::string exporterSecret;
  std::vector<std::string> exportValues;
};

void testExportValues(
    std::unique_ptr<HpkeContext>& context,
    const std::vector<std::string>& exportValues) {
  const size_t exportLength = 32;
  const std::vector<std::string> contexts = {
      "", "00", "54657374436f6e74657874"};
  ASSERT_EQ(exportValues.size(), contexts.size());

  for (size_t testNum = 0; testNum < contexts.size(); ++testNum) {
    std::unique_ptr<folly::IOBuf> exporterContext =
        toIOBuf(contexts.at(testNum));
    auto secret =
        context->exportSecret(std::move(exporterContext), exportLength);
    auto expectedSecret = toIOBuf(exportValues.at(testNum));

    EXPECT_TRUE(folly::IOBufEqualTo()(secret, expectedSecret));
  }
}

MATCHER_P(TrafficKeyMatcher, expectedKey, "") {
  return folly::IOBufEqualTo()(expectedKey->iv, arg->iv) &&
      folly::IOBufEqualTo()(expectedKey->key, arg->key);
}

class HpkeMockX25519KeyExchange : public X25519KeyExchange {
 public:
  MOCK_METHOD(void, generateKeyPair, ());
};

SetupParam getSetupParam(
    std::unique_ptr<X25519KeyExchange> kex,
    CipherSuite suite,
    std::string privateKey,
    std::string publicKey,
    std::unique_ptr<MockAeadCipher> cipher) {
  auto group = NamedGroup::x25519;
  kex->setKeyPair(toIOBuf(privateKey), toIOBuf(publicKey));

  std::unique_ptr<folly::IOBuf> suiteId =
      generateHpkeSuiteId(group, HashFunction::Sha256, suite);

  return SetupParam{
      std::make_unique<DHKEM>(
          std::move(kex),
          group,
          std::make_unique<fizz::hpke::Hkdf>(
              folly::IOBuf::copyBuffer("HPKE-07"),
              std::make_unique<HkdfImpl>(HkdfImpl::create<Sha256>()))),
      std::move(cipher),
      std::make_unique<fizz::hpke::Hkdf>(
          folly::IOBuf::copyBuffer("HPKE-07"),
          std::make_unique<HkdfImpl>(HkdfImpl::create<Sha256>())),
      std::move(suiteId),
  };
}

class HpkeTest : public ::testing::TestWithParam<Params> {};

TEST_P(HpkeTest, TestSetup) {
  auto testParam = GetParam();
  auto pkR = toIOBuf(testParam.pkR);
  auto info = toIOBuf(testParam.info);

  auto encapCipher =
      std::make_unique<MockAeadCipher>(getCipher(testParam.suite));
  TrafficKey encapExpectedTrafficKey{
      toIOBuf(testParam.key), toIOBuf(testParam.iv)};
  EXPECT_CALL(
      *encapCipher, _setKey(TrafficKeyMatcher(&encapExpectedTrafficKey)))
      .Times(1);
  auto encapKex = std::make_unique<HpkeMockX25519KeyExchange>();
  EXPECT_CALL(*encapKex, generateKeyPair()).Times(1);

  SetupResult setupResult = setupWithEncap(
      testParam.mode,
      pkR->coalesce(),
      info->clone(),
      PskInputs(
          testParam.mode, toIOBuf(testParam.psk), toIOBuf(testParam.pskId)),
      getSetupParam(
          std::move(encapKex),
          testParam.suite,
          testParam.skE,
          testParam.pkE,
          std::move(encapCipher)));
  std::unique_ptr<HpkeContext> encryptContext = std::move(setupResult.context);

  auto enc = std::move(setupResult.enc);
  auto decapCipher =
      std::make_unique<MockAeadCipher>(getCipher(testParam.suite));
  TrafficKey decapExpectedTrafficKey{
      toIOBuf(testParam.key), toIOBuf(testParam.iv)};
  EXPECT_CALL(
      *decapCipher, _setKey(TrafficKeyMatcher(&decapExpectedTrafficKey)))
      .Times(1);

  std::unique_ptr<HpkeContext> decryptContext = setupWithDecap(
      testParam.mode,
      enc->coalesce(),
      std::move(info),
      PskInputs(
          testParam.mode, toIOBuf(testParam.psk), toIOBuf(testParam.pskId)),
      getSetupParam(
          std::make_unique<X25519KeyExchange>(),
          testParam.suite,
          testParam.skR,
          testParam.pkR,
          std::move(decapCipher)));

  // Test encrypt/decrypt
  std::unique_ptr<folly::IOBuf> aad = toIOBuf("436f756e742d30");
  std::unique_ptr<folly::IOBuf> plaintext =
      toIOBuf("4265617574792069732074727574682c20747275746820626561757479");

  auto ciphertext = encryptContext->seal(aad.get(), plaintext->clone());
  auto expectedCiphertext = testParam.ciphertext;
  EXPECT_TRUE(folly::IOBufEqualTo()(ciphertext, toIOBuf(expectedCiphertext)));

  auto gotPlaintext = decryptContext->open(aad.get(), std::move(ciphertext));
  EXPECT_TRUE(folly::IOBufEqualTo()(gotPlaintext, plaintext));

  // Test exporter secret
  auto gotExporterSecretE = encryptContext->getExporterSecret();
  auto gotExporterSecretD = decryptContext->getExporterSecret();
  auto expectedExporterSecret = toIOBuf(testParam.exporterSecret);
  EXPECT_TRUE(
      folly::IOBufEqualTo()(gotExporterSecretE, expectedExporterSecret));
  EXPECT_TRUE(
      folly::IOBufEqualTo()(gotExporterSecretD, expectedExporterSecret));

  // Test export values
  testExportValues(encryptContext, testParam.exportValues);
  testExportValues(decryptContext, testParam.exportValues);
}

TEST_P(HpkeTest, TestKeySchedule) {
  auto testParam = GetParam();

  const auto prefix = folly::IOBuf::copyBuffer("HPKE-07");
  std::unique_ptr<fizz::hpke::Hkdf> hkdf = std::make_unique<fizz::hpke::Hkdf>(
      prefix->clone(), std::make_unique<HkdfImpl>(HkdfImpl::create<Sha256>()));

  std::unique_ptr<MockAeadCipher> cipher =
      std::make_unique<MockAeadCipher>(getCipher(testParam.suite));
  std::unique_ptr<folly::IOBuf> suiteId = generateHpkeSuiteId(
      testParam.group, HashFunction::Sha256, testParam.suite);
  TrafficKey expectedTrafficKey{toIOBuf(testParam.key), toIOBuf(testParam.iv)};
  EXPECT_CALL(*cipher, _setKey(TrafficKeyMatcher(&expectedTrafficKey)))
      .Times(1);

  struct KeyScheduleParams keyScheduleParams {
    testParam.mode, toIOBuf(testParam.sharedSecret), toIOBuf(testParam.info),
        PskInputs(
            testParam.mode,
            toIOBuf(testParam.psk),
            toIOBuf(testParam.pskId)),
        std::move(cipher), std::move(hkdf), std::move(suiteId),
        fizz::hpke::HpkeContext::Role::Sender
  };
  auto context = keySchedule(std::move(keyScheduleParams));

  EXPECT_TRUE(folly::IOBufEqualTo()(
      context->getExporterSecret(), toIOBuf(testParam.exporterSecret)));
}

/***
 * Test vectors sourced from IETF's HPKE draft.
 * https://raw.githubusercontent.com/cfrg/draft-irtf-cfrg-hpke/580119bb7bb45fd09a1079b920f8ef257f901309/test-vectors.json
 */
// clang-format off

INSTANTIATE_TEST_SUITE_P(
    TestVectors,
    HpkeTest,
    ::testing::
        Values(
            Params{
                Mode::Base,
                NamedGroup::x25519,
                HashFunction::Sha256,
                CipherSuite::TLS_AES_128_GCM_SHA256,
                "799b7b9a6a070e77ee9b9a2032f6624b273b532809c60200eba17ac3baf69a00",
                "4f6465206f6e2061204772656369616e2055726e",
                "6cee2e2755790708a2a1be22667883a5e3f9ec52810404a0d889a0ed3e28de00",
                "950897e0d37a8bdb0f2153edf5fa580a64b399c39fbb3d014f80983352a63617",
                "ecaf25b8485bcf40b9f013dbb96a6230f25733b8435bba0997a1dedbc7f78806",
                "a5912b20892e36905bac635267e2353d58f8cc7525271a2bf57b9c48d2ec2c07",
                "",
                "",
                "e20cee1bf5392ad2d3a442e231f187ae",
                "5d99b2f03c452f7a9441933a",
                "9418f1ae06eddc43aa911032aed4a951754ee2286a786733761857f8d96a7ec8d852da93bc5eeab49623344aba",
                "00c3cdacab28e981cc907d12e4f55f0aacae261dbb4eb610447a6bc431bfe2aa",
                {
                  "be82c06bd83fd6edd74385de5a70859b9e03def4c7bb224a10cfae86087f8a25",
                  "82cbfd3c2b2db75e2311d457e569cf12b6387eb4309bca8e77adb2f2b599fc85",
                  "c8387c1e6ec4f026c7f3577e3f29df51f46161295eec84c4f64a9174f7b64e4f",
                }
            },
            Params{
                Mode::Psk,
                NamedGroup::x25519,
                HashFunction::Sha256,
                CipherSuite::TLS_AES_128_GCM_SHA256,
                "eeca0089c3e7d96d31f7c492f719a7a6cddec0170e9aba954c7ac8ca98388e0d",
                "4f6465206f6e2061204772656369616e2055726e",
                "4c1feed23e15ec6a55b8457e0c0f42a3a1ab3ccc309b7cbb7ac6165fc657bd3b",
                "f16fa9440b2cb36c855b4b82fb87e1c02ce656dd132f7a7aec739294b6912768",
                "8e5430f0d821407670e5e3f6eecc9f52b2cad27b15a5fad1f3d05359ae30d81c",
                "13c789187a2dda71889e4b98dc5443624ae68f309cea91865561cfa207586e3a",
                "0247fd33b913760fa1fa51e1892d9f307fbe65eb171e8132c2af18555a738b82",
                "456e6e796e20447572696e206172616e204d6f726961",
                "70030b55bfb737d4f4355cf62302d281",
                "746d5e6255902701c3e0b99f",
                "63f7ed3d99e625d4a7373982b5f04daf0c3dfff39cac4b38eeb9d5c225cc3183bdbc91a053db9b195319cc8c45",
                "716043e2ac96b23e6f12983e11b6894e7b7dab8a9e40976b467c514f59700d9a",
                {
                  "7c40ceb745e14d19fceeac6e4756c796957fe5ff28709198c3f8cbdb5d368fe1",
                  "1ef0fd07bd40326f1b88f3545c92969cff202ca7186b9fd1315241f93fcc2edf",
                  "997368419db9490aa96c977cdd90bda8fd6234054d4add3d2f31aaaa2f8c1172",
                }
            },
            Params{
                Mode::Base,
                NamedGroup::x25519,
                HashFunction::Sha256,
                CipherSuite::TLS_AES_256_GCM_SHA384,
                "f189d166845d708b2aceb6b0fc36a656ca37dc625d1dde937d03f23da5e7d53b",
                "4f6465206f6e2061204772656369616e2055726e",
                "0c97c56fc519f7159adfb70de99529b5a28fd728802751aaa58bd685449022cf",
                "29bcfb25097963ef5dcd450103524f95a2e878abfb12e60139cad6a3e93f4534",
                "5f8d09d7d26db6de7ff1a49a0685579af06d8b905a51bc8a9314bc281510368e",
                "8395d456dad5c3a320d95a10b5551caa41beaf4a3a68fbf87503ba220631740d",
                "",
                "",
                "29b6985e93a71d68d77935d8372cf179db14bc21a3ad681e3afcabd287e46fd0",
                "52d2af88623a97733e068886",
                "451972846bb2c58ff6e6eb5ccc3bda8cc84fb6e93be5a1119cc32e3e374182d66d9a5910a14ec51baede71bedf",
                "29f070f3562ed755db05a24bd3ce1562f7cee293cc5531bbc9573863731566b3",
                {
                  "c5c27bd0dfeb9b36f7daa7da2c9a93a839ad7f43a1aeb566416989bd304df181",
                  "97301aa7db2313be2aaf233c4efc91d7f888f1d2dcb06f4dd5fbf07cc13b5c84",
                  "36c126f8cb75015204ea8ceb866b346fa33309f3723553b91eae547e15153d72",
                }
            },
            Params{
                Mode::Psk,
                NamedGroup::x25519,
                HashFunction::Sha256,
                CipherSuite::TLS_AES_256_GCM_SHA384,
                "73b4191d5c2c5a35bf45da277d569b03e90ba0bd0c85ee62dee88e40ebc9edf3",
                "4f6465206f6e2061204772656369616e2055726e",
                "6758aab4d768134201d1619a5cd272b610fe93d41b7764400c035ba59e8c6fae",
                "dc28e4a833daf5b3a87e8e9dc29b6a5fb435e15b95174d0761727c2258c22215",
                "1bd2abe796adf27a3391593a6f94a52d574e16f1197d031376f0e8c0dc387471",
                "dc05cc96b548d1eeecc07b4334adee7b5b743d62401257cedfbf7dce78c2f933",
                "0247fd33b913760fa1fa51e1892d9f307fbe65eb171e8132c2af18555a738b82",
                "456e6e796e20447572696e206172616e204d6f726961",
                "442bd67aa46f6266225a95e668f4368bc7c6bfc4a834f37cd8952ff3a65d9479",
                "945fd056e4a485e41e0835ec",
                "b764782d297c351a93e821c05a4ef4b770946a29e2ac10ef128bda3c5371dac890fe99ef0e914679d154a69723",
                "d306797db89ea0a16682d8dc3358cf5821ff193c65254d479f37b434c565dd06",
                {
                  "40cffbf96c76a21fbe59e121e05a628bb2ce7575622cc235d299f30741983fea",
                  "f4bb8649b2928f819eb93344dcab7b9d8802004e01fa092d1da61883731242ec",
                  "15bfb2c154e7527401cb3a33b1d60711ce55654ea93fb4552d5ac2a7d2d3a78d",
                }
            },
            Params{
                Mode::Psk,
                NamedGroup::x25519,
                HashFunction::Sha256,
                CipherSuite::TLS_CHACHA20_POLY1305_SHA256,
                "f2aa15c783c62c7e55485a61404d8beae0644d15042848e5adf3d315981337e1",
                "4f6465206f6e2061204772656369616e2055726e",
                "db1c9dfba77e1e3b8687ea18af207cffca803bdd983f955376b8271ef9c78a46",
                "8e4b29035c22b67b3a7a0f5a52f12b3ab17a9ae1f0c63b029137ba09f420224a",
                "4e335da3ec60e68c156586b8217de6801cb83b5a4de413645fcb112c00b2228b",
                "94ea1227a357dfd3548aadb9ef19d9974add594871498e123390a8bcb4db5d51",
                "0247fd33b913760fa1fa51e1892d9f307fbe65eb171e8132c2af18555a738b82",
                "456e6e796e20447572696e206172616e204d6f726961",
                "a603fe0f9897dc6ce042a467d6bd430a01cd679e930f1b5706ad425e4153496d",
                "318e48afae42913a928146e6",
                "c87f8158a501c7a2f31708bbdba10f9c5ad035624c3153eeb028e65b82f41f38cbe1cd9aafb10e502d328b83c1",
                "965e593816181bd8f14211f5e5773b3fa256a24972a1793165177987cb82cb6e",
                {
                  "23c31ee2757bbecf105f74c90bf1e640b6ddc545dc8d80b1abbf2aa9dd1786ce",
                  "05af7597519945fe8443f7cb84cdb651a8dd18cd7bbbd65d31095d3c69c1257e",
                  "5814619f842c7c328c9657854154e51b581c7bbd3b646bd773be67f93900a109",
                }
            },
            Params{
                Mode::Base,
                NamedGroup::x25519,
                HashFunction::Sha256,
                CipherSuite::TLS_CHACHA20_POLY1305_SHA256,
                "5f32519d9ca90b0572df7aa3b2e2f35376cafc61e027a406e03d6441ab818a7f",
                "4f6465206f6e2061204772656369616e2055726e",
                "efda8f0538ce6ab9f165aae26e02ad96dcb1775b248267174aeb3d140e002ee3",
                "1440805f4e60cbd34835baf0813c3071d17def1dbd8c04e75889bb2271d7823a",
                "14365bb26500e7cf263720c4ab04bd45b8e146b4f724facd1fa01d58b63975e4",
                "26147d5c2978bccc3cc03a4f9ac607560b5d83f852be4e9024f2cb7207d4c30e",
                "",
                "",
                "a17448a542d0d6d75e3b21be0a1f68607904b4802c6b19a7e7e90976aa00a5c8",
                "6f6b832dba944a91e5684514",
                "1b9ce69bd0e6b4242ac2dd841ef093fc9dfa9e684f81c2d1778fd3268ca5aa7d612cd87f72acd2aeaee084dee2",
                "bbbd4216184bd12888e0cec08e384c2e39639fe1527f220f3aa751f5290a9aa7",
                {
                  "996dc6fda1dc47e687613e0e221d64a3598e1ead9585177d22f230716569c04d",
                  "6d07b4e3e06ace3dc3f1b2a0826a0f896aa828769ff993c2e3829ae40325c27d",
                  "bb69068c4f7767331512d375e4ab0ca0c6c51446040096ea0ae1cc3f9a3f54bd",
                }
            }
    ));
// clang-format on

} // namespace test
} // namespace hpke
} // namespace fizz
