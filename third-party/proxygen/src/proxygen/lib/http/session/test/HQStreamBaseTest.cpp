/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <folly/portability/GMock.h>
#include <folly/portability/GTest.h>
#include <proxygen/lib/http/session/HQStreamBase.h>
#include <proxygen/lib/http/session/test/HQSessionMocks.h>

using namespace proxygen;
using namespace testing;

/**
 * A test to validate that the different stream base implementations
 * are working correctly.
 */
namespace {
constexpr quic::StreamId kDefaultIngressStream = 1;
constexpr quic::StreamId kDefaultEgressStream = 2;
constexpr quic::StreamId kDefaultBidirStream = 3;
} // namespace

class HQStreamBaseTest : public Test {
 public:
  void SetUp() override {
    ssEgressMapping_ =
        std::make_unique<detail::singlestream::SSEgress>(kDefaultEgressStream);
    ssIngressMapping_ = std::make_unique<detail::singlestream::SSIngress>(
        kDefaultIngressStream);
    ssBidirMapping_ =
        std::make_unique<detail::singlestream::SSBidir>(kDefaultBidirStream);
    csBidirMappingEmpty_ =
        std::make_unique<detail::composite::CSBidir>(folly::none, folly::none);
    csBidirMappingEgressSet_ = std::make_unique<detail::composite::CSBidir>(
        kDefaultEgressStream, folly::none);
    csBidirMappingIngressSet_ = std::make_unique<detail::composite::CSBidir>(
        folly::none, kDefaultIngressStream);
    csBidirMappingBothSet_ = std::make_unique<detail::composite::CSBidir>(
        kDefaultEgressStream, kDefaultIngressStream);
  }

  void TearDown() override {
  }

 protected:
  std::unique_ptr<detail::singlestream::SSEgress> ssEgressMapping_;
  std::unique_ptr<detail::singlestream::SSIngress> ssIngressMapping_;
  std::unique_ptr<detail::singlestream::SSBidir> ssBidirMapping_;
  std::unique_ptr<detail::composite::CSBidir> csBidirMappingEmpty_;
  std::unique_ptr<detail::composite::CSBidir> csBidirMappingEgressSet_;
  std::unique_ptr<detail::composite::CSBidir> csBidirMappingIngressSet_;
  std::unique_ptr<detail::composite::CSBidir> csBidirMappingBothSet_;
};

using HQStreamDeathTest = HQStreamBaseTest;

TEST_F(HQStreamBaseTest, TestSingleStreamBidir) {
  EXPECT_EQ(kDefaultBidirStream, ssBidirMapping_->getEgressStreamId());
  EXPECT_EQ(kDefaultBidirStream, ssBidirMapping_->getIngressStreamId());
  EXPECT_EQ(kDefaultBidirStream, ssBidirMapping_->getStreamId());

  EXPECT_TRUE(ssBidirMapping_->isUsing(kDefaultBidirStream));

  EXPECT_FALSE(ssBidirMapping_->isUsing(kDefaultEgressStream));
  EXPECT_FALSE(ssBidirMapping_->isUsing(kDefaultIngressStream));
}

TEST_F(HQStreamBaseTest, TestSingleStreamEgressOnly) {
  EXPECT_EQ(kDefaultEgressStream, ssEgressMapping_->getEgressStreamId());
  EXPECT_EQ(kDefaultEgressStream, ssEgressMapping_->getStreamId());

  EXPECT_TRUE(ssEgressMapping_->isUsing(kDefaultEgressStream));

  EXPECT_FALSE(ssEgressMapping_->isUsing(kDefaultBidirStream));
  EXPECT_FALSE(ssEgressMapping_->isUsing(kDefaultIngressStream));
}

TEST_F(HQStreamDeathTest, TestSingleStreamEgressOnly) {
  EXPECT_EXIT(ssEgressMapping_->getIngressStreamId(),
              KilledBySignal(SIGABRT),
              "Egress only stream");
}

TEST_F(HQStreamBaseTest, TestCompositeBidirEmpty) {
  EXPECT_FALSE(csBidirMappingEmpty_->isUsing(kDefaultEgressStream));
  EXPECT_FALSE(csBidirMappingEmpty_->isUsing(kDefaultIngressStream));
  EXPECT_FALSE(csBidirMappingEmpty_->isUsing(kDefaultBidirStream));
}

TEST_F(HQStreamDeathTest, TestCompositeBidirEmpty) {
  EXPECT_EXIT(csBidirMappingEmpty_->getIngressStreamId(),
              KilledBySignal(SIGABRT),
              "Ingress stream MUST be assigned before being accessed");
  EXPECT_EXIT(csBidirMappingEmpty_->getEgressStreamId(),
              KilledBySignal(SIGABRT),
              "Egress stream MUST be assigned before being accessed");
  EXPECT_EXIT(csBidirMappingEmpty_->getStreamId(),
              KilledBySignal(SIGABRT),
              "Ambiguous call 'getStreamId' on a composite stream");
}

TEST_F(HQStreamBaseTest, TestCompositeBidirEgress) {
  EXPECT_EQ(kDefaultEgressStream,
            csBidirMappingEgressSet_->getEgressStreamId());

  EXPECT_TRUE(csBidirMappingEgressSet_->isUsing(kDefaultEgressStream));

  EXPECT_FALSE(csBidirMappingEgressSet_->isUsing(kDefaultIngressStream));
  EXPECT_FALSE(csBidirMappingEgressSet_->isUsing(kDefaultBidirStream));
}

TEST_F(HQStreamDeathTest, TestCompositeBidirEgress) {
  EXPECT_EXIT(csBidirMappingEgressSet_->getIngressStreamId(),
              KilledBySignal(SIGABRT),
              "Ingress stream MUST be assigned before being accessed");
  EXPECT_EXIT(csBidirMappingEgressSet_->getStreamId(),
              KilledBySignal(SIGABRT),
              "Ambiguous call 'getStreamId' on a composite stream");
}

TEST_F(HQStreamBaseTest, TestCompositeBidirIngress) {
  EXPECT_EQ(kDefaultIngressStream,
            csBidirMappingIngressSet_->getIngressStreamId());
  EXPECT_FALSE(csBidirMappingIngressSet_->isUsing(kDefaultEgressStream));

  EXPECT_TRUE(csBidirMappingIngressSet_->isUsing(kDefaultIngressStream));
  EXPECT_FALSE(csBidirMappingIngressSet_->isUsing(kDefaultBidirStream));
}

TEST_F(HQStreamDeathTest, TestCompositeBidirIngress) {
  EXPECT_EXIT(csBidirMappingIngressSet_->getEgressStreamId(),
              KilledBySignal(SIGABRT),
              "Egress stream MUST be assigned before being accessed");
  EXPECT_EXIT(csBidirMappingIngressSet_->getStreamId(),
              KilledBySignal(SIGABRT),
              "Ambiguous call 'getStreamId' on a composite stream");
}

TEST_F(HQStreamBaseTest, TestCompositeBidirBoth) {
  EXPECT_EQ(kDefaultEgressStream, csBidirMappingBothSet_->getEgressStreamId());
  EXPECT_EQ(kDefaultIngressStream,
            csBidirMappingBothSet_->getIngressStreamId());

  EXPECT_TRUE(csBidirMappingBothSet_->isUsing(kDefaultEgressStream));
  EXPECT_TRUE(csBidirMappingBothSet_->isUsing(kDefaultIngressStream));

  EXPECT_FALSE(csBidirMappingBothSet_->isUsing(kDefaultBidirStream));
}

TEST_F(HQStreamDeathTest, TestCompositeBidirEmptyEgress) {
  EXPECT_EXIT(csBidirMappingBothSet_->getStreamId(),
              KilledBySignal(SIGABRT),
              "Ambiguous call 'getStreamId' on a composite stream");
}

TEST_F(HQStreamBaseTest, TestGetStreamDirection) {
  EXPECT_EQ(ssEgressMapping_->getStreamDirection(),
            HTTPException::Direction::EGRESS);

  EXPECT_EQ(ssIngressMapping_->getStreamDirection(),
            HTTPException::Direction::INGRESS);

  EXPECT_EQ(ssBidirMapping_->getStreamDirection(),
            HTTPException::Direction::INGRESS_AND_EGRESS);

  EXPECT_EQ(csBidirMappingEmpty_->getStreamDirection(),
            HTTPException::Direction::INGRESS_AND_EGRESS);

  EXPECT_EQ(csBidirMappingEgressSet_->getStreamDirection(),
            HTTPException::Direction::INGRESS_AND_EGRESS);

  EXPECT_EQ(csBidirMappingIngressSet_->getStreamDirection(),
            HTTPException::Direction::INGRESS_AND_EGRESS);

  EXPECT_EQ(csBidirMappingBothSet_->getStreamDirection(),
            HTTPException::Direction::INGRESS_AND_EGRESS);
}
