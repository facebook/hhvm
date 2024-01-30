/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
*/

#pragma once

#include <atomic>
#include <cassert>
#include <chrono>
#include <filesystem>
#include <map>
#include <string>
#include <vector>

#include <folly/portability/SysResource.h>
#include <folly/portability/Unistd.h>

#include <sys/syscall.h>

#include <pthread.h>
#include <signal.h>

#include "hphp/util/process-cpu.h"
#include "hphp/util/process-host.h"
#include "hphp/util/optional.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

/*
 * System-wide memory information from /proc/meminfo
 */
struct MemInfo {
  int64_t totalMb{-1};
  int64_t freeMb{-1};
  int64_t cachedMb{-1};
  int64_t buffersMb{-1};
  int64_t availableMb{-1};
  bool valid() const {
    return (totalMb | freeMb | cachedMb | buffersMb | availableMb) >= 0;
  }
};

/*
 * Information from /proc/self/status, along with other HHVM-specific memory
 * usage data.
 *
 * Kernel documentation: http://man7.org/linux/man-pages/man5/proc.5.html
 */
struct ProcStatus {
  static std::atomic_int64_t VmSizeKb;  // virtual memory size
  static std::atomic_int64_t VmRSSKb;   // RSS, not including hugetlb pages
  static std::atomic_int64_t VmHWMKb;   // peak RSS
  static std::atomic_int64_t VmSwapKb;  // swap usage
  static std::atomic_int64_t HugetlbPagesKb; // Hugetlb

  // Mapped but unused size, updated periodically when updating jemalloc stats.
  static std::atomic_int64_t UnusedKb;

  // Number of threads running in the current process.
  static std::atomic_int threads;
  static std::atomic_uint lastUpdate;   // seconds in std::chrono::steady_clock

 public:
  static auto totalRssKb() {            // include hugetlb pages
    return VmRSSKb.load(std::memory_order_relaxed) +
      HugetlbPagesKb.load(std::memory_order_relaxed);
  }
  static auto adjustedRssKb() {
    assert(valid());
    return VmRSSKb.load(std::memory_order_relaxed)
      + VmSwapKb.load(std::memory_order_relaxed)
      + HugetlbPagesKb.load(std::memory_order_relaxed)
      - UnusedKb.load(std::memory_order_relaxed);
  }
  static auto nThreads() {
    return threads.load(std::memory_order_relaxed);
  }
  static void updateUnused(int64_t unusedKb) {
    UnusedKb.store(unusedKb, std::memory_order_relaxed);
  }
  static bool valid() {
    return static_cast<bool>(lastUpdate.load(std::memory_order_acquire));
  }
  static unsigned time() {
    auto const d = std::chrono::steady_clock::now().time_since_epoch();
    return std::chrono::duration_cast<std::chrono::seconds>(d).count();
  }
  static void checkUpdate(int updateInterval) {
    if (time() >= lastUpdate.load(std::memory_order_acquire) + updateInterval) {
      update();
    }
  }
  static void update();
};

/*
* Information about system pressure (cpu, io, memory) read from
* /proc/pressure/{cpu,io,memory}
*
* PSI docs: https://facebookmicrosites.github.io/psi/docs/overview
*/
struct Pressure {
  // Pressure over the last 10, 60, and 300 seconds respectively
  double avg10;
  double avg60;
  double avg300;
  // Total number of accumulated microseconds
  uint64_t total;
};

struct ProcPressure {
  Optional<Pressure> some;
  Optional<Pressure> full;
};

///////////////////////////////////////////////////////////////////////////////

namespace Process {
// The maximum supported signal number is kNSig - 1.
constexpr unsigned kNSig =
#if defined(NSIG)
  NSIG
#elif defined(_NSIG)
  _NSIG
#else
  64
#endif
  ;

// Cached process statics
extern std::string HostName;
extern std::string CurrentWorkingDirectory;
extern char** Argv;

void InitProcessStatics();
inline void RecordArgv(char** argv) { // only call this in main()
  Argv = argv;
}

/**
 * Current executable's name.
 */
std::string GetAppName();

/**
 * Get command line with a process ID.
 */
std::string GetCommandLine(pid_t pid);

/**
 * Check if the current process is being run under GDB.  Will return false if
 * we're unable to read /proc/{getpid()}/status.
 */
bool IsUnderGDB();

/**
 * Get memory usage in MB by a process.
 */
int64_t GetMemUsageMb();

/**
 * Get the total systems cpu delay in milliseconds.
 *
 * The cpu delay measure the total amount of time that a processes were
 * runnable, but not running because the CPUs were busy.
 */
int64_t GetSystemCPUDelayMS();

ProcPressure GetCPUPressure(const std::filesystem::path& path = "/proc/pressure/cpu");
ProcPressure GetIOPressure(const std::filesystem::path& path = "/proc/pressure/io");
ProcPressure GetMemoryPressure(const std::filesystem::path& path = "/proc/pressure/memory");

/**
 * Get the number of threads running in the current process.
 */
int GetNumThreads();

/**
 * Get system-wide memory usage information.  If `checkCgroup2' is true and
 * HHVM is running inside a cgroup2, this function checks cgroup2's limits to
 * potentially reduce the amount of total and available memory.  Returns false
 * upon failure.  Note that previous value of `info` is reset, even upon
 * failure.
 */
bool GetMemoryInfo(MemInfo& info, bool checkCgroup2);

/**
 * Current thread's identifier.
 */
inline pthread_t GetThreadId() {
  return pthread_self();
}

/**
 * Current thread's identifier.
 */
inline uint64_t GetThreadIdForTrace() {
  return (uint64_t)pthread_self();
}

/*
 * Thread's process identifier.
 */
inline pid_t GetThreadPid() {
  return syscall(SYS_gettid);
}

/**
 * Are we in the main thread still?
 */
inline bool IsInMainThread() {
  return Process::GetThreadPid() == getpid();
}

/**
 * Get current working directory.
 */
std::string GetCurrentDirectory();

/**
 * Get current user's name.
 */
std::string GetCurrentUser();

/**
 * Get current user's home directory.
 */
std::string GetHomeDirectory();

/**
 * Set core dump filters to make sure hugetlb pages are included in coredumps.
 */
void SetCoreDumpHugePages();

/*
 * Write to /proc/self/oom_score_adj (for Linux only).  This affects the OOM
 * killer when it decides which process to kill.  Valid values are between
 * -1000 and 1000.  Lower values makes it less likely for the process to be
 * killed.  In particular, -1000 disables the OOM killer completely for the
 * current process.  Returns whether adjustment was successful.
 */
bool OOMScoreAdj(int adj = 1000);
/*
 * Sometimes we want to relaunch under modified environment.  It won't return
 * upon success, and returns -1 when an error occurs (similar to exec()).
 */
int Relaunch();

/** Replace the FDs, ensuring a clean STDIO, and avoiding conflicts.
 *
 * `fds` is a map where:
 * - keys are the target FD in the new process
 * - values are the FD in the current process
 *
 * Any FDs not specified (including STDIO) will be closed.
 *
 * If the TARGET is negative, it will be given any FD that does not conflict
 * with the other input.
 *
 * createDelegate() closes stdin/stdout/stderr, so some of our
 * received handles may be FD 0/1/2 - that doesn't mean they're
 * stdin/out/err though, but we need to make sure we don't accidentally
 * close them. Copy everything out of the 'unsafe' space, close everything
 * else, then move back.
 *
 * Returns a map of negative targets (if any) to their new actual FD.
 */
std::map<int, int> RemapFDsPreExec(const std::map<int, int>& fds);

const int FORK_AND_EXECVE_FLAG_NONE    = 0;
const int FORK_AND_EXECVE_FLAG_SETPGID = 1 << 0;
const int FORK_AND_EXECVE_FLAG_SETSID  = 1 << 1;
const int FORK_AND_EXECVE_FLAG_EXECVPE = 1 << 2;

/** Opens a process with the given arguments, environment, working directory,
 * and file descriptors.
 *
 * This *does not* use the shell; to use the shell, consider executing
 * `/bin/sh` with args `['-c', 'command'].
 *
 * `pgid` is ignored unless `FORK_AND_EXECVE_FLAG_SETPGID` is in `flags`.
 *
 * `FORK_AND_EXECVE_FLAG_SETPGID` and `FORK_AND_EXECVE_FLAG_SETSID` are
 * mutually exclusive.
 *
 * `fds` is a map where:
 * - keys are the target FD in the new process
 * - values are the FD in the current process
 *
 * Any FDs not specified (including STDIO) will be closed.
 *
 * On error, `errno` will be set.
 * - if fork() fails, -1 will be returned
 * - if chdir() fails, -2 will be returned
 * - if setsid() fails, -3 will be returned
 * - if setpgid() fails, -4 will be returned
 * - if execve() fails, -5 will be returned
 */
pid_t ForkAndExecve(
  const std::string& path,
  const std::vector<std::string>& argv,
  const std::vector<std::string>& envp,
  const std::string& cwd,
  const std::map<int, int>& fds,
  int flags = FORK_AND_EXECVE_FLAG_NONE,
  pid_t pgid = 0);

} // namespace Process

///////////////////////////////////////////////////////////////////////////////
}
