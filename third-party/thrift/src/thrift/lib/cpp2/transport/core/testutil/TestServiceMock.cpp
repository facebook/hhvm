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

#include <thrift/lib/cpp2/transport/core/testutil/TestServiceMock.h>

#include <chrono>
#include <thread>

#include <folly/io/async/AsyncSocket.h>
#include <thrift/lib/cpp2/async/HeaderClientChannel.h>
#include <thrift/lib/cpp2/async/RocketClientChannel.h>
#include <thrift/lib/cpp2/transport/core/ThriftClient.h>
#include <thrift/lib/cpp2/transport/util/ConnectionManager.h>

DECLARE_string(transport);

namespace testutil::testservice {

using namespace apache::thrift;

int32_t TestServiceMock::sumTwoNumbers(int32_t x, int32_t y) {
  sumTwoNumbers_(x, y); // just inform that this function is called
  return x + y;
}

int32_t TestServiceMock::add(int32_t x) {
  add_(x); // just inform that this function is called
  sum += x;
  return sum;
}

void TestServiceMock::addAfterDelay(int32_t delayMs, int32_t x) {
  /* sleep override */
  std::this_thread::sleep_for(std::chrono::milliseconds(delayMs));
  addAfterDelay_(delayMs, x); // just to inform that this function is called
  sum += x;
}

void TestServiceMock::onewayThrowsUnexpectedException(int32_t delayMs) {
  onewayThrowsUnexpectedException_(delayMs);
  std::this_thread::sleep_for(std::chrono::milliseconds(delayMs));
  throw std::runtime_error("mock_runtime_error");
}

void TestServiceMock::throwExpectedException(int32_t) {
  TestServiceException exception;
  *exception.message() = "mock_service_method_exception";
  throw exception;
}

void TestServiceMock::throwUnexpectedException(int32_t) {
  throw std::runtime_error("mock_runtime_error");
}

void TestServiceMock::sleep(int32_t timeMs) {
  /* sleep override */
  std::this_thread::sleep_for(std::chrono::milliseconds(timeMs));
}

void TestServiceMock::headers() {
  auto header = getConnectionContext()->getHeader();

  // Even if the method throws or not, put the header value and check if reaches
  // to the client in any case or not
  header->setHeader("header_from_server", "1");

  auto keyValue = header->getHeaders();
  if (keyValue.find("unexpected_exception") != keyValue.end()) {
    throw std::runtime_error("unexpected exception");
  }

  if (keyValue.find("expected_exception") != keyValue.end()) {
    TestServiceException exception;
    *exception.message() = "expected exception";
    throw exception;
  }

  if (keyValue.find("header_from_client") == keyValue.end() ||
      keyValue.find("header_from_client")->second != "2") {
    TestServiceException exception;
    *exception.message() = "Expected key/value, foo:bar, is missing";
    throw exception;
  }
}

void TestServiceMock::hello(
    std::string& result, std::unique_ptr<std::string> name) {
  hello_(*name);
  result = "Hello, " + *name;
}

void TestServiceMock::checkPort(int32_t port) {
  checkPort_(port);
  CHECK_EQ(port, getConnectionContext()->getPeerAddress()->getPort());
}

void TestServiceMock::echo(
    std::string& result, std::unique_ptr<folly::IOBuf> val) {
  echo_(*val);
  folly::io::Cursor c(val.get());
  result = c.readFixedString(val->computeChainDataLength());
}

void TestServiceMock::onewayLogBlob(std::unique_ptr<folly::IOBuf> val) {
  onewayLogBlob_(*val);
}

IntermHeaderService::IntermHeaderService(
    const std::string& host, int16_t port) {
  if (FLAGS_transport == "header") {
    HeaderClientChannel::Ptr channel;
    evbThread_.getEventBase()->runInEventBaseThreadAndWait([&]() {
      channel = HeaderClientChannel::newChannel(
          folly::AsyncSocket::UniquePtr(
              new folly::AsyncSocket(evbThread_.getEventBase(), host, port)));
    });
    client_ = std::make_unique<TestServiceAsyncClient>(std::move(channel));
  } else if (FLAGS_transport == "rocket") {
    RocketClientChannel::Ptr channel;
    evbThread_.getEventBase()->runInEventBaseThreadAndWait([&]() {
      channel = RocketClientChannel::newChannel(
          folly::AsyncSocket::UniquePtr(
              new folly::AsyncSocket(evbThread_.getEventBase(), host, port)));
    });
    client_ = std::make_unique<TestServiceAsyncClient>(std::move(channel));
  } else {
    auto mgr = ConnectionManager::getInstance();
    auto connection = mgr->getConnection(host, port);
    auto channel = ThriftClient::Ptr(new ThriftClient(connection));
    channel->setProtocolId(apache::thrift::protocol::T_COMPACT_PROTOCOL);
    client_ = std::make_unique<TestServiceAsyncClient>(std::move(channel));
  }
}

IntermHeaderService::~IntermHeaderService() {
  // destroy the client in the eventbase thread, otherwise use
  // PooledRequestChannel
  evbThread_.getEventBase()->runInEventBaseThread(
      [client = std::move(client_)]() {});
}

int32_t IntermHeaderService::callAdd(int32_t x) {
  auto rq = folly::RequestContext::get();
  auto ret = client_->future_add(x).get();
  EXPECT_EQ(rq, folly::RequestContext::get());
  return ret;
}

} // namespace testutil::testservice
