/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */
#include <vector>

#include <fizz/protocol/ech/ECHExtensions.h>
#include <fizz/protocol/ech/test/TestUtil.h>
#include <folly/portability/GMock.h>
#include <folly/portability/GTest.h>

namespace fizz {
namespace ech {
namespace test {

folly::StringPiece kOuterECHClientHelloExtensionData{
    "0000010001AA0003656e6300077061796c6f6164"};

Buf getBuf(folly::StringPiece hex) {
  auto data = unhexlify(hex);
  return folly::IOBuf::copyBuffer(data.data(), data.size());
}

TEST(ECHTest, TestConfigContentEncodeDecode) {
  // Encode config contents
  std::unique_ptr<folly::IOBuf> echConfigContentBuf =
      encode<ECHConfigContentDraft>(getECHConfigContent());

  // Decode config content
  folly::io::Cursor cursor(echConfigContentBuf.get());
  auto gotEchConfigContent = decode<ECHConfigContentDraft>(cursor);

  // Check decode(encode(content)) = content
  auto expectedEchConfigContent = getECHConfigContent();
  EXPECT_TRUE(folly::IOBufEqualTo()(
      gotEchConfigContent.public_name, expectedEchConfigContent.public_name));
  EXPECT_TRUE(folly::IOBufEqualTo()(
      gotEchConfigContent.key_config.public_key,
      expectedEchConfigContent.key_config.public_key));
  EXPECT_EQ(
      gotEchConfigContent.key_config.kem_id,
      expectedEchConfigContent.key_config.kem_id);
  EXPECT_EQ(
      gotEchConfigContent.key_config.config_id,
      expectedEchConfigContent.key_config.config_id);
  EXPECT_EQ(
      gotEchConfigContent.key_config.cipher_suites.size(),
      expectedEchConfigContent.key_config.cipher_suites.size());
  EXPECT_EQ(
      gotEchConfigContent.maximum_name_length,
      expectedEchConfigContent.maximum_name_length);
  EXPECT_EQ(gotEchConfigContent.extensions.size(), 1);
  auto ext = getExtension<Cookie>(gotEchConfigContent.extensions);
  EXPECT_EQ(
      folly::StringPiece(ext->cookie->coalesce()),
      folly::StringPiece("cookie"));
}

TEST(ECHTest, TestECHConfigListEncodeDecode) {
  // Make ECH configs
  ECHConfig echConfig1;
  echConfig1.version = ECHVersion::Draft15;
  echConfig1.ech_config_content =
      encode<ECHConfigContentDraft>(getECHConfigContent());
  ECHConfig echConfig2;
  echConfig2.version = ECHVersion::Draft15;
  echConfig2.ech_config_content =
      encode<ECHConfigContentDraft>(getECHConfigContent());
  ECHConfig echConfig3;
  echConfig3.version = ECHVersion::Draft15;
  echConfig3.ech_config_content =
      encode<ECHConfigContentDraft>(getECHConfigContent());

  // Encode ECH config List
  ECHConfigList echConfigList;
  echConfigList.configs = {echConfig1, echConfig2};
  auto encodedBuf = encode<ECHConfigList>(std::move(echConfigList));

  // Decode ECH config
  folly::io::Cursor cursor(encodedBuf.get());
  auto gotECHConfigList = decode<ECHConfigList>(cursor);

  // All ECHConfigs in ECHConfigList should be the same
  for (auto& echConfig : gotECHConfigList.configs) {
    EXPECT_EQ(echConfig.version, ECHVersion::Draft15);
    EXPECT_TRUE(folly::IOBufEqualTo()(
        echConfig.ech_config_content,
        encode<ECHConfigContentDraft>(getECHConfigContent())));
  }
}

TEST(ECHTest, TestECHConfigEncodeDecode) {
  // Encode ECH config
  ECHConfig echConfig;
  echConfig.version = ECHVersion::Draft15;
  echConfig.ech_config_content =
      encode<ECHConfigContentDraft>(getECHConfigContent());
  std::unique_ptr<folly::IOBuf> encodedBuf =
      encode<ECHConfig>(std::move(echConfig));

  // Decode ECH config
  folly::io::Cursor cursor(encodedBuf.get());
  auto gotECHConfig = decode<ECHConfig>(cursor);

  // Check decode(encode(config)) = config
  EXPECT_EQ(gotECHConfig.version, ECHVersion::Draft15);
  EXPECT_TRUE(folly::IOBufEqualTo()(
      gotECHConfig.ech_config_content,
      encode<ECHConfigContentDraft>(getECHConfigContent())));
}

TEST(ECHTest, TestOuterECHClientHelloEncode) {
  OuterECHClientHello ech;
  ech.cipher_suite = HpkeSymmetricCipherSuite{
      hpke::KDFId::Sha256, hpke::AeadId::TLS_AES_128_GCM_SHA256};
  ech.config_id = 0xAA;
  ech.enc = folly::IOBuf::copyBuffer("enc");
  ech.payload = folly::IOBuf::copyBuffer("payload");

  Extension encoded = encodeExtension<ech::OuterECHClientHello>(ech);

  EXPECT_EQ(encoded.extension_type, ExtensionType::encrypted_client_hello);
  // This was captured as the expected output from generating the result.
  EXPECT_TRUE(folly::IOBufEqualTo()(
      encoded.extension_data,
      folly::IOBuf::copyBuffer(
          folly::unhexlify(kOuterECHClientHelloExtensionData))));
}

TEST(ECHTest, TestOuterECHClientHelloDecode) {
  Extension e;
  e.extension_type = ExtensionType::encrypted_client_hello;
  e.extension_data = folly::IOBuf::copyBuffer(
      folly::unhexlify(kOuterECHClientHelloExtensionData));
  std::vector<Extension> vec;
  vec.push_back(std::move(e));
  auto ech = getExtension<OuterECHClientHello>(vec);

  EXPECT_EQ(ech->cipher_suite.kdf_id, hpke::KDFId::Sha256);
  EXPECT_EQ(ech->config_id, 0xAA);
  EXPECT_TRUE(folly::IOBufEqualTo()(ech->enc, folly::IOBuf::copyBuffer("enc")));
  EXPECT_TRUE(
      folly::IOBufEqualTo()(ech->payload, folly::IOBuf::copyBuffer("payload")));
}

} // namespace test
} // namespace ech
} // namespace fizz
