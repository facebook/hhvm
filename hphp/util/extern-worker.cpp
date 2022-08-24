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

#include "hphp/util/extern-worker.h"

#include "hphp/util/assertions.h"
#include "hphp/util/current-executable.h"
#include "hphp/util/hash-map.h"
#include "hphp/util/logger.h"
#include "hphp/util/match.h"
#include "hphp/util/struct-log.h"
#include "hphp/util/trace.h"

#include <folly/FileUtil.h>
#include <folly/Subprocess.h>
#include <folly/gen/Base.h>

#include <boost/filesystem.hpp>

#include <filesystem>
#include <mutex>

namespace fs = std::filesystem;

namespace HPHP::extern_worker {

//////////////////////////////////////////////////////////////////////

using namespace detail;
using namespace folly::gen;

//////////////////////////////////////////////////////////////////////

const char* const s_option = "--extern-worker";

std::atomic<uint64_t> RequestId::s_next{0};
std::atomic<uint64_t> RequestId::s_active{0};

const std::array<OutputType, 1> Client::s_valOutputType{OutputType::Val};
const std::array<OutputType, 1> Client::s_vecOutputType{OutputType::Vec};
const std::array<OutputType, 1> Client::s_optOutputType{OutputType::Opt};

ImplHook g_impl_hook{nullptr};

thread_local bool g_in_job{false};

//////////////////////////////////////////////////////////////////////

namespace {

//////////////////////////////////////////////////////////////////////

TRACE_SET_MOD(extern_worker);

//////////////////////////////////////////////////////////////////////

struct Registry {
  hphp_fast_string_map<JobBase*> registry;
  std::mutex lock;
};

Registry& registry() {
  static Registry registry;
  return registry;
}

//////////////////////////////////////////////////////////////////////

// folly::writeFile expects a container-like input, so this makes a
// ptr/length pair behave like one (without copying).
struct Adaptor {
  size_t size() const { return m_size; }
  bool empty() const { return !size(); }
  const char& operator[](size_t idx) const { return m_ptr[idx]; }
  const char* m_ptr;
  size_t m_size;
};

//////////////////////////////////////////////////////////////////////

}

//////////////////////////////////////////////////////////////////////

namespace detail {

//////////////////////////////////////////////////////////////////////

// Wrappers around the folly functions with error handling

std::string readFile(const fs::path& path) {
  std::string s;
  if (!folly::readFile(path.c_str(), s)) {
    throw Error{
      folly::sformat(
        "Unable to read input from {} [{}]",
        path.c_str(), folly::errnoStr(errno)
      )
    };
  }
  return s;
}

void writeFile(const fs::path& path,
               const char* ptr, size_t size) {
  if (!folly::writeFile(Adaptor{ptr, size}, path.c_str())) {
    throw Error{
      folly::sformat(
        "Unable to write output to {} [{}]",
        path.c_str(), folly::errnoStr(errno)
      )
    };
  }
}

//////////////////////////////////////////////////////////////////////

JobBase::JobBase(const std::string& name)
  : m_name{name}
{
  FTRACE(4, "registering remote worker command \"{}\"\n", m_name);
  auto& r = registry();
  std::lock_guard<std::mutex> _{r.lock};
  auto const insert = r.registry.emplace(m_name, this);
  always_assert(insert.second);
}

//////////////////////////////////////////////////////////////////////

}

//////////////////////////////////////////////////////////////////////

/*
 * A note on conventions: A worker expects to be invoked with the name
 * of the command, followed by 3 paths to directories. The directories
 * represent the config inputs (for init()), the inputs (for multiple
 * runs()), and the last is the output directory, which will be
 * written to.
 *
 * All three directories use the same format for representing
 * data:
 *
 * - The first level contains numbered directories (from 0 to N). Each
 * numbered directory represents a set of inputs/outputs. For outputs,
 * the number corresponds to the matching number of the inputs. The
 * config directory does not have this level, as there's only ever one
 * of them.
 *
 * - The next level contains numbered files or directories (from 0 to
 * N). Each file/directory specifies a particular input. The exact
 * number depends on the job being executed (both sides need to
 * agree). The format of the file or directory depends on that
 * input/output's type.
 *
 * - If the input/output is a "normal" type, it will be a file
 * containing the serialized data (using BlobEncoder) for that
 * input/output. Strings are a special case. If the type is a
 * std::string, it will not be serialized and the string will be
 * stored directly (this makes it easier to represent files as their
 * contents).
 *
 * - If the input/output is an Opt<T>, it will be represented like a
 * "normal" type of type T, except it is not required to be
 * present. If the file is not present (so a gap in the numbering), it
 * is assumed to be std::nullopt.
 *
 * - If the input/output is a Variadic<T>, it will be a
 * directory. Inside of that directory will be numbered files from
 * 0-N, one for each element of the vector. The vector elements will
 * be encoded as "normal" types.
 *
 * NB: Marker types cannot nest, so there aren't any possible deeper
 * levels than this.
 *
 * All implementations must follow this layout when setting up
 * execution. The actual execution on the worker side is completely
 * agnostic to the implementation.
 */

int main(int argc, char** argv) {
  try {
    always_assert(argc > 1);
    always_assert(!strcmp(argv[1], s_option));

    if (argc != 6) {
      std::cerr << "Usage: "
                << argv[0]
                << " " << s_option << " <command name>"
                << " <config dir>"
                << " <output dir>"
                << " <input dir>"
                << std::endl;
      return EXIT_FAILURE;
    }

    std::string name{argv[2]};
    fs::path configPath{argv[3]};
    fs::path outputPath{argv[4]};
    fs::path inputPath{argv[5]};

    FTRACE(2, "extern worker run(\"{}\", {}, {}, {})\n",
           name, configPath.native(), outputPath.native(),
           inputPath.native());

    // Lookup the registered job for the requested name.
    auto const worker = [&] {
      auto& r = registry();
      std::lock_guard<std::mutex> _{r.lock};
      auto const it = r.registry.find(name);
      if (it == r.registry.end()) {
        throw Error{folly::sformat("No command named `{}` registered", name)};
      }
      return it->second;
    }();

    // We insist on a clean output directory.
    if (!fs::create_directory(outputPath)) {
      throw Error{
        folly::sformat("Output directory {} already exists", outputPath.native())
      };
    }

    g_in_job = true;
    SCOPE_EXIT { g_in_job = false; };

    // First do any global initialization.
    time("init", [&] { worker->init(configPath); });
    time("run-all", [&] {
        // Then execute run() for each given set of inputs.
      for (size_t i = 0;; ++i) {
        auto const thisInput = inputPath / folly::to<std::string>(i);
        if (!fs::exists(thisInput)) break;

        auto const thisOutput = outputPath / folly::to<std::string>(i);
        FTRACE(4, "executing #{} ({} -> {})\n",
               i, thisInput.native(), thisOutput.native());
        fs::create_directory(thisOutput, outputPath);

        time(
          [&]{ return folly::sformat("run {}", i); },
          [&] { worker->run(thisInput, thisOutput); }
        );
      }
    });
    // Do any cleanup
    time("fini", [&] { worker->fini(outputPath); });

    return 0;
  } catch (const std::exception& exn) {
    std::cerr << "Error: " << exn.what() << std::endl;
    return EXIT_FAILURE;
  }
}

//////////////////////////////////////////////////////////////////////

namespace {

//////////////////////////////////////////////////////////////////////

/*
 * The builtin implementation which uses fork+exec (and stores data on
 * disk). It is always available, reasonably efficient, and assumed to
 * be "reliable" (IE, never needs a fallback).
 *
 * All data is stored under the "root" (which is the working-dir
 * specified in Options). The two sub-directories are "blobs" and
 * "execs".
 *
 * Data is stored under blobs/, using files with numerically
 * increasing names. Stored files are not stored at all (since they're
 * already on disk). The RefIds are just the path to the file (either
 * under blobs/ or where-ever the stored file was).
 *
 * Executions are stored under execs/. Each entry is a sub-directory
 * with numerically increasing names. Within each sub-directory is the
 * directory layout that the worker expects (see comment above int
 * main()). The data is not actually copied into the input
 * directories. Instead symlinks are created using the path in the
 * RefId. Outputs are not copied either, they just "live" in their
 * output directory.
 */
struct SubprocessImpl : public Client::Impl {
  SubprocessImpl(const Options&, Client&);
  ~SubprocessImpl() override;

  std::string session() const override { return m_root.native(); }

  bool isSubprocess() const override { return true; }
  bool supportsOptimistic() const override { return false; }
  bool isDisabled() const override { return false; }

  coro::Task<BlobVec> load(const RequestId&, IdVec) override;
  coro::Task<IdVec> store(const RequestId&, PathVec, BlobVec,
                          bool) override;
  coro::Task<std::vector<RefValVec>>
  exec(const RequestId&,
       const std::string&,
       RefValVec,
       std::vector<RefValVec>,
       const folly::Range<const OutputType*>&,
       const folly::Range<const OutputType*>*) override;

private:
  fs::path newBlob();
  fs::path newExec();
  static fs::path newRoot(const Options&);

  void doSubprocess(const RequestId&,
                    const std::string&,
                    const fs::path&,
                    const fs::path&,
                    const fs::path&,
                    const fs::path&);

  Options m_options;

  fs::path m_root;
  // SubprocessImpl doesn't need the m_size portion of RefId. So we
  // store an unique integer in it to help verify that the RefId came
  // from this implementation instance.
  size_t m_marker;

  std::atomic<size_t> m_nextBlob;
  std::atomic<size_t> m_nextExec;
};

SubprocessImpl::SubprocessImpl(const Options& options, Client& parent)
  : Impl{"subprocess", parent}
  , m_options{options}
  , m_root{newRoot(m_options)}
    // Cheap way of generating semi-unique integer:
  , m_marker{std::hash<std::string>{}(m_root.native())}
  , m_nextBlob{0}
  , m_nextExec{0}
{
  FTRACE(2, "Using subprocess extern-worker impl with root at {}\n",
         m_root.native());
  Logger::FInfo("Using subprocess extern-worker at {}", m_root.native());

  fs::create_directory(m_root / "blobs");
  fs::create_directory(m_root / "execs");
}

SubprocessImpl::~SubprocessImpl() {
  // Deleting everything under blobs/ and execs/ can take quite a
  // while...
  if (m_options.m_cleanup) {
    Logger::FInfo(
      "Cleaning up subprocess extern-worker at {}...",
      m_root.native()
    );
    auto const before = std::chrono::steady_clock::now();
    auto const removed = time(
      "subprocess cleanup",
      [&] {
        std::error_code ec; // Suppress exceptions
        return fs::remove_all(m_root, ec);
      }
    );
    auto const elapsed = std::chrono::duration_cast<std::chrono::duration<double>>(
      std::chrono::steady_clock::now() - before
    ).count();
    FTRACE(2, "removed {} files\n", removed);
    Logger::FInfo(
      "Done cleaning up subprocess extern-worker. "
      "({} files removed) (took {})",
      removed, folly::prettyPrint(elapsed, folly::PRETTY_TIME_HMS, false)
    );
  }
}

// Create paths for a new blob or a new execution. I lied a little bit
// above in the description of the directory layout. It's true that
// blobs and execs use numerically increasing names, but I shard them
// into directories of 10000 each, to keep their sizes reasonable.
fs::path SubprocessImpl::newBlob() {
  auto const id = m_nextBlob++;
  auto const blobRoot = m_root / "blobs";
  auto const mid = blobRoot / folly::sformat("{:05}", id / 10000);
  fs::create_directory(mid, blobRoot);
  return mid / folly::sformat("{:04}", id % 10000);
}

fs::path SubprocessImpl::newExec() {
  auto const id = m_nextExec++;
  auto const execRoot = m_root / "execs";
  auto const mid = execRoot / folly::sformat("{:05}", id / 10000);
  fs::create_directory(mid, execRoot);
  auto const full = mid / folly::sformat("{:04}", id % 10000);
  fs::create_directory(full, mid);
  return full;
}

// Ensure we always have an unique root under the working directory.
fs::path SubprocessImpl::newRoot(const Options& opts) {
  auto const base = opts.m_workingDir / "hphp-extern-worker";
  fs::create_directories(base);
  auto const full = base / boost::filesystem::unique_path(
    "%%%%-%%%%-%%%%-%%%%-%%%%-%%%%"
  ).native();
  fs::create_directory(full, base);
  return fs::canonical(full);
}

coro::Task<BlobVec> SubprocessImpl::load(const RequestId& requestId,
                                         IdVec ids) {
  // Read every file corresponding to the id and return their
  // contents.
  auto out = from(ids)
    | mapped([&] (const RefId& id) {
        assertx(id.m_size == m_marker);
        FTRACE(4, "{} reading blob from {}\n",
               requestId.tracePrefix(), id.m_id);
        auto blob = readFile(id.m_id);
        FTRACE(4, "{} blob is {} bytes\n",
               requestId.tracePrefix(), blob.size());
        return blob;
      })
    | as<std::vector>();
  HPHP_CORO_MOVE_RETURN(out);
}

coro::Task<IdVec> SubprocessImpl::store(const RequestId& requestId,
                                        PathVec paths,
                                        BlobVec blobs,
                                        bool) {
  // SubprocessImpl never "uploads" files because it uses symlinks. It
  // must write blobs to disk, however (which we classify as an
  // upload).
  stats().blobsUploaded += blobs.size();
  for (auto& b : blobs) stats().blobBytesUploaded += b.size();

  // Create RefIds from the given paths, then write the blobs to disk,
  // then use their paths.
  auto out =
    ((from(paths)
      | mapped([&] (const fs::path& p) {
          return RefId{fs::canonical(p).native(), m_marker};
        })
    ) +
    (from(blobs)
     | mapped([&] (const std::string& b) {
         auto const path = newBlob();
         FTRACE(4, "{} writing size {} blob to {}\n",
                requestId.tracePrefix(), b.size(), path.native());
         writeFile(path, b.data(), b.size());
         return RefId{fs::canonical(path).native(), m_marker};
       })
    ))
    | as<std::vector>();
  HPHP_CORO_MOVE_RETURN(out);
}

coro::Task<std::vector<RefValVec>>
SubprocessImpl::exec(const RequestId& requestId,
                     const std::string& command,
                     RefValVec config,
                     std::vector<RefValVec> inputs,
                     const folly::Range<const OutputType*>& output,
                     const folly::Range<const OutputType*>* finiOutput) {
  auto const execPath = newExec();
  auto const configPath = execPath / "config";
  auto const inputsPath = execPath / "input";
  auto const outputsPath = execPath / "output";

  FTRACE(4, "{} executing \"{}\" inside {} ({} runs)\n",
         requestId.tracePrefix(), command,
         execPath.native(), inputs.size());

  // Set up the directory structure that the worker expects:

  auto const symlink = [&] (const RefId& id,
                            const fs::path& path) {
    assertx(id.m_size == m_marker);
    fs::create_symlink(id.m_id, path);
    FTRACE(4, "{} symlinked {} to {}\n",
           requestId.tracePrefix(), id.m_id, path.native());
  };

  auto const prepare = [&] (const RefValVec& params,
                            const fs::path& parent) {
    for (size_t paramIdx = 0; paramIdx < params.size(); ++paramIdx) {
      auto const& p = params[paramIdx];
      auto const path = parent / folly::to<std::string>(paramIdx);
      match<void>(
        p,
        [&] (const RefId& id) { symlink(id, path); },
        [&] (const Optional<RefId>& id) {
          if (!id.has_value()) return;
          symlink(*id, path);
        },
        [&] (const IdVec& ids) {
          fs::create_directory(path, parent);
          for (size_t vecIdx = 0; vecIdx < ids.size(); ++vecIdx) {
            symlink(ids[vecIdx], path / folly::to<std::string>(vecIdx));
          }
        }
      );
    }
  };

  fs::create_directory(configPath, execPath);
  prepare(config, configPath);

  // Each set of inputs should always have the same size.
  if (debug && !inputs.empty()) {
    auto const size = inputs[0].size();
    for (size_t i = 1; i < inputs.size(); ++i) {
      always_assert(inputs[i].size() == size);
    }
  }

  fs::create_directory(inputsPath, execPath);
  for (size_t i = 0; i < inputs.size(); ++i) {
    auto const path = inputsPath / folly::to<std::string>(i);
    fs::create_directory(path, inputsPath);
    prepare(inputs[i], path);
  }

  // Do the actual fork+exec.
  doSubprocess(
    requestId,
    command,
    execPath,
    configPath,
    inputsPath,
    outputsPath
  );

  // Make RefIds corresponding to the outputs.

  auto const makeOutput =
    [&] (OutputType type, fs::path path) -> RefVal {
    switch (type) {
      case OutputType::Val:
        assertx(fs::exists(path));
        return RefId{std::move(path), m_marker};
      case OutputType::Opt:
        if (!fs::exists(path)) return std::nullopt;
        return make_optional(RefId{std::move(path), m_marker});
      case OutputType::Vec: {
        assertx(fs::exists(path));
        IdVec vec;
        size_t i = 0;
        while (true) {
          auto valPath = path / folly::to<std::string>(i);
          if (!fs::exists(valPath)) break;
          vec.emplace_back(valPath.native(), m_marker);
          ++i;
        }
        return vec;
      }
    }
    always_assert(false);
  };

  auto const makeOutputs =
    [&] (const fs::path& path,
         const folly::Range<const OutputType*>& outputTypes) {
    RefValVec vec;
    vec.reserve(outputTypes.size());
    for (size_t i = 0; i < outputTypes.size(); ++i) {
      vec.emplace_back(
        makeOutput(
          outputTypes[i],
          path / folly::to<std::string>(i)
        )
      );
    }
    return vec;
  };

  std::vector<RefValVec> out;
  out.reserve(inputs.size() + (finiOutput ? 1 : 0));
  for (size_t i = 0; i < inputs.size(); ++i) {
    out.emplace_back(
      makeOutputs(
        outputsPath / folly::to<std::string>(i),
        output
      )
    );
  }
  if (finiOutput) {
    out.emplace_back(
      makeOutputs(
        outputsPath / "fini",
        *finiOutput
      )
    );
  }
  HPHP_CORO_MOVE_RETURN(out);
}

void SubprocessImpl::doSubprocess(const RequestId& requestId,
                                  const std::string& command,
                                  const fs::path& execPath,
                                  const fs::path& configPath,
                                  const fs::path& inputPath,
                                  const fs::path& outputPath) {
  std::vector<std::string> args{
    current_executable_path(),
    s_option,
    command,
    configPath,
    outputPath,
    inputPath
  };

  // Propagate the TRACE option in the environment. We'll copy the
  // trace output into this process' trace output.
  std::vector<std::string> env;
  Optional<fs::path> traceFile;
  if (auto const trace = getenv("TRACE")) {
    traceFile = execPath / "trace.log";
    env.emplace_back(folly::sformat("TRACE={}", trace));
    env.emplace_back(folly::sformat("HPHP_TRACE_FILE={}", traceFile->c_str()));
  }

  FTRACE(4, "{} executing subprocess\n",
         requestId.tracePrefix());

  auto const DEBUG_ONLY before = std::chrono::steady_clock::now();

  // Do the actual fork+exec.
  folly::Subprocess subprocess{
    args,
    folly::Subprocess::Options{}
      .stdinFd(folly::Subprocess::DEV_NULL)
      .pipeStdout()
      .pipeStderr()
      .closeOtherFds(),
    nullptr,
    &env
  };

  auto const [_, stderr] = subprocess.communicate();
  auto const returnCode = subprocess.wait();

  FTRACE(
    4,
    "{} subprocess finished (took {}). Return code: {}, Stderr: {}\n",
    requestId.tracePrefix(),
    prettyPrint(
      std::chrono::duration_cast<std::chrono::duration<double>>(
        std::chrono::steady_clock::now() - before
      ).count(),
      folly::PRETTY_TIME,
      false
    ),
    returnCode.str(),
    stderr
  );

  // Do this before checking the return code. If the process failed,
  // we want to capture anything it logged before throwing.
  if (traceFile && fs::exists(*traceFile)) {
    auto const contents = readFile(*traceFile);
    if (!contents.empty()) {
      Trace::ftraceRelease(
        "vvvvvvvvvvvvvvvvvv remote-exec ({} \"{}\" {}) vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv\n"
        "{}"
        "^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^\n",
        requestId.toString(),
        command,
        execPath.native(),
        contents
      );
    }
  }

  if (!returnCode.exited() || returnCode.exitStatus() != 0) {
    throw Error{
      folly::sformat(
        "Execution of `{}` failed: {}\nstderr:\n{}",
        command,
        returnCode.str(),
        stderr
      )
    };
  }
}

//////////////////////////////////////////////////////////////////////

}

//////////////////////////////////////////////////////////////////////

Client::Client(folly::Executor::KeepAlive<> executor,
               const Options& options)
  : m_options{options}
  , m_stats{std::make_shared<Stats>()}
  , m_forceFallback{false}
{
  Timer _{"create impl"};
  // Look up which implementation to use. If a hook has been
  // registered, and we're allowed to use it according to the Options,
  // try to use it.
  if (g_impl_hook &&
      m_options.m_useSubprocess != Options::UseSubprocess::Always) {
    m_impl = g_impl_hook(m_options, executor, *this);
  }
  // The hook can return nullptr even if registered. In each case, we
  // have no special implementation to use.
  if (!m_impl) {
    // Use the subprocess implementation (which is always available),
    // unless the Options specifies we shouldn't. If not, it's a fatal
    // error.
    if (m_options.m_useSubprocess == Options::UseSubprocess::Never) {
      throw Error{"No non-subprocess impl available"};
    }
    m_impl = std::make_unique<SubprocessImpl>(m_options, *this);
  }
  FTRACE(2, "created \"{}\" impl\n", m_impl->name());
}

Client::~Client() {
  Timer _{[&] { return folly::sformat("destroy impl {}", m_impl->name()); }};
  m_impl.reset();
  m_fallbackImpl.reset();
}

std::unique_ptr<Client::Impl> Client::makeFallbackImpl() {
  // This will be called once from within LockFreeLazy, so we only
  // emit this warning once.
  Logger::Warning(
    "Certain operations will use local fallback from this "
    "point on and may run slower."
  );
  return std::make_unique<SubprocessImpl>(m_options, *this);
}

coro::Task<Ref<std::string>> Client::storeFile(fs::path path,
                                               bool optimistic) {
  RequestId requestId{"store file"};

  FTRACE(
    2, "{} storing {}{}\n",
    requestId.tracePrefix(),
    path.native(),
    optimistic ? " (optimistically)" : ""
  );

  ++m_stats->files;
  ++m_stats->storeCalls;
  SCOPE_EXIT {
    m_stats->storeLatencyUsec += std::chrono::duration_cast<
      std::chrono::microseconds
    >(requestId.elapsed()).count();
  };

  auto wasFallback = false;
  auto ids = HPHP_CORO_AWAIT(tryWithFallback<IdVec>(
    [&] (Impl& i, bool isFallback) {
      if (isFallback) ++m_stats->fileFallbacks;
      return i.store(
        requestId,
        PathVec{path},
        {},
        optimistic
      );
    },
    wasFallback
  ));
  assertx(ids.size() == 1);

  Ref<std::string> ref{std::move(ids[0]), wasFallback};
  HPHP_CORO_MOVE_RETURN(ref);
}

coro::Task<std::vector<Ref<std::string>>>
Client::storeFile(std::vector<fs::path> paths,
                  bool optimistic) {
  RequestId requestId{"store files"};

  FTRACE(
    2, "{} storing {} files{}\n",
    requestId.tracePrefix(),
    paths.size(),
    optimistic ? " (optimistically)" : ""
  );
  ONTRACE(4, [&] {
    for (auto const& p : paths) {
      FTRACE(4, "{} storing {}\n", requestId.tracePrefix(), p.native());
    }
  }());

  m_stats->files += paths.size();
  ++m_stats->storeCalls;
  SCOPE_EXIT {
    m_stats->storeLatencyUsec += std::chrono::duration_cast<
      std::chrono::microseconds
    >(requestId.elapsed()).count();
  };

  auto const DEBUG_ONLY size = paths.size();
  auto wasFallback = false;
  auto ids = HPHP_CORO_AWAIT(tryWithFallback<IdVec>(
    [&] (Impl& i, bool isFallback) {
      if (isFallback) m_stats->fileFallbacks += paths.size();
      return i.store(requestId, paths, {}, optimistic);
    },
    wasFallback
  ));
  assertx(ids.size() == size);

  auto out = from(ids)
    | move
    | mapped([&] (auto&& id) {
        return Ref<std::string>{std::move(id), wasFallback};
      })
    | as<std::vector>();
  HPHP_CORO_MOVE_RETURN(out);
}

//////////////////////////////////////////////////////////////////////

// Sleep some random amount corresponding to the number of retries
// we've tried.
void Client::Impl::throttleSleep(size_t retry,
                                 std::chrono::milliseconds base) {
  // Each retry doubles the size of the window. We select a random
  // amount from within that.
  auto const scale = uint64_t{1} << std::min(retry, size_t{16});
  auto const window = std::chrono::microseconds{base} * scale;

  static std::mt19937_64 engine = [&] {
    std::random_device rd;
    std::seed_seq seed{rd(), rd(), rd(), rd(), rd(), rd(), rd(), rd()};
    return std::mt19937_64{seed};
  }();

  std::uniform_int_distribution<> distrib(0, window.count());
  auto const wait = [&] {
    static std::mutex lock;
    std::lock_guard<std::mutex> _{lock};
    return std::chrono::microseconds{distrib(engine)};
  }();

  // Use normal sleep here, not coro friendly sleep. The whole purpose
  // is to slow down execution and using a coro sleep will just allow
  // a lot of other actions to run.
  std::this_thread::sleep_for(wait);
}

//////////////////////////////////////////////////////////////////////

std::string Client::Stats::toString(const std::string& phase,
                                    const std::string& extra) const {
  auto const bytes = [] (size_t b) {
    auto s = folly::prettyPrint(
      b,
      folly::PRETTY_BYTES
    );
    if (!s.empty() && s[s.size()-1] == ' ') s.resize(s.size()-1);
    return s;
  };

  auto const usecs = [] (size_t t) {
    auto s = prettyPrint(
      double(t) / 1000000.0,
      folly::PRETTY_TIME_HMS,
      false
    );
    if (!s.empty() && s[s.size()-1] == ' ') s.resize(s.size()-1);
    return s;
  };

  auto const pct = [] (size_t a, size_t b) {
    if (!b) return 0.0;
    return double(a) / b * 100.0;
  };

  auto const execs_ = execs.load();
  auto const allocatedCores = execAllocatedCores.load();
  auto const cpuUsecs = execCpuUsec.load();
  auto const execCalls_ = execCalls.load();
  auto const storeCalls_ = storeCalls.load();
  auto const loadCalls_ = loadCalls.load();

  return folly::sformat(
    "  {}:{}\n"
    "  Execs: {:,} total, {:,} cache-hits ({:.2f}%), {:,} optimistically, {:,} fallback\n"
    "  Files: {:,} total, {:,} read, {:,} queried, {:,} uploaded ({}), {:,} fallback\n"
    "  Blobs: {:,} total, {:,} queried, {:,} uploaded ({}), {:,} fallback\n"
    "  Cpu: {} usage, {:,} allocated cores ({}/core)\n"
    "  Mem: {} max used, {} reserved\n"
    "  {:,} downloads ({}), {:,} throttles\n"
    "  Avg Latency: {} exec, {} store, {} load",
    phase,
    extra.empty() ? "" : folly::sformat(" {}", extra),
    execs_,
    execCacheHits.load(),
    pct(execCacheHits.load(), execs_),
    optimisticExecs.load(),
    execFallbacks.load(),
    files.load(),
    filesRead.load(),
    filesQueried.load(),
    filesUploaded.load(),
    bytes(fileBytesUploaded.load()),
    fileFallbacks.load(),
    blobs.load(),
    blobsQueried.load(),
    blobsUploaded.load(),
    bytes(blobBytesUploaded.load()),
    blobFallbacks.load(),
    usecs(cpuUsecs),
    allocatedCores,
    usecs(allocatedCores ? (cpuUsecs / allocatedCores) : 0),
    bytes(execMaxUsedMem.load()),
    bytes(execReservedMem.load()),
    downloads.load(),
    bytes(bytesDownloaded.load()),
    throttles.load(),
    usecs(execCalls_ ? (execLatencyUsec.load() / execCalls_) : 0),
    usecs(storeCalls_ ? (storeLatencyUsec.load() / storeCalls_) : 0),
    usecs(loadCalls_ ? (loadLatencyUsec.load() / loadCalls_) : 0)
  );
}

void Client::Stats::logSample(const std::string& phase,
                              StructuredLogEntry& sample) const {
  sample.setInt(phase + "_total_execs", execs.load());
  sample.setInt(phase + "_cache_hits", execCacheHits.load());
  sample.setInt(phase + "_optimistically", optimisticExecs.load());
  sample.setInt(phase + "_fallbacks", execFallbacks.load());

  sample.setInt(phase + "_total_files", files.load());
  sample.setInt(phase + "_file_reads", filesRead.load());
  sample.setInt(phase + "_file_queries", filesQueried.load());
  sample.setInt(phase + "_file_stores", filesUploaded.load());
  sample.setInt(phase + "_file_stores_bytes", fileBytesUploaded.load());
  sample.setInt(phase + "_file_fallbacks", fileFallbacks.load());

  sample.setInt(phase + "_total_blobs", blobs.load());
  sample.setInt(phase + "_blob_queries", blobsQueried.load());
  sample.setInt(phase + "_blob_stores", blobsUploaded.load());
  sample.setInt(phase + "_blob_stores_bytes", blobBytesUploaded.load());
  sample.setInt(phase + "_blob_fallbacks", blobFallbacks.load());

  sample.setInt(phase + "_total_loads", downloads.load());
  sample.setInt(phase + "_total_loads_bytes", bytesDownloaded.load());
  sample.setInt(phase + "_throttles", throttles.load());

  sample.setInt(phase + "_exec_cpu_usec", execCpuUsec.load());
  sample.setInt(phase + "_exec_allocated_cores", execAllocatedCores.load());
  sample.setInt(phase + "_exec_max_used_mem", execMaxUsedMem.load());
  sample.setInt(phase + "_exec_reserved_mem", execReservedMem.load());

  auto const execCalls_ = execCalls.load();
  auto const storeCalls_ = storeCalls.load();
  auto const loadCalls_ = loadCalls.load();

  sample.setInt(
    phase + "_avg_exec_latency_usec",
    execCalls_ ? (execLatencyUsec.load() / execCalls_) : 0
  );
  sample.setInt(
    phase + "_avg_store_latency_usec",
    storeCalls_ ? (storeLatencyUsec.load() / storeCalls_) : 0
  );
  sample.setInt(
    phase + "_avg_load_latency_usec",
    loadCalls_ ? (loadLatencyUsec.load() / loadCalls_) : 0
  );
}

//////////////////////////////////////////////////////////////////////

}
