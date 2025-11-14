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
#ifdef __linux__
#include <ynl/generated/netdev-user.hpp>
#include <ynl/lib/ynl.hpp>
#endif

#include <thrift/conformance/stresstest/client/TestRunner.h>
#include <thrift/lib/cpp2/transport/rocket/payload/PayloadSerializer.h>

namespace apache::thrift::stress {

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
  ynl_error yerr;
  ynl_cpp::ynl_socket ys(ynl_cpp::get_ynl_netdev_family(), &yerr);
  if (!ys) {
    throw std::runtime_error("no netlink");
  }

  ynl_cpp::netdev_queue_get_req_dump req;
  req.ifindex = ifindex;
  auto resp = ynl_cpp::netdev_queue_get_dump(ys, req);
  for (auto const& queue : resp->objs) {
    if (queue.id.value_or(-1) == queueId) {
      if (!queue.napi_id.has_value()) {
        throw std::runtime_error(
            folly::to<std::string>("queue ", queueId, " has no napi id"));
      }
      return static_cast<int>(queue.napi_id.value());
    }
  }
  return -1;
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

  TestRunner testRunner(std::move(clientCfg));
  testRunner.runTests();

  return 0;
}
