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

#ifndef __PACKAGE_H__
#define __PACKAGE_H__

#include <compiler/hphp.h>
#include <util/string_bag.h>
#include <compiler/analysis/dependency_graph.h>
#include <util/file_cache.h>
#include <compiler/hphp_unique.h>

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
  friend class PackageHook;
public:
  Package(const char *root, bool bShortTags = true, bool bAspTags = false);

  void addAllFiles(bool force); // add from Option::PackageDirectories/Files

  void addSourceFile(const char *fileName);
  void addListFiles(const char *listFileName);
  void addStaticFile(const char *fileName);
  void addDirectory(const std::string &path, bool force);
  void addDirectory(const char *path, bool force);
  void addStaticDirectory(const std::string path);
  void addDirectory(const char *path, const char *postfix, bool force);

  bool parse();
  bool parse(const char *fileName);

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

  static void setHookHandler(void (*hookHandler)(Package *package,
                                                 const char *path,
                                                 HphpHookUniqueId id)) {
    m_hookHandler = hookHandler;
  }

private:
  std::string m_root;
  bool m_bShortTags;
  bool m_bAspTags;
  StringBag m_files;
  AnalysisResultPtr m_ar;
  int m_lineCount;
  int m_charCount;

  FileCachePtr m_fileCache;
  std::set<std::string> m_directories;
  std::set<std::string> m_staticDirectories;
  std::set<std::string> m_extraStaticFiles;

  void findFiles(std::vector<std::string> &out, const char *path,
                 const char *postfix);
  void findPHPFiles(std::vector<std::string> &out, const char *path);
  void findNonPHPFiles(std::vector<std::string> &out, const char *path,
                       bool exclude);
  void addDependencyParents(const char *path, const char *postfix,
                            DependencyGraph::KindOf kindOf);

  bool parseImpl(const char *fileName);

  // hook
  static void (*m_hookHandler)(Package *package, const char *path,
                               HphpHookUniqueId id);
};

///////////////////////////////////////////////////////////////////////////////
}
#endif // __PACKAGE_H__
