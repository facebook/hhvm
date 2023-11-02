/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <folly/MoveWrapper.h>
#include <folly/futures/Future.h>
#include <folly/portability/GMock.h>
#include <folly/portability/GTest.h>
#include <proxygen/httpserver/samples/hq/devious/DeviousBaton.h>
#include <proxygen/lib/http/webtransport/test/Mocks.h>

using devious::DeviousBaton;
using proxygen::test::MockWebTransport;
using testing::_;

struct Message {
  uint64_t id;
  std::unique_ptr<folly::IOBuf> message;
};
folly::SemiFuture<Message> expectSendMessage(MockWebTransport& wt) {
  auto contract = folly::makePromiseContract<Message>();

  EXPECT_CALL(wt, writeStreamData(_, _, _))
      .WillOnce(
          [&wt, promise = folly::MoveWrapper(std::move(contract.first))](
              uint64_t id, std::unique_ptr<folly::IOBuf> data, bool eof) mutable
          -> folly::Expected<folly::Unit, proxygen::WebTransport::ErrorCode> {
            Message m;
            m.id = id;
            m.message = std::move(data);
            promise->setValue(std::move(m));
            EXPECT_TRUE(eof);
            wt.cleanupStream(id);
            return folly::unit;
          })
      .RetiresOnSaturation();

  return std::move(contract.second);
}

class DeviousBatonTest : public testing::TestWithParam<uint8_t> {};

TEST_P(DeviousBatonTest, Basic) {
  MockWebTransport mockClientWt;
  MockWebTransport mockServerWt;
  mockServerWt.nextUniStreamId_++;
  mockServerWt.nextBidiStreamId_++;
  DeviousBaton client(&mockClientWt, DeviousBaton::Mode::CLIENT, [](auto) {});
  DeviousBaton server(&mockServerWt, DeviousBaton::Mode::SERVER, [](auto) {});

  auto req = client.makeRequest(0, 1, {GetParam()});
  auto nextMessageFuture = expectSendMessage(mockServerWt);
  auto res = server.onRequest(req);
  EXPECT_FALSE(res.hasError());
  server.start();
  DeviousBaton* nextActor = &client;
  MockWebTransport* nextActorWt = &mockClientWt;
  for (uint8_t baton = GetParam(); baton != 0; baton++) {
    EXPECT_TRUE(nextMessageFuture.isReady());
    auto nextMessage = std::move(nextMessageFuture).get();
    nextMessageFuture = expectSendMessage(*nextActorWt);
    DeviousBaton::BatonMessageState state;
    nextActor->onStreamData(
        nextMessage.id, state, std::move(nextMessage.message), true);
    nextActorWt->cleanupReadHandle(nextMessage.id);
    EXPECT_EQ(state.state, DeviousBaton::BatonMessageState::DONE);
    EXPECT_EQ(state.baton, baton);
    if (nextActor == &client) {
      nextActor = &server;
      nextActorWt = &mockServerWt;
    } else {
      nextActor = &client;
      nextActorWt = &mockClientWt;
    }
  }
  EXPECT_FALSE(res.hasError());
  EXPECT_TRUE(nextMessageFuture.isReady());
  auto nextMessage = std::move(nextMessageFuture).get();
  EXPECT_CALL(*nextActorWt, closeSession(_));
  DeviousBaton::BatonMessageState state;
  nextActor->onStreamData(
      nextMessage.id, state, std::move(nextMessage.message), true);
  nextActorWt->cleanupReadHandle(nextMessage.id);
  if (nextActor == &client) {
    mockServerWt.cleanupReadHandle(nextMessage.id);
  } else {
    mockClientWt.cleanupReadHandle(nextMessage.id);
  }
  EXPECT_EQ(state.state, DeviousBaton::BatonMessageState::DONE);
  EXPECT_EQ(state.baton, 0);
}

INSTANTIATE_TEST_SUITE_P(DeviousBatonTest,
                         DeviousBatonTest,
                         testing::Values(250, 1, 255));
