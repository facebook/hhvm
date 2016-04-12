/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2016 Facebook, Inc. (http://www.facebook.com)     |
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

#include <map>
#include <set>
#include <string>
#include <vector>

#include "hphp/compiler/analysis/analysis_result.h"
#include "hphp/compiler/analysis/class_scope.h"
#include "hphp/compiler/analysis/file_scope.h"
#include "hphp/compiler/analysis/variable_table.h"

#include "hphp/parser/scanner.h"

#include "hphp/runtime/base/config.h"
#include "hphp/runtime/base/ini-setting.h"
#include "hphp/runtime/base/preg.h"

#include "hphp/util/hdf.h"
#include "hphp/util/logger.h"
#include "hphp/util/process.h"
#include "hphp/util/text-util.h"

#include "hphp/hhbbc/hhbbc.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

std::string Option::RootDirectory;
std::set<std::string> Option::PackageDirectories;
std::set<std::string> Option::PackageFiles;
std::set<std::string> Option::PackageExcludeDirs;
std::set<std::string> Option::PackageExcludeFiles;
std::set<std::string> Option::PackageExcludePatterns;
std::set<std::string> Option::PackageExcludeStaticDirs;
std::set<std::string> Option::PackageExcludeStaticFiles;
std::set<std::string> Option::PackageExcludeStaticPatterns;
bool Option::CachePHPFile = false;

std::vector<std::string> Option::ParseOnDemandDirs;

std::map<std::string, std::string> Option::IncludeRoots;
std::map<std::string, std::string> Option::AutoloadRoots;
std::vector<std::string> Option::IncludeSearchPaths;
std::string Option::DefaultIncludeRoot;
std::map<std::string, int> Option::DynamicFunctionCalls;

bool Option::GeneratePickledPHP = false;
bool Option::GenerateInlinedPHP = false;
bool Option::GenerateTrimmedPHP = false;
bool Option::ConvertSuperGlobals = false;
bool Option::ConvertQOpExpressions = false;
std::string Option::ProgramPrologue;
std::string Option::TrimmedPrologue;
std::set<std::string, stdltistr> Option::DynamicInvokeFunctions;
std::set<std::string> Option::VolatileClasses;
std::map<std::string,std::string,stdltistr> Option::AutoloadClassMap;
std::map<std::string,std::string,stdltistr> Option::AutoloadFuncMap;
std::map<std::string,std::string> Option::AutoloadConstMap;
std::string Option::AutoloadRoot;

std::map<std::string, std::string> Option::FunctionSections;

bool Option::GenerateTextHHBC = false;
bool Option::GenerateBinaryHHBC = false;
std::string Option::RepoCentralPath;
bool Option::RepoDebugInfo = false;

std::string Option::IdPrefix = "$$";

std::string Option::LambdaPrefix = "df_";
std::string Option::Tab = "  ";

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

bool Option::DumpAst = false;
bool Option::WholeProgram = true;
bool Option::UseHHBBC = !getenv("HHVM_DISABLE_HHBBC2");
bool Option::RecordErrors = true;

bool Option::AllVolatile = false;

StringBag Option::OptionStrings;

bool Option::GenerateDocComments = true;

///////////////////////////////////////////////////////////////////////////////
// load from HDF file

void Option::LoadRootHdf(const IniSetting::Map& ini,
                         const Hdf &roots,
                         const std::string& name,
                         std::map<std::string, std::string> &map) {
  auto root_map_callback = [&] (const IniSetting::Map &ini_rm,
                                const Hdf &hdf_rm,
                                const std::string &ini_rm_key) {
    map[Config::GetString(ini_rm, hdf_rm, "root", "", false)] =
      Config::GetString(ini_rm, hdf_rm, "path", "", false);
  };
  Config::Iterate(root_map_callback, ini, roots, name);
}

void Option::LoadRootHdf(const IniSetting::Map& ini,
                         const Hdf &roots,
                         const std::string& name,
                         std::vector<std::string> &vec) {
  auto root_vec_callback = [&] (const IniSetting::Map &ini_rv,
                                const Hdf &hdf_rv,
                                const std::string &ini_rv_key) {
    vec.push_back(Config::GetString(ini_rv, hdf_rv, "", "", false));
  };
  Config::Iterate(root_vec_callback, ini, roots, name);
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
    std::string tmp;

#define READ_CG_OPTION(name)                    \
    tmp = Config::GetString(ini, config, "CodeGeneration."#name); \
    if (!tmp.empty()) {                         \
      name = OptionStrings.add(tmp.c_str());    \
    }

    READ_CG_OPTION(IdPrefix);
    READ_CG_OPTION(LambdaPrefix);
  }

  Config::Bind(DynamicInvokeFunctions, ini, config, "DynamicInvokeFunctions");
  Config::Bind(VolatileClasses, ini, config, "VolatileClasses");

  // build map from function names to sections
  auto function_sections_callback = [&] (const IniSetting::Map &ini_fs,
                                         const Hdf &hdf_fs,
                                         const std::string &ini_fs_key) {
    auto function_callback = [&] (const IniSetting::Map &ini_f,
                                  const Hdf &hdf_f,
                                  const std::string &ini_f_key) {
      FunctionSections[Config::GetString(ini_f, hdf_f, "", "", false)] =
        hdf_fs.exists() && !hdf_fs.isEmpty() ? hdf_fs.getName() : ini_fs_key;
    };
    Config::Iterate(function_callback, ini_fs, hdf_fs, "", false);
  };
  Config::Iterate(function_sections_callback, ini, config, "FunctionSections");

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

  // Just to silence warnings until we remove them from various config files
  (void)Config::GetByte(ini, config, "EnableEval", 0);
  (void)Config::GetBool(ini, config, "AllDynamic", true);

  Config::Bind(AllVolatile, ini, config, "AllVolatile");

  Config::Bind(GenerateDocComments, ini, config, "GenerateDocComments", true);
  Config::Bind(DumpAst, ini, config, "DumpAst", false);
  Config::Bind(WholeProgram, ini, config, "WholeProgram", true);
  Config::Bind(UseHHBBC, ini, config, "UseHHBBC", UseHHBBC);

  // Temporary, during file-cache migration.
  Config::Bind(FileCache::UseNewCache, ini, config, "UseNewCache", false);
}

void Option::Load() {
}

///////////////////////////////////////////////////////////////////////////////

std::string Option::GetAutoloadRoot(const std::string &name) {
  for (auto const& pair : AutoloadRoots) {
    if (name.substr(0, pair.first.length()) == pair.first) {
      return pair.second;
    }
  }
  return "";
}

std::string Option::MangleFilename(const std::string &name, bool id) {
  std::string ret = UserFilePrefix;
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
  for (auto const& pattern : patterns) {
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
