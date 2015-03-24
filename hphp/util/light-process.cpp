/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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
#include "hphp/util/light-process.h"

#include <string>
#include <vector>

#include <boost/scoped_array.hpp>

#include <sys/types.h>
#include <sys/wait.h>
#include <sys/socket.h>

#include <afdt.h>
#include <grp.h>
#include <stdlib.h>
#include <unistd.h>
#include <poll.h>
#include <pwd.h>
#include <signal.h>

#include <folly/String.h>

#include "hphp/util/process.h"
#include "hphp/util/logger.h"
#include "hphp/util/afdt-util.h"

namespace HPHP {

///////////////////////////////////////////////////////////////////////////////
// helper functions

Mutex LightProcess::s_mutex;

using afdt::send_fd;
using afdt::recv_fd;

static char **build_envp(const std::vector<std::string> &env) {
  char **envp = nullptr;
  int size = env.size();
  if (size) {
    envp = (char **)malloc((size + 1) * sizeof(char *));
    int j = 0;
    for (unsigned int i = 0; i < env.size(); i++, j++) {
      *(envp + j) = (char *)env[i].c_str();
    }
    *(envp + j) = nullptr;
  }
  return envp;
}

static void close_fds(const std::vector<int> &fds) {
  for (auto fd : fds) {
    ::close(fd);
  }
}

template<class Head, class... Tail>
void lwp_write(int afdt_fd, const Head& h, Tail&&... args) {
  if (afdt::sendRaw(afdt_fd, h, std::forward<Tail>(args)...)) {
    throw Exception("Failed in afdt::sendRaw: %s",
                    folly::errnoStr(errno).c_str());
  }
}

template<class Head, class... Tail>
void lwp_read(int afdt_fd, Head& h, Tail&... args) {
  if (afdt::recvRaw(afdt_fd, h, args...)) {
    throw Exception("Failed in afdt::recvRaw: %s",
                    folly::errnoStr(errno).c_str());
  }
}

///////////////////////////////////////////////////////////////////////////////
// shadow process tasks

static void do_popen(int afdt_fd) {
  std::string mode, buf, cwd;

  lwp_read(afdt_fd, mode, buf, cwd);
  bool read_only = (mode[0] == 'r');

  std::string old_cwd;

  if (!cwd.empty()) {
    old_cwd = Process::GetCurrentDirectory();
    if (old_cwd != cwd) {
      if (chdir(cwd.c_str())) {
        // Ignore chdir failures, because the compiled version might not
        // have the directory any more.
        Logger::Warning("Light Process failed chdir to %s.", cwd.c_str());
      }
    }
  }

  FILE *f = buf[0] ? ::popen(buf.c_str(), read_only ? "r" : "w") : nullptr;

  if (!old_cwd.empty() && chdir(old_cwd.c_str())) {
    // only here if we can't change the cwd back
  }

  if (f == nullptr) {
    Logger::Error("Light process failed popen: %d (%s).", errno,
                  folly::errnoStr(errno).c_str());
    lwp_write(afdt_fd, "error");
  } else {
    lwp_write(afdt_fd, "success", (int64_t)f);
    int fd = fileno(f);
    send_fd(afdt_fd, fd);
  }
}

static void do_pclose(int afdt_fd) {
  int64_t fptr = 0;
  lwp_read(afdt_fd, fptr);
  FILE *f = (FILE *)fptr;
  int32_t ret = ::pclose(f);

  lwp_write(afdt_fd, ret);
  if (ret < 0) {
    lwp_write(afdt_fd, static_cast<int32_t>(errno));
  }
}

static void do_proc_open(int afdt_fd) {
  std::string cmd, cwd;
  std::vector<std::string> env;
  std::vector<int> pvals;
  lwp_read(afdt_fd, cmd, cwd, env, pvals);

  std::vector<int> pkeys;
  for (int i = 0; i < pvals.size(); i++) {
    int fd = recv_fd(afdt_fd);
    if (fd < 0) {
      lwp_write(afdt_fd, "error", (int32_t)EPROTO);
      close_fds(pkeys);
      return;
    }
    pkeys.push_back(fd);
  }

  // indicate error if an empty command was received
  if (cmd.length() == 0) {
    lwp_write(afdt_fd, "error", (int32_t)ENOENT);
    return;
  }

  // now ready to start the child process
  pid_t child = fork();
  if (child == 0) {
    for (int i = 0; i < pvals.size(); i++) {
      dup2(pkeys[i], pvals[i]);
    }
    if (!cwd.empty() && chdir(cwd.c_str())) {
      // non-zero for error
      // chdir failed, the working directory remains unchanged
    }
    if (!env.empty()) {
      char **envp = build_envp(env);
      execle("/bin/sh", "sh", "-c", cmd.c_str(), nullptr, envp);
      free(envp);
    } else {
      execl("/bin/sh", "sh", "-c", cmd.c_str(), nullptr);
    }
    _Exit(HPHP_EXIT_FAILURE);
  } else if (child > 0) {
    // successfully created the child process
    lwp_write(afdt_fd, "success", (int64_t)child);
  } else {
    // failed creating the child process
    lwp_write(afdt_fd, "error", (int32_t)errno);
  }

  close_fds(pkeys);
}

static pid_t waited = 0;

static void kill_handler(int sig) {
  if (sig == SIGALRM && waited) {
    kill(waited, SIGKILL);
  }
}

static void do_waitpid(int afdt_fd) {
  int64_t p = -1;
  int options = 0;
  int timeout = 0;
  lwp_read(afdt_fd, p, options, timeout);

  pid_t pid = (pid_t)p;
  int stat;
  if (timeout > 0) {
    waited = pid;
    signal(SIGALRM, kill_handler);
    alarm(timeout);
  }

  pid_t ret = ::waitpid(pid, &stat, options);
  alarm(0); // cancel the previous alarm if not triggered yet
  waited = 0;
  lwp_write(afdt_fd, (int64_t)ret, (int32_t)stat);
  if (ret < 0) {
    lwp_write(afdt_fd, (int32_t)errno);
  }
}

static void do_change_user(int afdt_fd) {
  std::string uname;
  lwp_read(afdt_fd, uname);
  if (uname.length() > 0) {
    struct passwd *pw = getpwnam(uname.c_str());
    if (pw) {
      if (pw->pw_gid) {
        initgroups(pw->pw_name, pw->pw_gid);
        setgid(pw->pw_gid);
      }
      if (pw->pw_uid) {
        setuid(pw->pw_uid);
      }
    }
  }
}

///////////////////////////////////////////////////////////////////////////////
// light-weight process

static boost::scoped_array<LightProcess> g_procs;
static int g_procsCount = 0;
static bool s_handlerInited = false;
static LightProcess::LostChildHandler s_lostChildHandler;

LightProcess::LightProcess() : m_shadowProcess(0), m_afdt_fd(-1) { }

LightProcess::~LightProcess() {
}

void LightProcess::SigChldHandler(int sig, siginfo_t* info, void* ctx) {
  if (info->si_code != CLD_EXITED &&
      info->si_code != CLD_KILLED &&
      info->si_code != CLD_DUMPED) {
    return;
  }
  pid_t pid = info->si_pid;
  for (int i = 0; i < g_procsCount; ++i) {
    if (g_procs && g_procs[i].m_shadowProcess == pid) {
      // The exited process was a light process. Notify the callback, if any.
      if (s_lostChildHandler) {
        s_lostChildHandler(pid);
      }
      break;
    }
  }
}

void LightProcess::Initialize(const std::string &prefix, int count,
                              const std::vector<int> &inherited_fds) {
  if (prefix.empty() || count <= 0) {
    return;
  }

  if (Available()) {
    // already initialized
    return;
  }

  g_procs.reset(new LightProcess[count]);
  g_procsCount = count;

  auto afdt_filename = folly::sformat("{}.{}", prefix, getpid());

  // remove the possible leftover
  remove(afdt_filename.c_str());

  afdt_error_t err = AFDT_ERROR_T_INIT;
  auto afdt_lid = afdt_listen(afdt_filename.c_str(), &err);
  if (afdt_lid < 0) {
    Logger::Warning("Unable to afdt_listen to %s: %d %s",
                    afdt_filename.c_str(),
                    errno, folly::errnoStr(errno).c_str());
    return;
  }

  SCOPE_EXIT {
    ::close(afdt_lid);
    remove(afdt_filename.c_str());
  };

  for (int i = 0; i < count; i++) {
    if (!g_procs[i].initShadow(afdt_lid, afdt_filename, i, inherited_fds)) {
      for (int j = 0; j < i; j++) {
        g_procs[j].closeShadow();
      }
      g_procs.reset();
      g_procsCount = 0;
      break;
    }
  }

  if (!s_handlerInited) {
    struct sigaction sa;
    struct sigaction old_sa;
    sa.sa_sigaction = &LightProcess::SigChldHandler;
    sa.sa_flags = SA_SIGINFO | SA_NOCLDSTOP;
    if (sigaction(SIGCHLD, &sa, &old_sa) != 0) {
      Logger::Error("Couldn't install SIGCHLD handler");
      abort();
    }
    s_handlerInited = true;
  }
}

bool LightProcess::initShadow(int afdt_lid,
                              const std::string &afdt_filename, int id,
                              const std::vector<int> &inherited_fds) {
  Lock lock(m_procMutex);

  pid_t child = fork();
  if (child == 0) {
    // child
    Logger::ResetPid();
    pid_t sid = setsid();
    if (sid < 0) {
      Logger::Warning("Unable to setsid");
      exit(HPHP_EXIT_FAILURE);
    }
    afdt_error_t err = AFDT_ERROR_T_INIT;
    m_afdt_fd = afdt_connect(afdt_filename.c_str(), &err);
    if (m_afdt_fd < 0) {
      Logger::Warning("Unable to afdt_connect, filename %s: %d %s",
                      afdt_filename.c_str(),
                      errno, folly::errnoStr(errno).c_str());
      exit(HPHP_EXIT_FAILURE);
    }

    // don't hold on to previous light processes' pipes, inherited
    // fds, or the afdt listening socket
    for (int i = 0; i < id; i++) {
      g_procs[i].closeFiles();
    }
    close_fds(inherited_fds);
    ::close(afdt_lid);

    runShadow();
  } else if (child < 0) {
    // failed
    Logger::Warning("Unable to fork lightly: %d %s", errno,
                    folly::errnoStr(errno).c_str());
    return false;
  } else {
    // parent
    m_shadowProcess = child;

    sockaddr addr;
    socklen_t addrlen = sizeof(addr);
    m_afdt_fd = accept(afdt_lid, &addr, &addrlen);
    if (m_afdt_fd < 0) {
      Logger::Warning("Unable to establish afdt connection: %d %s",
                      errno, folly::errnoStr(errno).c_str());
      closeShadow();
      return false;
    }
  }
  return true;
}

void LightProcess::Close() {
  boost::scoped_array<LightProcess> procs;
  procs.swap(g_procs);
  int count = g_procsCount;
  g_procs.reset();
  g_procsCount = 0;

  for (int i = 0; i < count; i++) {
    procs[i].closeShadow();
  }
}

void LightProcess::closeShadow() {
  Lock lock(m_procMutex);
  if (m_shadowProcess) {
    lwp_write(m_afdt_fd, "exit");
    // removes the "zombie" process, so not to interfere with later waits
    ::waitpid(m_shadowProcess, nullptr, 0);
  }
  if (m_afdt_fd >= 0) {
    ::close(m_afdt_fd);
    m_afdt_fd = -1;
  }
  m_shadowProcess = 0;
}

void LightProcess::closeFiles() {
  ::close(m_afdt_fd);
}

bool LightProcess::Available() {
  return g_procsCount > 0;
}

void LightProcess::runShadow() {
  std::string buf;

  pollfd pfd[1];
  pfd[0].fd = m_afdt_fd;
  pfd[0].events = POLLIN;
  try {
    while (true) {
      int ret = poll(pfd, 1, -1);
      if (ret < 0 && errno == EINTR) {
        continue;
      }
      if (pfd[0].revents & POLLHUP) {
        // no more command can come in
        Logger::Error("Lost parent, LightProcess exiting");
        break;
      }
      if (pfd[0].revents & POLLIN) {
        lwp_read(m_afdt_fd, buf);
        if (buf == "exit") {
          Logger::Verbose("LightProcess exiting upon request");
          break;
        } else if (buf == "popen") {
          do_popen(m_afdt_fd);
        } else if (buf == "pclose") {
          do_pclose(m_afdt_fd);
        } else if (buf == "proc_open") {
          do_proc_open(m_afdt_fd);
        } else if (buf == "waitpid") {
          do_waitpid(m_afdt_fd);
        } else if (buf == "change_user") {
          do_change_user(m_afdt_fd);
        } else if (buf[0]) {
          Logger::Info("LightProcess got invalid command: %.20s", buf.c_str());
        }
      }
    }
  } catch (const std::exception& e) {
    Logger::Error("LightProcess exiting due to exception: %s", e.what());
  } catch (...) {
    Logger::Error("LightProcess exiting due to unknown exception");
  }

  ::close(m_afdt_fd);
  _Exit(0);
}

int LightProcess::GetId() {
  return (long)pthread_self() % g_procsCount;
}

FILE *LightProcess::popen(const char *cmd, const char *type,
                          const char *cwd /* = NULL */) {
  if (!Available()) {
    // fallback to normal popen
    Logger::Verbose("Light-weight fork not available; "
                    "use the heavy one instead.");
  } else {
    FILE *f = LightPopenImpl(cmd, type, cwd);
    if (f) {
      return f;
    }
    Logger::Verbose("Light-weight fork failed; use the heavy one instead.");
  }
  return HeavyPopenImpl(cmd, type, cwd);
}

FILE *LightProcess::HeavyPopenImpl(const char *cmd, const char *type,
                                   const char *cwd) {
  if (cwd && *cwd) {
    auto old_cwd = Process::GetCurrentDirectory();
    if (old_cwd != cwd) {
      Lock lock(s_mutex);
      if (chdir(cwd)) {
        Logger::Warning("Failed to chdir to %s.", cwd);
      }
      FILE *f = ::popen(cmd, type);
      if (chdir(old_cwd.c_str())) {
        // error occurred changing cwd back
      }
      return f;
    }
  }
  return ::popen(cmd, type);
}

FILE *LightProcess::LightPopenImpl(const char *cmd, const char *type,
                                   const char *cwd) {
  int id = GetId();
  Lock lock(g_procs[id].m_procMutex);

  auto fout = g_procs[id].m_afdt_fd;
  lwp_write(fout, "popen", type, cmd, cwd ? cwd : "");

  std::string buf;
  auto fin = g_procs[id].m_afdt_fd;
  lwp_read(fin, buf);
  if (buf == "error") {
    return nullptr;
  }

  int64_t fptr = 0;
  lwp_read(fin, fptr);
  if (!fptr) {
    Logger::Error("Light process failed to return the file pointer.");
    return nullptr;
  }

  int fd = recv_fd(fin);
  if (fd < 0) {
    Logger::Error("Light process failed to send the file descriptor.");
    return nullptr;
  }
  FILE *f = fdopen(fd, type);
  g_procs[id].m_popenMap[(int64_t)f] = fptr;

  return f;
}

int LightProcess::pclose(FILE *f) {
  if (!Available()) {
    return ::pclose(f);
  }

  int id = GetId();
  Lock lock(g_procs[id].m_procMutex);

  std::map<int64_t, int64_t>::iterator it = g_procs[id].m_popenMap.find((int64_t)f);
  if (it == g_procs[id].m_popenMap.end()) {
    // try to close it with normal pclose
    return ::pclose(f);
  }

  int64_t f2 = it->second;
  g_procs[id].m_popenMap.erase((int64_t)f);
  fclose(f);

  lwp_write(g_procs[id].m_afdt_fd, "pclose", f2);

  int ret = -1;
  lwp_read(g_procs[id].m_afdt_fd, ret);
  if (ret < 0) {
    lwp_read(g_procs[id].m_afdt_fd, errno);
  }
  return ret;
}

pid_t LightProcess::proc_open(const char *cmd, const std::vector<int> &created,
                              const std::vector<int> &desired,
                              const char *cwd,
                              const std::vector<std::string> &env) {
  int id = GetId();
  Lock lock(g_procs[id].m_procMutex);
  always_assert(Available());
  always_assert(created.size() == desired.size());

  auto fout = g_procs[id].m_afdt_fd;
  lwp_write(fout, "proc_open", cmd, cwd ? cwd : "",
            env, desired);

  bool error_send = false;
  int save_errno = 0;
  for (auto cfd : created) {
    if (!send_fd(g_procs[id].m_afdt_fd, cfd)) {
      error_send = true;
      save_errno = errno;
      break;
    }
  }

  std::string buf;
  auto fin = g_procs[id].m_afdt_fd;
  lwp_read(fin, buf);
  if (buf == "error") {
    lwp_read(fin, errno);
    if (error_send) {
      // On this error, the receiver side returns dummy errno,
      // use the sender side errno here.
      errno = save_errno;
    }
    return -1;
  }
  always_assert_flog(buf == "success",
                     "Unexpected message from light process: `{}'", buf);
  int64_t pid = -1;
  lwp_read(fin, pid);
  always_assert(pid);
  return (pid_t)pid;
}

pid_t LightProcess::waitpid(pid_t pid, int *stat_loc, int options,
                            int timeout) {
  if (!Available()) {
    // light process is not really there
    return ::waitpid(pid, stat_loc, options);
  }

  int id = GetId();
  Lock lock(g_procs[id].m_procMutex);

  auto fout = g_procs[id].m_afdt_fd;
  lwp_write(fout, "waitpid", (int64_t)pid, options, timeout);

  int64_t ret;
  int stat;
  auto fin = g_procs[id].m_afdt_fd;
  lwp_read(fin, ret, stat);

  *stat_loc = stat;
  if (ret < 0) {
    lwp_read(fin, errno);
  }
  return (pid_t)ret;
}

pid_t LightProcess::pcntl_waitpid(pid_t pid, int *stat_loc, int options) {
  if (!Available()) {
    return ::waitpid(pid, stat_loc, options);
  }

  int id = GetId();
  Lock lock(g_procs[id].m_procMutex);

  pid_t p = ::waitpid(pid, stat_loc, options);
  if (p == g_procs[id].m_shadowProcess) {
    // got the shadow process, wait again
    p = ::waitpid(pid, stat_loc, options);
  }

  return p;
}

void LightProcess::ChangeUser(const std::string &username) {
  if (username.empty()) return;
  for (int i = 0; i < g_procsCount; i++) {
    Lock lock(g_procs[i].m_procMutex);
    lwp_write(g_procs[i].m_afdt_fd, "change_user", username);
  }
}

void LightProcess::SetLostChildHandler(const LostChildHandler& handler) {
  s_lostChildHandler = handler;
}

///////////////////////////////////////////////////////////////////////////////
}
