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

#include <sys/resource.h>

#include <gflags/gflags.h>
#include <glog/logging.h>

#include <fizz/client/AsyncFizzClient.h>
#include <folly/FileUtil.h>
#include <folly/Random.h>
#include <folly/init/Init.h>
#include <folly/io/async/AsyncSocket.h>
#include <folly/io/async/EventBase.h>
#include <thrift/lib/cpp2/async/HeaderClientChannel.h>
#include <thrift/lib/cpp2/async/RocketClientChannel.h>

#include <fizz/protocol/CertUtils.h>
#include <thrift/test/integration/cpp2/gen-cpp2/ZeroCopyServiceAsyncClient.h>

enum ChannelType {
  CHANNEL_TYPE_HEADER = 0,
  CHANNEL_TYPE_ROCKET = 1,
};

DEFINE_string(server, "::1", "Addr for the thift server");
DEFINE_int32(port, 7878, "Port for the thrift server");
DEFINE_int32(type, CHANNEL_TYPE_HEADER, "Client channel type");
DEFINE_int32(num, 1, "Number of iterations");
DEFINE_int32(size, 4096, "Payload size");
DEFINE_int32(threshold, 32 * 1024, "Zerocopy threshold");
DEFINE_bool(debug_logs, false, "Debug logs");
DEFINE_bool(use_crypto, true, "Use crypto");
DEFINE_bool(verify, true, "Verify reply");
DEFINE_string(
    cert, "", "Path to file containing PEM encoded client certificate");
DEFINE_string(
    key,
    "",
    "Path to file containing PEM encoded key corresponding to client certificate. If unset and -cert is set, will use the value given in -cert");
using namespace thrift::zerocopy::cpp2;

namespace {

class Client {
 public:
  Client(const std::string& server, int port) {
    folly::SocketAddress addr(server, port, true);
    std::shared_ptr<fizz::client::FizzClientContext> fizzCtx;
    folly::AsyncTransport::UniquePtr socket;
    if (FLAGS_use_crypto) {
      auto context = std::make_shared<fizz::client::FizzClientContext>();
      std::vector<std::string> alpns;
      if (FLAGS_type == CHANNEL_TYPE_ROCKET) {
        alpns.push_back("rs");
      } else {
        alpns.push_back("thrift");
      }
      context->setSupportedAlpns(std::move(alpns));
      if (FLAGS_cert.size() > 0) {
        std::string cert;
        std::string key;
        CHECK(folly::readFile(FLAGS_cert.c_str(), cert));
        const char* keypath =
            FLAGS_key.size() > 0 ? FLAGS_key.c_str() : FLAGS_cert.c_str();
        CHECK(folly::readFile(keypath, key));
        context->setClientCertificate(
            fizz::CertUtils::makeSelfCert(std::move(cert), std::move(key)));
      }
      auto* fizzClient =
          new fizz::client::AsyncFizzClient(&evb_, std::move(context));
      socket.reset(fizzClient);
      fizzClient->connect(
          addr,
          nullptr,
          nullptr,
          folly::Optional<std::string>(),
          folly::Optional<std::string>());

    } else {
      socket.reset(
          new folly::AsyncSocket(&evb_, addr, 0, (FLAGS_threshold > 0)));
    }

    if (FLAGS_threshold > 0) {
      LOG(INFO) << "Adding zerocopy enable func with threshold = "
                << FLAGS_threshold;
      socket->setZeroCopyEnableFunc(
          [](const std::unique_ptr<folly::IOBuf>& buf) {
            auto len = static_cast<int>(buf->computeChainDataLength());
            if (FLAGS_debug_logs) {
              LOG(INFO) << len << "-" << FLAGS_threshold;
            }
            return len >= FLAGS_threshold;
          });
    }

    switch (FLAGS_type) {
      case CHANNEL_TYPE_ROCKET: {
        auto channel =
            apache::thrift::RocketClientChannel::newChannel(std::move(socket));
        client_ =
            std::make_unique<ZeroCopyServiceAsyncClient>(std::move(channel));
        break;
      }
      default: {
        auto channel =
            apache::thrift::HeaderClientChannel::newChannel(std::move(socket));
        client_ =
            std::make_unique<ZeroCopyServiceAsyncClient>(std::move(channel));
      }
    }
  }

  ~Client() {}

  void run(int num) {
    std::unique_ptr<uint8_t[]> s(new uint8_t[FLAGS_size]);
    std::mt19937 rnd(folly::randomNumberSeed());
    size_t n = FLAGS_size;
    uint8_t* wr = s.get();
    while (n) {
      uint32_t r = rnd();
      size_t len = (n > sizeof(uint32_t)) ? sizeof(uint32_t) : n;
      ::memcpy(wr, &r, len);
      wr += len;

      n -= len;
    }

    auto data = folly::IOBuf::copyBuffer(s.get(), FLAGS_size);
    for (int i = 0; i < num; i++) {
      folly::IOBuf ret;
      client_->sync_echo(ret, *data);
      if (FLAGS_verify) {
        ret.coalesce();
        CHECK_EQ(data->length(), ret.length());
        auto res = ::memcmp(data->data(), ret.data(), data->length());
        CHECK_EQ(res, 0);
      }

      if (FLAGS_debug_logs) {
        LOG(INFO) << "[" << i << "]: data = " << data->countChainElements()
                  << ":" << data->computeChainDataLength()
                  << " ret = " << ret.countChainElements() << ":"
                  << ret.computeChainDataLength();
      }
    }
  }

 private:
  folly::EventBase evb_;
  std::unique_ptr<ZeroCopyServiceAsyncClient> client_;
};
} // namespace

int main(int argc, char* argv[]) {
  struct rlimit rlim = {
      .rlim_cur = RLIM_INFINITY,
      .rlim_max = RLIM_INFINITY,
  };
  setrlimit(RLIMIT_MEMLOCK, &rlim); // best effort

  folly::init(&argc, &argv);
  Client client(FLAGS_server, FLAGS_port);
  client.run(FLAGS_num);

  return 0;
}
