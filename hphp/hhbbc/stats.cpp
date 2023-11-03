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
#include <folly/portability/Stdlib.h>

#include "hphp/runtime/vm/hhbc.h"

#include "hphp/util/trace.h"

#include "hphp/hhbbc/analyze.h"
#include "hphp/hhbbc/func-util.h"
#include "hphp/hhbbc/index.h"
#include "hphp/hhbbc/interp-internal.h"
#include "hphp/hhbbc/interp.h"
#include "hphp/hhbbc/context.h"
#include "hphp/hhbbc/misc.h"
#include "hphp/hhbbc/parallel.h"
#include "hphp/hhbbc/representation.h"

namespace HPHP::HHBBC {

//////////////////////////////////////////////////////////////////////

namespace {

TRACE_SET_MOD(hhbbc_stats);

//////////////////////////////////////////////////////////////////////

#define SPEC_TYPES(X)                                                \
  X(wh, "wait handle", is_specialized_wait_handle)                   \
  X(obj_sub, "sub obj", is_sub_obj)                                  \
  X(obj_exact, "exact obj", is_exact_obj)                            \
  X(obj_isect, "isect obj", is_isect_obj)                            \
  X(cls_sub, "sub class", is_sub_cls)                                \
  X(cls_exact, "exact class", is_exact_cls)                          \
  X(cls_isect, "isect class", is_isect_cls)                          \
  X(arr_val, "array value", is_specialized_array_like_arrval)        \
  X(arr_packedn, "array packedn", is_specialized_array_like_packedn) \
  X(arr_packed, "array packed", is_specialized_array_like_packed)    \
  X(arr_mapn, "array mapn", is_specialized_array_like_mapn)          \
  X(arr_map, "array map", is_specialized_array_like_map)             \
  X(str, "string", is_specialized_string)                            \
  X(lazy_cls, "lazy class", is_specialized_lazycls)                  \
  X(int, "int", is_specialized_int)                                  \
  X(dbl, "double", is_specialized_double)                            \

struct TypeStat {
#define X(y, ...)                               \
  std::atomic<uint64_t> sub_##y{};              \
  std::atomic<uint64_t> eq_##y{};
  HHBBC_TYPE_PREDEFINED(X)
#undef X

#define X(y, ...) std::atomic<uint64_t> spec_##y;
  SPEC_TYPES(X)
#undef X
};

#define TAG(x,...) 1 +
constexpr uint32_t kNumRATTags = REPO_AUTH_TYPE_TAGS(TAG) 0 ;
#undef TAG

}

struct Stats {
  std::array<std::atomic<uint64_t>,Op_count> op_counts{};
  std::array<std::atomic<uint64_t>,kNumRATTags> ratL_tags{};
  std::array<std::atomic<uint64_t>,kNumRATTags> ratStk_tags{};
  std::atomic<uint64_t> ratL_specialized_array{};
  std::atomic<uint64_t> ratStk_specialized_array{};
  std::atomic<uint64_t> totalClasses{};
  std::atomic<uint64_t> totalFunctions{};
  std::atomic<uint64_t> totalMethods{};
  std::atomic<uint64_t> effectfulFuncs{};
  std::atomic<uint64_t> effectFreeFuncs{};
  std::atomic<uint64_t> persistentSPropsPub{};
  std::atomic<uint64_t> persistentSPropsProt{};
  std::atomic<uint64_t> persistentSPropsPriv{};
  std::atomic<uint64_t> totalSProps{};
  TypeStat returns;
  TypeStat privateProps;
  TypeStat publicStatics;
  TypeStat privateStatics;
  TypeStat iterInitBase;
};

namespace {

void type_stat_string(std::string& ret,
                      const std::string& prefix,
                      const TypeStat& st) {
#define X(y, ...)                                               \
  if (st.eq_##y.load() > 0) {                                   \
    folly::format(&ret, "  {: <32}  {: >12}\n",                 \
                  folly::sformat("{}_=_{}", prefix, #y ":"),    \
                  st.eq_##y.load());                            \
  }                                                             \
  if (st.sub_##y.load() > 0) {                                  \
    folly::format(&ret, "  {: <32}  {: >12}\n",                 \
                  folly::sformat("{}_<_{}", prefix, #y ":"),    \
                  st.sub_##y.load());                           \
  }
  HHBBC_TYPE_PREDEFINED(X)
#undef X

  ret += "\n";

#define X(y, z, ...)                                                  \
  if (st.spec_##y.load() > 0) {                                       \
    folly::format(&ret, "  {: <32}  {: >12}\n",                       \
                  folly::sformat("{}_specialized {}", prefix, z ":"), \
                  st.spec_##y.load());                                \
  }
  SPEC_TYPES(X)
#undef X

  ret += "\n";
}

std::string show(const Stats& stats) {
  auto ret = std::string{};

  for (auto i = uint32_t{}; i < stats.op_counts.size(); ++i) {
    if (stats.op_counts[i].load() == 0) continue;
    folly::format(
      &ret,
      "  {: >30}:  {: >15}\n",
      opcodeToName(static_cast<Op>(i)),
      stats.op_counts[i].load()
    );
  }
  ret += "\n";

  type_stat_string(ret, "ret",            stats.returns);
  type_stat_string(ret, "priv_prop",      stats.privateProps);
  type_stat_string(ret, "pub_static",     stats.publicStatics);
  type_stat_string(ret, "priv_static",    stats.privateStatics);
  type_stat_string(ret, "iterInit_base",  stats.iterInitBase);

  folly::format(
    &ret,
    "            total_methods:  {: >12}\n"
    "              total_funcs:  {: >12}\n"
    "            total_classes:  {: >12}\n"
    "\n"
    "          effectful_funcs:  {: >12}\n"
    "        effect_free_funcs:  {: >12}\n"
    "\n"
    "             total_sprops:  {: >12}\n"
    "    persistent_sprops_pub:  {: >12}\n"
    "   persistent_sprops_prot:  {: >12}\n"
    "   persistent_sprops_priv:  {: >12}\n",
    stats.totalMethods.load(),
    stats.totalFunctions.load(),
    stats.totalClasses.load(),
    stats.effectfulFuncs.load(),
    stats.effectFreeFuncs.load(),
    stats.totalSProps.load(),
    stats.persistentSPropsPub.load(),
    stats.persistentSPropsProt.load(),
    stats.persistentSPropsPriv.load()
  );

  ret += "\n";
  using T = RepoAuthType::Tag;
  using U = std::underlying_type<T>::type;
#define TAG(x, ...)                                                     \
  if (stats.ratL_tags[static_cast<U>(T::x)].load() > 0 ||               \
      stats.ratStk_tags[static_cast<U>(T::x)].load() > 0) {             \
    folly::format(&ret,                                                 \
                  "  {: >28}:  {: >12}\n"                               \
                  "  {: >28}:  {: >12}\n",                              \
                  "RATL_" #x,                                           \
                  stats.ratL_tags[static_cast<U>(T::x)].load(),         \
                  "RATStk_" #x,                                         \
                  stats.ratStk_tags[static_cast<U>(T::x)].load());      \
  }
  REPO_AUTH_TYPE_TAGS(TAG)
#undef TAG

  if (stats.ratL_specialized_array.load() > 0 ||
      stats.ratStk_specialized_array.load() > 0) {
    folly::format(&ret,
                  "  {: >28}:  {: >12}\n"
                  "  {: >28}:  {: >12}\n",
                  "RATL_Arr_Special",
                  stats.ratL_specialized_array.load(),
                  "RATStk_Arr_Special",
                  stats.ratStk_specialized_array.load());
  }

  return ret;
}

//////////////////////////////////////////////////////////////////////

bool is_sub_obj(const Type& t) {
  return
    is_specialized_obj(t) &&
    !is_specialized_wait_handle(t) &&
    dobj_of(t).isSub();
}
bool is_exact_obj(const Type& t) {
  return
    is_specialized_obj(t) &&
    !is_specialized_wait_handle(t) &&
    dobj_of(t).isExact();
}
bool is_isect_obj(const Type& t) {
  return
    is_specialized_obj(t) &&
    !is_specialized_wait_handle(t) &&
    dobj_of(t).isIsect();
}

bool is_sub_cls(const Type& t) {
  return is_specialized_cls(t) && dcls_of(t).isSub();
}
bool is_exact_cls(const Type& t) {
  return is_specialized_cls(t) && dcls_of(t).isExact();
}
bool is_isect_cls(const Type& t) {
  return is_specialized_cls(t) && dcls_of(t).isIsect();
}

void add_type(TypeStat& stat, const Type& t) {
#define X(y, ...)                                     \
  if (t.subtypeOf(B##y)) {                            \
    if (B##y == BBottom || !t.is(BBottom)) {          \
      if (t.strictSubtypeOf(B##y)) ++stat.sub_##y;    \
      else ++stat.eq_##y;                             \
    }                                                 \
  }
  HHBBC_TYPE_PREDEFINED(X)
#undef X

#define X(a, b, c) if (c(t)) ++stat.spec_##a;
  SPEC_TYPES(X)
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

template <class OpCode>
bool in(StatsSS& /*env*/, const OpCode&) {
  return false;
}

bool in(StatsSS& env, const bc::IterInit& /*op*/) {
  add_type(env.stats.iterInitBase, topC(env));
  return false;
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
  assertx(tagInt < stats.ratL_tags.size());
  if (bc.op == Op::AssertRATL) {
    ++stats.ratL_tags[tagInt];
  } else {
    ++stats.ratStk_tags[tagInt];
  }

  if (rat.array()) {
    if (bc.op == Op::AssertRATL) {
      ++stats.ratL_specialized_array;
    } else {
      ++stats.ratStk_specialized_array;
    }
  }
}

void collect_func(Stats& stats, const Index& index, const php::Func& func) {
  if (!func.cls) ++stats.totalFunctions;

  auto const [ty, effectFree] = index.lookup_return_type_raw(&func).first;
  if (effectFree) {
    ++stats.effectFreeFuncs;
  } else {
    ++stats.effectfulFuncs;
  }

  add_type(stats.returns, ty);

  auto const cf = php::WideFunc::cns(&func);
  for (auto const bid : cf.blockRange()) {
    auto const blk = cf.blocks()[bid].get();
    if (blk->dead) continue;
    for (auto& bc : blk->hhbcs) {
      collect_simple(stats, bc);
    }
  }

  if (!options.extendedStats) return;

  auto const ctx = AnalysisContext { func.unit, cf, func.cls };
  IndexAdaptor adaptor{ index };
  auto const fa  = analyze_func(adaptor, ctx, CollectionOpts{});
  {
    Trace::Bump bumper{Trace::hhbbc, kStatsBump};
    for (auto const bid : cf.blockRange()) {
      auto const blk = cf.blocks()[bid].get();
      auto state = fa.bdata[bid].stateIn;
      if (!state.initialized) continue;

      CollectedInfo collect {
        adaptor, ctx, nullptr, CollectionOpts {}, nullptr, &fa
      };
      Interp interp { adaptor, ctx, collect, bid, blk, state };
      for (auto& bc : blk->hhbcs) {
        auto noop    = [] (BlockId, const State*) {};
        ISS env { interp, noop };
        StatsSS sss { env, stats };
        dispatch(sss, bc);
        if (state.unreachable) break;
      }
    }
  }
}

void collect_class(Stats& stats, const Index& index, const php::Class& cls) {
  ++stats.totalClasses;
  stats.totalMethods += cls.methods.size();

  for (auto const& kv : index.lookup_private_props(&cls)) {
    add_type(stats.privateProps, kv.second.ty);
  }
  for (auto const& kv : index.lookup_public_statics(&cls)) {
    add_type(stats.publicStatics, kv.second.ty);
  }
  for (auto const& kv : index.lookup_private_statics(&cls)) {
    add_type(stats.privateStatics, kv.second.ty);
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

//////////////////////////////////////////////////////////////////////

}

//////////////////////////////////////////////////////////////////////

StatsHolder::StatsHolder() {
  if (!Trace::moduleEnabledRelease(Trace::hhbbc_stats, 1)) return;
  stats = new Stats{};
}

StatsHolder::~StatsHolder() {
  delete stats;
}

StatsHolder allocate_stats() {
  return StatsHolder();
}

void collect_stats(const StatsHolder& stats,
                   const Index& index,
                   const php::Unit& unit) {
  if (!stats) return;
  index.for_each_unit_class(
    unit,
    [&] (const php::Class& c) {
      collect_class(*stats.stats, index, c);
      for (auto const& m : c.methods) {
        if (!m) continue;
        collect_func(*stats.stats, index, *m);
      }
    }
  );
  index.for_each_unit_func(
    unit,
    [&] (const php::Func& f) { collect_func(*stats.stats, index, f); }
  );
}

void print_stats(const StatsHolder& stats) {
  if (!stats) return;

  auto const str = show(*stats.stats);
  Trace::ftraceRelease("{}", str);

  if (!Trace::moduleEnabledRelease(Trace::hhbbc_stats, 2)) return;

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

}
