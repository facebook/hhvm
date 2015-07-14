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
#include <tuple>
#include <type_traits>

#include <boost/variant.hpp>

#include <tbb/concurrent_hash_map.h>

#include <folly/Conv.h>
#include <folly/String.h>
#include <folly/Format.h>
#include <folly/ScopeGuard.h>

#include "hphp/util/trace.h"
#include "hphp/runtime/vm/hhbc.h"

#include "hphp/hhbbc/misc.h"
#include "hphp/hhbbc/representation.h"
#include "hphp/hhbbc/parallel.h"
#include "hphp/hhbbc/index.h"
#include "hphp/hhbbc/interp-internal.h"
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

struct ISameCmp {
  bool equal(SString s1, SString s2) const {
    return s1->isame(s2);
  }

  size_t hash(SString s) const {
    return s->hash();
  }
};

/*
 * Information about builtins usage.
 *
 * The tuple contains the known return type for the builtin, the total
 * number of calls seen and the total number of calls that could be
 * reduced.  A builtin call is considered reducible if its output is a
 * constant and all its inputs are constants. That is not a guaranteed
 * condition but it gives us an idea of what's possible.
 */
using BuiltinInfo = tbb::concurrent_hash_map<
  SString,
  std::tuple<Type,uint64_t,uint64_t>,
  ISameCmp
>;

struct Builtins {
  std::atomic<uint64_t> totalBuiltins;
  std::atomic<uint64_t> reducibleBuiltins;
  BuiltinInfo builtinsInfo;
};

#define TAG(x) 1 +
constexpr uint32_t kNumRATTags = REPO_AUTH_TYPE_TAGS 0 ;
#undef TAG

struct Stats {
  std::array<std::atomic<uint64_t>,Op_count> op_counts;
  std::array<std::atomic<uint64_t>,kNumRATTags> ratL_tags;
  std::array<std::atomic<uint64_t>,kNumRATTags> ratStk_tags;
  std::atomic<uint64_t> ratL_specialized_array;
  std::atomic<uint64_t> ratStk_specialized_array;
  std::atomic<uint64_t> persistentClasses;
  std::atomic<uint64_t> persistentFunctions;
  std::atomic<uint64_t> uniqueClasses;
  std::atomic<uint64_t> uniqueFunctions;
  std::atomic<uint64_t> totalClasses;
  std::atomic<uint64_t> totalFunctions;
  std::atomic<uint64_t> totalMethods;
  std::atomic<uint64_t> persistentSPropsPub;
  std::atomic<uint64_t> persistentSPropsProt;
  std::atomic<uint64_t> persistentSPropsPriv;
  std::atomic<uint64_t> totalSProps;
  TypeStat returns;
  TypeStat privateProps;
  TypeStat privateStatics;
  TypeStat cgetmBase;
  TypeStat iterInitBase;
  TypeStat iterInitKBase;
  Builtins builtins;
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

std::string show(const Builtins& builtins) {
  auto ret = std::string{};

  if (builtins.builtinsInfo.begin() != builtins.builtinsInfo.end()) {
    ret += folly::format("Total number of builtin calls: {: >15}\n",
                         builtins.totalBuiltins.load()).str();
    ret += folly::format("Possible reducible builtins: {: >15}\n",
                         builtins.reducibleBuiltins.load()).str();

    ret += "Builtins Info:\n";
    for (auto it = builtins.builtinsInfo.begin();
         it != builtins.builtinsInfo.end(); ++it) {
      ret += folly::format(
        "  {: >30} [tot:{: >8}, red:{: >8}]\t\ttype: {}\n",
        it->first,
        std::get<1>(it->second),
        std::get<2>(it->second),
        show(std::get<0>(it->second))
      ).str();
    }
    ret += "\n";
  }
  return ret;
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

  type_stat_string(ret, "ret",            stats.returns);
  type_stat_string(ret, "priv_prop",      stats.privateProps);
  type_stat_string(ret, "priv_static",    stats.privateStatics);
  type_stat_string(ret, "cgetm_base",     stats.cgetmBase);
  type_stat_string(ret, "iterInit_base",  stats.iterInitBase);
  type_stat_string(ret, "iterInitK_base", stats.iterInitKBase);

  folly::format(
    &ret,
    "       total_methods:  {: >8}\n"
    "         total_funcs:  {: >8}\n"
    "        unique_funcs:  {: >8}\n"
    "    persistent_funcs:  {: >8}\n"
    "       total_classes:  {: >8}\n"
    "      unique_classes:  {: >8}\n"
    "  persistent_classes:  {: >8}\n"
    "\n"
    "        total_sprops:      {: >8}\n"
    "   persistent_sprops_pub:  {: >8}\n"
    "   persistent_sprops_prot: {: >8}\n"
    "   persistent_sprops_priv: {: >8}\n",
    stats.totalMethods.load(),
    stats.totalFunctions.load(),
    stats.uniqueFunctions.load(),
    stats.persistentFunctions.load(),
    stats.totalClasses.load(),
    stats.uniqueClasses.load(),
    stats.persistentClasses.load(),
    stats.totalSProps.load(),
    stats.persistentSPropsPub.load(),
    stats.persistentSPropsProt.load(),
    stats.persistentSPropsPriv.load()
  );

  ret += "\n";
  ret += show(stats.builtins);

  ret += "\n";
  using T = RepoAuthType::Tag;
  using U = std::underlying_type<T>::type;
#define TAG(x)                                                          \
  folly::format(&ret, "  {: >24}:  {: >8}\n"                            \
                      "  {: >24}:  {: >8}\n",                           \
                      "RATL_" #x,                                       \
                      stats.ratL_tags[static_cast<U>(T::x)].load(),     \
                      "RATStk_" #x,                                     \
                      stats.ratStk_tags[static_cast<U>(T::x)].load());
  REPO_AUTH_TYPE_TAGS
#undef TAG

  folly::format(&ret, "  {: >24}:  {: >8}\n"
                      "  {: >24}:  {: >8}\n",
                      "RATL_Arr_Special",
                      stats.ratL_specialized_array.load(),
                      "RATStk_Arr_Special",
                      stats.ratStk_specialized_array.load());

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

struct StatsSS : ISS {
  explicit StatsSS(ISS& env, Stats& stats)
    : ISS(env)
    , stats(stats)
  {}

  Stats& stats;
};

//////////////////////////////////////////////////////////////////////

template<class OpCode>
bool in(StatsSS& env, const OpCode&) {
  return false;
}

bool in(StatsSS& env, const bc::IterInit& op) {
  add_type(env.stats.iterInitBase, topC(env));
  return false;
}

bool in(StatsSS& env, const bc::IterInitK& op) {
  add_type(env.stats.iterInitKBase, topC(env));
  return false;
}

bool in(StatsSS& env, const bc::CGetM& op) {
  auto const pops = op.numPop();
  auto const ty = [&]() -> Type {
    switch (op.mvec.lcode) {
    case LL:
      return env.state.locals[op.mvec.locBase->id];
    case LC:
    case LR:
      return topT(env, pops - 1);
    case LH:
    case LGL:
    case LGC:
    case LNL:
    case LNC:
    case LSL:
    case LSC:
      return TTop;
    case InvalidLocationCode:
      break;
    }
    not_reached();
  }();
  add_type(env.stats.cgetmBase, ty);
  return false;
}

bool in(StatsSS& env, const bc::FCallBuiltin& op) {
  ++env.stats.builtins.totalBuiltins;

  bool reducible = op.arg1 > 0;
  for (auto i = uint32_t{0}; i < op.arg1; ++i) {
    auto t = topT(env, i);
    auto const v = tv(t);
    if (!v || v->m_type == KindOfUninit) {
      reducible = false;
      break;
    }
  }

  default_dispatch(env, op);

  auto builtin = op.str3;
  {
    BuiltinInfo::accessor acc;
    auto inserted = env.stats.builtins.builtinsInfo.insert(acc, builtin);
    if (inserted) {
      auto f = env.index.resolve_func(env.ctx, builtin);
      auto t = env.index.lookup_return_type(env.ctx, f);
      acc->second = std::make_tuple(t, 1, 0);
    } else {
      ++std::get<1>(acc->second);
      if (reducible) ++std::get<2>(acc->second);
    }
  }

  return true;
}

//////////////////////////////////////////////////////////////////////

// Run the interpreter. The "bool in(StatsSS&, const bc::XX)" functions
// can call the default dispatch on their own or return false to let
// the main loop call the default dispatch. Returning true stops
// the call to the default interpreter which implies the handler for
// that opcode must perform all the right steps wrt the state.
void dispatch(StatsSS& env, const Bytecode& op) {
#define O(opcode, ...)                                   \
  case Op::opcode:                                       \
    if (!in(env, op.opcode)) default_dispatch(env, op);  \
    return;

  switch (op.op) { OPCODES }
#undef O
  not_reached();
}

//////////////////////////////////////////////////////////////////////

// Simple stats about opcodes (that don't require full type
// information---those cases are only enabled when extendedStats is
// on).
void collect_simple(Stats& stats, const Bytecode& bc) {
  ++stats.op_counts[static_cast<uint64_t>(bc.op)];

  RepoAuthType rat;
  switch (bc.op) {
  case Op::AssertRATL:
    rat = bc.AssertRATL.rat;
    break;
  case Op::AssertRATStk:
    rat = bc.AssertRATStk.rat;
    break;
  default:
    return;
  }

  using U = std::underlying_type<RepoAuthType::Tag>::type;
  auto const tagInt = static_cast<U>(rat.tag());
  assert(tagInt < stats.ratL_tags.size());
  if (bc.op == Op::AssertRATL) {
    ++stats.ratL_tags[tagInt];
  } else {
    ++stats.ratStk_tags[tagInt];
  }

  if (rat.mayHaveArrData()) {
    if (rat.array()) {
      if (bc.op == Op::AssertRATL) {
        ++stats.ratL_specialized_array;
      } else {
        ++stats.ratStk_specialized_array;
      }
    }
  }
}

void collect_func(Stats& stats, const Index& index, php::Func& func) {
  if (!func.cls) {
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
      collect_simple(stats, bc);
    }
  }

  if (!options.extendedStats) return;

  auto const ctx = Context { func.unit, &func, func.cls };
  auto const fa  = analyze_func(index, ctx);
  {
    Trace::Bump bumper{Trace::hhbbc, kStatsBump};
    for (auto& blk : func.blocks) {
      auto state = fa.bdata[blk->id].stateIn;
      if (!state.initialized) continue;

      CollectedInfo collect { index, ctx, nullptr, nullptr };
      Interp interp { index, ctx, collect, borrow(blk), state };
      for (auto& bc : blk->hhbcs) {
        auto noop    = [] (php::Block&, const State&) {};
        auto flags   = StepFlags {};
        ISS env { interp, flags, noop };
        StatsSS sss { env, stats };
        dispatch(sss, bc);
      }
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
  stats.totalMethods += cls.methods.size();

  for (auto& kv : index.lookup_private_props(&cls)) {
    add_type(stats.privateProps, kv.second);
  }
  for (auto& kv : index.lookup_private_statics(&cls)) {
    add_type(stats.privateStatics, kv.second);
  }

  for (auto& prop : cls.properties) {
    if (prop.attrs & AttrStatic) {
      ++stats.totalSProps;
      if (prop.attrs & AttrPersistent) {
        if (prop.attrs & AttrPublic)    ++stats.persistentSPropsPub;
        if (prop.attrs & AttrProtected) ++stats.persistentSPropsProt;
        if (prop.attrs & AttrPrivate)   ++stats.persistentSPropsPriv;
      }
    }
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

  auto stats_file = options.stats_file;

  auto const file = [&] () -> std::FILE* {
    if (!stats_file.empty()) {
      return fopen(stats_file.c_str(), "w");
    }

    char fileBuf[] = "/tmp/hhbbcXXXXXX";
    auto const fd = mkstemp(fileBuf);
    stats_file = fileBuf;
    if (fd == -1) return nullptr;

    if (auto const fp = fdopen(fd, "w")) return fp;
    close(fd);
    return nullptr;
  }();

  if (file == nullptr) {
    std::cerr << "couldn't open file for stats: "
              << folly::errnoStr(errno) << '\n';
    return;
  }

  SCOPE_EXIT { fclose(file); };
  std::cout << "stats saved to " << stats_file << '\n';
  std::fwrite(str.c_str(), sizeof(char), str.size(), file);
  std::fflush(file);
}

//////////////////////////////////////////////////////////////////////

}}
