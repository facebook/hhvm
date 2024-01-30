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
#include "hphp/util/process.h"
#include "hphp/util/process-cpu.h"
#include "hphp/util/process-host.h"

#include <sys/types.h>
#include <stdlib.h>

#include <sys/fcntl.h>
#include <sys/utsname.h>
#include <sys/wait.h>
#include <pwd.h>

#include <folly/portability/Sockets.h>
#include <folly/portability/SysMman.h>
#include <folly/portability/Unistd.h>
#include <folly/Conv.h>
#include <folly/Format.h>
#include <folly/ScopeGuard.h>
#include <folly/String.h>

#include <filesystem>
#include <set>
#include <fstream>

#include "hphp/util/hugetlb.h"
#include "hphp/util/managed-arena.h"
#include "hphp/util/text-color.h"
#include "hphp/util/user-info.h"

namespace {
  const std::string kCpu = "cpu";
  const std::string kIo = "io";
  const std::string kMemory = "memory";
}

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

using std::string;

static void readString(FILE *f, string &out) {
  size_t nread = 0;
  constexpr unsigned int BUFFER_SIZE = 1024;
  char buf[BUFFER_SIZE];
  while ((nread = fread(buf, 1, BUFFER_SIZE, f)) != 0) {
    out.append(buf, nread);
  }
}

///////////////////////////////////////////////////////////////////////////////

// Cached process statics
std::string Process::HostName;
std::string Process::CurrentWorkingDirectory;
char** Process::Argv;
std::atomic_int64_t ProcStatus::VmSizeKb;
std::atomic_int64_t ProcStatus::VmRSSKb;
std::atomic_int64_t ProcStatus::VmHWMKb;
std::atomic_int64_t ProcStatus::VmSwapKb;
std::atomic_int64_t ProcStatus::HugetlbPagesKb;
std::atomic_int64_t ProcStatus::UnusedKb;
std::atomic_int ProcStatus::threads;
std::atomic_uint ProcStatus::lastUpdate;

void Process::InitProcessStatics() {
  HostName = GetHostName();
  CurrentWorkingDirectory = GetCurrentDirectory();
}

///////////////////////////////////////////////////////////////////////////////
// /proc/* parsing functions

std::string Process::GetCommandLine(pid_t pid) {
  auto const name = folly::sformat("/proc/{}/cmdline", pid);

  std::string cmdline;
  auto const f = fopen(name.c_str(), "r");
  if (f) {
    readString(f, cmdline);
    fclose(f);
  }

  std::string converted;
  for (auto ch : cmdline) {
    converted += ch ? ch : ' ';
  }
  return converted;
}

bool Process::IsUnderGDB() {
  auto const cmdStr = GetCommandLine(getppid());
  auto const cmdPiece = folly::StringPiece{cmdStr};

  if (cmdPiece.empty()) return false;

  auto const spaceIdx = std::min(cmdPiece.find(' '), cmdPiece.size() - 1);
  auto const binaryPiece = cmdPiece.subpiece(0, spaceIdx + 1);

  std::filesystem::path binaryPath(binaryPiece.begin(), binaryPiece.end());
  return binaryPath.filename() == "gdb ";
}

int64_t Process::GetMemUsageMb() {
  ProcStatus::update();
  return ProcStatus::valid() ? ProcStatus::adjustedRssKb() / 1024 : 0;
}

int64_t Process::GetSystemCPUDelayMS() {
  static FILE* fp = nullptr;
  if (!fp) {
    if (!(fp = fopen("/proc/schedstat", "r"))) {
      return -1;
    }
  }
  // Refresh the proc info.
  rewind(fp);
  fflush(fp);

  int64_t totalCpuDelay = 0;
  // Supposedly this should be enough to hold th important lines of the
  // schedstat file.
  char buf[320];
  while (fgets(buf, sizeof(buf), fp) != nullptr) {
    if (strncmp(buf, "cpu", 3) == 0) {
      uint64_t cpuDelay;
      if (sscanf(buf,
                 "%*s %*u %*u %*u %*u %*u %*u %*u %lu %*u",
                 &cpuDelay) != 1) {
        return -1;
      }
      totalCpuDelay += cpuDelay;
    }
  }
  // The kernel reports the information in nanoseconds.  Convert it
  // to milliseconds.
  return totalCpuDelay / 1000000;
}

ProcPressure getProcPressure(const std::filesystem::path &path) {
  ProcPressure procPressure;

  // Helper function to parse a single line from the pressure file. Expected to
  // be called with the entire line as input, for example:
  // some avg10=0.01 avg60=0.01 avg300=0.01 total=12345
  const auto parseLine = [](const std::string_view &line) -> Pressure {
    // Drop the 'some|full' + space prefix and load into parts
    std::vector<std::string> parts;
    folly::split(' ', line.substr(5), parts);

    assertx(parts.size() == 4); // Expect 4 metrics

    Pressure pressure;
    for (const auto &part : parts) {
      std::vector<std::string> subparts;
      folly::split('=', part, subparts);
      assertx(subparts.size() == 2); // Expect 2 values
      if (subparts[0] == "avg10") {
        pressure.avg10 = folly::to<double>(subparts[1]);
      } else if (subparts[0] == "avg60") {
        pressure.avg60 = folly::to<double>(subparts[1]);
      } else if (subparts[0] == "avg300") {
        pressure.avg300 = folly::to<double>(subparts[1]);
      } else if (subparts[0] == "total") {
        pressure.total = folly::to<uint64_t>(subparts[1]);
      }
    }
    return pressure;
  };

  std::ifstream file(path);
  if (file.is_open()) {
    std::string line;
    while (std::getline(file, line)) {
      if (line.starts_with("some")) {
        procPressure.some = parseLine(line);
      } else if (line.starts_with("full")) {
        procPressure.full = parseLine(line);
      }
    }
  }

  return procPressure;
}

ProcPressure Process::GetCPUPressure(const std::filesystem::path &path) {
  return getProcPressure(path);
}

ProcPressure Process::GetIOPressure(const std::filesystem::path &path) {
  return getProcPressure(path);
}

ProcPressure Process::GetMemoryPressure(const std::filesystem::path &path) {
  return getProcPressure(path);
}

int Process::GetNumThreads() {
  ProcStatus::update();
  return ProcStatus::valid() ? ProcStatus::nThreads() : 1;
}

/////////////////////////////////////////////////////////////////////////

namespace {

/*
 * Try to read the memory information from the given /cgroup2/memory.<fileName>
 * file if available.  For reference, see the "Memory Interface Files" section in
 * https://www.kernel.org/doc/Documentation/cgroup-v2.txt.
 *
 * In case this function fails to read from the given file, it returns -1.
 * Otherwise, it returns the size read converted to MBs.
 */
int64_t readCgroup2FileMb(const char* fileName) {
  std::string fullFileName = std::string("/cgroup2/memory.") + fileName;

  if (FILE* f = fopen(fullFileName.c_str(), "r")) {
    int64_t size;
    if (fscanf(f, "%ld", &size) != 1) return -1;
    fclose(f);
    return size >> 20;
  }

  return -1;
}

/*
 * If cgroup2 is enabled, update the MemInfo in `info' based on cgroup2 limits.
 */
void updateMemInfoWithCgroup2Info(MemInfo& info) {
  if (!ProcStatus::valid()) return;
  const int64_t cgroup2TotalMb = readCgroup2FileMb("max");
  const int64_t currUsageMb = ProcStatus::totalRssKb() / 1024;
  if (cgroup2TotalMb >= 0) {
    auto const availableMb_approx =
      std::max(cgroup2TotalMb - currUsageMb, int64_t{});
    if (availableMb_approx < info.availableMb) {
      info.availableMb = availableMb_approx;
    }
    if (cgroup2TotalMb < info.totalMb) {
      info.totalMb = cgroup2TotalMb;
    }
  }
}

// Files such as /proc/meminfo and /proc/self/status contain many lines
// formatted as one of the following:
//   <fieldName>: <number>
//   <fieldName>: <number> kB
// This function parses the line and return the number in it.  -1 is returned
// when the line isn't formatted as expected (until one day we need to read a
// line where -1 is a legit value).
int64_t readSize(const char* line, bool expectKB = false) {
  int64_t result = -1;
  char tail[8];
  auto n = sscanf(line, "%*s %" SCNd64 " %7s", &result, tail);
  if (expectKB) {
    if (n < 2) return -1;
    if (tail[0] != 'k' || tail[1] != 'B') return -1;
  }
  return result;
}

}

/////////////////////////////////////////////////////////////////////////

bool Process::GetMemoryInfo(MemInfo& info, bool checkCgroup2) {
  info = MemInfo{};
  FILE* f = fopen("/proc/meminfo", "r");
  if (f) {
    SCOPE_EXIT{ fclose(f); };

    char line[128];
    while (fgets(line, sizeof(line), f)) {
      auto const kb = readSize(line, true);
      if (!strncmp(line, "MemTotal:", 9)) {
        if (kb >= 0) info.totalMb = kb / 1024;
      } else if (!strncmp(line, "MemFree:", 8)) {
        if (kb >= 0) info.freeMb = kb / 1024;
      } else if (!strncmp(line, "Buffers:", 8)) {
        if (kb >= 0) info.buffersMb = kb / 1024;
      } else if (!strncmp(line, "Cached:", 7)) {
        if (kb >= 0) info.cachedMb = kb / 1024;
      } else if (!strncmp(line, "MemAvailable:", 13)) {
        if (kb >= 0) info.availableMb = kb / 1024;
      }
      if (info.valid()) {
        if (checkCgroup2) updateMemInfoWithCgroup2Info(info);
        return true;
      }
    }
    // If MemAvailable isn't available, which shouldn't be the case for kernel
    // versions later than 3.14, we get a rough estimation.
    if (info.availableMb < 0 && info.freeMb >= 0 &&
        info.cachedMb >= 0 && info.buffersMb >= 0) {
      info.availableMb = info.freeMb + info.cachedMb;
      return true;
    }
  }
  return false;
}

///////////////////////////////////////////////////////////////////////////////

std::string Process::GetAppName() {
  const char* progname = getenv("_");
  if (!progname || !*progname) {
    progname = "unknown program";
  }
  return progname;
}

std::string Process::GetCurrentUser() {
  const char *name = getenv("LOGNAME");
  if (name && *name) {
    return name;
  }

  auto buf = PasswdBuffer{};
  passwd *pwd;
  if (!getpwuid_r(geteuid(), &buf.ent, buf.data.get(), buf.size, &pwd) &&
      pwd && pwd->pw_name) {
    return pwd->pw_name;
  }
  return "";
}

std::string Process::GetCurrentDirectory() {
  char buf[PATH_MAX + 64];   // additional space for suffixes like " (deleted)";
  memset(buf, 0, sizeof(buf));
  if (char* cwd = getcwd(buf, PATH_MAX)) return cwd;

  if (errno != ENOENT) {
    return "";
  }
  // Read cwd symlink directly if it leads to the deleted path.
  int r = readlink("/proc/self/cwd", buf, sizeof(buf));
  if (r == -1) {
    return "";
  }
  auto const kDeleted = " (deleted)";
  auto const kDeletedLen = strlen(kDeleted);
  if (r >= kDeletedLen && !strcmp(buf + r - kDeletedLen, kDeleted)) {
    buf[r - kDeletedLen] = 0;
  }
  return &(buf[0]);
}

std::string Process::GetHomeDirectory() {
  string ret;

  const char *home = getenv("HOME");
  if (home && *home) {
    ret = home;
  } else {
    passwd *pwd = getpwent();
    if (pwd && pwd->pw_dir) {
      ret = pwd->pw_dir;
    }
  }

  if (ret.empty() || ret[ret.size() - 1] != '/') {
    ret += '/';
  }
  return ret;
}

void Process::SetCoreDumpHugePages() {
  /*
   * From documentation athttp://man7.org/linux/man-pages/man5/core.5.html
   *
   * The bits in coredump_filter have the following meanings:
   *
   *   bit 0  Dump anonymous private mappings.
   *   bit 1  Dump anonymous shared mappings.
   *   bit 2  Dump file-backed private mappings.
   *   bit 3  Dump file-backed shared mappings.
   *   bit 4 (since Linux 2.6.24) Dump ELF headers.
   *   bit 5 (since Linux 2.6.28) Dump private huge pages.
   *   bit 6 (since Linux 2.6.28) Dump shared huge pages.
   *   bit 7 (since Linux 4.4) Dump private DAX pages.
   *   bit 8 (since Linux 4.4) Dump shared DAX pages.
   */
  if (FILE* f = fopen("/proc/self/coredump_filter", "r+")) {
    unsigned mask = 0;
    if (fscanf(f, "%x", &mask)) {
      constexpr unsigned hugetlbMask = 0x60;
      if ((mask & hugetlbMask) != hugetlbMask) {
        mask |= hugetlbMask;
        rewind(f);
        fprintf(f, "0x%x", mask);
      }
    }
    fclose(f);
  }
}

void ProcStatus::update() {
  if (FILE* f = fopen("/proc/self/status", "r")) {
    char line[128];
    int64_t vmsize = 0, vmrss = 0, vmhwm = 0, vmswap = 0, hugetlb = 0;
    while (fgets(line, sizeof(line), f)) {
      if (!strncmp(line, "VmSize:", 7)) {
        vmsize = readSize(line, true);
      } else if (!strncmp(line, "VmRSS:", 6)) {
        vmrss = readSize(line, true);
      } else if (!strncmp(line, "VmHWM:", 6)) {
        vmhwm = readSize(line, true);
      } else if (!strncmp(line, "VmSwap:", 7)) {
        vmswap = readSize(line, true);
      } else if (!strncmp(line, "HugetlbPages:", 13)) {
        hugetlb = readSize(line, true);
      } else if (!strncmp(line, "Threads:", 8)) {
        threads.store(readSize(line, false), std::memory_order_relaxed);
      }
    }
    fclose(f);
    if (vmrss <= 0) {
      // Invalid
      lastUpdate.store(0, std::memory_order_release);
    } else {
      VmSizeKb.store(vmsize, std::memory_order_relaxed);
      VmRSSKb.store(vmrss, std::memory_order_relaxed);
      VmSwapKb.store(vmswap, std::memory_order_relaxed);
      VmHWMKb.store(vmhwm + hugetlb, std::memory_order_relaxed);
      HugetlbPagesKb.store(hugetlb, std::memory_order_relaxed);
      lastUpdate.store(time(), std::memory_order_release);
    }
#ifdef USE_JEMALLOC
    mallctl_epoch();
#if USE_JEMALLOC_EXTENT_HOOKS
    size_t unused = 0;
    // Various arenas where range of hugetlb pages can be reserved but only
    // partially used.
    unused += alloc::getRange(alloc::AddrRangeClass::VeryLow).retained();
    unused += alloc::getRange(alloc::AddrRangeClass::Low).retained();
    unused += alloc::getRange(alloc::AddrRangeClass::Uncounted).retained();
    if (alloc::g_arena0) {
      unused += alloc::g_arena0->retained();
    }
    for (auto const arena : alloc::g_local_arenas) {
      if (arena) unused += arena->retained();
    }
    updateUnused(unused >> 10); // convert to kB
#endif
#endif
  }
}

bool Process::OOMScoreAdj(int adj) {
  if (adj >= -1000 && adj < 1000) {
    if (auto f = fopen("/proc/self/oom_score_adj", "r+")) {
      fprintf(f, "%d", adj);
      fclose(f);
      return true;
    }
  }
  return false;
}

int Process::Relaunch() {
  if (!Argv) {
    errno = EINVAL;
    return -1;
  }
  return execvp(Argv[0], Argv);
}

std::map<int, int> Process::RemapFDsPreExec(const std::map<int, int>& fds) {
  std::map<int, int> unspecified;
  // 1. copy all to FDs outside of STDIO range
  std::map<int, int> dups;
  std::set<int> preserve_set;
  for (auto& [_target, current] : fds) {
    if (dups.find(current) != dups.end()) {
      continue;
    }

    int next_fd;
    bool conflict;
    do {
      conflict = false;
      // don't conflict with STDIO
      next_fd = dup(current);
      if (next_fd <= STDERR_FILENO) {
        conflict = true;
        continue;
      }
      // don't conflict with targets
      conflict = false;
      for (auto [target, _current] : fds) {
        if (next_fd == target) {
          conflict = true;
          break;
        }
      }
    } while (conflict);
    dups[current] = next_fd;
    preserve_set.emplace(next_fd);
  }

  // 2. clean up libc STDIO
  //
  // Don't want to swap the FD underlying these FILE*...
  //
  // If they are closed already, these are silent no-ops.
  fclose(stdin);
  fclose(stdout);
  fclose(stderr);

  // 3. close all FDs except our dups
  //
  // This includes the STDIO FDs as it's possible that:
  // - the FILE* were previously closed (so the fclose above were no-ops)
  // - the FDs were then re-used
  const char* fd_dir = "/proc/self/fd";
  // If you close FDs while in this loop, they get removed from /proc/self/fd
  // and the iterator gets sad ("Bad file descriptor: /proc/self/fd")
  std::set<int> fds_to_close;
  for (const auto& entry : std::filesystem::directory_iterator(fd_dir)) {
    char* endptr = nullptr;
    auto filename = entry.path().filename();
    const char* filename_c = filename.c_str();
    const int fd = strtol(filename_c, &endptr, 10);
    assert(endptr != filename_c); // no matching characters
    assert(*endptr == '\0'); // entire string
    if (preserve_set.find(fd) != preserve_set.end()) {
      continue;
    }
    fds_to_close.emplace(fd);
  }
  for (const auto& fd: fds_to_close) {
    close(fd);
  }

  // 4. Move the dups into place.
  for (const auto& [target, orig] : fds) {
    int tmp = dups[orig];

    if (target < 0 /* don't care what the FD is */) {
      unspecified[target] = tmp;
    } else {
      dup2(tmp, target);
    }
  }

  // 5. Close the dups; do this separately to above in case
  // the same orig was used for multiple targets
  for (const auto& [target, orig] : fds) {
    if (target < 0) {
      continue;
    }
    close(dups.at(orig));
  }
  return unspecified;
}

namespace {
  char **build_cstrarr(const std::vector<std::string> &vec) {
    char **cstrarr = nullptr;
    int size = vec.size();
    if (size) {
      cstrarr = (char **)malloc((size + 1) * sizeof(char *));
      int j = 0;
      for (unsigned int i = 0; i < vec.size(); i++, j++) {
        *(cstrarr + j) = (char *)vec[i].c_str();
      }
      *(cstrarr + j) = nullptr;
    }
    return cstrarr;
  }
} // namespace {

pid_t Process::ForkAndExecve(
  const std::string& path,
  const std::vector<std::string>& argv,
  const std::vector<std::string>& envp,
  const std::string& cwd,
  const std::map<int, int>& orig_fds,
  int flags,
  pid_t pgid
) {
  // Distinguish execve failure: if the write side of the pipe
  // is closed with no data, it succeeded.
  int fork_fds[2];
  pipe(fork_fds);
  int fork_r = fork_fds[0];
  int fork_w = fork_fds[1];
  fcntl(fork_w, F_SETFD, fcntl(fork_w, F_GETFD) | O_CLOEXEC);

  pid_t child = fork();

  if (child == -1) {
    return -1;
  }

  if (child == 0) {
    mprotect_1g_pages(PROT_READ);
    Process::OOMScoreAdj(1000);

    close(fork_r);

    // Need mutable copy
    std::map<int, int> fds(orig_fds);
    fds[-1] = fork_w;
    const auto remapped = Process::RemapFDsPreExec(fds);
    fork_w = remapped.at(-1);

    if (!cwd.empty()) {
      if (cwd != Process::GetCurrentDirectory()) {
        if (chdir(cwd.c_str()) == -1) {
          dprintf(fork_w, "%s %d", "chdir", errno);
          _Exit(HPHP_EXIT_FAILURE);
        }
      }
    }

    if (flags & Process::FORK_AND_EXECVE_FLAG_SETSID) {
      if (setsid() == -1) {
        dprintf(fork_w, "%s\n%d\n", "setsid", errno);
        _Exit(HPHP_EXIT_FAILURE);
      }
    } else if (flags & Process::FORK_AND_EXECVE_FLAG_SETPGID) {
      if (setpgid(0, pgid) == -1) {
        dprintf(fork_w, "%s %d", "setpgid", errno);
        _Exit(HPHP_EXIT_FAILURE);
      }
    }

    char** argv_arr = build_cstrarr(argv);
    char** envp_arr = build_cstrarr(envp);
    SCOPE_EXIT { free(argv_arr); free(envp_arr); };

    if (flags & Process::FORK_AND_EXECVE_FLAG_EXECVPE) {
      execvpe(path.c_str(), argv_arr, envp_arr);
    } else {
      execve(path.c_str(), argv_arr, envp_arr);
    }
    dprintf(fork_w, "%s %d", "execve", errno);
    _Exit(HPHP_EXIT_FAILURE);
  }

  close(fork_w);

  pollfd pfd[1];
  pfd[0].fd = fork_w;
  pfd[0].events = POLLIN;
  int ret;
  do {
    ret = poll(pfd, /* number of fds = */ 1, /* timeout = no timeout*/ -1);
  } while (ret == -1 && errno == EINTR);

  char buf[16];
  auto len = read(fork_r, &buf, sizeof(buf));
  close(fork_r);

  // Closed without write, which means close-on-exec
  if (len < 1) {
    return child;
  }

  char failed_call_buf[17]; // 16 + trailing null
  int saved_errno;
  if (sscanf(buf, "%16s %d", failed_call_buf, &saved_errno) != 2) {
    return -999;
  }

  // Doing the call => return value here instead of sending return values over
  // the pipe so that it's all in one place, and we're less likely to introduce
  // bugs when/if we add additional features.
  const std::string failed_call(failed_call_buf);
  SCOPE_EXIT { errno = saved_errno; };
  if (failed_call == "chdir") {
    return -2;
  }
  if (failed_call == "setsid") {
    return -3;
  }
  if (failed_call == "setpgid") {
    return -4;
  }
  if (failed_call == "execve") {
    return -5;
  }
  if (failed_call == "putenv") {
    return -6;
  }
  return -9999;
}

///////////////////////////////////////////////////////////////////////////////
}
