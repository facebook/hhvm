/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010 Facebook, Inc. (http://www.facebook.com)          |
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

#include <compiler/package.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <compiler/analysis/analysis_result.h>
#include <compiler/parser/parser.h>
#include <compiler/analysis/symbol_table.h>
#include <compiler/analysis/variable_table.h>
#include <compiler/option.h>
#include <util/process.h>
#include <util/util.h>
#include <util/logger.h>
#include <util/json.h>
#include <util/db_conn.h>
#include <util/db_query.h>
#include <util/exception.h>
#include <util/preprocess.h>
#include <util/job_queue.h>
#include <runtime/base/execution_context.h>

using namespace HPHP;
using namespace std;

///////////////////////////////////////////////////////////////////////////////

Package::Package(const char *root, bool bShortTags /* = true */,
                 bool bAspTags /* = false */)
  : m_bShortTags(bShortTags), m_bAspTags(bAspTags), m_files(4000),
    m_dispatcher(0), m_lineCount(0), m_charCount(0) {
  m_root = Util::normalizeDir(root);
  m_ar = AnalysisResultPtr(new AnalysisResult());
  m_fileCache = FileCachePtr(new FileCache());
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
  ASSERT(listFileName && *listFileName);
  FILE *f = fopen(listFileName, "r");
  if (f == NULL) {
    throw Exception("Unable to open %s: %s", listFileName,
                    Util::safe_strerror(errno).c_str());
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
  ASSERT(fileName && *fileName);
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
    Util::find(files, m_root, path, true);
  } else {
    Util::find(files, m_root, path, true,
               &Option::PackageExcludeDirs, &Option::PackageExcludeFiles);
    Option::FilterFiles(files, Option::PackageExcludePatterns);
  }
  int rootSize = m_root.size();
  for (unsigned int i = 0; i < files.size(); i++) {
    const string &file = files[i];
    ASSERT(file.substr(0, rootSize) == m_root);
    m_filesToParse.insert(file.substr(rootSize));
  }
}

void Package::getFiles(std::vector<std::string> &files) const {
  ASSERT(m_filesToParse.empty());

  files.clear();
  files.reserve(m_files.size());
  for (unsigned int i = 0; i < m_files.size(); i++) {
    const char *fileName = m_files.at(i);
    files.push_back(fileName);
  }
}

FileCachePtr Package::getFileCache() {
  for (set<string>::const_iterator iter = m_directories.begin();
       iter != m_directories.end(); ++iter) {
    vector<string> files;
    Util::find(files, m_root, iter->c_str(), false,
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
    Util::find(files, m_root, iter->c_str(), false);
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

  for (map<string,string>::const_iterator
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
    public JobQueueWorker<std::pair<const char *,bool>, true, true> {
public:
  bool m_ret;
  ParserWorker() : m_ret(true) {}
  virtual void doJob(JobType job) {
    bool ret;
    try {
      Package *package = (Package*)m_opaque;
      ret = package->parseImpl(job.first);
    } catch (Exception &e) {
      Logger::Error("%s", e.getMessage().c_str());
      ret = false;
    }
    if (!ret && job.second) m_ret = false;
  }
};

void Package::addSourceFile(const char *fileName, bool check /* = false */) {
  if (fileName && *fileName) {
    Lock lock(m_mutex);
    bool inserted = m_filesToParse.insert(Util::canonicalize(fileName)).second;
    if (inserted && m_dispatcher) {
      ((JobQueueDispatcher<ParserWorker::JobType,
        ParserWorker>*)m_dispatcher)->enqueue(
          make_pair(m_files.add(fileName), check));
    }
  }
}

bool Package::parse() {
  if (m_filesToParse.empty()) {
    return true;
  }

  unsigned int threadCount = Option::ParserThreadCount;
  if (threadCount > m_filesToParse.size()) {
    threadCount = m_filesToParse.size();
  }
  if (threadCount <= 0) threadCount = 1;

  JobQueueDispatcher<ParserWorker::JobType, ParserWorker>
    dispatcher(threadCount, true, 0, this);

  m_dispatcher = &dispatcher;

  std::set<string> files;
  files.swap(m_filesToParse);

  dispatcher.start();
  for (std::set<string>::iterator iter = files.begin(), end = files.end();
       iter != end; ++iter) {
    addSourceFile((*iter).c_str(), true);
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
  ASSERT(fileName);
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
    Scanner scanner(fullPath.c_str(), Option::ScannerType);
    Compiler::Parser parser(scanner, fileName, m_ar, sb.st_size);
    if (!parser.parse()) {
      throw Exception("Unable to parse file: %s %s", fullPath.c_str(),
                      parser.getMessage().c_str());
    }

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
  ofstream f(filename);
  if (f) {
    JSON::OutputStream o(f);
    JSON::MapStream ms(o);

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

    std::map<std::string, int> counts;
    SymbolTable::CountTypes(counts);
    m_ar->countReturnTypes(counts);

    ms.add("SymbolTypes");
    o << counts;

    ms.add("VariableTableFunctions");
    JSON::ListStream ls(o);
    BOOST_FOREACH(const std::string &f, m_ar->m_variableTableFunctions) {
      ls << f;
    }
    ls.done();

    ms.done();
    f.close();
  }
}

int Package::saveStatsToDB(ServerDataPtr server, int totalSeconds,
                           const std::string &branch, int revision) const {
  std::map<std::string, int> counts;
  SymbolTable::CountTypes(counts);
  m_ar->countReturnTypes(counts);
  ostringstream sout;
  JSON::OutputStream o(sout);
  o << counts;

  DBConn conn;
  conn.open(server);

  const char *sql = "INSERT INTO hphp_run (branch, revision, file, line, "
    "byte, program, function, class, types, time)";
  DBQuery q(&conn, sql);
  q.insert("'%s', %d, %d, %d, %d, %d, %d, %d, '%s', %d",
           branch.c_str(), revision,
           getFileCount(), getLineCount(), getCharCount(),
           1, m_ar->getFunctionCount(),
           m_ar->getClassCount(), sout.str().c_str(), totalSeconds);
  q.execute();
  return conn.getLastInsertId();
}

void Package::commitStats(ServerDataPtr server, int runId) const {
  DBConn conn;
  conn.open(server);

  {
    DBQuery q(&conn, "UPDATE hphp_dep");
    q.setField("parent_file = parent");
    q.filterBy("run = %d", runId);
    q.filterBy("kind IN ('PHPInclude', 'PHPTemplate')");
    q.execute();
  }
  {
    DBQuery q(&conn, "UPDATE hphp_run");
    q.setField("committed = 1");
    q.filterBy("id = %d", runId);
    q.execute();
  }
}
