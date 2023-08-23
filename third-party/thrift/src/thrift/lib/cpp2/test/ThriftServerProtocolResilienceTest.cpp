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

#include <folly/io/async/test/BlockingSocket.h>
#include <thrift/lib/cpp2/server/ThriftServer.h>
#include <thrift/lib/cpp2/test/gen-cpp2/TestService.h>
#include <thrift/lib/cpp2/util/ScopedServerInterfaceThread.h>

#include <folly/portability/GTest.h>

using namespace std;
using namespace folly;
using namespace apache::thrift;
using namespace apache::thrift::test;

THRIFT_FLAG_DECLARE_bool(server_header_reject_http);

class ThriftServerProtocolResilienceTest : public testing::Test {};

class Handler : public apache::thrift::ServiceHandler<TestService> {
 public:
  Future<unique_ptr<string>> future_sendResponse(int64_t size) override {
    return makeFuture(make_unique<string>(to<string>(size)));
  }
};

void testTHeaderWithData(
    const std::string& data, bool expectResponse, bool expectConnectionClose) {
  THRIFT_FLAG_SET_MOCK(server_header_reject_http, false);

  auto handler = make_shared<Handler>();
  ScopedServerInterfaceThread runner(handler);

  auto addr = runner.getAddress();
  folly::test::BlockingSocket socket(addr, nullptr);
  socket.open();

  auto sent =
      socket.write(reinterpret_cast<const uint8_t*>(data.c_str()), data.size());
  ASSERT_EQ(sent, data.size());

  uint8_t buffer[256];
  auto res = socket.read(buffer, 256);

  ASSERT_EQ(expectResponse, res > 0) << res;
  ASSERT_EQ(!expectConnectionClose, socket.getSocket()->good());
}

TEST_F(ThriftServerProtocolResilienceTest, TestHTTPResponse) {
  auto data = "HTTP/1.1 200 OK\r\nContent-Length: 10\r\n\r\n1234567890";
  testTHeaderWithData(data, false, true);
}

TEST_F(ThriftServerProtocolResilienceTest, TestHTTPRequest) {
  auto data = "GET / HTTP/1.1\r\nHost: localhost:8080\r\n";
  testTHeaderWithData(data, true, false);
}
