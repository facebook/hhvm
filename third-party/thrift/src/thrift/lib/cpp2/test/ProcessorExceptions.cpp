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

// This test checks for an exception when a server receives a struct
// with a required member missing.
// We test it by using a fake client (SampleService2) whose arguments
// definition define an optional field and send it to a server (SampleService)
// that expects a required argument

#include <gtest/gtest.h>
#include <thrift/lib/cpp/TApplicationException.h>
#include <thrift/lib/cpp2/async/HeaderClientChannel.h>
#include <thrift/lib/cpp2/async/RequestChannel.h>
#include <thrift/lib/cpp2/server/ThriftServer.h>
#include <thrift/lib/cpp2/test/gen-cpp2/SampleService.h>
#include <thrift/lib/cpp2/test/gen-cpp2/SampleService2.h>
#include <thrift/lib/cpp2/test/gen-cpp2/SampleService3.h>

#include <folly/io/async/AsyncSocket.h>
#include <folly/io/async/EventBase.h>
#include <thrift/lib/cpp2/util/ScopedServerThread.h>

#include <thrift/lib/cpp2/test/util/TestThriftServerFactory.h>

#include <memory>

using namespace apache::thrift;
using namespace apache::thrift::test::cpp2;
using namespace apache::thrift::util;
using apache::thrift::TApplicationException;

class SampleServiceHandler
    : public apache::thrift::ServiceHandler<SampleService> {
 public:
  int32_t return42(const MyArgs&, int32_t) override { return 42; }
};

std::shared_ptr<ThriftServer> getServer() {
  auto server = std::make_shared<ThriftServer>();
  server->setPort(0);
  server->setInterface(
      std::unique_ptr<SampleServiceHandler>(new SampleServiceHandler));
  return server;
}

int32_t call_return42(std::function<void(MyArgs2&)> isset_cb) {
  apache::thrift::TestThriftServerFactory<SampleServiceHandler> factory;
  ScopedServerThread sst(factory.create());
  folly::EventBase base;
  auto socket(folly::AsyncSocket::newSocket(&base, *sst.getAddress()));

  SampleService2AsyncClient client(
      HeaderClientChannel::newChannel(std::move(socket)));

  Inner2 inner;
  inner.i() = 7;
  MyArgs2 args;
  args.i() = {};
  args.s() = "qwerty";
  args.l() = {1, 2, 3};
  args.m() = {{"a", 1}, {"b", 2}};
  args.li() = {inner};
  args.mi() = {{11, inner}};
  args.complex_key() = {{inner, 11}};
  args.i()->i().ensure();
  (*args.li())[0].i().ensure();
  (*args.mi())[11].i().ensure();
  MyArgs2 all_is_set(args);
  isset_cb(args);

  try {
    int32_t ret = client.sync_return42(args, 5);
    /* second call (with all_is_set) should succeed */
    EXPECT_EQ(42, client.sync_return42(all_is_set, 5));
    return ret;
  } catch (...) {
    /* second call (with all_is_set) should succeed */
    EXPECT_EQ(42, client.sync_return42(all_is_set, 5));
    throw;
  }
}

TEST(ProcessorExceptionTest, ok_if_required_set) {
  EXPECT_EQ(42, call_return42([](MyArgs2&) {}));
}

TEST(ProcessorExceptionTest, throw_if_scalar_required_missing) {
  EXPECT_THROW(
      call_return42([](MyArgs2& a) { apache::thrift::unset_unsafe(a.s()); }),
      TApplicationException);
}

TEST(ProcessorExceptionTest, throw_if_inner_required_missing) {
  EXPECT_THROW(
      call_return42([](MyArgs2& a) { apache::thrift::unset_unsafe(a.i()); }),
      TApplicationException);
}

TEST(ProcessorExceptionTest, throw_if_inner_field_required_missing) {
  EXPECT_THROW(
      call_return42(
          [](MyArgs2& a) { apache::thrift::unset_unsafe(a.i()->i()); }),
      TApplicationException);
}

TEST(ProcessorExceptionTest, throw_if_list_required_missing) {
  EXPECT_THROW(
      call_return42([](MyArgs2& a) { apache::thrift::unset_unsafe(a.l()); }),
      TApplicationException);
}

TEST(ProcessorExceptionTest, throw_if_map_required_missing) {
  EXPECT_THROW(
      call_return42([](MyArgs2& a) { apache::thrift::unset_unsafe(a.m()); }),
      TApplicationException);
}

TEST(ProcessorExceptionTest, throw_if_list_of_struct_required_missing) {
  EXPECT_THROW(
      call_return42([](MyArgs2& a) { apache::thrift::unset_unsafe(a.li()); }),
      TApplicationException);
}

TEST(ProcessorExceptionTest, throw_if_list_inner_required_missing) {
  EXPECT_THROW(
      call_return42(
          [](MyArgs2& a) { apache::thrift::unset_unsafe((*a.li())[0].i()); }),
      TApplicationException);
}

TEST(ProcessorExceptionTest, throw_if_map_of_struct_required_missing) {
  EXPECT_THROW(
      call_return42([](MyArgs2& a) { apache::thrift::unset_unsafe(a.mi()); }),
      TApplicationException);
}

TEST(ProcessorExceptionTest, throw_if_map_inner_required_missing) {
  EXPECT_THROW(
      call_return42(
          [](MyArgs2& a) { apache::thrift::unset_unsafe((*a.mi())[11].i()); }),
      TApplicationException);
}

TEST(ProcessorExceptionTest, throw_if_map_key_required_missing) {
  EXPECT_THROW(
      call_return42(
          [](MyArgs2& a) { apache::thrift::unset_unsafe(a.complex_key()); }),
      TApplicationException);
}

TEST(ProcessorExceptionTest, throw_if_method_missing) {
  apache::thrift::TestThriftServerFactory<SampleServiceHandler> factory;
  ScopedServerThread sst(factory.create());
  folly::EventBase base;
  auto socket(folly::AsyncSocket::newSocket(&base, *sst.getAddress()));
  SampleService3AsyncClient client(
      HeaderClientChannel::newChannel(std::move(socket)));

  try {
    // call a method that doesn't exist on the server but exists on the client
    client.sync_doNothing();
    ADD_FAILURE() << "Expected call_doNothing to throw";
  } catch (TApplicationException& t) {
    EXPECT_STREQ("Method name doNothing not found", t.what());
    EXPECT_EQ(
        TApplicationException::TApplicationExceptionType::UNKNOWN_METHOD,
        t.getType());
  } catch (std::exception& e) {
    ADD_FAILURE()
        << "Wrong exception thrown, expected TApplicationException, got "
        << e.what();
  }
}

TEST(ProcessorExceptionTest, throw_if_map_key_inner_required_missing) {
  EXPECT_THROW(
      call_return42([](MyArgs2& a) {
        std::pair<Inner2, int> elem = *a.complex_key()->cbegin();
        apache::thrift::unset_unsafe(elem.first.i());
        a.complex_key()->clear();
        a.complex_key()->insert(elem);
      }),
      TApplicationException);
}
