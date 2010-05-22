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

#include <compiler/option.h>
#include <util/logger.h>
#include <compiler/analysis/analysis_result.h>
#include <compiler/analysis/file_scope.h>
#include <compiler/analysis/class_scope.h>
#include <compiler/analysis/variable_table.h>
#include <util/db_query.h>
#include <util/util.h>
#include <boost/algorithm/string/trim.hpp>

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
set<string> Option::PackageExcludeStaticFiles;
bool Option::CachePHPFile = false;

set<string> Option::AllowedBadPHPIncludes;
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

#if defined(HPHP_OSS)
string Option::IdPrefix = "___";
#else
string Option::IdPrefix = "$$";
#endif

string Option::LambdaPrefix = "df_";
string Option::Tab = "  ";

/**
 * They all have to be something different. Otherwise, there is always a chance
 * of name collision or incorrect code transformation.
 */
const char *Option::FunctionPrefix = "f_";
const char *Option::BuiltinFunctionPrefix = "x_";
const char *Option::InvokePrefix = "i_";
const char *Option::CreateObjectPrefix = "co_";
const char *Option::PseudoMainPrefix = "pm_";
const char *Option::VariablePrefix = "v_";
const char *Option::GlobalVariablePrefix = "gv_";
const char *Option::StaticVariablePrefix = "sv_";
const char *Option::ScalarArrayName = "sa_";
const char *Option::SystemScalarArrayName = "ssa_";
const char *Option::ClassPrefix = "c_";
const char *Option::ClassStaticsPrefix = "cs_";
const char *Option::ClassStaticsObjectPrefix = "cso_";
const char *Option::ClassStaticsIdGetterPrefix = "csig_";
const char *Option::ClassStaticInitializerPrefix = "csi_";
const char *Option::ClassStaticInitializerFlagPrefix = "csf_";
const char *Option::ClassWrapperFunctionPrefix = "cw_";
const char *Option::ObjectPrefix = "o_";
const char *Option::ObjectStaticPrefix = "os_";
const char *Option::SmartPtrPrefix = "p_";
const char *Option::MethodPrefix = "t_";
const char *Option::MethodImplPrefix = "ti_";
const char *Option::PropertyPrefix = "m_";
const char *Option::StaticPropertyPrefix = "s_";
const char *Option::ConstantPrefix = "k_";
const char *Option::ClassConstantPrefix = "q_";
const char *Option::ExceptionPrefix = "e_";
const char *Option::TempVariablePrefix = "r_";
const char *Option::EvalOrderTempPrefix = "eo_";
const char *Option::EvalInvokePrefix = "ei_";
const char *Option::SilencerPrefix = "sil_";

const char *Option::FFIFnPrefix = "ffi_";

const char *Option::TempPrefix = "tmp";
const char *Option::MapPrefix = "map";
const char *Option::IterPrefix = "iter";
const char *Option::InitPrefix = "inited_";

const char *Option::SystemFilePrefix = "sys/";
const char *Option::UserFilePrefix = "php/";
const char *Option::ClassHeaderPrefix = "cls/";
const char *Option::ClusterPrefix = "cpp/";
const char *Option::FFIFilePrefix = "ffi/";

bool Option::PreOptimization = false;
bool Option::PostOptimization = false;
bool Option::ScalarArrayOptimization = true;
bool Option::ScalarArrayCompression = true;
bool Option::LiteralStringCompression = false;
int Option::ScalarArrayFileCount = 1;
int Option::ScalarArrayOverflowLimit = 2000;
bool Option::SeparateCompilation = false;
bool Option::SeparateCompLib = false;
int Option::LiteralStringFileCount = 50;

std::string Option::RTTIOutputFile;
std::string Option::RTTIDirectory;
bool Option::GenRTTIProfileData = false;
bool Option::UseRTTIProfileData = false;

bool Option::StaticMethodAutoFix = false;

bool Option::GenerateCPPMacros = true;
bool Option::GenerateCPPMain = false;
bool Option::GenerateCPPComments = true;
bool Option::GenerateCPPMetaInfo = true;
bool Option::GenerateCPPNameSpace = true;
bool Option::KeepStatementsWithNoEffect = false;

int Option::ConditionalIncludeExpandLevel = 1;

int Option::DependencyMaxProgram = 1;
int Option::CodeErrorMaxProgram = 1;

bool Option::GenerateFFI = false;
Option::EvalLevel Option::EnableEval = NoEval;

std::string Option::JavaFFIRootPackage;

std::string Option::ProgramName;

bool Option::EnableXHP = false;

int Option::InvokeFewArgsCount = 6;
bool Option::PrecomputeLiteralStrings = false;
bool Option::FlattenInvoke = true;
int Option::InlineFunctionThreshold = -1;
bool Option::ControlEvalOrder = true;

bool Option::AllDynamic = false;
bool Option::AllVolatile = false;

std::string Option::FlibDirectory;
StringBag Option::OptionStrings;

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

  config["IncludeSearchPaths"].get(IncludeSearchPaths);
  config["PackageDirectories"].get(PackageDirectories);
  config["PackageExcludeDirs"].get(PackageExcludeDirs);
  config["PackageExcludeFiles"].get(PackageExcludeFiles);

  config["PackageExcludeStaticFiles"].get(PackageExcludeStaticFiles);
  CachePHPFile = config["CachePHPFile"].getBool();

  {
    Hdf cg = config["CodeGeneration"];
    string tmp;

#define READ_CG_OPTION(name)                    \
    tmp = cg[#name].getString();                \
    if (!tmp.empty()) {                         \
      name = OptionStrings.add(tmp.c_str());    \
    }

    READ_CG_OPTION(IdPrefix);
    READ_CG_OPTION(LambdaPrefix);
    READ_CG_OPTION(FunctionPrefix);
    READ_CG_OPTION(BuiltinFunctionPrefix);
    READ_CG_OPTION(InvokePrefix);
    READ_CG_OPTION(CreateObjectPrefix);
    READ_CG_OPTION(PseudoMainPrefix);
    READ_CG_OPTION(VariablePrefix);
    READ_CG_OPTION(GlobalVariablePrefix);
    READ_CG_OPTION(StaticVariablePrefix);
    READ_CG_OPTION(ScalarArrayName);
    READ_CG_OPTION(SystemScalarArrayName);
    READ_CG_OPTION(ClassPrefix);
    READ_CG_OPTION(ClassStaticsPrefix);
    READ_CG_OPTION(ClassStaticsObjectPrefix);
    READ_CG_OPTION(ClassStaticsIdGetterPrefix);
    READ_CG_OPTION(ClassStaticInitializerPrefix);
    READ_CG_OPTION(ClassStaticInitializerFlagPrefix);
    READ_CG_OPTION(ClassWrapperFunctionPrefix);
    READ_CG_OPTION(ObjectPrefix);
    READ_CG_OPTION(ObjectStaticPrefix);
    READ_CG_OPTION(SmartPtrPrefix);
    READ_CG_OPTION(MethodPrefix);
    READ_CG_OPTION(MethodImplPrefix);
    READ_CG_OPTION(PropertyPrefix);
    READ_CG_OPTION(StaticPropertyPrefix);
    READ_CG_OPTION(ConstantPrefix);
    READ_CG_OPTION(ClassConstantPrefix);
    READ_CG_OPTION(ExceptionPrefix);
    READ_CG_OPTION(TempVariablePrefix);
    READ_CG_OPTION(EvalOrderTempPrefix);
    READ_CG_OPTION(SilencerPrefix);
    READ_CG_OPTION(EvalInvokePrefix);
    READ_CG_OPTION(TempPrefix);
    READ_CG_OPTION(MapPrefix);
    READ_CG_OPTION(IterPrefix);
    READ_CG_OPTION(InitPrefix);
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

  ScalarArrayFileCount = config["ScalarArrayFileCount"].getByte(1);
  if (ScalarArrayFileCount <= 0) ScalarArrayFileCount = 1;
  LiteralStringFileCount = config["LiteralStringFileCount"].getInt32(1);
  if (LiteralStringFileCount <= 0) LiteralStringFileCount = 1;
  ScalarArrayOverflowLimit = config["ScalarArrayOverflowLimit"].getInt32(2000);
  if (ScalarArrayOverflowLimit <= 0) ScalarArrayOverflowLimit = 2000;
  FlibDirectory = config["FlibDirectory"].getString();
  EnableXHP = config["EnableXHP"].getBool();
  RTTIOutputFile = config["RTTIOutputFile"].getString();
  EnableEval = (EvalLevel)config["EnableEval"].getByte(0);
  AllDynamic = config["AllDynamic"].getBool();
  AllVolatile = config["AllVolatile"].getBool();

  OnLoad();
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
