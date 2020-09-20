#include <thread>
#include <mutex>
#include <cassert>
#include <atomic>

#include <unistd.h>

#include "parentimpl.h"


std::mutex watchdog_mut;
static int watchdog_count = 0;


static void check_and_die(int interval, int grace) noexcept {
  assert(interval > 0);
  assert(grace > 0);
  for (;;) {
    // when we get reparented
    // exit immediately
    if (getppid() == 1) {
      sleep(static_cast<unsigned>(grace));
      exit(20);
    }
    sleep(static_cast<unsigned>(interval));
  }
}


extern "C" void exit_on_parent_exit_(int interval, int grace) noexcept {
  assert(interval > 0);
  assert(grace > 0);
  std::lock_guard<std::mutex> guard(watchdog_mut);
  assert(watchdog_count == 0 || watchdog_count == 1);
  if (watchdog_count == 0) {
    ++watchdog_count;
    std::thread t(check_and_die, interval, grace);
    t.detach();
  }
  return;
}

extern "C" int get_watchdog_count_() noexcept {
  std::lock_guard<std::mutex> guard(watchdog_mut);
  return watchdog_count;
}
