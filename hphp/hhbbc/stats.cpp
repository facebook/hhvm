/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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
#include "hphp/hhbbc/stats.h"

#include <atomic>
#include <array>
#include <string>
#include <cinttypes>
#include <cstdlib>
#include <iostream>

#include "folly/String.h"
#include "folly/Format.h"
#include "folly/ScopeGuard.h"

#include "hphp/util/trace.h"
#include "hphp/runtime/vm/hhbc.h"

#include "hphp/hhbbc/misc.h"
#include "hphp/hhbbc/representation.h"
#include "hphp/hhbbc/parallel.h"
#include "hphp/hhbbc/index.h"

namespace HPHP { namespace HHBBC {

//////////////////////////////////////////////////////////////////////

namespace {

//////////////////////////////////////////////////////////////////////

struct Stats {
  std::array<std::atomic<uint64_t>,Op_count> op_counts;
};

std::string show(const Stats& stats) {
  auto ret = std::string{};

  ret += "Opcode counts:\n";
  for (auto i = uint32_t{}; i < stats.op_counts.size(); ++i) {
    ret += folly::format(
      "  {: >20}:  {: >15}\n",
      opcodeToName(static_cast<Op>(i)),
      stats.op_counts[i].load()
    ).str();
  }

  return ret;
}

//////////////////////////////////////////////////////////////////////

void count_opcodes(Stats& stats, const php::Program& program) {
  auto count_func = [&] (const php::Func& func) {
    for (auto& blk : func.blocks) {
      for (auto& bc : blk->hhbcs) {
        ++stats.op_counts[static_cast<uint64_t>(bc.op)];
      }
    }
  };

  parallel::for_each(
    program.units,
    [&] (const std::unique_ptr<php::Unit>& unit) {
      for (auto& c : unit->classes) {
        for (auto& m : c->methods) {
          count_func(*m);
        }
      }
      for (auto& x : unit->funcs) {
        count_func(*x);
      }
      count_func(*unit->pseudomain);
    }
  );
}

//////////////////////////////////////////////////////////////////////

}

//////////////////////////////////////////////////////////////////////

void print_stats(const Index&, const php::Program& program) {
  if (!Trace::moduleEnabledRelease(Trace::hhbbc_time, 2)) return;

  trace_time timer("stats");

  Stats stats{};
  count_opcodes(stats, program);

  auto const str = show(stats);
  std::cout << str;

  char fileBuf[] = "/tmp/hhbbcXXXXXX";
  int fd = mkstemp(fileBuf);
  if (fd == -1) {
    std::cerr << "couldn't open temporary file for stats: "
              << folly::errnoStr(errno) << '\n';
    return;
  }
  SCOPE_EXIT { close(fd); };
  auto file = fdopen(fd, "w");
  std::cout << "stats saved to " << fileBuf << '\n';
  std::fprintf(file, "%s", str.c_str());
  std::fflush(file);
}

//////////////////////////////////////////////////////////////////////

}}

