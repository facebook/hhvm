/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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

#include "hphp/runtime/ext/ext_process.h"
#include "hphp/runtime/ext/ext_file.h"
#include "hphp/runtime/ext/ext_function.h"
#include "hphp/runtime/ext/ext_string.h"
#include "hphp/runtime/base/string-buffer.h"
#include "hphp/runtime/base/zend-string.h"
#include "hphp/runtime/base/thread-init-fini.h"
#include "folly/String.h"
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include "hphp/util/lock.h"
#include "hphp/runtime/base/plain-file.h"
#include "hphp/util/light-process.h"
#include "hphp/util/logger.h"
#include "hphp/runtime/base/request-local.h"
#include "hphp/runtime/vm/repo.h"

#if !defined(_NSIG) && defined(NSIG)
# define _NSIG NSIG
#endif

extern char **environ;

namespace HPHP {

///////////////////////////////////////////////////////////////////////////////

// build environment pair list
static char **build_envp(CArrRef envs, std::vector<String> &senvs) {
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
      String file = g_vmContext->getContainingFileName();
      int line = g_vmContext->getLine();
      Logger::Warning("Command %s is not in the whitelist, called at %s:%d",
                      cmd_tmp, file.data(), line);
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

IMPLEMENT_DEFAULT_EXTENSION(pcntl);

int64_t f_pcntl_alarm(int seconds) {
  return alarm(seconds);
}

void f_pcntl_exec(const String& path, CArrRef args /* = null_array */,
                  CArrRef envs /* = null_array */) {
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
    raise_warning("Error has occured: (errno %d) %s",
                    errno, folly::errnoStr(errno).c_str());
  }

  free(envp);
  free(argv);
}

int64_t f_pcntl_fork() {
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

Variant f_pcntl_getpriority(int pid /* = 0 */,
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
      raise_warning("Unknown error %d has occured", errno);
      break;
    }
    return false;
  }
  return pri;
}

bool f_pcntl_setpriority(int priority, int pid /* = 0 */,
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
      raise_warning("Unknown error %d has occured", errno);
      break;
    }
    return false;
  }
  return true;
}

/* php_signal using sigaction is derrived from Advanced Programing
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
class SignalHandlers : public RequestEventHandler {
public:
  SignalHandlers() {
    memset(signaled, 0, sizeof(signaled));
    pthread_sigmask(SIG_SETMASK, NULL, &oldSet);
  }
  virtual void requestInit() {
    handlers.reset();
    // restore the old signal mask, thus unblock those that should be
    pthread_sigmask(SIG_SETMASK, &oldSet, NULL);
  }
  virtual void requestShutdown() {
    // block all signals
    sigset_t set;
    sigfillset(&set);
    pthread_sigmask(SIG_BLOCK, &set, NULL);

    handlers.reset();
  }

  Array handlers;
  int signaled[_NSIG];
  sigset_t oldSet;
};
IMPLEMENT_STATIC_REQUEST_LOCAL(SignalHandlers, s_signal_handlers);

// We must register the s_signal_handlers RequestEventHandler
// immediately: otherwise, pcntl_signal_handler might try to register
// it while processing a signal, which means calling malloc to insert
// it into various vectors and sets, which is not ok from a signal
// handler.
static InitFiniNode initSignalHandler(
  [] { s_signal_handlers.get(); },
  InitFiniNode::When::ThreadInit
);

static void pcntl_signal_handler(int signo) {
  if (signo > 0 && signo < _NSIG && !g_context.isNull()) {
    s_signal_handlers->signaled[signo] = 1;
    RequestInjectionData &data = ThreadInfo::s_threadInfo.getNoCheck()->
                                   m_reqInjectionData;
    data.setSignaledFlag();
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

bool f_pcntl_signal_dispatch() {
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

bool f_pcntl_signal(int signo, CVarRef handler,
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

  if (!f_is_callable(handler)) {
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

int64_t f_pcntl_wait(VRefParam status, int options /* = 0 */) {
  int child_id;
  int nstatus = 0;
  child_id = LightProcess::pcntl_waitpid(-1, &nstatus, options);
/*  if (options) {
    child_id = wait3(&nstatus, options, NULL);
  } else {
    child_id = wait(&nstatus);
  }*/
  status = nstatus;
  return child_id;
}

int64_t f_pcntl_waitpid(int pid, VRefParam status, int options /* = 0 */) {
  int nstatus = status;
  pid_t child_id = LightProcess::pcntl_waitpid((pid_t)pid, &nstatus, options);
  status = nstatus;
  return child_id;
}

int64_t f_pcntl_wexitstatus(int status) {
  return WEXITSTATUS(status);
}

bool f_pcntl_wifexited(int status) { return WIFEXITED(status);}
bool f_pcntl_wifsignaled(int status) { return WIFSIGNALED(status);}
bool f_pcntl_wifstopped(int status) { return WIFSTOPPED(status);}
int64_t f_pcntl_wstopsig(int status) { return WSTOPSIG(status);}
int64_t f_pcntl_wtermsig(int status) { return WTERMSIG(status);}

///////////////////////////////////////////////////////////////////////////////
// popen

#define EXEC_INPUT_BUF 4096

class ShellExecContext {
public:
  ShellExecContext() : m_proc(NULL) {
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

  FILE *exec(const char *cmd) {
    assert(m_proc == NULL);
    if (RuntimeOption::WhitelistExec && !check_cmd(cmd)) {
      return NULL;
    }
    m_proc = LightProcess::popen(cmd, "r", g_context->getCwd().data());
    if (m_proc == NULL) {
      raise_warning("Unable to execute '%s'", cmd);
    }
    return m_proc;
  }

  int exit() {
    int status = LightProcess::pclose(m_proc);
    m_proc = NULL;
    return status;
  }

private:
  void (*m_sig_handler)(int);
  FILE *m_proc;
};

String f_shell_exec(const String& cmd) {
  ShellExecContext ctx;
  FILE *fp = ctx.exec(cmd.c_str());
  if (!fp) return "";
  StringBuffer sbuf;
  sbuf.read(fp);
  return sbuf.detach();
}

String f_exec(const String& command, VRefParam output /* = null */,
              VRefParam return_var /* = null */) {
  ShellExecContext ctx;
  FILE *fp = ctx.exec(command.c_str());
  if (!fp) return "";
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
  if (!output.is(KindOfArray)) {
    output = Array(ArrayData::Create());
  }

  for (int i = 0; i < count; i++) {
    output.append(lines[i]);
  }

  if (!count || lines.empty()) {
    return String();
  }

  return f_rtrim(lines[count - 1].toString());
}

void f_passthru(const String& command, VRefParam return_var /* = null */) {
  ShellExecContext ctx;
  FILE *fp = ctx.exec(command.c_str());
  if (!fp) return;

  char buffer[1024];
  while (true) {
    int len = read(fileno(fp), buffer, sizeof(buffer) - 1);
    if (len == -1 && errno == EINTR) continue;
    if (len <= 0) break; // break on error or EOF
    buffer[len] = '\0';
    echo(String(buffer, len, CopyString));
  }
  int ret = ctx.exit();
  if (WIFEXITED(ret)) ret = WEXITSTATUS(ret);
  return_var = ret;
}

String f_system(const String& command, VRefParam return_var /* = null */) {
  ShellExecContext ctx;
  FILE *fp = ctx.exec(command.c_str());
  if (!fp) return "";
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

  for (int i = 0; i < count; i++) {
    echo(lines[i].toString());
    echo("\n");
  }
  if (!count || lines.empty()) {
    return String();
  }

  return f_rtrim(lines[count - 1].toString());
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
  virtual const String& o_getClassNameHook() const { return classnameof(); }

  int close() {
    // Although the PHP doc about proc_close() says that the pipes need to be
    // explicitly pclose()'ed, it seems that Zend is implicitly closing the
    // pipes when proc_close() is called.
    for (ArrayIter iter(pipes); iter; ++iter) {
      iter.second().toResource().getTyped<PlainFile>()->close();
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
  // do nothing here, as everything will be collected by SmartAllocator
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

  bool readFile(File *file) {
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
    Variant vfile = f_fopen(zfile.c_str(), zmode.c_str());
    if (!vfile.isResource()) {
      raise_warning("Unable to open specified file: %s (mode %s)",
                      zfile.data(), zmode.data());
      return false;
    } else {
      File *file = vfile.toResource().getTyped<File>();
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
      return Resource(NEWOBJ(PlainFile)(parentend, true));
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

static bool pre_proc_open(CArrRef descriptorspec,
                          vector<DescriptorItem> &items) {
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
      File *file = descitem.toResource().getTyped<File>();
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
        if (!item.openFile(descarr[int64_t(1)].toString(), descarr[int64_t(2)].toString())) {
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

static Variant post_proc_open(const String& cmd, Variant &pipes,
                              CVarRef env, vector<DescriptorItem> &items,
                              pid_t child) {
  if (child < 0) {
    /* failed to fork() */
    for (int i = 0; i < (int)items.size(); i++) {
      items[i].cleanup();
    }
    raise_warning("fork failed - %s", folly::errnoStr(errno).c_str());
    return false;
  }

  /* we forked/spawned and this is the parent */
  ChildProcess *proc = NEWOBJ(ChildProcess)();
  proc->command = cmd;
  proc->child = child;
  proc->env = env;

  // need to set pipes to a new empty array, ignoring whatever it was
  // previously set to
  pipes = Variant(Array::Create());

  for (int i = 0; i < (int)items.size(); i++) {
    Resource f = items[i].dupParent();
    if (!f.isNull()) {
      proc->pipes.append(f);
      pipes.set(items[i].index, f);
    }
  }
  return Resource(proc);
}

Variant f_proc_open(const String& cmd, CArrRef descriptorspec, VRefParam pipes,
                    const String& cwd /* = null_string */,
                    CVarRef env /* = null_variant */,
                    CVarRef other_options /* = null_variant */) {
  if (RuntimeOption::WhitelistExec && !check_cmd(cmd.data())) {
    return false;
  }

  std::vector<DescriptorItem> items;

  string scwd = "";
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
    for (std::map<string, string>::const_iterator iter =
        RuntimeOption::EnvVariables.begin();
        iter != RuntimeOption::EnvVariables.end(); ++iter) {
      enva.set(String(iter->first), String(iter->second));
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
    std::vector<int> created;
    std::vector<int> intended;
    for (int i = 0; i < (int)items.size(); i++) {
      created.push_back(items[i].childend);
      intended.push_back(items[i].index);
    }

    std::vector<std::string> envs;
    for (ArrayIter iter(enva); iter; ++iter) {
      StringBuffer nvpair;
      nvpair.append(iter.first().toString());
      nvpair.append('=');
      nvpair.append(iter.second().toString());
      string tmp = nvpair.detach().c_str();
      if (tmp.find('\n') == string::npos) {
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
  for (int i = 0; i < (int)items.size(); i++) {
    items[i].dupChild();
  }
  if (scwd.length() > 0 && chdir(scwd.c_str())) {
    // chdir failed, the working directory remains unchanged
  }
  vector<String> senvs; // holding those char *
  char **envp = build_envp(enva, senvs);
  execle("/bin/sh", "sh", "-c", cmd.data(), NULL, envp);
  free(envp);
  _exit(127);
}

bool f_proc_terminate(CResRef process, int signal /* = 0 */) {
  ChildProcess *proc = process.getTyped<ChildProcess>();
  return kill(proc->child, signal <= 0 ? SIGTERM : signal) == 0;
}

int64_t f_proc_close(CResRef process) {
  return process.getTyped<ChildProcess>()->close();
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

Array f_proc_get_status(CResRef process) {
  ChildProcess *proc = process.getTyped<ChildProcess>();

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

  ArrayInit ret(8);
  ret.set(s_command,  proc->command);
  ret.set(s_pid,      proc->child);
  ret.set(s_running,  running);
  ret.set(s_signaled, signaled);
  ret.set(s_stopped,  stopped);
  ret.set(s_exitcode, exitcode);
  ret.set(s_termsig,  termsig);
  ret.set(s_stopsig,  stopsig);
  return ret.create();
}

bool f_proc_nice(int increment) {
  if (nice(increment) < 0 && errno) {
    raise_warning("Only a super user may attempt to increase the "
                    "priority of a process");
    return false;
  }
  return true;
}

///////////////////////////////////////////////////////////////////////////////
// string functions

String f_escapeshellarg(const String& arg) {
  if (!arg.empty()) {
    char *ret = string_escape_shell_arg(arg.c_str());
    return String(ret, AttachString);
  }
  return arg;
}

String f_escapeshellcmd(const String& command) {
  if (!command.empty()) {
    char *ret = string_escape_shell_cmd(command.c_str());
    return String(ret, AttachString);
  }
  return command;
}

///////////////////////////////////////////////////////////////////////////////
}
