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

#include <cinttypes>
#include <condition_variable>
#include <mutex>
#include <signal.h>
#include <sstream>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <folly/DynamicConverter.h>
#include <folly/json.h>
#include <folly/FileUtil.h>

#include "hphp/runtime/base/ini-setting.h"
#include "hphp/runtime/base/zend-strtod.h"
#include "hphp/runtime/vm/repo.h"
#include "hphp/runtime/vm/unit-emitter.h"
#include "hphp/util/atomic-vector.h"
#include "hphp/util/compression.h"
#include "hphp/util/embedded-data.h"
#include "hphp/util/light-process.h"
#include "hphp/util/logger.h"
#include "hphp/util/match.h"
#include "hphp/util/md5.h"
#include "hphp/util/struct-log.h"
#include "hphp/util/timer.h"

#include <iostream>

namespace HPHP {

TRACE_SET_MOD(extern_compiler);

namespace {

bool createHackc(const std::string& path, const std::string& binary) {
  if (access(path.c_str(), R_OK|X_OK) == 0) {
    auto const fd = open(path.c_str(), O_RDONLY);
    if (fd != -1) {
      SCOPE_EXIT { close(fd); };
      std::string contents;
      if (folly::readFile(fd, contents) && contents == binary) return true;
    }
  }
  try {
    folly::writeFileAtomic(path, binary, 0755);
  } catch (std::system_error& ex) {
    return false;
  }
  return true;
}

THREAD_LOCAL(std::string, tl_extractPath);

std::mutex s_extractLock;
std::string s_extractPath;

folly::Optional<std::string> hackcExtractPath() {
  if (!RuntimeOption::EvalHackCompilerUseEmbedded) return folly::none;

  auto check = [] (const std::string& s) {
    return !s.empty() && access(s.data(), X_OK) == 0;
  };
  if (!tl_extractPath.isNull() && check(*tl_extractPath)) {
    return *tl_extractPath;
  }

  std::unique_lock<std::mutex> lock{s_extractLock};
  if (check(s_extractPath)) {
    *tl_extractPath.getCheck() = s_extractPath;
    return *tl_extractPath;
  }

  auto set = [&] (const std::string& s) {
    s_extractPath = s;
    *tl_extractPath.getCheck() = s;
    return s;
  };

  auto const trust = RuntimeOption::EvalHackCompilerTrustExtract;
  auto const location = RuntimeOption::EvalHackCompilerExtractPath;
  // As an optimization we can just choose to trust the extracted version
  // without reading it.
  if (trust && check(location)) return set(location);

  embedded_data desc;
  if (!get_embedded_data("hackc_binary", &desc)) {
    Logger::Error("Embedded hackc binary is missing");
    return folly::none;
  }
  auto const gz_binary = read_embedded_data(desc);
  int len = safe_cast<int>(gz_binary.size());

  auto const bin_str = gzdecode(gz_binary.data(), len);
  SCOPE_EXIT { free(bin_str); };
  if (!bin_str || !len) {
    Logger::Error("Embedded hackc binary could not be decompressed");
    return folly::none;
  }

  auto const binary = std::string(bin_str, len);
  if (createHackc(location, binary)) return set(location);

  int fd = -1;
  SCOPE_EXIT { if (fd != -1) close(fd); };

  auto fallback = RuntimeOption::EvalHackCompilerFallbackPath;
  if ((fd = mkstemp(&fallback[0])) == -1) {
    Logger::FError(
      "Unable to create temp file for hackc binary: {}", folly::errnoStr(errno)
    );
    return folly::none;
  }

  if (folly::writeFull(fd, binary.data(), binary.size()) == -1) {
    Logger::FError(
      "Failed to write extern hackc binary: {}", folly::errnoStr(errno)
    );
    return folly::none;
  }

  if (chmod(fallback.data(), 0755) != 0) {
    Logger::Error("Unable to mark hackc binary as writable");
    return folly::none;
  }

  return set(fallback);
}

std::string hackcCommand() {
  if (auto path = hackcExtractPath()) {
    return *path + " " + RuntimeOption::EvalHackCompilerArgs;
  }
  return RuntimeOption::EvalHackCompilerCommand;
}

struct CompileException : Exception {
  explicit CompileException(const std::string& what) : Exception(what) {}
  template<class... A>
  explicit CompileException(A&&... args)
    : Exception(folly::sformat(std::forward<A>(args)...))
  {}
};

struct CompilerFatal : std::runtime_error {
  explicit CompilerFatal(const std::string& str)
    : std::runtime_error(str)
  {}
};

[[noreturn]] void throwErrno(const char* what) {
  throw CompileException("{}: {}", what, folly::errnoStr(errno));
}

struct CompilerOptions {
  bool verboseErrors;
  uint64_t maxRetries;
  uint64_t workers;
  bool inheritConfig;
};

constexpr int kInvalidPid = -1;

struct ExternCompiler {
  explicit ExternCompiler(const CompilerOptions& options)
      : m_options(options)
    {}
  ExternCompiler(ExternCompiler&&) = default;
  ExternCompiler& operator=(ExternCompiler&&) = default;
  void detach_from_process () {
    // Called from forked processes. Resets inherited pid of compiler process to
    // prevent it being closed in case child process exits
    m_pid = kInvalidPid;
    // Don't call the destructor of the thread object.  The thread doesn't
    // belong to this child process, so we just silently make sure we don't
    // touch it.
    m_logStderrThread.release();
  }
  ~ExternCompiler() { stop(); }

  std::string extract_facts(
    const std::string& filename,
    folly::StringPiece code
  ) {
    if (!isRunning()) {
      start();
    }
    std::string facts;
    try {
      writeExtractFacts(filename, code);
      return readResult(nullptr /* structured log entry */);
    }
    catch (CompileException& ex) {
      stop();
      if (m_options.verboseErrors) {
        Logger::FError("ExternCompiler Error (facts): {}", ex.what());
      }
      throw;
    }
  }

  int64_t logTime(
    StructuredLogEntry& log,
    int64_t t,
    const char* name,
    bool first = false
  ) {
    if (!RuntimeOption::EvalLogExternCompilerPerf) return 0;
    int64_t current = Timer::GetCurrentTimeMicros();
    if (first) return current;
    int64_t diff = current - t;
    log.setInt(name, diff);
    FTRACE(2, "{} took {} us\n", name, diff);
    return current;
  }

  std::unique_ptr<UnitEmitter> compile(
    const char* filename,
    const MD5& md5,
    folly::StringPiece code,
    AsmCallbacks* callbacks
  ) {
    if (RuntimeOption::EvalHackCompilerReset &&
        m_compilations > RuntimeOption::EvalHackCompilerReset) {
      stop();
    }
    if (!isRunning()) {
      start();
    }

    std::string prog;
    std::unique_ptr<Unit> u;
    try {
      m_compilations++;
      StructuredLogEntry log;
      log.setStr("filename", filename);
      int64_t t = logTime(log, 0, nullptr, true);
      writeProgram(filename, md5, code);
      t = logTime(log, t, "send_source");
      prog = readResult(&log);
      t = logTime(log, t, "receive_hhas");
      auto ue = assemble_string(prog.data(),
                                prog.length(),
                                filename,
                                md5,
                                false /* swallow errors */,
                                callbacks
                              );
      logTime(log, t, "assemble_hhas");
      if (RuntimeOption::EvalLogExternCompilerPerf) {
        StructuredLog::log("hhvm_detailed_frontend_performance", log);
      }
      return ue;
    } catch (CompileException& ex) {
      stop();
      if (m_options.verboseErrors) {
        Logger::FError("ExternCompiler Error: {}", ex.what());
      }
      throw;
    } catch (CompilerFatal& ex) {
      if (m_options.verboseErrors) {
        Logger::FError("ExternCompiler Fatal: {}", ex.what());
      }
      throw;
    } catch (AssemblerError& ex) {
      if (m_options.verboseErrors) {
        auto const msg = folly::sformat(
          "{}\n"
          "========== PHP Source ==========\n"
          "{}\n"
          "========== ExternCompiler Result ==========\n"
          "{}\n",
          ex.what(),
          code,
          prog);
        Logger::FError("ExternCompiler Generated a bad unit: {}", msg);

        // Throw the extended message to ensure the fataling unit contains the
        // additional context
        throw AssemblerError(msg);
      }
      throw;
    } catch (std::runtime_error& ex) {
      if (m_options.verboseErrors) {
        Logger::FError("ExternCompiler Runtime Error: {}", ex.what());
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
  bool isRunning() const { return m_pid != kInvalidPid; }
  void stopLogStderrThread();

  void writeMessage(folly::dynamic& header, folly::StringPiece body);
  void writeConfigs();
  void writeProgram(const char* filename, MD5 md5, folly::StringPiece code);
  void writeExtractFacts(const std::string& filename, folly::StringPiece code);

  std::string readVersion() const;
  std::string readResult(StructuredLogEntry* log) const;

  pid_t m_pid{kInvalidPid};
  FILE* m_in{nullptr};
  FILE* m_out{nullptr};
  std::string m_version;

  FILE* m_err{nullptr};
  std::unique_ptr<std::thread> m_logStderrThread;

  unsigned m_compilations{0};
  const CompilerOptions& m_options;
};

struct CompilerGuard;

struct CompilerPool {
  explicit CompilerPool(CompilerOptions&& options)
    : m_options(options)
    , m_compilers(options.workers, nullptr)
  {}

  std::pair<size_t, ExternCompiler*> getCompiler();
  void releaseCompiler(size_t id, ExternCompiler* ptr);
  void start();
  void shutdown(bool detach_compilers);
  CompilerResult compile(const char* code,
                         int len,
                         const char* filename,
                         const MD5& md5,
                         AsmCallbacks* callbacks,
                         bool& internal_error);
  ParseFactsResult extract_facts(const CompilerGuard& compiler,
                                 const std::string& filename,
                                 const char* code,
                                 int len);
  std::string getVersionString() { return m_version; }
  std::pair<uint64_t, bool> getMaxRetriesAndVerbosity() const {
    return std::make_pair(m_options.maxRetries, m_options.verboseErrors);
  }
 private:
  CompilerOptions m_options;
  std::atomic<size_t> m_freeCount{0};
  std::mutex m_compilerLock;
  std::condition_variable m_compilerCv;
  AtomicVector<ExternCompiler*> m_compilers;
  std::string m_version;
};

struct CompilerGuard final: public FactsParser {
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
  for (int i = 0; i < nworkers; ++i) {
    m_compilers[i].store(new ExternCompiler(m_options),
        std::memory_order_relaxed);
  }

  CompilerGuard g(*this);
  m_version = g->getVersionString();
}

void CompilerPool::shutdown(bool detach_compilers) {
  for (int i = 0; i < m_compilers.size(); ++i) {
    if (auto c = m_compilers.exchange(i, nullptr)) {
      if (detach_compilers) {
        c->detach_from_process();
      }
      delete c;
    }
  }
}


template<typename F>
auto run_compiler(
  const CompilerGuard& compiler,
  int maxRetries,
  bool verboseErrors,
  F&& func,
  bool& internal_error) ->
  boost::variant<
    typename std::result_of<F&&(const CompilerGuard&)>::type,
    std::string
  >
{
  std::stringstream err;

  size_t retry = 0;
  const size_t max = std::max<size_t>(1, maxRetries + 1);
  while (retry++ < max) {
    try {
      internal_error = false;
      return func(compiler);
    } catch (AssemblerError& ex) {
      // Assembler rejected hhas generated by external compiler
      return ex.what();
    } catch (CompilerFatal& ex) {
      // ExternCompiler returned an error when building this unit
      return ex.what();
    } catch (CompileException& ex) {
      internal_error = true;
      // Swallow and retry, we return infra errors in bulk once the retry limit
      // is exceeded.
      err << ex.what();
      if (retry < max) err << '\n';
    } catch (std::runtime_error& ex) {
      internal_error = true;
      // Nontransient, don't bother with a retry.
      return ex.what();
    }
  }

  if (verboseErrors) {
    Logger::Error(
      "ExternCompiler encountered too many communication errors, giving up."
    );
  }
  return err.str();
}

ParseFactsResult extract_facts_worker(const CompilerGuard& compiler,
                               const std::string& filename,
                               const char* code,
                               int len,
                               int maxRetries,
                               bool verboseErrors
) {
  auto extract = [&](const CompilerGuard& c) {
    auto result = c->extract_facts(filename, folly::StringPiece(code, len));
    return FactsJSONString { result };
  };
  auto internal_error = false;
  return run_compiler(
    compiler,
    maxRetries,
    verboseErrors,
    extract,
    internal_error);
}

CompilerResult CompilerPool::compile(const char* code,
                                     int len,
                                     const char* filename,
                                     const MD5& md5,
                                     AsmCallbacks* callbacks,
                                     bool& internal_error
) {
  auto compile = [&](const CompilerGuard& c) {
    return c->compile(filename,
                      md5,
                      folly::StringPiece(code, len),
                      callbacks);
  };
  return run_compiler(
    CompilerGuard(*this),
    m_options.maxRetries,
    m_options.verboseErrors,
    compile,
    internal_error);
}

////////////////////////////////////////////////////////////////////////////////

std::string readline(FILE* f) {
  char* line = nullptr;
  size_t mx = 0;
  ssize_t len = 0;
  SCOPE_EXIT { free(line); };

  for (auto tries = 0; tries < 10; tries++) {
    if ((len = getline(&line, &mx, f)) >= 0) {
      break;
    }
    if (errno == EINTR) {
      // Signal. Maybe Xenon? Just try again within reason.
      ::clearerr(f);
      continue;
    }
    // Non-EINTR error.
    break;
  }

  if (len < 0) {
    throwErrno("error reading line");
  }

  return len ? std::string(line, len - 1) : std::string();
}

std::string ExternCompiler::readVersion() const {
  // Note the utter lack of error handling. We're really expecting the version
  // JSON to be the first thing we get from the compiler daemon, and that it has
  // a "version" field, and that the value at the field is indeed a string...
  const auto line = readline(m_out);
  return folly::parseJson(line).at("version").asString();
}

std::string ExternCompiler::readResult(StructuredLogEntry* log) const {
  const auto line = readline(m_out);
  const auto header = folly::parseJson(line);
  const std::string type = header.getDefault("type", "").asString();
  const std::size_t bytes = header.getDefault("bytes", 0).asInt();

  const auto logResult = [&] (auto name, auto t) {
    if (log != nullptr) log->setInt(name, t);
    FTRACE(2, "{} took {} us\n", name, t);
  };

  if (RuntimeOption::EvalLogExternCompilerPerf) {
    if (auto parsing_time = header.get_ptr("parsing_time")) {
      logResult("extern_parsing", parsing_time->asInt());
    }
    if (auto codegen_time = header.get_ptr("codegen_time")) {
      logResult("extern_codegen", codegen_time->asInt());
    }
    if (auto printing_time = header.get_ptr("printing_time")) {
      logResult("extern_printing", printing_time->asInt());
    }
  }

  if (type == "success") {
    std::string program(bytes, '\0');
    if (bytes != 0 && fread(&program[0], bytes, 1, m_out) != 1) {
      throwErrno("reading input program");
    }
    return program;
  } else if (type == "error") {
    // We don't need to restart the pipe -- the compiler just wasn't able to
    // build this file...
    throw CompilerFatal(
      header.getDefault("error", "[no 'error' field]").asString());
  } else {
    throw CompilerFatal("unknown message type, " + type);
  }

  not_reached();
}

void ExternCompiler::stopLogStderrThread() {
  SCOPE_EXIT { m_err = nullptr; };
  if (m_err) {
    fclose(m_err);   // need to unblock getline()
  }
  if (m_logStderrThread && m_logStderrThread->joinable()) {
    m_logStderrThread->join();
  }
}

void ExternCompiler::writeMessage(
  folly::dynamic& header,
  folly::StringPiece body
) {
  const auto bytes = body.size();
  header["bytes"] = bytes;
  const auto jsonHeader = folly::toJson(header);
  if (
    fprintf(m_in, "%s\n", jsonHeader.data()) == -1 ||
    (bytes > 0 && fwrite(body.begin(), bytes, 1, m_in) != 1)
  ) {
    throwErrno("error writing message");
  }
  fflush(m_in);
}

struct ConfigBuilder {
  template<typename T>
  ConfigBuilder& addField(folly::StringPiece key, const T& data) {
    if (!m_config.isObject()) {
      m_config = folly::dynamic::object();
    }

    m_config[key] = folly::dynamic::object(
      "global_value", folly::toDynamic(data));

    return *this;
  }

  std::string toString() const {
    return m_config.isNull() ? "" : folly::toJson(m_config);
  }

 private:
  folly::dynamic m_config{nullptr};
};

void ExternCompiler::writeConfigs() {
  static const std::string boundConfig = [this] () -> std::string {
    if (m_options.inheritConfig) {
      // necessary to initialize zend-strtod, which is used to serialize
      // boundConfig to JSON (!)
      zend_get_bigint_data();
      return IniSetting::GetAllAsJSON();
    }
    return "";
  }();

  // Some configs, like IncludeRoots, can't easily be Config::Bind(ed), so here
  // we create a place to dump miscellaneous config values HackC might want.
  static const std::string miscConfig = [this] () -> std::string {
    if (m_options.inheritConfig) {
      return ConfigBuilder()
        .addField("hhvm.include_roots", RuntimeOption::IncludeRoots)
        .toString();
    }
    return "";
  }();

  folly::dynamic header = folly::dynamic::object("type", "config");
  writeMessage(header, boundConfig);
  writeMessage(header, miscConfig);
}

void ExternCompiler::writeProgram(
  const char* filename,
  MD5 md5,
  folly::StringPiece code
) {
  folly::dynamic header = folly::dynamic::object
    ("type", "code")
    ("md5", md5.toString())
    ("file", filename)
    ("is_systemlib", !SystemLib::s_inited);
  writeMessage(header, code);
}

void ExternCompiler::writeExtractFacts(
  const std::string& filename,
  folly::StringPiece code
) {
  folly::dynamic header = folly::dynamic::object
    ("type", "facts")
    ("file", filename);
  writeMessage(header, code);
}

struct CompilerManager final {
  int get_delegate() { return m_delegate; }
  std::mutex& get_delegate_lock() { return m_delegateLock; }
  void set_username(const std::string& username) { m_username = username; }
  void ensure_started();
  void shutdown();
  void detach_after_fork();
  bool hackc_enabled() { return (bool)m_hackc_pool; }
  CompilerPool& get_hackc_pool();
private:
  void stop(bool detach_compilers);
  int m_delegate{kInvalidPid};
  std::mutex m_delegateLock;

  std::unique_ptr<CompilerPool> m_hackc_pool;

  std::atomic<bool> m_started{false};
  std::mutex m_compilers_start_lock;
  folly::Optional<std::string> m_username;
} s_manager;

struct UseLightDelegate final {
  UseLightDelegate()
    : m_lock(s_manager.get_delegate_lock())
    , m_prev(LightProcess::setThreadLocalAfdtOverride(s_manager.get_delegate()))
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
  // This is super-gross: it's possible we're in a forked child -- but fork()
  // doesn't -- can't -- copy over threads, so m_logStderrThread is rubbish --
  // but joinable() in the child. When the child's ~ExternCompiler() destructor
  // is called, it will call m_logStderrThread's destructor, terminating a
  // joinable but unjoined thread, which causes a panic. We really shouldn't be
  // mixing threads with forking, but we should just about get away with it
  // here.
  SCOPE_EXIT {
    stopLogStderrThread();
  };

  if (m_pid == kInvalidPid) return;

  SCOPE_EXIT {
    // We must close err before in, otherwise there's a race:
    // - hackc tries to read from stdin
    // - stdin is closed
    // - hackc writes an error to stderr
    // - the error handler thread in HHVM spews out the error
    // This makes the tests unrunnable, but probably doesn't have a practical
    // effect on real usage other than log spew on process shutdown.
    if (m_err) fclose(m_err);
    if (m_in) fclose(m_in);
    if (m_out) fclose(m_out);
    m_err = m_in = m_out = nullptr;
    m_pid = kInvalidPid;
  };

  m_compilations = 0;

  auto ret = kill(m_pid, SIGTERM);
  if (ret == -1) {
    Logger::FWarning(
      "ExternCompiler: kill failed: {}, {}",
      errno,
      folly::errnoStr(errno).c_str());
  }

  int status, code;
  {
    UseLightDelegate useDelegate;
    ret = LightProcess::waitpid(m_pid, &status, 0, 2);
    if (ret != m_pid) {
      Logger::FWarning(
        "ExternCompiler: unable to wait for compiler process, return code {},"
        "errno: {}, {}",
        ret,
        errno,
        folly::errnoStr(errno).c_str());
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
  if (m_pid != kInvalidPid) return;

  Pipe in, out, err;
  std::vector<int> created = {in.remoteIn(), out.remoteOut(), err.remoteOut()};
  std::vector<int> wanted = {STDIN_FILENO, STDOUT_FILENO, STDERR_FILENO};
  std::vector<std::string> env;

  auto const command = hackcCommand();

  if (!command.empty()) {
    UseLightDelegate useDelegate;

    m_pid = LightProcess::proc_open(
      command.c_str(),
      created,
      wanted,
      nullptr /* cwd */,
      env
    );
  } else {
    Logger::Error("Unable to get external command");
    throw BadCompilerException("Unable to get external command");
  }

  if (m_pid == kInvalidPid) {
    const auto msg = folly::to<std::string>(
      "Unable to start external compiler with command: ", command.c_str());
    Logger::Error(msg);
    throw BadCompilerException(msg);
  }

  m_in = in.detach("w");
  m_out = out.detach("r");
  m_err = err.detach("r");

  m_logStderrThread = std::make_unique<std::thread>([&]() {
      int ret = 0;
      auto pid = m_pid;
      try {
        pollfd pfd[] = {{fileno(m_err), POLLIN, 0}};
        while ((ret = poll(pfd, 1, -1)) != -1) {
          if (ret == 0) continue;
          if (pfd[0].revents & (POLLHUP | POLLNVAL | POLLERR)) {
            throw std::runtime_error("hangup");
          }
          if (pfd[0].revents) {
            const auto line = readline(m_err);
            Logger::FError("[external compiler {}]: {}", pid, line);
          }
        }
      } catch (const std::exception& exc) {
        // The stderr output messes with expected test output, which presumably
        // come from non-server runs.
        if (RuntimeOption::ServerMode) {
          Logger::FVerbose(
            "Ceasing to log stderr from external compiler ({}): {}",
            pid,
            exc.what());
        }
      }
    });

  // Here we expect the very first communication from the external compiler
  // process to be a single line of JSON representing the compiler version.
  try {
    m_version = readVersion();
  } catch (const CompileException& exc) {
    throw BadCompilerException(
      "Couldn't read version message from external compiler");
  }

  // For...reasons...the external compiler process misses the first line of
  // output on the pipe, so we open communications with a single newline.
  if (fprintf(m_in, "\n") == -1) {
    throw BadCompilerException("Couldn't write initial newline");
  }
  fflush(m_in);

  writeConfigs();
}

folly::Optional<CompilerOptions> hackcConfiguration() {
  if (hackc_mode() == HackcMode::kNever) {
    return folly::none;
  }

  return CompilerOptions{
    RuntimeOption::EvalHackCompilerVerboseErrors,
    RuntimeOption::EvalHackCompilerMaxRetries,
    RuntimeOption::EvalHackCompilerWorkers,
    RuntimeOption::EvalHackCompilerInheritConfig,
  };
}

CompilerResult hackc_compile(
  const char* code,
  int len,
  const char* filename,
  const MD5& md5,
  AsmCallbacks* callbacks,
  bool& internal_error
) {
  return s_manager.get_hackc_pool().compile(
    code, len, filename,md5, callbacks, internal_error
  );
}

////////////////////////////////////////////////////////////////////////////////
}

HackcMode hackc_mode() {
  if (!RuntimeOption::EvalHackCompilerDefault) {
    return HackcMode::kNever;
  }

  if (hackcCommand() == "" || !RuntimeOption::EvalHackCompilerWorkers) {
    return HackcMode::kNever;
  }

  if (RuntimeOption::EvalHackCompilerFallback) return HackcMode::kFallback;

  return HackcMode::kFatal;
}

void CompilerManager::ensure_started() {
  if (m_started.load(std::memory_order_acquire)) {
    return;
  }
  std::unique_lock<std::mutex> l(m_compilers_start_lock);
  if (m_started.load(std::memory_order_relaxed)) {
    return;
  }
  auto hackConfig = hackcConfiguration();

  if (hackConfig) {
    m_delegate = LightProcess::createDelegate();
  }

  if (hackConfig) {
    m_hackc_pool = std::make_unique<CompilerPool>(std::move(*hackConfig));
  }

  if (m_delegate != kInvalidPid && m_username) {
    std::unique_lock<std::mutex> lock(m_delegateLock);
    LightProcess::ChangeUser(m_delegate, m_username.value());
  }

  if (m_hackc_pool) m_hackc_pool->start();

  m_started.store(true, std::memory_order_release);
}

void CompilerManager::stop(bool detach_compilers) {
  if (m_hackc_pool) {
    m_hackc_pool->shutdown(detach_compilers);
    m_hackc_pool = nullptr;
  }

  close(m_delegate);
  m_delegate = kInvalidPid;
  m_started.store(false, std::memory_order_relaxed);
}

void CompilerManager::shutdown() {
  stop(false);
}

void CompilerManager::detach_after_fork() {
  stop(true);
}

CompilerPool& CompilerManager::get_hackc_pool() {
  ensure_started();
  return *m_hackc_pool;
}

void compilers_start() {
  s_manager.ensure_started();
#if FOLLY_HAVE_PTHREAD_ATFORK
  pthread_atfork(
    nullptr /* prepare */,
    nullptr /* parent */,
    compilers_detach_after_fork /* child */
  );
#endif
}

void compilers_set_user(const std::string& username) {
  s_manager.set_username(username);
}

void compilers_shutdown() {
  s_manager.shutdown();
  std::unique_lock<std::mutex> lock{s_extractLock};
  if (!s_extractPath.empty() &&
      s_extractPath != RuntimeOption::EvalHackCompilerExtractPath) {
    unlink(s_extractPath.data());
  }
}

void compilers_detach_after_fork() {
  s_manager.detach_after_fork();
  std::unique_lock<std::mutex> lock{s_extractLock};
  if (!s_extractPath.empty() &&
      s_extractPath != RuntimeOption::EvalHackCompilerExtractPath) {
    s_extractPath.clear();
  }
}

std::unique_ptr<FactsParser> acquire_facts_parser() {
  return std::make_unique<CompilerGuard>(s_manager.get_hackc_pool());
}

ParseFactsResult extract_facts(
  const FactsParser& facts_parser,
  const std::string& filename,
  const char* code,
  int len
) {
  size_t maxRetries;
  bool verboseErrors;
  std::tie(maxRetries, verboseErrors) =
    s_manager.get_hackc_pool().getMaxRetriesAndVerbosity();
  return extract_facts_worker(
    dynamic_cast<const CompilerGuard&>(facts_parser),
    filename,
    code,
    len,
    maxRetries,
    verboseErrors);
}

std::string hackc_version() {
  return s_manager.get_hackc_pool().getVersionString();
}

bool startsWith(const char* big, const char* small) {
  return strncmp(big, small, strlen(small)) == 0;
}

bool isFileHack(const char* code, size_t codeLen) {
  // if the file starts with a shebang
  if (codeLen > 2 && strncmp(code, "#!", 2) == 0) {
    // reset code to the next char after the shebang line
    const char* loc = reinterpret_cast<const char*>(
        memchr(code, '\n', codeLen));
    if (!loc) {
      return false;
    }

    ptrdiff_t offset = loc - code;
    code = loc + 1;
    codeLen -= offset + 1;
  }

  return codeLen > strlen("<?hh") && startsWith(code, "<?hh");
}

std::unique_ptr<UnitCompiler> UnitCompiler::create(const char* code,
                                                   int codeLen,
                                                   const char* filename,
                                                   const MD5& md5
) {
  s_manager.ensure_started();
  if (SystemLib::s_inited || RuntimeOption::EvalUseExternCompilerForSystemLib) {
    auto const hcMode = hackc_mode();
    if (hcMode != HackcMode::kNever && s_manager.hackc_enabled()) {
      return std::make_unique<HackcUnitCompiler>(
        code, codeLen, filename, md5, hcMode);
    }
  }

  return nullptr;
}

std::unique_ptr<UnitEmitter> HackcUnitCompiler::compile(
  AsmCallbacks* callbacks) const {
  bool ice = false;
  auto res = hackc_compile(m_code,
                           m_codeLen,
                           m_filename,
                           m_md5,
                           callbacks,
                           ice);
  std::unique_ptr<UnitEmitter> unitEmitter;
  match<void>(
    res,
    [&] (std::unique_ptr<UnitEmitter>& ue) {
      unitEmitter = std::move(ue);
    },
    [&] (std::string& err) {
      if (m_hackcMode != HackcMode::kFallback) {
        unitEmitter = createFatalUnit(
          makeStaticString(m_filename),
          m_md5,
          FatalOp::Runtime,
          makeStaticString(err));
      }
    }
  );

  if (unitEmitter) unitEmitter->m_ICE = ice;
  return unitEmitter;
}

////////////////////////////////////////////////////////////////////////////////
}
