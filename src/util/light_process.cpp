/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010 Facebook, Inc. (http://www.facebook.com)          |
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

#include "light_process.h"
#include "process.h"
#include "util.h"

#include <afdt.h>
#include <string>
#include <vector>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <poll.h>
#include <pwd.h>

using namespace std;

namespace HPHP {

///////////////////////////////////////////////////////////////////////////////
// helper functions

static const unsigned int BUFFER_SIZE = 4096;

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
  int ret = afdt_send_fd_msg(afdt_fd, 0, 0, fd, &err);
  return ret >= 0;
}

static int recv_fd(int afdt_fd) {
  int fd;
  afdt_error_t err;
  uint8_t afdt_buf[AFDT_MSGLEN];
  uint32_t afdt_len;
  if (afdt_recv_fd_msg(afdt_fd, afdt_buf, &afdt_len, &fd, &err) < 0) {
    return -1;
  }
  return fd;
}

static char **build_envp(const vector<string> &env) {
  char **envp = NULL;
  int size = env.size();
  if (size) {
    envp = (char **)malloc((size + 1) * sizeof(char *));
    int j = 0;
    for (unsigned int i = 0; i < env.size(); i++, j++) {
      *(envp + j) = (char *)env[i].c_str();
    }
    *(envp + j) = NULL;
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

  if (!fgets(buf, BUFFER_SIZE, fin)) buf[0] = '\0';
  bool read_only = (buf[0] == 'r');

  read_buf(fin, buf);
  FILE *f = buf[0] ? ::popen(buf, read_only ? "r" : "w") : NULL;
  if (f == NULL) {
    // no need to send the errno back, as the main process will try ::popen
    fprintf(fout, "error\n");
    fflush(fout);
  } else {
    fprintf(fout, "success\n%lld\n", (int64)f);
    fflush(fout);
    int fd = fileno(f);
    send_fd(afdt_fd, fd);
  }
}

static void do_pclose(FILE *fin, FILE *fout) {
  char buf[BUFFER_SIZE];

  int64 fptr = 0;
  read_buf(fin, buf);
  sscanf(buf, "%lld", &fptr);
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
      fprintf(fout, "error\n%d\n", errno);
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
    if (strlen(cwd) > 0) {
      if (chdir(cwd)) { // non-zero for error
        fprintf(fout, "error\n%d\n", errno);
        fflush(fout);
        close_fds(pkeys);
        return;
      }
    }
    if (!env.empty()) {
      char **envp = build_envp(env);
      execle("/bin/sh", "sh", "-c", cmd, NULL, envp);
      free(envp);
    } else {
      execl("/bin/sh", "sh", "-c", cmd, NULL);
    }
    _exit(127);
  } else if (child > 0) {
    // successfully created the child process
    fprintf(fout, "%lld\n", (int64)child);
    fflush(fout);
  } else {
    // failed creating the child process
    fprintf(fout, "error\n%d\n", errno);
    fflush(fout);
  }

  close_fds(pkeys);
}

static void do_waitpid(FILE *fin, FILE *fout) {
  char buf[BUFFER_SIZE];
  read_buf(fin, buf);
  int64 p = -1;
  int options = 0;
  sscanf(buf, "%lld %d", &p, &options);
  pid_t pid = (pid_t)p;
  int stat;
  pid_t ret = ::waitpid(pid, &stat, options);
  fprintf(fout, "%lld %d\n", (int64)ret, stat);
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
    if (pw && pw->pw_uid) {
      setuid(pw->pw_uid);
    }
  }
}

///////////////////////////////////////////////////////////////////////////////
// light-weight process

static vector<LightProcess> g_procs;

LightProcess::LightProcess()
: m_shadowProcess(0), m_fin(NULL), m_fout(NULL), m_afdt_fd(-1) { }

LightProcess::~LightProcess() {
}

void LightProcess::Initialize(const std::string &prefix, int count) {
  if (prefix.empty() || count <= 0) {
    return;
  }

  if (Available()) {
    // already initialized
    return;
  }

  g_procs.resize(count);

  for (int i = 0; i < count; i++) {
    if (!g_procs[i].initShadow(prefix, i)) {
      for (int j = 0; j < i; j++) {
        g_procs[j].closeShadow();
      }
      g_procs.clear();
      break;
    }
  }
}

bool LightProcess::initShadow(const std::string &prefix, int id) {
  Lock lock(m_procMutex);

  ostringstream os;
  os << prefix << "." << getpid() << "." << id;
  m_afdtFilename = os.str();

  // remove the possible leftover
  remove(m_afdtFilename.c_str());

  afdt_error_t err;
  int lfd = afdt_listen(m_afdtFilename.c_str(), &err);
  if (lfd < 0) {
    Logger::Warning("Unable to afdt_listen");
    return false;
  }

  CPipe p1, p2;
  if (!p1.open() || !p2.open()) {
    Logger::Warning("Unable to create pipe: %d %s", errno,
                    Util::safe_strerror(errno).c_str());
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
    m_afdt_fd =
      afdt_connect(m_afdtFilename.c_str(), &err);
    if (m_afdt_fd < 0) {
      Logger::Warning("Unable to afdt_connect");
      exit(-1);
    }
    int fd1 = p1.detachOut();
    int fd2 = p2.detachIn();
    p1.close();
    p2.close();
    runShadow(fd1, fd2);
  } else if (child < 0) {
    // failed
    Logger::Warning("Unable to fork lightly: %d %s", errno,
                    Util::safe_strerror(errno).c_str());
    return false;
  } else {
    // parent
    m_fin = fdopen(p2.detachOut(), "r");
    m_fout = fdopen(p1.detachIn(), "w");
    m_shadowProcess = child;

    sockaddr addr;
    socklen_t addrlen;
    m_afdt_fd = accept(lfd, &addr, &addrlen);
    if (m_afdt_fd < 0) {
      Logger::Warning("Unable to establish afdt connection");
      closeShadow();
      return false;
    }
  }
  return true;
}

void LightProcess::Close() {
  for (unsigned int i = 0; i < g_procs.size(); i++) {
    g_procs[i].closeShadow();
  }
  g_procs.clear();
}

void LightProcess::closeShadow() {
  Lock lock(m_procMutex);
  if (m_shadowProcess) {
    fprintf(m_fout, "exit\n");
    fflush(m_fout);
    fclose(m_fin);
    fclose(m_fout);
    // removes the "zombie" process, so not to interfere with later waits
    ::waitpid(m_shadowProcess, NULL, 0);
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

bool LightProcess::Available() {
  return !g_procs.empty();
}

void LightProcess::runShadow(int fdin, int fdout) {
  FILE *fin = fdopen(fdin, "r");
  FILE *fout = fdopen(fdout, "w");

  char buf[BUFFER_SIZE];

  pollfd pfd[1];
  pfd[0].fd = fdin;
  pfd[0].events = POLLIN;
  while (true) {
    poll(pfd, 1, -1);
    if (pfd[0].revents & POLLHUP) {
      // no more command can come in
      break;
    }
    else if (pfd[0].revents & POLLIN) {
      if (!fgets(buf, BUFFER_SIZE, fin)) buf[0] = '\0';
      if (strncmp(buf, "exit", 4) == 0) {
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
      }
    }
  }

  fclose(fin);
  fclose(fout);
  ::close(m_afdt_fd);
  remove(m_afdtFilename.c_str());
  exit(0);
}

int LightProcess::GetId() {
  return (int)pthread_self() % g_procs.size();
}

FILE *LightProcess::popen(const char *cmd, const char *type) {
  if (!Available()) {
    // fallback to normal popen
    Logger::Verbose("Light-weight fork not available; "
                    "use the heavy one instead.");
    return ::popen(cmd, type);
  }

  int id = GetId();
  Lock lock(g_procs[id].m_procMutex);

  fprintf(g_procs[id].m_fout, "popen\n%s\n%s\n", type, cmd);
  fflush(g_procs[id].m_fout);

  char buf[BUFFER_SIZE];
  read_buf(g_procs[id].m_fin, buf);
  if (strncmp(buf, "error", 5) == 0) {
    Logger::Verbose("Light-weight fork failed; use the heavy one instead.");
    return ::popen(cmd, type);
  }

  int64 fptr = 0;
  read_buf(g_procs[id].m_fin, buf);
  sscanf(buf, "%lld", &fptr);
  if (!fptr) {
    Logger::Verbose("Light-weight fork failed; use the heavy one instead.");
    return ::popen(cmd, type);
  }

  int fd = recv_fd(g_procs[id].m_afdt_fd);
  if (fd < 0) {
    Logger::Verbose("Light-weight fork failed; use the heavy one instead.");
    return ::popen(cmd, type);
  }
  FILE *f = fdopen(fd, type);
  g_procs[id].m_popenMap[(int64)f] = fptr;

  return f;
}

int LightProcess::pclose(FILE *f) {
  if (!Available()) {
    return ::pclose(f);
  }

  int id = GetId();
  Lock lock(g_procs[id].m_procMutex);

  map<int64, int64>::iterator it = g_procs[id].m_popenMap.find((int64)f);
  if (it == g_procs[id].m_popenMap.end()) {
    // try to close it with normal pclose
    return ::pclose(f);
  }

  g_procs[id].m_popenMap.erase((int64)f);
  fclose(f);
  fprintf(g_procs[id].m_fout, "pclose\n%lld\n", it->second);
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
  assert(Available());
  assert(created.size() == desired.size());

  fprintf(g_procs[id].m_fout, "proc_open\n%s\n%s\n", cmd, cwd);
  fprintf(g_procs[id].m_fout, "%d\n", (int)env.size());
  for (unsigned int i = 0; i < env.size(); i++) {
    fprintf(g_procs[id].m_fout, "%s\n", env[i].c_str());
  }

  fprintf(g_procs[id].m_fout, "%d\n", (int)created.size());

  for (unsigned int i = 0; i < desired.size(); i++) {
    fprintf(g_procs[id].m_fout, "%d\n", desired[i]);
  }
  fflush(g_procs[id].m_fout);
  char buf[BUFFER_SIZE];
  for (unsigned int i = 0; i < created.size(); i++) {
    if (!send_fd(g_procs[id].m_afdt_fd, created[i])) break;
  }

  read_buf(g_procs[id].m_fin, buf);
  if (strncmp(buf, "error", 5) == 0) {
    read_buf(g_procs[id].m_fin, buf);
    sscanf(buf, "%d", &errno);
    return -1;
  }
  int64 pid = -1;
  sscanf(buf, "%lld", &pid);
  return (pid_t)pid;
}

pid_t LightProcess::waitpid(pid_t pid, int *stat_loc, int options) {
  if (!Available()) {
    // light process is not really there
    return ::waitpid(pid, stat_loc, options);
  }

  int id = GetId();
  Lock lock(g_procs[id].m_procMutex);

  fprintf(g_procs[id].m_fout, "waitpid\n%lld %d\n", (int64)pid, options);
  fflush(g_procs[id].m_fout);

  char buf[BUFFER_SIZE];
  read_buf(g_procs[id].m_fin, buf);
  if (!buf[0]) return -1;
  int64 ret;
  int stat;
  sscanf(buf, "%lld %d", &ret, &stat);
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
  for (unsigned i = 0; i < g_procs.size(); i++) {
    Lock lock(g_procs[i].m_procMutex);
    fprintf(g_procs[i].m_fout, "change_user\n%s\n", username.c_str());
    fflush(g_procs[i].m_fout);
  }
}

///////////////////////////////////////////////////////////////////////////////
}
