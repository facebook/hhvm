/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <exception>

#include <gtest/gtest.h>
#include <folly/coro/BlockingWait.h>
#include <folly/coro/Coroutine.h>
#include <folly/io/async/ScopedEventBaseThread.h>

#include <thrift/lib/cpp2/test/gen-cpp2/Coroutine.h>
#include <thrift/lib/cpp2/util/ScopedServerInterfaceThread.h>

using apache::thrift::Cpp2RequestContext;
using apache::thrift::RequestParams;
using apache::thrift::ScopedServerInterfaceThread;
using apache::thrift::concurrency::ThreadManager;
using folly::EventBase;

using apache::thrift::test::Coroutine;
using apache::thrift::test::CoroutineAsyncClient;
using apache::thrift::test::CoroutineSvNull;
using apache::thrift::test::Ex;
using apache::thrift::test::SumRequest;
using apache::thrift::test::SumResponse;

const static int kNoParameterReturnValue = 123;
static int voidReturnValue;

#if FOLLY_HAS_COROUTINES

class CoroutineServiceHandlerCoro
    : virtual public apache::thrift::ServiceHandler<Coroutine> {
 public:
  void computeSumNoCoro(
      SumResponse& response, std::unique_ptr<SumRequest> request) override {
    *response.sum_ref() = *request->x_ref() + *request->y_ref();
  }

  folly::coro::Task<std::unique_ptr<SumResponse>> co_computeSum(
      std::unique_ptr<SumRequest> request) override {
    auto response = std::make_unique<SumResponse>();
    *response->sum_ref() = *request->x_ref() + *request->y_ref();
    co_return response;
  }

  folly::coro::Task<int32_t> co_computeSumPrimitive(
      int32_t x, int32_t y) override {
    co_return x + y;
  }

  folly::coro::Task<void> co_computeSumVoid(int32_t x, int32_t y) override {
    voidReturnValue = x + y;
    co_return;
  }

  folly::coro::Task<std::unique_ptr<SumResponse>> co_computeSumThrows(
      std::unique_ptr<SumRequest> /* request */) override {
    co_await folly::coro::suspend_never{};
    throw std::runtime_error("Not implemented");
  }

  folly::coro::Task<int32_t> co_computeSumThrowsPrimitive(
      int32_t, int32_t) override {
    co_await folly::coro::suspend_never{};
    throw std::runtime_error("Not implemented");
  }

  folly::coro::Task<int32_t> co_noParameters() override {
    co_return kNoParameterReturnValue;
  }

  folly::Future<std::unique_ptr<SumResponse>> future_implementedWithFutures()
      override {
    auto result = std::make_unique<SumResponse>();
    *result->sum_ref() = kNoParameterReturnValue;
    return folly::makeFuture(std::move(result));
  }

  folly::Future<int32_t> future_implementedWithFuturesPrimitive() override {
    return folly::makeFuture(kNoParameterReturnValue);
  }

  folly::coro::Task<int32_t> co_takesRequestParams(
      RequestParams params) override {
    Cpp2RequestContext* requestContext = params.getRequestContext();
    folly::Executor* handlerExecutor = params.getHandlerExecutor();
    EventBase* eventBase = params.getEventBase();
    // It's hard to check that these pointers are what we expect them to be; we
    // can at least make sure that they point to valid memory, though.
    *(volatile char*)requestContext;
    *(volatile char*)handlerExecutor;
    *(volatile char*)eventBase;
    co_return 0;
  }

  folly::coro::Task<void> co_onewayRequest(int32_t x) override {
    onewayRequestPromise.setValue(x);
    co_return;
  }

  folly::coro::Task<std::unique_ptr<SumResponse>> co_computeSumThrowsUserEx(
      std::unique_ptr<SumRequest>) override {
    throw Ex();
  }

  folly::coro::Task<int32_t> co_computeSumThrowsUserExPrimitive(
      int32_t, int32_t) override {
    throw Ex();
  }

  folly::Promise<int32_t> onewayRequestPromise;
};

#endif

class CoroutineServiceHandlerFuture
    : virtual public apache::thrift::ServiceHandler<Coroutine> {
 public:
  void computeSumNoCoro(
      SumResponse& response, std::unique_ptr<SumRequest> request) override {
    *response.sum_ref() = *request->x_ref() + *request->y_ref();
  }

  folly::Future<std::unique_ptr<SumResponse>> future_computeSum(
      std::unique_ptr<SumRequest> request) override {
    auto response = std::make_unique<SumResponse>();
    *response->sum_ref() = *request->x_ref() + *request->y_ref();
    return folly::makeFuture(std::move(response));
  }

  folly::Future<int32_t> future_computeSumPrimitive(
      int32_t x, int32_t y) override {
    return folly::makeFuture(x + y);
  }

  folly::Future<folly::Unit> future_computeSumVoid(
      int32_t x, int32_t y) override {
    voidReturnValue = x + y;
    return folly::makeFuture(folly::Unit{});
  }

  folly::Future<std::unique_ptr<SumResponse>> future_computeSumThrows(
      std::unique_ptr<SumRequest> /* request */) override {
    return folly::makeFuture<std::unique_ptr<SumResponse>>(
        folly::exception_wrapper(
            std::in_place, std::runtime_error("Not implemented")));
  }

  folly::Future<int32_t> future_computeSumThrowsPrimitive(
      int32_t, int32_t) override {
    return folly::makeFuture<int32_t>(folly::exception_wrapper(
        std::in_place, std::runtime_error("Not implemented")));
  }

  folly::Future<int32_t> future_noParameters() override {
    return folly::makeFuture(kNoParameterReturnValue);
  }

  folly::Future<std::unique_ptr<SumResponse>> future_implementedWithFutures()
      override {
    auto result = std::make_unique<SumResponse>();
    *result->sum_ref() = kNoParameterReturnValue;
    return folly::makeFuture(std::move(result));
  }

  folly::Future<int32_t> future_implementedWithFuturesPrimitive() override {
    return folly::makeFuture(kNoParameterReturnValue);
  }

  folly::Future<int32_t> future_takesRequestParams() override {
    // Future functionality is tested elsewhere; we only need this method to
    // make the code compile.
    return folly::makeFuture(0);
  }

  folly::Future<folly::Unit> future_onewayRequest(int32_t x) override {
    onewayRequestPromise.setValue(x);
    return folly::makeFuture(folly::Unit());
  }

  folly::Future<std::unique_ptr<SumResponse>> future_computeSumThrowsUserEx(
      std::unique_ptr<SumRequest> /* request */) override {
    return folly::makeFuture<std::unique_ptr<SumResponse>>(
        folly::exception_wrapper(std::in_place, Ex()));
  }

  folly::Future<int32_t> future_computeSumThrowsUserExPrimitive(
      int32_t, int32_t) override {
    return folly::makeFuture<int32_t>(
        folly::exception_wrapper(std::in_place, Ex()));
  }

  folly::Promise<int32_t> onewayRequestPromise;
};

template <class Handler>
class CoroutineTest : public testing::Test {
 public:
  CoroutineTest()
      : handler_(std::make_shared<Handler>()),
        ssit_(handler_),
        client_(ssit_.newClient<CoroutineAsyncClient>()) {}

 protected:
  template <typename Func>
  void expectSumResults(Func computeSum) {
    for (int i = 0; i < 10; ++i) {
      for (int j = 0; j < 10; ++j) {
        EXPECT_EQ(i + j, computeSum(i, j));
      }
    }
  }
  std::shared_ptr<Handler> handler_;
  ScopedServerInterfaceThread ssit_;
  std::unique_ptr<CoroutineAsyncClient> client_;
};

TYPED_TEST_CASE_P(CoroutineTest);

TYPED_TEST_P(CoroutineTest, SumNoCoro) {
  this->expectSumResults([&](int x, int y) {
    SumRequest request;
    *request.x_ref() = x;
    *request.y_ref() = y;
    SumResponse response;
    this->client_->sync_computeSumNoCoro(response, request);
    return *response.sum_ref();
  });
}

TYPED_TEST_P(CoroutineTest, Sum) {
  this->expectSumResults([&](int x, int y) {
    SumRequest request;
    *request.x_ref() = x;
    *request.y_ref() = y;
    SumResponse response;
    this->client_->sync_computeSum(response, request);
    return *response.sum_ref();
  });
}

TYPED_TEST_P(CoroutineTest, SumPrimitive) {
  this->expectSumResults([&](int x, int y) {
    return this->client_->sync_computeSumPrimitive(x, y);
  });
}

TYPED_TEST_P(CoroutineTest, SumVoid) {
  this->expectSumResults([&](int x, int y) {
    this->client_->sync_computeSumVoid(x, y);
    return voidReturnValue;
  });
}

TYPED_TEST_P(CoroutineTest, SumUnimplemented) {
  for (int i = 0; i < 10; ++i) {
    bool error = false;
    try {
      SumRequest request;
      *request.x_ref() = i;
      *request.y_ref() = i;
      SumResponse response;
      this->client_->sync_computeSumUnimplemented(response, request);
    } catch (...) {
      error = true;
    }
    EXPECT_TRUE(error);
  }
  this->expectSumResults([&](int x, int y) {
    return this->client_->sync_computeSumPrimitive(x, y);
  });
}

TYPED_TEST_P(CoroutineTest, SumUnimplementedPrimitive) {
  for (int i = 0; i < 10; ++i) {
    bool error = false;
    try {
      this->client_->sync_computeSumUnimplementedPrimitive(i, i);
    } catch (...) {
      error = true;
    }
    EXPECT_TRUE(error);
  }
  this->expectSumResults([&](int x, int y) {
    return this->client_->sync_computeSumPrimitive(x, y);
  });
}

TYPED_TEST_P(CoroutineTest, SumThrows) {
  for (int i = 0; i < 10; ++i) {
    bool error = false;
    try {
      SumRequest request;
      *request.x_ref() = i;
      *request.y_ref() = i;
      SumResponse response;
      this->client_->sync_computeSumThrows(response, request);
    } catch (...) {
      error = true;
    }
    EXPECT_TRUE(error);
  }
  this->expectSumResults([&](int x, int y) {
    return this->client_->sync_computeSumPrimitive(x, y);
  });
}

TYPED_TEST_P(CoroutineTest, SumThrowsPrimitive) {
  for (int i = 0; i < 10; ++i) {
    bool error = false;
    try {
      this->client_->sync_computeSumThrowsPrimitive(i, i);
    } catch (...) {
      error = true;
    }
    EXPECT_TRUE(error);
  }
  this->expectSumResults([&](int x, int y) {
    return this->client_->sync_computeSumPrimitive(x, y);
  });
}

TYPED_TEST_P(CoroutineTest, NoParameters) {
  EXPECT_EQ(kNoParameterReturnValue, this->client_->sync_noParameters());
  EXPECT_EQ(kNoParameterReturnValue, this->client_->sync_noParameters());
  EXPECT_EQ(kNoParameterReturnValue, this->client_->sync_noParameters());
}

TYPED_TEST_P(CoroutineTest, ImplemetedWithFutures) {
  SumResponse response;
  *response.sum_ref() = 0;
  this->client_->sync_implementedWithFutures(response);
  EXPECT_EQ(kNoParameterReturnValue, *response.sum_ref());

  *response.sum_ref() = 0;
  this->client_->sync_implementedWithFutures(response);
  EXPECT_EQ(kNoParameterReturnValue, *response.sum_ref());

  *response.sum_ref() = 0;
  this->client_->sync_implementedWithFutures(response);
  EXPECT_EQ(kNoParameterReturnValue, *response.sum_ref());
}

TYPED_TEST_P(CoroutineTest, ImplemetedWithFuturesPrimitive) {
  EXPECT_EQ(
      kNoParameterReturnValue,
      this->client_->sync_implementedWithFuturesPrimitive());
  EXPECT_EQ(
      kNoParameterReturnValue,
      this->client_->sync_implementedWithFuturesPrimitive());
  EXPECT_EQ(
      kNoParameterReturnValue,
      this->client_->sync_implementedWithFuturesPrimitive());
}

TYPED_TEST_P(CoroutineTest, Oneway) {
  auto f = this->handler_->onewayRequestPromise.getSemiFuture();
  this->client_->sync_onewayRequest(35);
  EXPECT_EQ(35, std::move(f).via(&folly::InlineExecutor::instance()).get());
}

TYPED_TEST_P(CoroutineTest, TakesRequestParams) {
  this->client_->sync_takesRequestParams();
}

TYPED_TEST_P(CoroutineTest, SumThrowsUserEx) {
  for (int i = 0; i < 10; ++i) {
    bool error = false;
    try {
      SumRequest request;
      *request.x_ref() = i;
      *request.y_ref() = i;
      SumResponse response;
      this->client_->sync_computeSumThrowsUserEx(response, request);
    } catch (const Ex&) {
      error = true;
    }
    EXPECT_TRUE(error);
  }
}

TYPED_TEST_P(CoroutineTest, SumThrowsUserExPrimitive) {
  for (int i = 0; i < 10; ++i) {
    bool error = false;
    try {
      this->client_->sync_computeSumThrowsUserExPrimitive(i, i);
    } catch (const Ex&) {
      error = true;
    }
    EXPECT_TRUE(error);
  }
}

REGISTER_TYPED_TEST_CASE_P(
    CoroutineTest,
    SumNoCoro,
    Sum,
    SumPrimitive,
    SumVoid,
    SumUnimplemented,
    SumUnimplementedPrimitive,
    SumThrows,
    SumThrowsPrimitive,
    NoParameters,
    ImplemetedWithFutures,
    ImplemetedWithFuturesPrimitive,
    Oneway,
    TakesRequestParams,
    SumThrowsUserEx,
    SumThrowsUserExPrimitive);

INSTANTIATE_TYPED_TEST_CASE_P(
    CoroutineTest,
    CoroutineTest,
    decltype(testing::Types<
#if FOLLY_HAS_COROUTINES
             CoroutineServiceHandlerCoro,
#endif
             CoroutineServiceHandlerFuture>{}));

class CoroutineNullTest : public testing::Test {
 public:
  CoroutineNullTest()
      : ssit_(std::make_shared<CoroutineSvNull>()),
        client_(ssit_.newClient<CoroutineAsyncClient>()) {}
  ScopedServerInterfaceThread ssit_;
  std::unique_ptr<CoroutineAsyncClient> client_;
};

TEST_F(CoroutineNullTest, Basics) {
  SumRequest request;
  *request.x_ref() = 123;
  *request.y_ref() = 123;

  SumResponse response;

  *response.sum_ref() = 123;
  client_->sync_computeSumNoCoro(response, request);
  EXPECT_EQ(0, *response.sum_ref());

  *response.sum_ref() = 123;
  client_->sync_computeSum(response, request);
  EXPECT_EQ(0, *response.sum_ref());

  EXPECT_EQ(0, client_->sync_computeSumPrimitive(123, 456));

  client_->sync_computeSumVoid(123, 456);

  client_->sync_noParameters();
}

#if FOLLY_HAS_COROUTINES
class CoroutineClientTest : public testing::Test {
 protected:
  CoroutineClientTest()
      : ssit_(std::make_shared<CoroutineServiceHandlerCoro>()),
        client_(ssit_.newClient<CoroutineAsyncClient>()) {}

  ScopedServerInterfaceThread ssit_;
  EventBase eventBase_;
  std::unique_ptr<CoroutineAsyncClient> client_;
};

TEST_F(CoroutineClientTest, SumCoroClient) {
  SumRequest request;
  *request.x_ref() = 123;
  *request.y_ref() = 123;

  client_->co_computeSum(request)
      .semi()
      .via(&eventBase_)
      .then([&](folly::Try<SumResponse> response) {
        EXPECT_EQ(246, *response->sum_ref());
      })
      .getVia(&eventBase_);
}

TEST_F(CoroutineClientTest, SumPrimitiveCoroClient) {
  client_->co_computeSumPrimitive(12, 408)
      .semi()
      .via(&eventBase_)
      .then([&](folly::Try<int32_t> result) { EXPECT_EQ(420, *result); })
      .getVia(&eventBase_);
}

TEST_F(CoroutineClientTest, SumVoidCoroClient) {
  client_->co_computeSumVoid(11, 22)
      .semi()
      .via(&eventBase_)
      .then([&](folly::Try<folly::Unit>) { EXPECT_EQ(33, voidReturnValue); })
      .getVia(&eventBase_);
}

TEST_F(CoroutineClientTest, SumUnimplementedCoroClient) {
  SumRequest request;
  *request.x_ref() = 43;
  *request.y_ref() = 179;
  client_->co_computeSumUnimplemented(request)
      .semi()
      .via(&eventBase_)
      .then([&](folly::Try<SumResponse> response) {
        EXPECT_THROW(
            response.throwUnlessValue(), apache::thrift::TApplicationException);
      })
      .getVia(&eventBase_);
}

TEST_F(CoroutineClientTest, SumUnimplementedPrimitiveCoroClient) {
  client_->co_computeSumUnimplementedPrimitive(12, 34)
      .semi()
      .via(&eventBase_)
      .then([&](folly::Try<int32_t> response) {
        EXPECT_THROW(
            response.throwUnlessValue(), apache::thrift::TApplicationException);
      })
      .getVia(&eventBase_);
}

TEST_F(CoroutineClientTest, SumThrowsCoroClient) {
  SumRequest request;
  *request.x_ref() = 290;
  *request.y_ref() = 321;
  client_->co_computeSumThrows(request)
      .semi()
      .via(&eventBase_)
      .then([&](folly::Try<SumResponse> response) {
        EXPECT_THROW(response.throwUnlessValue(), std::exception);
      })
      .getVia(&eventBase_);
}

TEST_F(CoroutineClientTest, SumThrowsPrimitiveCoroClient) {
  client_->co_computeSumThrowsPrimitive(523, 8103)
      .semi()
      .via(&eventBase_)
      .then([&](folly::Try<int32_t> response) {
        EXPECT_THROW(response.throwUnlessValue(), std::exception);
      })
      .getVia(&eventBase_);
}

TEST_F(CoroutineClientTest, noParametersCoroClient) {
  client_->co_noParameters()
      .semi()
      .via(&eventBase_)
      .then([&](folly::Try<int32_t> result) {
        EXPECT_EQ(kNoParameterReturnValue, *result);
      })
      .getVia(&eventBase_);
}

TEST_F(CoroutineClientTest, implementedWithFuturesCoroClient) {
  client_->co_implementedWithFutures()
      .semi()
      .via(&eventBase_)
      .then([&](folly::Try<SumResponse> response) {
        EXPECT_EQ(kNoParameterReturnValue, *response->sum_ref());
      })
      .getVia(&eventBase_);
}

TEST_F(CoroutineClientTest, implementedWitFuturesPrimitiveCoroClient) {
  client_->co_implementedWithFuturesPrimitive()
      .semi()
      .via(&eventBase_)
      .then([&](folly::Try<int32_t> result) {
        EXPECT_EQ(kNoParameterReturnValue, *result);
      })
      .getVia(&eventBase_);
}

TEST_F(CoroutineClientTest, takesRequestParamsCoroClient) {
  client_->co_takesRequestParams()
      .semi()
      .via(&eventBase_)
      .then([&](folly::Try<int32_t> result) { EXPECT_EQ(0, *result); })
      .getVia(&eventBase_);
}

TEST_F(CoroutineClientTest, rpcOptionsCoroClient) {
  apache::thrift::RpcOptions opts;
  client_->co_computeSumPrimitive(opts, 12, 408)
      .semi()
      .via(&eventBase_)
      .then([&](folly::Try<int32_t> result) { EXPECT_EQ(420, *result); })
      .getVia(&eventBase_);
}

TEST_F(CoroutineClientTest, rpcOptionsCancellableCoroClient) {
  folly::CancellationSource source;
  apache::thrift::RpcOptions opts;
  folly::coro::co_withCancellation(
      source.getToken(), client_->co_computeSumPrimitive(opts, 12, 408))
      .semi()
      .via(&eventBase_)
      .then([&](folly::Try<int32_t> result) { EXPECT_EQ(420, *result); })
      .getVia(&eventBase_);
}

TEST_F(CoroutineClientTest, cancellableCoroClient) {
  folly::CancellationSource source;
  folly::coro::co_withCancellation(
      source.getToken(), client_->co_computeSumPrimitive(12, 408))
      .semi()
      .via(&eventBase_)
      .then([&](folly::Try<int32_t> result) { EXPECT_EQ(420, *result); })
      .getVia(&eventBase_);
}

TEST_F(CoroutineClientTest, cancelCoroClient) {
  folly::CancellationSource source;
  source.requestCancellation();
  folly::coro::co_withCancellation(
      source.getToken(), client_->co_computeSumPrimitive(12, 408))
      .semi()
      .via(&eventBase_)
      .then([&](folly::Try<int32_t> result) {
        EXPECT_TRUE(result.hasException<folly::OperationCancelled>());
      })
      .getVia(&eventBase_);
}

TEST(CoroutineExceptionTest, completesHandlerCallback) {
  class CoroutineServiceHandlerThrowing
      : virtual public apache::thrift::ServiceHandler<Coroutine> {
   public:
    folly::coro::Task<std::unique_ptr<SumResponse>> co_computeSumThrows(
        std::unique_ptr<SumRequest> /* request */) override {
      throw std::runtime_error("Not implemented");
    }

    folly::coro::Task<int32_t> co_computeSumThrowsPrimitive(
        int32_t, int32_t) override {
      throw std::runtime_error("Not implemented");
    }

    folly::coro::Task<void> co_onewayRequest(int32_t) override {
      throw std::runtime_error("Not implemented");
    }
  };

  CoroutineServiceHandlerThrowing handler;

  folly::ScopedEventBaseThread ebt;
  auto tm = ThreadManager::newSimpleThreadManager(1);

  apache::thrift::Cpp2RequestContext cpp2reqCtx(nullptr);
  auto cb = std::make_unique<
      apache::thrift::HandlerCallback<std::unique_ptr<SumResponse>>>(
      nullptr,
      nullptr,
      apache::thrift::HandlerCallbackBase::MethodNameInfo{"", "", "", ""},
      nullptr,
      nullptr,
      0,
      ebt.getEventBase(),
      tm.get(),
      &cpp2reqCtx);
  handler.async_tm_computeSumThrows(std::move(cb), nullptr);

  auto cb2 = std::make_unique<apache::thrift::HandlerCallback<int32_t>>(
      nullptr,
      nullptr,
      apache::thrift::HandlerCallbackBase::MethodNameInfo{"", "", "", ""},
      nullptr,
      nullptr,
      0,
      ebt.getEventBase(),
      tm.get(),
      &cpp2reqCtx);
  handler.async_tm_computeSumThrowsPrimitive(std::move(cb2), 0, 0);

  auto cb3 = std::make_unique<apache::thrift::HandlerCallbackOneWay>(
      nullptr,
      nullptr,
      apache::thrift::HandlerCallbackBase::MethodNameInfo{"", "", "", ""},
      nullptr,
      ebt.getEventBase(),
      tm.get(),
      &cpp2reqCtx);
  handler.async_tm_onewayRequest(std::move(cb3), 0);
}

TEST(CoroutineHeaderTest, customHeaderTest) {
  class CoroHandler : virtual public apache::thrift::ServiceHandler<Coroutine> {
   public:
    folly::coro::Task<std::unique_ptr<::apache::thrift::test::SumResponse>>
    co_computeSum(
        apache::thrift::RequestParams params,
        std::unique_ptr<::apache::thrift::test::SumRequest> request) override {
      if (folly::get_ptr(params.getRequestContext()->getHeaders(), "foo")) {
        auto header = params.getRequestContext()->getHeader();
        if (header) {
          header->setHeader("header_from_server", "1");
        }
      }
      auto response = std::make_unique<SumResponse>();
      response->sum_ref() = *request->x_ref() + *request->y_ref();
      co_return response;
    }
  };

  std::shared_ptr<CoroHandler> handler = std::make_shared<CoroHandler>();
  ScopedServerInterfaceThread ssit{handler};
  auto client = ssit.newClient<CoroutineAsyncClient>();

  apache::thrift::RpcOptions rpcOptions;
  rpcOptions.setWriteHeader("foo", "bar");

  SumRequest sumRequest;
  sumRequest.x_ref() = 42;
  sumRequest.y_ref() = 123;
  auto result =
      folly::coro::blockingWait(client->co_computeSum(rpcOptions, sumRequest));
  auto ptr = folly::get_ptr(rpcOptions.getReadHeaders(), "header_from_server");
  EXPECT_NE(nullptr, ptr);
  EXPECT_EQ("1", *ptr);
  EXPECT_EQ(165, *result.sum_ref());
}
#endif
