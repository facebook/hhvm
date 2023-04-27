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
#include <wangle/bootstrap/ServerBootstrap.h>
#include <wangle/channel/AsyncSocketHandler.h>
#include <wangle/codec/LineBasedFrameDecoder.h>
#include <wangle/codec/StringCodec.h>

using namespace folly;
using namespace wangle;

DEFINE_int32(port, 8080, "echo server port");

typedef Pipeline<IOBufQueue&, std::string> EchoPipeline;

// the main logic of our echo server; receives a string and writes it straight
// back
class EchoHandler : public HandlerAdapter<std::string> {
 public:
  void read(Context* ctx, std::string msg) override {
    std::cout << "handling " << msg << std::endl;
    write(ctx, msg + "\r\n");
  }
};

// where we define the chain of handlers for each messeage received
class EchoPipelineFactory : public PipelineFactory<EchoPipeline> {
 public:
  EchoPipeline::Ptr newPipeline(std::shared_ptr<AsyncTransport> sock) override {
    auto pipeline = EchoPipeline::create();
    pipeline->addBack(AsyncSocketHandler(sock));
    pipeline->addBack(LineBasedFrameDecoder(8192));
    pipeline->addBack(StringCodec());
    pipeline->addBack(EchoHandler());
    pipeline->finalize();
    return pipeline;
  }
};

int main(int argc, char** argv) {
  folly::Init init(&argc, &argv);

  ServerBootstrap<EchoPipeline> server;
  server.childPipeline(std::make_shared<EchoPipelineFactory>());
  server.bind(FLAGS_port);
  server.waitForStop();

  return 0;
}
