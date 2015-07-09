/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
   | Copyright (c) 1997-2010 The PHP Group                                |
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
#include "hphp/runtime/ext/process/ext_process.h"

#include <cstdlib>
#include <vector>
#include <string>
#include <iostream>

#include <sys/time.h>
#include <sys/resource.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>

#include <folly/String.h>

#include "hphp/util/light-process.h"
#include "hphp/util/lock.h"
#include "hphp/util/logger.h"

#include "hphp/runtime/base/array-init.h"
#include "hphp/runtime/base/builtin-functions.h"
#include "hphp/runtime/base/plain-file.h"
#include "hphp/runtime/base/request-local.h"
#include "hphp/runtime/base/string-buffer.h"
#include "hphp/runtime/base/surprise-flags.h"
#include "hphp/runtime/base/thread-info.h"
#include "hphp/runtime/base/thread-init-fini.h"
#include "hphp/runtime/base/string-util.h"
#include "hphp/runtime/base/zend-string.h"
#include "hphp/runtime/ext/std/ext_std_file.h"
#include "hphp/runtime/ext/std/ext_std_function.h"
#include "hphp/runtime/ext/string/ext_string.h"
#include "hphp/runtime/vm/repo.h"
#include "hphp/runtime/base/request-event-handler.h"

#if !defined(_NSIG) && defined(NSIG)
# define _NSIG NSIG
#endif

extern char **environ;

namespace HPHP {

///////////////////////////////////////////////////////////////////////////////

// build environment pair list
static char **build_envp(const Array& envs, std::vector<String> &senvs) {
  char **envp = NULL;
  int size = envs.size();
  if (size) {
    envp = (char **)malloc((size + 1) * sizeof(char *));
    int i = 0;
    for (ArrayIter iter(envs); iter; ++iter, ++i) {
      StringBuffer nvpair;
      nvpair.append(iter.first().toString());
      nvpair.append('=');
      nvpair.append(iter.second().toString());

      String env = nvpair.detach();
      senvs.push_back(env);
      *(envp + i) = (char *)env.data();
    }
    *(envp + i) = NULL;
  }
  return envp;
}

// check whitelist
static bool check_cmd(const char *cmd) {
  const char *cmd_tmp = cmd;
  while (true) {
    bool allow = false;
    while (isblank(*cmd_tmp)) cmd_tmp++;
    const char *space = strchr(cmd_tmp, ' ');
    unsigned int cmd_len = strlen(cmd_tmp);
    if (space) {
      cmd_len = space - cmd_tmp;
    }
    for (unsigned int i = 0; i < RuntimeOption::AllowedExecCmds.size(); i++) {
      std::string &allowedCmd = RuntimeOption::AllowedExecCmds[i];
      if (allowedCmd.size() != cmd_len) {
        continue;
      }
      if (strncmp(allowedCmd.c_str(), cmd_tmp, allowedCmd.size()) == 0) {
        allow = true;
        break;
      }
    }
    if (!allow) {
      auto const file = g_context->getContainingFileName();
      int line = g_context->getLine();
      Logger::Warning("Command %s is not in the whitelist, called at %s:%d",
                      cmd_tmp, file->data(), line);
      if (!RuntimeOption::WhitelistExecWarningOnly) {
        return false;
      }
    }
    const char *bar = strchr(cmd_tmp, '|');
    if (!bar) { // no pipe, we are done
      return true;
    }
    cmd_tmp = bar + 1;
  }
  return false;
}

///////////////////////////////////////////////////////////////////////////////
// pcntl

static class ProcessExtension final : public Extension {
public:
  ProcessExtension() : Extension("pcntl", NO_EXTENSION_VERSION_YET) {}
  void moduleInit() override {
    HHVM_FE(pcntl_alarm);
    HHVM_FE(pcntl_exec);
    HHVM_FE(pcntl_fork);
    HHVM_FE(pcntl_getpriority);
    HHVM_FE(pcntl_setpriority);
    HHVM_FE(pcntl_signal);
    HHVM_FE(pcntl_sigprocmask);
    HHVM_FE(pcntl_wait);
    HHVM_FE(pcntl_waitpid);
    HHVM_FE(pcntl_wexitstatus);
    HHVM_FE(pcntl_signal_dispatch);
    HHVM_FE(pcntl_wifexited);
    HHVM_FE(pcntl_wifsignaled);
    HHVM_FE(pcntl_wifstopped);
    HHVM_FE(pcntl_wstopsig);
    HHVM_FE(pcntl_wtermsig);
    HHVM_FE(shell_exec);
    HHVM_FE(exec);
    HHVM_FE(passthru);
    HHVM_FE(system);
    HHVM_FE(proc_open);
    HHVM_FE(proc_terminate);
    HHVM_FE(proc_close);
    HHVM_FE(proc_get_status);
    HHVM_FE(proc_nice);
    HHVM_FE(escapeshellarg);
    HHVM_FE(escapeshellcmd);

    loadSystemlib("process");
  }
} s_process_extension;

int64_t HHVM_FUNCTION(pcntl_alarm,
                      int seconds) {
  return alarm(seconds);
}

void HHVM_FUNCTION(pcntl_exec,
                   const String& path,
                   const Array& args /* = null_array */,
                   const Array& envs /* = null_array */) {
  if (RuntimeOption::WhitelistExec && !check_cmd(path.data())) {
    return;
  }
  if (Repo::prefork()) {
    raise_error("execing is disallowed in multi-threaded mode");
    return;
  }

  // build argumnent list
  std::vector<String> sargs; // holding those char *
  int size = args.size();
  char **argv = (char **)malloc((size + 2) * sizeof(char *));
  *argv = (char *)path.data();
  int i = 1;
  if (size) {
    sargs.reserve(size);
    for (ArrayIter iter(args); iter; ++iter, ++i) {
      String arg = iter.second().toString();
      sargs.push_back(arg);
      *(argv + i) = (char *)arg.data();
    }
  }
  *(argv + i) = NULL;

  // build environment pair list
  std::vector<String> senvs; // holding those char *
  char **envp = build_envp(envs, senvs);
  if (execve(path.c_str(), argv, envp) == -1) {
    raise_warning("Error has occurred: (errno %d) %s",
                    errno, folly::errnoStr(errno).c_str());
  }

  free(envp);
  free(argv);
}

int64_t HHVM_FUNCTION(pcntl_fork) {
  if (RuntimeOption::ServerExecutionMode()) {
    raise_error("forking is disallowed in server mode");
    return -1;
  }
  if (Repo::prefork()) {
    raise_error("forking is disallowed in multi-threaded mode");
    return -1;
  }

  std::cout.flush();
  std::cerr.flush();
  pid_t pid = fork();
  Repo::postfork(pid);
  return pid;
}

Variant HHVM_FUNCTION(pcntl_getpriority,
                      int pid /* = 0 */,
                      int process_identifier /* = 0 */) {
  if (pid == 0) {
    pid = getpid();
  }
  if (process_identifier == 0) {
    process_identifier = PRIO_PROCESS;
  }

  // this isn't thread-safe, but probably not a huge deal
  errno = 0;
  int pri = getpriority(process_identifier, pid);
  if (errno) {
    switch (errno) {
    case ESRCH:
      raise_warning("Error %d: No process was located using the given "
                      "parameters", errno);
      break;
    case EINVAL:
      raise_warning("Error %d: Invalid identifier flag", errno);
      break;
    default:
      raise_warning("Unknown error %d has occurred", errno);
      break;
    }
    return false;
  }
  return pri;
}

bool HHVM_FUNCTION(pcntl_setpriority,
                   int priority,
                   int pid /* = 0 */,
                   int process_identifier /* = 0 */) {
  if (pid == 0) {
    pid = getpid();
  }
  if (process_identifier == 0) {
    process_identifier = PRIO_PROCESS;
  }

  if (setpriority(process_identifier, pid, priority)) {
    switch (errno) {
    case ESRCH:
      raise_warning("Error %d: No process was located using the given "
                      "parameters", errno);
      break;
    case EINVAL:
      raise_warning("Error %d: Invalid identifier flag", errno);
      break;
    case EPERM:
      raise_warning("Error %d: A process was located, but neither its "
                      "effective nor real user ID matched the effective "
                      "user ID of the caller", errno);
      break;
    case EACCES:
      raise_warning("Error %d: Only a super user may attempt to increase "
                      "the process priority", errno);
      break;
    default:
      raise_warning("Unknown error %d has occurred", errno);
      break;
    }
    return false;
  }
  return true;
}

/* php_signal using sigaction is derived from Advanced Programing
 * in the Unix Environment by W. Richard Stevens p 298. */
typedef void Sigfunc(int);
static Sigfunc *php_signal(int signo, Sigfunc *func, bool restart) {
  struct sigaction act,oact;
  act.sa_handler = func;
  sigemptyset(&act.sa_mask);
  act.sa_flags = 0;
  if (signo == SIGALRM || (!restart)) {
#ifdef SA_INTERRUPT
    act.sa_flags |= SA_INTERRUPT; /* SunOS */
#endif
  } else {
#ifdef SA_RESTART
    act.sa_flags |= SA_RESTART; /* SVR4, 4.3+BSD */
#endif
  }
  if (sigaction(signo, &act, &oact) < 0)
    return SIG_ERR;

  return oact.sa_handler;
}

/* Our custom signal handler that calls the appropriate php_function */
struct SignalHandlers final : RequestEventHandler {
  SignalHandlers() {
    memset(signaled, 0, sizeof(signaled));
    pthread_sigmask(SIG_SETMASK, NULL, &oldSet);
  }
  void requestInit() override {
    handlers.reset();
    // restore the old signal mask, thus unblock those that should be
    pthread_sigmask(SIG_SETMASK, &oldSet, NULL);
    inited.store(true);
  }
  void requestShutdown() override {
    // block all signals
    sigset_t set;
    sigfillset(&set);
    pthread_sigmask(SIG_BLOCK, &set, NULL);

    handlers.reset();
    inited.store(false);
  }
  Array handlers;
  int signaled[_NSIG];
  sigset_t oldSet;
  std::atomic<bool> inited;
};
IMPLEMENT_STATIC_REQUEST_LOCAL(SignalHandlers, s_signal_handlers);

static bool signalHandlersInited() {
  return s_signal_handlers.getInited() && s_signal_handlers->inited.load();
}

static void pcntl_signal_handler(int signo) {
  if (signo > 0 && signo < _NSIG && signalHandlersInited()) {
    s_signal_handlers->signaled[signo] = 1;
    setSurpriseFlag(SignaledFlag);
  }
}

class SignalHandlersStaticInitializer {
public:
  SignalHandlersStaticInitializer() {
    signal(SIGALRM, pcntl_signal_handler);
    signal(SIGUSR1, pcntl_signal_handler);
    signal(SIGUSR2, pcntl_signal_handler);
  }
};
static SignalHandlersStaticInitializer s_signal_handlers_initializer;

bool HHVM_FUNCTION(pcntl_signal_dispatch) {
  if (!signalHandlersInited()) return true;
  int *signaled = s_signal_handlers->signaled;
  for (int i = 0; i < _NSIG; i++) {
    if (signaled[i]) {
      signaled[i] = 0;
      if (s_signal_handlers->handlers.exists(i)) {
        vm_call_user_func(s_signal_handlers->handlers[i],
                               make_packed_array(i));
      }
    }
  }
  return true;
}

bool HHVM_FUNCTION(pcntl_signal,
                   int signo,
                   const Variant& handler,
                   bool restart_syscalls /* = true */) {
  /* Special long value case for SIG_DFL and SIG_IGN */
  if (handler.isInteger()) {
    int64_t handle = handler.toInt64();
    if (handle != (long)SIG_DFL && handle != (long)SIG_IGN) {
      raise_warning("Invalid value for handle argument specified");
    }
    if (php_signal(signo, (Sigfunc *)handle, restart_syscalls) == SIG_ERR) {
      raise_warning("Error assigning signal");
      return false;
    }
    return true;
  }

  if (!is_callable(handler)) {
    raise_warning("%s is not a callable function name error",
                    handler.toString().data());
    return false;
  }

  s_signal_handlers->handlers.set(signo, handler);

  if (php_signal(signo, pcntl_signal_handler, restart_syscalls) == SIG_ERR) {
    raise_warning("Error assigning signal");
    return false;
  }
  return true;
}

bool HHVM_FUNCTION(pcntl_sigprocmask,
                   int how,
                   const Array& set,
                   VRefParam oldset) {
  if (how != SIG_BLOCK && how != SIG_UNBLOCK && how != SIG_SETMASK) {
    goto invalid_argument;
  }

  { // Variable scope so that goto doesn't cross definitions
    sigset_t cset;
    sigset_t coldset;

    sigemptyset(&cset);
    for (ArrayIter iter(set); iter; ++iter) {
      auto value = iter.second().toInt64();
      if (sigaddset(&cset, value) == -1) {
        goto invalid_argument;
      }
    }

    int result = pthread_sigmask(how, &cset, &coldset);
    if (result != 0) {
      return false;
    }
    Array aoldset;
    for (int signum = 1; true; ++signum) {
      result = sigismember(&coldset, signum);
      if (result == 1) {
        aoldset.append(signum);
      } else if (result == -1) {
        // invalid signal number
        break;
      }
    }
    oldset = aoldset;

    return true;
  }

invalid_argument:
  raise_warning("pcntl_sigprocmask(): Invalid argument");
  return false;
}

int64_t HHVM_FUNCTION(pcntl_wait,
                      VRefParam status,
                      int options /* = 0 */) {
  int nstatus = status;
  auto const child_id = LightProcess::pcntl_waitpid(-1, &nstatus, options);
/*  if (options) {
    child_id = wait3(&nstatus, options, NULL);
  } else {
    child_id = wait(&nstatus);
  }*/
  status = nstatus;
  return child_id;
}

int64_t HHVM_FUNCTION(pcntl_waitpid,
                      int pid,
                      VRefParam status,
                      int options /* = 0 */) {
  int nstatus = status;
  auto const child_id = LightProcess::pcntl_waitpid(
    (pid_t)pid,
    &nstatus,
    options
  );
  status = nstatus;
  return child_id;
}

int64_t HHVM_FUNCTION(pcntl_wexitstatus,
                      int status) {
  return WEXITSTATUS(status);
}

bool HHVM_FUNCTION(pcntl_wifexited,
                   int status) {
  return WIFEXITED(status);
}

bool HHVM_FUNCTION(pcntl_wifsignaled,
                   int status) {
  return WIFSIGNALED(status);
}

bool HHVM_FUNCTION(pcntl_wifstopped,
                   int status) {
  return WIFSTOPPED(status);
}

int64_t HHVM_FUNCTION(pcntl_wstopsig,
                      int status) {
  return WSTOPSIG(status);
}

int64_t HHVM_FUNCTION(pcntl_wtermsig,
                      int status) {
  return WTERMSIG(status);
}

///////////////////////////////////////////////////////////////////////////////
// popen

#define EXEC_INPUT_BUF 4096

namespace {

class ShellExecContext final {
public:
  ShellExecContext() {
    m_sig_handler = signal(SIGCHLD, SIG_DFL);
  }

  ~ShellExecContext() {
    if (m_proc) {
      LightProcess::pclose(m_proc);
    }
    if (m_sig_handler) {
      signal(SIGCHLD, m_sig_handler);
    }
  }

  FILE *exec(const String& cmd_string) {
    assert(m_proc == nullptr);
    const auto cmd = cmd_string.c_str();
    if (RuntimeOption::WhitelistExec && !check_cmd(cmd)) {
      return nullptr;
    }
    if (strlen(cmd) != cmd_string.size()) {
      raise_warning("NULL byte detected. Possible attack");
      return nullptr;
    }
    m_proc = LightProcess::popen(cmd, "r", g_context->getCwd().data());
    if (m_proc == nullptr) {
      raise_warning("Unable to execute '%s'", cmd);
    }
    return m_proc;
  }

  int exit() {
    int status = LightProcess::pclose(m_proc);
    m_proc = nullptr;
    return status;
  }

private:
  void (*m_sig_handler)(int);
  FILE *m_proc{nullptr};
};

}

Variant HHVM_FUNCTION(shell_exec,
                      const String& cmd) {
  ShellExecContext ctx;
  FILE *fp = ctx.exec(cmd);
  if (!fp) return init_null();
  StringBuffer sbuf;
  sbuf.read(fp);
  auto ret = sbuf.detach();
  if (ret.empty() && !RuntimeOption::EnableHipHopSyntax) {
    // Match php5
    return init_null();
  }
  return ret;
}

String HHVM_FUNCTION(exec,
                     const String& command,
                     VRefParam output /* = null */,
                     VRefParam return_var /* = null */) {
  ShellExecContext ctx;
  FILE *fp = ctx.exec(command);
  if (!fp) return empty_string();
  StringBuffer sbuf;
  sbuf.read(fp);

  Array lines = StringUtil::Explode(sbuf.detach(), "\n").toArray();
  int ret = ctx.exit();
  if (WIFEXITED(ret)) ret = WEXITSTATUS(ret);
  return_var = ret;
  int count = lines.size();
  if (count > 0 && lines[count - 1].toString().empty()) {
    count--; // remove explode()'s last empty line
  }

  PackedArrayInit pai(count);
  for (int i = 0; i < count; i++) {
    pai.append(lines[i]);
  }
  output.wrapped() = pai.toArray();

  if (!count || lines.empty()) {
    return String();
  }

  return HHVM_FN(rtrim)(lines[count - 1].toString());
}

void HHVM_FUNCTION(passthru,
                   const String& command,
                   VRefParam return_var /* = null */) {
  ShellExecContext ctx;
  FILE *fp = ctx.exec(command);
  if (!fp) return;

  char buffer[1024];
  while (true) {
    int len = read(fileno(fp), buffer, sizeof(buffer) - 1);
    if (len == -1 && errno == EINTR) continue;
    if (len <= 0) break; // break on error or EOF
    buffer[len] = '\0';
    g_context->write(String(buffer, len, CopyString));
  }
  int ret = ctx.exit();
  if (WIFEXITED(ret)) ret = WEXITSTATUS(ret);
  return_var = ret;
}

String HHVM_FUNCTION(system,
                     const String& command,
                     VRefParam return_var /* = null */) {
  ShellExecContext ctx;
  FILE *fp = ctx.exec(command);
  if (!fp) return empty_string();
  StringBuffer sbuf;
  if (fp) {
    sbuf.read(fp);
  }

  Array lines = StringUtil::Explode(sbuf.detach(), "\n").toArray();
  int ret = ctx.exit();
  if (WIFEXITED(ret)) ret = WEXITSTATUS(ret);
  return_var = ret;
  int count = lines.size();
  if (count > 0 && lines[count - 1].toString().empty()) {
    count--; // remove explode()'s last empty line
  }

  auto& ectx = *g_context;
  for (int i = 0; i < count; i++) {
    ectx.write(lines[i].toString());
    ectx.write("\n");
  }
  if (!count || lines.empty()) {
    return String();
  }

  return HHVM_FN(rtrim)(lines[count - 1].toString());
}

///////////////////////////////////////////////////////////////////////////////
// proc

class ChildProcess : public SweepableResourceData {
public:
  DECLARE_RESOURCE_ALLOCATION(ChildProcess)

  pid_t child;
  Array pipes;
  String command;
  Variant env;

  CLASSNAME_IS("process");
  // overriding ResourceData
  const String& o_getClassNameHook() const override { return classnameof(); }

  int close() {
    // Although the PHP doc about proc_close() says that the pipes need to be
    // explicitly pclose()'ed, it seems that Zend is implicitly closing the
    // pipes when proc_close() is called.
    for (ArrayIter iter(pipes); iter; ++iter) {
      cast<PlainFile>(iter.second())->close();
    }
    pipes.clear();

    pid_t wait_pid;
    int wstatus;
    do {
      wait_pid = LightProcess::waitpid(child, &wstatus, 0,
                                       RuntimeOption::RequestTimeoutSeconds);
    } while (wait_pid == -1 && errno == EINTR);

    if (wait_pid == -1) {
      return -1;
    }

    if (WIFEXITED(wstatus)) {
      wstatus = WEXITSTATUS(wstatus);
    }
    return wstatus;
  }
};

void ChildProcess::sweep() {
  // do nothing here, as everything will be collected by MemoryManager
}

#define DESC_PIPE       1
#define DESC_FILE       2
#define DESC_PARENT_MODE_WRITE  8

const StaticString s_w("w");

class DescriptorItem {
public:
  DescriptorItem() :
    index(-1), parentend(-1), childend(-1), mode(-1), mode_flags(-1) {
  }

  ~DescriptorItem() {
  }

  void cleanup() {
    if (childend  >= 0) close(childend);
    if (parentend >= 0) close(parentend);
  }

  int index;               // desired fd number in child process
  int parentend, childend; // fds for pipes in parent/child
  int mode;                // mode for proc_open code
  int mode_flags;          // mode flags for opening fds

  static Mutex s_mutex;    // Prevents another thread from forking at the
                           // same time, before FD_CLOEXEC is set on the fds.
                           // NOTE: no need to lock with light processes.

  bool readFile(const req::ptr<File>& file) {
    mode = DESC_FILE;
    childend = dup(file->fd());
    if (childend < 0) {
      raise_warning("unable to dup File-Handle for descriptor %d - %s",
                      index, folly::errnoStr(errno).c_str());
      return false;
    }
    return true;
  }

  bool readPipe(const String& zmode) {
    mode = DESC_PIPE;
    int newpipe[2];
    if (0 != pipe(newpipe)) {
      raise_warning("unable to create pipe %s",
                      folly::errnoStr(errno).c_str());
      return false;
    }

    if (zmode != s_w) {
      parentend = newpipe[1];
      childend = newpipe[0];
      mode |= DESC_PARENT_MODE_WRITE;
    } else {
      parentend = newpipe[0];
      childend = newpipe[1];
    }
    mode_flags = mode & DESC_PARENT_MODE_WRITE ? O_WRONLY : O_RDONLY;
    return true;
  }

  bool openFile(const String& zfile, const String& zmode) {
    mode = DESC_FILE;
      /* try a wrapper */
    Variant vfile = HHVM_FN(fopen)(zfile.c_str(), zmode.c_str());
    if (!vfile.isResource()) {
      raise_warning("Unable to open specified file: %s (mode %s)",
                      zfile.data(), zmode.data());
      return false;
    } else {
      auto file = cast<File>(vfile);
      file->flush();
      childend = dup(file->fd());
      if (childend < 0) {
        raise_warning("unable to dup File-Handle for descriptor %d - %s",
                        index, folly::errnoStr(errno).c_str());
        return false;
      }
      return true;
    }
  }

  void dupChild() {
    if ((mode & ~DESC_PARENT_MODE_WRITE) == DESC_PIPE) {
      close(parentend); parentend = -1;
    }
    if (dup2(childend, index) < 0) {
      perror("dup2");
    }
    if (childend != index) {
      close(childend); childend = -1;
    }
  }

  /* clean up all the child ends and then open streams on the parent
   * ends, where appropriate */
  Resource dupParent() {
    close(childend); childend = -1;

    if ((mode & ~DESC_PARENT_MODE_WRITE) == DESC_PIPE) {
      /* mark the descriptor close-on-exec, so that it won't be inherited
         by potential other children */
      fcntl(parentend, F_SETFD, FD_CLOEXEC);
      return Resource(req::make<PlainFile>(parentend, true));
    }

    return Resource();
  }
};

/**
 * This mutex must be non-reentrant so when the child process tries to unlock
 * it after a fork(), the call to pthread_mutex_unlock() will succeed.
 */
Mutex DescriptorItem::s_mutex(false);

const StaticString s_pipe("pipe");
const StaticString s_file("file");

static bool pre_proc_open(const Array& descriptorspec,
                          std::vector<DescriptorItem> &items) {
  /* walk the descriptor spec and set up files/pipes */
  items.resize(descriptorspec.size());
  int i = 0;
  for (ArrayIter iter(descriptorspec); iter; ++iter, ++i) {
    DescriptorItem &item = items[i];

    String index = iter.first();
    if (!index.isNumeric()) {
      raise_warning("descriptor spec must be an integer indexed array");
      break;
    }
    item.index = index.toInt32();

    Variant descitem = iter.second();
    if (descitem.isResource()) {
      auto file = cast<File>(descitem);
      if (!item.readFile(file)) break;
    } else if (!descitem.is(KindOfArray)) {
      raise_warning("Descriptor must be either an array or a File-Handle");
      break;
    } else {
      Array descarr = descitem.toArray();
      if (!descarr.exists(int64_t(0))) {
        raise_warning("Missing handle qualifier in array");
        break;
      }
      String ztype = descarr[int64_t(0)].toString();
      if (ztype == s_pipe) {
        if (!descarr.exists(int64_t(1))) {
          raise_warning("Missing mode parameter for 'pipe'");
          break;
        }
        if (!item.readPipe(descarr[int64_t(1)].toString())) break;
      } else if (ztype == s_file) {
        if (!descarr.exists(int64_t(1))) {
          raise_warning("Missing file name parameter for 'file'");
          break;
        }
        if (!descarr.exists(int64_t(2))) {
          raise_warning("Missing mode parameter for 'file'");
          break;
        }
        if (!item.openFile(descarr[int64_t(1)].toString(),
                           descarr[int64_t(2)].toString())) {
          break;
        }
      } else {
        raise_warning("%s is not a valid descriptor spec", ztype.data());
        break;
      }
    }
  }

  if (i >= descriptorspec.size()) return true;
  for (int j = 0; j < i; j++) {
    items[j].cleanup();
  }
  return false;
}

static Variant post_proc_open(const String& cmd, Variant& pipes,
                              const Variant& env,
                              std::vector<DescriptorItem> &items,
                              pid_t child) {
  if (child < 0) {
    /* failed to fork() */
    for (auto& item : items) {
      item.cleanup();
    }
    raise_warning("fork failed - %s", folly::errnoStr(errno).c_str());
    return false;
  }

  /* we forked/spawned and this is the parent */
  auto proc = req::make<ChildProcess>();
  proc->command = cmd;
  proc->child = child;
  proc->env = env;

  // need to set pipes to a new empty array, ignoring whatever it was
  // previously set to
  pipes = Variant(Array::Create());

  for (auto& item : items) {
    Resource f = item.dupParent();
    if (!f.isNull()) {
      proc->pipes.append(f);
      pipes.toArrRef().set(item.index, f);
    }
  }
  return Variant(std::move(proc));
}

Variant HHVM_FUNCTION(proc_open,
                      const String& cmd,
                      const Array& descriptorspec,
                      VRefParam pipes,
                      const String& cwd /* = null_string */,
                      const Variant& env /* = null_variant */,
                      const Variant& other_options /* = null_variant */) {
  if (RuntimeOption::WhitelistExec && !check_cmd(cmd.data())) {
    return false;
  }
  if (cmd.size() != strlen(cmd.c_str())) {
    raise_warning("NULL byte detected. Possible attack");
    return false;
  }

  std::vector<DescriptorItem> items;

  std::string scwd = "";
  if (!cwd.empty()) {
    scwd = cwd.c_str();
  } else if (!g_context->getCwd().empty()) {
    scwd = g_context->getCwd().c_str();
  }

  Array enva;

  if (env.isNull()) {
    // Build out an environment that conceptually matches what we'd
    // see if we were to iterate the environment and call getenv()
    // for each name.

    // Env vars defined in the hdf file go in first
    for (const auto& envvar : RuntimeOption::EnvVariables) {
      enva.set(String(envvar.first), String(envvar.second));
    }

    // global environment overrides the hdf
    for (char **env = environ; env && *env; env++) {
      char *p = strchr(*env, '=');
      if (p) {
        String name(*env, p - *env, CopyString);
        String val(p + 1, CopyString);
        enva.set(name, val);
      }
    }

    // and then any putenv() changes take precedence
    for (ArrayIter iter(g_context->getEnvs()); iter; ++iter) {
      enva.set(iter.first(), iter.second());
    }
  } else {
    enva = env.toArray();
  }

  pid_t child;

  if (LightProcess::Available()) {
    // light process available
    // there is no need to do any locking, because the forking is delegated
    // to the light process
    if (!pre_proc_open(descriptorspec, items)) return false;
    const int item_size = items.size();
    std::vector<int> created;
    created.reserve(item_size);
    std::vector<int> intended;
    intended.reserve(item_size);
    for (int i = 0; i < item_size; i++) {
      const auto& item = items[i];
      created.push_back(item.childend);
      intended.push_back(item.index);
    }

    std::vector<std::string> envs;
    for (ArrayIter iter(enva); iter; ++iter) {
      StringBuffer nvpair;
      nvpair.append(iter.first().toString());
      nvpair.append('=');
      nvpair.append(iter.second().toString());
      std::string tmp = nvpair.detach().c_str();
      if (tmp.find('\n') == std::string::npos) {
        envs.push_back(tmp);
      }
    }

    child = LightProcess::proc_open(cmd.c_str(), created, intended,
                                    scwd.c_str(), envs);
    assert(child);
    return post_proc_open(cmd, pipes, enva, items, child);
  } else {
    /* the unix way */
    Lock lock(DescriptorItem::s_mutex);
    if (!pre_proc_open(descriptorspec, items)) return false;
    child = fork();
    if (child) {
      // the parent process
      return post_proc_open(cmd, pipes, enva, items, child);
    }
  }

  assert(child == 0);
  /* this is the child process */

  /* close those descriptors that we just opened for the parent stuff,
   * dup new descriptors into required descriptors and close the original
   * cruft */
  for (auto& item : items) {
    item.dupChild();
  }
  if (scwd.length() > 0 && chdir(scwd.c_str())) {
    // chdir failed, the working directory remains unchanged
  }
  std::vector<String> senvs; // holding those char *
  char **envp = build_envp(enva, senvs);
  execle("/bin/sh", "sh", "-c", cmd.data(), NULL, envp);
  free(envp);
  _exit(127);
}

bool HHVM_FUNCTION(proc_terminate,
                   const Resource& process,
                   int signal /* = SIGTERM */) {
  return kill(cast<ChildProcess>(process)->child, signal) == 0;
}

int64_t HHVM_FUNCTION(proc_close,
                      const Resource& process) {
  return cast<ChildProcess>(process)->close();
}

const StaticString
  s_command("command"),
  s_pid("pid"),
  s_running("running"),
  s_signaled("signaled"),
  s_stopped("stopped"),
  s_exitcode("exitcode"),
  s_termsig("termsig"),
  s_stopsig("stopsig");

Array HHVM_FUNCTION(proc_get_status,
                    const Resource& process) {
  auto proc = cast<ChildProcess>(process);

  errno = 0;
  int wstatus;
  pid_t wait_pid =
    LightProcess::waitpid(proc->child, &wstatus, WNOHANG|WUNTRACED);

  bool running = true, signaled = false, stopped = false;
  int exitcode = -1, termsig = 0, stopsig = 0;
  if (wait_pid == proc->child) {
    if (WIFEXITED(wstatus)) {
      running = false;
      exitcode = WEXITSTATUS(wstatus);
    }
    if (WIFSIGNALED(wstatus)) {
      running = false;
      signaled = true;
      termsig = WTERMSIG(wstatus);
    }
    if (WIFSTOPPED(wstatus)) {
      stopped = true;
      stopsig = WSTOPSIG(wstatus);
    }
  } else if (wait_pid == -1) {
    running = false;
  }

  return make_map_array(
    s_command,  proc->command,
    s_pid,      proc->child,
    s_running,  running,
    s_signaled, signaled,
    s_stopped,  stopped,
    s_exitcode, exitcode,
    s_termsig,  termsig,
    s_stopsig,  stopsig
  );
}

bool HHVM_FUNCTION(proc_nice,
                   int increment) {
  if (nice(increment) < 0 && errno) {
    raise_warning("Only a super user may attempt to increase the "
                    "priority of a process");
    return false;
  }
  return true;
}

///////////////////////////////////////////////////////////////////////////////
// string functions

const StaticString s_twosinglequotes("''");

String HHVM_FUNCTION(escapeshellarg,
                     const String& arg) {
  if (!arg.empty()) {
    return string_escape_shell_arg(arg.c_str());
  } else if (!RuntimeOption::EnableHipHopSyntax) {
    return String(s_twosinglequotes);
  }
  return arg;
}

String HHVM_FUNCTION(escapeshellcmd,
                     const String& command) {
  if (!command.empty()) {
    return string_escape_shell_cmd(command.c_str());
  }
  return command;
}

///////////////////////////////////////////////////////////////////////////////
}
