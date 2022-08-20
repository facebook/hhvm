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

#ifndef THRIFT_TEST_LOADGEN_CONTROLLER_H_
#define THRIFT_TEST_LOADGEN_CONTROLLER_H_ 1

#include <condition_variable>
#include <mutex>

#include <thrift/lib/cpp/test/loadgen/IntervalTimer.h>
#include <thrift/lib/cpp/test/loadgen/LoadConfig.h>

namespace apache {
namespace thrift {

namespace concurrency {

class PosixThreadFactory;

} // namespace concurrency

namespace loadgen {

class WorkerFactory;
class WorkerIf;
class Monitor;

class Controller {
 public:
  Controller(
      WorkerFactory* factory,
      Monitor* monitor,
      std::shared_ptr<LoadConfig> config,
      apache::thrift::concurrency::PosixThreadFactory* threadFactory = nullptr);

  Controller(const Controller&) = delete;
  Controller& operator=(const Controller&) = delete;

  void run(
      uint32_t numThreads, uint32_t maxThreads, double monitorInterval = 1.0);

 private:
  class WorkerRunner;
  typedef std::vector<std::shared_ptr<WorkerIf>> WorkerVector;

  void createWorkerThreads(uint32_t numThreads);
  void startWorkers(uint32_t numThreads);
  void runMonitor(double interval);
  std::shared_ptr<WorkerIf> createWorker();

  std::mutex initMutex_;
  std::condition_variable initCondVar_;
  uint32_t numThreads_;
  uint32_t maxThreads_;
  WorkerFactory* workerFactory_;
  Monitor* monitor_;
  WorkerVector workers_;
  IntervalTimer intervalTimer_;
  std::shared_ptr<LoadConfig> config_;
  apache::thrift::concurrency::PosixThreadFactory* threadFactory_;
};

} // namespace loadgen
} // namespace thrift
} // namespace apache

#endif // THRIFT_TEST_LOADGEN_CONTROLLER_H_
