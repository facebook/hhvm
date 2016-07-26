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

#include "hphp/util/process-exec.h"

#include <sys/types.h>
#include <cstdlib>

#ifdef _MSC_VER
#include <lmcons.h>
#include <ShlObj.h>
#include <Windows.h>
#else
#include <sys/utsname.h>
#include <sys/wait.h>
#include <pwd.h>
#include <folly/portability/Sockets.h>
#include <folly/portability/Unistd.h>
#endif

#include "hphp/util/logger.h"
#include "hphp/util/text-color.h"

#include <folly/String.h>

namespace HPHP { namespace proc {
////////////////////////////////////////////////////////////////////////////////

namespace {
////////////////////////////////////////////////////////////////////////////////

struct CPipe {
  CPipe()  { m_fds[0] = m_fds[1] = 0;}
  ~CPipe() { close();}

  bool open()  { close(); return !pipe(m_fds);}
  void close() {
    for (int i = 0; i <= 1; i++) {
      if (m_fds[i]) {
        ::close(m_fds[i]);
        m_fds[i] = 0;
      }
    }
  }

  int getIn() const  { return m_fds[1];}
  int getOut() const { return m_fds[0];}
  int detachIn()  { int fd = m_fds[1]; m_fds[1] = 0; return fd;}
  int detachOut() { int fd = m_fds[0]; m_fds[0] = 0; return fd;}
  bool dupIn2(int fd) { return dup2(m_fds[1], fd) >= 0;}
  bool dupOut2(int fd) { return dup2(m_fds[0], fd) >= 0;}

private:
  int m_fds[2];
};

////////////////////////////////////////////////////////////////////////////////

int exec(const char* path, const char* argv[], int* fdin, int* fdout, int* fderr
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
  auto args = (char*)malloc(sizeof(char) * (arglen + 1));
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
  auto const pid = (int)procInfo->dwProcessId;
#else
  auto const pid = fork();
  if (pid < 0) {
    Logger::Error("Unable to fork: %d %s", errno,
                  folly::errnoStr(errno).c_str());
    return 0;
  }
  if (pid == 0) {
    /*
     * I don't know why, but things work a lot better if this process ignores
     * the tstp signal (ctrl-Z).  If not, it locks up if you hit ctrl-Z then
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

////////////////////////////////////////////////////////////////////////////////
}

bool exec(const char* path, const char* argv[], const char* in,
          std::string& out, std::string* err /* = nullptr */,
          bool color /* = false */) {

  int fdin = 0; int fdout = 0; int fderr = 0;
#ifdef _MSC_VER
  PROCESS_INFORMATION procInf;
  auto const pid = exec(path, argv, &fdin, &fdout, &fderr, &procInf);
#else
  auto const pid = exec(path, argv, &fdin, &fdout, &fderr);
#endif
  if (pid == 0) return false;

  {
    auto const sin = fdopen(fdin, "w");
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
        auto const e = read(fdout, buffer, sizeof buffer);
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
        auto const e = read(fderr, buffer, sizeof buffer);
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
  } else {
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

////////////////////////////////////////////////////////////////////////////////

/*
 * Copied from http://www-theorie.physik.unizh.ch/~dpotter/howto/daemonize
 */
void daemonize(const char* stdoutFile /* = "/dev/null" */,
               const char* stderrFile /* = "/dev/null" */) {
  // Already a daemon.
  if (getppid() == 1) return;

#ifdef _MSC_VER
  // We are Windows, fear us!
  umask(0);
#else
  // Fork off the parent process.
  auto const pid = fork();
  if (pid < 0) {
    exit(EXIT_FAILURE);
  }
  // If we got a good PID, then we can exit the parent process.
  if (pid > 0) {
    exit(EXIT_SUCCESS);
  }

  // At this point we are executing as the child process.

  // Change the file mode mask.
  umask(0);

  // Create a new SID for the child process.
  auto const sid = setsid();
  if (sid < 0) {
    exit(EXIT_FAILURE);
  }
#endif

  // Change the current working directory.  This prevents the current directory
  // from being locked; hence not being able to remove it.
  if ((chdir("/")) < 0) {
    exit(EXIT_FAILURE);
  }

  // Redirect standard files to /dev/null.
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

////////////////////////////////////////////////////////////////////////////////
}}
