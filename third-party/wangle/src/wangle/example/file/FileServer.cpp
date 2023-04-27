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
#include <wangle/channel/FileRegion.h>
#include <wangle/codec/LineBasedFrameDecoder.h>
#include <wangle/codec/StringCodec.h>

using namespace folly;
using namespace wangle;

DEFINE_int32(port, 11219, "test file server port");

typedef Pipeline<IOBufQueue&, std::string> FileServerPipeline;

class FileServerHandler : public HandlerAdapter<std::string> {
 public:
  void read(Context* ctx, std::string filename) override {
    if (filename == "bye") {
      close(ctx);
    }

    int fd = open(filename.c_str(), O_RDONLY);
    if (fd == -1) {
      write(
          ctx, sformat("Error opening {}: {}\r\n", filename, strerror(errno)));
      return;
    }

    struct stat buf;
    if (fstat(fd, &buf) == -1) {
      write(
          ctx,
          sformat("Could not stat file {}: {}\r\n", filename, strerror(errno)));
      return;
    }

    FileRegion fileRegion(fd, 0, buf.st_size);
    auto guard = ctx->getPipelineShared();
    fileRegion.transferTo(ctx->getTransport())
        .thenError(
            folly::tag_t<std::exception>{},
            [this, guard, ctx, filename](const std::exception& e) {
              write(
                  ctx,
                  sformat(
                      "Error sending file {}: {}\r\n",
                      filename,
                      exceptionStr(e)));
            });
  }

  void readException(Context* ctx, exception_wrapper ew) override {
    write(ctx, sformat("Error: {}\r\n", exceptionStr(ew)))
        .thenValue([this, ctx](auto&&) { close(ctx); });
  }

  void transportActive(Context* ctx) override {
    SocketAddress localAddress;
    ctx->getTransport()->getLocalAddress(&localAddress);
    write(ctx, "Welcome to " + localAddress.describe() + "!\r\n");
    write(ctx, "Type the name of a file and it will be streamed to you!\r\n");
    write(ctx, "Type 'bye' to exit.\r\n");
  }
};

class FileServerPipelineFactory : public PipelineFactory<FileServerPipeline> {
 public:
  FileServerPipeline::Ptr newPipeline(
      std::shared_ptr<AsyncTransport> sock) override {
    auto pipeline = FileServerPipeline::create();
    pipeline->addBack(AsyncSocketHandler(sock));
    pipeline->addBack(LineBasedFrameDecoder());
    pipeline->addBack(StringCodec());
    pipeline->addBack(FileServerHandler());
    pipeline->finalize();

    return pipeline;
  }
};

int main(int argc, char** argv) {
  folly::Init init(&argc, &argv);

  ServerBootstrap<FileServerPipeline> server;
  server.childPipeline(std::make_shared<FileServerPipelineFactory>());
  server.bind(FLAGS_port);
  server.waitForStop();

  return 0;
}
