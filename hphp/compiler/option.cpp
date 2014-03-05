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

#include <boost/algorithm/string/trim.hpp>
#include <map>
#include <set>
#include <vector>

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
#include "hphp/runtime/base/preg.h"
#include "hphp/runtime/base/ini-setting.h"

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
set<string> Option::DynamicInvokeFunctions;
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
bool Option::UseHHBBC = getenv("HHVM_HHBBC");
bool Option::RecordErrors = true;
std::string Option::DocJson;

bool Option::AllDynamic = true;
bool Option::AllVolatile = false;

StringBag Option::OptionStrings;

bool Option::GenerateDocComments = true;

void (*Option::m_hookHandler)(Hdf &config);
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
#define BIND(name, ...) \
        IniSetting::Bind(IniSetting::CORE, IniSetting::PHP_INI_SYSTEM, \
                         "hhvm." #name, __VA_ARGS__)

  LoadRootHdf(config["IncludeRoots"], IncludeRoots);
  BIND(include_roots, &IncludeRoots);
  LoadRootHdf(config["AutoloadRoots"], AutoloadRoots);
  BIND(autoload_roots, &AutoloadRoots);

  config["PackageFiles"].get(PackageFiles);
  BIND(package_files, &PackageFiles);
  config["IncludeSearchPaths"].get(IncludeSearchPaths);
  BIND(include_search_paths, &IncludeSearchPaths);
  config["PackageDirectories"].get(PackageDirectories);
  BIND(package_directories, &PackageDirectories);
  config["PackageExcludeDirs"].get(PackageExcludeDirs);
  BIND(package_exclude_dirs, &PackageExcludeDirs);
  config["PackageExcludeFiles"].get(PackageExcludeFiles);
  BIND(package_exclude_files, &PackageExcludeFiles);
  config["PackageExcludePatterns"].get(PackageExcludePatterns);
  BIND(package_exclude_patterns, &PackageExcludePatterns);
  config["PackageExcludeStaticDirs"].get(PackageExcludeStaticDirs);
  BIND(package_exclude_static_dirs, &PackageExcludeStaticDirs);
  config["PackageExcludeStaticFiles"].get(PackageExcludeStaticFiles);
  BIND(package_exclude_static_files, &PackageExcludeStaticFiles);
  config["PackageExcludeStaticPatterns"].get(PackageExcludeStaticPatterns);
  BIND(package_exclude_static_patterns, &PackageExcludeStaticPatterns);
  CachePHPFile = config["CachePHPFile"].getBool();
  BIND(cache_php_file, &CachePHPFile);

  config["ParseOnDemandDirs"].get(ParseOnDemandDirs);
  BIND(parse_on_demand_dirs, &ParseOnDemandDirs);

  {
    Hdf cg = config["CodeGeneration"];
    string tmp;

#define READ_CG_OPTION(name)                    \
    tmp = cg[#name].getString();                \
    if (!tmp.empty()) {                         \
      name = OptionStrings.add(tmp.c_str());    \
    }                                           \

    READ_CG_OPTION(IdPrefix);
    BIND(code_generation.id_prefix, IniSetting::SetAndGet<std::string>(
      [](const std::string& value) {
        if (value.empty()) {
          return false;
        }
        IdPrefix = OptionStrings.add(value.c_str());
        return true;
      },
      [] {
        return IdPrefix;
      }
    ));
    READ_CG_OPTION(LambdaPrefix);
    BIND(code_generation.lambda_prefix, IniSetting::SetAndGet<std::string>(
      [](const std::string& value) {
        if (value.empty()) {
          return false;
        }
        LambdaPrefix = OptionStrings.add(value.c_str());
        DynamicFunctionPrefixes.push_back(LambdaPrefix);
        return true;
      },
      [] {
        return LambdaPrefix;
      }
    ));
  }

  config["DynamicFunctionPrefix"].get(DynamicFunctionPrefixes);
  BIND(dynamic_function_prefix, &DynamicFunctionPrefixes);
  config["DynamicFunctionPostfix"].get(DynamicFunctionPostfixes);
  BIND(dynamic_function_postfix, &DynamicFunctionPostfixes);
  config["DynamicMethodPrefix"].get(DynamicMethodPrefixes);
  BIND(dynamic_method_prefix, &DynamicMethodPrefixes);
  config["DynamicInvokeFunctions"].get(DynamicInvokeFunctions);
  BIND(dynamic_invoke_functions, &DynamicInvokeFunctions);
  config["VolatileClasses"].get(VolatileClasses);
  BIND(volatile_classes, &VolatileClasses);

  // build map from function names to sections
  for (Hdf hdf = config["FunctionSections"].firstChild(); hdf.exists();
       hdf = hdf.next()) {
    for (Hdf hdfFunc = hdf.firstChild(); hdfFunc.exists();
         hdfFunc = hdfFunc.next()) {
           FunctionSections[hdfFunc.getString()] = hdf.getName();
    }
  }
  typedef std::map<std::string, std::map<std::string, std::string> > MapOfMap;
  BIND(function_setions, IniSetting::SetAndGet<MapOfMap>(
    [](const MapOfMap& value) {
      for (auto& root : value) {
        for (auto& item : root.second) {
          FunctionSections[item.second] = root.first;
        }
      }
      return true;
    },
    nullptr
  ));

  {
    Hdf repo = config["Repo"];
    {
      Hdf repoCentral = repo["Central"];
      RepoCentralPath = repoCentral["Path"].getString();
      BIND(repo.central.path, &RepoCentralPath);
    }
    RepoDebugInfo = repo["DebugInfo"].getBool(false);
    BIND(repo.debug_info, &RepoDebugInfo);
  }

  {
    Hdf autoloadMap = config["AutoloadMap"];
    autoloadMap["class"].get(AutoloadClassMap);
    BIND(autoload_map.class, &AutoloadClassMap);
    autoloadMap["function"].get(AutoloadFuncMap);
    BIND(autoload_map.function, &AutoloadFuncMap);
    autoloadMap["constant"].get(AutoloadConstMap);
    BIND(autoload_map.constant, &AutoloadConstMap);
    AutoloadRoot = autoloadMap["root"].getString();
    BIND(autoload_map.root, &AutoloadRoot);
  }

  HardTypeHints = config["HardTypeHints"].getBool(true);
  BIND(hard_type_hints, &HardTypeHints);
  HardConstProp = config["HardConstProp"].getBool(true);
  BIND(hard_const_prop, &HardConstProp);

  EnableHipHopSyntax = config["EnableHipHopSyntax"].getBool();
  BIND(enable_hip_hop_syntax, &EnableHipHopSyntax);
  EnableZendCompat = config["EnableZendCompat"].getBool();
  BIND(enable_zend_compat, &EnableZendCompat);
  JitEnableRenameFunction = config["JitEnableRenameFunction"].getBool();
  BIND(jit_enable_rename_function, &JitEnableRenameFunction);
  EnableHipHopExperimentalSyntax =
    config["EnableHipHopExperimentalSyntax"].getBool();
  BIND(enable_hip_hop_experimental_syntax, &EnableHipHopExperimentalSyntax);
  EnableShortTags = config["EnableShortTags"].getBool(true);
  BIND(enable_short_tags, &EnableShortTags);

  EnableAspTags = config["EnableAspTags"].getBool();
  BIND(enable_asp_tags, &EnableAspTags);

  EnableXHP = config["EnableXHP"].getBool(false);
  BIND(enable_xhp, &EnableXHP);

  if (EnableHipHopSyntax) {
    // If EnableHipHopSyntax is true, it forces EnableXHP to true
    // regardless of how it was set in the config
    EnableXHP = true;
  }

  ParserThreadCount = config["ParserThreadCount"].getInt32(0);
  if (ParserThreadCount <= 0) {
    ParserThreadCount = Process::GetCPUCount();
  }
  BIND(parser_thread_count, IniSetting::SetAndGet<int32_t>(
    [](const int32_t& value) {
      int32_t set = value;
      if (value <= 0) {
        set = Process::GetCPUCount();
      }
      ParserThreadCount = set;
      return true;
    },
    []() { return ParserThreadCount; }
  ));

  EnableEval = (EvalLevel)config["EnableEval"].getByte(0);
  BIND(parser_thread_count, (char*)&EnableEval);
  AllDynamic = config["AllDynamic"].getBool(true);
  BIND(all_dynamic, (char*)&AllDynamic);
  AllVolatile = config["AllVolatile"].getBool();
  BIND(all_valatile, (char*)&AllVolatile);

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

  BIND(generate_doc_comments, &GenerateDocComments);
  BIND(eliminate_dead_code, &EliminateDeadCode);
  BIND(copy_prop, &CopyProp);
  BIND(local_copy_prop, &LocalCopyProp);
  BIND(string_loop_opts, &StringLoopOpts);
  BIND(auto_inline, &AutoInline);
  BIND(control_flow, &ControlFlow);
  BIND(variable_coalescing, &VariableCoalescing);
  BIND(array_access_idempotent, &ArrayAccessIdempotent);
  BIND(dump_ast, &DumpAst);
  BIND(whole_program, &WholeProgram);
  BIND(use_hhbbc, &UseHHBBC);

  // Temporary, during file-cache migration.
  FileCache::UseNewCache   = config["UseNewCache"].getBool(false);
  BIND(use_new_cache, &FileCache::UseNewCache);

  if (m_hookHandler) m_hookHandler(config);

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
