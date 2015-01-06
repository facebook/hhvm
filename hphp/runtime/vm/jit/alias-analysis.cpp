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

// Locations that refer to ranges of the eval stack are expanded into
// individual stack slots only if smaller than this threshold.
constexpr int kMaxExpandedStackRange = 16;

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
        [&] (InterpOneEffects x)  { visit(x.killed); },
        [&] (ReturnEffects x)     { visit(x.killed); },
        [&] (KillFrameLocals)     {},
        [&] (CallEffects x)       { visit(x.killed); visit(x.stack); },
        [&] (IterEffects x)       { visit(x.killed); },
        [&] (IterEffects2 x)      { visit(x.killed); },
        [&] (MayLoadStore x)      { visit(x.loads); visit(x.stores); },
        [&] (PureLoad x)          { visit(x.src); },
        [&] (PureStore x)         { visit(x.dst); },
        [&] (PureStoreNT x)       { visit(x.dst); },
        [&] (PureSpillFrame x)    { visit(x.dst); },
        [&] (ExitEffects x)       { visit(x.live); visit(x.kill); }
      );
    }
  }
}

uint32_t add_class(AliasAnalysis& ret, AliasClass acls) {
  auto const ins = ret.locations.insert(std::make_pair(acls, ALocMeta{}));
  if (!ins.second) return ins.first->second.index;
  FTRACE(1, "    new: {}\n", show(acls));
  auto& meta = ins.first->second;
  meta.index = ret.locations.size() - 1;
  return meta.index;
};

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
  assert(!find(acls));

  auto ret = ALocBits{};

  // We may have some special may-alias sets for multi-slot stack ranges.  If
  // one of these is present, we can use that for the stack portion.
  // Otherwise, we need to merge all_stack, because we didn't track which stack
  // locations it can alias earlier.
  if (auto const stk = acls.stack()) {
    if (stk->size > 1) {
      auto const it = stack_ranges.find(*stk);
      ret |= it != end(stack_ranges) ? it->second : all_stack;
    } else {
      if (auto const slot = find(*stk)) {
        ret.set(slot->index);
      } else {
        ret |= all_stack;
      }
    }
  } else if (acls.maybe(AStackAny)) {
    ret |= all_stack;
  }

  if (acls.maybe(APropAny))   ret |= all_props;
  if (acls.maybe(AElemIAny))  ret |= all_elemIs;
  if (acls.maybe(AFrameAny))  ret |= all_frame;

  return ret;
}

ALocBits AliasAnalysis::must_alias(AliasClass acls) const {
  if (auto const info = find(acls)) {
    auto ret = ALocBits{};
    ret.set(info->index);
    return ret;
  }

  auto const it = must_alias_map.find(acls);
  if (it == end(must_alias_map)) return ALocBits{};
  return it->second;
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
  auto conflict_array_index = jit::hash_map<uint64_t,ALocBits>{};

  /*
   * Stack offset conflict sets: any stack alias class based off a different
   * StkPtr base is presumed to potentially alias.  See the comments above
   * AStack in alias-class.h.
   */
  auto conflict_stkptrs = jit::hash_map<SSATmp*,ALocBits>{};

  visit_locations(blocks, [&] (AliasClass acls) {
    if (blocks.size() >= kMaxTrackedALocs) return;

    if (auto const prop = acls.is_prop()) {
      conflict_prop_offset[prop->offset].set(add_class(ret, acls));
      return;
    }

    if (auto const elemI = acls.is_elemI()) {
      conflict_array_index[elemI->idx].set(add_class(ret, acls));
      return;
    }

    if (auto const frame = acls.is_frame()) {
      add_class(ret, acls);
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
     * of memory (DecRefMem, for example).  In particular this means we want
     * that AliasClass to have an entry in the must_alias_map, so we'll
     * populate it later.  Currently most of these situations should probably
     * bail at kMaxExpandedStackRange, although there are some situations that
     * won't (e.g. instructions like CoerceStk, or DecRefStk, which will have
     * an AHeapAny (from re-entry) unioned with a single stack slot).
     */
    if (auto const stk = acls.stack()) {
      if (stk->size > 1) {
        ret.must_alias_map[acls];
      }
      if (stk->size > kMaxExpandedStackRange) return;

      ALocBits conf_set;
      for (auto idx = int32_t{0}; idx < stk->size; ++idx) {
        AliasClass const single = AStack { stk->base, stk->offset - idx, 1 };
        auto const id = add_class(ret, single);
        conf_set.set(id);
        conflict_stkptrs[stk->base].set(id);
      }

      if (stk->size > 1) {
        FTRACE(2, "    range {}:  {}\n", show(acls), show(conf_set));
        ret.stack_ranges[acls] = conf_set;
      }

      return;
    }
  });

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
      for (auto& kv : conflict_stkptrs) {
        if (kv.first != stk->base) {
          if (kv.first->type() <= Type::StkPtr ||
              stk->base->type() <= Type::StkPtr) {
            meta.conflicts |= kv.second;
          }
        }
      }
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
     * iterating the must_alias_map for each location.  Since kMaxTrackedALocs
     * is bounded by a constant, it's kinda O(must_alias_map), but not in a
     * good way.  The number of locations is currently generally small, so this
     * is probably ok for now---but if we remove the limit we may need to
     * revisit this so it can't blow up.
     */
    if (kv.first.is_stack()) {
      for (auto& maEnt : ret.must_alias_map) {
        if (kv.first <= maEnt.first) {
          FTRACE(2, "  ({}) {} must_alias {}\n",
            kv.second.index,
            show(kv.first),
            show(maEnt.first));
          maEnt.second.set(kv.second.index);
        } else {
          FTRACE(3, "  !must_alias: {} and {}\n",
            show(kv.first),
            show(maEnt.first));
        }
      }
    }
  }

  return ret;
}

//////////////////////////////////////////////////////////////////////

std::string show(ALocBits bits) {
  std::ostringstream out;
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
                      " {: <20}       : {}\n",
    "all props",  show(linfo.all_props),
    "all elemIs", show(linfo.all_elemIs),
    "all frame",  show(linfo.all_frame),
    "all stack",  show(linfo.all_stack)
  );
  for (auto& kv : linfo.must_alias_map) {
    folly::format(&ret, " ma {: <17}       : {}\n",
      show(kv.first),
      show(kv.second));
  }
  return ret;
}

//////////////////////////////////////////////////////////////////////

}}
