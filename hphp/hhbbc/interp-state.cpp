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

namespace HPHP::HHBBC {

//////////////////////////////////////////////////////////////////////

namespace {

const StaticString s_Throwable("Throwable");

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
          auto const baseUpdated = diter.baseUpdated || siter.baseUpdated;
          auto const baseLocal = (diter.baseLocal != siter.baseLocal)
            ? NoLocalId
            : diter.baseLocal;
          auto const keyLocal = (diter.keyLocal != siter.keyLocal)
            ? NoLocalId
            : diter.keyLocal;
          auto const initBlock = (diter.initBlock != siter.initBlock)
            ? NoBlockId
            : diter.initBlock;
          auto const baseCannotBeObject =
            diter.baseCannotBeObject && siter.baseCannotBeObject;
          auto const changed =
            !equivalently_refined(key, diter.types.key) ||
            !equivalently_refined(value, diter.types.value) ||
            count != diter.types.count ||
            throws1 != diter.types.mayThrowOnInit ||
            throws2 != diter.types.mayThrowOnNext ||
            keyLocal != diter.keyLocal ||
            baseLocal != diter.baseLocal ||
            baseUpdated != diter.baseUpdated ||
            initBlock != diter.initBlock ||
            baseCannotBeObject != diter.baseCannotBeObject;
          diter.types =
            IterTypes {
              std::move(key),
              std::move(value),
              count,
              throws1,
              throws2
            };
          diter.baseUpdated = baseUpdated;
          diter.baseLocal = baseLocal;
          diter.keyLocal = keyLocal;
          diter.initBlock = initBlock;
          diter.baseCannotBeObject = baseCannotBeObject;
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
      if (ti.baseUpdated) folly::format(&str, " (updated)");
      return str;
    }
  );
}

//////////////////////////////////////////////////////////////////////

CollectedInfo::CollectedInfo(const IIndex& index,
                             Context ctx,
                             ClassAnalysis* cls,
                             CollectionOpts opts,
                             ClsConstantWork* clsCns,
                             const FuncAnalysis* fa)
    : props{index, ctx, cls}
    , methods{ctx, cls}
    , clsCns{clsCns}
    , opts{fa ? opts | CollectionOpts::Optimizing : opts}
    , publicSPropMutations{index.using_class_dependencies() && !fa}
{
  if (fa) {
    unfoldableFuncs = fa->unfoldableFuncs;
  }
}

//////////////////////////////////////////////////////////////////////

State with_throwable_only(const IIndex& index, const State& src) {
  auto throwable = subObj(builtin_class(index, s_Throwable.get()));
  auto ret          = State{};
  ret.initialized   = src.initialized;
  ret.thisType      = src.thisType;
  ret.thisLoc       = src.thisLoc;

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
  ret.iters         = src.iters;
  ret.stack.push_elem(std::move(throwable), NoLocalId);
  return ret;
}

//////////////////////////////////////////////////////////////////////

PropertiesInfo::PropertiesInfo(const IIndex& index,
                               Context ctx,
                               ClassAnalysis* cls)
  : m_cls(cls)
  , m_func(ctx.func)
{
  if (m_cls == nullptr && ctx.cls != nullptr) {
    m_privateProperties = index.lookup_private_props(ctx.cls);
    m_privateStatics    = index.lookup_private_statics(ctx.cls);
  }
}

const PropState& PropertiesInfo::privatePropertiesRaw() const {
  if (m_cls != nullptr) {
    return m_cls->privateProperties;
  }
  return m_privateProperties;
}

const PropState& PropertiesInfo::privateStaticsRaw() const {
  if (m_cls != nullptr) {
    return m_cls->privateStatics;
  }
  return m_privateStatics;
}

const PropStateElem*
PropertiesInfo::readPrivateProp(SString name) const {
  if (m_cls != nullptr) {
    auto const it = m_cls->privateProperties.find(name);
    if (it == m_cls->privateProperties.end()) return nullptr;
    if (m_cls->work) m_cls->work->worklist.addPropDep(name, *m_func);
    return &it->second;
  }
  auto const it = m_privateProperties.find(name);
  return it == m_privateProperties.end() ? nullptr : &it->second;
}

const PropStateElem*
PropertiesInfo::readPrivateStatic(SString name) const {
  if (m_cls != nullptr) {
    auto const it = m_cls->privateStatics.find(name);
    if (it == m_cls->privateStatics.end()) return nullptr;
    if (m_cls->work) m_cls->work->worklist.addPropDep(name, *m_func);
    return &it->second;
  }
  auto const it = m_privateStatics.find(name);
  return it == m_privateStatics.end() ? nullptr : &it->second;
}

void PropertiesInfo::mergeInPrivateProp(const IIndex& index,
                                        SString name,
                                        const Type& t) {
  if (!m_cls || t.is(BBottom)) return;
  auto it = m_cls->privateProperties.find(name);
  if (it == m_cls->privateProperties.end()) return;
  if (m_cls->work) {
    m_cls->work->worklist.addPropMutateDep(name, *m_func);
    m_cls->work->propMutators[m_func].emplace(name);
  }
  it->second.everModified = true;
  auto newT = union_of(
    it->second.ty,
    adjust_type_for_prop(index, *m_cls->ctx.cls, it->second.tc, t)
  );
  if (it->second.ty.strictlyMoreRefined(newT)) {
    it->second.ty = std::move(newT);
    if (m_cls->work) {
      always_assert(!m_cls->work->noPropRefine);
      m_cls->work->worklist.scheduleForProp(name);
    }
  }
}

void PropertiesInfo::mergeInPrivateStatic(const IIndex& index,
                                          SString name,
                                          const Type& t,
                                          bool ignoreConst,
                                          bool mustBeReadOnly) {
  if (!m_cls || t.is(BBottom)) return;
  auto it = m_cls->privateStatics.find(name);
  if (it == m_cls->privateStatics.end()) return;
  if (!ignoreConst && (it->second.attrs & AttrIsConst)) return;
  if (mustBeReadOnly && !(it->second.attrs & AttrIsReadonly)) return;
  if (m_cls->work) {
    m_cls->work->worklist.addPropMutateDep(name, *m_func);
    m_cls->work->propMutators[m_func].emplace(name);
  }
  it->second.everModified = true;
  auto newT = union_of(
    it->second.ty,
    adjust_type_for_prop(index, *m_cls->ctx.cls, it->second.tc, t)
  );
  if (it->second.ty.strictlyMoreRefined(newT)) {
    it->second.ty = std::move(newT);
    if (m_cls->work) {
      always_assert(!m_cls->work->noPropRefine);
      m_cls->work->worklist.scheduleForProp(name);
    }
  }
}

void PropertiesInfo::mergeInPrivateStaticPreAdjusted(SString name,
                                                     const Type& t) {
  if (!m_cls || t.is(BBottom)) return;
  auto it = m_cls->privateStatics.find(name);
  if (it == m_cls->privateStatics.end()) return;
  if (m_cls->work) {
    m_cls->work->worklist.addPropMutateDep(name, *m_func);
    m_cls->work->propMutators[m_func].emplace(name);
  }
  it->second.everModified = true;
  auto newT = union_of(it->second.ty, t);
  if (it->second.ty.strictlyMoreRefined(newT)) {
    it->second.ty = std::move(newT);
    if (m_cls->work) {
      always_assert(!m_cls->work->noPropRefine);
      m_cls->work->worklist.scheduleForProp(name);
    }
  }
}

void PropertiesInfo::mergeInAllPrivateProps(const IIndex& index,
                                            const Type& t) {
  if (!m_cls || t.is(BBottom)) return;
  for (auto& kv : m_cls->privateProperties) {
    if (m_cls->work) {
      m_cls->work->worklist.addPropMutateDep(kv.first, *m_func);
      m_cls->work->propMutators[m_func].emplace(kv.first);
    }
    kv.second.everModified = true;
    auto newT = union_of(
      kv.second.ty,
      adjust_type_for_prop(index, *m_cls->ctx.cls, kv.second.tc, t)
    );
    if (kv.second.ty.strictlyMoreRefined(newT)) {
      kv.second.ty = std::move(newT);
      if (m_cls->work) {
        always_assert(!m_cls->work->noPropRefine);
        m_cls->work->worklist.scheduleForProp(kv.first);
      }
    }
  }
}

void PropertiesInfo::mergeInAllPrivateStatics(const IIndex& index,
                                              const Type& t,
                                              bool ignoreConst,
                                              bool mustBeReadOnly) {
  if (!m_cls || t.is(BBottom)) return;
  for (auto& kv : m_cls->privateStatics) {
    if (!ignoreConst && (kv.second.attrs & AttrIsConst)) continue;
    if (mustBeReadOnly && !(kv.second.attrs & AttrIsReadonly)) continue;
    if (m_cls->work) {
      m_cls->work->worklist.addPropMutateDep(kv.first, *m_func);
      m_cls->work->propMutators[m_func].emplace(kv.first);
    }
    kv.second.everModified = true;
    auto newT = union_of(
      kv.second.ty,
      adjust_type_for_prop(index, *m_cls->ctx.cls, kv.second.tc, t)
    );
    if (kv.second.ty.strictlyMoreRefined(newT)) {
      kv.second.ty = std::move(newT);
      if (m_cls->work) {
        always_assert(!m_cls->work->noPropRefine);
        m_cls->work->worklist.scheduleForProp(kv.first);
      }
    }
  }
}

void PropertiesInfo::setInitialValue(const php::Prop& prop,
                                     TypedValue val,
                                     bool satisfies,
                                     bool deepInit) {
  m_inits[&prop] = PropInitInfo{val, satisfies, deepInit};
}

const PropInitInfo*
PropertiesInfo::getInitialValue(const php::Prop& prop) const {
  return folly::get_ptr(m_inits, &prop);
}

//////////////////////////////////////////////////////////////////////

MethodsInfo::MethodsInfo(Context ctx, ClassAnalysis* cls)
  : m_cls{cls}
  , m_func{ctx.func}
{}

Optional<Index::ReturnType> MethodsInfo::lookupReturnType(const php::Func& f) {
  if (!m_cls || !m_cls->work) return std::nullopt;
  auto const& types = m_cls->work->returnTypes;
  auto const it = types.find(&f);
  if (it == types.end()) return std::nullopt;
  m_cls->work->worklist.addReturnTypeDep(f, *m_func);
  return it->second;
}

//////////////////////////////////////////////////////////////////////

void merge_closure_use_vars_into(ClosureUseVarMap& dst,
                                 const php::Class& clo,
                                 CompactVector<Type> types) {
  auto& current = dst[&clo];
  if (current.empty()) {
    current = std::move(types);
    return;
  }

  assertx(types.size() == current.size());
  for (auto i = uint32_t{0}; i < current.size(); ++i) {
    current[i] |= std::move(types[i]);
  }
}

template<class JoinOp>
bool merge_impl(State& dst, const State& src, JoinOp join) {
  if (!dst.initialized) {
    dst.copy_and_compact(src);
    return true;
  }

  assertx(src.initialized);
  assertx(dst.locals.size() == src.locals.size());
  assertx(dst.iters.size() == src.iters.size());
  assertx(dst.stack.size() == src.stack.size());

  if (src.unreachable) {
    // If we're coming from unreachable code and the dst is already
    // initialized, it doesn't change the dst (whether it is reachable or not).
    return false;
  }
  if (dst.unreachable) {
    // If we're going to code currently believed to be unreachable, take the
    // src state, and consider the dest state changed only if the source state
    // was reachable.
    dst.copy_and_compact(src);
    return !src.unreachable;
  }

  auto changed = false;

  auto const thisType = join(dst.thisType, src.thisType);
  if (thisType != dst.thisType) {
    changed = true;
    dst.thisType = thisType;
  }

  if (dst.thisLoc != src.thisLoc) {
    if (dst.thisLoc != NoLocalId) {
      dst.thisLoc = NoLocalId;
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

  for (auto i = size_t{0}; i < dst.iters.size(); ++i) {
    if (merge_into(dst.iters[i], src.iters[i], join)) {
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
          assertx(l == i || killSet == dstSet);
          changed = true;
        }
      } else {
        killLocEquiv(dst, i);
        changed = true;
      }
    }
  }

  return changed;
}

bool merge_into(State& dst, const State& src) {
  return merge_impl<Type(Type, Type)>(dst, src, union_of);
}

bool widen_into(State& dst, const State& src) {
  return merge_impl(dst, src, widening_union);
}

void InterpStack::refill(size_t elemIx, size_t indexLow,
                         int numPop, int numPush) {
  auto constexpr NoIndex =
    std::numeric_limits<decltype(index)::value_type>::max();

  if (numPush) {
    index.erase(index.begin() + indexLow, index.begin() + indexLow + numPush);
    elems.erase(elems.begin() + elemIx, elems.begin() + elemIx + numPush);
    for (auto i = indexLow; i < index.size(); i++) {
      if (index[i] >= elemIx) {
        auto DEBUG_ONLY ii = index[i] -= numPush;
        assertx(ii >= elemIx);
      }
    }
  }

  if (numPush != numPop) {
    for (auto i = elemIx; i < elems.size(); i++) {
      auto& elem = elems[i];
      if (elem.index >= indexLow) {
        elem.index += numPop - numPush;
      }
    }
  }

  if (!numPop) return;

  auto const indexHigh = indexLow + numPop;
  index.insert(index.begin() + indexLow, numPop, NoIndex);
  for (auto i = elemIx; i--; ) {
    auto const& elm = elems[i];
    if (elm.index >= indexLow &&
        elm.index < indexHigh &&
        index[elm.index] == NoIndex) {
      index[elm.index] = i;
      if (!--numPop) return;
    }
  }

  always_assert(false);
}

void InterpStack::rewind(int numPop, int numPush) {
  refill(elems.size() - numPush, index.size() - numPush, numPop, numPush);
}

void InterpStack::kill(int numPop, int numPush, uint32_t id) {
  for (auto i = elems.size(); i--; ) {
    auto const &elem = elems[i];
    if (elem.id == id) {
      for (auto j = 1; j < numPush; j++) {
        if (!i || elems[--i].id != id) always_assert(false);
      }
      return refill(i, elems[i].index, numPop, numPush);
    }
  }

  always_assert(false);
}

void InterpStack::insert_after(int numPop, int numPush, const Type* types,
                               uint32_t numInst, uint32_t id) {
  for (auto i = elems.size(); i--; ) {
    auto const &elem = elems[i];
    if (elem.id == id) {
      auto const indexLow = elem.index + 1 - numPop;
      index.erase(index.begin() + indexLow, index.begin() + indexLow + numPop);
      auto const elemIx = i + 1;
      elems.resize(elems.size() + numPush);
      for (auto j = elems.size() - numPush; j-- > elemIx; ) {
        auto& e = elems[j + numPush];
        if (numPush) e = std::move(elems[j]);
        e.index += numPush - numPop;
        if (e.id != StackElem::NoId) e.id += numInst;
      }
      for (auto j = indexLow; j < index.size(); j++) {
        index[j] += numPush;
      }
      index.insert(index.begin() + indexLow, numPush, uint32_t{});
      for (auto j = 0; j < numPush; j++) {
        auto& e = elems[elemIx + j];
        e.type = types[j];
        e.equivLoc = NoLocalId;
        e.index = indexLow + j;
        e.id = id + numInst;
        index[indexLow + j] = elemIx + j;
      }
      return;
    }
  }

  always_assert(false);
}

void InterpStack::peek(int numPop,
                       const StackElem** values,
                       int numPush) const {
  for (auto i = 0; i < numPop; i++) values[i] = nullptr;

  auto const sz = index.size() - numPush;
  for (auto i = elems.size() - numPush; i--; ) {
    auto const& elm = elems[i];
    if (elm.index >= sz &&
        elm.index - sz < numPop &&
        values[elm.index - sz] == nullptr) {
      values[elm.index - sz] = &elm;
      if (!--numPop) return;
    }
  }

  always_assert(false);
}

//////////////////////////////////////////////////////////////////////

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

std::string show(const php::Func& f, const CollectedInfo::MInstrState& s) {
  if (s.arrayChain.empty()) return show(f, s.base);
  return folly::sformat(
    "{} ({})",
    show(f, s.base),
    [&]{
      using namespace folly::gen;
      return from(s.arrayChain)
        | map([&] (const CollectedInfo::MInstrState::ArrayChainEnt& e) {
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
    folly::format(&ret, "thisType({})\n", show(st.thisType));
  }

  if (st.thisLoc != NoLocalId) {
    folly::format(&ret, "thisLoc({})\n", st.thisLoc);
  }

  for (auto i = size_t{0}; i < st.locals.size(); ++i) {
    folly::format(&ret, "{: <8} :: {}\n",
                  local_string(f, i),
                  show(st.locals[i]));
  }

  for (auto i = size_t{0}; i < st.iters.size(); ++i) {
    folly::format(&ret, "iter {: <2}  :: {}\n", i, show(f, st.iters[i]));
  }

  for (auto i = size_t{0}; i < st.stack.size(); ++i) {
    folly::format(&ret, "stk[{:02}] :: {} [{}]\n",
                  i,
                  show(st.stack[i].type),
                  st.stack[i].equivLoc == NoLocalId ? "" :
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

  if (collect.mInstrState.base.loc != BaseLoc::None) {
    folly::format(&ret, "mInstrState   :: {}\n", show(f, collect.mInstrState));
  }

  return ret;
}

std::string property_state_string(const PropertiesInfo& props) {
  std::string ret;

  for (auto const& kv : props.privatePropertiesRaw()) {
    folly::format(&ret, "$this->{: <14} :: {}\n", kv.first, show(kv.second.ty));
  }
  for (auto const& kv : props.privateStaticsRaw()) {
    folly::format(&ret, "self::${: <14} :: {}\n", kv.first, show(kv.second.ty));
  }

  return ret;
}

//////////////////////////////////////////////////////////////////////

}
