/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <map>
#include <memory>
#include <set>
#include <utility>
#include <vector>
#include <folly/String.h>
#include "hphp/compiler/analysis/analysis_result.h"
#include "hphp/compiler/parser/parser.h"
#include "hphp/compiler/analysis/symbol_table.h"
#include "hphp/compiler/analysis/variable_table.h"
#include "hphp/compiler/option.h"
#include "hphp/compiler/json.h"
#include "hphp/util/process.h"
#include "hphp/util/logger.h"
#include "hphp/util/exception.h"
#include "hphp/util/job-queue.h"
#include "hphp/runtime/base/file-util.h"
#include "hphp/runtime/base/execution-context.h"
#include "hphp/runtime/base/program-functions.h"

using namespace HPHP;
using std::set;

///////////////////////////////////////////////////////////////////////////////

Package::Package(const char *root, bool bShortTags /* = true */,
                 bool bAspTags /* = false */)
  : m_files(4000), m_dispatcher(0), m_lineCount(0), m_charCount(0) {
  m_root = FileUtil::normalizeDir(root);
  m_ar = AnalysisResultPtr(new AnalysisResult());
  m_fileCache = std::make_shared<FileCache>();
}

void Package::addAllFiles(bool force) {
  if (Option::PackageDirectories.empty() && Option::PackageFiles.empty()) {
    addDirectory("/", force);
  } else {
    for (set<string>::const_iterator iter = Option::PackageDirectories.begin();
         iter != Option::PackageDirectories.end(); ++iter) {
      addDirectory(*iter, force);
    }
    for (set<string>::const_iterator iter = Option::PackageFiles.begin();
         iter != Option::PackageFiles.end(); ++iter) {
      addSourceFile((*iter).c_str());
    }
  }
}

void Package::addInputList(const char *listFileName) {
  assert(listFileName && *listFileName);
  FILE *f = fopen(listFileName, "r");
  if (f == nullptr) {
    throw Exception("Unable to open %s: %s", listFileName,
                    folly::errnoStr(errno).c_str());
  }
  char fileName[PATH_MAX];
  while (fgets(fileName, sizeof(fileName), f)) {
    int len = strlen(fileName);
    if (fileName[len - 1] == '\n') fileName[len - 1] = '\0';
    len = strlen(fileName);
    if (len) {
      if (fileName[len - 1] == '/') {
        addDirectory(fileName, false);
      } else {
        addSourceFile(fileName);
      }
    }
  }
  fclose(f);
}

void Package::addStaticFile(const char *fileName) {
  assert(fileName && *fileName);
  m_extraStaticFiles.insert(fileName);
}

void Package::addStaticDirectory(const std::string path) {
  m_staticDirectories.insert(path);
}

void Package::addDirectory(const std::string &path, bool force) {
  addDirectory(path.c_str(), force);
}

void Package::addDirectory(const char *path, bool force) {
  m_directories.insert(path);
  addPHPDirectory(path, force);
}

void Package::addPHPDirectory(const char *path, bool force) {
  vector<string> files;
  if (force) {
    FileUtil::find(files, m_root, path, true);
  } else {
    FileUtil::find(files, m_root, path, true,
                   &Option::PackageExcludeDirs, &Option::PackageExcludeFiles);
    Option::FilterFiles(files, Option::PackageExcludePatterns);
  }
  int rootSize = m_root.size();
  for (unsigned int i = 0; i < files.size(); i++) {
    const string &file = files[i];
    assert(file.substr(0, rootSize) == m_root);
    m_filesToParse.insert(file.substr(rootSize));
  }
}

void Package::getFiles(std::vector<std::string> &files) const {
  assert(m_filesToParse.empty());

  files.clear();
  files.reserve(m_files.size());
  for (unsigned int i = 0; i < m_files.size(); i++) {
    const char *fileName = m_files.at(i);
    files.push_back(fileName);
  }
}

std::shared_ptr<FileCache> Package::getFileCache() {
  for (set<string>::const_iterator iter = m_directories.begin();
       iter != m_directories.end(); ++iter) {
    vector<string> files;
    FileUtil::find(files, m_root, iter->c_str(), false,
                   &Option::PackageExcludeStaticDirs,
                   &Option::PackageExcludeStaticFiles);
    Option::FilterFiles(files, Option::PackageExcludeStaticPatterns);
    for (unsigned int i = 0; i < files.size(); i++) {
      string &file = files[i];
      string rpath = file.substr(m_root.size());
      if (!m_fileCache->fileExists(rpath.c_str())) {
        Logger::Verbose("saving %s", file.c_str());
        m_fileCache->write(rpath.c_str(), file.c_str());
      }
    }
  }
  for (set<string>::const_iterator iter = m_staticDirectories.begin();
       iter != m_staticDirectories.end(); ++iter) {
    vector<string> files;
    FileUtil::find(files, m_root, iter->c_str(), false);
    for (unsigned int i = 0; i < files.size(); i++) {
      string &file = files[i];
      string rpath = file.substr(m_root.size());
      if (!m_fileCache->fileExists(rpath.c_str())) {
        Logger::Verbose("saving %s", file.c_str());
        m_fileCache->write(rpath.c_str(), file.c_str());
      }
    }
  }
  for (set<string>::const_iterator iter = m_extraStaticFiles.begin();
       iter != m_extraStaticFiles.end(); ++iter) {
    const char *file = iter->c_str();
    if (!m_fileCache->fileExists(file)) {
      string fullpath = m_root + file;
      Logger::Verbose("saving %s", fullpath.c_str());
      m_fileCache->write(file, fullpath.c_str());
    }
  }

  for (std::map<string,string>::const_iterator
         iter = m_discoveredStaticFiles.begin();
       iter != m_discoveredStaticFiles.end(); ++iter) {
    const char *file = iter->first.c_str();
    if (!m_fileCache->fileExists(file)) {
      const char *fullpath = iter->second.c_str();
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

class ParserWorker :
    public JobQueueWorker<std::pair<const char *,bool>, Package*, true, true> {
public:
  bool m_ret;
  ParserWorker() : m_ret(true) {}
  void doJob(JobType job) override {
    bool ret;
    try {
      Package *package = m_context;
      ret = package->parseImpl(job.first);
    } catch (Exception &e) {
      Logger::Error("%s", e.getMessage().c_str());
      ret = false;
    } catch (...) {
      Logger::Error("Fatal: An unexpected exception was thrown");
      m_ret = false;
      return;
    }
    if (!ret && job.second) {
      Logger::Error("Fatal: Unable to stat/parse %s", job.first);
      m_ret = false;
    }
  }

  void onThreadExit() override {
    hphp_memory_cleanup();
  }
};

void Package::addSourceFile(const char *fileName, bool check /* = false */) {
  if (fileName && *fileName) {
    Lock lock(m_mutex);
    auto canonFileName =
      FileUtil::canonicalize(String(fileName)).toCppString();
    bool inserted = m_filesToParse.insert(canonFileName).second;
    if (inserted && m_dispatcher) {
      ((JobQueueDispatcher<ParserWorker>*)m_dispatcher)->enqueue(
          std::make_pair(m_files.add(fileName), check));
    }
  }
}

bool Package::parse(bool check) {
  if (m_filesToParse.empty()) {
    return true;
  }

  unsigned int threadCount = Option::ParserThreadCount;
  if (threadCount > m_filesToParse.size()) {
    threadCount = m_filesToParse.size();
  }
  if (threadCount <= 0) threadCount = 1;

  JobQueueDispatcher<ParserWorker>
    dispatcher(threadCount, true, 0, false, this);

  m_dispatcher = &dispatcher;

  std::set<string> files;
  files.swap(m_filesToParse);

  dispatcher.start();
  for (std::set<string>::iterator iter = files.begin(), end = files.end();
       iter != end; ++iter) {
    addSourceFile((*iter).c_str(), check);
  }
  dispatcher.waitEmpty();

  m_dispatcher = 0;

  std::vector<ParserWorker*> workers;
  dispatcher.getWorkers(workers);
  for (unsigned int i = 0; i < workers.size(); i++) {
    ParserWorker *worker = workers[i];
    if (!worker->m_ret) return false;
  }

  return true;
}

bool Package::parse(const char *fileName) {
  return parseImpl(m_files.add(fileName));
}

bool Package::parseImpl(const char *fileName) {
  assert(fileName);
  if (fileName[0] == 0) return false;

  string fullPath;
  if (fileName[0] == '/') {
    fullPath = fileName;
  } else {
    fullPath = m_root + fileName;
  }

  struct stat sb;
  if (stat(fullPath.c_str(), &sb)) {
    if (fullPath.find(' ') == string::npos) {
      Logger::Error("Unable to stat file %s", fullPath.c_str());
    }
    return false;
  }
  if ((sb.st_mode & S_IFMT) == S_IFDIR) {
    Logger::Error("Unable to parse directory: %s", fullPath.c_str());
    return false;
  }

  int lines = 0;
  try {
    Logger::Verbose("parsing %s ...", fullPath.c_str());
    Scanner scanner(fullPath, Option::GetScannerType(), true);
    Compiler::Parser parser(scanner, fileName, m_ar, sb.st_size);
    parser.parse();
    lines = parser.line1();
  } catch (FileOpenException &e) {
    Logger::Error("%s", e.getMessage().c_str());
    return false;
  }

  m_lineCount += lines;
  struct stat fst;
  stat(fullPath.c_str(), &fst);
  m_charCount += fst.st_size;

  Lock lock(m_mutex);
  if (m_extraStaticFiles.find(fileName) == m_extraStaticFiles.end() &&
      m_discoveredStaticFiles.find(fileName) == m_discoveredStaticFiles.end()) {
    if (Option::CachePHPFile) {
      m_discoveredStaticFiles[fileName] = fullPath;
    } else {
      m_discoveredStaticFiles[fileName] = "";
    }
  }
  return true;
}

///////////////////////////////////////////////////////////////////////////////

void Package::saveStatsToFile(const char *filename, int totalSeconds) const {
  std::ofstream f(filename);
  if (f) {
    JSON::CodeError::OutputStream o(f, m_ar);
    JSON::CodeError::MapStream ms(o);

    ms.add("FileCount", getFileCount())
      .add("LineCount", getLineCount())
      .add("CharCount", getCharCount())
      .add("FunctionCount", m_ar->getFunctionCount())
      .add("ClassCount", m_ar->getClassCount())
      .add("TotalTime", totalSeconds);

    if (getLineCount()) {
      ms.add("AvgCharPerLine", getCharCount() / getLineCount());
    }
    if (m_ar->getFunctionCount()) {
      ms.add("AvgLinePerFunc", getLineCount()/m_ar->getFunctionCount());
    }

    ms.add("VariableTableFunctions");
    JSON::CodeError::ListStream ls(o);
    for (const std::string &f: m_ar->m_variableTableFunctions) {
      ls << f;
    }
    ls.done();

    ms.done();
    f.close();
  }
}
