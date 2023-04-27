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
#include <folly/memory/UninitializedMemoryHacks.h>

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

// If passed via the command-line, the worker is running in "local"
// mode and will use a different mechanism to read inputs/write
// outputs.
const char* const g_local_option = "--local";

// If running in local mode, the FD of the pipe used to communicate
// output back to the parent.
constexpr int g_local_pipe_fd = 3;

// For the subprocess impl, if the size of the data is <= this
// constant, we'll store it "inline" in the ref itself, rather than
// writing it to disk. This values probably needs more tuning.
constexpr size_t g_inline_size = 64;

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

// Represents an open file. Used this over readFile/writeFile if you
// want to maintain a persistent FD, or want to just read/write just a
// portion of the file. Only reading or appending to the file is
// supported.
struct FD {
  FD(const fs::path& path, bool read, bool write, bool create)
      : m_path{path}, m_offset{0} {
    assertx(IMPLIES(create, write));

    auto flags = O_CLOEXEC;
    if (read) {
      flags |= (write ? O_RDWR : O_RDONLY);
    } else {
      assertx(write);
      flags |= O_WRONLY;
    }

    if (write) flags |= O_APPEND;
    if (create) {
      flags |= O_CREAT | O_EXCL;
      // We're creating it, so the file is empty
      m_offset = 0;
    }

    auto fd = folly::openNoInt(m_path.c_str(), flags);
    if (fd < 0) {
      throw Error{
        folly::sformat(
          "Unable to open {} [{}]",
          m_path.native(), folly::errnoStr(errno)
        )
      };
    }
    m_fd = fd;
    SCOPE_FAIL { ::close(m_fd); };

    // If we're going to write to the file, but not creating it, we
    // don't know what the end of the file is, so find the current
    // offset.
    if (write && !create) syncOffset();
  }
  ~FD() { if (m_fd >= 0) ::close(m_fd); }

  FD(const FD&) = delete;
  FD(FD&& o) noexcept : m_fd{o.m_fd}, m_path{o.m_path}, m_offset{o.m_offset}
  { o.m_fd = -1; }
  FD& operator=(const FD&) = delete;
  FD& operator=(FD&& o) noexcept {
    std::swap(m_fd, o.m_fd);
    std::swap(m_path, o.m_path);
    std::swap(m_offset, o.m_offset);
    return *this;
  }

  const fs::path& path() const { return m_path; }

  std::string read(size_t offset, size_t size) const {
    assertx(m_fd >= 0);

    std::string data;
    folly::resizeWithoutInitialization(data, size);

    auto const read = folly::preadFull(m_fd, data.data(), size, offset);
    if (read == size) return data;
    if (read < 0) {
      throw Error{
        folly::sformat(
          "Failed reading {} bytes from {} at {} [{}]",
          size, m_path.native(), offset, folly::errnoStr(errno)
        )
      };
    }
    throw Error{
      folly::sformat(
        "Partial read from {} at {} (expected {}, actual {})",
        m_path.native(), offset, size, read
      )
    };
  }

  size_t append(const std::string& data) {
    assertx(m_fd >= 0);

    auto const written = folly::writeFull(m_fd, data.data(), data.size());
    if (written < 0) {
      throw Error{
        folly::sformat(
          "Failed writing {} bytes to {} [{}]",
          data.size(), m_path.native(), folly::errnoStr(errno)
        )
      };
    }
    if (written != data.size()) {
      throw Error{
        folly::sformat(
          "Partial write to {} (expected {}, actual {})",
          m_path.native(), data.size(), written
        )
      };
    }
    auto const prev = m_offset;
    m_offset += written;
    return prev;
  }

  // Update m_offset to the end of the file. This is only needed if
  // you've opened an already created file, or if you know someone
  // else has written to it.
  void syncOffset() {
    assertx(m_fd >= 0);
    auto const size = ::lseek(m_fd, 0, SEEK_END);
    if (size < 0) {
      throw Error{
        folly::sformat(
          "Unable to seek to end of {}",
          m_path.native()
        )
      };
    }
    m_offset = size;
  }

private:
  int m_fd;
  fs::path m_path;
  size_t m_offset;
};

//////////////////////////////////////////////////////////////////////

// Read from the FD (which is assumed to be a pipe) and append the
// contents to the given string. Return true if the pipe is now
// closed, or false otherwise. The false case can only happen if the
// FD is non-blocking.
bool readFromPipe(int fd, std::string& s) {
  auto realEnd = s.size();
  SCOPE_EXIT { s.resize(realEnd); };
  while (true) {
    assertx(realEnd <= s.size());
    auto spaceLeft = s.size() - realEnd;
    while (spaceLeft < 1024) {
      folly::resizeWithoutInitialization(
        s, std::max<size_t>(4096, s.size() * 2)
      );
      spaceLeft = s.size() - realEnd;
    }
    auto const read =
      folly::readNoInt(fd, s.data() + realEnd, spaceLeft);
    if (read < 0) {
      if (errno == EAGAIN) return false;
      throw Error{
        folly::sformat(
          "Failed reading from pipe {} [{}]",
          fd,
          folly::errnoStr(errno)
        )
      };
    } else if (read == 0) {
      return true;
    }
    realEnd += read;
  }
}

// Read from the FD (which is assumed to be a pipe) until it is closed
// (returns EOF). Returns all data read. Only use this with a blocking
// FD, as it will spin otherwise.
std::string readFromPipe(int fd) {
  std::string out;
  while (!readFromPipe(fd, out)) {}
  return out;
}

// Write all of the given data to the FD (which is assumed to be a
// pipe).
void writeToPipe(int fd, const char* data, size_t size) {
  auto const written = folly::writeFull(fd, data, size);
  if (written < 0) {
    throw Error{
      folly::sformat(
        "Failed writing {} bytes to pipe {} [{}]",
        size, fd, folly::errnoStr(errno)
      )
    };
  }
  if (written != size) {
    throw Error{
      folly::sformat(
        "Partial write to pipe {} (expected {}, actual {})",
        fd, size, written
      )
    };
  }
}

//////////////////////////////////////////////////////////////////////

/*
 * "Serialized" API:
 *
 * For this mode a worker expects to be invoked with --local, followed
 * by the name of the command, the root of the blob files, and the
 * name of the blob file to write results to.
 *
 * This mode uses blob files written locally, and communicates its
 * input/output via serialized RefIds via stdin and pipes. It's meant
 * for SubprocessImpl and is more efficient than the "File" mode
 * below.
 *
 * See SubprocessImpl for more description.
 */

struct SerializedSource : public detail::ISource {
  SerializedSource(fs::path root, std::string s)
    : m_source{std::move(s)}
    , m_decoder{m_source.data(), m_source.size()}
    , m_currentInput{0}
    , m_numInputs{0}
    , m_root{std::move(root)}
  {}
  ~SerializedSource() = default;

  std::string blob() override {
    return refToBlob(decodeRefId(m_decoder));
  }
  Optional<std::string> optBlob() override {
    auto r = decodeOptRefId(m_decoder);
    if (!r) return std::nullopt;
    return refToBlob(*r);
  }

  BlobVec variadic() override {
    return from(decodeRefIdVec(m_decoder))
      | map([&] (const RefId& r) { return refToBlob(r); })
      | as<std::vector>();
  }

  void initDone() override {
    assertx(m_numInputs == 0);
    assertx(m_currentInput == 0);
    m_decoder(m_numInputs);
  }
  bool inputEnd() const override {
    return m_currentInput >= m_numInputs;
  }
  void nextInput() override {
    assertx(!inputEnd());
    ++m_currentInput;
  }
  void finish() override { m_decoder.assertDone(); }

  static RefId decodeRefId(BlobDecoder& d) {
    // Optimize RefId representation for space. If m_size is <= the
    // inline size, we know that m_extra is zero, and that m_id has
    // the same length as m_size, so we do not need to encode it
    // twice.
    decltype(RefId::m_size) size;
    d(size);
    if (size <= g_inline_size) {
      assertx(d.remaining() >= size);
      std::string id{(const char*)d.data(), size};
      d.advance(size);
      return RefId{std::move(id), size, 0};
    } else {
      decltype(RefId::m_id) id;
      decltype(RefId::m_extra) offset;
      d(id);
      d(offset);
      return RefId{std::move(id), size, offset};
    }
  }

  static Optional<RefId> decodeOptRefId(BlobDecoder& d) {
    bool present;
    d(present);
    if (!present) return std::nullopt;
    return decodeRefId(d);
  }

  static IdVec decodeRefIdVec(BlobDecoder& d) {
    std::vector<RefId> out;
    size_t size;
    d(size);
    out.reserve(size);
    std::generate_n(
      std::back_inserter(out),
      size,
      [&] { return decodeRefId(d); }
    );
    return out;
  }

private:
  std::string refToBlob(const RefId& r) {
    if (r.m_size <= g_inline_size) {
      assertx(r.m_id.size() == r.m_size);
      assertx(!r.m_extra);
      return r.m_id;
    }

    fs::path path{r.m_id};
    if (path.is_absolute()) {
      return FD{path, true, false, false}.read(r.m_extra, r.m_size);
    }

    auto it = m_fdCache.find(path.native());
    if (it == m_fdCache.end()) {
      auto [elem, emplaced] = m_fdCache.emplace(
        path.native(),
        FD{m_root / path, true, false, false}
      );
      assertx(emplaced);
      it = elem;
    }
    return it->second.read(r.m_extra, r.m_size);
  }

  std::string m_source;
  BlobDecoder m_decoder;
  size_t m_currentInput;
  size_t m_numInputs;

  fs::path m_root;
  hphp_fast_map<std::string, FD> m_fdCache;
};

struct SerializedSink : public detail::ISink {
  explicit SerializedSink(fs::path outputFile)
    : m_fd{outputFile, false, true, false} {}

  void blob(const std::string& b) override {
    encodeRefId(makeRefId(b), m_encoder);
  }
  void optBlob(const Optional<std::string>& b) override {
    encodeOptRefId(b ? makeRefId(*b) : Optional<RefId>{}, m_encoder);
  }
  void variadic(const BlobVec& v) override {
    auto const refs = from(v)
      | map([&] (const std::string& b) { return makeRefId(b); })
      | as<std::vector>();
    encodeRefIdVec(refs, m_encoder);
  }
  void nextOutput() override {}
  void startFini() override {}

  void finish() override {
    writeToPipe(
      g_local_pipe_fd,
      (const char*)m_encoder.data(),
      m_encoder.size()
    );
    ::close(g_local_pipe_fd);
  }

  static void encodeRefId(const RefId& r, BlobEncoder& e) {
    // Optimized RefId encoding. If it's an inline ref, we can encode
    // it more optimally. See above in SerializedSource::decodeRefId.
    e(r.m_size);
    if (r.m_size <= g_inline_size) {
      assertx(r.m_id.size() == r.m_size);
      assertx(!r.m_extra);
      e.writeRaw(r.m_id.data(), r.m_id.size());
    } else {
      e(r.m_id);
      e(r.m_extra);
    }
  }

  static void encodeOptRefId(const Optional<RefId>& r, BlobEncoder& e) {
    if (r) {
      e(true);
      encodeRefId(*r, e);
    } else {
      e(false);
    }
  }

  static void encodeRefIdVec(const IdVec& v, BlobEncoder& e) {
    e((size_t)v.size());
    for (auto const& r : v) encodeRefId(r, e);
  }

private:
  RefId makeRefId(const std::string& b) {
    if (b.size() <= g_inline_size) return RefId{b, b.size(), 0};
    auto const offset = m_fd.append(b);
    return RefId{m_fd.path().filename(), b.size(), offset};
  }

  FD m_fd;
  BlobEncoder m_encoder;
};

//////////////////////////////////////////////////////////////////////

/*
 * "File" API:
 *
 * For this mode a worker expects to be invoked with the name of the
 * command, followed by 3 paths to directories. The directories
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
 */

struct FileSource : public detail::ISource {
  FileSource(fs::path config, fs::path input)
    : m_configPath{std::move(config)}
    , m_inputPath{std::move(input)}
    , m_itemIdx{0}
    , m_inputIdx{0}
    , m_itemBase{m_configPath} {}
  ~FileSource() = default;

  std::string blob() override {
    auto const filename = m_itemBase / folly::to<std::string>(m_itemIdx++);
    return detail::readFile(filename);
  }
  Optional<std::string> optBlob() override {
    auto const filename = m_itemBase / folly::to<std::string>(m_itemIdx++);
    if (!fs::exists(filename)) return std::nullopt;
    return detail::readFile(filename);
  }

  BlobVec variadic() override {
    BlobVec out;
    auto const vecBase = m_itemBase / folly::to<std::string>(m_itemIdx++);
    for (size_t i = 0;; ++i) {
      auto const valPath = vecBase / folly::to<std::string>(i);
      // A break in the numbering means the end of the vector.
      if (!fs::exists(valPath)) break;
      out.emplace_back(detail::readFile(valPath));
    }
    return out;
  }

  void initDone() override {
    assertx(m_itemBase == m_configPath);
    assertx(m_inputIdx == 0);
    m_itemIdx = 0;
    m_itemBase = m_inputPath / "0";
  }

  bool inputEnd() const override {
    assertx(m_itemBase == m_inputPath / folly::to<std::string>(m_inputIdx));
    return !fs::exists(m_itemBase);
  }

  void nextInput() override {
    assertx(m_itemBase == m_inputPath / folly::to<std::string>(m_inputIdx));
    m_itemIdx = 0;
    m_itemBase = m_inputPath / folly::to<std::string>(++m_inputIdx);
  }

  void finish() override {}
private:
  fs::path m_configPath;
  fs::path m_inputPath;

  size_t m_itemIdx;
  size_t m_inputIdx;
  fs::path m_itemBase;
};

struct FileSink : public detail::ISink {
  explicit FileSink(fs::path base)
    : m_base{std::move(base)}
    , m_itemIdx{0}
    , m_outputIdx{0}
  {
    // We insist on a clean output directory.
    if (!fs::create_directory(m_base)) {
      throw Error{
        folly::sformat("Output directory {} already exists", m_base.native())
      };
    }
  }

  void blob(const std::string& b) override { write(b); }
  void optBlob(const Optional<std::string>& b) override {
    if (!b) {
      makeDir();
      ++m_itemIdx;
    } else {
      write(*b);
    }
  }
  void variadic(const BlobVec& v) override {
    auto const dir = makeDir();
    auto const vecDir = dir / folly::to<std::string>(m_itemIdx);
    fs::create_directory(vecDir, dir);
    for (size_t i = 0; i < v.size(); ++i) {
      auto const& b = v[i];
      writeFile(vecDir / folly::to<std::string>(i), b.data(), b.size());
    }
    ++m_itemIdx;
  }

  void nextOutput() override {
    assertx(m_outputIdx.has_value());
    ++*m_outputIdx;
    m_itemIdx = 0;
  }
  void startFini() override {
    assertx(m_outputIdx.has_value());
    m_outputIdx.reset();
    m_itemIdx = 0;
  }
  void finish() override {}
private:
  fs::path currentDir() {
    if (!m_outputIdx) return m_base / "fini";
    return m_base / folly::to<std::string>(*m_outputIdx);
  }

  fs::path makeDir() {
    auto const outputDir = currentDir();
    if (!m_itemIdx) fs::create_directory(outputDir, m_base);
    return outputDir;
  }

  void write(const std::string& b) {
    auto const dir = makeDir();
    writeFile(dir / folly::to<std::string>(m_itemIdx), b.data(), b.size());
    ++m_itemIdx;
  }

  fs::path m_base;
  size_t m_itemIdx;
  Optional<size_t> m_outputIdx;
};

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

namespace {

// Parse the command-line and create the appropriate Source and Sink
// for input and output.
std::tuple<
  std::unique_ptr<ISource>,
  std::unique_ptr<ISink>,
  std::string
>
parseOptions(int argc, char** argv) {
  always_assert(argc > 1);
  always_assert(!strcmp(argv[1], s_option));

  if (argc >= 2 && !strcmp(argv[2], g_local_option)) {
    if (argc != 6) {
      std::cerr << "Usage: "
                << argv[0]
                << " " << s_option
                << " " << g_local_option
                << " <command name>"
                << " <root>"
                << " <output file>"
                << std::endl;
      return std::make_tuple(nullptr, nullptr, "");
    }

    std::string name{argv[3]};
    fs::path root{argv[4]};
    fs::path outputFile{argv[5]};

    FTRACE(2, "extern worker run (local) (\"{}\", {}, {})\n",
           name, root.native(), outputFile.native());

    // Input comes from STDIN
    auto source =
      time("read-pipe", [] { return readFromPipe(STDIN_FILENO); });
    return std::make_tuple(
      std::make_unique<SerializedSource>(
        std::move(root),
        std::move(source)
      ),
      std::make_unique<SerializedSink>(std::move(outputFile)),
      std::move(name)
    );
  } else if (argc != 6) {
    std::cerr << "Usage: "
              << argv[0]
              << " " << s_option
              << " <command name>"
              << " <config dir>"
              << " <output dir>"
              << " <input dir>"
              << std::endl;
    return std::make_tuple(nullptr, nullptr, "");
  } else {
    std::string name{argv[2]};
    fs::path configPath{argv[3]};
    fs::path outputPath{argv[4]};
    fs::path inputPath{argv[5]};

    FTRACE(2, "extern worker run(\"{}\", {}, {}, {})\n",
           name, configPath.native(), outputPath.native(),
           inputPath.native());

    return std::make_tuple(
      std::make_unique<FileSource>(
        std::move(configPath),
        std::move(inputPath)
      ),
      std::make_unique<FileSink>(std::move(outputPath)),
      std::move(name)
    );
  }
}

}

//////////////////////////////////////////////////////////////////////

int main(int argc, char** argv) {
  Timer _{"main"};

  try {
    auto const [source, sink, name] = parseOptions(argc, argv);
    if (!source) return EXIT_FAILURE;

    // Lookup the registered job for the requested name.
    auto const worker = [&, name = name] {
      auto& r = registry();
      std::lock_guard<std::mutex> _{r.lock};
      auto const it = r.registry.find(name);
      if (it == r.registry.end()) {
        throw Error{folly::sformat("No command named `{}` registered", name)};
      }
      return it->second;
    }();

    g_in_job = true;
    SCOPE_EXIT { g_in_job = false; };

    // First do any global initialization.
    time("init", [&, &source = source] { worker->init(*source); });
    time("run-all", [&, &source = source, &sink = sink] {
      // Then execute run() until we're out of inputs.
      size_t run = 0;
      while (!source->inputEnd()) {
        time(
          [&] { return folly::sformat("run {}", run); },
          [&, &source = source, &sink = sink] { worker->run(*source, *sink); }
        );
        ++run;
      }
      source->finish();
    });
    // Do any cleanup and flush output to its final destination
    time("fini", [&, &sink = sink] { worker->fini(*sink); });
    time("flush", [&sink = sink] { sink->finish(); });
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
 * specified in Options).
 *
 * Data is stored in blob files under the root, whose names are
 * assigned numerically. The blob files are pooled. When a thread
 * needs to write, it checks out a free file, append the data to it,
 * then "returns" it back to the pool. The particular blob file data
 * is stored to is therefore arbitrary. Files are not stored at all
 * (since they're already on disk).
 *
 * The RefIds are just the path to the file, either a blob file, or
 * some other file. If the data was stored in a blob file, the m_extra
 * of the RefId will contain its offset in the blob file. For stored
 * files, m_extra will always be zero.
 *
 * As an optimization, if the data is less or equal to g_inline_size,
 * then its stored "inline" in the RefId. That is, m_id is the blob
 * itself. Inline/non-inline blobs can be distinguished by the m_size
 * field. This also applies to stored files (if sufficiently small,
 * we'll read it and store it inline).
 *
 * Workers are forked, given their inputs, calculate outputs, then
 * write their outputs and exit. Input is given to the worker via
 * stdin (as a stream of serialized RefIds). As part of the
 * command-line, the worker is given an output file to write its
 * output to. It does (creating RefIds in the process), and reports
 * the output RefIds via a pipe (g_local_pipe_fd).
 */
struct SubprocessImpl : public Client::Impl {
  SubprocessImpl(const Options&, Client&);
  ~SubprocessImpl() override;

  std::string session() const override { return m_fdManager->root().native(); }

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
       const folly::Range<const OutputType*>*,
       Client::ExecMetadata
       ) override;

private:
  // Manage the pool of blob files
  struct FDManager {
    explicit FDManager(fs::path);

    const fs::path& root() const { return m_root; }

    // Acquire a FD to the given file to read from. If the path does
    // not correspond to a blob file, nullptr is returned.
    const FD* acquireForRead(const fs::path&);
    // Acquire a FD to a blob file to append to. You don't have a say
    // in which file you get. The FD must be returned via release()
    // when done. You have exclusive access to this FD.
    FD* acquireForAppend();
    // Release a FD acquired from acquireForAppend.
    void release(FD&);
  private:
    std::unique_ptr<FD> newFD();

    fs::path m_root;
    folly_concurrent_hash_map_simd<std::string, std::unique_ptr<FD>> m_fds;

    std::mutex m_lock;
    std::stack<FD*> m_forAppend;
    size_t m_nextBlob;
  };

  // Similar to FDManager, but manages trace files. We have workers
  // re-use trace files to avoid generating a huge number of time.
  struct TraceFileManager {
    fs::path get();
    void put(fs::path);
  private:
    std::mutex m_lock;
    std::stack<fs::path> m_paths;
    size_t m_nextId{0};
  };

  static fs::path newRoot(const Options&);

  coro::Task<std::string> doSubprocess(const RequestId&,
                                       const std::string&,
                                       std::string,
                                       const fs::path&);

  Options m_options;
  std::unique_ptr<FDManager> m_fdManager;
  TraceFileManager m_traceManager;
};

SubprocessImpl::SubprocessImpl(const Options& options, Client& parent)
  : Impl{"subprocess", parent}
  , m_options{options}
  , m_fdManager{std::make_unique<FDManager>(newRoot(m_options))}
{
  FTRACE(2, "Using subprocess extern-worker impl with root at {}\n",
         m_fdManager->root().native());
  Logger::FInfo(
    "Using subprocess extern-worker at {}",
    m_fdManager->root().native()
  );
}

SubprocessImpl::~SubprocessImpl() {
  auto const root = m_fdManager->root();
  // Destroy FD manager first, to ensure no open FDs to any files
  // while cleaning up.
  m_fdManager.reset();

  if (m_options.m_cleanup) {
    Logger::FInfo(
      "Cleaning up subprocess extern-worker at {}...",
      root.native()
    );
    auto const before = std::chrono::steady_clock::now();
    auto const removed = time(
      "subprocess cleanup",
      [&] {
        std::error_code ec; // Suppress exceptions
        return fs::remove_all(root, ec);
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
        FTRACE(4, "{} reading blob from {}\n",
               requestId.tracePrefix(), id.toString());
        if (id.m_size <= g_inline_size) {
          // Inline data requires no file read
          assertx(id.m_id.size() == id.m_size);
          assertx(!id.m_extra);
          return id.m_id;
        }
        if (auto const fd = m_fdManager->acquireForRead(id.m_id)) {
          // The data is in a blob file, so use a cached FD
          return fd->read(id.m_extra, id.m_size);
        }
        // It's some other (non-blob) file. Create an ephemeral FD to
        // read it.
        return FD{id.m_id, true, false, false}.read(id.m_extra, id.m_size);
      })
    | as<std::vector>();
  HPHP_CORO_MOVE_RETURN(out);
}

coro::Task<IdVec> SubprocessImpl::store(const RequestId& requestId,
                                        PathVec paths,
                                        BlobVec blobs,
                                        bool) {
  // SubprocessImpl never "uploads" files, but it must write to disk
  // (which we classify as an upload).

  FD* fd = nullptr;
  SCOPE_EXIT { if (fd) m_fdManager->release(*fd); };

  // Update stats. Skip blobs which we'll store inline.
  for (auto const& b : blobs) {
    if (b.size() <= g_inline_size) continue;
    ++stats().blobsUploaded;
    stats().blobBytesUploaded += b.size();
    if (!fd) fd = m_fdManager->acquireForAppend();
    assertx(fd);
  }

  auto out =
    ((from(paths)
      | mapped([&] (const fs::path& p) {
          auto fileSize = fs::file_size(p);
          if (fileSize <= g_inline_size) {
            FTRACE(4, "{} storing file {} inline\n",
                   requestId.tracePrefix(),
                   p.native());
            auto const contents = readFile(p);
            // Size of file could theoretically change between
            // stat-ing and reading it.
            if (contents.size() <= g_inline_size) {
              return RefId{contents, contents.size(), 0};
            }
            fileSize = contents.size();
          }
          // We distinguish blob files from non-blob files by making
          // sure non-blob files are always absolute paths (blob files
          // are always relative).
          return RefId{fs::canonical(p).native(), fileSize, 0};
        })
    ) +
    (from(blobs)
     | mapped([&] (const std::string& b) {
         if (b.size() <= g_inline_size) {
           FTRACE(4, "{} storing blob inline\n",
                  requestId.tracePrefix());
           return RefId{b, b.size(), 0};
         }
         FTRACE(4, "{} writing size {} blob to {}\n",
                requestId.tracePrefix(), b.size(), fd->path().native());
         auto const offset = fd->append(b);
         RefId r{fd->path().filename().native(), b.size(), offset};
         FTRACE(4, "{} written as {}\n", requestId.tracePrefix(), r.toString());
         return r;
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
                     const folly::Range<const OutputType*>* finiOutput,
                     Client::ExecMetadata) {
  FTRACE(4, "{} executing \"{}\" ({} runs)\n",
         requestId.tracePrefix(), command, inputs.size());

  HPHP_CORO_SAFE_POINT;

  // Each set of inputs should always have the same size.
  if (debug && !inputs.empty()) {
    auto const size = inputs[0].size();
    for (size_t i = 1; i < inputs.size(); ++i) {
      always_assert(inputs[i].size() == size);
    }
  }

  // Encode all the inputs
  BlobEncoder encoder;
  auto const encodeParams = [&] (const RefValVec& params) {
    for (auto const& param : params) {
      match<void>(
        param,
        [&] (const RefId& id) {
          SerializedSink::encodeRefId(id, encoder);
        },
        [&] (const Optional<RefId>& id) {
          SerializedSink::encodeOptRefId(id, encoder);
        },
        [&] (const IdVec& ids) {
          SerializedSink::encodeRefIdVec(ids, encoder);
        }
      );
    }
  };
  encodeParams(config);
  encoder((size_t)inputs.size());
  for (auto const& input : inputs) encodeParams(input);

  // Acquire a FD. We're not actually going to write to it, but the
  // worker will. This ensures the worker has exclusive access to the
  // file while it's running.
  auto fd = m_fdManager->acquireForAppend();
  SCOPE_EXIT { if (fd) m_fdManager->release(*fd); };

  // Do the actual fork+exec.
  auto const outputBlob = HPHP_CORO_AWAIT(
    doSubprocess(
      requestId,
      command,
      std::string{(const char*)encoder.data(), encoder.size()},
      fd->path()
    )
  );
  // The worker (maybe) wrote to the file, so we need to re-sync our
  // tracked offset.
  fd->syncOffset();

  // Decode the output
  BlobDecoder decoder{outputBlob.data(), outputBlob.size()};
  auto const makeOutput = [&] (OutputType type) -> RefVal {
    switch (type) {
      case OutputType::Val:
        return SerializedSource::decodeRefId(decoder);
      case OutputType::Opt:
        return SerializedSource::decodeOptRefId(decoder);
      case OutputType::Vec:
        return SerializedSource::decodeRefIdVec(decoder);
    }
    always_assert(false);
  };

  auto const makeOutputs = [&] (const folly::Range<const OutputType*>& types) {
    RefValVec vec;
    vec.reserve(types.size());
    for (auto const type : types) {
      vec.emplace_back(makeOutput(type));
    }
    return vec;
  };

  std::vector<RefValVec> out;
  out.reserve(inputs.size() + (finiOutput ? 1 : 0));
  std::generate_n(
    std::back_inserter(out),
    inputs.size(),
    [&] { return makeOutputs(output); }
  );
  if (finiOutput) out.emplace_back(makeOutputs(*finiOutput));

  decoder.assertDone();
  HPHP_CORO_MOVE_RETURN(out);
}

coro::Task<std::string>
SubprocessImpl::doSubprocess(const RequestId& requestId,
                             const std::string& command,
                             std::string inputBlob,
                             const fs::path& outputPath) {
  std::vector<std::string> args{
    current_executable_path(),
    s_option,
    g_local_option,
    command,
    m_fdManager->root().native(),
    outputPath.native()
  };

  // Propagate the TRACE option in the environment. We'll copy the
  // trace output into this process' trace output.
  std::vector<std::string> env;
  Optional<fs::path> traceFile;
  SCOPE_EXIT { if (traceFile) m_traceManager.put(std::move(*traceFile)); };
  if (auto const trace = getenv("TRACE")) {
    traceFile = m_traceManager.get();
    auto const fullPath = m_fdManager->root() / *traceFile;
    env.emplace_back(folly::sformat("TRACE={}", trace));
    env.emplace_back(folly::sformat("HPHP_TRACE_FILE={}", fullPath.c_str()));
  }

  FTRACE(
    4, "{} executing subprocess for '{}'{}(input size: {})\n",
    requestId.tracePrefix(),
    command,
    traceFile
      ? folly::sformat(" (trace-file: {}) ", traceFile->native())
      : "",
    inputBlob.size()
  );

  HPHP_CORO_SAFE_POINT;

  auto const before = std::chrono::steady_clock::now();

  // Do the actual fork+exec.
  folly::Subprocess subprocess{
    args,
    folly::Subprocess::Options{}
      .parentDeathSignal(SIGKILL)
      .pipeStdin()
      .pipeStdout()
      .pipeStderr()
      .fd(g_local_pipe_fd, folly::Subprocess::PIPE_OUT)
      .closeOtherFds(),
    nullptr,
    &env
  };

  std::string output;
  std::string stderr;
  size_t inputWritten = 0;

  // Communicate with the worker. When available, read from worker's
  // stderr and output pipe and store the data. Throw everything else
  // away. Attempt to write inputs to the worker's stdin whenever it
  // has free space.
  subprocess.communicate(
    [&] (int parentFd, int childFd) { // Read
      if (childFd == g_local_pipe_fd) {
        return readFromPipe(parentFd, output);
      } else if (childFd == STDERR_FILENO) {
        return readFromPipe(parentFd, stderr);
      } else {
        // Worker is writing to some other random FD (including
        // stdout). Ignore it.
        char dummy[512];
        folly::readNoInt(parentFd, dummy, sizeof dummy);
      }
      return false;
    },
    [&] (int parentFd, int childFd) { // Write
      // Close any writable FD except stdin
      if (childFd != STDIN_FILENO) return true;
      // We've written all of the input, so close stdin. This will
      // signal to the worker that input is all sent.
      if (inputWritten >= inputBlob.size()) return true;

      // Otherwise write what we can
      auto const toWrite = inputBlob.size() - inputWritten;
      auto const written =
        folly::writeNoInt(parentFd, inputBlob.data() + inputWritten, toWrite);
      if (written < 0) {
        if (errno == EAGAIN) return false;
        throw Error{
          folly::sformat(
            "Failed writing {} bytes to subprocess input for '{}' [{}]",
            toWrite, command, folly::errnoStr(errno)
          )
        };
      } else if (written == 0) {
        return true;
      }
      inputWritten += written;
      return false;
    }
  );

  auto const returnCode = subprocess.wait();
  auto const elapsed = std::chrono::steady_clock::now() - before;

  stats().execCpuUsec +=
    std::chrono::duration_cast<std::chrono::microseconds>(elapsed).count();
  ++stats().execAllocatedCores;

  FTRACE(
    4,
    "{} subprocess finished (took {}). "
    "Output size: {}, Return code: {}, Stderr: {}\n",
    requestId.tracePrefix(),
    prettyPrint(
      std::chrono::duration_cast<std::chrono::duration<double>>(
        elapsed
      ).count(),
      folly::PRETTY_TIME,
      false
    ),
    output.size(),
    returnCode.str(),
    stderr
  );

  HPHP_CORO_SAFE_POINT;

  // Do this before checking the return code. If the process failed,
  // we want to capture anything it logged before throwing.
  if (traceFile && fs::exists(m_fdManager->root() / *traceFile)) {
    auto const contents = readFile(m_fdManager->root() / *traceFile);
    if (!contents.empty()) {
      Trace::ftraceRelease(
        "vvvvvvvvvvvvvvvvvv remote-exec ({} \"{}\") vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv\n"
        "{}"
        "^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^\n",
        requestId.toString(),
        command,
        contents
      );
    }
  }

  if (!returnCode.exited() || returnCode.exitStatus() != 0) {
    throw WorkerError{
      folly::sformat(
        "Execution of `{}` failed: {}\nstderr:\n{}",
        command,
        returnCode.str(),
        stderr
      )
    };
  }
  if (inputWritten != inputBlob.size()) {
    throw WorkerError{
      folly::sformat(
        "Execution of `{}` failed: Process failed to consume input\n",
        command
      )
    };
  }

  HPHP_CORO_MOVE_RETURN(output);
}

//////////////////////////////////////////////////////////////////////

SubprocessImpl::FDManager::FDManager(fs::path root)
  : m_root{std::move(root)}
  , m_nextBlob{0}
{}

const FD* SubprocessImpl::FDManager::acquireForRead(const fs::path& path) {
  // Blob files are always relative
  if (path.is_absolute()) return nullptr;
  // Only already created blob files should be in m_fds. So, if it's
  // not present, it can't be a blob file.
  auto const it = m_fds.find(path.native());
  if (it == m_fds.end()) return nullptr;
  return it->second.get();
}

FD* SubprocessImpl::FDManager::acquireForAppend() {
  // Use the blob file at the top of m_forAppend.
  std::scoped_lock<std::mutex> _{m_lock};
  if (m_forAppend.empty()) {
    // If empty, there's no free blob file. We need to create a new
    // one.
    auto fd = newFD();
    auto const path = fd->path();
    auto const [elem, emplaced] =
      m_fds.emplace(path.filename().native(), std::move(fd));
    assertx(emplaced);
    m_forAppend.push(elem->second.get());
  }
  auto fd = m_forAppend.top();
  m_forAppend.pop();
  return fd;
}

void SubprocessImpl::FDManager::release(FD& fd) {
  if (debug) {
    // Should be returning something cached
    auto const it = m_fds.find(fd.path().filename().native());
    always_assert(it != m_fds.end());
    always_assert(it->second.get() == &fd);
  }
  std::scoped_lock<std::mutex> _{m_lock};
  m_forAppend.push(&fd);
}

std::unique_ptr<FD> SubprocessImpl::FDManager::newFD() {
  // We deliberately keep the blob filename as small as possible
  // because they get serialized a lot and it keeps the input/output
  // sizes small.
  auto const id = m_nextBlob++;
  auto const filename = m_root / folly::to<std::string>(id);
  return std::make_unique<FD>(filename, true, true, true);
}

//////////////////////////////////////////////////////////////////////

fs::path SubprocessImpl::TraceFileManager::get() {
  std::scoped_lock<std::mutex> _{m_lock};
  if (m_paths.empty()) {
    auto const id = m_nextId++;
    m_paths.push(folly::sformat("trace-{:04}.log", id));
  }
  auto path = std::move(m_paths.top());
  m_paths.pop();
  return path;
}

void SubprocessImpl::TraceFileManager::put(fs::path path) {
  std::scoped_lock<std::mutex> _{m_lock};
  m_paths.push(std::move(path));
}

//////////////////////////////////////////////////////////////////////

}

//////////////////////////////////////////////////////////////////////

Client::Client(folly::Executor::KeepAlive<> executor,
               const Options& options)
  : m_options{options}
  , m_stats{std::make_shared<Stats>()}
  , m_forceFallback{false}
  , m_fallbackSem{1}
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

  auto const pct = [] (size_t a, size_t b) -> std::string {
    if (!b) return "--";
    return folly::sformat("{:.2f}%", double(a) / b * 100.0);
  };

  auto const execWorkItems_ = execWorkItems.load();
  auto const allocatedCores = execAllocatedCores.load();
  auto const cpuUsecs = execCpuUsec.load();
  auto const execCalls_ = execCalls.load();
  auto const storeCalls_ = storeCalls.load();
  auto const loadCalls_ = loadCalls.load();

  return folly::sformat(
    "  {}:{}\n"
    "  Execs: {:,} ({:,}) total, {:,} cache-hits ({}), {:,} optimistically, {:,} fallback\n"
    "  Files: {:,} total, {:,} read, {:,} queried, {:,} uploaded ({}), {:,} fallback\n"
    "  Blobs: {:,} total, {:,} queried, {:,} uploaded ({}), {:,} fallback\n"
    "  Cpu: {} usage, {:,} allocated cores ({}/core)\n"
    "  Mem: {} max used, {} reserved\n"
    "  {:,} downloads ({}), {:,} throttles\n"
    "  Avg Latency: {} exec, {} store, {} load",
    phase,
    extra.empty() ? "" : folly::sformat(" {}", extra),
    execCalls_,
    execWorkItems_,
    execCacheHits.load(),
    pct(execCacheHits.load(), execCalls_),
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
  sample.setInt(phase + "_total_execs", execCalls.load());
  sample.setInt(phase + "_total_exec_work_items", execWorkItems.load());
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
