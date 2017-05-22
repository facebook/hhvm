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

#include "hphp/runtime/vm/hack-compiler.h"

#include "hphp/runtime/vm/as.h"
#include "hphp/runtime/vm/repo.h"
#include "hphp/runtime/vm/verifier/check.h"
#include "hphp/util/atomic-vector.h"
#include "hphp/util/light-process.h"
#include "hphp/util/logger.h"
#include "hphp/util/md5.h"

#include <condition_variable>
#include <mutex>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>

namespace HPHP {

namespace {

struct CompilerException : Exception {
  explicit CompilerException(const std::string& what) : Exception(what) {}
  template<class... A>
  explicit CompilerException(A&&... args)
    : Exception(folly::sformat(std::forward<A>(args)...))
  {}
};

[[noreturn]] void throwErrno(const char* what) {
  throw CompilerException("{}: {}", what, folly::errnoStr(errno));
}

struct HackCompiler {
  HackCompiler() = default;
  HackCompiler(HackCompiler&&) = default;
  HackCompiler& operator=(HackCompiler&&) = default;
  ~HackCompiler() { if (isRunning()) stop(); }

  std::unique_ptr<Unit> compile(
    const char* filename,
    const MD5& md5,
    folly::StringPiece code
  ) {
    if (!isRunning()) start();

    std::string prog;
    std::unique_ptr<Unit> u;
    try {
      writeProgram(filename, md5, code);
      prog = readProgram();
      auto ue = assemble_string(
        prog.data(),
        prog.length(),
        filename,
        md5,
        false /* swallow errors */
      );

      auto origin = *filename ? UnitOrigin::File : UnitOrigin::Eval;
      Repo::get().commitUnit(ue.get(), origin);

      u = ue->create();
      if (RuntimeOption::EvalHackCompilerVerify) {
        Verifier::checkUnit(u.get(), Verifier::kThrow);
      }

      return u;
    } catch (CompilerException& ex) {
      stop();
      if (RuntimeOption::EvalHackCompilerVerboseErrors) {
        Logger::FError("HackCompiler Error: {}", ex.what());
      }
      throw;
    } catch (std::runtime_error& ex) {
      if (RuntimeOption::EvalHackCompilerVerboseErrors) {
        auto const msg = folly::sformat(
          "{}\n"
          "========== HackCompiler Result ==========\n"
          "{}\n"
          "============ Assembler Result ===========\n"
          "{}\n",
          ex.what(),
          prog,
          u ? u->toString().c_str() : "No unit was produced."
        );
        Logger::FError("HackCompiler Generated a bad unit: {}", msg);

        // Throw the extended message to ensure the fataling unit contains the
        // additional context
        throw std::runtime_error(msg);
      }
      throw;
    }
  }

private:
  void start();
  void stop();
  bool isRunning() const { return m_pid != -1; }

  void writeProgram(const char* filename, MD5 md5, folly::StringPiece code);
  std::string readProgram() const;

  pid_t m_pid{-1};
  FILE* m_in{nullptr};
  FILE* m_out{nullptr};
};

int s_delegate = -1;
std::atomic<size_t> s_freeCount{0};
std::mutex s_compilerLock;
std::condition_variable s_compilerCv;
AtomicVector<HackCompiler*> s_compilers{0,nullptr};

std::pair<size_t, HackCompiler*> getCompiler() {
  std::unique_lock<std::mutex> l(s_compilerLock);

  s_compilerCv.wait(l, [] {
    return s_freeCount.load(std::memory_order_relaxed) != 0;
  });
  s_freeCount -= 1;

  for (size_t id = 0; id < s_compilers.size(); ++id) {
    auto ret = s_compilers.exchange(id, nullptr);
    if (ret) return std::make_pair(id, ret);
  }

  not_reached();
}

void releaseCompiler(size_t id, HackCompiler* ptr) {
  std::unique_lock<std::mutex> l(s_compilerLock);

  s_compilers[id].store(ptr, std::memory_order_relaxed);
  s_freeCount += 1;

  l.unlock();
  s_compilerCv.notify_one();
}

struct HackCompilerGuard final {
  HackCompilerGuard() { std::tie(m_index, m_ptr) = getCompiler(); }
  ~HackCompilerGuard() { releaseCompiler(m_index, m_ptr); }
  HackCompilerGuard(HackCompilerGuard&&) = delete;
  HackCompilerGuard& operator=(HackCompilerGuard&&) = delete;

  HackCompiler* operator->() const { return m_ptr; }
private:
  size_t m_index;
  HackCompiler* m_ptr;
};

////////////////////////////////////////////////////////////////////////////////

std::string readline(FILE* f) {
  char* line = nullptr;
  size_t mx = 0;
  ssize_t len = 0;
  SCOPE_EXIT { free(line); };

  if ((len = getline(&line, &mx, f)) < 0) {
    throwErrno("error reading line");
  }

  return len ? std::string(line, len - 1) : std::string();
}

std::string HackCompiler::readProgram() const {
  auto const start = readline(m_out);
  if (start.compare(0, 7, "ERROR: ", 7) == 0) {
    // We don't need to restart the pipe-- the compiler just wasn't able to
    // build this file...
    throw std::runtime_error(start.substr(7));
  }
  auto const len = folly::to<size_t>(start);

  std::string program(len, '\0');
  if (fread(&program[0], len, 1, m_out) != 1) {
    throwErrno("reading input program");
  }

  return program;
}

void HackCompiler::writeProgram(
  const char* filename,
  MD5 md5,
  folly::StringPiece code
) {
  auto const md5s = md5.toString();
  if (
    fprintf(m_in, "%s\n%s\n%lu\n", filename, md5s.c_str(), code.size()) == -1 ||
    fwrite(code.begin(), code.size(), 1, m_in) != 1
  ) {
    throwErrno("error writing input");
  }
  fflush(m_in);
}

struct UseLightDelegate final {
  UseLightDelegate()
    : m_prev(LightProcess::setThreadLocalAfdtOverride(s_delegate))
  {}
  ~UseLightDelegate() {
    LightProcess::setThreadLocalAfdtOverride(std::move(m_prev));
  }
private:
  std::unique_ptr<LightProcess> m_prev;
};

void HackCompiler::stop() {
  if (m_pid == -1) return;

  UseLightDelegate useDelegate;

  SCOPE_EXIT {
    if (m_in) fclose(m_in);
    if (m_out) fclose(m_out);
    m_in = m_out = nullptr;
    m_pid = -1;
  };

  int status, code;
  kill(m_pid, SIGTERM);
  if (LightProcess::waitpid(m_pid, &status, 0, 2) != m_pid) {
    Logger::FWarning("HackCompiler: unable to wait for compiler process");
    return;
  }

  if (WIFEXITED(status) && (code = WEXITSTATUS(status)) != 0) {
    Logger::FWarning("HackCompiler: exited with status code {}", code);
  } else if (WIFSIGNALED(status) && (code = WTERMSIG(status)) != SIGTERM) {
    Logger::FWarning(
      "HackCompiler: terminated by signal {}{}",
      code,
      WCOREDUMP(status) ? " (code dumped)" : ""
    );
  }
}

struct Pipe final {
  Pipe() {
    if (pipe2(fds, O_CLOEXEC) == -1) throwErrno("unable to open pipe");
  }
  ~Pipe() {
    if (fds[0] != -1) close(fds[0]);
    if (fds[1] != -1) close(fds[1]);
  }
  FILE* detach(const char* mode) {
    auto ret = fdopen(fds[*mode == 'r' ? 0 : 1], mode);
    if (!ret) throwErrno("unable to fdopen pipe");
    close(fds[*mode == 'r' ? 1 : 0]);
    fds[0] = fds[1] = -1;
    return ret;
  }
  int remoteIn() const { return fds[0]; }
  int remoteOut() const { return fds[1]; }
  int fds[2];
};

void HackCompiler::start() {
  if (m_pid != -1) return;

  UseLightDelegate useDelegate;

  // For now we dump stderr to /dev/null, we should probably start logging this
  // at some point.
  auto const err = open("/dev/null", O_WRONLY);
  SCOPE_EXIT { close(err); };

  Pipe in, out;
  std::vector<int> created = {in.remoteIn(), out.remoteOut(), err};
  std::vector<int> wanted = {STDIN_FILENO, STDOUT_FILENO, STDERR_FILENO};
  std::vector<std::string> env;

  m_pid = LightProcess::proc_open(
    RuntimeOption::EvalHackCompilerCommand.c_str(),
    created,
    wanted,
    nullptr /* cwd */,
    env
  );

  if (m_pid == -1) throwErrno("unable to start compiler");

  m_in = in.detach("w");
  m_out = out.detach("r");
}

////////////////////////////////////////////////////////////////////////////////
}

HackcMode hackc_mode() {
  if (!RuntimeOption::EvalHackCompilerDefault) {
    return HackcMode::kNever;
  }

  if (RuntimeOption::EvalHackCompilerCommand == "" ||
      !RuntimeOption::EvalHackCompilerWorkers) {
    return HackcMode::kNever;
  }

  if (RuntimeOption::EvalHackCompilerFallback) return HackcMode::kFallback;
  return HackcMode::kFatal;
}

void hackc_init() {
  if (hackc_mode() == HackcMode::kNever) return;

  auto const nworkers = RuntimeOption::EvalHackCompilerWorkers;
  s_freeCount.store(nworkers, std::memory_order_relaxed);
  UnsafeReinitEmptyAtomicVector(s_compilers, nworkers);
  for (int i = 0; i < nworkers; ++i) {
    s_compilers[i].store(new HackCompiler, std::memory_order_relaxed);
  }
  s_delegate = LightProcess::createDelegate();
}

void hackc_shutdown() {
  if (hackc_mode() == HackcMode::kNever) return;

  for (int i = 0; i < s_compilers.size(); ++i) {
    auto c = s_compilers.exchange(i, nullptr);
    delete c;
  }
  close(s_delegate);
}

HackcResult hackc_compile(
  const char* code,
  int len,
  const char* filename,
  const MD5& md5
) {
  always_assert(hackc_mode() != HackcMode::kNever);

  HackCompilerGuard compiler;
  std::stringstream err;

  size_t retry = 0;
  const size_t mx = std::max<size_t>(
    1, RuntimeOption::EvalHackCompilerMaxRetries + 1
  );
  while (retry++ < mx) {
    try {
      return compiler->compile(filename, md5, folly::StringPiece(code, len));
    } catch (CompilerException& ex) {
      // Swallow and retry, we return infra errors in bulk once the retry limit
      // is exceeded.
      err << ex.what();
      if (retry < mx) err << '\n';
    } catch(std::runtime_error& ex) {
      // Nontransient, don't bother with a retry.
      return ex.what();
    }
  }

  if (RuntimeOption::EvalHackCompilerVerboseErrors) {
    Logger::Error(
      "HackCompiler encountered too many communication errors, giving up."
    );
  }

  return err.str();
}

////////////////////////////////////////////////////////////////////////////////
}
