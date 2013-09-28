/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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

#ifndef incl_HPHP_PACKAGE_H_
#define incl_HPHP_PACKAGE_H_

#include "hphp/compiler/hphp.h"
#include "hphp/util/string-bag.h"
#include "hphp/util/file-cache.h"
#include "hphp/util/mutex.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

DECLARE_BOOST_TYPES(ServerData);
DECLARE_BOOST_TYPES(AnalysisResult);

/**
 * A package contains a list of directories and files that will be parsed
 * and analyzed together. No files outside of a package will be considered
 * in type inferences. One single AnalysisResult will be generated and it
 * contains all classes, functions, variables, constants and their types.
 * Therefore, a package is really toppest entry point for parsing.
 */
class Package {
public:
  explicit Package(const char *root,
                   bool bShortTags = true,
                   bool bAspTags = false);

  void addAllFiles(bool force); // add from Option::PackageDirectories/Files

  void addSourceFile(const char *fileName, bool check = false);
  void addInputList(const char *listFileName);
  void addStaticFile(const char *fileName);
  void addDirectory(const std::string &path, bool force);
  void addDirectory(const char *path, bool force);
  void addStaticDirectory(const std::string path);
  void addPHPDirectory(const char *path, bool force);

  bool parse(bool check);
  bool parse(const char *fileName);
  bool parseImpl(const char *fileName);

  AnalysisResultPtr getAnalysisResult() { return m_ar;}
  int getFileCount() const { return m_files.size();}
  int getLineCount() const { return m_lineCount;}
  int getCharCount() const { return m_charCount;}
  void getFiles(std::vector<std::string> &files) const;

  void saveStatsToFile(const char *filename, int totalSeconds) const;
  int saveStatsToDB(ServerDataPtr server, int totalSeconds,
                    const std::string &branch, int revision) const;
  void commitStats(ServerDataPtr server, int runId) const;

  const std::string& getRoot() const { return m_root;}
  FileCachePtr getFileCache();

private:
  std::string m_root;
  std::set<std::string> m_filesToParse;
  StringBag m_files;
  void *m_dispatcher;

  Mutex m_mutex;
  AnalysisResultPtr m_ar;
  int m_lineCount;
  int m_charCount;

  FileCachePtr m_fileCache;
  std::set<std::string> m_directories;
  std::set<std::string> m_staticDirectories;
  std::set<std::string> m_extraStaticFiles;
  std::map<std::string,std::string> m_discoveredStaticFiles;
};

///////////////////////////////////////////////////////////////////////////////
}
#endif // incl_HPHP_PACKAGE_H_
