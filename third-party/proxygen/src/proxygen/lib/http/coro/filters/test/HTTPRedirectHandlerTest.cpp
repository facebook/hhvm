/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "proxygen/lib/http/coro/filters/HTTPRedirectHandler.h"
#include "proxygen/lib/http/codec/test/TestUtils.h"
#include "proxygen/lib/http/coro/HTTPFixedSource.h"
#include "proxygen/lib/http/coro/HTTPSourceReader.h"
#include "proxygen/lib/http/coro/test/HTTPTestSources.h"
#include <folly/logging/xlog.h>

#include "proxygen/lib/http/coro/test/Mocks.h"
#include <folly/coro/GmockHelpers.h>
#include <folly/coro/GtestHelpers.h>
#include <folly/coro/Sleep.h>

using namespace testing;

namespace proxygen::coro::test {

HTTPFixedSource* getRedirectResponse(
    uint16_t code,
    const std::string& location,
    std::unique_ptr<folly::IOBuf> body = nullptr) {
  auto resp = HTTPFixedSource::makeFixedResponse(code);
  if (!location.empty()) {
    resp->msg_->getHeaders().set(HTTP_HEADER_LOCATION, location);
  }
  resp->body_ = std::move(body);
  return resp;
}

class MockSessionFactory : public HTTPSessionFactory {
 public:
  ~MockSessionFactory() override = default;
  MOCK_METHOD(folly::coro::Task<GetSessionResult>,
              getSessionWithReservation,
              (std::string,
               uint16_t,
               bool,
               std::chrono::milliseconds,
               folly::Optional<std::string>));
  MOCK_METHOD(bool, requiresAbsoluteURLs, (), (const));
};

class HTTPRedirectHandlerTest : public testing::Test {
 public:
  void SetUp() override {
    ON_CALL(*mockSessionFactory_, requiresAbsoluteURLs)
        .WillByDefault(ReturnPointee(&requiresAbsoluteURLs_));
  }

  folly::coro::Task<void> run(
      HTTPSource* reqSource,
      HTTPSource* respSource,
      uint16_t expectedCode = 200,
      std::function<void(const std::string&)> redirectCallback = nullptr) {
    HTTPRedirectHandler redirectHandler(mockSessionFactory_);
    // ok if nullptr
    redirectHandler.setRedirectCallback(std::move(redirectCallback));

    HTTPSourceReader reqReader;
    reqReader.setSource(redirectHandler.getRequestFilter(reqSource));
    co_await reqReader.read();
    HTTPSourceReader respReader;
    respReader.onHeaders(
        [expectedCode](std::unique_ptr<HTTPMessage> resp, bool, bool) {
          EXPECT_EQ(resp->getStatusCode(), expectedCode);
          return HTTPSourceReader::Continue;
        });
    respReader.setSource(redirectHandler.getResponseFilter(respSource));
    co_await respReader.read();
  }

 protected:
  void expectRedirect(
      const std::string& host,
      uint16_t port,
      bool secure,
      std::function<folly::coro::Task<HTTPSourceHolder>(
          HTTPSourceHolder, HTTPCoroSession::RequestReservation)> sendFn) {
    EXPECT_CALL(*mockSessionFactory_,
                getSessionWithReservation(host, port, secure, _, _))
        .WillOnce(folly::coro::gmock_helpers::CoReturnByMove(
            HTTPSessionFactory::GetSessionResult(
                HTTPCoroSession::RequestReservation(), &mockSession_)));
    EXPECT_CALL(mockSession_, sendRequest(_, _))
        .WillOnce(folly::coro::gmock_helpers::CoInvoke(
            [sendFn =
                 std::move(sendFn)](HTTPSourceHolder request,
                                    HTTPCoroSession::RequestReservation res)
                -> folly::coro::Task<HTTPSourceHolder> {
              return sendFn(std::move(request), std::move(res));
            }));
  }

  std::shared_ptr<MockSessionFactory> mockSessionFactory_{
      std::make_shared<MockSessionFactory>()};
  MockHTTPCoroSession mockSession_;
  bool requiresAbsoluteURLs_{false};
};

CO_TEST_F(HTTPRedirectHandlerTest, NoRedirect) {
  co_await run(
      HTTPFixedSource::makeFixedRequest(URL("https://www.facebook.com/")),
      HTTPFixedSource::makeFixedResponse(200, "success"));
}

CO_TEST_F(HTTPRedirectHandlerTest, BasicRedirects) {
  auto codes = {301, 302, 303, 307};
  for (auto code : codes) {
    expectRedirect("www.facebook-redirect.com",
                   443,
                   true,
                   [](HTTPSourceHolder, HTTPCoroSession::RequestReservation)
                       -> folly::coro::Task<HTTPSourceHolder> {
                     co_return HTTPFixedSource::makeFixedResponse(200,
                                                                  "success");
                   });
    co_await run(
        HTTPFixedSource::makeFixedRequest(URL("https://www.facebook.com/")),
        getRedirectResponse(code, "https://www.facebook-redirect.com/"));
  }
}

CO_TEST_F(HTTPRedirectHandlerTest, AbsoluteRedirect) {
  requiresAbsoluteURLs_ = true;
  expectRedirect(
      "www.facebook-redirect.com",
      443,
      true,
      [](HTTPSourceHolder reqSource, HTTPCoroSession::RequestReservation)
          -> folly::coro::Task<HTTPSourceHolder> {
        auto headerEvent = co_await reqSource.readHeaderEvent();
        EXPECT_EQ(headerEvent.headers->getURL(),
                  "https://www.facebook-redirect.com/");
        co_return HTTPFixedSource::makeFixedResponse(200, "success");
      });
  co_await run(
      HTTPFixedSource::makeFixedRequest(URL("https://www.facebook.com/")),
      getRedirectResponse(302, "https://www.facebook-redirect.com/"));
}

// 301 redirect on a POST is not handleable
CO_TEST_F(HTTPRedirectHandlerTest, PostRedirect301) {
  co_await run(
      HTTPFixedSource::makeFixedRequest(
          URL("https://www.facebook.com/"), HTTPMethod::POST, makeBuf(100)),
      getRedirectResponse(301, "https://www.facebook-redirect.com/"),
      301);
}

// 303 redirect on a POST discards the request body and converts to GET
CO_TEST_F(HTTPRedirectHandlerTest, NoBodyOn303PostRedirect) {
  expectRedirect(
      "www.facebook-redirect.com",
      443,
      true,
      [](HTTPSourceHolder request, HTTPCoroSession::RequestReservation)
          -> folly::coro::Task<HTTPSourceHolder> {
        auto headerEvent = co_await request.readHeaderEvent();
        EXPECT_EQ(headerEvent.headers->getMethod(), HTTPMethod::GET);
        EXPECT_TRUE(headerEvent.eom);
        // validate content length header is cleared
        EXPECT_FALSE(headerEvent.headers->getHeaders().exists(
            HTTP_HEADER_CONTENT_LENGTH));
        co_return HTTPFixedSource::makeFixedResponse(200, "success");
      });
  co_await run(
      HTTPFixedSource::makeFixedRequest(
          URL("https://www.facebook.com/"), HTTPMethod::POST, makeBuf(100)),
      getRedirectResponse(303, "https://www.facebook-redirect.com/"));
}

// Non GET/HEAD/POST are unredirectable
CO_TEST_F(HTTPRedirectHandlerTest, OptionsRedirect) {
  co_await run(HTTPFixedSource::makeFixedRequest(
                   URL("https://www.facebook.com/"), HTTPMethod::OPTIONS),
               getRedirectResponse(303, "https://www.facebook-redirect.com/"),
               303);
}

// Can only redirect to http or https
CO_TEST_F(HTTPRedirectHandlerTest, UnknownScheme) {
  auto maybe = co_await co_awaitTry(
      run(HTTPFixedSource::makeFixedRequest(URL("https://www.facebook.com/")),
          getRedirectResponse(302, "masque://www.facebook-redirect.com/")));
  EXPECT_TRUE(maybe.hasException());
  EXPECT_EQ(maybe.tryGetExceptionObject<HTTPRedirectHandler::Exception>()->type,
            HTTPRedirectHandler::Exception::Type::UnsupportedScheme);
}

// Must have a location header
CO_TEST_F(HTTPRedirectHandlerTest, NoLocation) {
  auto maybe = co_await co_awaitTry(
      run(HTTPFixedSource::makeFixedRequest(URL("https://www.facebook.com/")),
          getRedirectResponse(302, "")));
  EXPECT_TRUE(maybe.hasException());
  EXPECT_EQ(maybe.tryGetExceptionObject<HTTPRedirectHandler::Exception>()->type,
            HTTPRedirectHandler::Exception::Type::InvalidRedirect);
}

// Exceeded MaxRedirects (default=1)
CO_TEST_F(HTTPRedirectHandlerTest, MaxRedirects) {
  expectRedirect("www.facebook-redirect.com",
                 443,
                 true,
                 [](HTTPSourceHolder, HTTPCoroSession::RequestReservation)
                     -> folly::coro::Task<HTTPSourceHolder> {
                   co_return getRedirectResponse(
                       302, "https://www.facebook-redirect.com/");
                 });
  auto maybe = co_await co_awaitTry(
      run(HTTPFixedSource::makeFixedRequest(URL("https://www.facebook.com/")),
          getRedirectResponse(302, "https://www.facebook-redirect.com/")));
  EXPECT_TRUE(maybe.hasException());
  EXPECT_EQ(maybe.tryGetExceptionObject<HTTPRedirectHandler::Exception>()->type,
            HTTPRedirectHandler::Exception::Type::MaxRedirects);
}

// Location header unparsable
CO_TEST_F(HTTPRedirectHandlerTest, InvalidLocation) {
  auto maybe = co_await co_awaitTry(
      run(HTTPFixedSource::makeFixedRequest(URL("https://www.facebook.com/")),
          getRedirectResponse(302, ":")));
  EXPECT_TRUE(maybe.hasException());
  EXPECT_EQ(maybe.tryGetExceptionObject<HTTPRedirectHandler::Exception>()->type,
            HTTPRedirectHandler::Exception::Type::InvalidRedirect);
}

CO_TEST_F(HTTPRedirectHandlerTest, AbortIncompleteSourceOnError) {
  class StopFilter : public HTTPSourceFilter {
   public:
    void stopReading(
        folly::Optional<const proxygen::coro::HTTPErrorCode> err) override {
      stopped_ = true;
      HTTPSourceFilter::stopReading(err);
    }
    bool stopped_{false};
  };

  HTTPRedirectHandler redirectHandler(mockSessionFactory_);
  HTTPSourceReader reqReader;
  reqReader.setSource(redirectHandler.getRequestFilter(
      HTTPFixedSource::makeFixedRequest(URL("https://www.facebook.com/"))));
  co_await reqReader.read();
  HTTPSourceReader respReader;
  StopFilter stopFilter;
  stopFilter.setSource(getRedirectResponse(302, ":", makeBuf(10)));
  respReader.setSource(redirectHandler.getResponseFilter(&stopFilter));
  auto maybe = co_await co_awaitTry(respReader.read());
  EXPECT_TRUE(maybe.hasException());
  EXPECT_EQ(maybe.tryGetExceptionObject<HTTPRedirectHandler::Exception>()->type,
            HTTPRedirectHandler::Exception::Type::InvalidRedirect);
  // source stopped before HTTPRedirectHandler goes out of scope
  EXPECT_TRUE(stopFilter.stopped_);
}

// A relative redirect with an absolute original URL will use the URL
// host/port/scheme
CO_TEST_F(HTTPRedirectHandlerTest, RelativeRedirectAbsoluteOriginal) {
  expectRedirect(
      "www.facebook.com",
      443,
      true,
      [](HTTPSourceHolder request, HTTPCoroSession::RequestReservation)
          -> folly::coro::Task<HTTPSourceHolder> {
        auto headerEvent = co_await request.readHeaderEvent();
        EXPECT_EQ(headerEvent.headers->getURL(), "/redirect");
        EXPECT_EQ(headerEvent.headers->getHeaders().getSingleOrEmpty(
                      HTTP_HEADER_HOST),
                  "www.facebook.com");
        co_return HTTPFixedSource::makeFixedResponse(200, "success");
      });

  auto req = HTTPFixedSource::makeFixedRequest("/");
  req->msg_->setURL("https://www.facebook.com");
  req->msg_->setSecure(true);
  co_await run(req, getRedirectResponse(302, "/redirect"));
}

// A relative redirect with an absolute original URL will use the URL
// host/port/scheme, even through a proxy that requires absolute URLs
CO_TEST_F(HTTPRedirectHandlerTest, RelativeRedirectAbsoluteOriginalProxy) {
  requiresAbsoluteURLs_ = true;
  expectRedirect(
      "www.facebook.com",
      443,
      true,
      [](HTTPSourceHolder request, HTTPCoroSession::RequestReservation)
          -> folly::coro::Task<HTTPSourceHolder> {
        auto headerEvent = co_await request.readHeaderEvent();
        EXPECT_EQ(headerEvent.headers->getURL(),
                  "https://www.facebook.com/redirect");
        co_return HTTPFixedSource::makeFixedResponse(200, "success");
      });

  auto req = HTTPFixedSource::makeFixedRequest("/");
  req->msg_->setURL("https://www.facebook.com");
  req->msg_->setSecure(true);
  co_await run(req, getRedirectResponse(302, "/redirect"));
}

// A relative redirect with an relative original URL will use the Host header
// and original request scheme
CO_TEST_F(HTTPRedirectHandlerTest, RelativeRedirectHostHeader) {
  expectRedirect(
      "www.facebook.com",
      80,
      false,
      [](HTTPSourceHolder request, HTTPCoroSession::RequestReservation)
          -> folly::coro::Task<HTTPSourceHolder> {
        auto headerEvent = co_await request.readHeaderEvent();
        EXPECT_EQ(headerEvent.headers->getURL(), "/redirect");
        EXPECT_EQ(headerEvent.headers->getHeaders().getSingleOrEmpty(
                      HTTP_HEADER_HOST),
                  "www.facebook.com");
        co_return HTTPFixedSource::makeFixedResponse(200, "success");
      });
  co_await run(
      HTTPFixedSource::makeFixedRequest(URL("http://www.facebook.com/")),
      getRedirectResponse(302, "/redirect"));
}

// Handles non-default port in the host header
CO_TEST_F(HTTPRedirectHandlerTest, RelativeRedirectHostHeaderNonDefaultPort) {
  expectRedirect(
      "www.facebook.com",
      444,
      true,
      [](HTTPSourceHolder request, HTTPCoroSession::RequestReservation)
          -> folly::coro::Task<HTTPSourceHolder> {
        auto headerEvent = co_await request.readHeaderEvent();
        EXPECT_EQ(headerEvent.headers->getURL(), "/redirect");
        EXPECT_EQ(headerEvent.headers->getHeaders().getSingleOrEmpty(
                      HTTP_HEADER_HOST),
                  "www.facebook.com:444");
        co_return HTTPFixedSource::makeFixedResponse(200, "success");
      });
  co_await run(
      HTTPFixedSource::makeFixedRequest(URL("https://www.facebook.com:444/")),
      getRedirectResponse(302, "/redirect"));
}

// If the original URL was relative and no Host header is present, a relative
// redirect will fail.  This is a limitation of the filter architecture, since
// there's no way to retrieve the original session details.
CO_TEST_F(HTTPRedirectHandlerTest, RelativeRedirectNoHostHeader) {
  auto maybe = co_await co_awaitTry(run(HTTPFixedSource::makeFixedRequest("/"),
                                        getRedirectResponse(302, "/redirect")));
  EXPECT_TRUE(maybe.hasException());
  EXPECT_EQ(maybe.tryGetExceptionObject<HTTPRedirectHandler::Exception>()->type,
            HTTPRedirectHandler::Exception::Type::InvalidRedirect);
}

// Relative redirect when the original Host header was garbage
CO_TEST_F(HTTPRedirectHandlerTest, RelativeRedirectBadHostHeader) {
  auto req = HTTPFixedSource::makeFixedRequest("/");
  req->msg_->getHeaders().set(HTTP_HEADER_HOST, "abc:123456789");
  auto maybe =
      co_await co_awaitTry(run(req, getRedirectResponse(302, "/redirect")));
  EXPECT_TRUE(maybe.hasException());
  EXPECT_EQ(maybe.tryGetExceptionObject<HTTPRedirectHandler::Exception>()->type,
            HTTPRedirectHandler::Exception::Type::InvalidRedirect);
}

// The redirect response contains a body, but we discard it (will manifest
// as stopReading, and the redirect source will delete itself, otherwise it
// would be a leak.
CO_TEST_F(HTTPRedirectHandlerTest, RedirectWithBody) {
  expectRedirect(
      "www.facebook-redirect.com",
      443,
      true,
      [](HTTPSourceHolder request, HTTPCoroSession::RequestReservation)
          -> folly::coro::Task<HTTPSourceHolder> {
        auto headerEvent = co_await request.readHeaderEvent();
        EXPECT_TRUE(headerEvent.eom);
        co_return HTTPFixedSource::makeFixedResponse(200, "success");
      });
  co_await run(
      HTTPFixedSource::makeFixedRequest(URL("https://www.facebook.com/")),
      getRedirectResponse(
          301, "https://www.facebook-redirect.com/", makeBuf(100)));
}

// stopReading before the first redirect comes back -- pass through to the
// original response source.
CO_TEST_F(HTTPRedirectHandlerTest, StopReading) {
  HTTPRedirectHandler redirectHandler(mockSessionFactory_);
  HTTPSourceReader reqReader;
  reqReader.setSource(redirectHandler.getRequestFilter(
      HTTPFixedSource::makeFixedRequest(URL("https://www.facebook.com/"))));
  co_await reqReader.read();
  MockHTTPSource mockSource;
  folly::CancellationSource cancellationSource;
  EXPECT_CALL(mockSource, readHeaderEvent())
      .WillOnce(folly::coro::gmock_helpers::CoInvoke(
          [&cancellationSource]() -> folly::coro::Task<HTTPHeaderEvent> {
            co_await folly::coro::co_withCancellation(
                cancellationSource.getToken(),
                folly::coro::sleep(std::chrono::minutes(1)));
            XLOG(FATAL) << "Unreachable";
          }));
  EXPECT_CALL(mockSource, stopReading(_))
      .WillOnce(Invoke(
          [&cancellationSource] { cancellationSource.requestCancellation(); }));
  auto respSource = redirectHandler.getResponseFilter(&mockSource);

  auto headerFut = co_withExecutor(co_await folly::coro::co_current_executor,
                                   respSource->readHeaderEvent())
                       .start()
                       .via(co_await folly::coro::co_current_executor);
  co_await folly::coro::co_reschedule_on_current_executor;
  respSource->stopReading();
  EXPECT_THROW(co_await std::move(headerFut), folly::OperationCancelled);
}

// Stop reading while connecting.  Connect in progress is cancelled.
CO_TEST_F(HTTPRedirectHandlerTest, StopReadingWhileConnecting) {
  EXPECT_CALL(
      *mockSessionFactory_,
      getSessionWithReservation("www.facebook-redirect.com", 443, true, _, _))
      .WillOnce(folly::coro::gmock_helpers::CoInvoke(
          [](std::string, uint16_t, bool, std::chrono::milliseconds, auto)
              -> folly::coro::Task<HTTPSessionFactory::GetSessionResult> {
            co_await folly::coro::sleep(std::chrono::minutes(1));
            XLOG(FATAL) << "Unreachable";
          }));
  HTTPRedirectHandler redirectHandler(mockSessionFactory_);
  HTTPSourceReader reqReader;
  reqReader.setSource(redirectHandler.getRequestFilter(
      HTTPFixedSource::makeFixedRequest(URL("https://www.facebook.com/"))));
  co_await reqReader.read();
  auto respSource = redirectHandler.getResponseFilter(
      getRedirectResponse(303, "https://www.facebook-redirect.com/"));

  auto headerFut = co_withExecutor(co_await folly::coro::co_current_executor,
                                   respSource->readHeaderEvent())
                       .start()
                       .via(co_await folly::coro::co_current_executor);
  co_await folly::coro::co_reschedule_on_current_executor;
  respSource->stopReading();
  EXPECT_THROW(co_await std::move(headerFut), folly::OperationCancelled);
}

// Stop reading while sending, send is cancelled.
CO_TEST_F(HTTPRedirectHandlerTest, StopReadingWhileSending) {
  expectRedirect("www.facebook-redirect.com",
                 443,
                 true,
                 [](HTTPSourceHolder, HTTPCoroSession::RequestReservation)
                     -> folly::coro::Task<HTTPSourceHolder> {
                   co_await folly::coro::sleep(std::chrono::minutes(1));
                   XLOG(FATAL) << "Unreachable";
                 });
  HTTPRedirectHandler redirectHandler(mockSessionFactory_);
  HTTPSourceReader reqReader;
  reqReader.setSource(redirectHandler.getRequestFilter(
      HTTPFixedSource::makeFixedRequest(URL("https://www.facebook.com/"))));
  co_await reqReader.read();
  auto respSource = redirectHandler.getResponseFilter(
      getRedirectResponse(303, "https://www.facebook-redirect.com/"));

  auto headerFut = co_withExecutor(co_await folly::coro::co_current_executor,
                                   respSource->readHeaderEvent())
                       .start()
                       .via(co_await folly::coro::co_current_executor);
  co_await folly::coro::co_reschedule_on_current_executor;
  co_await folly::coro::co_reschedule_on_current_executor;
  respSource->stopReading();
  EXPECT_THROW(co_await std::move(headerFut), folly::OperationCancelled);
}

CO_TEST_F(HTTPRedirectHandlerTest, TestCallback) {
  expectRedirect("www.facebook-redirect.com",
                 443,
                 true,
                 [](HTTPSourceHolder, HTTPCoroSession::RequestReservation)
                     -> folly::coro::Task<HTTPSourceHolder> {
                   co_return HTTPFixedSource::makeFixedResponse(200, "success");
                 });
  std::string redirectUrl;
  co_await run(
      HTTPFixedSource::makeFixedRequest(URL("https://www.facebook.com/")),
      getRedirectResponse(302, "https://www.facebook-redirect.com/"),
      200,
      [&redirectUrl](const std::string& url) { redirectUrl = url; });

  EXPECT_EQ(redirectUrl, "https://www.facebook-redirect.com/");
}

} // namespace proxygen::coro::test
