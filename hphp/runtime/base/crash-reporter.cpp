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
#include "hphp/runtime/base/crash-reporter.h"
#include "hphp/util/stack-trace.h"
#include "hphp/util/light-process.h"
#include "hphp/util/logger.h"

#include "hphp/runtime/base/program-functions.h"
#include "hphp/runtime/base/execution-context.h"
#include "hphp/runtime/ext/ext_error.h"
#include "hphp/runtime/debugger/debugger.h"
#include "hphp/runtime/vm/ringbuffer-print.h"

namespace HPHP {

//////////////////////////////////////////////////////////////////////

bool IsCrashing = false;

static void bt_handler(int sig) {
  // In case we crash again in the signal hander or something
  signal(sig, SIG_DFL);

  IsCrashing = true;
  // Generating a stack dumps significant time, try to stop threads
  // from flushing bad data or generating more faults meanwhile
  if (sig==SIGQUIT || sig==SIGILL || sig==SIGSEGV || sig==SIGBUS) {
    LightProcess::Close();
    // leave running for SIGTERM SIGFPE SIGABRT
  }

  if (RuntimeOption::EvalDumpRingBufferOnCrash) {
    Trace::dumpRingBuffer(RuntimeOption::EvalDumpRingBufferOnCrash);
  }

  if (RuntimeOption::EvalSpinOnCrash) {
    char buf[128];
    snprintf(buf, 127,
             "Crashed. Waiting for debugger to attach pid %d\n", getpid());
    buf[127] = 0;
    write(STDERR_FILENO, buf, strlen(buf));
    for (;;) sleep(1);
  }

  // Turn on stack traces for coredumps
  StackTrace::Enabled = true;
  StackTraceNoHeap st;

  char pid[sizeof(Process::GetProcessId())*3+2]; // '-' and \0
  sprintf(pid,"%u",Process::GetProcessId());
  char tracefn[RuntimeOption::CoreDumpReportDirectory.length()
               + strlen("/stacktrace..log") + strlen(pid) + 1];
  sprintf(tracefn, "%s/stacktrace.%s.log",
          RuntimeOption::CoreDumpReportDirectory.c_str(), pid);

  int debuggerCount = RuntimeOption::EnableDebugger ?
    Eval::Debugger::CountConnectedProxy() : 0;

  st.log(strsignal(sig), tracefn, pid, kCompilerId, debuggerCount);

  int fd = ::open(tracefn, O_APPEND|O_WRONLY, S_IRUSR|S_IWUSR);
  if (fd >= 0) {
    if (!g_context.isNull()) {
      dprintf(fd, "\nPHP Stacktrace:\n\n%s",
              debug_string_backtrace(false).data());
    }
    ::close(fd);
  }

  if (!RuntimeOption::CoreDumpEmail.empty()) {
    char format [] = "cat %s | mail -s \"Stack Trace from %s\" '%s'";
    char cmdline[strlen(format)+strlen(tracefn)
                 +strlen(Process::GetAppName().c_str())
                 +strlen(RuntimeOption::CoreDumpEmail.c_str())+1];
    sprintf(cmdline, format, tracefn, Process::GetAppName().c_str(),
            RuntimeOption::CoreDumpEmail.c_str());
    Util::ssystem(cmdline);
  }

  // Calling all of these library functions in a signal handler
  // is completely undefined behavior, but we seem to get away with it.
  // Do it last just in case

  Logger::Error("Core dumped: %s", strsignal(sig));

  // Give the debugger a chance to do extra logging if there are any attached
  // debugger clients.
  Eval::Debugger::LogShutdown(Eval::Debugger::ShutdownKind::Abnormal);

  if (!g_context.isNull()) {
    // sync up gdb Dwarf info so that gdb can do a full backtrace
    // from the core file. Do this at the very end as syncing needs
    // to allocate memory for the ELF file.
    g_vmContext->syncGdbState();
  }

  // re-raise the signal and pass it to the default handler
  // to terminate the process.
  raise(sig);
}

void install_crash_reporter() {
  signal(SIGQUIT, bt_handler);
  signal(SIGILL,  bt_handler);
  signal(SIGFPE,  bt_handler);
  signal(SIGSEGV, bt_handler);
  signal(SIGBUS,  bt_handler);
  signal(SIGABRT, bt_handler);

  register_assert_fail_logger(&StackTraceNoHeap::AddExtraLogging);
}

//////////////////////////////////////////////////////////////////////

}
