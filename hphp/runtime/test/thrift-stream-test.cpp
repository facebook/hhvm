/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
*/

#include <cstdint>
#include <exception>
#include <stdexcept>
#include <string>
#include <tuple>
#include <utility>

#include <folly/ExceptionWrapper.h>
#include <folly/io/IOBuf.h>
#include <folly/io/async/ScopedEventBaseThread.h>
#include <gtest/gtest.h>
#include <thrift/lib/cpp2/async/ClientStreamBridge.h>
#include <thrift/lib/cpp2/async/StreamCallbacks.h>
#include <thrift/lib/cpp2/async/StreamPayload.h>

#include "hphp/runtime/base/array-init.h"
#include "hphp/runtime/base/comparisons.h"
#include "hphp/runtime/base/type-array.h"
#include "hphp/runtime/base/type-object.h"
#include "hphp/runtime/ext/asio/ext_asio.h"
#include "hphp/runtime/ext/asio/ext_external-thread-event-wait-handle.h"
#include "hphp/runtime/ext/asio/ext_wait-handle.h"
#include "hphp/runtime/ext/thrift/ext_thrift.h"
#include "hphp/system/systemlib.h"

namespace HPHP {
namespace {

struct FirstResponseCallback final
  : apache::thrift::detail::ClientStreamBridge::FirstResponseCallback {
  void onFirstResponse(
    apache::thrift::FirstResponsePayload&&,
    apache::thrift::detail::ClientStreamBridge::ClientPtr stream
  ) override {
    ptr = std::move(stream);
  }

  void onFirstResponseError(folly::exception_wrapper) override {
    std::terminate();
  }

  apache::thrift::detail::ClientStreamBridge::ClientPtr ptr;
};

struct ServerCallback final : apache::thrift::StreamServerCallback {
  bool onStreamRequestN(int32_t n) override {
    credits += n;
    return true;
  }

  void onStreamCancel() override {
    canceled = true;
  }

  void resetClientCallback(apache::thrift::StreamClientCallback&) override {
    std::terminate();
  }

  int32_t credits = 0;
  bool canceled = false;
};

struct ThriftStreamTest : testing::Test {
  void SetUp() override {
    client = apache::thrift::detail::ClientStreamBridge::create(&firstResponse);
    std::ignore = client->onFirstResponse(
      {nullptr, {}}, eventBase.getEventBase(), &server
    );
    object = thrift::TClientBufferedStream::newInstance();
    stream = thrift::TClientBufferedStream::GetDataOrThrowException(object.get());
    stream->init(std::move(firstResponse.ptr), {8, 0});
  }

  void TearDown() override {
    object = Object{};
    // Flush cancellation before destroying the server callback.
    eventBase.getEventBase()->runInEventBaseThreadAndWait([] {});
  }

  Object genNext() {
    return thrift::HHVM_MN(TClientBufferedStream, genNext)(object.get());
  }

  folly::ScopedEventBaseThread eventBase;
  FirstResponseCallback firstResponse;
  ServerCallback server;
  apache::thrift::StreamClientCallback* client = nullptr;
  Object object;
  thrift::TClientBufferedStream* stream = nullptr;
};

struct ThriftStreamHeaderTest : ThriftStreamTest,
                              testing::WithParamInterface<bool> {};

TEST_P(ThriftStreamHeaderTest, HeaderPreservesOpenStream) {
  Object pending;
  if (!GetParam()) pending = genNext();
  eventBase.getEventBase()->runInEventBaseThreadAndWait([&] {
    EXPECT_TRUE(client->onStreamHeaders({{}, {}}));
  });
  if (GetParam()) {
    stream->queue_ = stream->streamBridge_->getMessages();
    pending = genNext();
  }

  // With no payload or completion, an empty batch must keep the stream open.
  auto const result = HHVM_FN(join)(pending).toArray();
  ASSERT_TRUE(result[0].isArray());
  EXPECT_TRUE(result[0].toArray().empty());
  EXPECT_TRUE(result[1].isNull());
  EXPECT_TRUE(stream->streamBridge_);
  EXPECT_EQ(8, stream->outstanding_);
}

INSTANTIATE_TEST_SUITE_P(BufferedAndWaiting, ThriftStreamHeaderTest,
                        testing::Bool());

TEST_F(ThriftStreamTest, OrderedHeadersReplenishCredits) {
  // Exceed the initial credit budget using only ordered headers.
  for (int batch = 0; batch < 3; ++batch) {
    eventBase.getEventBase()->runInEventBaseThreadAndWait([&] {
      for (int i = 0; i < 4; ++i) {
        std::ignore = client->onStreamNext(
          {folly::IOBuf::create(0), {}, true}
        );
      }
    });
    auto pending = genNext();
    // Wait for queued RequestN callbacks before reading server.credits.
    eventBase.getEventBase()->runInEventBaseThreadAndWait([] {});
    auto const result = HHVM_FN(join)(pending).toArray();
    ASSERT_TRUE(result[0].isArray());
    EXPECT_TRUE(result[0].toArray().empty());
    EXPECT_TRUE(result[1].isNull());
    EXPECT_EQ(4, stream->outstanding_);
    EXPECT_EQ(batch * 4, server.credits);
  }

  // The refill threshold leaves a header and completion for the next drain.
  eventBase.getEventBase()->runInEventBaseThreadAndWait([&] {
    std::ignore = client->onStreamNext({folly::IOBuf::copyBuffer("A"), {}});
    EXPECT_TRUE(client->onStreamHeaders({{}, {}}));
    std::ignore = client->onStreamNext({folly::IOBuf::create(0), {}, true});
    std::ignore = client->onStreamNext({folly::IOBuf::copyBuffer("B"), {}});
    std::ignore = client->onStreamNext({folly::IOBuf::copyBuffer("C"), {}});
    EXPECT_TRUE(client->onStreamHeaders({{}, {}}));
    client->onStreamComplete();
  });
  auto result = HHVM_FN(join)(genNext()).toArray();
  EXPECT_TRUE(same(result[0], Variant{make_vec_array("A", "B", "C")}));
  EXPECT_TRUE(result[1].isNull());
  EXPECT_EQ(4, stream->outstanding_);
  EXPECT_TRUE(stream->streamBridge_);

  result = HHVM_FN(join)(genNext()).toArray();
  EXPECT_TRUE(result[0].isNull());
  EXPECT_TRUE(result[1].isNull());
  EXPECT_FALSE(stream->streamBridge_);
  EXPECT_FALSE(genNext());
}

TEST_F(ThriftStreamTest, EmptyCompletion) {
  auto pending = genNext();
  eventBase.getEventBase()->runInEventBaseThreadAndWait([&] {
    client->onStreamComplete();
  });
  auto const result = HHVM_FN(join)(pending).toArray();
  EXPECT_TRUE(result[0].isNull());
  EXPECT_TRUE(result[1].isNull());
  EXPECT_FALSE(genNext());
}

TEST_F(ThriftStreamTest, PayloadBeforeError) {
  eventBase.getEventBase()->runInEventBaseThreadAndWait([&] {
    EXPECT_TRUE(client->onStreamHeaders({{}, {}}));
    std::ignore = client->onStreamNext({folly::IOBuf::copyBuffer("value"), {}});
    client->onStreamError(
      folly::make_exception_wrapper<std::runtime_error>("stream error")
    );
  });
  auto const result = HHVM_FN(join)(genNext()).toArray();
  ASSERT_TRUE(result[0].isArray());
  auto const values = result[0].toArray();
  ASSERT_EQ(1, values.size());
  EXPECT_EQ("value", values[0].toString().toCppString());
  ASSERT_TRUE(result[1].isString());
  EXPECT_NE(std::string::npos,
            result[1].toString().toCppString().find("stream error"));
  EXPECT_FALSE(genNext());
}

TEST_F(ThriftStreamTest, PayloadBeforeEncodedError) {
  eventBase.getEventBase()->runInEventBaseThreadAndWait([&] {
    EXPECT_TRUE(client->onStreamHeaders({{}, {}}));
    std::ignore = client->onStreamNext({folly::IOBuf::copyBuffer("value"), {}});
    client->onStreamError(
      folly::make_exception_wrapper<apache::thrift::detail::EncodedError>(
        folly::IOBuf::copyBuffer("encoded error")
      )
    );
  });
  auto const result = HHVM_FN(join)(genNext()).toArray();
  ASSERT_TRUE(result[0].isArray());
  auto const values = result[0].toArray();
  ASSERT_EQ(2, values.size());
  EXPECT_EQ("value", values[0].toString().toCppString());
  EXPECT_EQ("encoded error", values[1].toString().toCppString());
  EXPECT_TRUE(result[1].isNull());
  EXPECT_FALSE(genNext());
}

TEST_F(ThriftStreamTest, CancelPendingRead) {
  auto pending = genNext();
  auto exception = SystemLib::AllocInvalidOperationExceptionObject("cancelled");
  auto handle = wait_handle<c_ExternalThreadEventWaitHandle>(pending.get());
  ASSERT_TRUE(handle->cancel(exception));

  object = Object{};
  eventBase.getEventBase()->runInEventBaseThreadAndWait([] {});
  EXPECT_TRUE(server.canceled);
  try {
    HHVM_FN(join)(pending);
    FAIL() << "Cancelled read succeeded";
  } catch (const Object& thrown) {
    EXPECT_EQ(exception.get(), thrown.get());
  }
}

}
}
