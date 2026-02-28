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

#include <thrift/lib/cpp2/test/gen-cpp2/Bug.h>
#include <thrift/lib/cpp2/test/gen-cpp2/Raiser.h>
#include <thrift/lib/cpp2/util/ScopedServerInterfaceThread.h>

#include <fmt/core.h>
#include <gtest/gtest.h>
#include <folly/coro/BlockingWait.h>

using namespace std;
using namespace folly;
using namespace apache::thrift;
using namespace apache::thrift::test;

class lulz : public exception {
 public:
  explicit lulz(string message) noexcept : message_(std::move(message)) {}
  const char* what() const noexcept override { return message_.c_str(); }

 private:
  string message_;
};

namespace {

using AppExn = TApplicationException;

struct Context {
  string name;
  string ex_type;
  string ex_what;
  exception_wrapper ew;
  explicit Context(string name) : name(name) {}
};

class ExceptionTrackingEventHandler : public TProcessorEventHandler {
 public:
  explicit ExceptionTrackingEventHandler(
      std::vector<std::shared_ptr<Context>>* contexts) {
    contexts_ = contexts;
  }
  void* getContext(std::string_view fn_name, TConnectionContext*) override {
    contexts_->push_back(std::make_shared<Context>(std::string(fn_name)));
    return contexts_->back().get();
  }
  void userException(
      void* ctx,
      std::string_view fn_name,
      std::string_view ex,
      std::string_view ex_what) override {
    auto context = static_cast<Context*>(ctx);
    CHECK_EQ(context->name, fn_name);
    context->ex_type = std::string(ex);
    context->ex_what = std::string(ex_what);
  }
  void userExceptionWrapped(
      void* ctx,
      std::string_view fn_name,
      bool declared,
      const folly::exception_wrapper& ew) override {
    TProcessorEventHandler::userExceptionWrapped(ctx, fn_name, declared, ew);
    auto context = static_cast<Context*>(ctx);
    context->ew = ew;
  }

 private:
  std::vector<std::shared_ptr<Context>>* contexts_;
};

class RaiserHandler : public apache::thrift::ServiceHandler<Raiser> {
 public:
  RaiserHandler(
      vector<shared_ptr<TProcessorEventHandler>> handlers,
      Function<exception_ptr()> go)
      : handlers_(std::move(handlers)), go_(wrap(std::move(go))) {}
  RaiserHandler(
      vector<shared_ptr<TProcessorEventHandler>> handlers,
      Function<exception_wrapper()> go)
      : handlers_(std::move(handlers)), go_(wrap(std::move(go))) {}

  unique_ptr<AsyncProcessor> getProcessor() override {
    auto processor = apache::thrift::ServiceHandler<Raiser>::getProcessor();
    for (auto handler : handlers_) {
      processor->addEventHandler(handler);
    }
    return processor;
  }

 protected:
  void async_tm_doBland(HandlerCallbackPtr<void> cb) override {
    go_(std::move(cb));
  }
  void async_tm_doRaise(HandlerCallbackPtr<void> cb) override {
    go_(std::move(cb));
  }
  void async_tm_get200(HandlerCallbackPtr<string> cb) override {
    go_(std::move(cb));
  }
  void async_tm_get500(HandlerCallbackPtr<string> cb) override {
    go_(std::move(cb));
  }

  template <typename E>
  Function<void(HandlerCallbackBase::Ptr)> wrap(E e) {
    return [e = std::move(e)](HandlerCallbackBase::Ptr cb) mutable {
      cb->exception(e());
    };
  }

 private:
  vector<shared_ptr<TProcessorEventHandler>> handlers_;
  Function<void(HandlerCallbackBase::Ptr)> go_;
};

class BugServiceHandler : public apache::thrift::ServiceHandler<Bug> {
 public:
  BugServiceHandler() {}

  folly::coro::Task<void> co_fun1() override { throw Start(); }

  folly::coro::Task<void> co_fun2() override { throw FirstBlood(); }

  folly::coro::Task<void> co_fun3() override { throw DoubleKill(); }

  folly::coro::Task<void> co_fun4() override { throw TripleKill(); }

  folly::coro::Task<void> co_fun5() override {
    try {
      throw overrideExceptionMetadata(TripleKill());
    } catch (const TripleKill&) {
      throw;
    } catch (...) {
      LOG(FATAL) << "Should be caught by the catch block above";
    }
  }

  folly::coro::Task<void> co_fun6() override {
    try {
      throw overrideExceptionMetadata(TripleKill()).setServer().setTransient();
    } catch (const TripleKill&) {
      throw;
    } catch (...) {
      LOG(FATAL) << "Should be caught by the catch block above";
    }
  }
};

} // namespace

class ThriftServerExceptionTest : public testing::Test {
 public:
  EventBase eb;

  string message{"rofl"};
  vector<shared_ptr<Context>> contexts;
  shared_ptr<ExceptionTrackingEventHandler> evhandler{
      make_shared<ExceptionTrackingEventHandler>(&contexts)};
  vector<shared_ptr<TProcessorEventHandler>> evhandlers{
      static_pointer_cast<TProcessorEventHandler>(evhandler),
  };

  const Context& ctx() const { return *contexts.back(); }

  template <class E>
  std::exception_ptr to_eptr(const E& e) {
    try {
      throw e;
    } catch (E&) {
      return std::current_exception();
    }
  }

  template <class E>
  exception_wrapper to_wrap(const E& e) {
    return exception_wrapper(e); // just an alias
  }

  lulz make_lulz() const { return lulz(message); }
  Banal make_banal() const { return Banal(); }
  Fiery make_fiery() const {
    Fiery f;
    *f.message() = message;
    return f;
  }

  template <class V, class F>
  bool exn(Future<V> fv, F&& f) {
    exception_wrapper wrap = fv.waitVia(&eb).result().exception();
    return wrap.with_exception(std::forward<F>(f));
  }
};

TEST_F(ThriftServerExceptionTest, dummy_test) {
  auto handler = make_shared<BugServiceHandler>();
  ScopedServerInterfaceThread runner(handler);

  apache::thrift::RpcOptions rpcOptions;
  auto client = runner.newClient<BugAsyncClient>();

  try {
    folly::coro::blockingWait(client->co_fun1(rpcOptions));
  } catch (...) {
  }
  auto reader = rpcOptions.getReadHeaders();
  auto errorClass =
      apache::thrift::detail::deserializeErrorClassification(reader["exm"]);
  EXPECT_EQ(*errorClass.kind(), ErrorKind::UNSPECIFIED);
  EXPECT_EQ(*errorClass.blame(), ErrorBlame::UNSPECIFIED);
  EXPECT_EQ(*errorClass.safety(), ErrorSafety::UNSPECIFIED);

  try {
    folly::coro::blockingWait(client->co_fun2(rpcOptions));
  } catch (...) {
  }
  reader = rpcOptions.getReadHeaders();
  errorClass =
      apache::thrift::detail::deserializeErrorClassification(reader["exm"]);
  EXPECT_EQ(*errorClass.kind(), ErrorKind::UNSPECIFIED);
  EXPECT_EQ(*errorClass.blame(), ErrorBlame::UNSPECIFIED);
  EXPECT_EQ(*errorClass.safety(), ErrorSafety::SAFE);

  try {
    folly::coro::blockingWait(client->co_fun3(rpcOptions));
  } catch (...) {
  }
  reader = rpcOptions.getReadHeaders();
  errorClass =
      apache::thrift::detail::deserializeErrorClassification(reader["exm"]);
  EXPECT_EQ(*errorClass.kind(), ErrorKind::STATEFUL);
  EXPECT_EQ(*errorClass.blame(), ErrorBlame::CLIENT);
  EXPECT_EQ(*errorClass.safety(), ErrorSafety::UNSPECIFIED);

  try {
    folly::coro::blockingWait(client->co_fun4(rpcOptions));
  } catch (...) {
  }
  reader = rpcOptions.getReadHeaders();
  errorClass =
      apache::thrift::detail::deserializeErrorClassification(reader["exm"]);
  EXPECT_EQ(*errorClass.kind(), ErrorKind::PERMANENT);
  EXPECT_EQ(*errorClass.blame(), ErrorBlame::CLIENT);
  EXPECT_EQ(*errorClass.safety(), ErrorSafety::SAFE);

  try {
    folly::coro::blockingWait(client->co_fun5(rpcOptions));
  } catch (...) {
  }
  reader = rpcOptions.getReadHeaders();
  errorClass =
      apache::thrift::detail::deserializeErrorClassification(reader["exm"]);
  EXPECT_EQ(*errorClass.kind(), ErrorKind::PERMANENT);
  EXPECT_EQ(*errorClass.blame(), ErrorBlame::CLIENT);
  EXPECT_EQ(*errorClass.safety(), ErrorSafety::SAFE);
  EXPECT_EQ("apache::thrift::test::TripleKill", reader["uex"]);

  try {
    folly::coro::blockingWait(client->co_fun6(rpcOptions));
  } catch (...) {
  }
  reader = rpcOptions.getReadHeaders();
  errorClass =
      apache::thrift::detail::deserializeErrorClassification(reader["exm"]);
  EXPECT_EQ(*errorClass.kind(), ErrorKind::TRANSIENT);
  EXPECT_EQ(*errorClass.blame(), ErrorBlame::SERVER);
  EXPECT_EQ(*errorClass.safety(), ErrorSafety::SAFE);
  EXPECT_EQ("apache::thrift::test::TripleKill", reader["uex"]);
}

TEST_F(ThriftServerExceptionTest, bland_with_exception_ptr) {
  Function<exception_ptr()> go = [&] { return to_eptr(make_lulz()); };

  auto handler = make_shared<RaiserHandler>(evhandlers, std::move(go));
  ScopedServerInterfaceThread runner(handler);

  auto client = runner.newClient<RaiserAsyncClient>(&eb);

  auto lulz_s = string{"lulz"};
  auto lulz_w = fmt::format("lulz: {}", message);

  EXPECT_TRUE(exn(client->future_doBland(), [&](const AppExn& e) {
    EXPECT_EQ(AppExn::TApplicationExceptionType::UNKNOWN, e.getType());
    EXPECT_EQ(lulz_w, string(e.what()));
    EXPECT_EQ(lulz_s, ctx().ex_type);
    EXPECT_EQ(lulz_w, ctx().ex_what);
    EXPECT_TRUE(ctx().ew.is_compatible_with<lulz>());
  }));
  EXPECT_TRUE(exn(client->future_doRaise(), [&](const AppExn& e) {
    EXPECT_EQ(AppExn::TApplicationExceptionType::UNKNOWN, e.getType());
    EXPECT_EQ(lulz_w, string(e.what()));
    EXPECT_EQ(lulz_s, ctx().ex_type);
    EXPECT_EQ(lulz_w, ctx().ex_what);
    EXPECT_TRUE(ctx().ew.is_compatible_with<lulz>());
  }));
  EXPECT_TRUE(exn(client->future_get200(), [&](const AppExn& e) {
    EXPECT_EQ(AppExn::TApplicationExceptionType::UNKNOWN, e.getType());
    EXPECT_EQ(lulz_w, string(e.what()));
    EXPECT_EQ(lulz_s, ctx().ex_type);
    EXPECT_EQ(lulz_w, ctx().ex_what);
    EXPECT_TRUE(ctx().ew.is_compatible_with<lulz>());
  }));
  EXPECT_TRUE(exn(client->future_get500(), [&](const AppExn& e) {
    EXPECT_EQ(AppExn::TApplicationExceptionType::UNKNOWN, e.getType());
    EXPECT_EQ(lulz_w, string(e.what()));
    EXPECT_EQ(lulz_s, ctx().ex_type);
    EXPECT_EQ(lulz_w, ctx().ex_what);
    EXPECT_TRUE(ctx().ew.is_compatible_with<lulz>());
  }));
}

TEST_F(ThriftServerExceptionTest, banal_with_exception_ptr) {
  Function<exception_ptr()> go = [&] { return to_eptr(make_banal()); };

  auto handler = make_shared<RaiserHandler>(evhandlers, std::move(go));
  ScopedServerInterfaceThread runner(handler);

  auto client = runner.newClient<RaiserAsyncClient>(&eb);

  auto banal_s = string{"apache::thrift::test::Banal"};
  auto banal_w_guess = fmt::format("{0}: ::{0}", banal_s);
  auto banal_w_known = fmt::format("::{0}", banal_s);

  EXPECT_TRUE(exn(client->future_doBland(), [&](const AppExn& e) {
    EXPECT_EQ(banal_w_guess, string(e.what()));
    EXPECT_EQ(banal_s, ctx().ex_type);
    EXPECT_EQ(banal_w_guess, ctx().ex_what);
    EXPECT_TRUE(ctx().ew.is_compatible_with<Banal>());
  }));
  EXPECT_TRUE(exn(client->future_doRaise(), [&](const Banal& e) {
    EXPECT_EQ(banal_w_known, string(e.what()));
    EXPECT_EQ(banal_s, ctx().ex_type);
    EXPECT_EQ(banal_w_known, ctx().ex_what);
    EXPECT_TRUE(ctx().ew.is_compatible_with<Banal>());
  }));
  EXPECT_TRUE(exn(client->future_get200(), [&](const AppExn& e) {
    EXPECT_EQ(banal_w_guess, string(e.what()));
    EXPECT_EQ(banal_s, ctx().ex_type);
    EXPECT_EQ(banal_w_guess, ctx().ex_what);
    EXPECT_TRUE(ctx().ew.is_compatible_with<Banal>());
  }));
  EXPECT_TRUE(exn(client->future_get500(), [&](const Banal& e) {
    EXPECT_EQ(banal_w_known, string(e.what()));
    EXPECT_EQ(banal_s, ctx().ex_type);
    EXPECT_EQ(banal_w_known, ctx().ex_what);
    EXPECT_TRUE(ctx().ew.is_compatible_with<Banal>());
  }));
}

TEST_F(ThriftServerExceptionTest, fiery_with_exception_ptr) {
  Function<exception_ptr()> go = [&] { return to_eptr(make_fiery()); };

  auto handler = make_shared<RaiserHandler>(evhandlers, std::move(go));
  ScopedServerInterfaceThread runner(handler);

  auto client = runner.newClient<RaiserAsyncClient>(&eb);

  auto fiery_s = string{"apache::thrift::test::Fiery"};
  auto fiery_w_guess = fmt::format("{0}: ::{0}", fiery_s);
  auto fiery_w_known = fmt::format("::{0}", fiery_s);

  EXPECT_TRUE(exn(client->future_doBland(), [&](const AppExn& e) {
    EXPECT_EQ(fiery_w_guess, string(e.what()));
    EXPECT_EQ(fiery_s, ctx().ex_type);
    EXPECT_EQ(fiery_w_guess, ctx().ex_what);
    EXPECT_TRUE(ctx().ew.is_compatible_with<Fiery>());
  }));
  EXPECT_TRUE(exn(client->future_doRaise(), [&](const Fiery& e) {
    EXPECT_EQ(fiery_w_known, string(e.what()));
    EXPECT_EQ(message, *e.message());
    EXPECT_EQ(fiery_s, ctx().ex_type);
    EXPECT_EQ(fiery_w_known, ctx().ex_what);
    EXPECT_TRUE(ctx().ew.is_compatible_with<Fiery>());
  }));
  EXPECT_TRUE(exn(client->future_get200(), [&](const AppExn& e) {
    EXPECT_EQ(fiery_w_guess, string(e.what()));
    EXPECT_EQ(fiery_s, ctx().ex_type);
    EXPECT_EQ(fiery_w_guess, ctx().ex_what);
    EXPECT_TRUE(ctx().ew.is_compatible_with<Fiery>());
  }));
  EXPECT_TRUE(exn(client->future_get500(), [&](const Fiery& e) {
    EXPECT_EQ(fiery_w_known, string(e.what()));
    EXPECT_EQ(message, *e.message());
    EXPECT_EQ(fiery_s, ctx().ex_type);
    EXPECT_EQ(fiery_w_known, ctx().ex_what);
    EXPECT_TRUE(ctx().ew.is_compatible_with<Fiery>());
  }));
}

TEST_F(ThriftServerExceptionTest, bland_with_exception_wrapper) {
  Function<exception_wrapper()> go = [&] { return to_wrap(make_lulz()); };

  auto handler = make_shared<RaiserHandler>(evhandlers, std::move(go));
  ScopedServerInterfaceThread runner(handler);

  auto client = runner.newClient<RaiserAsyncClient>(&eb);

  auto lulz_s = string{"lulz"};
  auto lulz_w = fmt::format("lulz: {}", message);

  EXPECT_TRUE(exn(client->future_doBland(), [&](const AppExn& e) {
    EXPECT_EQ(AppExn::TApplicationExceptionType::UNKNOWN, e.getType());
    EXPECT_EQ(lulz_w, string(e.what()));
    EXPECT_EQ(lulz_s, ctx().ex_type);
    EXPECT_EQ(lulz_w, ctx().ex_what);
    EXPECT_TRUE(ctx().ew.is_compatible_with<lulz>());
  }));
  EXPECT_TRUE(exn(client->future_doRaise(), [&](const AppExn& e) {
    EXPECT_EQ(AppExn::TApplicationExceptionType::UNKNOWN, e.getType());
    EXPECT_EQ(lulz_w, string(e.what()));
    EXPECT_EQ(lulz_s, ctx().ex_type);
    EXPECT_EQ(lulz_w, ctx().ex_what);
    EXPECT_TRUE(ctx().ew.is_compatible_with<lulz>());
  }));
  EXPECT_TRUE(exn(client->future_get200(), [&](const AppExn& e) {
    EXPECT_EQ(AppExn::TApplicationExceptionType::UNKNOWN, e.getType());
    EXPECT_EQ(lulz_w, string(e.what()));
    EXPECT_EQ(lulz_s, ctx().ex_type);
    EXPECT_EQ(lulz_w, ctx().ex_what);
    EXPECT_TRUE(ctx().ew.is_compatible_with<lulz>());
  }));
  EXPECT_TRUE(exn(client->future_get500(), [&](const AppExn& e) {
    EXPECT_EQ(AppExn::TApplicationExceptionType::UNKNOWN, e.getType());
    EXPECT_EQ(lulz_w, string(e.what()));
    EXPECT_EQ(lulz_s, ctx().ex_type);
    EXPECT_EQ(lulz_w, ctx().ex_what);
    EXPECT_TRUE(ctx().ew.is_compatible_with<lulz>());
  }));
}

TEST_F(ThriftServerExceptionTest, banal_with_exception_wrapper) {
  Function<exception_wrapper()> go = [&] { return to_wrap(make_banal()); };

  auto handler = make_shared<RaiserHandler>(evhandlers, std::move(go));
  ScopedServerInterfaceThread runner(handler);

  auto client = runner.newClient<RaiserAsyncClient>(&eb);

  auto banal_s = string{"apache::thrift::test::Banal"};
  auto banal_w_guess = fmt::format("{0}: ::{0}", banal_s);
  auto banal_w_known = fmt::format("::{0}", banal_s);

  EXPECT_TRUE(exn(client->future_doBland(), [&](const AppExn& e) {
    EXPECT_EQ(banal_w_guess, string(e.what()));
    EXPECT_EQ(banal_s, ctx().ex_type);
    EXPECT_EQ(banal_w_guess, ctx().ex_what);
    EXPECT_TRUE(ctx().ew.is_compatible_with<Banal>());
  }));
  EXPECT_TRUE(exn(client->future_doRaise(), [&](const Banal& e) {
    EXPECT_EQ(banal_w_known, string(e.what()));
    EXPECT_EQ(banal_s, ctx().ex_type);
    EXPECT_EQ(banal_w_known, ctx().ex_what);
    EXPECT_TRUE(ctx().ew.is_compatible_with<Banal>());
  }));
  EXPECT_TRUE(exn(client->future_get200(), [&](const AppExn& e) {
    EXPECT_EQ(banal_w_guess, string(e.what()));
    EXPECT_EQ(banal_s, ctx().ex_type);
    EXPECT_EQ(banal_w_guess, ctx().ex_what);
    EXPECT_TRUE(ctx().ew.is_compatible_with<Banal>());
  }));
  EXPECT_TRUE(exn(client->future_get500(), [&](const Banal& e) {
    EXPECT_EQ(banal_w_known, string(e.what()));
    EXPECT_EQ(banal_s, ctx().ex_type);
    EXPECT_EQ(banal_w_known, ctx().ex_what);
    EXPECT_TRUE(ctx().ew.is_compatible_with<Banal>());
  }));
}

TEST_F(ThriftServerExceptionTest, fiery_with_exception_wrapper) {
  Function<exception_wrapper()> go = [&] { return to_wrap(make_fiery()); };

  auto handler = make_shared<RaiserHandler>(evhandlers, std::move(go));
  ScopedServerInterfaceThread runner(handler);

  auto client = runner.newClient<RaiserAsyncClient>(&eb);

  auto fiery_s = string{"apache::thrift::test::Fiery"};
  auto fiery_w_guess = fmt::format("{0}: ::{0}", fiery_s);
  auto fiery_w_known = fmt::format("::{0}", fiery_s);

  EXPECT_TRUE(exn(client->future_doBland(), [&](const AppExn& e) {
    EXPECT_EQ(fiery_w_guess, string(e.what()));
    EXPECT_EQ(fiery_s, ctx().ex_type);
    EXPECT_EQ(fiery_w_guess, ctx().ex_what);
    EXPECT_TRUE(ctx().ew.is_compatible_with<Fiery>());
  }));
  EXPECT_TRUE(exn(client->future_doRaise(), [&](const Fiery& e) {
    EXPECT_EQ(fiery_w_known, string(e.what()));
    EXPECT_EQ(message, *e.message());
    EXPECT_EQ(fiery_s, ctx().ex_type);
    EXPECT_EQ(fiery_w_known, ctx().ex_what);
    EXPECT_TRUE(ctx().ew.is_compatible_with<Fiery>());
  }));
  EXPECT_TRUE(exn(client->future_get200(), [&](const AppExn& e) {
    EXPECT_EQ(fiery_w_guess, string(e.what()));
    EXPECT_EQ(fiery_s, ctx().ex_type);
    EXPECT_EQ(fiery_w_guess, ctx().ex_what);
    EXPECT_TRUE(ctx().ew.is_compatible_with<Fiery>());
  }));
  EXPECT_TRUE(exn(client->future_get500(), [&](const Fiery& e) {
    EXPECT_EQ(fiery_w_known, string(e.what()));
    EXPECT_EQ(message, *e.message());
    EXPECT_EQ(fiery_s, ctx().ex_type);
    EXPECT_EQ(fiery_w_known, ctx().ex_what);
    EXPECT_TRUE(ctx().ew.is_compatible_with<Fiery>());
  }));
}
