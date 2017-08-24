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

#ifndef incl_HPHP_OPTION_H_
#define incl_HPHP_OPTION_H_

#include <map>
#include <set>
#include <vector>
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/util/functional.h"
#include "hphp/util/string-bag.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

struct Hdf;

struct IniSettingMap;

struct Option {
  /**
   * Load options from different sources.
   */
  static void Load(const IniSettingMap& ini, Hdf &config);
  static void Load(); // load default options

  /**
   * Directories to add to a package.
   */
  static std::string RootDirectory;
  static std::set<std::string> PackageDirectories;

  /**
   * Files to add to a package.
   */
  static std::set<std::string> PackageFiles;

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
   * Directories in which files are parsed on-demand, when parse-on-demand
   * is off.
   */
  static std::vector<std::string> ParseOnDemandDirs;

  /**
   * Whether to store PHP source files in static file cache.
   */
  static bool CachePHPFile;

  /**
   * Legal root directory expressions in an include expression. For example,
   *
   *   include_once $PHP_ROOT . '/lib.php';
   *
   * Here, "$PHP_ROOT" is a legal include root. Stores what it resolves to.
   *
   *   Option::IncludeRoots["$PHP_ROOT"] = "";
   *   Option::IncludeRoots["$LIB_ROOT"] = "lib";
   */
  static std::map<std::string, std::string> IncludeRoots;
  static std::map<std::string, std::string> AutoloadRoots;
  static std::vector<std::string> IncludeSearchPaths;

  /**
   * PHP include root expression to use when generating PHP trimmed code.
   */
  static std::string DefaultIncludeRoot;

  /**
   * PHP functions that can be assumed to always return a certain constant
   * value.
   */
  static hphp_string_imap<std::string> ConstantFunctions;

  /**
   * Optimization flags
   */
  static bool PreOptimization;

  /**
   * CodeGenerator options for PHP.
   */
  static bool GeneratePickledPHP;
  static bool GenerateInlinedPHP;
  static bool GenerateTrimmedPHP;
  static bool ConvertSuperGlobals;    // $GLOBALS['var'] => global $var
  static std::string ProgramPrologue;
  static std::string TrimmedPrologue;
  static std::set<std::string> VolatileClasses;
  static std::map<std::string,std::string, stdltistr> AutoloadClassMap;
  static std::map<std::string,std::string, stdltistr> AutoloadFuncMap;
  static std::map<std::string,std::string> AutoloadConstMap;
  static std::string AutoloadRoot;

  /**
   * CodeGenerator options for HHBC.
   */
  static bool GenerateTextHHBC;
  static bool GenerateHhasHHBC;
  static bool GenerateBinaryHHBC;
  static std::string RepoCentralPath;
  static bool RepoDebugInfo;

  static std::vector<std::string> APCProfile;

  /**
   * Names of hot and cold functions to be marked in sources.
   */
  static std::map<std::string, std::string> FunctionSections;

  /**
   * A somewhat unique prefix for system identifiers.
   */
  static std::string IdPrefix;
  static std::string LambdaPrefix;
  static std::string Tab;

  /**
   * Name resolution helpers.
   */
  static const char *UserFilePrefix;

  /**
   * Turn it off for cleaner unit tests.
   */
  static bool KeepStatementsWithNoEffect;

  /**
   * Whether or not name matches AUTOLOAD files. If not, returns empty. If
   * yes, returns root directory for the file.
   */
  static std::string GetAutoloadRoot(const std::string &name);

  /**
   * Turning a file name into an identifier. When id is false, preserve
   * "/" in file paths.
   */
  static std::string MangleFilename(const std::string &name, bool id);

  static std::string ProgramName;

  static bool ParseTimeOpts;
  static bool EnableHipHopExperimentalSyntax;
  static bool EnableShortTags;
  static bool EnableAspTags;
  static int ParserThreadCount;

  static int GetScannerType();

  /**
   * "Volatile" means a class or a function can be declared dynamically.
   */
  static bool AllVolatile;

  /**
   * Output options
   */
  static bool GenerateDocComments;
  static bool DumpAst;
  static bool WholeProgram;
  static bool UseHHBBC;  // see hhbbc/README
  static bool RecordErrors;

private:
  static StringBag OptionStrings;

  static void LoadRootHdf(const IniSettingMap& ini, const Hdf &roots,
                          const std::string& name,
                          std::map<std::string, std::string> &map);
};

///////////////////////////////////////////////////////////////////////////////
}
#endif // incl_HPHP_OPTION_H_
