/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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
#include "hphp/util/process.h"
#include "util.h"
#include "hphp/util/logger.h"
#include "folly/String.h"

#include <afdt.h>
#include <string>
#include <vector>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <poll.h>
#include <pwd.h>
#include <signal.h>
#include <boost/scoped_array.hpp>

namespace HPHP {

///////////////////////////////////////////////////////////////////////////////
// helper functions

static const unsigned int BUFFER_SIZE = 4096;
Mutex LightProcess::s_mutex;

static void read_buf(FILE *fin, char *buf) {
  if (!fgets(buf, BUFFER_SIZE, fin)) {
    buf[0] = '\0';
    return;
  }
  // get rid of '\n'
  buf[strlen(buf) - 1] = '\0';
}

static bool send_fd(int afdt_fd, int fd) {
  afdt_error_t err;
  errno = 0;
  int ret = afdt_send_fd_msg(afdt_fd, 0, 0, fd, &err);
  if (ret < 0 && errno == 0) {
    // Set non-empty errno if afdt_send_fd_msg doesn't set one on error
    errno = EPROTO;
  }
  return ret >= 0;
}

static int recv_fd(int afdt_fd) {
  int fd;
  afdt_error_t err;
  uint8_t afdt_buf[AFDT_MSGLEN];
  uint32_t afdt_len;
  errno = 0;
  if (afdt_recv_fd_msg(afdt_fd, afdt_buf, &afdt_len, &fd, &err) < 0) {
    if (errno == 0) {
      // Set non-empty errno if afdt_send_fd_msg doesn't set one on error
      errno = EPROTO;
    }
    return -1;
  }
  return fd;
}

static char **build_envp(const vector<string> &env) {
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

static void close_fds(const vector<int> &fds) {
  for (unsigned int i = 0; i < fds.size(); i++) {
    ::close(fds[i]);
  }
}

///////////////////////////////////////////////////////////////////////////////
// shadow process tasks

static void do_popen(FILE *fin, FILE *fout, int afdt_fd) {
  char buf[BUFFER_SIZE];
  char cwd[BUFFER_SIZE];

  if (!fgets(buf, BUFFER_SIZE, fin)) buf[0] = '\0';
  bool read_only = (buf[0] == 'r');

  read_buf(fin, buf);

  string old_cwd = Process::GetCurrentDirectory();
  read_buf(fin, cwd);
  if (old_cwd != cwd) {
    if (chdir(cwd)) {
      // Ignore chdir failures, because the compiled version might not have the
      // directory any more.
      Logger::Warning("Light Process failed chdir to %s.", cwd);
    }
  }

  FILE *f = buf[0] ? ::popen(buf, read_only ? "r" : "w") : nullptr;

  if (old_cwd != cwd && chdir(old_cwd.c_str())) {
    // only here if we can't change the cwd back
  }

  if (f == nullptr) {
    Logger::Error("Light process failed popen: %d (%s).", errno,
                  folly::errnoStr(errno).c_str());
    fprintf(fout, "error\n");
    fflush(fout);
  } else {
    fprintf(fout, "success\n%" PRId64 "\n", (int64_t)f);
    fflush(fout);
    int fd = fileno(f);
    send_fd(afdt_fd, fd);
  }
}

static void do_pclose(FILE *fin, FILE *fout) {
  char buf[BUFFER_SIZE];

  int64_t fptr = 0;
  read_buf(fin, buf);
  sscanf(buf, "%" PRId64, &fptr);
  FILE *f = (FILE *)fptr;
  int ret = ::pclose(f);

  fprintf(fout, "%d\n", ret);
  if (ret < 0) {
    fprintf(fout, "%d\n", errno);
  }
  fflush(fout);
}

static void do_proc_open(FILE *fin, FILE *fout, int afdt_fd) {
  char cmd[BUFFER_SIZE];
  read_buf(fin, cmd);
  if (strlen(cmd) == 0) {
    fprintf(fout, "error\n%d\n", ENOENT);
    fflush(fout);
    return;
  }

  char cwd[BUFFER_SIZE];
  read_buf(fin, cwd);

  char buf[BUFFER_SIZE];
  int env_size = 0;
  vector<string> env;
  read_buf(fin, buf);
  sscanf(buf, "%d", &env_size);
  for (int i = 0; i < env_size; i++) {
    read_buf(fin, buf);
    env.push_back(buf);
  }

  int pipe_size = 0;
  read_buf(fin, buf);
  sscanf(buf, "%d", &pipe_size);
  vector<int> pvals;
  for (int i = 0; i < pipe_size; i++) {
    int fd_value;
    read_buf(fin, buf);
    sscanf(buf, "%d", &fd_value);
    pvals.push_back(fd_value);
  }

  vector<int> pkeys;
  for (int i = 0; i < pipe_size; i++) {
    int fd = recv_fd(afdt_fd);
    if (fd < 0) {
      fprintf(fout, "error\n%d\n", EPROTO);
      fflush(fout);
      close_fds(pkeys);
      return;
    }
    pkeys.push_back(fd);
  }

  // now ready to start the child process
  pid_t child = fork();
  if (child == 0) {
    for (int i = 0; i < pipe_size; i++) {
      dup2(pkeys[i], pvals[i]);
    }
    if (strlen(cwd) > 0 && chdir(cwd)) {
      // non-zero for error
      // chdir failed, the working directory remains unchanged
    }
    if (!env.empty()) {
      char **envp = build_envp(env);
      execle("/bin/sh", "sh", "-c", cmd, nullptr, envp);
      free(envp);
    } else {
      execl("/bin/sh", "sh", "-c", cmd, nullptr);
    }
    _exit(127);
  } else if (child > 0) {
    // successfully created the child process
    fprintf(fout, "%" PRId64 "\n", (int64_t)child);
    fflush(fout);
  } else {
    // failed creating the child process
    fprintf(fout, "error\n%d\n", errno);
    fflush(fout);
  }

  close_fds(pkeys);
}

static pid_t waited = 0;

static void kill_handler(int sig) {
  if (sig == SIGALRM && waited) {
    kill(waited, SIGKILL);
  }
}

static void do_waitpid(FILE *fin, FILE *fout) {
  char buf[BUFFER_SIZE];
  read_buf(fin, buf);
  int64_t p = -1;
  int options = 0;
  int timeout = 0;
  sscanf(buf, "%" PRId64 " %d %d", &p, &options, &timeout);
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
  fprintf(fout, "%" PRId64 " %d\n", (int64_t)ret, stat);
  if (ret < 0) {
    fprintf(fout, "%d\n", errno);
  }
  fflush(fout);
}

static void do_change_user(FILE *fin, FILE *fout) {
  char uname[BUFFER_SIZE];
  read_buf(fin, uname);
  if (strlen(uname) > 0) {
    struct passwd *pw = getpwnam(uname);
    if (pw) {
      if (pw->pw_gid) {
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

LightProcess::LightProcess()
: m_shadowProcess(0), m_fin(nullptr), m_fout(nullptr), m_afdt_fd(-1),
  m_afdt_lfd(-1) { }

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
                              const vector<int> &inherited_fds) {
  if (prefix.empty() || count <= 0) {
    return;
  }

  if (Available()) {
    // already initialized
    return;
  }

  g_procs.reset(new LightProcess[count]);
  g_procsCount = count;

  for (int i = 0; i < count; i++) {
    if (!g_procs[i].initShadow(prefix, i, inherited_fds)) {
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

bool LightProcess::initShadow(const std::string &prefix, int id,
                              const vector<int> &inherited_fds) {
  Lock lock(m_procMutex);

  std::ostringstream os;
  os << prefix << "." << getpid() << "." << id;
  m_afdtFilename = os.str();

  // remove the possible leftover
  remove(m_afdtFilename.c_str());

  afdt_error_t err;
  m_afdt_lfd = afdt_listen(m_afdtFilename.c_str(), &err);
  if (m_afdt_lfd < 0) {
    Logger::Warning("Unable to afdt_listen to %s: %d %s",
                    m_afdtFilename.c_str(),
                    errno, folly::errnoStr(errno).c_str());
    return false;
  }

  CPipe p1, p2;
  if (!p1.open() || !p2.open()) {
    Logger::Warning("Unable to create pipe: %d %s", errno,
                    folly::errnoStr(errno).c_str());
    return false;
  }

  pid_t child = fork();
  if (child == 0) {
    // child
    pid_t sid = setsid();
    if (sid < 0) {
      Logger::Warning("Unable to setsid");
      exit(-1);
    }
    m_afdt_fd = afdt_connect(m_afdtFilename.c_str(), &err);
    if (m_afdt_fd < 0) {
      Logger::Warning("Unable to afdt_connect, filename %s: %d %s",
                      m_afdtFilename.c_str(),
                      errno, folly::errnoStr(errno).c_str());
      exit(-1);
    }
    int fd1 = p1.detachOut();
    int fd2 = p2.detachIn();
    p1.close();
    p2.close();

    // don't hold on to previous light processes' pipes, inherited
    // fds, or the afdt listening socket
    for (int i = 0; i < id; i++) {
      g_procs[i].closeFiles();
    }
    close_fds(inherited_fds);
    ::close(m_afdt_lfd);

    runShadow(fd1, fd2);
  } else if (child < 0) {
    // failed
    Logger::Warning("Unable to fork lightly: %d %s", errno,
                    folly::errnoStr(errno).c_str());
    return false;
  } else {
    // parent
    m_fin = fdopen(p2.detachOut(), "r");
    m_fout = fdopen(p1.detachIn(), "w");
    m_shadowProcess = child;

    sockaddr addr;
    socklen_t addrlen = sizeof(addr);
    m_afdt_fd = accept(m_afdt_lfd, &addr, &addrlen);
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
    fprintf(m_fout, "exit\n");
    fflush(m_fout);
    fclose(m_fin);
    fclose(m_fout);
    // removes the "zombie" process, so not to interfere with later waits
    ::waitpid(m_shadowProcess, nullptr, 0);
  }
  if (!m_afdtFilename.empty()) {
    remove(m_afdtFilename.c_str());
  }
  if (m_afdt_fd >= 0) {
    ::close(m_afdt_fd);
    m_afdt_fd = -1;
  }
  m_shadowProcess = 0;
}

void LightProcess::closeFiles() {
  fclose(m_fin);
  fclose(m_fout);
  ::close(m_afdt_fd);
  ::close(m_afdt_lfd);
}

bool LightProcess::Available() {
  return g_procsCount > 0;
}

void LightProcess::runShadow(int fdin, int fdout) {
  FILE *fin = fdopen(fdin, "r");
  FILE *fout = fdopen(fdout, "w");

  char buf[BUFFER_SIZE];

  pollfd pfd[1];
  pfd[0].fd = fdin;
  pfd[0].events = POLLIN;
  while (true) {
    int ret = poll(pfd, 1, -1);
    if (ret < 0 && errno == EINTR) {
      continue;
    }
    if (pfd[0].revents & POLLIN) {
      if (!fgets(buf, BUFFER_SIZE, fin)) buf[0] = '\0';
      if (strncmp(buf, "exit", 4) == 0) {
        Logger::Info("LightProcess exiting upon request");
        break;
      } else if (strncmp(buf, "popen", 5) == 0) {
        do_popen(fin, fout, m_afdt_fd);
      } else if (strncmp(buf, "pclose", 6) == 0) {
        do_pclose(fin, fout);
      } else if (strncmp(buf, "proc_open", 9) == 0) {
        do_proc_open(fin, fout, m_afdt_fd);
      } else if (strncmp(buf, "waitpid", 7) == 0) {
        do_waitpid(fin, fout);
      } else if (strncmp(buf, "change_user", 11) == 0) {
        do_change_user(fin, fout);
      } else if (buf[0]) {
        Logger::Info("LightProcess got invalid command: %.20s", buf);
      }
    } else if (pfd[0].revents & POLLHUP) {
      // no more command can come in
      Logger::Error("Lost parent, LightProcess exiting");
      break;
    }
  }

  fclose(fin);
  fclose(fout);
  ::close(m_afdt_fd);
  remove(m_afdtFilename.c_str());
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
    string old_cwd = Process::GetCurrentDirectory();
    if (old_cwd != cwd) {
      Lock lock(s_mutex);
      if (chdir(cwd)) {
        Logger::Warning("Failed to chdir to %s.", cwd);
      }
      FILE *f = ::popen(cmd, type);
      if (chdir(old_cwd.c_str())) {
        // error occured changing cwd back
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

  fprintf(g_procs[id].m_fout, "popen\n%s\n%s\n%s\n", type, cmd, cwd);
  fflush(g_procs[id].m_fout);

  char buf[BUFFER_SIZE];
  read_buf(g_procs[id].m_fin, buf);
  if (strncmp(buf, "error", 5) == 0) {
    return nullptr;
  }

  int64_t fptr = 0;
  read_buf(g_procs[id].m_fin, buf);
  sscanf(buf, "%" PRId64, &fptr);
  if (!fptr) {
    Logger::Error("Light process failed to return the file pointer.");
    return nullptr;
  }

  int fd = recv_fd(g_procs[id].m_afdt_fd);
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
  fprintf(g_procs[id].m_fout, "pclose\n%" PRId64 "\n", f2);
  fflush(g_procs[id].m_fout);

  char buf[BUFFER_SIZE];
  read_buf(g_procs[id].m_fin, buf);
  int ret = -1;
  sscanf(buf, "%d", &ret);
  if (ret < 0) {
    read_buf(g_procs[id].m_fin, buf);
    sscanf(buf, "%d", &errno);
  }
  return ret;
}

pid_t LightProcess::proc_open(const char *cmd, const vector<int> &created,
                              const vector<int> &desired,
                              const char *cwd, const vector<string> &env) {
  int id = GetId();
  Lock lock(g_procs[id].m_procMutex);
  always_assert(Available());
  always_assert(created.size() == desired.size());

  if (fprintf(g_procs[id].m_fout, "proc_open\n%s\n%s\n", cmd, cwd) <= 0) {
    Logger::Error("Failed to send command proc_open");
    return -1;
  }
  fprintf(g_procs[id].m_fout, "%d\n", (int)env.size());
  for (unsigned int i = 0; i < env.size(); i++) {
    fprintf(g_procs[id].m_fout, "%s\n", env[i].c_str());
  }

  fprintf(g_procs[id].m_fout, "%d\n", (int)created.size());

  for (unsigned int i = 0; i < desired.size(); i++) {
    fprintf(g_procs[id].m_fout, "%d\n", desired[i]);
  }
  fflush(g_procs[id].m_fout);
  bool error_send = false;
  int save_errno = 0;
  for (unsigned int i = 0; i < created.size(); i++) {
    if (!send_fd(g_procs[id].m_afdt_fd, created[i])) {
      error_send = true;
      save_errno = errno;
      break;
    }
  }

  char buf[BUFFER_SIZE];
  read_buf(g_procs[id].m_fin, buf);
  if (strncmp(buf, "error", 5) == 0) {
    read_buf(g_procs[id].m_fin, buf);
    sscanf(buf, "%d", &errno);
    if (error_send) {
      // On this error, the receiver side returns dummy errno,
      // use the sender side errno here.
      errno = save_errno;
    }
    return -1;
  }
  int64_t pid = -1;
  sscanf(buf, "%" PRId64, &pid);
  assert(pid);
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

  fprintf(g_procs[id].m_fout, "waitpid\n%" PRId64 " %d %d\n", (int64_t)pid, options,
          timeout);
  fflush(g_procs[id].m_fout);

  char buf[BUFFER_SIZE];
  read_buf(g_procs[id].m_fin, buf);
  if (!buf[0]) return -1;
  int64_t ret;
  int stat;
  sscanf(buf, "%" PRId64 " %d", &ret, &stat);
  *stat_loc = stat;
  if (ret < 0) {
    read_buf(g_procs[id].m_fin, buf);
    sscanf(buf, "%d", &errno);
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

void LightProcess::ChangeUser(const string &username) {
  if (username.empty()) return;
  for (int i = 0; i < g_procsCount; i++) {
    Lock lock(g_procs[i].m_procMutex);
    fprintf(g_procs[i].m_fout, "change_user\n%s\n", username.c_str());
    fflush(g_procs[i].m_fout);
  }
}

void LightProcess::SetLostChildHandler(const LostChildHandler& handler) {
  s_lostChildHandler = handler;
}

///////////////////////////////////////////////////////////////////////////////
}
