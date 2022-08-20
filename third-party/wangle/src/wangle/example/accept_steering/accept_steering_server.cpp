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

#include <folly/portability/GFlags.h>

#include <folly/init/Init.h>
#include <wangle/bootstrap/AcceptRoutingHandler.h>
#include <wangle/bootstrap/RoutingDataHandler.h>
#include <wangle/bootstrap/ServerBootstrap.h>
#include <wangle/channel/AsyncSocketHandler.h>

using namespace folly;
using namespace wangle;

DEFINE_int32(port, 23, "test server port");

/**
 * A simple server that hashes connections to worker threads
 * based on the first character typed in by the client.
 */

class NaiveRoutingDataHandler : public RoutingDataHandler<char> {
 public:
  NaiveRoutingDataHandler(uint64_t connId, Callback* cob)
      : RoutingDataHandler<char>(connId, cob) {}

  bool parseRoutingData(folly::IOBufQueue& bufQueue, RoutingData& routingData)
      override {
    if (bufQueue.chainLength() == 0) {
      return false;
    }

    auto buf = bufQueue.move();
    buf->coalesce();
    // Use the first byte for hashing to a worker
    routingData.routingData = buf->data()[0];
    routingData.bufQueue.append(std::move(buf));
    return true;
  }
};

class NaiveRoutingDataHandlerFactory : public RoutingDataHandlerFactory<char> {
 public:
  std::shared_ptr<RoutingDataHandler<char>> newHandler(
      uint64_t connId,
      RoutingDataHandler<char>::Callback* cob) override {
    return std::make_shared<NaiveRoutingDataHandler>(connId, cob);
  }
};

class ThreadPrintingHandler : public BytesToBytesHandler {
 public:
  explicit ThreadPrintingHandler(const char& routingData)
      : routingData_(routingData) {}

  void transportActive(Context* ctx) override {
    std::stringstream out;
    out << "You were hashed to thread " << std::this_thread::get_id()
        << " based on '" << routingData_ << "'" << std::endl;
    write(ctx, IOBuf::copyBuffer(out.str()));
    close(ctx);
  }

 private:
  char routingData_;
};

class ServerPipelineFactory
    : public RoutingDataPipelineFactory<DefaultPipeline, char> {
 public:
  DefaultPipeline::Ptr newPipeline(
      std::shared_ptr<AsyncTransport> sock,
      const char& routingData,
      RoutingDataHandler<char>*,
      std::shared_ptr<TransportInfo> transportInfo) override {
    auto pipeline = DefaultPipeline::create();
    pipeline->addBack(AsyncSocketHandler(sock));
    pipeline->addBack(ThreadPrintingHandler(routingData));
    pipeline->finalize();

    pipeline->setTransportInfo(transportInfo);

    LOG(INFO) << "Created new server pipeline. Local address = "
              << *(transportInfo->localAddr)
              << ", remote address = " << *(transportInfo->remoteAddr);

    return pipeline;
  }
};

int main(int argc, char** argv) {
  folly::Init init(&argc, &argv);

  auto routingHandlerFactory =
      std::make_shared<NaiveRoutingDataHandlerFactory>();
  auto childPipelineFactory = std::make_shared<ServerPipelineFactory>();

  ServerBootstrap<DefaultPipeline> server;
  server.pipeline(
      std::make_shared<AcceptRoutingPipelineFactory<DefaultPipeline, char>>(
          &server, routingHandlerFactory, childPipelineFactory));
  server.bind(FLAGS_port);
  server.waitForStop();

  return 0;
}
