/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
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

#include <folly/portability/SysTime.h>
#include <folly/portability/SysResource.h>
#include <folly/portability/Unistd.h>
#include <fcntl.h>
#include <signal.h>

#include <folly/String.h>

#include "hphp/util/hugetlb.h"
#include "hphp/util/light-process.h"
#include "hphp/util/lock.h"
#include "hphp/util/logger.h"

#include "hphp/runtime/base/array-init.h"
#include "hphp/runtime/base/builtin-functions.h"
#include "hphp/runtime/base/plain-file.h"
#include "hphp/runtime/base/request-event-handler.h"
#include "hphp/runtime/base/request-local.h"
#include "hphp/runtime/base/string-buffer.h"
#include "hphp/runtime/base/string-util.h"
#include "hphp/runtime/base/surprise-flags.h"
#include "hphp/runtime/base/thread-info.h"
#include "hphp/runtime/base/zend-string.h"
#include "hphp/runtime/ext/std/ext_std_file.h"
#include "hphp/runtime/ext/std/ext_std_function.h"
#include "hphp/runtime/ext/string/ext_string.h"
#include "hphp/runtime/server/cli-server.h"
#include "hphp/runtime/vm/repo.h"

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

static struct ProcessExtension final : Extension {
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

    HHVM_RC_INT_SAME(SIGABRT);
    HHVM_RC_INT_SAME(SIGALRM);
    HHVM_RC_INT_SAME(SIGBUS);
    HHVM_RC_INT_SAME(SIGCHLD);
    HHVM_RC_INT_SAME(SIGCONT);
    HHVM_RC_INT_SAME(SIGFPE);
    HHVM_RC_INT_SAME(SIGHUP);
    HHVM_RC_INT_SAME(SIGILL);
    HHVM_RC_INT_SAME(SIGINT);
    HHVM_RC_INT_SAME(SIGIO);
    HHVM_RC_INT_SAME(SIGIOT);
    HHVM_RC_INT_SAME(SIGKILL);
    HHVM_RC_INT_SAME(SIGPIPE);
    HHVM_RC_INT_SAME(SIGPROF);
    HHVM_RC_INT_SAME(SIGQUIT);
    HHVM_RC_INT_SAME(SIGSEGV);
    HHVM_RC_INT_SAME(SIGSTOP);
    HHVM_RC_INT_SAME(SIGSYS);
    HHVM_RC_INT_SAME(SIGTERM);
    HHVM_RC_INT_SAME(SIGTRAP);
    HHVM_RC_INT_SAME(SIGTSTP);
    HHVM_RC_INT_SAME(SIGTTIN);
    HHVM_RC_INT_SAME(SIGTTOU);
    HHVM_RC_INT_SAME(SIGURG);
    HHVM_RC_INT_SAME(SIGUSR1);
    HHVM_RC_INT_SAME(SIGUSR2);
    HHVM_RC_INT_SAME(SIGVTALRM);
    HHVM_RC_INT_SAME(SIGWINCH);
    HHVM_RC_INT_SAME(SIGXCPU);
    HHVM_RC_INT_SAME(SIGXFSZ);
    HHVM_RC_INT_SAME(SIG_BLOCK);
    HHVM_RC_INT_SAME(SIG_UNBLOCK);
    HHVM_RC_INT_SAME(SIG_SETMASK);

    HHVM_RC_INT(SIG_DFL, (int64_t)SIG_DFL);
    HHVM_RC_INT(SIG_ERR, (int64_t)SIG_ERR);
    HHVM_RC_INT(SIG_IGN, (int64_t)SIG_IGN);

    // http://marc.info/?l=php-cvs&m=100289252314474&w=2
    HHVM_RC_INT(SIGBABY, SIGSYS);
    HHVM_RC_INT(SIGCLD, SIGCHLD);
    HHVM_RC_INT(SIGPOLL, SIGIO);

#ifdef __linux__
    HHVM_RC_INT_SAME(SIGPWR);
    HHVM_RC_INT_SAME(SIGSTKFLT);
#endif

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
  if (is_cli_mode()) {
    raise_error("forking not available via server CLI execution");
    return -1;
  }
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
    if (RuntimeOption::EvalPerfMemEventRequestFreq != 0) {
      sigdelset(&set, SIGIO);
    }
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

struct SignalHandlersStaticInitializer {
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
  auto const invalid_argument = [&] {
    raise_warning("pcntl_sigprocmask(): Invalid argument");
    return false;
  };

  if (how != SIG_BLOCK && how != SIG_UNBLOCK && how != SIG_SETMASK) {
    return invalid_argument();
  }

  sigset_t cset;
  sigset_t coldset;

  sigemptyset(&cset);
  for (ArrayIter iter(set); iter; ++iter) {
    auto value = iter.second().toInt64();
    if (sigaddset(&cset, value) == -1) {
      return invalid_argument();
    }
  }

  if (pthread_sigmask(how, &cset, &coldset)) {
    return false;
  }

  auto aoldset = Array::Create();
  for (int signum = 1; signum < NSIG; ++signum) {
    auto const result = sigismember(&coldset, signum);
    if (result == 1) {
      aoldset.append(signum);
    } else if (result == -1) {
      // Invalid signal number.
      break;
    }
  }

  oldset.assignIfRef(aoldset);
  return true;
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
  status.assignIfRef(nstatus);
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
  status.assignIfRef(nstatus);
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
}
