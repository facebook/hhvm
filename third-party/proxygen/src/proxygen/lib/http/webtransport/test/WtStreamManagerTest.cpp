/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <proxygen/lib/http/codec/test/TestUtils.h>
#include <proxygen/lib/http/webtransport/WtStreamManager.h>

#include <folly/portability/GMock.h>
#include <folly/portability/GTest.h>
#include <quic/priority/HTTPPriorityQueue.h>

namespace proxygen::coro::test {

using WtStreamManager = detail::WtStreamManager;

// Helper to check which stream is next in the priority queue
void expectNextWritable(const quic::PriorityQueue& pq,
                        uint64_t expectedStreamId) {
  EXPECT_FALSE(pq.empty());
  auto nextId = pq.peekNextScheduledID();
  EXPECT_TRUE(nextId.isStreamID());
  EXPECT_EQ(nextId.asStreamID(), expectedStreamId);
}

struct WtSmEgressCb : WtStreamManager::EgressCallback {
  WtSmEgressCb() = default;
  ~WtSmEgressCb() override = default;
  void eventsAvailable() noexcept override {
    evAvail_ = true;
  }
  bool evAvail_{false};
};

struct WtSmIngressCb : WtStreamManager::IngressCallback {
  WtSmIngressCb() = default;
  ~WtSmIngressCb() override = default;
  void onNewPeerStream(uint64_t streamId) noexcept override {
    peerIds.push_back(streamId);
  }
  std::vector<uint64_t> peerIds;
};

using WtConfig = WtStreamManager::WtConfig;
using MaxStreamsBidi = WtStreamManager::MaxStreamsBidi;
using MaxStreamsUni = WtStreamManager::MaxStreamsUni;
using Result = WtStreamManager::Result;
using MaxConnData = WtStreamManager::MaxConnData;
using MaxStreamData = WtStreamManager::MaxStreamData;
using ResetStream = WtStreamManager::ResetStream;
using DrainSession = WtStreamManager::DrainSession;
using CloseSession = WtStreamManager::CloseSession;
using StopSending = WtStreamManager::StopSending;

TEST(WtStreamManager, BasicSelfBidi) {
  WtConfig config{.peerMaxStreamsBidi = 2};
  WtSmEgressCb egressCb;
  WtSmIngressCb ingressCb;
  auto priorityQueue = std::make_unique<quic::HTTPPriorityQueue>();
  WtStreamManager streamManager{
      detail::WtDir::Client, config, egressCb, ingressCb, *priorityQueue};

  // 0x00 is the next expected bidi stream id for client
  EXPECT_TRUE(streamManager.canCreateBidi());
  auto bidiRes = streamManager.getOrCreateBidiHandle(0x00);
  EXPECT_NE(bidiRes.readHandle, nullptr);
  EXPECT_NE(bidiRes.writeHandle, nullptr);

  // creating out of order streams is allowed (e.g. 0x08 vs 0x04)
  // necessary to support quic/http3 as they both use underlying quic stream id
  EXPECT_TRUE(streamManager.canCreateBidi());
  bidiRes = streamManager.getOrCreateBidiHandle(0x08);
  EXPECT_NE(bidiRes.readHandle, nullptr);
  EXPECT_NE(bidiRes.writeHandle, nullptr);

  // limit saturated => getOrCreateBidiHandle returns nullptr
  EXPECT_FALSE(streamManager.canCreateBidi());
  bidiRes = streamManager.getOrCreateBidiHandle(0x04);
  EXPECT_EQ(bidiRes.readHandle, nullptr);
  EXPECT_EQ(bidiRes.writeHandle, nullptr);
}

TEST(WtStreamManager, BasicSelfUni) {
  WtConfig config{.peerMaxStreamsUni = 2};
  WtSmEgressCb egressCb;
  WtSmIngressCb ingressCb;
  auto priorityQueue = std::make_unique<quic::HTTPPriorityQueue>();
  WtStreamManager streamManager{
      detail::WtDir::Client, config, egressCb, ingressCb, *priorityQueue};

  // 0x02 is the next expected uni stream id for client
  EXPECT_TRUE(streamManager.canCreateUni());
  auto bidiRes = streamManager.getOrCreateBidiHandle(0x02);
  EXPECT_EQ(bidiRes.readHandle, nullptr); // egress only
  EXPECT_NE(bidiRes.writeHandle, nullptr);

  // creating out of order streams is allowed (e.g. 0x0a vs 0x06)
  // necessary to support quic/http3 as they both use underlying quic stream id
  EXPECT_TRUE(streamManager.canCreateUni());
  bidiRes = streamManager.getOrCreateBidiHandle(0x0a);
  EXPECT_EQ(bidiRes.readHandle, nullptr); // egress only
  EXPECT_NE(bidiRes.writeHandle, nullptr);

  // limit saturated => getOrCreateBidiHandle returns nullptr
  EXPECT_FALSE(streamManager.canCreateUni());
  bidiRes = streamManager.getOrCreateBidiHandle(0x06);
  EXPECT_EQ(bidiRes.readHandle, nullptr);
  EXPECT_EQ(bidiRes.writeHandle, nullptr);
}

TEST(WtStreamManager, BasicPeerBidi) {
  WtConfig config{.selfMaxStreamsBidi = 2};
  WtSmEgressCb egressCb;
  WtSmIngressCb ingressCb;
  auto priorityQueue = std::make_unique<quic::HTTPPriorityQueue>();
  WtStreamManager streamManager{
      detail::WtDir::Client, config, egressCb, ingressCb, *priorityQueue};

  // 0x01 is the next expected bidi stream for server
  auto bidiRes = streamManager.getOrCreateBidiHandle(0x01);
  EXPECT_NE(bidiRes.readHandle, nullptr);
  EXPECT_NE(bidiRes.writeHandle, nullptr);

  // receiving out of order streams is allowed (e.g. 0x09 vs 0x05)
  // hmm should we reject this for http/2 (technically violates max streams)
  bidiRes = streamManager.getOrCreateBidiHandle(0x09);
  EXPECT_NE(bidiRes.readHandle, nullptr);
  EXPECT_NE(bidiRes.writeHandle, nullptr);

  // limit saturated => getOrCreateBidiHandle returns nullptr
  bidiRes = streamManager.getOrCreateBidiHandle(0x05);
  EXPECT_EQ(bidiRes.readHandle, nullptr);
  EXPECT_EQ(bidiRes.writeHandle, nullptr);

  CHECK_EQ(ingressCb.peerIds.size(), 2);
  EXPECT_EQ(ingressCb.peerIds[0], 0x01);
  EXPECT_EQ(ingressCb.peerIds[1], 0x09);
}

TEST(WtStreamManager, BasicPeerUni) {
  WtConfig config{.selfMaxStreamsUni = 2};
  WtSmEgressCb egressCb;
  WtSmIngressCb ingressCb;
  auto priorityQueue = std::make_unique<quic::HTTPPriorityQueue>();
  WtStreamManager streamManager{
      detail::WtDir::Client, config, egressCb, ingressCb, *priorityQueue};

  // 0x03 is the next expected uni stream for server
  auto bidiRes = streamManager.getOrCreateBidiHandle(0x03);
  EXPECT_NE(bidiRes.readHandle, nullptr);
  EXPECT_EQ(bidiRes.writeHandle, nullptr); // ingress only

  // receiving out of order streams is allowed (e.g. 0x0b vs 0x07)
  // hmm should we reject this for http/2 (technically violates max streams)
  bidiRes = streamManager.getOrCreateBidiHandle(0x0b);
  EXPECT_NE(bidiRes.readHandle, nullptr);
  EXPECT_EQ(bidiRes.writeHandle, nullptr); // ingress only

  // limit saturated => no handle returned
  bidiRes = streamManager.getOrCreateBidiHandle(0x07);
  EXPECT_EQ(bidiRes.readHandle, nullptr);
  EXPECT_EQ(bidiRes.writeHandle, nullptr); // ingress only

  // validate peer streams
  CHECK_EQ(ingressCb.peerIds.size(), 2);
  EXPECT_EQ(ingressCb.peerIds[0], 0x03);
  EXPECT_EQ(ingressCb.peerIds[1], 0x0b);
}

TEST(WtStreamManager, NextBidiUniHandle) {
  WtConfig config{.peerMaxStreamsBidi = 2, .peerMaxStreamsUni = 2};
  WtSmEgressCb egressCb;
  WtSmIngressCb ingressCb;
  auto priorityQueue = std::make_unique<quic::HTTPPriorityQueue>();
  WtStreamManager streamManager{
      detail::WtDir::Client, config, egressCb, ingressCb, *priorityQueue};

  // next egress handle tests
  auto uni = CHECK_NOTNULL(streamManager.createEgressHandle());
  EXPECT_EQ(uni->getID(), 0x02);
  uni = streamManager.createEgressHandle();
  EXPECT_EQ(uni->getID(), 0x06);

  // next bidi handle test
  auto bidi = streamManager.createBidiHandle();
  EXPECT_NE(bidi.readHandle, nullptr);
  EXPECT_NE(bidi.writeHandle, nullptr);
  EXPECT_EQ(bidi.readHandle->getID(), bidi.writeHandle->getID());
  EXPECT_EQ(bidi.readHandle->getID(), 0x00);

  bidi = streamManager.createBidiHandle();
  EXPECT_NE(bidi.readHandle, nullptr);
  EXPECT_NE(bidi.writeHandle, nullptr);
  EXPECT_EQ(bidi.readHandle->getID(), bidi.writeHandle->getID());
  EXPECT_EQ(bidi.readHandle->getID(), 0x04);
}

TEST(WtStreamManager, StreamLimits) {
  WtConfig config{};
  WtSmEgressCb egressCb;
  WtSmIngressCb ingressCb;
  auto priorityQueue = std::make_unique<quic::HTTPPriorityQueue>();
  WtStreamManager streamManager{
      detail::WtDir::Client, config, egressCb, ingressCb, *priorityQueue};

  // a single egress handle should succeed
  auto uni = streamManager.createEgressHandle();
  EXPECT_NE(uni, nullptr);

  // a single bidi handle should succeed
  auto bidi = streamManager.createBidiHandle();
  EXPECT_NE(bidi.readHandle, nullptr);
  EXPECT_NE(bidi.writeHandle, nullptr);

  // next egress handle should fail
  uni = streamManager.createEgressHandle();
  EXPECT_EQ(uni, nullptr);

  // next bidi handle should fail
  bidi = streamManager.createBidiHandle();
  EXPECT_EQ(bidi.readHandle, nullptr);
  EXPECT_EQ(bidi.writeHandle, nullptr);

  // peer grants one additional credit for each of bidi and uni
  EXPECT_TRUE(streamManager.onMaxStreams(MaxStreamsBidi{2}));
  EXPECT_TRUE(streamManager.onMaxStreams(MaxStreamsUni{2}));

  // next egress handle should succeed
  uni = CHECK_NOTNULL(streamManager.createEgressHandle());
  EXPECT_NE(uni, nullptr);

  // next bidi handle should succeed
  bidi = streamManager.createBidiHandle();
  EXPECT_NE(bidi.readHandle, nullptr);
  EXPECT_NE(bidi.writeHandle, nullptr);
}

TEST(WtStreamManager, EnqueueIngressData) {
  WtConfig config{.peerMaxStreamsBidi = 2};
  WtSmEgressCb egressCb;
  WtSmIngressCb ingressCb;
  auto priorityQueue = std::make_unique<quic::HTTPPriorityQueue>();
  WtStreamManager streamManager{
      detail::WtDir::Client, config, egressCb, ingressCb, *priorityQueue};

  // next createBidiHandle should succeed
  auto one = streamManager.createBidiHandle();
  auto two = streamManager.createBidiHandle();
  CHECK(one.readHandle && one.writeHandle && two.readHandle && two.writeHandle);

  constexpr auto kBufLen = 65'535;

  EXPECT_EQ(streamManager.bufferedBytes(*one.readHandle), 0);
  // both conn & stream recv window exactly full, expect success
  EXPECT_TRUE(
      streamManager.enqueue(*one.readHandle, {makeBuf(kBufLen), false}));
  EXPECT_EQ(streamManager.bufferedBytes(*one.readHandle), kBufLen);
  // enqueuing a additional byte in one will fail (stream recv window full)
  EXPECT_FALSE(
      streamManager.enqueue(*one.readHandle, {makeBuf(kBufLen), false}));

  // enqueuing a single byte in two will fail (conn recv window full)
  EXPECT_FALSE(streamManager.enqueue(*two.readHandle, {makeBuf(1), false}));
  auto twoFut = two.readHandle->readStreamData();
  EXPECT_FALSE(twoFut.isReady()); // no data buffered, ::enqueue failed

  // reading stream data should succeed
  auto oneFut = one.readHandle->readStreamData();
  EXPECT_TRUE(oneFut.isReady()); // enqueue should fulfill promise
  EXPECT_EQ(oneFut.value().data->computeChainDataLength(), kBufLen);
  EXPECT_FALSE(oneFut.value().fin);
  EXPECT_EQ(streamManager.bufferedBytes(*one.readHandle), 0);
}

TEST(WtStreamManager, WriteEgressHandle) {
  WtConfig config{.peerMaxStreamsBidi = 2};
  WtSmEgressCb egressCb;
  WtSmIngressCb ingressCb;
  auto priorityQueue = std::make_unique<quic::HTTPPriorityQueue>();
  WtStreamManager streamManager{
      detail::WtDir::Client, config, egressCb, ingressCb, *priorityQueue};

  // next two ::createBidiHandle should succeed
  auto one = streamManager.createBidiHandle();
  auto two = streamManager.createBidiHandle();
  CHECK(one.readHandle && one.writeHandle && two.readHandle && two.writeHandle);

  // Set priorities to make ordering predictable (urgency, incremental)
  one.writeHandle->setPriority(quic::HTTPPriorityQueue::Priority(
      0, false)); // Higher priority (urgency 0)
  two.writeHandle->setPriority(quic::HTTPPriorityQueue::Priority(
      1, false)); // Lower priority (urgency 1)

  constexpr auto kBufLen = 65'535;
  // kBufLen will fill up both conn & stream egress windows
  auto res = one.writeHandle->writeStreamData(
      /*data=*/makeBuf(kBufLen), /*fin=*/false, /*byteEventCallback=*/nullptr);
  EXPECT_TRUE(res.hasValue() && res.value() == WebTransport::FCState::BLOCKED);
  // enqueue an additional byte into one
  res = one.writeHandle->writeStreamData(
      /*data=*/makeBuf(1), /*fin=*/true, /*byteEventCallback=*/nullptr);
  EXPECT_TRUE(res.hasValue() && res.value() == WebTransport::FCState::BLOCKED);

  // each stream has an individual egress buffer of kBufLen before applying
  // backpressure => writing (kBufLen - 1) bytes into two should return
  // UNBLOCKED
  res = two.writeHandle->writeStreamData(
      makeBuf(kBufLen - 1), /*fin=*/true, /*byteEventCallback=*/nullptr);
  EXPECT_TRUE(res.hasValue() &&
              res.value() == WebTransport::FCState::UNBLOCKED);

  // we should be able to dequeue kBufLen data from one.writeHandle
  expectNextWritable(*priorityQueue, one.writeHandle->getID());
  EXPECT_EQ(streamManager.nextWritable(), one.writeHandle);
  auto dequeue = streamManager.dequeue(*one.writeHandle, /*atMost=*/kBufLen);
  EXPECT_EQ(dequeue.data->computeChainDataLength(), kBufLen);
  EXPECT_FALSE(dequeue.fin);
  // one still in queue but blocked on conn FC - dequeue again to move to
  // connFcBlockedStreams_
  EXPECT_EQ(streamManager.nextWritable(), nullptr);
  dequeue = streamManager.dequeue(*one.writeHandle, /*atMost=*/kBufLen);
  EXPECT_EQ(dequeue.data, nullptr);
  EXPECT_FALSE(dequeue.fin);

  // two is now next writable - dequeue would move to connFcBlockedStreams_
  expectNextWritable(*priorityQueue, two.writeHandle->getID());

  // grant one a single byte of stream credit; dequeuing from one should yield
  // nothing since we're still blocked on conn flow control
  EXPECT_TRUE(streamManager.onMaxData(
      MaxStreamData{{kBufLen + 1}, one.writeHandle->getID()}));
  // two is still next writable (one is blocked on conn FC)
  expectNextWritable(*priorityQueue, two.writeHandle->getID());

  // grant one additional byte of conn credit; dequeue from one should yield
  // byte + eof
  EXPECT_TRUE(streamManager.onMaxData(MaxConnData{kBufLen + 1}));
  // one is re-inserted from connFcBlockedStreams_ and should be next (higher
  // priority)
  expectNextWritable(*priorityQueue, one.writeHandle->getID());
  EXPECT_EQ(streamManager.nextWritable(), one.writeHandle);
  dequeue = streamManager.dequeue(*one.writeHandle, /*atMost=*/kBufLen);
  EXPECT_EQ(dequeue.data->computeChainDataLength(), 1);
  EXPECT_TRUE(dequeue.fin);

  // stream two is next writable, but blocked on conn egress flow control
  expectNextWritable(*priorityQueue, two.writeHandle->getID());

  // grant enough conn fc credit to unblock two completely
  EXPECT_TRUE(streamManager.onMaxData(MaxConnData{kBufLen * 2}));
  // two still next
  expectNextWritable(*priorityQueue, two.writeHandle->getID());
  EXPECT_EQ(streamManager.nextWritable(), two.writeHandle);
  dequeue = streamManager.dequeue(*two.writeHandle, /*atMost=*/kBufLen);
  EXPECT_EQ(dequeue.data->computeChainDataLength(), kBufLen - 1);
  EXPECT_TRUE(dequeue.fin);
}

TEST(WtStreamManager, DequeueWriteConnFcBlocked) {
  WtConfig config{.peerMaxStreamsUni = 1};
  WtSmEgressCb egressCb;
  WtSmIngressCb ingressCb;
  auto priorityQueue = std::make_unique<quic::HTTPPriorityQueue>();
  WtStreamManager streamManager{
      detail::WtDir::Client, config, egressCb, ingressCb, *priorityQueue};

  auto uni = CHECK_NOTNULL(streamManager.createEgressHandle());
  // write kBufLen & fin into stream (fills egress buffer)
  constexpr auto kBufLen = 65'535;
  auto res = uni->writeStreamData(
      makeBuf(kBufLen), /*fin=*/true, /*byteEventCallback=*/nullptr);
  EXPECT_TRUE(res.hasValue() && *res == WebTransport::FCState::BLOCKED);

  // we should be able to dequeue kBufLen data from one.writeHandle
  expectNextWritable(*priorityQueue, uni->getID());
  auto dequeue = streamManager.dequeue(*uni, /*atMost=*/kBufLen);
  EXPECT_TRUE(dequeue.data && dequeue.fin);
}

TEST(WtStreamManager, BidiHandleCancellation) {
  WtConfig config{};
  WtSmEgressCb egressCb;
  WtSmIngressCb ingressCb;
  auto priorityQueue = std::make_unique<quic::HTTPPriorityQueue>();
  WtStreamManager streamManager{
      detail::WtDir::Client, config, egressCb, ingressCb, *priorityQueue};

  // next ::createBidiHandle should succeed
  auto one = streamManager.createBidiHandle();
  CHECK(one.readHandle && one.writeHandle);

  auto res = one.writeHandle->writeStreamData(
      /*data=*/makeBuf(100), /*fin=*/false, /*byteEventCallback=*/nullptr);
  EXPECT_TRUE(res.hasValue() &&
              res.value() == WebTransport::FCState::UNBLOCKED);

  // StreamManager::onStopSending should request cancellation of egress handle
  auto ct = one.writeHandle->getCancelToken();
  streamManager.onStopSending({one.writeHandle->getID(), 0x00});
  EXPECT_TRUE(ct.isCancellationRequested());
  // stream should be removed from queue after cancellation
  EXPECT_EQ(streamManager.nextWritable(), nullptr);
  EXPECT_TRUE(priorityQueue->empty());

  // StreamManager::onResetStream should request cancellation of ingress handle
  auto fut = one.readHandle->readStreamData();
  ct = one.readHandle->getCancelToken();
  streamManager.onResetStream({one.writeHandle->getID(), 0x00});
  EXPECT_TRUE(ct.isCancellationRequested());
  EXPECT_TRUE(fut.isReady() && fut.hasException());
}

TEST(WtStreamManager, GrantFlowControlCredit) {
  WtConfig config{.peerMaxStreamsBidi = 2};
  WtSmEgressCb egressCb;
  WtSmIngressCb ingressCb;
  auto priorityQueue = std::make_unique<quic::HTTPPriorityQueue>();
  WtStreamManager streamManager{
      detail::WtDir::Client, config, egressCb, ingressCb, *priorityQueue};

  constexpr auto kBufLen = 65'535;

  // next ::createBidiHandle should succeed
  auto one = streamManager.createBidiHandle();
  CHECK(one.readHandle && one.writeHandle);
  // fills up both conn- & stream-level flow control
  EXPECT_TRUE(streamManager.enqueue(*one.readHandle,
                                    {makeBuf(kBufLen), /*fin=*/false}));

  auto fut = one.readHandle->readStreamData();
  EXPECT_TRUE(fut.isReady() && fut.hasValue());

  EXPECT_TRUE(std::exchange(egressCb.evAvail_, false)); // callback should have
                                                        // triggered
  auto events = streamManager.moveEvents();
  CHECK_GE(events.size(), 2); // one conn & one stream fc
  auto maxStreamData = std::get<MaxStreamData>(events[0]);
  auto maxConnData = std::get<MaxConnData>(events[1]);
  EXPECT_EQ(maxStreamData.streamId, one.readHandle->getID());
  EXPECT_EQ(maxStreamData.maxData, kBufLen * 2);
  EXPECT_EQ(maxConnData.maxData, kBufLen * 2);

  // next ::createBidiHandle should succeed
  auto two = streamManager.createBidiHandle();
  CHECK(two.readHandle && two.writeHandle);
  // fills up both conn- & stream-level flow control
  fut = two.readHandle->readStreamData();
  EXPECT_TRUE(
      streamManager.enqueue(*two.readHandle, {makeBuf(kBufLen), /*fin=*/true}));
  // should have triggered only connection-level flow control since fin=true
  events = streamManager.moveEvents();
  EXPECT_GE(events.size(), 1);
  EXPECT_TRUE(std::holds_alternative<MaxConnData>(events[0]));

  // validate that receiving a reset_stream releases connection-level flow
  // control
  auto* ingress = CHECK_NOTNULL(streamManager.getOrCreateIngressHandle(0x03));
  streamManager.enqueue(*ingress, {makeBuf(kBufLen), /*fin=*/false});
  streamManager.onResetStream(ResetStream{ingress->getID(), 0x00});
  events = streamManager.moveEvents();
  CHECK_GE(events.size(), 1);
  maxConnData = std::get<MaxConnData>(events[0]);
  EXPECT_EQ(maxConnData.maxData,
            kBufLen * 4); // we've previously already issued 2x kBufLen
                          // increments, this is the 3rd
}

TEST(WtStreamManager, GrantConnFlowControlCreditAfterRead) {
  WtConfig config{.peerMaxStreamsBidi = 2};
  WtSmEgressCb egressCb;
  WtSmIngressCb ingressCb;
  auto priorityQueue = std::make_unique<quic::HTTPPriorityQueue>();
  WtStreamManager streamManager{
      detail::WtDir::Client, config, egressCb, ingressCb, *priorityQueue};

  constexpr auto kBufLen = 65'535;

  // enqueue a total of kBufLen bytes across handles one & two
  auto one = streamManager.createBidiHandle();
  auto two = streamManager.createBidiHandle();
  CHECK(one.readHandle && one.writeHandle && two.readHandle && two.writeHandle);

  EXPECT_TRUE(streamManager.enqueue(*one.readHandle,
                                    {makeBuf(kBufLen - 1), /*fin=*/true}));
  EXPECT_TRUE(
      streamManager.enqueue(*two.readHandle, {makeBuf(1), /*fin=*/true}));

  // we should release conn-fc credit only after app reading kBufLen / 2 bytes
  // => reading the single bytes from two should not release conn-fc
  auto read = two.readHandle->readStreamData();
  EXPECT_TRUE(read.isReady());
  EXPECT_FALSE(egressCb.evAvail_);

  // reading from one releases flow control (total bytes read > kBufLen / 2)
  read = one.readHandle->readStreamData();
  EXPECT_TRUE(read.isReady());

  EXPECT_TRUE(std::exchange(egressCb.evAvail_, false));
  auto events = streamManager.moveEvents();
  EXPECT_EQ(events.size(), 1);
  EXPECT_TRUE(std::holds_alternative<MaxConnData>(events[0]));
}

TEST(WtStreamManager, NonDefaultFlowControlValues) {
  WtConfig config{};
  config.peerMaxConnData = 100;
  config.peerMaxStreamDataBidi = config.peerMaxStreamDataUni = 60;

  config.selfMaxConnData = config.selfMaxStreamDataBidi =
      config.selfMaxStreamDataUni = 100;
  WtSmEgressCb egressCb;
  WtSmIngressCb ingressCb;
  auto priorityQueue = std::make_unique<quic::HTTPPriorityQueue>();
  WtStreamManager streamManager{
      detail::WtDir::Client, config, egressCb, ingressCb, *priorityQueue};
  constexpr auto kBufLen = 100;

  // enqueue 100 bytes of data
  auto one = streamManager.createBidiHandle();
  CHECK(one.readHandle && one.writeHandle);
  EXPECT_TRUE(streamManager.enqueue(*one.readHandle,
                                    {makeBuf(kBufLen), /*fin=*/false}));

  // read synchronously available
  auto fut = one.readHandle->readStreamData();
  EXPECT_TRUE(fut.isReady() && fut.hasValue());

  EXPECT_TRUE(std::exchange(egressCb.evAvail_, false)); // callback should have
                                                        // triggered
  auto events = streamManager.moveEvents();
  CHECK_GE(events.size(), 2); // one conn & one stream fc
  auto maxStreamData = std::get<MaxStreamData>(events[0]);
  auto maxConnData = std::get<MaxConnData>(events[1]);
  EXPECT_EQ(maxStreamData.streamId, one.readHandle->getID());
  EXPECT_EQ(maxStreamData.maxData, kBufLen * 2);
  EXPECT_EQ(maxConnData.maxData, kBufLen * 2);

  // dequeuing data from one.writeHandle is limited by stream egress fc, which
  // is 60
  one.writeHandle->writeStreamData(
      makeBuf(kBufLen), /*fin=*/true, /*byteEventCallback=*/nullptr);
  // one is next writable
  EXPECT_EQ(streamManager.nextWritable(), one.writeHandle);
  expectNextWritable(*priorityQueue, one.writeHandle->getID());
  auto dequeue = streamManager.dequeue(*one.writeHandle,
                                       std::numeric_limits<uint64_t>::max());
  EXPECT_EQ(dequeue.data->computeChainDataLength(), 60);
  EXPECT_FALSE(dequeue.fin);

  // dequeuing data from two's writeHandle is now limited by conn egress fc,
  // which is 100-60=40
  auto* two = CHECK_NOTNULL(streamManager.createEgressHandle());
  two->writeStreamData(
      makeBuf(kBufLen), /*fin=*/true, /*byteEventCallback=*/nullptr);
  EXPECT_EQ(streamManager.nextWritable(), two);
  expectNextWritable(*priorityQueue, two->getID());
  dequeue = streamManager.dequeue(*two, std::numeric_limits<uint64_t>::max());
  EXPECT_EQ(dequeue.data->computeChainDataLength(), 40);
  EXPECT_FALSE(dequeue.fin);
}

TEST(WtStreamManager, ResetStreamReleasesConnFlowControl) {
  WtConfig config{.selfMaxStreamsUni = 10};
  WtSmEgressCb egressCb;
  WtSmIngressCb ingressCb;
  auto priorityQueue = std::make_unique<quic::HTTPPriorityQueue>();
  WtStreamManager streamManager{
      detail::WtDir::Client, config, egressCb, ingressCb, *priorityQueue};
  constexpr auto kBufLen = 65'535;
  constexpr auto kHalfBufLen = (kBufLen / 2) + 1;

  auto* one = CHECK_NOTNULL(streamManager.getOrCreateIngressHandle(0x03));
  auto* two = CHECK_NOTNULL(streamManager.getOrCreateIngressHandle(0x07));
  auto* three = CHECK_NOTNULL(streamManager.getOrCreateIngressHandle(0x0b));

  // queue kHalfBufLen across all handles; ensure the last reset releases
  // conn fc
  streamManager.enqueue(*one, {makeBuf(kHalfBufLen - 2), /*fin=*/false});
  streamManager.onResetStream({one->getID()});
  EXPECT_FALSE(egressCb.evAvail_);

  streamManager.enqueue(*two, {makeBuf(1), /*fin=*/false});
  streamManager.onResetStream({two->getID()});
  EXPECT_FALSE(egressCb.evAvail_);

  streamManager.enqueue(*three, {makeBuf(1), /*fin=*/false});
  streamManager.onResetStream({three->getID()});
  EXPECT_TRUE(egressCb.evAvail_);

  // validate MaxConnData value
  auto events = streamManager.moveEvents();
  EXPECT_EQ(events.size(), 1);
  auto maxConnData = std::get<MaxConnData>(events[0]);
  EXPECT_EQ(maxConnData.maxData, kBufLen + kHalfBufLen);
}

TEST(WtStreamManager, StopSendingResetStreamTest) {
  WtConfig config{};
  WtSmEgressCb egressCb;
  WtSmIngressCb ingressCb;
  auto priorityQueue = std::make_unique<quic::HTTPPriorityQueue>();
  WtStreamManager streamManager{
      detail::WtDir::Client, config, egressCb, ingressCb, *priorityQueue};
  constexpr auto kBufLen = 65'535;

  // next ::createBidiHandle should succeed
  auto one = streamManager.createBidiHandle();
  CHECK(one.readHandle && one.writeHandle);
  auto id = one.readHandle->getID();

  // stop sending should invoke callback and resolve pending promise
  auto rp = one.readHandle->readStreamData();
  one.readHandle->stopSending(0);
  EXPECT_TRUE(rp.isReady() && rp.hasException()); // exception via stopSending
  EXPECT_TRUE(std::exchange(egressCb.evAvail_, false));
  auto events = streamManager.moveEvents();
  EXPECT_EQ(events.size(), 1);
  auto stopSending = std::get<StopSending>(events[0]);
  EXPECT_EQ(stopSending.streamId, id);
  EXPECT_EQ(stopSending.err, 0);

  // fill up egress buffer to ensure ::resetStream resolves pending promise
  one.writeHandle->writeStreamData(
      makeBuf(kBufLen), /*fin=*/false, /*byteEventCallback=*/nullptr);
  auto wp = one.writeHandle->awaitWritable();

  // reset stream should invoke callback and resolve pending promise
  one.writeHandle->resetStream(/*error=*/1);
  EXPECT_TRUE(wp.hasValue() && wp->isReady() && wp->hasException());
  EXPECT_TRUE(std::exchange(egressCb.evAvail_, false));
  events = streamManager.moveEvents();
  EXPECT_EQ(events.size(), 1);
  auto resetStream = std::get<ResetStream>(events[0]);
  EXPECT_EQ(resetStream.streamId, id);
  EXPECT_EQ(resetStream.err, 1);

  // bidirectionally reset => stream deleted
  EXPECT_FALSE(streamManager.hasStreams());

  // ::resetStream on a unidirectional egress stream should erase the stream
  auto* egress = CHECK_NOTNULL(streamManager.createEgressHandle());
  EXPECT_TRUE(streamManager.hasStreams());
  egress->resetStream(/*error=*/0); // read side is closed for an egress-only
                                    // handle; bidirectionally complete after
                                    // ::resetStream
  EXPECT_FALSE(streamManager.hasStreams());
}

TEST(WtStreamManager, AwaitWritableTest) {
  WtConfig config{};
  WtSmEgressCb egressCb;
  WtSmIngressCb ingressCb;
  auto priorityQueue = std::make_unique<quic::HTTPPriorityQueue>();
  WtStreamManager streamManager{
      detail::WtDir::Client, config, egressCb, ingressCb, *priorityQueue};

  constexpr auto kBufLen = 65'535;
  // next ::createBidiHandle should succeed
  auto eh = CHECK_NOTNULL(streamManager.createEgressHandle());

  // await writable future should be synchronously ready & equal to kBufLen
  // (default egress stream fc)
  auto await = eh->awaitWritable();
  EXPECT_TRUE(await.hasValue() && await.value().isReady() &&
              await.value().value() == kBufLen);

  // send kBufLen + 1 bytes
  auto fcRes = eh->writeStreamData(
      makeBuf(kBufLen + 1), /*fin=*/false, /*byteEventCallback=*/nullptr);
  EXPECT_TRUE(fcRes.hasValue() &&
              fcRes.value() == WebTransport::FCState::BLOCKED);

  // await writable future; not ready awaiting egress buffer space
  await = eh->awaitWritable();
  EXPECT_TRUE(await.hasValue() && !await.value().isReady());

  // granting additional fc credit does not unblock egress buffer
  streamManager.onMaxData({{kBufLen + 2}, eh->getID()});
  EXPECT_TRUE(await.hasValue() && !await.value().isReady());

  // dequeue will resolve promise
  streamManager.dequeue(*eh, kBufLen);
  EXPECT_TRUE(await->isReady() &&
              await->value() == kBufLen - 1); // minus one because we enqueued
                                              // kBufLen + 1 bytes of data
}

TEST(WtStreamManager, WritableStreams) {
  WtConfig config{.peerMaxStreamsUni = 3};
  WtSmEgressCb egressCb;
  WtSmIngressCb ingressCb;
  auto priorityQueue = std::make_unique<quic::HTTPPriorityQueue>();
  WtStreamManager streamManager{
      detail::WtDir::Client, config, egressCb, ingressCb, *priorityQueue};
  constexpr auto kBufLen = 65'535;
  constexpr auto kAtMost = std::numeric_limits<uint64_t>::max();

  // next two ::createEgressHandle should succeed
  auto one = CHECK_NOTNULL(streamManager.createEgressHandle());
  auto two = CHECK_NOTNULL(streamManager.createEgressHandle());

  // 1 byte + eof; next writableStream == one
  auto writeRes = one->writeStreamData(
      makeBuf(1), /*fin=*/true, /*byteEventCallback=*/nullptr);
  EXPECT_TRUE(writeRes.hasValue() &&
              writeRes.value() == WebTransport::FCState::UNBLOCKED);
  EXPECT_TRUE(std::exchange(egressCb.evAvail_, false));
  // next writable stream is one
  EXPECT_EQ(streamManager.nextWritable(), one);
  expectNextWritable(*priorityQueue, one->getID());

  // dequeue should yield the expected results
  auto dequeue = streamManager.dequeue(*one, kAtMost);
  EXPECT_TRUE(dequeue.data->length() == 1 && dequeue.fin);

  // no more writableStreams
  EXPECT_EQ(streamManager.nextWritable(), nullptr);
  EXPECT_TRUE(priorityQueue->empty());

  // write kBufLen data; which will exceed conn flow control by one byte)
  writeRes = two->writeStreamData(
      makeBuf(kBufLen), /*fin=*/false, /*byteEventCallback=*/nullptr);
  EXPECT_TRUE(writeRes.hasValue() &&
              writeRes.value() == WebTransport::FCState::BLOCKED);
  EXPECT_TRUE(std::exchange(egressCb.evAvail_, false));
  // next writable stream is two
  EXPECT_EQ(streamManager.nextWritable(), two);
  expectNextWritable(*priorityQueue, two->getID());

  // dequeue will yield kBufLen - 1 (limited by conn flow control)
  dequeue = streamManager.dequeue(*two, kAtMost);
  EXPECT_EQ(dequeue.data->length(), kBufLen - 1);
  EXPECT_FALSE(dequeue.fin);
  // two still in queue but blocked on conn FC - dequeue again to move to
  // connFcBlockedStreams_
  EXPECT_EQ(streamManager.nextWritable(), nullptr);
  dequeue = streamManager.dequeue(*two, kAtMost);
  EXPECT_EQ(dequeue.data, nullptr);
  EXPECT_FALSE(dequeue.fin);
  // no writable streams, blocked on conn flow control
  EXPECT_TRUE(priorityQueue->empty());

  // issue one additional byte of conn fc
  EXPECT_TRUE(streamManager.onMaxData(MaxConnData{kBufLen + 1}));

  // two is now writable after conn flow control granted
  EXPECT_EQ(streamManager.nextWritable(), two);
  expectNextWritable(*priorityQueue, two->getID());
  dequeue = streamManager.dequeue(*two, kAtMost);
  EXPECT_EQ(dequeue.data->length(), 1);
  EXPECT_FALSE(dequeue.fin);

  // FIN-only stream should still be writable even when connection FC is blocked
  auto three = CHECK_NOTNULL(streamManager.createEgressHandle());
  three->writeStreamData(nullptr, /*fin=*/true, /*byteEventCallback=*/nullptr);
  EXPECT_EQ(streamManager.nextWritable(), three);
  expectNextWritable(*priorityQueue, three->getID());
}

TEST(WtStreamManager, DrainWtSession) {
  WtConfig config{.peerMaxStreamsBidi = 2, .peerMaxStreamsUni = 2};
  WtSmEgressCb egressCb;
  WtSmIngressCb ingressCb;
  auto priorityQueue = std::make_unique<quic::HTTPPriorityQueue>();
  WtStreamManager streamManager{
      detail::WtDir::Client, config, egressCb, ingressCb, *priorityQueue};

  // drain session, expect enqueued event
  streamManager.onDrainSession({});
  streamManager.drain();
  EXPECT_TRUE(egressCb.evAvail_);
  auto events = streamManager.moveEvents();
  CHECK(!events.empty() && std::holds_alternative<DrainSession>(events[0]));

  // streams can still be created after drain
  auto bidi = streamManager.createBidiHandle();
  auto* egress = streamManager.createEgressHandle();
  EXPECT_TRUE(bidi.readHandle && bidi.writeHandle && egress);

  // shutdown session
  streamManager.shutdown(CloseSession{});

  // no streams can be opened after shutdown
  bidi = streamManager.createBidiHandle();
  egress = streamManager.createEgressHandle();
  EXPECT_FALSE(bidi.readHandle || bidi.writeHandle || egress);
}

TEST(WtStreamManager, CloseWtSession) {
  WtConfig config{};
  WtSmEgressCb egressCb;
  WtSmIngressCb ingressCb;
  auto priorityQueue = std::make_unique<quic::HTTPPriorityQueue>();
  WtStreamManager streamManager{
      detail::WtDir::Client, config, egressCb, ingressCb, *priorityQueue};

  // ensure cancellation source is cancelled when invoked ::onCloseSession
  auto one = streamManager.createBidiHandle();
  auto oneRead = one.readHandle->readStreamData();

  auto* two = streamManager.createEgressHandle();
  auto cts = {one.readHandle->getCancelToken(),
              one.writeHandle->getCancelToken(),
              two->getCancelToken()};

  streamManager.onCloseSession(CloseSession{0, ""});
  for (auto& ct : cts) {
    EXPECT_TRUE(ct.isCancellationRequested());
  }

  // read promise should have exception set
  EXPECT_TRUE(oneRead.isReady() && oneRead.hasException());

  // sends a CloseSession in response to a peer initiated CloseSession;
  // this is okay wrt WebTransport rfc
  auto events = streamManager.moveEvents();
  EXPECT_TRUE(std::holds_alternative<CloseSession>(events.back()));

  // a locally initiated close should enqueue an event
  streamManager.shutdown(CloseSession{0, ""});
}

TEST(WtStreamManager, ResetStreamReliableSize) {
  WtConfig config{};
  WtSmEgressCb egressCb;
  WtSmIngressCb ingressCb;
  auto priorityQueue = std::make_unique<quic::HTTPPriorityQueue>();
  WtStreamManager streamManager{
      detail::WtDir::Client, config, egressCb, ingressCb, *priorityQueue};

  // enqueue 100 bytes, ensure ::onResetStream does not deallocate stream until
  // after 100 bytes are read
  auto ingress = CHECK_NOTNULL(streamManager.getOrCreateIngressHandle(0x03));
  streamManager.enqueue(*ingress, {makeBuf(100), /*fin=*/false});

  auto ct = ingress->getCancelToken();
  streamManager.onResetStream(
      ResetStream{ingress->getID(), 0x00, /*reliableSize=*/100});
  // stream still alive
  EXPECT_TRUE(streamManager.hasStreams());
  EXPECT_FALSE(ct.isCancellationRequested());

  // read the 100 buffered bytes successfully
  auto read = ingress->readStreamData();
  EXPECT_TRUE(read.isReady() &&
              read.value().data->computeChainDataLength() == 100 &&
              read.value().fin == false);

  // future read fails with exception
  read = ingress->readStreamData();
  EXPECT_TRUE(read.isReady() && read.hasException());
  EXPECT_FALSE(streamManager.hasStreams());
}

TEST(WtStreamManager, InvalidCtrlFrames) {
  WtConfig config{};
  WtSmEgressCb egressCb;
  WtSmIngressCb ingressCb;
  auto priorityQueue = std::make_unique<quic::HTTPPriorityQueue>();
  WtStreamManager streamManager{
      detail::WtDir::Client, config, egressCb, ingressCb, *priorityQueue};

  auto bidi = streamManager.createBidiHandle();
  // stream grant offset (i.e. 0) < stream current offset (i.e. 64KiB)
  EXPECT_EQ(
      streamManager.onMaxData(MaxStreamData{{0}, bidi.readHandle->getID()}),
      Result::Fail);

  // conn grant offset (i.e. 0) < conn stream offset (i.e. 64KiB) fails
  EXPECT_EQ(streamManager.onMaxData(MaxConnData{0}), Result::Fail);

  // MaxStreamsBidi{0} < peer.bidi (i.e. 1) fails
  EXPECT_EQ(streamManager.onMaxStreams(MaxStreamsBidi{0}), Result::Fail);

  // MaxStreamsUni{0} < peer.uni (i.e. 1) fails
  EXPECT_EQ(streamManager.onMaxStreams(MaxStreamsUni{0}), Result::Fail);
}

TEST(WtStreamManager, IssueMaxStreamsBidiUni) {
  WtConfig config{.selfMaxStreamsBidi = 2, .selfMaxStreamsUni = 2};
  WtSmEgressCb egressCb;
  WtSmIngressCb ingressCb;
  auto priorityQueue = std::make_unique<quic::HTTPPriorityQueue>();
  WtStreamManager streamManager{
      detail::WtDir::Client, config, egressCb, ingressCb, *priorityQueue};

  auto peerBidi = streamManager.getOrCreateBidiHandle(0x01);
  CHECK(peerBidi.readHandle && peerBidi.writeHandle);
  auto peerUni = CHECK_NOTNULL(streamManager.getOrCreateIngressHandle(0x03));

  // rst_stream & stop_sending on peerBidi will close stream => issue
  // MaxStreamsBidi
  peerBidi.readHandle->stopSending(0);
  peerBidi.writeHandle->resetStream(0);

  // reading fin on peerUni will close stream => issue MaxStreamsUni
  streamManager.enqueue(*peerUni, {makeBuf(0), true});
  auto read = peerUni->readStreamData();
  EXPECT_TRUE(read.isReady());

  EXPECT_TRUE(egressCb.evAvail_);
  // validate MaxStreamsBidi&MaxStreamsUni
  auto events = streamManager.moveEvents();
  EXPECT_EQ(events.size(), 4);
  // events[0] & events[1] are StopSending & ResetStream
  auto maxStreamsBidi = std::get<MaxStreamsBidi>(events[2]);
  auto maxStreamsUni = std::get<MaxStreamsUni>(events[3]);
  EXPECT_EQ(maxStreamsBidi.maxStreams, 3);
  EXPECT_EQ(maxStreamsUni.maxStreams, 3);
}

TEST(WtStreamManager, ShutdownOpenStreams) {
  WtConfig config;
  WtSmEgressCb egressCb;
  WtSmIngressCb ingressCb;
  auto priorityQueue = std::make_unique<quic::HTTPPriorityQueue>();
  WtStreamManager streamManager{
      detail::WtDir::Client, config, egressCb, ingressCb, *priorityQueue};

  // create bidi stream
  auto bidi = streamManager.createBidiHandle();
  CHECK(bidi.readHandle && bidi.writeHandle);

  // reset one direction
  bidi.writeHandle->resetStream(0);

  // ::shutdown with streams still open
  streamManager.shutdown(CloseSession{});
}

TEST(WtStreamManager, NoopNonExistentStreams) {
  WtConfig config{.peerMaxStreamsBidi = 2, .peerMaxStreamsUni = 2};
  WtSmEgressCb egressCb;
  WtSmIngressCb ingressCb;
  auto priorityQueue = std::make_unique<quic::HTTPPriorityQueue>();
  WtStreamManager streamManager{
      detail::WtDir::Client, config, egressCb, ingressCb, *priorityQueue};

  streamManager.onStopSending({.streamId = 0});
  streamManager.onResetStream({.streamId = 1});
  streamManager.onMaxData({.streamId = 2});

  {
    // no stream state expected
    auto handle = streamManager.getBidiHandle(0);
    EXPECT_FALSE(handle.readHandle || handle.writeHandle);
  }

  {
    // no stream state expected
    auto handle = streamManager.getBidiHandle(1);
    EXPECT_FALSE(handle.readHandle || handle.writeHandle);
  }

  {
    // no stream state expected
    auto handle = streamManager.getBidiHandle(2);
    EXPECT_FALSE(handle.readHandle || handle.writeHandle);
  }
}

TEST(WtStreamManager, OnlyFinPending) {
  WtConfig config{.peerMaxStreamsUni = 2};
  WtSmEgressCb egressCb;
  WtSmIngressCb ingressCb;
  auto priorityQueue = std::make_unique<quic::HTTPPriorityQueue>();
  WtStreamManager streamManager{
      detail::WtDir::Client, config, egressCb, ingressCb, *priorityQueue};

  auto one = CHECK_NOTNULL(streamManager.createEgressHandle());
  auto two = CHECK_NOTNULL(streamManager.createEgressHandle());

  // Set priorities to make ordering predictable (urgency, incremental)
  one->setPriority(quic::HTTPPriorityQueue::Priority(
      0, false)); // Higher priority (urgency 0)
  two->setPriority(quic::HTTPPriorityQueue::Priority(
      1, false)); // Lower priority (urgency 1)

  one->writeStreamData(
      /*data*/ makeBuf(1), /*fin=*/true, /*byteEventCallback=*/nullptr);
  // next expected writable stream is one
  EXPECT_EQ(streamManager.nextWritable(), one);
  expectNextWritable(*priorityQueue, one->getID());

  two->writeStreamData(
      /*data*/ nullptr, /*fin=*/true, /*byteEventCallback=*/nullptr);
  // one still has higher priority, so it should still be next
  EXPECT_EQ(streamManager.nextWritable(), one);
  expectNextWritable(*priorityQueue, one->getID());
}

TEST(WtStreamManager, NextWritableReturnsNullptrWhenQueueEmpty) {
  WtConfig config{.peerMaxStreamsUni = 2};
  WtSmEgressCb egressCb;
  WtSmIngressCb ingressCb;
  auto priorityQueue = std::make_unique<quic::HTTPPriorityQueue>();
  WtStreamManager streamManager{
      detail::WtDir::Client, config, egressCb, ingressCb, *priorityQueue};

  // Queue is empty, nextWritable should return nullptr
  EXPECT_EQ(streamManager.nextWritable(), nullptr);

  // Create a stream but don't write data - queue should still be empty
  auto one = CHECK_NOTNULL(streamManager.createEgressHandle());
  EXPECT_EQ(streamManager.nextWritable(), nullptr);

  // Write data to make stream writable
  one->writeStreamData(
      makeBuf(10), /*fin=*/false, /*byteEventCallback=*/nullptr);
  EXPECT_NE(streamManager.nextWritable(), nullptr);
  EXPECT_EQ(streamManager.nextWritable(), one);
}

TEST(WtStreamManager, NextWritableReturnsNullptrWhenHeadIsDatagram) {
  WtConfig config{.peerMaxStreamsUni = 2};
  WtSmEgressCb egressCb;
  WtSmIngressCb ingressCb;
  auto priorityQueue = std::make_unique<quic::HTTPPriorityQueue>();
  WtStreamManager streamManager{
      detail::WtDir::Client, config, egressCb, ingressCb, *priorityQueue};

  // Insert a datagram directly into the priority queue (bypassing
  // StreamPriorityQueue)
  auto datagramId = quic::PriorityQueue::Identifier::fromDatagramFlowID(42);
  priorityQueue->insertOrUpdate(datagramId,
                                quic::HTTPPriorityQueue::Priority(0, false));

  // Even though there's something in the queue, nextWritable should return
  // nullptr because the head is a datagram, not a stream
  EXPECT_FALSE(priorityQueue->empty());
  EXPECT_EQ(streamManager.nextWritable(), nullptr);

  // Now add a stream with lower priority
  auto one = CHECK_NOTNULL(streamManager.createEgressHandle());
  one->setPriority(quic::HTTPPriorityQueue::Priority(1, false));
  one->writeStreamData(
      makeBuf(10), /*fin=*/false, /*byteEventCallback=*/nullptr);

  // Datagram is still at head (higher priority), so nextWritable still returns
  // nullptr
  EXPECT_EQ(streamManager.nextWritable(), nullptr);

  // Remove the datagram, now the stream should be at the head
  priorityQueue->erase(datagramId);
  EXPECT_EQ(streamManager.nextWritable(), one);
}

using WtReadHandle = WebTransport::StreamReadHandle;
using WtWriteHandle = WebTransport::StreamWriteHandle;

struct WtReadCallback : WtStreamManager::ReadCallback {
  ~WtReadCallback() override = default;
  void readReady(WtReadHandle&) noexcept override {
    signalled = true;
  }

  bool signalled = false;
};

struct MockDeliveryCallback : WebTransport::ByteEventCallback {
  MOCK_METHOD(void, onByteEvent, (uint64_t, uint64_t), (noexcept, override));
  MOCK_METHOD(void,
              onByteEventCanceled,
              (uint64_t, uint64_t),
              (noexcept, override));
};

TEST(WtStreamManager, PerStreamCallbacks) {
  WtConfig config{};
  WtSmEgressCb egressCb;
  WtSmIngressCb ingressCb;
  auto priorityQueue = std::make_unique<quic::HTTPPriorityQueue>();
  WtStreamManager streamManager{
      detail::WtDir::Client, config, egressCb, ingressCb, *priorityQueue};

  auto bidi = streamManager.createBidiHandle();
  CHECK(bidi.readHandle && bidi.writeHandle);

  // set stream callbacks
  WtReadCallback rcb;
  streamManager.setReadCb(*bidi.readHandle, &rcb);

  bidi.writeHandle->writeStreamData(
      makeBuf(10), /*fin=*/false, /*byteEventCallback=*/nullptr);

  // read before enqueue
  auto read = bidi.readHandle->readStreamData();
  EXPECT_FALSE(read.isReady());
  // enqueue 10 bytes
  streamManager.enqueue(*bidi.readHandle, {.data = makeBuf(10), .fin = false});
  EXPECT_TRUE(read.isReady());
  EXPECT_TRUE(std::exchange(rcb.signalled, false)); // signalled

  // read after enqueue
  streamManager.enqueue(*bidi.readHandle, {.data = makeBuf(10), .fin = false});
  read = bidi.readHandle->readStreamData();
  EXPECT_TRUE(read.isReady());
  EXPECT_TRUE(std::exchange(rcb.signalled, false)); // signalled
}

TEST(WtStreamManager, DeliveryCallback) {
  WtConfig config{.peerMaxStreamsUni = 5};
  WtSmEgressCb egressCb;
  WtSmIngressCb ingressCb;
  auto priorityQueue = std::make_unique<quic::HTTPPriorityQueue>();
  WtStreamManager streamManager{
      detail::WtDir::Client, config, egressCb, ingressCb, *priorityQueue};

  MockDeliveryCallback cb1, cb2, cb3, cb4, cb5;

  // single write with callback
  auto* h1 = CHECK_NOTNULL(streamManager.createEgressHandle());
  h1->writeStreamData(makeBuf(100), /*fin=*/true, &cb1);
  auto deq1 = streamManager.dequeue(*h1, 100);
  EXPECT_EQ(deq1.data->computeChainDataLength(), 100);
  EXPECT_TRUE(deq1.fin);
  EXPECT_EQ(deq1.deliveryCallback, &cb1);

  // multiple writes with different callbacks
  auto* h2 = CHECK_NOTNULL(streamManager.createEgressHandle());
  h2->writeStreamData(makeBuf(100), /*fin=*/false, &cb1);
  h2->writeStreamData(makeBuf(50), /*fin=*/false, &cb2);
  h2->writeStreamData(makeBuf(75), /*fin=*/true, &cb3);

  auto d2a = streamManager.dequeue(*h2, 100);
  EXPECT_EQ(d2a.data->computeChainDataLength(), 100);
  EXPECT_EQ(d2a.deliveryCallback, &cb1);

  auto d2b = streamManager.dequeue(*h2, 100);
  EXPECT_EQ(d2b.data->computeChainDataLength(), 50);
  EXPECT_EQ(d2b.deliveryCallback, &cb2);

  auto d2c = streamManager.dequeue(*h2, 100);
  EXPECT_EQ(d2c.data->computeChainDataLength(), 75);
  EXPECT_TRUE(d2c.fin);
  EXPECT_EQ(d2c.deliveryCallback, &cb3);

  // writes without callbacks coalesce with final callback write
  auto* h3 = CHECK_NOTNULL(streamManager.createEgressHandle());
  h3->writeStreamData(makeBuf(100), /*fin=*/false, nullptr);
  h3->writeStreamData(makeBuf(100), /*fin=*/false, nullptr);
  h3->writeStreamData(makeBuf(50), /*fin=*/true, &cb4);

  auto deq3 = streamManager.dequeue(*h3, 300);
  EXPECT_EQ(deq3.data->computeChainDataLength(), 250);
  EXPECT_TRUE(deq3.fin);
  EXPECT_EQ(deq3.deliveryCallback, &cb4);

  // callback delivered only when write completes
  auto* h4 = CHECK_NOTNULL(streamManager.createEgressHandle());
  h4->writeStreamData(makeBuf(100), /*fin=*/true, &cb5);

  auto d4a = streamManager.dequeue(*h4, 60);
  EXPECT_EQ(d4a.data->computeChainDataLength(), 60);
  EXPECT_EQ(d4a.deliveryCallback, nullptr);

  auto d4b = streamManager.dequeue(*h4, 60);
  EXPECT_EQ(d4b.data->computeChainDataLength(), 40);
  EXPECT_TRUE(d4b.fin);
  EXPECT_EQ(d4b.deliveryCallback, &cb5);

  // fin-only write with callback
  auto* h5 = CHECK_NOTNULL(streamManager.createEgressHandle());
  h5->writeStreamData(makeBuf(50), /*fin=*/false, nullptr);
  auto d5a = streamManager.dequeue(*h5, 100);
  EXPECT_EQ(d5a.data->computeChainDataLength(), 50);
  EXPECT_EQ(d5a.deliveryCallback, nullptr);

  h5->writeStreamData(nullptr, /*fin=*/true, &cb1);
  auto d5b = streamManager.dequeue(*h5, 100);
  EXPECT_EQ(d5b.data->computeChainDataLength(), 0);
  EXPECT_TRUE(d5b.fin);
  EXPECT_EQ(d5b.deliveryCallback, &cb1);
}

TEST(WtStreamManager, ByteEventCancellation) {
  WtConfig config{.peerMaxStreamsUni = 2};
  WtSmEgressCb egressCb;
  WtSmIngressCb ingressCb;
  auto priorityQueue = std::make_unique<quic::HTTPPriorityQueue>();
  WtStreamManager streamManager{
      detail::WtDir::Client, config, egressCb, ingressCb, *priorityQueue};

  ::testing::StrictMock<MockDeliveryCallback> cb1, cb2;

  auto* h = CHECK_NOTNULL(streamManager.createEgressHandle());
  auto id = h->getID();

  // first write: 100 bytes at offset 0 => last byte offset = 99
  h->writeStreamData(makeBuf(100), /*fin=*/false, &cb1);
  // second write: 50 bytes at offset 100 => last byte offset = 149
  h->writeStreamData(makeBuf(50), /*fin=*/false, &cb2);
  EXPECT_CALL(cb1, onByteEventCanceled(id, 99));
  EXPECT_CALL(cb2, onByteEventCanceled(id, 149));

  // reset stream triggers cancellation of all pending byte event callbacks
  h->resetStream(/*error=*/0);
}

} // namespace proxygen::coro::test
