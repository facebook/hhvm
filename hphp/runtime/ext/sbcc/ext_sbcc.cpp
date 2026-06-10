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

#include "hphp/runtime/base/array-init.h"
#include "hphp/runtime/base/config.h"
#include "hphp/runtime/ext/sbcc/format/sbcc-cache.h"
#include "hphp/runtime/ext/extension.h"
#include "hphp/util/configs/eval.h"
#include "hphp/util/configs/repo.h"
#include "hphp/util/logger.h"
#include "hphp/util/struct-log.h"

namespace HPHP {

///////////////////////////////////////////////////////////////////////////////

// Returns per-request hit/miss/corrupt counters.
Array HHVM_FUNCTION(sbcc_get_stats) {
  auto stats = SandboxBytecodeCache::getRequestStats();
  return make_dict_array(
    OptString("hits"),    stats.hits,
    OptString("misses"),  stats.misses,
    OptString("corrupt"), stats.corrupt
  );
}

// Reset per-request counters to zero.
void HHVM_FUNCTION(sbcc_reset_stats) {
  SandboxBytecodeCache::resetRequestStats();
}

// Returns process-wide (global) ServiceData counter values.
Array HHVM_FUNCTION(sbcc_get_global_stats) {
  auto stats = SandboxBytecodeCache::getStats();
  return make_dict_array(
    OptString("hits"),        stats.hits,
    OptString("misses"),      stats.misses,
    OptString("corrupt"),     stats.corrupt,
    OptString("init_errors"), stats.init_errors
  );
}

///////////////////////////////////////////////////////////////////////////////

static struct SBCCExtension final : Extension {
  SBCCExtension() : Extension("sbcc", "1.0", "sandbox_infra") {}

  // Only register the cache layer when SBCC is configured.
  // This avoids adding a dispatcher hop for servers that don't use SBCC.
  void moduleLoad(const IniSetting::Map& ini, Hdf config) override {
    // Read Eval.SBCCPath directly from the Hdf config tree rather than using
    // Config::Bind. The typed accessor Cfg::Eval::SBCCPath (auto-generated
    // from configs.specification) already binds this option during
    // RuntimeOption::Load. Calling Config::Bind here would double-bind it,
    // which corrupts the config system state in repo_separate builds and
    // causes unrelated cli-server tests to fail.
    auto sbccPath = config["Eval"]["SBCCPath"];
    if (sbccPath.exists() && !sbccPath.configGetString("").empty()) {
      SandboxBytecodeCache::registerAsLayer();
    }
  }

  // Open and validate the .sbcc file now that RepoOptions is initialized.
  void moduleInit() override {
    if (Cfg::Repo::Authoritative || Cfg::Eval::SBCCPath.empty()) return;
    if (Cfg::Eval::EnableDecl) {
      Logger::FInfo("SBCC disabled: incompatible with Eval.EnableDecl");
      return;
    }
    try {
      SandboxBytecodeCache::init(Cfg::Eval::SBCCPath);
      Logger::FInfo("Loaded sandbox bytecode cache from {}", Cfg::Eval::SBCCPath);
    } catch (const std::exception& e) {
      SandboxBytecodeCache::incrementInitErrors();
      Logger::FError("Failed to load sandbox bytecode cache: {}", e.what());
    }
  }

  void moduleShutdown() override {
    SandboxBytecodeCache::destroy();
  }

  void requestInit() override {
    SandboxBytecodeCache::resetRequestStats();
  }

  void requestShutdown() override {
    auto stats = SandboxBytecodeCache::getRequestStats();
    if (stats.corrupt > 0) {
      Logger::Warning("SBCC: %ld corrupt cache %s this request",
                      stats.corrupt,
                      stats.corrupt == 1 ? "entry" : "entries");
    }

    // Log per-request SBCC stats to Scuba when any SBCC activity occurred.
    if (SandboxBytecodeCache::has() &&
        (stats.hits > 0 || stats.misses > 0)) {
      StructuredLogEntry ent;
      ent.setInt("hits",    stats.hits);
      ent.setInt("misses",  stats.misses);
      ent.setInt("corrupt", stats.corrupt);
      StructuredLog::log("hhvm_sbcc_stats", ent);
    }

    SandboxBytecodeCache::onRequestShutdown();
  }

  // Register Hack-visible natives.
  void moduleRegisterNative() override {
    HHVM_FALIAS(HH\\SBCC\\get_stats, sbcc_get_stats);
    HHVM_FALIAS(HH\\SBCC\\reset_stats, sbcc_reset_stats);
    HHVM_FALIAS(HH\\SBCC\\get_global_stats, sbcc_get_global_stats);
  }

} s_sbcc_extension;

///////////////////////////////////////////////////////////////////////////////

} // namespace HPHP
