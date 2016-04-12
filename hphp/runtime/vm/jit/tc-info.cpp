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

#include "hphp/util/data-block.h"

#include <folly/Format.h>

#include <algorithm>
#include <string>
#include <vector>

namespace HPHP { namespace jit {

///////////////////////////////////////////////////////////////////////////////

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
