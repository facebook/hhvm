/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2016 Facebook, Inc. (http://www.facebook.com)     |
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
#include <poll.h>
#include <unistd.h>
#endif

#include <folly/Conv.h>
#include <folly/ScopeGuard.h>
#include <folly/String.h>

#include <boost/filesystem.hpp>

#include "hphp/util/logger.h"
#include "hphp/util/async-func.h"
#include "hphp/util/text-color.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////
// helpers

using std::string;

static void swap_fd(const string &filename, FILE *fdesc) {
  FILE *f = fopen(filename.c_str(), "a");
  if (f == nullptr || dup2(fileno(f), fileno(fdesc)) < 0) {
    if (f) fclose(f);
    _Exit(HPHP_EXIT_FAILURE);
  }
}

///////////////////////////////////////////////////////////////////////////////

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

bool Process::Exec(const char *path, const char *argv[], const char *in,
                   string &out, string *err /* = NULL */,
                   bool color /* = false */) {

  int fdin = 0; int fdout = 0; int fderr = 0;
#ifdef _MSC_VER
  PROCESS_INFORMATION procInf;
  int pid = Exec(path, argv, &fdin, &fdout, &fderr, &procInf);
#else
  int pid = Exec(path, argv, &fdin, &fdout, &fderr);
#endif
  if (pid == 0) return false;

  {
    FILE* sin = fdopen(fdin, "w");
    if (!sin) return false;
    SCOPE_EXIT { fclose(sin); };
    if (in && *in) {
      fwrite(in, 1, strlen(in), sin);
    }
  }

  char buffer[4096];
  if (fcntl(fdout, F_SETFL, O_NONBLOCK)) {
    perror("fcntl failed on fdout");
  }

  if (fcntl(fderr, F_SETFL, O_NONBLOCK)) {
    perror("fcntl failed on fderr");
  }

#ifdef _MSC_VER
  HANDLE fds[2];
  DWORD handleCount = 0;
  if (fdout)
    fds[handleCount++] = (HANDLE)_get_osfhandle(fdout);
  if (fderr)
    fds[handleCount++] = (HANDLE)_get_osfhandle(fderr);
#endif

  while (fdout || fderr) {
#ifdef _MSC_VER
    DWORD res =
      WaitForMultipleObjectsEx(handleCount, fds, false, INFINITE, true);
    if (res == WAIT_IO_COMPLETION)
      continue;

    DWORD hc = 0;
#else
    pollfd fds[2];
    int n = 0;
    if (fdout) {
      fds[n].fd = fdout;
      fds[n].events = POLLIN | POLLHUP;
      n++;
    }
    if (fderr) {
      fds[n].fd = fderr;
      fds[n].events = POLLIN | POLLHUP;
      n++;
    }

    n = poll(fds, n, -1);
    if (n < 0) {
      continue;
    }

    n = 0;
#endif
    if (fdout) {
#ifdef _MSC_VER
      if (res == WAIT_OBJECT_0 + hc++ || res == WAIT_ABANDONED_0 + hc++) {
#else
      if (fds[n++].revents & (POLLIN | POLLHUP)) {
#endif
        int e = read(fdout, buffer, sizeof buffer);
        if (e <= 0) {
          close(fdout);
          fdout = 0;
        } else {
          if (color && s_stdout_color) {
            out.append(s_stdout_color);
            out.append(buffer, e);
            out.append(ANSI_COLOR_END);
          } else {
            out.append(buffer, e);
          }
        }
      }
    }

    if (fderr) {
#ifdef _MSC_VER
      if (res == WAIT_OBJECT_0 + hc++ || res == WAIT_ABANDONED_0 + hc++) {
#else
      if (fds[n++].revents & (POLLIN | POLLHUP)) {
#endif
        int e = read(fderr, buffer, sizeof buffer);
        if (e <= 0) {
          close(fderr);
          fderr = 0;
        } else if (err) {
          if (color && s_stdout_color) {
            err->append(s_stderr_color);
            err->append(buffer, e);
            err->append(ANSI_COLOR_END);
          } else {
            err->append(buffer, e);
          }
        }
      }
    }
  }

  int status;
  bool ret = false;
#ifdef _MSC_VER
  if (WaitForSingleObject(procInf.hProcess, INFINITE) == WAIT_FAILED) {
    Logger::Error("Failed to wait for `%s'\n", path);
  }
  DWORD st;
  if (GetExitCodeProcess(procInf.hProcess, &st))
    Logger::Error("Failed to get the process exit code\n");
  status = (int)st;

  if (status != EXIT_SUCCESS) {
      Logger::Verbose("Status %d running command: `%s'\n", status, path);
      if (argv) {
        while (*argv) {
          Logger::Verbose("  arg: `%s'\n", *argv);
          argv++;
        }
      }
  }
  else {
    ret = true;
  }

  CloseHandle(procInf.hProcess);
  CloseHandle(procInf.hThread);
#else
  if (waitpid(pid, &status, 0) != pid) {
    Logger::Error("Failed to wait for `%s'\n", path);
  } else if (WIFEXITED(status)) {
    if (WEXITSTATUS(status) != 0) {
      Logger::Verbose("Status %d running command: `%s'\n",
                      WEXITSTATUS(status), path);
      if (argv) {
        while (*argv) {
          Logger::Verbose("  arg: `%s'\n", *argv);
          argv++;
        }
      }
    } else {
      ret = true;
    }
  } else {
    Logger::Verbose("Non-normal exit\n");
    if (WIFSIGNALED(status)) {
      Logger::Verbose("  signaled with %d\n", WTERMSIG(status));
    }
  }
#endif
  return ret;
}

int Process::Exec(const std::string &cmd, const std::string &outf,
                  const std::string &errf) {
#ifdef _MSC_VER
  STARTUPINFO sInf;
  PROCESS_INFORMATION pInf;
  ZeroMemory(&sInf, sizeof(sInf));
  ZeroMemory(&pInf, sizeof(pInf));
  sInf.cb = sizeof(STARTUPINFO);
  sInf.dwFlags = STARTF_USESTDHANDLES;
  sInf.hStdError = fopen(errf.c_str(), "a");
  sInf.hStdOutput = fopen(outf.c_str(), "a");
  if (!CreateProcess(cmd.c_str(), nullptr, nullptr, nullptr, true,
    0, nullptr, nullptr, &sInf, &pInf)) {
    Logger::Error("Unable to CreateProcess: %d %s", errno,
      folly::errnoStr(errno).c_str());
    return 0;
  }
  WaitForSingleObject(pInf.hProcess, INFINITE);
  DWORD status;
  GetExitCodeProcess(pInf.hProcess, &status);
  CloseHandle(pInf.hProcess);
  CloseHandle(pInf.hThread);
  return (int)status;
#else
  std::vector<std::string> argvs;
  folly::split(' ', cmd, argvs);
  if (argvs.empty()) {
    return -1;
  }

  int pid = fork();
  if (pid < 0) {
    Logger::Error("Unable to fork: %d %s", errno,
                  folly::errnoStr(errno).c_str());
    return 0;
  }
  if (pid == 0) {
    signal(SIGTSTP,SIG_IGN);

    swap_fd(outf, stdout);
    swap_fd(errf, stderr);

    int count = argvs.size();
    char **argv = (char**)calloc(count + 1, sizeof(char*));
    for (int i = 0; i < count; i++) {
      argv[i] = (char*)argvs[i].c_str();
    }
    argv[count] = nullptr;

    execvp(argv[0], argv);
    Logger::Error("Failed to exec `%s'\n", cmd.c_str());
    _Exit(HPHP_EXIT_FAILURE);
  }
  int status = -1;
  wait(&status);
  return status;
#endif
}

int Process::Exec(const char *path, const char *argv[], int *fdin, int *fdout,
                  int *fderr
#ifdef _MSC_VER
                  , PROCESS_INFORMATION* procInfo
#endif
  ) {
  CPipe pipein, pipeout, pipeerr;
  if (!pipein.open() || !pipeout.open() || !pipeerr.open()) {
    return 0;
  }

#ifdef _MSC_VER
  STARTUPINFO sInf;
  ZeroMemory(&sInf, sizeof(sInf));
  ZeroMemory(procInfo, sizeof(*procInfo));
  sInf.cb = sizeof(STARTUPINFO);
  sInf.dwFlags = STARTF_USESTDHANDLES;
  sInf.hStdInput = (HANDLE)_get_osfhandle(pipein.getOut());
  sInf.hStdError = (HANDLE)_get_osfhandle(pipeerr.getIn());
  sInf.hStdOutput = (HANDLE)_get_osfhandle(pipeout.getIn());
  size_t arglen = 0;
  for (int i = 0; argv[i]; i++) {
    arglen += strlen(argv[i]);
    if (i > 0)
      arglen++; // Space separator
  }
  char* args = (char*)malloc(sizeof(char) * (arglen + 1));
  for (int i = 0; argv[i]; i++) {
    if (i > 0)
      strcat(args, " ");
    strcat(args, argv[i]);
  }
  if (!CreateProcess(path, args, nullptr, nullptr, true,
    0, nullptr, nullptr, &sInf, procInfo)) {
    Logger::Error("Unable to CreateProcess: %d %s", errno,
      folly::errnoStr(errno).c_str());
    free(args);
    return 0;
  }
  free(args);
  int pid = (int)procInfo->dwProcessId;
#else
  int pid = fork();
  if (pid < 0) {
    Logger::Error("Unable to fork: %d %s", errno,
                  folly::errnoStr(errno).c_str());
    return 0;
  }
  if (pid == 0) {
    /**
     * I don't know why, but things work a lot better if this process ignores
     * the tstp signal (ctrl-Z). If not, it locks up if you hit ctrl-Z then
     * "bg" the program.
     */
    signal(SIGTSTP,SIG_IGN);

    if (pipein.dupOut2(fileno(stdin)) && pipeout.dupIn2(fileno(stdout)) &&
        pipeerr.dupIn2(fileno(stderr))) {
      pipeout.close(); pipeerr.close(); pipein.close();

      const char *argvnull[2] = {"", nullptr};
      execvp(path, const_cast<char**>(argv ? argv : argvnull));
    }
    Logger::Error("Failed to exec `%s'\n", path);
    _Exit(HPHP_EXIT_FAILURE);
  }
#endif
  if (fdout) *fdout = pipeout.detachOut();
  if (fderr) *fderr = pipeerr.detachOut();
  if (fdin)  *fdin  = pipein.detachIn();
#ifdef _MSC_VER
  pipein.close();
  pipeerr.close();
  pipeout.close();
#endif
  return pid;
}

/**
 * Copied from http://www-theorie.physik.unizh.ch/~dpotter/howto/daemonize
 */
void Process::Daemonize(const char *stdoutFile /* = "/dev/null" */,
                        const char *stderrFile /* = "/dev/null" */) {
  pid_t pid, sid;

  /* already a daemon */
  if (getppid() == 1) return;

#ifdef _MSC_VER
  // We are Windows, fear us!
  umask(0);
#else
  /* Fork off the parent process */
  pid = fork();
  if (pid < 0) {
    exit(EXIT_FAILURE);
  }
  /* If we got a good PID, then we can exit the parent process. */
  if (pid > 0) {
    exit(EXIT_SUCCESS);
  }

  /* At this point we are executing as the child process */

  /* Change the file mode mask */
  umask(0);

  /* Create a new SID for the child process */
  sid = setsid();
  if (sid < 0) {
    exit(EXIT_FAILURE);
  }
#endif

  /* Change the current working directory.  This prevents the current
     directory from being locked; hence not being able to remove it. */
  if ((chdir("/")) < 0) {
    exit(EXIT_FAILURE);
  }

  /* Redirect standard files to /dev/null */
  if (!freopen("/dev/null", "r", stdin)) exit(EXIT_FAILURE);
  if (stdoutFile && *stdoutFile) {
    if (!freopen(stdoutFile, "a", stdout)) exit(EXIT_FAILURE);
  } else {
    if (!freopen("/dev/null", "w", stdout)) exit(EXIT_FAILURE);
  }
  if (stderrFile && *stderrFile) {
    if (!freopen(stderrFile, "a", stderr)) exit(EXIT_FAILURE);
  } else {
    if (!freopen("/dev/null", "w", stderr)) exit(EXIT_FAILURE);
  }
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

///////////////////////////////////////////////////////////////////////////////
}
