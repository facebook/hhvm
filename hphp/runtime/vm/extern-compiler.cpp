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

#include "hphp/runtime/vm/extern-compiler.h"

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

struct CompilerOptions {
  bool verboseErrors;
  bool verifyUnit;
  uint64_t maxRetries;
  uint64_t workers;
  std::string command;
};

struct ExternCompiler {
  explicit ExternCompiler(const CompilerOptions& options)
    : m_options(options) {}
  ExternCompiler(ExternCompiler&&) = default;
  ExternCompiler& operator=(ExternCompiler&&) = default;
  ~ExternCompiler() { if (isRunning()) stop(); }

  std::unique_ptr<Unit> compile(
    const char* filename,
    const MD5& md5,
    folly::StringPiece code
  ) {
    if (RuntimeOption::EvalHackCompilerReset &&
        m_compilations > RuntimeOption::EvalHackCompilerReset) {
      stop();
    }

    if (!isRunning()) start();

    std::string prog;
    std::unique_ptr<Unit> u;
    try {
      m_compilations++;
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
      if (m_options.verifyUnit) {
        Verifier::checkUnit(u.get(), Verifier::kThrow);
      }

      return u;
    } catch (CompilerException& ex) {
      stop();
      if (m_options.verboseErrors) {
        Logger::FError("ExternCompiler Error: {}", ex.what());
      }
      throw;
    } catch (std::runtime_error& ex) {
      if (m_options.verboseErrors) {
        auto const msg = folly::sformat(
          "{}\n"
          "========== PHP Source ==========\n"
          "{}\n"
          "========== ExternCompiler Result ==========\n"
          "{}\n"
          "============ Assembler Result ===========\n"
          "{}\n",
          ex.what(),
          code,
          prog,
          u ? u->toString().c_str() : "No unit was produced."
        );
        Logger::FError("ExternCompiler Generated a bad unit: {}", msg);

        // Throw the extended message to ensure the fataling unit contains the
        // additional context
        throw std::runtime_error(msg);
      }
      throw;
    }
  }

  std::string getVersionString() {
    if (!isRunning()) start();
    return m_version;
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
  std::string m_version;

  unsigned m_compilations{0};
  const CompilerOptions& m_options;
};

int s_delegate = -1;
std::mutex s_delegateLock;

struct CompilerPool {
  explicit CompilerPool(CompilerOptions&& options)
    : m_options(options) {}

  std::pair<size_t, ExternCompiler*> getCompiler();
  void releaseCompiler(size_t id, ExternCompiler* ptr);
  void start();
  void shutdown();
  CompilerResult compile(const char* code, int len,
      const char* filename, const MD5& md5);
  std::string getVersionString() { return m_version; }

 private:
  std::atomic<size_t> m_freeCount{0};
  std::mutex m_compilerLock;
  std::condition_variable m_compilerCv;
  AtomicVector<ExternCompiler*> m_compilers{0, nullptr};
  CompilerOptions m_options;
  std::string m_version;
};

struct CompilerGuard final {
  explicit CompilerGuard(CompilerPool& pool)
    : m_pool(pool) {
    std::tie(m_index, m_ptr) = m_pool.getCompiler();
  }

  ~CompilerGuard() {
    m_pool.releaseCompiler(m_index, m_ptr);
  }

  CompilerGuard(CompilerGuard&&) = delete;
  CompilerGuard& operator=(CompilerGuard&&) = delete;

  ExternCompiler* operator->() const { return m_ptr; }

private:
  size_t m_index;
  ExternCompiler* m_ptr;
  CompilerPool& m_pool;
};

std::pair<size_t, ExternCompiler*> CompilerPool::getCompiler() {
  std::unique_lock<std::mutex> l(m_compilerLock);

  m_compilerCv.wait(l, [&] {
    return m_freeCount.load(std::memory_order_relaxed) != 0;
  });
  m_freeCount -= 1;

  for (size_t id = 0; id < m_compilers.size(); ++id) {
    auto ret = m_compilers.exchange(id, nullptr);
    if (ret) return std::make_pair(id, ret);
  }

  not_reached();
}

void CompilerPool::releaseCompiler(size_t id, ExternCompiler* ptr) {
  std::unique_lock<std::mutex> l(m_compilerLock);

  m_compilers[id].store(ptr, std::memory_order_relaxed);
  m_freeCount += 1;

  l.unlock();
  m_compilerCv.notify_one();
}

void CompilerPool::start() {
  auto const nworkers = m_options.workers;
  m_freeCount.store(nworkers, std::memory_order_relaxed);
  UnsafeReinitEmptyAtomicVector(m_compilers, nworkers);
  for (int i = 0; i < nworkers; ++i) {
    m_compilers[i].store(new ExternCompiler(m_options),
        std::memory_order_relaxed);
  }

  CompilerGuard g(*this);
  m_version = g->getVersionString();

}

void CompilerPool::shutdown() {
  for (int i = 0; i < m_compilers.size(); ++i) {
    auto c = m_compilers.exchange(i, nullptr);
    delete c;
  }
}

CompilerResult CompilerPool::compile(const char* code, int len,
    const char* filename, const MD5& md5) {
  CompilerGuard compiler(*this);
  std::stringstream err;

  size_t retry = 0;
  const size_t max = std::max<size_t>(
    1, m_options.maxRetries + 1
  );
  while (retry++ < max) {
    try {
      return compiler->compile(filename, md5, folly::StringPiece(code, len));
    } catch (CompilerException& ex) {
      // Swallow and retry, we return infra errors in bulk once the retry limit
      // is exceeded.
      err << ex.what();
      if (retry < max) err << '\n';
    } catch (std::runtime_error& ex) {
      // Nontransient, don't bother with a retry.
      return ex.what();
    }
  }

  if (m_options.verboseErrors) {
    Logger::Error(
      "ExternCompiler encountered too many communication errors, giving up."
    );
  }

  return err.str();
}

std::unique_ptr<CompilerPool> s_hackc_pool = nullptr;
std::unique_ptr<CompilerPool> s_php7_pool = nullptr;


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

std::string ExternCompiler::readProgram() const {
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

void ExternCompiler::writeProgram(
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
    : m_lock(s_delegateLock)
    , m_prev(LightProcess::setThreadLocalAfdtOverride(s_delegate))
  {}

  UseLightDelegate(UseLightDelegate&&) = delete;
  UseLightDelegate& operator=(UseLightDelegate&&) = delete;

  ~UseLightDelegate() {
    LightProcess::setThreadLocalAfdtOverride(std::move(m_prev));
  }
private:
  std::unique_lock<std::mutex> m_lock;
  std::unique_ptr<LightProcess> m_prev;
};

void ExternCompiler::stop() {
  if (m_pid == -1) return;

  SCOPE_EXIT {
    if (m_in) fclose(m_in);
    if (m_out) fclose(m_out);
    m_in = m_out = nullptr;
    m_pid = -1;
  };

  m_compilations = 0;

  int status, code;
  kill(m_pid, SIGTERM);

  {
    UseLightDelegate useDelegate;

    if (LightProcess::waitpid(m_pid, &status, 0, 2) != m_pid) {
      Logger::Warning("ExternCompiler: unable to wait for compiler process");
      return;
    }
  }

  if (WIFEXITED(status) && (code = WEXITSTATUS(status)) != 0) {
    Logger::FWarning("ExternCompiler: exited with status code {}", code);
  } else if (WIFSIGNALED(status) && (code = WTERMSIG(status)) != SIGTERM) {
    Logger::FWarning(
      "ExternCompiler: terminated by signal {}{}",
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

void ExternCompiler::start() {
  if (m_pid != -1) return;

  // For now we dump stderr to /dev/null, we should probably start logging this
  // at some point.
  auto const err = open("/dev/null", O_WRONLY);
  SCOPE_EXIT { close(err); };

  Pipe in, out;
  std::vector<int> created = {in.remoteIn(), out.remoteOut(), err};
  std::vector<int> wanted = {STDIN_FILENO, STDOUT_FILENO, STDERR_FILENO};
  std::vector<std::string> env;

  {
    UseLightDelegate useDelegate;

    m_pid = LightProcess::proc_open(
      m_options.command.c_str(),
      created,
      wanted,
      nullptr /* cwd */,
      env
    );
  }

  if (m_pid == -1) throwErrno("unable to start compiler");

  m_in = in.detach("w");
  m_out = out.detach("r");

  m_version = readline(m_out);
}

folly::Optional<CompilerOptions> hackcConfiguration() {
  if (hackc_mode() == HackcMode::kNever) {
    return folly::none;
  }

  return CompilerOptions{
    RuntimeOption::EvalHackCompilerVerboseErrors,
    RuntimeOption::EvalHackCompilerVerify,
    RuntimeOption::EvalHackCompilerMaxRetries,
    RuntimeOption::EvalHackCompilerWorkers,
    RuntimeOption::EvalHackCompilerCommand
  };
}

folly::Optional<CompilerOptions> php7Configuration() {
  if (!RuntimeOption::EvalPHP7CompilerEnabled) {
    return folly::none;
  }

  return CompilerOptions{
    true, // verboseErrors
    true, // verifyUnit
    0, // maxRetries
    1, // workers
    RuntimeOption::EvalPHP7CompilerCommand, // command
  };
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

void compilers_init() {
  auto php7Config = php7Configuration();
  auto hackConfig = hackcConfiguration();

  if (php7Config || hackConfig) {
    s_delegate = LightProcess::createDelegate();
  }

  if (hackConfig) {
    s_hackc_pool = std::make_unique<CompilerPool>(std::move(*hackConfig));
    s_hackc_pool->start();
  }

  if (php7Config) {
    s_php7_pool = std::make_unique<CompilerPool>(std::move(*php7Config));
    s_php7_pool->start();
  }
}

void compilers_set_user(const std::string& username) {
  if (s_delegate == -1) return;

  std::unique_lock<std::mutex> lock(s_delegateLock);
  LightProcess::ChangeUser(s_delegate, username);
}

void compilers_shutdown() {
  if (s_hackc_pool) {
    s_hackc_pool->shutdown();
  }

  if (s_php7_pool) {
    s_php7_pool->shutdown();
  }

  close(s_delegate);
}

CompilerResult hackc_compile(
  const char* code,
  int len,
  const char* filename,
  const MD5& md5
) {
  always_assert(s_hackc_pool);
  return s_hackc_pool->compile(code, len, filename, md5);
}

CompilerResult php7_compile(
  const char* code,
  int len,
  const char* filename,
  const MD5& md5
) {
  always_assert(s_php7_pool);
  return s_php7_pool->compile(code, len, filename, md5);
}

std::string hackc_version() {
  always_assert(s_hackc_pool);
  return s_hackc_pool->getVersionString();
}

std::string php7c_version() {
  always_assert(s_php7_pool);
  return s_php7_pool->getVersionString();
}


////////////////////////////////////////////////////////////////////////////////
}
