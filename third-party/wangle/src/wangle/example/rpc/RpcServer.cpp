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

#include <folly/executors/CPUThreadPoolExecutor.h>
#include <folly/init/Init.h>
#include <wangle/bootstrap/ServerBootstrap.h>
#include <wangle/channel/AsyncSocketHandler.h>
#include <wangle/channel/EventBaseHandler.h>
#include <wangle/codec/LengthFieldBasedFrameDecoder.h>
#include <wangle/codec/LengthFieldPrepender.h>
#include <wangle/service/ExecutorFilter.h>
#include <wangle/service/ServerDispatcher.h>
#include <wangle/service/Service.h>

#include <wangle/example/rpc/ServerSerializeHandler.h>

using namespace folly;
using namespace wangle;

using thrift::test::Bonk;
using thrift::test::Xtruct;

using SerializePipeline = wangle::Pipeline<IOBufQueue&, Xtruct>;

DEFINE_int32(port, 8080, "test server port");

class RpcService : public Service<Bonk, Xtruct> {
 public:
  Future<Xtruct> operator()(Bonk request) override {
    // Oh no, we got Bonked!  Quick, Bonk back
    printf("Bonk: %s, %i\n", request.message()->c_str(), *request.type());

    /* sleep override: ignore lint
     * useful for testing dispatcher behavior by hand
     */
    // Wait for a bit
    return futures::sleepUnsafe(std::chrono::seconds(*request.type()))
        .thenValue([request](auto&&) {
          Xtruct response;
          *response.string_thing() = "Stop saying " + *request.message() + "!";
          *response.i32_thing() = *request.type();
          return response;
        });
  }
};

class RpcPipelineFactory : public PipelineFactory<SerializePipeline> {
 public:
  SerializePipeline::Ptr newPipeline(
      std::shared_ptr<AsyncTransport> sock) override {
    auto pipeline = SerializePipeline::create();
    pipeline->addBack(AsyncSocketHandler(sock));
    // ensure we can write from any thread
    pipeline->addBack(EventBaseHandler());
    pipeline->addBack(LengthFieldBasedFrameDecoder());
    pipeline->addBack(LengthFieldPrepender());
    pipeline->addBack(ServerSerializeHandler());
    // We could use a serial dispatcher instead easily
    // pipeline->addBack(SerialServerDispatcher<Bonk>(&service_));
    // Or a Pipelined Dispatcher
    // pipeline->addBack(PipelinedServerDispatcher<Bonk>(&service_));
    pipeline->addBack(MultiplexServerDispatcher<Bonk, Xtruct>(&service_));
    pipeline->finalize();

    return pipeline;
  }

 private:
  ExecutorFilter<Bonk, Xtruct> service_{
      std::make_shared<CPUThreadPoolExecutor>(10),
      std::make_shared<RpcService>()};
};

int main(int argc, char** argv) {
  folly::Init init(&argc, &argv);

  ServerBootstrap<SerializePipeline> server;
  server.childPipeline(std::make_shared<RpcPipelineFactory>());
  server.bind(FLAGS_port);
  server.waitForStop();

  return 0;
}
