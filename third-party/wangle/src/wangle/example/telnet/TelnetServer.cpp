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

DEFINE_int32(port, 23, "test telnet server port");

typedef Pipeline<IOBufQueue&, std::string> TelnetPipeline;

class TelnetHandler : public HandlerAdapter<std::string> {
 public:
  void read(Context* ctx, std::string msg) override {
    if (msg.empty()) {
      write(ctx, "Please type something.\r\n");
    } else if (msg == "bye") {
      write(ctx, "Have a fabulous day!\r\n").thenValue([ctx, this](auto&&) {
        close(ctx);
      });
    } else {
      write(ctx, "Did you say '" + msg + "'?\r\n");
    }
  }

  void transportActive(Context* ctx) override {
    auto sock = ctx->getTransport();
    SocketAddress localAddress;
    sock->getLocalAddress(&localAddress);
    write(ctx, "Welcome to " + localAddress.describe() + "!\r\n");
    write(ctx, "Type 'bye' to disconnect.\r\n");
  }
};

class TelnetPipelineFactory : public PipelineFactory<TelnetPipeline> {
 public:
  TelnetPipeline::Ptr newPipeline(
      std::shared_ptr<AsyncTransport> sock) override {
    auto pipeline = TelnetPipeline::create();
    pipeline->addBack(AsyncSocketHandler(sock));
    pipeline->addBack(LineBasedFrameDecoder(8192));
    pipeline->addBack(StringCodec());
    pipeline->addBack(TelnetHandler());
    pipeline->finalize();

    return pipeline;
  }
};

int main(int argc, char** argv) {
  folly::Init init(&argc, &argv);

  ServerBootstrap<TelnetPipeline> server;
  server.childPipeline(std::make_shared<TelnetPipelineFactory>());
  server.bind(FLAGS_port);
  server.waitForStop();

  return 0;
}
