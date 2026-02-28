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

#include <folly/Benchmark.h>
#include <folly/init/Init.h>
#include <thrift/lib/cpp2/test/gen-cpp2/TestService.h>
#include <thrift/lib/cpp2/util/ScopedServerInterfaceThread.h>

using namespace apache::thrift;
using namespace apache::thrift::test;

struct Handler : TestServiceSvNull {
  void async_eb_eventBaseAsync(
      HandlerCallbackPtr<std::unique_ptr<std::string>> cb) override {
    cb->result(std::make_unique<std::string>());
  }
};

template <bool isHeader, bool isEb>
void bench(size_t iters) {
  folly::BenchmarkSuspender susp;
  ScopedServerInterfaceThread runner(std::make_shared<Handler>());
  folly::EventBase eb;
  auto sock = folly::AsyncSocket::UniquePtr(
      new folly::AsyncSocket(&eb, runner.getAddress()));
  auto chan = isHeader
      ? RequestChannel::Ptr(
            HeaderClientChannel::newChannel(
                HeaderClientChannel::WithoutRocketUpgrade{}, std::move(sock)))
      : RocketClientChannel::newChannel(std::move(sock));
  TestServiceAsyncClient client(std::move(chan));

  client.sync_echoInt(0); // Set up connection
  std::string out;
  susp.dismiss();

  while (iters--) {
    if (isEb) {
      client.sync_eventBaseAsync(out);
    } else {
      client.sync_sendResponse(out, 0);
    }
  }

  susp.rehire();
}

BENCHMARK(Header_tm_Rpc_Bench, iters) {
  bench<true, false>(iters);
}
BENCHMARK(Header_eb_Rpc_Bench, iters) {
  bench<true, true>(iters);
}
BENCHMARK(Rocket_tm_Rpc_Bench, iters) {
  bench<false, false>(iters);
}
BENCHMARK(Rocket_eb_Rpc_Bench, iters) {
  bench<false, true>(iters);
}

int main(int argc, char** argv) {
  folly::Init init(&argc, &argv);
  folly::runBenchmarks();
  return 0;
}
