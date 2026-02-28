/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <folly/coro/GtestHelpers.h>
#include <folly/io/async/EventBaseManager.h>
#include <string>

#include "proxygen/lib/http/coro/HTTPEvents.h"
#include "proxygen/lib/http/coro/HTTPFilterFactoryHandler.h"
#include "proxygen/lib/http/coro/HTTPFixedSource.h"
#include "proxygen/lib/http/coro/HTTPHybridSource.h"
#include "proxygen/lib/http/coro/HTTPSource.h"
#include "proxygen/lib/http/coro/HTTPSourceFilter.h"
#include "proxygen/lib/http/coro/filters/FilterFactory.h"
#include "proxygen/lib/http/coro/filters/MutateFilter.h"
#include "proxygen/lib/http/coro/test/Mocks.h"

namespace proxygen::coro::test {

class HTTPFilterFactoryHandlerTests : public ::testing::Test {
 protected:
  void SetUp() override {
    EXPECT_CALL(mockSource_, readHeaderEvent())
        .WillOnce(folly::coro::gmock_helpers::CoInvoke(
            [&]() -> folly::coro::Task<HTTPHeaderEvent> {
              co_return HTTPHeaderEvent(std::make_unique<HTTPMessage>(), true);
            }));
  }
  void TearDown() override {
  }

  MockHTTPSessionContext mockSession_;
  MockHTTPSource mockSource_;

  /**
   * FilterFactory that simply appends the name of the filter (e.g.
   * "reqFilterA", "resFilterA") to the "req-filter" or "res-filter" header
   * field value
   */
  enum class Direction { Request, Response, RequestResponse };
  class AppendFilterNameFilterFactory : public FilterFactory {
   public:
    AppendFilterNameFilterFactory(std::string filterName, Direction direction)
        : filterName_(std::move(filterName)), direction_(direction) {
    }

    std::pair<HTTPSourceFilter*, HTTPSourceFilter*> makeFilters() override {
      HTTPSourceFilter *reqFilter{nullptr}, *resFilter{nullptr};

      if (direction_ != Direction::Response) {
        reqFilter = new MutateFilter(
            /*source*/
            nullptr,
            [this](HTTPHeaderEvent& event) {
              auto& headers = event.headers->getHeaders();
              const auto& value = headers.getSingleOrEmpty("req-filter");
              auto newValue =
                  folly::to<std::string>(value, "req", filterName_, ",");
              headers.set("req-filter", newValue);
            },
            nullptr);
        reqFilter->setHeapAllocated();
      }

      if (direction_ != Direction::Request) {
        resFilter = new MutateFilter(
            /*source*/
            nullptr,
            [this](HTTPHeaderEvent& event) {
              auto& headers = event.headers->getHeaders();
              const auto& value = headers.getSingleOrEmpty("res-filter");
              auto newValue =
                  folly::to<std::string>(value, "res", filterName_, ",");
              headers.set("res-filter", newValue);
            },
            nullptr);
        resFilter->setHeapAllocated();
      }

      return {reqFilter, resFilter};
    }

    std::string filterName_;
    Direction direction_;
  };

  /**
   * Handler that reads the request to completion expects the "req-filter"
   * header contains the expected value. This is done to validate the order in
   * which req are installed by HTTPServer.
   */
  class ReqFilterNameHandler : public HTTPHandler {
   public:
    ReqFilterNameHandler(std::string expectedReqFilterValue = "")
        : expectedReqFilterValue_(std::move(expectedReqFilterValue)) {
    }

    folly::coro::Task<HTTPSourceHolder> handleRequest(
        folly::EventBase* /*evb*/,
        HTTPSessionContextPtr /*ctx*/,
        HTTPSourceHolder requestSource) override {
      // copy over "req-filter" header over to the response
      std::string reqFilterHeaderValue;
      HTTPSourceReader reader(std::move(requestSource));
      co_await reader
          .onHeaders([this](std::unique_ptr<HTTPMessage> msg,
                            bool /*final*/,
                            bool /*eom*/) {
            CHECK(msg);
            const auto& headers = msg->getHeaders();
            EXPECT_EQ(headers.getSingleOrEmpty("req-filter"),
                      expectedReqFilterValue_);
            return HTTPSourceReader::Continue;
          })
          .read();

      co_return sourceFactory ? sourceFactory()
                              : HTTPFixedSource::makeFixedResponse(200, "OK");
    }

    std::string expectedReqFilterValue_;
    std::function<HTTPSource*(void)> sourceFactory{nullptr};
  };
};

CO_TEST_F(HTTPFilterFactoryHandlerTests, TestEmptyFilters) {
  auto handler = std::make_shared<ReqFilterNameHandler>();
  auto filterFactoryHandler = std::make_shared<HTTPFilterFactoryHandler>();
  filterFactoryHandler->setNextHandler(handler);

  auto result = co_await filterFactoryHandler->handleRequest(
      folly::EventBaseManager::get()->getEventBase(),
      mockSession_.acquireKeepAlive(),
      HTTPSourceHolder(&mockSource_));

  auto headerEvent =
      co_await folly::coro::co_awaitTry(result.readHeaderEvent());
  EXPECT_EQ(headerEvent->headers->getHeaders().getSingleOrEmpty("res-filter"),
            "");
}

// Validate order of filter execution – on the request path, filterA get's
// executed first then filterB, and on the response path filterC gets executed
// first then filterA.
CO_TEST_F(HTTPFilterFactoryHandlerTests, TestFilterFactoriesSimple) {
  std::string expectedReqFilterValue = "reqFilterA,reqFilterB,";
  std::string expectedRespFilterValue = "resFilterC,resFilterA,";

  auto handler = std::make_shared<ReqFilterNameHandler>(expectedReqFilterValue);
  auto filterFactoryHandler = std::make_shared<HTTPFilterFactoryHandler>();
  filterFactoryHandler->setNextHandler(handler);

  // create req&res "filterA", req "filterB" and res "filterC" and insert into
  // filterFactories list
  auto reqResFilterA = std::make_shared<AppendFilterNameFilterFactory>(
      "FilterA", Direction::RequestResponse);
  auto reqFilterB = std::make_shared<AppendFilterNameFilterFactory>(
      "FilterB", Direction::Request);
  auto resFilterC = std::make_shared<AppendFilterNameFilterFactory>(
      "FilterC", Direction::Response);
  filterFactoryHandler->addFilterFactory(std::move(reqResFilterA))
      .addFilterFactory(std::move(reqFilterB))
      .addFilterFactory(std::move(resFilterC));

  auto result = co_await filterFactoryHandler->handleRequest(
      folly::EventBaseManager::get()->getEventBase(),
      mockSession_.acquireKeepAlive(),
      HTTPSourceHolder(&mockSource_));

  auto headerEvent =
      co_await folly::coro::co_awaitTry(result.readHeaderEvent());
  EXPECT_EQ(headerEvent->headers->getHeaders().getSingleOrEmpty("res-filter"),
            expectedRespFilterValue);
}

// Similar to the test above, but here the handler returns nullptr. Validate
// order of filter execution – on the request path, filterA get's executed
// first then filterB, and on the response path filterC gets executed first
// then filterA (even when nullptr source is returned by the handler).
CO_TEST_F(HTTPFilterFactoryHandlerTests, TestHandlerNullptrSource) {
  std::string expectedReqFilterValue = "reqFilterA,reqFilterB,";
  std::string expectedRespFilterValue = "resFilterC,resFilterA,";

  auto handler = std::make_shared<ReqFilterNameHandler>(expectedReqFilterValue);
  handler->sourceFactory = []() { return nullptr; };
  auto filterFactoryHandler = std::make_shared<HTTPFilterFactoryHandler>();
  filterFactoryHandler->setNextHandler(handler);

  // create req&res "filterA", req "filterB" and res "filterC" and insert into
  // filterFactories list
  auto reqResFilterA = std::make_shared<AppendFilterNameFilterFactory>(
      "FilterA", Direction::RequestResponse);
  auto reqFilterB = std::make_shared<AppendFilterNameFilterFactory>(
      "FilterB", Direction::Request);
  auto resFilterC = std::make_shared<AppendFilterNameFilterFactory>(
      "FilterC", Direction::Response);
  filterFactoryHandler->addFilterFactory(std::move(reqResFilterA))
      .addFilterFactory(std::move(reqFilterB))
      .addFilterFactory(std::move(resFilterC));

  auto result = co_await filterFactoryHandler->handleRequest(
      folly::EventBaseManager::get()->getEventBase(),
      mockSession_.acquireKeepAlive(),
      HTTPSourceHolder(&mockSource_));

  auto headerEvent =
      co_await folly::coro::co_awaitTry(result.readHeaderEvent());
  EXPECT_EQ(headerEvent->headers->getHeaders().getSingleOrEmpty("res-filter"),
            expectedRespFilterValue);
}

} // namespace proxygen::coro::test
