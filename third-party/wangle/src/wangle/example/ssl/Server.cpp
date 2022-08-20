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

#include <folly/init/Init.h>
#include <folly/io/async/AsyncSSLSocket.h>
#include <folly/portability/GFlags.h>
#include <folly/ssl/Init.h>
#include <wangle/acceptor/Acceptor.h>
#include <wangle/bootstrap/ServerBootstrap.h>
#include <wangle/channel/AsyncSocketHandler.h>
#include <wangle/codec/LineBasedFrameDecoder.h>
#include <wangle/codec/StringCodec.h>
#include <wangle/ssl/TLSCredProcessor.h>

DEFINE_string(cert_path, "", "Path to cert pem");
DEFINE_string(key_path, "", "Path to cert key");
DEFINE_string(ca_path, "", "Path to trusted CA file");
DEFINE_int32(port, 8080, "Listen port");
DEFINE_string(tickets_path, "", "Path for ticket seeds");
DEFINE_uint32(num_workers, 2, "Number of worker threads");
DEFINE_bool(
    enable_share_ssl_ctx,
    true,
    "Enable sharing SSL context configs between worker threads");

/**
 * This is meant to be a simple server that accepts plaintext and SSL on a
 * single port.  To test:
 * plaintext: nc localhost 8080
 * ssl: openssl s_client -connect localhost:8080
 *
 * Test cert and key are available in wangle/ssl/test/certs
 */
using namespace wangle;
using namespace folly;

typedef Pipeline<IOBufQueue&, std::string> EchoPipeline;

namespace {
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

// Init the processor callbacks.  It's fine to do this
// even if nothing is being watched
void initCredProcessorCallbacks(
    ServerBootstrap<EchoPipeline>& sb,
    TLSCredProcessor& processor) {
  // set up ticket seed callback
  processor.addTicketCallback([&](TLSTicketKeySeeds seeds) {
    if (FLAGS_enable_share_ssl_ctx) {
      sb.getSharedSSLContextManager()->updateTLSTicketKeys(seeds);
    } else {
      // update
      sb.forEachWorker([&](Acceptor* acceptor) {
        if (!acceptor) {
          // this condition can happen if the processor triggers before the
          // server is ready / listening
          return;
        }
        auto evb = acceptor->getEventBase();
        if (!evb) {
          return;
        }
        evb->runInEventBaseThread([acceptor, seeds] {
          acceptor->setTLSTicketSecrets(
              seeds.oldSeeds, seeds.currentSeeds, seeds.newSeeds);
        });
      });
    }
  });

  // Reconfigure SSL when we detect cert or CA changes.
  processor.addCertCallback([&] {
    if (FLAGS_enable_share_ssl_ctx) {
      sb.getSharedSSLContextManager()->reloadSSLContextConfigs();
    } else {
      sb.forEachWorker([&](Acceptor* acceptor) {
        if (!acceptor) {
          return;
        }
        auto evb = acceptor->getEventBase();
        if (!evb) {
          return;
        }
        evb->runInEventBaseThread(
            [acceptor] { acceptor->resetSSLContextConfigs(); });
      });
    }
  });
}
} // namespace

int main(int argc, char** argv) {
  folly::Init init(&argc, &argv);
  folly::ssl::init();

  ServerSocketConfig cfg;
  folly::Optional<TLSTicketKeySeeds> seeds;

  ServerBootstrap<EchoPipeline> sb;
  TLSCredProcessor processor;

  if (!FLAGS_tickets_path.empty()) {
    seeds = TLSCredProcessor::processTLSTickets(FLAGS_tickets_path);
    if (seeds) {
      cfg.initialTicketSeeds = *seeds;
      // watch for changes
      processor.setTicketPathToWatch(FLAGS_tickets_path);
    }
  }

  if (!FLAGS_cert_path.empty() && !FLAGS_key_path.empty()) {
    VLOG(0) << "Configuring SSL";
    SSLContextConfig sslCfg;
    sslCfg.addCertificate(FLAGS_cert_path, FLAGS_key_path, "");
    sslCfg.clientCAFile = FLAGS_ca_path;
    sslCfg.isDefault = true;
    cfg.sslContextConfigs.push_back(sslCfg);
    // IMPORTANT: when allowing both plaintext and ssl on the same port,
    // the acceptor requires 9 bytes of data to determine what kind of
    // connection is coming in.  If the client does not send 9 bytes the
    // connection will idle out before the EchoCallback receives data.
    cfg.allowInsecureConnectionsOnSecureServer = true;

    // reload ssl contexts when certs change
    std::set<std::string> pathsToWatch{FLAGS_cert_path, FLAGS_key_path};
    if (!FLAGS_ca_path.empty()) {
      pathsToWatch.insert(FLAGS_ca_path);
    }
    processor.setCertPathsToWatch(std::move(pathsToWatch));
  }

  initCredProcessorCallbacks(sb, processor);

  // workers
  auto workers =
      std::make_shared<folly::IOThreadPoolExecutor>(FLAGS_num_workers);

  // create a server
  sb.acceptorConfig(cfg);
  sb.childPipeline(std::make_shared<EchoPipelineFactory>());
  sb.setUseSharedSSLContextManager(FLAGS_enable_share_ssl_ctx);
  sb.group(workers);

  sb.bind(FLAGS_port);
  sb.waitForStop();
  return 0;
}
