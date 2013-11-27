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

#ifndef incl_HPHP_OPTION_H_
#define incl_HPHP_OPTION_H_

#include "hphp/util/hdf.h"
#include "hphp/util/string-bag.h"
#include "hphp/util/base.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////
DECLARE_BOOST_TYPES(BlockScope);
DECLARE_BOOST_TYPES(FileScope);

class Option {
public:
  /**
   * Directory that has system HPHP files for loading builtin classes, etc.
   */
  static std::string GetSystemRoot();

  /**
   * Load options from different sources.
   */
  static void Load(Hdf &config);
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
   * PHP functions that will take a function name and make a dynamic call.
   */
  static std::map<std::string, int> DynamicFunctionCalls;

  /**
   * Optimization flags
   */
  static bool PreOptimization;
  static bool PostOptimization;
  static bool AnalyzePerfectVirtuals;
  static bool HardTypeHints;

  /**
   * Separate compilation
   */
  static bool SeparateCompilation;
  static bool SeparateCompLib;

  /**
   * CodeGenerator options for PHP.
   */
  static bool GeneratePickledPHP;
  static bool GenerateInlinedPHP;
  static bool GenerateTrimmedPHP;
  static bool GenerateInferredTypes;  // comments on constant/variable tables
  static bool ConvertSuperGlobals;    // $GLOBALS['var'] => global $var
  static bool ConvertQOpExpressions;  // $var = $exp ? $yes : $no => if-else
  static std::string ProgramPrologue;
  static std::string TrimmedPrologue;
  static std::vector<std::string> DynamicFunctionPrefixes;
  static std::vector<std::string> DynamicFunctionPostfixes;
  static std::vector<std::string> DynamicMethodPrefixes;
  static std::vector<std::string> DynamicMethodPostfixes;
  static std::vector<std::string> DynamicClassPrefixes;
  static std::vector<std::string> DynamicClassPostfixes;
  static std::set<std::string> DynamicInvokeFunctions;
  static std::set<std::string> VolatileClasses;
  static std::map<std::string,std::string> AutoloadClassMap;
  static std::map<std::string,std::string> AutoloadFuncMap;
  static std::map<std::string,std::string> AutoloadConstMap;
  static std::string AutoloadRoot;

  /**
   * CodeGenerator options for HHBC.
   */
  static bool GenerateTextHHBC;
  static bool GenerateBinaryHHBC;
  static std::string RepoCentralPath;
  static bool RepoDebugInfo;

  /**
   * Names of hot and cold functions to be marked in sources.
   */
  static std::map<std::string, std::string> FunctionSections;

  /**
   * A somewhat unique prefix for system identifiers.
   */
  static std::string IdPrefix;
  static std::string LabelEscape;
  static std::string LambdaPrefix;
  static std::string Tab;

  /**
   * Name resolution helpers.
   */
  static const char *UserFilePrefix;
  static const char *ClassHeaderPrefix;

  /**
   * Turn it off for cleaner unit tests.
   */
  static bool KeepStatementsWithNoEffect;

  /**
   * When we have an include inside a function or a method, how many levels
   * do we expand? If 0, we rely on "require" vs. "include" to give explicit
   * instructions. If 1, we only inline just one level, and all deeper
   * includes are considered as libraries, and they will be moved to top
   * level. If -1, we completely disable conditional include handling.
   */
  static int ConditionalIncludeExpandLevel;

  /**
   * Maximum number of examplar programs to store in each output.
   */
  static int DependencyMaxProgram;
  static int CodeErrorMaxProgram;

  /**
   * Whether or not name matches dynamic function/class prefx/postfix lists.
   */
  static bool IsDynamicFunction(bool method, const std::string &name);
  static bool IsDynamicClass(const std::string &name);

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

  enum EvalLevel {
    NoEval = 0, // calling eval is a fatal
    LimitedEval = 1, // eval is supported in a limited way with no perf hit
    FullEval = 2 // eval is supported but with a performance hit
  };

  static EvalLevel EnableEval;

  static std::string ProgramName;

  static bool ParseTimeOpts;
  static bool OutputHHBC;
  static bool EnableHipHopSyntax;
  static bool EnableZendCompat;
  static bool JitEnableRenameFunction;
  static bool EnableHipHopExperimentalSyntax;
  static bool EnableShortTags;
  static bool EnableAspTags;
  static bool EnableXHP;
  static int ParserThreadCount;

  static int GetScannerType();

  /**
   * "Dynamic" means a function or a method can be invoked dynamically.
   * "Volatile" means a class or a function can be declared dynamically.
   */
  static bool AllDynamic;
  static bool AllVolatile;

  /**
   * Optimizations
   */
  static int InvokeFewArgsCount;
  static int InlineFunctionThreshold;
  static bool EliminateDeadCode;
  static bool CopyProp;
  static bool LocalCopyProp;
  static bool StringLoopOpts;
  static int AutoInline;
  static bool ArrayAccessIdempotent;

  /**
   * Output options
   */
  static bool GenerateDocComments;
  static bool ControlFlow;
  static bool VariableCoalescing;
  static bool DumpAst;
  static bool WholeProgram;
  static bool UseHHBBC;  // see hhbbc/README
  static bool RecordErrors;
  static std::string DocJson; // filename to dump doc JSON to

  static void setHookHandler(void (*hookHandler)(Hdf &config)) {
    m_hookHandler = hookHandler;
  }

  static bool (*PersistenceHook)(BlockScopeRawPtr scope, FileScopeRawPtr fs);
private:
  /**
   * Directory that has system HPHP files for loading builtin classes, etc.
   */
  static std::string SystemRoot;
  static StringBag OptionStrings;

  static void LoadRootHdf(const Hdf &roots, std::map<std::string,
                          std::string> &map);
  static void LoadRootHdf(const Hdf &roots, std::vector<std::string> &vec);
  static void OnLoad();

  static bool IsDynamic(const std::string &name,
                        const std::vector<std::string> &prefixes,
                        const std::vector<std::string> &postfixes);

  static void (*m_hookHandler)(Hdf &config);
};

///////////////////////////////////////////////////////////////////////////////
}
#endif // incl_HPHP_OPTION_H_
