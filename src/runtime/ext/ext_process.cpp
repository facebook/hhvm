/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010 Facebook, Inc. (http://www.facebook.com)          |
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

#include <runtime/ext/ext_process.h>
#include <runtime/ext/ext_file.h>
#include <runtime/ext/ext_function.h>
#include <runtime/base/util/string_buffer.h>
#include <runtime/base/zend/zend_string.h>
#include <unistd.h>
#include <fcntl.h>
#include <util/lock.h>
#include <runtime/base/file/plain_file.h>
#include <util/light_process.h>
#include <runtime/base/util/request_local.h>

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
      nvpair += iter.first().toString();
      nvpair += '=';
      nvpair += iter.second().toString();

      String env = nvpair.detach();
      senvs.push_back(env);
      *(envp + i) = (char *)env.data();
    }
    *(envp + i) = NULL;
  }
  return envp;
}

///////////////////////////////////////////////////////////////////////////////
// pcntl

void f_pcntl_exec(CStrRef path, CArrRef args /* = null_array */,
                  CArrRef envs /* = null_array */) {
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
  if (execve(path, argv, envp) == -1) {
    raise_warning("Error has occured: (errno %d) %s",
                    errno, Util::safe_strerror(errno).c_str());
  }

  free(envp);
  free(argv);
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
  }
  virtual void requestInit() {
    handlers.reset();
  }
  virtual void requestShutdown() {
    handlers.reset();
  }

  Array handlers;
  int signaled[_NSIG];
};
IMPLEMENT_STATIC_REQUEST_LOCAL(SignalHandlers, s_signal_handlers);

static void pcntl_signal_handler(int signo) {
  if (signo > 0 && signo < _NSIG) {
    s_signal_handlers->signaled[signo] = 1;
    ThreadInfo::s_threadInfo->m_reqInjectionData.signaled = true;
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
  ThreadInfo::s_threadInfo->m_reqInjectionData.signaled = false;
  int *signaled = s_signal_handlers->signaled;
  bool error = false;
  for (int i = 0; i < _NSIG; i++) {
    if (signaled[i]) {
      signaled[i] = 0;
      if (s_signal_handlers->handlers.exists(i)) {
        try {
          f_call_user_func_array(s_signal_handlers->handlers[i],
                                 CREATE_VECTOR1(i));
        } catch (...) {
          error = true;
        }
      }
    }
  }
  return !error;
}

bool f_pcntl_signal(int signo, CVarRef handler,
                    bool restart_syscalls /* = true */) {
  /* Special long value case for SIG_DFL and SIG_IGN */
  if (handler.isInteger()) {
    int64 handle = handler.toInt64();
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

int f_pcntl_wait(Variant status, int options /* = 0 */) {
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

int f_pcntl_waitpid(int pid, Variant status, int options /* = 0 */) {
  int nstatus = status.toInt64();
  pid_t child_id = LightProcess::pcntl_waitpid((pid_t)pid, &nstatus, options);
  status = nstatus;
  return child_id;
}

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
    ASSERT(m_proc == NULL);
    m_proc = LightProcess::popen(cmd, "r");
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

String f_shell_exec(CStrRef cmd) {
  ShellExecContext ctx;
  FILE *fp = ctx.exec(cmd);
  StringBuffer sbuf;
  sbuf.read(fp);
  return sbuf.detach();
}

String f_exec(CStrRef command, Variant output /* = null */,
              Variant return_var /* = null */) {
  ShellExecContext ctx;
  FILE *fp = ctx.exec(command);
  if (!fp) return "";
  StringBuffer sbuf;
  sbuf.read(fp);

  Array lines = StringUtil::Explode(sbuf, "\n");
  return_var = ctx.exit();
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
  return StringUtil::Trim(lines[count - 1], StringUtil::TrimRight);
}

void f_passthru(CStrRef command, Variant return_var /* = null */) {
  ShellExecContext ctx;
  FILE *fp = ctx.exec(command);

  char buffer[1024];
  while (true) {
    int len = fread(buffer, 1, sizeof(buffer) - 1, fp);
    if (len == 0) break;
    buffer[len] = '\0';
    echo(String(buffer, len, AttachLiteral));
  }
  return_var = ctx.exit();
}

String f_system(CStrRef command, Variant return_var /* = null */) {
  ShellExecContext ctx;
  FILE *fp = ctx.exec(command);
  StringBuffer sbuf;
  if (fp) {
    sbuf.read(fp);
  }

  Array lines = StringUtil::Explode(sbuf, "\n");
  return_var = ctx.exit();
  int count = lines.size();
  if (count > 0 && lines[count - 1].toString().empty()) {
    count--; // remove explode()'s last empty line
  }

  for (int i = 0; i < count; i++) {
    echo(lines[i]);
    echo("\n");
  }
  if (!count || lines.empty()) {
    return String();
  }
  return StringUtil::Trim(lines[count - 1], StringUtil::TrimRight);
}

///////////////////////////////////////////////////////////////////////////////
// proc

class ChildProcess : public SweepableResourceData {
public:
  DECLARE_OBJECT_ALLOCATION(ChildProcess);

  pid_t child;
  Array pipes;
  String command;
  Variant env;

  // overriding ResourceData
  const char *o_getClassName() const { return "Process";}

  int close() {
    pid_t wait_pid;
    int wstatus;
    do {
      wait_pid = LightProcess::waitpid(child, &wstatus, 0);
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
IMPLEMENT_OBJECT_ALLOCATION_NO_DEFAULT_SWEEP(ChildProcess);
void ChildProcess::sweep() {
  // do nothing here, as everything will be collected by SmartAllocator
}

#define DESC_PIPE       1
#define DESC_FILE       2
#define DESC_PARENT_MODE_WRITE  8

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

  bool readFile(File *file) {
    mode = DESC_FILE;
    childend = dup(file->fd());
    if (childend < 0) {
      raise_warning("unable to dup File-Handle for descriptor %ld - %s",
                      index, Util::safe_strerror(errno).c_str());
      return false;
    }
    return true;
  }

  bool readPipe(CStrRef zmode) {
    mode = DESC_PIPE;
    int newpipe[2];
    if (0 != pipe(newpipe)) {
      raise_warning("unable to create pipe %s",
                      Util::safe_strerror(errno).c_str());
      return false;
    }

    if (zmode != "w") {
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

  bool openFile(CStrRef zfile, CStrRef zmode) {
    mode = DESC_FILE;
    /* try a wrapper */
    FILE *file = fopen(zfile, zmode);
    if (!file) {
      raise_warning("Unable to open specified file: %s (mode %s)",
                      zfile.data(), zmode.data());
      return false;
    }
    childend = fileno(file);
    return true;
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
  Object dupParent() {
    close(childend); childend = -1;

    if ((mode & ~DESC_PARENT_MODE_WRITE) == DESC_PIPE) {
      const char *mode_string = NULL;
      switch(mode_flags) {
      case O_WRONLY:
        mode_string = "w";
        break;
      case O_RDONLY:
        mode_string = "r";
        break;
      case O_RDWR:
        mode_string = "r+";
        break;
      }
      FILE *f = fdopen(parentend, mode_string);
      /* mark the descriptor close-on-exec, so that it won't be inherited
         by potential other children */
      fcntl(parentend, F_SETFD, FD_CLOEXEC);
      return Object(NEW(PlainFile)(f, true));
    }

    return Object();
  }
};

Variant f_proc_open(CStrRef cmd, CArrRef descriptorspec, Variant pipes,
                    CStrRef cwd /* = null_string */, CVarRef env /* = null_variant */,
                    CVarRef other_options /* = null_variant */) {
  /* walk the descriptor spec and set up files/pipes */
  std::vector<DescriptorItem> items;
  items.resize(descriptorspec.size());
  int i = 0;
  for (ArrayIter iter(descriptorspec); iter; ++iter, ++i) {
    DescriptorItem &item = items[i];

    String index = iter.first();
    if (!index.isNumeric()) {
      raise_warning("descriptor spec must be an integer indexed array");
      return false;
    }
    item.index = index.toInt32();

    Variant descitem = iter.second();
    if (descitem.isResource()) {
      File *file = descitem.toObject().getTyped<File>();
      if (!item.readFile(file)) {
        return false;
      }
    } else if (!descitem.is(KindOfArray)) {
      raise_warning("Descriptor must be either an array or a File-Handle");
      return false;
    } else {
      Array descarr = descitem.toArray();
      if (!descarr.exists(0LL)) {
        raise_warning("Missing handle qualifier in array");
        return false;
      }
      String ztype = descarr[0LL].toString();
      if (ztype == "pipe") {
        if (!descarr.exists(1LL)) {
          raise_warning("Missing mode parameter for 'pipe'");
          return false;
        }
        if (!item.readPipe(descarr[1LL].toString())) {
          return false;
        }
      } else if (ztype == "file") {
        if (!descarr.exists(1LL)) {
          raise_warning("Missing file name parameter for 'file'");
          return false;
        }
        if (!descarr.exists(2LL)) {
          raise_warning("Missing mode parameter for 'file'");
          return false;
        }
        if (!item.openFile(descarr[1LL].toString(), descarr[2LL].toString())) {
          return false;
        }
      } else {
        raise_warning("%s is not a valid descriptor spec", ztype.data());
        return false;
      }
    }
  }

  pid_t child;

  if (LightProcess::Available()) {
    // light-weight process available
    std::vector<int> created;
    std::vector<int> intended;
    for (i = 0; i < (int)items.size(); i++) {
      created.push_back(items[i].childend);
      intended.push_back(items[i].index);
    }

    std::vector<std::string> envs;
    for (ArrayIter iter(env.toArray()); iter; ++iter) {
      StringBuffer nvpair;
      nvpair += iter.first().toString();
      nvpair += '=';
      nvpair += iter.second().toString();
      envs.push_back(nvpair.detach().c_str());
    }

    child = LightProcess::proc_open(cmd.c_str(), created, intended,
                                    cwd.c_str(), envs);
  } else {
    /* the unix way */
    child = fork();
    if (child == 0) {
      /* this is the child process */

      /* close those descriptors that we just opened for the parent stuff,
       * dup new descriptors into required descriptors and close the original
       * cruft */
      for (i = 0; i < (int)items.size(); i++) {
        items[i].dupChild();
      }
      if (!cwd.empty()) {
        if (chdir(cwd) < 0) {
          // chdir failed, the working directory remains unchanged
        }
      }
      if (!env.isNull()) {
        std::vector<String> senvs; // holding those char *
        char **envp = build_envp(env.toArray(), senvs);
        execle("/bin/sh", "sh", "-c", cmd.data(), NULL, envp);
        free(envp);
      } else {
        execl("/bin/sh", "sh", "-c", cmd.data(), NULL);
      }
      _exit(127);

    }
  }
  if (child < 0) {
    /* failed to fork() */
    for (i = 0; i < (int)items.size(); i++) {
      items[i].cleanup();
    }
    raise_warning("fork failed - %s", Util::safe_strerror(errno).c_str());
    return false;
  }

  /* we forked/spawned and this is the parent */
  ChildProcess *proc = NEW(ChildProcess)();
  proc->command = cmd;
  proc->child = child;
  proc->env = env;
  for (i = 0; i < (int)items.size(); i++) {
    Object f = items[i].dupParent();
    proc->pipes.append(f);
    pipes.set(items[i].index, f);
  }
  return Object(proc);
}

bool f_proc_terminate(CObjRef process, int signal /* = 0 */) {
  ChildProcess *proc = process.getTyped<ChildProcess>();
  return kill(proc->child, signal <= 0 ? SIGTERM : signal) == 0;
}

int f_proc_close(CObjRef process) {
  return process.getTyped<ChildProcess>()->close();
}

Array f_proc_get_status(CObjRef process) {
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

  Array ret;
  ret.set("command",  proc->command);
  ret.set("pid",      proc->child);
  ret.set("running",  running);
  ret.set("signaled", signaled);
  ret.set("stopped",  stopped);
  ret.set("exitcode", exitcode);
  ret.set("termsig",  termsig);
  ret.set("stopsig",  stopsig);
  return ret;
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

String f_escapeshellarg(CStrRef arg) {
  if (!arg.empty()) {
    char *ret = string_escape_shell_arg(arg);
    return String(ret, AttachString);
  }
  return arg;
}

String f_escapeshellcmd(CStrRef command) {
  if (!command.empty()) {
    char *ret = string_escape_shell_cmd(command);
    return String(ret, AttachString);
  }
  return command;
}

///////////////////////////////////////////////////////////////////////////////
}
