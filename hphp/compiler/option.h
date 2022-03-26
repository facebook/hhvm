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

#pragma once

#include <map>
#include <set>
#include <string>
#include <vector>

#include "hphp/util/functional.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

struct Hdf;

struct IniSettingMap;

struct Option {
  /**
   * Load options from different sources.
   */
  static void Load(const IniSettingMap& ini, Hdf &config);

  /**
   * File path patterns for excluding files from a package scan of programs.
   */
  static std::set<std::string> PackageExcludeDirs;
  static std::set<std::string> PackageExcludeFiles;
  static std::set<std::string> PackageExcludePatterns;
  static std::set<std::string> PackageExcludeStaticFiles;
  static std::set<std::string> PackageExcludeStaticDirs;
  static std::set<std::string> PackageExcludeStaticPatterns;

  static bool IsFileExcluded(const std::string &file,
                             const std::set<std::string> &patterns);
  static void FilterFiles(std::vector<std::string> &files,
                          const std::set<std::string> &patterns);

  /**
   * Whether to store PHP source files in static file cache.
   */
  static bool CachePHPFile;

  /*
   * Autoload information for resolving parse on-demand
   */
  static std::map<std::string,std::string, stdltistr> AutoloadClassMap;
  static std::map<std::string,std::string, stdltistr> AutoloadFuncMap;
  static std::map<std::string,std::string> AutoloadConstMap;
  static std::string AutoloadRoot;

  /*
   * Whether to generate HHBC, HHAS, or a textual dump of HHBC
   */
  static bool GenerateTextHHBC;
  static bool GenerateHhasHHBC;
  static bool GenerateBinaryHHBC;

  /*
   * Number of threads to use for parsing
   */
  static int ParserThreadCount;

  /*
   * The number of files (on average) we'll group together for a
   * worker during parsing. Files in directories (including sub-dirs)
   * with more than ParserDirGroupSizeLimit files won't be grouped
   * with files outside of those directories.
   */
  static int ParserGroupSize;
  static int ParserDirGroupSizeLimit;

  /*
   * If true, we'll free async state (which can take a while) in
   * another thread asynchronously. If false, it will be done
   * synchronously.
   */
  static bool ParserAsyncCleanup;

  /* Config passed to extern_worker::Client */
  static std::string ExternWorkerUseCase;
  static bool ExternWorkerForceSubprocess;
  static int ExternWorkerTimeoutSecs;
  static bool ExternWorkerUseExecCache;
  static bool ExternWorkerCleanup;
  static std::string ExternWorkerWorkingDir;

private:
  static const int kDefaultParserGroupSize;
  static const int kDefaultParserDirGroupSizeLimit;

  static void LoadRootHdf(const IniSettingMap& ini, const Hdf &roots,
                          const std::string& name,
                          std::map<std::string, std::string> &map);
};

///////////////////////////////////////////////////////////////////////////////
}
