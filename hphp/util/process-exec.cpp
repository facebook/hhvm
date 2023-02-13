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

#include "hphp/util/process-exec.h"

#include <sys/types.h>
#include <cstdlib>

#include <sys/utsname.h>
#include <sys/wait.h>
#include <pwd.h>
#include <folly/portability/Sockets.h>
#include <folly/portability/Unistd.h>

#include "hphp/util/logger.h"
#include "hphp/util/text-color.h"

#include <folly/String.h>

namespace HPHP::proc {
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

int exec(const char* path, const char* argv[],
         int* fdin, int* fdout, int* fderr) {
  CPipe pipein, pipeout, pipeerr;
  if (!pipein.open() || !pipeout.open() || !pipeerr.open()) {
    return 0;
  }

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

  if (fdout) *fdout = pipeout.detachOut();
  if (fderr) *fderr = pipeerr.detachOut();
  if (fdin)  *fdin  = pipein.detachIn();
  return pid;
}

////////////////////////////////////////////////////////////////////////////////
}

bool exec(const char* path, const char* argv[], const char* in,
          std::string& out, std::string* err /* = nullptr */,
          bool color /* = false */) {

  int fdin = 0; int fdout = 0; int fderr = 0;
  auto const pid = exec(path, argv, &fdin, &fdout, &fderr);
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

  while (fdout || fderr) {
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
    if (fdout) {
      if (fds[n++].revents & (POLLIN | POLLHUP)) {
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
      if (fds[n++].revents & (POLLIN | POLLHUP)) {
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

static std::atomic<int> s_forkEnabledInDebugger(0);

EnableForkInDebuggerGuard::EnableForkInDebuggerGuard() {
  ++s_forkEnabledInDebugger;
}

EnableForkInDebuggerGuard::~EnableForkInDebuggerGuard() {
  --s_forkEnabledInDebugger;
}

bool EnableForkInDebuggerGuard::isForkEnabledInDebugger() {
  return s_forkEnabledInDebugger;
}

}
