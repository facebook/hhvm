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
#include <string>
#include <vector>

#include "hphp/util/hash-map.h"
#include "hphp/util/hash-set.h"

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
  static hphp_fast_string_set PackageExcludeDirs;
  static hphp_fast_string_set PackageExcludeFiles;
  static hphp_fast_string_set PackageExcludePatterns;
  static hphp_fast_string_set PackageExcludeStaticFiles;
  static hphp_fast_string_set PackageExcludeStaticDirs;
  static hphp_fast_string_set PackageExcludeStaticPatterns;

  static bool IsFileExcluded(const std::string& file,
                             const hphp_fast_string_set& patterns);
  static void FilterFiles(std::vector<std::string>& files,
                          const hphp_fast_string_set& patterns);

  /**
   * Whether to store PHP source files in static file cache.
   */
  static bool CachePHPFile;

  /*
   * If true, HHBBC will const fold File and Dir bytecodes to static
   * strings (using SourceRoot).
   */
  static bool ConstFoldFileBC;

  /*
   * Whether to generate HHBC, HHAS, a textual dump of HHBC, or none.
   */
  static bool GenerateTextHHBC;
  static bool GenerateHhasHHBC;
  static bool GenerateBinaryHHBC;
  static bool NoOutputHHBC;

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

  /*
   * If true, as an optimization, we'll assume the files have already
   * been stored with extern_worker previously and proceed as if they
   * had. If not (so execution fails), we'll then store them and try
   * again. This avoids doing a lot of redundant stores in the common
   * case.
   */
  static bool ParserOptimisticStore;

  /*
   * When an ActiveDeployment is specified, we disable SymbolRefs
   * logic and only compile the requested files. This option will
   * be used to force include SymbolRefs even when ActiveDeployment
   * is specified.
   */
  static bool ForceEnableSymbolRefs;

  /* Config passed to extern_worker::Client */
  static std::string ExternWorkerUseCase;
  static std::string ExternWorkerFeaturesFile;
  static bool ExternWorkerForceSubprocess;
  static int ExternWorkerTimeoutSecs;
  static bool ExternWorkerUseExecCache;
  static bool ExternWorkerCleanup;
  static bool ExternWorkerUseRichClient;
  static bool ExternWorkerUseZippyRichClient;
  static bool ExternWorkerUseP2P;
  static int ExternWorkerCasConnectionCount;
  static int ExternWorkerEngineConnectionCount;
  static int ExternWorkerAcConnectionCount;
  static bool ExternWorkerVerboseLogging;
  static int ExternWorkerThrottleRetries;
  static int ExternWorkerThrottleBaseWaitMSecs;
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
