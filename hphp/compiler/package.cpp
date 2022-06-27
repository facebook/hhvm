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

#include "hphp/compiler/package.h"

#include <exception>
#include <fstream>
#include <map>
#include <memory>
#include <set>
#include <sys/stat.h>
#include <sys/types.h>
#include <utility>
#include <vector>

#include <boost/filesystem.hpp>

#include <folly/String.h>
#include <folly/portability/Dirent.h>
#include <folly/portability/Unistd.h>

#include "hphp/compiler/analysis/analysis_result.h"
#include "hphp/compiler/option.h"
#include "hphp/hhvm/process-init.h"
#include "hphp/runtime/base/coeffects-config.h"
#include "hphp/runtime/base/execution-context.h"
#include "hphp/runtime/base/file-util-defs.h"
#include "hphp/runtime/base/file-util.h"
#include "hphp/runtime/base/program-functions.h"
#include "hphp/runtime/vm/as.h"
#include "hphp/runtime/vm/func-emitter.h"
#include "hphp/runtime/vm/unit-emitter.h"
#include "hphp/runtime/vm/unit-parser.h"
#include "hphp/util/exception.h"
#include "hphp/util/extern-worker.h"
#include "hphp/util/hash.h"
#include "hphp/util/logger.h"
#include "hphp/util/process.h"
#include "hphp/util/timer.h"
#include "hphp/zend/zend-string.h"

using namespace HPHP;
using namespace extern_worker;

namespace fs = boost::filesystem;

///////////////////////////////////////////////////////////////////////////////

const StaticString s_EntryPoint("__EntryPoint");

///////////////////////////////////////////////////////////////////////////////

// Configuration for parse workers. This should contain any runtime
// options which can affect HackC (or the interface to it).
struct Package::Config {
  Config() = default;

  static Config make() {
    Config c;
    #define R(Opt) c.Opt = RO::Opt;
    UNITCACHEFLAGS()
    #undef R
    c.EvalAbortBuildOnCompilerError = RO::EvalAbortBuildOnCompilerError;
    c.EvalAbortBuildOnVerifyError = RO::EvalAbortBuildOnVerifyError;
    c.IncludeRoots = RO::IncludeRoots;
    c.coeffects = CoeffectsConfig::exportForParse();
    return c;
  }

  void apply() const {
    #define R(Opt) RO::Opt = Opt;
    UNITCACHEFLAGS()
    #undef R
    RO::EvalAbortBuildOnCompilerError = EvalAbortBuildOnCompilerError;
    RO::EvalAbortBuildOnVerifyError = EvalAbortBuildOnVerifyError;
    RO::IncludeRoots = IncludeRoots;
    CoeffectsConfig::importForParse(coeffects);
  }

  template <typename SerDe> void serde(SerDe& sd) {
    #define R(Opt) sd(Opt);
    UNITCACHEFLAGS()
    #undef R
    sd(EvalAbortBuildOnCompilerError)
      (EvalAbortBuildOnVerifyError)
      (IncludeRoots)
      (coeffects);
  }

private:
  #define R(Opt) decltype(RuntimeOption::Opt) Opt;
  UNITCACHEFLAGS()
  #undef R
  bool EvalAbortBuildOnCompilerError;
  bool EvalAbortBuildOnVerifyError;
  decltype(RO::IncludeRoots) IncludeRoots;
  CoeffectsConfig coeffects;
};

///////////////////////////////////////////////////////////////////////////////

Package::AsyncState::AsyncState()
  : m_executor{
      "HPHPcWorker",
      0,
      size_t(Option::ParserThreadCount <= 0 ? 1 : Option::ParserThreadCount),
      [] {
        hphp_thread_init();
        g_context.getCheck();
      },
      [] { hphp_thread_exit(); },
      std::chrono::minutes{15}
    }
  , m_client{m_executor.sticky(), makeOptions()}
  , m_config{
      [this] { return m_client.store(Config::make()); },
      m_executor.sticky()
    }
  , m_repoOptions{m_client}
{
}

Options Package::AsyncState::makeOptions() {
  Options options;
  options
    .setUseCase(Option::ExternWorkerUseCase)
    .setUseSubprocess(Option::ExternWorkerForceSubprocess
                        ? Options::UseSubprocess::Always
                        : Options::UseSubprocess::Fallback)
    .setCacheExecs(Option::ExternWorkerUseExecCache)
    .setCleanup(Option::ExternWorkerCleanup)
    .setUseEdenFS(RuntimeOption::EvalUseEdenFS)
    .setUseRichClient(Option::ExternWorkerUseRichClient)
    .setUseZippyRichClient(Option::ExternWorkerUseZippyRichClient)
    .setUseP2P(Option::ExternWorkerUseP2P);
  if (Option::ExternWorkerTimeoutSecs > 0) {
    options.setTimeout(std::chrono::seconds{Option::ExternWorkerTimeoutSecs});
  }
  if (!Option::ExternWorkerWorkingDir.empty()) {
    options.setWorkingDir(Option::ExternWorkerWorkingDir);
  }
  return options;
}

///////////////////////////////////////////////////////////////////////////////

Package::Package(const char* root, bool parseOnDemand)
  : m_parseFailed{false}
  , m_parseOnDemand{parseOnDemand}
  , m_total{0}
{
  m_root = FileUtil::normalizeDir(root);
  m_ar = std::make_shared<AnalysisResult>();
  m_fileCache = std::make_shared<FileCache>();
}

void Package::createAsyncState() {
  assertx(!m_async);
  m_async = std::make_unique<AsyncState>();
}

Optional<std::thread> Package::clearAsyncState() {
  if (!m_async) return std::nullopt;
  if (!Option::ParserAsyncCleanup) {
    // If we don't want to cleanup asynchronously, do so now.
    m_async.reset();
    return std::nullopt;
  }
  // All the thread does is reset the unique_ptr to run the dtor.
  return std::thread{
    [a = std::move(m_async)] () mutable { a.reset(); }
  };
}

void Package::addInputList(const std::string& listFileName) {
  assert(!listFileName.empty());
  auto const f = fopen(listFileName.c_str(), "r");
  if (f == nullptr) {
    throw Exception("Unable to open %s: %s", listFileName.c_str(),
                    folly::errnoStr(errno).c_str());
  }
  char fileName[PATH_MAX];
  while (fgets(fileName, sizeof(fileName), f)) {
    int len = strlen(fileName);
    if (fileName[len - 1] == '\n') fileName[len - 1] = '\0';
    len = strlen(fileName);
    if (len) {
      if (FileUtil::isDirSeparator(fileName[len - 1])) {
        addDirectory(fileName);
      } else {
        addSourceFile(fileName);
      }
    }
  }
  fclose(f);
}

void Package::addStaticFile(const std::string& fileName) {
  assert(!fileName.empty());
  m_extraStaticFiles.insert(fileName);
}

void Package::addStaticDirectory(const std::string& path) {
  m_staticDirectories.insert(path);
}

void Package::addDirectory(const std::string& path) {
  m_directories.emplace(path);
}

void Package::addSourceFile(const std::string& fileName) {
  if (fileName.empty()) return;
  auto canonFileName =
    FileUtil::canonicalize(String(fileName)).toCppString();
  m_filesToParse.emplace(std::move(canonFileName), true);
}

std::shared_ptr<FileCache> Package::getFileCache() {
  for (auto const& dir : m_directories) {
    std::vector<std::string> files;
    FileUtil::find(files, m_root, dir, /* php */ false,
                   &Option::PackageExcludeStaticDirs,
                   &Option::PackageExcludeStaticFiles);
    Option::FilterFiles(files, Option::PackageExcludeStaticPatterns);
    for (auto& file : files) {
      auto const rpath = file.substr(m_root.size());
      if (!m_fileCache->fileExists(rpath.c_str())) {
        Logger::Verbose("saving %s", file.c_str());
        m_fileCache->write(rpath.c_str(), file.c_str());
      }
    }
  }
  for (auto const& dir : m_staticDirectories) {
    std::vector<std::string> files;
    FileUtil::find(files, m_root, dir, /* php */ false);
    for (auto& file : files) {
      auto const rpath = file.substr(m_root.size());
      if (!m_fileCache->fileExists(rpath.c_str())) {
        Logger::Verbose("saving %s", file.c_str());
        m_fileCache->write(rpath.c_str(), file.c_str());
      }
    }
  }
  for (auto const& file : m_extraStaticFiles) {
    if (!m_fileCache->fileExists(file.c_str())) {
      auto const fullpath = m_root + file;
      Logger::Verbose("saving %s", fullpath.c_str());
      m_fileCache->write(file.c_str(), fullpath.c_str());
    }
  }

  for (auto const& pair : m_discoveredStaticFiles) {
    auto const file = pair.first.c_str();
    if (!m_fileCache->fileExists(file)) {
      const char *fullpath = pair.second.c_str();
      Logger::Verbose("saving %s", fullpath[0] ? fullpath : file);
      if (fullpath[0]) {
        m_fileCache->write(file, fullpath);
      } else {
        m_fileCache->write(file);
      }
    }
  }

  return m_fileCache;
}

///////////////////////////////////////////////////////////////////////////////

namespace {

std::unique_ptr<UnitEmitter>
createSymlinkWrapper(const std::string& fileName,
                     const std::string& targetPath,
                     std::unique_ptr<UnitEmitter> origUE) {
  auto found = false;

  std::ostringstream ss;
  for (auto const& fe : origUE->fevec()) {
    auto const& attrs = fe->userAttributes;
    if (attrs.find(s_EntryPoint.get()) != attrs.end()) {
      found = true;
      std::string escapedName;
      folly::cEscape(fe->name->toCppString(), escapedName);
      ss << ".function{} [unique persistent "
        "\"__EntryPoint\"(\"\"\"y:0:{}\"\"\")] (4,7) <\"\" N  > "
        "entrypoint$symlink$" << string_sha1(fileName) << "() {\n"
         << "  String \"" << targetPath << "\"\n"
         << "  ReqOnce\n"
         << "  PopC\n"
         << "  NullUninit\n"
         << "  NullUninit\n"
         << "  FCallFuncD <> 0 1 \"\" \"\" - \"\" \"" << escapedName << "\"\n"
         << "  PopC\n"
         << "  Null\n"
         << "  RetC\n"
         << "}\n\n";
      break;
    }
  }
  if (!found) return nullptr;

  auto const content = ss.str();
  return assemble_string(
    content.data(),
    content.size(),
    fileName.c_str(),
    SHA1{string_sha1(content)},
    Native::s_noNativeFuncs,
    false
  );
}

///////////////////////////////////////////////////////////////////////////////

// Metadata for a parse job. Just filename things that we need to
// resolve when we have the whole source tree available.
struct FileMeta {
  FileMeta() = default;
  FileMeta(std::string f, Optional<std::string> o)
    : m_filename{std::move(f)}, m_targetPath{std::move(o)} {}

  // The (relative) filename of the file
  std::string m_filename;
  // If the file is a symlink, what its target is
  Optional<std::string> m_targetPath;

  template <typename SerDe> void serde(SerDe& sd) {
    sd(m_filename)
      (m_targetPath);
  }
};

// Wraps an unique_ptr<UnitEmitter> as a return value from a parse job
// and its serialization logic.
struct UnitEmitterWrapper {
  UnitEmitterWrapper() = default;
  /* implicit */ UnitEmitterWrapper(std::unique_ptr<UnitEmitter> ue)
    : m_ue{std::move(ue)} {}

  std::unique_ptr<UnitEmitter> m_ue;

  template <typename SerDe> void serde(SerDe& sd) {
    if constexpr (SerDe::deserializing) {
      assertx(!m_ue);

      bool present;
      sd(present);
      if (present) {
        SHA1 sha1;
        const StringData* filepath;
        sd(sha1);
        sd(filepath);

        auto ue = std::make_unique<UnitEmitter>(
          sha1, SHA1{}, Native::s_noNativeFuncs
        );
        ue->m_filepath = makeStaticString(filepath);
        ue->serde(sd, false);
        m_ue = std::move(ue);
      }
    } else {
      if (m_ue) {
        sd(true);
        sd(m_ue->sha1());
        sd(m_ue->m_filepath);
        m_ue->serde(sd, false);
      } else {
        sd(false);
      }
    }
  }
};

// Metadata about the UnitEmitter obtained during parsing. This is
// returned separately from the UnitEmitter, since we need this to
// find new files for parse on-demand, and we can avoid loading the
// entire UnitEmitter just to get this.
struct ParseMeta {
  // Symbols present in the unit. This will be used to find new files
  // for parse on-demand.
  SymbolRefs m_symbol_refs;
  // If not empty, parsing resulted in an ICE and configuration
  // indicated that this should be fatal.
  std::string m_abort;

  template <typename SerDe> void serde(SerDe& sd) {
    sd(m_symbol_refs)
      (m_abort);
  }
};

// Extern-worker job for parsing a source file into an UnitEmitter
// (and some other metadata).
struct ParseJob {
  static std::string name() { return "hphpc-parse"; }

  static void init(const Package::Config& config) {
    rds::local::init();

    Hdf hdf;
    IniSetting::Map ini = IniSetting::Map::object;
    RO::Load(ini, hdf);

    config.apply();
    Logger::LogLevel = Logger::LogError;

    // Inhibit extensions and systemlib from being initialized. It
    // takes a while and we don't need it.
    register_process_init(true);
    hphp_process_init(true);

    // Don't use unit emitter's caching here, we're relying on
    // extern-worker to do that for us.
    g_unit_emitter_cache_hook = nullptr;

    // This is a lie, but a lot of stuff breaks if you don't set it to
    // true (when false, everything assumes you're parsing systemlib).
    SystemLib::s_inited = true;
  }
  static void fini() {
    hphp_process_exit();
    rds::local::fini();
  }

  static Multi<ParseMeta, UnitEmitterWrapper>
  makeOutput(std::unique_ptr<UnitEmitter> ue) {
    if (!ue) return std::make_tuple(ParseMeta{}, UnitEmitterWrapper{});
    auto symbolRefs = std::move(ue->m_symbol_refs);
    return std::make_tuple(ParseMeta{std::move(symbolRefs)}, std::move(ue));
  }

  static Multi<ParseMeta, UnitEmitterWrapper> run(
      const std::string& content,
      const FileMeta& meta,
      const RepoOptionsFlags& repoOptions) {
    auto const& fileName = meta.m_filename;

    try {
      if (RO::EvalAllowHhas) {
        if (fileName.size() > 5 &&
            !fileName.compare(fileName.size() - 5, std::string::npos, ".hhas")) {
          auto ue = assemble_string(
            content.data(),
            content.size(),
            fileName.c_str(),
            SHA1{string_sha1(content)},
            Native::s_noNativeFuncs
          );
          if (meta.m_targetPath) {
            ue = createSymlinkWrapper(
              fileName, *meta.m_targetPath, std::move(ue)
            );
            if (!ue) {
              // If the symlink contains no EntryPoint we don't do
              // anything but it is still success
              return makeOutput(nullptr);
            }
          }
          return makeOutput(std::move(ue));
        }
      }

      LazyUnitContentsLoader loader{
        SHA1{mangleUnitSha1(string_sha1(content), fileName, repoOptions)},
        content,
        repoOptions
      };

      auto const mode =
        RO::EvalAbortBuildOnCompilerError ? CompileAbortMode::AllErrors :
        RO::EvalAbortBuildOnVerifyError   ? CompileAbortMode::VerifyErrors :
        CompileAbortMode::OnlyICE;

      auto uc = UnitCompiler::create(
        loader,
        fileName.c_str(),
        Native::s_noNativeFuncs,
        nullptr, // TODO: repo mode support for decl providers
        false,
        false
      );
      assertx(uc);

      std::unique_ptr<UnitEmitter> ue;
      try {
        auto cacheHit = false;
        ue = uc->compile(cacheHit, mode);
        // We disabled UnitCompiler caching, so we shouldn't have any
        // hits.
        assertx(!cacheHit);
      } catch (const CompilerAbort& exn) {
        return std::make_tuple(ParseMeta{{}, exn.what()}, UnitEmitterWrapper{});
      }

      if (ue) {
        if (!ue->m_ICE && meta.m_targetPath) {
          ue =
            createSymlinkWrapper(fileName, *meta.m_targetPath, std::move(ue));
          if (!ue) {
            // If the symlink contains no EntryPoint we don't do anything but it
            // is still success
            return makeOutput(nullptr);
          }
        }
        return makeOutput(std::move(ue));
      } else {
        throw Error{
          folly::sformat(
            "Unable to compile using {} compiler: {}",
            uc->getName(),
            fileName
          )
        };
      }
    } catch (const std::exception& exn) {
      throw Error{
        folly::sformat("While parsing `{}`: {}", fileName, exn.what())
      };
    }
  }
};
Job<ParseJob> g_parseJob;

}

///////////////////////////////////////////////////////////////////////////////

/*
 * File grouping:
 *
 * Since every invocation of an extern-worker worker has some fixed
 * overhead, we want to parse multiple files per invocation. We also
 * want to leverage any caching that extern-worker has for job
 * execution. Since we assume that source files will change over time,
 * we don't want to group too many files together (if one file
 * changes, we'll have to reparse all of them in that
 * group). Furthermore, to maximize cache effectiveness, we want to
 * group files together in a deterministic way. Finally, there may be
 * different "subsections" of the source tree, which are only parsed
 * depending on the input files configeration (for example, some
 * builds may discard test directories and some might not). Again, we
 * want to maximize caching across these different "flavor" of builds
 * and try to avoid grouping together files from these different
 * subsets.
 *
 * We utilize the following scheme to try to accomplish all
 * this. First we define a group size (Option::ParserGroupSize). This
 * is the amount of files (on average) we'll group together in one
 * job. Input files are discovered by walking directories
 * recursively. We proceed bottom up. For every directory, we first
 * process its sub-directories. Each sub-directory processed returns
 * the groups it has already created (each roughly containing
 * Option::ParserGroupSize) files, along with any "left over" files
 * which have not been grouped. These results are all aggregated
 * together, and any files in the current directory are added to the
 * ungrouped set. If the number of files in the ungrouped set exceeds
 * Option::ParserDirGroupSizeLimit, then we attempt to group all of
 * those files.
 *
 * Grouping is done by hashing the files' names, and then using
 * consistent_hash to assign them to one of N buckets (where N is the
 * number of files divided by Option::ParserGroupSize rounded up). The
 * consistent hashing ensures that the minimal amount of disruption
 * happens when we add/remove files. (Adding one file will change
 * exactly one bucket, etc).
 *
 * If we grouped the files, they are returned to the parent directory
 * as groups (along with any groups from sub-directories). Otherwise
 * the files are returned as ungrouped and the process repeats in the
 * parent.
 *
 * The idea behind Option::ParserDirGroupSizeLimit is to try to
 * partition the source tree into distinct chunks and only grouping
 * files within those chunks. So, if you don't compile one of those
 * chunks (say because you're not compiling tests, for example), it
 * won't affect the files in other chunks. Otherwise if that test code
 * was mixed in with the rest of the groups, they'd all miss the cache
 * and have to be rerun. This is a heuristic, but in practice it seems
 * to work well.
 *
 * Once you reach the top level, any remaining ungrouped files (along
 * with any top level files added in by config) are grouped together.
 *
 * Before parsing, we sort all of the groups by their summed file
 * size. We want to start parsing larger groups first because they'll
 * probably take the longest.
 */

// Given the path of a directory, find all (relevant) files in that
// directory (and sub-directories), and attempt to group them.
coro::Task<Package::GroupResult>
Package::groupDirectories(std::string path) {
  // We're not going to be blocking on I/O here, so make sure we're
  // running on the thread pool.
  HPHP_CORO_RESCHEDULE_ON_CURRENT_EXECUTOR;

  GroupResult result;
  std::vector<coro::Task<GroupResult>> dirs;

  FileUtil::find(
    m_root, path, /* php */ true,
    [&] (const std::string& name, bool dir, size_t size) {
      if (!dir) {
        if (Option::PackageExcludeFiles.count(name) ||
            Option::IsFileExcluded(name, Option::PackageExcludePatterns)) {
          return false;
        }

        if (!name.empty()) {
          auto canonFileName =
            FileUtil::canonicalize(String(name)).toCppString();
          if (m_parsedFiles.emplace(std::move(canonFileName), true).second) {
            result.m_ungrouped.emplace_back(FileAndSize{name, size});
          }
        }
        return true;
      }
      if (Option::PackageExcludeDirs.count(name)) return false;
      if (path == name ||
          (name.size() == path.size() + 1 &&
           name.back() == FileUtil::getDirSeparator() &&
           name.compare(0, path.size(), path) == 0)) {
        // find immediately calls us back with a canonicalized version
        // of path; we want to ignore that, and let it proceed to
        // iterate the directory.
        return true;
      }

      // Process the directory as a new job
      dirs.emplace_back(groupDirectories(name));

      // Don't iterate the directory in this job.
      return false;
    }
  );

  // Coalesce the sub results
  for (auto& sub : HPHP_CORO_AWAIT(coro::collectRange(std::move(dirs)))) {
    result.m_grouped.insert(
      result.m_grouped.end(),
      std::make_move_iterator(sub.m_grouped.begin()),
      std::make_move_iterator(sub.m_grouped.end())
    );
    result.m_ungrouped.insert(
      result.m_ungrouped.end(),
      std::make_move_iterator(sub.m_ungrouped.begin()),
      std::make_move_iterator(sub.m_ungrouped.end())
    );
  }

  // Have we gathered enough files to assign them to groups?
  if (result.m_ungrouped.size() >= Option::ParserDirGroupSizeLimit) {
    groupFiles(result.m_grouped, std::move(result.m_ungrouped));
    assertx(result.m_ungrouped.empty());
  }
  HPHP_CORO_MOVE_RETURN(result);
}

// Group sets of files together using consistent hashing
void Package::groupFiles(ParseGroups& groups,
                         FileAndSizeVec files) {
  if (files.empty()) return;

  assertx(Option::ParserGroupSize > 0);
  // Number of buckets
  auto const numNew =
    (files.size() + (Option::ParserGroupSize - 1)) / Option::ParserGroupSize;

  auto const origSize = groups.size();
  groups.resize(origSize + numNew);

  // Assign to buckets
  for (auto& [file, size] : files) {
    auto const idx = consistent_hash(
      hash_string_cs(file.c_str(), file.native().size()),
      numNew
    );
    assertx(idx < numNew);
    groups[origSize + idx].m_files.emplace_back(std::move(file));
    groups[origSize + idx].m_size += size;
  }

  // We could (though unlikely) have empty buckets. Remove them so we
  // don't have to deal with this when parsing.
  groups.erase(
    std::remove_if(
      groups.begin() + origSize,
      groups.end(),
      [] (const ParseGroup& g) { return g.m_files.empty(); }
    ),
    groups.end()
  );

  // Keep the order of the files within the bucket deterministic
  for (size_t i = origSize; i < groups.size(); ++i) {
    std::sort(groups[i].m_files.begin(), groups[i].m_files.end());
  }
}

// Parse all of the files in the given group, returning a vector of
// "ondemand" files obtained from that parsing.
coro::Task<Package::FileAndSizeVec> Package::parseGroups(ParseGroups groups) {
  if (groups.empty()) HPHP_CORO_RETURN(FileAndSizeVec{});

  // Parse the groups from highest combined file size to lowest. The
  // larger groups will probably take longer to compile, so we want to
  // start those earliest.
  std::sort(
    groups.begin(),
    groups.end(),
    [] (const ParseGroup& a, const ParseGroup& b) {
      if (a.m_size != b.m_size) return b.m_size < a.m_size;
      if (a.m_files.size() != b.m_files.size()) {
        return b.m_files.size() < a.m_files.size();
      }
      return a.m_files < b.m_files;
    }
  );

  // Kick off the parsing. Each group gets its own sticky ticket (so
  // earlier groups will get scheduling priority over later ones).
  std::vector<coro::TaskWithExecutor<FileAndSizeVec>> tasks;
  for (auto& group : groups) {
    tasks.emplace_back(
      parseGroup(std::move(group))
        .scheduleOn(m_async->m_executor.sticky())
    );
  }

  // Gather the on-demand files and return them
  FileAndSizeVec ondemand;
  for (auto& paths : HPHP_CORO_AWAIT(coro::collectRange(std::move(tasks)))) {
    ondemand.insert(
      ondemand.end(),
      std::make_move_iterator(paths.begin()),
      std::make_move_iterator(paths.end())
    );
  }

  HPHP_CORO_MOVE_RETURN(ondemand);
}

// The actual parse loop. Find the initial set of inputs (from
// configuration), parse them, gather on-demand files, then repeat the
// process until we have no new files to parse. Everything "under"
// this call works asynchronously.
void Package::parseAll() {
  auto work = coro::invoke(
    [this] () -> coro::Task<void> {
      // Find the initial set of groups
      auto groups = HPHP_CORO_AWAIT(coro::invoke(
        [&] () -> coro::Task<ParseGroups> {

        Timer timer{Timer::WallTime, "finding inputs"};

        std::vector<coro::Task<GroupResult>> tasks;
        for (auto& dir : m_directories) {
          tasks.emplace_back(groupDirectories(std::move(dir)));
        }

        // Gather together all top level files
        GroupResult top;
        for (auto& result :
               HPHP_CORO_AWAIT(coro::collectRange(std::move(tasks)))) {
          top.m_grouped.insert(
            top.m_grouped.end(),
            std::make_move_iterator(result.m_grouped.begin()),
            std::make_move_iterator(result.m_grouped.end())
          );
          top.m_ungrouped.insert(
            top.m_ungrouped.end(),
            std::make_move_iterator(result.m_ungrouped.begin()),
            std::make_move_iterator(result.m_ungrouped.end())
          );
        }

        // If there's any ungrouped files left over, group those now
        groupFiles(top.m_grouped, std::move(top.m_ungrouped));
        assertx(top.m_ungrouped.empty());

        // Finally add in any files explicitly added via configuration
        // and group them.
        FileAndSizeVec extraFiles;
        for (auto& file : m_filesToParse) {
          if (!m_parsedFiles.insert(file).second) continue;
          extraFiles.emplace_back(FileAndSize{std::move(file.first), 0});
        }
        groupFiles(top.m_grouped, std::move(extraFiles));

        HPHP_CORO_RETURN(std::move(top.m_grouped));
      }));

      // Parse the "main" round and get any ondemand files
      FileAndSizeVec ondemand;
      {
        Timer timer{Timer::WallTime, "parsing inputs"};
        ondemand = HPHP_CORO_AWAIT(parseGroups(std::move(groups)));
        m_ar->sample().setInt("parsing_inputs_micros", timer.getMicroSeconds());
      }

      if (ondemand.empty()) HPHP_CORO_RETURN_VOID;

      Timer timer{Timer::WallTime, "parsing on-demand"};
      // We have ondemand files, so keep parsing until we have nothing
      // more to parse.
      do {
        assertx(groups.empty());
        groupFiles(groups, std::move(ondemand));
        ondemand = HPHP_CORO_AWAIT(parseGroups(std::move(groups)));
      } while (!ondemand.empty());
      m_ar->sample().setInt("parsing_ondemand_micros", timer.getMicroSeconds());

      HPHP_CORO_RETURN_VOID;
    }
  ).scheduleOn(m_async->m_executor.sticky());
  coro::wait(std::move(work));
}

bool Package::parse() {
  assertx(m_async);

  Logger::FInfo(
    "parsing using {} threads using {}{}",
    m_async->m_executor.numThreads(),
    m_async->m_client.implName(),
    coro::using_coros ? "" : " (coros disabled!)"
  );

  HphpSession _{Treadmill::SessionKind::CompilerEmit};

  // Treat any symbol refs from systemlib as if they were part of the
  // original Package.
  for (auto& ue : SystemLib::claimRegisteredUnitEmitters()) {
    FileAndSizeVec ondemand;
    resolveOnDemand(ondemand, ue->m_symbol_refs, true);
    for (auto const& p : ondemand) addSourceFile(p.m_path);
    addUnitEmitter(std::move(ue));
  }
  parseAll();
  return !m_parseFailed.load();
}

// Parse a group using extern-worker, hand off the UnitEmitter
// obtained, and return any on-demand files from the parsing.
coro::Task<Package::FileAndSizeVec> Package::parseGroup(ParseGroup group) {
  using namespace folly::gen;

  // Make sure we're running on the thread we should be
  HPHP_CORO_RESCHEDULE_ON_CURRENT_EXECUTOR;

  try {
    // First build the inputs for the job
    std::vector<folly::fs::path> paths;
    std::vector<FileMeta> metas;
    std::vector<coro::Task<Ref<RepoOptionsFlags>>> options;

    paths.reserve(group.m_files.size());
    metas.reserve(group.m_files.size());
    options.reserve(group.m_files.size());
    for (auto& fileName : group.m_files) {
      assertx(!fileName.empty());

      std::string fullPath;
      if (FileUtil::isDirSeparator(fileName.native().front())) {
        fullPath = fileName;
      } else {
        fullPath = m_root + fileName.native();
      }

      struct stat sb;

      auto const doStat = [&] {
        if (lstat(fullPath.c_str(), &sb)) {
          if (fullPath.find(' ') == std::string::npos) {
            Logger::Error("Unable to stat file %s", fullPath.c_str());
          }
          return false;
        }
        if ((sb.st_mode & S_IFMT) == S_IFDIR) {
          Logger::Error("Unable to parse directory: %s", fullPath.c_str());
          return false;
        }
        return true;
      }();
      if (!doStat) {
        Logger::FError("Fatal: Unable to stat/parse {}", fileName.native());
        m_parseFailed.store(true);
        continue;
      }

      if (!m_extraStaticFiles.count(fileName)) {
        m_discoveredStaticFiles.emplace(
          fileName,
          Option::CachePHPFile ? fullPath : ""
        );
      }

      Optional<std::string> targetPath;
      if (S_ISLNK(sb.st_mode)) {
        auto const target = fs::canonical(fullPath);
        targetPath.emplace(fs::relative(target, m_root).native());
      }

      // Most files will have the same RepoOptions, so we cache them
      auto const& repoOptions = RepoOptions::forFile(fullPath.data()).flags();
      options.emplace_back(
        m_async->m_repoOptions.get(
          repoOptions.cacheKeySha1(),
          repoOptions,
          HPHP_CORO_CURRENT_EXECUTOR
        )
      );
      paths.emplace_back(std::move(fullPath));
      metas.emplace_back(std::move(fileName), std::move(targetPath));
    }

    if (paths.empty()) {
      assertx(metas.empty());
      assertx(options.empty());
      HPHP_CORO_RETURN(FileAndSizeVec{});
    }

    // Free up some memory before awaiting
    decltype(group.m_files){}.swap(group.m_files);
    auto const workItems = paths.size();

    Optional<std::vector<Ref<RepoOptionsFlags>>> storedOptions;

    using InputsT = std::vector<decltype(g_parseJob)::InputsT>;
    using OutputsT = std::vector<decltype(g_parseJob)::ReturnT>;

    auto const doStore =
      [&] (bool opportunistic) ->
      coro::Task<std::pair<const Ref<Config>*, InputsT>> {
      // Store the inputs and get their refs
      auto [fileRefs, metaRefs, optionRefs, configRef] =
        HPHP_CORO_AWAIT(
          coro::collect(
            m_async->m_client.storeFile(
              paths,
              opportunistic
            ),
            m_async->m_client.storeMulti(
              metas,
              opportunistic
            ),
            // If we already called doStore, then options will be
            // empty here (but storedOptions will be set).
            coro::collectRange(std::move(options)),
            *m_async->m_config
          )
        );

      assertx(fileRefs.size() == workItems);
      assertx(metaRefs.size() == workItems);

      // Options are never stored opportunistically. The first time we
      // call doStore, we'll store them above and store the results in
      // storedOptions. If we call this again, the above store for
      // options will do nothing, and we'll just reload the
      // storedOptions.
      if (storedOptions) {
        assertx(!opportunistic);
        assertx(optionRefs.empty());
        assertx(storedOptions->size() == workItems);
        optionRefs = *std::move(storedOptions);
      } else {
        assertx(optionRefs.size() == workItems);
        if (opportunistic) storedOptions = optionRefs;
      }

      // "Tuplize" the input refs (so they're in the format that
      // extern-worker expects).
      std::vector<decltype(g_parseJob)::InputsT> inputs;
      inputs.reserve(workItems);
      for (size_t i = 0; i < workItems; ++i) {
        inputs.emplace_back(
          std::move(fileRefs[i]),
          std::move(metaRefs[i]),
          std::move(optionRefs[i])
        );
      }
      HPHP_CORO_RETURN(std::make_pair(configRef, std::move(inputs)));
    };

    auto const doExec = [&] (auto inputs,
                             bool opportunistic) -> coro::Task<OutputsT> {
      // Run the job. This does the parsing.
      auto outputRefs = HPHP_CORO_AWAIT(
        m_async->m_client.exec(
          g_parseJob,
          std::make_tuple(*inputs.first),
          std::move(inputs.second),
          opportunistic
        )
      );
      assertx(outputRefs.size() == workItems);
      HPHP_CORO_MOVE_RETURN(outputRefs);
    };

    auto outputRefs = HPHP_CORO_AWAIT(coro::invoke(
      [&] () -> coro::Task<OutputsT> {
        if (Option::ParserOptimisticStore &&
            m_async->m_client.supportsOptimistic()) {
          // Try optimistic mode first. We won't actually store
          // anything, just generate the Refs. If something isn't
          // actually present in the workers, the execution will throw
          // an exception. If everything is present, we've skipped a
          // lot of work.
          auto inputs = HPHP_CORO_AWAIT(doStore(true));
          try {
            HPHP_CORO_RETURN(HPHP_CORO_AWAIT(doExec(std::move(inputs), true)));
          } catch (const extern_worker::Error&) {}
        }
        // Either optimistic mode isn't enabled, or it failed
        // above. Try again, actually storing everything this time.
        auto inputs = HPHP_CORO_AWAIT(doStore(false));
        HPHP_CORO_RETURN(HPHP_CORO_AWAIT(doExec(std::move(inputs), false)));
      }
    ));

    // Load the outputs
    auto outputs =
      HPHP_CORO_AWAIT(m_async->m_client.load(std::move(outputRefs)));
    assertx(outputs.size() == workItems);

    m_total += workItems;

    // Process the outputs
    FileAndSizeVec ondemand;
    for (auto& [meta, wrapper] : outputs) {
      // The Unit had an ICE and we're configured to treat that as a
      // fatal error. Here is where we die on it.
      if (!meta.m_abort.empty()) {
        fprintf(stderr, "%s", meta.m_abort.c_str());
        _Exit(1);
      }
      // If we produced an UnitEmitter, hand it off for whatever
      // processing we need to do with it.
      if (wrapper.m_ue) addUnitEmitter(std::move(wrapper.m_ue));
      // Resolve any symbol refs into files to parse ondemand
      resolveOnDemand(ondemand, meta.m_symbol_refs);
    }

    HPHP_CORO_MOVE_RETURN(ondemand);
  } catch (const Exception& e) {
    Logger::FError(
      "Fatal: An unexpected exception was thrown while parsing: {}",
      e.getMessage()
    );
    m_parseFailed.store(true);
  } catch (const Error& e) {
    Logger::FError("Extern worker error while parsing: {}",
                   e.what());
    m_parseFailed.store(true);
  } catch (const std::exception& e) {
    Logger::FError(
      "Fatal: An unexpected exception was thrown while parsing: {}",
      e.what()
    );
    m_parseFailed.store(true);
  } catch (...) {
    Logger::Error("Fatal: An unexpected exception was thrown while parsing");
    m_parseFailed.store(true);
  }

  HPHP_CORO_RETURN(FileAndSizeVec{});
}

void Package::addUnitEmitter(std::unique_ptr<UnitEmitter> ue) {
  if (m_ar->program().get()) {
    HHBBC::add_unit_to_program(ue.get(), *m_ar->program());
  } else {
    m_ar->addHhasFile(std::move(ue));
  }
}

///////////////////////////////////////////////////////////////////////////////

void Package::resolveOnDemand(FileAndSizeVec& out,
                              const SymbolRefs& symbolRefs,
                              bool report) {
  if (!m_parseOnDemand) return;

  auto const& onPath = [&] (const std::string& path) {
    auto rpath = [&] {
      if (path.compare(0, m_root.length(), m_root) == 0) {
        return path.substr(m_root.length());
      }
      return path;
    }();
    if (rpath.empty()) return;
    if (Option::PackageExcludeFiles.count(rpath) > 0) return;
    if (Option::IsFileExcluded(rpath, Option::PackageExcludePatterns)) return;

    auto canon = FileUtil::canonicalize(String(std::move(rpath))).toCppString();
    assertx(!canon.empty());

    // Only parse a file once. This ensures we eventually run out
    // of things to parse.
    if (report || m_parsedFiles.emplace(canon, true).second) {
      auto const absolute = [&] {
        if (FileUtil::isDirSeparator(canon.front())) {
          return canon;
        } else {
          return m_root + canon;
        }
      }();

      struct stat sb;
      if (stat(absolute.c_str(), &sb)) {
        Logger::FError("Unable to stat {}", absolute);
        m_parseFailed.store(true);
        return;
      }
      out.emplace_back(FileAndSize{std::move(canon), (size_t)sb.st_size});
    }
  };

  auto const onMap = [&] (auto const& syms, auto const& m) {
    for (auto const& sym : syms) {
      auto const it = m.find(sym);
      if (it == m.end()) continue;
      onPath(Option::AutoloadRoot + it->second);
    }
  };

  for (auto const& [kind, syms] : symbolRefs) {
    switch (kind) {
      case SymbolRef::Include:
        for (auto const& name : syms) onPath(name);
        break;
      case SymbolRef::Class:
        onMap(syms, Option::AutoloadClassMap);
        break;
      case SymbolRef::Function:
        onMap(syms, Option::AutoloadFuncMap);
        break;
      case SymbolRef::Constant:
        onMap(syms, Option::AutoloadConstMap);
        break;
    }
  }
}

///////////////////////////////////////////////////////////////////////////////
