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

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

using std::string;

static void readString(FILE *f, string &out) {
  size_t nread = 0;
  const unsigned int BUFFER_SIZE = 1024;
  char buf[BUFFER_SIZE];
  while ((nread = fread(buf, 1, BUFFER_SIZE, f)) != 0) {
    out.append(buf, nread);
  }
}

///////////////////////////////////////////////////////////////////////////////

// Cached process statics
std::string Process::HostName;
std::string Process::CurrentWorkingDirectory;

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

int64_t Process::GetProcessRSS(pid_t pid) {
  string name = "/proc/" + folly::to<string>(pid) + "/status";

  string status;
  FILE * f = fopen(name.c_str(), "r");
  if (f) {
    readString(f, status);
    fclose(f);
  }

  std::vector<std::string> lines;
  folly::split('\n', status, lines, true /* ignoreEmpty */);
  for (unsigned int i = 0; i < lines.size(); i++) {
    string &line = lines[i];
    if (line.find("VmRSS:") == 0) {
      for (unsigned int j = strlen("VmRSS:"); j < line.size(); j++) {
        if (line[j] != ' ') {
          long long mem = atoll(line.c_str() + j);
          return mem/1024;
        }
      }
    }
  }

  return 0;
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

    // Return size in MB
    auto const parseLine = [] (const char* item, const char* line) -> int64_t {
      int64_t amount = -1;
      char mult = 'b';
      char format[64];
      snprintf(format, sizeof(format), "%s: %%%s %%c", item, PRId64);
      sscanf(line, format, &amount, &mult);
      if (amount <= 0) return -1;
      if (mult == 'k' || mult == 'K') return amount >> 10;
      if (mult == 'm' || mult == 'M') return amount;
      if (mult == 'g' || mult == 'G') return amount << 10;
      return amount >> 20;
    };

    char buf[128];
    while (fgets(buf, sizeof(buf), f)) {
      info.freeMb = std::max(info.freeMb, parseLine("MemFree", buf));
      info.buffersMb = std::max(info.buffersMb, parseLine("Buffers", buf));
      info.cachedMb = std::max(info.cachedMb, parseLine("Cached", buf));
      if (info.valid()) return true;
    }
  }
  return false;
}

int Process::GetCPUCount() {
  return sysconf(_SC_NPROCESSORS_ONLN);
}

size_t Process::GetCodeFootprint(pid_t pid) {
  // /proc/<pid>/statm reports the following whitespace-separated values (in
  // terms of page counts):
  //    size       total program size
  //    resident   resident set size
  //    share      shared pages
  //    text       text (code)
  //    lib        library (unused in Linux 2.6)
  //    data       data/stack
  //    dt         dirty pages (unused in Linux 2.6)
  //
  // Return (share + text), under the assumption that share consists only of
  // shared libraries.
  string name = "/proc/" + folly::to<string>(pid) + "/statm";

  string statm;
  FILE * f = fopen(name.c_str(), "r");
  if (f) {
    readString(f, statm);
    fclose(f);
  }

  size_t pageSize = size_t(sysconf(_SC_PAGESIZE));
  size_t pos0, pos1 = 0;
#define STATM_FIELD_NEXT() do {                                               \
  pos0 = pos1;                                                                \
  pos1 = statm.find(" ", pos0) + 1;                                           \
} while (0)
#define STATM_FIELD_READ(name)                                                \
  STATM_FIELD_NEXT();                                                         \
  size_t name = strtoull(statm.substr(pos0, pos1-pos0).c_str(),               \
                         nullptr, 0) * pageSize;
  STATM_FIELD_NEXT(); // size.
  STATM_FIELD_NEXT(); // resident.
  STATM_FIELD_READ(share);
  STATM_FIELD_READ(text);
#undef STATM_FIELD_NEXT
#undef STATM_FIELD_READ
  return share + text;
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
  passwd *pwd = getpwuid(geteuid());
  if (pwd && pwd->pw_name) {
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

///////////////////////////////////////////////////////////////////////////////
}
