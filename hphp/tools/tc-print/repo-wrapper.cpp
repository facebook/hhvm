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
#include "hphp/runtime/vm/repo-file.h"
#include "hphp/runtime/vm/repo-global-data.h"
#include "hphp/runtime/vm/runtime.h"
#include "hphp/runtime/vm/unit-emitter.h"

namespace HPHP { namespace jit {

RepoWrapper::RepoWrapper(const char* repoSchema,
                         const std::string& repoFileName,
                         Hdf& config,
                         const bool shouldPrint) {
  if (setenv("HHVM_RUNTIME_REPO_SCHEMA", repoSchema, 1 /* overwrite */)) {
    fprintf(stderr, "Could not set repo schema");
    exit(EXIT_FAILURE);
  }

  if (shouldPrint) {
    printf("# Repo schema: %s\n", repoSchemaId().begin());
  }

  register_process_init();
  hphp_thread_init(true);
  g_context.getCheck();

  IniSetting::Map ini = IniSetting::Map::object;
  RuntimeOption::Load(ini, config);

  hasRepo = !repoFileName.empty();
  if (hasRepo) RepoFile::init(repoFileName);

  RuntimeOption::AlwaysUseRelativePath = false;
  RuntimeOption::SafeFileAccess = false;
  RuntimeOption::EvalAllowHhas = true;
  RuntimeOption::SandboxMode = true; // So we get Unit::m_funcTable
  RuntimeOption::RepoAuthoritative = true;
  RuntimeOption::EvalLowStaticArrays = false; // save some low mem
  RuntimeOption::EvalVerifySystemLibHasNativeImpl = false;

  if (hasRepo) {
    RepoFile::loadGlobalTables();
    RepoFile::globalData().load();
  }
}

RepoWrapper::~RepoWrapper() {
  CacheType::const_iterator it;
  for (it = unitCache.begin(); it != unitCache.end(); it++) delete it->second;

  hphp_thread_exit(true);
}

void RepoWrapper::addUnit(Unit* unit) {
  unitCache.insert({unit->sha1(), unit});
}

Unit* RepoWrapper::getUnit(SHA1 sha1) {
  if (!hasRepo) return nullptr;

  CacheType::const_iterator it = unitCache.find(sha1);
  if (it != unitCache.end()) return it->second;

  auto const unit = [&] () -> Unit* {
    auto const path = RepoFile::findUnitPath(sha1);
    if (!path) return nullptr;
    auto const ue =
      RepoFile::loadUnitEmitter(path, nullptr, false);
    if (!ue) return nullptr;
    return ue->create().release();
  }();

  if (unit) unitCache.insert({sha1, unit});
  return unit;
}

Func* RepoWrapper::getFunc(SHA1 sha1, Id funcSn) {
  if (!hasRepo) return nullptr;

  auto const unit = getUnit(sha1);
  if (!unit) {
    return nullptr;
  }

  Func* func = nullptr;
  unit->forEachFunc([&](Func* f) {
    if (f->sn() == funcSn) {
      func = f;
      return true;
    }
    return false;
  });

  return func;
}

} }
