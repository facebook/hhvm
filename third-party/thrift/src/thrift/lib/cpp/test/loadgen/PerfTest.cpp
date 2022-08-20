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

/*
 * This is simple program to test how fast the loadgen infrastructure can run.
 * It provides a really simple Worker class that does nothing at all; therefore
 * the performance of the program should be limited only by the speed of the
 * loadgen library code.
 */

#include <thrift/lib/cpp/concurrency/Util.h>

#include <thrift/lib/cpp/test/loadgen/QpsMonitor.h>
#include <thrift/lib/cpp/test/loadgen/Worker.h>
#include <thrift/lib/cpp/test/loadgen/loadgen.h>

#include "common/config/Flags.h"

DEFINE_double(interval, 1.0, "number of seconds between statistics output");
DEFINE_int32(ops_per_conn, 1, "number of operations per connection");
DEFINE_int32(num_threads, 8, "number of worker threads");
DEFINE_bool(qps_monitor, false, "use the simple QPS monitor");

using std::shared_ptr;
namespace loadgen = apache::thrift::loadgen;

class PerfLoadConfig : public loadgen::LoadConfig {
 public:
  uint32_t getNumOpTypes() const override { return 1; }

  uint32_t pickOpType() override { return 0; }

  uint32_t pickOpsPerConnection() override { return FLAGS_ops_per_conn; }

  uint32_t getNumWorkerThreads() const override { return FLAGS_num_threads; }

  std::string getOpName(uint32_t /* opType */) override { return "noop"; }
};

class PerfWorker : public loadgen::Worker<void, PerfLoadConfig> {
 public:
  shared_ptr<void> createConnection() override { return shared_ptr<void>(); }

  void performOperation(
      const shared_ptr<void>& /* client */, uint32_t /* op */) override {
    return;
  }
};

int main(int argc, char* argv[]) {
  facebook::config::Flags::initFlags(&argc, &argv, true);

  if (argc != 1) {
    fprintf(stderr, "error: unhandled arguments:");
    for (int n = 1; n < argc; ++n) {
      fprintf(stderr, " %s", argv[n]);
    }
    fprintf(stderr, "\n");
    return 1;
  }

  shared_ptr<PerfLoadConfig> config(new PerfLoadConfig);

  if (FLAGS_qps_monitor) {
    loadgen::QpsMonitor monitor(config);
    loadgen::runLoadGen<PerfWorker>(config, FLAGS_interval, &monitor);
  } else {
    loadgen::runLoadGen<PerfWorker>(config, FLAGS_interval);
  }

  return 0;
}
