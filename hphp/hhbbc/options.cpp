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

#include "hphp/hhbbc/options.h"

#include <mutex>

#include "hphp/util/alloc.h"
#include "hphp/util/logger.h"
#include "hphp/util/process.h"
#include "hphp/util/trace.h"

namespace HPHP { namespace HHBBC {

//////////////////////////////////////////////////////////////////////

namespace {

std::mutex s_maxMemMutex;
std::string s_maxMemPhase;
int64_t s_maxMemUsageMb;

void update_memory_stats(const char* what, const char* when,
                         const std::string& extra) {
  auto const phase = extra.empty()
    ? folly::sformat("{} {}", what, when)
    : folly::sformat("{} {} ({})", what, when, extra);

  auto const usage = Process::GetMemUsageMb();
  {
    std::lock_guard<std::mutex> _{s_maxMemMutex};
    if (usage > s_maxMemUsageMb) {
      s_maxMemUsageMb = usage;
      s_maxMemPhase = phase;
    }
  }

  if (Trace::moduleEnabledRelease(Trace::hhbbc_mem, 1)) {
    auto const message = folly::sformat("RSS at {}: {} Mb\n", phase, usage);
    Trace::traceRelease("%s", message.c_str());
  }
}

}

//////////////////////////////////////////////////////////////////////

struct Options options;

void profile_memory(const char* what, const char* when,
                    const std::string& extra) {
  update_memory_stats(what, when, extra);

  if (options.profileMemory.empty()) return;

  auto name = folly::sformat(
    "{}_{}{}_{}",
    options.profileMemory,
    what,
    extra.empty() ? extra : folly::sformat("_{}", extra),
    when
  );

  while (true) {
    auto const pos = name.find_first_of(" :\"'");
    if (pos == std::string::npos) break;
    name[pos] = '_';
  }
  jemalloc_pprof_dump(name, true);
}

void summarize_memory() {
  std::lock_guard<std::mutex> _{s_maxMemMutex};
  if (!s_maxMemUsageMb) return;
  Logger::Info("%s", folly::sformat(
    "Max RSS at {}: {} Mb",
    s_maxMemPhase,
    s_maxMemUsageMb
  ).c_str());
}

//////////////////////////////////////////////////////////////////////

}}
