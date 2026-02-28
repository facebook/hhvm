/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <proxygen/lib/http/codec/test/TestUtils.h>
#include <proxygen/lib/http/coro/HTTPFixedSource.h>
#include <proxygen/lib/http/coro/HTTPSourceReader.h>
#include <proxygen/lib/http/coro/HTTPStreamSource.h>
#include <proxygen/lib/http/coro/filters/TransformFilter.h>
#include <proxygen/lib/http/coro/util/test/TestHelpers.h>

using namespace testing;

namespace proxygen::coro::test {

class TransformFilterTest : public testing::Test {
 public:
  folly::DrivableExecutor* getExecutor() {
    return &evb_;
  }

 protected:
  folly::EventBase evb_;
};

CO_TEST_F_X(TransformFilterTest, BasicTest) {
  auto msg = makeResponse(200);
  // create response source
  HTTPStreamSource respSource(&evb_);
  respSource.headers(std::move(msg), /*eom=*/false);
  respSource.body(
      folly::IOBuf::copyBuffer("hello"), /*padding=*/0, /*eom=*/false);

  // header transform filter adds two header fields into HTTPHeaders
  TransformFilter::HeaderTransformFn headerHook =
      [](folly::Try<HTTPHeaderEvent>&& headerEvent) {
        CHECK(!headerEvent.hasException());
        // add two random header fields
        auto& headers = headerEvent->headers->getHeaders();
        headers.add("x-header-a", "x-value-a");
        headers.add("x-header-b", "x-value-b");
        return std::move(headerEvent);
      };

  // body transform filter ("hello", eom=false) -> ("world", eom=true)
  TransformFilter::BodyTransformFn bodyHook =
      [](folly::Try<HTTPBodyEvent>&& bodyEvent) {
        CHECK(!bodyEvent.hasException());
        // replace "hello" in body with "world"
        EXPECT_EQ(bodyEvent->eventType, HTTPBodyEvent::BODY);
        CHECK(!bodyEvent->event.body.empty());
        auto bodyStr = bodyEvent->event.body.move()->to<std::string>();
        EXPECT_EQ(bodyStr, "hello");
        bodyEvent->event.body.append(folly::IOBuf::copyBuffer("world"));
        // inject eom to terminate HTTPSourceReader
        bodyEvent->eom = true;
        return std::move(bodyEvent);
      };

  auto* transformSource = new TransformFilter(
      &respSource, std::move(headerHook), std::move(bodyHook));
  transformSource->setHeapAllocated();

  HTTPSourceReader reader(transformSource);
  reader.onHeaders(
      [](std::unique_ptr<HTTPMessage> msg, bool /*final*/, bool eom) {
        const auto& headers = msg->getHeaders();
        EXPECT_TRUE(headers.exists("x-header-a") &&
                    headers.exists("x-header-b"));
        EXPECT_EQ(headers.getSingleOrEmpty("x-header-a"), "x-value-a");
        EXPECT_EQ(headers.getSingleOrEmpty("x-header-b"), "x-value-b");
        EXPECT_FALSE(eom);
        return HTTPSourceReader::Continue;
      });

  reader.onBody([](BufQueue body, bool /*eom*/) {
    CHECK(!body.empty());
    auto bodyStr = body.move()->to<std::string>();
    EXPECT_EQ(bodyStr, "world");
    return HTTPSourceReader::Continue;
  });

  // read response
  auto res = co_await co_awaitTry(reader.read());
  EXPECT_FALSE(res.hasException());
}

CO_TEST_F_X(TransformFilterTest, PassthruFilter) {
  auto msg = makeResponse(200);
  constexpr std::string_view kBodyStr = "hello!";
  // save ptr to verify same underlying msg
  auto* pMsg = msg.get();
  auto body = folly::IOBuf::wrapBuffer(kBodyStr.begin(), kBodyStr.size());

  // create response source, enqueue events
  HTTPStreamSource respSource(&evb_);
  respSource.headers(std::move(msg), /*eom=*/false);
  respSource.body(std::move(body), /*padding=*/0, /*eom=*/true);

  // create passthru transform filter
  auto* transformSource = new TransformFilter(&respSource, nullptr, nullptr);
  transformSource->setHeapAllocated();

  HTTPSourceReader reader(transformSource);
  // verify ptrs are equal
  reader.onHeaders(
      [=](std::unique_ptr<HTTPMessage> msg, bool /*final*/, bool /*eom*/) {
        EXPECT_EQ(msg.get(), pMsg);
        return HTTPSourceReader::Continue;
      });
  reader.onBody([&kBodyStr](BufQueue body, bool /*eom*/) {
    EXPECT_EQ((void*)CHECK_NOTNULL(body.front())->data(), kBodyStr.begin());
    return HTTPSourceReader::Continue;
  });

  auto res = co_await co_awaitTry(reader.read());
  EXPECT_FALSE(res.hasException());
}

CO_TEST_F_X(TransformFilterTest, InvokeOnError) {
  auto* respSource =
      new HTTPErrorSource(HTTPError(HTTPErrorCode::CANCEL, "cancelled"));
  bool headerCbInvoked = false;
  TransformFilter::HeaderTransformFn headerHook =
      [&headerCbInvoked](folly::Try<HTTPHeaderEvent>&& headerEvent) {
        EXPECT_TRUE(headerEvent.hasException());
        headerCbInvoked = true;
        return std::move(headerEvent);
      };
  TransformFilter::BodyTransformFn bodyHook =
      [](folly::Try<HTTPBodyEvent>&& bodyEvent) {
        LOG(FATAL) << "unreachable";
        return folly::Try<HTTPBodyEvent>();
      };

  auto* transformSource = new TransformFilter(
      respSource, std::move(headerHook), std::move(bodyHook));
  transformSource->setHeapAllocated();

  HTTPSourceReader reader(transformSource);
  // read response
  auto res = co_await co_awaitTry(reader.read());
  EXPECT_TRUE(res.hasException());
  EXPECT_TRUE(headerCbInvoked);
}

CO_TEST_F_X(TransformFilterTest, TransformBodyToError) {
  auto respSource = HTTPFixedSource::makeFixedResponse(200, "~body~");
  // no-op header event
  TransformFilter::HeaderTransformFn headerHook =
      [](folly::Try<HTTPHeaderEvent>&& headerEvent) {
        EXPECT_FALSE(headerEvent.hasException());
        return std::move(headerEvent);
      };
  // transform body event into error arbitrarily just because
  TransformFilter::BodyTransformFn bodyHook =
      [](folly::Try<HTTPBodyEvent>&& bodyEvent) {
        EXPECT_FALSE(bodyEvent.hasException());
        return folly::Try<HTTPBodyEvent>(
            HTTPError(HTTPErrorCode::INTERNAL_ERROR, "~~error~~"));
      };

  auto* transformSource = new TransformFilter(
      respSource, std::move(headerHook), std::move(bodyHook));
  transformSource->setHeapAllocated();

  HTTPSourceReader reader(transformSource);

  reader.onBody([](BufQueue body, bool /*eom*/) {
    LOG(FATAL) << "shouldn't happen";
    return HTTPSourceReader::Continue;
  });

  // read response
  auto res = co_await co_awaitTry(reader.read());
  EXPECT_TRUE(res.hasException());
}

} // namespace proxygen::coro::test
