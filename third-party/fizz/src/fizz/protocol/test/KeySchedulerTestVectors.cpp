/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#include <folly/portability/GTest.h>

#include <fizz/backend/openssl/OpenSSL.h>
#include <fizz/crypto/KeyDerivation.h>
#include <fizz/protocol/KeyScheduler.h>

#include <folly/String.h>

using namespace folly;

namespace fizz {
namespace test {

// Test vectors from the "Simple 1-RTT Handshake" trace in RFC 8448, Section 3.
// That handshake uses TLS_AES_128_GCM_SHA256, so the key schedule is driven by
// a SHA-256 KeyDerivation. Inputs (the all-zero PSK, the ECDHE shared secret,
// and the transcript hashes) reproduce the intermediate secrets, traffic keys,
// and IVs published in the RFC.
namespace {

// {client} create an ephemeral x25519 key pair -> shared ECDHE secret, used as
// the IKM when extracting the handshake secret.
constexpr StringPiece kEcdheSecret{
    "8bd4054fb55b9d63fdfbacf9f04b9f0d35e6d63f537563efd46272900f89492d"};

// Transcript hash of ClientHello..ServerHello.
constexpr StringPiece kClientHelloServerHelloHash{
    "860c06edc07858ee8e78f0e7428c58edd6b43f2ca3e6e95f02ed063cf0e1cad8"};
// Transcript hash of ClientHello..server Finished.
constexpr StringPiece kClientHelloServerFinishedHash{
    "9608102a0f1ccc6db6250b7b7e417b1a000eaada3daae4777a7686c9ff83df13"};
// Transcript hash of ClientHello..client Finished.
constexpr StringPiece kClientHelloClientFinishedHash{
    "209145a96ee8e2a122ff810047cc952684658d6049e86429426db87c54ad143d"};

// tls13 c hs traffic / tls13 s hs traffic
constexpr StringPiece kClientHandshakeTrafficSecret{
    "b3eddb126e067f35a780b3abf45e2d8f3b1a950738f52e9600746a0e27a55a21"};
constexpr StringPiece kServerHandshakeTrafficSecret{
    "b67b7d690cc16c4e75e54213cb2d37b4e9c912bcded9105d42befd59d391ad38"};

// tls13 c ap traffic / tls13 s ap traffic
constexpr StringPiece kClientAppTrafficSecret{
    "9e40646ce79a7f9dc05af8889bce6552875afa0b06df0087f792ebb7c17504a5"};
constexpr StringPiece kServerAppTrafficSecret{
    "a11af9f05531f856ad47116b45a950328204b4f44bfb6b3a4b4f1f3fcb631643"};

// tls13 exp master / tls13 res master
constexpr StringPiece kExporterMasterSecret{
    "fe22f881176eda18eb8f44529e6792c50c9a3f89452f68d8ae311b4309d3cf50"};
constexpr StringPiece kResumptionMasterSecret{
    "7df235f2031d2a051287d02b0241b0bfdaf86cc856231f2d5aba46c434ec196c"};

// Handshake write traffic key/iv (derived from the handshake traffic secrets).
constexpr StringPiece kServerHandshakeKey{"3fce516009c21727d0f2e4e86ee403bc"};
constexpr StringPiece kServerHandshakeIv{"5d313eb2671276ee13000b30"};
constexpr StringPiece kClientHandshakeKey{"dbfaa693d1762c5b666af5d950258d01"};
constexpr StringPiece kClientHandshakeIv{"5bd3c71b836e0b76bb73265f"};

// Application write traffic key/iv (derived from the app traffic secrets).
constexpr StringPiece kServerAppKey{"9f02283b6c9c07efc26bb9f2ac92e356"};
constexpr StringPiece kServerAppIv{"cf782b88dd83549aadf1e984"};
constexpr StringPiece kClientAppKey{"17422dda596ed5d9acd890e3c63f5051"};
constexpr StringPiece kClientAppIv{"5b78923dee08579033e523d9"};

// tls13 resumption, with the NewSessionTicket nonce as the context.
constexpr StringPiece kTicketNonce{"0000"};
constexpr StringPiece kResumptionSecret{
    "4ecd0eb6ec3b4d87f5d6028f922ca4c5851a277fd41311c9e62d2c9492e1c4f3"};

// SHA-256 of the empty string, used as the transcript hash when deriving
// secrets whose context is an empty message (e.g. the PSK binder key).
constexpr StringPiece kBlankHash{
    "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855"};

// AES-128-GCM key length and TLS 1.3 record IV length.
constexpr size_t kKeyLength = 16;
constexpr size_t kIvLength = 12;

// Test vectors from the "Resumed 0-RTT Handshake" trace in RFC 8448, Section 4.
// This handshake resumes the Section 3 session, so the PSK fed into the key
// schedule is exactly the resumption secret derived there (kResumptionSecret).
namespace zerortt {

// The resumption handshake uses a fresh x25519 key pair; this is the resulting
// ECDHE shared secret, used as the IKM when extracting the handshake secret.
constexpr StringPiece kEcdheSecret{
    "f44194756ff9ec9d25180635d66ea6824c6ab3bf179977be37f723570e7ccb2e"};

// Transcript hash of the ClientHello (including the PSK binder). Used for the
// client early traffic and early exporter secrets.
constexpr StringPiece kClientHelloHash{
    "08ad0fa05d7c7233b1775ba2ff9f4c5b8b59276b7f227f13a976245f5d960913"};
// Transcript hash of ClientHello..ServerHello.
constexpr StringPiece kClientHelloServerHelloHash{
    "f736cb34fe25e701551bee6fd24c1cc7102a7daf9405cb15d97aafe16f757d03"};
// Transcript hash of ClientHello..server Finished.
constexpr StringPiece kClientHelloServerFinishedHash{
    "b0aeffc46a2cfe33114e6fd7d51f9f04b1ca3c497dab08934a774a9d9ad7dbf3"};
// Transcript hash of ClientHello..client Finished.
constexpr StringPiece kClientHelloClientFinishedHash{
    "c3c122e0bd907a4a3ff6112d8fd53dbf89c773d9552e8b6b9d56d361b3a97bf6"};

// res binder (the PSK binder key, derived with an empty transcript hash).
constexpr StringPiece kResumptionPskBinderSecret{
    "69fe131a3bbad5d63c64eebcc30e395b9d8107726a13d074e389dbc8a4e47256"};

// tls13 c e traffic / tls13 e exp master
constexpr StringPiece kClientEarlyTrafficSecret{
    "3fbbe6a60deb66c30a32795aba0eff7eaa10105586e7be5c09678d63b6caab62"};
constexpr StringPiece kEarlyExporterMasterSecret{
    "b2026866610937d7423e5be90862ccf24c0e6091186d34f812089ff5be2ef7df"};

// tls13 c hs traffic / tls13 s hs traffic
constexpr StringPiece kClientHandshakeTrafficSecret{
    "2faac08f851d35fea3604fcb4de82dc62c9b164a70974d0462e27f1ab278700f"};
constexpr StringPiece kServerHandshakeTrafficSecret{
    "fe927ae271312e8bf0275b581c54eef020450dc4ecffaa05a1a35d27518e7803"};

// tls13 c ap traffic / tls13 s ap traffic
constexpr StringPiece kClientAppTrafficSecret{
    "2abbf2b8e381d23dbebe1dd2a7d16a8bf484cb4950d23fb7fb7fa8547062d9a1"};
constexpr StringPiece kServerAppTrafficSecret{
    "cc21f1bf8feb7dd5fa505bd9c4b468a9984d554a993dc49e6d285598fb672691"};

// tls13 exp master / tls13 res master
constexpr StringPiece kExporterMasterSecret{
    "3fd93d4ffddc98e64b14dd107aedf8ee4add23f4510f58a4592d0b201bee56b4"};
constexpr StringPiece kResumptionMasterSecret{
    "5e95bdf1f89005ea2e9aa0ba85e728e3c19c5fe0c699e3f5bee59faebd0b5406"};

// Early data write traffic key/iv (from the client early traffic secret).
constexpr StringPiece kEarlyKey{"920205a5b7bf2115e6fc5c2942834f54"};
constexpr StringPiece kEarlyIv{"6d475f0993c8e564610db2b9"};

// Handshake write traffic key/iv.
constexpr StringPiece kServerHandshakeKey{"27c6bdc0a3dcea39a47326d79bc9e4ee"};
constexpr StringPiece kServerHandshakeIv{"9569ecdd4d0536705e9ef725"};
constexpr StringPiece kClientHandshakeKey{"b1530806f4adfeac83f1413032bbfa82"};
constexpr StringPiece kClientHandshakeIv{"eb50c16be7654abf99dd06d9"};

// Application write traffic key/iv.
constexpr StringPiece kServerAppKey{"e857c690a34c5a9129d833619684f95e"};
constexpr StringPiece kServerAppIv{"0685d6b561aab9ef1013faf9"};
constexpr StringPiece kClientAppKey{"3cf122f301c6358ca7989553250efd72"};
constexpr StringPiece kClientAppIv{"ab1aec26aa78b8fc1176b9ac"};
} // namespace zerortt

std::vector<uint8_t> fromHex(StringPiece hex) {
  auto bytes = unhexlify(hex);
  return std::vector<uint8_t>(bytes.begin(), bytes.end());
}

std::string toHex(ByteRange bytes) {
  return hexlify(bytes);
}
} // namespace

class KeySchedulerTestVectors : public testing::Test {
 protected:
  void SetUp() override {
    ks_ = std::make_unique<KeyScheduler>(
        std::make_unique<KeyDerivationImpl>(openssl::hasherFactory<Sha256>()));
  }

  // Advances the scheduler through the early and handshake secret extractions
  // using the RFC 8448 Section 3 inputs: an all-zero PSK (no pre-shared key)
  // and the sample ECDHE shared secret.
  void deriveThroughHandshakeSecret() {
    const std::vector<uint8_t> zeroPsk(Sha256::HashLen, 0);
    ASSERT_EQ(ks_->deriveEarlySecret(err_, range(zeroPsk)), Status::Success);
    const auto ecdhe = fromHex(kEcdheSecret);
    ASSERT_EQ(ks_->deriveHandshakeSecret(err_, range(ecdhe)), Status::Success);
  }

  Error err_;
  std::unique_ptr<KeyScheduler> ks_;
};

TEST_F(KeySchedulerTestVectors, HandshakeTrafficSecrets) {
  deriveThroughHandshakeSecret();

  const auto transcript = fromHex(kClientHelloServerHelloHash);
  DerivedSecret clientHs;
  DerivedSecret serverHs;
  ASSERT_EQ(
      ks_->getSecret(
          clientHs,
          err_,
          HandshakeSecrets::ClientHandshakeTraffic,
          range(transcript)),
      Status::Success);
  ASSERT_EQ(
      ks_->getSecret(
          serverHs,
          err_,
          HandshakeSecrets::ServerHandshakeTraffic,
          range(transcript)),
      Status::Success);

  EXPECT_EQ(toHex(range(clientHs.secret)), kClientHandshakeTrafficSecret);
  EXPECT_EQ(toHex(range(serverHs.secret)), kServerHandshakeTrafficSecret);
}

TEST_F(KeySchedulerTestVectors, MasterAndAppTrafficSecrets) {
  deriveThroughHandshakeSecret();
  ASSERT_EQ(ks_->deriveMasterSecret(err_), Status::Success);

  const auto serverFinishedHash = fromHex(kClientHelloServerFinishedHash);
  DerivedSecret exporter;
  ASSERT_EQ(
      ks_->getSecret(
          exporter,
          err_,
          MasterSecrets::ExporterMaster,
          range(serverFinishedHash)),
      Status::Success);
  EXPECT_EQ(toHex(range(exporter.secret)), kExporterMasterSecret);

  const auto clientFinishedHash = fromHex(kClientHelloClientFinishedHash);
  DerivedSecret resumption;
  ASSERT_EQ(
      ks_->getSecret(
          resumption,
          err_,
          MasterSecrets::ResumptionMaster,
          range(clientFinishedHash)),
      Status::Success);
  EXPECT_EQ(toHex(range(resumption.secret)), kResumptionMasterSecret);

  ASSERT_EQ(
      ks_->deriveAppTrafficSecrets(err_, range(serverFinishedHash)),
      Status::Success);
  EXPECT_EQ(
      toHex(range(ks_->getSecret(AppTrafficSecrets::ClientAppTraffic).secret)),
      kClientAppTrafficSecret);
  EXPECT_EQ(
      toHex(range(ks_->getSecret(AppTrafficSecrets::ServerAppTraffic).secret)),
      kServerAppTrafficSecret);
}

TEST_F(KeySchedulerTestVectors, TrafficKeys) {
  // getTrafficKey derives from an explicit traffic secret and is independent of
  // the scheduler's internal state, so feed each RFC 8448 traffic secret in
  // directly and check the resulting write key and IV.
  const std::vector<std::tuple<StringPiece, StringPiece, StringPiece>> cases{
      {kServerHandshakeTrafficSecret, kServerHandshakeKey, kServerHandshakeIv},
      {kClientHandshakeTrafficSecret, kClientHandshakeKey, kClientHandshakeIv},
      {kServerAppTrafficSecret, kServerAppKey, kServerAppIv},
      {kClientAppTrafficSecret, kClientAppKey, kClientAppIv},
  };

  for (const auto& [trafficSecret, expectedKey, expectedIv] : cases) {
    const auto secret = fromHex(trafficSecret);
    TrafficKey trafficKey;
    ASSERT_EQ(
        ks_->getTrafficKey(
            trafficKey, err_, range(secret), kKeyLength, kIvLength),
        Status::Success);
    EXPECT_EQ(toHex(trafficKey.key->coalesce()), expectedKey);
    EXPECT_EQ(toHex(trafficKey.iv->coalesce()), expectedIv);
  }
}

TEST_F(KeySchedulerTestVectors, ResumptionSecret) {
  const auto resumptionMaster = fromHex(kResumptionMasterSecret);
  const auto ticketNonce = fromHex(kTicketNonce);
  Buf secret;
  ASSERT_EQ(
      ks_->getResumptionSecret(
          secret, err_, range(resumptionMaster), range(ticketNonce)),
      Status::Success);
  EXPECT_EQ(toHex(secret->coalesce()), kResumptionSecret);
}

// Reproduces the RFC 8448 Section 4 "Resumed 0-RTT Handshake" key schedule.
class KeySchedulerZeroRttTestVectors : public testing::Test {
 protected:
  void SetUp() override {
    ks_ = std::make_unique<KeyScheduler>(
        std::make_unique<KeyDerivationImpl>(openssl::hasherFactory<Sha256>()));
    // The early secret is extracted from the resumption secret carried over
    // from the Section 3 handshake.
    const auto psk = fromHex(kResumptionSecret);
    ASSERT_EQ(ks_->deriveEarlySecret(err_, range(psk)), Status::Success);
  }

  Error err_;
  std::unique_ptr<KeyScheduler> ks_;
};

TEST_F(KeySchedulerZeroRttTestVectors, EarlySecrets) {
  // The PSK binder key uses an empty transcript hash.
  const auto blankHash = fromHex(kBlankHash);
  DerivedSecret binder;
  ASSERT_EQ(
      ks_->getSecret(
          binder, err_, EarlySecrets::ResumptionPskBinder, range(blankHash)),
      Status::Success);
  EXPECT_EQ(toHex(range(binder.secret)), zerortt::kResumptionPskBinderSecret);

  // The early traffic and early exporter secrets use the ClientHello hash.
  const auto clientHelloHash = fromHex(zerortt::kClientHelloHash);
  DerivedSecret clientEarly;
  DerivedSecret earlyExporter;
  ASSERT_EQ(
      ks_->getSecret(
          clientEarly,
          err_,
          EarlySecrets::ClientEarlyTraffic,
          range(clientHelloHash)),
      Status::Success);
  ASSERT_EQ(
      ks_->getSecret(
          earlyExporter,
          err_,
          EarlySecrets::EarlyExporter,
          range(clientHelloHash)),
      Status::Success);
  EXPECT_EQ(
      toHex(range(clientEarly.secret)), zerortt::kClientEarlyTrafficSecret);
  EXPECT_EQ(
      toHex(range(earlyExporter.secret)), zerortt::kEarlyExporterMasterSecret);
}

TEST_F(KeySchedulerZeroRttTestVectors, HandshakeTrafficSecrets) {
  const auto ecdhe = fromHex(zerortt::kEcdheSecret);
  ASSERT_EQ(ks_->deriveHandshakeSecret(err_, range(ecdhe)), Status::Success);

  const auto transcript = fromHex(zerortt::kClientHelloServerHelloHash);
  DerivedSecret clientHs;
  DerivedSecret serverHs;
  ASSERT_EQ(
      ks_->getSecret(
          clientHs,
          err_,
          HandshakeSecrets::ClientHandshakeTraffic,
          range(transcript)),
      Status::Success);
  ASSERT_EQ(
      ks_->getSecret(
          serverHs,
          err_,
          HandshakeSecrets::ServerHandshakeTraffic,
          range(transcript)),
      Status::Success);

  EXPECT_EQ(
      toHex(range(clientHs.secret)), zerortt::kClientHandshakeTrafficSecret);
  EXPECT_EQ(
      toHex(range(serverHs.secret)), zerortt::kServerHandshakeTrafficSecret);
}

TEST_F(KeySchedulerZeroRttTestVectors, MasterAndAppTrafficSecrets) {
  const auto ecdhe = fromHex(zerortt::kEcdheSecret);
  ASSERT_EQ(ks_->deriveHandshakeSecret(err_, range(ecdhe)), Status::Success);
  ASSERT_EQ(ks_->deriveMasterSecret(err_), Status::Success);

  const auto serverFinishedHash =
      fromHex(zerortt::kClientHelloServerFinishedHash);
  DerivedSecret exporter;
  ASSERT_EQ(
      ks_->getSecret(
          exporter,
          err_,
          MasterSecrets::ExporterMaster,
          range(serverFinishedHash)),
      Status::Success);
  EXPECT_EQ(toHex(range(exporter.secret)), zerortt::kExporterMasterSecret);

  const auto clientFinishedHash =
      fromHex(zerortt::kClientHelloClientFinishedHash);
  DerivedSecret resumption;
  ASSERT_EQ(
      ks_->getSecret(
          resumption,
          err_,
          MasterSecrets::ResumptionMaster,
          range(clientFinishedHash)),
      Status::Success);
  EXPECT_EQ(toHex(range(resumption.secret)), zerortt::kResumptionMasterSecret);

  ASSERT_EQ(
      ks_->deriveAppTrafficSecrets(err_, range(serverFinishedHash)),
      Status::Success);
  EXPECT_EQ(
      toHex(range(ks_->getSecret(AppTrafficSecrets::ClientAppTraffic).secret)),
      zerortt::kClientAppTrafficSecret);
  EXPECT_EQ(
      toHex(range(ks_->getSecret(AppTrafficSecrets::ServerAppTraffic).secret)),
      zerortt::kServerAppTrafficSecret);
}

TEST_F(KeySchedulerZeroRttTestVectors, TrafficKeys) {
  const std::vector<std::tuple<StringPiece, StringPiece, StringPiece>> cases{
      {zerortt::kClientEarlyTrafficSecret,
       zerortt::kEarlyKey,
       zerortt::kEarlyIv},
      {zerortt::kServerHandshakeTrafficSecret,
       zerortt::kServerHandshakeKey,
       zerortt::kServerHandshakeIv},
      {zerortt::kClientHandshakeTrafficSecret,
       zerortt::kClientHandshakeKey,
       zerortt::kClientHandshakeIv},
      {zerortt::kServerAppTrafficSecret,
       zerortt::kServerAppKey,
       zerortt::kServerAppIv},
      {zerortt::kClientAppTrafficSecret,
       zerortt::kClientAppKey,
       zerortt::kClientAppIv},
  };

  for (const auto& [trafficSecret, expectedKey, expectedIv] : cases) {
    const auto secret = fromHex(trafficSecret);
    TrafficKey trafficKey;
    ASSERT_EQ(
        ks_->getTrafficKey(
            trafficKey, err_, range(secret), kKeyLength, kIvLength),
        Status::Success);
    EXPECT_EQ(toHex(trafficKey.key->coalesce()), expectedKey);
    EXPECT_EQ(toHex(trafficKey.iv->coalesce()), expectedIv);
  }
}

} // namespace test
} // namespace fizz
