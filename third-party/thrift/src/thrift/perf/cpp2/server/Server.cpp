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

#include <glog/logging.h>

#include <folly/init/Init.h>
#include <folly/portability/GFlags.h>
#include <folly/system/HardwareConcurrency.h>

#include <proxygen/httpserver/HTTPServerOptions.h>
#include <thrift/lib/cpp2/server/ThriftServer.h>
#include <thrift/lib/cpp2/transport/http2/common/HTTP2RoutingHandler.h>
#include <thrift/perf/cpp2/server/BenchmarkHandler.h>
#include <thrift/perf/cpp2/util/QPSStats.h>

#include <unistd.h>
#include <thread>

DEFINE_int32(port, 7777, "Server port");
DEFINE_string(
    unix_socket_path, "", "Unix socket to listen on, supersedes port");

DEFINE_int32(io_threads, 0, "Number of IO threads (0 means number of cores)");
DEFINE_int32(cpu_threads, 0, "Number of CPU threads (0 means number of cores)");
DEFINE_int32(stats_interval_sec, 1, "Seconds between stats");
DEFINE_int32(terminate_sec, 0, "How long to run server (0 means forever)");

using apache::thrift::HTTP2RoutingHandler;
using apache::thrift::ThriftServer;
using apache::thrift::ThriftServerAsyncProcessorFactory;
using facebook::thrift::benchmarks::BenchmarkHandler;
using facebook::thrift::benchmarks::QPSStats;
using proxygen::HTTPServerOptions;
using std::thread;

std::unique_ptr<HTTP2RoutingHandler> createHTTP2RoutingHandler(
    std::shared_ptr<ThriftServer> server) {
  auto h2_options = std::make_unique<HTTPServerOptions>();
  h2_options->threads = static_cast<size_t>(server->getNumIOWorkerThreads());
  h2_options->idleTimeout = server->getIdleTimeout();
  h2_options->shutdownOn = {SIGINT, SIGTERM};
  return std::make_unique<HTTP2RoutingHandler>(
      std::move(h2_options), server->getThriftProcessor(), *server);
}

int main(int argc, char** argv) {
  const folly::Init init(&argc, &argv);

  int32_t numCores = folly::available_concurrency();
  if (FLAGS_io_threads == 0) {
    FLAGS_io_threads = numCores;
  }
  if (FLAGS_cpu_threads == 0) {
    FLAGS_cpu_threads = numCores;
  }
  LOG(INFO) << "Using " << FLAGS_io_threads << " IO threads";
  LOG(INFO) << "Using " << FLAGS_cpu_threads << " CPU threads";

  QPSStats stats;

  auto handler = std::make_shared<BenchmarkHandler>(&stats);
  auto cpp2PFac =
      std::make_shared<ThriftServerAsyncProcessorFactory<BenchmarkHandler>>(
          handler);

  auto server = std::make_shared<ThriftServer>();
  if (!FLAGS_unix_socket_path.empty()) {
    folly::AsyncServerSocket::UniquePtr sock{new folly::AsyncServerSocket};
    folly::SocketAddress addr;
    addr.setFromPath(FLAGS_unix_socket_path);
    sock->bind(addr);
    server->useExistingSocket(std::move(sock));
    LOG(INFO) << "Listening on " << FLAGS_unix_socket_path;
  } else {
    LOG(INFO) << "Listening on port " << FLAGS_port;
    server->setPort(FLAGS_port);
  }
  server->setNumIOWorkerThreads(FLAGS_io_threads);
  server->setNumCPUWorkerThreads(FLAGS_cpu_threads);
  server->setInterface(cpp2PFac);

  server->addRoutingHandler(createHTTP2RoutingHandler(server));

  thread logger([&] {
    int32_t elapsedTimeSec = 0;
    if (FLAGS_terminate_sec == 0) {
      // Essentially infinite time.
      FLAGS_terminate_sec = 100000000;
    }
    for (;;) {
      int32_t sleepTimeSec = std::min(
          FLAGS_terminate_sec - elapsedTimeSec, FLAGS_stats_interval_sec);
      /* sleep override */
      std::this_thread::sleep_for(std::chrono::seconds(sleepTimeSec));
      stats.printStats(sleepTimeSec);
      elapsedTimeSec += sleepTimeSec;
      if (elapsedTimeSec >= FLAGS_terminate_sec) {
        server->stop();
        break;
      }
    }
  });

  server->serve();
  logger.join();

  LOG(INFO) << "Server terminating";
  return 0;
}
