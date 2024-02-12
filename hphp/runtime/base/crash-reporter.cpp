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

#include "hphp/runtime/base/configs/debugger.h"
#include "hphp/runtime/base/configs/jit.h"
#include "hphp/runtime/base/execution-context.h"
#include "hphp/runtime/base/file-util.h"
#include "hphp/runtime/base/program-functions.h"
#include "hphp/runtime/debugger/debugger.h"
#include "hphp/runtime/ext/std/ext_std_errorfunc.h"
#include "hphp/runtime/ext/vsdebug/debugger.h"
#include "hphp/runtime/ext/vsdebug/ext_vsdebug.h"
#include "hphp/runtime/server/http-request-handler.h"
#include "hphp/runtime/vm/jit/cg-meta.h"
#include "hphp/runtime/vm/jit/fixup.h"
#include "hphp/runtime/vm/jit/mcgen.h"
#include "hphp/runtime/vm/jit/prof-data-serialize.h"
#include "hphp/runtime/vm/jit/tc.h"
#include "hphp/runtime/vm/jit/trans-db.h"
#include "hphp/runtime/vm/ringbuffer-print.h"

#include "hphp/util/build-info.h"
#include "hphp/util/compilation-flags.h"
#include "hphp/util/logger.h"
#include "hphp/util/process.h"
#include "hphp/util/stack-trace.h"

#include <fcntl.h>
#include <signal.h>
#ifdef __x86_64__
#include <ucontext.h>
#endif

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
  CreateCppStackFallback,
  ReportHeader,
  ReportAssertDetail,
  ReportTrap,
  DumpTreadmill,
  ReportCppStack,
  ReportPhpStack,
  ReportApproximatePhpStack,
  DumpTransDB,
  DumpProfileData,
  SendEmail,
  Log,
  Done,
};

static CrashReportStage s_crash_report_stage;

// These ranges contain debug information in the core that can be extracted
// using the helper tool `hphp/tools/extract_from_core.sh`.  They start at
// kDebugAddr and each start is page size aligned.  These static variables
// should be kept in sync with that utility.
static uintptr_t s_jitprof_start;
static uintptr_t s_jitprof_end;
static uintptr_t s_stacktrace_start;
static uintptr_t s_stacktrace_end;

static const char* s_newIgnorelist[] = {
  "_ZN4HPHP16StackTraceNoHeap",
  "_ZN5folly10symbolizer17getStackTraceSafe",
  "_ZN4HPHP10bt_handlerEi",
  "_ZN5folly6fibers12_GLOBAL__N_120sigsegvSignalHandlerEiP9siginfo_tPv",
  "killpg"
};

static void bt_timeout_handler(int sig) {
  signal(SIGABRT, SIG_DFL);
  abort();
}

void bt_handler(int sigin, siginfo_t* info, void* args) {
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
  static Optional<StackTraceNoHeap> st;
  static void* sig_addr = info ? info->si_addr : nullptr;

  static FILE* stacktraceFile = nullptr;

#ifdef __x86_64__
  static uintptr_t sig_rip = ((ucontext_t*) args)->uc_mcontext.gregs[REG_RIP];
#endif

  switch (s_crash_report_stage) {
    case CrashReportStage::DumpRingBuffer:
      s_crash_report_stage = CrashReportStage::SpinOnCrash;

      if (RuntimeOption::EvalDumpRingBufferOnCrash) {
        Trace::dumpRingBuffer(RuntimeOption::EvalDumpRingBufferOnCrash, 0);
      }
      [[fallthrough]];
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
      [[fallthrough]];
    case CrashReportStage::CheckFD:
      s_crash_report_stage = CrashReportStage::CreateCppStack;
      if (fd < 0) {
        // Nothing to do if we can't write the file
        signal(sig, SIG_DFL);
        raise(sig);
        return;
      }
      [[fallthrough]];
    case CrashReportStage::CreateCppStack:
      s_crash_report_stage = CrashReportStage::CreateCppStackFallback;

      // Turn on stack traces for coredumps
      StackTrace::Enabled = true;
      StackTrace::FunctionIgnorelist = s_newIgnorelist;
      StackTrace::FunctionIgnorelistCount = 5;
      st.emplace();
      [[fallthrough]];
    case CrashReportStage::CreateCppStackFallback:
      s_crash_report_stage = CrashReportStage::ReportHeader;
      // If the attempt to create the cpp stack failed above, try it
      // again, using a simpler fallback.
      if (!st.has_value()) st.emplace(true, true);
      [[fallthrough]];
    case CrashReportStage::ReportHeader:
      s_crash_report_stage = CrashReportStage::ReportAssertDetail;
      {
        auto const debuggerCount = [&] {
          if (Cfg::Debugger::EnableHphpd) {
            return Eval::Debugger::CountConnectedProxy();
          }

          HPHP::VSDEBUG::Debugger* vspDebugger =
          HPHP::VSDEBUG::VSDebugExtension::getDebugger();
          if (vspDebugger != nullptr && vspDebugger->clientConnected()) {
            return 1;
          }

          return 0;
        }();
        StackTraceNoHeap::log(strsignal(sig), fd, compilerId().begin(), debuggerCount);
      }
      [[fallthrough]];
    case CrashReportStage::ReportAssertDetail:
    {
      // Don't point s_crash_report_stage to the next one. We want to
      // spin here until we're done
      std::string msg;
      while (AssertDetailImpl::readAndRemove(msg)) {
        write(fd, msg.c_str(), msg.size());
        ::fsync(fd);
      }
      [[fallthrough]];
    }
    case CrashReportStage::ReportTrap:
      s_crash_report_stage = CrashReportStage::DumpTreadmill;

      if (sig == SIGILL && sig_addr) {
        auto reason = jit::getTrapReason((jit::CTCA)sig_addr);
        if (reason) {
          dprintf(fd, "Detected jit::trap at %p: from %s:%d\n\n",
                  sig_addr, reason->file, reason->line);
        }
      }
      [[fallthrough]];
    case CrashReportStage::DumpTreadmill:
      s_crash_report_stage = CrashReportStage::ReportCppStack;
      ::fsync(fd);
      dprintf(
        fd,
        "\n-------------------------------"
        "Treadmill Information"
        "----------------------------\n%s\n",
        Treadmill::dumpTreadmillInfo(true).data()
      );
      [[fallthrough]];
    case CrashReportStage::ReportCppStack:
      s_crash_report_stage = CrashReportStage::ReportPhpStack;

      // flush so if C++ stack walking crashes, we still have this output so far
      ::fsync(fd);
      if (st.has_value()) {
        st->printStackTrace(fd);
      } else {
        folly::StringPiece msg{"(Unable to obtain cpp stack trace)"};
        write(fd, msg.begin(), msg.size());
      }
      [[fallthrough]];
    case CrashReportStage::ReportPhpStack:
      // flush so if php stack-walking crashes, we still have this output so far
      ::fsync(fd);

      s_crash_report_stage = CrashReportStage::ReportApproximatePhpStack;
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

      s_crash_report_stage = CrashReportStage::DumpTransDB;

      [[fallthrough]];
    case CrashReportStage::ReportApproximatePhpStack:
      if (s_crash_report_stage == CrashReportStage::ReportApproximatePhpStack) {
        // We were unable to find a fixup for the backtrace; scan the stack to
        // find a VM frame, fake the vmpc, and attempt again.
        s_crash_report_stage = CrashReportStage::DumpTransDB;

        if (auto const ar = jit::findVMFrameForDebug()) {
          auto const frame = BTFrame::regular(ar, kInvalidOffset);
          auto const addr = [&] () -> jit::CTCA {
            if (sig != SIGILL && sig != SIGSEGV) return (jit::CTCA) sig_addr;
#if defined(__x86_64__)
            return (jit::CTCA) sig_rip;
#else
            return (jit::CTCA) 0;
#endif
          }();
          auto const trace = createCrashBacktrace(frame, (jit::CTCA) addr);

          dprintf(fd, "\nPHP Stacktrace:\n\n%s",
                  stringify_backtrace(trace, true).data());
          ::close(fd);
        }
      }

      [[fallthrough]];
    case CrashReportStage::DumpTransDB:
      s_crash_report_stage = CrashReportStage::DumpProfileData;

      if (jit::transdb::enabled() && Cfg::Jit::Enabled) {
        jit::tc::dump(true);
      }
      [[fallthrough]];
    case CrashReportStage::DumpProfileData: {
      s_crash_report_stage = CrashReportStage::SendEmail;
      auto frontier = kDebugAddr;
      auto const mapFileIn = [&frontier](const std::string& filename,
                                         uintptr_t& start,
                                         uintptr_t& end) {

        auto file = fopen(filename.c_str(), "r");
        size_t size = 0;
        start = end = frontier;
        if (file) {
          fseek(file, 0, SEEK_END);
          size = ftell(file);
          end = start + size;
          fseek(file, 0, SEEK_SET);
          if (size > 0) {
            mmap(reinterpret_cast<void*>(start),
                 size, PROT_READ | PROT_WRITE,
                 MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
            fread(reinterpret_cast<void*>(start), 1, size, file);
          }
          fclose(file);
        }
        frontier = (end + s_pageSize - 1) & -s_pageSize;
      };
      if (auto const serdesFile = jit::getFilenameDeserialized()) {
        // Copy JitSerdesFile to fixed address (the debug range), so that they
        // get included in the coredump.
        mapFileIn(*serdesFile, s_jitprof_start, s_jitprof_end);
      }
      auto const& stacktraceFile = RuntimeOption::StackTraceFilename;
      mapFileIn(stacktraceFile, s_stacktrace_start, s_stacktrace_end);
    }
      [[fallthrough]];
    case CrashReportStage::SendEmail:
      s_crash_report_stage = CrashReportStage::Log;

      if (!RuntimeOption::CoreDumpEmail.empty()) {
        char format [] = "cat %s | mail -s \"Stack Trace from %s\" '%s'";
        size_t cmdline_size = sizeof(char) *(strlen(format)
             +RuntimeOption::StackTraceFilename.length()
             +strlen(Process::GetAppName().c_str())
             +strlen(RuntimeOption::CoreDumpEmail.c_str())+1);
        char* cmdline = (char*)alloca(cmdline_size);
        snprintf(cmdline, cmdline_size, format, RuntimeOption::StackTraceFilename.c_str(),
                Process::GetAppName().c_str(),
                RuntimeOption::CoreDumpEmail.c_str());
        FileUtil::ssystem(cmdline);
      }
      [[fallthrough]];
    case CrashReportStage::Log:
      s_crash_report_stage = CrashReportStage::Done;

      // Calling all of these library functions in a signal handler
      // is completely undefined behavior, but we seem to get away with it.
      // Do it last just in case

      Logger::FError("Core dumped: {}", strsignal(sig));
      Logger::FError("Stack trace in {}", RuntimeOption::StackTraceFilename);

      if (RO::EvalDumpStacktraceToErrorLogOnCrash) {
        stacktraceFile = fopen(RuntimeOption::StackTraceFilename.c_str(), "r");
        if (stacktraceFile) {
          char line[256];
          while (fgets(line, sizeof(line), stacktraceFile)) {
            // Strip the newline, or just truncate the line if it's larger than
            // 256 characters.
            line[MIN(strcspn(line, "\n"), 255)] = 0;
            Logger::FError("{}", line);
          }
        }
      }

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
      [[fallthrough]];
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
  if (!RO::EvalSanitizeReqHeap) {
    // SIGSEGV is handled by the request heap sanitizer when it is enabled.
    CHECK_ERR(sigaction(SIGSEGV, &sa, &osa));
  }
  CHECK_ERR(sigaction(SIGABRT, &sa, &osa));
}

//////////////////////////////////////////////////////////////////////

}
