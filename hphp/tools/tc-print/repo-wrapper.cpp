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
#include "hphp/tools/tc-print/repo-wrapper.h"

#include <cstdio>

#include "hphp/hhvm/process-init.h"
#include "hphp/util/hdf.h"
#include "hphp/util/repo-schema.h"
#include "hphp/compiler/option.h"
#include "hphp/runtime/base/rds.h"
#include "hphp/runtime/base/program-functions.h"
#include "hphp/runtime/base/config.h"
#include "hphp/runtime/ext_hhvm/ext_hhvm.h"
#include "hphp/runtime/vm/runtime.h"

namespace HPHP { namespace jit {

RepoWrapper::RepoWrapper(const char* repoSchema,
                         const std::string& configFile) {
  kRepoSchemaId = repoSchema;

  printf("# Config file: %s\n", configFile.c_str());
  printf("# Repo schema: %s\n", kRepoSchemaId);

  register_process_init();
  initialize_repo();
  hphp_thread_init();
  IniSetting::Map ini = IniSetting::Map::object;
  Hdf config;
  if (!configFile.empty()) {
    Config::ParseConfigFile(configFile, ini, config);
    // Disable logging to suppress harmless errors about setrlimit.
    config["Log"]["Level"] = "None";
  }
  RuntimeOption::Load(ini, config);
  RuntimeOption::RepoCommit = false;
  compile_file(nullptr, 0, MD5(), nullptr);

  repo = &Repo::get();

  RuntimeOption::AlwaysUseRelativePath = false;
  RuntimeOption::SafeFileAccess = false;
  RuntimeOption::EvalAllowHhas = true;
  Option::WholeProgram = false;

  LitstrTable::init();
  LitstrTable::get().setWriting();
  RuntimeOption::RepoAuthoritative = true;
  repo->loadGlobalData(true /* allowFailure */);

  std::string hhasLib;
  auto const phpLib = get_systemlib(&hhasLib);
  always_assert(!hhasLib.empty() && !phpLib.empty());
  auto phpUnit = compile_string(phpLib.c_str(), phpLib.size(),
                                "systemlib.php");
  addUnit(phpUnit);
  auto hhasUnit = compile_string(hhasLib.c_str(), hhasLib.size(),
                                 "systemlib.hhas");
  addUnit(hhasUnit);

  ClassInfo::Load();
  auto nativeFunc = build_native_func_unit(hhbc_ext_funcs,
                                           hhbc_ext_funcs_count);
  addUnit(nativeFunc);
  auto nativeClass = build_native_class_unit(hhbc_ext_classes,
                                             hhbc_ext_class_count);
  addUnit(nativeClass);

  SystemLib::s_inited = true;

  LitstrTable::get().setReading();
}

RepoWrapper::~RepoWrapper() {
  CacheType::const_iterator it;
  for (it = unitCache.begin(); it != unitCache.end(); it++) delete it->second;
}

void RepoWrapper::addUnit(Unit* unit) {
  unitCache.insert({unit->md5(), unit});
}

Unit* RepoWrapper::getUnit(MD5 md5) {
  CacheType::const_iterator it = unitCache.find(md5);
  if (it != unitCache.end()) return it->second;
  auto unit = repo->loadUnit("", md5).release();

  if (unit) unitCache.insert({md5, unit});
  return unit;
}

} }
