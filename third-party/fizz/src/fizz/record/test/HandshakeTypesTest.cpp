/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#include <folly/portability/GMock.h>
#include <folly/portability/GTest.h>

#include <fizz/record/Extensions.h>
#include <folly/String.h>

using namespace folly;

// Real client hello captured from
// firefox nightly
static const std::string chlo =
    "0304383cd7adc8587"
    "e3b608ef98f8b47e5f29dd6124a1f258549"
    "34ad85157dd5a7a400001ac02bc02fcca9c"
    "ca8c02cc030c00ac009c013c014002f0035"
    "000a010001d000000014001200000f74726"
    "9732e66696c6970706f2e696f00170000ff"
    "01000100000a00080006001700180019000"
    "b0002010000230000337400000010001700"
    "1502683208737064792f332e31086874747"
    "02f312e31000500050100000000ff020002"
    "000d0033014b014900170041048d5e897c8"
    "96b17e1679766c14c785dd2c328c3eecc7d"
    "bfd2e2e817cd35c786aceea79bf1286ab8a"
    "5c3c464c46f5ba06338b24ea96ce442a4d1"
    "3356902dfcd1e90100010032c84b9fc5e4f"
    "12bf1b10da62506105f26d6913eb6a6ca34"
    "c454963b85d3bbbd9360994db2baa28c217"
    "e98cb6c40ed5a51246867910b5586dab299"
    "5cb2c7c6298ab6606906911c08913adabe2"
    "5901b7907b915b5962ad9e4639475292b18"
    "0b651929e53b7fb38e7150bae3360aa3a30"
    "b5a22facff2dd8716ef3239887f1f781757"
    "b5ef4c68e7ed31e5bfbcb0ebb5d86794322"
    "c01bd5d456292e8c0276efb05296cd24c10"
    "cd388cf51ee798cf7e18638c17e44874bc3"
    "bd1f697424511051f686db6a84b604d1bb4"
    "0f6041c0c0e28be2a98829c78c7baea2dd3"
    "49f0219443007e88fb3406dc4d9756e1076"
    "1ef0eef675b4c4625f5d0aab3d6c7f57747"
    "f4008d5a5000d0018001604010501060102"
    "010403050306030203050204020202";

static const std::string ssl3chlo =
    "03005880ff04e8d5e0af70a2fe55fd90a5380a184f78c213aa8a4142010cac742e3600004cc014c00a0039003800880087c00fc00500350084c013c00900330032009a009900450044c00ec004002f00960041c012c00800160013c00dc003000a0007c011c007c00cc0020005000400ff0100";

// From https://martinthomson.github.io/tls13-vectors/
static const std::string chloPsk =
    "03034e2d3805200a9433ebdb4f1bf85d0a773c65a7430aa904c13966e49ab96efe2500003e130113031302c02bc02fcca9cca8c00ac009c013c023c027c014009eccaa00330032006700390038006b00160013009c002f003c0035003d000a00050004010001950015003b00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000b0009000006736572766572ff01000100000a00140012001d00170018001901000101010201030104000b00020100002800260024001d0020621bb90eca697f96d7ddc2966218ae0e7961268d8def400f8d7805172501932f002a0000002b000706030403030302000d0020001e040305030603020308040805080604010501060102010402050206020202002d00020101002900bd009800924e535321db0251332a66d4efd3b0285509fd8a26b801211c72f5f9012b8a6f35005069950156fc02abe56744e7c7d2798675e8372cbba2a93e2436bcd3dbac7662e64e187379bec081051957c0da819d44fafb13d833752c7340a32df5e133e3717566ac66b4cc417a4d0afaa64493997dba0cd6e601bc11a4ce5506236c2c4094d1ea2329d1756ac32783ef158e91a92c44d0a8dc290021202deccf4db1a231fa7359797967c09aa4ea69c29ecd781b41cbae9b1d7e4c0ff9";

static const std::string encodedShlo =
    "03039f2bfbe752cb49bc82303dd32a80cf60483a38e44bfb695ebd0280bcc1c70c5b00130100002e00330024001d00208db235a330ee184b953a981ecfb23b05380768ed12050ec2f0ec62b74ef8f835002b00020304";

static const std::string encodedEoed = "";

static const std::string encodedEncryptedExtensions =
    "001c000a00140012001d0017001800190100010101020103010400000000";

static const std::string encodedCertificate =
    "000001b50001b0308201ac30820115a003020102020102300d06092a864886f70d01010b0500300e310c300a06035504031303727361301e170d3136303733303031323335395a170d3236303733303031323335395a300e310c300a0603550403130372736130819f300d06092a864886f70d010101050003818d0030818902818100b4bb498f8279303d980836399b36c6988c0c68de55e1bdb826d3901a2461eafd2de49a91d015abbc9a95137ace6c1af19eaa6af98c7ced43120998e187a80ee0ccb0524b1b018c3e0b63264d449a6d38e22a5fda430846748030530ef0461c8ca9d9efbfae8ea6d1d03e2bd193eff0ab9a8002c47428a6d35a8d88d79f7f1e3f0203010001a31a301830090603551d1304023000300b0603551d0f0404030205a0300d06092a864886f70d01010b05000381810085aad2a0e5b9276b908c65f73a7267170618a54c5f8a7b337d2df7a594365417f2eae8f8a58c8f8172f9319cf36b7fd6c55b80f21a03015156726096fd335e5e67f2dbf102702e608ccae6bec1fc63a42a99be5c3eb7107c3c54e9b9eb2bd5203b1c3b84e0a8b2f759409ba3eac9d91d402dcc0cc8f8961229ac9187b42b4de10000";

static const std::string encodedCertVerify =
    "080400805db9706f9bd41ab01be55f75b136cb89dda63dc6e4510e40c7203cb87f4eba2b122644018640641bde97e03d4caa1d670371b8bf81374d5126f88df68b87ef6c706cf9c0ee04063d8e65cb403433fb006c800e307b79b3a51fbae6089c2f3988ddfe04a760902e0a2141046054bdf807cf48cd3ce83f58a149ba35b7ff6c2f2a";

static const std::string nst =
    "0002a300d0a8dc290000924e535321db0251332a66d4efd3b0285509fd8a26b801211c72f5f9012b8a6f35005069950156fc02abe56744e7c7d2798675e8372cbba2a93e2436bcd3dbac7662e64e187379bec081051957c0da819d44fafb13d833752c7340a32df5e133e3717566ac66b4cc417a4d0afaa64493997dba0cd6e601bc11a4ce5506236c2c4094d1ea2329d1756ac32783ef158e91a92c440008002e000400020000";

static const std::string encodedKeyUpdate = "00";

static const std::string encodedCertRequest = "00000a000d0006000406030807";

static const std::string encodedCompressedCertificate =
    "000100face000009666f6f62617262617a";

namespace fizz {
namespace test {

class HandshakeTypesTest : public testing::Test {
 protected:
  template <class T>
  T decodeHex(const std::string& hex) {
    auto data = unhexlify(hex);
    auto buf = IOBuf::copyBuffer(data.data(), data.size());
    return decode<T>(std::move(buf));
  }

  template <class T>
  std::string encodeHex(T&& msg) {
    auto buf = encode(std::forward<T>(msg));
    auto str = buf->moveToFbString().toStdString();
    return hexlify(str);
  }
};

TEST_F(HandshakeTypesTest, ChloEncodeDecode) {
  auto clientHello = decodeHex<ClientHello>(chlo);

  auto sigAlgs = getExtension<SignatureAlgorithms>(clientHello.extensions);
  ASSERT_TRUE(sigAlgs);
  const auto& schemes = sigAlgs->supported_signature_algorithms;
  ASSERT_EQ(11, schemes.size());
  ASSERT_FALSE(
      schemes.end() ==
      std::find(
          schemes.begin(),
          schemes.end(),
          SignatureScheme::ecdsa_secp256r1_sha256));

  auto groups = getExtension<SupportedGroups>(clientHello.extensions);
  const auto& groupNames = groups->named_group_list;
  ASSERT_EQ(3, groupNames.size());
  ASSERT_FALSE(
      groupNames.end() ==
      std::find(groupNames.begin(), groupNames.end(), NamedGroup::secp256r1));

  auto clientShares = getExtension<ClientKeyShare>(clientHello.extensions);
  ASSERT_TRUE(clientShares);
  ASSERT_EQ(2, clientShares->client_shares.size());

  ASSERT_FALSE(getExtension<ClientPresharedKey>(clientHello.extensions));
  ASSERT_FALSE(getExtension<ClientEarlyData>(clientHello.extensions));
  ASSERT_FALSE(getExtension<Cookie>(clientHello.extensions));

  auto reencoded = encodeHex(std::move(clientHello));
  EXPECT_EQ(chlo, reencoded);
}

TEST_F(HandshakeTypesTest, SSL3ChloDecode) {
  auto clientHello = decodeHex<ClientHello>(ssl3chlo);
  EXPECT_TRUE(clientHello.extensions.empty());
}

TEST_F(HandshakeTypesTest, ChloDecidePsk) {
  auto clientHello = decodeHex<ClientHello>(chloPsk);

  EXPECT_EQ(35, getBinderLength(clientHello));
}

TEST_F(HandshakeTypesTest, ChloEncodeCopy) {
  auto clientHello = decodeHex<ClientHello>(chlo);
  EXPECT_EQ(encodeHex(clientHello), chlo);
  EXPECT_EQ(encodeHex(clientHello), chlo);
}

TEST_F(HandshakeTypesTest, NstEncodeDecode) {
  auto ticket = decodeHex<NewSessionTicket>(nst);

  EXPECT_EQ(ticket.ticket_lifetime, 0x0002a300);
  EXPECT_EQ(ticket.ticket_age_add, 0xd0a8dc29);
  EXPECT_TRUE(ticket.ticket_nonce->empty());
  EXPECT_EQ(ticket.ticket->computeChainDataLength(), 146);

  auto reencoded = encodeHex(std::move(ticket));
  EXPECT_EQ(nst, reencoded);
}

bool extensionsMatch(const Extension& expected, const Extension& actual) {
  if (expected.extension_type != actual.extension_type) {
    return false;
  }
  return folly::IOBufEqualTo()(
      *expected.extension_data, *actual.extension_data);
}

TEST_F(HandshakeTypesTest, EncodeAndDecodeSigAlgs) {
  auto clientHello = decodeHex<ClientHello>(chlo);
  auto sigAlgs = getExtension<SignatureAlgorithms>(clientHello.extensions);
  ASSERT_TRUE(sigAlgs);
  auto ext = encodeExtension(*sigAlgs);
  auto original = findExtension(
      clientHello.extensions, ExtensionType::signature_algorithms);
  EXPECT_TRUE(extensionsMatch(*original, ext));
}

TEST_F(HandshakeTypesTest, EncodeAndDecodeClientKeyShare) {
  auto clientHello = decodeHex<ClientHello>(chlo);
  auto share = getExtension<ClientKeyShare>(clientHello.extensions);
  ASSERT_TRUE(share);
  auto ext = encodeExtension(*share);
  auto original =
      findExtension(clientHello.extensions, ExtensionType::key_share);
  EXPECT_TRUE(extensionsMatch(*original, ext));
}

TEST_F(HandshakeTypesTest, EncodeAndDecodeServerHello) {
  auto shlo = decodeHex<ServerHello>(encodedShlo);
  EXPECT_EQ(shlo.legacy_version, ProtocolVersion::tls_1_2);
  EXPECT_EQ(shlo.random.front(), 0x9f);
  EXPECT_EQ(shlo.random.back(), 0x5b);
  EXPECT_EQ(shlo.cipher_suite, CipherSuite::TLS_AES_128_GCM_SHA256);
  EXPECT_EQ(shlo.extensions.size(), 2);
  EXPECT_TRUE(getExtension<ServerKeyShare>(shlo.extensions).has_value());
  EXPECT_TRUE(
      getExtension<ServerSupportedVersions>(shlo.extensions).has_value());
  auto reencoded = encodeHex(std::move(shlo));
  EXPECT_EQ(reencoded, encodedShlo);
}

TEST_F(HandshakeTypesTest, EncodeAndDecodeEndOfEarlyData) {
  auto eoed = decodeHex<EndOfEarlyData>(encodedEoed);
  auto reencoded = encodeHex(std::move(eoed));
  EXPECT_EQ(reencoded, encodedEoed);
}

TEST_F(HandshakeTypesTest, EncodeAndDecodeEncryptedExtensions) {
  auto ee = decodeHex<EncryptedExtensions>(encodedEncryptedExtensions);
  EXPECT_EQ(ee.extensions.size(), 2);
  EXPECT_TRUE(getExtension<SupportedGroups>(ee.extensions).has_value());
  auto reencoded = encodeHex(std::move(ee));
  EXPECT_EQ(reencoded, encodedEncryptedExtensions);
}

TEST_F(HandshakeTypesTest, EncodeAndDecodedCertificate) {
  auto cert = decodeHex<CertificateMsg>(encodedCertificate);
  EXPECT_EQ(cert.certificate_request_context->computeChainDataLength(), 0);
  EXPECT_EQ(cert.certificate_list.size(), 1);
  EXPECT_EQ(cert.certificate_list[0].cert_data->computeChainDataLength(), 432);
  EXPECT_TRUE(cert.certificate_list[0].extensions.empty());
  auto reencoded = encodeHex(std::move(cert));
  EXPECT_EQ(reencoded, encodedCertificate);
}

TEST_F(HandshakeTypesTest, EncodedAndDecodeCertificateVerify) {
  auto verify = decodeHex<CertificateVerify>(encodedCertVerify);
  EXPECT_EQ(verify.algorithm, SignatureScheme::rsa_pss_sha256);
  EXPECT_EQ(verify.signature->computeChainDataLength(), 128);
  auto reencoded = encodeHex(std::move(verify));
  EXPECT_EQ(reencoded, encodedCertVerify);
}

TEST_F(HandshakeTypesTest, EncodedAndDecodeKeyUpdated) {
  auto keyUpdate = decodeHex<KeyUpdate>(encodedKeyUpdate);
  EXPECT_EQ(keyUpdate.request_update, KeyUpdateRequest::update_not_requested);
  auto reencoded = encodeHex(std::move(keyUpdate));
  EXPECT_EQ(reencoded, encodedKeyUpdate);
}

TEST_F(HandshakeTypesTest, EncodedAndDecodeCertificateRequest) {
  auto cr = decodeHex<CertificateRequest>(encodedCertRequest);
  EXPECT_TRUE(cr.certificate_request_context->empty());
  EXPECT_EQ(cr.extensions.size(), 1);
  EXPECT_TRUE(getExtension<SignatureAlgorithms>(cr.extensions).has_value());
  auto reencoded = encodeHex(std::move(cr));
  EXPECT_EQ(reencoded, encodedCertRequest);
}

TEST_F(HandshakeTypesTest, EncodeAndDecodeCompressedCertificate) {
  auto cc = decodeHex<CompressedCertificate>(encodedCompressedCertificate);
  EXPECT_EQ(cc.algorithm, CertificateCompressionAlgorithm::zlib);
  EXPECT_EQ(cc.uncompressed_length, 0x00face);
  EXPECT_EQ(
      StringPiece(cc.compressed_certificate_message->coalesce()),
      StringPiece("foobarbaz"));
  auto reencoded = encodeHex(std::move(cc));
  EXPECT_EQ(reencoded, encodedCompressedCertificate);
}
} // namespace test
} // namespace fizz
