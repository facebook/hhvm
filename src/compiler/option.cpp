/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
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

#include <compiler/option.h>
#include <compiler/analysis/analysis_result.h>
#include <compiler/analysis/file_scope.h>
#include <compiler/analysis/class_scope.h>
#include <compiler/analysis/variable_table.h>
#include <util/parser/scanner.h>
#include <util/logger.h>
#include <util/db_query.h>
#include <util/util.h>
#include <util/process.h>
#include <boost/algorithm/string/trim.hpp>
#include <runtime/base/preg.h>

using namespace HPHP;
using namespace std;
using namespace boost;

///////////////////////////////////////////////////////////////////////////////

std::string Option::SystemRoot;
std::string Option::RootDirectory;
set<string> Option::PackageDirectories;
set<string> Option::PackageFiles;
set<string> Option::PackageExcludeDirs;
set<string> Option::PackageExcludeFiles;
set<string> Option::PackageExcludePatterns;
set<string> Option::PackageExcludeStaticDirs;
set<string> Option::PackageExcludeStaticFiles;
set<string> Option::PackageExcludeStaticPatterns;
bool Option::CachePHPFile = false;

vector<string> Option::ParseOnDemandDirs;

map<string, string> Option::IncludeRoots;
map<string, string> Option::AutoloadRoots;
vector<string> Option::IncludeSearchPaths;
string Option::DefaultIncludeRoot;
vector<Option::SepExtensionOptions> Option::SepExtensions;
map<string, int> Option::DynamicFunctionCalls;

bool Option::GeneratePickledPHP = false;
bool Option::GenerateInlinedPHP = false;
bool Option::GenerateTrimmedPHP = false;
bool Option::GenerateInlineComments = true;
bool Option::GenerateInferredTypes = false;
bool Option::ConvertSuperGlobals = false;
bool Option::ConvertQOpExpressions = false;
string Option::ProgramPrologue;
string Option::TrimmedPrologue;
vector<string> Option::DynamicFunctionPrefixes;
vector<string> Option::DynamicFunctionPostfixes;
vector<string> Option::DynamicMethodPrefixes;
vector<string> Option::DynamicMethodPostfixes;
vector<string> Option::DynamicClassPrefixes;
vector<string> Option::DynamicClassPostfixes;
set<string> Option::DynamicInvokeFunctions;
set<string> Option::VolatileClasses;

map<string, string> Option::FunctionSections;

#if defined(HPHP_OSS)
string Option::IdPrefix = "___";
string Option::LabelEscape = "___";
#else
string Option::IdPrefix = "$$";
string Option::LabelEscape = "$";
#endif

string Option::LambdaPrefix = "df_";
string Option::Tab = "  ";

/**
 * They all have to be something different. Otherwise, there is always a chance
 * of name collision or incorrect code transformation.
 */
const char *Option::FunctionPrefix = "f_";
const char *Option::TypedFunctionPrefix = "ft_";
const char *Option::BuiltinFunctionPrefix = "x_";
const char *Option::InvokePrefix = "i_";
const char *Option::InvokeFewArgsPrefix = "ifa_";
const char *Option::InvokeSinglePrefix = "is_";
const char *Option::CreateObjectPrefix = "co_";
const char *Option::CreateObjectOnlyPrefix = "coo_";
const char *Option::PseudoMainPrefix = "pm_";
const char *Option::VariablePrefix = "v_";
const char *Option::HiddenVariablePrefix = "h_";
const char *Option::GlobalVariablePrefix = "gv_";
const char *Option::StaticVariablePrefix = "sv_";
const char *Option::ScalarArrayName = "sa_";
const char *Option::SystemScalarArrayName = "ssa_";
const char *Option::ClassPrefix = "c_";
const char *Option::ClassStaticsPrefix = "cs_";
const char *Option::ClassStaticsObjectPrefix = "cso_";
const char *Option::ClassStaticsCallbackPrefix = "cwo_";
const char *Option::ClassStaticsIdGetterPrefix = "csig_";
const char *Option::ClassStaticInitializerPrefix = "csi_";
const char *Option::ClassStaticInitializerFlagPrefix = "csf_";
const char *Option::ClassWrapperFunctionPrefix = "cw_";
const char *Option::ObjectPrefix = "o_";
const char *Option::ObjectStaticPrefix = "os_";
const char *Option::SmartPtrPrefix = "p_";
const char *Option::MethodPrefix = "t_";
const char *Option::TypedMethodPrefix = "tt_";
const char *Option::MethodWrapperPrefix = "mf_";
const char *Option::MethodImplPrefix = "ti_";
const char *Option::TypedMethodImplPrefix = "tti_";
const char *Option::PropertyPrefix = "m_";
const char *Option::StaticPropertyPrefix = "s_";
const char *Option::ConstantPrefix = "k_";
const char *Option::ClassConstantPrefix = "q_";
const char *Option::ExceptionPrefix = "e_";
const char *Option::TempVariablePrefix = "r_";
const char *Option::EvalOrderTempPrefix = "eo_";
const char *Option::CallInfoPrefix = "ci_";
const char *Option::SilencerPrefix = "sil_";

const char *Option::FFIFnPrefix = "ffi_";

const char *Option::TempPrefix = "tmp";
const char *Option::MapPrefix = "map";
const char *Option::IterPrefix = "iter";
const char *Option::InitPrefix = "inited_";
const char *Option::SwitchPrefix = "switch";

const char *Option::SystemFilePrefix = "sys/";
const char *Option::UserFilePrefix = "php/";
const char *Option::ClassHeaderPrefix = "cls/";
const char *Option::ClusterPrefix = "cpp/";
const char *Option::FFIFilePrefix = "ffi/";

bool Option::PreOptimization = false;
bool Option::PostOptimization = false;
bool Option::ScalarArrayOptimization = true;
bool Option::ScalarArrayCompression = true;
int Option::ScalarArrayFileCount = 1;
int Option::ScalarArrayOverflowLimit = 2000;
bool Option::SeparateCompilation = false;
bool Option::SeparateCompLib = false;
bool Option::UseNamedScalarArray = true;
int Option::LiteralStringFileCount = 10;
bool Option::AnalyzePerfectVirtuals = true;
bool Option::HardTypeHints = true;

std::string Option::RTTIOutputFile;
std::string Option::RTTIDirectory;
bool Option::GenRTTIProfileData = false;
bool Option::UseRTTIProfileData = false;

bool Option::GenerateCPPMacros = true;
bool Option::GenerateCPPMain = false;
bool Option::GenerateCPPComments = true;
bool Option::GenerateCPPMetaInfo = true;
bool Option::GenerateCPPNameSpace = true;
bool Option::GenConcat = false;
bool Option::GenArrayCreate = true;
bool Option::GenHashTableInvokeFile = true;
bool Option::GenHashTableInvokeFunc = true;
bool Option::GenHashTableDynClass= true;
bool Option::GenHashTableGetConstant = true;
bool Option::KeepStatementsWithNoEffect = false;

int Option::ConditionalIncludeExpandLevel = 1;

int Option::DependencyMaxProgram = 1;
int Option::CodeErrorMaxProgram = 1;

bool Option::GenerateFFI = false;
Option::EvalLevel Option::EnableEval = NoEval;

std::string Option::JavaFFIRootPackage;

std::string Option::ProgramName;
std::string Option::PreprocessedPartitionConfig;

bool Option::ParseTimeOpts = true;
bool Option::EnableHipHopSyntax = false;
bool Option::EnableHipHopExperimentalSyntax = false;
bool Option::EnableShortTags = true;
bool Option::EnableAspTags = false;
bool Option::EnableXHP = true;
bool Option::NativeXHP = true;
int Option::ScannerType = Scanner::AllowShortTags;
int Option::ParserThreadCount = 0;

int Option::InvokeFewArgsCount = 6;
bool Option::InvokeWithSpecificArgs = true;
bool Option::FlattenInvoke = true;
int Option::InlineFunctionThreshold = -1;
bool Option::ControlEvalOrder = true;
bool Option::UseVirtualDispatch = false;
bool Option::EliminateDeadCode = true;
bool Option::CopyProp = false;
bool Option::LocalCopyProp = true;
bool Option::StringLoopOpts = true;
bool Option::AutoInline = false;
bool Option::ControlFlow = true;
bool Option::VariableCoalescing = false;
bool Option::DumpAst = false;

bool Option::AllDynamic = true;
bool Option::AllVolatile = false;

StringBag Option::OptionStrings;

bool Option::GenerateCppLibCode = false;
bool Option::GenerateSourceInfo = false;
bool Option::GenerateDocComments = true;
bool Option::FlAnnotate = false;
bool Option::SystemGen = false;
bool Option::SplitDynamicClassTable = true;
bool Option::PregenerateCPP = false;
bool Option::UseMethodIndex = false;
bool Option::GenerateFFIStaticBinding = true;

int Option::GCCOptimization[] = {0, 0, 0};

void (*Option::m_hookHandler)(Hdf &config);

///////////////////////////////////////////////////////////////////////////////
// load from a PHP file

std::string Option::GetSystemRoot() {
  if (SystemRoot.empty()) {
    const char *home = getenv("HPHP_HOME");
    if (!home || !*home) {
      throw Exception("Environment variable HPHP_HOME is not set.");
    }
    SystemRoot = home;
    SystemRoot += "/src";
  }
  return SystemRoot;
}

///////////////////////////////////////////////////////////////////////////////
// load from HDF file

void Option::LoadRootHdf(const Hdf &roots, map<string, string> &map) {
  if (roots.exists()) {
    for (Hdf hdf = roots.firstChild(); hdf.exists(); hdf = hdf.next()) {
      map[hdf["root"].get()] = hdf["path"].get();
    }
  }
}

void Option::LoadRootHdf(const Hdf &roots, vector<string> &vec) {
  if (roots.exists()) {
    for (Hdf hdf = roots.firstChild(); hdf.exists(); hdf = hdf.next()) {
      vec.push_back(hdf.getString(""));
    }
  }
}

void Option::Load(Hdf &config) {
  LoadRootHdf(config["IncludeRoots"], IncludeRoots);
  LoadRootHdf(config["AutoloadRoots"], AutoloadRoots);

  config["PackageFiles"].get(PackageFiles);
  config["IncludeSearchPaths"].get(IncludeSearchPaths);
  config["PackageDirectories"].get(PackageDirectories);
  config["PackageExcludeDirs"].get(PackageExcludeDirs);
  config["PackageExcludeFiles"].get(PackageExcludeFiles);
  config["PackageExcludePatterns"].get(PackageExcludePatterns);
  config["PackageExcludeStaticDirs"].get(PackageExcludeStaticDirs);
  config["PackageExcludeStaticFiles"].get(PackageExcludeStaticFiles);
  config["PackageExcludeStaticPatterns"].get(PackageExcludeStaticPatterns);
  CachePHPFile = config["CachePHPFile"].getBool();

  config["ParseOnDemandDirs"].get(ParseOnDemandDirs);

  {
    Hdf cg = config["CodeGeneration"];
    string tmp;

#define READ_CG_OPTION(name)                    \
    tmp = cg[#name].getString();                \
    if (!tmp.empty()) {                         \
      name = OptionStrings.add(tmp.c_str());    \
    }

    READ_CG_OPTION(IdPrefix);
    READ_CG_OPTION(LabelEscape);
    READ_CG_OPTION(LambdaPrefix);
    READ_CG_OPTION(FunctionPrefix);
    READ_CG_OPTION(BuiltinFunctionPrefix);
    READ_CG_OPTION(InvokePrefix);
    READ_CG_OPTION(CreateObjectPrefix);
    READ_CG_OPTION(PseudoMainPrefix);
    READ_CG_OPTION(VariablePrefix);
    READ_CG_OPTION(HiddenVariablePrefix);
    READ_CG_OPTION(GlobalVariablePrefix);
    READ_CG_OPTION(StaticVariablePrefix);
    READ_CG_OPTION(ScalarArrayName);
    READ_CG_OPTION(SystemScalarArrayName);
    READ_CG_OPTION(ClassPrefix);
    READ_CG_OPTION(ClassStaticsPrefix);
    READ_CG_OPTION(ClassStaticsObjectPrefix);
    READ_CG_OPTION(ClassStaticsCallbackPrefix);
    READ_CG_OPTION(ClassStaticsIdGetterPrefix);
    READ_CG_OPTION(ClassStaticInitializerPrefix);
    READ_CG_OPTION(ClassStaticInitializerFlagPrefix);
    READ_CG_OPTION(ClassWrapperFunctionPrefix);
    READ_CG_OPTION(ObjectPrefix);
    READ_CG_OPTION(ObjectStaticPrefix);
    READ_CG_OPTION(SmartPtrPrefix);
    READ_CG_OPTION(MethodPrefix);
    READ_CG_OPTION(MethodWrapperPrefix);
    READ_CG_OPTION(MethodImplPrefix);
    READ_CG_OPTION(PropertyPrefix);
    READ_CG_OPTION(StaticPropertyPrefix);
    READ_CG_OPTION(ConstantPrefix);
    READ_CG_OPTION(ClassConstantPrefix);
    READ_CG_OPTION(ExceptionPrefix);
    READ_CG_OPTION(TempVariablePrefix);
    READ_CG_OPTION(EvalOrderTempPrefix);
    READ_CG_OPTION(SilencerPrefix);
    READ_CG_OPTION(TempPrefix);
    READ_CG_OPTION(MapPrefix);
    READ_CG_OPTION(IterPrefix);
    READ_CG_OPTION(InitPrefix);
    READ_CG_OPTION(SwitchPrefix);
    READ_CG_OPTION(FFIFnPrefix);
    READ_CG_OPTION(SystemFilePrefix);
    READ_CG_OPTION(UserFilePrefix);
    READ_CG_OPTION(ClassHeaderPrefix);
    READ_CG_OPTION(ClusterPrefix);
    READ_CG_OPTION(FFIFilePrefix);
  }

  int count = 0;
  for (Hdf hdf = config["SepExtensions"].firstChild(); hdf.exists();
       hdf = hdf.next()) {
    ++count;
  }
  SepExtensions.resize(count);
  count = 0;
  for (Hdf hdf = config["SepExtensions"].firstChild(); hdf.exists();
       hdf = hdf.next()) {
    SepExtensionOptions &options = SepExtensions[count++];
    options.name = hdf.getName();
    options.soname = hdf["soname"].getString();
    options.include_path = hdf["include"].getString();
    options.lib_path = hdf["libpath"].getString();
    options.shared = hdf["shared"].getBool();
  }

  config["DynamicFunctionPrefix"].get(DynamicFunctionPrefixes);
  config["DynamicFunctionPostfix"].get(DynamicFunctionPostfixes);
  config["DynamicMethodPrefix"].get(DynamicMethodPrefixes);
  config["DynamicInvokeFunctions"].get(DynamicInvokeFunctions);
  config["VolatileClasses"].get(VolatileClasses);

  // build map from function names to sections
  for (Hdf hdf = config["FunctionSections"].firstChild(); hdf.exists();
       hdf = hdf.next()) {
    for (Hdf hdfFunc = hdf.firstChild(); hdfFunc.exists();
         hdfFunc = hdfFunc.next()) {
           FunctionSections[hdfFunc.getString()] = hdf.getName();
    }
  }

  ScalarArrayFileCount = config["ScalarArrayFileCount"].getByte(1);
  if (ScalarArrayFileCount <= 0) ScalarArrayFileCount = 1;
  LiteralStringFileCount = config["LiteralStringFileCount"].getInt32(10);
  if (LiteralStringFileCount <= 0) LiteralStringFileCount = 10;
  HardTypeHints = config["HardTypeHints"].getBool(true);
  ScalarArrayOverflowLimit = config["ScalarArrayOverflowLimit"].getInt32(2000);
  if (ScalarArrayOverflowLimit <= 0) ScalarArrayOverflowLimit = 2000;
  if (UseNamedScalarArray) {
    ScalarArrayOptimization = true;
    ScalarArrayCompression = true;
  }

  EnableHipHopSyntax = config["EnableHipHopSyntax"].getBool();
  EnableHipHopExperimentalSyntax =
    config["EnableHipHopExperimentalSyntax"].getBool();
  EnableShortTags = config["EnableShortTags"].getBool(true);
  if (EnableShortTags) ScannerType |= Scanner::AllowShortTags;
  else ScannerType &= ~Scanner::AllowShortTags;

  EnableAspTags = config["EnableAspTags"].getBool();
  if (EnableAspTags) ScannerType |= Scanner::AllowAspTags;
  else ScannerType &= ~Scanner::AllowAspTags;

  EnableXHP = config["EnableXHP"].getBool(true);
  NativeXHP = config["NativeXHP"].getBool(true);
  if (EnableXHP && !NativeXHP) ScannerType |= Scanner::PreprocessXHP;
  else ScannerType &= ~Scanner::PreprocessXHP;

  ParserThreadCount = config["ParserThreadCount"].getInt32(0);
  if (ParserThreadCount <= 0) {
    ParserThreadCount = Process::GetCPUCount();
  }

  RTTIOutputFile = config["RTTIOutputFile"].getString();
  EnableEval = (EvalLevel)config["EnableEval"].getByte(0);
  AllDynamic = config["AllDynamic"].getBool(true);
  AllVolatile = config["AllVolatile"].getBool();

  GenerateCppLibCode = config["GenerateCppLibCode"].getBool(false);
  GenerateSourceInfo = config["GenerateSourceInfo"].getBool(false);
  GenerateDocComments = config["GenerateDocComments"].getBool(true);
  UseVirtualDispatch = config["UseVirtualDispatch"].getBool(false);
  EliminateDeadCode  = config["EliminateDeadCode"].getBool(true);
  CopyProp           = config["CopyProp"].getBool(false);
  LocalCopyProp      = config["LocalCopyProp"].getBool(true);
  StringLoopOpts     = config["StringLoopOpts"].getBool(true);
  AutoInline         = config["AutoInline"].getBool(false);
  ControlFlow        = config["ControlFlow"].getBool(true);
  VariableCoalescing = config["VariableCoalescing"].getBool(false);
  DumpAst            = config["DumpAst"].getBool(false);
  PregenerateCPP     = config["PregenerateCPP"].getBool(false);
  GenerateFFIStaticBinding = config["GenerateFFIStaticBinding"].getBool(true);

  {
    Hdf gccOptimization = config["GCCOptimization"];
    GCCOptimization[0] = gccOptimization["O0"].getInt32(0);
    GCCOptimization[1] = gccOptimization["O1"].getInt32(GCCOptimization[0]);
    GCCOptimization[2] = gccOptimization["O2"].getInt32(GCCOptimization[1]);
  }

  if (m_hookHandler) m_hookHandler(config);

  OnLoad();
}

int Option::GetOptimizationLevel(int length) {
  if (length <= 0) return 3;
  for (int i = 2; i >= 0; --i) {
    int min = GCCOptimization[i];
    if (length < min || min == 0) return i + 1;
  }
  return 0;
}

void Option::Load() {
  OnLoad();
}

void Option::OnLoad() {
  // all lambda functions are dynamic automatically
  DynamicFunctionPrefixes.push_back(LambdaPrefix);
}

///////////////////////////////////////////////////////////////////////////////

bool Option::IsDynamicFunction(bool method, const std::string &name) {
  if (method) {
    return IsDynamic(name, DynamicMethodPrefixes, DynamicMethodPostfixes);
  }
  return IsDynamic(name, DynamicFunctionPrefixes, DynamicFunctionPostfixes);
}

bool Option::IsDynamicClass(const std::string &name) {
  return IsDynamic(name, DynamicClassPrefixes, DynamicClassPostfixes);
}

bool Option::IsDynamic(const std::string &name,
                       const std::vector<std::string> &prefixes,
                       const std::vector<std::string> &postfixes) {
  if (name.substr(0, 4) == "dyn_") return true;

  for (unsigned int i = 0; i < prefixes.size(); i++) {
    const string &prefix = prefixes[i];
    if (name.substr(0, prefix.length()) == prefix) {
      return true;
    }
  }

  for (unsigned int i = 0; i < postfixes.size(); i++) {
    const string &postfix = postfixes[i];
    if (name.length() > postfix.length() &&
        name.substr(name.length() - postfix.length()) == postfix) {
      return true;
    }
  }

  return false;
}

std::string Option::GetAutoloadRoot(const std::string &name) {
  for (map<string, string>::const_iterator iter = AutoloadRoots.begin();
       iter != AutoloadRoots.end(); ++iter) {
    if (name.substr(0, iter->first.length()) == iter->first) {
      return iter->second;
    }
  }
  return "";
}

std::string Option::MangleFilename(const std::string &name, bool id) {
  string ret = UserFilePrefix;
  ret += name;

  if (id) {
    Util::replaceAll(ret, "/", "$");
    Util::replaceAll(ret, "-", "_");
    Util::replaceAll(ret, ".", "_");
  }
  return ret;
}

std::string Option::FormatClusterFile(int index) {
  char buf[PATH_MAX];
  snprintf(buf, sizeof(buf), "%s%03d", ClusterPrefix, index);
  return buf;
}

bool Option::IsFileExcluded(const std::string &file,
                            const std::set<std::string> &patterns) {
  String sfile(file.c_str(), file.size(), AttachLiteral);
  for (set<string>::const_iterator iter = patterns.begin();
       iter != patterns.end(); ++iter) {
    const std::string &pattern = *iter;
    Variant matches;
    Variant ret = preg_match(String(pattern.c_str(), pattern.size(),
                                    AttachLiteral), sfile, matches);
    if (ret.toInt64() > 0) {
      return true;
    }
  }
  return false;
}

void Option::FilterFiles(std::vector<std::string> &files,
                         const std::set<std::string> &patterns) {
  for (int i = files.size() - 1; i >= 0; i--) {
    if (IsFileExcluded(files[i], patterns)) {
      files.erase(files.begin() + i);
    }
  }
}

