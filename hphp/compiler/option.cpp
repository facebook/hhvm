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
hphp_string_imap<std::string> Option::ConstantFunctions;

bool Option::GeneratePickledPHP = false;
bool Option::GenerateInlinedPHP = false;
bool Option::GenerateTrimmedPHP = false;
bool Option::ConvertSuperGlobals = false;
std::string Option::ProgramPrologue;
std::string Option::TrimmedPrologue;
std::set<std::string> Option::VolatileClasses;
std::map<std::string,std::string,stdltistr> Option::AutoloadClassMap;
std::map<std::string,std::string,stdltistr> Option::AutoloadFuncMap;
std::map<std::string,std::string> Option::AutoloadConstMap;
std::string Option::AutoloadRoot;

std::vector<std::string> Option::APCProfile;

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

bool Option::KeepStatementsWithNoEffect = false;

std::string Option::ProgramName;

bool Option::ParseTimeOpts = true;
bool Option::EnableHipHopExperimentalSyntax = false;
bool Option::EnableShortTags = true;
bool Option::EnableAspTags = false;
int Option::ParserThreadCount = 0;

int Option::GetScannerType() {
  int type = 0;
  if (EnableShortTags) type |= Scanner::AllowShortTags;
  if (EnableAspTags) type |= Scanner::AllowAspTags;
  if (RuntimeOption::EnableXHP) type |= Scanner::AllowXHPSyntax;
  if (RuntimeOption::EnableHipHopSyntax) type |= Scanner::AllowHipHopSyntax;
  return type;
}

bool Option::DumpAst = false;
bool Option::WholeProgram = true;
bool Option::UseHHBBC = !getenv("HHVM_DISABLE_HHBBC");
bool Option::RecordErrors = true;

bool Option::AllVolatile = false;

bool Option::GenerateDocComments = true;

///////////////////////////////////////////////////////////////////////////////
// load from HDF file

void Option::LoadRootHdf(const IniSetting::Map& ini,
                         const Hdf &roots,
                         const std::string& name,
                         std::map<std::string, std::string> &map) {
  auto root_map_callback = [&](const IniSetting::Map& ini_rm, const Hdf& hdf_rm,
                               const std::string& /*ini_rm_key*/) {
    map[Config::GetString(ini_rm, hdf_rm, "root", "", false)] =
      Config::GetString(ini_rm, hdf_rm, "path", "", false);
  };
  Config::Iterate(root_map_callback, ini, roots, name);
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

  Config::Bind(IdPrefix, ini, config, "CodeGeneration.IdPrefix", IdPrefix);
  Config::Bind(LambdaPrefix, ini, config,
               "CodeGeneration.LambdaPrefix", LambdaPrefix);

  Config::Bind(RuntimeOption::DynamicInvokeFunctions,
               ini, config, "DynamicInvokeFunctions",
               RuntimeOption::DynamicInvokeFunctions);
  Config::Bind(VolatileClasses, ini, config, "VolatileClasses");

  for (auto& str : Config::GetVector(ini, config, "ConstantFunctions")) {
    std::string func;
    std::string value;
    if (folly::split('|', str, func, value)) {
      ConstantFunctions[func] = value;
    } else {
      std::cerr << folly::format("Invalid ConstantFunction: '{}'\n", str);
    }
  }

  // build map from function names to sections
  auto function_sections_callback = [&](const IniSetting::Map& ini_fs,
                                        const Hdf& hdf_fs,
                                        const std::string& ini_fs_key) {
    auto function_callback = [&](const IniSetting::Map& ini_f, const Hdf& hdf_f,
                                 const std::string& /*ini_f_key*/) {
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

  Config::Bind(HHBBC::options.HardTypeHints, ini, config,
               "HardTypeHints", true);
  Config::Bind(HHBBC::options.HardReturnTypeHints, ini, config,
               "HardReturnTypeHints", true);
  Config::Bind(HHBBC::options.HardConstProp, ini, config,
               "HardConstProp", true);
  Config::Bind(HHBBC::options.ElideAutoloadInvokes, ini, config,
               "ElideAutoloadInvokes", true);

  Config::Bind(APCProfile, ini, config, "APCProfile");

  Config::Bind(HHBBC::options.CheckThisTypeHints, ini, config,
               "CheckThisTypeHints", false);
  Config::Bind(RuntimeOption::EnableHipHopSyntax,
               ini, config, "EnableHipHopSyntax",
               RuntimeOption::EnableHipHopSyntax);
  Config::Bind(RuntimeOption::EvalPromoteEmptyObject,
               ini, config, "PromoteEmptyObject",
               RuntimeOption::EvalPromoteEmptyObject);
  Config::Bind(RuntimeOption::EnableZendCompat, ini, config, "EnableZendCompat",
               RuntimeOption::EnableZendCompat);
  Config::Bind(RuntimeOption::EvalJitEnableRenameFunction,
               ini, config, "JitEnableRenameFunction",
               RuntimeOption::EvalJitEnableRenameFunction);
  Config::Bind(EnableHipHopExperimentalSyntax, ini,
               config, "EnableHipHopExperimentalSyntax");
  Config::Bind(EnableShortTags, ini, config, "EnableShortTags", true);
  Config::Bind(RuntimeOption::EvalHackArrCompatNotices,
               ini, config, "HackArrCompatNotices",
               RuntimeOption::EvalHackArrCompatNotices);

  {
    // Hack
    Config::Bind(RuntimeOption::IntsOverflowToInts, ini, config,
                 "Hack.Lang.IntsOverflowToInts",
                 RuntimeOption::IntsOverflowToInts);
    Config::Bind(RuntimeOption::StrictArrayFillKeys, ini, config,
                 "Hack.Lang.StrictArrayFillKeys",
                 RuntimeOption::StrictArrayFillKeys);
    Config::Bind(RuntimeOption::DisallowDynamicVarEnvFuncs, ini, config,
                 "Hack.Lang.DisallowDynamicVarEnvFuncs",
                 RuntimeOption::DisallowDynamicVarEnvFuncs);
  }

  Config::Bind(EnableAspTags, ini, config, "EnableAspTags");

  Config::Bind(RuntimeOption::EnableXHP, ini, config, "EnableXHP",
               RuntimeOption::EnableXHP);

  if (RuntimeOption::EnableHipHopSyntax) {
    // If EnableHipHopSyntax is true, it forces EnableXHP to true
    // regardless of how it was set in the config
    RuntimeOption::EnableXHP = true;
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
  auto const it = std::remove_if(
    files.begin(),
    files.end(),
    [&](const std::string& file) { return IsFileExcluded(file, patterns); });
  files.erase(it, files.end());
}

//////////////////////////////////////////////////////////////////////

}
