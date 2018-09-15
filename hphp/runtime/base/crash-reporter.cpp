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

#include "hphp/runtime/base/crash-reporter.h"

#include "hphp/runtime/base/execution-context.h"
#include "hphp/runtime/base/file-util.h"
#include "hphp/runtime/base/program-functions.h"
#include "hphp/runtime/debugger/debugger.h"
#include "hphp/runtime/ext/std/ext_std_errorfunc.h"
#include "hphp/runtime/ext/vsdebug/debugger.h"
#include "hphp/runtime/ext/vsdebug/ext_vsdebug.h"
#include "hphp/runtime/server/http-request-handler.h"
#include "hphp/runtime/vm/jit/cg-meta.h"
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

#include <folly/portability/Fcntl.h>
#include <folly/portability/Stdio.h>

#include <atomic>

namespace HPHP {

//////////////////////////////////////////////////////////////////////

std::atomic<pid_t> CrashingThread{};

enum class CrashReportStage {
  DumpRingBuffer,
  SpinOnCrash,
  CheckFD,
  CreateCppStack,
  ReportHeader,
  ReportAssertDetail,
  ReportTrap,
  ReportCppStack,
  ReportPhpStack,
  DumpTransDB,
  SendEmail,
  Log,
  Done,
};

static CrashReportStage s_crash_report_stage;

static const char* s_newBlacklist[] = {
  "_ZN4HPHP16StackTraceNoHeap",
  "_ZN5folly10symbolizer17getStackTraceSafe",
  "_ZN4HPHPL10bt_handlerEi",
  "killpg"
};

static void bt_timeout_handler(int sig) {
  signal(SIGABRT, SIG_DFL);
  abort();
}

static void bt_handler(int sigin, siginfo_t* info, void*) {
  auto tid = Process::GetThreadPid();
  pid_t expected{};
  if (CrashingThread.compare_exchange_strong(expected, tid,
                                             std::memory_order_relaxed)) {
    // We're the first crashing thread, go ahead and report everything.
    if (RuntimeOption::StackTraceTimeout > 0) {
      signal(SIGALRM, bt_timeout_handler);
      alarm(RuntimeOption::StackTraceTimeout);
    }
  } else {
    if (expected != tid) {
      // Another thread is already dealing with its own crash. Spin
      // until its done.
      while (true) {
        sleep(RuntimeOption::StackTraceTimeout + 1);
      }
    }
  }

  // Make a stacktrace file to prove we were crashing. Do this before anything
  // else has a chance to deadlock us.

  // If we get here, we're running single threaded, but we might still
  // re-enter if we crash while generating the report - so make fd
  // static so its available to all re-entered calls.
  static auto const fd = ::open(RuntimeOption::StackTraceFilename.c_str(),
                                O_CREAT|O_TRUNC|O_WRONLY, S_IRUSR|S_IWUSR);

  static int sig = sigin;
  static folly::Optional<StackTraceNoHeap> st;
  static void* sig_addr = info ? info->si_addr : nullptr;

  switch (s_crash_report_stage) {
    case CrashReportStage::DumpRingBuffer:
      s_crash_report_stage = CrashReportStage::SpinOnCrash;

      if (RuntimeOption::EvalDumpRingBufferOnCrash) {
        Trace::dumpRingBuffer(RuntimeOption::EvalDumpRingBufferOnCrash, 0);
      }
      // fall through
    case CrashReportStage::SpinOnCrash:
      s_crash_report_stage = CrashReportStage::CheckFD;
      if (RuntimeOption::EvalSpinOnCrash) {
        char buf[128];
        snprintf(buf, 127,
                 "Crashed. Waiting for debugger to attach pid %d\n", getpid());
        buf[127] = 0;
        write(STDERR_FILENO, buf, strlen(buf));
        for (;;) sleep(1);
      }
      // fall through
    case CrashReportStage::CheckFD:
      s_crash_report_stage = CrashReportStage::CreateCppStack;
      if (fd < 0) {
        // Nothing to do if we can't write the file
        signal(sig, SIG_DFL);
        raise(sig);
        return;
      }
      // fall through
    case CrashReportStage::CreateCppStack:
      s_crash_report_stage = CrashReportStage::ReportHeader;

      // Turn on stack traces for coredumps
      StackTrace::Enabled = true;
      StackTrace::FunctionBlacklist = s_newBlacklist;
      StackTrace::FunctionBlacklistCount = 3;
      st.emplace();
      // fall through
    case CrashReportStage::ReportHeader:
      s_crash_report_stage = CrashReportStage::ReportAssertDetail;
      {
        auto const debuggerCount = [&] {
          if (RuntimeOption::EnableHphpdDebugger) {
            return Eval::Debugger::CountConnectedProxy();
          }

          HPHP::VSDEBUG::Debugger* vspDebugger =
          HPHP::VSDEBUG::VSDebugExtension::getDebugger();
          if (vspDebugger != nullptr && vspDebugger->clientConnected()) {
            return 1;
          }

          return 0;
        }();
        st->log(strsignal(sig), fd, compilerId().begin(), debuggerCount);
      }
      // fall through
    case CrashReportStage::ReportAssertDetail:
    {
      // Don't point s_crash_report_stage to the next one. We want to
      // spin here until we're done
      std::string msg;
      while (AssertDetailImpl::readAndRemove(msg)) {
        write(fd, msg.c_str(), msg.size());
        ::fsync(fd);
      }
      // fall through
    }
    case CrashReportStage::ReportTrap:
      s_crash_report_stage = CrashReportStage::ReportCppStack;

      if (sig == SIGILL && sig_addr) {
        auto reason = jit::getTrapReason((jit::CTCA)sig_addr);
        if (reason) {
          dprintf(fd, "Detected jit::trap at %p: from %s:%d\n\n",
                  sig_addr, reason->file, reason->line);
        }
      }
      // fall through
    case CrashReportStage::ReportCppStack:
      s_crash_report_stage = CrashReportStage::ReportPhpStack;

      // flush so if C++ stack walking crashes, we still have this output so far
      ::fsync(fd);
      st->printStackTrace(fd);
      // fall through
    case CrashReportStage::ReportPhpStack:
      s_crash_report_stage = CrashReportStage::DumpTransDB;

      // flush so if php stack-walking crashes, we still have this output so far
      ::fsync(fd);

      // Don't attempt to determine function arguments in the PHP backtrace, as
      // that might involve re-entering the VM.
      if (!g_context.isNull() && !tl_sweeping) {
        dprintf(fd, "\nPHP Stacktrace:\n\n%s",
                debug_string_backtrace(
                    /*skip*/false,
                    /*ignore_args*/true
                ).data());
      }
      ::close(fd);
      // fall through
    case CrashReportStage::DumpTransDB:
      s_crash_report_stage = CrashReportStage::SendEmail;

      if (jit::transdb::enabled()) {
        jit::tc::dump(true);
      }
      // fall through
    case CrashReportStage::SendEmail:
      s_crash_report_stage = CrashReportStage::Log;

      if (!RuntimeOption::CoreDumpEmail.empty()) {
        char format [] = "cat %s | mail -s \"Stack Trace from %s\" '%s'";
        char* cmdline = (char*)alloca(
            sizeof(char) *
            (strlen(format)
             +RuntimeOption::StackTraceFilename.length()
             +strlen(Process::GetAppName().c_str())
             +strlen(RuntimeOption::CoreDumpEmail.c_str())+1));
        sprintf(cmdline, format, RuntimeOption::StackTraceFilename.c_str(),
                Process::GetAppName().c_str(),
                RuntimeOption::CoreDumpEmail.c_str());
        FileUtil::ssystem(cmdline);
      }
      // fall through
    case CrashReportStage::Log:
      s_crash_report_stage = CrashReportStage::Done;

      // Calling all of these library functions in a signal handler
      // is completely undefined behavior, but we seem to get away with it.
      // Do it last just in case

      Logger::FError("Core dumped: {}", strsignal(sig));
      Logger::FError("Stack trace in {}", RuntimeOption::StackTraceFilename);

      // Flush whatever access logs are still pending
      Logger::FlushAll();
      HttpRequestHandler::GetAccessLog().flushAllWriters();

      // Give the debugger a chance to do extra logging if there are
      // any attached debugger clients.
      Eval::Debugger::LogShutdown(Eval::Debugger::ShutdownKind::Abnormal);

      if (!g_context.isNull()) {
        // sync up gdb Dwarf info so that gdb can do a full backtrace
        // from the core file. Do this at the very end as syncing needs
        // to allocate memory for the ELF file.
        g_context->syncGdbState();
      }
      // fall through
    case CrashReportStage::Done:
      // re-raise the signal and pass it to the default handler
      // to terminate the process.
      signal(sig, SIG_DFL);
      raise(sig);
  }
}

void install_crash_reporter() {
  // TSAN instruments malloc() to make sure it can't be used in signal
  // handlers.  Unfortunately, we use malloc() all over the place in
  // bt_handler.  Don't install our handler when running with tsan.
  if (use_tsan) return;

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
  sa.sa_flags |= SA_ONSTACK | SA_SIGINFO | SA_NODEFER;
  sa.sa_sigaction = &bt_handler;

  CHECK_ERR(sigaction(SIGQUIT, &sa, &osa));
  CHECK_ERR(sigaction(SIGBUS,  &sa, &osa));
  CHECK_ERR(sigaction(SIGILL,  &sa, &osa));
  CHECK_ERR(sigaction(SIGFPE,  &sa, &osa));
  CHECK_ERR(sigaction(SIGSEGV, &sa, &osa));
  CHECK_ERR(sigaction(SIGABRT, &sa, &osa));
#endif
}

//////////////////////////////////////////////////////////////////////

}
