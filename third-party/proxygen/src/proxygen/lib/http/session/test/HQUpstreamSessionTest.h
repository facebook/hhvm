/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <folly/futures/Future.h>
#include <folly/io/async/EventBaseManager.h>
#include <folly/portability/GTest.h>
#include <limits>
#include <proxygen/lib/http/HTTPHeaderSize.h>
#include <proxygen/lib/http/codec/HQControlCodec.h>
#include <proxygen/lib/http/codec/HQStreamCodec.h>
#include <proxygen/lib/http/codec/HQUnidirectionalCodec.h>
#include <proxygen/lib/http/codec/HTTP1xCodec.h>
#include <proxygen/lib/http/codec/test/TestUtils.h>
#include <proxygen/lib/http/session/test/HQSessionMocks.h>
#include <proxygen/lib/http/session/test/HQSessionTestCommon.h>
#include <proxygen/lib/http/session/test/HTTPSessionMocks.h>
#include <proxygen/lib/http/session/test/HTTPTransactionMocks.h>
#include <proxygen/lib/http/session/test/MockQuicSocketDriver.h>
#include <proxygen/lib/http/session/test/MockSessionObserver.h>
#include <proxygen/lib/http/session/test/TestUtils.h>
#include <quic/api/test/MockQuicSocket.h>
#include <wangle/acceptor/ConnectionManager.h>

class HQUpstreamSessionTest : public HQSessionTest {
 public:
  HQUpstreamSessionTest(
      folly::Optional<TestParams> overrideParams = folly::none)
      : HQSessionTest(proxygen::TransportDirection::UPSTREAM, overrideParams) {
  }

  void SetUp() override;
  void TearDown() override;

 protected:
  std::pair<proxygen::HTTPCodec::StreamID, std::unique_ptr<proxygen::HTTPCodec>>
  makeCodec(proxygen::HTTPCodec::StreamID id);

  void sendResponse(quic::StreamId id,
                    const proxygen::HTTPMessage& resp,
                    std::unique_ptr<folly::IOBuf> body = nullptr,
                    bool eom = true);

  void sendPartialBody(quic::StreamId id,
                       std::unique_ptr<folly::IOBuf> body,
                       bool eom = true);

  quic::StreamId nextUnidirectionalStreamId();

  void sendGoaway(
      quic::StreamId lastStreamId,
      std::chrono::milliseconds delay = std::chrono::milliseconds(0));

  template <class HandlerType>
  std::unique_ptr<testing::StrictMock<HandlerType>> openTransactionBase(
      bool expectStartPaused = false);

  std::unique_ptr<testing::StrictMock<proxygen::MockHTTPHandler>>
  openTransaction();

  void flushAndLoop(
      bool eof = false,
      std::chrono::milliseconds eofDelay = std::chrono::milliseconds(0),
      std::chrono::milliseconds initialDelay = std::chrono::milliseconds(0),
      std::function<void()> extraEventsFn = std::function<void()>());

  void flushAndLoopN(
      uint64_t n,
      bool eof = false,
      std::chrono::milliseconds eofDelay = std::chrono::milliseconds(0),
      std::chrono::milliseconds initialDelay = std::chrono::milliseconds(0),
      std::function<void()> extraEventsFn = std::function<void()>());

  bool flush(
      bool eof = false,
      std::chrono::milliseconds eofDelay = std::chrono::milliseconds(0),
      std::chrono::milliseconds initialDelay = std::chrono::milliseconds(0),
      std::function<void()> extraEventsFn = std::function<void()>());

  testing::StrictMock<proxygen::MockController>& getMockController();

  std::unique_ptr<proxygen::MockSessionObserver> setMockSessionObserver();

  // Representation of stream data
  // If create with a push id, can be used
  // as a push stream (requires writing the stream preface
  // followed by unframed push id)
  struct ServerStream {
    ServerStream(proxygen::HTTPCodec::StreamID cId,
                 std::unique_ptr<proxygen::HTTPCodec> c,
                 folly::Optional<proxygen::hq::PushId> pId = folly::none)
        : codecId(cId), codec(std::move(c)), pushId(pId) {
    }

    // Transport stream id
    proxygen::HTTPCodec::StreamID id;

    folly::IOBufQueue buf{folly::IOBufQueue::cacheChainLength()};
    bool readEOF{false};
    proxygen::HTTPCodec::StreamID codecId;

    std::unique_ptr<proxygen::HTTPCodec> codec;

    folly::Optional<proxygen::hq::PushId> pushId;
  };

  proxygen::MockConnectCallback connectCb_;
  std::unordered_map<quic::StreamId, ServerStream> streams_;
  folly::IOBufQueue encoderWriteBuf_{folly::IOBufQueue::cacheChainLength()};
  folly::IOBufQueue decoderWriteBuf_{folly::IOBufQueue::cacheChainLength()};
};
