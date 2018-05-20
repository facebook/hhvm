#include <atomic>
#include <cstdio>
#include <vector>
#include <thread>

#include "hphp/util/hardware-counter.h"
#include "hphp/util/process.h"

namespace HPHP {

namespace {

void print_counter(const std::string& name, int64_t val, void* v) {
  std::printf("HW: %d %s = %" PRId64 "\n",
              *(int*)v, name.c_str(), val);
}

void test(uint64_t count, uint64_t reps, int thread) {
  HardwareCounter::s_counter.getCheck();
  std::atomic<uint64_t> v{};
  HardwareCounter::GetPerfEvents(print_counter, (void*)&thread);
  for (uint64_t i = 0; i < count; i++) {
    if (reps) {
      HardwareCounter::Reset();
      for (uint64_t j = 0; j < reps; j++) {
        ++v;
      }
    }
    HardwareCounter::GetPerfEvents(
      [] (const std::string& name, int64_t val, void* p) {
        *(decltype(v)*)p += val;
      },
      &v);
  }
  HardwareCounter::GetPerfEvents(print_counter, (void*)&thread);
}

void usage(int argc, const char** argv) {
  std::printf("usage: %s [--count=n] [--reps=n] [--threads=n] "
              "[--events=s] [--disable] [--fast[={1,0}]]\n",
              argc ? argv[0] : "hwctr-perf");
}

const char* check_arg(const char* name, const char* arg) {
  auto const len = strlen(name);
  if (!strncmp(name, arg, len)) return arg + len;
  return nullptr;
}

int run(int argc, const char** argv) {
  if (argc == 2 && !strcmp(argv[1], "--help")) {
    usage(argc, argv);
    return 0;
  }
  uint64_t count = 1000;
  uint64_t reps = 1000000;
  int nthread = -1;
  const char* events = "";
  bool enabled = true;
  bool fast = false;
  for (int i = 1; i < argc; i++) {
    if (auto const arg = check_arg("--count=", argv[i])) {
      count = atol(arg);
      continue;
    }
    if (auto const arg = check_arg("--threads=", argv[i])) {
      nthread = atol(arg);
      continue;
    }
    if (auto const arg = check_arg("--reps=", argv[i])) {
      reps = atol(arg);
      continue;
    }
    if (auto const arg = check_arg("--events=", argv[i])) {
      events = arg;
      continue;
    }
    if (auto const arg = check_arg("--disable", argv[i])) {
      if (!*arg) {
        enabled = false;
        continue;
      }
    }
    if (auto const arg = check_arg("--fast", argv[i])) {
      if (!*arg || (*arg == '=' && atol(arg))) {
        fast = true;
        continue;
      }
    }
    std::printf("Invalid argument: %s\n", argv[i]);
    usage(argc, argv);
    exit(1);
  }

  HardwareCounter::Init(enabled, events, false, false, fast, -1);

  if (nthread == 0) {
    test(count, reps, 0);
  } else {
    auto cpuCount = Process::GetCPUCount();
    if (nthread < 0) nthread = cpuCount;
    if (nthread > cpuCount * 10) nthread = cpuCount * 10;

    std::vector<std::thread> threads;
    threads.reserve(nthread);
    for (auto i = 1; i <= nthread; i++) {
      threads.emplace_back([=] { test(count, reps, i); });
    }
    for (auto& t : threads) t.join();
  }

  return 0;
}

}
}

int main(int argc, const char** argv) {
  HPHP::run(argc, argv);
}
