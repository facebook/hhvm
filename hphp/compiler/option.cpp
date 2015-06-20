/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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

#include "hphp/compiler/option.h"
#include "hphp/compiler/analysis/analysis_result.h"
#include "hphp/compiler/analysis/file_scope.h"
#include "hphp/compiler/analysis/class_scope.h"
#include "hphp/compiler/analysis/variable_table.h"
#include "hphp/runtime/base/ini-setting.h"
#include "hphp/parser/scanner.h"
#include "hphp/util/logger.h"
#include "hphp/util/text-util.h"
#include "hphp/util/process.h"
#include "hphp/hhbbc/hhbbc.h"
#include <boost/algorithm/string/trim.hpp>
#include <map>
#include <set>
#include <vector>
#include "hphp/runtime/base/preg.h"
#include "hphp/runtime/base/config.h"

namespace HPHP {

using std::set;
using std::map;
using std::map;

///////////////////////////////////////////////////////////////////////////////

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
map<string, int> Option::DynamicFunctionCalls;

bool Option::GeneratePickledPHP = false;
bool Option::GenerateInlinedPHP = false;
bool Option::GenerateTrimmedPHP = false;
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
set<string, stdltistr> Option::DynamicInvokeFunctions;
set<string> Option::VolatileClasses;
map<string,string> Option::AutoloadClassMap;
map<string,string> Option::AutoloadFuncMap;
map<string,string> Option::AutoloadConstMap;
string Option::AutoloadRoot;

map<string, string> Option::FunctionSections;

bool Option::GenerateTextHHBC = false;
bool Option::GenerateBinaryHHBC = false;
string Option::RepoCentralPath;
bool Option::RepoDebugInfo = false;

string Option::IdPrefix = "$$";

string Option::LambdaPrefix = "df_";
string Option::Tab = "  ";

const char *Option::UserFilePrefix = "php/";

bool Option::PreOptimization = false;
bool Option::PostOptimization = false;
bool Option::SeparateCompilation = false;
bool Option::SeparateCompLib = false;
bool Option::AnalyzePerfectVirtuals = true;
bool Option::HardTypeHints = true;
bool Option::HardReturnTypeHints = false;
bool Option::HardConstProp = true;

bool Option::KeepStatementsWithNoEffect = false;

int Option::ConditionalIncludeExpandLevel = 1;

int Option::DependencyMaxProgram = 1;
int Option::CodeErrorMaxProgram = 1;

Option::EvalLevel Option::EnableEval = NoEval;

std::string Option::ProgramName;

bool Option::ParseTimeOpts = true;
bool Option::EnableHipHopSyntax = false;
bool Option::EnableZendCompat = false;
bool Option::JitEnableRenameFunction = false;
bool Option::EnableHipHopExperimentalSyntax = false;
bool Option::EnableShortTags = true;
bool Option::EnableAspTags = false;
bool Option::EnableXHP = false;
bool Option::IntsOverflowToInts = false;
HackStrictOption
  Option::StrictArrayFillKeys = HackStrictOption::OFF,
  Option::DisallowDynamicVarEnvFuncs = HackStrictOption::OFF;
int Option::ParserThreadCount = 0;

int Option::GetScannerType() {
  int type = 0;
  if (EnableShortTags) type |= Scanner::AllowShortTags;
  if (EnableAspTags) type |= Scanner::AllowAspTags;
  if (EnableXHP) type |= Scanner::AllowXHPSyntax;
  if (EnableHipHopSyntax) type |= Scanner::AllowHipHopSyntax;
  return type;
}

int Option::InvokeFewArgsCount = 6;
int Option::InlineFunctionThreshold = -1;
bool Option::EliminateDeadCode = false;
bool Option::LocalCopyProp = false;
int Option::AutoInline = 0;
bool Option::VariableCoalescing = false;
bool Option::ArrayAccessIdempotent = false;
bool Option::DumpAst = false;
bool Option::WholeProgram = true;
bool Option::UseHHBBC = !getenv("HHVM_DISABLE_HHBBC2");
bool Option::RecordErrors = true;

bool Option::AllDynamic = true;
bool Option::AllVolatile = false;

StringBag Option::OptionStrings;

bool Option::GenerateDocComments = true;

///////////////////////////////////////////////////////////////////////////////
// load from HDF file

void Option::LoadRootHdf(const IniSetting::Map& ini, const Hdf &roots,
                         const std::string& name, map<string, string> &map) {
  if (roots.exists()) {
    for (Hdf hdf = roots[name].firstChild(); hdf.exists(); hdf = hdf.next()) {
      map[Config::Get(ini, hdf, "root", "", false)] =
        Config::Get(ini, hdf, "path", "", false);
    }
  }
}

void Option::LoadRootHdf(const IniSetting::Map& ini, const Hdf &roots,
                         const std::string& name, vector<string> &vec) {
  if (roots.exists()) {
    for (Hdf hdf = roots[name].firstChild(); hdf.exists(); hdf = hdf.next()) {
      vec.push_back(Config::GetString(ini, hdf, "", "", false));
    }
  }
}

void Option::Load(const IniSetting::Map& ini, Hdf &config) {
  LoadRootHdf(ini, config, "IncludeRoots", IncludeRoots);
  LoadRootHdf(ini, config, "AutoloadRoots", AutoloadRoots);

  Config::Bind(PackageFiles, ini, config, "PackageFiles", PackageFiles);
  Config::Bind(IncludeSearchPaths, ini, config, "IncludeSearchPaths");
  Config::Bind(PackageDirectories, ini, config, "PackageDirectories");
  Config::Bind(PackageExcludeDirs, ini, config, "PackageExcludeDirs");
  Config::Bind(PackageExcludeFiles, ini, config, "PackageExcludeFiles");
  Config::Bind(PackageExcludePatterns, ini, config, "PackageExcludePatterns");
  Config::Bind(PackageExcludeStaticDirs, ini,
               config, "PackageExcludeStaticDirs");
  Config::Bind(PackageExcludeStaticFiles, ini,
               config, "PackageExcludeStaticFiles");
  Config::Bind(PackageExcludeStaticFiles, ini,
               config, "PackageExcludeStaticPatterns");
  Config::Bind(CachePHPFile, ini, config, "CachePHPFile");

  Config::Bind(ParseOnDemandDirs, ini, config, "ParseOnDemandDirs");

  {
    string tmp;

#define READ_CG_OPTION(name)                    \
    tmp = Config::GetString(ini, config, "CodeGeneration."#name); \
    if (!tmp.empty()) {                         \
      name = OptionStrings.add(tmp.c_str());    \
    }

    READ_CG_OPTION(IdPrefix);
    READ_CG_OPTION(LambdaPrefix);
  }

  Config::Bind(DynamicFunctionPrefixes, ini, config, "DynamicFunctionPrefix");
  Config::Bind(DynamicFunctionPostfixes, ini, config, "DynamicFunctionPostfix");
  Config::Bind(DynamicMethodPrefixes, ini, config, "DynamicMethodPrefix");
  Config::Bind(DynamicInvokeFunctions, ini, config, "DynamicInvokeFunctions");
  Config::Bind(VolatileClasses, ini, config, "VolatileClasses");

  // build map from function names to sections
  for (Hdf hdf = config["FunctionSections"].firstChild(); hdf.exists();
       hdf = hdf.next()) {
    for (Hdf hdfFunc = hdf.firstChild(); hdfFunc.exists();
         hdfFunc = hdfFunc.next()) {
           FunctionSections[Config::GetString(ini, hdfFunc, "", "", false)]
             = hdf.getName();
    }
  }

  {
    // Repo
    {
      // Repo Central
      Config::Bind(RepoCentralPath, ini, config, "Repo.Central.Path");
    }
    Config::Bind(RepoDebugInfo, ini, config, "Repo.DebugInfo", false);
  }

  {
    // AutoloadMap
    Config::Bind(AutoloadClassMap, ini, config, "AutoloadMap.class");
    Config::Bind(AutoloadFuncMap, ini, config, "AutoloadMap.function");
    Config::Bind(AutoloadConstMap, ini, config, "AutoloadMap.constant");
    Config::Bind(AutoloadRoot, ini, config, "AutoloadMap.root");
  }

  Config::Bind(HardTypeHints, ini, config, "HardTypeHints", true);
  Config::Bind(HardReturnTypeHints, ini, config, "HardReturnTypeHints", false);
  Config::Bind(HardConstProp, ini, config, "HardConstProp", true);

  Config::Bind(EnableHipHopSyntax, ini, config, "EnableHipHopSyntax");
  Config::Bind(EnableZendCompat, ini, config, "EnableZendCompat");
  Config::Bind(JitEnableRenameFunction, ini, config, "JitEnableRenameFunction");
  Config::Bind(EnableHipHopExperimentalSyntax, ini,
               config, "EnableHipHopExperimentalSyntax");
  Config::Bind(EnableShortTags, ini, config, "EnableShortTags", true);

  {
    // Hack
    Config::Bind(IntsOverflowToInts, ini, config,
                 "Hack.Lang.IntsOverflowToInts", EnableHipHopSyntax);
    Config::Bind(StrictArrayFillKeys, ini, config,
                 "Hack.Lang.StrictArrayFillKeys");
    Config::Bind(DisallowDynamicVarEnvFuncs, ini, config,
                 "Hack.Lang.DisallowDynamicVarEnvFuncs");
  }

  Config::Bind(EnableAspTags, ini, config, "EnableAspTags");

  Config::Bind(EnableXHP, ini, config, "EnableXHP", false);

  if (EnableHipHopSyntax) {
    // If EnableHipHopSyntax is true, it forces EnableXHP to true
    // regardless of how it was set in the config
    EnableXHP = true;
  }

  Config::Bind(ParserThreadCount, ini, config, "ParserThreadCount", 0);
  if (ParserThreadCount <= 0) {
    ParserThreadCount = Process::GetCPUCount();
  }

  EnableEval = (EvalLevel) Config::GetByte(ini, config, "EnableEval", 0);
  Config::Bind(AllDynamic, ini, config, "AllDynamic", true);
  Config::Bind(AllVolatile, ini, config, "AllVolatile");

  Config::Bind(GenerateDocComments, ini, config, "GenerateDocComments", true);
  Config::Bind(EliminateDeadCode, ini, config, "EliminateDeadCode", false);
  Config::Bind(LocalCopyProp, ini, config, "LocalCopyProp", false);
  Config::Bind(AutoInline, ini, config, "AutoInline", 0);
  Config::Bind(VariableCoalescing, ini, config, "VariableCoalescing", false);
  Config::Bind(ArrayAccessIdempotent, ini, config, "ArrayAccessIdempotent",
               false);
  Config::Bind(DumpAst, ini, config, "DumpAst", false);
  Config::Bind(WholeProgram, ini, config, "WholeProgram", true);
  Config::Bind(UseHHBBC, ini, config, "UseHHBBC", UseHHBBC);

  // Temporary, during file-cache migration.
  Config::Bind(FileCache::UseNewCache, ini, config, "UseNewCache", false);

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
    replaceAll(ret, "/", "$");
    replaceAll(ret, "-", "_");
    replaceAll(ret, ".", "_");
  }
  return ret;
}

bool Option::IsFileExcluded(const std::string &file,
                            const std::set<std::string> &patterns) {
  String sfile(file.c_str(), file.size(), CopyString);
  for (set<string>::const_iterator iter = patterns.begin();
       iter != patterns.end(); ++iter) {
    const std::string &pattern = *iter;
    Variant matches;
    Variant ret = preg_match(String(pattern.c_str(), pattern.size(),
                                    CopyString), sfile, &matches);
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

//////////////////////////////////////////////////////////////////////

void initialize_hhbbc_options() {
  if (!Option::UseHHBBC) return;
  HHBBC::options.AllFuncsInterceptable  = Option::JitEnableRenameFunction;
  HHBBC::options.InterceptableFunctions = HHBBC::make_method_map(
                                            Option::DynamicInvokeFunctions);
  HHBBC::options.HardConstProp          = Option::HardConstProp;
  HHBBC::options.HardTypeHints          = Option::HardTypeHints;
  HHBBC::options.HardReturnTypeHints    = Option::HardReturnTypeHints;
  HHBBC::options.DisallowDynamicVarEnvFuncs =
    (Option::DisallowDynamicVarEnvFuncs == HackStrictOption::ON);
}

//////////////////////////////////////////////////////////////////////

}
