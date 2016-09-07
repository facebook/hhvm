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

#include "hphp/runtime/vm/jit/tc-info.h"

#include "hphp/runtime/base/rds.h"
#include "hphp/runtime/base/runtime-option.h"

#include "hphp/runtime/vm/jit/code-cache.h"
#include "hphp/runtime/vm/jit/mc-generator.h"
#include "hphp/runtime/vm/jit/prof-data.h"

#include "hphp/util/data-block.h"
#include "hphp/util/build-info.h"

#include <folly/Format.h>

#include <algorithm>
#include <string>
#include <vector>

namespace HPHP { namespace jit {

namespace {

bool dumpTCCode(const char* filename) {
#define OPEN_FILE(F, SUFFIX)                                    \
  std::string F ## name = std::string(filename).append(SUFFIX); \
  FILE* F = fopen(F ## name .c_str(),"wb");                     \
  if (F == nullptr) return false;                               \
  SCOPE_EXIT{ fclose(F); };

  OPEN_FILE(ahotFile,       "_ahot");
  OPEN_FILE(aFile,          "_a");
  OPEN_FILE(aprofFile,      "_aprof");
  OPEN_FILE(acoldFile,      "_acold");
  OPEN_FILE(afrozenFile,    "_afrozen");
  OPEN_FILE(helperAddrFile, "_helpers_addrs.txt");

#undef OPEN_FILE

  // dump starting from the hot region
  auto result = true;
  auto writeBlock = [&](const CodeBlock& cb, FILE* file) {
    if (result) {
      auto const count = cb.used();
      result = fwrite(cb.base(), 1, count, file) == count;
    }
  };

  writeBlock(mcg->code().hot(), ahotFile);
  writeBlock(mcg->code().main(), aFile);
  writeBlock(mcg->code().prof(), aprofFile);
  writeBlock(mcg->code().cold(), acoldFile);
  writeBlock(mcg->code().frozen(), afrozenFile);
  return result;
}

bool dumpTCData() {
  gzFile tcDataFile = gzopen("/tmp/tc_data.txt.gz", "w");
  if (!tcDataFile) return false;

  if (!gzprintf(tcDataFile,
                "repo_schema      = %s\n"
                "ahot.base        = %p\n"
                "ahot.frontier    = %p\n"
                "a.base           = %p\n"
                "a.frontier       = %p\n"
                "aprof.base       = %p\n"
                "aprof.frontier   = %p\n"
                "acold.base       = %p\n"
                "acold.frontier   = %p\n"
                "afrozen.base     = %p\n"
                "afrozen.frontier = %p\n\n",
                repoSchemaId().begin(),
                mcg->code().hot().base(),    mcg->code().hot().frontier(),
                mcg->code().main().base(),   mcg->code().main().frontier(),
                mcg->code().prof().base(),   mcg->code().prof().frontier(),
                mcg->code().cold().base(),   mcg->code().cold().frontier(),
                mcg->code().frozen().base(), mcg->code().frozen().frontier())) {
    return false;
  }

  if (!gzprintf(tcDataFile, "total_translations = %zu\n\n",
                mcg->tx().getNumTranslations())) {
    return false;
  }

  // Print all translations, including their execution counters. If global
  // counters are disabled (default), fall back to using ProfData, covering
  // only profiling translations.
  if (!RuntimeOption::EvalJitTransCounters && Translator::isTransDBEnabled()) {
    // Admin requests do not automatically init ProfData, so do it explicitly.
    // No need for matching exit call; data is immortal with trans DB enabled.
    requestInitProfData();
  }
  for (TransID t = 0; t < mcg->tx().getNumTranslations(); t++) {
    int64_t count = 0;
    if (RuntimeOption::EvalJitTransCounters) {
      count = mcg->tx().getTransCounter(t);
    } else if (auto prof = profData()) {
      assertx(mcg->tx().getTransCounter(t) == 0);
      count = prof->transCounter(t);
    }
    auto const ret = gzputs(
      tcDataFile, mcg->tx().getTransRec(t)->print(count).c_str()
    );
    if (ret == -1) {
      return false;
    }
  }

  gzclose(tcDataFile);
  return true;
}

////////////////////////////////////////////////////////////////////////////////
}

bool dumpTC(bool ignoreLease /* = false */) {
  if (!mcg) return false;

  std::unique_lock<SimpleMutex> codeLock;
  std::unique_lock<SimpleMutex> metaLock;
  if (!ignoreLease) {
    codeLock = mcg->lockCode();
    metaLock = mcg->lockMetadata();
  }
  return dumpTCData() && dumpTCCode("/tmp/tc_dump");
}

std::vector<UsageInfo> getUsageInfo() {
  auto const& code = mcg->code();
  std::vector<UsageInfo> tcUsageInfo;

  mcg->code().forEachBlock([&] (const char* name, const CodeBlock& a) {
    tcUsageInfo.emplace_back(UsageInfo{
      std::string("code.") + name,
      a.used(),
      a.capacity(),
      true
    });
  });
  tcUsageInfo.emplace_back(UsageInfo{
    "data",
    code.data().used(),
    code.data().capacity(),
    true
  });
  tcUsageInfo.emplace_back(UsageInfo{
    "RDS",
    rds::usedBytes(),
    RuntimeOption::EvalJitTargetCacheSize * 3 / 4,
    false
  });
  tcUsageInfo.emplace_back(UsageInfo{
    "RDSLocal",
    rds::usedLocalBytes(),
    RuntimeOption::EvalJitTargetCacheSize * 3 / 4,
    false
  });
  tcUsageInfo.emplace_back(UsageInfo{
    "persistentRDS",
    rds::usedPersistentBytes(),
    RuntimeOption::EvalJitTargetCacheSize / 4,
    false
  });
  return tcUsageInfo;
}

std::string getTCSpace() {
  std::string usage;
  size_t total_size = 0;
  size_t total_capacity = 0;

  auto const add_row = [&] (const UsageInfo& ui) {
    auto const percent = ui.capacity ?  100 * ui.used / ui.capacity : 0;

    usage += folly::format(
      "mcg: {:9} bytes ({}%) in {}\n",
      ui.used, percent, ui.name
    ).str();

    if (ui.global) {
      total_size += ui.used;
      total_capacity += ui.capacity;
    }
  };

  auto const uis = getUsageInfo();
  std::for_each(uis.begin(), uis.end(), add_row);
  add_row(UsageInfo { "total", total_size, total_capacity, false });

  return usage;
}

std::string getTCAddrs() {
  std::string addrs;

  mcg->code().forEachBlock([&] (const char* name, const CodeBlock& a) {
    addrs += folly::format("{}: {}\n", name, a.base()).str();
  });
  return addrs;
}

///////////////////////////////////////////////////////////////////////////////

}}
