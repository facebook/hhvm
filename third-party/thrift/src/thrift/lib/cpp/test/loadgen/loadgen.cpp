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

#include <boost/scoped_ptr.hpp>
#include <thrift/lib/cpp/test/loadgen/Controller.h>
#include <thrift/lib/cpp/test/loadgen/LatencyMonitor.h>

using namespace boost;

namespace apache {
namespace thrift {
namespace loadgen {

void runLoadGen(
    WorkerFactory* factory,
    const std::shared_ptr<LoadConfig>& config,
    double interval,
    Monitor* monitor,
    apache::thrift::concurrency::PosixThreadFactory* threadFactory) {
  scoped_ptr<LatencyMonitor> defaultMonitor;
  if (monitor == nullptr) {
    defaultMonitor.reset(new LatencyMonitor(config));
    monitor = defaultMonitor.get();
  }

  Controller controller(factory, monitor, config, threadFactory);
  controller.run(
      config->getNumWorkerThreads(), config->getMaxWorkerThreads(), interval);
}

} // namespace loadgen
} // namespace thrift
} // namespace apache
