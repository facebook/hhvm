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

#include "hphp/runtime/base/config.h"
#include "hphp/runtime/base/configs/configs.h"
#include "hphp/runtime/base/ini-setting.h"
#include "hphp/runtime/base/preg.h"
#include "hphp/runtime/base/variable-unserializer.h"

#include "hphp/util/hdf.h"
#include "hphp/util/logger.h"
#include "hphp/util/process.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

hphp_fast_string_set Option::PackageExcludeDirs;
hphp_fast_string_set Option::PackageExcludeFiles;
hphp_fast_string_set Option::PackageExcludePatterns;
hphp_fast_string_set Option::PackageExcludeStaticDirs;
hphp_fast_string_set Option::PackageExcludeStaticFiles;
hphp_fast_string_set Option::PackageExcludeStaticPatterns;

bool Option::CachePHPFile = false;

bool Option::ConstFoldFileBC = false;

bool Option::GenerateTextHHBC = false;
bool Option::GenerateHhasHHBC = false;
bool Option::GenerateBinaryHHBC = false;
bool Option::NoOutputHHBC = false;

int Option::ParserThreadCount = 0;

// These default sizes were selected by experimentation
const int Option::kDefaultParserGroupSize = 500;
const int Option::kDefaultParserDirGroupSizeLimit = 50000;

int Option::ParserGroupSize = kDefaultParserGroupSize;
int Option::ParserDirGroupSizeLimit = kDefaultParserDirGroupSizeLimit;
bool Option::ParserAsyncCleanup = true;
bool Option::ParserOptimisticStore = true;

bool Option::ForceEnableSymbolRefs = false;

std::string Option::ExternWorkerUseCase;
std::string Option::ExternWorkerFeaturesFile;
std::string Option::ExternWorkerPath;
int Option::ExternWorkerTimeoutSecs = 0;
bool Option::ExternWorkerUseExecCache = true;
bool Option::ExternWorkerCleanup = true;
bool Option::ExternWorkerUseRichClient = true;
bool Option::ExternWorkerUseZippyRichClient = true;
bool Option::ExternWorkerUseP2P = false;
int Option::ExternWorkerCasConnectionCount = 16;
int Option::ExternWorkerEngineConnectionCount = 6;
int Option::ExternWorkerAcConnectionCount = 16;
bool Option::ExternWorkerVerboseLogging = false;
std::string Option::ExternWorkerWorkingDir;

int Option::ExternWorkerThrottleRetries = -1;
int Option::ExternWorkerThrottleBaseWaitMSecs = -1;

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

  for (auto& str : Config::GetStrVector(ini, config, "ConstantFunctions")) {
    std::string func;
    std::string value;
    if (folly::split('|', str, func, value)) {
      VariableUnserializer uns{
        value.data(), value.size(),
        VariableUnserializer::Type::Internal,
        /* allowUnknownSerializableClass = */ false,
        empty_dict_array()
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

  Cfg::LoadForCompiler(ini, config);

  {
    // Repo
    Config::Bind(RuntimeOption::RepoDebugInfo,
                 ini, config, "Repo.DebugInfo",
                 RuntimeOption::RepoDebugInfo);
  }

  Config::Bind(RuntimeOption::EvalCheckPropTypeHints, ini, config,
               "CheckPropTypeHints", RuntimeOption::EvalCheckPropTypeHints);

  Config::Bind(RuntimeOption::EvalHackArrCompatSerializeNotices,
               ini, config, "HackArrCompatSerializeNotices",
               RuntimeOption::EvalHackArrCompatSerializeNotices);
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

  Config::Bind(RuntimeOption::EnableXHP, ini, config, "EnableXHP",
               RuntimeOption::EnableXHP);

  Config::Bind(ParserThreadCount, ini, config, "ParserThreadCount", 0);
  if (ParserThreadCount <= 0) {
    ParserThreadCount = Process::GetCPUCount();
  }

  Config::Bind(ForceEnableSymbolRefs, ini, config,
               "ForceEnableSymbolRefs", false);

  Config::Bind(RuntimeOption::EvalGenerateDocComments, ini, config,
               "GenerateDocComments", RuntimeOption::EvalGenerateDocComments);
  Config::Bind(RuntimeOption::EvalUseHHBBC, ini, config, "UseHHBBC",
               RuntimeOption::EvalUseHHBBC);

  Config::Bind(ParserGroupSize, ini, config,
               "ParserGroupSize", kDefaultParserGroupSize);
  Config::Bind(ParserDirGroupSizeLimit, ini, config,
               "ParserDirGroupSizeLimit", kDefaultParserDirGroupSizeLimit);
  if (ParserGroupSize <= 0) ParserGroupSize = kDefaultParserGroupSize;
  if (ParserDirGroupSizeLimit <= 0) {
    ParserDirGroupSizeLimit = kDefaultParserDirGroupSizeLimit;
  }

  Config::Bind(ConstFoldFileBC, ini, config,
               "ConstFoldFileBC", ConstFoldFileBC);

  Config::Bind(ParserAsyncCleanup, ini, config,
               "ParserAsyncCleanup", ParserAsyncCleanup);
  Config::Bind(ParserOptimisticStore, ini, config,
               "ParserOptimisticStore", ParserOptimisticStore);

  // Use case id for remote extern worker implementation.
  // If empty, use the builtin Subprocess impl.
  Config::Bind(ExternWorkerUseCase, ini, config, "ExternWorker.UseCase",
               ExternWorkerUseCase);
  Config::Bind(ExternWorkerFeaturesFile, ini, config,
               "ExternWorker.FeaturesFile", ExternWorkerFeaturesFile);
  // If not set or empty, default to current_executable_path()
  Config::Bind(ExternWorkerPath, ini, config, "ExternWorker.Path",
               ExternWorkerPath);
  Config::Bind(ExternWorkerTimeoutSecs, ini, config, "ExternWorker.TimeoutSecs",
               ExternWorkerTimeoutSecs);
  Config::Bind(ExternWorkerUseExecCache, ini, config,
               "ExternWorker.UseExecCache", ExternWorkerUseExecCache);
  Config::Bind(ExternWorkerCleanup, ini, config, "ExternWorker.Cleanup",
               ExternWorkerCleanup);
  Config::Bind(ExternWorkerWorkingDir, ini, config, "ExternWorker.WorkingDir",
               ExternWorkerWorkingDir);
  Config::Bind(ExternWorkerUseRichClient, ini, config,
               "ExternWorker.UseRichClient", ExternWorkerUseRichClient);
  Config::Bind(ExternWorkerUseZippyRichClient, ini, config,
               "ExternWorker.UseZippyRichClient",
               ExternWorkerUseZippyRichClient);
  Config::Bind(ExternWorkerUseP2P, ini, config, "ExternWorker.UseP2P",
               ExternWorkerUseP2P);
  Config::Bind(ExternWorkerCasConnectionCount, ini, config,
               "ExternWorker.CasConnectionCount",
               ExternWorkerCasConnectionCount);
  Config::Bind(ExternWorkerEngineConnectionCount, ini, config,
               "ExternWorker.EngineConnectionCount",
               ExternWorkerEngineConnectionCount);
  Config::Bind(ExternWorkerAcConnectionCount, ini, config,
               "ExternWorker.AcConnectionCount",
               ExternWorkerAcConnectionCount);
  Config::Bind(ExternWorkerVerboseLogging, ini, config,
               "ExternWorker.VerboseLogging",
               ExternWorkerVerboseLogging);
  Config::Bind(ExternWorkerThrottleRetries, ini, config,
               "ExternWorker.ThrottleRetries",
               ExternWorkerThrottleRetries);
  Config::Bind(ExternWorkerThrottleBaseWaitMSecs, ini, config,
               "ExternWorker.ThrottleBaseWaitMSecs",
               ExternWorkerThrottleBaseWaitMSecs);
}

///////////////////////////////////////////////////////////////////////////////

bool Option::IsFileExcluded(const std::string& file,
                            const hphp_fast_string_set& patterns) {
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

void Option::FilterFiles(std::vector<std::string>& files,
                         const hphp_fast_string_set& patterns) {
  auto const it = std::remove_if(
    files.begin(),
    files.end(),
    [&](const std::string& file) { return IsFileExcluded(file, patterns); });
  files.erase(it, files.end());
}

//////////////////////////////////////////////////////////////////////

}
