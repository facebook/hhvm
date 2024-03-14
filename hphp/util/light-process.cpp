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
#include "hphp/util/light-process.h"

#include <memory>
#include <set>
#include <string>
#include <vector>

#include <boost/thread/barrier.hpp>

#include <folly/portability/SysMman.h>
#include <folly/portability/SysResource.h>
#include <folly/portability/Sockets.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <afdt.h>
#include <grp.h>
#include <stdlib.h>
#include <folly/portability/Unistd.h>

#include <pwd.h>
#include <signal.h>

#include <folly/Memory.h>
#include <folly/String.h>

#include "hphp/util/afdt-util.h"
#include "hphp/util/compatibility.h"
#include "hphp/util/configs/server.h"
#include "hphp/util/hardware-counter.h"
#include "hphp/util/hash.h"
#include "hphp/util/hugetlb.h"
#include "hphp/util/logger.h"
#include "hphp/util/process.h"
#include "hphp/util/struct-log.h"
#include "hphp/util/timer.h"
#include "hphp/util/user-info.h"

namespace HPHP {

///////////////////////////////////////////////////////////////////////////////
// helper functions

namespace {

__thread LightProcess* tl_proc;
bool s_trackProcessTimes = false;
Mutex s_mutex;

using afdt::send_fd;
using afdt::recv_fd;

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

void close_fds(const std::vector<int> &fds) {
  for (auto fd : fds) {
    ::close(fd);
  }
}

template<class Head, class... Tail>
void lwp_write(int afdt_fd, const Head& h, Tail&&... args) {
  try {
    afdt::sendx(afdt_fd, h, std::forward<Tail>(args)...);
  } catch (const std::runtime_error& ex) {
    throw Exception("Failed in afdt::sendx: %s", ex.what());
  }
}

template<class Head, class... Tail>
void lwp_read(int afdt_fd, Head& h, Tail&... args) {
  try {
    afdt::recvx(afdt_fd, h, args...);
  } catch (const std::runtime_error& ex) {
    throw Exception("Failed in afdt::recvx: %s", ex.what());
  }
}

// not thread safe - needs a lock when called from the main
// process.
int popen_impl(const char* cmd, const char* mode, pid_t* out_pid) {
  int p[2];
  auto const read = *mode == 'r';
  if (!read && *mode != 'w') return -1;

  if (pipe2(p, O_CLOEXEC) < 0) {
    return -1;
  }

  auto pid = fork();
  if (pid < 0) {
    close(p[0]);
    close(p[1]);
    return -1;
  }
  int child_pipe = read ? 1 : 0;
  if (pid == 0) {
    // child
    mprotect_1g_pages(PROT_READ);
    // If anything goes wrong, let the OOM killer kill this child process.
    Process::OOMScoreAdj(1000);
    // replace stdin or stdout with the appropriate end
    // of the pipe
    if (p[child_pipe] == child_pipe) {
      // pretty unlikely, but if it was we must clear CLOEXEC,
      // and the only way to do that is to dup it to a new fd,
      // and then dup2 it back
      p[child_pipe] = fcntl(child_pipe, F_DUPFD_CLOEXEC, 3);
    }
    dup2(p[child_pipe], child_pipe);
    // no need to close p[child_pipe] because of O_CLOEXEC

    signal(SIGINT, SIG_DFL);
    sigset_t eset;
    sigemptyset(&eset);
    sigprocmask(SIG_SETMASK, &eset, nullptr);
    execl("/bin/sh", "sh", "-c", cmd, nullptr);
    Logger::Warning("Failed to exec: `%s'", cmd);
    _Exit(HPHP_EXIT_FAILURE);
  }
  // parent

  // close the pipe we're not using
  close(p[child_pipe]);
  *out_pid = pid;
  return p[1-child_pipe];
}

int64_t ru2microseconds(const rusage& ru) {
  int64_t time_us = ru.ru_utime.tv_usec;
  time_us += ru.ru_stime.tv_usec;
  int64_t time_s = ru.ru_utime.tv_sec;
  time_s += ru.ru_stime.tv_sec;
  return time_us + time_s * 1000000;
}

/*
 * Hardware counters can be configured to measure sub-process times,
 * but note that any given LightProcess will be running jobs for
 * multiple request threads (each request thread binds to a single
 * LightProcess, but its a many to one mapping).
 *
 * This means that between the fork/exec and the waitpid, the LightProcess
 * could fork/exec more processes for different requests. The solution
 * is to fork/exec in a thread, and start the hardware counters there.
 *
 * The hardware counter will then measure time for that thread, and all
 * its children - which is exactly what we want.
 *
 * HardwareCounterWrapper & co take care of that.
 */
struct HardwareCounterWrapperArg {
  boost::barrier barrier{2};
  pthread_t thr;
  int afdt_fd;
  pid_t (*func)(int);
  pid_t pid;
  int64_t *events;
};

std::map<pid_t, std::unique_ptr<HardwareCounterWrapperArg>> s_pidToHCWMap;

void* hardwareCounterWrapper(void* varg) {
  auto arg = (HardwareCounterWrapperArg*)varg;

  HardwareCounter::s_counter.getCheck();
  HardwareCounter::Reset();
  arg->pid = arg->func(arg->afdt_fd);
  // tell the main thread that pid has been set.
  arg->barrier.wait();
  if (arg->pid < 0) return nullptr;

  // Wait until the main thread is ready to join us.
  // Note that even though we have multiple threads running in the LightProcess
  // now, at the time any of the do_* functions are called, any live threads are
  // waiting here on this barrier; so there is no problem with fork, malloc,
  // exec.
  arg->barrier.wait();
  HardwareCounter::GetPerfEvents(
    [](const std::string& event, int64_t value, void* data) {
      auto events = reinterpret_cast<int64_t*>(data);
      if (event == "instructions") {
        events[0] = value;
      } else if (event == "loads") {
        events[1] = value;
      } else if (event == "stores") {
        events[2] = value;
      }
    },
    arg->events);

  return nullptr;
}

void hardwareCounterWrapperHelper(pid_t (*func)(int), int afdt_fd) {
  if (!s_trackProcessTimes) {
    func(afdt_fd);
    return;
  }

  auto arg = std::make_unique<HardwareCounterWrapperArg>();
  arg->afdt_fd = afdt_fd;
  arg->func = func;
  if (pthread_create(&arg->thr, nullptr, hardwareCounterWrapper, arg.get())) {
    throw Exception("Failed to pthread_create: %s",
                    folly::errnoStr(errno).c_str());
  }
  // Wait for the pid to be set. Note that we must not add any code here that
  // could cause issues for the fork (eg malloc); we should wait on the barrier
  // immediately after the thread is created.
  arg->barrier.wait();
  if (arg->pid > 0) {
    // successfully forked, so don't join until waitpid.
    auto& map_entry = s_pidToHCWMap[arg->pid];
    map_entry = std::move(arg);
  } else {
    // fork failed, join now.
    pthread_join(arg->thr, nullptr);
  }
}

///////////////////////////////////////////////////////////////////////////////
// shadow process tasks

pid_t do_popen_helper(int afdt_fd) {
  std::string mode, buf, cwd;

  lwp_read(afdt_fd, mode, buf, cwd);

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

  pid_t pid;

  auto fd = buf.empty() ? -1 :
    popen_impl(buf.c_str(), mode.c_str(), &pid);

  if (!old_cwd.empty() && chdir(old_cwd.c_str())) {
    // only here if we can't change the cwd back
  }

  if (fd < 0) {
    Logger::Error("Light process failed popen: %d (%s).", errno,
                  folly::errnoStr(errno).c_str());
    lwp_write(afdt_fd, "error");
  } else {
    lwp_write(afdt_fd, "success", pid);
    send_fd(afdt_fd, fd);
    // the fd is now owned by the main process, close our copy
    close(fd);
  }
  return pid;
}

void do_popen(int afdt_fd) {
  hardwareCounterWrapperHelper(do_popen_helper, afdt_fd);
}

pid_t do_fork_and_execve_helper(int afdt_fd) {
  std::string path, cwd;
  std::vector<std::string> argv, envp;
  int flags, pgid;
  size_t fd_count;

  lwp_read(afdt_fd, path, cwd, argv, envp, flags, pgid, fd_count);
  std::map<int, int> fds;
  for (size_t i = 0; i < fd_count; ++i) {
    int target_fd;
    lwp_read(afdt_fd, target_fd);
    int current_fd = recv_fd(afdt_fd);
    fds[target_fd] = current_fd;
  }

  pid_t pid = Process::ForkAndExecve(path, argv, envp, cwd, fds, flags, pgid);
  if (pid> 0) {
    lwp_write(afdt_fd, "success", pid);
  } else {
    lwp_write(afdt_fd, "error", (int) pid, (int) errno);
  }

  for (auto [_target, fd] : fds) {
    close(fd);
  }
  return pid;
}

void do_fork_and_execve(int afdt_fd) {
  hardwareCounterWrapperHelper(do_fork_and_execve_helper, afdt_fd);
}

pid_t do_proc_open_helper(int afdt_fd) {
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
      return -1;
    }
    pkeys.push_back(fd);
  }

  // indicate error if an empty command was received
  if (cmd.length() == 0) {
    lwp_write(afdt_fd, "error", (int32_t)ENOENT);
    return -1;
  }

  // now ready to start the child process
  pid_t child = fork();
  if (child == 0) {
    mprotect_1g_pages(PROT_READ);
    Process::OOMScoreAdj(1000);
    std::map<int, int> dup_fds;
    for (int i = 0; i < pvals.size(); i++) {
      dup_fds[pvals[i]] = pkeys[i];
    }
    Process::RemapFDsPreExec(dup_fds);

    if (!cwd.empty() && chdir(cwd.c_str())) {
      // non-zero for error
      // chdir failed, the working directory remains unchanged
    }
    if (!env.empty()) {
      char **envp = build_cstrarr(env);
      execle("/bin/sh", "sh", "-c", cmd.c_str(), nullptr, envp);
      free(envp);
    } else {
      execl("/bin/sh", "sh", "-c", cmd.c_str(), nullptr);
    }
    _Exit(HPHP_EXIT_FAILURE);
  }

  if (child > 0) {
    // successfully created the child process
    lwp_write(afdt_fd, "success", child);
  } else {
    // failed creating the child process
    lwp_write(afdt_fd, "error", errno);
  }

  close_fds(pkeys);
  return child;
}

void do_proc_open(int afdt_fd) {
  hardwareCounterWrapperHelper(do_proc_open_helper, afdt_fd);
}

pid_t waited = 0;

void kill_handler(int sig) {
  if (sig == SIGALRM && waited) {
    kill(waited, SIGKILL);
  }
}

void do_waitpid(int afdt_fd) {
  pid_t pid = -1;
  int options = 0;
  int timeout = 0;
  lwp_read(afdt_fd, pid, options, timeout);

  int stat;
  if (timeout > 0) {
    waited = pid;
    signal(SIGALRM, kill_handler);
    alarm(timeout);
  }

  rusage ru;
  int64_t time_us = 0;
  const auto ret = ::wait4(pid, &stat, options, &ru);
  alarm(0); // cancel the previous alarm if not triggered yet
  waited = 0;
  int64_t events[] = { 0, 0, 0 };
  if (ret > 0 && s_trackProcessTimes) {
    time_us = ru2microseconds(ru);
    auto it = s_pidToHCWMap.find(ret);
    if (it == s_pidToHCWMap.end()) {
      throw Exception("pid not in map: %s",
                      folly::errnoStr(errno).c_str());
    }

    auto hcw = std::move(it->second);
    s_pidToHCWMap.erase(it);
    hcw->events = events;
    hcw->barrier.wait();
    pthread_join(hcw->thr, nullptr);
  }

  lwp_write(afdt_fd, ret, errno, stat,
            time_us, events[0], events[1], events[2]);
}

void do_change_user(int afdt_fd) {
  std::string uname;
  lwp_read(afdt_fd, uname);
  if (!uname.length()) return;

  StructuredLogEntry* log = nullptr;
  int err = 0;
  SCOPE_EXIT {
    if (log) {
      log->setInt("errno", err);
      log->setStr("new_user", uname);
      StructuredLog::log("hhvm_lightprocess_error", *log);
      delete log;
    }
  };
  auto buf = PasswdBuffer{};
  struct passwd *pw;
  if (getpwnam_r(uname.c_str(), &buf.ent, buf.data.get(), buf.size, &pw)) {
    err = errno;
    log = new StructuredLogEntry();
    log->setStr("function", "getpwnam_r");
    if (Cfg::Server::LightProcessStrictUser) {
      throw std::runtime_error{"getpwnam_r(): " + folly::errnoStr(err)};
    }
    return;
  }
  if (!pw) {
    log = new StructuredLogEntry();
    log->setStr("function", "getpwnam_r");
    if (Cfg::Server::LightProcessStrictUser) {
      throw std::runtime_error{"getpwnam_r(): not found"};
    }
    return;
  }
  if (pw->pw_gid) {
    if (initgroups(pw->pw_name, pw->pw_gid)) {
      err = errno;
      log = new StructuredLogEntry();
      log->setStr("function", "initgroups");
    }
    if (setgid(pw->pw_gid)) {
      if (!log) {
        err = errno;
        log = new StructuredLogEntry();
        log->setStr("function", "setgid");
        if (Cfg::Server::LightProcessStrictUser) {
          throw std::runtime_error{"setgid():" + folly::errnoStr(err)};
        }
      }
    }
  }
  if (pw->pw_uid) {
    if (setuid(pw->pw_uid)) {
      if (!log) {
        err = errno;
        log = new StructuredLogEntry();
        log->setStr("function", "setuid");
        if (Cfg::Server::LightProcessStrictUser) {
          throw std::runtime_error{"setuid():" + folly::errnoStr(err)};
        }
      }
    }
  }
}


void do_clone_delegate(int afdt_fd) {
  auto const d = LightProcess::createDelegate();
  if (d < 0) {
    lwp_write(afdt_fd, "error");
    return;
  }
  lwp_write(afdt_fd, "success");
  send_fd(afdt_fd, d);
  close(d);
}

///////////////////////////////////////////////////////////////////////////////
// light-weight process

std::unique_ptr<LightProcess[]> g_procs;
int g_procsCount = 0;
bool s_handlerInited = false;
LightProcess::LostChildHandler s_lostChildHandler;
std::map<FILE*, pid_t> s_popenMap;

} // namespace

LightProcess::LightProcess() : m_shadowProcess(0), m_afdt_fd(-1) { }

LightProcess::~LightProcess() {
}

void LightProcess::SigChldHandler(int /*sig*/, siginfo_t* info, void* /*ctx*/) {
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
                              bool trackProcessTimes,
                              const std::vector<int> &inherited_fds) {
  s_trackProcessTimes = trackProcessTimes;

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
    Logger::Error("Unable to afdt_listen to %s: %d %s",
                  afdt_filename.c_str(),
                  errno, folly::errnoStr(errno).c_str());
    abort();
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
    struct sigaction sa = {};
    sa.sa_sigaction = &LightProcess::SigChldHandler;
    sa.sa_flags = SA_SIGINFO | SA_NOCLDSTOP;
    if (sigaction(SIGCHLD, &sa, nullptr) != 0) {
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
    mprotect_1g_pages(PROT_READ);
    if (s_trackProcessTimes) {
      HardwareCounter::RecordSubprocessTimes();
    }
    Logger::ResetPid();
    pid_t sid = setsid();
    if (sid < 0) {
      Logger::Warning("Unable to setsid");
      _Exit(HPHP_EXIT_FAILURE);
    }
    afdt_error_t err = AFDT_ERROR_T_INIT;
    auto afdt_fd = afdt_connect(afdt_filename.c_str(), &err);
    if (afdt_fd < 0) {
      Logger::Warning("Unable to afdt_connect, filename %s: %d %s",
                      afdt_filename.c_str(),
                      errno, folly::errnoStr(errno).c_str());
      _Exit(HPHP_EXIT_FAILURE);
    }

    // shadow process doesn't use g_procs
    for (int i = 0; i < id; i++) {
      g_procs[i].closeFiles();
    }
    g_procs.reset();
    g_procsCount = 0;
    close_fds(inherited_fds);
    ::close(afdt_lid);
    // Tell the OOM killer never to kill a light process.  Killing it will cause
    // the entire server to exit, and won't free much memory anyway.
    Process::OOMScoreAdj(-1000);
    runShadow(afdt_fd);
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

void LightProcess::runShadow(int afdt_fd) {
  std::string buf;

  auto error = false;

  pollfd pfd[1];
  pfd[0].fd = afdt_fd;
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
        lwp_read(afdt_fd, buf);
        if (buf == "exit") {
          Logger::Verbose("LightProcess exiting upon request");
          break;
        } else if (buf == "popen") {
          do_popen(afdt_fd);
        } else if (buf == "proc_open") {
          do_proc_open(afdt_fd);
        } else if (buf == "execve") {
          do_fork_and_execve(afdt_fd);
        } else if (buf == "waitpid") {
          do_waitpid(afdt_fd);
        } else if (buf == "change_user") {
          do_change_user(afdt_fd);
        } else if (buf == "clone_delegate") {
          do_clone_delegate(afdt_fd);
        } else if (buf[0]) {
          Logger::Info("LightProcess got invalid command: %.20s", buf.c_str());
        }
      }
    }
  } catch (const std::exception& e) {
    error = true;
    Logger::Error("LightProcess exiting due to exception: %s", e.what());
  } catch (...) {
    error = true;
    Logger::Error("LightProcess exiting due to unknown exception");
  }

  ::close(afdt_fd);
  _Exit(error ? HPHP_EXIT_FAILURE : 0);
}

namespace {

int GetId() {
  return hash_int64((long)pthread_self()) % g_procsCount;
}

NEVER_INLINE
void handleException(const char* call) {
  try {
    throw;
  } catch (const std::exception& e) {
    Logger::Error("LightProcess::%s failed due to exception: %s",
                  call, e.what());
  } catch (...) {
    Logger::Error("LightProcess::%s failed due to unknown exception",
                  call);
  }
}

template <class R, class F1>
R runLight(const char* call, F1 body, R failureResult) {
  try {
    auto proc = tl_proc ? tl_proc : &g_procs[GetId()];
    Lock lock(proc->mutex());

    return body(proc);
  } catch (...) {
    handleException(call);
  }
  return failureResult;
}

}

void LightProcess::Close() {
  std::unique_ptr<LightProcess[]> procs;
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
    try {
      lwp_write(m_afdt_fd, "exit");
    } catch (...) {
      handleException("closeShadow");
    }
    // removes the "zombie" process, so not to interfere with later waits
    while (true) {
      auto r = ::waitpid(m_shadowProcess, nullptr, 0);
      // retry on EINTR
      if (r != -1 || errno != EINTR) {
        break;
      }
    }

    m_shadowProcess = 0;
  }
  closeFiles();
}

void LightProcess::closeFiles() {
  if (m_afdt_fd >= 0) {
    ::close(m_afdt_fd);
    m_afdt_fd = -1;
  }
}

bool LightProcess::Available() {
  if (tl_proc) return true;
  return g_procsCount > 0;
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
    if (tl_proc) {
      Logger::Verbose("Light-weight fork failed in remote CLI mode.");
      return nullptr;
    }
    Logger::Verbose("Light-weight fork failed; use the heavy one instead.");
  }
  return HeavyPopenImpl(cmd, type, cwd);
}

FILE *LightProcess::HeavyPopenImpl(const char *cmd, const char *type,
                                   const char *cwd) {
  int fd = -1;
  pid_t pid;
  Lock lock(s_mutex);
  if (cwd && *cwd) {
    auto old_cwd = Process::GetCurrentDirectory();
    if (old_cwd != cwd) {
      if (chdir(cwd)) {
        Logger::Warning("Failed to chdir to %s.", cwd);
      }
      fd = popen_impl(cmd, type, &pid);
      if (chdir(old_cwd.c_str())) {
        // error occurred changing cwd back
      }
      if (fd < 0) return nullptr;
    }
  }
  if (fd < 0) {
    fd = popen_impl(cmd, type, &pid);
  }
  if (fd < 0) return nullptr;
  auto f = fdopen(fd, type);
  if (f) {
    s_popenMap[f] = pid;
  }
  return f;
}

FILE *LightProcess::LightPopenImpl(const char *cmd, const char *type,
                                   const char *cwd) {
  return runLight("popen", [&] (LightProcess* proc) -> FILE* {
      auto afdt_fd = proc->m_afdt_fd;
      lwp_write(afdt_fd, "popen", type, cmd, cwd ? cwd : "");

      std::string buf;
      lwp_read(afdt_fd, buf);
      if (buf == "error") {
        return nullptr;
      }

      pid_t pid;
      lwp_read(afdt_fd, pid);
      assert(pid);
      int fd = recv_fd(afdt_fd);
      if (fd < 0) {
        Logger::Error("Light process failed to send the file descriptor.");
        return nullptr;
      }
      FILE *f = fdopen(fd, type);
      if (f) {
        proc->m_popenMap[f] = pid;
      }
      return f;
    }, static_cast<FILE*>(nullptr));
}

int LightProcess::pclose(FILE *f) {
  pid_t pid;
  if (!Available()) {
    Lock lock(s_mutex);
    auto it = s_popenMap.find(f);
    if (it == s_popenMap.end()) return -1;
    pid = it->second;
    s_popenMap.erase(it);
  } else {
    auto proc = [] {
      if (tl_proc) return tl_proc;
      return &g_procs[GetId()];
    }();
    Lock lock(proc->m_procMutex);

    auto it = proc->m_popenMap.find(f);
    if (it == proc->m_popenMap.end()) return -1;
    pid = it->second;
    proc->m_popenMap.erase(it);
  }

  fclose(f);
  int status;
  if (LightProcess::waitpid(pid, &status, 0, 0) < 0) return -1;
  return status;
}

pid_t LightProcess::ForkAndExecve(const std::string& path,
                                  const std::vector<std::string>& argv,
                                  const std::vector<std::string>& envp,
                                  const std::string& cwd,
                                  const std::map<int, int>& fds,
                                  int options,
                                  pid_t pgid) {
  return runLight("execve", [&] (LightProcess* proc) -> pid_t {
      auto fin = proc->m_afdt_fd;
      auto fout = proc->m_afdt_fd;

      lwp_write(fout, "execve", path, cwd, argv, envp, options, pgid, (size_t) fds.size());
      for (const auto& [target_fd, current_fd]: fds) {
        lwp_write(fout, target_fd);
        send_fd(fout, current_fd);
      }

      std::string buf;
      lwp_read(fin, buf);
      if (buf == "success") {
        pid_t pid;
        lwp_read(fin, pid);
        return pid;
      }
      assert(buf == "error");

      int ret;
      int saved_errno;
      lwp_read(fin, ret, saved_errno);
      errno = saved_errno;
      return (pid_t) ret;
  }, static_cast<pid_t>(-1));
}

pid_t LightProcess::proc_open(const char *cmd, const std::vector<int> &created,
                              const std::vector<int> &desired,
                              const char *cwd,
                              const std::vector<std::string> &env) {
  always_assert(Available());
  always_assert(created.size() == desired.size());

  return runLight("proc_open", [&] (LightProcess* proc) -> pid_t {
      auto fout = proc->m_afdt_fd;
      lwp_write(fout, "proc_open", cmd, cwd ? cwd : "",
                env, desired);

      bool error_send = false;
      int save_errno = 0;
      for (auto cfd : created) {
        if (!send_fd(proc->m_afdt_fd, cfd)) {
          error_send = true;
          save_errno = errno;
          break;
        }
      }

      std::string buf;
      auto fin = proc->m_afdt_fd;
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
      pid_t pid = -1;
      lwp_read(fin, pid);
      always_assert(pid);
      return pid;
    }, static_cast<pid_t>(-1));
}

pid_t LightProcess::waitpid(pid_t pid, int *stat_loc, int options,
                            int timeout) {
  if (!Available()) {
    // light process is not really there
    rusage ru;
    const auto ret = wait4(pid, stat_loc, options, &ru);
    if (ret > 0 && s_trackProcessTimes) {
      s_extra_request_nanoseconds += ru2microseconds(ru) * 1000;
    }
    return ret;
  }

  return runLight("waitpid", [&] (LightProcess* proc) -> pid_t {
      lwp_write(proc->m_afdt_fd, "waitpid", pid, options, timeout);

      pid_t ret;
      int stat;
      int err;
      int64_t time_us, events[3];
      lwp_read(proc->m_afdt_fd, ret, err, stat,
               time_us, events[0], events[1], events[2]);

      *stat_loc = stat;
      if (ret < 0) {
        errno = err;
      } else if (s_trackProcessTimes) {
        s_extra_request_nanoseconds += time_us * 1000;
        HardwareCounter::IncInstructionCount(events[0]);
        HardwareCounter::IncLoadCount(events[1]);
        HardwareCounter::IncStoreCount(events[2]);
      }

      return ret;
    }, static_cast<pid_t>(-1));
}

void LightProcess::ChangeUser(int afdt, const std::string& username) {
  if (!username.empty()) lwp_write(afdt, "change_user", username);
}

void LightProcess::ChangeUser(const std::string& username) {
  if (username.empty()) return;
  for (int i = 0; i < g_procsCount; i++) {
    Lock lock(g_procs[i].m_procMutex);
    lwp_write(g_procs[i].m_afdt_fd, "change_user", username);
  }
}

void LightProcess::SetLostChildHandler(const LostChildHandler& handler) {
  s_lostChildHandler = handler;
}

std::unique_ptr<LightProcess> LightProcess::setThreadLocalAfdtOverride(
  std::unique_ptr<LightProcess> p
) {
  auto ret = std::unique_ptr<LightProcess>(tl_proc);
  tl_proc = p.release();
  return ret;
}

std::unique_ptr<LightProcess> LightProcess::setThreadLocalAfdtOverride(int fd) {
  auto ret = std::unique_ptr<LightProcess>(tl_proc);
  tl_proc = new LightProcess;
  tl_proc->m_afdt_fd = fd;
  return ret;
}

int LightProcess::cloneDelegate() {
  always_assert(tl_proc != nullptr);
  return runLight("clone_delegate", [&] (LightProcess* proc) {
    auto const afdt_fd = proc->m_afdt_fd;
    lwp_write(afdt_fd, "clone_delegate");

    std::string buf;
    lwp_read(afdt_fd, buf);
    if (buf == "error") return -1;
    return recv_fd(afdt_fd);
  }, -1);
}

void LightProcess::shutdownDelegate() {
  always_assert(tl_proc != nullptr);
  runLight("shutdown_delegate", [&] (LightProcess* proc) {
    auto const afdt_fd = proc->m_afdt_fd;
    lwp_write(afdt_fd, "exit");
    return 0;
  }, 0);
}

int LightProcess::createDelegate() {
  int pair[2];
  if (socketpair(AF_UNIX, SOCK_STREAM, 0, pair)) {
    Logger::Warning("Unable to create a unix socket pair: %s",
                    folly::errnoStr(errno).c_str());
    return -1;
  }

  pid_t child = fork();

  if (child < 0) {
    Logger::Warning("Unable to fork delegate process: %s",
                    folly::errnoStr(errno).c_str());
    close(pair[0]);
    close(pair[1]);
    return -1;
  }

  if (child == 0) {
    // child
    fclose(stdin);
    fclose(stdout);
    fclose(stderr);

    mprotect_1g_pages(PROT_READ);
    if (s_trackProcessTimes) {
      HardwareCounter::RecordSubprocessTimes();
    }
    Logger::ResetPid();
    pid_t sid = setsid();
    if (sid < 0) {
      Logger::Warning("Unable to setsid");
      _Exit(HPHP_EXIT_FAILURE);
    }

    close(pair[0]);
#ifdef __APPLE__
    {
      int newfd = dup2(pair[1], 0);
      always_assert(newfd == 0);
    }
    close(pair[1]);
    pair[1] = 0;
#endif
    runShadow(pair[1]);
  }

  always_assert(child > 0);
  close(pair[1]);
  return pair[0];
}

///////////////////////////////////////////////////////////////////////////////
}
