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

#include <memory>

#include <folly/portability/GTest.h>

#include <folly/io/IOBuf.h>
#include <folly/io/async/EventBase.h>
#include <folly/io/async/EventBaseManager.h>
#include <thrift/lib/cpp2/protocol/CompactProtocol.h>
#include <thrift/lib/cpp2/protocol/Protocol.h>
#include <thrift/lib/cpp2/server/Cpp2Worker.h>
#include <thrift/lib/cpp2/server/ThriftServer.h>
#include <thrift/lib/cpp2/transport/http2/common/SingleRpcChannel.h>
#include <thrift/lib/cpp2/transport/http2/common/testutil/FakeProcessors.h>
#include <thrift/lib/cpp2/transport/http2/common/testutil/FakeResponseHandler.h>
#include <thrift/lib/cpp2/transport/http2/server/ThriftRequestHandler.h>

namespace {
proxygen::HTTPException makeHTTPException(proxygen::ProxygenError err) {
  proxygen::HTTPException ex(
      proxygen::HTTPException::Direction::INGRESS_AND_EGRESS, "");
  ex.setProxygenError(err);
  return ex;
}
} // namespace

namespace apache {
namespace thrift {

using folly::EventBaseManager;
using proxygen::HTTPMessage;

class ThriftRequestHandlerTest : public testing::Test {
 public:
  // Sets up for a test.
  // TODO: Parameterize for different channel implementations.
  ThriftRequestHandlerTest() {
    EventBaseManager::get()->setEventBase(eventBase_.get(), true);
    responseHandler_ = std::make_unique<FakeResponseHandler>(eventBase_.get());
    processor_ = std::make_unique<EchoProcessor>(
        "extrakey", "extravalue", "<eom>", eventBase_.get());
    // This test assumes metadata is passed in the header, so we need
    // to use SingleRpcChannel.  The second parameter 1 enables this.
    // requestHandler_ deletes itself.
    requestHandler_ = new ThriftRequestHandler(processor_.get(), worker_);
    responseHandler_->getTransaction()->setHandler(requestHandler_);
  }

  // Tears down after the test.
  ~ThriftRequestHandlerTest() override {}

  std::unique_ptr<IOBuf> makeBody() {
    auto queue = std::make_unique<IOBufQueue>();
    CompactProtocolWriter writer;
    writer.setOutput(queue.get());
    writer.writeMessageBegin("dummy", MessageType::T_CALL, 0);
    queue->append("payload");
    return queue->move();
  }

  void onError(proxygen::ProxygenError error) {
    // This simulates the HTTPTransaction behavior
    requestHandler_->onError(makeHTTPException(error));
    requestHandler_->detachTransaction();
  }

 protected:
  std::unique_ptr<folly::EventBase> eventBase_{
      std::make_unique<folly::EventBase>()};
  apache::thrift::ThriftServer server_;
  std::shared_ptr<apache::thrift::Cpp2Worker> worker_{
      apache::thrift::Cpp2Worker::create(&server_, eventBase_.get())};
  std::unique_ptr<FakeResponseHandler> responseHandler_;
  std::unique_ptr<EchoProcessor> processor_;
  ThriftRequestHandler* requestHandler_;
};

// Tests the interaction between ThriftRequestHandler and
// SingleRpcChannel without any errors.
TEST_F(ThriftRequestHandlerTest, SingleRpcChannelNoErrors) {
  auto msg = std::make_unique<HTTPMessage>();
  auto& headers = msg->getHeaders();
  headers.set("key1", "value1");
  headers.set("key2", "value2");
  eventBase_->runInEventBaseThread([&] {
    requestHandler_->onHeadersComplete(std::move(msg));
    requestHandler_->onBody(makeBody());
    requestHandler_->onEOM();
  });
  eventBase_->loopOnce();
  requestHandler_->detachTransaction();
  auto outputHeaders = responseHandler_->getHeaders();
  auto outputPayload = responseHandler_->getBody();
  EXPECT_EQ(3, outputHeaders->size());
  EXPECT_EQ("value1", outputHeaders->at("key1"));
  EXPECT_EQ("value2", outputHeaders->at("key2"));
  EXPECT_EQ("extravalue", outputHeaders->at("extrakey"));
  EXPECT_EQ("payload<eom>", outputPayload);
}

// Tests the interaction between ThriftRequestHandler and
// SingleRpcChannel with an error after the entire RPC has been
// processed.
TEST_F(ThriftRequestHandlerTest, SingleRpcChannelErrorAtEnd) {
  auto msg = std::make_unique<HTTPMessage>();
  auto& headers = msg->getHeaders();
  headers.set("key1", "value1");
  headers.set("key2", "value2");
  eventBase_->runInEventBaseThread([&] {
    requestHandler_->onHeadersComplete(std::move(msg));
    requestHandler_->onBody(makeBody());
    requestHandler_->onEOM();
  });
  eventBase_->loopOnce();
  onError(proxygen::kErrorUnknown);
  auto outputHeaders = responseHandler_->getHeaders();
  auto outputPayload = responseHandler_->getBody();
  EXPECT_EQ(3, outputHeaders->size());
  EXPECT_EQ("value1", outputHeaders->at("key1"));
  EXPECT_EQ("value2", outputHeaders->at("key2"));
  EXPECT_EQ("extravalue", outputHeaders->at("extrakey"));
  EXPECT_EQ("payload<eom>", outputPayload);
}

// Tests the interaction between ThriftRequestHandler and
// SingleRpcChannel with an error after onEOM() - but before the
// callbacks take place.
TEST_F(ThriftRequestHandlerTest, SingleRpcChannelErrorBeforeCallbacks) {
  auto msg = std::make_unique<HTTPMessage>();
  auto& headers = msg->getHeaders();
  headers.set("key1", "value1");
  headers.set("key2", "value2");
  eventBase_->runInEventBaseThread([&] {
    requestHandler_->onHeadersComplete(std::move(msg));
    requestHandler_->onBody(makeBody());
    requestHandler_->onEOM();
    onError(proxygen::kErrorUnknown);
  });
  eventBase_->loopOnce();
  auto outputHeaders = responseHandler_->getHeaders();
  auto outputPayload = responseHandler_->getBody();
  EXPECT_EQ(0, outputHeaders->size());
  EXPECT_EQ("", outputPayload);
}

// Tests the interaction between ThriftRequestHandler and
// SingleRpcChannel with an error before onEOM().
TEST_F(ThriftRequestHandlerTest, SingleRpcChannelErrorBeforeEOM) {
  auto msg = std::make_unique<HTTPMessage>();
  auto& headers = msg->getHeaders();
  headers.set("key1", "value1");
  headers.set("key2", "value2");
  requestHandler_->onHeadersComplete(std::move(msg));
  requestHandler_->onBody(makeBody());
  onError(proxygen::kErrorUnknown);
  eventBase_->loopOnce();
  auto outputHeaders = responseHandler_->getHeaders();
  auto outputPayload = responseHandler_->getBody();
  EXPECT_EQ(0, outputHeaders->size());
  EXPECT_EQ("", outputPayload);
}

// Tests the interaction between ThriftRequestHandler and
// SingleRpcChannel with an error before onBody().
TEST_F(ThriftRequestHandlerTest, SingleRpcChannelErrorBeforeOnBody) {
  auto msg = std::make_unique<HTTPMessage>();
  auto& headers = msg->getHeaders();
  headers.set("key1", "value1");
  headers.set("key2", "value2");
  requestHandler_->onHeadersComplete(std::move(msg));
  onError(proxygen::kErrorUnknown);
  eventBase_->loopOnce();
  auto outputHeaders = responseHandler_->getHeaders();
  auto outputPayload = responseHandler_->getBody();
  EXPECT_EQ(0, outputHeaders->size());
  EXPECT_EQ("", outputPayload);
}

// Tests the interaction between ThriftRequestHandler and
// SingleRpcChannel with a timeout between onBody calls
TEST_F(ThriftRequestHandlerTest, SingleRpcChannelTimeoutDuringBody) {
  auto msg = std::make_unique<HTTPMessage>();
  auto& headers = msg->getHeaders();
  headers.set("key1", "value1");
  headers.set("key2", "value2");
  responseHandler_->getTransaction()->onIngressHeadersComplete(std::move(msg));
  responseHandler_->getTransaction()->onIngressBody(
      folly::IOBuf::wrapBuffer("hello world", 11), 0);
  responseHandler_->getTransaction()->onIngressTimeout();
  EXPECT_TRUE(responseHandler_->getTransaction()->isEgressComplete());
  eventBase_->loopOnce();
  responseHandler_->getTransaction()->onIngressBody(
      folly::IOBuf::wrapBuffer("hello world", 11), 0);
  auto outputHeaders = responseHandler_->getHeaders();
  auto outputPayload = responseHandler_->getBody();
  EXPECT_EQ(0, outputHeaders->size());
  EXPECT_EQ("", outputPayload);
}

// Tests the interaction between ThriftRequestHandler and
// SingleRpcChannel with an error right at the beginning.
TEST_F(ThriftRequestHandlerTest, SingleRpcChannelErrorAtBeginning) {
  onError(proxygen::kErrorUnknown);
  eventBase_->loopOnce();
  auto outputHeaders = responseHandler_->getHeaders();
  auto outputPayload = responseHandler_->getBody();
  EXPECT_EQ(0, outputHeaders->size());
  EXPECT_EQ("", outputPayload);
}

} // namespace thrift
} // namespace apache
