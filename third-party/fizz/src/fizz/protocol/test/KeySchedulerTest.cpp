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
  ks_->deriveEarlySecret(psk);
  EXPECT_CALL(*kd_, deriveSecret(_, _, _, _)).Times(4);
  ks_->getSecret(EarlySecrets::ExternalPskBinder, transcript_);
  ks_->getSecret(EarlySecrets::ResumptionPskBinder, transcript_);
  ks_->getSecret(EarlySecrets::ResumptionPskBinder, transcript_);
  ks_->getSecret(EarlySecrets::EarlyExporter, transcript_);

  EXPECT_CALL(*kd_, deriveSecret(_, _, _, _)).Times(1);
  EXPECT_CALL(*kd_, hkdfExtract(_, _));
  ks_->deriveHandshakeSecret();
  EXPECT_CALL(*kd_, deriveSecret(_, _, _, _)).Times(2);
  ks_->getSecret(HandshakeSecrets::ClientHandshakeTraffic, transcript_);
  ks_->getSecret(HandshakeSecrets::ServerHandshakeTraffic, transcript_);

  EXPECT_CALL(*kd_, deriveSecret(_, _, _, _)).Times(1);
  EXPECT_CALL(*kd_, hkdfExtract(_, _));
  ks_->deriveMasterSecret();
  EXPECT_CALL(*kd_, deriveSecret(_, _, _, _)).Times(2);
  ks_->getSecret(MasterSecrets::ExporterMaster, transcript_);
  ks_->getSecret(MasterSecrets::ResumptionMaster, transcript_);

  EXPECT_CALL(*kd_, deriveSecret(_, _, _, _)).Times(2);
  ks_->deriveAppTrafficSecrets(transcript_);
  ks_->getSecret(AppTrafficSecrets::ClientAppTraffic);
  ks_->getSecret(AppTrafficSecrets::ServerAppTraffic);
}

TEST_F(KeySchedulerTest, TestEarlyEcdhe) {
  StringPiece psk{"psk"};
  EXPECT_CALL(*kd_, hkdfExtract(_, _));
  ks_->deriveEarlySecret(psk);
  EXPECT_CALL(*kd_, deriveSecret(_, _, _, _)).Times(4);
  ks_->getSecret(EarlySecrets::ExternalPskBinder, transcript_);
  ks_->getSecret(EarlySecrets::ResumptionPskBinder, transcript_);
  ks_->getSecret(EarlySecrets::ResumptionPskBinder, transcript_);
  ks_->getSecret(EarlySecrets::EarlyExporter, transcript_);

  StringPiece ecdhe{"ecdhe"};
  EXPECT_CALL(*kd_, deriveSecret(_, _, _, _)).Times(1);
  EXPECT_CALL(*kd_, hkdfExtract(_, _));
  ks_->deriveHandshakeSecret(ecdhe);
  EXPECT_CALL(*kd_, deriveSecret(_, _, _, _)).Times(2);
  ks_->getSecret(HandshakeSecrets::ClientHandshakeTraffic, transcript_);
  ks_->getSecret(HandshakeSecrets::ServerHandshakeTraffic, transcript_);

  EXPECT_CALL(*kd_, deriveSecret(_, _, _, _)).Times(1);
  EXPECT_CALL(*kd_, hkdfExtract(_, _));
  ks_->deriveMasterSecret();
  EXPECT_CALL(*kd_, deriveSecret(_, _, _, _)).Times(2);
  ks_->getSecret(MasterSecrets::ExporterMaster, transcript_);
  ks_->getSecret(MasterSecrets::ResumptionMaster, transcript_);

  EXPECT_CALL(*kd_, deriveSecret(_, _, _, _)).Times(2);
  ks_->deriveAppTrafficSecrets(transcript_);
  ks_->getSecret(AppTrafficSecrets::ClientAppTraffic);
  ks_->getSecret(AppTrafficSecrets::ServerAppTraffic);
}

TEST_F(KeySchedulerTest, TestNoEarly) {
  StringPiece ecdhe{"ecdhe"};
  EXPECT_CALL(*kd_, deriveSecret(_, _, _, _)).Times(1);
  EXPECT_CALL(*kd_, hkdfExtract(_, _)).Times(2);
  ks_->deriveHandshakeSecret(ecdhe);
  EXPECT_CALL(*kd_, deriveSecret(_, _, _, _)).Times(2);
  ks_->getSecret(HandshakeSecrets::ClientHandshakeTraffic, transcript_);
  ks_->getSecret(HandshakeSecrets::ServerHandshakeTraffic, transcript_);

  EXPECT_CALL(*kd_, deriveSecret(_, _, _, _)).Times(1);
  EXPECT_CALL(*kd_, hkdfExtract(_, _));
  ks_->deriveMasterSecret();
  EXPECT_CALL(*kd_, deriveSecret(_, _, _, _)).Times(2);
  ks_->getSecret(MasterSecrets::ExporterMaster, transcript_);
  ks_->getSecret(MasterSecrets::ResumptionMaster, transcript_);

  EXPECT_CALL(*kd_, deriveSecret(_, _, _, _)).Times(2);
  ks_->deriveAppTrafficSecrets(transcript_);
  ks_->getSecret(AppTrafficSecrets::ClientAppTraffic);
  ks_->getSecret(AppTrafficSecrets::ServerAppTraffic);
}

TEST_F(KeySchedulerTest, TestKeyUpdate) {
  StringPiece ecdhe{"ecdhe"};
  ks_->deriveHandshakeSecret(ecdhe);
  ks_->deriveMasterSecret();
  ks_->deriveAppTrafficSecrets(transcript_);

  EXPECT_CALL(*kd_, _expandLabel(_, _, _, _));
  EXPECT_EQ(ks_->clientKeyUpdate(), 1);
  EXPECT_CALL(*kd_, _expandLabel(_, _, _, _));
  EXPECT_EQ(ks_->clientKeyUpdate(), 2);

  EXPECT_CALL(*kd_, _expandLabel(_, _, _, _));
  EXPECT_EQ(ks_->serverKeyUpdate(), 1);
  EXPECT_CALL(*kd_, _expandLabel(_, _, _, _));
  EXPECT_EQ(ks_->serverKeyUpdate(), 2);
}

TEST_F(KeySchedulerTest, TestTrafficKey) {
  EXPECT_CALL(*kd_, _expandLabel(_, _, _, _)).Times(2);
  StringPiece trafficSecret{"secret"};
  ks_->getTrafficKey(trafficSecret, 10, 10);
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
  ks_->getTrafficKeyWithLabel(trafficSecret, keyLabel, ivLabel, 10, 10);
}

TEST_F(KeySchedulerTest, TestClonability) {
  StringPiece ecdhe{"ecdhe"};

  MockKeyDerivation* newKeyDerivation = nullptr;
  EXPECT_CALL(*kd_, clone()).WillOnce(InvokeWithoutArgs([&]() {
    auto kd = std::make_unique<MockKeyDerivation>();
    newKeyDerivation = kd.get();
    return kd;
  }));
  EXPECT_CALL(*kd_, deriveSecret(_, _, _, _)).Times(1);
  ks_->deriveHandshakeSecret(ecdhe);

  auto cloned = ks_->clone();
  ASSERT_NE(newKeyDerivation, nullptr);

  StringPiece transcript1("transcript1");
  StringPiece transcript2("transcript1");
  EXPECT_CALL(
      *newKeyDerivation,
      deriveSecret(_, _, Eq(folly::ByteRange(transcript2)), _))
      .Times(2);
  EXPECT_CALL(*kd_, deriveSecret(_, _, Eq(folly::ByteRange(transcript1)), _))
      .Times(2);

  auto t1sh =
      ks_->getSecret(HandshakeSecrets::ServerHandshakeTraffic, transcript1);
  auto t1ch =
      ks_->getSecret(HandshakeSecrets::ClientHandshakeTraffic, transcript1);
  auto t2sh =
      cloned->getSecret(HandshakeSecrets::ServerHandshakeTraffic, transcript2);
  auto t2ch =
      cloned->getSecret(HandshakeSecrets::ClientHandshakeTraffic, transcript2);
  EXPECT_EQ(t1sh, t2sh);
  EXPECT_EQ(t1ch, t2ch);
}

} // namespace test
} // namespace fizz
