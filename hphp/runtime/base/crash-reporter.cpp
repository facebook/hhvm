/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2016 Facebook, Inc. (http://www.facebook.com)     |
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

#include "hphp/runtime/base/crash-reporter.h"

#include "hphp/runtime/base/execution-context.h"
#include "hphp/runtime/base/file-util.h"
#include "hphp/runtime/base/program-functions.h"
#include "hphp/runtime/debugger/debugger.h"
#include "hphp/runtime/ext/std/ext_std_errorfunc.h"
#include "hphp/runtime/ext/xdebug/server.h"
#include "hphp/runtime/server/http-request-handler.h"
#include "hphp/runtime/vm/jit/mcgen.h"
#include "hphp/runtime/vm/jit/tc.h"
#include "hphp/runtime/vm/jit/trans-db.h"
#include "hphp/runtime/vm/ringbuffer-print.h"

#include "hphp/util/build-info.h"
#include "hphp/util/compilation-flags.h"
#include "hphp/util/logger.h"
#include "hphp/util/process.h"
#include "hphp/util/stack-trace.h"

#include <signal.h>

namespace HPHP {

//////////////////////////////////////////////////////////////////////

bool IsCrashing = false;

static const char* s_newBlacklist[] = {
  "_ZN4HPHP16StackTraceNoHeap",
  "_ZN5folly10symbolizer17getStackTraceSafe",
  "_ZN4HPHPL10bt_handlerEi",
  "killpg"
};

static void bt_handler(int sig) {
  if (IsCrashing) {
    // If we re-enter bt_handler while already crashing, just abort. This
    // includes if we hit the timeout set below.
    signal(SIGABRT, SIG_DFL);
    abort();
  }

  // TSAN instruments malloc() to make sure it can't be used in signal handlers.
  // Unfortunately, we use malloc() all over the place here.  This is bad, but
  // ends up working most of the time.  Just abort so TSAN won't infinite crash
  // loop.
  if (use_tsan) {
    signal(SIGABRT, SIG_DFL);
    abort();
  }

  // In case we crash again in the signal handler or something. Do this before
  // setting up the timeout to avoid potential races.
  IsCrashing = true;
  signal(sig, SIG_DFL);

  if (RuntimeOption::StackTraceTimeout > 0) {
    signal(SIGALRM, bt_handler);
    alarm(RuntimeOption::StackTraceTimeout);
  }

  // Make a stacktrace file to prove we were crashing. Do this before anything
  // else has a chance to deadlock us.
  int fd = ::open(RuntimeOption::StackTraceFilename.c_str(),
                  O_CREAT|O_TRUNC|O_WRONLY, S_IRUSR|S_IWUSR);

  if (RuntimeOption::EvalDumpRingBufferOnCrash) {
    Trace::dumpRingBuffer(RuntimeOption::EvalDumpRingBufferOnCrash, 0);
  }

  if (RuntimeOption::EvalSpinOnCrash) {
    char buf[128];
    snprintf(buf, 127,
             "Crashed. Waiting for debugger to attach pid %d\n", getpid());
    buf[127] = 0;
    write(STDERR_FILENO, buf, strlen(buf));
    for (;;) sleep(1);
  }

  if (fd < 0) {
    // Nothing to do if we can't write the file
    raise(sig);
    return;
  }

  // Turn on stack traces for coredumps
  StackTrace::Enabled = true;
  StackTrace::FunctionBlacklist = s_newBlacklist;
  StackTrace::FunctionBlacklistCount = 3;
  StackTraceNoHeap st;

  auto const debuggerCount = [&] {
    if (RuntimeOption::EnableDebugger) {
      return Eval::Debugger::CountConnectedProxy();
    }
    // We don't have a count of xdebug clients across all requests, so just
    // check the current request.
    if (XDebugServer::isAttached()) {
      return 1;
    }
    return 0;
  }();

  st.log(strsignal(sig), fd, compilerId().begin(), debuggerCount);

  // flush so if php crashes us we still have this output so far
  ::fsync(fd);

  if (fd >= 0) {
    // Don't attempt to determine function arguments in the PHP backtrace, as
    // that might involve re-entering the VM.
    if (!g_context.isNull()) {
      dprintf(fd, "\nPHP Stacktrace:\n\n%s",
              debug_string_backtrace(
                /*skip*/false,
                /*ignore_args*/true
              ).data());
    }
    ::close(fd);
  }

  if (jit::transdb::enabled()) {
    jit::tc::dump(true);
  }

  if (!RuntimeOption::CoreDumpEmail.empty()) {
    char format [] = "cat %s | mail -s \"Stack Trace from %s\" '%s'";
    char* cmdline = (char*)alloca(sizeof(char) *
                (strlen(format)
                 +RuntimeOption::StackTraceFilename.length()
                 +strlen(Process::GetAppName().c_str())
                 +strlen(RuntimeOption::CoreDumpEmail.c_str())+1));
    sprintf(cmdline, format, RuntimeOption::StackTraceFilename.c_str(),
            Process::GetAppName().c_str(),
            RuntimeOption::CoreDumpEmail.c_str());
    FileUtil::ssystem(cmdline);
  }

  // Calling all of these library functions in a signal handler
  // is completely undefined behavior, but we seem to get away with it.
  // Do it last just in case

  Logger::Error("Core dumped: %s", strsignal(sig));
  Logger::Error("Stack trace in %s", RuntimeOption::StackTraceFilename.c_str());

  // Flush whatever access logs are still pending
  Logger::FlushAll();
  HttpRequestHandler::GetAccessLog().flushAllWriters();

  // Give the debugger a chance to do extra logging if there are any attached
  // debugger clients.
  Eval::Debugger::LogShutdown(Eval::Debugger::ShutdownKind::Abnormal);

  if (!g_context.isNull()) {
    // sync up gdb Dwarf info so that gdb can do a full backtrace
    // from the core file. Do this at the very end as syncing needs
    // to allocate memory for the ELF file.
    g_context->syncGdbState();
  }

  // re-raise the signal and pass it to the default handler
  // to terminate the process.
  raise(sig);
}

void install_crash_reporter() {
#ifdef _MSC_VER
  signal(SIGILL,  bt_handler);
  signal(SIGFPE,  bt_handler);
  signal(SIGSEGV, bt_handler);
  signal(SIGABRT, bt_handler);
#else
  struct sigaction sa{};
  struct sigaction osa;
  sigemptyset(&sa.sa_mask);
  // By default signal handlers are run on the signaled thread's stack.
  // In case of stack overflow running the SIGSEGV signal handler on
  // the same stack leads to another SIGSEGV and crashes the program.
  // Use SA_ONSTACK, so alternate stack is used (only if configured via
  // sigaltstack).
  sa.sa_flags |= SA_ONSTACK;
  sa.sa_handler = &bt_handler;

  CHECK_ERR(sigaction(SIGQUIT, &sa, &osa));
  CHECK_ERR(sigaction(SIGBUS,  &sa, &osa));
  CHECK_ERR(sigaction(SIGILL,  &sa, &osa));
  CHECK_ERR(sigaction(SIGFPE,  &sa, &osa));
  CHECK_ERR(sigaction(SIGSEGV, &sa, &osa));
  CHECK_ERR(sigaction(SIGABRT, &sa, &osa));
#endif


  register_assert_fail_logger(&StackTraceNoHeap::AddExtraLogging);
}

//////////////////////////////////////////////////////////////////////

}
