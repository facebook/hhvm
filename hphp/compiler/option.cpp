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

#include <algorithm>
#include <map>
#include <set>
#include <string>
#include <vector>

#include "hphp/compiler/analysis/analysis_result.h"

#include "hphp/parser/scanner.h"

#include "hphp/runtime/base/config.h"
#include "hphp/runtime/base/ini-setting.h"
#include "hphp/runtime/base/preg.h"
#include "hphp/runtime/base/variable-unserializer.h"

#include "hphp/util/file-cache.h"
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

std::vector<std::string> Option::IncludeSearchPaths;

std::set<std::string> Option::VolatileClasses;
std::map<std::string,std::string,stdltistr> Option::AutoloadClassMap;
std::map<std::string,std::string,stdltistr> Option::AutoloadFuncMap;
std::map<std::string,std::string> Option::AutoloadConstMap;
std::string Option::AutoloadRoot;

bool Option::GenerateTextHHBC = false;
bool Option::GenerateHhasHHBC = false;
bool Option::GenerateBinaryHHBC = false;
std::string Option::RepoCentralPath;

std::string Option::IdPrefix = "$$";

std::string Option::LambdaPrefix = "df_";
std::string Option::Tab = "  ";

const char *Option::UserFilePrefix = "php/";

bool Option::KeepStatementsWithNoEffect = false;

std::string Option::ProgramName;

bool Option::EnableShortTags = true;
int Option::ParserThreadCount = 0;

int Option::GetScannerType() {
  int type = 0;
  if (EnableShortTags) type |= Scanner::AllowShortTags;
  return type;
}

bool Option::WholeProgram = true;
bool Option::RecordErrors = true;

bool Option::AllVolatile = false;

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
  LoadRootHdf(ini, config, "IncludeRoots", RuntimeOption::IncludeRoots);
  LoadRootHdf(ini, config, "AutoloadRoots", RuntimeOption::AutoloadRoots);

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

  Config::Bind(VolatileClasses, ini, config, "VolatileClasses");

  Config::GetBool(ini, config, "FlattenTraits");

  for (auto& str : Config::GetStrVector(ini, config, "ConstantFunctions")) {
    std::string func;
    std::string value;
    if (folly::split('|', str, func, value)) {
      VariableUnserializer uns{
        value.data(), value.size(),
        VariableUnserializer::Type::Internal,
        false, empty_dict_array()
      };
      try {
        auto v = uns.unserialize();
        v.setEvalScalar();
        RuntimeOption::ConstantFunctions[func] = *v.asTypedValue();
        continue;
      } catch (const Exception& e) {
        // fall through and log
      }
    }
    Logger::FError("Invalid ConstantFunction: '{}'\n", str);
  }

  {
    // Repo
    {
      // Repo Central
      Config::Bind(RepoCentralPath, ini, config, "Repo.Central.Path");
    }
    Config::Bind(RuntimeOption::RepoDebugInfo,
                 ini, config, "Repo.DebugInfo",
                 RuntimeOption::RepoDebugInfo);
  }

  {
    // AutoloadMap
    // not using Bind here because those maps are enormous and cause performance
    // problems when showing up later
    AutoloadClassMap = Config::GetMapC(ini, config, "AutoloadMap.class");
    AutoloadFuncMap = Config::GetMapC(ini, config, "AutoloadMap.function");
    AutoloadConstMap = Config::GetMap(ini, config, "AutoloadMap.constant");
    AutoloadRoot = Config::GetString(ini, config, "AutoloadMap.root");
  }

 Config::Bind(RuntimeOption::EvalCheckPropTypeHints, ini, config,
               "CheckPropTypeHints", RuntimeOption::EvalCheckPropTypeHints);

  Config::Bind(RuntimeOption::EnableHipHopSyntax,
               ini, config, "EnableHipHopSyntax",
               RuntimeOption::EnableHipHopSyntax);
  Config::Bind(RuntimeOption::EvalJitEnableRenameFunction,
               ini, config, "JitEnableRenameFunction",
               RuntimeOption::EvalJitEnableRenameFunction);
  Config::Bind(EnableShortTags, ini, config, "EnableShortTags", true);

#define BIND_HAC_OPTION(Name, Def)                      \
  Config::Bind(RuntimeOption::EvalHackArrCompat##Name,  \
               ini, config, "HackArrCompat" #Name,      \
               RuntimeOption::EvalHackArrCompat##Def);

#define BIND_HAC_OPTION_SELF(Name)  BIND_HAC_OPTION(Name, Name)

  BIND_HAC_OPTION_SELF(Notices)
  BIND_HAC_OPTION(CheckCompare, Notices)
  BIND_HAC_OPTION_SELF(SerializeNotices)
  BIND_HAC_OPTION_SELF(CompactSerializeNotices)

#undef BIND_HAC_OPTION_SELF
#undef BIND_HAC_OPTION

  Config::Bind(RuntimeOption::EvalHackArrDVArrs,
               ini, config, "HackArrDVArrs",
               RuntimeOption::EvalHackArrDVArrs);

  Config::Bind(RuntimeOption::EvalForbidDynamicCallsToFunc,
               ini, config, "ForbidDynamicCallsToFunc",
               RuntimeOption::EvalForbidDynamicCallsToFunc);
  Config::Bind(RuntimeOption::EvalForbidDynamicCallsToClsMeth,
               ini, config, "ForbidDynamicCallsToClsMeth",
               RuntimeOption::EvalForbidDynamicCallsToClsMeth);
  Config::Bind(RuntimeOption::EvalForbidDynamicCallsToInstMeth,
               ini, config, "ForbidDynamicCallsToInstMeth",
               RuntimeOption::EvalForbidDynamicCallsToInstMeth);
  Config::Bind(RuntimeOption::EvalForbidDynamicConstructs,
               ini, config, "ForbidDynamicConstructs",
               RuntimeOption::EvalForbidDynamicConstructs);
  Config::Bind(RuntimeOption::EvalForbidDynamicCallsWithAttr,
               ini, config, "ForbidDynamicCallsWithAttr",
               RuntimeOption::EvalForbidDynamicCallsWithAttr);
  Config::Bind(RuntimeOption::EvalLogKnownMethodsAsDynamicCalls,
               ini, config, "LogKnownMethodsAsDynamicCalls",
               RuntimeOption::EvalLogKnownMethodsAsDynamicCalls);
  Config::Bind(RuntimeOption::EvalNoticeOnBuiltinDynamicCalls,
               ini, config, "NoticeOnBuiltinDynamicCalls",
               RuntimeOption::EvalNoticeOnBuiltinDynamicCalls);
  Config::Bind(RuntimeOption::EvalAbortBuildOnVerifyError,
               ini, config, "AbortBuildOnVerifyError",
               RuntimeOption::EvalAbortBuildOnVerifyError);

  {
    // Hack
    Config::Bind(RuntimeOption::CheckIntOverflow, ini, config,
                 "Hack.Lang.CheckIntOverflow",
                 RuntimeOption::CheckIntOverflow);
    Config::Bind(RuntimeOption::StrictArrayFillKeys, ini, config,
                 "Hack.Lang.StrictArrayFillKeys",
                 RuntimeOption::StrictArrayFillKeys);
  }

  Config::Bind(RuntimeOption::EnableXHP, ini, config, "EnableXHP",
               RuntimeOption::EnableXHP);

  Config::Bind(ParserThreadCount, ini, config, "ParserThreadCount", 0);
  if (ParserThreadCount <= 0) {
    ParserThreadCount = Process::GetCPUCount();
  }

  // Just to silence warnings until we remove them from various config files
  (void)Config::GetByte(ini, config, "EnableEval", 0);
  (void)Config::GetBool(ini, config, "AllDynamic", true);

  Config::Bind(AllVolatile, ini, config, "AllVolatile");

  Config::Bind(RuntimeOption::EvalGenerateDocComments, ini, config,
               "GenerateDocComments", RuntimeOption::EvalGenerateDocComments);
  Config::Bind(WholeProgram, ini, config, "WholeProgram", true);
  Config::Bind(RuntimeOption::EvalUseHHBBC, ini, config, "UseHHBBC",
               RuntimeOption::EvalUseHHBBC);

  // Temporary, during file-cache migration.
  Config::Bind(FileCache::UseNewCache, ini, config, "UseNewCache", false);

  Config::Bind(RuntimeOption::EvalNoticeOnCoerceForStrConcat, ini, config,
               "NoticeOnCoerceForStrConcat",
               RuntimeOption::EvalNoticeOnCoerceForStrConcat);

  Config::Bind(RuntimeOption::EvalNoticeOnCoerceForBitOp, ini, config,
               "NoticeOnCoerceForBitOp",
               RuntimeOption::EvalNoticeOnCoerceForBitOp);

  RO::EvalArrayProvenance = false;
  RO::EvalLogArrayProvenance = false;
}

void Option::Load() {
}

///////////////////////////////////////////////////////////////////////////////

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
