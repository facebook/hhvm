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

TRACE_SET_MOD(hhir_aloc);

namespace {

//////////////////////////////////////////////////////////////////////

template<class Visit>
void visit_locations(const BlockList& blocks, Visit visit) {
  for (auto& blk : blocks) {
    FTRACE(1, "B{}:\n", blk->id());
    for (auto& inst : *blk) {
      auto const effects = canonicalize(memory_effects(inst));
      FTRACE(1, "  {: <30} -- {}\n", show(effects), inst.toString());
      match<void>(
        effects,
        [&] (IrrelevantEffects) {},
        [&] (UnknownEffects)    {},
        [&] (InterpOneEffects)  {},
        [&] (ReturnEffects)     {},
        [&] (KillFrameLocals)   {},
        [&] (CallEffects)       {},
        [&] (IterEffects)       {},
        [&] (IterEffects2)      {},
        [&] (MayLoadStore x)    { visit(x.loads); visit(x.stores); },
        [&] (PureLoad x)        { visit(x.src); },
        [&] (PureStore x)       { visit(x.dst); },
        [&] (PureStoreNT x)     { visit(x.dst); }
      );
    }
  }
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
  auto ret = ALocBits{};
  if (acls.maybe(APropAny))   ret |= all_props;
  if (acls.maybe(AElemIAny))  ret |= all_elemIs;
  if (acls.maybe(AFrameAny))  ret |= all_frame;
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
  auto conflict_array_index = jit::hash_map<uint64_t,ALocBits>{};

  visit_locations(blocks, [&] (AliasClass acls) {
    if (blocks.size() >= kMaxTrackedALocs) return;

    if (auto const prop = acls.prop()) {
      auto const ins = ret.locations.insert(std::make_pair(acls, ALocMeta{}));
      if (!ins.second) return;
      FTRACE(1, "    new: {}\n", show(acls));
      auto& meta = ins.first->second;
      meta.index = ret.locations.size() - 1;
      conflict_prop_offset[prop->offset].set(meta.index);
      return;
    }

    if (auto const elemI = acls.elemI()) {
      auto const ins = ret.locations.insert(std::make_pair(acls, ALocMeta{}));
      if (!ins.second) return;
      FTRACE(1, "    new: {}\n", show(acls));
      auto& meta = ins.first->second;
      meta.index = ret.locations.size() - 1;
      conflict_array_index[elemI->idx].set(meta.index);
      return;
    }

    if (auto const frame = acls.frame()) {
      auto const ins = ret.locations.insert(std::make_pair(acls, ALocMeta{}));
      if (!ins.second) return;
      FTRACE(1, "   new: {}\n", show(acls));
      auto& meta = ins.first->second;
      meta.index = ret.locations.size() - 1;
      return;
    }
  });

  if (ret.locations.size() == kMaxTrackedALocs) {
    FTRACE(1, "max locations limit was reached\n");
  }

  auto make_conflict_set = [&] (AliasClass acls, ALocMeta& meta) {
    if (auto const prop = acls.prop()) {
      meta.conflicts = conflict_prop_offset[prop->offset];
      meta.conflicts.reset(meta.index);
      ret.all_props.set(meta.index);
      return;
    }
    if (auto const elemI = acls.elemI()) {
      meta.conflicts = conflict_array_index[elemI->idx];
      meta.conflicts.reset(meta.index);
      ret.all_elemIs.set(meta.index);
      return;
    }
    if (auto const frame = acls.frame()) {
      ret.all_frame.set(meta.index);
      ret.per_frame_bits[frame->fp].set(meta.index);
      return;
    }
  };

  ret.locations_inv.resize(ret.locations.size());
  for (auto& kv : ret.locations) {
    make_conflict_set(kv.first, kv.second);
    ret.locations_inv[kv.second.index] = kv.second;
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
    folly::format(&ret, " {: <20} = {: >3} : {}\n",
      show(kv.first),
      kv.second.index,
      show(kv.second.conflicts));
  }
  folly::format(&ret, " {: <20}       : {}\n"
                      " {: <20}       : {}\n"
                      " {: <20}       : {}\n",
    "all props",  show(linfo.all_props),
    "all elemIs", show(linfo.all_elemIs),
    "all frame",  show(linfo.all_frame)
  );
  return ret;
}

//////////////////////////////////////////////////////////////////////

}}
