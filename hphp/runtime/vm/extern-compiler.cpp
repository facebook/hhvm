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
#include <string>
#include <sys/types.h>
#include <sys/wait.h>

#include <folly/compression/Zstd.h>
#include <folly/DynamicConverter.h>
#include <folly/json.h>
#include <folly/FileUtil.h>
#include <folly/system/ThreadName.h>

#include "hphp/hack/src/facts/rust_facts_ffi.h"
#include "hphp/hack/src/hhbc/compile_ffi.h"
#include "hphp/hack/src/hhbc/compile_ffi_types.h"
#include "hphp/hack/src/parser/positioned_full_trivia_parser_ffi.h"
#include "hphp/hack/src/parser/positioned_full_trivia_parser_ffi_types.h"
#include "hphp/runtime/base/file-stream-wrapper.h"
#include "hphp/runtime/base/ini-setting.h"
#include "hphp/runtime/base/stream-wrapper-registry.h"
#include "hphp/runtime/vm/native.h"
#include "hphp/runtime/vm/repo.h"
#include "hphp/runtime/vm/unit-emitter.h"
#include "hphp/util/atomic-vector.h"
#include "hphp/util/embedded-data.h"
#include "hphp/util/gzip.h"
#include "hphp/util/hackc-log.h"
#include "hphp/util/light-process.h"
#include "hphp/util/logger.h"
#include "hphp/util/match.h"
#include "hphp/util/sha1.h"
#include "hphp/util/struct-log.h"
#include "hphp/util/timer.h"
#include "hphp/zend/zend-strtod.h"

#include <iostream>

namespace HPHP {

TRACE_SET_MOD(extern_compiler);

UnitEmitterCacheHook g_unit_emitter_cache_hook = nullptr;
static std::string s_bound_config;
static std::string s_misc_config;

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
    FTRACE(3, "writing {} atomically\n", path);
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
    if (s.empty()) return false;
    FTRACE(3, "checking file '{}'\n", s);
    return ::access(s.data(), X_OK) == 0;
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
  auto const binary = [&]() -> std::string {
    auto const cbinary = read_embedded_data(desc);
    auto const bin_size = cbinary.size();
    tracing::Block _{
      "compile-unit-uncompress",
      [&] {
        return tracing::Props{}
          .add("binary_size", bin_size);
      }
    };
    auto const codec = folly::io::zstd::getCodec(folly::io::zstd::Options{1});
    try {
      return codec->uncompress(cbinary);
    } catch (std::runtime_error&) {
      Logger::Error("Embedded hackc binary could not be zstd decompressed");
    }
    FTRACE(3, "attempting gzip uncompress\n");
    auto len = safe_cast<int>(cbinary.size());
    auto const bin_str = gzdecode(cbinary.data(), len);
    SCOPE_EXIT { if (bin_str) free(bin_str); };
    if (!bin_str || !len) {
      Logger::Error("Embedded hackc binary could not be gz decompressed");
      return {};
    }
    return std::string{bin_str, safe_cast<size_t>(len)};
  }();
  if (binary.empty()) return {};
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
  std::string log_stats = RuntimeOption::EvalLogHackcMemStats ?
    " --enable-logging-stats" : "";

  if (auto path = hackcExtractPath()) {
    return folly::to<std::string>(*path, " ", RuntimeOption::EvalHackCompilerArgs, log_stats);
  }
  return RuntimeOption::EvalHackCompilerCommand + log_stats;
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

struct Pipe final {
  explicit Pipe(bool nonblocking = false) {
    int flags = O_CLOEXEC;
    if (nonblocking) flags |= O_NONBLOCK;

    if (pipe2(fds, flags) == -1) throwErrno("unable to open pipe");
  }
  ~Pipe() {
    close();
  }

  FILE* detach(const char* mode) {
    auto ret = fdopen(fds[*mode == 'r' ? 0 : 1], mode);
    if (!ret) throwErrno("unable to fdopen pipe");
    ::close(fds[*mode == 'r' ? 1 : 0]);
    fds[0] = fds[1] = -1;
    return ret;
  }
  int remoteIn() const { return fds[0]; }
  int remoteOut() const { return fds[1]; }
  void close() {
    if (fds[0] != -1) {
      ::close(fds[0]);
      fds[0] = -1;
    }
    if (fds[1] != -1) {
      ::close(fds[1]);
      fds[1] = -1;
    }
  }
  int fds[2];
};

std::string readline(FILE* f);

struct LogThread {
  LogThread(int pid, FILE* file)
    : m_pid(pid), m_file(file), m_signalPipe(true)
  {
    m_thread = std::make_unique<std::thread>([this]() {
        folly::setThreadName("extern-compiler-log");

        int ret = 0;
        auto pid = m_pid;
        auto signalFD = m_signalPipe.remoteIn();
        try {
          pollfd pfd[] = {
            {fileno(m_file), POLLIN, 0},
            {signalFD, POLLIN, 0 },
          };
          while ((ret = poll(pfd, 2, -1)) != -1) {
            if (ret == 0) continue;
            if (pfd[0].revents & (POLLHUP | POLLNVAL | POLLERR) ||
                pfd[1].revents) {
              throw std::runtime_error("hangup");
            }
            if (pfd[0].revents) {
              const auto line = readline(m_file);
              Logger::FError("[external compiler {}]: {}", pid, line);
            }
          }
        } catch (const std::exception& exc) {
          // The stderr output messes with expected test output, which
          // presumably come from non-server runs.
          if (RuntimeOption::ServerMode) {
            Logger::FVerbose(
              "Ceasing to log stderr from external compiler ({}): {}",
              pid,
              exc.what());
          }
        }
      });
  }
  ~LogThread() {
      stop();
  }

  void stop() {
    if (m_thread && m_thread->joinable()) {
      SCOPE_EXIT {
        m_signalPipe.close();
      };

      // Signal thread to exit.
      write(m_signalPipe.remoteOut(), "X", 1);
      m_thread->join();
    }

    m_pid = kInvalidPid;
    m_file = nullptr;
  }

private:
  int m_pid;
  FILE* m_file;
  Pipe m_signalPipe;
  std::unique_ptr<std::thread> m_thread;
};

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
    folly::StringPiece code,
    const RepoOptions& options
  ) {
    if (!isRunning()) {
      start();
    }
    std::string facts;
    try {
      writeExtractFacts(filename, code, options);
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

  std::string ffp_parse_file(
    const std::string& filename,
    folly::StringPiece code,
    const RepoOptions& options
  ) {
    if (!isRunning()) {
      start();
    }
    try {
      writeParseFile(filename, code, options);
      return readResult(nullptr /* structured log entry */);
    }
    catch (CompileException& ex) {
      stop();
      if (m_options.verboseErrors) {
        Logger::FError("ExternCompiler Error (parse): {}", ex.what());
      }
      throw;
    }
  }

  // To avoid ambiguity with std::string in variant
  struct Hhas { std::string s; };

  Hhas compile(
    const char* filename,
    const SHA1& sha1,
    folly::StringPiece code,
    bool forDebuggerEval,
    const RepoOptions& options
  ) {
    try {
      tracing::Block _{
        "extern-compiler-invoke",
        [&] {
          return tracing::Props{}
            .add("filename", filename)
            .add("code_size", code.size());
        }
      };
      if (!isRunning()) start();
      writeProgram(filename, sha1, code, forDebuggerEval, options);
      StructuredLogEntry log;
      auto const hhas = readResult(&log);
      if (RuntimeOption::EvalLogExternCompilerPerf) {
        log.setStr("filename", filename);
        StructuredLog::log("hhvm_detailed_frontend_performance", log);
      }
      return Hhas { hhas };
    } catch (const CompileException& ex) {
      stop();
      if (m_options.verboseErrors) {
        Logger::FError("ExternCompiler Error: {}", ex.what());
      }
      throw;
    } catch (const CompilerFatal&) {
      // this catch is here so we don't fall into the std::runtime_error one
      throw;
    } catch (const FatalErrorException&) {
      // we want these to propagate out of the compiler
      throw;
    } catch (const std::runtime_error& ex) {
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
  void writeProgram(const char* filename, SHA1 sha1, folly::StringPiece code,
                    bool forDebuggerEval, const RepoOptions& options);
  void writeExtractFacts(const std::string& filename, folly::StringPiece code,
                         const RepoOptions&);
  void writeParseFile(const std::string& filename, folly::StringPiece code,
                      const RepoOptions&);

  std::string readVersion() const;
  std::string readResult(StructuredLogEntry* log) const;

  pid_t m_pid{kInvalidPid};
  FILE* m_in{nullptr};
  FILE* m_out{nullptr};
  std::string m_version;

  FILE* m_err{nullptr};
  std::unique_ptr<LogThread> m_logStderrThread;

  const CompilerOptions& m_options;
};

struct CompilerGuard;

struct CompilerPool {
  explicit CompilerPool(CompilerOptions&& options)
    : m_options(std::move(options)) {}

  std::unique_ptr<ExternCompiler> getCompiler();
  void releaseCompiler(std::unique_ptr<ExternCompiler> ptr);
  void start();
  void shutdown(bool detach_compilers);
  CompilerResult compile(const char* code,
                         int len,
                         const char* filename,
                         const SHA1& sha1,
                         const Native::FuncTable& nativeFuncs,
                         bool forDebuggerEval,
                         bool& internal_error,
                         const RepoOptions& options,
                         CompileAbortMode mode);
  FfpResult parse(std::string name, const char* code, int len,
                  const RepoOptions&);
  ParseFactsResult extract_facts(const CompilerGuard& compiler,
                                 const std::string& filename,
                                 const char* code,
                                 int len,
                                 const RepoOptions& options);
  std::string getVersionString() { return m_version; }
  std::pair<uint64_t, bool> getMaxRetriesAndVerbosity() const {
    return std::make_pair(m_options.maxRetries, m_options.verboseErrors);
  }
 private:
  CompilerOptions m_options;
  std::mutex m_compilerLock;
  std::condition_variable m_compilerCv;
  std::vector<std::unique_ptr<ExternCompiler>> m_compilers;
  std::string m_version;
};

struct CompilerGuard final: public FactsParser {
  explicit CompilerGuard(CompilerPool& pool)
    : m_ptr{pool.getCompiler()}
    , m_pool{pool} {}

  ~CompilerGuard() {
    m_pool.releaseCompiler(std::move(m_ptr));
  }

  CompilerGuard(const CompilerGuard&) = delete;
  CompilerGuard(CompilerGuard&&) = delete;
  CompilerGuard& operator=(const CompilerGuard&) = delete;
  CompilerGuard& operator=(CompilerGuard&&) = delete;

  ExternCompiler* operator->() const { return m_ptr.get(); }

private:
  std::unique_ptr<ExternCompiler> m_ptr;
  CompilerPool& m_pool;
};

std::unique_ptr<ExternCompiler> CompilerPool::getCompiler() {
  // If this is zero, we'll wait forever....
  assertx(m_options.workers > 0);

  std::unique_lock<std::mutex> l{m_compilerLock};
  m_compilerCv.wait(l, [&] { return !m_compilers.empty(); });
  auto compiler = std::move(m_compilers.back());
  m_compilers.pop_back();
  return compiler;
}

void CompilerPool::releaseCompiler(std::unique_ptr<ExternCompiler> c) {
  std::unique_lock<std::mutex> l(m_compilerLock);
  m_compilers.emplace_back(std::move(c));
  m_compilerCv.notify_one();
}

void CompilerPool::start() {
  always_assert(m_options.workers > 0);

  {
    std::unique_lock<std::mutex> l{m_compilerLock};
    for (size_t i = 0; i < m_options.workers; ++i) {
      m_compilers.emplace_back(std::make_unique<ExternCompiler>(m_options));
    }
  }

  CompilerGuard g{*this};
  m_version = g->getVersionString();
}

void CompilerPool::shutdown(bool detach) {
  std::unique_lock<std::mutex> l{m_compilerLock};
  if (detach) {
    for (auto const& c : m_compilers) c->detach_from_process();
  }
  m_compilers.clear();
}

template<typename F>
auto run_compiler(
  const CompilerGuard& compiler,
  int maxRetries,
  bool verboseErrors,
  F&& func,
  bool& internal_error,
  CompileAbortMode mode) ->
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
    } catch (FatalErrorException&) {
      // let these propagate out of the compiler
      throw;
    } catch (CompilerFatal& ex) {
      if (mode >= CompileAbortMode::AllErrors) internal_error = true;
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
                               bool verboseErrors,
                               const RepoOptions& options
) {
  auto extract = [&](const CompilerGuard& c) {
    auto result = c->extract_facts(filename, folly::StringPiece(code, len),
                                   options);
    return FactsJSONString { result };
  };
  auto internal_error = false;
  return run_compiler(
    compiler,
    maxRetries,
    verboseErrors,
    extract,
    internal_error,
    CompileAbortMode::Never);
}

namespace {
CompilerResult assemble_string_handle_errors(const char* code,
                                             const char* hhas,
                                             size_t hhas_size,
                                             const char* filename,
                                             const SHA1& sha1,
                                             const Native::FuncTable& nativeFuncs,
                                             bool& internal_error,
                                             CompileAbortMode mode) {
  try {
    return assemble_string(hhas,
                           hhas_size,
                           filename,
                           sha1,
                           nativeFuncs,
                           false);  /* swallow errors */
  } catch (const FatalErrorException&) {
    throw;
  } catch (const AssemblerFatal& ex) {
    // Assembler returned an error when building this unit
    if (mode >= CompileAbortMode::VerifyErrors) internal_error = true;
    return ex.what();
  } catch (const AssemblerUnserializationError& ex) {
    // Variable unserializer threw when called from the assembler, treat it
    // as an internal error.
    internal_error = true;
    return ex.what();
  } catch (const AssemblerError& ex) {
    if (mode >= CompileAbortMode::VerifyErrors) internal_error = true;

    if (RuntimeOption::EvalHackCompilerVerboseErrors) {
      auto const msg = folly::sformat(
        "{}\n"
        "========== PHP Source ==========\n"
        "{}\n"
        "========== ExternCompiler Result ==========\n"
        "{}\n",
        ex.what(),
        code,
        hhas
      );
      Logger::FError("ExternCompiler Generated a bad unit: {}", msg);
      return msg;
    } else {
      return ex.what();
    }
  } catch (const std::exception& ex) {
    internal_error = true;
    return ex.what();
  }
}
}

CompilerResult CompilerPool::compile(const char* code,
                                     int len,
                                     const char* filename,
                                     const SHA1& sha1,
                                     const Native::FuncTable& nativeFuncs,
                                     bool forDebuggerEval,
                                     bool& internal_error,
                                     const RepoOptions& options,
                                     CompileAbortMode mode) {
  tracing::BlockNoTrace _{"compile-unit-emitter"};

  auto const hhas = run_compiler(
    CompilerGuard{*this},
    m_options.maxRetries,
    m_options.verboseErrors,
    [&] (const CompilerGuard& c) {
      return c->compile(filename,
                        sha1,
                        folly::StringPiece(code, len),
                        forDebuggerEval,
                        options);
    },
    internal_error,
    mode
  );

  return match<CompilerResult>(
    hhas,
    [&] (const ExternCompiler::Hhas& s) -> CompilerResult {
      return assemble_string_handle_errors(code,
                                           s.s.data(),
                                           s.s.size(),
                                           filename,
                                           sha1,
                                           nativeFuncs,
                                           internal_error,
                                           mode);
    },
    [] (std::string s) { return s; }
  );
}

FfpResult CompilerPool::parse(
  std::string file,
  const char* code,
  int len,
  const RepoOptions& options
) {
  auto compile = [&](const CompilerGuard& c) {
    auto result = c->ffp_parse_file(file, folly::StringPiece(code, len),
                                    options);
    return FfpJSONString { result };
  };
  auto internal_error = false;
  return run_compiler(
    CompilerGuard(*this),
    m_options.maxRetries,
    m_options.verboseErrors,
    compile,
    internal_error,
    CompileAbortMode::Never);
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
  HackC::logOptions(header.getDefault("config_jsons", nullptr));

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
  if (m_logStderrThread) {
    m_logStderrThread->stop();
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
  folly::dynamic header = folly::dynamic::object("type", "config");
  writeMessage(header, s_bound_config);
  writeMessage(header, s_misc_config);
}

void ExternCompiler::writeProgram(
  const char* filename,
  SHA1 sha1,
  folly::StringPiece code,
  bool forDebuggerEval,
  const RepoOptions& options
) {
  folly::dynamic header = folly::dynamic::object
    ("type", "code")
    ("sha1", sha1.toString())
    ("file", filename)
    ("is_systemlib", !SystemLib::s_inited)
    ("for_debugger_eval", forDebuggerEval)
    ("config_overrides", options.toDynamic())
    ("log_hackc_mem_stats", RuntimeOption::EvalLogHackcMemStats);
  writeMessage(header, code);
}

void ExternCompiler::writeExtractFacts(
  const std::string& filename,
  folly::StringPiece code,
  const RepoOptions& options
) {
  folly::dynamic header = folly::dynamic::object
    ("type", "facts")
    ("file", filename)
    ("config_overrides", options.toDynamic());
  writeMessage(header, code);
}

void ExternCompiler::writeParseFile(
  const std::string& filename,
  folly::StringPiece code,
  const RepoOptions& options
) {
  folly::dynamic header = folly::dynamic::object
    ("type", "parse")
    ("file", filename)
    ("config_overrides", options.toDynamic());
  writeMessage(header, code);
}

struct CompilerManager final {
  int get_delegate() { return m_delegate; }
  std::mutex& get_delegate_lock() { return m_delegateLock; }
  void set_username(const std::string& username) { m_username = username; }
  void ensure_started();
  void shutdown();
  void detach_after_fork();
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
      WCOREDUMP(status) ? " (core dumped)" : ""
    );
  }
}

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

  m_logStderrThread = std::make_unique<LogThread>(m_pid, m_err);

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

CompilerResult hackc_compile(
  const char* code,
  int len,
  const char* filename,
  const SHA1& sha1,
  const Native::FuncTable& nativeFuncs,
  bool forDebuggerEval,
  bool& internal_error,
  const RepoOptions& options,
  CompileAbortMode mode
) {
  if (RuntimeOption::EvalHackCompilerUseCompilerPool) {
    return s_manager.get_hackc_pool().compile(
      code,
      len,
      filename,
      sha1,
      nativeFuncs,
      forDebuggerEval,
      internal_error,
      options,
      mode
    );
  } else {
    std::uint8_t flags = 0;
    if(forDebuggerEval) {
      flags |= FOR_DEBUGGER_EVAL;
    }
    if(!SystemLib::s_inited) {
      flags |= IS_SYSTEMLIB;
    }
    flags |= DUMP_SYMBOL_REFS;

    std::string aliased_namespaces = options.getAliasedNamespacesConfig();

    hackc_compile_native_environment const native_env = {
      filename,
      aliased_namespaces.data(),
      s_misc_config.data(),
      RuntimeOption::EvalEmitClassPointers,
      RuntimeOption::CheckIntOverflow,
      options.getCompilerFlags(),
      options.getParserFlags(),
      flags
    };

    hackc_compile_output_config const output{true, nullptr};

    std::array<char, 256> buf;
    buf.fill(0);
    hackc_error_buf_t error_buf {buf.data(), buf.size()};

    hackc_compile_from_text_ptr hhas{
      hackc_compile_from_text(&native_env, code, &output, &error_buf)
    };
    if (hhas) {
      return assemble_string_handle_errors(code,
                                           hhas.get(),
                                           strlen(hhas.get()),
                                           filename,
                                           sha1,
                                           nativeFuncs,
                                           internal_error,
                                           mode);
    } else {
      throwErrno(buf.data());
    }
  }
}
////////////////////////////////////////////////////////////////////////////////
}

void CompilerManager::ensure_started() {
  if (m_started.load(std::memory_order_acquire)) {
    return;
  }
  std::unique_lock<std::mutex> l(m_compilers_start_lock);
  if (m_started.load(std::memory_order_relaxed)) {
    return;
  }
  s_bound_config = []() -> std::string {
    if (RuntimeOption::EvalHackCompilerInheritConfig) {
      // necessary to initialize zend-strtod, which is used to serialize
      // boundConfig to JSON (!)
      zend_get_bigint_data();
      return IniSetting::GetAllAsJSON();
    }
    return "";
  }();
  // Some configs, like IncludeRoots, can't easily be Config::Bind(ed), so here
  // we create a place to dump miscellaneous config values HackC might want.
  s_misc_config = []() -> std::string {
    if (RuntimeOption::EvalHackCompilerInheritConfig) {
      return ConfigBuilder()
        .addField("hhvm.include_roots", RuntimeOption::IncludeRoots)
        .toString();
    }
    return "";
  }();

  m_delegate = LightProcess::createDelegate();
  if (m_delegate != kInvalidPid && m_username) {
    std::unique_lock<std::mutex> lock(m_delegateLock);
    LightProcess::ChangeUser(m_delegate, m_username.value());
  }

  CompilerOptions hackcConfig {
    RuntimeOption::EvalHackCompilerVerboseErrors,
    RuntimeOption::EvalHackCompilerMaxRetries,
    RuntimeOption::EvalHackCompilerWorkers,
    RuntimeOption::EvalHackCompilerInheritConfig,
  };
  m_hackc_pool = std::make_unique<CompilerPool>(std::move(hackcConfig));
  m_hackc_pool->start();

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
  int len,
  const RepoOptions& options
) {
  if (RuntimeOption::EvalHackCompilerUseCompilerPool) {
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
      verboseErrors,
      options);
  } else {
    auto const get_facts = [&](const char* source_text) -> ParseFactsResult {
      try {
        hackc_extract_as_json_ptr facts{
          hackc_extract_as_json(options.getFactsFlags(), filename.data(), source_text, true)
        };
        if (facts) {
          std::string facts_str{facts.get()};
          return FactsJSONString { facts_str };
        }
        return FactsJSONString { "" }; // Swallow errors from HackC
      } catch (const std::exception& e) {
        return FactsJSONString { "" }; // Swallow errors from HackC
      }
    };

    if (code && code[0] != '\0') {
      return get_facts(code);
    } else {
      auto w = Stream::getWrapperFromURI(StrNR(filename));
      if (!(w && dynamic_cast<FileStreamWrapper*>(w))) {
        throwErrno("Failed to extract facts: Could not get FileStreamWrapper.");
      }
      const auto f = w->open(StrNR(filename), "r", 0, nullptr);
      if (!f) throwErrno("Failed to extract facts: Could not read source code.");
      auto const str = f->read();
      return get_facts(str.data());
    }
  }
}

FfpResult ffp_parse_file(
  std::string file,
  const char *contents,
  int size,
  const RepoOptions& options
) {
  if (RuntimeOption::EvalHackCompilerUseCompilerPool) {
    return s_manager.get_hackc_pool().parse(file, contents, size, options);
  } else {
    auto const env = options.getParserEnvironment();
    hackc_parse_positioned_full_trivia_ptr parse_tree{
      hackc_parse_positioned_full_trivia(file.c_str(), contents, &env)
    };
    if (parse_tree) {
      std::string ffp_str{parse_tree.get()};
      return FfpJSONString { ffp_str };
    } else {
      return FfpJSONString { "{}" };
    }
  }
}

std::string hackc_version() {
  return s_manager.get_hackc_pool().getVersionString();
}

std::unique_ptr<UnitCompiler>
UnitCompiler::create(const char* code,
                     int codeLen,
                     const char* filename,
                     const SHA1& sha1,
                     const Native::FuncTable& nativeFuncs,
                     bool forDebuggerEval,
                     const RepoOptions& options) {
  auto const make = [code, codeLen, filename, sha1, forDebuggerEval,
                     &nativeFuncs, &options] {
    s_manager.ensure_started();
    return std::make_unique<HackcUnitCompiler>(
      code,
      codeLen,
      filename,
      sha1,
      nativeFuncs,
      forDebuggerEval,
      options
    );
  };

  if (g_unit_emitter_cache_hook && !forDebuggerEval) {
    return std::make_unique<CacheUnitCompiler>(
      code,
      codeLen,
      filename,
      sha1,
      nativeFuncs,
      false,
      options,
      std::move(make)
    );
  } else {
    return make();
  }
}

std::unique_ptr<UnitEmitter> HackcUnitCompiler::compile(
    bool& cacheHit,
    CompileAbortMode mode) {
  auto ice = false;
  cacheHit = false;
  auto res = hackc_compile(m_code,
                           m_codeLen,
                           m_filename,
                           m_sha1,
                           m_nativeFuncs,
                           m_forDebuggerEval,
                           ice,
                           m_options,
                           mode);
  auto unitEmitter = match<std::unique_ptr<UnitEmitter>>(
    res,
    [&] (std::unique_ptr<UnitEmitter>& ue) {
      return std::move(ue);
    },
    [&] (std::string& err) {
      switch (mode) {
      case CompileAbortMode::Never:
        break;
      case CompileAbortMode::AllErrorsNull:
        return std::unique_ptr<UnitEmitter>{};
      case CompileAbortMode::OnlyICE:
      case CompileAbortMode::VerifyErrors:
      case CompileAbortMode::AllErrors:
        // run_compiler will promote errors to ICE as appropriate based on mode
        if (ice) {
          fprintf(
            stderr,
            "Encountered an internal error while processing HHAS for %s, "
            "bailing because Eval.AbortBuildOnCompilerError is set\n\n%s",
            m_filename, err.data()
          );
          _Exit(1);
        }
      }
      return createFatalUnit(
        makeStaticString(m_filename),
        m_sha1,
        FatalOp::Runtime,
        err
      );
    }
  );

  if (unitEmitter) unitEmitter->m_ICE = ice;
  return unitEmitter;
}

std::unique_ptr<UnitEmitter>
CacheUnitCompiler::compile(bool& cacheHit, CompileAbortMode mode) {
  assertx(g_unit_emitter_cache_hook);
  cacheHit = true;
  return g_unit_emitter_cache_hook(
    m_filename,
    m_sha1,
    m_codeLen,
    [&] (bool wantsICE) {
      if (!m_fallback) m_fallback = m_makeFallback();
      assertx(m_fallback);
      return m_fallback->compile(
        cacheHit,
        wantsICE ? mode : CompileAbortMode::AllErrorsNull
      );
    },
    m_nativeFuncs
  );
}

////////////////////////////////////////////////////////////////////////////////
}
