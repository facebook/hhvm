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

#include <thrift/conformance/stresstest/StressTest.h>

#include <folly/init/Init.h>
#include <folly/memory/IoUringArena.h>
#ifdef __linux__
#include <common/network/NapiId.h>
#endif
#include <thrift/conformance/stresstest/util/IoUringUtil.h>

#include <thrift/conformance/stresstest/client/TestRunner.h>
#include <thrift/lib/cpp2/transport/rocket/payload/PayloadSerializer.h>

namespace apache::thrift::stress {
namespace {
constexpr size_t kIoUringArenaSize = 256 * 1024 * 1024;
}

using namespace apache::thrift::rocket;

// Stress tests may override the default main() behavior by defining this weak
// symbol.
FOLLY_ATTR_WEAK int customStressTestMain(int argc, char* argv[])
// Mac doesn't like undefined weak symbols, but our dev-mode linker doesn't like
// defined weak symbols. ¯\_(ツ)_/¯
#ifdef __APPLE__
{
  (void)argc;
  (void)argv;
  return 0;
}
#else
    ;
#endif

#ifdef __linux__
int resolve_napi_callback(int ifindex, uint32_t queueId) {
  return facebook::network::resolveQueueNapiId(ifindex, queueId);
}

int src_port_callback(
    const folly::IPAddress& destAddr,
    uint16_t destPort,
    int targetQueueId,
    const char* ifname,
    uint16_t startPort,
    uint16_t minPort,
    uint16_t maxPort) {
  return facebook::network::findSrcPortForQueueId(
      destAddr,
      destPort,
      static_cast<uint32_t>(targetQueueId),
      ifname,
      startPort,
      minPort,
      maxPort);
}
#endif

} // namespace apache::thrift::stress

using namespace apache::thrift::stress;

int main(int argc, char* argv[]) {
  const folly::Init init(&argc, &argv);

  if (&customStressTestMain) {
    return customStressTestMain(argc, argv);
  }

  // create a test runner instance
  auto clientCfg = ClientConfig::createFromFlags();
  if (FLAGS_io_zctx) {
    if (!folly::IoUringArena::init(kIoUringArenaSize)) {
      LOG(WARNING) << "Failed to initialize IoUringArena";
    }
  }

  TestRunner testRunner(std::move(clientCfg));
  testRunner.runTests();

  return 0;
}
