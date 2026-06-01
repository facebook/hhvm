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

#include <gflags/gflags.h>
#include <folly/init/Init.h>
#include <folly/logging/xlog.h>
#include <thrift/lib/cpp2/fast_thrift/bench/runner/ServerInterface.h>
#include <thrift/lib/cpp2/fast_thrift/bench/tcp/TcpServer.h>

#include <csignal>
#include <memory>

DEFINE_int32(port, 0, "Port to listen on (0 = system assigned)");
DEFINE_int32(io_threads, 8, "Number of IO threads");
DEFINE_string(test_type, "tcp", "Type of benchmark to run (tcp, rocket, etc.)");
DEFINE_uint64(
    zero_copy_threshold,
    0,
    "Minimum payload size in bytes for MSG_ZEROCOPY (0 = disabled)");

namespace {

std::unique_ptr<apache::thrift::fast_thrift::bench::Server> gServer;

void signalHandler(int sig) {
  XLOG(INFO) << "Received signal " << sig << ", shutting down...";
  if (gServer) {
    gServer->stop();
  }
}

std::unique_ptr<apache::thrift::fast_thrift::bench::Server> createServer(
    const std::string& testType,
    folly::SocketAddress address,
    uint32_t numIOThreads,
    size_t zeroCopyThreshold) {
  if (testType == "tcp") {
    return std::make_unique<apache::thrift::fast_thrift::bench::TcpServer>(
        std::move(address), numIOThreads, zeroCopyThreshold);
  } else {
    LOG(FATAL) << "Unknown test type: " << testType;
  }
}

} // namespace

int main(int argc, char* argv[]) {
  folly::Init init(&argc, &argv);

  std::signal(SIGINT, signalHandler);
  std::signal(SIGTERM, signalHandler);

  folly::SocketAddress address;
  address.setFromLocalPort(FLAGS_port);

  gServer = createServer(
      FLAGS_test_type,
      std::move(address),
      FLAGS_io_threads,
      static_cast<size_t>(FLAGS_zero_copy_threshold));
  gServer->start();

  XLOG(INFO) << "Server started on " << gServer->getAddress();
  XLOG(INFO) << "Test type: " << FLAGS_test_type;

  pause();

  return 0;
}
