/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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
#include <memory>

#include <boost/variant.hpp>

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
#include "hphp/hhbbc/interp.h"
#include "hphp/hhbbc/analyze.h"

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
  X(Arr)                                        \
  X(SArr)                                       \
  X(CArr)                                       \
  X(ArrE)                                       \
  X(ArrN)                                       \
  X(SArrN)                                      \
  X(SArrE)                                      \
  X(CArrN)                                      \
  X(CArrE)                                      \
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
  std::atomic<uint64_t> persistentClasses;
  std::atomic<uint64_t> persistentFunctions;
  std::atomic<uint64_t> uniqueClasses;
  std::atomic<uint64_t> uniqueFunctions;
  std::atomic<uint64_t> totalClasses;
  std::atomic<uint64_t> totalFunctions;
  TypeStat returns;
  TypeStat privateProps;
  TypeStat privateStatics;
  TypeStat cgetmBase;
};

void type_stat_string(std::string& ret,
                      const std::string& prefix,
                      const TypeStat& st) {
#define X(x)                                    \
  folly::format(&ret, "  {}_=_{: <9} {: >8}\n",  \
    prefix, #x ":", st.eq_##x.load());          \
  folly::format(&ret, "  {}_<_{: <9} {: >8}\n",  \
    prefix, #x ":", st.sub_##x.load());
  STAT_TYPES
#undef X
  ret += "\n";
}

std::string show(const Stats& stats) {
  auto ret = std::string{};

  for (auto i = uint32_t{}; i < stats.op_counts.size(); ++i) {
    folly::format(
      &ret,
      "  {: >20}:  {: >15}\n",
      opcodeToName(static_cast<Op>(i)),
      stats.op_counts[i].load()
    );
  }
  ret += "\n";

  type_stat_string(ret, "ret", stats.returns);
  type_stat_string(ret, "priv_prop", stats.privateProps);
  type_stat_string(ret, "priv_static", stats.privateStatics);
  type_stat_string(ret, "cgetm_base", stats.cgetmBase);

  folly::format(
    &ret,
    "         total_funcs:  {: >8}\n"
    "        unique_funcs:  {: >8}\n"
    "    persistent_funcs:  {: >8}\n"
    "       total_classes:  {: >8}\n"
    "      unique_classes:  {: >8}\n"
    "  persistent_classes:  {: >8}\n",
    stats.totalFunctions.load(),
    stats.uniqueFunctions.load(),
    stats.persistentFunctions.load(),
    stats.totalClasses.load(),
    stats.uniqueClasses.load(),
    stats.persistentClasses.load()
  );

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

struct StatsVisitor : boost::static_visitor<void> {
  explicit StatsVisitor(Stats& stats, const State& state)
    : m_stats(stats)
    , m_state(state)
  {}

  void operator()(const bc::CGetM& op) const {
    auto const pops = op.numPop();
    auto const ty = [&] {
      switch (op.mvec.lcode) {
      case LL:
        return m_state.locals[op.mvec.locBase->id];
      case LC:
      case LR:
        return topT(pops - 1);
      case LH:
      case LGL:
      case LGC:
      case LNL:
      case LNC:
      case LSL:
      case LSC:
        return TTop;
      case NumLocationCodes:
        break;
      }
      not_reached();
    }();
    add_type(m_stats.cgetmBase, ty);
  }

  template<class T> void operator()(const T& op) const {}

private:
  Type topT(uint32_t idx = 0) const {
    assert(idx < m_state.stack.size());
    return m_state.stack[m_state.stack.size() - idx - 1];
  }

private:
  Stats& m_stats;
  const State& m_state;
};

//////////////////////////////////////////////////////////////////////

void collect_func(Stats& stats, const Index& index, php::Func& func) {
  auto const isPM = func.unit->pseudomain.get() == &func;
  if (!func.cls && !isPM) {
    ++stats.totalFunctions;
    if (func.attrs & AttrPersistent) {
      ++stats.persistentFunctions;
    }
    if (func.attrs & AttrUnique) {
      ++stats.uniqueFunctions;
    }
  }

  auto const ty = index.lookup_return_type_raw(&func);
  add_type(stats.returns, ty);

  for (auto& blk : func.blocks) {
    for (auto& bc : blk->hhbcs) {
      ++stats.op_counts[static_cast<uint64_t>(bc.op)];
    }
  }

  if (!options.extendedStats || isPM) return;

  auto const ctx = Context { func.unit, &func, func.cls };
  auto const fa  = analyze_func(index, ctx);
  for (auto& blk : func.blocks) {
    auto state = fa.bdata[blk->id].stateIn;
    if (!state.initialized) continue;

    PropertiesInfo props { index, ctx, nullptr };
    auto interp = Interp { index, ctx, props, borrow(blk), state };
    for (auto& bc : blk->hhbcs) {
      auto visitor = StatsVisitor { stats, state };
      visit(bc, visitor);
      step(interp, bc);
    }
  }
}

void collect_class(Stats& stats, const Index& index, const php::Class& cls) {
  ++stats.totalClasses;
  if (cls.attrs & AttrPersistent) {
    ++stats.persistentClasses;
  }
  if (cls.attrs & AttrUnique) {
    ++stats.uniqueClasses;
  }
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

