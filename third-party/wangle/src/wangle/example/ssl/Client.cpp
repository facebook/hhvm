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

#include <folly/init/Init.h>
#include <folly/io/async/AsyncSSLSocket.h>
#include <folly/io/async/DelayedDestruction.h>
#include <folly/io/async/SSLOptions.h>
#include <folly/portability/GFlags.h>
#include <folly/ssl/Init.h>
#include <wangle/bootstrap/ClientBootstrap.h>
#include <wangle/channel/AsyncSocketHandler.h>
#include <wangle/channel/EventBaseHandler.h>
#include <wangle/client/persistence/LRUPersistentCache.h>
#include <wangle/client/ssl/SSLSessionPersistentCache.h>
#include <wangle/codec/LineBasedFrameDecoder.h>
#include <wangle/codec/StringCodec.h>

using namespace wangle;
using namespace folly;

DEFINE_string(ip, "::1", "Ip address to connect to");
DEFINE_uint32(port, 8080, "Port to connect to");
DEFINE_bool(ssl, true, "Whether to use SSL");
DEFINE_string(cache_file, "", "Session cache file");
DEFINE_string(cert_path, "", "Path to client cert pem");
DEFINE_string(key_path, "", "Path to client cert key pem");
DEFINE_string(ca_path, "", "Path to trusted CA file");

const std::string SESSION_KEY = "test_client";

/**
 * This is meant to be a simple client that can be configured for SSL or
 * plaintext.  To test:
 * openssl s_server -key <key_path> -cert <cert_path> -accept <port>
 * OR: ./server --cert_path <cert_path> --key_path <key_path>
 *
 * Run ./client and enter text
 *
 * Test cert and key are available in wangle/ssl/test/certs
 */
namespace {
typedef Pipeline<folly::IOBufQueue&, std::string> EchoPipeline;

// the handler for receiving messages back from the server
class EchoHandler : public HandlerAdapter<std::string> {
 public:
  void read(Context*, std::string msg) override {
    std::cout << "received back: " << msg;
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

// chains the handlers together to define the response pipeline
class EchoPipelineFactory : public PipelineFactory<EchoPipeline> {
 public:
  EchoPipeline::Ptr newPipeline(std::shared_ptr<AsyncTransport> sock) override {
    auto pipeline = EchoPipeline::create();
    pipeline->addBack(AsyncSocketHandler(sock));
    pipeline->addBack(
        EventBaseHandler()); // ensure we can write from any thread
    pipeline->addBack(LineBasedFrameDecoder(8192, false));
    pipeline->addBack(StringCodec());
    pipeline->addBack(EchoHandler());
    pipeline->finalize();
    return pipeline;
  }
};

class EchoClientBootstrap : public ClientBootstrap<EchoPipeline> {
 public:
  void makePipeline(std::shared_ptr<folly::AsyncTransport> socket) override {
    auto sslSock = socket->getUnderlyingTransport<AsyncSSLSocket>();
    if (sslSock) {
      sslSock->setSessionKey(SESSION_KEY);
    }
    ClientBootstrap<EchoPipeline>::makePipeline(std::move(socket));
  }
};

std::shared_ptr<SSLContext> createSSLContext() {
  auto context = std::make_shared<SSLContext>();
  if (!FLAGS_ca_path.empty()) {
    context->loadTrustedCertificates(FLAGS_ca_path.c_str());
    // don't do peer name validation
    context->authenticate(true, false);
    // verify the server cert
    context->setVerificationOption(SSLContext::SSLVerifyPeerEnum::VERIFY);
  }
  if (!FLAGS_cert_path.empty() && !FLAGS_key_path.empty()) {
    context->loadCertificate(FLAGS_cert_path.c_str());
    context->loadPrivateKey(FLAGS_key_path.c_str());
    if (!context->isCertKeyPairValid()) {
      throw std::runtime_error("Cert and key do not match");
    }
  }
  folly::ssl::setSignatureAlgorithms<folly::ssl::SSLCommonOptions>(*context);
  return context;
}
} // namespace

int main(int argc, char** argv) {
  folly::Init init(&argc, &argv);
  folly::ssl::init();

  // an in memory ssl session cache used for caching sessions
  std::shared_ptr<SSLSessionPersistentCache> cache;
  if (!FLAGS_cache_file.empty()) {
    cache = std::make_shared<SSLSessionPersistentCache>(
        FLAGS_cache_file,
        wangle::PersistentCacheConfig::Builder()
            .setCapacity(100)
            .setSyncInterval(std::chrono::seconds(60))
            .build());
  }

  EchoClientBootstrap client;
  client.group(std::make_shared<IOThreadPoolExecutor>(1));
  client.pipelineFactory(std::make_shared<EchoPipelineFactory>());

  if (FLAGS_ssl) {
    auto ctx = createSSLContext();
    // attach the context to the cache
    if (cache) {
      wangle::SSLSessionCallbacks::attachCallbacksToContext(
          ctx.get(), cache.get());
    }
    client.sslContext(ctx);
  }

  SocketAddress addr(FLAGS_ip.c_str(), FLAGS_port);
  VLOG(0) << "Connecting";
  auto pipeline = client.connect(addr).get();
  VLOG(0) << "Connected";
  try {
    while (true) {
      std::string line;
      std::getline(std::cin, line);
      if (line == "") {
        VLOG(0) << "End";
        break;
      }
      VLOG(0) << "Sending " << line;
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
