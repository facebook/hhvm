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
#include <folly/String.h>

#include <fcntl.h>
#include <signal.h>

#include "hphp/runtime/base/array-init.h"
#include "hphp/runtime/base/array-iterator.h"
#include "hphp/runtime/base/builtin-functions.h"
#include "hphp/runtime/base/init-fini-node.h"
#include "hphp/runtime/base/plain-file.h"
#include "hphp/runtime/base/rds.h"
#include "hphp/runtime/base/request-event-handler.h"
#include "hphp/runtime/base/string-buffer.h"
#include "hphp/runtime/base/string-util.h"
#include "hphp/runtime/base/surprise-flags.h"
#include "hphp/runtime/base/request-info.h"
#include "hphp/runtime/base/zend-string.h"
#include "hphp/runtime/ext/std/ext_std_file.h"
#include "hphp/runtime/ext/std/ext_std_function.h"
#include "hphp/runtime/ext/string/ext_string.h"
#include "hphp/runtime/server/cli-server.h"
#include "hphp/runtime/server/xbox-server.h"
#include "hphp/runtime/vm/unit-parser.h"
#include "hphp/util/hugetlb.h"
#include "hphp/util/light-process.h"
#include "hphp/util/lock.h"
#include "hphp/util/logger.h"
#include "hphp/util/process.h"
#include "hphp/util/sync-signal.h"

extern char **environ;

namespace HPHP {

///////////////////////////////////////////////////////////////////////////////

// build environment pair list
static char** build_envp(const Array& envs, req::vector<String> &senvs) {
  auto const size = envs.size();
  if (!size) return nullptr;
  auto envp = req::make_raw_array<char*>(size + 1);
  size_t i = 0;
  for (ArrayIter iter(envs); iter; ++iter, ++i) {
    StringBuffer nvpair;
    nvpair.append(iter.first().toString());
    nvpair.append('=');
    nvpair.append(iter.second().toString());
    String env = nvpair.detach();
    senvs.push_back(env);
    envp[i] = (char*)env.data();
  }
  envp[i] = nullptr;
  return envp;
}

///////////////////////////////////////////////////////////////////////////////
// pcntl

static struct ProcessExtension final : Extension {
  ProcessExtension() : Extension("pcntl", NO_EXTENSION_VERSION_YET, NO_ONCALL_YET) {}
  std::vector<std::string> hackFiles() const override {
    return {"process"};
  }
  void moduleRegisterNative() override;
} s_process_extension;

int64_t HHVM_FUNCTION(pcntl_alarm,
                      int64_t seconds) {
  return alarm(seconds);
}

namespace {

static SimpleMutex s_lock;

bool cantPrefork() {
  if (num_1g_pages() > 0 || RuntimeOption::EvalFileBackedColdArena) {
    // We put data on shared pages, which won't work with fork().
    return true;
  }
  s_lock.lock();
  XboxServer::Stop();
  if (AsyncFuncImpl::count()) {
    XboxServer::Restart();
    s_lock.unlock();
    return true;
  }
  folly::SingletonVault::singleton()->destroyInstances();
  return false;
}

void postfork(pid_t pid) {
  folly::SingletonVault::singleton()->reenableInstances();
  XboxServer::Restart();
  if (pid == 0) {
    Logger::ResetPid();
    new (&s_lock) SimpleMutex();
  } else {
    s_lock.unlock();
  }
}

}

void HHVM_FUNCTION(pcntl_exec,
                   const String& path,
                   const Array& args /* = null_array */,
                   const Array& envs /* = null_array */) {
  if (cantPrefork()) {
    raise_error("execing is disallowed in multi-threaded mode");
    return;
  }

  // build argument list
  req::vector<String> sargs; // holding those char *
  auto const size = args.size();
  auto argv = req::make_raw_array<char*>(size + 2);
  argv[0] = (char*)path.data();
  int i = 1;
  if (size) {
    sargs.reserve(size);
    for (ArrayIter iter(args); iter; ++iter, ++i) {
      String arg = iter.second().toString();
      sargs.push_back(arg);
      argv[i] = (char*)arg.data();
    }
  }
  argv[i] = nullptr;

  // build environment pair list
  req::vector<String> senvs; // holding those char *
  auto envp = build_envp(envs, senvs);
  if (execve(path.c_str(), argv, envp) == -1) {
    raise_warning("Error has occurred: (errno %d) %s",
                    errno, folly::errnoStr(errno).c_str());
  }

  req::free(envp);
  req::free(argv);
}

int64_t HHVM_FUNCTION(pcntl_fork) {
  if (is_cli_server_mode()) {
    raise_error("forking not available via server CLI execution");
    return -1;
  }
  if (RuntimeOption::ServerExecutionMode()) {
    raise_error("forking is disallowed in server mode");
    return -1;
  }
  if (cantPrefork()) {
    raise_error("forking is disallowed in multi-threaded mode");
    return -1;
  }

  std::cout.flush();
  std::cerr.flush();
  pid_t pid = fork();
  postfork(pid);
  if (pid == 0) {
    postfork_restart_handler_thread();
  }
  return pid;
}

Variant HHVM_FUNCTION(pcntl_getpriority,
                      int64_t pid /* = 0 */,
                      int64_t process_identifier /* = 0 */) {
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
                   int64_t priority,
                   int64_t pid /* = 0 */,
                   int64_t process_identifier /* = 0 */) {
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


static rds::Link<Array, rds::Mode::Normal> g_signal_handlers;

// bitmask of signal number with custom handler
static uint32_t g_handlerMask = 0;

// Only the following signals are possible to handle in PHP.  Other signals are
// reserved for use by HHVM itself and its extensions.  The boolean value
// associated with a signal indicates whether the process should terminate if a
// signal handler isn't available.
#define SYNC_SIGNALS                            \
  SIG(SIGHUP, true)                             \
  SIG(SIGINT, true)                             \
  SIG(SIGALRM, true)                            \
  SIG(SIGTERM, true)                            \
  SIG(SIGUSR1, false)                           \
  SIG(SIGUSR2, false)                           \
  SIG(SIGWINCH, false)

// This runs in the sync-signal handler thread.
static void sig_handler_cli(int signo) {
  if (g_handlerMask & (1u << signo)) {
    RequestInfo::BroadcastSignal(signo);
  } else if (!RuntimeOption::ServerExecutionMode()) {
    auto const raise_and_exit = [] (int sig) {
      // Forward to the default handler.
      reset_sync_signals();
      raise(sig);
      pthread_exit(nullptr);
    };
    switch (signo) {
#define SIG(S, E) case S: if (E) {              \
        raise_and_exit(signo);                  \
        return;                                 \
      } else {                                  \
        break;                                  \
      }
      SYNC_SIGNALS
#undef SIG
     default:
      not_reached();
    }
  }
}

static void setup_sync_signals() {
#define SIG(S, ...) sync_signal(S, sig_handler_cli);
  SYNC_SIGNALS
#undef SIG
}

static InitFiniNode init(setup_sync_signals, InitFiniNode::When::ProcessInit);

static bool signalHandlersInited() {
  return g_signal_handlers.bound() && g_signal_handlers.isInit();
}

// This is called automatically in handle_request_surprise().  So if you are
// using HHVM, you don't really need to call this.  Even if you do, chances are
// that signal handlers are already called automatically before your invokation.
bool HHVM_FUNCTION(pcntl_signal_dispatch) {
  while (int signum = RID().getAndClearNextPendingSignal()) {
    if (signalHandlersInited() && g_signal_handlers->exists(signum)) {
      auto const handler = (*g_signal_handlers)[signum];
      if (handler.isNull()) { // SIG_IGN
        continue;
      }
      // We generally catch any exception a handler might throw, except
      // ExitException and ResourceExceededException.
      try {
        vm_call_user_func(handler, make_vec_array(signum));
      } catch (const ExitException& ) {
        throw;
      } catch (const ResourceExceededException& ) {
        throw;
      } catch (const Object& e) {
        std::string what;
        try {
          what = throwable_to_string(e.get()).c_str();
        } catch (...) {
          what = "(unable to call toString())";
        }
        std::string handlerName = "signal handler";
        try {
          auto const name = handler.toString().c_str();
          handlerName += folly::sformat(" '{}'", name);
        } catch (...) {
        }
        raise_warning("%s threw %s",
                      handlerName.c_str(), what.c_str());

      } catch (std::exception& e) {
        std::string handlerName = "signal handler";
        try {
          auto const name = handler.toString().c_str();
          handlerName += folly::sformat(" '{}'", name);
        } catch (...) {
        }
        raise_warning("%s threw an exception: %s",
                      handlerName.c_str(), e.what());
      } catch (...) {
        std::string handlerName = "signal handler";
        try {
          auto const name = handler.toString().c_str();
          handlerName += folly::sformat(" '{}'", name);
        } catch (...) {
        }
        raise_warning("%s threw and unknown exception",
                      handlerName.c_str());
      }
    } else if (!RuntimeOption::ServerExecutionMode()) {
      switch (signum) {
#define SIG(S, E) case S: if (E) _Exit(signum + 128); else break;
        SYNC_SIGNALS
      }
    }
  }
  return true;
}

bool HHVM_FUNCTION(pcntl_signal,
                   int64_t signo,
                   const Variant& handler,
                   bool restart_syscalls /* = true */) {
  /* Special long value case for SIG_DFL and SIG_IGN */
  if (handler.isInteger()) {
    int64_t handle = handler.toInt64();
    if (handle == (long)SIG_DFL) {
      // Uninstall previous handler for the signal, if any.
      if (signalHandlersInited()) g_signal_handlers->remove(signo);
      g_handlerMask &= ~(1u << signo);
      return true;
    }
    if (handle != (long)SIG_IGN) {
      raise_warning("Invalid value for handle argument specified");
      return false;
    }
    // A null signal hanlder indicates SIG_IGN.
    return HHVM_FN(pcntl_signal)(signo, init_null(), restart_syscalls);
  }
  if (!is_callable(handler) && !handler.isNull()) {
    raise_warning("%s is not a callable function name",
                  handler.toString().data());
    return false;
  }

  if (!g_signal_handlers.bound()) {
    g_signal_handlers.bind(rds::Mode::Normal, rds::LinkID{"SignalHandlers"});
  }
  if (!g_signal_handlers.isInit()) {
    g_signal_handlers.initWith(empty_dict_array());
  }
  g_signal_handlers->set(signo, handler);
  g_handlerMask |= (1u << signo);

  return true;
}

bool HHVM_FUNCTION(pcntl_sigprocmask,
                   int64_t how,
                   const Array& set,
                   Array& oldset) {
  auto const invalid_argument = [&] {
    raise_warning("pcntl_sigprocmask(): Invalid argument");
    return false;
  };

  if (how != SIG_BLOCK && how != SIG_UNBLOCK && how != SIG_SETMASK) {
    return invalid_argument();
  }

  if (RuntimeOption::ServerExecutionMode()) {
    // Forbid manipulation of signal masks in server mode.
    raise_warning("pcntl_sigprocmask() not supported in server mode");
    return false;
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

  oldset = Array::CreateVec();
  for (int signum = 1; signum < Process::kNSig; ++signum) {
    auto const result = sigismember(&coldset, signum);
    if (result == 1) {
      oldset.append(signum);
    } else if (result == -1) {
      // Invalid signal number.
      break;
    }
  }

  return true;
}

int64_t HHVM_FUNCTION(pcntl_wait,
                      int64_t& status,
                      int64_t options /* = 0 */) {
  int nstatus = -1;
  auto const child_id = LightProcess::waitpid(-1, &nstatus, options);
/*  if (options) {
    child_id = wait3(&nstatus, options, NULL);
  } else {
    child_id = wait(&nstatus);
  }*/
  status = nstatus;
  return child_id;
}

int64_t HHVM_FUNCTION(pcntl_waitpid,
                      int64_t pid,
                      int64_t& status,
                      int64_t options /* = 0 */) {
  int nstatus = -1;
  auto const child_id = LightProcess::waitpid(
    (pid_t)pid,
    &nstatus,
    options
  );
  status = nstatus;
  return child_id;
}

int64_t HHVM_FUNCTION(pcntl_wexitstatus,
                      int64_t status) {
  return WEXITSTATUS(status);
}

bool HHVM_FUNCTION(pcntl_wifexited,
                   int64_t status) {
  return WIFEXITED(status);
}

bool HHVM_FUNCTION(pcntl_wifsignaled,
                   int64_t status) {
  return WIFSIGNALED(status);
}

bool HHVM_FUNCTION(pcntl_wifstopped,
                   int64_t status) {
  return WIFSTOPPED(status);
}

int64_t HHVM_FUNCTION(pcntl_wstopsig,
                      int64_t status) {
  return WSTOPSIG(status);
}

int64_t HHVM_FUNCTION(pcntl_wtermsig,
                      int64_t status) {
  return WTERMSIG(status);
}

void ProcessExtension::moduleRegisterNative() {
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

  HHVM_RC_INT_SAME(SIGPWR);
  HHVM_RC_INT_SAME(SIGSTKFLT);
}

///////////////////////////////////////////////////////////////////////////////
}
