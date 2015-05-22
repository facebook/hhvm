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
#include "hphp/runtime/vm/jit/alias-analysis.h"

#include <utility>
#include <sstream>

#include "hphp/util/match.h"
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
        [&] (CallEffects x)       { visit(x.kills); visit(x.stack); },
        [&] (GeneralEffects x)    { visit(x.loads);
                                    visit(x.stores);
                                    visit(x.moves);
                                    visit(x.kills); },
        [&] (PureLoad x)          { visit(x.src); },
        [&] (PureStore x)         { visit(x.dst); },
        [&] (PureSpillFrame x)    { visit(x.stk); visit(x.ctx); },
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

template<class T>
ALocBits may_alias_part(const AliasAnalysis& aa,
                        AliasClass acls,
                        folly::Optional<T> proj,
                        AliasClass any,
                        ALocBits pessimistic) {
  if (proj) {
    if (auto const meta = aa.find(*proj)) {
      return ALocBits{meta->conflicts}.set(meta->index);
    }
    assertx(acls.maybe(any));
    return pessimistic;
  }
  return acls.maybe(any) ? pessimistic : ALocBits{};
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
    assertx(acls <= any);
    return ret;
  }
  return any <= acls ? all : ret;
}

//////////////////////////////////////////////////////////////////////

}

AliasAnalysis::AliasAnalysis(const IRUnit& unit)
  : per_frame_bits(unit.numTmps())
{}

folly::Optional<ALocMeta> AliasAnalysis::find(AliasClass acls) const {
  auto const it = locations.find(acls);
  if (it == end(locations)) return folly::none;
  return it->second;
}

ALocBits AliasAnalysis::may_alias(AliasClass acls) const {
  if (auto const meta = find(acls)) {
    return ALocBits{meta->conflicts}.set(meta->index);
  }

  auto ret = ALocBits{};

  if (auto const stk = acls.stack()) {
    if (stk->size == 1) {
      if (auto const meta = find(*stk)) {
        // Different stack slots never alias each other
        assertx(meta->conflicts.none());
        ret.set(meta->index);
      }
      // Otherwise the stack slot is untracked, no need to pessimize.
    } else {
      auto const it = stack_ranges.find(*stk);
      // Since individual stack slots are canonicalized, we could be less
      // pessimistic even if the entry is not in `stack_ranges', but that
      // would require iterating over `all_stack'.
      ret |= it != end(stack_ranges) ? it->second : all_stack;
    }
  } else if (acls.maybe(AStackAny)) {
    ret |= all_stack;
  }

  if (auto const frame = acls.frame()) {
    if (frame->ids.hasSingleValue()) {
      if (auto const slot = find(*frame)) {
        ret.set(slot->index);
      }
      // Otherwise the location is untracked.
    } else {
      auto const it = frame_ranges.find(*frame);
      if (it != end(frame_ranges)) {
        ret |= it->second;
      } else {
        if (frame->ids.hasUpperRange()) {
          ret |= all_frame;
        }
        for (uint32_t id = 0; id <= AliasIdSet::BitsetMax; ++id) {
          if (auto const slot = find(AFrame {frame->fp, id})) {
            if (frame->ids.test(id)) {
              ret.set(slot->index);
            } else {
              ret.reset(slot->index);
            }
          }
        }
      }
    }
  } else if (acls.maybe(AFrameAny)) {
    ret |= all_frame;
  }

  ret |= may_alias_part(*this, acls, acls.prop(), APropAny, all_props);
  ret |= may_alias_part(*this, acls, acls.elemI(), AElemIAny, all_elemIs);
  ret |= may_alias_part(*this, acls, acls.mis(), AMIStateAny, all_mistate);
  ret |= may_alias_part(*this, acls, acls.ref(), ARefAny, all_refs);

  return ret;
}

ALocBits AliasAnalysis::expand(AliasClass acls) const {
  if (auto const info = find(acls)) return ALocBits{}.set(info->index);

  auto ret = ALocBits{};

  if (auto const stack = acls.stack()) {
    if (stack->size == 1) {
      if (auto const info = find(*stack)) {
        ret.set(info->index);
      }
    } else {
      auto const it = stack_ranges.find(*stack);
      if (it != end(stack_ranges)) {
        ret |= it->second;
      }
      // We could iterate over `all_stack' and check individually, but since we
      // don't combine stack slots in `AliasClass::precise_union()', doing so
      // would not really help us much for now.
    }
  } else if (AStackAny <= acls) {
    ret |= all_stack;
  }

  if (auto const frame = acls.frame()) {
    if (frame->ids.hasSingleValue()) {
      if (auto const meta = find(*frame)) {
        ret.set(meta->index);
      }
    } else {
      auto const it = frame_ranges.find(*frame);
      if (it != end(frame_ranges)) {
        ret |= it->second;
      } else {
        for (uint32_t id = 0; id <= AliasIdSet::BitsetMax; ++id) {
          AliasClass const single_frame = AFrame { frame->fp, id };
          if (auto const meta = find(single_frame)) {
            ret.set(meta->index);
          }
        }
      }
    }
  } else if (AFrameAny <= acls) {
    ret |= all_frame;
  }

  ret |= expand_part(*this, acls, acls.frame(), AFrameAny, all_frame);
  ret |= expand_part(*this, acls, acls.prop(), APropAny, all_props);
  ret |= expand_part(*this, acls, acls.elemI(), AElemIAny, all_elemIs);
  ret |= expand_part(*this, acls, acls.mis(), AMIStateAny, all_mistate);
  ret |= expand_part(*this, acls, acls.ref(), ARefAny, all_refs);

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
  auto conflict_prop_offset = jit::hash_map<uint32_t,ALocBits>{};
  auto conflict_array_index = jit::hash_map<int64_t,ALocBits>{};

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

    if (auto const ref = acls.is_ref()) {
      if (auto const index = add_class(ret, acls)) {
        ret.all_refs.set(*index);
      }
      return;
    }

    if (acls.is_mis()) {
      add_class(ret, acls);
      return;
    }

    if (auto const frame = acls.frame()) {
      always_assert(!frame->ids.empty());
      if (frame->ids.hasSingleValue()) {
        add_class(ret, *frame);
      } else {
        ret.frame_ranges[AliasClass { *frame }];

        if (frame->ids.size() <= kMaxExpandedSize) {
          for (uint32_t id = 0; id < AliasIdSet::BitsetMax; ++id) {
            if (frame->ids.test(id)) {
              add_class(ret, AFrame { frame->fp, id });
            }
          }
        }
      }
      return;
    }

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
        ret.stack_ranges[AliasClass { *stk }];
      }
      if (stk->size > kMaxExpandedSize) return;

      for (auto stkidx = int32_t{0}; stkidx < stk->size; ++stkidx) {
        add_class(ret, AStack { stk->offset - stkidx, 1 });
      }
    }
  });

  always_assert(ret.locations.size() <= kMaxTrackedALocs);
  if (ret.locations.size() == kMaxTrackedALocs) {
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
      ret.per_frame_bits[frame->fp].set(meta.index);
      return;
    }

    if (auto const stk = acls.is_stack()) {
      ret.all_stack.set(meta.index);
      return;
    }

    if (auto const mis = acls.is_mis()) {
      ret.all_mistate.set(meta.index);
      return;
    }

    if (auto const ref = acls.is_ref()) {
      meta.conflicts = ret.all_refs;
      meta.conflicts.reset(meta.index);
      return;
    }

    always_assert_flog(0, "AliasAnalysis assigned an AliasClass an id "
      "but it didn't match a situation we undestood: {}\n", show(acls));
  };

  ret.locations_inv.resize(ret.locations.size());
  for (auto& kv : ret.locations) {
    make_conflict_set(kv.first, kv.second);
    ret.locations_inv[kv.second.index] = kv.second;

    /*
     * Note: this is probably more complex than it needs to be, because we're
     * iterating the stack_ranges for each location.  Since kMaxTrackedALocs
     * is bounded by a constant, it's kinda O(stack_ranges), but not in a
     * good way.  The number of locations is currently generally small, so this
     * is probably ok for now---but if we remove the limit we may need to
     * revisit this so it can't blow up.
     */
    if (kv.first.is_stack()) {
      for (auto& ent : ret.stack_ranges) {
        if (kv.first <= ent.first) {
          FTRACE(2, "  ({}) {} <= {}\n",
            kv.second.index,
            show(kv.first),
            show(ent.first));
          ent.second.set(kv.second.index);
        }
      }
    } else if (kv.first.is_frame()) {
      for (auto& ent : ret.frame_ranges) {
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

std::string show(const AliasAnalysis& linfo) {
  auto ret = std::string{};
  for (auto& kv : linfo.locations) {
    auto conf = kv.second.conflicts;
    conf.set(kv.second.index);
    folly::format(&ret, " {: <20} = {: >3} : {}\n",
      show(kv.first),
      kv.second.index,
      show(conf));
  }
  folly::format(&ret, " {: <20}       : {}\n"
                      " {: <20}       : {}\n"
                      " {: <20}       : {}\n"
                      " {: <20}       : {}\n"
    "all props",  show(linfo.all_props),
    "all elemIs", show(linfo.all_elemIs),
    "all refs",   show(linfo.all_refs),
    "all frame",  show(linfo.all_frame)
  );
  for (auto& kv : linfo.frame_ranges) {
    folly::format(&ret, " ex {: <17}       : {}\n",
      show(kv.first),
      show(kv.second));
  }
  folly::format(&ret, " {: <20}       : {}\n",
     "all stack",  show(linfo.all_stack));
  for (auto& kv : linfo.stack_ranges) {
    folly::format(&ret, " ex {: <17}       : {}\n",
      show(kv.first),
      show(kv.second));
  }
  return ret;
}

//////////////////////////////////////////////////////////////////////

}}
