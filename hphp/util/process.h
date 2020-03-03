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

#ifndef incl_HPHP_PROCESS_H_
#define incl_HPHP_PROCESS_H_

#include <string>

#include <folly/portability/Unistd.h>

#ifdef _MSC_VER
# include <windows.h>
#else
# include <sys/syscall.h>
#endif

#include <pthread.h>
#include <signal.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

/*
 * System-wide memory infomation from /proc/meminfo
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
 * Infomation from /proc/self/status, along with other HHVM-specific memory
 * usage data.
 *
 * Kernel documentation: http://man7.org/linux/man-pages/man5/proc.5.html
 */
struct ProcStatus {
  int64_t VmSizeKb{-1};                 // virtual memory size
  int64_t VmRSSKb{-1};                  // RSS, not including hugetlb pages
  int64_t VmHWMKb{-1};                  // peak RSS
  int64_t HugetlbPagesKb{0};            // Hugetlb mappings (2M + 1G)

  // 'Real' memory usage that includes VMRSS and HugetlbPages, but excludes
  // unused space held by jemalloc.  This is mostly used to track regressions.
  int64_t adjustedRSSKb{-1};

  // Number of threads running in the current process.
  int Threads{-1};

  // Constructor reads /proc/self/status and fill in the fields.
  ProcStatus();
  // Subtrace memory that is mapped in but unused.
  void registerUnused(int64_t unusedKb) {
    auto const r = adjustedRSSKb - unusedKb;
    if (r >= 0) adjustedRSSKb = r;
  }

  bool valid() const {
    return VmSizeKb > 0 && VmRSSKb > 0 && VmHWMKb > 0 &&
      HugetlbPagesKb >= 0 &&
      Threads > 0;
  }
};

///////////////////////////////////////////////////////////////////////////////

struct Process {
  // The maximum supported signal number is kNSig - 1.
  static constexpr unsigned kNSig =
#if defined(NSIG)
    NSIG
#elif defined(_NSIG)
    _NSIG
#else
    64
#endif
    ;

  // Cached process statics
  static std::string HostName;
  static std::string CurrentWorkingDirectory;
  static char** Argv;

  static void InitProcessStatics();
  static void RecordArgv(char** argv) { // only call this in main()
    Argv = argv;
  }

  /**
   * Current executable's name.
   */
  static std::string GetAppName();

  /**
   * This machine'a name.
   */
  static std::string GetHostName();

  /**
   * Get command line with a process ID.
   */
  static std::string GetCommandLine(pid_t pid);

  /**
   * Check if the current process is being run under GDB.  Will return false if
   * we're unable to read /proc/{getpid()}/status.
   */
  static bool IsUnderGDB();

  /**
   * Get memory usage in MB by a process.
   */
  static int64_t GetMemUsageMb();

  /**
   * Get the number of threads running in the current process.
   */
  static int GetNumThreads();

  /**
   * Get system-wide memory usage information.  Returns false upon
   * failure.  Note that previous value of `info` is reset, even upon
   * failure.
   */
  static bool GetMemoryInfo(MemInfo& info);

  /**
   * Current thread's identifier.
   */
  static pthread_t GetThreadId() {
    return pthread_self();
  }

  /**
   * Current thread's identifier.
   */
  static uint64_t GetThreadIdForTrace() {
    // For tracing purposes this just needs to be unique, pthread_t is not
    // portable but even if it's a pointer to a struct like on OSX this will
    // produce a unique value. If we support platforms where this isn't the
    // case we will need to revisit this.
#ifdef __linux__
    return pthread_self();
#else
    return (uint64_t)pthread_self();
#endif
  }

  /*
   * Thread's process identifier.
   */
  static pid_t GetThreadPid() {
#ifdef __FreeBSD__
# if __FreeBSD__version > 900030
    return pthread_getthreadid_np();
# else
    long tid;
    syscall(SYS_thr_self, &tid);
    return (pid_t) tid;
# endif
#elif defined(_MSC_VER)
    return GetCurrentThreadId();
#else
    return syscall(SYS_gettid);
#endif
  }

  /**
   * Are we in the main thread still?
   */
  static bool IsInMainThread() {
    return Process::GetThreadPid() == getpid();
  }

  /**
   * Get CPU information.
   */
  static int GetCPUCount();
  static std::string GetCPUModel();

  /**
   * Get current working directory.
   */
  static std::string GetCurrentDirectory();

  /**
   * Get current user's name.
   */
  static std::string GetCurrentUser();

  /**
   * Get current user's home directory.
   */
  static std::string GetHomeDirectory();

  /**
   * Set core dump filters to make sure hugetlb pages are included in coredumps.
   */
  static void SetCoreDumpHugePages();

  /*
   * Write to /proc/self/oom_score_adj (for Linux only).  This affects the OOM
   * killer when it decides which process to kill.  Valid values are between
   * -1000 and 1000.  Lower values makes it less likely for the process to be
   * killed.  In particular, -1000 disables the OOM killer completely for the
   * current process.  Returns whether adjustment was successful.
   */
  static bool OOMScoreAdj(int adj = 1000);
  /*
   * Sometimes we want to relaunch under modified environment.  It won't return
   * upon success, and returns -1 when an error occurs (similar to exec()).
   */
  static int Relaunch();
};

///////////////////////////////////////////////////////////////////////////////
}

#endif
