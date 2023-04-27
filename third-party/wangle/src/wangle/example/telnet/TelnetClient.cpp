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

#include <iostream>

#include <folly/portability/GFlags.h>

#include <folly/init/Init.h>
#include <wangle/bootstrap/ClientBootstrap.h>
#include <wangle/channel/AsyncSocketHandler.h>
#include <wangle/channel/EventBaseHandler.h>
#include <wangle/codec/LineBasedFrameDecoder.h>
#include <wangle/codec/StringCodec.h>

using namespace folly;
using namespace wangle;

DEFINE_int32(port, 23, "test telnet server port");
DEFINE_string(host, "::1", "test telnet server address");

typedef Pipeline<folly::IOBufQueue&, std::string> TelnetPipeline;

class TelnetHandler : public HandlerAdapter<std::string> {
 public:
  void read(Context*, std::string msg) override {
    std::cout << msg;
  }
  void readException(Context* ctx, exception_wrapper e) override {
    std::cout << exceptionStr(e) << std::endl;
    close(ctx);
  }
  void readEOF(Context* ctx) override {
    std::cout << "EOF received :(" << std::endl;
    close(ctx);
  }
};

class TelnetPipelineFactory : public PipelineFactory<TelnetPipeline> {
 public:
  TelnetPipeline::Ptr newPipeline(
      std::shared_ptr<AsyncTransport> sock) override {
    auto pipeline = TelnetPipeline::create();
    pipeline->addBack(AsyncSocketHandler(sock));
    pipeline->addBack(
        EventBaseHandler()); // ensure we can write from any thread
    pipeline->addBack(LineBasedFrameDecoder(8192, false));
    pipeline->addBack(StringCodec());
    pipeline->addBack(TelnetHandler());
    pipeline->finalize();

    return pipeline;
  }
};

int main(int argc, char** argv) {
  folly::Init init(&argc, &argv);

  ClientBootstrap<TelnetPipeline> client;
  client.group(std::make_shared<folly::IOThreadPoolExecutor>(1));
  client.pipelineFactory(std::make_shared<TelnetPipelineFactory>());
  auto pipeline = client.connect(SocketAddress(FLAGS_host, FLAGS_port)).get();

  try {
    while (true) {
      std::string line;
      std::getline(std::cin, line);
      if (line == "") {
        break;
      }

      // Sync write will throw exception if server goes away
      pipeline->write(line + "\r\n").get();
      if (line == "bye") {
        pipeline->close();
        break;
      }
    }
  } catch (const std::exception& e) {
    std::cout << exceptionStr(e) << std::endl;
  }

  return 0;
}
