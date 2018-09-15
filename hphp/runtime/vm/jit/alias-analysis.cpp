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
#include "hphp/runtime/vm/jit/alias-analysis.h"

#include <utility>
#include <sstream>

#include "hphp/util/match.h"
#include "hphp/runtime/base/perf-warning.h"
#include "hphp/runtime/vm/jit/ir-unit.h"
#include "hphp/runtime/vm/jit/cfg.h"
#include "hphp/runtime/vm/jit/memory-effects.h"
#include "hphp/runtime/vm/jit/alias-class.h"

namespace HPHP { namespace jit {

TRACE_SET_MOD(hhir_alias);

namespace {

//////////////////////////////////////////////////////////////////////

// Locations that refer to ranges of the eval stack or multiple frame locals
// are expanded into individual locations only if smaller than this threshold.
constexpr int kMaxExpandedSize = 16;

template<class Visit>
void visit_locations(const BlockList& blocks, Visit visit) {
  for (auto& blk : blocks) {
    FTRACE(1, "B{}:\n", blk->id());
    for (auto& inst : *blk) {
      auto const effects = canonicalize(memory_effects(inst));
      FTRACE(1, "  {: <30} -- {}\n", show(effects), inst.toString());
      match<void>(
        effects,
        [&] (IrrelevantEffects)   {},
        [&] (UnknownEffects)      {},
        [&] (ReturnEffects x)     { visit(x.kills); },
        [&] (CallEffects x)       { visit(x.kills);
                                    visit(x.stack);
                                    visit(x.locals);
                                    visit(x.callee); },
        [&] (GeneralEffects x)    { visit(x.loads);
                                    visit(x.stores);
                                    visit(x.moves);
                                    visit(x.kills); },
        [&] (PureLoad x)          { visit(x.src); },
        [&] (PureStore x)         { visit(x.dst); },
        [&] (PureSpillFrame x)    { visit(x.stk);
                                    visit(x.ctx);
                                    visit(x.callee); },
        [&] (ExitEffects x)       { visit(x.live); visit(x.kills); }
      );
    }
  }
}

folly::Optional<uint32_t> add_class(AliasAnalysis& ret, AliasClass acls) {
  assertx(acls.isSingleLocation());
  auto const ins = ret.locations.insert(std::make_pair(acls, ALocMeta{}));
  if (!ins.second) return ins.first->second.index;
  if (ret.locations.size() > kMaxTrackedALocs) {
    always_assert(ret.locations.size() == kMaxTrackedALocs + 1);
    ret.locations.erase(acls);
    return folly::none;
  }
  FTRACE(1, "    new: {}\n", show(acls));
  auto& meta = ins.first->second;
  meta.index = ret.locations.size() - 1;
  always_assert(meta.index < kMaxTrackedALocs);
  return meta.index;
};

// Expand a location into a set of locations that may alias it. This is for
// locals and class-ref locations where the location may contain a discrete set
// of local/class-ref slots.
template<class T>
ALocBits may_alias_component(const AliasAnalysis& aa,
                             AliasClass acls,
                             folly::Optional<T> proj,
                             const AliasAnalysis::LocationMap& sets,
                             AliasClass any,
                             ALocBits pessimistic) {
  if (proj) {
    auto ret = ALocBits{};
    if (proj->ids.hasSingleValue()) {
      if (auto const slot = aa.find(*proj)) {
        ret.set(slot->index);
      }
      // Otherwise the location is untracked, and cannot interfere with any
      // tracked location.
    } else {
      auto const it = sets.find(*proj);
      if (it != end(sets)) {
        ret |= it->second;
      } else {
        ret |= pessimistic;
      }
    }
    return ret;
  }
  return acls.maybe(any) ? pessimistic : ALocBits{};
}

template<class T>
ALocBits may_alias_part(const AliasAnalysis& aa,
                        AliasClass acls,
                        folly::Optional<T> proj,
                        AliasClass any,
                        ALocBits pessimistic) {
  if (proj) {
    if (auto meta = aa.find(*proj)) {
      return ALocBits{meta->conflicts}.set(meta->index);
    }
    assertx(acls.maybe(any));
    return pessimistic;
  }
  return acls.maybe(any) ? pessimistic : ALocBits{};
}

// Expand a location into a set of locations which definitely contain it. This
// is for locals and class-ref locations where the location may contain a
// discrete set of local/class-ref slots.
template<class T>
ALocBits expand_component(const AliasAnalysis& aa,
                          AliasClass acls,
                          folly::Optional<T> loc,
                          const AliasAnalysis::LocationMap& sets,
                          AliasClass any,
                          ALocBits all) {
  if (loc) {
    auto ret = ALocBits{};
    if (loc->ids.hasSingleValue()) {
      if (auto const meta = aa.find(*loc)) {
        ret.set(meta->index);
      }
    } else {
      auto const it = sets.find(*loc);
      if (it != end(sets)) {
        ret |= it->second;
      }
      // We could iterate over everything and set corresponding bits, but that
      // seldom adds value.
    }
    return ret;
  }
  return any <= acls ? all : ALocBits{};
}

template<class T>
ALocBits expand_part(const AliasAnalysis& aa,
                     AliasClass acls,
                     folly::Optional<T> proj,
                     AliasClass any,
                     ALocBits all) {
  auto ret = ALocBits{};
  if (proj) {
    if (auto const meta = aa.find(*proj)) {
      return ret.set(meta->index);      // A single tracked location.
    }
    assertx(acls.maybe(any));
    return ret;
  }
  return any <= acls ? all : ret;
}

template<class T>
bool collect_component(AliasAnalysis& aa,
                       folly::Optional<T> loc,
                       AliasAnalysis::LocationMap& map) {
  if (loc) {
    assertx(!loc->ids.empty());
    if (loc->ids.hasSingleValue()) {
      add_class(aa, *loc);
    } else {
      auto complete = true;
      auto range = ALocBits{};
      if (loc->ids.size() <= kMaxExpandedSize) {
        for (uint32_t id = 0; id < AliasIdSet::BitsetMax; ++id) {
          if (loc->ids.test(id)) {
            if (auto const index = add_class(aa, T { loc->fp, id })) {
              range.set(*index);
            } else {
              complete = false;
            }
          }
        }
      }
      if (complete) map[AliasClass { *loc }] = range;
    }
    return true;
  }
  return false;
}

//////////////////////////////////////////////////////////////////////

}

AliasAnalysis::AliasAnalysis(const IRUnit& /*unit*/) {}

folly::Optional<ALocMeta> AliasAnalysis::find(AliasClass acls) const {
  auto const it = locations.find(acls);
  if (it == end(locations)) return folly::none;
  return it->second;
}

ALocBits AliasAnalysis::may_alias(AliasClass acls) const {
  if (auto meta = find(acls)) {
    return ALocBits{meta->conflicts}.set(meta->index);
  }

  auto ret = ALocBits{};

  // Handle stacks specially to be less pessimistic.  We can always use the
  // expand map to find stack locations that may alias our class.
  auto const stk = acls.stack();
  if (stk && stk->size > 1) {
    auto const it = stk_expand_map.find(*stk);
    ret |= it != end(stk_expand_map) ? it->second : all_stack;
  } else {
    ret |= may_alias_part(*this, acls, acls.stack(), AStackAny, all_stack);
  }

  ret |= may_alias_component(*this, acls, acls.frame(), local_sets,
                             AFrameAny, all_frame);
  ret |= may_alias_component(*this, acls, acls.clsRefSlot(), clsref_sets,
                             AClsRefSlotAny, all_clsRefSlot);

  ret |= may_alias_part(*this, acls, acls.rds(), ARdsAny, all_rds);

  if (auto const mis = acls.mis()) {
    auto const add_mis = [&] (AliasClass cls) {
      assertx(cls.isSingleLocation());
      if (cls <= *mis) {
        if (auto meta = find(cls)) {
          ret |= ALocBits{meta->conflicts}.set(meta->index);
        }
        // The location is untracked.
      }
    };

    add_mis(AMIStateTempBase);
    add_mis(AMIStateTvRef);
    add_mis(AMIStateTvRef2);
    add_mis(AMIStateBase);
    add_mis(AMIStatePropS);
  }

  ret |= may_alias_part(*this, acls, acls.prop(), APropAny, all_props);
  ret |= may_alias_part(*this, acls, acls.elemI(), AElemIAny, all_elemIs);
  ret |= may_alias_part(*this, acls, acls.ref(), ARefAny, all_ref);
  ret |= may_alias_part(*this, acls, acls.iterPos(), AIterPosAny, all_iterPos);
  ret |= may_alias_part(*this, acls, acls.iterBase(), AIterBaseAny,
                        all_iterBase);
  ret |= may_alias_part(*this, acls, acls.cufIterFunc(), ACufIterFuncAny,
                        all_cufIterFunc);
  ret |= may_alias_part(*this, acls, acls.cufIterCtx(), ACufIterCtxAny,
                        all_cufIterCtx);
  ret |= may_alias_part(*this, acls, acls.cufIterInvName(), ACufIterInvNameAny,
                        all_cufIterInvName);
  ret |= may_alias_part(*this, acls, acls.cufIterDynamic(), ACufIterDynamicAny,
                        all_cufIterDynamic);

  return ret;
}

ALocBits AliasAnalysis::expand(AliasClass acls) const {
  if (auto const info = find(acls)) return ALocBits{}.set(info->index);

  auto ret = ALocBits{};

  // We want to handle stacks partially specially, because they can be expanded
  // in some situations even if they don't have an ALocMeta.
  if (auto const stk = acls.stack()) {
    auto const it = stk->size > 1 ? stk_expand_map.find(*stk)
                                  : end(stk_expand_map);
    if (it != end(stk_expand_map)) {
      ret |= it->second;
    } else {
      ret |= expand_part(*this, acls, stk, AStackAny, all_stack);
    }
  } else if (AStackAny <= acls) {
    ret |= all_stack;
  }

  ret |= expand_component(*this, acls, acls.frame(), local_sets,
                          AFrameAny, all_frame);
  ret |= expand_component(*this, acls, acls.clsRefSlot(), clsref_sets,
                          AClsRefSlotAny, all_clsRefSlot);

  ret |= expand_part(*this, acls, acls.rds(), ARdsAny, all_rds);

  if (auto const mis = acls.mis()) {
    auto const add_mis = [&] (AliasClass cls) {
      assertx(cls.isSingleLocation());
      if (cls <= *mis) {
        if (auto const meta = find(cls)) {
          ret.set(meta->index);
        }
      }
    };

    add_mis(AMIStateTempBase);
    add_mis(AMIStateTvRef);
    add_mis(AMIStateTvRef2);
    add_mis(AMIStateBase);
    add_mis(AMIStatePropS);
  }

  ret |= expand_part(*this, acls, acls.prop(), APropAny, all_props);
  ret |= expand_part(*this, acls, acls.elemI(), AElemIAny, all_elemIs);
  ret |= expand_part(*this, acls, acls.ref(), ARefAny, all_ref);
  ret |= expand_part(*this, acls, acls.iterPos(), AIterPosAny, all_iterPos);
  ret |= expand_part(*this, acls, acls.iterBase(), AIterBaseAny, all_iterBase);
  ret |= expand_part(*this, acls, acls.cufIterFunc(), ACufIterFuncAny,
                     all_cufIterFunc);
  ret |= expand_part(*this, acls, acls.cufIterCtx(), ACufIterCtxAny,
                     all_cufIterCtx);
  ret |= expand_part(*this, acls, acls.cufIterInvName(), ACufIterInvNameAny,
                     all_cufIterInvName);
  ret |= expand_part(*this, acls, acls.cufIterDynamic(), ACufIterDynamicAny,
                     all_cufIterDynamic);

  return ret;
}

AliasAnalysis collect_aliases(const IRUnit& unit, const BlockList& blocks) {
  FTRACE(1, "collect_aliases:vvvvvvvvvvvvvvvvvvvv\n");
  SCOPE_EXIT { FTRACE(1, "collect_aliases:^^^^^^^^^^^^^^^^^^^^\n"); };

  auto ret = AliasAnalysis{unit};

  /*
   * Right now we compute the conflict sets for object properties based only on
   * object property offsets, and for arrays based only on index.  Everything
   * colliding in that regard is assumed to possibly alias.
   */
  auto conflict_prop_offset = jit::fast_map<uint32_t,ALocBits>{};
  auto conflict_array_index = jit::fast_map<int64_t,ALocBits>{};

  visit_locations(blocks, [&] (AliasClass acls) {
    if (auto const prop = acls.is_prop()) {
      if (auto const index = add_class(ret, acls)) {
        conflict_prop_offset[prop->offset].set(*index);
      }
      return;
    }

    if (auto const elemI = acls.is_elemI()) {
      if (auto const index = add_class(ret, acls)) {
        conflict_array_index[elemI->idx].set(*index);
      }
      return;
    }

    if (acls.is_rds()) {
      add_class(ret, acls);
      return;
    }

    if (acls.is_ref()) {
      if (auto const index = add_class(ret, acls)) {
        ret.all_ref.set(*index);
      }
      return;
    }

    if (acls.is_mis() && acls.isSingleLocation()) {
      add_class(ret, acls);
      return;
    }

    if (acls.is_iterPos() || acls.is_iterBase()) {
      add_class(ret, acls);
      return;
    }

    if (acls.is_cufIterFunc() ||
        acls.is_cufIterCtx() ||
        acls.is_cufIterInvName() ||
        acls.is_cufIterDynamic()) {
      add_class(ret, acls);
      return;
    }

    if (collect_component(ret, acls.frame(), ret.local_sets)) return;
    if (collect_component(ret, acls.clsRefSlot(), ret.clsref_sets)) return;

    /*
     * Note that unlike the above we're going to assign location ids to the
     * individual stack slots in AStack portions of AliasClasses that are
     * unions of AStack ranges with other classes.  (I.e. basically we're using
     * stack() instead of is_stack() here, so it will match things that are
     * only partially stacks.)
     *
     * The reason for this is that many instructions can have effects like
     * that, when they can re-enter and do things to the stack in some range
     * (below the re-entry depth, for example), but also affect some other type
     * of memory (CastStk, for example).  In particular this means we want that
     * AliasClass to have an entry in the stack_ranges, so we'll populate
     * it later.  Currently most of these situations should probably bail at
     * kMaxExpandedStackRange, although there are some situations that won't
     * (e.g. instructions like CoerceStk, which will have an AHeapAny (from
     * re-entry) unioned with a single stack slot).
     */
    if (auto const stk = acls.stack()) {
      if (stk->size > 1) {
        ret.stk_expand_map[AliasClass { *stk }];
      }
      if (stk->size > kMaxExpandedSize) return;

      auto complete = true;
      auto range = ALocBits{};
      for (auto stkidx = int32_t{0}; stkidx < stk->size; ++stkidx) {
        AliasClass single = AStack { stk->offset - stkidx, 1 };
        if (auto const index = add_class(ret, single)) {
          range.set(*index);
        } else {
          complete = false;
        }
      }

      if (stk->size > 1 && complete) {
        FTRACE(2, "    range {}:  {}\n", show(acls), show(range));
        ret.stack_ranges[acls] = range;
      }
    }
  });

  always_assert(ret.locations.size() <= kMaxTrackedALocs);
  if (ret.locations.size() == kMaxTrackedALocs) {
    logLowPriPerfWarning(
      "alias-analysis kMaxTrackedALocs",
      25000,
      [&](StructuredLogEntry& cols) {
        auto const func = unit.context().func;
        cols.setStr("func", func->fullName()->slice());
        cols.setStr("filename", func->unit()->filepath()->slice());
        cols.setStr("hhir_unit", show(unit));
      }
    );
    FTRACE(1, "max locations limit was reached\n");
  }

  auto make_conflict_set = [&] (AliasClass acls, ALocMeta& meta) {
    if (auto const prop = acls.is_prop()) {
      meta.conflicts = conflict_prop_offset[prop->offset];
      meta.conflicts.reset(meta.index);
      ret.all_props.set(meta.index);
      return;
    }

    if (auto const elemI = acls.is_elemI()) {
      meta.conflicts = conflict_array_index[elemI->idx];
      meta.conflicts.reset(meta.index);
      ret.all_elemIs.set(meta.index);
      return;
    }

    if (auto const frame = acls.is_frame()) {
      ret.all_frame.set(meta.index);
      return;
    }

    if (auto const slot = acls.is_clsRefSlot()) {
      ret.all_clsRefSlot.set(meta.index);
      return;
    }

    if (acls.is_stack()) {
      ret.all_stack.set(meta.index);
      return;
    }

    if (acls.is_iterPos()) {
      ret.all_iterPos.set(meta.index);
      return;
    }

    if (acls.is_iterBase()) {
      ret.all_iterBase.set(meta.index);
      return;
    }

    if (acls.is_cufIterFunc()) {
      ret.all_cufIterFunc.set(meta.index);
      return;
    }

    if (acls.is_cufIterCtx()) {
      ret.all_cufIterCtx.set(meta.index);
      return;
    }

    if (acls.is_cufIterInvName()) {
      ret.all_cufIterInvName.set(meta.index);
      return;
    }

    if (acls.is_cufIterDynamic()) {
      ret.all_cufIterDynamic.set(meta.index);
      return;
    }

    if (acls.is_ref()) {
      meta.conflicts = ret.all_ref;
      meta.conflicts.reset(meta.index);
      return;
    }

    if (acls.is_rds()) {
      ret.all_rds.set(meta.index);
      return;
    }

    if (acls.is_mis()) {
      // We don't maintain an all_mistate set so there's nothing to do here but
      // avoid hitting the assert below.
      return;
    }

    always_assert_flog(0, "AliasAnalysis assigned an AliasClass an id "
      "but it didn't match a situation we understood: {}\n", show(acls));
  };

  ret.locations_inv.resize(ret.locations.size());
  for (auto& kv : ret.locations) {
    make_conflict_set(kv.first, kv.second);
    ret.locations_inv[kv.second.index] = kv.second;

    /*
     * Note: this is probably more complex than it needs to be, because we're
     * iterating the stk_expand_map for each location.  Since kMaxTrackedALocs
     * is bounded by a constant, it's kinda O(stk_expand_map), but not in a
     * good way.  The number of locations is currently generally small, so this
     * is probably ok for now---but if we remove the limit we may need to
     * revisit this so it can't blow up.
     */
    if (kv.first.is_stack()) {
      for (auto& ent : ret.stk_expand_map) {
        if (kv.first <= ent.first) {
          FTRACE(2, "  ({}) {} <= {}\n",
            kv.second.index,
            show(kv.first),
            show(ent.first));
          ent.second.set(kv.second.index);
        }
      }
    } else if (kv.first.is_frame()) {
      for (auto& ent : ret.local_sets) {
        if (kv.first <= ent.first) {
          FTRACE(2, "  ({}) {} <= {}\n",
            kv.second.index,
            show(kv.first),
            show(ent.first));
          ent.second.set(kv.second.index);
        }
      }
    } else if (kv.first.is_clsRefSlot()) {
      for (auto& ent : ret.clsref_sets) {
        if (kv.first <= ent.first) {
          FTRACE(2, "  ({}) {} <= {}\n",
            kv.second.index,
            show(kv.first),
            show(ent.first));
          ent.second.set(kv.second.index);
        }
      }
    }
  }

  return ret;
}

//////////////////////////////////////////////////////////////////////

std::string show(ALocBits bits) {
  std::ostringstream out;
  if (bits.none()) {
    return "0";
  }
  if (bits.all()) {
    return "-1";
  }
  out << bits;
  return out.str();
}

std::string show(const AliasAnalysis& ainfo) {
  auto ret = std::string{};
  std::vector<const decltype(ainfo.locations)::value_type*> sorted;
  sorted.reserve(ainfo.locations.size());
  for (auto& kv : ainfo.locations) {
    sorted.push_back(&kv);
  }
  std::sort(sorted.begin(), sorted.end(),
            [](auto const* a, auto const* b) {
              return a->second.index < b->second.index;
            });

  for (auto const* kv : sorted) {
    auto conf = kv->second.conflicts;
    conf.set(kv->second.index);
    folly::format(&ret, " {: <20} = {: >3} : {}\n",
      show(kv->first),
      kv->second.index,
      show(conf));
  }
  folly::format(
      &ret,
      " {: <20}       : {}\n"
      " {: <20}       : {}\n"
      " {: <20}       : {}\n"
      " {: <20}       : {}\n"
      " {: <20}       : {}\n"
      " {: <20}       : {}\n"
      " {: <20}       : {}\n"
      " {: <20}       : {}\n"
      " {: <20}       : {}\n"
      " {: <20}       : {}\n"
      " {: <20}       : {}\n",

      "all props",          show(ainfo.all_props),
      "all elemIs",         show(ainfo.all_elemIs),
      "all refs",           show(ainfo.all_ref),
      "all iterPos",        show(ainfo.all_iterPos),
      "all iterBase",       show(ainfo.all_iterBase),
      "all cufIterFunc",    show(ainfo.all_cufIterFunc),
      "all cufIterCtx",     show(ainfo.all_cufIterCtx),
      "all cufIterInvName", show(ainfo.all_cufIterInvName),
      "all cufIterDynamic", show(ainfo.all_cufIterDynamic),
      "all frame",          show(ainfo.all_frame),
      "all clsRefSlot",     show(ainfo.all_clsRefSlot),
      "all rds",            show(ainfo.all_rds)
  );
  for (auto& kv : ainfo.local_sets) {
    folly::format(&ret, " ex {: <17}       : {}\n",
      show(kv.first),
      show(kv.second));
  }
  folly::format(&ret, " {: <20}       : {}\n",
     "all stack",  show(ainfo.all_stack));
  for (auto& kv : ainfo.stack_ranges) {
    folly::format(&ret, " ex {: <17}       : {}\n",
      show(kv.first),
      show(kv.second));
  }
  return ret;
}

//////////////////////////////////////////////////////////////////////

}}
