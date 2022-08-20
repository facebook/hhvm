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

#ifndef THRIFT_TEST_LOADGEN_LOADCONFIG_H_
#define THRIFT_TEST_LOADGEN_LOADCONFIG_H_ 1

#include <inttypes.h>
#include <string>
#include <boost/random.hpp>

namespace apache {
namespace thrift {
namespace loadgen {

class LoadConfig {
 public:
  virtual ~LoadConfig() {}

  virtual uint32_t getNumOpTypes() const = 0;

  virtual uint32_t pickOpType() = 0;
  virtual uint32_t pickOpsPerConnection() = 0;

  /**
   * Return a human-readable name for an operation type.
   *
   * By default, just converts the integer value to a string, but this may be
   * overridden by subclasses.
   */
  virtual std::string getOpName(uint32_t opType);

  /**
   * Get the number of worker threads to run.
   */
  virtual uint32_t getNumWorkerThreads() const = 0;

  /**
   * Get the desired number of queries per second.
   *
   * The workers should attempt to perform only this many operations per
   * second.  (It is possible that they will perform less than this if they
   * cannot drive the requested qps rate.)
   *
   * @return Return the desired qps rate, or 0 if operations should be
   *         performed as fast as possible.
   */
  virtual uint64_t getDesiredQPS() const { return 0; }

  /**
   * Get the max number of worker threads to run.
   *
   * This enables loadgen to increase worker threads up to this number
   * in order to drive desired QPS.
   */
  virtual uint32_t getMaxWorkerThreads() const { return getNumWorkerThreads(); }
};

} // namespace loadgen
} // namespace thrift
} // namespace apache

#endif // THRIFT_TEST_LOADGEN_LOADCONFIG_H_
