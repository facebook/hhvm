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

#include "hphp/compiler/analysis/analysis_result.h"
#include "hphp/compiler/json.h"
#include "hphp/compiler/option.h"
#include "hphp/runtime/base/execution-context.h"
#include "hphp/runtime/base/file-util-defs.h"
#include "hphp/runtime/base/file-util.h"
#include "hphp/runtime/base/program-functions.h"
#include "hphp/runtime/vm/as.h"
#include "hphp/runtime/vm/extern-compiler.h"
#include "hphp/runtime/vm/repo.h"
#include "hphp/runtime/vm/unit-emitter.h"
#include "hphp/util/exception.h"
#include "hphp/util/job-queue.h"
#include "hphp/util/logger.h"
#include "hphp/util/process.h"
#include "hphp/zend/zend-string.h"

using namespace HPHP;
using std::set;

///////////////////////////////////////////////////////////////////////////////

Package::Package(const char* root, bool /*bShortTags*/ /* = true */)
    : m_dispatcher(nullptr), m_lineCount(0), m_charCount(0) {
  m_root = FileUtil::normalizeDir(root);
  m_ar = std::make_shared<AnalysisResult>();
  m_fileCache = std::make_shared<FileCache>();
}

void Package::addAllFiles(bool force) {
  if (Option::PackageDirectories.empty() && Option::PackageFiles.empty()) {
    addDirectory("/", force);
  } else {
    for (auto const& dir : Option::PackageDirectories) {
      addDirectory(dir, force);
    }
    for (auto const& file : Option::PackageFiles) {
      addSourceFile(file);
    }
  }
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
        addDirectory(fileName, false);
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

void Package::addDirectory(const std::string &path, bool force) {
  m_directories[path] |= force;
}

void Package::addHHJSDirectory(const std::string &path, bool force) {
  m_hhjsDirectories[path] |= force;
}

std::shared_ptr<FileCache> Package::getFileCache() {
  for (auto const& dir : m_directories) {
    std::vector<std::string> files;
    FileUtil::find(files, m_root, dir.first,
                   /* php */ false, /* js */ true, /* other */ true,
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
    FileUtil::find(files, m_root, dir,
                   /* php */ false, /* js */ true, /* other */ true);
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

struct ParseItem {
  ParseItem() : fileName(nullptr), check(false), force(false), js(false) {}
  ParseItem(const std::string* file, bool check, bool js) :
      fileName(file),
      check(check),
      force(false),
      js(js)
    {}
  ParseItem(const std::string& dir, bool force, bool js) :
      dirName(dir),
      fileName(nullptr),
      check(false),
      force(force),
      js(js)
    {}
  std::string dirName;
  const std::string* fileName;
  bool check; // whether its an error if the file isn't found
  bool force; // true to skip filters
  bool js; // whether to check for JS files
};

struct ParserWorker
    : JobQueueWorker<ParseItem, Package*, true, true>
{
  bool m_ret{true};
  void doJob(JobType job) override {
    auto const ret = [&] {
      try {
        if (job.fileName) {
          return m_context->parseImpl(job.fileName);
        }
        m_context->addSourceDirectory(job.dirName, job.force, job.js);
        return true;
      } catch (Exception& e) {
        Logger::Error(e.getMessage());
        return false;
      } catch (...) {
        Logger::Error("Fatal: An unexpected exception was thrown");
        return false;
      }
    }();
    if (!ret && job.check) {
      Logger::Error("Fatal: Unable to stat/parse %s", job.fileName->c_str());
      m_ret = false;
    }
  }

  void onThreadEnter() override {
    g_context.getCheck();
  }
  void onThreadExit() override {
    hphp_memory_cleanup();
  }
};

using ParserDispatcher = JobQueueDispatcher<ParserWorker>;

}

///////////////////////////////////////////////////////////////////////////////

void Package::addSourceFile(const std::string& fileName,
                            bool check /* = false */,
                            bool js /* = false */) {
  if (!fileName.empty()) {
    auto canonFileName =
      FileUtil::canonicalize(String(fileName)).toCppString();
    auto const file = [&] {
      Lock lock(m_mutex);
      auto const info = m_filesToParse.insert(canonFileName);
      return info.second && m_dispatcher ? &*info.first : nullptr;
    }();

    if (file) {
      static_cast<ParserDispatcher*>(m_dispatcher)->enqueue({file, check, js});
    }
  }
}

void Package::addSourceDirectory(const std::string& path,
                                 bool force,
                                 bool js /* = false */) {
  FileUtil::find(m_root, path,
    /* php */ true, /* js */ js, /* other */ false,
    [&] (const std::string& name, bool dir) {
      if (!dir) {
        if (!force) {
          if (Option::PackageExcludeFiles.count(name) ||
              Option::IsFileExcluded(name, Option::PackageExcludePatterns)) {
            return false;
          }
        }
        addSourceFile(name, true, js);
        return true;
      }
      if (!force && Option::PackageExcludeDirs.count(name)) {
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
      static_cast<ParserDispatcher*>(m_dispatcher)->enqueue({name, force, js});
      // Don't iterate the directory in this job.
      return false;
    });
}

bool Package::parse(bool check) {
  if (m_filesToParse.empty() && m_directories.empty() &&
      m_hhjsDirectories.empty()) {
    return true;
  }

  auto const threadCount = Option::ParserThreadCount <= 0 ?
    1 : Option::ParserThreadCount;

  std::thread unit_emitter_thread {
    [&] {
      hphp_thread_init();
      hphp_session_init(Treadmill::SessionKind::CompilerEmit);
      SCOPE_EXIT {
        hphp_context_exit();
        hphp_session_exit();
        hphp_thread_exit();
      };

      static const unsigned kBatchSize = 8;

      std::vector<std::unique_ptr<UnitEmitter>> batched_ues;

      auto commitSome = [&] {
        batchCommit(batched_ues);
        {
          Lock lock(m_ar->getMutex());
          for (auto& ue : batched_ues) {
            m_ar->addHhasFile(std::move(ue));
          }
        }
        batched_ues.clear();
      };

      while (auto ue = m_ueq.pop()) {
        if (m_stop_caching.load(std::memory_order_relaxed)) {
          Lock lock(m_ar->getMutex());
          do {
            m_ar->addHhasFile(std::move(ue));
          } while ((ue = m_ueq.pop()) != nullptr);
          break;
        }
        batched_ues.push_back(std::move(ue));
        if (batched_ues.size() == kBatchSize) {
          commitSome();
        }
      }
      if (batched_ues.size()) commitSome();
    }
  };

  if (RuntimeOption::RepoLocalPath.size() &&
      RuntimeOption::RepoLocalMode != "--") {
    auto units = Repo::get().enumerateUnits(RepoIdLocal, false);
    for (auto& elm : units) {
      m_locally_cached_bytecode.insert(elm.first);
    }
  }

  // If we're using the hack compiler, make sure it agrees on the thread count.
  RuntimeOption::EvalHackCompilerWorkers = threadCount;
  ParserDispatcher dispatcher { threadCount, threadCount, 0, false, this };

  m_dispatcher = &dispatcher;

  auto const files = std::move(m_filesToParse);

  dispatcher.start();
  for (auto const& file : files) {
    addSourceFile(file, check);
  }
  for (auto const& dir : m_directories) {
    addSourceDirectory(dir.first, dir.second);
  }
  if (RuntimeOption::EvalEnableHHJS) {
    for (auto const& dir : m_hhjsDirectories) {
      addSourceDirectory(dir.first, dir.second, /* js */ true);
    }
  }
  dispatcher.waitEmpty();
  if (!m_cache_only) {
    m_stop_caching.store(true, std::memory_order_relaxed);
  }
  m_ueq.push(nullptr);
  unit_emitter_thread.join();

  m_dispatcher = nullptr;

  auto workers = dispatcher.getWorkers();
  for (unsigned int i = 0; i < workers.size(); i++) {
    ParserWorker *worker = workers[i];
    if (!worker->m_ret) return false;
  }

  return true;
}

/*
 * Note that the string pointed to by fileName must live until the
 * Package is destroyed. Its expected to be an element of
 * m_filesToParse.
 */
bool Package::parseImpl(const std::string* fileName) {
  if (fileName->empty()) return false;

  std::string fullPath;
  if (FileUtil::isDirSeparator(fileName->front())) {
    fullPath = *fileName;
  } else {
    fullPath = m_root + *fileName;
  }

  struct stat sb;
  if (stat(fullPath.c_str(), &sb)) {
    if (fullPath.find(' ') == std::string::npos) {
      Logger::Error("Unable to stat file %s", fullPath.c_str());
    }
    return false;
  }
  if ((sb.st_mode & S_IFMT) == S_IFDIR) {
    Logger::Error("Unable to parse directory: %s", fullPath.c_str());
    return false;
  }

  if (RuntimeOption::EvalAllowHhas) {
    if (fileName->size() > 5 &&
        !fileName->compare(fileName->size() - 5, std::string::npos, ".hhas")) {
      std::ifstream s(*fileName);
      std::string content {
        std::istreambuf_iterator<char>(s), std::istreambuf_iterator<char>()
      };
      SHA1 sha1{string_sha1(content)};

      std::unique_ptr<UnitEmitter> ue{
        assemble_string(content.data(), content.size(), fileName->c_str(), sha1,
                        Native::s_noNativeFuncs)
      };
      Lock lock(m_ar->getMutex());
      m_ar->addHhasFile(std::move(ue));
      return true;
    }
  }

  auto report = [&] (int lines) {
    struct stat fst;
    // @lint-ignore HOWTOEVEN1
    stat(fullPath.c_str(), &fst);

    Lock lock(m_mutex);
    m_lineCount += lines;
    m_charCount += fst.st_size;
    if (!m_extraStaticFiles.count(*fileName) &&
        !m_discoveredStaticFiles.count(*fileName)) {
      if (Option::CachePHPFile) {
        m_discoveredStaticFiles[*fileName] = fullPath;
      } else {
        m_discoveredStaticFiles[*fileName] = "";
      }
    }
  };

  std::ifstream s(fullPath);
  std::string content {
    std::istreambuf_iterator<char>(s), std::istreambuf_iterator<char>() };

  auto const& options = RepoOptions::forFile(fileName->data());
  auto const sha1 = SHA1{mangleUnitSha1(string_sha1(content),
                                        options)};
  if (RuntimeOption::RepoLocalPath.size() &&
      RuntimeOption::RepoLocalMode != "--" &&
      m_locally_cached_bytecode.count(*fileName)) {
    // Try the repo; if it's not already there, invoke the compiler.
    if (auto ue = Repo::get().urp().loadEmitter(
          *fileName, sha1, Native::s_noNativeFuncs
        )) {
      for (auto& ent : ue->m_symbol_refs) {
        m_ar->parseOnDemandBy(ent.first, ent.second);
      }
      Lock lock(m_ar->getMutex());
      m_ar->addHhasFile(std::move(ue));
      return true;
    }
  }

  // Invoke external compiler. If it fails to compile the file we log an
  // error and and skip it.
  auto uc = UnitCompiler::create(
    content.data(), content.size(), fileName->c_str(), sha1,
    Native::s_noNativeFuncs, false, options);
  assertx(uc);
  try {
    auto ue = uc->compile(true);
    if (ue && !ue->m_ICE) {
      for (auto& ent : ue->m_symbol_refs) {
        m_ar->parseOnDemandBy(ent.first, ent.second);
      }
      if (RuntimeOption::RepoCommit &&
          RuntimeOption::RepoLocalPath.size() &&
          RuntimeOption::RepoLocalMode == "rw") {
        m_ueq.push(std::move(ue));
      } else {
        Lock lock(m_ar->getMutex());
        m_ar->addHhasFile(std::move(ue));
      }
      report(0);
      return true;
    } else {
      Logger::Error(
        "Unable to compile using %s compiler: %s",
        uc->getName(),
        fullPath.c_str());
      return false;
    }
  } catch (const BadCompilerException& exc) {
    Logger::Error("Bad external compiler: %s", exc.what());
    return false;
  }
}

///////////////////////////////////////////////////////////////////////////////

void Package::saveStatsToFile(const char *filename, int totalSeconds) const {
  std::ofstream f(filename);
  if (f) {
    JSON::CodeError::OutputStream o(f);
    JSON::CodeError::MapStream ms(o);

    ms.add("FileCount", getFileCount())
      .add("LineCount", getLineCount())
      .add("CharCount", getCharCount())
      .add("TotalTime", totalSeconds);

    if (getLineCount()) {
      ms.add("AvgCharPerLine", getCharCount() / getLineCount());
    }

    ms.done();
    f.close();
  }
}
