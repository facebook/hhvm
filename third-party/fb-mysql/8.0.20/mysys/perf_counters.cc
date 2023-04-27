// Copyright 2004-present Facebook. All Rights Reserved.
#include "perf_counters.h"
#include <cassert>
#include <memory>
#include <string>
#include <utility>

#if defined(FB_DYNO) && FB_DYNO
#include <dyno_counters.h>
#endif

#include <ctime>

static inline uint64_t diff_timespec(const timespec &ts1,
                                     const timespec &ts2) noexcept {
  return ((ts1.tv_sec - ts2.tv_sec) * 1000000000ULL + ts1.tv_nsec -
          ts2.tv_nsec);
}

////////////////////////////////////////////////////////////////////////////////
namespace utils {

template <class T>
using unique_ptr = std::unique_ptr<T>;

class SimplePerfCounterFactory : public PerfCounterFactory {
  std::shared_ptr<PerfCounter> makeSharedPerfCounter(
      PerfCounterMode mode, PerfCounterType c_type) const override;

  std::unique_ptr<PerfCounter> makePerfCounter(
      PerfCounterMode mode, PerfCounterType c_type) const override;
};

class ThreadPerfCounterImpl : public PerfCounter {
 private:
  PerfCounterType _counter_type;
  timespec _time_beg;
  bool _started;

  uint64_t get_impl(timespec *time_cur) const noexcept;

 public:
  explicit ThreadPerfCounterImpl(PerfCounterType counterType)
      : _counter_type(counterType), _started(false) {}

  void start() noexcept override;
  void stop() noexcept override { _started = false; }
  uint64_t get() const noexcept override {
    timespec time_cur;
    return get_impl(&time_cur);
  }

  uint64_t getAndRestart() noexcept override {
    timespec time_cur;
    const uint64_t diff_cost = get_impl(&time_cur);
    _time_beg = time_cur;
    _started = true;
    return diff_cost;
  }
  uint64_t getAndStop() noexcept override {
    const uint64_t diff_cost = get();
    _started = false;
    return diff_cost;
  }
  float getMultiplexFactor() const noexcept override { return 1.0; }
};

void ThreadPerfCounterImpl::start() noexcept {
  const int cpu_res = clock_gettime(CLOCK_THREAD_CPUTIME_ID, &_time_beg);
  _started = (cpu_res == 0);
}

uint64_t ThreadPerfCounterImpl::get_impl(timespec *time_cur) const noexcept {
  if (!_started) return 0;

  const int cpu_cur = clock_gettime(CLOCK_THREAD_CPUTIME_ID, time_cur);
  if (cpu_cur != 0) return 0;

  const uint64_t diff_ns = diff_timespec((*time_cur), _time_beg);
  // assuming 2.4 GHz.
  return ((diff_ns * 1.0) / 0.41666);
}

class ProcessPerfCounterImpl : public PerfCounter {
 public:
  explicit ProcessPerfCounterImpl(PerfCounterType) {}

  void start() noexcept override {}
  void stop() noexcept override {}
  uint64_t get() const noexcept override { return 0; }
  uint64_t getAndRestart() noexcept override { return 0; }
  uint64_t getAndStop() noexcept override { return 0; }
  float getMultiplexFactor() const noexcept override { return 1.0; }
};

template <PerfCounterMode>
struct CounterModeToType;

template <>
struct CounterModeToType<PerfCounterMode::THREAD> {
  using type = ThreadPerfCounterImpl;
};

template <>
struct CounterModeToType<PerfCounterMode::PROCESS> {
  using type = ProcessPerfCounterImpl;
};

template <template <typename> class T, class D, class... Args>
struct PerfCounterMaker;

template <class D, class... Args>
struct PerfCounterMaker<std::shared_ptr, D, Args...> {
  static std::shared_ptr<D> make(Args &&... args) {
    return std::make_shared<D>(std::forward<Args>(args)...);
  }
};

template <class D, class... Args>
struct PerfCounterMaker<unique_ptr, D, Args...> {
  static std::unique_ptr<D> make(Args &&... args) {
    return std::unique_ptr<D>(new D(std::forward<Args>(args)...));
  }
};

template <template <typename> class T, PerfCounterMode mode, class... Args>
T<PerfCounter> makePerfCounterBase(PerfCounterType counterType,
                                   Args &&... args) {
  return PerfCounterMaker<
      T, typename CounterModeToType<mode>::type, PerfCounterType,
      Args...>::make(std::forward<PerfCounterType>(counterType),
                     std::forward<Args>(args)...);
}

template <PerfCounterMode mode, class... Args>
std::shared_ptr<PerfCounter> makeSharedPerfCounter(PerfCounterType counterType,
                                                   Args &&... args) {
  return makePerfCounterBase<std::shared_ptr, mode, Args...>(
      counterType, std::forward<Args>(args)...);
}

template <PerfCounterMode mode, class... Args>
std::unique_ptr<PerfCounter> makePerfCounter(PerfCounterType counterType,
                                             Args &&... args) {
  return makePerfCounterBase<unique_ptr, mode, Args...>(
      counterType, std::forward<Args>(args)...);
}

std::shared_ptr<PerfCounter> SimplePerfCounterFactory::makeSharedPerfCounter(
    PerfCounterMode mode, PerfCounterType c_type) const {
  switch (mode) {
    case PerfCounterMode::THREAD:
      return utils::makeSharedPerfCounter<PerfCounterMode::THREAD>(c_type);
    case PerfCounterMode::PROCESS:
      return utils::makeSharedPerfCounter<PerfCounterMode::PROCESS>(c_type);
    default:
      assert(0);
  }
  return nullptr;
}

std::unique_ptr<PerfCounter> SimplePerfCounterFactory::makePerfCounter(
    PerfCounterMode mode, PerfCounterType c_type) const {
  switch (mode) {
    case PerfCounterMode::THREAD:
      return utils::makePerfCounter<PerfCounterMode::THREAD>(c_type);
    case PerfCounterMode::PROCESS:
      return utils::makePerfCounter<PerfCounterMode::PROCESS>(c_type);
    default:
      assert(0);
  }
  return nullptr;
}

std::shared_ptr<PerfCounterFactory> PerfCounterFactory::getFactory(
    const std::string &factory_name) {
#if defined(FB_DYNO) && FB_DYNO
  // this should bleed into FB code and FB libraries
  if (factory_name == "dyno") {
    return DynoPerfCounterFactory();
  }
#endif

  if (factory_name == "simple") {
    return std::shared_ptr<PerfCounterFactory>(new SimplePerfCounterFactory);
  }
  return nullptr;
}

}  // namespace utils
