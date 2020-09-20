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
#include "hphp/tools/tc-print/repo-wrapper.h"

#include <cstdio>

#include "hphp/hhvm/process-init.h"
#include "hphp/util/hdf.h"
#include "hphp/util/build-info.h"
#include "hphp/compiler/option.h"
#include "hphp/runtime/base/rds.h"
#include "hphp/runtime/base/program-functions.h"
#include "hphp/runtime/base/config.h"
#include "hphp/runtime/vm/native.h"
#include "hphp/runtime/vm/runtime.h"

namespace HPHP { namespace jit {

RepoWrapper::RepoWrapper(const char* repoSchema,
                         const std::string& configFile,
                         const bool shouldPrint) {
  if (setenv("HHVM_RUNTIME_REPO_SCHEMA", repoSchema, 1 /* overwrite */)) {
    fprintf(stderr, "Could not set repo schema");
    exit(EXIT_FAILURE);
  }

  if (shouldPrint) {
    printf("# Config file: %s\n", configFile.c_str());
    printf("# Repo schema: %s\n", repoSchemaId().begin());
  }

  register_process_init();
  initialize_repo();
  hphp_thread_init();
  g_context.getCheck();
  IniSetting::Map ini = IniSetting::Map::object;
  Hdf config;
  if (!configFile.empty()) {
    Config::ParseConfigFile(configFile, ini, config);
    // Disable logging to suppress harmless errors about setrlimit.
    config["Log"]["Level"] = "None";
  }
  RuntimeOption::Load(ini, config);
  RuntimeOption::RepoCommit = false;
  hphp_compiler_init();

  repo = &Repo::get();

  RuntimeOption::AlwaysUseRelativePath = false;
  RuntimeOption::SafeFileAccess = false;
  RuntimeOption::EvalAllowHhas = true;
  RuntimeOption::SandboxMode = true; // So we get Unit::m_funcTable
  Option::WholeProgram = false;

  LitstrTable::init();
  RuntimeOption::RepoAuthoritative = repo->hasGlobalData();
  repo->loadGlobalData();

  std::string hhasLib;
  auto const phpLib = get_systemlib(&hhasLib);
  if (!phpLib.empty()) {
    auto phpUnit = compile_string(phpLib.c_str(), phpLib.size(),
                                  "systemlib.php",
                                  Native::s_systemNativeFuncs,
                                  RepoOptions::defaults());
    addUnit(phpUnit);
  }
  if (!hhasLib.empty()) {
    auto hhasUnit = compile_string(hhasLib.c_str(), hhasLib.size(),
                                   "systemlib.hhas",
                                   Native::s_systemNativeFuncs,
                                   RepoOptions::defaults());
    addUnit(hhasUnit);
  }

  SystemLib::s_inited = true;
}

RepoWrapper::~RepoWrapper() {
  CacheType::const_iterator it;
  for (it = unitCache.begin(); it != unitCache.end(); it++) delete it->second;
}

void RepoWrapper::addUnit(Unit* unit) {
  unitCache.insert({unit->sha1(), unit});
}

Unit* RepoWrapper::getUnit(SHA1 sha1) {
  CacheType::const_iterator it = unitCache.find(sha1);
  if (it != unitCache.end()) return it->second;
  auto unit = repo->loadUnit("", sha1, Native::s_noNativeFuncs).release();

  if (unit) unitCache.insert({sha1, unit});
  return unit;
}

} }
