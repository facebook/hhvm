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

#include "hphp/runtime/vm/jit/tc.h"
#include "hphp/runtime/vm/jit/tc-internal.h"

#include "hphp/runtime/base/rds.h"
#include "hphp/runtime/base/runtime-option.h"

#include "hphp/runtime/vm/jit/code-cache.h"
#include "hphp/runtime/vm/jit/mcgen.h"
#include "hphp/runtime/vm/jit/prof-data.h"
#include "hphp/runtime/vm/jit/trans-db.h"

#include "hphp/util/data-block.h"
#include "hphp/util/build-info.h"

#include <folly/Format.h>

#include <algorithm>
#include <string>
#include <vector>

namespace HPHP { namespace jit { namespace tc {

namespace {

bool dumpTCCode(folly::StringPiece filename) {
#define OPEN_FILE(F, SUFFIX)                                            \
  auto const F ## name = folly::to<std::string>(filename, SUFFIX);      \
  FILE* F = fopen(F ## name .c_str(),"wb");                             \
  if (F == nullptr) return false;                                       \
  SCOPE_EXIT{ fclose(F); };

  OPEN_FILE(ahotFile,       "_ahot");
  OPEN_FILE(aFile,          "_a");
  OPEN_FILE(aprofFile,      "_aprof");
  OPEN_FILE(acoldFile,      "_acold");
  OPEN_FILE(afrozenFile,    "_afrozen");

#undef OPEN_FILE

  // dump starting from the hot region
  auto result = true;
  auto writeBlock = [&](const CodeBlock& cb, FILE* file) {
    if (result) {
      auto const count = cb.used();
      result = fwrite(cb.base(), 1, count, file) == count;
    }
  };

  writeBlock(code().hot(), ahotFile);
  writeBlock(code().main(), aFile);
  writeBlock(code().prof(), aprofFile);
  writeBlock(code().cold(), acoldFile);
  writeBlock(code().frozen(), afrozenFile);
  return result;
}

bool dumpTCData() {
  auto const dataPath = RuntimeOption::EvalDumpTCPath + "/tc_data.txt.gz";
  gzFile tcDataFile = gzopen(dataPath.c_str(), "w");
  if (!tcDataFile) return false;
  SCOPE_EXIT { gzclose(tcDataFile); };

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
                code().hot().base(),    code().hot().frontier(),
                code().main().base(),   code().main().frontier(),
                code().prof().base(),   code().prof().frontier(),
                code().cold().base(),   code().cold().frontier(),
                code().frozen().base(), code().frozen().frontier())) {
    return false;
  }

  if (!gzprintf(tcDataFile, "total_translations = %zu\n\n",
                transdb::getNumTranslations())) {
    return false;
  }

  // Print all translations, including their execution counters. If global
  // counters are disabled (default), fall back to using ProfData, covering
  // only profiling translations.
  if (transdb::enabled()) {
    // Admin requests do not automatically init ProfData, so do it explicitly.
    // No need for matching exit call; data is immortal with trans DB enabled.
    requestInitProfData();
  }
  const TransRec invalid;
  assertx(!invalid.isValid());
  for (TransID t = 0; t < transdb::getNumTranslations(); t++) {
    auto transRec = transdb::getTransRec(t);
    if (!transRec) transRec = &invalid;
    auto const ret = gzputs(tcDataFile, transRec->print().c_str());
    if (ret == -1) {
      return false;
    }
  }

  return true;
}

////////////////////////////////////////////////////////////////////////////////
}

bool dumpEnabled() {
  return RuntimeOption::EvalDumpTC ||
         RuntimeOption::EvalDumpIR ||
         RuntimeOption::EvalDumpRegion ||
         RuntimeOption::EvalDumpInlDecision ||
         RuntimeOption::EvalDumpCallTargets ||
         RuntimeOption::EvalDumpLayoutCFG ||
         RuntimeOption::EvalDumpVBC;
}

bool dump(bool ignoreLease /* = false */) {
  if (!mcgen::initialized()) return false;

  std::unique_lock<SimpleMutex> codeLock;
  std::unique_lock<SimpleMutex> metaLock;
  if (!ignoreLease) {
    codeLock = lockCode();
    metaLock = lockMetadata();
  }
  return dumpTCData() &&
    dumpTCCode(RuntimeOption::EvalDumpTCPath + "/tc_dump");
}

std::vector<UsageInfo> getUsageInfo() {
  std::vector<UsageInfo> tcUsageInfo;

  code().forEachBlock([&] (const char* name, const CodeBlock& a) {
    tcUsageInfo.emplace_back(UsageInfo{
      std::string("code.") + name,
      a.used(),
      a.capacity(),
      true
    });
  });
  tcUsageInfo.emplace_back(UsageInfo{
    "data",
    code().data().used(),
    code().data().capacity(),
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

  code().forEachBlock([&] (const char* name, const CodeBlock& a) {
    addrs += folly::format("{}: {}\n", name, a.base()).str();
  });
  return addrs;
}

std::vector<TCMemInfo> getTCMemoryUsage() {
  std::vector<TCMemInfo> ret;
  code().forEachBlock(
    [&](const char* name, const CodeBlock& a) {
      ret.emplace_back(TCMemInfo{
        name,
        a.used(),
        a.numAllocs(),
        a.numFrees(),
        a.bytesFree(),
        a.blocksFree()
      });
    }
  );
  return ret;
}

}}}
