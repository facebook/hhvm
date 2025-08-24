/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <proxygen/lib/http/codec/test/TestUtils.h>
#include <proxygen/lib/http/coro/HTTPFixedSource.h>
#include <proxygen/lib/http/coro/HTTPSourceFilter.h>
#include <proxygen/lib/http/coro/filters/DecompressionFilter.h>
#include <proxygen/lib/http/coro/filters/DecompressionFilterFactory.h>
#include <proxygen/lib/http/coro/test/HTTPTestSources.h>
#include <proxygen/lib/http/coro/util/test/TestHelpers.h>

using namespace testing;

namespace {

using namespace proxygen::coro;
using folly::coro::co_error;

HTTPSource* getEgressDecompressionFilter(HTTPSource* source) {
  auto decompressionFilters = ClientDecompressionFilterFactory{}.makeFilters();
  // unused, avoid memory leak
  delete decompressionFilters.second;
  decompressionFilters.first->setSource(source);
  return decompressionFilters.first;
}

HTTPSource* getIngressDecompressionFilter(HTTPSource* source) {
  auto decompressionFilters = ClientDecompressionFilterFactory{}.makeFilters();
  // unused, avoid memory leak
  delete decompressionFilters.first;
  decompressionFilters.second->setSource(source);
  return decompressionFilters.second;
}

}; // namespace

namespace proxygen::coro::test {

class DecompressionEgressFilterTest : public testing::Test {
 public:
  folly::DrivableExecutor* getExecutor() {
    return &evb_;
  }

 protected:
  folly::EventBase evb_;
};

CO_TEST_F_X(DecompressionEgressFilterTest, TestExceptionPassthru) {
  // test yielding exception on header event
  {
    YieldExceptionSource headerEventExceptionSource{
        YieldExceptionSource::Stage::HeaderEvent,
        YieldExceptionSource::MessageType::Request};

    // wrap reqSource with an egress decompression filter
    auto* egressFilter =
        getEgressDecompressionFilter(&headerEventExceptionSource);

    auto headerEvent = co_await co_awaitTry(egressFilter->readHeaderEvent());
    XCHECK(headerEvent.hasException());
  }

  // test yielding exception on body event
  {
    YieldExceptionSource bodyEventExceptionSource =
        YieldExceptionSource{YieldExceptionSource::Stage::BodyEvent,
                             YieldExceptionSource::MessageType::Request};
    auto* egressFilter =
        getEgressDecompressionFilter(&bodyEventExceptionSource);

    auto headerEvent = co_await co_awaitTry(egressFilter->readHeaderEvent());
    XCHECK(!headerEvent.hasException());
    auto bodyEvent = co_await co_awaitTry(egressFilter->readBodyEvent());
    XCHECK(bodyEvent.hasException());
  }
}

CO_TEST_F_X(DecompressionEgressFilterTest, SkipExistingAcceptEncoding) {
  // create req HTTPMessage with existing "accept-encoding: foo" header
  auto getReq = makeGetRequest();
  getReq->getHeaders().set(HTTP_HEADER_ACCEPT_ENCODING, "foo");
  auto* reqSource =
      HTTPFixedSource::makeFixedSource(std::move(getReq), nullptr);

  // wrap reqSource with an egress decompression filter
  auto* egressFilter = getEgressDecompressionFilter(reqSource);

  // check filter doesn't modify header
  auto headerEvent = co_await co_awaitTry(egressFilter->readHeaderEvent());
  XCHECK(!headerEvent.hasException() && headerEvent->eom);
  auto acceptEncoding = headerEvent->headers->getHeaders().getSingleOrEmpty(
      HTTP_HEADER_ACCEPT_ENCODING);
  EXPECT_EQ(acceptEncoding, "foo");
}

CO_TEST_F_X(DecompressionEgressFilterTest, AddAcceptEncodingIfMissing) {
  // create req HTTPMessage without "accept-encoding" header
  auto reqSource = HTTPFixedSource::makeFixedSource(makeGetRequest(), nullptr);

  // wrap reqSource with an egress decompression filter
  auto* egressFilter = getEgressDecompressionFilter(reqSource);

  // check filter adds headers
  auto headerEvent = co_await co_awaitTry(egressFilter->readHeaderEvent());
  XCHECK(!headerEvent.hasException() && headerEvent->eom);
  auto acceptEncoding = headerEvent->headers->getHeaders().getSingleOrEmpty(
      HTTP_HEADER_ACCEPT_ENCODING);
  EXPECT_EQ(acceptEncoding, "gzip, deflate, zstd");
}

class DecompressionIngressFilterTest : public testing::Test {
 public:
  folly::DrivableExecutor* getExecutor() {
    return &evb_;
  }

 protected:
  folly::EventBase evb_;
};

CO_TEST_F_X(DecompressionIngressFilterTest, TestExceptionPassthru) {
  // test yielding exception on header event
  {
    YieldExceptionSource headerEventExceptionSource{
        YieldExceptionSource::Stage::HeaderEvent,
        YieldExceptionSource::MessageType::Response};

    // wrap reqSource with an egress decompression filter
    auto* ingressFilter =
        getIngressDecompressionFilter(&headerEventExceptionSource);

    auto headerEvent = co_await co_awaitTry(ingressFilter->readHeaderEvent());
    XCHECK(headerEvent.hasException());
  }

  // test yielding exception on body event
  {
    YieldExceptionSource bodyEventExceptionSource =
        YieldExceptionSource{YieldExceptionSource::Stage::BodyEvent,
                             YieldExceptionSource::MessageType::Response};

    auto* ingressFilter =
        getIngressDecompressionFilter(&bodyEventExceptionSource);

    auto headerEvent = co_await co_awaitTry(ingressFilter->readHeaderEvent());
    XCHECK(!headerEvent.hasException());
    auto bodyEvent = co_await co_awaitTry(ingressFilter->readBodyEvent());
    XCHECK(bodyEvent.hasException());
  }
}

struct DecompressionIngressFilterCompressionParam {
  DecompressionIngressFilterCompressionParam(std::string_view encoding,
                                             std::string_view body)
      : encoding{encoding}, body{folly::unhexlify(body)} {
  }

  const std::string encoding;
  const std::string body;
};

class DecompressionIngressFilterCompressionTest
    : public DecompressionIngressFilterTest
    , public testing::WithParamInterface<
          DecompressionIngressFilterCompressionParam> {};

CO_TEST_P_X(DecompressionIngressFilterCompressionTest,
            TestClientDecompression) {
  HTTPFixedSource respSource(
      makeResponse(200),
      /*body=*/folly::IOBuf::wrapBuffer(folly::ByteRange{GetParam().body}));
  respSource.msg_->getHeaders().set(HTTP_HEADER_CONTENT_ENCODING,
                                    GetParam().encoding);

  // should we modify the headers if just eom (i.e. no body)?
  auto* ingressFilter = getIngressDecompressionFilter(&respSource);
  auto headerEvent = co_await co_awaitTry(ingressFilter->readHeaderEvent());
  XCHECK(!headerEvent.hasException());
  const auto& respHeaders = headerEvent->headers->getHeaders();
  // should have been stripped by the filter
  EXPECT_FALSE(respHeaders.exists(HTTP_HEADER_CONTENT_ENCODING));
  EXPECT_FALSE(respHeaders.exists(HTTP_HEADER_CONTENT_LENGTH));
  const auto& transferEncoding =
      respHeaders.getSingleOrEmpty(HTTP_HEADER_TRANSFER_ENCODING);
  EXPECT_EQ(transferEncoding, "chunked");

  auto bodyEvent = co_await co_awaitTry(ingressFilter->readBodyEvent());
  XCHECK(!bodyEvent.hasException());
  XCHECK(bodyEvent->eventType == HTTPBodyEvent::EventType::BODY &&
         bodyEvent->eom);
  EXPECT_EQ(bodyEvent->event.body.move()->toString(),
            "abcdefghijklmnopqrstuvwxyz");
}

CO_TEST_P_X(DecompressionIngressFilterCompressionTest,
            TestServerDecompression) {
  HTTPFixedSource reqSource(
      makePostRequest(GetParam().body.size()),
      /*body=*/folly::IOBuf::wrapBuffer(folly::ByteRange{GetParam().body}));
  reqSource.msg_->getHeaders().set(HTTP_HEADER_CONTENT_ENCODING,
                                   GetParam().encoding);

  auto&& [requestFilter, responseFilter] =
      ServerDecompressionFilterFactory{}.makeFilters();
  XCHECK(responseFilter == nullptr);
  XCHECK(requestFilter != nullptr);
  requestFilter->setSource(&reqSource);

  const auto headerEvent =
      co_await co_awaitTry(requestFilter->readHeaderEvent());
  XCHECK(headerEvent.hasValue());
  const auto& respHeaders = headerEvent->headers->getHeaders();
  // should have been stripped by the filter
  EXPECT_FALSE(respHeaders.exists(HTTP_HEADER_CONTENT_ENCODING));
  EXPECT_FALSE(respHeaders.exists(HTTP_HEADER_CONTENT_LENGTH));
  const auto& transferEncoding =
      respHeaders.getSingleOrEmpty(HTTP_HEADER_TRANSFER_ENCODING);
  EXPECT_EQ(transferEncoding, "chunked");

  auto bodyEvent = co_await co_awaitTry(requestFilter->readBodyEvent());
  XCHECK(bodyEvent.hasValue() &&
         bodyEvent->eventType == HTTPBodyEvent::EventType::BODY &&
         bodyEvent->eom);
  EXPECT_EQ(bodyEvent->event.body.move()->toString(),
            "abcdefghijklmnopqrstuvwxyz");
}

INSTANTIATE_TEST_SUITE_P(
    DecompressionIngressFilterCompressionTests,
    DecompressionIngressFilterCompressionTest,
    testing::Values(DecompressionIngressFilterCompressionParam(
                        "gzip",
                        // gzip of "abcdefghijklmnopqrstuvwxyz"
                        "1f8b080092a3326600ff011a00e5ff6162636465666768696a6b6c"
                        "6d6e6f7071727374"
                        "75767778797abd50274c1a000000"),
                    DecompressionIngressFilterCompressionParam(
                        "deflate",
                        // deflate of "abcdefghijklmnopqrstuvwxyz"
                        "7801011a00e5ff6162636465666768696a6b6c6d6e6f7071727374"
                        "75767778797a90860b20"),
                    DecompressionIngressFilterCompressionParam(
                        "zstd",
                        // zstd of "abcdefghijklmnopqrstuvwxyz"
                        "28b52ffd0458d100006162636465666768696a6b6c6d6e6f707172"
                        "737475767778797a5c8389fa")),
    [](const auto& info) { return info.param.encoding; });

CO_TEST_F_X(DecompressionIngressFilterTest, TestUnsupportedCompression) {
  HTTPFixedSource respSource(
      makeResponse(200),
      /*body=*/folly::IOBuf::copyBuffer("abcdefghijklmnopqrstuvwxyz"));
  respSource.msg_->getHeaders().set(HTTP_HEADER_CONTENT_ENCODING,
                                    "foo-bar-baz");

  auto* ingressFilter = getIngressDecompressionFilter(&respSource);
  auto headerEvent = co_await co_awaitTry(ingressFilter->readHeaderEvent());
  XCHECK(!headerEvent.hasException());
  const auto& respHeaders = headerEvent->headers->getHeaders();
  // should NOT have been stripped by the filter
  const auto& contentEncoding =
      respHeaders.getSingleOrEmpty(HTTP_HEADER_CONTENT_ENCODING);
  EXPECT_EQ(contentEncoding, "foo-bar-baz");
  EXPECT_TRUE(respHeaders.exists(HTTP_HEADER_CONTENT_LENGTH));

  auto bodyEvent = co_await co_awaitTry(ingressFilter->readBodyEvent());
  XCHECK(!bodyEvent.hasException());
  XCHECK(bodyEvent->eventType == HTTPBodyEvent::EventType::BODY &&
         bodyEvent->eom);
  // should be identical to original body
  EXPECT_EQ(bodyEvent->event.body.move()->to<std::string>(),
            "abcdefghijklmnopqrstuvwxyz");
}

CO_TEST_F_X(DecompressionIngressFilterTest, TestMalformedCompression) {
  // gzip of "abcdefghijklmnopqrstuvwxyz" prefixed with "faceb00c"
  auto gzip = folly::unhexlify(
      "faceb00c"
      "1f8b080092a3326600ff011a00e5ff6162636465666768696a6b6c6d6e6f707172737475"
      "767778797abd50274c1a000000");

  HTTPFixedSource respSource(makeResponse(200),
                             /*body=*/folly::IOBuf::copyBuffer(gzip));
  respSource.msg_->getHeaders().set(HTTP_HEADER_CONTENT_ENCODING, "gzip");

  auto* ingressFilter = getIngressDecompressionFilter(&respSource);
  auto headerEvent = co_await co_awaitTry(ingressFilter->readHeaderEvent());
  XCHECK(!headerEvent.hasException());
  const auto& respHeaders = headerEvent->headers->getHeaders();
  // should have been stripped by the filter
  EXPECT_FALSE(respHeaders.exists(HTTP_HEADER_CONTENT_ENCODING));
  EXPECT_FALSE(respHeaders.exists(HTTP_HEADER_CONTENT_LENGTH));
  const auto& transferEncoding =
      respHeaders.getSingleOrEmpty(HTTP_HEADER_TRANSFER_ENCODING);
  EXPECT_EQ(transferEncoding, "chunked");

  auto bodyEvent = co_await co_awaitTry(ingressFilter->readBodyEvent());
  XCHECK(bodyEvent.hasException());
}

class MockStatsCallback : public DecompressionIngressFilter::StatsCallback {
 public:
  MOCK_METHOD(void, onDecompressionAlgo, (const std::string& algo), (override));
  MOCK_METHOD(void, onDecompressionError, (), (override));
};

CO_TEST_P_X(DecompressionIngressFilterCompressionTest,
            TestStatsCallback_Normal) {
  HTTPFixedSource respSource(
      makeResponse(200),
      /*body=*/folly::IOBuf::wrapBuffer(folly::ByteRange{GetParam().body}));
  respSource.msg_->getHeaders().set(HTTP_HEADER_CONTENT_ENCODING,
                                    GetParam().encoding);

  auto cb = std::make_shared<MockStatsCallback>();
  auto* ingressFilter = new DecompressionIngressFilter(&respSource, cb);
  ingressFilter->setHeapAllocated();

  EXPECT_CALL(*cb, onDecompressionAlgo(GetParam().encoding)).Times(1);
  EXPECT_CALL(*cb, onDecompressionError()).Times(0);

  co_await co_awaitTry(ingressFilter->readHeaderEvent());
  co_await co_awaitTry(ingressFilter->readBodyEvent());
}

CO_TEST_F_X(DecompressionIngressFilterTest, TestStatsCallback_Unsupported) {
  HTTPFixedSource respSource(
      makeResponse(200),
      /*body=*/folly::IOBuf::copyBuffer("abcdefghijklmnopqrstuvwxyz"));
  respSource.msg_->getHeaders().set(HTTP_HEADER_CONTENT_ENCODING,
                                    "foo-bar-baz");

  auto cb = std::make_shared<MockStatsCallback>();
  auto* ingressFilter = new DecompressionIngressFilter(&respSource, cb);
  ingressFilter->setHeapAllocated();

  EXPECT_CALL(*cb, onDecompressionAlgo("")).Times(1);
  EXPECT_CALL(*cb, onDecompressionError()).Times(0);
  co_await co_awaitTry(ingressFilter->readHeaderEvent());
  co_await co_awaitTry(ingressFilter->readBodyEvent());
}

CO_TEST_F_X(DecompressionIngressFilterTest, TestStatsCallback_Failure) {
  // gzip of "abcdefghijklmnopqrstuvwxyz" prefixed with "faceb00c"
  auto gzip = folly::unhexlify(
      "faceb00c"
      "1f8b080092a3326600ff011a00e5ff6162636465666768696a6b6c6d6e6f707172737475"
      "767778797abd50274c1a000000");

  HTTPFixedSource respSource(makeResponse(200),
                             /*body=*/folly::IOBuf::copyBuffer(gzip));
  respSource.msg_->getHeaders().set(HTTP_HEADER_CONTENT_ENCODING, "gzip");

  auto cb = std::make_shared<MockStatsCallback>();
  auto* ingressFilter = new DecompressionIngressFilter(&respSource, cb);
  ingressFilter->setHeapAllocated();

  EXPECT_CALL(*cb, onDecompressionAlgo("gzip")).Times(1);
  EXPECT_CALL(*cb, onDecompressionError()).Times(1);

  co_await co_awaitTry(ingressFilter->readHeaderEvent());
  auto bodyEvent = co_await co_awaitTry(ingressFilter->readBodyEvent());
  XCHECK(bodyEvent.hasException());
}

} // namespace proxygen::coro::test
