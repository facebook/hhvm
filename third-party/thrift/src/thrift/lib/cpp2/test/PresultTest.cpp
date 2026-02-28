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

#include <gtest/gtest.h>
#include <folly/io/async/AsyncSocket.h>
#include <thrift/lib/cpp2/async/HeaderClientChannel.h>
#include <thrift/lib/cpp2/server/ThriftServer.h>
#include <thrift/lib/cpp2/test/gen-cpp2/PresultService.h>
#include <thrift/lib/cpp2/test/util/TestThriftServerFactory.h>
#include <thrift/lib/cpp2/util/ScopedServerThread.h>

using namespace cpp2;
using namespace apache::thrift;
using namespace apache::thrift::util;

class PresultServiceInterface
    : public apache::thrift::ServiceHandler<PresultService> {
 public:
  void methodVoid() override {}
  bool methodBool(bool x) override { return x; }
  int8_t methodByte(int8_t x) override { return x; }
  int16_t methodI16(int16_t x) override { return x; }
  int32_t methodI32(int32_t x) override { return x; }
  int64_t methodI64(int64_t x) override { return x; }
  float methodFloat(float x) override { return x; }
  double methodDouble(double x) override { return x; }
  Enum methodEnum(Enum x) override { return x; }
  void methodString(std::string& ret, std::unique_ptr<std::string> x) override {
    ret = *x;
  }

  void methodBinary(std::string& ret, std::unique_ptr<std::string> x) override {
    ret = *x;
  }

  void methodIOBuf(
      folly::IOBuf& ret, std::unique_ptr<folly::IOBuf> x) override {
    ret = std::move(*x);
  }

  void methodIOBufPtr(
      std::unique_ptr<folly::IOBuf>& ret,
      std::unique_ptr<std::unique_ptr<folly::IOBuf>> x) override {
    ret = std::move(*x);
  }

  void methodList(
      std::vector<int32_t>& ret,
      std::unique_ptr<std::vector<int32_t>> x) override {
    ret = *x;
  }

  void methodListBool(
      std::vector<bool>& ret, std::unique_ptr<std::vector<bool>> x) override {
    ret = *x;
  }

  void methodDeque(
      std::deque<int32_t>& ret,
      std::unique_ptr<std::deque<int32_t>> x) override {
    ret = *x;
  }

  void methodMap(
      std::map<int32_t, int64_t>& ret,
      std::unique_ptr<std::map<int32_t, int64_t>> x) override {
    ret = *x;
  }

  void methodUnorderedMap(
      std::unordered_map<int32_t, int64_t>& ret,
      std::unique_ptr<std::unordered_map<int32_t, int64_t>> x) override {
    ret = *x;
  }

  void methodSet(
      std::set<int32_t>& ret, std::unique_ptr<std::set<int32_t>> x) override {
    ret = *x;
  }

  void methodUnorderedSet(
      std::unordered_set<int32_t>& ret,
      std::unique_ptr<std::unordered_set<int32_t>> x) override {
    ret = *x;
  }

  void methodStruct(Struct& ret, std::unique_ptr<Struct> x) override {
    ret = *x;
  }

  void methodException(Struct& /* ret */, int32_t which) override {
    if (which) {
      throw Exception1(FRAGILE, 5);
    } else {
      throw Exception2(FRAGILE, "ex2");
    }
  }
};

std::shared_ptr<PresultServiceAsyncClient> getClient(
    const ScopedServerThread& sst, folly::EventBase& eb) {
  auto socket = folly::AsyncSocket::newSocket(&eb, *sst.getAddress());
  auto channel = HeaderClientChannel::newChannel(
      HeaderClientChannel::WithoutRocketUpgrade{}, std::move(socket));
  auto client = std::make_shared<PresultServiceAsyncClient>(std::move(channel));
  return client;
}

template <class F, class Arg>
void run(int& count, F&& f, Arg&& arg) {
  count++;
  f().thenValue([arg, &count](const Arg& res) {
    EXPECT_EQ(arg, res);
    count--;
  });
}

template <class F>
void run(int& count, F&& f) {
  count++;
  f().thenValue([&count](auto&&) { count--; });
}

#define F(method, ...) \
  [client] { return client->future_##method(__VA_ARGS__); }, ##__VA_ARGS__

TEST(Presult, Presult) {
  folly::EventBase eb;
  apache::thrift::TestThriftServerFactory<PresultServiceInterface> factory;
  factory.useSimpleThreadManager(false);
  ScopedServerThread sst(factory.create());
  auto client = getClient(sst, eb);

  int count = 0;
  run(count, F(methodVoid));
  run(count, F(methodBool, true));
  run(count, F(methodByte, 5));
  run(count, F(methodI16, 2000));
  run(count, F(methodI32, 1 << 20));
  run(count, F(methodI64, 1L << 40));
  run(count, F(methodFloat, 3.14f));
  run(count, F(methodDouble, 2.71));
  run(count, F(methodEnum, Enum::Value2));
  run(count, F(methodString, std::string("hello")));
  run(count, F(methodBinary, std::string("binary")));
  run(count, F(methodList, std::vector<int32_t>({1, 2, 3})));
  run(count, F(methodListBool, std::vector<bool>({true, false, true})));
  run(count, F(methodDeque, std::deque<int32_t>({1, 2, 3})));
  run(count,
      F(methodMap, std::map<int32_t, int64_t>({{1, 11}, {2, 22}, {3, 33}})));
  run(count,
      F(methodUnorderedMap,
        std::unordered_map<int32_t, int64_t>({{1, 11}, {2, 22}, {3, 33}})));
  run(count, F(methodSet, std::set<int32_t>({1, 2, 3})));
  run(count, F(methodUnorderedSet, std::unordered_set<int32_t>({1, 2, 3})));
  run(count, F(methodStruct, Struct(FRAGILE, 5)));

  {
    count++;
    std::string data = "iobuf";
    auto iobuf = folly::IOBuf::wrapBuffer(folly::StringPiece(data));
    client->future_methodIOBuf(*iobuf).thenValue(
        [data, &count](folly::IOBuf&& res) {
          EXPECT_EQ(folly::StringPiece(res.coalesce()).str(), data);
          count--;
        });
  }

  {
    count++;
    std::string data = "iobufptr";
    auto iobuf = folly::IOBuf::wrapBuffer(folly::StringPiece(data));
    client->future_methodIOBufPtr(std::move(iobuf))
        .thenValue([data, &count](std::unique_ptr<folly::IOBuf> res) {
          EXPECT_EQ(folly::StringPiece(res->coalesce()).str(), data);
          count--;
        });
  }

  count++;
  client->future_methodException(1)
      .thenValue([](const Struct&) { FAIL(); })
      .thenError(
          folly::tag_t<Exception1>{},
          [&count](const Exception1& e) {
            EXPECT_EQ(*e.code(), 5);
            count--;
          })
      .thenError(folly::tag_t<std::exception>{}, [](const std::exception&) {
        FAIL();
      });
  count++;
  client->future_methodException(0)
      .thenValue([](const Struct&) { FAIL(); })
      .thenError(
          folly::tag_t<Exception2>{},
          [&count](const Exception2& e) {
            EXPECT_EQ(*e.message(), "ex2");
            count--;
          })
      .thenError(folly::tag_t<std::exception>{}, [](const std::exception&) {
        FAIL();
      });

  eb.loop();
  EXPECT_EQ(count, 0);
}
