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

#include <vector>

#include "hphp/runtime/base/program-functions.h"
#include "hphp/runtime/base/emulate-zend.h"
#include "hphp/runtime/base/backtrace.h"
#include "hphp/runtime/base/execution-context.h"
#include "hphp/hhvm/process-init.h"
#include "hphp/compiler/compiler.h"
#include "hphp/hhbbc/hhbbc.h"

#include "hphp/util/embedded-data.h"
#include "hphp/util/embedded-vfs.h"
#include "hphp/util/logger.h"
#include "hphp/util/process.h"
#include "hphp/util/process-exec.h"
#include "hphp/util/stack-trace.h"
#include "hphp/util/struct-log.h"
#include "hphp/util/text-util.h"

#include <folly/Format.h>
#include <folly/Singleton.h>

#include <dlfcn.h>
#include <spawn.h>

/*
 * These are here to work around a gcc-5 lto bug. Without them,
 * certain symbols don't get defined, even though they're referenced,
 * but the build succeeds, and the references get set to nullptr (so
 * calls to vector<string>::~vector() end up as a call to 0.
 *
 * See t15096405
 */
std::vector<std::string> dummy_vec { "hello", "foo" };
std::set<std::string> dummy_set { "hello" };

int main(int argc, char** argv) {
  // Also for t15096405
  std::string (*ptr1)(std::string&&, const char*) = std::operator+;
  std::string (*ptr2)(const char*, std::string&&) = std::operator+;
  if (!argc) {
    return intptr_t(ptr1) + intptr_t(ptr2);
  }

  HPHP::Process::RecordArgv(argv);
  int len = strlen(argv[0]);
  if (len >= 4 && !strcmp(argv[0] + len - 4, "hphp")) {
    return HPHP::compiler_main(argc, argv);
  }
  if (argc > 1 && !strcmp(argv[1], "--hphp")) {
    argv[1] = argv[0];
    return HPHP::compiler_main(argc - 1, argv + 1);
  }

  if (len >= 5 && !strcmp(argv[0] + len - 5, "hhbbc")) {
    return HPHP::HHBBC::main(argc, argv);
  }
  if (argc > 1 && !strcmp(argv[1], "--hhbbc")) {
    argv[1] = "hhbbc";
    return HPHP::HHBBC::main(argc - 1, argv + 1);
  }

  HPHP::register_process_init();
  if (len >= 3 && !strcmp(argv[0] + len - 3, "php")) {
    return HPHP::emulate_zend(argc, argv);
  }
  if (argc > 1 && !strcmp(argv[1], "--php")) {
    argv[1] = argv[0];
    return HPHP::emulate_zend(argc - 1, argv + 1);
  }

  if (argc > 1 && !strcmp(argv[1], "--info")) {
    // prepare a command line of "<command> --php -r 'phpinfo();'"
    std::vector<char*> args;
    args.push_back(argv[0]);
    args.push_back("-r");
    args.push_back("phpinfo();");
    return HPHP::emulate_zend(args.size(), args.data());
  }

  HPHP::embedded_data data;
  if (!HPHP::get_embedded_data("repo", &data)) {
    return HPHP::execute_program(argc, argv);
  }
  std::string repo;
  HPHP::string_printf(repo, "%s:%u:%u",
                      data.m_filename.c_str(),
                      (unsigned)data.m_start, (unsigned)data.m_len);
  HPHP::sqlite3_embedded_initialize(nullptr, true);

  std::vector<char*> args;
  args.push_back(argv[0]);
  args.push_back("-vRepo.Authoritative=true");
  args.push_back("-vRepo.Local.Mode=r-");
  repo = "-vRepo.Local.Path=" + repo;
  args.push_back(const_cast<char*>(repo.c_str()));
  if (argc > 1) {
    args.insert(args.end(), argv + 1, argv + argc);
  }
  return HPHP::execute_program(args.size(), &args[0]);
}

#ifdef __linux__

static bool s_forkDisabledInMainProcess = false;
static bool s_forkLoggedInMainProcess = false;
static const pid_t s_mainPid = getpid();  // child process id will differ

void DisableFork() {
  s_forkDisabledInMainProcess = true;
}

void EnableForkLogging() {
  s_forkLoggedInMainProcess = true;
}

static void logForkAttempt(const char* func_name) {
  try {
    HPHP::StructuredLogEntry sample;
    HPHP::StackTrace st(HPHP::StackTrace::Force{});
    sample.setStr("function", func_name);
    sample.setStackTrace("cpp_stack", st);

    if (!HPHP::g_context.isNull() && !HPHP::tl_sweeping) {
      auto bt = HPHP::createBacktrace(
        HPHP::BacktraceArgs().ignoreArgs(true)
      );
      HPHP::addBacktraceToStructLog(bt, sample);
      auto transport = HPHP::g_context->getTransport();
      if (transport) {
        sample.setStr("url", transport->getUrl());
        sample.setStr("host_header", transport->getHeader("Host"));
      }
    }

    HPHP::StructuredLog::log("hhvm_forking", sample);
  } catch (...) {
    HPHP::Logger::Warning("Failed to log forking attempt");
  }
}

// Shared logic around all forking function wrappers:
// - resolve real implementation via dlsym
// - check if forking should be disabled
//
// returns address of real implementation for a given function, if forking
// should be allowed, and nullptr otherwise.
template<class FUNC_PTR>
static FUNC_PTR forking_wrapper(FUNC_PTR* real_func, const char* func_name) {
  if (*real_func == nullptr) {
    *real_func = (FUNC_PTR)dlsym(RTLD_NEXT, func_name);
  }

  if (s_forkLoggedInMainProcess &&
      getpid() == s_mainPid &&
      HPHP::StructuredLog::enabled()) {
    logForkAttempt(func_name);
  }

  if (!s_forkDisabledInMainProcess ||
      getpid() != s_mainPid ||
      HPHP::proc::EnableForkInDebuggerGuard::isForkEnabledInDebugger()) {
    if (*real_func) {
      return *real_func;
    } else {
      HPHP::Logger::Error(
          folly::sformat("cannot find an implementation of {}()", func_name)
      );
      assert(false);
    }
  }

  return nullptr;
}


extern "C" {
  // Note: in glibc, fork is a weak symbol aliasing to __fork.  Here we redefine
  // fork to intercept the call (only for Linux).  To make it works reliably,
  // this piece of code should always be statically linked into the binary.
  // Here we put it in the same file as main().  If you ever want to move it
  // into a separate library, you should make sure it is always statically
  // linked in the build system.

  pid_t fork() {
    static decltype(&fork) real_fork = nullptr;
    auto func = forking_wrapper(&real_fork, "fork");
    if (func != nullptr) {
      return func();
    }
    errno = ENOSYS;
    return -1;
  }

  int system(const char* line) {
    static decltype(&system) real_system = nullptr;
    auto func = forking_wrapper(&real_system, "system");
    if (func != nullptr) {
      return func(line);
    }
    errno = ENOSYS;
    return -1;
  }

  FILE* popen(const char* command, const char* type) {
    static decltype(&popen) real_popen = nullptr;
    auto func = forking_wrapper(&real_popen, "popen");
    if (func != nullptr) {
      return func(command, type);
    }
    errno = ENOSYS;
    return nullptr;
  }

  int posix_spawn(pid_t *pid, const char *path,
                  const posix_spawn_file_actions_t *file_actions,
                  const posix_spawnattr_t *attrp,
                  char *const argv[], char *const envp[]) {
    static decltype(&posix_spawn) real_posix_spawn = nullptr;
    auto func = forking_wrapper(&real_posix_spawn, "posix_spawn");
    if (func != nullptr) {
      return func(pid, path, file_actions, attrp, argv, envp);
    }
    errno = ENOSYS;
    return errno;
  }

  int posix_spawnp(pid_t *pid, const char *path,
                  const posix_spawn_file_actions_t *file_actions,
                  const posix_spawnattr_t *attrp,
                  char *const argv[], char *const envp[]) {
    static decltype(&posix_spawnp) real_posix_spawnp = nullptr;
    auto func = forking_wrapper(&real_posix_spawnp, "posix_spawnp");
    if (func != nullptr) {
      return func(pid, path, file_actions, attrp, argv, envp);
    }
    errno = ENOSYS;
    return errno;
  }
}

#endif
