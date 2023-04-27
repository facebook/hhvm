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

#pragma once

#include <folly/portability/GTest.h>

#include <folly/Function.h>
#include <folly/io/IOBufQueue.h>
#include <folly/io/async/EventBase.h>
#include <folly/io/async/EventBaseLocal.h>
#include <thrift/lib/cpp/concurrency/ThreadManager.h>
#include <thrift/lib/cpp2/server/ThriftProcessor.h>
#include <thrift/lib/cpp2/server/ThriftServer.h>
#include <thrift/lib/cpp2/transport/core/testutil/FakeChannel.h>
#include <thrift/lib/cpp2/transport/core/testutil/TestServiceMock.h>
#include <thrift/lib/cpp2/transport/core/testutil/gen-cpp2/TestService.tcc>
#include <thrift/lib/thrift/gen-cpp2/RpcMetadata_types.h>

namespace apache {
namespace thrift {

class Cpp2Worker;

class CoreTestFixture : public testing::Test {
 public:
  // Sets up for a test.
  CoreTestFixture();

  // Tears down after the test.
  ~CoreTestFixture() override;

  // Schedules the test in the event base and runs the event base loop
  // so the event base is running for the entirety of the test.  The
  // loop is terminated when FakeChannel::sendThriftResponse() is
  // called (which must happen as part of running "test").
  void runInEventBaseThread(folly::Function<void()> test);

  // Send the two integers to be serialized for 'sumTwoNumbers'
  static void serializeSumTwoNumbers(
      int32_t x,
      int32_t y,
      bool wrongMethodName,
      folly::IOBufQueue* request,
      apache::thrift::RequestRpcMetadata* metadata);

  // Receive the deserialized integer that results from 'sumTwoNumbers'
  static int32_t deserializeSumTwoNumbers(folly::IOBuf* buf);

  static RequestRpcMetadata makeMetadata(
      std::string name, RpcKind kind = RpcKind::SINGLE_REQUEST_SINGLE_RESPONSE);

  // Deserialize the exception if possible, return false otherwise.
  static bool deserializeException(
      folly::IOBuf* buf, TApplicationException* tae);

  std::unique_ptr<Cpp2ConnContext> newCpp2ConnContext();

 protected:
  ThriftServer server_;
  std::shared_ptr<testing::StrictMock<testutil::testservice::TestServiceMock>>
      service_ = std::make_shared<
          testing::StrictMock<testutil::testservice::TestServiceMock>>();
  ThriftProcessor processor_;
  folly::EventBase eventBase_;
  std::shared_ptr<FakeChannel> channel_;
  std::shared_ptr<Cpp2Worker> worker_;
};

} // namespace thrift
} // namespace apache
