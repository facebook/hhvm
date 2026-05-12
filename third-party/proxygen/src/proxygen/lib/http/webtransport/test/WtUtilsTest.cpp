/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <proxygen/lib/http/webtransport/WtUtils.h>

#include <folly/io/IOBufQueue.h>
#include <folly/portability/GTest.h>
#include <proxygen/lib/http/codec/webtransport/WebTransportFramer.h>

using namespace proxygen;
using proxygen::detail::WtEventVisitor;
using proxygen::detail::WtStreamManager;

namespace {

std::string coalesceToString(folly::IOBufQueue& queue) {
  auto buf = queue.move();
  return buf ? buf->toString() : std::string{};
}

} // namespace

// 1) With the default protocol unset, WtEventVisitor's serialization for a
//    ResetStream event must match what writeWTResetStream produces with the
//    WT_CAPSULE wire format.
TEST(WtEventVisitorTest, ResetStream_DefaultProtocol_MatchesWtCapsule) {
  const WtStreamManager::ResetStream event{
      .streamId = 4, .err = 0x42, .reliableSize = 100};

  folly::IOBufQueue visitorBuf{folly::IOBufQueue::cacheChainLength()};
  WtEventVisitor{.egress = visitorBuf}(event);

  folly::IOBufQueue directBuf{folly::IOBufQueue::cacheChainLength()};
  writeWTResetStream(
      directBuf,
      WTResetStreamCapsule{.streamId = event.streamId,
                           .appProtocolErrorCode = uint32_t(event.err),
                           .reliableSize = event.reliableSize},
      FrameProtocol::WT_CAPSULE);

  EXPECT_EQ(coalesceToString(visitorBuf), coalesceToString(directBuf));
}

// 2) With protocol=QMUX, WtEventVisitor's serialization for a ResetStream event
//    must match what writeWTResetStream produces with the QMUX wire format —
//    i.e. the protocol field is plumbed through to the underlying writer.
TEST(WtEventVisitorTest, ResetStream_QmuxProtocol_MatchesQmuxEncoding) {
  const WtStreamManager::ResetStream event{
      .streamId = 4, .err = 0x42, .reliableSize = 100};

  folly::IOBufQueue visitorBuf{folly::IOBufQueue::cacheChainLength()};
  WtEventVisitor{.egress = visitorBuf, .protocol = FrameProtocol::QMUX}(event);

  folly::IOBufQueue directBuf{folly::IOBufQueue::cacheChainLength()};
  writeWTResetStream(
      directBuf,
      WTResetStreamCapsule{.streamId = event.streamId,
                           .appProtocolErrorCode = uint32_t(event.err),
                           .reliableSize = event.reliableSize},
      FrameProtocol::QMUX);

  EXPECT_EQ(coalesceToString(visitorBuf), coalesceToString(directBuf));
}

// 3) Sanity check: the two protocols actually serialize to different bytes for
//    the same event. If they didn't, tests (1) and (2) would tautologically
//    pass even with the protocol field ignored.
TEST(WtEventVisitorTest, ResetStream_TwoProtocols_ProduceDifferentBytes) {
  const WtStreamManager::ResetStream event{
      .streamId = 4, .err = 0x42, .reliableSize = 100};

  folly::IOBufQueue capsuleBuf{folly::IOBufQueue::cacheChainLength()};
  WtEventVisitor{.egress = capsuleBuf}(event);

  folly::IOBufQueue qmuxBuf{folly::IOBufQueue::cacheChainLength()};
  WtEventVisitor{.egress = qmuxBuf, .protocol = FrameProtocol::QMUX}(event);

  EXPECT_NE(coalesceToString(capsuleBuf), coalesceToString(qmuxBuf));
}

// 4) MaxStreamsBidi takes an extra `isBidi` argument before `protocol` in the
//    underlying writer. Cover it explicitly to confirm the protocol field
//    survived being placed in the right argument slot.
TEST(WtEventVisitorTest, MaxStreamsBidi_QmuxProtocol_MatchesQmuxEncoding) {
  const WtStreamManager::MaxStreamsBidi event{{.maxStreams = 50}};

  folly::IOBufQueue visitorBuf{folly::IOBufQueue::cacheChainLength()};
  WtEventVisitor{.egress = visitorBuf, .protocol = FrameProtocol::QMUX}(event);

  folly::IOBufQueue directBuf{folly::IOBufQueue::cacheChainLength()};
  writeWTMaxStreams(directBuf,
                    WTMaxStreamsCapsule{.maximumStreams = event.maxStreams},
                    /*isBidi=*/true,
                    FrameProtocol::QMUX);

  EXPECT_EQ(coalesceToString(visitorBuf), coalesceToString(directBuf));
}
