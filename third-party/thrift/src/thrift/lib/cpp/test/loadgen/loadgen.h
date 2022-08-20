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

#ifndef THRIFT_TEST_LOADGEN_LOADGEN_H_
#define THRIFT_TEST_LOADGEN_LOADGEN_H_ 1

#include <thrift/lib/cpp/test/loadgen/Worker.h>

namespace apache {
namespace thrift {

namespace concurrency {

class PosixThreadFactory;

} // namespace concurrency

namespace loadgen {

class LoadConfig;
class Monitor;

/**
 * Run load generation.
 *
 * @param factory     The WorkerFactory to use to create each worker thread.
 * @param config      The LoadConfig object that describes the operations.
 * @param interval    The number of seconds between each line of statistics
 *                    output.
 * @param monitor     The Monitor to use for printing statistics.  If nullptr,
 *                    a LatencyMonitor will be used.
 */
void runLoadGen(
    WorkerFactory* factory,
    const std::shared_ptr<LoadConfig>& config,
    double interval = 1.0,
    Monitor* monitor = nullptr,
    apache::thrift::concurrency::PosixThreadFactory* threadFactory = nullptr);

/**
 * Run load generation.
 *
 * This is a helper function around runLoadGen() that automatically creates a
 * SimpleWorkerFactory.
 */
template <typename WorkerT, typename ConfigT>
void runLoadGen(
    const std::shared_ptr<ConfigT>& config,
    double interval = 1.0,
    Monitor* monitor = nullptr,
    apache::thrift::concurrency::PosixThreadFactory* threadFactory = nullptr) {
  SimpleWorkerFactory<WorkerT, ConfigT> factory(config);
  runLoadGen(&factory, config, interval, monitor, threadFactory);
}

/**
 * Run load generation.
 *
 * This is a helper function around runLoadGen() that automatically creates a
 * SimpleWorkerFactory.
 */
template <typename WorkerT>
void runLoadGen(
    const std::shared_ptr<LoadConfig>& config,
    double interval = 1.0,
    Monitor* monitor = nullptr,
    apache::thrift::concurrency::PosixThreadFactory* threadFactory = nullptr) {
  runLoadGen<WorkerT, LoadConfig>(config, interval, monitor, threadFactory);
}

} // namespace loadgen
} // namespace thrift
} // namespace apache

#endif // THRIFT_TEST_LOADGEN_LOADGEN_H_
