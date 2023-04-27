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

#include "wangle/bootstrap/ClientBootstrap.h"
#include "wangle/bootstrap/ServerBootstrap.h"
#include "wangle/channel/Handler.h"
#include "wangle/channel/broadcast/ObservingHandler.h"

#include <folly/portability/GTest.h>
#include <glog/logging.h>

using namespace wangle;
using namespace folly;

using BytesPipeline = Pipeline<IOBufQueue&, std::unique_ptr<IOBuf>>;
using TestObsPipeline = ObservingPipeline<std::shared_ptr<folly::IOBuf>>;

using TestServer = ServerBootstrap<BytesPipeline>;
using TestClient = ClientBootstrap<TestObsPipeline>;

struct TestRoutingData {
  std::string data;
  bool operator==(const TestRoutingData& other) const {
    return this->data == other.data;
  }
  bool operator<(const TestRoutingData& other) const {
    return this->data < other.data;
  }
};

class TestPipelineFactory : public PipelineFactory<BytesPipeline> {
 public:
  BytesPipeline::Ptr newPipeline(
      std::shared_ptr<AsyncTransport> /* unused */) override {
    pipelines_++;
    auto pipeline = BytesPipeline::create();
    pipeline->addBack(new BytesToBytesHandler());
    pipeline->finalize();
    return pipeline;
  }
  std::atomic<int> pipelines_{0};
};

class CustomPipelineFactory : public TestPipelineFactory,
                              public ObservingPipelineFactory<
                                  std::shared_ptr<folly::IOBuf>,
                                  TestRoutingData> {
 public:
  CustomPipelineFactory()
      : ObservingPipelineFactory<
            std::shared_ptr<folly::IOBuf>,
            TestRoutingData>(nullptr, nullptr) {}

  TestObsPipeline::Ptr newPipeline(
      std::shared_ptr<folly::AsyncTransport> socket,
      const TestRoutingData& routingData,
      RoutingDataHandler<TestRoutingData>* /* unused */,
      std::shared_ptr<TransportInfo> /* unused */) override {
    routingData_ = routingData;
    auto pipeline = TestObsPipeline::create();
    pipeline->addBack(AsyncSocketHandler(socket));
    pipeline->finalize();
    routingPipelines_++;
    return pipeline;
  }

  BytesPipeline::Ptr newPipeline(
      std::shared_ptr<AsyncTransport> sock) override {
    // Should not be called.
    ADD_FAILURE() << "Should not be called, "
                  << "this function is typically called from "
                  << "makePipeline that has been overridden in this "
                  << "test to call a different version of newPipeline.";
    return TestPipelineFactory::newPipeline(sock);
  }

  TestRoutingData routingData_;
  std::atomic<int> routingPipelines_{0};
};

class CustomPipelineMakerTestClient : public TestClient {
 public:
  explicit CustomPipelineMakerTestClient(
      const TestRoutingData& routingData,
      const std::shared_ptr<CustomPipelineFactory>& factory)
      : routingData_(routingData), factory_(factory) {}

  void makePipeline(std::shared_ptr<folly::AsyncTransport> socket) override {
    setPipeline(factory_->newPipeline(socket, routingData_, nullptr, nullptr));
  }

  TestRoutingData routingData_;
  std::shared_ptr<CustomPipelineFactory> factory_;
};

TEST(ObservingClientPipelineTest, CustomPipelineMaker) {
  TestServer server;
  auto factory = std::make_shared<TestPipelineFactory>();
  server.childPipeline(factory);
  server.bind(0);
  auto base = EventBaseManager::get()->getEventBase();

  SocketAddress address;
  server.getSockets()[0]->getAddress(&address);

  TestRoutingData routingData;
  routingData.data = "Test";
  auto clientPipelineFactory = std::make_shared<CustomPipelineFactory>();
  auto client = std::make_unique<CustomPipelineMakerTestClient>(
      routingData, clientPipelineFactory);

  client->connect(address, std::chrono::milliseconds(0));
  base->loop();
  server.stop();
  server.join();

  EXPECT_EQ(1, clientPipelineFactory->routingPipelines_);
  EXPECT_EQ(routingData, clientPipelineFactory->routingData_);
  EXPECT_EQ(0, clientPipelineFactory->pipelines_);
}
