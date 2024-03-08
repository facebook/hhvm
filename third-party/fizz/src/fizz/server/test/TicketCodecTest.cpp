/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#include <folly/io/async/ssl/OpenSSLTransportCertificate.h>
#include <folly/portability/GMock.h>
#include <folly/portability/GTest.h>

#include <fizz/server/TicketCodec.h>

#include <fizz/crypto/test/TestUtil.h>
#include <fizz/protocol/test/Mocks.h>
#include <fizz/server/test/Mocks.h>

static constexpr folly::StringPiece ticket{
    "03041301000673656372657400056964656e74004444444400000000000000190268320000000000000000000f"};
static constexpr folly::StringPiece ticketWithAppTokenAndHandshakeTime{
    "03041301000673656372657400056964656e7400444444440000000000000019026832000b68656c6c6f20776f726c64000000000000000f"};
static constexpr folly::StringPiece ticketWithAppToken{
    "03041301000673656372657400056964656e7400444444440000000000000019026832000b68656c6c6f20776f726c64"};
static constexpr folly::StringPiece ticketWithoutAppToken{
    "03041301000673656372657400056964656e7400444444440000000000000019026832"};
static constexpr folly::StringPiece ticketNoAlpn{
    "03041301000673656372657400056964656e7400444444440000000000000019000000000000000000000f"};
static constexpr folly::StringPiece ticketClientAuthX509{
    "03041301000673656372657400056964656e740103653082036130820249a003020102020900c3420836ac1ca26f300d06092a864886f70d01010b0500304b310b3009060355040613025553310b300906035504080c024e593111300f06035504070c084e657720596f726b310d300b060355040b0c0446697a7a310d300b06035504030c0446697a7a301e170d3136313232393036323431385a170d3431303832303036323431385a304b310b3009060355040613025553310b300906035504080c024e593111300f06035504070c084e657720596f726b310d300b060355040b0c0446697a7a310d300b06035504030c0446697a7a30820122300d06092a864886f70d01010105000382010f003082010a0282010100c564999066687e557e86734a655b8252bd1e39e758b45204535ea60113cfdca3ea1c5adc117b9d039ff8ea1a2881f49bd9662a11a09e96d6371a23c6f963dd8610c48b98788489af2fe83f89353b2cb988866931e212b3018c74d76d35d87c72ee9bc4249cba4bbc047098f403136c585a3c4b2b087cee51c39ec24c7a25c7071bb82b1faba09c6b73c4d2073d51767629a4c936ea61f2058f0dd8a8f00bb9627629bc8632d105ede9e505007f21b8d4413942be5c79e0fbfcc0217400b462445bfaf1fef2835169b49f364a9485173c874248c0933baaa3f9416fca977448de0f5d6ffa0d425e1a2ddb5c5aa5f5717ccaccba66085e1cab2f80f0e54a438ee50203010001a348304630090603551d1304023000300b0603551d0f0404030205e0302c0603551d1104253023820a2a2e66697a7a2e636f6d820866697a7a2e636f6d820b6578616d706c652e6e6574300d06092a864886f70d01010b050003820101008a48bf0c71489acb196f08af3e0fa4a2e878a7ad2c25b71d856bacc17c9c62cac25cde58b6b406940deb7f03b832ceb1a1995f43ac86c3ac3c273d156b9bf1576ee69035cee0cb4b4dda2f61780c1332bcabc39aa6b4f89b23f92b88934e78a05d50e23bf1f551342419b1d457c7679520f9ff032d662f2cc37a1bd3fa618ce810d9f9f3da1afff2476160e82629add8807cd11d64e3b808bc675e5b80a794d1f58d83b5fe6af5c951cdae976439a6f622d744c9c753c3cce2fd038646115ebe3711fa9e9cf8d2abdbf3928aff7e2dfbdb68596f771924286af79abb1bd848330752e24874e5940fb24bcf10cf1712461cd279283f6ef2fec6c593c5c1a0d70d4444444400000000000000190268320000000000000000000f"};
static constexpr folly::StringPiece ticketClientAuthIdentityOnly{
    "03041301000673656372657400056964656e74020008636c69656e7469644444444400000000000000190268320000000000000000000f"};

namespace fizz {
namespace server {
namespace test {

static ResumptionState getTestResumptionState(
    std::shared_ptr<SelfCert> cert,
    std::shared_ptr<PeerCert> peerCert) {
  ResumptionState rs;
  rs.version = ProtocolVersion::tls_1_3;
  rs.cipher = CipherSuite::TLS_AES_128_GCM_SHA256;
  rs.resumptionSecret = folly::IOBuf::copyBuffer("secret");
  rs.serverCert = cert;
  rs.clientCert = peerCert;
  rs.ticketAgeAdd = 0x44444444;
  rs.ticketIssueTime = std::chrono::time_point<std::chrono::system_clock>(
      std::chrono::seconds(25));
  rs.alpn = "h2";
  rs.handshakeTime = std::chrono::time_point<std::chrono::system_clock>(
      std::chrono::seconds(15));
  return rs;
}

static ResumptionState x509Decode(Buf encoded) {
  OpenSSLFactory factory;
  CertManager certManager;
  return TicketCodec<CertificateStorage::X509>::decode(
      std::move(encoded), factory, certManager);
}

TEST(TicketCodecTest, TestEncode) {
  auto cert = std::make_shared<MockSelfCert>();
  auto rs = getTestResumptionState(cert, nullptr);
  EXPECT_CALL(*cert, getIdentity()).WillOnce(Return("ident"));
  EXPECT_FALSE(rs.appToken);
  auto encoded = TicketCodec<CertificateStorage::X509>::encode(std::move(rs));
  EXPECT_TRUE(folly::IOBufEqualTo()(encoded, toIOBuf(ticket)))
      << folly::hexlify(encoded->coalesce());
}

TEST(TicketCodecTest, TestEncodeWithAppTokenAndTime) {
  auto cert = std::make_shared<MockSelfCert>();
  auto rs = getTestResumptionState(cert, nullptr);
  rs.appToken = folly::IOBuf::copyBuffer("hello world");
  EXPECT_CALL(*cert, getIdentity()).WillOnce(Return("ident"));
  auto encoded = TicketCodec<CertificateStorage::X509>::encode(std::move(rs));
  EXPECT_TRUE(folly::IOBufEqualTo()(
      encoded, toIOBuf(ticketWithAppTokenAndHandshakeTime)))
      << folly::hexlify(encoded->coalesce());
}

TEST(TicketCodecTest, TestEncodeNoAlpn) {
  auto cert = std::make_shared<MockSelfCert>();
  auto rs = getTestResumptionState(cert, nullptr);
  rs.alpn = folly::none;
  EXPECT_CALL(*cert, getIdentity()).WillOnce(Return("ident"));
  auto encoded = TicketCodec<CertificateStorage::X509>::encode(std::move(rs));
  EXPECT_TRUE(folly::IOBufEqualTo()(encoded, toIOBuf(ticketNoAlpn)))
      << folly::hexlify(encoded->coalesce());
}

TEST(TicketCodecTest, TestEncodeClientAuthX509) {
  auto cert = std::make_shared<MockSelfCert>();
  auto peerCert = std::make_shared<MockPeerCert>();
  auto rs = getTestResumptionState(cert, peerCert);
  EXPECT_CALL(*cert, getIdentity()).WillOnce(Return("ident"));
  EXPECT_CALL(*peerCert, getX509()).Times(2).WillRepeatedly(Invoke([]() {
    return getCert(kRSACertificate);
  }));
  auto encoded = TicketCodec<CertificateStorage::X509>::encode(std::move(rs));
  EXPECT_TRUE(folly::IOBufEqualTo()(encoded, toIOBuf(ticketClientAuthX509)))
      << folly::hexlify(encoded->coalesce());
}

TEST(TicketCodecTest, TestEncodeClientAuthIdentityOnly) {
  auto cert = std::make_shared<MockSelfCert>();
  auto peerCert = std::make_shared<MockPeerCert>();
  auto rs = getTestResumptionState(cert, peerCert);
  EXPECT_CALL(*cert, getIdentity()).WillOnce(Return("ident"));
  EXPECT_CALL(*peerCert, getIdentity()).WillOnce(Return("clientid"));
  auto encoded =
      TicketCodec<CertificateStorage::IdentityOnly>::encode(std::move(rs));
  EXPECT_TRUE(
      folly::IOBufEqualTo()(encoded, toIOBuf(ticketClientAuthIdentityOnly)))
      << folly::hexlify(encoded->coalesce());
}

TEST(TicketCodecTest, TestFactoryCert) {
  auto cert = std::make_shared<MockSelfCert>();
  auto peerCert = std::make_shared<MockPeerCert>();
  auto rs = getTestResumptionState(cert, peerCert);
  EXPECT_CALL(*cert, getIdentity()).WillOnce(Return("ident"));
  EXPECT_CALL(*peerCert, getX509()).Times(2).WillRepeatedly(Invoke([]() {
    return getCert(kRSACertificate);
  }));
  auto factory = std::make_unique<MockFactory>();
  auto certManager = std::make_unique<MockCertManager>();
  auto factoryCert = std::make_shared<MockPeerCert>();
  EXPECT_CALL(*factory, _makePeerCert(_, _)).WillOnce(Return(factoryCert));
  EXPECT_CALL(*factoryCert, getIdentity()).WillOnce(Return("factory clientid"));
  EXPECT_CALL(*certManager, getCert(_)).WillOnce(Return(nullptr));
  auto encoded = TicketCodec<CertificateStorage::X509>::encode(std::move(rs));
  auto drs = TicketCodec<CertificateStorage::X509>::decode(
      std::move(encoded), *factory, *certManager);
  EXPECT_TRUE(drs.clientCert);
  EXPECT_EQ(drs.clientCert->getIdentity(), "factory clientid");
}

TEST(TicketCodecTest, TestEncodeNoX509) {
  auto cert = std::make_shared<MockSelfCert>();
  auto peerCert = std::make_shared<MockPeerCert>();
  auto rs = getTestResumptionState(cert, peerCert);
  EXPECT_CALL(*cert, getIdentity()).WillOnce(Return("ident"));
  EXPECT_CALL(*peerCert, getX509()).WillOnce(Invoke([]() { return nullptr; }));
  EXPECT_CALL(*peerCert, getIdentity()).WillOnce(Return("clientid"));
  auto encoded = TicketCodec<CertificateStorage::X509>::encode(std::move(rs));
  auto drs = x509Decode(std::move(encoded));
  EXPECT_TRUE(drs.clientCert);
  EXPECT_EQ(drs.clientCert->getIdentity(), "clientid");
  EXPECT_EQ(
      dynamic_cast<const folly::OpenSSLTransportCertificate*>(
          drs.clientCert.get()),
      nullptr);
}

TEST(TicketCodecTest, TestDecodeDifferentStorage) {
  auto cert = std::make_shared<MockSelfCert>();
  auto peerCert = std::make_shared<MockPeerCert>();
  auto rs = getTestResumptionState(cert, peerCert);
  EXPECT_CALL(*cert, getIdentity()).WillOnce(Return("ident"));
  EXPECT_CALL(*peerCert, getX509()).Times(2).WillRepeatedly(Invoke([]() {
    return getCert(kRSACertificate);
  }));
  auto encoded = TicketCodec<CertificateStorage::X509>::encode(std::move(rs));
  auto drs = x509Decode(std::move(encoded));
  EXPECT_TRUE(drs.clientCert);
  EXPECT_EQ(drs.clientCert->getIdentity(), "Fizz");
  auto opensslCert = dynamic_cast<const folly::OpenSSLTransportCertificate*>(
      drs.clientCert.get());
  EXPECT_NE(opensslCert, nullptr);
  EXPECT_NE(opensslCert->getX509(), nullptr);

  rs = getTestResumptionState(cert, peerCert);
  EXPECT_CALL(*cert, getIdentity()).WillOnce(Return("ident"));
  EXPECT_CALL(*peerCert, getIdentity()).WillOnce(Return("FizzIdOnly"));
  auto encodedIdOnly =
      TicketCodec<CertificateStorage::IdentityOnly>::encode(std::move(rs));
  auto drsX509 = x509Decode(std::move(encodedIdOnly));
  EXPECT_TRUE(drsX509.clientCert);
  EXPECT_EQ(drsX509.clientCert->getIdentity(), "FizzIdOnly");
  EXPECT_EQ(
      dynamic_cast<const folly::OpenSSLTransportCertificate*>(
          drsX509.clientCert.get()),
      nullptr);
}

TEST(TicketCodecTest, TestDecode) {
  auto rs = x509Decode(toIOBuf(ticketClientAuthX509));
  EXPECT_EQ(rs.version, ProtocolVersion::tls_1_3);
  EXPECT_EQ(rs.cipher, CipherSuite::TLS_AES_128_GCM_SHA256);
  EXPECT_TRUE(folly::IOBufEqualTo()(
      rs.resumptionSecret, folly::IOBuf::copyBuffer("secret")));
  EXPECT_EQ(rs.ticketAgeAdd, 0x44444444);
  EXPECT_EQ(
      rs.ticketIssueTime,
      std::chrono::time_point<std::chrono::system_clock>(
          std::chrono::seconds(25)));
  EXPECT_EQ(*rs.alpn, "h2");
  EXPECT_TRUE(rs.clientCert);
  EXPECT_EQ(rs.clientCert->getIdentity(), "Fizz");
  EXPECT_EQ(
      rs.handshakeTime,
      std::chrono::time_point<std::chrono::system_clock>(
          std::chrono::seconds(15)));
}

TEST(TicketCodecTest, TestDecodeNoAlpn) {
  auto rs = x509Decode(toIOBuf(ticketNoAlpn));
  EXPECT_FALSE(rs.alpn.has_value());
}

TEST(TicketCodecTest, TestDecodeTooShort) {
  auto buf = toIOBuf(ticket);
  buf->trimEnd(1);
  EXPECT_THROW(x509Decode(std::move(buf)), std::exception);
}

TEST(TicketCodecTest, TestDecodeWithAppToken) {
  auto rs = x509Decode(toIOBuf(ticketWithAppToken));
  EXPECT_EQ(rs.version, ProtocolVersion::tls_1_3);
  EXPECT_EQ(rs.cipher, CipherSuite::TLS_AES_128_GCM_SHA256);
  EXPECT_TRUE(folly::IOBufEqualTo()(
      rs.resumptionSecret, folly::IOBuf::copyBuffer("secret")));
  EXPECT_EQ(rs.ticketAgeAdd, 0x44444444);
  EXPECT_EQ(
      rs.ticketIssueTime,
      std::chrono::time_point<std::chrono::system_clock>(
          std::chrono::seconds(25)));
  EXPECT_EQ(*rs.alpn, "h2");
  EXPECT_TRUE(folly::IOBufEqualTo()(
      rs.appToken, folly::IOBuf::copyBuffer("hello world")));
  EXPECT_EQ(
      rs.handshakeTime,
      std::chrono::time_point<std::chrono::system_clock>(
          std::chrono::seconds(25)));
}

TEST(TicketCodecTest, TestDecodeWithAppTokenAndTime) {
  auto rs = x509Decode(toIOBuf(ticketWithAppTokenAndHandshakeTime));
  EXPECT_EQ(rs.version, ProtocolVersion::tls_1_3);
  EXPECT_EQ(rs.cipher, CipherSuite::TLS_AES_128_GCM_SHA256);
  EXPECT_TRUE(folly::IOBufEqualTo()(
      rs.resumptionSecret, folly::IOBuf::copyBuffer("secret")));
  EXPECT_EQ(rs.ticketAgeAdd, 0x44444444);
  EXPECT_EQ(
      rs.ticketIssueTime,
      std::chrono::time_point<std::chrono::system_clock>(
          std::chrono::seconds(25)));
  EXPECT_EQ(*rs.alpn, "h2");
  EXPECT_TRUE(folly::IOBufEqualTo()(
      rs.appToken, folly::IOBuf::copyBuffer("hello world")));
  EXPECT_EQ(
      rs.handshakeTime,
      std::chrono::time_point<std::chrono::system_clock>(
          std::chrono::seconds(15)));
}

TEST(TicketCodecTest, TestDecodeWithEmptyAppToken) {
  auto rs = x509Decode(toIOBuf(ticket));
  EXPECT_EQ(rs.version, ProtocolVersion::tls_1_3);
  EXPECT_EQ(rs.cipher, CipherSuite::TLS_AES_128_GCM_SHA256);
  EXPECT_TRUE(folly::IOBufEqualTo()(
      rs.resumptionSecret, folly::IOBuf::copyBuffer("secret")));
  EXPECT_EQ(rs.ticketAgeAdd, 0x44444444);
  EXPECT_EQ(
      rs.ticketIssueTime,
      std::chrono::time_point<std::chrono::system_clock>(
          std::chrono::seconds(25)));
  EXPECT_EQ(*rs.alpn, "h2");
  EXPECT_EQ(
      rs.handshakeTime,
      std::chrono::time_point<std::chrono::system_clock>(
          std::chrono::seconds(15)));
}

TEST(TicketCodecTest, TestDecodeWithoutAppToken) {
  auto rs = x509Decode(toIOBuf(ticketWithoutAppToken));
  EXPECT_EQ(rs.version, ProtocolVersion::tls_1_3);
  EXPECT_EQ(rs.cipher, CipherSuite::TLS_AES_128_GCM_SHA256);
  EXPECT_TRUE(folly::IOBufEqualTo()(
      rs.resumptionSecret, folly::IOBuf::copyBuffer("secret")));
  EXPECT_EQ(rs.ticketAgeAdd, 0x44444444);
  EXPECT_EQ(
      rs.ticketIssueTime,
      std::chrono::time_point<std::chrono::system_clock>(
          std::chrono::seconds(25)));
  EXPECT_EQ(*rs.alpn, "h2");
  EXPECT_EQ(
      rs.handshakeTime,
      std::chrono::time_point<std::chrono::system_clock>(
          std::chrono::seconds(25)));
}
} // namespace test
} // namespace server
} // namespace fizz
