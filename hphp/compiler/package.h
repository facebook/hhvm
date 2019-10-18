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

#ifndef incl_HPHP_PACKAGE_H_
#define incl_HPHP_PACKAGE_H_

#include <map>
#include <memory>
#include <set>
#include <vector>

#include <folly/Optional.h>

#include "hphp/util/file-cache.h"
#include "hphp/util/mutex.h"
#include "hphp/hhbbc/hhbbc.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

struct AnalysisResult;
using AnalysisResultPtr = std::shared_ptr<AnalysisResult>;

/**
 * A package contains a list of directories and files that will be parsed
 * and analyzed together. No files outside of a package will be considered
 * in type inferences. One single AnalysisResult will be generated and it
 * contains all classes, functions, variables, constants and their types.
 * Therefore, a package is really toppest entry point for parsing.
 */
struct Package {
  explicit Package(const char *root, bool bShortTags = true);

  void addAllFiles(bool force); // add from Option::PackageDirectories/Files

  void addSourceFile(const std::string& fileName, bool check = false);
  void addInputList(const std::string& listFileName);
  void addStaticFile(const std::string& fileName);
  void addDirectory(const std::string &path, bool force);
  void addStaticDirectory(const std::string& path);
  void addSourceDirectory(const std::string& path, bool force);

  bool parse(bool check, std::thread& unit_emitter_thread);
  bool parseImpl(const std::string* fileName);

  AnalysisResultPtr getAnalysisResult() { return m_ar;}
  void resetAr() { m_ar.reset(); }
  int getFileCount() const { return m_filesToParse.size();}
  int getLineCount() const { return m_lineCount;}
  int getCharCount() const { return m_charCount;}

  void saveStatsToFile(const char *filename, int totalSeconds) const;

  const std::string& getRoot() const { return m_root;}
  std::shared_ptr<FileCache> getFileCache();

  void addUnitEmitter(std::unique_ptr<UnitEmitter> ue);
private:
  std::string m_root;
  std::set<std::string> m_filesToParse;

  void *m_dispatcher;

  Mutex m_mutex;
  AnalysisResultPtr m_ar;
  int m_lineCount;
  int m_charCount;

  std::shared_ptr<FileCache> m_fileCache;
  std::map<std::string,bool> m_directories;
  std::set<std::string> m_staticDirectories;
  std::set<std::string> m_extraStaticFiles;
  std::map<std::string,std::string> m_discoveredStaticFiles;
  folly::Optional<HHBBC::UnitEmitterQueue> m_ueq;
  hphp_fast_set<std::string> m_locally_cached_bytecode;
};

///////////////////////////////////////////////////////////////////////////////
}
#endif // incl_HPHP_PACKAGE_H_
