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
#include "hphp/hhbbc/interp-state.h"

#include <string>

#include <folly/Format.h>
#include <folly/Conv.h>
#include <folly/gen/Base.h>
#include <folly/gen/String.h>

#include "hphp/util/match.h"
#include "hphp/hhbbc/analyze.h"
#include "hphp/hhbbc/interp-internal.h"
#include "hphp/hhbbc/context.h"

namespace HPHP { namespace HHBBC {

//////////////////////////////////////////////////////////////////////

namespace {

template<class JoinOp>
bool merge_into(Iter& dst, const Iter& src, JoinOp join) {
  auto const mergeCounts = [](IterTypes::Count c1, IterTypes::Count c2) {
    if (c1 == c2) return c1;
    if (c1 == IterTypes::Count::Any) return c1;
    if (c2 == IterTypes::Count::Any) return c2;
    auto const nonEmpty = [](IterTypes::Count c) {
      if (c == IterTypes::Count::Empty || c == IterTypes::Count::ZeroOrOne) {
        return IterTypes::Count::Any;
      }
      return IterTypes::Count::NonEmpty;
    };
    if (c1 == IterTypes::Count::NonEmpty) return nonEmpty(c2);
    if (c2 == IterTypes::Count::NonEmpty) return nonEmpty(c1);
    return IterTypes::Count::ZeroOrOne;
  };

  return match<bool>(
    dst,
    [&] (DeadIter) {
      match<void>(
        src,
        [] (DeadIter) {},
        [] (const LiveIter&) {
          always_assert(false && "merging dead iter with live iter");
        }
      );
      return false;
    },
    [&] (LiveIter& diter) {
      return match<bool>(
        src,
        [&] (DeadIter) {
          always_assert(false && "merging live iter with dead iter");
          return false;
        },
        [&] (const LiveIter& siter) {
          auto key = join(diter.types.key, siter.types.key);
          auto value = join(diter.types.value, siter.types.value);
          auto const count = mergeCounts(diter.types.count, siter.types.count);
          auto const throws1 =
            diter.types.mayThrowOnInit || siter.types.mayThrowOnInit;
          auto const throws2 =
            diter.types.mayThrowOnNext || siter.types.mayThrowOnNext;
          auto const baseLocal = (diter.baseLocal != siter.baseLocal)
            ? NoLocalId
            : diter.baseLocal;
          auto const keyLocal = (diter.keyLocal != siter.keyLocal)
            ? NoLocalId
            : diter.keyLocal;
          auto const initBlock = (diter.initBlock != siter.initBlock)
            ? NoBlockId
            : diter.initBlock;
          auto const changed =
            !equivalently_refined(key, diter.types.key) ||
            !equivalently_refined(value, diter.types.value) ||
            count != diter.types.count ||
            throws1 != diter.types.mayThrowOnInit ||
            throws2 != diter.types.mayThrowOnNext ||
            keyLocal != diter.keyLocal ||
            baseLocal != diter.baseLocal ||
            initBlock != diter.initBlock;
          diter.types =
            IterTypes {
              std::move(key),
              std::move(value),
              count,
              throws1,
              throws2
            };
          diter.baseLocal = baseLocal;
          diter.keyLocal = keyLocal;
          diter.initBlock = initBlock;
          return changed;
        }
      );
    }
  );
}

}

//////////////////////////////////////////////////////////////////////

std::string show(const php::Func& f, const Iter& iter) {
  return match<std::string>(
    iter,
    [&] (DeadIter) { return "dead"; },
    [&] (const LiveIter& ti) {
      auto str = folly::sformat(
        "{}, {}",
        show(ti.types.key),
        show(ti.types.value)
      );
      if (ti.initBlock != NoBlockId) {
        folly::format(&str, " (init=blk:{})", ti.initBlock);
      }
      if (ti.baseLocal != NoLocalId) {
        folly::format(&str, " (base={})", local_string(f, ti.baseLocal));
      }
      if (ti.keyLocal != NoLocalId) {
        folly::format(&str, " (key={})", local_string(f, ti.keyLocal));
      }
      return str;
    }
  );
}

//////////////////////////////////////////////////////////////////////

CollectedInfo::CollectedInfo(const Index& index,
                             Context ctx,
                             ClassAnalysis* cls,
                             PublicSPropIndexer* publicStatics,
                             CollectionOpts opts,
                             const FuncAnalysis* fa)
    : props{index, ctx, cls}
    , publicStatics{publicStatics}
    , opts{fa ? opts | CollectionOpts::Optimizing : opts}
{
  if (fa) {
    localStaticTypes = fa->localStaticTypes;
    unfoldableFuncs = fa->unfoldableFuncs;
  }
}

//////////////////////////////////////////////////////////////////////

bool operator==(const ActRec& a, const ActRec& b) {
  auto const fsame =
    a.func.hasValue() != b.func.hasValue() ? false :
    a.func.hasValue() ? a.func->same(*b.func) :
    true;
  auto const fsame2 =
    a.fallbackFunc.hasValue() != b.fallbackFunc.hasValue() ? false :
    a.fallbackFunc.hasValue() ? a.fallbackFunc->same(*b.fallbackFunc) :
    true;
  return a.kind == b.kind && fsame && fsame2 &&
         equivalently_refined(a.context, b.context);
}
bool operator!=(const ActRec& a, const ActRec& b) { return !(a == b); }

State without_stacks(const State& src) {
  auto ret          = State{};
  ret.initialized   = src.initialized;
  ret.thisAvailable = src.thisAvailable;
  ret.thisLocToKill = src.thisLocToKill;

  if (UNLIKELY(src.locals.size() > (1LL << 50))) {
    // gcc 4.9 has a bug where it will spit out a warning:
    //
    // > In function 'HPHP::HHBBC::State HPHP::HHBBC::without_stacks':
    // > cc1plus: error: iteration 461168601842738791u invokes undefined
    // >   behavior [-Werror=aggressive-loop-optimizations]
    // > include/c++/4.9.x/bits/stl_algobase.h:334:4: note: containing loop
    //
    // The warning is a bug because it computes the number of
    // iterations by subtracting two pointers; and the result *cannot*
    // exceed 461168601842738790.  (its also a bug because it
    // shouldn't generate such warnings in its own headers).
    //
    // in any case, this disables it, and generates no code in O3 builds
    not_reached();
  }

  ret.locals        = src.locals;
  ret.clsRefSlots   = src.clsRefSlots;
  ret.iters         = src.iters;
  return ret;
}

//////////////////////////////////////////////////////////////////////

PropertiesInfo::PropertiesInfo(const Index& index,
                               Context ctx,
                               ClassAnalysis* cls)
  : m_cls(cls)
{
  if (m_cls == nullptr && ctx.cls != nullptr) {
    m_privateProperties = index.lookup_private_props(ctx.cls);
    m_privateStatics    = index.lookup_private_statics(ctx.cls);
  }
}

PropState& PropertiesInfo::privateProperties() {
  if (m_cls != nullptr) {
    return m_cls->privateProperties;
  }
  return m_privateProperties;
}

PropState& PropertiesInfo::privateStatics() {
  if (m_cls != nullptr) {
    return m_cls->privateStatics;
  }
  return m_privateStatics;
}

const PropState& PropertiesInfo::privateProperties() const {
  return const_cast<PropertiesInfo*>(this)->privateProperties();
}

const PropState& PropertiesInfo::privateStatics() const {
  return const_cast<PropertiesInfo*>(this)->privateStatics();
}

void PropertiesInfo::setBadPropInitialValues() {
  if (m_cls) m_cls->badPropInitialValues = true;
}

//////////////////////////////////////////////////////////////////////

void merge_closure_use_vars_into(ClosureUseVarMap& dst,
                                 php::Class* clo,
                                 std::vector<Type> types) {
  auto& current = dst[clo];
  if (current.empty()) {
    current = std::move(types);
    return;
  }

  assert(types.size() == current.size());
  for (auto i = uint32_t{0}; i < current.size(); ++i) {
    current[i] |= std::move(types[i]);
  }
}

void widen_props(PropState& props) {
  for (auto& prop : props) {
    prop.second = widen_type(std::move(prop.second));
  }
}

bool merge_into(ActRec& dst, const ActRec& src) {
  if (dst != src) {
    if (dst.kind != src.kind) {
      dst = ActRec { FPIKind::Unknown, TTop };
    } else {
      dst = ActRec { src.kind, union_of(dst.context, src.context) };
    }
    return true;
  }
  if (dst.foldable != src.foldable) {
    dst.foldable = false;
    return true;
  }
  return false;
}

template<class JoinOp>
bool merge_impl(State& dst, const State& src, JoinOp join) {
  if (!dst.initialized) {
    dst = src;
    return true;
  }

  assert(src.initialized);
  assert(dst.locals.size() == src.locals.size());
  assert(dst.iters.size() == src.iters.size());
  assert(dst.clsRefSlots.size() == src.clsRefSlots.size());
  assert(dst.stack.size() == src.stack.size());
  assert(dst.fpiStack.size() == src.fpiStack.size());

  if (src.unreachable) {
    // If we're coming from unreachable code and the dst is already
    // initialized, it doesn't change the dst (whether it is reachable or not).
    return false;
  }
  if (dst.unreachable) {
    // If we're going to code currently believed to be unreachable, take the
    // src state, and consider the dest state changed only if the source state
    // was reachable.
    dst = src;
    return !src.unreachable;
  }

  auto changed = false;

  auto const available = dst.thisAvailable && src.thisAvailable;
  if (available != dst.thisAvailable) {
    changed = true;
    dst.thisAvailable = available;
  }

  if (dst.thisLocToKill != src.thisLocToKill) {
    if (dst.thisLocToKill != NoLocalId) {
      dst.thisLocToKill = NoLocalId;
      changed = true;
    }
  }

  for (auto i = size_t{0}; i < dst.stack.size(); ++i) {
    auto newT = join(dst.stack[i].type, src.stack[i].type);
    if (!equivalently_refined(dst.stack[i].type, newT)) {
      changed = true;
      dst.stack[i].type = std::move(newT);
    }
    if (dst.stack[i].equivLoc != src.stack[i].equivLoc) {
      changed = true;
      dst.stack[i].equivLoc = NoLocalId;
    }
  }

  for (auto i = size_t{0}; i < dst.locals.size(); ++i) {
    auto newT = join(dst.locals[i], src.locals[i]);
    if (!equivalently_refined(dst.locals[i], newT)) {
      changed = true;
      dst.locals[i] = std::move(newT);
    }
  }

  for (auto i = size_t{0}; i < dst.clsRefSlots.size(); ++i) {
    auto newT = join(dst.clsRefSlots[i], src.clsRefSlots[i]);
    assert(newT.subtypeOf(BCls));
    if (!equivalently_refined(dst.clsRefSlots[i], newT)) {
      changed = true;
      dst.clsRefSlots[i] = std::move(newT);
    }
  }

  for (auto i = size_t{0}; i < dst.iters.size(); ++i) {
    if (merge_into(dst.iters[i], src.iters[i], join)) {
      changed = true;
    }
  }

  for (auto i = size_t{0}; i < dst.fpiStack.size(); ++i) {
    if (merge_into(dst.fpiStack[i], src.fpiStack[i])) {
      changed = true;
    }
  }

  if (src.equivLocals.size() < dst.equivLocals.size()) {
    for (auto i = src.equivLocals.size(); i < dst.equivLocals.size(); ++i) {
      if (dst.equivLocals[i] != NoLocalId) {
        killLocEquiv(dst, i);
        changed = true;
      }
    }
    dst.equivLocals.resize(src.equivLocals.size());
  }

  auto processed = uint64_t{0};
  for (auto i = LocalId{0}; i < dst.equivLocals.size(); ++i) {
    if (i < sizeof(uint64_t) * CHAR_BIT && (processed >> i) & 1) continue;
    auto dstLoc = dst.equivLocals[i];
    if (dstLoc == NoLocalId) continue;
    auto srcLoc = i < src.equivLocals.size() ? src.equivLocals[i] : NoLocalId;
    if (srcLoc != dstLoc) {
      if (srcLoc != NoLocalId &&
          dst.equivLocals.size() < sizeof(uint64_t) * CHAR_BIT) {

        auto computeSet = [&] (const State& s, LocalId start) {
          auto result = uint64_t{0};
          auto l = start;
          do {
            result |= uint64_t{1} << l;
            l = s.equivLocals[l];
          } while (l != start);
          return result;
        };

        auto dstSet = computeSet(dst, i);
        auto srcSet = computeSet(src, i);

        auto newSet = dstSet & srcSet;
        if (!(newSet & (newSet - 1))) {
          newSet = 0;
        }
        auto killSet = dstSet - newSet;
        if (killSet) {
          auto l = i;
          do {
            processed |= uint64_t{1} << l;
            auto const next = dst.equivLocals[l];
            if ((killSet >> l) & 1) {
              killLocEquiv(dst, l);
            }
            l = next;
          } while (l != i && l != NoLocalId);
          assert(l == i || killSet == dstSet);
          changed = true;
        }
      } else {
        killLocEquiv(dst, i);
        changed = true;
      }
    }
  }

  auto const sz = std::max(dst.localStaticBindings.size(),
                           src.localStaticBindings.size());
  if (sz) {
    CompactVector<LocalStaticBinding> lsb;
    for (auto i = size_t{0}; i < sz; i++) {
      auto b1 = i < dst.localStaticBindings.size() ?
        dst.localStaticBindings[i] : LocalStaticBinding::None;
      auto b2 = i < src.localStaticBindings.size() ?
        src.localStaticBindings[i] : LocalStaticBinding::None;

      if (b1 != LocalStaticBinding::None || b2 != LocalStaticBinding::None) {
        lsb.resize(i + 1);
        lsb[i] = b1 == b2 ? b1 : LocalStaticBinding::Maybe;
        changed |= lsb[i] != b1;
      }
    }
    dst.localStaticBindings = std::move(lsb);
  }

  return changed;
}

bool merge_into(State& dst, const State& src) {
  return merge_impl(dst, src, union_of);
}

bool widen_into(State& dst, const State& src) {
  return merge_impl(dst, src, widening_union);
}

//////////////////////////////////////////////////////////////////////

static std::string fpiKindStr(FPIKind k) {
  switch (k) {
  case FPIKind::Unknown:     return "unk";
  case FPIKind::CallableArr: return "arr";
  case FPIKind::Func:        return "func";
  case FPIKind::Ctor:        return "ctor";
  case FPIKind::ObjMeth:     return "objm";
  case FPIKind::ObjMethNS:   return "objm?";
  case FPIKind::ClsMeth:     return "clsm";
  case FPIKind::ObjInvoke:   return "invoke";
  case FPIKind::Builtin:     return "builtin";
  }
  not_reached();
}

std::string show(const ActRec& a) {
  return folly::to<std::string>(
    "ActRec { ",
    fpiKindStr(a.kind),
    a.cls || a.func ? ": " : "",
    a.cls ? show(*a.cls) : "",
    a.cls && a.func ? "::" : "",
    a.func ? show(*a.func) : "",
    a.fallbackFunc ? show(*a.fallbackFunc) : "",
    a.foldable ? " (foldable)" : "",
    " ", show(a.context),
    " }"
  );
}

std::string show(const php::Func& f, const Base& b) {
  auto const locName = [&]{
    return b.locName ? folly::sformat("\"{}\"", b.locName) : "-";
  };
  auto const local = [&]{
    return b.locLocal != NoLocalId ? local_string(f, b.locLocal) : "-";
  };

  switch (b.loc) {
    case BaseLoc::None:
      return "none";
    case BaseLoc::Elem:
      return folly::to<std::string>(
        "elem{", show(b.type), ",", show(b.locTy), "}"
      );
    case BaseLoc::Prop:
      return folly::to<std::string>(
        "prop{", show(b.type), ",", show(b.locTy), ",", locName(), "}"
      );
    case BaseLoc::StaticProp:
      return folly::to<std::string>(
        "sprop{", show(b.type), ",", show(b.locTy), ",", locName(), "}"
      );
    case BaseLoc::Local:
      return folly::to<std::string>(
        "local{", show(b.type), ",", locName(), ",", local(), "}"
      );
    case BaseLoc::This:
      return folly::to<std::string>("this{", show(b.type), "}");
    case BaseLoc::Stack:
      return folly::to<std::string>(
        "stack{", show(b.type), ",", b.locSlot, "}"
      );
    case BaseLoc::Global:
      return folly::to<std::string>("global{", show(b.type), "}");
  }
  not_reached();
}

std::string show(const php::Func& f, const State::MInstrState& s) {
  if (s.arrayChain.empty()) return show(f, s.base);
  return folly::sformat(
    "{} ({})",
    show(f, s.base),
    [&]{
      using namespace folly::gen;
      return from(s.arrayChain)
        | map([&] (const State::MInstrState::ArrayChainEnt& e) {
            return folly::sformat("<{},{}>", show(e.key), show(e.base));
          })
        | unsplit<std::string>(" -> ");
    }()
  );
}

std::string state_string(const php::Func& f, const State& st,
                         const CollectedInfo& collect) {
  std::string ret;

  if (!st.initialized) {
    ret = "state: uninitialized\n";
    return ret;
  }

  folly::format(&ret, "state{}:\n", st.unreachable ? " (unreachable)" : "");
  if (f.cls) {
    folly::format(&ret, "thisAvailable({})\n", st.thisAvailable);
  }

  if (st.thisLocToKill != NoLocalId) {
    folly::format(&ret, "thisLocToKill({})\n", st.thisLocToKill);
  }

  for (auto i = size_t{0}; i < st.locals.size(); ++i) {
    auto staticLocal = [&] () -> std::string {
      if (i >= st.localStaticBindings.size() ||
          st.localStaticBindings[i] == LocalStaticBinding::None) {
        return "";
      }

      if (i >= collect.localStaticTypes.size()) {
        return " (!!! unknown static !!!)";
      }

      return folly::sformat(
        " ({}static: {})",
        st.localStaticBindings[i] == LocalStaticBinding::Maybe ? "maybe-" : "",
        show(collect.localStaticTypes[i]));
    };
    folly::format(&ret, "{: <8} :: {}{}\n",
                  local_string(f, i),
                  show(st.locals[i]),
                  staticLocal());
  }

  for (auto i = size_t{0}; i < st.iters.size(); ++i) {
    folly::format(&ret, "iter {: <2}  :: {}\n", i, show(f, st.iters[i]));
  }

  for (auto i = size_t{0}; i < st.clsRefSlots.size(); ++i) {
    folly::format(&ret, "class-ref slot {: <2}   :: {}\n",
                  i, show(st.clsRefSlots[i]));
  }

  for (auto i = size_t{0}; i < st.stack.size(); ++i) {
    folly::format(&ret, "stk[{:02}] :: {} [{}]\n",
                  i,
                  show(st.stack[i].type),
                  st.stack[i].equivLoc == NoLocalId ? "" :
                  st.stack[i].equivLoc == StackDupId ? "Dup" :
                  local_string(f, st.stack[i].equivLoc));
  }

  for (auto i = size_t{0}; i < st.equivLocals.size(); ++i) {
    if (st.equivLocals[i] == NoLocalId) continue;
    folly::format(&ret, "{: <8} ==", local_string(f, i));
    for (auto j = st.equivLocals[i]; j != i; j = st.equivLocals[j]) {
      ret += " ";
      ret += local_string(f, j);
    }
    ret += "\n";
  }

  if (st.mInstrState.base.loc != BaseLoc::None) {
    folly::format(&ret, "mInstrState   :: {}\n", show(f, st.mInstrState));
  }

  if (st.mInstrStateDefine) {
    folly::format(
      &ret,
      "mInstrState (define)   :: {}\n",
      show(f, *st.mInstrStateDefine)
    );
  }

  return ret;
}

std::string property_state_string(const PropertiesInfo& props) {
  std::string ret;

  for (auto& kv : props.privateProperties()) {
    folly::format(&ret, "$this->{: <14} :: {}\n", kv.first, show(kv.second));
  }
  for (auto& kv : props.privateStatics()) {
    folly::format(&ret, "self::${: <14} :: {}\n", kv.first, show(kv.second));
  }

  return ret;
}

//////////////////////////////////////////////////////////////////////

}}
