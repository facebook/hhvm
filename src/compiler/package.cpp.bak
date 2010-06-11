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
#include <util/process.h>
#include <util/util.h>
#include <compiler/analysis/analysis_result.h>
#include <compiler/parser/parser.h>
#include <util/logger.h>
#include <util/json.h>
#include <compiler/analysis/symbol_table.h>
#include <compiler/option.h>
#include <util/db_conn.h>
#include <util/db_query.h>
#include <util/exception.h>
#include <util/preprocess.h>

using namespace HPHP;
using namespace std;

///////////////////////////////////////////////////////////////////////////////
// statics

void (*Package::m_hookHandler)(Package *package, const char *path,
                               HphpHookUniqueId id);

///////////////////////////////////////////////////////////////////////////////

Package::Package(const char *root, bool bShortTags /* = true */,
                 bool bAspTags /* = false */)
  : m_bShortTags(bShortTags), m_bAspTags(bAspTags), m_files(4000),
    m_lineCount(0), m_charCount(0) {
  m_root = root;
  if (!m_root.empty() && m_root[m_root.size() - 1] != '/') m_root += "/";
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

void Package::addSourceFile(const char *fileName) {
  ASSERT(fileName && *fileName);
  m_files.add(Util::canonicalize(fileName).c_str());
}

void Package::addListFiles(const char *listFileName) {
  ASSERT(listFileName && *listFileName);
  FILE *f = fopen(listFileName, "r");
  if (f == NULL) {
    throw Exception("Unable to open %s: %s", listFileName,
                    Util::safe_strerror(errno).c_str());
  }
  char fileName[PATH_MAX];
  while (fgets(fileName, sizeof(fileName), f)) {
    int len = strlen(fileName);
    if (fileName[len - 1] == '\n') fileName[len-1] = '\0';
    addSourceFile(fileName);
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

  addDirectory(path, "*.php", force);
#ifdef HAVE_PHPT
  addDirectory(path, "*.phpt", force);
#endif
  addDirectory(path, "", force); // look for PHP files without postfix

  if (m_hookHandler) {
    m_hookHandler(this, path, onPackageAddDirectory);
  }
}

void Package::addDependencyParents(const char *path, const char *postfix,
                                   DependencyGraph::KindOf kindOf) {
  vector<string> files;
  findFiles(files, path, postfix);
  DependencyGraphPtr dep = m_ar->getDependencyGraph();
  int rootSize = m_root.size();
  for (unsigned int i = 0; i < files.size(); i++) {
    const string &file = files[i];
    ASSERT(file.substr(0, rootSize) == m_root);
    dep->addParent(kindOf, "", file.substr(rootSize), ConstructPtr());
  }
}

void Package::findFiles(std::vector<std::string> &out, const char *path,
                        const char *postfix) {
  ASSERT(postfix && *postfix);
  if (!path) path = "";
  if (*path == '/') path++;

  string fullPath = m_root + path;
  const char *argv[] = {"", "-L", (char*)fullPath.c_str(),
                        "-name", (char*)postfix, NULL};
  string files;
  Process::Exec("find", argv, NULL, files);
  Util::split('\n', files.c_str(), out, true);
}

void Package::findPHPFiles(std::vector<std::string> &out, const char *path) {
  if (!path) path = "";
  if (*path == '/') path++;

  string fullPath = m_root + path;
  const char *argv[] = {"", "-L", (char*)fullPath.c_str(),
                        "-type", "f",
                        "-regex", ".*/[A-Za-z0-9_\\-]+",
                        "-not", "-regex", ".*/\\.svn/.*",
                        "-not", "-regex", ".*/\\.git/.*",
                        /* Do not use [A-Z] below. That seems to be
                           broken outside of the C locale, and we dont
                           know what locale find will run in */
                        "-not", "-regex", ".*/[ABCDEFGHIJKLMNOPQRSTUVWXYZ]+",
                        "-not", "-regex", ".*/tags",

                        "-exec", "perl", "-n", "-e",
                        "/php\\s*$/ && print $ARGV.\"\\n\"; exit",
                        "{}", ";",

                        NULL};

  string files;
  Process::Exec("find", argv, NULL, files);
  Util::split('\n', files.c_str(), out, true);
}

void Package::findNonPHPFiles(vector<string> &out, const char *path,
                              bool exclude) {
  if (!path) path = "";
  if (*path == '/') path++;

  string fullPath = m_root + path;
  DIR *dir = opendir(fullPath.c_str());
  if (dir == NULL) {
    Logger::Error("findNonPHPFiles: unable to open directory %s",
                  fullPath.c_str());
    return;
  }

  dirent *e;
  while (e = readdir(dir)) {
    char *ename = e->d_name;
    if (strcmp(ename, ".") == 0 || strcmp(ename, "..") == 0) {
      continue;
    }

    string fe = fullPath +
      (fullPath[fullPath.length() - 1] != '/' ? "/" : "") + ename;
    struct stat se;
    if (stat(fe.c_str(), &se) != 0) {
      // unable to stat, maybe a broken symbolic link
      // do not include it
      continue;
    }
    if ((se.st_mode & S_IFMT) == S_IFDIR) {
      if (strcmp(ename, ".svn") && strcmp(ename, ".git")) {
        string subdir = path;
        if (subdir[subdir.length() - 1] != '/') subdir += '/';
        subdir += ename;
        findNonPHPFiles(out, subdir.c_str(), exclude);
      }
      continue;
    }

    if (strcmp(ename, "tags") == 0) {
      continue;
    }
    size_t len = strlen(ename);
    if (len >= 4 && ename[len - 4] == '.' && ename[len - 3] == 'p' &&
        ename[len - 2] == 'h' && ename[len - 1] == 'p') {
      // ending with .php
      continue;
    }
#ifdef HAVE_PHPT
    if (len >= 5 && ename[len - 5] == '.' && ename[len - 4] == 'p' &&
        ename[len - 3] == 'h' && ename[len - 2] == 'p' &&
        ename[len - 1] == 't') {
      // ending with .phpt
      continue;
    }
#endif
    string fullname = string(path) + "/" + ename;
    if (!exclude ||
        Option::PackageExcludeStaticFiles.find(fullname) ==
        Option::PackageExcludeStaticFiles.end()) {
      out.push_back(fe);
    }
  }

  closedir(dir);
}

void Package::addDirectory(const char *path, const char *postfix, bool force) {
  ASSERT(path && *path);
  if (*path == '/') path++;

  vector<string> files;
  if (postfix && *postfix) {
    findFiles(files, path, postfix);
  } else {
    findPHPFiles(files, path);
  }
  for (unsigned int i = 0; i < files.size(); i++) {
    const string &file = files[i];
    bool excluded = false;
    if (!force) {
      for (set<string>::const_iterator iter =
             Option::PackageExcludeDirs.begin();
           iter != Option::PackageExcludeDirs.end(); ++iter) {
        if (file.find(*iter) == Option::RootDirectory.size()) {
          excluded = true;
          break;
        }
      }
    }
    if (!excluded) {
      ASSERT(file.substr(0, m_root.size()) == m_root);
      string name = file.substr(m_root.size());
      if (Option::PackageExcludeFiles.find(name) ==
          Option::PackageExcludeFiles.end()) {
        m_files.add(name.c_str());
      }
    }
  }
}

void Package::getFiles(std::vector<std::string> &files) const {
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
    findNonPHPFiles(files, iter->c_str(), true);
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
    findNonPHPFiles(files, iter->c_str(), false);
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
  return m_fileCache;
}

///////////////////////////////////////////////////////////////////////////////

bool Package::parse() {
  hphp_const_char_set files;
  for (unsigned int i = 0; i < m_files.size(); i++) {
    const char *fileName = m_files.at(i);
    if (files.find(fileName) == files.end()) {
      files.insert(fileName);
      if (!parseImpl(fileName)) return false;
    }
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
    Logger::Error("Unable to stat file %s", fullPath.c_str());
    return false;
  }

  try {
    ifstream f(fullPath.c_str());
    stringstream ss;
    istream *is = Option::EnableXHP ? preprocessXHP(f, ss, fullPath) : &f;

    Scanner scanner(new ylmm::basic_buffer(*is, false, true),
                    m_bShortTags, m_bAspTags);
    Logger::Verbose("parsing %s ...", fullPath.c_str());
    ParserPtr parser(new Parser(scanner, fileName, sb.st_size, m_ar));
    if (parser->parse()) {
      throw Exception("Unable to parse file: %s\n%s", fullPath.c_str(),
                      parser->getMessage().c_str());
    }

    m_lineCount += parser->line1();
    struct stat fst;
    stat(fullPath.c_str(), &fst);
    m_charCount += fst.st_size;

  } catch (std::runtime_error) {
    Logger::Error("Unable to open file %s", fullPath.c_str());
    return false;
  }

  if (!m_fileCache->fileExists(fileName) &&
      m_extraStaticFiles.find(fileName) == m_extraStaticFiles.end()) {
    if (Option::CachePHPFile) {
      m_fileCache->write(fileName, fullPath.c_str()); // name + content
    } else {
      m_fileCache->write(fileName); // just name, without content
    }
  }
  return true;
}

///////////////////////////////////////////////////////////////////////////////

void Package::saveStatsToFile(const char *filename, int totalSeconds) const {
  ofstream f(filename);
  if (f) {
    JSON::OutputStream o(f);

    f << "var FileCount = " << getFileCount() << ";\n"
      << "var LineCount = " << getLineCount() << ";\n"
      << "var CharCount = " << getCharCount() << ";\n"
      << "var FunctionCount = " << m_ar->getFunctionCount() << ";\n"
      << "var ClassCount = " << m_ar->getClassCount() << ";\n"
      << "var TotalTime = " << totalSeconds << ";\n";

    if (getLineCount()) {
      f << "var AvgCharPerLine = " << (getCharCount()/getLineCount()) << ";\n";
    }
    if (m_ar->getFunctionCount()) {
      f << "var AvgLinePerFunc = ";
      f << (getLineCount()/m_ar->getFunctionCount()) << ";\n";
    }

    std::map<std::string, int> counts;
    SymbolTable::CountTypes(counts);
    m_ar->countReturnTypes(counts);
    f << "var SymbolTypes = ";
    o << counts;
    f << ";\n";

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
