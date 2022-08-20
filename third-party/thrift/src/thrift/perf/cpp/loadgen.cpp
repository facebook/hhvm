/*
 * Copyright (c) Facebook, Inc. and its affiliates.
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

#include <thrift/lib/cpp/test/loadgen/loadgen.h>

#include <signal.h>

#include <folly/Random.h>
#include <folly/init/Init.h>

#include <thrift/lib/cpp/test/loadgen/QpsMonitor.h>
#include <thrift/lib/cpp/test/loadgen/RNG.h>
#include <thrift/perf/cpp/AsyncClientWorker2.h>
#include <thrift/perf/cpp/ClientLoadConfig.h>
#include <thrift/perf/cpp/ClientWorker2.h>

DEFINE_double(interval, 1.0, "number of seconds between statistics output");

using namespace apache::thrift;
using namespace apache::thrift::test;

int main(int argc, char* argv[]) {
  folly::init(&argc, &argv, true);

  signal(SIGINT, exit);

  if (argc != 1) {
    fprintf(stderr, "error: unhandled arguments:");
    for (int n = 1; n < argc; ++n) {
      fprintf(stderr, " %s", argv[n]);
    }
    fprintf(stderr, "\n");
    return 1;
  }

  loadgen::RNG::setGlobalSeed(folly::Random::rand64());

  std::shared_ptr<ClientLoadConfig> config(new ClientLoadConfig);
  if (config->useAsync()) {
    loadgen::runLoadGen<apache::thrift::AsyncClientWorker2>(
        config, FLAGS_interval);
  } else {
    loadgen::runLoadGen<ClientWorker2>(config, FLAGS_interval);
  }

  return 0;
}
