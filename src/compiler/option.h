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

#ifndef __OPTION_H__
#define __OPTION_H__

#include <util/hdf.h>
#include <util/string_bag.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

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
   * Separable extensions that are loaded dynamically.
   */
  struct SepExtensionOptions {
    std::string name;
    std::string soname;
    std::string include_path;
    std::string lib_path;
    bool shared;
  };
  static std::vector<SepExtensionOptions> SepExtensions;

  /**
   * Optimization flags
   */
  static bool PreOptimization;
  static bool PostOptimization;
  static bool ScalarArrayOptimization;
  static bool ScalarArrayCompression;
  static int ScalarArrayFileCount;
  static int ScalarArrayOverflowLimit;
  static int LiteralStringFileCount;
  static bool AnalyzePerfectVirtuals;
  static bool HardTypeHints;

  /**
   * RTTI profiling metadata output file
   */
  static std::string RTTIOutputFile;

  /**
   * Directory of RTTI profiling data, used for second pass compilation
   */
  static std::string RTTIDirectory;

  static bool GenRTTIProfileData;
  static bool UseRTTIProfileData;

  /**
   * Generate concatN (n > 6) service routines
   */
  static bool GenConcat;

  /**
   * Generate array_createN service routines
   */
  static bool GenArrayCreate;

  /**
   * Generate hash-table lookup based invoke_file
   */
  static bool GenHashTableInvokeFile;

  /**
   * Generate hash-table lookup based function invoke
   */
  static bool GenHashTableInvokeFunc;

  /**
   * Generate hash-table lookup based dynamic class functions
   * get_class_var_init, create_object_only, etc.
   */
  static bool GenHashTableDynClass;

  /**
   * Generate hash-table lookup based get_constant
   */
  static bool GenHashTableGetConstant;

  /**
   * Separate compilation
   */
  static bool SeparateCompilation;
  static bool SeparateCompLib;
  static bool UseNamedScalarArray;

  /**
   * CodeGenerator options for PHP.
   */
  static bool GeneratePickledPHP;
  static bool GenerateInlinedPHP;
  static bool GenerateTrimmedPHP;
  static bool GenerateInlineComments; // comments on inlined file names
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
  static const char *FunctionPrefix;
  static const char *TypedFunctionPrefix;
  static const char *BuiltinFunctionPrefix;
  static const char *InvokePrefix;
  static const char *InvokeFewArgsPrefix;
  static const char *InvokeSinglePrefix;
  static const char *CreateObjectPrefix;
  static const char *CreateObjectOnlyPrefix;
  static const char *PseudoMainPrefix;
  static const char *VariablePrefix;
  static const char *HiddenVariablePrefix;
  static const char *GlobalVariablePrefix;
  static const char *StaticVariablePrefix;
  static const char *ScalarArrayName;
  static const char *SystemScalarArrayName;
  static const char *ClassPrefix;
  static const char *ClassStaticsPrefix;
  static const char *ClassStaticsObjectPrefix;
  static const char *ClassStaticsCallbackPrefix;
  static const char *ClassStaticsIdGetterPrefix;
  static const char *ClassStaticInitializerPrefix;
  static const char *ClassStaticInitializerFlagPrefix;
  static const char *ClassWrapperFunctionPrefix;
  static const char *ObjectPrefix;
  static const char *ObjectStaticPrefix;
  static const char *SmartPtrPrefix;
  static const char *MethodPrefix;
  static const char *TypedMethodPrefix;
  static const char *MethodWrapperPrefix;
  static const char *MethodImplPrefix;
  static const char *TypedMethodImplPrefix;
  static const char *PropertyPrefix;
  static const char *StaticPropertyPrefix;
  static const char *ConstantPrefix;
  static const char *ClassConstantPrefix;
  static const char *ExceptionPrefix;
  static const char *TempVariablePrefix;
  static const char *EvalOrderTempPrefix;
  static const char *SilencerPrefix;
  static const char *CallInfoPrefix;

  static const char *TempPrefix;
  static const char *MapPrefix;
  static const char *IterPrefix;
  static const char *InitPrefix;
  static const char *SwitchPrefix;

  static const char *FFIFnPrefix;

  static const char *SystemFilePrefix;
  static const char *UserFilePrefix;
  static const char *ClassHeaderPrefix;
  static const char *ClusterPrefix;
  static const char *FFIFilePrefix;

  /**
   * Turn it off for cleaner unit tests.
   */
  static bool GenerateCPPMacros;    // all macros
  static bool GenerateCPPMain;      // include and main()
  static bool GenerateCPPComments;  // section comments
  static bool GenerateCPPMetaInfo;  // class map
  static bool GenerateCPPNameSpace; // namespace HPHP
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

  /**
   * Returns a name for a clustered .cpp file.
   */
  static std::string FormatClusterFile(int index);

  static bool GenerateFFI;

  enum EvalLevel {
    NoEval = 0, // calling eval is a fatal
    LimitedEval = 1, // eval is supported in a limited way with no perf hit
    FullEval = 2 // eval is supported but with a performance hit
  };

  static EvalLevel EnableEval;

  /**
   * The root package for Java FFI stubs (dot-separated). The default is php.
   */
  static std::string JavaFFIRootPackage;

  static std::string ProgramName;
  static std::string PreprocessedPartitionConfig; // generated by ppp.php

  static bool ParseTimeOpts;
  static bool EnableHipHopSyntax;
  static bool EnableHipHopExperimentalSyntax;
  static bool EnableShortTags;
  static bool EnableAspTags;
  static bool EnableXHP;
  static bool NativeXHP;
  static int ScannerType;
  static int ParserThreadCount;

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
  static bool InvokeWithSpecificArgs;
  static bool FlattenInvoke;
  static int InlineFunctionThreshold;
  static bool ControlEvalOrder;
  static bool UseVirtualDispatch;
  static bool EliminateDeadCode;
  static bool CopyProp;
  static bool LocalCopyProp;
  static bool StringLoopOpts;
  static bool AutoInline;

  /**
   * Output options
   */
  static bool GenerateCppLibCode;
  static bool GenerateSourceInfo;
  static bool GenerateDocComments;
  static bool FlAnnotate; // annotate emitted code with compiler file-line info
  static bool SystemGen; // -t cpp -f sys
  static bool SplitDynamicClassTable;
  static bool ControlFlow;
  static bool VariableCoalescing;
  static bool DumpAst;
  static bool PregenerateCPP;
  static bool GenerateFFIStaticBinding;

  static int GCCOptimization[3];
  static int GetOptimizationLevel(int length);

  static void setHookHandler(void (*hookHandler)(Hdf &config)) {
    m_hookHandler = hookHandler;
  }

  static bool UseMethodIndex;
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
#endif // __OPTION_H__
