/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#include <folly/portability/GTest.h>

#include <fizz/protocol/KeyScheduler.h>

#include <fizz/crypto/test/Mocks.h>

using namespace folly;
using namespace testing;

namespace fizz {
namespace test {

class KeySchedulerTest : public testing::Test {
 public:
  void SetUp() override {
    auto kd = std::make_unique<MockKeyDerivation>();
    kd_ = kd.get();
    ON_CALL(*kd_, hashLength()).WillByDefault(Return(4));
    ON_CALL(*kd_, _expandLabel(_, _, _, _))
        .WillByDefault(InvokeWithoutArgs([]() { return IOBuf::create(0); }));
    ks_ = std::make_unique<KeyScheduler>(std::move(kd));
  }

 protected:
  StringPiece transcript_{"hash"};
  MockKeyDerivation* kd_;
  std::unique_ptr<KeyScheduler> ks_;
};

TEST_F(KeySchedulerTest, TestEarly) {
  StringPiece psk{"psk"};
  EXPECT_CALL(*kd_, hkdfExtract(_, _));
  Error err;
  EXPECT_EQ(ks_->deriveEarlySecret(err, psk), Status::Success);
  EXPECT_CALL(*kd_, _deriveSecret(_, _, _, _)).Times(4);
  DerivedSecret secret1, secret2, secret3, secret4;
  EXPECT_EQ(
      ks_->getSecret(
          secret1, err, EarlySecrets::ExternalPskBinder, transcript_),
      Status::Success);
  EXPECT_EQ(
      ks_->getSecret(
          secret2, err, EarlySecrets::ResumptionPskBinder, transcript_),
      Status::Success);
  EXPECT_EQ(
      ks_->getSecret(
          secret3, err, EarlySecrets::ResumptionPskBinder, transcript_),
      Status::Success);
  EXPECT_EQ(
      ks_->getSecret(secret4, err, EarlySecrets::EarlyExporter, transcript_),
      Status::Success);

  EXPECT_CALL(*kd_, _deriveSecret(_, _, _, _)).Times(1);
  EXPECT_CALL(*kd_, hkdfExtract(_, _));
  EXPECT_EQ(ks_->deriveHandshakeSecret(err), Status::Success);
  EXPECT_CALL(*kd_, _deriveSecret(_, _, _, _)).Times(2);
  DerivedSecret hsSecret1, hsSecret2;
  EXPECT_EQ(
      ks_->getSecret(
          hsSecret1,
          err,
          HandshakeSecrets::ClientHandshakeTraffic,
          transcript_),
      Status::Success);
  EXPECT_EQ(
      ks_->getSecret(
          hsSecret2,
          err,
          HandshakeSecrets::ServerHandshakeTraffic,
          transcript_),
      Status::Success);

  EXPECT_CALL(*kd_, _deriveSecret(_, _, _, _)).Times(1);
  EXPECT_CALL(*kd_, hkdfExtract(_, _));
  EXPECT_EQ(ks_->deriveMasterSecret(err), Status::Success);
  EXPECT_CALL(*kd_, _deriveSecret(_, _, _, _)).Times(2);
  DerivedSecret msSecret1, msSecret2;
  EXPECT_EQ(
      ks_->getSecret(
          msSecret1, err, MasterSecrets::ExporterMaster, transcript_),
      Status::Success);
  EXPECT_EQ(
      ks_->getSecret(
          msSecret2, err, MasterSecrets::ResumptionMaster, transcript_),
      Status::Success);

  EXPECT_CALL(*kd_, _deriveSecret(_, _, _, _)).Times(2);
  EXPECT_EQ(ks_->deriveAppTrafficSecrets(err, transcript_), Status::Success);
  ks_->getSecret(AppTrafficSecrets::ClientAppTraffic);
  ks_->getSecret(AppTrafficSecrets::ServerAppTraffic);
}

TEST_F(KeySchedulerTest, TestEarlyEcdhe) {
  StringPiece psk{"psk"};
  EXPECT_CALL(*kd_, hkdfExtract(_, _));
  Error err;
  EXPECT_EQ(ks_->deriveEarlySecret(err, psk), Status::Success);
  EXPECT_CALL(*kd_, _deriveSecret(_, _, _, _)).Times(4);
  DerivedSecret secret1, secret2, secret3, secret4;
  EXPECT_EQ(
      ks_->getSecret(
          secret1, err, EarlySecrets::ExternalPskBinder, transcript_),
      Status::Success);
  EXPECT_EQ(
      ks_->getSecret(
          secret2, err, EarlySecrets::ResumptionPskBinder, transcript_),
      Status::Success);
  EXPECT_EQ(
      ks_->getSecret(
          secret3, err, EarlySecrets::ResumptionPskBinder, transcript_),
      Status::Success);
  EXPECT_EQ(
      ks_->getSecret(secret4, err, EarlySecrets::EarlyExporter, transcript_),
      Status::Success);

  StringPiece ecdhe{"ecdhe"};
  EXPECT_CALL(*kd_, _deriveSecret(_, _, _, _)).Times(1);
  EXPECT_CALL(*kd_, hkdfExtract(_, _));
  EXPECT_EQ(ks_->deriveHandshakeSecret(err, ecdhe), Status::Success);
  EXPECT_CALL(*kd_, _deriveSecret(_, _, _, _)).Times(2);
  DerivedSecret hsSecret1, hsSecret2;
  EXPECT_EQ(
      ks_->getSecret(
          hsSecret1,
          err,
          HandshakeSecrets::ClientHandshakeTraffic,
          transcript_),
      Status::Success);
  EXPECT_EQ(
      ks_->getSecret(
          hsSecret2,
          err,
          HandshakeSecrets::ServerHandshakeTraffic,
          transcript_),
      Status::Success);

  EXPECT_CALL(*kd_, _deriveSecret(_, _, _, _)).Times(1);
  EXPECT_CALL(*kd_, hkdfExtract(_, _));
  EXPECT_EQ(ks_->deriveMasterSecret(err), Status::Success);
  EXPECT_CALL(*kd_, _deriveSecret(_, _, _, _)).Times(2);
  DerivedSecret msSecret1, msSecret2;
  EXPECT_EQ(
      ks_->getSecret(
          msSecret1, err, MasterSecrets::ExporterMaster, transcript_),
      Status::Success);
  EXPECT_EQ(
      ks_->getSecret(
          msSecret2, err, MasterSecrets::ResumptionMaster, transcript_),
      Status::Success);

  EXPECT_CALL(*kd_, _deriveSecret(_, _, _, _)).Times(2);
  EXPECT_EQ(ks_->deriveAppTrafficSecrets(err, transcript_), Status::Success);
  ks_->getSecret(AppTrafficSecrets::ClientAppTraffic);
  ks_->getSecret(AppTrafficSecrets::ServerAppTraffic);
}

TEST_F(KeySchedulerTest, TestNoEarly) {
  StringPiece ecdhe{"ecdhe"};
  EXPECT_CALL(*kd_, _deriveSecret(_, _, _, _)).Times(1);
  EXPECT_CALL(*kd_, hkdfExtract(_, _)).Times(2);
  Error err;
  EXPECT_EQ(ks_->deriveHandshakeSecret(err, ecdhe), Status::Success);
  EXPECT_CALL(*kd_, _deriveSecret(_, _, _, _)).Times(2);
  DerivedSecret hsSecret1, hsSecret2;
  EXPECT_EQ(
      ks_->getSecret(
          hsSecret1,
          err,
          HandshakeSecrets::ClientHandshakeTraffic,
          transcript_),
      Status::Success);
  EXPECT_EQ(
      ks_->getSecret(
          hsSecret2,
          err,
          HandshakeSecrets::ServerHandshakeTraffic,
          transcript_),
      Status::Success);

  EXPECT_CALL(*kd_, _deriveSecret(_, _, _, _)).Times(1);
  EXPECT_CALL(*kd_, hkdfExtract(_, _));
  EXPECT_EQ(ks_->deriveMasterSecret(err), Status::Success);
  EXPECT_CALL(*kd_, _deriveSecret(_, _, _, _)).Times(2);
  DerivedSecret msSecret1, msSecret2;
  EXPECT_EQ(
      ks_->getSecret(
          msSecret1, err, MasterSecrets::ExporterMaster, transcript_),
      Status::Success);
  EXPECT_EQ(
      ks_->getSecret(
          msSecret2, err, MasterSecrets::ResumptionMaster, transcript_),
      Status::Success);

  EXPECT_CALL(*kd_, _deriveSecret(_, _, _, _)).Times(2);
  EXPECT_EQ(ks_->deriveAppTrafficSecrets(err, transcript_), Status::Success);
  ks_->getSecret(AppTrafficSecrets::ClientAppTraffic);
  ks_->getSecret(AppTrafficSecrets::ServerAppTraffic);
}

TEST_F(KeySchedulerTest, TestKeyUpdate) {
  StringPiece ecdhe{"ecdhe"};
  Error err;
  EXPECT_EQ(ks_->deriveHandshakeSecret(err, ecdhe), Status::Success);
  EXPECT_EQ(ks_->deriveMasterSecret(err), Status::Success);
  EXPECT_EQ(ks_->deriveAppTrafficSecrets(err, transcript_), Status::Success);

  uint32_t clientUpdate;
  EXPECT_CALL(*kd_, _expandLabel(_, _, _, _));
  EXPECT_EQ(ks_->clientKeyUpdate(clientUpdate, err), Status::Success);
  EXPECT_EQ(clientUpdate, 1);
  EXPECT_CALL(*kd_, _expandLabel(_, _, _, _));
  EXPECT_EQ(ks_->clientKeyUpdate(clientUpdate, err), Status::Success);
  EXPECT_EQ(clientUpdate, 2);

  uint32_t serverUpdate;
  EXPECT_CALL(*kd_, _expandLabel(_, _, _, _));
  EXPECT_EQ(ks_->serverKeyUpdate(serverUpdate, err), Status::Success);
  EXPECT_EQ(serverUpdate, 1);
  EXPECT_CALL(*kd_, _expandLabel(_, _, _, _));
  EXPECT_EQ(ks_->serverKeyUpdate(serverUpdate, err), Status::Success);
  EXPECT_EQ(serverUpdate, 2);
}

TEST_F(KeySchedulerTest, TestTrafficKey) {
  EXPECT_CALL(*kd_, _expandLabel(_, _, _, _)).Times(2);
  StringPiece trafficSecret{"secret"};
  Error err;
  TrafficKey tk;
  EXPECT_EQ(
      ks_->getTrafficKey(tk, err, trafficSecret, 10, 10), Status::Success);
}

TEST_F(KeySchedulerTest, TestTrafficKeyWithLabel) {
  StringPiece trafficSecret{"secret"};
  StringPiece keyLabel{"fookey"};
  StringPiece ivLabel{"fooiv"};

  InSequence seq;
  EXPECT_CALL(*kd_, _expandLabel(_, _, _, _))
      .WillOnce(
          Invoke([&](auto secret, auto label, const auto&, const auto& len) {
            EXPECT_EQ(folly::hexlify(trafficSecret), folly::hexlify(secret));
            EXPECT_EQ(folly::hexlify(label), folly::hexlify(keyLabel));
            auto res = IOBuf::create(len);
            res->append(len);
            return res;
          }));
  EXPECT_CALL(*kd_, _expandLabel(_, _, _, _))
      .WillOnce(
          Invoke([&](auto secret, auto label, const auto&, const auto& len) {
            EXPECT_EQ(folly::hexlify(trafficSecret), folly::hexlify(secret));
            EXPECT_EQ(folly::hexlify(label), folly::hexlify(ivLabel));
            auto res = IOBuf::create(len);
            res->append(len);
            return res;
          }));
  Error err;
  TrafficKey tk;
  EXPECT_EQ(
      ks_->getTrafficKeyWithLabel(
          tk, err, trafficSecret, keyLabel, ivLabel, 10, 10),
      Status::Success);
}

TEST_F(KeySchedulerTest, TestClonability) {
  StringPiece ecdhe{"ecdhe"};

  MockKeyDerivation* newKeyDerivation = nullptr;
  EXPECT_CALL(*kd_, clone()).WillOnce(InvokeWithoutArgs([&]() {
    auto kd = std::make_unique<MockKeyDerivation>();
    newKeyDerivation = kd.get();
    return kd;
  }));
  EXPECT_CALL(*kd_, _deriveSecret(_, _, _, _)).Times(1);
  Error err;
  EXPECT_EQ(ks_->deriveHandshakeSecret(err, ecdhe), Status::Success);

  auto cloned = ks_->clone();
  ASSERT_NE(newKeyDerivation, nullptr);

  StringPiece transcript1("transcript1");
  StringPiece transcript2("transcript1");
  EXPECT_CALL(
      *newKeyDerivation,
      _deriveSecret(_, _, Eq(folly::ByteRange(transcript2)), _))
      .Times(2);
  EXPECT_CALL(*kd_, _deriveSecret(_, _, Eq(folly::ByteRange(transcript1)), _))
      .Times(2);

  DerivedSecret t1sh, t1ch, t2sh, t2ch;
  EXPECT_EQ(
      ks_->getSecret(
          t1sh, err, HandshakeSecrets::ServerHandshakeTraffic, transcript1),
      Status::Success);
  EXPECT_EQ(
      ks_->getSecret(
          t1ch, err, HandshakeSecrets::ClientHandshakeTraffic, transcript1),
      Status::Success);
  Error err2;
  EXPECT_EQ(
      cloned->getSecret(
          t2sh, err2, HandshakeSecrets::ServerHandshakeTraffic, transcript2),
      Status::Success);
  EXPECT_EQ(
      cloned->getSecret(
          t2ch, err2, HandshakeSecrets::ClientHandshakeTraffic, transcript2),
      Status::Success);
  EXPECT_EQ(t1sh, t2sh);
  EXPECT_EQ(t1ch, t2ch);
}

} // namespace test
} // namespace fizz
