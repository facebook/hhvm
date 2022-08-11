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
#include <filesystem>
#include <fstream>
#include <map>
#include <memory>
#include <set>
#include <sys/stat.h>
#include <sys/types.h>
#include <utility>
#include <vector>

#include <folly/String.h>
#include <folly/portability/Dirent.h>
#include <folly/portability/Unistd.h>

#include "hphp/compiler/option.h"
#include "hphp/hack/src/hackc/ffi_bridge/compiler_ffi.rs.h"
#include "hphp/hhvm/process-init.h"
#include "hphp/runtime/base/execution-context.h"
#include "hphp/runtime/base/file-util-defs.h"
#include "hphp/runtime/base/file-util.h"
#include "hphp/runtime/base/program-functions.h"
#include "hphp/runtime/base/static-string-table.h"
#include "hphp/runtime/vm/as.h"
#include "hphp/runtime/vm/func-emitter.h"
#include "hphp/runtime/vm/preclass-emitter.h"
#include "hphp/runtime/vm/type-alias-emitter.h"
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

///////////////////////////////////////////////////////////////////////////////

const StaticString s_EntryPoint("__EntryPoint");

///////////////////////////////////////////////////////////////////////////////

Package::Package(const std::string& root,
                 bool parseOnDemand,
                 coro::TicketExecutor& executor,
                 extern_worker::Client& client)
  : m_root{root}
  , m_failed{false}
  , m_parseOnDemand{parseOnDemand}
  , m_total{0}
  , m_callback{nullptr}
  , m_fileCache{std::make_shared<FileCache>()}
  , m_executor{executor}
  , m_client{client}
  , m_config{
      [this] { return m_client.store(Config::make()); },
      m_executor.sticky()
    }
  , m_repoOptions{client}
{
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

///////////////////////////////////////////////////////////////////////////////

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

Package::FileMetaVec s_fileMetas;
Package::ParseMetaVec s_parseMetas;
Package::IndexMetaVec s_indexMetas;

size_t s_fileMetasIdx{0};

// Construct parse metadata for the given unit-emitter
UnitEmitterSerdeWrapper output(std::unique_ptr<UnitEmitter> ue) {
  if (!ue) {
    s_parseMetas.emplace_back();
    return UnitEmitterSerdeWrapper{};
  }

  Package::ParseMeta meta;
  meta.m_symbol_refs = std::move(ue->m_symbol_refs);
  meta.m_filepath = ue->m_filepath;

  for (size_t n = 0; n < ue->numPreClasses(); ++n) {
    auto pce = ue->pce(n);
    if (pce->attrs() & AttrEnum) {
      meta.m_definitions.m_enums.emplace_back(pce->name());
    } else {
      meta.m_definitions.m_classes.emplace_back(pce->name());
    }
  }
  for (auto const& fe : ue->fevec()) {
    if (fe->attrs & AttrIsMethCaller) {
      meta.m_definitions.m_methCallers.emplace_back(fe->name);
    } else {
      meta.m_definitions.m_funcs.emplace_back(fe->name);
    }
  }
  for (auto const& t : ue->typeAliases()) {
    meta.m_definitions.m_typeAliases.emplace_back(t->name());
  }
  for (auto const& c : ue->constants()) {
    meta.m_definitions.m_constants.emplace_back(c.name);
  }
  for (auto const& m : ue->modules()) {
    meta.m_definitions.m_modules.emplace_back(m.name);
  }

  s_parseMetas.emplace_back(std::move(meta));
  return std::move(ue);
}

// HHVM shutdown code shared by different Job types.
void finishJob() {
  hphp_process_exit();
  rds::local::fini();
}

///////////////////////////////////////////////////////////////////////////////

}

///////////////////////////////////////////////////////////////////////////////

void Package::parseInit(const Config& config, FileMetaVec meta) {
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

  s_fileMetas = std::move(meta);
  s_fileMetasIdx = 0;
}

Package::ParseMetaVec Package::parseFini() {
  assertx(s_fileMetasIdx == s_fileMetas.size());
  assertx(s_parseMetas.size() == s_fileMetas.size());
  finishJob();
  return std::move(s_parseMetas);
}

UnitEmitterSerdeWrapper
Package::parseRun(const std::string& content,
                  const RepoOptionsFlags& repoOptions) {
  if (s_fileMetasIdx >= s_fileMetas.size()) {
    throw Error{
      folly::sformat("Encountered {} inputs, but only {} file metas",
                     s_fileMetasIdx+1, s_fileMetas.size())
    };
  }
  auto const& meta = s_fileMetas[s_fileMetasIdx++];
  auto const& fileName = meta.m_filename;

  try {
    if (RO::EvalAllowHhas && folly::StringPiece(fileName).endsWith(".hhas")) {
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
          return output(nullptr);
        }
      }
      return output(std::move(ue));
    }

    LazyUnitContentsLoader loader{
      SHA1{mangleUnitSha1(string_sha1(content), fileName, repoOptions)},
      content,
      repoOptions,
      {} // TODO: repo mode support for decl providers
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
      ParseMeta meta;
      meta.m_abort = exn.what();
      s_parseMetas.emplace_back(std::move(meta));
      return UnitEmitterSerdeWrapper{};
    }

    if (ue) {
      if (!ue->m_ICE && meta.m_targetPath) {
        ue =
          createSymlinkWrapper(fileName, *meta.m_targetPath, std::move(ue));
        if (!ue) {
          // If the symlink contains no EntryPoint we don't do anything but it
          // is still success
          return output(nullptr);
        }
      }
      return output(std::move(ue));
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
Package::groupDirectories(std::string path, bool exclude_dirs) {
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
      if (exclude_dirs && Option::PackageExcludeDirs.count(name)) {
        // Only skip excluded dirs when requested.
        return false;
      }
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
      dirs.emplace_back(groupDirectories(name, exclude_dirs));

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
void Package::groupFiles(Groups& groups, FileAndSizeVec files) {
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
      [] (const Group& g) { return g.m_files.empty(); }
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
coro::Task<Package::FileAndSizeVec> Package::parseGroups(
  Groups groups,
  const UnitIndex& index
) {
  if (groups.empty()) HPHP_CORO_RETURN(FileAndSizeVec{});

  // Kick off the parsing. Each group gets its own sticky ticket (so
  // earlier groups will get scheduling priority over later ones).
  std::vector<coro::TaskWithExecutor<FileAndSizeVec>> tasks;
  for (auto& group : groups) {
    tasks.emplace_back(
      parseGroup(std::move(group), index)
        .scheduleOn(m_executor.sticky())
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

coro::Task<Package::Groups> Package::groupAll(bool exclude_dirs) {
  Timer timer{
    Timer::WallTime,
    exclude_dirs ? "finding parse inputs" : "finding index inputs"
  };
  std::vector<coro::Task<GroupResult>> tasks;
  for (auto& dir : m_directories) {
    tasks.emplace_back(groupDirectories(std::move(dir), exclude_dirs));
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

  // Sort the groups from highest combined file size to lowest.
  // Larger groups will probably take longer to process, so we want to
  // start those earliest.
  std::sort(
    top.m_grouped.begin(),
    top.m_grouped.end(),
    [] (const Group& a, const Group& b) {
      if (a.m_size != b.m_size) return b.m_size < a.m_size;
      if (a.m_files.size() != b.m_files.size()) {
        return b.m_files.size() < a.m_files.size();
      }
      return a.m_files < b.m_files;
    }
  );

  HPHP_CORO_RETURN(std::move(top.m_grouped));
}

// The actual parse loop. Find the initial set of inputs (from
// configuration), parse them, gather on-demand files, then repeat the
// process until we have no new files to parse. Everything "under"
// this call works asynchronously.
coro::Task<void> Package::parseAll(const UnitIndex& index) {
  // Find the initial set of groups
  auto groups = HPHP_CORO_AWAIT(groupAll(true));

  // Parse the "main" round and get any ondemand files
  FileAndSizeVec ondemand;
  {
    Timer timer{Timer::WallTime, "parsing inputs"};
    ondemand = HPHP_CORO_AWAIT(parseGroups(std::move(groups), index));
    m_parsingInputs = std::chrono::microseconds{timer.getMicroSeconds()};
  }

  if (ondemand.empty()) {
    HPHP_CORO_RETURN_VOID;
  }

  Timer timer{Timer::WallTime, "parsing on-demand"};
  // We have ondemand files, so keep parsing until we have nothing
  // more to parse.
  do {
    assertx(groups.empty());
    groupFiles(groups, std::move(ondemand));
    ondemand = HPHP_CORO_AWAIT(parseGroups(std::move(groups), index));
  } while (!ondemand.empty());
  m_parsingOndemand = std::chrono::microseconds{timer.getMicroSeconds()};
  HPHP_CORO_RETURN_VOID;
}

coro::Task<bool> Package::parse(const UnitIndex& index,
                                const ParseCallback& callback,
                                const LocalCallback& localCallback) {
  assertx(callback);
  assertx(localCallback);
  assertx(!m_callback);

  Logger::FInfo(
    "parsing using {} threads using {}{}",
    m_executor.numThreads(),
    m_client.implName(),
    coro::using_coros ? "" : " (coros disabled!)"
  );

  m_callback = &callback;
  SCOPE_EXIT { m_callback = nullptr; };

  HphpSession _{Treadmill::SessionKind::CompilerEmit};

  // Treat any symbol refs from systemlib as if they were part of the
  // original Package.
  UEVec localUEs;
  for (auto& ue : SystemLib::claimRegisteredUnitEmitters()) {
    FileAndSizeVec ondemand;
    resolveOnDemand(ondemand, ue->m_symbol_refs, index, true);
    for (auto const& p : ondemand) {
      addSourceFile(p.m_path);
      Logger::FVerbose("systemlib unit {} -> {}",
          ue->m_filepath,
          p.m_path.string()
      );
    }
    localUEs.emplace_back(std::move(ue));
  }

  std::vector<coro::TaskWithExecutor<void>> tasks;
  if (!localUEs.empty()) {
    auto task = localCallback(std::move(localUEs));
    tasks.emplace_back(std::move(task).scheduleOn(m_executor.sticky()));
  }
  tasks.emplace_back(parseAll(index).scheduleOn(m_executor.sticky()));
  HPHP_CORO_AWAIT(coro::collectRange(std::move(tasks)));
  HPHP_CORO_RETURN(!m_failed.load());
}

namespace {

bool statFile(const std::filesystem::path& fileName, const std::string& root,
               std::string& fullPath, Optional<std::string>& targetPath) {
  if (FileUtil::isDirSeparator(fileName.native().front())) {
    fullPath = fileName;
  } else {
    fullPath = root + fileName.native();
  }

  struct stat sb;
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
  if (S_ISLNK(sb.st_mode)) {
    auto const target = std::filesystem::canonical(fullPath);
    targetPath.emplace(std::filesystem::relative(target, root).native());
  }
  return true;
}

using InputsT = std::vector<
  std::tuple<Ref<std::string>, Ref<RepoOptionsFlags>>
>;

coro::Task<std::tuple<
  const Ref<Package::Config>*,
  Ref<std::vector<Package::FileMeta>>,
  InputsT
>>
storeInputs(
  bool optimistic,
  std::vector<std::filesystem::path>& paths,
  std::vector<Package::FileMeta>& metas,
  std::vector<coro::Task<Ref<RepoOptionsFlags>>>& options,
  Optional<std::vector<Ref<RepoOptionsFlags>>>& storedOptions,
  extern_worker::Client& client,
  const coro::AsyncValue<extern_worker::Ref<Package::Config>>& config
) {
  auto workItems = paths.size();

  // Store the inputs and get their refs
  auto [fileRefs, metasRef, optionRefs, configRef] =
    HPHP_CORO_AWAIT(
      coro::collect(
        client.storeFile(paths, optimistic),
        optimistic
          ? client.storeOptimistically(metas)
          : client.store(metas),
        // If we already called storeInputs, then options will be
        // empty here (but storedOptions will be set).
        coro::collectRange(std::move(options)),
        *config
      )
    );

  assertx(fileRefs.size() == workItems);

  // Options are never stored optimistically. The first time we call
  // storeInputs, we'll store them above and store the results in
  // storedOptions. If we call this again, the above store for options
  // will do nothing, and we'll just reload the storedOptions.
  if (storedOptions) {
    assertx(!optimistic);
    assertx(optionRefs.empty());
    assertx(storedOptions->size() == workItems);
    optionRefs = *std::move(storedOptions);
  } else {
    assertx(optionRefs.size() == workItems);
    if (optimistic) storedOptions = optionRefs;
  }

  // "Tuplize" the input refs (so they're in the format that
  // extern-worker expects).
  InputsT inputs;
  inputs.reserve(workItems);
  for (size_t i = 0; i < workItems; ++i) {
    inputs.emplace_back(
      std::move(fileRefs[i]),
      std::move(optionRefs[i])
    );
  }

  HPHP_CORO_RETURN(std::make_tuple(configRef, metasRef, std::move(inputs)));
}

}

coro::Task<void> Package::prepareInputs(
  Group group,
  std::vector<std::filesystem::path>& paths,
  std::vector<Package::FileMeta>& metas,
  std::vector<coro::Task<Ref<RepoOptionsFlags>>>& options
) {
  paths.reserve(group.m_files.size());
  metas.reserve(group.m_files.size());
  options.reserve(group.m_files.size());
  for (auto& fileName : group.m_files) {
    assertx(!fileName.empty());

    std::string fullPath;
    Optional<std::string> targetPath;
    if (!statFile(fileName, m_root, fullPath, targetPath)) {
      Logger::FError("Fatal: Unable to stat/parse {}", fileName.native());
      m_failed.store(true);
      continue;
    }

    // Most files will have the same RepoOptions, so we cache them
    auto const& repoOptions = RepoOptions::forFile(fullPath.data()).flags();
    options.emplace_back(
      m_repoOptions.get(
        repoOptions.cacheKeySha1(),
        repoOptions,
        HPHP_CORO_CURRENT_EXECUTOR
      )
    );
    paths.emplace_back(std::move(fullPath));
    metas.emplace_back(std::move(fileName), std::move(targetPath));
  }
  HPHP_CORO_RETURN_VOID;
}

// Parse a group using extern-worker, hand off the UnitEmitter
// obtained, and return any on-demand files from the parsing.
coro::Task<Package::FileAndSizeVec> Package::parseGroup(
    Group group,
    const UnitIndex& index
) {
  using namespace folly::gen;

  // Make sure we're running on the thread we should be
  HPHP_CORO_RESCHEDULE_ON_CURRENT_EXECUTOR;

  try {
    // First build the inputs for the job
    std::vector<std::filesystem::path> paths;
    std::vector<FileMeta> metas;
    std::vector<coro::Task<Ref<RepoOptionsFlags>>> options;
    HPHP_CORO_AWAIT(prepareInputs(std::move(group), paths, metas, options));

    if (paths.empty()) {
      assertx(metas.empty());
      assertx(options.empty());
      HPHP_CORO_RETURN(FileAndSizeVec{});
    }

    auto const workItems = paths.size();
    for (size_t i = 0; i < workItems; i++) {
      auto const& fileName = metas[i].m_filename;
      if (!m_extraStaticFiles.count(fileName)) {
        m_discoveredStaticFiles.emplace(
          fileName,
          Option::CachePHPFile ? paths[i] : ""
        );
      }
    }

    Optional<std::vector<Ref<RepoOptionsFlags>>> storedOptions;

    auto parseMetas = HPHP_CORO_AWAIT(coro::invoke(
      [&] () -> coro::Task<ParseMetaVec> {
        assertx(m_callback);

        if (Option::ParserOptimisticStore &&
            m_client.supportsOptimistic()) {
          // Try optimistic mode first. We won't actually store
          // anything, just generate the Refs. If something isn't
          // actually present in the workers, the execution will throw
          // an exception. If everything is present, we've skipped a
          // lot of work.
          auto [configRef, metasRef, fileRefs] = HPHP_CORO_AWAIT(storeInputs(
              true, paths, metas, options, storedOptions, m_client, m_config
          ));
          try {
            HPHP_CORO_RETURN(HPHP_CORO_AWAIT((*m_callback)(
              *configRef, std::move(metasRef), std::move(fileRefs), true
            )));
          } catch (const extern_worker::Error&) {}
        }
        // Either optimistic mode isn't enabled, or it failed
        // above. Try again, actually storing everything this time.
        auto [configRef, metasRef, fileRefs] = HPHP_CORO_AWAIT(storeInputs(
            false, paths, metas, options, storedOptions, m_client, m_config
        ));
        HPHP_CORO_RETURN(HPHP_CORO_AWAIT((*m_callback)(
          *configRef, std::move(metasRef), std::move(fileRefs), false
        )));
      }
    ));

    always_assert(parseMetas.size() == workItems);
    m_total += workItems;

    // Process the outputs
    FileAndSizeVec ondemand;
    for (auto& meta : parseMetas) {
      // The Unit had an ICE and we're configured to treat that as a
      // fatal error. Here is where we die on it.
      if (!meta.m_abort.empty()) {
        fprintf(stderr, "%s", meta.m_abort.c_str());
        _Exit(1);
      }
      // Resolve any symbol refs into files to parse ondemand
      resolveOnDemand(ondemand, meta.m_symbol_refs, index);
    }
    HPHP_CORO_MOVE_RETURN(ondemand);
  } catch (const Exception& e) {
    Logger::FError(
      "Fatal: An unexpected exception was thrown while parsing: {}",
      e.getMessage()
    );
    m_failed.store(true);
  } catch (const Error& e) {
    Logger::FError("Extern worker error while parsing: {}",
                   e.what());
    m_failed.store(true);
  } catch (const std::exception& e) {
    Logger::FError(
      "Fatal: An unexpected exception was thrown while parsing: {}",
      e.what()
    );
    m_failed.store(true);
  } catch (...) {
    Logger::Error("Fatal: An unexpected exception was thrown while parsing");
    m_failed.store(true);
  }

  HPHP_CORO_RETURN(FileAndSizeVec{});
}

///////////////////////////////////////////////////////////////////////////////

void Package::resolveOnDemand(FileAndSizeVec& out,
                              const SymbolRefs& symbolRefs,
                              const UnitIndex& index,
                              bool report) {
  if (!m_parseOnDemand) return;

  auto const& onPath = [&] (const StringData* rpath_str) {
    if (rpath_str->empty()) return;
    auto rpath = rpath_str->toCppString();
    if (Option::PackageExcludeFiles.count(rpath) > 0 ||
        Option::IsFileExcluded(rpath, Option::PackageExcludePatterns)) {
      // Found symbol in UnitIndex, but the corresponding file was excluded.
      Logger::FVerbose("excluding ondemand file {}", rpath_str);
      return;
    }

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
        m_failed.store(true);
        return;
      }
      out.emplace_back(FileAndSize{std::move(canon), (size_t)sb.st_size});
    }
  };

  auto const onMap = [&] (auto const& syms, auto const& sym_to_file) {
    for (auto const& sym : syms) {
      auto const it = sym_to_file.find(makeStaticString(sym));
      if (it == sym_to_file.end()) continue;
      onPath(it->second);
    }
  };

  for (auto const& [kind, syms] : symbolRefs) {
    switch (kind) {
      case SymbolRef::Include:
        for (auto const& path : syms) {
          auto const rpath = path.compare(0, m_root.length(), m_root) == 0
            ? path.substr(m_root.length())
            : path;
          onPath(makeStaticString(rpath));
        }
        break;
      case SymbolRef::Class:
        onMap(syms, index.types);
        break;
      case SymbolRef::Function:
        onMap(syms, index.funcs);
        break;
      case SymbolRef::Constant:
        onMap(syms, index.constants);
        break;
    }
  }
}

///////////////////////////////////////////////////////////////////////////////

namespace {
struct UnitDecls {
  uint64_t nopos_hash;
  std::string serialized;

  template <typename SerDe> void serde(SerDe& sd) {
    sd(nopos_hash)
      (serialized);
  }
};

// Extern-worker job for computing decls and autoload-index from source files.
struct IndexJob {
  static std::string name() { return "hphpc-index"; }

  static void init(const Package::Config& config,
                   Package::FileMetaVec meta) {
    Package::parseInit(config, std::move(meta));
  }

  static Package::IndexMetaVec fini() {
    return Package::indexFini();
  }

  static UnitDecls run(
      const std::string& content,
      const RepoOptionsFlags& repoOptions
  );
};

Job<IndexJob> g_indexJob;
}

Package::IndexMetaVec Package::indexFini() {
  assertx(s_fileMetasIdx == s_fileMetas.size());
  assertx(s_indexMetas.size() == s_indexMetas.size());
  finishJob();
  return std::move(s_indexMetas);
}

// Index one source file:
// 1. compute decls
// 2. use facts from decls to compute IndexMeta (decl names in each file)
// 3. save serialized decls in UnitDecls as job output
UnitDecls IndexJob::run(
    const std::string& content,
    const RepoOptionsFlags& repoOptions
) {
  if (s_fileMetasIdx >= s_fileMetas.size()) {
    throw Error{
      folly::sformat("Encountered {} inputs, but only {} file metas",
                     s_fileMetasIdx+1, s_fileMetas.size())
    };
  }
  auto const& meta = s_fileMetas[s_fileMetasIdx++];
  auto const& fileName = meta.m_filename;

  auto const bail = [&](const char* message) -> UnitDecls {
    Package::IndexMeta summary;
    summary.error = message;
    s_indexMetas.emplace_back(std::move(summary));
    return UnitDecls{};
  };

  if (meta.m_targetPath) {
    // When/if this symlink is parsed, it's UnitEmitter will be a generated
    // stub function that calls the entrypoint of the target file (if any).
    // Don't index it since we don't expect any external references to the
    // generated stub entry point function.
    return bail("not indexing symlink");
  }

  if (RO::EvalAllowHhas && folly::StringPiece(fileName).endsWith(".hhas")) {
    return bail("cannot index hhas");
  }

  auto decl_flags = repoOptions.getDeclFlags();
  auto decl_options = hackc_create_direct_decl_parse_options(
      decl_flags,
      repoOptions.getAliasedNamespacesConfig()
  );
  auto decls = hackc_direct_decl_parse(*decl_options, fileName, content);
  if (decls.has_errors) {
    return bail("decl parser error");
  }

  // Get Facts from Decls, then populate IndexMeta.
  auto facts = hackc_decls_to_facts_cpp_ffi(decl_flags, decls, "");
  Package::IndexMeta summary;
  for (auto& e : facts.facts.types) {
    summary.types.emplace_back(makeStaticString(std::string(e.name)));
  }
  for (auto& e : facts.facts.functions) {
    summary.funcs.emplace_back(makeStaticString(std::string(e)));
  }
  for (auto& e : facts.facts.constants) {
    summary.constants.emplace_back(makeStaticString(std::string(e)));
  }
  s_indexMetas.emplace_back(std::move(summary));
  return UnitDecls{
    decls.nopos_hash,
    std::string{decls.serialized.begin(), decls.serialized.end()}
  };
}

coro::Task<bool> Package::index(const IndexCallback& callback) {
  Logger::FInfo(
    "indexing using {} threads using {}{}",
    m_executor.numThreads(),
    m_client.implName(),
    coro::using_coros ? "" : " (coros disabled!)"
  );

  // TODO: index systemlib. But here is too late; they have already been
  // parsed into UEs at startup, and not yet claimed.
  HPHP_CORO_AWAIT(indexAll(callback).scheduleOn(m_executor.sticky()));
  HPHP_CORO_RETURN(!m_failed.load());
}

coro::Task<void> Package::indexAll(const IndexCallback& callback) {
  // Find the indexing groups
  auto groups = HPHP_CORO_AWAIT(groupAll(false));
  Logger::FInfo("indexing {:,} groups", groups.size());

  // Index all files
  Timer timer{Timer::WallTime, "indexing files"};
  HPHP_CORO_AWAIT(indexGroups(callback, std::move(groups)));
  HPHP_CORO_RETURN_VOID;
}

// Index all of the files in the given groups
coro::Task<void> Package::indexGroups(const IndexCallback& callback,
                                      Groups groups) {
  if (groups.empty()) HPHP_CORO_RETURN_VOID;

  // Kick off indexing. Each group gets its own sticky ticket (so
  // earlier groups will get scheduling priority over later ones).
  std::vector<coro::TaskWithExecutor<void>> tasks;
  for (auto& group : groups) {
    tasks.emplace_back(
      indexGroup(callback, std::move(group)).scheduleOn(m_executor.sticky())
    );
  }
  HPHP_CORO_AWAIT(coro::collectRange(std::move(tasks)));
  HPHP_CORO_RETURN_VOID;
}

// Index a group using extern-worker, invoke callback with each IndexMeta.
coro::Task<void> Package::indexGroup(const IndexCallback& callback,
                                     Group group) {
  using namespace folly::gen;

  // Make sure we're running on the thread we should be
  HPHP_CORO_RESCHEDULE_ON_CURRENT_EXECUTOR;

  try {
    // First build the inputs for the job
    std::vector<std::filesystem::path> paths;
    std::vector<FileMeta> metas;
    std::vector<coro::Task<Ref<RepoOptionsFlags>>> options;
    HPHP_CORO_AWAIT(prepareInputs(std::move(group), paths, metas, options));

    if (paths.empty()) {
      assertx(metas.empty());
      assertx(options.empty());
      HPHP_CORO_RETURN_VOID;
    }

    auto const workItems = paths.size();
    Optional<std::vector<Ref<RepoOptionsFlags>>> storedOptions;

    using ExecT = decltype(g_indexJob)::ExecT;
    auto const doExec = [&] (
        auto configRef, auto metasRef, auto fileRefs, bool optimistic
    ) -> coro::Task<ExecT> {
      auto out = HPHP_CORO_AWAIT(
        m_client.exec(
          g_indexJob,
          std::make_tuple(*configRef, std::move(metasRef)),
          std::move(fileRefs),
          optimistic
        )
      );
      HPHP_CORO_MOVE_RETURN(out);
    };

    auto [declsRefs, summariesRef] = HPHP_CORO_AWAIT(coro::invoke(
      [&] () -> coro::Task<ExecT> {
        if (Option::ParserOptimisticStore &&
            m_client.supportsOptimistic()) {
          // Try optimistic mode first. We won't actually store
          // anything, just generate the Refs. If something isn't
          // actually present in the workers, the execution will throw
          // an exception. If everything is present, we've skipped a
          // lot of work.
          auto [configRef, metasRef, fileRefs] = HPHP_CORO_AWAIT(storeInputs(
              true, paths, metas, options, storedOptions, m_client, m_config
          ));
          try {
            HPHP_CORO_RETURN(HPHP_CORO_AWAIT(doExec(
              configRef, std::move(metasRef), std::move(fileRefs), true
            )));
          } catch (const extern_worker::Error&) {}
        }
        // Either optimistic mode isn't enabled, or it failed
        // above. Try again, actually storing everything this time.
        auto [configRef, metasRef, fileRefs] = HPHP_CORO_AWAIT(storeInputs(
            false, paths, metas, options, storedOptions, m_client, m_config
        ));
        HPHP_CORO_RETURN(HPHP_CORO_AWAIT(doExec(
          configRef, std::move(metasRef), std::move(fileRefs), false
        )));
      }
    ));

    // Load the summaries but leave decls in external storage
    auto summaries = HPHP_CORO_AWAIT(m_client.load(summariesRef));
    assertx(metas.size() == workItems);
    assertx(declsRefs.size() == workItems);
    always_assert(summaries.size() == workItems);
    m_total += workItems;

    // Process the summaries
    for (size_t i = 0; i < workItems; ++i) {
      auto& meta = metas[i];
      auto& summary = summaries[i];
      // TODO: pass decls_refs[i] to callback
      if (summary.error.empty()) {
        callback(std::move(meta.m_filename), std::move(summary));
      } else {
        // Could not parse decls in this file. Compiler may fail, or produce
        // a unit that fatals when run.
        Logger::FWarning("Warning: decl-parser error in {}: {}",
            meta.m_filename, summary.error
        );
      }
    }
    HPHP_CORO_RETURN_VOID;
  } catch (const Exception& e) {
    Logger::FError(
      "Fatal: An unexpected exception was thrown while indexing: {}",
      e.getMessage()
    );
    m_failed.store(true);
  } catch (const Error& e) {
    Logger::FError("Extern worker error while indexing: {}",
                   e.what());
    m_failed.store(true);
  } catch (const std::exception& e) {
    Logger::FError(
      "Fatal: An unexpected exception was thrown while indexing: {}",
      e.what()
    );
    m_failed.store(true);
  } catch (...) {
    Logger::Error("Fatal: An unexpected exception was thrown while indexing");
    m_failed.store(true);
  }
  HPHP_CORO_RETURN_VOID;
}

///////////////////////////////////////////////////////////////////////////////
