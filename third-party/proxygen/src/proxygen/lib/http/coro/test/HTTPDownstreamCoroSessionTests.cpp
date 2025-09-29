/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "proxygen/lib/http/coro/HTTPFixedSource.h"
#include "proxygen/lib/http/coro/test/HTTPCoroSessionTests.h"
#include "proxygen/lib/http/coro/test/HTTPTestSources.h"
#include "proxygen/lib/http/coro/test/Mocks.h"
#include "proxygen/lib/http/coro/util/test/TestHelpers.h"
#include <proxygen/lib/http/session/test/MockHTTPSessionStats.h>

#include "folly/coro/GmockHelpers.h"
#include <folly/coro/Sleep.h>
#include <folly/logging/xlog.h>
#include <quic/priority/HTTPPriorityQueue.h>

using namespace proxygen;
using namespace testing;
using TransportErrorCode = folly::coro::TransportIf::ErrorCode;

namespace {
const quic::StreamId kQPACKEncoderEgressStreamId = 7;
const quic::StreamId kQPACKDecoderEgressStreamId = 11;
} // namespace

namespace proxygen::coro::test {

class HTTPDownstreamSessionTest : public HTTPCoroSessionTest {
 public:
  HTTPDownstreamSessionTest()
      : HTTPCoroSessionTest(TransportDirection::DOWNSTREAM),
        handler_(std::make_shared<proxygen::coro::test::MockHTTPHandler>()),
        clientCodec_(peerCodec_.get()) {
  }

  void SetUp() override {
    SettingsId datagramSetting =
        *hq::hqToHttpSettingsId(hq::SettingId::H3_DATAGRAM);
    bool enableDatagrams = isHQ() && GetParam().enableDatagrams;
    setTestCodecSetting(
        clientCodec_->getEgressSettings(), datagramSetting, enableDatagrams);
    HTTPCoroSessionTest::setUp(handler_);
    session_->setSetting(datagramSetting, enableDatagrams);
  }

  using HandlerFn = std::function<folly::coro::Task<HTTPSourceHolder>(
      folly::EventBase *, HTTPSessionContextPtr, HTTPSourceHolder)>;

  std::shared_ptr<HandlerFn> addSimpleStrictHandler(HandlerFn handlerFn) {
    // If the function captures any local variables, it needs to outlive
    // handleRequest(), because it's run in a coroutine.
    auto sharedHandlerFn = std::make_shared<HandlerFn>(std::move(handlerFn));
    EXPECT_CALL(*handler_, handleRequest(_, _, _))
        .WillOnce(folly::coro::gmock_helpers::CoInvoke(
            [sharedHandlerFn](folly::EventBase *evb,
                              HTTPSessionContextPtr ctx,
                              HTTPSourceHolder source)
                -> folly::coro::Task<HTTPSourceHolder> {
              CHECK_EQ(ctx->getEventBase(), evb);
              return (*sharedHandlerFn)(evb, ctx, std::move(source));
            }))
        .RetiresOnSaturation();

    return sharedHandlerFn;
  }

  void parseOutput() {
    isHQ() ? parseOutputHQ() : parseOutputUniplex();
  }

  void parseOneStream(quic::MockQuicSocketDriver::StreamStatePair &stream,
                      bool skipPreface) {
    if ((stream.first & 0x3) == 2) {
      // Don't parse client-initiated uni
      return;
    }
    hq::HQUnidirectionalCodec *controlCodec{nullptr};
    class Callback : public hq::HQUnidirectionalCodec::Callback {
     public:
      void onError(HTTPCodec::StreamID, const HTTPException &, bool) override {
        EXPECT_TRUE(false);
      }
    } cb;
    hq::QPACKEncoderCodec qpackEncoderCodec(multiCodec_->getQPACKCodec(), cb);
    hq::QPACKDecoderCodec qpackDecoderCodec(multiCodec_->getQPACKCodec(), cb);
    if (!skipPreface && (stream.first & 0x3) == 3 &&
        !stream.second.writeBuf.empty()) {
      folly::io::Cursor cursor(stream.second.writeBuf.front());
      auto preface = quic::follyutils::decodeQuicInteger(cursor);
      XCHECK(preface);
      size_t toTrim = preface->second;
      auto g = folly::makeGuard(
          [&toTrim, &stream] { stream.second.writeBuf.trimStart(toTrim); });
      switch ((hq::UnidirectionalStreamType)preface->first) {
        case hq::UnidirectionalStreamType::CONTROL:
          controlCodec = multiCodec_;
          break;
        case hq::UnidirectionalStreamType::QPACK_ENCODER:
          controlCodec = &qpackEncoderCodec;
          break;
        case hq::UnidirectionalStreamType::QPACK_DECODER:
          controlCodec = &qpackDecoderCodec;
          break;
        case hq::UnidirectionalStreamType::PUSH: {
          auto pushID = quic::follyutils::decodeQuicInteger(cursor);
          multiCodec_->addCodec(stream.first);
          XCHECK(pushID);
          pushMap_.emplace(pushID->first, stream.first);
          toTrim += pushID->second;
          // Parse the rest of the push later
          return;
        }
        default:
          XLOG(FATAL) << "Bad uni stream type";
      }
    }
    if (!stream.second.writeBuf.empty()) {
      XLOG(DBG4) << "Decoding stream id=" << stream.first;
      if (controlCodec) {
        if (!stream.second.writeBuf.empty()) {
          controlCodec->onUnidirectionalIngress(stream.second.writeBuf.move());
        }
      } else {
        multiCodec_->setCurrentStream(stream.first);
        auto consumed =
            clientCodec_->onIngress(*stream.second.writeBuf.front());
        stream.second.writeBuf.trimStart(consumed);
        EXPECT_EQ(stream.second.writeBuf.chainLength(), 0);
      }
      if (stream.second.writeEOF) {
        if (controlCodec) {
          controlCodec->onUnidirectionalIngressEOF();
        } else {
          multiCodec_->setCurrentStream(stream.first);
          clientCodec_->onIngressEOF();
        }
      }
    }
  }

  void parseOutputHQ() {
    // Parse all uni control streams and pushIDs
    for (auto &stream : muxTransport_->socketDriver_.streams_) {
      if ((stream.first & 0x3) == 3 && !stream.second.writeBuf.empty()) {
        parseOneStream(stream, false);
      }
    }
    // Parse all data streams and pushes
    for (auto &stream : muxTransport_->socketDriver_.streams_) {
      parseOneStream(stream, true);
    }
  }

  // TODO: this only expects a GET request
  folly::coro::Task<HTTPCodec::StreamID> expectRequest(
      HTTPSourceHolder &requestSource,
      HTTPMethod method,
      folly::StringPiece path,
      bool eom = true,
      HTTPHeaders *headers = nullptr) {
    auto streamID = *requestSource.getStreamID();
    auto req = co_await co_awaitTry(requestSource.readHeaderEvent());
    EXPECT_FALSE(req.hasException());
    EXPECT_EQ(req->headers->getSeqNo(),
              HTTPCodec::streamIDToSeqNo(GetParam().codecProtocol, streamID));
    EXPECT_EQ(req->headers->getMethod(), method);
    EXPECT_EQ(req->headers->getPathAsStringPiece(), path);
    EXPECT_TRUE(req->isFinal());
    EXPECT_EQ(req->eom, eom);
    if (headers) {
      headers->forEach([reqHeaders = req->headers->getHeaders()](
                           const std::string &name, const std::string &value) {
        EXPECT_EQ(reqHeaders.getSingleOrEmpty(name), value);
      });
    }
    co_return streamID;
  }

  static folly::coro::Task<HTTPError> expectHeaderError(
      HTTPSourceHolder &requestSource,
      std::optional<HTTPErrorCode> expectedCode = std::nullopt) {
    auto headerEvent = co_await co_awaitTry(requestSource.readHeaderEvent());
    EXPECT_TRUE(headerEvent.hasException());
    auto err = getHTTPError(headerEvent);
    if (expectedCode) {
      EXPECT_EQ(err.code, *expectedCode);
    }
    co_return err;
  }

  static folly::coro::Task<HTTPErrorCode> expectBodyError(
      HTTPSourceHolder &requestSource,
      std::optional<HTTPErrorCode> expectedCode = std::nullopt) {
    auto bodyEvent =
        co_await co_awaitTry(readBodyEventNoSuspend(requestSource));
    EXPECT_TRUE(bodyEvent.hasException());
    auto code = getHTTPError(bodyEvent).code;
    if (expectedCode) {
      EXPECT_EQ(code, *expectedCode);
    }
    co_return code;
  }

  void expectResponseHeaders(HTTPCodec::StreamID id, uint16_t statusCode) {
    expectResponseHeaders(
        id,
        [statusCode](HTTPCodec::StreamID, std::shared_ptr<HTTPMessage> resp) {
          EXPECT_EQ(resp->getStatusCode(), statusCode);
        });
  }

  void expectResponseHeaders(
      HTTPCodec::StreamID id,
      std::function<void(HTTPCodec::StreamID, std::shared_ptr<HTTPMessage>)>
          callback = nullptr) {
    if (callback) {
      EXPECT_CALL(callbacks_, onHeadersComplete(id, _))
          .WillOnce(Invoke(std::move(callback)))
          .RetiresOnSaturation();
    } else {
      EXPECT_CALL(callbacks_, onHeadersComplete(id, _));
    }
  }

  void expectResponseBody(HTTPCodec::StreamID id) {
    EXPECT_CALL(callbacks_, onBody(id, _, _));
  }

  void expectResponseEOM(HTTPCodec::StreamID id) {
    EXPECT_CALL(callbacks_, onMessageComplete(id, _));
  }

  void expectResponse(HTTPCodec::StreamID id,
                      uint16_t statusCode,
                      HTTPHeaders *headers = nullptr,
                      bool expectBody = true) {
    expectResponseHeaders(
        id,
        [statusCode, headers](HTTPCodec::StreamID,
                              std::shared_ptr<HTTPMessage> resp) {
          EXPECT_EQ(resp->getStatusCode(), statusCode);
          if (headers) {
            headers->forEach(
                [respHeaders = resp->getHeaders()](const std::string &name,
                                                   const std::string &value) {
                  EXPECT_EQ(respHeaders.getSingleOrEmpty(name), value);
                });
          }
        });
    if (expectBody) {
      expectResponseBody(id);
    }
    expectResponseEOM(id);
  }

  HTTPCodec::StreamID sendRequest(folly::StringPiece path,
                                  std::unique_ptr<folly::IOBuf> body = nullptr,
                                  bool eom = true,
                                  bool eof = false) {
    HTTPMessage req;
    if (body) {
      req.setMethod(HTTPMethod::POST);
      // TODO: should be optional?
      req.setIsChunked(true);
    } else {
      req.setMethod(HTTPMethod::GET);
    }
    req.setURL(path);
    // TODO: some HTTP/1.0 tests?  Don't say 0.9
    req.setHTTPVersion(1, 1);
    return sendRequest(req, std::move(body), eom, eof);
  }

  HTTPCodec::StreamID sendRequest(HTTPMessage &req,
                                  std::unique_ptr<folly::IOBuf> body = nullptr,
                                  bool eom = true,
                                  bool eof = false) {

    if (eof) {
      req.setWantsKeepalive(false);
    }
    if (body) {
      req.getHeaders().set(
          HTTP_HEADER_CONTENT_LENGTH,
          folly::to<std::string>(body->computeChainDataLength()));
    }
    HTTPCodec::StreamID id = sendRequestHeader(std::move(req), eom && !body);
    if (body) {
      sendBody(id, std::move(body), eom);
    }
    transport_->addReadEvent(id, writeBuf_.move(), eom);
    if (eof) {
      generateGoaway();
      transport_->addReadEvent(nullptr, IS_H1());
    }
    return id;
  }

  HTTPCodec::StreamID createStreamID() {
    HTTPCodec::StreamID id;
    if (isHQ()) {
      id = muxTransport_->nextBidirectionalStreamId_;
      muxTransport_->nextBidirectionalStreamId_ += 4;
      multiCodec_->addCodec(id);
    } else {
      id = clientCodec_->createStream();
    }
    return id;
  }

  HTTPCodec::StreamID sendRequestHeader(HTTPMessage req,
                                        bool eom = false,
                                        bool flushQPACK = true) {
    auto id = createStreamID();
    sendRequestHeader(id, std::move(req), eom, flushQPACK);
    return id;
  }

  void sendRequestHeader(HTTPCodec::StreamID id,
                         HTTPMessage req,
                         bool eom = false,
                         bool flushQPACK = true) {
    clientCodec_->generateHeader(writeBuf_, id, req, eom);
    if (flushQPACK) {
      flushQPACKEncoder();
    }
  }

  void sendBody(HTTPCodec::StreamID id,
                std::unique_ptr<folly::IOBuf> body = nullptr,
                bool eom = true,
                bool flush = false) {
    clientCodec_->generateBody(
        writeBuf_, id, std::move(body), HTTPCodec::NoPadding, eom);
    if (flush) {
      transport_->addReadEvent(id, writeBuf_.move(), false);
    }
  }

  void sendPadding(HTTPCodec::StreamID id,
                   uint16_t amount,
                   bool flush = false) {
    clientCodec_->generatePadding(writeBuf_, id, amount);
    if (flush) {
      transport_->addReadEvent(id, writeBuf_.move(), false);
    }
  }

  HTTPSource *makeResponseWithPush(
      std::function<OnEOMSource::CallbackReturn()> eomCallback) {
    auto resp = HTTPFixedSource::makeFixedResponse(200, makeBuf(100));
    auto promise = std::make_unique<HTTPMessage>(getGetRequest("/push"));
    auto pushRespSource = HTTPFixedSource::makeFixedResponse(200, makeBuf(75));
    auto onEOMSource = new OnEOMSource(pushRespSource, eomCallback);
    resp->pushes_.emplace_back(std::move(promise), onEOMSource, false);
    return resp;
  }

  void expectPush(HTTPCodec::StreamID id,
                  bool response,
                  std::optional<ErrorCode> error) {
    EXPECT_CALL(callbacks_, onPushMessageBegin(_, id, _))
        .WillOnce(
            Invoke([this, response, error](HTTPCodec::StreamID pushID,
                                           HTTPCodec::StreamID assocStreamID,
                                           HTTPMessage * /*promise*/) {
              auto expectedPushStreamID =
                  isHQ() ? pushMap_.find(pushID)->second : pushID;
              if (response) {
                // expect a whole response
                expectResponse(expectedPushStreamID, 200);
              } else {
                if (!error || !isHQ()) {
                  // The stopSending jumps the headers in the write queue in
                  // MockQuicSocketDriver
                  expectResponseHeaders(expectedPushStreamID);
                }
                if (error) {
                  expectStreamAbort(expectedPushStreamID, *error);
                }
              }
              // Now expect the promise to be complete, %@$# gmock
              auto expectedPushId = (isHQ()) ? assocStreamID : pushID;
              EXPECT_CALL(callbacks_, onHeadersComplete(expectedPushId, _))
                  .WillOnce(Invoke([](HTTPCodec::StreamID /*id*/,
                                      std::shared_ptr<HTTPMessage> promise) {
                    EXPECT_EQ(promise->getPathAsStringPiece(), "/push");
                  }))
                  .RetiresOnSaturation();
            }));
  }

  std::shared_ptr<HandlerFn> addHandlerWithByteEvents(
      MockByteEventCallback &mockByteEventCallback,
      uint8_t headerEvents,
      uint8_t bodyEvents,
      const std::function<void(HTTPFixedSource &,
                               TestHTTPTransport &,
                               HTTPCodec &,
                               folly::IOBufQueue &)> &setupFn,
      uint32_t maxBytesPerRead = std::numeric_limits<uint32_t>::max()) {
    auto handler =
        addSimpleStrictHandler([this,
                                &mockByteEventCallback,
                                headerEvents,
                                bodyEvents,
                                setupFn,
                                maxBytesPerRead](folly::EventBase *evb,
                                                 HTTPSessionContextPtr /*ctx*/,
                                                 HTTPSourceHolder requestSource)
                                   -> folly::coro::Task<HTTPSourceHolder> {
          co_await expectRequest(requestSource, HTTPMethod::GET, "/");
          auto filter =
              new ByteEventFilter(headerEvents,
                                  bodyEvents,
                                  mockByteEventCallback.getWeakRefCountedPtr(),
                                  maxBytesPerRead);
          auto resp = HTTPFixedSource::makeFixedResponse(200, makeBuf(100));
          if (setupFn) {
            setupFn(*resp, *transport_, *clientCodec_, writeBuf_);
          }
          filter->setSource(resp);
          generateGoaway(0, ErrorCode::NO_ERROR);
          transport_->addReadEvent(nullptr, IS_H1());
          co_return filter;
        });
    return handler;
  }

  std::shared_ptr<MockHTTPHandler> handler_;
  HTTPCodec *clientCodec_{nullptr};
  folly::F14FastMap<uint64_t, HTTPCodec::StreamID> pushMap_;
};

// Use this test class for h1 only tests
using H1DownstreamSessionTest = HTTPDownstreamSessionTest;
// Use this test class for h2 only tests
using H2DownstreamSessionTest = HTTPDownstreamSessionTest;
// Use this test class for h3 only tests
using HQDownstreamSessionTest = HTTPDownstreamSessionTest;
// Use this test class for h3 datagram only tests
using HQDownstreamSessionDatagramTest = HTTPDownstreamSessionTest;
// Use this test class for h3 only tests, with static-only QPACK
using HQStaticQPACKDownstreamSessionTest = HTTPDownstreamSessionTest;
// Use this test class for h1/h2 only tests
using H12DownstreamSessionTest = HTTPDownstreamSessionTest;
// Use this test class for h2/h3 only tests
using H2QDownstreamSessionTest = HTTPDownstreamSessionTest;

TEST_P(HTTPDownstreamSessionTest, TestEOF) {
  transport_->addReadEvent(nullptr, true);
  evb_.loopOnce();
}

TEST_P(HTTPDownstreamSessionTest, Simple) {
  auto id = sendRequest("/");

  auto handler =
      addSimpleStrictHandler([this](folly::EventBase *evb,
                                    HTTPSessionContextPtr /*ctx*/,
                                    HTTPSourceHolder requestSource)
                                 -> folly::coro::Task<HTTPSourceHolder> {
        co_await expectRequest(requestSource, HTTPMethod::GET, "/");
        EXPECT_EQ(session_->numIncomingStreams(), 1);
        EXPECT_EQ(session_->numOutgoingStreams(), 0);
        generateGoaway(0, ErrorCode::NO_ERROR);
        transport_->addReadEvent(nullptr, IS_H1());
        co_return HTTPFixedSource::makeFixedResponse(200, makeBuf(100));
      });

  evb_.loop();
  expectResponse(id, 200);
  parseOutput();
}

TEST_P(HTTPDownstreamSessionTest, PaddedRequest) {
  const auto id = sendRequestHeader(getPostRequest(10));
  sendPadding(id, 20);
  sendBody(id, makeBuf(5), /*eom=*/false);
  sendPadding(id, 30);
  sendBody(id, makeBuf(5), /*eom=*/false);
  sendPadding(id, 40);
  sendBody(id, makeBuf(0), /*eom=*/true);
  transport_->addReadEvent(id, writeBuf_.move(), true);

  auto handler =
      addSimpleStrictHandler([this, id](folly::EventBase *evb,
                                        HTTPSessionContextPtr /*ctx*/,
                                        HTTPSourceHolder requestSource)
                                 -> folly::coro::Task<HTTPSourceHolder> {
        const auto id2 =
            co_await expectRequest(requestSource, HTTPMethod::POST, "/", false);
        EXPECT_EQ(id, id2);
        EXPECT_EQ(session_->numIncomingStreams(), 1);
        EXPECT_EQ(session_->numOutgoingStreams(), 0);
        auto bodyEvent =
            co_await co_awaitTry(readBodyEventNoSuspend(requestSource));
        EXPECT_EQ(HTTPBodyEvent::EventType::BODY, bodyEvent->eventType);
        EXPECT_EQ(5 + 5 + 0, bodyEvent->event.body.chainLength());
        EXPECT_TRUE(bodyEvent->eom);
        generateGoaway(0, ErrorCode::NO_ERROR);
        transport_->addReadEvent(nullptr, IS_H1());
        co_return HTTPFixedSource::makeFixedResponse(200, makeBuf(100));
      });

  evb_.loop();
  expectResponse(id, 200);
  parseOutput();
}

TEST_P(HTTPDownstreamSessionTest, HeadResponse) {
  auto req = getGetRequest();
  req.setMethod(HTTPMethod::HEAD);
  auto id = sendRequest(req);

  auto handler =
      addSimpleStrictHandler([this](folly::EventBase *evb,
                                    HTTPSessionContextPtr /*ctx*/,
                                    HTTPSourceHolder requestSource)
                                 -> folly::coro::Task<HTTPSourceHolder> {
        co_await expectRequest(requestSource, HTTPMethod::HEAD, "/");
        EXPECT_EQ(session_->numIncomingStreams(), 1);
        EXPECT_EQ(session_->numOutgoingStreams(), 0);
        generateGoaway(0, ErrorCode::NO_ERROR);
        transport_->addReadEvent(nullptr, IS_H1());
        auto resp = HTTPFixedSource::makeFixedResponse(200);
        resp->msg_->getHeaders().set(HTTP_HEADER_CONTENT_LENGTH, "100");
        co_return resp;
      });

  evb_.loop();
  HTTPHeaders expected;
  expected.set(HTTP_HEADER_CONTENT_LENGTH, "100");
  expectResponse(id, 200, &expected, false);
  parseOutput();
}

TEST_P(HTTPDownstreamSessionTest, SimpleByteEvents) {
  auto id = sendRequest("/");

  MockByteEventCallback mockByteEventCallback;
  auto handler =
      addHandlerWithByteEvents(mockByteEventCallback,
                               uint8_t(HTTPByteEvent::Type::TRANSPORT_WRITE) |
                                   uint8_t(HTTPByteEvent::Type::KERNEL_WRITE) |
                                   uint8_t(HTTPByteEvent::Type::NIC_TX) |
                                   uint8_t(HTTPByteEvent::Type::CUMULATIVE_ACK),
                               uint8_t(HTTPByteEvent::Type::TRANSPORT_WRITE) |
                                   uint8_t(HTTPByteEvent::Type::KERNEL_WRITE),
                               nullptr);

  auto expectedEvents = 6;
  EXPECT_CALL(mockByteEventCallback, onByteEvent(_))
      .Times(expectedEvents)
      .WillRepeatedly(Invoke([this, id](HTTPByteEvent event) {
        // Validate streamOffset field
        EXPECT_GT(event.streamOffset, event.bodyOffset);
        if (isHQ()) {
          // For HTTP/3, streamOffset should equal transportOffset
          EXPECT_EQ(event.streamOffset, event.transportOffset)
              << "HTTP/3 streamOffset should equal transportOffset";
        } else {
          EXPECT_LE(event.streamOffset, event.transportOffset);
        }

        if (event.type == HTTPByteEvent::Type::TRANSPORT_WRITE &&
            event.fieldSectionInfo) {
          EXPECT_EQ(event.fieldSectionInfo->type,
                    HTTPByteEvent::FieldSectionInfo::Type::HEADERS);
          EXPECT_TRUE(event.fieldSectionInfo->finalHeaders);
          EXPECT_EQ(event.bodyOffset, 0);
          EXPECT_GE(event.transportOffset,
                    event.fieldSectionInfo->size.compressed);
        } else if (event.type == HTTPByteEvent::Type::KERNEL_WRITE &&
                   event.eom) {
          EXPECT_EQ(event.bodyOffset, 100);
          if (isHQ()) {
            EXPECT_EQ(muxTransport_->socketDriver_.streams_[id].nextWriteOffset,
                      event.transportOffset);
          } else {
            EXPECT_EQ(transportState_.writeOffset, event.transportOffset);
          }
        }
      }));
  evb_.loop();
  expectResponse(id, 200);
  parseOutput();
}

TEST_P(HTTPDownstreamSessionTest, TrailerByteEvents) {
  auto id = sendRequest("/");

  MockByteEventCallback mockByteEventCallback;
  auto handler = addHandlerWithByteEvents(
      mockByteEventCallback,
      0,
      uint8_t(HTTPByteEvent::Type::TRANSPORT_WRITE),
      [](HTTPFixedSource &resp,
         TestHTTPTransport &,
         HTTPCodec &,
         folly::IOBufQueue &) {
        resp.trailers_ = std::make_unique<HTTPHeaders>();
        resp.trailers_->add("x-trailer-1", "foo");
      });

  EXPECT_CALL(mockByteEventCallback, onByteEvent(_))
      .Times(2)
      .WillRepeatedly(Invoke([](HTTPByteEvent event) {
        if (event.type == HTTPByteEvent::Type::TRANSPORT_WRITE &&
            event.fieldSectionInfo) {
          EXPECT_TRUE(event.eom);
          EXPECT_EQ(event.fieldSectionInfo->type,
                    HTTPByteEvent::FieldSectionInfo::Type::TRAILERS);
          // finalHeaders=true is questionable for trailers?
          EXPECT_TRUE(event.fieldSectionInfo->finalHeaders);
          EXPECT_EQ(event.bodyOffset, 100);
          EXPECT_GE(event.transportOffset,
                    event.fieldSectionInfo->size.compressed + 100);
        } else {
          EXPECT_EQ(event.bodyOffset, 100);
          EXPECT_FALSE(event.eom);
        }
      }));
  evb_.loop();
  expectResponse(id, 200);
  parseOutput();
}

TEST_P(HTTPDownstreamSessionTest, ByteEventErrors) {
  auto id = sendRequest("/");

  auto mockByteEventCallback = std::make_shared<MockByteEventCallback>();
  auto handler = addHandlerWithByteEvents(
      *mockByteEventCallback,
      0, /* no events */
      uint8_t(HTTPByteEvent::Type::TRANSPORT_WRITE),
      [&mockByteEventCallback](HTTPFixedSource &,
                               TestHTTPTransport &transport,
                               HTTPCodec &,
                               folly::IOBufQueue &) {
        mockByteEventCallback.reset();
      });
  evb_.loop();
  expectResponse(id, 200);
  parseOutput();
}

// Close the socket before a response can be written, byte events are registered
// and then cancelled
TEST_P(HTTPDownstreamSessionTest, ByteEventsCancelOnWriteError) {
  sendRequest("/");

  MockByteEventCallback mockByteEventCallback;
  auto handler =
      addHandlerWithByteEvents(mockByteEventCallback,
                               uint8_t(HTTPByteEvent::Type::KERNEL_WRITE),
                               uint8_t(HTTPByteEvent::Type::TRANSPORT_WRITE) |
                                   uint8_t(HTTPByteEvent::Type::KERNEL_WRITE),
                               nullptr);

  EXPECT_CALL(mockByteEventCallback, onByteEvent(_))
      .WillOnce(Invoke([this](const HTTPByteEvent &event) {
        EXPECT_EQ(event.type, HTTPByteEvent::Type::TRANSPORT_WRITE);
        if (isHQ()) {
          muxTransport_->socketDriver_.setStrictErrorCheck(false);
          muxTransport_->socketDriver_.closeConnection();
        } else {
          static_cast<TestUniplexTransport *>(transport_)->shutdownWrite();
        }
      }));
  EXPECT_CALL(mockByteEventCallback, onByteEventCanceled(_, _))
      .Times(2)
      .WillRepeatedly(
          Invoke([](const HTTPByteEvent &event, const HTTPError &err) {
            XLOG(DBG4) << "onByteEventCanceled t=" << uint8_t(event.type)
                       << " off=" << event.transportOffset;
            EXPECT_EQ(event.type, HTTPByteEvent::Type::KERNEL_WRITE);
            EXPECT_EQ(err.code, HTTPErrorCode::TRANSPORT_WRITE_ERROR);
          }));
  evb_.loop();
  expectedError_ = TransportErrorCode::NETWORK_ERROR;
}

TEST_P(H1DownstreamSessionTest, ReadDataAfterCancel) {
  EXPECT_CALL(lifecycleObs_, onWrite(_, _)).WillOnce([&]() {
    // send second request after response for prior request is written
    sendRequest("/");

    // subsequently trigger h1 codec to become not reusable
    evb_.runInEventBaseThreadAlwaysEnqueue(
        [&]() { session_->closeWhenIdle(); });
  });

  auto handler =
      addSimpleStrictHandler([this](folly::EventBase *evb,
                                    HTTPSessionContextPtr /*ctx*/,
                                    HTTPSourceHolder requestSource)
                                 -> folly::coro::Task<HTTPSourceHolder> {
        co_await expectRequest(requestSource, HTTPMethod::GET, "/");
        co_return HTTPFixedSource::makeFixedResponse(200, makeBuf(100));
      });

  auto id = sendRequest("/");
  evb_.loop();

  expectResponse(id, 200);
  parseOutput();
  // expect graceful shutdown (i.e. eof & not rst)
  XCHECK(!transportState_.closedWithReset && transportState_.writesClosed);
}

TEST_P(H2DownstreamSessionTest, ByteEventsCancelOnProtocolError) {
  sendRequest("/");

  MockByteEventCallback mockByteEventCallback;
  auto handler = addHandlerWithByteEvents(
      mockByteEventCallback,
      uint8_t(HTTPByteEvent::Type::KERNEL_WRITE),
      uint8_t(HTTPByteEvent::Type::KERNEL_WRITE),
      [](HTTPFixedSource &,
         TestHTTPTransport &transport,
         HTTPCodec &clientCodec,
         folly::IOBufQueue &writeBuf) {
        clientCodec.generateWindowUpdate(writeBuf, 0, 0x7fffffff);
        transport.addReadEvent(writeBuf.move(), false);
      });

  // Header and Body events are cancelled from different code paths.  The header
  // event is cancelled from HTTPCoroSession::connectionError -> cancelEvents,
  // the queued body event is cancelled from HTTPBodyEventQueue::clear.
  EXPECT_CALL(mockByteEventCallback, onByteEventCanceled(_, _))
      .Times(2)
      .WillRepeatedly(
          Invoke([](const HTTPByteEvent &event, const HTTPError &err) {
            EXPECT_EQ(event.type, HTTPByteEvent::Type::KERNEL_WRITE);
            EXPECT_EQ(err.code, HTTPErrorCode::FLOW_CONTROL_ERROR);
          }));
  evb_.loop();
}

TEST_P(H12DownstreamSessionTest, ByteEventTimeout) {
  session_->setByteEventTimeout(std::chrono::seconds(2));
  sendRequest("/");

  static_cast<TestUniplexTransport *>(transport_)->setByteEventsEnabled(false);
  MockByteEventCallback mockByteEventCallback;
  auto handler = addHandlerWithByteEvents(
      mockByteEventCallback, uint8_t(HTTPByteEvent::Type::NIC_TX), 0, nullptr);

  EXPECT_CALL(mockByteEventCallback, onByteEventCanceled(_, _))
      .WillOnce(Invoke([](const HTTPByteEvent &event, const HTTPError &err) {
        EXPECT_EQ(event.type, HTTPByteEvent::Type::NIC_TX);
        EXPECT_EQ(err.code, HTTPErrorCode::READ_TIMEOUT);
      }));
  evb_.loop();
}

TEST_P(H12DownstreamSessionTest, TooManyByteEvents) {
  sendRequest("/");

  MockByteEventCallback mockByteEventCallback;
  auto handler =
      addHandlerWithByteEvents(mockByteEventCallback,
                               0,
                               uint8_t(HTTPByteEvent::Type::NIC_TX) |
                                   uint8_t(HTTPByteEvent::Type::CUMULATIVE_ACK),
                               nullptr,
                               1 /*byte at a time*/);

  EXPECT_CALL(mockByteEventCallback, onByteEventCanceled(_, _)).Times(104);
  EXPECT_CALL(mockByteEventCallback, onByteEvent(_)).Times(96);

  evb_.loop();
}

TEST_P(H12DownstreamSessionTest, CancelPendingByteEvents) {
  session_->setWriteTimeout(std::chrono::milliseconds(250));
  auto id = sendRequest("/");

  MockByteEventCallback mockByteEventCallback;
  auto handler =
      addHandlerWithByteEvents(mockByteEventCallback,
                               0,
                               uint8_t(HTTPByteEvent::Type::KERNEL_WRITE) |
                                   uint8_t(HTTPByteEvent::Type::NIC_TX) |
                                   uint8_t(HTTPByteEvent::Type::CUMULATIVE_ACK),
                               nullptr,
                               50 /* two body byte events */);
  // Write timeout
  transport_->pauseWrites(id);

  EXPECT_CALL(mockByteEventCallback, onByteEventCanceled(_, _)).Times(6);
  evb_.loop();
  expectedError_ = TransportErrorCode::NETWORK_ERROR;
}

TEST_P(H12DownstreamSessionTest, TxAckBeforeScheduleByteEvents) {
  static_cast<TestUniplexTransport *>(transport_)->setFastTxAckEvents(true);
  sendRequest("/");

  MockByteEventCallback mockByteEventCallback;
  auto handler =
      addHandlerWithByteEvents(mockByteEventCallback,
                               0,
                               uint8_t(HTTPByteEvent::Type::NIC_TX) |
                                   uint8_t(HTTPByteEvent::Type::CUMULATIVE_ACK),
                               nullptr);

  EXPECT_CALL(mockByteEventCallback, onByteEvent(_)).Times(2);
  evb_.loop();
}

TEST_P(HQDownstreamSessionTest, IngressBackpressureUnderLimit) {
  // Send two 20,000 byte body post requests (which does not exceed the default
  // buffer limit).
  auto id1 =
      sendRequest("/", folly::IOBuf::copyBuffer(std::string(20000, 'a')));
  auto id2 =
      sendRequest("/", folly::IOBuf::copyBuffer(std::string(20000, 'b')));

  // Since we're below the default buffered threshold, we shouldn't be calling
  // pauseRead.
  EXPECT_CALL(*muxTransport_->socketDriver_.sock_, pauseRead(id1)).Times(0);
  EXPECT_CALL(*muxTransport_->socketDriver_.sock_, pauseRead(id2)).Times(0);

  /**
   * Created to maintain a reference to the HTTPSourceHolder so no implicit
   * invocation to stopReading() occurs when the HTTPSourceHolder goes out
   * of scope in the handler. This effectively prevents the buffered data
   * from draining.
   */
  HTTPSourceHolder reqSource1{nullptr};
  HTTPSourceHolder reqSource2{nullptr};

  auto handler1 =
      addSimpleStrictHandler([this, &reqSource1](folly::EventBase *evb,
                                                 HTTPSessionContextPtr /*ctx*/,
                                                 HTTPSourceHolder requestSource)
                                 -> folly::coro::Task<HTTPSourceHolder> {
        co_await expectRequest(requestSource, HTTPMethod::POST, "/", false);
        reqSource1 = std::move(requestSource);
        co_return HTTPFixedSource::makeFixedResponse(200, makeBuf(100));
      });

  auto handler2 =
      addSimpleStrictHandler([this, &reqSource2](folly::EventBase *evb,
                                                 HTTPSessionContextPtr /*ctx*/,
                                                 HTTPSourceHolder requestSource)
                                 -> folly::coro::Task<HTTPSourceHolder> {
        co_await expectRequest(requestSource, HTTPMethod::POST, "/", false);
        reqSource2 = std::move(requestSource);
        co_return HTTPFixedSource::makeFixedResponse(200, makeBuf(100));
      });

  loopN(3);
  reqSource1.stopReading();
  reqSource2.stopReading();
  evb_.loop();

  expectResponse(id1, 200);
  expectResponse(id2, 200);
  parseOutput();
}

TEST_P(H1DownstreamSessionTest, IngressBackpressureLimitExceeded) {
  NiceMock<MockLifecycleObserver> lifecycleCb;
  session_->addLifecycleObserver(&lifecycleCb);
  folly::coro::Baton baton;

  auto handler =
      addSimpleStrictHandler([this, &baton](folly::EventBase *evb,
                                            HTTPSessionContextPtr /*ctx*/,
                                            HTTPSourceHolder requestSource)
                                 -> folly::coro::Task<HTTPSourceHolder> {
        co_await expectRequest(requestSource, HTTPMethod::POST, "/", false);
        // simulate lack of consuming source, applying backpressure when
        // exceeding limit
        co_await baton;

        // read source to completion and return 200
        auto eomSeen = false;
        do {
          auto bodyEvent =
              co_await co_awaitTry(readBodyEventNoSuspend(requestSource));
          EXPECT_FALSE(bodyEvent.hasException());
          EXPECT_EQ(bodyEvent->eventType, HTTPBodyEvent::BODY);
          eomSeen = bodyEvent->eom;
        } while (!eomSeen);

        co_return HTTPFixedSource::makeFixedResponse(200, makeBuf(100));
      });

  auto id = sendRequestHeader(getPostRequest(72'000));
  // send first 35k, expect LifecycleObserver::onRead()
  EXPECT_CALL(lifecycleCb, onRead(_, _, _)).Times(AtLeast(1));
  sendBody(id, makeBuf(35'000));
  transport_->addReadEvent(id, writeBuf_.move());
  loopN(1);

  // send next 35k, expect LifecycleObserver::onRead() – limit is exceeded at
  // this point
  EXPECT_CALL(lifecycleCb, onRead(_, _, _)).Times(AtLeast(1));
  sendBody(id, makeBuf(35'000));
  transport_->addReadEvent(id, writeBuf_.move());
  loopN(1);

  // read loop is blocked, sending next 2k should not invoke
  // LifecycleObserver::onRead()
  EXPECT_CALL(lifecycleCb, onRead(_, _, _)).Times(0);
  sendBody(id, makeBuf(2'000));
  transport_->addReadEvent(id, writeBuf_.move());
  loopN(1);

  // retire expectation and post baton to consume source
  session_->removeLifecycleObserver(&lifecycleCb);
  baton.post();
  transport_->addReadEvent(nullptr, true);

  evb_.loop();
  expectResponse(id, 200);
  parseOutput();
}

TEST_P(H1DownstreamSessionTest, IngressBackpressureSetReadBufferLimit) {
  NiceMock<MockLifecycleObserver> lifecycleCb;
  session_->addLifecycleObserver(&lifecycleCb);
  folly::coro::Baton baton;
  session_->setReadBufferLimit(500);

  auto handler =
      addSimpleStrictHandler([this, &baton](folly::EventBase *evb,
                                            HTTPSessionContextPtr /*ctx*/,
                                            HTTPSourceHolder requestSource)
                                 -> folly::coro::Task<HTTPSourceHolder> {
        co_await expectRequest(requestSource, HTTPMethod::POST, "/", false);
        // simulate lack of consuming source, applying backpressure when
        // exceeding limit
        co_await baton;

        // read source to completion and return 200
        auto eomSeen = false;
        do {
          auto bodyEvent =
              co_await co_awaitTry(readBodyEventNoSuspend(requestSource));
          EXPECT_FALSE(bodyEvent.hasException());
          EXPECT_EQ(bodyEvent->eventType, HTTPBodyEvent::BODY);
          eomSeen = bodyEvent->eom;
        } while (!eomSeen);

        co_return HTTPFixedSource::makeFixedResponse(200, makeBuf(100));
      });

  auto id = sendRequestHeader(getPostRequest(850));
  // send first 300, expect LifecycleObserver::onRead()
  EXPECT_CALL(lifecycleCb, onRead(_, _, _)).Times(AtLeast(1));
  sendBody(id, makeBuf(300), /*eom=*/false);
  transport_->addReadEvent(id, writeBuf_.move());
  loopN(1);

  // send next 300, expect LifecycleObserver::onRead() – limit is exceeded at
  // this point
  EXPECT_CALL(lifecycleCb, onRead(_, _, _)).Times(AtLeast(1));
  sendBody(id, makeBuf(300), /*eom=*/false);
  transport_->addReadEvent(id, writeBuf_.move());
  loopN(1);

  // read loop is blocked, sending next 250 should not invoke
  // LifecycleObserver::onRead()
  EXPECT_CALL(lifecycleCb, onRead(_, _, _)).Times(0);
  sendBody(id, makeBuf(250), /*eom=*/false);
  transport_->addReadEvent(id, writeBuf_.move());
  loopN(1);

  // retire expectation and post baton to consume source
  session_->removeLifecycleObserver(&lifecycleCb);
  baton.post();
  transport_->addReadEvent(nullptr, true);

  evb_.loop();
  expectResponse(id, 200);
  parseOutput();
}

TEST_P(H1DownstreamSessionTest, IngressBackpressureLimitExceededTimeout) {
  // similar to above test, but handles the timeout case
  folly::coro::Baton baton;
  // set the conn read timeout for sake of test
  session_->setConnectionReadTimeout(std::chrono::milliseconds(200));
  // set the readBufferLimit to 0 (no-op)
  session_->setReadBufferLimit(0);
  // post baton after read timeout ms has passed
  evb_.runAfterDelay([&]() { baton.post(); }, 250);

  auto handler =
      addSimpleStrictHandler([this, &baton](folly::EventBase *evb,
                                            HTTPSessionContextPtr /*ctx*/,
                                            HTTPSourceHolder requestSource)
                                 -> folly::coro::Task<HTTPSourceHolder> {
        co_await expectRequest(requestSource, HTTPMethod::POST, "/", false);
        // block on baton
        co_await baton;

        // connectionError should have queued error in stream source
        auto bodyEvent =
            co_await co_awaitTry(readBodyEventNoSuspend(requestSource));
        EXPECT_TRUE(bodyEvent.hasException());

        co_return HTTPFixedSource::makeFixedResponse(200, makeBuf(100));
      });

  auto id = sendRequestHeader(getPostRequest(72'000));
  // send first 35k, expect LifecycleObserver::onRead()
  EXPECT_CALL(lifecycleObs_, onRead(_, _, _)).Times(AtLeast(1));
  sendBody(id, makeBuf(35'000));
  transport_->addReadEvent(id, writeBuf_.move());
  loopN(1);

  // send next 35k, expect LifecycleObserver::onRead() – limit is exceeded at
  // this point
  EXPECT_CALL(lifecycleObs_, onRead(_, _, _)).Times(AtLeast(1));
  sendBody(id, makeBuf(35'000));
  transport_->addReadEvent(id, writeBuf_.move());
  loopN(1);

  // read loop is blocked, sending next 2k should not invoke
  // LifecycleObserver::onRead()
  EXPECT_CALL(lifecycleObs_, onRead(_, _, _)).Times(0);
  sendBody(id, makeBuf(2'000));
  transport_->addReadEvent(id, writeBuf_.move());
  loopN(1);

  // retire expectation and send eof
  transport_->addReadEvent(nullptr, true);

  evb_.loop();
  expectedError_ = TransportErrorCode::NETWORK_ERROR;
}

TEST_P(HQDownstreamSessionTest, IngressBackpressureOverLimit) {
  /**
   * Created to maintain a reference to the HTTPSourceHolder so no implicit
   * invocation to stopReading() occurs when the HTTPSourceHolder goes out
   * of scope in the handler. This effectively prevents the buffered data
   * from draining.
   */
  HTTPSourceHolder reqSource1{nullptr};
  HTTPSourceHolder reqSource2{nullptr};

  /**
   * Send two 70,000+ byte POST requests (which exceeds the default buffer limit
   * and should call pauseRead()) and one 20,000 bytes POST request (which
   * doesn't exceed and should proceed to process regardless of first two
   * requests exceeding).
   */
  auto id1 =
      sendRequest("/", folly::IOBuf::copyBuffer(std::string(70000, 'a')));
  auto id2 =
      sendRequest("/", folly::IOBuf::copyBuffer(std::string(70000, 'b')));
  auto id3 =
      sendRequest("/", folly::IOBuf::copyBuffer(std::string(20000, 'c')));

  /**
   * The test is as follows:
   *
   * 1. We send two requests with 70,000+ bytes in the body.
   *
   * 2. The first two handlers `std::move` the requestSource outside of the
   *    lambda scope to prevent an implicit .stopReading(), allowing the buffer
   *    to fill up. Once we buffer more than the threshold, we expect
   *    .pauseRead() on id1 and id2. The third handler should be able to read
   *    the 20,000 byte POST request to completion since the ingress
   *    backpressure is per-stream (first two requests exceeding should not
   *    affect third request).
   */

  // Flag to be set once the third handler has completed execution. Expected to
  // be true to verify that the third request was not blocked on the first two
  // requests being limited by backpressure.
  bool handler3End = false;

  // handler for id3
  auto handler3 = addSimpleStrictHandler(
      [this, &handler3End](folly::EventBase *evb,
                           HTTPSessionContextPtr /*ctx*/,
                           HTTPSourceHolder requestSource)
          -> folly::coro::Task<HTTPSourceHolder> {
        // Handler for id3 should be able to read the post request to
        // completion
        co_await expectRequest(requestSource, HTTPMethod::POST, "/", false);
        bool eomSeen = false;
        while (!eomSeen) {
          auto bodyEvent =
              co_await co_awaitTry(readBodyEventNoSuspend(requestSource));
          EXPECT_FALSE(bodyEvent.hasException());
          EXPECT_EQ(bodyEvent->eventType, HTTPBodyEvent::BODY);
          eomSeen = bodyEvent->eom;
        }

        handler3End = true;
        co_return HTTPFixedSource::makeFixedResponse(200, makeBuf(100));
      });

  // handler for id2
  auto handler2 =
      addSimpleStrictHandler([this, &reqSource2](folly::EventBase *evb,
                                                 HTTPSessionContextPtr /*ctx*/,
                                                 HTTPSourceHolder requestSource)
                                 -> folly::coro::Task<HTTPSourceHolder> {
        co_await expectRequest(requestSource, HTTPMethod::POST, "/", false);
        reqSource2 = std::move(requestSource);
        co_return HTTPFixedSource::makeFixedResponse(200, makeBuf(100));
      });

  // handler for id1
  auto handler1 =
      addSimpleStrictHandler([this, &reqSource1](folly::EventBase *evb,
                                                 HTTPSessionContextPtr /*ctx*/,
                                                 HTTPSourceHolder requestSource)
                                 -> folly::coro::Task<HTTPSourceHolder> {
        co_await expectRequest(requestSource, HTTPMethod::POST, "/", false);
        reqSource1 = std::move(requestSource);
        co_return HTTPFixedSource::makeFixedResponse(200, makeBuf(100));
      });

  // before any requests have been processed, handler flag should be false
  EXPECT_FALSE(handler3End);

  // First two requests should have been paused for reading by the socket.
  EXPECT_CALL(*muxTransport_->socketDriver_.sock_, pauseRead(id1)).Times(1);
  EXPECT_CALL(*muxTransport_->socketDriver_.sock_, pauseRead(id2)).Times(1);

  auto fn = folly::coro::co_invoke([&]() -> folly::coro::Task<void> {
    // Third request should have been read to completion (handler3End = true).
    EXPECT_TRUE(handler3End);

    // First two requests should then be resumed after consuming some of the
    // data.
    EXPECT_CALL(*muxTransport_->socketDriver_.sock_, resumeRead(id1)).Times(1);
    EXPECT_CALL(*muxTransport_->socketDriver_.sock_, resumeRead(id2)).Times(1);

    auto bodyEvent1 = co_await co_awaitTry(readBodyEventNoSuspend(reqSource1));
    auto bodyEvent2 = co_await co_awaitTry(readBodyEventNoSuspend(reqSource2));

    EXPECT_FALSE(bodyEvent1.hasException());
    EXPECT_FALSE(bodyEvent2.hasException());

    co_return;
  });

  loopN(4);
  co_withExecutor(&evb_, std::move(fn)).start();
  evb_.loop();

  expectResponse(id1, 200);
  expectResponse(id2, 200);
  expectResponse(id3, 200);

  parseOutput();
}

TEST_P(HQDownstreamSessionTest, IngressBackpressureOverLimitDelayedQPACK) {
  /**
   * This test is very similar to the one above. It sends a large POST request
   * (70KB) without consuming the source, causing backpressure logic to kick in
   * and pause the stream. However, we will delay flushing QPACK instructions to
   * validate that when the QPACK instructions are received, we don't
   * prematurely resume reading from an ingress limited stream.
   */
  auto req = getPostRequest(70000);
  req.getHeaders().add("X-FB-Debug", "rfccffgvtvnenjkbtitkfdufddnvbecu");
  auto id = sendRequestHeader(req, /*eom=*/false, /*flushQPACK=*/false);
  sendBody(id, folly::IOBuf::copyBuffer(std::string(70000, 'a')));
  transport_->addReadEvent(id, writeBuf_.move());

  /**
   * Created to maintain a reference to the HTTPSourceHolder so no implicit
   * invocation to stopReading() occurs when the HTTPSourceHolder goes out
   * of scope in the handler. This effectively prevents the buffered data
   * from draining.
   */
  HTTPSourceHolder reqSource1{nullptr};

  /**
   * `pauseRead()` should be invoked on the quic socket twice here, once due
   * to QPACK delay, and another due to ingress limit being exceeded.
   * `resumeRead()` should never be called yet since we will not be consuming
   * the buffer or flushing QPACK instructions.
   */
  EXPECT_CALL(*muxTransport_->socketDriver_.sock_, pauseRead(id)).Times(2);
  EXPECT_CALL(*muxTransport_->socketDriver_.sock_, resumeRead(id)).Times(0);

  auto handler =
      addSimpleStrictHandler([this, &reqSource1](folly::EventBase *evb,
                                                 HTTPSessionContextPtr /*ctx*/,
                                                 HTTPSourceHolder requestSource)
                                 -> folly::coro::Task<HTTPSourceHolder> {
        co_await expectRequest(requestSource, HTTPMethod::POST, "/", false);
        reqSource1 = std::move(requestSource);
        co_return HTTPFixedSource::makeFixedResponse(200, makeBuf(100));
      });

  loopN(2);

  EXPECT_GT(multiCodec_->getQPACKEncoderWriteBuf().chainLength(), 0);

  // flushing QPACK should not call resumeRead()
  EXPECT_CALL(*muxTransport_->socketDriver_.sock_, resumeRead(id)).Times(0);
  flushQPACKEncoder();

  // send eom for the post req
  transport_->addReadEvent(id, nullptr, true);
  loopN(2);

  // consuming some buffered data should resume reading
  auto fn = folly::coro::co_invoke([&]() -> folly::coro::Task<void> {
    EXPECT_CALL(*muxTransport_->socketDriver_.sock_, resumeRead(id)).Times(1);

    auto bodyEvent = co_await co_awaitTry(readBodyEventNoSuspend(reqSource1));

    EXPECT_FALSE(bodyEvent.hasException());

    co_return;
  });

  co_withExecutor(&evb_, std::move(fn)).start();
  loopN(1);

  expectResponse(id, 200);
  parseOutput();
}

TEST_P(HQDownstreamSessionTest, HQEgressStreamLimitExceeded) {
  auto &socketDriver = muxTransport_->socketDriver_;
  auto quicSocket = socketDriver.getSocket();

  auto id = sendRequest("/");
  auto handler = addSimpleStrictHandler(
      [this, quicSocket, id](folly::EventBase *evb,
                             HTTPSessionContextPtr /*ctx*/,
                             HTTPSourceHolder requestSource)
          -> folly::coro::Task<HTTPSourceHolder> {
        co_await expectRequest(requestSource, HTTPMethod::GET, "/");

        // simulate lack of stream flow credits, set flow control wndw to 0.
        quicSocket->setStreamFlowControlWindow(id, 0);

        // response will have to be returned in batches as we incrementally gain
        // more flow control credit
        co_return HTTPFixedSource::makeFixedResponse(200, makeBuf(20000));
      });
  loopN(4);

  // Since we're blocked on egress limit, we can validate that looping one more
  // time won't write any new data
  auto numWriteChainCalls = socketDriver.getNumWriteChainInvocations(id);

  // looping again without receiving additional FC credit won't resume write
  // loop (and invoke writeChain())
  loopN(1);
  EXPECT_EQ(socketDriver.getNumWriteChainInvocations(id), numWriteChainCalls);

  // receiving additional stream FC will resume write loop.
  quicSocket->setStreamFlowControlWindow(id, 25000);
  evb_.loop();
  EXPECT_GT(socketDriver.getNumWriteChainInvocations(id), numWriteChainCalls);

  expectResponse(id, 200);
  parseOutput();
}

TEST_P(HQDownstreamSessionTest, HQEgressIndependentStreamLimitExceeded) {
  // Send two requests, only one of which is flow control blocked. The other
  // stream should complete successfully.
  auto &socketDriver = muxTransport_->socketDriver_;
  auto quicSocket = socketDriver.getSocket();

  session_->setWriteTimeout(std::chrono::milliseconds(250));

  auto id1 = sendRequest("/");
  auto id2 = sendRequest("/");
  auto handler2 = addSimpleStrictHandler(
      [this, quicSocket, id2](folly::EventBase *evb,
                              HTTPSessionContextPtr /*ctx*/,
                              HTTPSourceHolder requestSource)
          -> folly::coro::Task<HTTPSourceHolder> {
        co_await expectRequest(requestSource, HTTPMethod::GET, "/");

        // simulate lack of stream flow credits.
        quicSocket->setStreamFlowControlWindow(id2, 2000);

        // response will have to be returned in batches as we incrementally gain
        // more flow control credit
        co_return HTTPFixedSource::makeFixedResponse(200, makeBuf(20000));
      });

  auto handler1 =
      addSimpleStrictHandler([this, quicSocket](folly::EventBase *evb,
                                                HTTPSessionContextPtr /*ctx*/,
                                                HTTPSourceHolder requestSource)
                                 -> folly::coro::Task<HTTPSourceHolder> {
        co_await expectRequest(requestSource, HTTPMethod::GET, "/");
        co_return HTTPFixedSource::makeFixedResponse(200, makeBuf(20000));
      });

  evb_.loop();

  // we expect the first response to be fully completed
  expectResponse(id1, 200);

  // we expect the second response to be partially completed and timed out
  expectResponseHeaders(id2);
  EXPECT_CALL(callbacks_, onBody(id2, _, _)).Times(AtLeast(1));
  expectStreamAbort(id2, ErrorCode::FLOW_CONTROL_ERROR);
  parseOutput();
}

TEST_P(H12DownstreamSessionTest, BodyError) {
  sendRequest("/bodyError_11");
  auto handler =
      addSimpleStrictHandler([this](folly::EventBase *evb,
                                    HTTPSessionContextPtr /*ctx*/,
                                    HTTPSourceHolder requestSource)
                                 -> folly::coro::Task<HTTPSourceHolder> {
        co_await expectRequest(requestSource, HTTPMethod::GET, "/bodyError_11");
        generateGoaway(0, ErrorCode::NO_ERROR);
        co_return new ErrorSource(
            std::string("error error error error"), false, 11);
      });

  evb_.loop();
  parseOutput();
  notExpectedError_ = TransportErrorCode::TIMED_OUT;
}

TEST_P(HQStaticQPACKDownstreamSessionTest, StaticOnlyQPACK) {
  // Add some non-static headers to the request and response
  auto req = getGetRequest();
  req.getHeaders().add("X-FB-Debug", "rbjjvdgukrcbrivgehgcvtbrvnjvbgdj");
  auto id = sendRequest(req);

  auto handler =
      addSimpleStrictHandler([this](folly::EventBase *evb,
                                    HTTPSessionContextPtr /*ctx*/,
                                    HTTPSourceHolder requestSource)
                                 -> folly::coro::Task<HTTPSourceHolder> {
        co_await expectRequest(requestSource, HTTPMethod::GET, "/");
        generateGoaway(0, ErrorCode::NO_ERROR);
        auto resp = HTTPFixedSource::makeFixedResponse(200, makeBuf(100));
        resp->msg_->getHeaders().add("X-FB-Debug",
                                     "dtklfefbhjgnjudrerfnggurghgigkbi");
        co_return resp;
      });

  evb_.loop();
  expectResponse(id, 200);
  parseOutput();
  EXPECT_EQ(multiCodec_->getQPACKCodec().getCompressionInfo().egress.inserts_,
            0);
  EXPECT_EQ(multiCodec_->getQPACKCodec().getCompressionInfo().ingress.inserts_,
            0);
}

TEST_P(HTTPDownstreamSessionTest, TwoRequests) {
  MockHTTPSessionStats sessionStats;
  session_->setSessionStats(&sessionStats);

  // Spin the loop once so the settings are processed before the requests
  // arrive
  evb_.loopOnce();
  auto req = getGetRequest("/");
  // Exercise dynamic table
  req.getHeaders().add("X-FB-Debug", "cdhhhnlkjkehtdjrtbintgdcdiinngdh");
  auto id1 = sendRequest(req);
  auto id2 = sendRequest(req);

  auto handler1 =
      addSimpleStrictHandler([this](folly::EventBase *evb,
                                    HTTPSessionContextPtr /*ctx*/,
                                    HTTPSourceHolder requestSource)
                                 -> folly::coro::Task<HTTPSourceHolder> {
        co_await expectRequest(requestSource, HTTPMethod::GET, "/");
        co_return HTTPFixedSource::makeFixedResponse(200, makeBuf(100));
      });
  auto handler2 =
      addSimpleStrictHandler([this](folly::EventBase *evb,
                                    HTTPSessionContextPtr /*ctx*/,
                                    HTTPSourceHolder requestSource)
                                 -> folly::coro::Task<HTTPSourceHolder> {
        co_await expectRequest(requestSource, HTTPMethod::GET, "/");
        clientCodec_->generateGoaway(writeBuf_, 0, ErrorCode::NO_ERROR);
        transport_->addReadEvent(writeBuf_.move(), IS_H1());
        co_return HTTPFixedSource::makeFixedResponse(200, makeBuf(100));
      });

  EXPECT_CALL(sessionStats, _recordTransactionsServed(2));
  EXPECT_CALL(sessionStats, _recordTransactionOpened()).Times(2);
  EXPECT_CALL(sessionStats, _recordTransactionClosed()).Times(2);
  EXPECT_CALL(sessionStats, _recordSessionReused());
  evb_.loop();
  expectResponse(id1, 200);
  expectResponse(id2, 200);
  if (isHQ()) {
    // Expect QPACK encoder stream has the dynamic header in it
    EXPECT_EQ(
        multiCodec_->getQPACKCodec().getCompressionInfo().ingress.inserts_, 0);
  }

  parseOutput();
}

CO_TEST_P(H2DownstreamSessionTest, ResetRateLimit) {
  // send first request (not to be reset below) to verify that the session will
  // abort any active streams when the rate limit is exceeded
  auto msg = getPostRequest(/*contentLength=*/200);
  sendRequest(msg, /*body=*/nullptr, /*eom=*/false, /*eof=*/false);
  HTTPSourceHolder keepaliveReqSource{};

  // handler extracts out requestSource to prevent destructor invoking
  // ::stopReading()
  auto handler = addSimpleStrictHandler(
      [this, &keepaliveReqSource](folly::EventBase *evb,
                                  HTTPSessionContextPtr /*ctx*/,
                                  HTTPSourceHolder requestSource)
          -> folly::coro::Task<HTTPSourceHolder> {
        co_await expectRequest(
            requestSource, HTTPMethod::POST, /*path=*/"/", /*eom=*/false);
        keepaliveReqSource.setSource(requestSource.release());
        co_return HTTPFixedSource::makeFixedResponse(200);
      });
  loopN(4);

  // saturate reset rate limit, ::onIngressError will be invoked
  EXPECT_CALL(lifecycleObs_, onIngressError(_, kErrorMessage));
  constexpr uint16_t kResetRateLimit = 1'000;
  for (uint32_t i = 0; i <= kResetRateLimit; i++) {
    auto req = getGetRequest("/");
    auto id = sendRequest(req);
    resetStream(id, ErrorCode::STREAM_CLOSED);
  }
  loopN(2);

  // verify first reqSource was aborted by expecting error on reading body event
  folly::coro::co_withExecutor(
      &evb_,
      [](HTTPSourceHolder reqSource) -> folly::coro::Task<void> {
        auto bodyEvent = co_await co_awaitTry(reqSource.readBodyEvent());
        CHECK(bodyEvent.hasException());
        auto *ex = bodyEvent.tryGetExceptionObject<HTTPError>();
        EXPECT_TRUE(ex && ex->code == HTTPErrorCode::CANCEL);
      }(std::move(keepaliveReqSource)))
      .start();

  evb_.loop();
  co_return;
}

TEST_P(HTTPDownstreamSessionTest, StreamIdSeqNo) {
  // Spin the loop once so the settings are processed before the requests
  // arrive
  evb_.loopOnce();
  auto req = getGetRequest("/");
  // Exercise dynamic table
  req.getHeaders().add("X-FB-Debug", "cdhhhnlkjkehtdjrtbintgdcdiinngdh");
  auto id1 = sendRequest(req);
  auto id2 = sendRequest(req);

  auto handler1 =
      addSimpleStrictHandler([this, id1](folly::EventBase *evb,
                                         HTTPSessionContextPtr /*ctx*/,
                                         HTTPSourceHolder requestSource)
                                 -> folly::coro::Task<HTTPSourceHolder> {
        co_await expectRequest(requestSource, HTTPMethod::GET, "/");
        EXPECT_EQ(session_->getSequenceNumberFromStreamId(id1), 0);
        co_return HTTPFixedSource::makeFixedResponse(200, makeBuf(100));
      });
  auto handler2 =
      addSimpleStrictHandler([this, id2](folly::EventBase *evb,
                                         HTTPSessionContextPtr /*ctx*/,
                                         HTTPSourceHolder requestSource)
                                 -> folly::coro::Task<HTTPSourceHolder> {
        co_await expectRequest(requestSource, HTTPMethod::GET, "/");
        clientCodec_->generateGoaway(writeBuf_, 0, ErrorCode::NO_ERROR);
        transport_->addReadEvent(writeBuf_.move(), IS_H1());
        EXPECT_EQ(session_->getSequenceNumberFromStreamId(id2), 1);
        co_return HTTPFixedSource::makeFixedResponse(200, makeBuf(100));
      });

  evb_.loop();
  expectResponse(id1, 200);
  expectResponse(id2, 200);
  if (isHQ()) {
    // Expect QPACK encoder stream has the dynamic header in it
    EXPECT_EQ(
        multiCodec_->getQPACKCodec().getCompressionInfo().ingress.inserts_, 0);
  }

  parseOutput();
}

TEST_P(HTTPDownstreamSessionTest, InvalidPeerCertificate) {
  auto req = getGetRequest("/");
  auto id1 = sendRequest(req);

  MockAsyncTransportCertificate *mockCert = nullptr;
  // set up expectations
  if (isHQ()) {
    mockCert = muxTransport_->socketDriver_.mockCertificate.get();
  } else {
    auto *uniplexTransport = static_cast<TestUniplexTransport *>(transport_);
    mockCert = &uniplexTransport->mockCertificate;
  }

  EXPECT_CALL(*mockCert, getIdentity()).WillOnce(Return("bad_actor"));

  auto handler1 =
      addSimpleStrictHandler([this](folly::EventBase *evb,
                                    HTTPSessionContextPtr ctx,
                                    HTTPSourceHolder requestSource)
                                 -> folly::coro::Task<HTTPSourceHolder> {
        co_await expectRequest(requestSource, HTTPMethod::GET, "/");
        CHECK(ctx);
        auto peerIdentity = ctx->getPeerCertificate()->getIdentity();
        co_return peerIdentity == "good_actor"
            ? HTTPFixedSource::makeFixedResponse(200, "ok!")
            : HTTPFixedSource::makeFixedResponse(407, "not ok!");
      });

  evb_.loop();
  expectResponse(id1, 407);
  parseOutput();
}

TEST_P(HTTPDownstreamSessionTest, LifecycleObserver) {
  StrictMock<MockLifecycleObserver> lifecycleCb;
  EXPECT_CALL(lifecycleCb, onAttached(_));
  session_->addLifecycleObserver(&lifecycleCb);

  EXPECT_CALL(lifecycleCb, onRead(_, _, _)).Times(AtLeast(1));
  if (!IS_H1()) {
    EXPECT_CALL(lifecycleCb, onSettings(_, _));
    if (!isHQ()) {
      EXPECT_CALL(lifecycleCb, onSettingsAck(_));
    }
  }
  EXPECT_CALL(lifecycleCb, onTransactionAttached(_));
  EXPECT_CALL(lifecycleCb, onRequestBegin(_));
  EXPECT_CALL(lifecycleCb, onActivateConnection(_));
  EXPECT_CALL(lifecycleCb, onIngressMessage(_, _));
  if (IS_H1()) {
    EXPECT_CALL(lifecycleCb, onIngressEOF(_));
  }
  EXPECT_CALL(lifecycleCb, onWrite(_, _)).Times(AtLeast(1));
  EXPECT_CALL(lifecycleCb, onRequestEnd(_, _));
  EXPECT_CALL(lifecycleCb, onDrainStarted(_));
  EXPECT_CALL(lifecycleCb, onTransactionDetached(_));
  if (!IS_H1()) {
    EXPECT_CALL(lifecycleCb, onGoaway(_, _, _));
  }
  EXPECT_CALL(lifecycleCb, onDeactivateConnection(_));
  EXPECT_CALL(lifecycleCb, onDestroy(_));

  clientCodec_->generateSettingsAck(writeBuf_);
  auto id = sendRequest("/", nullptr, true, true);

  auto handler =
      addSimpleStrictHandler([this](folly::EventBase *evb,
                                    HTTPSessionContextPtr ctx,
                                    HTTPSourceHolder requestSource)
                                 -> folly::coro::Task<HTTPSourceHolder> {
        auto peerIP = ctx->getPeerAddress();
        auto localIP = ctx->getLocalAddress();
        EXPECT_GT(peerIP.getAddressStr().size(), 0);
        EXPECT_GT(localIP.getAddressStr().size(), 0);
        const auto &setupTinfo = ctx->getSetupTransportInfo();
        EXPECT_EQ(*setupTinfo.appProtocol, "blarf");
        wangle::TransportInfo curInfo;
        bool gotSocketInfo =
            ctx->getCurrentTransportInfo(&curInfo, /*includeSetupFields=*/true);
        EXPECT_TRUE(gotSocketInfo);
        EXPECT_EQ(*curInfo.appProtocol, "blarf");
        co_await expectRequest(requestSource, HTTPMethod::GET, "/");
        co_return HTTPFixedSource::makeFixedResponse(200, makeBuf(100));
      });

  evb_.loop();
  expectResponse(id, 200);
  parseOutput();
}

TEST_P(HTTPDownstreamSessionTest, LifecycleObserverRemoveCallback) {
  StrictMock<MockLifecycleObserver> lifecycleCb, otherLifecycleCb;
  EXPECT_CALL(lifecycleCb, onAttached(_));
  session_->addLifecycleObserver(&lifecycleCb);
  EXPECT_CALL(otherLifecycleCb, onAttached(_));
  session_->addLifecycleObserver(&otherLifecycleCb);

  EXPECT_CALL(lifecycleCb, onRead(_, _, _)).Times(AtLeast(1));
  EXPECT_CALL(otherLifecycleCb, onRead(_, _, _))
      .Times(AtLeast(1))
      .WillOnce([&, otherLifecycleCbPtr = &otherLifecycleCb]() {
        // remove otherLifecycleCb observer from session
        session_->removeLifecycleObserver(otherLifecycleCbPtr);
      });

  // since we removed the observer, we should never expect a call
  if (!IS_H1()) {
    EXPECT_CALL(lifecycleCb, onSettings(_, _));
    EXPECT_CALL(otherLifecycleCb, onSettings(_, _)).Times(0);
    if (!isHQ()) {
      EXPECT_CALL(lifecycleCb, onSettingsAck(_));
      EXPECT_CALL(otherLifecycleCb, onSettingsAck(_)).Times(0);
    }
  }
  EXPECT_CALL(lifecycleCb, onTransactionAttached(_));
  EXPECT_CALL(otherLifecycleCb, onTransactionAttached(_)).Times(0);

  EXPECT_CALL(lifecycleCb, onRequestBegin(_));
  EXPECT_CALL(otherLifecycleCb, onRequestBegin(_)).Times(0);

  EXPECT_CALL(lifecycleCb, onActivateConnection(_));
  EXPECT_CALL(otherLifecycleCb, onActivateConnection(_)).Times(0);

  EXPECT_CALL(lifecycleCb, onIngressMessage(_, _));
  EXPECT_CALL(otherLifecycleCb, onIngressMessage(_, _)).Times(0);

  if (IS_H1()) {
    EXPECT_CALL(lifecycleCb, onIngressEOF(_));
    EXPECT_CALL(otherLifecycleCb, onIngressEOF(_)).Times(0);
  }
  EXPECT_CALL(lifecycleCb, onWrite(_, _)).Times(AtLeast(1));
  EXPECT_CALL(otherLifecycleCb, onWrite(_, _)).Times(AnyNumber());

  EXPECT_CALL(lifecycleCb, onRequestEnd(_, _));
  EXPECT_CALL(otherLifecycleCb, onRequestEnd(_, _)).Times(0);

  EXPECT_CALL(lifecycleCb, onDrainStarted(_));
  EXPECT_CALL(otherLifecycleCb, onDrainStarted(_)).Times(0);

  EXPECT_CALL(lifecycleCb, onTransactionDetached(_));
  EXPECT_CALL(otherLifecycleCb, onTransactionDetached(_)).Times(0);

  if (!IS_H1()) {
    EXPECT_CALL(lifecycleCb, onGoaway(_, _, _));
    EXPECT_CALL(otherLifecycleCb, onGoaway(_, _, _)).Times(0);
  }

  EXPECT_CALL(lifecycleCb, onDeactivateConnection(_));
  EXPECT_CALL(otherLifecycleCb, onDeactivateConnection(_)).Times(0);

  EXPECT_CALL(lifecycleCb, onDestroy(_));
  EXPECT_CALL(otherLifecycleCb, onDestroy(_)).Times(0);

  clientCodec_->generateSettingsAck(writeBuf_);
  // two loops neede for h3 to pass
  evb_.loopOnce();
  evb_.loopOnce();
  auto id = sendRequest("/", nullptr, true, true);

  auto handler =
      addSimpleStrictHandler([this](folly::EventBase *evb,
                                    HTTPSessionContextPtr ctx,
                                    HTTPSourceHolder requestSource)
                                 -> folly::coro::Task<HTTPSourceHolder> {
        auto peerIP = ctx->getPeerAddress();
        auto localIP = ctx->getLocalAddress();
        EXPECT_GT(peerIP.getAddressStr().size(), 0);
        EXPECT_GT(localIP.getAddressStr().size(), 0);
        const auto &setupTinfo = ctx->getSetupTransportInfo();
        EXPECT_EQ(*setupTinfo.appProtocol, "blarf");
        wangle::TransportInfo curInfo;
        bool gotSocketInfo =
            ctx->getCurrentTransportInfo(&curInfo, /*includeSetupFields=*/true);
        EXPECT_TRUE(gotSocketInfo);
        EXPECT_EQ(*curInfo.appProtocol, "blarf");
        co_await expectRequest(requestSource, HTTPMethod::GET, "/");
        co_return HTTPFixedSource::makeFixedResponse(200, makeBuf(100));
      });

  evb_.loop();
  expectResponse(id, 200);
  parseOutput();
}

TEST_P(HTTPDownstreamSessionTest, MultiLifecycleObserver) {
  StrictMock<MockLifecycleObserver> lifecycleCb, otherLifecycleCb;
  EXPECT_CALL(lifecycleCb, onAttached(_));
  session_->addLifecycleObserver(&lifecycleCb);
  EXPECT_CALL(otherLifecycleCb, onAttached(_));
  session_->addLifecycleObserver(&otherLifecycleCb);

  EXPECT_CALL(lifecycleCb, onRead(_, _, _)).Times(AtLeast(1));
  EXPECT_CALL(otherLifecycleCb, onRead(_, _, _)).Times(AtLeast(1));
  if (!IS_H1()) {
    EXPECT_CALL(lifecycleCb, onSettings(_, _));
    EXPECT_CALL(otherLifecycleCb, onSettings(_, _));
    if (!isHQ()) {
      EXPECT_CALL(lifecycleCb, onSettingsAck(_));
      EXPECT_CALL(otherLifecycleCb, onSettingsAck(_));
    }
  }
  EXPECT_CALL(lifecycleCb, onTransactionAttached(_));
  EXPECT_CALL(otherLifecycleCb, onTransactionAttached(_));

  EXPECT_CALL(lifecycleCb, onRequestBegin(_));
  EXPECT_CALL(otherLifecycleCb, onRequestBegin(_));

  EXPECT_CALL(lifecycleCb, onActivateConnection(_));
  EXPECT_CALL(otherLifecycleCb, onActivateConnection(_));

  EXPECT_CALL(lifecycleCb, onIngressMessage(_, _));
  EXPECT_CALL(otherLifecycleCb, onIngressMessage(_, _));
  if (IS_H1()) {
    EXPECT_CALL(lifecycleCb, onIngressEOF(_));
    EXPECT_CALL(otherLifecycleCb, onIngressEOF(_));
  }
  EXPECT_CALL(lifecycleCb, onWrite(_, _)).Times(AtLeast(1));
  EXPECT_CALL(otherLifecycleCb, onWrite(_, _)).Times(AtLeast(1));

  EXPECT_CALL(lifecycleCb, onRequestEnd(_, _));
  EXPECT_CALL(otherLifecycleCb, onRequestEnd(_, _));

  EXPECT_CALL(lifecycleCb, onDrainStarted(_));
  EXPECT_CALL(otherLifecycleCb, onDrainStarted(_));

  EXPECT_CALL(lifecycleCb, onTransactionDetached(_));
  EXPECT_CALL(otherLifecycleCb, onTransactionDetached(_));

  if (!IS_H1()) {
    EXPECT_CALL(lifecycleCb, onGoaway(_, _, _));
    EXPECT_CALL(otherLifecycleCb, onGoaway(_, _, _));
  }

  EXPECT_CALL(lifecycleCb, onDeactivateConnection(_));
  EXPECT_CALL(otherLifecycleCb, onDeactivateConnection(_));

  EXPECT_CALL(lifecycleCb, onDestroy(_));
  EXPECT_CALL(otherLifecycleCb, onDestroy(_));

  clientCodec_->generateSettingsAck(writeBuf_);
  auto id = sendRequest("/", nullptr, true, true);

  auto handler =
      addSimpleStrictHandler([this](folly::EventBase *evb,
                                    HTTPSessionContextPtr ctx,
                                    HTTPSourceHolder requestSource)
                                 -> folly::coro::Task<HTTPSourceHolder> {
        auto peerIP = ctx->getPeerAddress();
        auto localIP = ctx->getLocalAddress();
        EXPECT_GT(peerIP.getAddressStr().size(), 0);
        EXPECT_GT(localIP.getAddressStr().size(), 0);
        const auto &setupTinfo = ctx->getSetupTransportInfo();
        EXPECT_EQ(*setupTinfo.appProtocol, "blarf");
        wangle::TransportInfo curInfo;
        bool gotSocketInfo =
            ctx->getCurrentTransportInfo(&curInfo, /*includeSetupFields=*/true);
        EXPECT_TRUE(gotSocketInfo);
        EXPECT_EQ(*curInfo.appProtocol, "blarf");
        co_await expectRequest(requestSource, HTTPMethod::GET, "/");
        co_return HTTPFixedSource::makeFixedResponse(200, makeBuf(100));
      });

  evb_.loop();
  expectResponse(id, 200);
  parseOutput();
}

TEST_P(H1DownstreamSessionTest, NonKeepAliveResponse) {
  auto req = getGetRequest();
  req.setHTTPVersion(1, 0);
  req.setWantsKeepalive(false);
  auto id = sendRequestHeader(req, true);
  transport_->addReadEvent(id, writeBuf_.move(), false);
  auto handler1 =
      addSimpleStrictHandler([&](folly::EventBase *evb,
                                 HTTPSessionContextPtr /*ctx*/,
                                 HTTPSourceHolder requestSource)
                                 -> folly::coro::Task<HTTPSourceHolder> {
        auto headerEvent = co_await requestSource.readHeaderEvent();
        co_return HTTPFixedSource::makeFixedResponse(200, makeBuf(100));
      });
  evb_.loop();
  expectResponse(id, 200);
  parseOutput();
}

TEST_P(H1DownstreamSessionTest, AntiPipeline) {
  // Send two requests in one flight.  The parse loop will pause until
  // the first request detaches.
  InSequence enforceOrder;
  auto id1 = sendRequest("/");
  auto id2 = sendRequest("/", nullptr, true, true);
  bool handler1complete = false;
  auto handler1 =
      addSimpleStrictHandler([&](folly::EventBase *evb,
                                 HTTPSessionContextPtr /*ctx*/,
                                 HTTPSourceHolder requestSource)
                                 -> folly::coro::Task<HTTPSourceHolder> {
        EXPECT_EQ(*requestSource.getStreamID(), id1);
        auto onEOMSource = new OnEOMSource(
            HTTPFixedSource::makeFixedResponse(200, makeBuf(100)),
            [&handler1complete]() -> OnEOMSource::CallbackReturn {
              handler1complete = true;
              co_return folly::none;
            });
        co_await expectRequest(requestSource, HTTPMethod::GET, "/");
        co_return onEOMSource;
      });
  auto handler2 =
      addSimpleStrictHandler([&](folly::EventBase *evb,
                                 HTTPSessionContextPtr /*ctx*/,
                                 HTTPSourceHolder requestSource)
                                 -> folly::coro::Task<HTTPSourceHolder> {
        EXPECT_EQ(*requestSource.getStreamID(), id2);
        EXPECT_TRUE(handler1complete);
        co_await expectRequest(requestSource, HTTPMethod::GET, "/");
        co_return HTTPFixedSource::makeFixedResponse(206, makeBuf(100));
      });

  evb_.loop();
  expectResponse(id1, 200);
  expectResponse(id2, 206);
  parseOutput();
}

TEST_P(H1DownstreamSessionTest, AntiPipelineCancel) {
  // Send two requests in one flight.  Cancel the readLoop while waiting for
  // the antiPipelineBaton_
  InSequence enforceOrder;
  auto id1 = sendRequest("/");
  sendRequest("/", nullptr, true, true);
  auto handler1 =
      addSimpleStrictHandler([&](folly::EventBase *evb,
                                 HTTPSessionContextPtr /*ctx*/,
                                 HTTPSourceHolder requestSource)
                                 -> folly::coro::Task<HTTPSourceHolder> {
        EXPECT_EQ(*requestSource.getStreamID(), id1);
        co_await expectRequest(requestSource, HTTPMethod::GET, "/");
        // cancel the session in the next loop
        evb->runInLoop([this] { cancellationSource_.requestCancellation(); });
        co_await folly::coro::sleepReturnEarlyOnCancel(
            std::chrono::seconds(60));
        auto cancelToken = co_await folly::coro::co_current_cancellation_token;
        EXPECT_TRUE(cancelToken.isCancellationRequested());
        co_return HTTPFixedSource::makeFixedResponse(200, makeBuf(100));
      });
  // request2 is never parsed,
  auto handler2 =
      addSimpleStrictHandler([&](folly::EventBase *evb,
                                 HTTPSessionContextPtr /*ctx*/,
                                 HTTPSourceHolder requestSource)
                                 -> folly::coro::Task<HTTPSourceHolder> {
        co_await expectHeaderError(requestSource);
        co_return nullptr;
      });
  evb_.loop();
  // request1 response is
  EXPECT_TRUE(transportState_.writeEvents.empty());
  expectedError_ = TransportErrorCode::NETWORK_ERROR;
}

// H2 only, H1 needs to test with transport read error
TEST_P(H2QDownstreamSessionTest, ResetStream) {
  sendRequest("/", nullptr, false, false);

  auto handler =
      addSimpleStrictHandler([this](folly::EventBase *evb,
                                    HTTPSessionContextPtr /*ctx*/,
                                    HTTPSourceHolder requestSource)
                                 -> folly::coro::Task<HTTPSourceHolder> {
        auto id = *requestSource.getStreamID();
        co_await expectRequest(requestSource, HTTPMethod::GET, "/", false);
        resetStream(id, ErrorCode::CANCEL);
        transport_->addReadEvent(writeBuf_.move(), true);
        auto bodyEvent =
            co_await co_awaitTry(readBodyEventNoSuspend(requestSource));
        EXPECT_TRUE(bodyEvent.hasException());
        auto err = getHTTPError(bodyEvent);
        EXPECT_TRUE(isCancelled(err.code));
        EXPECT_TRUE(err.msg.ends_with("details=received RST_STREAM from peer"));
        co_return nullptr;
      });

  evb_.loop();
  parseOutput();
}

TEST_P(H2QDownstreamSessionTest, ResetEgressIngressOpen) {
  sendRequest("/", nullptr, false, false);

  auto expectCoroCancelled =
      [](HTTPSourceHolder requestSource) -> folly::coro::Task<void> {
    auto err = co_await co_awaitTry(readBodyEventNoSuspend(requestSource));
    EXPECT_TRUE(err.hasException());
    auto httpErr = getHTTPError(err);
    EXPECT_EQ(httpErr.code, HTTPErrorCode::CORO_CANCELLED);
    EXPECT_EQ(httpErr.msg,
              "HTTP source aborted err=CORO_CANCELLED, details=Sent reset");
  };

  auto handler =
      addSimpleStrictHandler([&](folly::EventBase *evb,
                                 HTTPSessionContextPtr /*ctx*/,
                                 HTTPSourceHolder requestSource)
                                 -> folly::coro::Task<HTTPSourceHolder> {
        co_await expectRequest(requestSource, HTTPMethod::GET, "/", false);
        co_withExecutor(evb, expectCoroCancelled(std::move(requestSource)))
            .start();
        co_return new HTTPErrorSource(HTTPError(HTTPErrorCode::CANCEL));
      });

  evb_.loop();
}

TEST_P(H2QDownstreamSessionTest, ResetStreamAwaitingHeaders) {
  sendRequest("/", nullptr, false, false);

  class SleepAndErrorSource : public HTTPSource {
   public:
    folly::coro::Task<HTTPHeaderEvent> readHeaderEvent() override {
      co_await folly::coro::sleepReturnEarlyOnCancel(std::chrono::seconds(60));
      EXPECT_TRUE((co_await folly::coro::co_current_cancellation_token)
                      .isCancellationRequested());
      delete this;
      co_yield folly::coro::co_error(
          HTTPError(HTTPErrorCode::CANCEL, "cancelled by session"));
    }

    folly::coro::Task<HTTPBodyEvent> readBodyEvent(uint32_t) override {
      XCHECK(false);
    }

    void stopReading(
        folly::Optional<const proxygen::coro::HTTPErrorCode>) override {
      XCHECK(false);
    }
  };

  auto handler =
      addSimpleStrictHandler([&](folly::EventBase *evb,
                                 HTTPSessionContextPtr /*ctx*/,
                                 HTTPSourceHolder requestSource)
                                 -> folly::coro::Task<HTTPSourceHolder> {
        auto id = requestSource.getStreamID();
        co_await expectRequest(requestSource, HTTPMethod::GET, "/", false);
        // Queue a reset stream
        resetStream(*id, ErrorCode::CANCEL);
        transport_->addReadEvent(writeBuf_.move(), true);
        co_return new SleepAndErrorSource();
      });

  evb_.loop();
}

TEST_P(HTTPDownstreamSessionTest, HoldContext) {
  auto id = sendRequest("/", nullptr, true, true);

  HTTPSessionContextPtr ctxHolder;
  auto handler =
      addSimpleStrictHandler([this, &ctxHolder](folly::EventBase *evb,
                                                HTTPSessionContextPtr ctx,
                                                HTTPSourceHolder requestSource)
                                 -> folly::coro::Task<HTTPSourceHolder> {
        co_await expectRequest(requestSource, HTTPMethod::GET, "/");
        EXPECT_EQ(session_->numIncomingStreams(), 1);
        EXPECT_EQ(session_->numOutgoingStreams(), 0);
        ctxHolder = std::move(ctx);
        co_return HTTPFixedSource::makeFixedResponse(200, makeBuf(100));
      });

  loopN(2); // takes 2 loops to process
  if (isHQ()) {
    // MockQuicSocketDriver takes extra loops to egress
    loopN(2);
  }
  expectResponse(id, 200);
  parseOutput();
  evb_.loopOnce();
  EXPECT_EQ(session_->numIncomingStreams(), 0);
  ctxHolder.reset();
  evb_.loopOnce();
  if (isHQ()) {
    // H3 has to wait for all control stream data to be ack'd before closing
    evb_.loop();
  }
}

TEST_P(H2DownstreamSessionTest, ResetStreamNoError) {
  // Client sends RST_STREAM with NO_ERROR while server has not finished
  // consuming ingress.
  sendRequest("/", nullptr, /*eom=*/false, /*eof=*/false);

  auto handler =
      addSimpleStrictHandler([this](folly::EventBase * /*evb*/,
                                    HTTPSessionContextPtr /*ctx*/,
                                    HTTPSourceHolder requestSource)
                                 -> folly::coro::Task<HTTPSourceHolder> {
        auto id = *requestSource.getStreamID();
        co_await expectRequest(
            requestSource, HTTPMethod::GET, "/", /*eom=*/false);

        // Inbound RST_STREAM with NO_ERROR from the peer.
        resetStream(id, ErrorCode::NO_ERROR);
        transport_->addReadEvent(writeBuf_.move(), /*eom=*/true);

        // Expect the body read to surface a CANCEL error (normalized).
        auto bodyEvent =
            co_await co_awaitTry(readBodyEventNoSuspend(requestSource));
        EXPECT_TRUE(bodyEvent.hasException());

        auto err = getHTTPError(bodyEvent);
        EXPECT_TRUE(isCancelled(err.code));
        co_return nullptr;
      });

  evb_.loop();
}

// H2 only, H1 needs to test with transport write error
TEST_P(H2QDownstreamSessionTest, StopReadingDefault500Source) {
  // Stop reading as soon as headers are received without returning source. This
  // defaults to 500 source internally.
  auto id = sendRequest("/", nullptr, false, false);

  auto handler =
      addSimpleStrictHandler([](folly::EventBase *evb,
                                HTTPSessionContextPtr /*ctx*/,
                                HTTPSourceHolder requestSource)
                                 -> folly::coro::Task<HTTPSourceHolder> {
        requestSource.stopReading();
        co_return nullptr;
      });

  evb_.loop();
  expectResponse(id, 500, nullptr, false);
  if (isHQ()) {
    EXPECT_EQ(muxTransport_->socketDriver_.streams_[id].error,
              HTTP3::ErrorCode::HTTP_NO_ERROR);
  } else {
    expectStreamAbort(id, ErrorCode::NO_ERROR);
  }
  parseOutput();
}

TEST_P(HTTPDownstreamSessionTest, ContentLengthMismatch) {
  auto handler =
      addSimpleStrictHandler([this](folly::EventBase *evb,
                                    HTTPSessionContextPtr /*ctx*/,
                                    HTTPSourceHolder requestSource)
                                 -> folly::coro::Task<HTTPSourceHolder> {
        co_await expectRequest(requestSource, HTTPMethod::POST, "/", false);

        // body event should have an exception due to content length mismatch
        auto bodyEvent =
            co_await co_awaitTry(readBodyEventNoSuspend(requestSource));
        EXPECT_TRUE(bodyEvent.hasException());
        auto httpError = getHTTPError(bodyEvent);
        EXPECT_EQ(httpError.code, HTTPErrorCode::CONTENT_LENGTH_MISMATCH);
        EXPECT_EQ(httpError.msg.rfind(
                      "Content-Length/body mismatch on ingress: expected= ", 0),
                  0);

        co_return HTTPFixedSource::makeFixedResponse(200, makeBuf(100));
      });

  // create msg with content length = 200
  auto msg = getPostRequest(/*contentLength=*/200);
  msg.setIsChunked(true);
  msg.getHeaders().set(HTTP_HEADER_TRANSFER_ENCODING, "chunked");
  auto id = sendRequestHeader(std::move(msg));
  transport_->addReadEvent(id, writeBuf_.move(), /*eom=*/false);
  loopN(1);

  if (IS_H1()) {
    sendBody(id, makeBuf(300));
    transport_->addReadEvent(id, writeBuf_.move());
  } else {
    sendBody(id, makeBuf(100), /*eom=*/true);
    transport_->addReadEvent(id, writeBuf_.move(), /*eof(h1)/eom(hq)*/ isHQ());
  }

  evb_.loop();
  if (IS_H1()) {
    expectedError_ = TransportErrorCode::NETWORK_ERROR;
  } else {
    expectStreamAbort(id, ErrorCode::INTERNAL_ERROR);
  }
  parseOutput();
}

TEST_P(HTTPDownstreamSessionTest, HandlerThrowsResetStream) {
  auto id = sendRequest("/");

  auto handler =
      addSimpleStrictHandler([this](folly::EventBase *evb,
                                    HTTPSessionContextPtr /*ctx*/,
                                    HTTPSourceHolder requestSource)
                                 -> folly::coro::Task<HTTPSourceHolder> {
        auto headers = co_await co_awaitTry(requestSource.readHeaderEvent());
        EXPECT_FALSE(headers.hasException());
        EXPECT_TRUE(headers->eom);
        if (!isHQ()) {
          transport_->addReadEvent(nullptr, true);
        }
        throw std::runtime_error("Panic at the disco!");
        co_return nullptr;
      });

  evb_.loop();
  expectResponse(id, 500, nullptr, true);
  parseOutput();
}

// H2 only.  H1 cannot serialize body after trailers.  It's trailers or EOM
TEST_P(H2QDownstreamSessionTest, StateMachineError) {
  // Send request, send trailers (no EOM), send body
  auto req = getGetRequest();
  req.getHeaders().removeAll();
  req.getHeaders().add("cache-control", "max-age=0"); // static header
  auto id = sendRequest(req, nullptr, false, false);
  HTTPHeaders trailers;
  // Use QPACK static table only here
  trailers.set("x-forwarded-for", "");
  clientCodec_->generateTrailers(writeBuf_, id, trailers);
  if (isHQ()) {
    EXPECT_TRUE(multiCodec_->getQPACKEncoderWriteBuf().empty());
  } else {
    // Our H2 codec always marks EOM on trailers.  Clear the flag.
    auto frameHeader = writeBuf_.split(9);
    frameHeader->writableData()[4] &= ~0x01;
    frameHeader->prependChain(writeBuf_.move());
    writeBuf_.append(std::move(frameHeader));
  }
  clientCodec_->generateBody(
      writeBuf_, id, makeBuf(10), HTTPCodec::NoPadding, false);
  transport_->addReadEvent(id, writeBuf_.move(), false);

  auto handler =
      addSimpleStrictHandler([this](folly::EventBase *evb,
                                    HTTPSessionContextPtr /*ctx*/,
                                    HTTPSourceHolder requestSource)
                                 -> folly::coro::Task<HTTPSourceHolder> {
        auto err = co_await expectHeaderError(
            requestSource, HTTPErrorCode::INVALID_STATE_TRANSITION);
        XCHECK(err.httpMessage);
        EXPECT_EQ(
            err.httpMessage->getHeaders().getSingleOrEmpty("cache-control"),
            "max-age=0");
        transport_->addReadEvent(nullptr, true);
        co_return nullptr;
      });

  evb_.loop();
  expectStreamAbort(id, ErrorCode::PROTOCOL_ERROR);
  parseOutput();
}

TEST_P(HTTPDownstreamSessionTest, SimplePost) {
  auto id = sendRequest("/", makeBuf(100), true, true);

  auto handler =
      addSimpleStrictHandler([this](folly::EventBase *evb,
                                    HTTPSessionContextPtr /*ctx*/,
                                    HTTPSourceHolder requestSource)
                                 -> folly::coro::Task<HTTPSourceHolder> {
        co_await expectRequest(requestSource, HTTPMethod::POST, "/", false);
        auto bodyEvent =
            co_await co_awaitTry(readBodyEventNoSuspend(requestSource));
        EXPECT_FALSE(bodyEvent.hasException());
        EXPECT_EQ(bodyEvent->eventType, HTTPBodyEvent::BODY);
        EXPECT_EQ(bodyEvent->event.body.chainLength(), 100);
        EXPECT_TRUE(bodyEvent->eom);
        co_return HTTPFixedSource::makeFixedResponse(200, makeBuf(100));
      });

  evb_.loop();
  expectResponse(id, 200);
  parseOutput();
}

TEST_P(HTTPDownstreamSessionTest,
       HandlerGracefullyTerminateIngressFixedResponse) {
  auto id = sendRequest("/", makeBuf(2000), false);

  /**
   * We allow returning a source without reading request to completion, however
   * some caveats worth noting:
   *
   *
   * - in http/2 & http/3 case, we issue a RST_STREAM (h2) / STOP_SENDING (h3)
   *   frame after the response is sent with reason = NO_ERROR as outlined by
   *   the RFC
   *
   * - in http/1.1, in the event of returning a source gracefully (i.e. not
   * - yielding or throwing an exception), we keep the connection alive
   *
   *
   */
  auto handler =
      addSimpleStrictHandler([this](folly::EventBase *evb,
                                    HTTPSessionContextPtr /*ctx*/,
                                    HTTPSourceHolder requestSource)
                                 -> folly::coro::Task<HTTPSourceHolder> {
        co_await expectRequest(requestSource, HTTPMethod::POST, "/", false);
        co_return HTTPFixedSource::makeFixedResponse(401, "unauthorized!");
      });

  loopN(2);
  EXPECT_FALSE(transportState_.closedWithReset);
  EXPECT_FALSE(transportState_.writesClosed);
  expectResponse(id, 401, nullptr, true);
  evb_.loop();
  parseOutput();
}

TEST_P(HTTPDownstreamSessionTest, HandlerGracefullyTerminateIngressYieldError) {
  auto id = sendRequest("/", makeBuf(2000), false);
  // Yielding an error from the handler defaults to 500 status code. Almost
  // identical to the previous test, except http/1.1 behavior differs: this
  // aborts the connection as this is not considered a "graceful" close
  auto handler =
      addSimpleStrictHandler([this](folly::EventBase *evb,
                                    HTTPSessionContextPtr /*ctx*/,
                                    HTTPSourceHolder requestSource)
                                 -> folly::coro::Task<HTTPSourceHolder> {
        // returning source without reading request to completion should work
        co_await expectRequest(requestSource, HTTPMethod::POST, "/", false);
        requestSource.stopReading();
        co_yield folly::coro::co_error(
            HTTPError(HTTPErrorCode::REFUSED_STREAM, "not interested!"));
      });

  loopN(2);
  // h1 closes writes
  EXPECT_EQ(IS_H1(), transportState_.writesClosed);
  expectResponse(id, 500, nullptr, true);
  evb_.loop();
  parseOutput();
}

// Push tests -- H2/Q only, H1 doesn't support push
TEST_P(H2QDownstreamSessionTest, Push) {
  // Receive a request and send a complete response including a complete push
  auto id = sendRequest("/", nullptr, true, false);

  auto handler =
      addSimpleStrictHandler([this](folly::EventBase *evb,
                                    HTTPSessionContextPtr /*ctx*/,
                                    HTTPSourceHolder requestSource)
                                 -> folly::coro::Task<HTTPSourceHolder> {
        co_await expectRequest(requestSource, HTTPMethod::GET, "/");
        co_return makeResponseWithPush([this]() -> OnEOMSource::CallbackReturn {
          EXPECT_EQ(session_->numOutgoingStreams(), 1);
          auto pushStream = isHQ() ? 15 : 2; // hard-coded
          clientCodec_->generateGoaway(
              writeBuf_, pushStream + 4, ErrorCode::NO_ERROR);
          transport_->addReadEvent(writeBuf_.move(), false);
          co_return folly::none;
        });
      });

  evb_.loop();
  expectResponse(id, 200);
  expectPush(id, /*response=*/true, /*error=*/std::nullopt);
  parseOutput();
}

TEST_P(H2QDownstreamSessionTest, PushEgressRstStream) {
  // Receive a request and send a complete response including a push, but
  // reset the push before EOM
  auto id = sendRequest("/", nullptr, true, false);

  auto handler =
      addSimpleStrictHandler([this](folly::EventBase *evb,
                                    HTTPSessionContextPtr /*ctx*/,
                                    HTTPSourceHolder requestSource)
                                 -> folly::coro::Task<HTTPSourceHolder> {
        co_await expectRequest(requestSource, HTTPMethod::GET, "/");
        co_return makeResponseWithPush([this]() -> OnEOMSource::CallbackReturn {
          EXPECT_EQ(session_->numOutgoingStreams(), 1);
          clientCodec_->generateGoaway(writeBuf_, 19, ErrorCode::NO_ERROR);
          transport_->addReadEvent(writeBuf_.move(), false);
          co_return HTTPError(HTTPErrorCode::CANCEL, "");
        });
      });

  evb_.loop();
  expectResponse(id, 200);
  expectPush(id, /*response=*/false, ErrorCode::CANCEL);
  parseOutput();
}

TEST_P(H2QDownstreamSessionTest, PushIngressRstStream) {
  // Receive a request and send a complete response including a push, but
  // receive a RST_STREAM on the push before finishing it.
  auto id = sendRequest("/");
  auto handler =
      addSimpleStrictHandler([this](folly::EventBase *evb,
                                    HTTPSessionContextPtr /*ctx*/,
                                    HTTPSourceHolder requestSource)
                                 -> folly::coro::Task<HTTPSourceHolder> {
        co_await expectRequest(requestSource, HTTPMethod::GET, "/");
        co_return makeResponseWithPush([this]() -> OnEOMSource::CallbackReturn {
          EXPECT_EQ(session_->numOutgoingStreams(), 1);
          if (isHQ()) {
            // Wait a couple loops for the headers to get flushed
            co_await rescheduleN(2);
          }
          auto pushStream = isHQ() ? 15 : 2; // hard-coded
          resetStream(pushStream, ErrorCode::CANCEL);
          clientCodec_->generateGoaway(
              writeBuf_, pushStream + 4, ErrorCode::NO_ERROR);
          transport_->addReadEvent(writeBuf_.move(), false);
          // stall this coroutine, it will get cancelled.
          TimedBaton baton(&evb_);
          co_await baton.wait();
          EXPECT_EQ(baton.getStatus(), TimedBaton::Status::cancelled);
          co_return folly::none;
        });
      });

  evb_.loop();
  expectResponse(id, 200);
  expectPush(id, /*response=*/false, /*error=*/std::nullopt);
  parseOutput();
}

// TODO: Test HQ MAX_PUSH_ID exceeded too
TEST_P(H2QDownstreamSessionTest, EgressPushStreamLimitExceeded) {
  if (isHQ()) {
    muxTransport_->socketDriver_.setMaxUniStreams(3);
  } else {
    setTestCodecSetting(clientCodec_->getEgressSettings(),
                        SettingsId::MAX_CONCURRENT_STREAMS,
                        0);
    clientCodec_->generateSettings(writeBuf_);
    transport_->addReadEvent(writeBuf_.move(), false);
  }
  evb_.loopOnce();

  // Receive a request and try to push, but it will fail because the peer's
  // limit is 0
  auto id = sendRequest("/", nullptr, true, true);

  auto handler =
      addSimpleStrictHandler([this](folly::EventBase *evb,
                                    HTTPSessionContextPtr /*ctx*/,
                                    HTTPSourceHolder requestSource)
                                 -> folly::coro::Task<HTTPSourceHolder> {
        co_await expectRequest(requestSource, HTTPMethod::GET, "/");
        co_return makeResponseWithPush([]() -> OnEOMSource::CallbackReturn {
          XLOG(FATAL) << "EOM is never read, because session calls stopSending";
          co_return folly::none;
        });
      });

  evb_.loop();
  expectResponse(id, 200);
  parseOutput();
}

// TODO: HQ push
TEST_P(H2DownstreamSessionTest, EgressPushNotSupported) {
  setTestCodecSetting(
      clientCodec_->getEgressSettings(), SettingsId::ENABLE_PUSH, 0);
  clientCodec_->generateSettings(writeBuf_);
  transport_->addReadEvent(writeBuf_.move(), false);
  evb_.loopOnce();

  // Receive a request and try to push, but it will fail because the peer
  // disabled push
  auto id = sendRequest("/", nullptr, true, true);

  auto handler =
      addSimpleStrictHandler([this](folly::EventBase *evb,
                                    HTTPSessionContextPtr /*ctx*/,
                                    HTTPSourceHolder requestSource)
                                 -> folly::coro::Task<HTTPSourceHolder> {
        co_await expectRequest(requestSource, HTTPMethod::GET, "/");
        co_return makeResponseWithPush([]() -> OnEOMSource::CallbackReturn {
          XLOG(FATAL) << "EOM is never read, because session calls stopSending";
          co_return folly::none;
        });
      });

  evb_.loop();
  expectResponse(id, 200);
  parseOutput();
}

// H2 only, H1 doesn't support PING
// H3 supports ping differently
TEST_P(H2DownstreamSessionTest, Ping) {
  session_->sendPing();
  clientCodec_->generatePingRequest(writeBuf_, 12345);
  // TODO: technically, we should be generating a ping reply with the value
  // generated in sendPing, but it's not known until later.
  clientCodec_->generatePingReply(writeBuf_, 54321);
  EXPECT_CALL(lifecycleObs_, onPingReplySent(_));
  EXPECT_CALL(lifecycleObs_, onPingReplyReceived());
  transport_->addReadEvent(writeBuf_.move(), true);
  evb_.loop();
  EXPECT_CALL(callbacks_, onPingRequest(_));
  EXPECT_CALL(callbacks_, onPingReply(12345));
  parseOutput();
}

// H2 only, H1 doesn't have settings
// HQ doesn't invoke the onSettingsOutgoingStreamsFull callback, it can
// only become full by creating streams
TEST_P(H2DownstreamSessionTest, StreamsFull) {
  // It's full again when the conn shuts down
  EXPECT_CALL(lifecycleObs_, onSettingsOutgoingStreamsFull(_));
  EXPECT_CALL(lifecycleObs_, onSettingsOutgoingStreamsNotFull(_));
  setTestCodecSetting(
      clientCodec_->getEgressSettings(), SettingsId::MAX_CONCURRENT_STREAMS, 0);
  clientCodec_->generateSettings(writeBuf_);
  setTestCodecSetting(clientCodec_->getEgressSettings(),
                      SettingsId::MAX_CONCURRENT_STREAMS,
                      10);
  clientCodec_->generateSettings(writeBuf_);
  transport_->addReadEvent(writeBuf_.move(), true);
  evb_.loop();
}

// Flow control tests -- H2 only, H1 doesn't support flow control
// H3 does f/c in the transport
TEST_P(H2DownstreamSessionTest, EgressStreamFlowControl) {
  NiceMock<MockHTTPSessionStats> sessionStats;
  session_->setSessionStats(&sessionStats);
  EXPECT_CALL(sessionStats, _recordTransactionStalled()).Times(2);
  auto id = sendRequest("/", nullptr, false, false);
  setTestCodecSetting(
      clientCodec_->getEgressSettings(), SettingsId::INITIAL_WINDOW_SIZE, 0);
  clientCodec_->generateSettings(writeBuf_);
  transport_->addReadEvent(writeBuf_.move(), false);
  clientCodec_->generateEOM(writeBuf_, id);
  transport_->addReadEvent(id, writeBuf_.move(), false);

  auto handler1 =
      addSimpleStrictHandler([this](folly::EventBase *evb,
                                    HTTPSessionContextPtr /*ctx*/,
                                    HTTPSourceHolder requestSource)
                                 -> folly::coro::Task<HTTPSourceHolder> {
        co_await expectRequest(requestSource, HTTPMethod::GET, "/");
        co_return HTTPFixedSource::makeFixedResponse(200, makeBuf(100));
      });
  auto id2 = sendRequest("/");
  auto handler2 =
      addSimpleStrictHandler([this](folly::EventBase *evb,
                                    HTTPSessionContextPtr /*ctx*/,
                                    HTTPSourceHolder requestSource)
                                 -> folly::coro::Task<HTTPSourceHolder> {
        co_await expectRequest(requestSource, HTTPMethod::GET, "/");
        co_return HTTPFixedSource::makeFixedResponse(200, makeBuf(100));
      });

  loopN(2); // takes 2 loops to process
  {
    expectResponseHeaders(id);
    expectResponseHeaders(id2);
    parseOutput();
  }
  windowUpdate(id, 100);
  windowUpdate(id2, 100);
  transport_->addReadEvent(nullptr, true);
  evb_.loop();
  expectResponseBody(id);
  expectResponseEOM(id);
  expectResponseBody(id2);
  expectResponseEOM(id2);
  parseOutput();
}

TEST_P(H2DownstreamSessionTest, IngressStreamFlowControlError) {
  session_->setSetting(SettingsId::INITIAL_WINDOW_SIZE, 10);
  constexpr uint16_t kRequestDataLen = 50'000;
  auto id = sendRequest("/", makeBuf(kRequestDataLen), true, true);

  // peer violating stream fc should trigger a rst_stream and a connection-level
  // window_update

  auto handler =
      addSimpleStrictHandler([](folly::EventBase *evb,
                                HTTPSessionContextPtr /*ctx*/,
                                HTTPSourceHolder requestSource)
                                 -> folly::coro::Task<HTTPSourceHolder> {
        co_await expectHeaderError(requestSource,
                                   HTTPErrorCode::FLOW_CONTROL_ERROR);
        co_return nullptr;
      });

  // amount should be half the window capacity, but better not to hardcode the
  // value
  EXPECT_CALL(callbacks_, onWindowUpdate(/*id=*/0, /*amount=*/_));
  evb_.loop();
  expectStreamAbort(id, ErrorCode::FLOW_CONTROL_ERROR);
  parseOutput();
}

// H2 only, H1 doesn't send RST_STREAM
// For HQ, a stream over the limit is a protocol error from the transport
TEST_P(H2DownstreamSessionTest, IngressStreamLimitExceeded) {
  session_->setSetting(SettingsId::MAX_CONCURRENT_STREAMS, 0);

  // Refuse a request over the stream limit
  auto id = sendRequest("/", nullptr, true, true);

  evb_.loop();
  expectStreamAbort(id, ErrorCode::REFUSED_STREAM);
  parseOutput();
}

TEST_P(H1DownstreamSessionTest, EgressErrorRst) {
  // Source returns an error instead of EOM, connection closed with reset
  auto id = sendRequest("/");

  auto handler =
      addSimpleStrictHandler([&](folly::EventBase *evb,
                                 HTTPSessionContextPtr /*ctx*/,
                                 HTTPSourceHolder requestSource)
                                 -> folly::coro::Task<HTTPSourceHolder> {
        co_await expectRequest(requestSource, HTTPMethod::GET, "/", true);
        // session's codec should auto add connection: close
        co_return new OnEOMSource(
            HTTPFixedSource::makeFixedResponse(200, makeBuf(100)),
            []() -> OnEOMSource::CallbackReturn {
              co_return HTTPError(HTTPErrorCode::CANCEL, "");
            });
      });

  evb_.loop();
  expectResponseHeaders(id);
  parseOutput();
  expectedError_ = TransportErrorCode::NETWORK_ERROR;
}

TEST_P(H1DownstreamSessionTest, EgressErrorRstPipeline) {
  auto id = sendRequest("/");
  sendRequest("/");

  auto handler =
      addSimpleStrictHandler([this](folly::EventBase *evb,
                                    HTTPSessionContextPtr /*ctx*/,
                                    HTTPSourceHolder requestSource)
                                 -> folly::coro::Task<HTTPSourceHolder> {
        co_await expectRequest(requestSource, HTTPMethod::GET, "/", true);
        // session's codec should auto add connection: close
        co_return new OnEOMSource(
            HTTPFixedSource::makeFixedResponse(200, makeBuf(100)),
            []() -> OnEOMSource::CallbackReturn {
              co_return HTTPError(HTTPErrorCode::CANCEL, "");
            });
      });

  evb_.loop();
  expectResponseHeaders(id);
  parseOutput();
  expectedError_ = TransportErrorCode::NETWORK_ERROR;
}

// H1 Goaway tests
TEST_P(H1DownstreamSessionTest, ReceiveGoaway) {
  InSequence enforceOrder;
  // Send one request, expect response
  auto id1 = sendRequest("/");

  auto handler1 =
      addSimpleStrictHandler([this](folly::EventBase *evb,
                                    HTTPSessionContextPtr /*ctx*/,
                                    HTTPSourceHolder requestSource)
                                 -> folly::coro::Task<HTTPSourceHolder> {
        co_await expectRequest(requestSource, HTTPMethod::GET, "/", true);
        // session's codec should auto add connection: close
        co_return HTTPFixedSource::makeFixedResponse(200, makeBuf(100));
      });

  // Client sends request with Connection: close, server terminates connection
  // after stream completes
  clientCodec_->generateGoaway(writeBuf_, 0, ErrorCode::NO_ERROR);
  auto id2 = sendRequest("/");
  HTTPHeaders expectedHeaders;
  expectedHeaders.set(HTTP_HEADER_CONNECTION, "close");

  auto handler2 =
      addSimpleStrictHandler([&](folly::EventBase *evb,
                                 HTTPSessionContextPtr /*ctx*/,
                                 HTTPSourceHolder requestSource)
                                 -> folly::coro::Task<HTTPSourceHolder> {
        co_await expectRequest(
            requestSource, HTTPMethod::GET, "/", true, &expectedHeaders);
        // session's codec should auto add connection: close
        co_return HTTPFixedSource::makeFixedResponse(200, makeBuf(100));
      });
  evb_.loop();
  expectResponse(id1, 200);
  expectResponse(id2, 200, &expectedHeaders);
  parseOutput();
}

TEST_P(H1DownstreamSessionTest, SendConnectionClose) {
  auto id = sendRequest("/");
  session_->initiateDrain();

  auto handler =
      addSimpleStrictHandler([this](folly::EventBase *evb,
                                    HTTPSessionContextPtr /*ctx*/,
                                    HTTPSourceHolder requestSource)
                                 -> folly::coro::Task<HTTPSourceHolder> {
        co_await expectRequest(requestSource, HTTPMethod::GET, "/", true);
        // session's codec should auto add connection: close
        co_return HTTPFixedSource::makeFixedResponse(200, makeBuf(100));
      });

  evb_.loop();
  HTTPHeaders expectedHeaders;
  expectedHeaders.set(HTTP_HEADER_CONNECTION, "close");
  expectResponse(id, 200, &expectedHeaders);
  parseOutput();
}

// H2 Goaway tests
TEST_P(H2QDownstreamSessionTest, ReceiveGoaway) {
  // Server receives GOAWAY, no EOF
  generateGoaway(0, ErrorCode::NO_ERROR);
  evb_.loop();
}

TEST_P(HTTPDownstreamSessionTest, SendGoaway) {
  // Server sends GOAWAY
  session_->setConnectionReadTimeout(std::chrono::milliseconds(250));
  session_->initiateDrain();
  evb_.loop();
}

TEST_P(HTTPDownstreamSessionTest, CloseWhenIdleNoStreams) {
  // Should shutdown immediately
  session_->setConnectionReadTimeout(std::chrono::milliseconds(250));
  session_->closeWhenIdle();
  evb_.loopOnce();
  if (isHQ()) {
    loopN(3); // Sad
  }
}

TEST_P(HTTPDownstreamSessionTest, CloseWhenIdleWithStream) {
  auto id = sendRequest("/");

  auto handler =
      addSimpleStrictHandler([this](folly::EventBase *evb,
                                    HTTPSessionContextPtr /*ctx*/,
                                    HTTPSourceHolder requestSource)
                                 -> folly::coro::Task<HTTPSourceHolder> {
        co_await expectRequest(requestSource, HTTPMethod::GET, "/");
        session_->closeWhenIdle();
        co_return HTTPFixedSource::makeFixedResponse(200, makeBuf(100));
      });

  evb_.loop();
  expectResponse(id, 200);
  parseOutput();
}

TEST_P(HTTPDownstreamSessionTest, EOFWithStream) {
  auto id = sendRequestHeader(getPostRequest(20), false);
  transport_->addReadEvent(id, writeBuf_.move(), false);

  auto handler =
      addSimpleStrictHandler([this](folly::EventBase *evb,
                                    HTTPSessionContextPtr /*ctx*/,
                                    HTTPSourceHolder requestSource)
                                 -> folly::coro::Task<HTTPSourceHolder> {
        co_await expectRequest(requestSource, HTTPMethod::POST, "/", false);
        EXPECT_TRUE(requestSource);              // haven't read eom yet
        transport_->addReadEvent(nullptr, true); // and EOF
        // TODO: local/no-error = TRANSPORT_EOF
        auto expectedError = (isHQ()) ? HTTPErrorCode::TRANSPORT_READ_ERROR
                                      : HTTPErrorCode::TRANSPORT_EOF;
        co_await expectBodyError(requestSource, expectedError);
        co_return nullptr;
      });

  evb_.loop();
  if (IS_H1()) {
    // H1 egresses a 400 (body error), with no body, and EOM (should it be
    // rst?)
    expectResponse(id, 400, nullptr, false);
  } else if (isHQ()) {
    // H3 can't send anything after CONNECTION_CLOSE, and can't close the
    // socket in error, because it's gone.
  } else {
    // H2 egresses a 500 (handleRequest error)
    expectResponse(id, 500, nullptr, false);
  }
  parseOutput();
}

TEST_P(H2QDownstreamSessionTest, ReceiveGoawayWithOpenStream) {
  // Client sends request and GOAWAY, server terminates connection
  // after stream completes and final GOAWAY is sent.
  auto id = sendRequest("/");
  generateGoaway(0, ErrorCode::NO_ERROR);

  auto handler =
      addSimpleStrictHandler([this](folly::EventBase *evb,
                                    HTTPSessionContextPtr /*ctx*/,
                                    HTTPSourceHolder requestSource)
                                 -> folly::coro::Task<HTTPSourceHolder> {
        co_await expectRequest(requestSource, HTTPMethod::GET, "/");
        // Force a context switch out of this coro to ensure the GOAWAY
        // is processed
        co_await folly::coro::sleepReturnEarlyOnCancel(
            std::chrono::milliseconds(100));
        co_return HTTPFixedSource::makeFixedResponse(200, makeBuf(100));
      });
  evb_.loop();
  expectResponse(id, 200);
  expectGoaway(1, ErrorCode::NO_ERROR);
  parseOutput();
}

// HQ doesn't have GOAWAY errors
TEST_P(H2DownstreamSessionTest, ReceiveGoawayErrorWithOpenStream) {
  // Client sends unterminated POST request and GOAWAY with error and EOF.

  sendRequest("/", makeBuf(20), false);
  generateGoaway(0, ErrorCode::PROTOCOL_ERROR);
  // handleRequest is never called because the error cancels it first
  evb_.loop();
  expectGoaway(1, ErrorCode::NO_ERROR);
  parseOutput();
}

TEST_P(HTTPDownstreamSessionTest, IdleTimeoutNoStreams) {
  std::chrono::milliseconds connIdleTimeout{200};
  // Just run the loop, the session wtill timeout, drain and close
  session_->setConnectionReadTimeout(connIdleTimeout);
  auto start = std::chrono::steady_clock::now();
  evb_.loop();
  auto end = std::chrono::steady_clock::now();
  EXPECT_GE(std::chrono::duration_cast<std::chrono::milliseconds>(end - start)
                .count(),
            connIdleTimeout.count());
}

TEST_P(HQDownstreamSessionTest, QuicIdleTimeoutWithStream) {
  // Send an unterminated POST request, the deliver an idle timeout from the
  // transport
  auto id = sendRequest("/", makeBuf(20), false);
  auto handler =
      addSimpleStrictHandler([this](folly::EventBase *evb,
                                    HTTPSessionContextPtr /*ctx*/,
                                    HTTPSourceHolder requestSource)
                                 -> folly::coro::Task<HTTPSourceHolder> {
        co_await expectRequest(requestSource, HTTPMethod::POST, "/", false);
        co_await readBodyEventNoSuspend(requestSource);
        muxTransport_->socketDriver_.deliverConnectionError(quic::QuicError(
            quic::LocalErrorCode::IDLE_TIMEOUT, "idle timeout"));
        muxTransport_->socketDriver_.closeConnection();
        auto res = co_await co_awaitTry(readBodyEventNoSuspend(requestSource));
        EXPECT_TRUE(res.hasException());
        EXPECT_EQ(getHTTPError(res).code, HTTPErrorCode::TRANSPORT_EOF);
        co_return nullptr;
      });
  evb_.loop();
  EXPECT_CALL(callbacks_, onMessageBegin(id, _)).Times(0);
  parseOutput();
}

TEST_P(HQDownstreamSessionTest, IdleTimeoutResetWithPing) {
  std::chrono::milliseconds connIdleTimeout{200};
  session_->setConnectionReadTimeout(connIdleTimeout);

  for (int i = 1; i <= 4; i++) {
    muxTransport_->socketDriver_.addPingReceivedReadEvent(
        std::chrono::milliseconds(100));
  }
  // Just run the loop, the session will timeout, drain and close
  auto start = std::chrono::steady_clock::now();
  evb_.loop();
  auto end = std::chrono::steady_clock::now();
  EXPECT_GE(std::chrono::duration_cast<std::chrono::milliseconds>(end - start)
                .count(),
            connIdleTimeout.count() * 2);
}

TEST_P(H12DownstreamSessionTest, WebSocketUpgrade) {
  HTTPMessage req;
  req.setHTTPVersion(1, 1);
  req.setURL("/");
  req.getHeaders().set(HTTP_HEADER_HOST, "foo.com");
  req.setEgressWebsocketUpgrade();
  req.setMethod(HTTPMethod::GET);
  sendRequest(req, nullptr, false, false);
  HTTPStreamSource respSource(&evb_);

  auto readBlob =
      [&](HTTPSourceHolder requestSource) -> folly::coro::Task<void> {
    auto bodyEvent = co_await readBodyEventNoSuspend(requestSource);
    EXPECT_EQ(bodyEvent.eventType, HTTPBodyEvent::BODY);
    EXPECT_EQ(bodyEvent.event.body.chainLength(), 100);
    respSource.body(makeBuf(100), 0, true);
    transport_->addReadEvent(nullptr, IS_H1());
    if (!bodyEvent.eom) {
      bodyEvent = co_await readBodyEventNoSuspend(requestSource);
    }
    EXPECT_TRUE(bodyEvent.eom);
  };
  auto handler =
      addSimpleStrictHandler([&](folly::EventBase *evb,
                                 HTTPSessionContextPtr /*ctx*/,
                                 HTTPSourceHolder requestSource)
                                 -> folly::coro::Task<HTTPSourceHolder> {
        auto headerEvent = co_await requestSource.readHeaderEvent();
        if (IS_H1()) {
          EXPECT_EQ(headerEvent.headers->getMethod(), HTTPMethod::GET);
        } else {
          EXPECT_EQ(headerEvent.headers->getMethod(), HTTPMethod::CONNECT);
        }
        EXPECT_TRUE(headerEvent.headers->isIngressWebsocketUpgrade());
        EXPECT_FALSE(headerEvent.eom);
        auto resp = makeResponse(200);
        resp->setEgressWebsocketUpgrade();
        respSource.headers(std::move(resp));
        sendBody(*requestSource.getStreamID(), makeBuf(100), true, true);
        session_->closeWhenIdle();
        co_withExecutor(evb, readBlob(std::move(requestSource))).start();
        co_return &respSource;
      });
  evb_.loop();
}

// HQ write timeouts
TEST_P(H12DownstreamSessionTest, WriteTimeout) {
  // Pause writes after receiving the request so write will timeout
  session_->setWriteTimeout(std::chrono::milliseconds(250));
  sendRequest("/");

  auto handler =
      addSimpleStrictHandler([this](folly::EventBase *evb,
                                    HTTPSessionContextPtr /*ctx*/,
                                    HTTPSourceHolder requestSource)
                                 -> folly::coro::Task<HTTPSourceHolder> {
        auto id = co_await expectRequest(requestSource, HTTPMethod::GET, "/");
        transport_->pauseWrites(id);
        co_return HTTPFixedSource::makeFixedResponse(200, makeBuf(100));
      });
  evb_.loop();
  // Socket is closed with RST for H1 and H2
  expectedError_ = TransportErrorCode::NETWORK_ERROR;
}

class HTTPContinueSource : public HTTPSource {
 public:
  HTTPContinueSource() {
    setHeapAllocated();
  }

  folly::coro::Task<HTTPHeaderEvent> readHeaderEvent() override {
    if (firstTime_) {
      firstTime_ = false;
      auto msg = std::make_unique<HTTPMessage>();
      msg->setHTTPVersion(1, 1);
      msg->setStatusCode(100);
      co_return HTTPHeaderEvent(std::move(msg), false);
    } else {
      co_await folly::coro::sleepReturnEarlyOnCancel(std::chrono::seconds(60));
      delete this;
      co_yield folly::coro::co_error(HTTPError(HTTPErrorCode::PROTOCOL_ERROR));
    }
  }

  folly::coro::Task<HTTPBodyEvent> readBodyEvent(uint32_t) override {
    co_yield folly::coro::co_error(HTTPError(HTTPErrorCode::PROTOCOL_ERROR));
  }

  void stopReading(
      folly::Optional<const proxygen::coro::HTTPErrorCode>) override {
    delete this;
  }

 private:
  bool firstTime_{true};
};

TEST_P(HTTPDownstreamSessionTest, ReadTimeout) {
  // Send an incomplete POST request.  The handler will read a timeout error,
  // and the session will send a 408 Client Timeout response automatically.
  session_->setStreamReadTimeout(std::chrono::milliseconds(250));
  auto id = sendRequestHeader(getPostRequest(100));
  sendBody(id, makeBuf(10), false);
  transport_->addReadEvent(id, writeBuf_.move(), false);
  generateGoaway(0, ErrorCode::NO_ERROR);

  auto handler =
      addSimpleStrictHandler([](folly::EventBase *evb,
                                HTTPSessionContextPtr /*ctx*/,
                                HTTPSourceHolder requestSource)
                                 -> folly::coro::Task<HTTPSourceHolder> {
        auto headerEvent =
            co_await co_awaitTry(requestSource.readHeaderEvent());
        EXPECT_FALSE(headerEvent.hasException());
        EXPECT_FALSE(headerEvent->eom);
        // Start a coro to read the POST body, which never comes
        co_withExecutor(
            evb,
            [](HTTPSourceHolder requestSource) -> folly::coro::Task<void> {
              auto bodyEvent =
                  co_await co_awaitTry(readBodyEventNoSuspend(requestSource));
              EXPECT_FALSE(bodyEvent.hasException());
              EXPECT_FALSE(bodyEvent->eom);
              co_await expectBodyError(requestSource,
                                       HTTPErrorCode::READ_TIMEOUT);
            }(std::move(requestSource)))
            .start();
        // return a 100 continue, to show that we can still 408 after
        // non-final
        co_return new HTTPContinueSource();
      });
  evb_.loop();
  InSequence enforceOrder;
  expectResponseHeaders(id, 100);
  HTTPHeaders expectedHeaders;
  if (IS_H1()) {
    expectedHeaders.add(HTTP_HEADER_CONNECTION, "close");
  }
  expectResponse(id, 408, &expectedHeaders, false);
  parseOutput();
}

TEST_P(H1DownstreamSessionTest, HeadersReadTimeout) {
  // Send an incomplete GET request.  The handler will read a timeout error,
  // and the session will send a 408 Client Timeout response automatically.
  session_->setStreamReadTimeout(std::chrono::milliseconds(250));
  std::string incompleteHeaders("GET / HTTP/1.1");
  HTTPCodec::StreamID id = 1;
  transport_->addReadEvent(
      id, folly::IOBuf::copyBuffer(incompleteHeaders), false);

  auto handler =
      addSimpleStrictHandler([](folly::EventBase *evb,
                                HTTPSessionContextPtr /*ctx*/,
                                HTTPSourceHolder requestSource)
                                 -> folly::coro::Task<HTTPSourceHolder> {
        co_await expectHeaderError(requestSource, HTTPErrorCode::READ_TIMEOUT);
        // This "400" never gets read, the session generates it's own 408
        co_return HTTPFixedSource::makeFixedResponse(400);
      });
  evb_.loop();
  HTTPHeaders expectedHeaders;
  expectedHeaders.add(HTTP_HEADER_CONNECTION, "close");
  expectResponse(1, 408, &expectedHeaders, false);
  parseOutput();
}

TEST_P(H2DownstreamSessionTest, HeadersReadTimeout) {
  // Send an incomplete GET request.  H2 delays onMessageBegin until the
  // headers are complete, so the *connection* timeout will fire
  session_->setStreamReadTimeout(std::chrono::milliseconds(100));
  session_->setConnectionReadTimeout(std::chrono::milliseconds(250));
  auto id = sendRequestHeader(getGetRequest(), true);
  auto buf = writeBuf_.split(writeBuf_.chainLength() - 1);
  writeBuf_.move();
  writeBuf_.append(std::move(buf));
  transport_->addReadEvent(id, writeBuf_.move(), false);
  // No handler is created
  evb_.loop();
  // request was not received
  expectGoaway(0, ErrorCode::NO_ERROR);
  parseOutput();
}

TEST_P(H1DownstreamSessionTest, BodyReadTimeout) {
  // incomplete GET req => session detects ingress timeout => sends 408
  session_->setStreamReadTimeout(std::chrono::milliseconds(250));
  constexpr std::string_view req{"GET / HTTP/1.1"};
  HTTPCodec::StreamID id = 1;
  transport_->addReadEvent(id, folly::IOBuf::copyBuffer(req), false);

  auto handler =
      addSimpleStrictHandler([](folly::EventBase *evb,
                                HTTPSessionContextPtr /*ctx*/,
                                HTTPSourceHolder requestSource)
                                 -> folly::coro::Task<HTTPSourceHolder> {
        co_await expectHeaderError(requestSource, HTTPErrorCode::READ_TIMEOUT);
        // return exception
        co_yield folly::coro::co_error(std::runtime_error("ex"));
      });
  evb_.loop();
  HTTPHeaders expectedHeaders;
  expectedHeaders.add(HTTP_HEADER_CONNECTION, "close");
  expectResponse(1, 408, &expectedHeaders, false);
  parseOutput();
}

TEST_P(HQDownstreamSessionTest, HeadersReadTimeout) {
  // Send an incomplete GET request.
  session_->setStreamReadTimeout(std::chrono::milliseconds(100));
  session_->setConnectionReadTimeout(std::chrono::milliseconds(250));
  auto id = sendRequestHeader(getGetRequest(), true);
  auto buf = writeBuf_.split(writeBuf_.chainLength() - 1);
  writeBuf_.move();
  writeBuf_.append(std::move(buf));
  transport_->addReadEvent(id, writeBuf_.move(), false);
  auto handler =
      addSimpleStrictHandler([](folly::EventBase *evb,
                                HTTPSessionContextPtr /*ctx*/,
                                HTTPSourceHolder requestSource)
                                 -> folly::coro::Task<HTTPSourceHolder> {
        co_await expectHeaderError(requestSource, HTTPErrorCode::READ_TIMEOUT);
        // This "400" never gets read, the session generates it's own 408
        co_return HTTPFixedSource::makeFixedResponse(400);
      });
  evb_.loop();
  // request was not received
  expectResponse(id, 408, /*headers=*/nullptr, /*expectBody=*/false);
  parseOutput();
}

TEST_P(H2QDownstreamSessionTest, StreamFlowControlTimeout) {
  // Open connection flow control window, but starve the request for stream
  // flow control
  windowUpdate(4000);
  session_->setWriteTimeout(std::chrono::milliseconds(250));
  auto id = sendRequest("/");
  generateGoaway();

  auto handler =
      addSimpleStrictHandler([this](folly::EventBase *evb,
                                    HTTPSessionContextPtr /*ctx*/,
                                    HTTPSourceHolder requestSource)
                                 -> folly::coro::Task<HTTPSourceHolder> {
        co_await expectRequest(requestSource, HTTPMethod::GET, "/");
        co_return HTTPFixedSource::makeFixedResponse(200, makeBuf(70000));
      });
  evb_.loop();
  expectResponseHeaders(id);
  EXPECT_CALL(callbacks_, onBody(id, _, _)).Times(AtLeast(1));
  expectStreamAbort(id, ErrorCode::FLOW_CONTROL_ERROR);
  parseOutput();
}

TEST_P(H2DownstreamSessionTest, ConnFlowControlTimeout) {
  // Open stream flow control window, but starve the connection of flow
  // control
  NiceMock<MockHTTPSessionStats> sessionStats;
  session_->setSessionStats(&sessionStats);
  EXPECT_CALL(sessionStats, _recordSessionStalled());
  windowUpdate(1, 4000);
  session_->setWriteTimeout(std::chrono::milliseconds(250));
  auto id = sendRequest("/");

  auto handler =
      addSimpleStrictHandler([this](folly::EventBase *evb,
                                    HTTPSessionContextPtr /*ctx*/,
                                    HTTPSourceHolder requestSource)
                                 -> folly::coro::Task<HTTPSourceHolder> {
        co_await expectRequest(requestSource, HTTPMethod::GET, "/");
        co_return HTTPFixedSource::makeFixedResponse(200, makeBuf(70000));
      });
  evb_.loop();
  expectResponseHeaders(id);
  EXPECT_CALL(callbacks_, onBody(id, _, _)).Times(AtLeast(1));
  expectGoaway(id, ErrorCode::FLOW_CONTROL_ERROR);
  parseOutput();
}

TEST_P(HQDownstreamSessionTest, ConnFlowControlTimeout) {
  // Open stream flow control window, but starve the connection of flow
  // control
  session_->setWriteTimeout(std::chrono::milliseconds(250));
  muxTransport_->socketDriver_.setConnectionFlowControlWindow(6000);
  auto id = sendRequest("/");

  auto handler =
      addSimpleStrictHandler([this](folly::EventBase *evb,
                                    HTTPSessionContextPtr /*ctx*/,
                                    HTTPSourceHolder requestSource)
                                 -> folly::coro::Task<HTTPSourceHolder> {
        co_await expectRequest(requestSource, HTTPMethod::GET, "/");
        co_return HTTPFixedSource::makeFixedResponse(200, makeBuf(70000));
      });

  evb_.loop();
  expectResponseHeaders(id);
  EXPECT_CALL(callbacks_, onBody(id, _, _)).Times(AtLeast(1));
  expectedError_ = TransportErrorCode::NETWORK_ERROR;
  parseOutput();
}

// HQ doesn't have any backpressure now
TEST_P(H2DownstreamSessionTest, TransactionBufferTimeout) {
  // Open 2 streams.  Stream 1 keeps getting all the conn flow control,
  // eventually stream 2 will timeout.
  session_->setWriteTimeout(std::chrono::milliseconds(250));
  auto id1 = sendRequest("/");
  auto id2 = sendRequest("/");
  generateGoaway();

  auto handler1 =
      addSimpleStrictHandler([this](folly::EventBase *evb,
                                    HTTPSessionContextPtr /*ctx*/,
                                    HTTPSourceHolder requestSource)
                                 -> folly::coro::Task<HTTPSourceHolder> {
        co_await expectRequest(requestSource, HTTPMethod::GET, "/");
        co_return HTTPFixedSource::makeFixedResponse(
            200, makeBuf(http2::kInitialWindow * 2));
      });
  auto handler2 =
      addSimpleStrictHandler([this](folly::EventBase *evb,
                                    HTTPSessionContextPtr /*ctx*/,
                                    HTTPSourceHolder requestSource)
                                 -> folly::coro::Task<HTTPSourceHolder> {
        co_await expectRequest(requestSource, HTTPMethod::GET, "/");
        co_return HTTPFixedSource::makeFixedResponse(
            200, makeBuf(http2::kInitialWindow * 2));
      });
  auto grantFlowControlFn = [this] {
    windowUpdate(http2::kInitialWindow);
    windowUpdate(1, http2::kInitialWindow);
  };
  // Will reset the conn flow control timer but not the stream's buf space
  // timer
  evb_.runAfterDelay(grantFlowControlFn, 150);
  evb_.loop();
  expectResponseHeaders(id1);
  EXPECT_CALL(callbacks_, onBody(id1, _, _)).Times(AtLeast(1));
  expectResponseHeaders(id2);
  expectStreamAbort(id2, ErrorCode::INTERNAL_ERROR);
  parseOutput();
}

// H2 only, HQ needs a different way to generate a session error
TEST_P(H2DownstreamSessionTest, CodecError) {
  // Send a stream with ID=0
  if (isHQ()) {
    multiCodec_->addCodec(1);
  }
  clientCodec_->generateHeader(writeBuf_, 1, getGetRequest(), true);
  auto writeBuf = writeBuf_.move();
  writeBuf->writableData()[8] = 0;
  transport_->addReadEvent(1, std::move(writeBuf), false);

  evb_.loop();
  expectGoaway(0, ErrorCode::PROTOCOL_ERROR);
  parseOutput();
}

TEST_P(H1DownstreamSessionTest, CodecHTTPError) {
  std::string badRequest("BLARF");
  transport_->addReadEvent(1, folly::IOBuf::copyBuffer(badRequest), false);
  evb_.loop();
  expectResponse(1, 400, nullptr, false);
  parseOutput();
}

TEST_P(H2DownstreamSessionTest, CodecHTTPError) {
  HTTPMessage req = getGetRequest();
  req.getHeaders().add(HTTP_HEADER_CONTENT_LENGTH, "100");
  req.getHeaders().add(HTTP_HEADER_CONTENT_LENGTH, "200");
  auto id = sendRequestHeader(req, true);
  transport_->addReadEvent(id, writeBuf_.move(), false);
  generateGoaway();

  evb_.loop();
  expectResponse(id, 400, nullptr, false);
  parseOutput();
}

TEST_P(HQDownstreamSessionTest, CodecHTTPError) {
  HTTPMessage req = getGetRequest();
  req.getHeaders().add(HTTP_HEADER_CONTENT_LENGTH, "100");
  req.getHeaders().add(HTTP_HEADER_CONTENT_LENGTH, "200");
  auto id = sendRequestHeader(req, true);
  transport_->addReadEvent(id, writeBuf_.move(), true);
  auto handler =
      addSimpleStrictHandler([](folly::EventBase *evb,
                                HTTPSessionContextPtr /*ctx*/,
                                HTTPSourceHolder requestSource)
                                 -> folly::coro::Task<HTTPSourceHolder> {
        co_await expectHeaderError(requestSource,
                                   HTTPErrorCode::HEADER_PARSE_ERROR);
        // This "408" never gets read, the session generates it's own 400
        co_return HTTPFixedSource::makeFixedResponse(408);
      });
  generateGoaway();

  evb_.loop();
  expectResponse(id, 400, nullptr, false);
  parseOutput();
}

TEST_P(H1DownstreamSessionTest, CodecHTTPErrorStreamEgressComplete) {
  std::string malformedChunkHeader("z\r\n");
  auto req = getPostRequest(100);
  req.setIsChunked(true);
  req.getHeaders().set(HTTP_HEADER_TRANSFER_ENCODING, "chunked");
  auto id = sendRequestHeader(req);
  transport_->addReadEvent(id, writeBuf_.move(), false);
  auto handler =
      addSimpleStrictHandler([this](folly::EventBase *evb,
                                    HTTPSessionContextPtr /*ctx*/,
                                    HTTPSourceHolder requestSource)
                                 -> folly::coro::Task<HTTPSourceHolder> {
        auto event = co_await requestSource.readHeaderEvent();
        // Async wait for ingress error but return the response early.
        co_withExecutor(
            &evb_,
            [](HTTPSourceHolder requestSource) -> folly::coro::Task<void> {
              co_await expectBodyError(requestSource);
            }(std::move(requestSource)))
            .start();
        co_return HTTPFixedSource::makeFixedResponse(301);
      });
  loopN(2); // Takes 2 loops to process
  // Bad chunk header
  transport_->addReadEvent(
      id, folly::IOBuf::copyBuffer(malformedChunkHeader), false);

  evb_.loop();
  expectResponse(id, 301, nullptr, false);
  parseOutput();
  expectedError_ = TransportErrorCode::NETWORK_ERROR;
}

TEST_P(H1DownstreamSessionTest, CodecHTTPErrorStreamReadInProgress) {
  std::string malformedChunkHeader("z\r\n");
  auto req = getPostRequest(100);
  req.setIsChunked(true);
  req.getHeaders().set(HTTP_HEADER_TRANSFER_ENCODING, "chunked");
  auto id = sendRequestHeader(req);
  transport_->addReadEvent(id, writeBuf_.move(), false);
  transport_->addReadEvent(
      id, folly::IOBuf::copyBuffer(malformedChunkHeader), false);
  // handleRequest is never called because the error cancels it first
  evb_.loop();
  expectResponse(id, 400, nullptr, false);
  parseOutput();
}

TEST_P(HTTPDownstreamSessionTest, HandlerNoResponse) {
  // Handler returns nullptr instead of a response source
  auto id = sendRequest("/", nullptr, true, true);
  addSimpleStrictHandler([](folly::EventBase *evb,
                            HTTPSessionContextPtr /*ctx*/,
                            HTTPSourceHolder requestSource)
                             -> folly::coro::Task<HTTPSourceHolder> {
    auto event = co_await co_awaitTry(requestSource.readHeaderEvent());
    EXPECT_TRUE(event->eom);
    co_return nullptr;
  });
  evb_.loop();
  expectResponse(id, 500, nullptr, false);
  parseOutput();
}

TEST_P(HTTPDownstreamSessionTest, ResponseHeadersError) {
  // Return a response that immediately errors in readHeaderEvent
  auto id = sendRequest("/", nullptr, true, true);
  addSimpleStrictHandler([](folly::EventBase *evb,
                            HTTPSessionContextPtr /*ctx*/,
                            HTTPSourceHolder requestSource)
                             -> folly::coro::Task<HTTPSourceHolder> {
    auto event = co_await co_awaitTry(requestSource.readHeaderEvent());
    EXPECT_TRUE(event->eom);
    co_return new HTTPErrorSource(HTTPError(HTTPErrorCode::ENHANCE_YOUR_CALM));
  });
  evb_.loop();
  if (IS_H1()) {
    expectedError_ = TransportErrorCode::NETWORK_ERROR;
  } else {
    expectStreamAbort(id, ErrorCode::ENHANCE_YOUR_CALM);
    parseOutput();
  }
}

CO_TEST_P_X(HTTPDownstreamSessionTest, IngressErrorCustomResponse) {
  /**
   * On specific ingress errors (e.g. timeout), we allow a handler to egress a
   * custom http response as long as it contains the expected status code (e.g.
   * 408 in timeout case).
   *
   * We send headers but no body event, causing the ::readBodyEvent invocation
   * to timeout – since we're generating the expected status code (408), the
   * session won't override the response
   */
  session_->setStreamReadTimeout(std::chrono::milliseconds(50));

  auto handler =
      addSimpleStrictHandler([&](folly::EventBase *evb,
                                 HTTPSessionContextPtr /*ctx*/,
                                 HTTPSourceHolder requestSource)
                                 -> folly::coro::Task<HTTPSourceHolder> {
        auto headers = co_await co_awaitTry(
            expectRequest(requestSource, HTTPMethod::POST, "/", false));
        CHECK(headers.hasValue());

        auto res = co_await co_awaitTry(readBodyEventNoSuspend(requestSource));
        auto err = res.tryGetExceptionObject<HTTPError>();
        EXPECT_TRUE(err && err->code == HTTPErrorCode::READ_TIMEOUT);
        co_return HTTPFixedSource::makeFixedResponse(
            408, "custom response generated");
      });

  // send msg headers but no body triggering ::readBodyEvent to t/o
  auto msg = getChunkedPostRequest();
  auto id = sendRequestHeader(std::move(msg));
  transport_->addReadEvent(id, writeBuf_.move(), /*eom=*/false);

  folly::coro::Baton waitForEgressEom;
  EXPECT_CALL(lifecycleObs_, onDrainStarted(_)).WillOnce([&]() {
    waitForEgressEom.post();
  });
  co_await waitForEgressEom;
  expectResponseHeaders(id, 408);
  EXPECT_CALL(callbacks_, onBody(id, _, _)).WillOnce([](auto, auto body, auto) {
    EXPECT_EQ(body->template to<std::string>(), "custom response generated");
  });
  expectResponseEOM(id);
  parseOutput();
}

CO_TEST_P_X(HTTPDownstreamSessionTest, IngressErrorCustomResponseMismatch) {
  /**
   * Similar to the unit test above – but if the handler does not yield the
   * expected response status code, we will generate a default http response on
   * behalf the app/handler
   */
  session_->setStreamReadTimeout(std::chrono::milliseconds(50));

  auto handler =
      addSimpleStrictHandler([&](folly::EventBase *evb,
                                 HTTPSessionContextPtr /*ctx*/,
                                 HTTPSourceHolder requestSource)
                                 -> folly::coro::Task<HTTPSourceHolder> {
        auto headers = co_await co_awaitTry(
            expectRequest(requestSource, HTTPMethod::POST, "/", false));
        CHECK(headers.hasValue());

        auto res = co_await co_awaitTry(readBodyEventNoSuspend(requestSource));
        auto err = res.tryGetExceptionObject<HTTPError>();
        EXPECT_TRUE(err && err->code == HTTPErrorCode::READ_TIMEOUT);
        co_return HTTPFixedSource::makeFixedResponse(
            500, "custom response generated");
      });

  // send msg headers but no body triggering ::readBodyEvent to t/o
  auto msg = getChunkedPostRequest();
  auto id = sendRequestHeader(std::move(msg));
  transport_->addReadEvent(id, writeBuf_.move(), /*eom=*/false);

  folly::coro::Baton waitForEgressEom;
  EXPECT_CALL(lifecycleObs_, onDrainStarted(_)).WillOnce([&]() {
    waitForEgressEom.post();
  });
  co_await waitForEgressEom;
  expectResponse(id, 408, /*headers=*/nullptr, /*expectBody=*/false);
  parseOutput();
}

// H3 specific tests
TEST_P(HQDownstreamSessionTest, CreateControlStreamFail) {
  // Kill the default session
  session_->closeWhenIdle();

  // Make a new muxTransport with no uni credit
  TestCoroMultiplexTransport muxTransport(&evb_, direction_);
  transport_ = &muxTransport;
  muxTransport.socketDriver_.setMaxUniStreams(0);
  auto codec = std::make_unique<hq::HQMultiCodec>(direction_);
  wangle::TransportInfo tinfo;
  auto session = HTTPCoroSession::makeDownstreamCoroSession(
      muxTransport.getSocket(), handler_, std::move(codec), std::move(tinfo));
  NiceMock<MockLifecycleObserver> lifecycleCb;
  session->addLifecycleObserver(&lifecycleCb);
  folly::coro::co_withCancellation(cancellationSource_.getToken(),
                                   session->run())
      .start();
  EXPECT_CALL(lifecycleCb, onDestroy(_));
  loopN(4);
  EXPECT_EQ(HTTP3::ErrorCode(*muxTransport.socketDriver_.getConnErrorCode()),
            HTTP3::ErrorCode::HTTP_STREAM_CREATION_ERROR);
}

TEST_P(HQDownstreamSessionTest, StopSending) {
  auto id = sendRequestHeader(getPostRequest(20), false);
  transport_->addReadEvent(id, writeBuf_.move(), false);

  auto handler =
      addSimpleStrictHandler([this, id](folly::EventBase *evb,
                                        HTTPSessionContextPtr /*ctx*/,
                                        HTTPSourceHolder requestSource)
                                 -> folly::coro::Task<HTTPSourceHolder> {
        co_await expectRequest(requestSource, HTTPMethod::POST, "/", false);
        muxTransport_->socketDriver_.addStopSending(
            id, HTTP3::ErrorCode::HTTP_EXCESSIVE_LOAD);
        co_await expectBodyError(requestSource, HTTPErrorCode::CORO_CANCELLED);
        generateGoaway(0, ErrorCode::NO_ERROR);
        co_return nullptr;
      });

  evb_.loop();
  expectStreamAbort(id, ErrorCode::CANCEL);
  parseOutput();
}

TEST_P(HQDownstreamSessionTest, UniDispatchEdgeCases) {
  loopN(2); // to finish dispatching control stream

  // === EOF only
  auto id = muxTransport_->nextUnidirectionalStreamId_;
  muxTransport_->nextUnidirectionalStreamId_ += 4;
  transport_->addReadEvent(id, nullptr, true);
  loopN(2); // Sadly takes 2 looks after first read event until the peek fires
  // Now the peek cb is gone
  EXPECT_TRUE(muxTransport_->socketDriver_.streams_[id].peekCB == nullptr);

  // == Stream without enough data to dispatch
  std::array<uint8_t, 2> twoByteGreaseId{0x40, 0x33};
  id = muxTransport_->nextUnidirectionalStreamId_;
  muxTransport_->nextUnidirectionalStreamId_ += 4;
  transport_->addReadEvent(
      id, folly::IOBuf::wrapBuffer(twoByteGreaseId.data(), 1), false);
  loopN(2);
  // Still has a peek cb installed
  EXPECT_TRUE(muxTransport_->socketDriver_.streams_[id].peekCB != nullptr);
  transport_->addReadEvent(id, nullptr, true);
  evb_.loopOnce();
  // Now the peek cb is gone
  EXPECT_TRUE(muxTransport_->socketDriver_.streams_[id].peekCB == nullptr);

  // === Grease stream, EOF
  id = muxTransport_->nextUnidirectionalStreamId_;
  muxTransport_->nextUnidirectionalStreamId_ += 4;
  transport_->addReadEvent(
      id, folly::IOBuf::wrapBuffer(twoByteGreaseId.data(), 2), true);
  loopN(2);
  // Peek cb is gone
  EXPECT_TRUE(muxTransport_->socketDriver_.streams_[id].peekCB == nullptr);
  // Stream complete - stopSending called
  EXPECT_EQ(muxTransport_->socketDriver_.streams_[id].error,
            HTTP3::ErrorCode::HTTP_STREAM_CREATION_ERROR);
  // Buf not consumed
  EXPECT_FALSE(muxTransport_->socketDriver_.streams_[id].readBuf.empty());

  // === Grease stream, no EOF
  id = muxTransport_->nextUnidirectionalStreamId_;
  muxTransport_->nextUnidirectionalStreamId_ += 4;
  transport_->addReadEvent(
      id, folly::IOBuf::wrapBuffer(twoByteGreaseId.data(), 2), false);
  loopN(2);
  // Now the peek cb is gone
  EXPECT_TRUE(muxTransport_->socketDriver_.streams_[id].peekCB == nullptr);
  // It told us to stop sending
  EXPECT_EQ(muxTransport_->socketDriver_.streams_[id].error,
            HTTP3::ErrorCode::HTTP_STREAM_CREATION_ERROR);
  // Send some data on the grease stream
  transport_->addReadEvent(id, makeBuf(10));
  evb_.loopOnce();
  // App didn't read the data or consume the preface
  EXPECT_EQ(muxTransport_->socketDriver_.streams_[id].readBuf.chainLength(),
            12);
  // Now close the stream
  transport_->addReadEvent(id, nullptr, true);
  evb_.loopOnce();

  session_->initiateDrain();
  evb_.loop();
}

TEST_P(H2QDownstreamSessionTest, StreamAfterGoaway) {
  // 1st req is accepted
  auto id1 = sendRequest("/");
  auto handler =
      addSimpleStrictHandler([this](folly::EventBase *evb,
                                    HTTPSessionContextPtr /*ctx*/,
                                    HTTPSourceHolder requestSource)
                                 -> folly::coro::Task<HTTPSourceHolder> {
        co_await expectRequest(requestSource, HTTPMethod::GET, "/");
        co_return HTTPFixedSource::makeFixedResponse(200, makeBuf(100));
      });

  loopN(1);

  // Send GOAWAY
  session_->initiateDrain();
  auto id2 = sendRequest("/");
  // no handler is created for id2
  evb_.loop();
  expectResponse(id1, 200, nullptr, true);
  expectGoaway(id1, ErrorCode::NO_ERROR);
  if (isHQ()) {
    expectStreamAbort(id2, ErrorCode::REFUSED_STREAM);
  }
  parseOutput();
}

TEST_P(H2DownstreamSessionTest, BasicRFC9218PriorityUrgency) {
  /* Headers are processed in the order that they're sent (req: u=7, then req2:
   * u=1), but after the stream priorities are set from reading the headers,
   * addStreamBodyDataToWriteBuf() takes the stream with the highest priority
   * and generates the body for that stream first.
   */
  InSequence enforceOrder;
  auto req = getGetRequest("/");
  req.getHeaders().add("priority",
                       "u=7"); // urgency is in descending order of priority, so
                               // this is actually lower in priority than u=1
  auto req2 = getGetRequest("/");
  req2.getHeaders().add("priority", "u=1");

  auto id = sendRequestHeader(req, true, true);
  auto id2 = sendRequestHeader(req2, true, true);

  auto handler =
      addSimpleStrictHandler([&](folly::EventBase *evb,
                                 HTTPSessionContextPtr /*ctx*/,
                                 HTTPSourceHolder requestSource)
                                 -> folly::coro::Task<HTTPSourceHolder> {
        co_await expectRequest(requestSource, HTTPMethod::GET, "/");
        co_return HTTPFixedSource::makeFixedResponse(200, makeBuf(100));
      });

  auto handler2 =
      addSimpleStrictHandler([this](folly::EventBase *evb,
                                    HTTPSessionContextPtr /*ctx*/,
                                    HTTPSourceHolder requestSource)
                                 -> folly::coro::Task<HTTPSourceHolder> {
        co_await expectRequest(requestSource, HTTPMethod::GET, "/");
        co_return HTTPFixedSource::makeFixedResponse(200, makeBuf(100));
      });

  transport_->addReadEvent(writeBuf_.move());
  loopN(2);
  EXPECT_CALL(callbacks_,
              onHeadersComplete(id, _)); // onHeadersComplete() in the
                                         // order the reqs are sent
  EXPECT_CALL(callbacks_, onHeadersComplete(id2, _));
  EXPECT_CALL(callbacks_,
              onBody(id2, _, _)); // id2 gets body written first since it has
                                  // the higher priority
  EXPECT_CALL(callbacks_, onMessageComplete(id2, _));
  EXPECT_CALL(callbacks_, onBody(id, _, _));
  EXPECT_CALL(callbacks_, onMessageComplete(id, _));
  parseOutput();
}

TEST_P(HQDownstreamSessionTest, onPriorityHeaderUrgency) {
  auto req = getGetRequest();
  req.getHeaders().add("priority", "u=1");

  auto id = sendRequestHeader(req, true, true);
  muxTransport_->socketDriver_.expectSetPriority(
      id, quic::HTTPPriorityQueue::Priority(1, false));

  auto handler =
      addSimpleStrictHandler([this](folly::EventBase *evb,
                                    HTTPSessionContextPtr /*ctx*/,
                                    HTTPSourceHolder requestSource)
                                 -> folly::coro::Task<HTTPSourceHolder> {
        co_await expectRequest(requestSource, HTTPMethod::GET, "/");
        generateGoaway(0, ErrorCode::NO_ERROR);
        co_return HTTPFixedSource::makeFixedResponse(200, makeBuf(100));
      });

  transport_->addReadEvent(id, writeBuf_.move(), true);
  evb_.loop();
  expectResponse(id, 200);
  parseOutputHQ();
}

TEST_P(HQDownstreamSessionTest, onPriorityHeaderUrgencyIncremental) {
  auto req = getGetRequest();
  req.getHeaders().add("priority", "u=2, i");

  auto id = sendRequestHeader(req, true, true);
  muxTransport_->socketDriver_.expectSetPriority(
      id, quic::HTTPPriorityQueue::Priority(2, true));

  auto handler =
      addSimpleStrictHandler([this](folly::EventBase *evb,
                                    HTTPSessionContextPtr /*ctx*/,
                                    HTTPSourceHolder requestSource)
                                 -> folly::coro::Task<HTTPSourceHolder> {
        co_await expectRequest(requestSource, HTTPMethod::GET, "/");
        generateGoaway(0, ErrorCode::NO_ERROR);
        co_return HTTPFixedSource::makeFixedResponse(200, makeBuf(100));
      });

  transport_->addReadEvent(id, writeBuf_.move(), true);
  evb_.loop();
  expectResponse(id, 200);
  parseOutputHQ();
}

TEST_P(HQDownstreamSessionTest, onPriorityUpdateFrame) {
  loopN(2);
  auto req = getGetRequest();
  /**
   * pri header should be ignored since we're sending the PriorityUpdate frame
   * first (simulating delay).
   */
  req.getHeaders().add("priority", "u=2");

  auto id = createStreamID();
  muxTransport_->socketDriver_.expectSetPriority(
      id, quic::HTTPPriorityQueue::Priority(3, true));

  /**
   * Generate PriorityUpdate frame, expected to be buffered by session since
   * we're receiving an pri update for a stream id we haven't yet received.
   */
  clientCodec_->generatePriority(writeBuf_, id, HTTPPriority(3, true));
  transport_->addReadEvent(writeBuf_.move());
  loopN(2);

  // Send request with pri header (should be ignored)
  sendRequestHeader(id, req, true, true);
  transport_->addReadEvent(id, writeBuf_.move(), true);

  auto handler =
      addSimpleStrictHandler([this](folly::EventBase *evb,
                                    HTTPSessionContextPtr /*ctx*/,
                                    HTTPSourceHolder requestSource)
                                 -> folly::coro::Task<HTTPSourceHolder> {
        co_await expectRequest(requestSource, HTTPMethod::GET, "/");
        co_return HTTPFixedSource::makeFixedResponse(200, makeBuf(100));
      });
  loopN(1);

  session_->initiateDrain();
  evb_.loop();
  expectResponse(id, 200);
  parseOutputHQ();
}

TEST_P(HQDownstreamSessionTest, onServerPriorityHeader) {
  auto req = getGetRequest();

  auto id = sendRequestHeader(req, true, true);
  muxTransport_->socketDriver_.expectSetPriority(
      id, quic::HTTPPriorityQueue::Priority(1, false));

  auto handler =
      addSimpleStrictHandler([this](folly::EventBase *evb,
                                    HTTPSessionContextPtr /*ctx*/,
                                    HTTPSourceHolder requestSource)
                                 -> folly::coro::Task<HTTPSourceHolder> {
        co_await expectRequest(requestSource, HTTPMethod::GET, "/");
        generateGoaway(0, ErrorCode::NO_ERROR);
        auto resp = HTTPFixedSource::makeFixedResponse(200, makeBuf(100));
        resp->msg_->getHeaders().add("priority", "u=1");
        co_return resp;
      });

  transport_->addReadEvent(id, writeBuf_.move(), true);
  evb_.loop();
  expectResponse(id, 200);
  parseOutputHQ();
}

TEST_P(HQDownstreamSessionTest, DelayedQPACK) {
  auto req = getGetRequest();
  req.getHeaders().add("X-FB-Debug", "rfccffgvtvnenjkbtitkfdufddnvbecu");
  auto id = sendRequestHeader(req, /*eom=*/true, /*flushQPACK=*/false);
  auto handler =
      addSimpleStrictHandler([this](folly::EventBase *evb,
                                    HTTPSessionContextPtr /*ctx*/,
                                    HTTPSourceHolder requestSource)
                                 -> folly::coro::Task<HTTPSourceHolder> {
        co_await expectRequest(requestSource, HTTPMethod::GET, "/");
        generateGoaway(0, ErrorCode::NO_ERROR);
        co_return HTTPFixedSource::makeFixedResponse(200, makeBuf(100));
      });

  transport_->addReadEvent(id, writeBuf_.move(), true);
  EXPECT_GT(multiCodec_->getQPACKEncoderWriteBuf().chainLength(), 0);
  loopN(2);
  flushQPACKEncoder();
  evb_.loop();
  expectResponse(id, 200);
  parseOutput();
}

TEST_P(HQDownstreamSessionTest, cancelQPACK) {
  auto req = getGetRequest();
  req.getHeaders().add("X-FB-Debug", "rfccffgvtvnenjkbtitkfdufddnvbecu");
  auto id = sendRequestHeader(req);
  // discard part of request, header won't get qpack-ack'd
  writeBuf_.trimEnd(writeBuf_.chainLength() - 3);
  transport_->addReadEvent(id, writeBuf_.move(), false);
  auto handler =
      addSimpleStrictHandler([](folly::EventBase *evb,
                                HTTPSessionContextPtr /*ctx*/,
                                HTTPSourceHolder requestSource)
                                 -> folly::coro::Task<HTTPSourceHolder> {
        co_await expectHeaderError(requestSource,
                                   HTTPErrorCode::REQUEST_CANCELLED);
        co_return nullptr;
      });
  loopN(2);
  resetStream(id, ErrorCode::CANCEL, /*stopSending=*/false);
  session_->initiateDrain();
  evb_.loop();
  // this will evict all headers, which is only legal if the cancellation is
  // emitted and processed.
  parseOutput();
  multiCodec_->getQPACKCodec().setEncoderHeaderTableSize(0,
                                                         /*updateMax=*/false);
  EXPECT_EQ(*muxTransport_->socketDriver_.streams_[id].error,
            HTTP3::ErrorCode::HTTP_REQUEST_CANCELLED);
}

TEST_P(HQDownstreamSessionTest, DelayedQPACKCanceled) {
  auto req = getGetRequest();
  req.getHeaders().add("X-FB-Debug", "rfccffgvtvnenjkbtitkfdufddnvbecu");
  auto id = sendRequestHeader(req, /*eom=*/true, /*flushQPACK=*/false);
  transport_->addReadEvent(id, writeBuf_.move(), false);
  auto handler =
      addSimpleStrictHandler([](folly::EventBase *evb,
                                HTTPSessionContextPtr /*ctx*/,
                                HTTPSourceHolder requestSource)
                                 -> folly::coro::Task<HTTPSourceHolder> {
        // Maybe REQUEST_CANCELLED someday
        co_await expectHeaderError(requestSource,
                                   HTTPErrorCode::CORO_CANCELLED);
        co_return nullptr;
      });

  // receive header block with unsatisfied dep
  loopN(1);

  // cancel this request
  muxTransport_->socketDriver_.addStopSending(
      id, HTTP3::ErrorCode::HTTP_REQUEST_CANCELLED);
  loopN(1);

  // Now send the dependency
  session_->initiateDrain();
  flushQPACKEncoder();
  evb_.loop();
}

TEST_P(HQDownstreamSessionTest, DelayedQPACKTimeout) {
  auto req = getPostRequest(10);
  req.getHeaders().add("X-FB-Debug", "rfccffgvtvnenjkbtitkfdufddnvbecu");
  auto id = sendRequestHeader(req, /*eom=*/true, /*flushQPACK=*/false);
  folly::IOBufQueue reqTail(folly::IOBufQueue::cacheChainLength());
  reqTail.append(writeBuf_.move());
  writeBuf_.append(reqTail.split(reqTail.chainLength() / 2));
  // reqTail now has the second half of request
  transport_->addReadEvent(id, writeBuf_.move(), false);
  writeBuf_.append(reqTail.move());

  auto handler =
      addSimpleStrictHandler([this, id](folly::EventBase *evb,
                                        HTTPSessionContextPtr /*ctx*/,
                                        HTTPSourceHolder requestSource)
                                 -> folly::coro::Task<HTTPSourceHolder> {
        co_await expectHeaderError(requestSource, HTTPErrorCode::READ_TIMEOUT);
        // Also flushes reqTail from writeBuf_
        sendBody(id, makeBuf(10), true, true);
        session_->initiateDrain();
        co_return nullptr;
      });

  evb_.loop();
  expectResponse(id, 408, /*headers=*/nullptr, /*expectBody=*/false);
  parseOutput();
}

TEST_P(HQDownstreamSessionTest, DelayedQPACKTimeoutLoopOnceUAF) {
  auto req = getPostRequest(10);
  req.getHeaders().add("X-FB-Debug", "rfccffgvtvnenjkbtitkfdufddnvbecu");
  auto id = sendRequestHeader(req, /*eom=*/true, /*flushQPACK=*/false);
  folly::IOBufQueue reqTail(folly::IOBufQueue::cacheChainLength());
  reqTail.append(writeBuf_.move());
  writeBuf_.append(reqTail.split(reqTail.chainLength() / 2));
  // reqTail now has the second half of request
  transport_->addReadEvent(id, writeBuf_.move(), false);
  writeBuf_.append(reqTail.move());

  auto handler =
      addSimpleStrictHandler([this, id](folly::EventBase *evb,
                                        HTTPSessionContextPtr /*ctx*/,
                                        HTTPSourceHolder requestSource)
                                 -> folly::coro::Task<HTTPSourceHolder> {
        co_await expectHeaderError(requestSource, HTTPErrorCode::READ_TIMEOUT);
        // Also flushes reqTail from writeBuf_
        sendBody(id, makeBuf(10), true, true);
        co_await folly::coro::co_reschedule_on_current_executor;
        session_->initiateDrain();
        co_return nullptr;
      });

  evb_.loop();
  expectResponse(id, 408, /*headers=*/nullptr, /*expectBody=*/false);
  parseOutput();
}

TEST_P(HQDownstreamSessionTest, DelayedQPACKICI) {
  loopN(3);
  auto req = getPostRequest(10);
  req.getHeaders().add("X-FB-Debug", "rfccffgvtvnenjkbtitkfdufddnvbecu");
  sendRequestHeader(req);
  // Don't add the actual request, just the QPACK data
  loopN(3);
  // Should get an ICI
  EXPECT_GE(muxTransport_->socketDriver_.streams_[kQPACKDecoderEgressStreamId]
                .writeBuf.chainLength(),
            2);
  session_->initiateDrain();
  evb_.loop();
  parseOutput();
}

TEST_P(HQDownstreamSessionTest, QPACKEncoderLimited) {
  muxTransport_->socketDriver_.getSocket()->setStreamFlowControlWindow(
      kQPACKEncoderEgressStreamId, 10);
  EXPECT_CALL(lifecycleObs_, onDeactivateConnection(_))
      // Free the encoder flow control after the request completes
      .WillOnce(Invoke([this](const HTTPCoroSession &) {
        muxTransport_->socketDriver_.setStreamFlowControlWindow(
            kQPACKEncoderEgressStreamId, 100);
      }));
  auto req = getGetRequest();
  auto id = sendRequest(req);
  auto handler =
      addSimpleStrictHandler([this](folly::EventBase *evb,
                                    HTTPSessionContextPtr /*ctx*/,
                                    HTTPSourceHolder requestSource)
                                 -> folly::coro::Task<HTTPSourceHolder> {
        co_await expectRequest(requestSource, HTTPMethod::GET, "/");
        auto resp = HTTPFixedSource::makeFixedResponse(200, makeBuf(100));
        resp->msg_->getHeaders().add("X-FB-Debug",
                                     "rfccffgvtvnenjkbtitkfdufddnvbecu");
        session_->initiateDrain();
        co_return resp;
      });
  evb_.loop();

  // QPACK will attempt to index the header, but cannot reference it because
  // it runs out of stream flow control
  EXPECT_GT(muxTransport_->socketDriver_.streams_[id].writeBuf.chainLength(),
            30);

  expectResponse(id, 200);
  parseOutput();
  // The insert happens after f/c is released
  EXPECT_EQ(multiCodec_->getQPACKCodec().getCompressionInfo().ingress.inserts_,
            1);
}

TEST_P(HQDownstreamSessionTest, DelayedQPACKStopSendingReset) {
  auto req = getGetRequest();
  req.getHeaders().add("X-FB-Debug", "rfccffgvtvnenjkbtitkfdufddnvbecu");
  auto id = sendRequestHeader(req, /*eom=*/true, /*flushQPACK=*/false);
  transport_->addReadEvent(id, writeBuf_.move(), false);
  auto handler =
      addSimpleStrictHandler([this](folly::EventBase *evb,
                                    HTTPSessionContextPtr /*ctx*/,
                                    HTTPSourceHolder requestSource)
                                 -> folly::coro::Task<HTTPSourceHolder> {
        session_->initiateDrain();
        co_await expectHeaderError(requestSource,
                                   HTTPErrorCode::REQUEST_CANCELLED);
        co_return nullptr;
      });
  // receive header block with unsatisfied dep
  loopN(2);

  // cancel this request
  muxTransport_->socketDriver_.addStopSending(
      id, HTTP3::ErrorCode::HTTP_REQUEST_CANCELLED);
  muxTransport_->socketDriver_.addReadError(
      id,
      HTTP3::ErrorCode::HTTP_REQUEST_CANCELLED,
      std::chrono::milliseconds(0));
  loopN(2);

  // Now send the dependency
  flushQPACKEncoder();
  evb_.loop();
}

TEST_P(HQDownstreamSessionTest, DatagramNotSupportedTransport) {
  // destroy previous session
  session_->closeWhenIdle();
  evb_.loop();

  // create new downstream session to rx settings without transport support
  TestCoroMultiplexTransport muxTransport(&evb_, direction_);
  transport_ = &muxTransport;
  auto codec = std::make_unique<hq::HQMultiCodec>(direction_);
  wangle::TransportInfo tinfo;
  auto session = HTTPCoroSession::makeDownstreamCoroSession(
      muxTransport.getSocket(), handler_, std::move(codec), std::move(tinfo));
  folly::coro::co_withCancellation(cancellationSource_.getToken(),
                                   session->run())
      .start();
  loopN(4);

  // simulate lack of support for datagrams at transport level
  EXPECT_CALL(*muxTransport.getSocket(), getDatagramSizeLimit())
      .WillRepeatedly(Return(0));

  // rx datagram settings
  auto peerCodec = std::make_unique<hq::HQMultiCodec>(oppositeDirection());
  setTestCodecSetting(
      peerCodec->getEgressSettings(), SettingsId::_HQ_DATAGRAM, 1);
  peerCodec->generateSettings(writeBuf_);
  muxTransport.addReadEvent(writeBuf_.move(), false);
  loopN(4);

  // expect conn err
  EXPECT_EQ(muxTransport.socketDriver_.getConnErrorCode(),
            static_cast<quic::ApplicationErrorCode>(
                HTTP3::ErrorCode::HTTP_SETTINGS_ERROR));
}

TEST_P(HQDownstreamSessionDatagramTest, RxDatagramInvalidStreamId) {
  auto &socketDriver = muxTransport_->socketDriver_;
  EXPECT_GT(session_->getDatagramSizeLimit(), 0);
  // add datagram callback as settings are modified after ::start()
  socketDriver.datagramCB_ = static_cast<HTTPQuicCoroSession *>(session_);

  // initiate client bidi request
  auto id = sendRequestHeader(
      getPostRequest(100), /*eom=*/false, /*flushQPACK=*/false);
  transport_->addReadEvent(id, writeBuf_.move(), true);

  // waits for header event (blocked on QPACK)
  auto handler =
      addSimpleStrictHandler([](folly::EventBase *evb,
                                HTTPSessionContextPtr /*ctx*/,
                                HTTPSourceHolder requestSource)
                                 -> folly::coro::Task<HTTPSourceHolder> {
        co_await expectHeaderError(requestSource);
        co_return nullptr;
      });
  loopN(3);

  // empty datagram fails to parse stream id and result in conn error
  socketDriver.addDatagram(folly::IOBuf::copyBuffer(""));
  socketDriver.addDatagramsAvailableReadEvent();
  evb_.loop();

  // expect conn err
  EXPECT_EQ(socketDriver.getConnErrorCode(),
            static_cast<quic::ApplicationErrorCode>(
                HTTPErrorCode::GENERAL_PROTOCOL_ERROR));
  expectedError_ = folly::coro::TransportIf::ErrorCode::NETWORK_ERROR;
}

TEST_P(HQDownstreamSessionDatagramTest, RxDatagramStreamIdLimitExceeded) {
  auto &socketDriver = muxTransport_->socketDriver_;
  EXPECT_GT(session_->getDatagramSizeLimit(), 0);
  // add datagram callback as settings are modified after ::start()
  socketDriver.datagramCB_ = static_cast<HTTPQuicCoroSession *>(session_);

  // initiate client bidi request
  auto id = sendRequestHeader(
      getPostRequest(100), /*eom=*/false, /*flushQPACK=*/false);
  transport_->addReadEvent(id, writeBuf_.move(), true);

  // waits for header event (blocked on QPACK)
  auto handler =
      addSimpleStrictHandler([](folly::EventBase *evb,
                                HTTPSessionContextPtr /*ctx*/,
                                HTTPSourceHolder requestSource)
                                 -> folly::coro::Task<HTTPSourceHolder> {
        co_await expectHeaderError(requestSource);
        co_return nullptr;
      });
  loopN(3);

  // stringify std::numeric_limits<uint64_t>::max()
  std::string strStreamId(8, '\xff');
  socketDriver.addDatagram(folly::IOBuf::copyBuffer(strStreamId));
  socketDriver.addDatagramsAvailableReadEvent();
  evb_.loop();

  // expect conn err
  EXPECT_EQ(socketDriver.getConnErrorCode(),
            static_cast<quic::ApplicationErrorCode>(
                HTTPErrorCode::GENERAL_PROTOCOL_ERROR));
  expectedError_ = folly::coro::TransportIf::ErrorCode::NETWORK_ERROR;
}

TEST_P(HQDownstreamSessionDatagramTest, RxDatagramsPriorToStream) {
  auto &socketDriver = muxTransport_->socketDriver_;
  EXPECT_GT(session_->getDatagramSizeLimit(), 0);
  // add datagram callback as settings are modified after ::start()
  socketDriver.datagramCB_ = static_cast<HTTPQuicCoroSession *>(session_);

  auto streamID = createStreamID();
  auto flowID = std::byte(streamID >> 2);

  // "hello" datagram for first bidi stream
  std::vector<char> helloMsg = {(char)(flowID), 'h', 'e', 'l', 'l', 'o'};
  auto helloBuf = folly::IOBuf::copyBuffer(helloMsg);

  // send 3x "hello" datagrams
  const int numDatagramsToSend = 3;
  for (int idx = 0; idx < numDatagramsToSend; idx++) {
    socketDriver.addDatagram(helloBuf->clone());
  }
  socketDriver.addDatagramsAvailableReadEvent();
  // loop so session can buffer datagrams
  loopN(2);

  // send request with eom
  sendRequestHeader(streamID, getGetRequest("/"), /*eom=*/true);
  transport_->addReadEvent(streamID, writeBuf_.move(), /*eom=*/true);

  // verify datagrams were sent to source
  auto handler =
      addSimpleStrictHandler([this](folly::EventBase *evb,
                                    HTTPSessionContextPtr /*ctx*/,
                                    HTTPSourceHolder requestSource)
                                 -> folly::coro::Task<HTTPSourceHolder> {
        co_await expectRequest(
            requestSource, HTTPMethod::GET, "/", /*eom=*/false);

        // expect three datagrams and then an empty body with eom
        for (int idx = 0; idx < numDatagramsToSend; idx++) {
          auto bodyEvent = co_await readBodyEventNoSuspend(requestSource);
          EXPECT_EQ(bodyEvent.eventType, HTTPBodyEvent::EventType::DATAGRAM);
          EXPECT_EQ(bodyEvent.event.datagram->to<std::string>(), "hello");
        }
        auto eomBodyEvent = co_await readBodyEventNoSuspend(requestSource);
        EXPECT_EQ(eomBodyEvent.eventType, HTTPBodyEvent::EventType::BODY);
        EXPECT_EQ(eomBodyEvent.event.body.chainLength(), 0);

        co_return HTTPFixedSource::makeFixedResponse(200, makeBuf(40));
      });

  // expect 200 response
  evb_.loop();
  expectResponse(streamID, 200);
  parseOutputHQ();
}

TEST_P(HQDownstreamSessionDatagramTest, RxStreamPriorToDatagrams) {
  auto &socketDriver = muxTransport_->socketDriver_;
  EXPECT_GT(session_->getDatagramSizeLimit(), 0);
  // add datagram callback as settings are modified after ::start()
  socketDriver.datagramCB_ = static_cast<HTTPQuicCoroSession *>(session_);

  const int numDatagramsToSend = 3;

  // handler verifies datagrams were sent to source
  auto handler =
      addSimpleStrictHandler([this](folly::EventBase *evb,
                                    HTTPSessionContextPtr /*ctx*/,
                                    HTTPSourceHolder requestSource)
                                 -> folly::coro::Task<HTTPSourceHolder> {
        co_await expectRequest(
            requestSource, HTTPMethod::GET, "/", /*eom=*/false);

        // expect three datagrams and then an empty body with eom
        for (int idx = 0; idx < numDatagramsToSend; idx++) {
          auto bodyEvent = co_await readBodyEventNoSuspend(requestSource);
          EXPECT_EQ(bodyEvent.eventType, HTTPBodyEvent::EventType::DATAGRAM);
          EXPECT_EQ(bodyEvent.event.datagram->to<std::string>(), "hello");
        }
        auto eomBodyEvent = co_await readBodyEventNoSuspend(requestSource);
        EXPECT_EQ(eomBodyEvent.eventType, HTTPBodyEvent::EventType::BODY);
        EXPECT_EQ(eomBodyEvent.event.body.chainLength(), 0);

        co_return HTTPFixedSource::makeFixedResponse(200, makeBuf(40));
      });

  // send request without eom
  auto streamID = sendRequest("/", nullptr, /*eom=*/false);
  auto flowID = std::byte(streamID >> 2);
  loopN(2);

  // "hello" datagram for first client bidi stream id = 0
  std::vector<char> helloMsg = {(char)flowID, 'h', 'e', 'l', 'l', 'o'};
  auto helloBuf = folly::IOBuf::copyBuffer(helloMsg);

  // send 3x "hello" datagrams
  for (int idx = 0; idx < numDatagramsToSend; idx++) {
    socketDriver.addDatagram(helloBuf->clone());
  }
  // loop so session can deliver datagrams to source
  socketDriver.addDatagramsAvailableReadEvent();
  loopN(2);

  // serialize eom
  transport_->addReadEvent(streamID, nullptr, true);

  // expect 200 response
  evb_.loop();
  expectResponse(streamID, 200);
  parseOutputHQ();
}

INSTANTIATE_TEST_SUITE_P(
    HTTPDownstreamSessionTest,
    HTTPDownstreamSessionTest,
    Values(TestParams({.codecProtocol = CodecProtocol::HTTP_1_1}),
           TestParams({.codecProtocol = CodecProtocol::HTTP_2}),
           TestParams({.codecProtocol = CodecProtocol::HQ})),
    paramsToTestName);

INSTANTIATE_TEST_SUITE_P(
    HTTPDownstreamSessionTest,
    H1DownstreamSessionTest,
    Values(TestParams({.codecProtocol = CodecProtocol::HTTP_1_1})),
    paramsToTestName);

INSTANTIATE_TEST_SUITE_P(
    HTTPDownstreamSessionTest,
    H2DownstreamSessionTest,
    Values(TestParams({.codecProtocol = CodecProtocol::HTTP_2})),
    paramsToTestName);

INSTANTIATE_TEST_SUITE_P(
    HTTPDownstreamSessionTest,
    HQDownstreamSessionTest,
    Values(TestParams({.codecProtocol = CodecProtocol::HQ})),
    paramsToTestName);

INSTANTIATE_TEST_SUITE_P(HTTPDownstreamSessionTest,
                         HQDownstreamSessionDatagramTest,
                         Values(TestParams({.codecProtocol = CodecProtocol::HQ,
                                            .enableDatagrams = true})),
                         paramsToTestName);

INSTANTIATE_TEST_SUITE_P(HTTPDownstreamSessionTest,
                         HQStaticQPACKDownstreamSessionTest,
                         Values(TestParams({.codecProtocol = CodecProtocol::HQ,
                                            .useDynamicTable = false})),
                         paramsToTestName);

INSTANTIATE_TEST_SUITE_P(
    HTTPDownstreamSessionTest,
    H12DownstreamSessionTest,
    Values(TestParams({.codecProtocol = CodecProtocol::HTTP_1_1}),
           TestParams({.codecProtocol = CodecProtocol::HTTP_2})),
    paramsToTestName);

INSTANTIATE_TEST_SUITE_P(
    HTTPDownstreamSessionTest,
    H2QDownstreamSessionTest,
    Values(TestParams({.codecProtocol = CodecProtocol::HTTP_2}),
           TestParams({.codecProtocol = CodecProtocol::HQ})),
    paramsToTestName);

} // namespace proxygen::coro::test
