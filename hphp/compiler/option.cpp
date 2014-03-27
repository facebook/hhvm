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
#include "hphp/parser/scanner.h"
#include "hphp/util/logger.h"
#include "hphp/util/db-query.h"
#include "hphp/util/text-util.h"
#include "hphp/util/process.h"
#include "hphp/hhbbc/hhbbc.h"
#include <boost/algorithm/string/trim.hpp>
#include <map>
#include <set>
#include <vector>
#include "hphp/runtime/base/preg.h"

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
bool Option::EliminateDeadCode = true;
bool Option::CopyProp = false;
bool Option::LocalCopyProp = true;
bool Option::StringLoopOpts = true;
int Option::AutoInline = 0;
bool Option::ControlFlow = true;
bool Option::VariableCoalescing = false;
bool Option::ArrayAccessIdempotent = false;
bool Option::DumpAst = false;
bool Option::WholeProgram = true;
bool Option::UseHHBBC = !getenv("HHVM_DISABLE_HHBBC");
bool Option::RecordErrors = true;
std::string Option::DocJson;

bool Option::AllDynamic = true;
bool Option::AllVolatile = false;

StringBag Option::OptionStrings;

bool Option::GenerateDocComments = true;

bool (*Option::PersistenceHook)(BlockScopeRawPtr scope, FileScopeRawPtr file);

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
    READ_CG_OPTION(LambdaPrefix);
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

  {
    Hdf repo = config["Repo"];
    {
      Hdf repoCentral = repo["Central"];
      RepoCentralPath = repoCentral["Path"].getString();
    }
    RepoDebugInfo = repo["DebugInfo"].getBool(false);
  }

  {
    Hdf autoloadMap = config["AutoloadMap"];
    autoloadMap["class"].get(AutoloadClassMap);
    autoloadMap["function"].get(AutoloadFuncMap);
    autoloadMap["constant"].get(AutoloadConstMap);
    AutoloadRoot = autoloadMap["root"].getString();
  }

  HardTypeHints = config["HardTypeHints"].getBool(true);
  HardConstProp = config["HardConstProp"].getBool(true);

  EnableHipHopSyntax = config["EnableHipHopSyntax"].getBool();
  EnableZendCompat = config["EnableZendCompat"].getBool();
  JitEnableRenameFunction = config["JitEnableRenameFunction"].getBool();
  EnableHipHopExperimentalSyntax =
    config["EnableHipHopExperimentalSyntax"].getBool();
  EnableShortTags = config["EnableShortTags"].getBool(true);

  IntsOverflowToInts =
    config["Hack"]["Lang"]["IntsOverflowToInts"].getBool(EnableHipHopSyntax);

  EnableAspTags = config["EnableAspTags"].getBool();

  EnableXHP = config["EnableXHP"].getBool(false);

  if (EnableHipHopSyntax) {
    // If EnableHipHopSyntax is true, it forces EnableXHP to true
    // regardless of how it was set in the config
    EnableXHP = true;
  }

  ParserThreadCount = config["ParserThreadCount"].getInt32(0);
  if (ParserThreadCount <= 0) {
    ParserThreadCount = Process::GetCPUCount();
  }

  EnableEval = (EvalLevel)config["EnableEval"].getByte(0);
  AllDynamic = config["AllDynamic"].getBool(true);
  AllVolatile = config["AllVolatile"].getBool();

  GenerateDocComments      = config["GenerateDocComments"].getBool(true);
  EliminateDeadCode        = config["EliminateDeadCode"].getBool(true);
  CopyProp                 = config["CopyProp"].getBool(false);
  LocalCopyProp            = config["LocalCopyProp"].getBool(true);
  StringLoopOpts           = config["StringLoopOpts"].getBool(true);
  AutoInline               = config["AutoInline"].getInt32(0);
  ControlFlow              = config["ControlFlow"].getBool(true);
  VariableCoalescing       = config["VariableCoalescing"].getBool(false);
  ArrayAccessIdempotent    = config["ArrayAccessIdempotent"].getBool(false);
  DumpAst                  = config["DumpAst"].getBool(false);
  WholeProgram             = config["WholeProgram"].getBool(true);
  UseHHBBC                 = config["UseHHBBC"].getBool(UseHHBBC);

  // Temporary, during file-cache migration.
  FileCache::UseNewCache   = config["UseNewCache"].getBool(false);

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
                                    CopyString), sfile, matches);
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
  HHBBC::options.InterceptableFunctions = Option::DynamicInvokeFunctions;
  HHBBC::options.HardConstProp          = Option::HardConstProp;
  HHBBC::options.HardTypeHints          = Option::HardTypeHints;
}

//////////////////////////////////////////////////////////////////////

}
