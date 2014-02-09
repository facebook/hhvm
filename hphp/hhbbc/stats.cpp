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

#include "folly/Conv.h"
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

#define STAT_TYPES                              \
  X(Gen)                                        \
  X(InitGen)                                    \
  X(Ref)                                        \
  X(Cell)                                       \
  X(InitCell)                                   \
  X(Unc)                                        \
  X(InitUnc)                                    \
  X(Obj)                                        \
  X(OptObj)                                     \
  X(Null)                                       \
  X(Bottom)

struct TypeStat {
#define X(x) std::atomic<uint64_t> sub_##x; \
             std::atomic<uint64_t> eq_##x;
  STAT_TYPES
#undef X
};

struct Stats {
  std::array<std::atomic<uint64_t>,Op_count> op_counts;
  TypeStat returns;
  TypeStat privateProps;
  TypeStat privateStatics;
};

std::string type_stat_string(const std::string& prefix, const TypeStat& st) {
  auto ret = std::string{};
#define X(x)                                      \
  ret += folly::format("  {} = {: <9} {: >8}\n",  \
    prefix, #x ":", st.eq_##x.load()).str();      \
  ret += folly::format("  {} < {: <9} {: >8}\n",  \
    prefix, #x ":", st.sub_##x.load()).str();
  STAT_TYPES
#undef X
  ret += "\n";
  return ret;
}

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
  ret += "\n";

  ret += type_stat_string("ret", stats.returns);
  ret += type_stat_string("priv prop", stats.privateProps);
  ret += type_stat_string("priv static", stats.privateStatics);

  return ret;
}

//////////////////////////////////////////////////////////////////////

void add_type(TypeStat& stat, const Type& t) {
#define X(x)                                    \
  if (t.strictSubtypeOf(T##x)) ++stat.sub_##x;  \
  if (t == T##x) ++stat.eq_##x;
  STAT_TYPES
#undef X
}

//////////////////////////////////////////////////////////////////////

void collect_func(Stats& stats, const Index& index, const php::Func& func) {
  auto const ty = index.lookup_return_type_raw(&func);
  add_type(stats.returns, ty);

  for (auto& blk : func.blocks) {
    for (auto& bc : blk->hhbcs) {
      ++stats.op_counts[static_cast<uint64_t>(bc.op)];
    }
  }
}

void collect_class(Stats& stats, const Index& index, const php::Class& cls) {
  for (auto& kv : index.lookup_private_props(&cls)) {
    add_type(stats.privateProps, kv.second);
  }
  for (auto& kv : index.lookup_private_statics(&cls)) {
    add_type(stats.privateStatics, kv.second);
  }
}

void collect_stats(Stats& stats,
                   const Index& index,
                   const php::Program& program) {
  parallel::for_each(
    program.units,
    [&] (const std::unique_ptr<php::Unit>& unit) {
      for (auto& c : unit->classes) {
        collect_class(stats, index, *c);
        for (auto& m : c->methods) {
          collect_func(stats, index, *m);
        }
      }
      for (auto& x : unit->funcs) {
        collect_func(stats, index, *x);
      }
      collect_func(stats, index, *unit->pseudomain);
    }
  );
}

//////////////////////////////////////////////////////////////////////

}

//////////////////////////////////////////////////////////////////////

void print_stats(const Index& index, const php::Program& program) {
  if (!Trace::moduleEnabledRelease(Trace::hhbbc_time, 1)) return;

  trace_time timer("stats");

  Stats stats{};
  collect_stats(stats, index, program);

  auto const str = show(stats);
  if (Trace::moduleEnabledRelease(Trace::hhbbc_time, 2)) {
    std::cout << str;
  }

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

