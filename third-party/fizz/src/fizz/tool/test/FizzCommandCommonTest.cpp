/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#include <folly/portability/GMock.h>
#include <folly/portability/GTest.h>

#include <fizz/protocol/ech/Types.h>
#include <fizz/tool/FizzCommandCommon.h>
#include <folly/container/Array.h>

namespace fizz {
namespace tool {
namespace test {

TEST(FizzCommandCommonTest, TestParseECHConfigsBase64) {
  auto echConfig64 =
      "AEj+DQBEAQAgACAX5SnnUbopIr5I/MqIqLWuSAZckHI2sR+aIr0slN2uGAAEAAEAAWQVZWNoLXB1YmxpYy5hdG1ldGEuY29tAAA=";
  auto expectedPubKey =
      "17e529e751ba2922be48fcca88a8b5ae48065c907236b11f9a22bd2c94ddae18";
  auto echConfigs = parseECHConfigsBase64(echConfig64);
  ASSERT_EQ(echConfigs->configs.size(), 1);
  auto firstConfig = echConfigs->configs[0];
  ASSERT_EQ(firstConfig.version, ech::ECHVersion::Draft15);
  folly::io::Cursor cursor(firstConfig.ech_config_content.get());
  auto echConfigContent = decode<ech::ECHConfigContentDraft>(cursor);
  ASSERT_TRUE(folly::IOBufEqualTo()(
      echConfigContent.public_name,
      folly::IOBuf::copyBuffer("ech-public.atmeta.com")));
  ASSERT_EQ(echConfigContent.key_config.kem_id, hpke::KEMId::x25519);
  ASSERT_EQ(
      echConfigContent.key_config.cipher_suites[0].kdf_id, hpke::KDFId::Sha256);
  ASSERT_EQ(
      echConfigContent.key_config.cipher_suites[0].aead_id,
      hpke::AeadId::TLS_AES_128_GCM_SHA256);
  ASSERT_EQ(echConfigContent.maximum_name_length, 100);
  ASSERT_TRUE(folly::IOBufEqualTo()(
      echConfigContent.key_config.public_key,
      folly::IOBuf::copyBuffer(folly::unhexlify(expectedPubKey))));
}

TEST(FizzCommandCommonTest, TestValidHostPortFromString) {
  struct ExpectedValues {
    std::string input;
    std::string host;
    uint16_t port;
  };
  auto results = folly::make_array(
      ExpectedValues{"www.example.com:80", "www.example.com", 80},
      ExpectedValues{"[::1]:80", "::1", 80},
      ExpectedValues{"127.0.0.1:80", "127.0.0.1", 80});
  for (auto result : results) {
    std::string host;
    uint16_t port;
    std::tie(host, port) = hostPortFromString(result.input);
    ASSERT_EQ(result.host, host);
    ASSERT_EQ(result.port, port);
  }
}

TEST(FizzCommandCommonTest, TestInvalidV6HostPortFromString) {
  auto inputs = folly::make_array("::1:80", "[::1:80", "::1]:80");
  for (auto input : inputs) {
    ASSERT_THROW(hostPortFromString(input), std::runtime_error);
  }
}

TEST(FizzCommandCommonTest, TestMissingPortHostPortFromString) {
  auto inputs =
      folly::make_array("www.example.com", "127.0.0.1", "[::1]", "::1");
  for (auto input : inputs) {
    ASSERT_THROW(hostPortFromString(input), std::runtime_error);
  }
}

void checkECHConfigContent(const ech::ECHConfigContentDraft& echConfigContent) {
  ASSERT_TRUE(folly::IOBufEqualTo()(
      echConfigContent.public_name, folly::IOBuf::copyBuffer("publicname")));
  auto expectedPubKey =
      "049d87bcaddb65d8dcf6df8b148a9679b5b710db19c95a9badfff13468cb358b4e21d24a5c826112658ebb96d64e2985dfb41c1948334391a4aa81b67837e2dbf0";
  ASSERT_TRUE(folly::IOBufEqualTo()(
      echConfigContent.key_config.public_key,
      folly::IOBuf::copyBuffer(folly::unhexlify(expectedPubKey))));
  ASSERT_EQ(echConfigContent.key_config.kem_id, hpke::KEMId::secp256r1);
  ASSERT_EQ(
      echConfigContent.key_config.cipher_suites[0].kdf_id, hpke::KDFId::Sha256);
  ASSERT_EQ(
      echConfigContent.key_config.cipher_suites[0].aead_id,
      hpke::AeadId::TLS_AES_128_GCM_SHA256);
  ASSERT_EQ(echConfigContent.maximum_name_length, 100);

  ASSERT_EQ(echConfigContent.extensions.size(), 1);
  ASSERT_EQ(
      echConfigContent.extensions[0].extension_type, ExtensionType::cookie);
  ASSERT_EQ(echConfigContent.key_config.config_id, 144);
}

TEST(FizzCommandCommonTest, TestParseECHConfigsSuccess) {
  auto json = folly::parseJson(R"(
      {
        "echconfigs": [{
                "version": "Draft15",
                "public_name": "publicname",
                "public_key": "049d87bcaddb65d8dcf6df8b148a9679b5b710db19c95a9badfff13468cb358b4e21d24a5c826112658ebb96d64e2985dfb41c1948334391a4aa81b67837e2dbf0",
                "kem_id": "secp256r1",
                "cipher_suites": [{
                        "kdf_id": "Sha256",
                        "aead_id": "TLS_AES_128_GCM_SHA256"
                }],
                "maximum_name_length": 100,
                "extensions": "002c00080006636f6f6b6965",
                "config_id": 144
        }]
      }
  )");
  folly::Optional<ech::ECHConfigList> echConfigList = parseECHConfigs(json);

  ASSERT_TRUE(echConfigList.has_value());

  auto echConfigs = echConfigList->configs;

  ASSERT_EQ(echConfigs.size(), 1);
  auto echConfig = echConfigs[0];
  ASSERT_EQ(echConfig.version, ech::ECHVersion::Draft15);

  folly::io::Cursor cursor(echConfig.ech_config_content.get());
  auto echConfigContent = decode<ech::ECHConfigContentDraft>(cursor);
  checkECHConfigContent(echConfigContent);
}

TEST(FizzCommandCommonTest, TestParseECHConfigsWithHexNumsSuccess) {
  auto json = folly::parseJson(R"(
      {
        "echconfigs": [{
                "version": "Draft15",
                "public_name": "publicname",
                "public_key": "049d87bcaddb65d8dcf6df8b148a9679b5b710db19c95a9badfff13468cb358b4e21d24a5c826112658ebb96d64e2985dfb41c1948334391a4aa81b67837e2dbf0",
                "kem_id": "secp256r1",
                "cipher_suites": [{
                        "kdf_id": "Sha256",
                        "aead_id": "TLS_AES_128_GCM_SHA256"
                }],
                "maximum_name_length": "0x64",
                "extensions": "002c00080006636f6f6b6965",
                "config_id": "0x90"
        }]
      }
  )");

  folly::Optional<ech::ECHConfigList> echConfigList = parseECHConfigs(json);

  ASSERT_TRUE(echConfigList.has_value());

  auto echConfigs = echConfigList->configs;

  ASSERT_EQ(echConfigs.size(), 1);
  auto echConfig = echConfigs[0];
  ASSERT_EQ(echConfig.version, ech::ECHVersion::Draft15);

  folly::io::Cursor cursor(echConfig.ech_config_content.get());
  auto echConfigContent = decode<ech::ECHConfigContentDraft>(cursor);
  checkECHConfigContent(echConfigContent);
}

TEST(FizzCommandCommonTest, TestParseECHConfigsFailure) {
  auto json = folly::parseJson(R"(
      {
        "echconfigs": [{
          "version": "V2"
        }]
      }
  )");
  folly::Optional<ech::ECHConfigList> echConfigs = parseECHConfigs(json);
  ASSERT_FALSE(echConfigs.has_value());
}

TEST(FizzCommandCommonTest, TestParseECHConfigsJsonExceptions) {
  auto testJson = folly::parseJson(R"(
      {
        "echconfigs": [{
                "version": "Draft15",
                "public_name": "publicname",
                "public_key": "049d87bcaddb65d8dcf6df8b148a9679b5b710db19c95a9badfff13468cb358b4e21d24a5c826112658ebb96d64e2985dfb41c1948334391a4aa81b67837e2dbf0",
                "kem_id": "secp256r1",
                "cipher_suites": [{
                        "kdf_id": "Sha256",
                        "aead_id": "TLS_AES_128_GCM_SHA256"
                }],
                "maximum_name_length": 100,
                "extensions": "002c00080006636f6f6b6965",
                "config_id": 144
        }]
      }
  )");

  // Test an exception is thrown when the json is invalid.
  auto invalidJson = R"({"echconfigs"})";
  ASSERT_THROW(parseECHConfigs(invalidJson), std::runtime_error);

  // Test an exception is thrown on an invalid KDF id.
  auto wrongKDFJson = testJson;
  wrongKDFJson["echconfigs"][0]["cipher_suites"][0]["aead_id"] = "Sha473873";
  ASSERT_THROW(parseECHConfigs(wrongKDFJson), std::runtime_error);

  // Test an exception is thrown on an invalid Aead id.
  auto wrongAeadIdJson = testJson;
  wrongAeadIdJson["echconfigs"][0]["cipher_suites"][0]["aead_id"] =
      "TLS_AES_something";
  ASSERT_THROW(parseECHConfigs(wrongAeadIdJson), std::runtime_error);

  // Test an exception is thrown on an invalid KEM id.
  auto wrongKEMJson = testJson;
  wrongKEMJson["echconfigs"][0]["kem_id"] = "secp48398";
  ASSERT_THROW(parseECHConfigs(wrongKEMJson), std::runtime_error);

  // Test that an exception is thrown when you try to pass a non-numeric string
  // for config_id
  auto badConfigIdJson = testJson;
  badConfigIdJson["echconfigs"][0]["config_id"] = "number";
  ASSERT_THROW(parseECHConfigs(badConfigIdJson), std::runtime_error);

  // Test that an exception is thrown when the numbers provided cannot be
  // represented using the numeric type.
  auto tooBigConfigIdJson = testJson;
  tooBigConfigIdJson["echconfigs"][0]["config_id"] = "0x100";
  ASSERT_THROW(parseECHConfigs(tooBigConfigIdJson), std::runtime_error);
  auto tooBigMaxLenJson = testJson;
  tooBigMaxLenJson["echconfigs"][0]["maximum_name_length"] = "0x1000";
  ASSERT_THROW(parseECHConfigs(tooBigMaxLenJson), std::runtime_error);
}

TEST(FizzCommandCommonTest, TestReadECHConfigsJsonException) {
  // Test an exception is thrown when no file is provided.
  ASSERT_THROW(readECHConfigsJson(""), std::runtime_error);

  // Test an exception is thrown when we are unable to read the file provided.
  ASSERT_THROW(readECHConfigsJson("test.txt"), std::runtime_error);
}

} // namespace test
} // namespace tool
} // namespace fizz
