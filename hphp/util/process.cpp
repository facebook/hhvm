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

#include <sys/types.h>
#include <stdlib.h>

#ifdef _MSC_VER
#include <lmcons.h>
#include <Windows.h>
#include <ShlObj.h>
#else
#include <sys/utsname.h>
#include <sys/wait.h>
#include <pwd.h>
#include <folly/portability/Sockets.h>
#include <folly/portability/Unistd.h>
#endif

#include <folly/Conv.h>
#include <folly/Format.h>
#include <folly/ScopeGuard.h>
#include <folly/String.h>

#include <boost/filesystem.hpp>

#include "hphp/util/text-color.h"
#include "hphp/util/user-info.h"

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

  boost::filesystem::path binaryPath(binaryPiece.begin(), binaryPiece.end());
  return binaryPath.filename() == "gdb ";
}

int64_t Process::GetMemUsageMb() {
  ProcStatus status;                    // read /proc/self/status
  return status.valid() ? status.adjustedRSSKb / 1024 : 0;
}

int Process::GetNumThreads() {
  ProcStatus status;
  return status.valid() ? status.Threads : 1;
}

// Files such as /proc/meminfo and /proc/self/status contain many lines
// formatted as one of the following:
//   <fieldName>: <number>
//   <fieldName>: <number> kB
// This function parses the line and return the number in it.  -1 is returned
// when the line isn't formatted as expected (until one day we need to read a
// line where -1 is a legit value).
static int64_t readSize(const char* line, bool expectKB = false) {
  int64_t result = -1;
  char tail[8];
  auto n = sscanf(line, "%*s %" SCNd64 " %7s", &result, tail);
  if (expectKB) {
    if (n < 2) return -1;
    if (tail[0] != 'k' || tail[1] != 'B') return -1;
  }
  return result;
}

bool Process::GetMemoryInfo(MemInfo& info) {
#ifdef _WIN32
#error "Process::GetMemoryInfo() doesn't support Windows (yet)."
  return false;
#endif

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
      if (info.valid()) return true;
    }
    // If MemAvailable isn't available, which shouldn't be the case for kernel
    // versions later than 3.14, we get a rough esitmation.
    if (info.availableMb < 0 && info.freeMb >= 0 &&
        info.cachedMb >= 0 && info.buffersMb >= 0) {
      info.availableMb = info.freeMb + info.cachedMb;
      return true;
    }
  }
  return false;
}

int Process::GetCPUCount() {
  return sysconf(_SC_NPROCESSORS_ONLN);
}

#ifdef __x86_64__
static __inline void do_cpuid(u_int ax, u_int *p) {
  asm volatile ("cpuid"
                : "=a" (p[0]), "=b" (p[1]), "=c" (p[2]), "=d" (p[3])
                : "0" (ax));
}
#elif defined(_M_X64)
#include <intrin.h>
static ALWAYS_INLINE void do_cpuid(int func, uint32_t* p) {
  __cpuid((int*)p, func);
}
#endif

std::string Process::GetCPUModel() {
#if defined(__x86_64__) || defined(_M_X64)
  uint32_t regs[4];
  do_cpuid(0, regs);

  const int vendor_size = sizeof(regs[1])*3;
  std::swap(regs[2], regs[3]);
  uint32_t cpu_exthigh = 0;
  if (memcmp(regs + 1, "GenuineIntel", vendor_size) == 0 ||
      memcmp(regs + 1, "AuthenticAMD", vendor_size) == 0) {
    do_cpuid(0x80000000, regs);
    cpu_exthigh = regs[0];
  }

  char cpu_brand[3 * sizeof(regs) + 1];
  char *brand = cpu_brand;
  if (cpu_exthigh >= 0x80000004) {
    for (u_int i = 0x80000002; i < 0x80000005; i++) {
      do_cpuid(i, regs);
      memcpy(brand, regs, sizeof(regs));
      brand += sizeof(regs);
    }
  }
  *brand = '\0';
  assert(brand - cpu_brand < sizeof(cpu_brand));
  return cpu_brand;

#else
  // On non-x64, fall back to calling uname
  std::string model = "Unknown ";
  struct utsname uname_buf;
  uname(&uname_buf);
  model.append(uname_buf.machine);
  return model;

#endif  // __x86_64__
}

///////////////////////////////////////////////////////////////////////////////

std::string Process::GetAppName() {
  const char* progname = getenv("_");
  if (!progname || !*progname) {
    progname = "unknown program";
  }
  return progname;
}

std::string Process::GetHostName() {
  char hostbuf[128];
  hostbuf[0] = '\0'; // for cleaner valgrind output when gethostname() fails
  gethostname(hostbuf, sizeof(hostbuf));
  hostbuf[sizeof(hostbuf) - 1] = '\0';
  return hostbuf;
}

std::string Process::GetCurrentUser() {
  const char *name = getenv("LOGNAME");
  if (name && *name) {
    return name;
  }

#ifdef _MSC_VER
  char username[UNLEN + 1];
  DWORD username_len = UNLEN + 1;
  if (GetUserName(username, &username_len))
    return std::string(username, username_len);
#else
  auto buf = PasswdBuffer{};
  passwd *pwd;
  if (!getpwuid_r(geteuid(), &buf.ent, buf.data.get(), buf.size, &pwd) &&
      pwd && pwd->pw_name) {
    return pwd->pw_name;
  }
#endif
  return "";
}

std::string Process::GetCurrentDirectory() {
  auto const kDeleted = " (deleted)";
  auto const kDeletedLen = strlen(kDeleted);

  // Allocate additional space for kDeleted part.
  char* buf = (char*)alloca(sizeof(char) * (PATH_MAX + kDeletedLen));
  memset(buf, 0, PATH_MAX + kDeletedLen);
  char* cwd = getcwd(buf, PATH_MAX);

  if (cwd != nullptr) {
    return cwd;
  }

#if defined(__linux__)
  if (errno != ENOENT) {
    return "";
  }
  // Read cwd symlink directly if it leads to the deleted path.
  int r = readlink("/proc/self/cwd", buf, sizeof(buf));
  if (r == -1) {
    return "";
  }

  if (r >= kDeletedLen && !strcmp(buf + r - kDeletedLen, " (deleted)")) {
    buf[r - kDeletedLen] = 0;
  }
  return buf;
#else
  // /proc/self/cwd is not available.
  return "";
#endif
}

std::string Process::GetHomeDirectory() {
  string ret;

  const char *home = getenv("HOME");
  if (home && *home) {
    ret = home;
  } else {
#ifdef _MSC_VER
    PWSTR path;
    if (SHGetKnownFolderPath(FOLDERID_UsersFiles, 0, nullptr, &path) == S_OK) {
      char hPath[PATH_MAX];
      size_t len = wcstombs(hPath, path, MAX_PATH);
      CoTaskMemFree(path);
      ret = std::string(hPath, len);
    }
#else
    passwd *pwd = getpwent();
    if (pwd && pwd->pw_dir) {
      ret = pwd->pw_dir;
    }
#endif
  }

  if (ret.empty() || ret[ret.size() - 1] != '/') {
    ret += '/';
  }
  return ret;
}

void Process::SetCoreDumpHugePages() {
#if defined(__linux__)
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
#endif
}

ProcStatus::ProcStatus() {
#ifdef __linux__
  if (FILE* f = fopen("/proc/self/status", "r")) {
    char line[128];
    while (fgets(line, sizeof(line), f)) {
      if (!strncmp(line, "VmSize:", 7)) {
        VmSizeKb = readSize(line, true);
      } else if (!strncmp(line, "VmRSS:", 6)) {
        VmRSSKb = readSize(line, true);
      } else if (!strncmp(line, "VmHWM:", 6)) {
        VmHWMKb = readSize(line, true);
      } else if (!strncmp(line, "HugetlbPages:", 13)) {
        HugetlbPagesKb = readSize(line, true);
      } else if (!strncmp(line, "Threads:", 8)) {
        Threads = readSize(line, false);
      }
    }
    fclose(f);
    if (!valid()) return;
    adjustedRSSKb = VmRSSKb + HugetlbPagesKb;
    VmHWMKb += HugetlbPagesKb;
  }
#endif
}

bool Process::OOMScoreAdj(int adj) {
#ifdef __linux__
  if (adj >= -1000 && adj < 1000) {
    if (auto f = fopen("/proc/self/oom_score_adj", "r+")) {
      fprintf(f, "%d", adj);
      fclose(f);
      return true;
    }
  }
#endif
  return false;
}

int Process::Relaunch() {
  if (!Argv) {
    errno = EINVAL;
    return -1;
  }
  return execvp(Argv[0], Argv);
}

///////////////////////////////////////////////////////////////////////////////
}
