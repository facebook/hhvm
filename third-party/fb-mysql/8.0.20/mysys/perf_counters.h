// Copyright 2004-present Facebook. All Rights Reserved.

#ifndef PERF_COUNTERS_H_INCLUDED
#define PERF_COUNTERS_H_INCLUDED

#include <cstdint>

#include <memory>
#include <string>

namespace utils {

enum class PerfCounterMode {
  THREAD = 1,
  PROCESS = 2,
};

enum class PerfCounterType {
  INSTRUCTIONS = 1,
  CPU_CYCLES = 2,
};

class PerfCounter {
 public:
  virtual ~PerfCounter() noexcept {}

  // call start() in order to start counter
  virtual void start() noexcept = 0;

  // call stop() in order to stop counter
  virtual void stop() noexcept = 0;

  // get() will return the accumulated counter value
  // since start() or getAndRestart() call.
  // get() will not reset the counter,
  // so consecutive get() calls will return increasing values
  virtual uint64_t get() const noexcept = 0;

  // similar to get() but getAndRestart() resets the counter.
  virtual uint64_t getAndRestart() noexcept = 0;

  // similar to get() but getAndStop() stops the counter.
  virtual uint64_t getAndStop() noexcept = 0;

  // returns the multiplex factor
  virtual float getMultiplexFactor() const noexcept = 0;
};

class PerfCounterFactory {
 public:
  virtual ~PerfCounterFactory() noexcept {}

  static std::shared_ptr<PerfCounterFactory> getFactory(
      const std::string &factory_name);

  virtual std::shared_ptr<PerfCounter> makeSharedPerfCounter(
      PerfCounterMode mode, PerfCounterType c_type) const = 0;

  virtual std::unique_ptr<PerfCounter> makePerfCounter(
      PerfCounterMode mode, PerfCounterType c_type) const = 0;
};

}  // namespace utils

#endif  // #ifndef PERF_COUNTERS_H_INCLUDED
