/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */
#include <fizz/backend/openssl/OpenSSL.h>
#include <fizz/client/PskSerializationUtils.h>
#include <fizz/client/test/Utilities.h>
#include <fizz/crypto/test/TestUtil.h>
#include <fizz/protocol/test/Mocks.h>
#include <folly/portability/GMock.h>
#include <folly/portability/GTest.h>

using namespace testing;

namespace fizz {
namespace client {
namespace test {

static std::shared_ptr<const fizz::Cert> getCert(
    folly::StringPiece encodedPem) {
  auto certs = folly::ssl::OpenSSLCertUtils::readCertsFromBuffer(encodedPem);
  CHECK_EQ(certs.size(), 1);
  return openssl::CertUtils::makePeerCert(std::move(certs[0]));
}
class PskSerializationTest : public Test {
 public:
  void SetUp() override {
    ticketTime_ = std::chrono::system_clock::now();
  }

 protected:
  CachedPsk getCachedPsk(std::string pskName) {
    auto psk = getTestPsk(pskName, ticketTime_);
    psk.clientCert = getCert(fizz::test::kClientAuthClientCert);
    psk.serverCert = getCert(fizz::test::kRSACertificate);
    return psk;
  }

  std::chrono::system_clock::time_point ticketTime_;
};

TEST_F(PskSerializationTest, TestDeserializationCompatibility) {
  // This is the output of the `TestSerialization` serializePsk call, at
  // a specific commit. If this test fails, this implies that your change
  // may cause old PSKs to no longer be recognized.
  constexpr folly::StringPiece kSerializedPSK =
      "0007536f6d6550736b0010726573756d7074696f6e7365637265740304130101001d02683211111111000001912edab3a60000000066b3ee4c000003653082036130820249a003020102020900c3420836ac1ca26f300d06092a864886f70d01010b0500304b310b3009060355040613025553310b300906035504080c024e593111300f06035504070c084e657720596f726b310d300b060355040b0c0446697a7a310d300b06035504030c0446697a7a301e170d3136313232393036323431385a170d3431303832303036323431385a304b310b3009060355040613025553310b300906035504080c024e593111300f06035504070c084e657720596f726b310d300b060355040b0c0446697a7a310d300b06035504030c0446697a7a30820122300d06092a864886f70d01010105000382010f003082010a0282010100c564999066687e557e86734a655b8252bd1e39e758b45204535ea60113cfdca3ea1c5adc117b9d039ff8ea1a2881f49bd9662a11a09e96d6371a23c6f963dd8610c48b98788489af2fe83f89353b2cb988866931e212b3018c74d76d35d87c72ee9bc4249cba4bbc047098f403136c585a3c4b2b087cee51c39ec24c7a25c7071bb82b1faba09c6b73c4d2073d51767629a4c936ea61f2058f0dd8a8f00bb9627629bc8632d105ede9e505007f21b8d4413942be5c79e0fbfcc0217400b462445bfaf1fef2835169b49f364a9485173c874248c0933baaa3f9416fca977448de0f5d6ffa0d425e1a2ddb5c5aa5f5717ccaccba66085e1cab2f80f0e54a438ee50203010001a348304630090603551d1304023000300b0603551d0f0404030205e0302c0603551d1104253023820a2a2e66697a7a2e636f6d820866697a7a2e636f6d820b6578616d706c652e6e6574300d06092a864886f70d01010b050003820101008a48bf0c71489acb196f08af3e0fa4a2e878a7ad2c25b71d856bacc17c9c62cac25cde58b6b406940deb7f03b832ceb1a1995f43ac86c3ac3c273d156b9bf1576ee69035cee0cb4b4dda2f61780c1332bcabc39aa6b4f89b23f92b88934e78a05d50e23bf1f551342419b1d457c7679520f9ff032d662f2cc37a1bd3fa618ce810d9f9f3da1afff2476160e82629add8807cd11d64e3b808bc675e5b80a794d1f58d83b5fe6af5c951cdae976439a6f622d744c9c753c3cce2fd038646115ebe3711fa9e9cf8d2abdbf3928aff7e2dfbdb68596f771924286af79abb1bd848330752e24874e5940fb24bcf10cf1712461cd279283f6ef2fec6c593c5c1a0d70d000004b3308204af30820297a00302010202021000300d06092a864886f70d01010b0500301a310b3009060355040613025553310b300906035504080c0243413020170d3137303930313231303535325a180f32313137303830383231303535325a3063310b3009060355040613025553310b300906035504080c024341310d300b060355040b0c0446697a7a3114301206035504030c0b46697a7a20436c69656e743122302006092a864886f70d010901161366697a7a636c69656e744066697a7a2e636f6d30820122300d06092a864886f70d01010105000382010f003082010a0282010100cd4e11b945374235fdacc78269560f5f51bc21390692f456d456e8e4ff48ae6c3fb8e7979d3285b49de1fabfa8980eb4edaab29877e3935b83b71c07d8623554421871ed6efaa61288b75f594390d736b969b786853fcf25238d560d3699a68c666c528b92fb7fff71e41e417c879c82eacdfbb5b7f3510dca9d3a6a483709202d3484cbd7e02ef303a1e30a00275a8a720ae11de486e6a3aa7b6730be471fcb53da866a79b10b5e6c8fb555a2d7496d974b88bfe7da0cee37d602cd99656d8462b4f7cf4ab76c488bf4e573423ba6bf97e2f64251ecfa25fd0de9d0c0f8ffc4cf03a78415ff4560fa20b85e9fd912af37bbaa0a32211fdafaefd35a50141c650203010001a381b33081b030090603551d1304023000301106096086480186f84201010404030205a0302106096086480186f842010d04141612436c69656e74204365727469666963617465301d0603551d0e041604144fd33780c836b63319a9c1ab0f4c0fe8269a4903301f0603551d230418301680141f1aa5f942e80856ba9a0bd74434cb32b17e5e19300e0603551d0f0101ff0404030205e0301d0603551d250416301406082b0601050507030206082b06010505070304300d06092a864886f70d01010b05000382020100704671d2974c542864e59db7f5d1dc1ded2c90cfa28f5a9c573ec3efc2b4d31408e4f5fc82dd0ed8e45d0f7e53a7339203906dc533524505f6c7cf2766817fde5acef539d715c7d7d08fc8b7361f9f421dd514183d42889f07fc41f59236fab01217d7b3af618623e3ba93d6eb3664ce39c3ceb384b66498390871d33203d66136d40393fa16184d50e98576093efc4523647bc46a35eed7ac320987d64e1b27d8a97b83b1d184f267749c200292a5a37b41de1e459325391d34b4a9e833695e6332520f308dc3af8f775334e12daaa4c01f1f30ccbf58942b144cba2d1c65e956132fd97f8630a42c5d3055515ad66858fac78a305e644af257b7d79020a6c206853a859462e96ecedd0a6e93ba9b9f27a66863880bc5cc0f0faf753d0ce78ea0b8e63e198be2b163ca6db085496dc2bd91b70121bfd3ec79d01c2388bd351014f3e1493d1f461a6c88e19a2713ce9fa1edf2549fdc0d6f4be4df0f8139732a236735ded87edaa37b020786b85fcb538904d41d0545350aa23ec12b04e5f5c9baf5bb0445717fae230895b5cc518b9a2d319348169ba5be6c1be75ed1369ad60166f45f8458a8fa13356fce1a4e36d6723405ce51862e3ee913bddc1ae43d8d071c39e07bb2c1e40ed6ca76c6f207551930ee4e7889ed9e31233ebf60cc4433c17e042433ab57ffd9d9e549f120224d8076c5b5bdead821f73ac70aab5755dd00000000000001912eda8c96";

  auto psk = fizz::client::deserializePsk(
      fizz::openssl::certificateSerializer(),
      folly::ByteRange(folly::unhexlify(kSerializedPSK)));
  EXPECT_EQ(psk.psk, "SomePsk");
  EXPECT_EQ(psk.secret, "resumptionsecret");
  EXPECT_EQ(psk.type, PskType::Resumption);
  EXPECT_EQ(psk.version, ProtocolVersion::tls_1_3);
  EXPECT_EQ(psk.cipher, CipherSuite::TLS_AES_128_GCM_SHA256);
  EXPECT_EQ(psk.group, NamedGroup::x25519);
  EXPECT_EQ(psk.maxEarlyDataSize, 0);
  EXPECT_EQ(psk.ticketAgeAdd, 0x11111111);
  EXPECT_EQ(psk.alpn, "h2");
  EXPECT_EQ(psk.clientCert->getIdentity(), "Fizz Client");
  EXPECT_EQ(psk.serverCert->getIdentity(), "Fizz");
}

static std::shared_ptr<fizz::Cert> makeMockCertWithIdentity(
    const std::string& ident) {
  auto cert = std::make_shared<fizz::test::MockCert>();
  EXPECT_CALL(*cert, getIdentity()).WillRepeatedly(Return(ident));
  return cert;
}

TEST_F(PskSerializationTest, TestSerializationWithMock) {
  auto psk = getCachedPsk("SomePsk");
  psk.clientCert = makeMockCertWithIdentity("Fizz Client");
  psk.serverCert = makeMockCertWithIdentity("Fizz Server");
  StrictMock<fizz::test::MockCertificateSerialization> mockSerializer;
  EXPECT_CALL(mockSerializer, serialize(_))
      .Times(2)
      .WillRepeatedly(Invoke([](auto&& cert) {
        std::string repr = fmt::format("serialized: {}", cert.getIdentity());
        return folly::IOBuf::copyBuffer(repr);
      }));
  EXPECT_CALL(mockSerializer, deserialize(_))
      .Times(2)
      .WillRepeatedly(Invoke([](folly::ByteRange range) {
        folly::StringPiece sp(
            reinterpret_cast<const char*>(range.data()), range.size());
        EXPECT_NE(sp.find("serialized: "), std::string::npos);
        if (sp.find("Fizz Client") != std::string::npos) {
          return makeMockCertWithIdentity("client cert");
        } else {
          return makeMockCertWithIdentity("server cert");
        }
      }));
  std::string serializedPsk = fizz::client::serializePsk(mockSerializer, psk);
  EXPECT_TRUE(serializedPsk.find("serialized: ") != std::string::npos);
  auto psk2 = fizz::client::deserializePsk(
      mockSerializer, folly::ByteRange(serializedPsk));
  EXPECT_NE(psk2.clientCert, nullptr);
  EXPECT_NE(psk2.serverCert, nullptr);

  EXPECT_EQ(psk2.clientCert->getIdentity(), "client cert");
  EXPECT_EQ(psk2.serverCert->getIdentity(), "server cert");
}

TEST_F(PskSerializationTest, TestSerializationThatFails) {
  auto psk = getCachedPsk("SomePsk");
  psk.clientCert = makeMockCertWithIdentity("Fizz Client");
  psk.serverCert = makeMockCertWithIdentity("Fizz Server");
  StrictMock<fizz::test::MockCertificateSerialization> mockSerializer;
  Sequence s;
  EXPECT_CALL(mockSerializer, serialize(_))
      .InSequence(s)
      .WillOnce(Return(nullptr));
  EXPECT_CALL(mockSerializer, serialize(_))
      .InSequence(s)
      .WillOnce(Throw(std::runtime_error("failed serialization")));

  // When serialization fails, then we expect that deserialize is not called
  EXPECT_CALL(mockSerializer, deserialize(_)).Times(0);
  std::string serializedPsk = fizz::client::serializePsk(mockSerializer, psk);
  auto psk2 = fizz::client::deserializePsk(
      mockSerializer, folly::ByteRange(serializedPsk));
  EXPECT_EQ(psk2.clientCert, nullptr);
  EXPECT_EQ(psk2.serverCert, nullptr);
}

TEST_F(PskSerializationTest, TestDeserializationThatFails) {
  auto psk = getCachedPsk("SomePsk");
  psk.clientCert = makeMockCertWithIdentity("Fizz Client");
  psk.serverCert = makeMockCertWithIdentity("Fizz Server");
  StrictMock<fizz::test::MockCertificateSerialization> mockSerializer;
  EXPECT_CALL(mockSerializer, serialize(_))
      .Times(2)
      .WillRepeatedly(
          Invoke([](auto&&) { return folly::IOBuf::copyBuffer("hello"); }));

  Sequence s;
  EXPECT_CALL(mockSerializer, deserialize(_))
      .InSequence(s)
      .WillOnce(Return(nullptr));
  EXPECT_CALL(mockSerializer, deserialize(_))
      .InSequence(s)
      .WillOnce(Throw(std::runtime_error("failed deserialization")));
  std::string serializedPsk = fizz::client::serializePsk(mockSerializer, psk);
  auto psk2 = fizz::client::deserializePsk(
      mockSerializer, folly::ByteRange(serializedPsk));
  EXPECT_EQ(psk2.clientCert, nullptr);
  EXPECT_EQ(psk2.serverCert, nullptr);
}

TEST_F(PskSerializationTest, TestSerialization) {
  auto psk = getCachedPsk("SomePsk");
  std::string serializedPsk =
      fizz::client::serializePsk(fizz::openssl::certificateSerializer(), psk);
  EXPECT_FALSE(serializedPsk.empty());
  auto psk2 = fizz::client::deserializePsk(
      fizz::openssl::certificateSerializer(), folly::ByteRange(serializedPsk));
  EXPECT_EQ(psk.psk, psk2.psk);
  EXPECT_EQ(psk.secret, psk2.secret);
  EXPECT_EQ(psk.type, psk2.type);
  EXPECT_EQ(psk.version, psk2.version);
  EXPECT_EQ(psk.cipher, psk2.cipher);
  EXPECT_EQ(psk.group, psk2.group);
  EXPECT_EQ(psk.maxEarlyDataSize, psk2.maxEarlyDataSize);
  EXPECT_EQ(psk.ticketAgeAdd, psk2.ticketAgeAdd);
  EXPECT_EQ(psk.alpn, psk2.alpn);
  ASSERT_NE(psk.clientCert, nullptr);
  EXPECT_EQ(psk.clientCert->getIdentity(), "Fizz Client");
  ASSERT_NE(psk.serverCert, nullptr);
  EXPECT_EQ(psk.serverCert->getIdentity(), "Fizz");
  // Time may have slight rounding errors, make sure its negligible
  auto ticketHandshakeTimeDiff =
      std::chrono::duration_cast<std::chrono::milliseconds>(
          psk.ticketHandshakeTime - psk2.ticketHandshakeTime);
  auto ticketExpirationTimeDiff =
      std::chrono::duration_cast<std::chrono::seconds>(
          psk.ticketExpirationTime - psk2.ticketExpirationTime);
  auto ticketIssueTimeDiff =
      std::chrono::duration_cast<std::chrono::milliseconds>(
          psk.ticketIssueTime - psk2.ticketIssueTime);
  EXPECT_EQ(ticketHandshakeTimeDiff.count(), 0);
  EXPECT_EQ(ticketExpirationTimeDiff.count(), 0);
  EXPECT_EQ(ticketIssueTimeDiff.count(), 0);
}

TEST_F(PskSerializationTest, TestTimestampOverflow) {
  // The timestamps for all of the time points are
  // numeric_limits<int64_t>::max()
  constexpr folly::StringPiece kValidPskWithBadTimestamps =
      "0007536f6d6550736b0010726573756d7074696f6e7365637265740304130101001d026832111111117fffffffffffffff7fffffffffffffff0000000000000000000000017fffffffffffffff";
  auto psk = fizz::client::deserializePsk(
      fizz::openssl::certificateSerializer(),
      folly::ByteRange(folly::unhexlify(kValidPskWithBadTimestamps)));

  EXPECT_GT(psk.ticketIssueTime.time_since_epoch().count(), 0);
  EXPECT_GT(psk.ticketExpirationTime.time_since_epoch().count(), 0);
  EXPECT_GT(psk.ticketHandshakeTime.time_since_epoch().count(), 0);
}

TEST_F(PskSerializationTest, TestInvalidSerializationThrows) {
  auto psk = getCachedPsk("SomePsk");
  std::string serializedPsk =
      fizz::client::serializePsk(fizz::openssl::certificateSerializer(), psk);
  EXPECT_FALSE(serializedPsk.empty());
  serializedPsk.insert(10, "HI!");
  ASSERT_ANY_THROW({
    fizz::client::deserializePsk(
        fizz::openssl::certificateSerializer(),
        folly::ByteRange(serializedPsk));
  });
}

} // namespace test
} // namespace client
} // namespace fizz
