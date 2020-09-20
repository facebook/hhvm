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
#include "hphp/runtime/vm/jit/opt.h"

#include <iterator>
#include <string>

#include <folly/Format.h>
#include <folly/Optional.h>
#include <folly/ScopeGuard.h>

#include "hphp/util/bisector.h"
#include "hphp/util/bitset-utils.h"
#include "hphp/util/dataflow-worklist.h"
#include "hphp/util/match.h"
#include "hphp/util/trace.h"

#include "hphp/runtime/vm/jit/alias-analysis.h"
#include "hphp/runtime/vm/jit/cfg.h"
#include "hphp/runtime/vm/jit/containers.h"
#include "hphp/runtime/vm/jit/ir-unit.h"
#include "hphp/runtime/vm/jit/memory-effects.h"
#include "hphp/runtime/vm/jit/mutation.h"
#include "hphp/runtime/vm/jit/pass-tracer.h"
#include "hphp/runtime/vm/jit/state-multi-map.h"
#include "hphp/runtime/vm/jit/state-vector.h"
#include "hphp/runtime/vm/jit/timer.h"

/*
  This implements partial-redundancy elimination for stores.

  The basic algorithm follows Morel & Renvoise "Global Optimization by
  Suppression of Partial Redundancies".  That paper talks about redundancy of
  expressions, so we have to "reverse" everything to apply it to stores
  (Anticipated <=> Available, In <=> Out).

  Some general terminology:
    - A store to a location L is /available/ at code position P if it has
      happened on all paths to P, and there is no interference (loads or
      other stores) of L between those stores and P.
    - A store to a location L is /anticipated/ at code position P if it occurs
      between P and any subsequent use of L.

  There are two forms of (non)-transparency for stores.  A (possible) use of
  the result of a candidate store prevents the candidate from being moved past
  the use, and also prevents the candidate from being killed by a later store.
  This is very similar to transparency of expressions.  But in addition, a
  possibly interfering store following a candidate store prevents the candidate
  from moving past the store, but does not prevent the candidate from being
  killed by a later store.  We represent these two criteria via alteredAnt and
  alteredAvl (see below).

  This also affects local availabilty; a store may not be available in the
  sense that it can be moved to the end of the block (because of a possible
  conflicting write), but it may still be available in the sense that it can be
  killed by a later store.  We split this out into AvlLoc (can be moved to the
  end), and DelLoc (can be deleted if it's anticipated out).

  Finally, we make some simplifications to the CONST term in M & R to
  reduce the number of bitvectors involved, and reduce the bidirectionality
  of the data flow equations.

  The final equations are:

  Local bitvectors (contents of BlockAnalysis), per block B:
    antLoc[L]     : There exists a store to L in B which is anticipated at the
                    start of B---and thus can be moved there, without changing
                    the meaning of the program.
    avlLoc[L]     : There exists a store to L in B which is available at the
                    end of B---and thus can be moved there, without changing
                    the meaning of the program.
    delLoc[L]     : There exists a store to L in B which could be killed if it
                    would be redundant at the end of B (even though it may not
                    be legal to move it there).

    alteredAnt[L] : B contains a possible use of L.  This prevents a store to L
                    from being killed by an otherwise-redundant store in a
                    later block.  (It also prevents stores from being moved
                    forward through this block.)
    alteredAvl[L] : B contains a possible use or store of L.  This prevents a
                    store to L from being moved through this block, but stores
                    might still be allowed to be eliminated due to redundancy
                    with later stores (unless, of course, alteredAnt[L] holds).

  Global bitvectors:
    Anticipated: (backward walk, initial 1s except in exit blocks)
    AntIn = AntLoc | (AntOut & ~alteredAnt);
    AntOut = Product(succs, AntIn(s))

    Partially Anticipated: (backward walk, initial 0s)
    PAntIn = AntLoc | (PAntOut & ~alteredAnt)
    PAntOut = Union(succs, PAntIn(s))

    Available: (forward walk, initial 1s except in entry block)
    AvlIn = Product(preds, AvlOut(p))
    AvlOut = AvlLoc | (AvlIn & ~alteredAvl);

    Placement Possible: (forward walk, initialized to AvlIn except entry, and
                         AvlOut except exit block)
    PPIn = Product(preds, PPOut(p))
    PPOut = PAntOut & (AvlLoc | (PPIn & ~alteredAvl)) &
            Product(succs, PPIn(s) | AntIn(s))

      Roughly speaking, at any given block, we're interested in the stores that
      are PAntOut, because they're redundant on at least one path from this
      block to the exit of the region.  We can place a store at the end of a
      block if it's AvlLoc (by definition) or if it's PPIn and "transparent" to
      the block; but since we're actually going to be inserting at the start of
      blocks, there's no point marking a block PPOut unless the store is PPIn
      or it's redundant in each of the following blocks.

      Note that we could use 1s instead of AvlIn/AvlOut, but convergence would
      be slower, and we would still need to deal with stores where the
      bitvectors say they're available, but we can't construct a suitable
      store).

    Insert = PPIn & ~AntIn & (~PPOut | alteredAvl);
      We want to insert if it's possible, and the store is not redundant, and we
      can't push the store any later.

    Delete = (AntOut & DelLoc) | (PPOut & AvlLoc);
      AntOut & DelLoc means it's redundant; we'll just kill it.
      PPOut => PPIn | AntIn in each successor, meaning that for each successor
      (recursively) we can either push the store into that successor, or it's
      redundant there.
*/

namespace HPHP { namespace jit {

TRACE_SET_MOD(hhir_store);

namespace {

//////////////////////////////////////////////////////////////////////

using PostOrderId = uint32_t;

/*
  TrackedStore is used to keep track of which stores are available at
  the beginning and end of each Block.
  Its value can be
    - Unseen (either there is no store available, or we
      haven't processed it yet),
    - Instruction, in which case it holds a pointer to the
      store which is available there,
    - Phi..Phi+kMaxTrackedAlocs, in which case it holds a pointer to the
      Block where the phi would have to be inserted, or
    - Bad which means that although stores are available on all
      paths to this point they're not compatible in some way.

    - Pending and Processed are used while building phis to handle
      cycles of phis.

  For the Phi case, we just need to ensure that we can differentiate
  Phis in the same block for different ALocations; this is just used
  for the same() method for the combine_ts().
*/
struct TrackedStore {
  enum Kind : int16_t {
    Unseen,
    Instruction,
    Phi,
    Bad = Phi + kMaxTrackedALocs,
    Pending,
    Processed,
  };

  TrackedStore() = default;
  TrackedStore(const TrackedStore&) = default;
  explicit TrackedStore(IRInstruction* i) { m_ptr.set(Instruction, i); }
  explicit TrackedStore(Block* b, uint32_t id) {
    m_ptr.set(static_cast<Kind>(Phi + id), b);
  }

  static TrackedStore BadVal() { TrackedStore s; s.setBad(); return s; }
  const IRInstruction* instruction() const {
    return kind() == Instruction ?
      static_cast<const IRInstruction*>(m_ptr.ptr()) : nullptr;
  }
  IRInstruction* instruction() {
    return kind() == Instruction ?
      static_cast<IRInstruction*>(m_ptr.ptr()) : nullptr;
  }
  const Block* block() const {
    return isPhi() ? static_cast<const Block*>(m_ptr.ptr()) : nullptr;
  }
  Block* block() {
    return isPhi() ? static_cast<Block*>(m_ptr.ptr()) : nullptr;
  }
  Block* pending() {
    return kind() == Pending ? static_cast<Block*>(m_ptr.ptr()) : nullptr;
  }
  const Block* pending() const {
    return kind() == Pending ? static_cast<const Block*>(m_ptr.ptr()) : nullptr;
  }
  IRInstruction* processed() {
    return kind() == Processed ?
      static_cast<IRInstruction*>(m_ptr.ptr()) : nullptr;
  }
  const IRInstruction* processed() const {
    return kind() == Processed ?
      static_cast<const IRInstruction*>(m_ptr.ptr()) : nullptr;
  }
  bool isUnseen() const {
    return kind() == Unseen;
  }
  bool isBad() const {
    return kind() == Bad;
  }
  bool isPhi() const {
    return kind() >= Phi && kind() < Bad;
  }
  void set(IRInstruction* inst) { m_ptr.set(Instruction, inst); }
  void set(Block* block, uint32_t id) {
    m_ptr.set(static_cast<Kind>(Phi + id), block);
  }
  void setPending(Block* block) { m_ptr.set(Pending, block); }
  void setProcessed(IRInstruction* inst) { m_ptr.set(Processed, inst); }
  void reset() { m_ptr.set(Unseen, nullptr); }
  void setBad() { m_ptr.set(Bad, nullptr); }
  bool same(const TrackedStore& other) const {
    return kind() == other.kind() && m_ptr.ptr() == other.m_ptr.ptr();
  }
  friend DEBUG_ONLY bool operator>=(const TrackedStore& a,
                                    const TrackedStore& b) {
    return a.kind() >= b.kind();
  }
  std::string toString() const {
    if (isUnseen()) return "Unseen";
    if (isBad()) return "Bad";
    char* type;
    if (instruction()) type = "Inst";
    else if (isPhi()) type = "Phi";
    else if (pending()) type = "Pending";
    else if (processed()) type = "Processed";
    else type = "XXX";
    return folly::sformat("{}:0x{:x}",type, uintptr_t(m_ptr.ptr()));
  }
 private:
  Kind kind() const { return m_ptr.tag(); }
  CompactTaggedPtr<void, Kind> m_ptr;
};

struct StoreKey {
  enum Where { In, Out };
  StoreKey(uint32_t id, Where w, uint32_t aId) :
      blkId(id), where(w), alocId(aId) {}
  StoreKey(const Block* blk, Where w, uint32_t aId) :
      blkId(blk->id()), where(w), alocId(aId) {}
  StoreKey(const Block& blk, Where w, uint32_t aId) :
      blkId(blk.id()), where(w), alocId(aId) {}

  uint32_t blkId;
  Where    where;
  uint32_t alocId;
};

struct StoreKeyHashCmp {
  size_t operator()(const StoreKey& k) const {
    return (k.blkId * 2 + (int)k.where) * kMaxTrackedALocs + k.alocId;
  }
  size_t operator()(const StoreKey& k1, const StoreKey& k2) const {
    return
      k1.blkId == k2.blkId &&
      k1.where == k2.where &&
      k1.alocId == k2.alocId;
  }
  static uint32_t size(uint32_t numBlocks) {
    return (numBlocks + 1) * 2 * kMaxTrackedALocs;
  }
};

using MovableStoreMap = hphp_hash_map<StoreKey, TrackedStore,
                                      StoreKeyHashCmp, StoreKeyHashCmp>;

struct BlockState {
  PostOrderId id;
  ALocBits antIn;
  ALocBits antOut;
  ALocBits pAntIn;
  ALocBits pAntOut;
  ALocBits ppIn;
  ALocBits ppOut;
};

// Environment for the whole optimization pass.
struct Global {
  explicit Global(IRUnit& unit)
    : unit(unit)
    , poBlockList(poSortCfg(unit))
    , ainfo(collect_aliases(unit, poBlockList))
    , blockStates(unit, BlockState{})
    , seenStores(unit, 0)
  {}

  IRUnit& unit;
  BlockList poBlockList;
  AliasAnalysis ainfo;

  /*
   * Keep a mapping from DefLabel blocks, to the Alocs of stores that
   * might depend on them.  Normally we don't need to worry about
   * this, because ssa guarantees that a store is dominated by the def
   * of its address. But in a loop, if the store's address depends on
   * a phi, the store might be partially available at the DefLabel.
   */
  hphp_fast_map<Block*,ALocBits> blk2Aloc;

  MovableStoreMap trackedStoreMap;
  // Block states are indexed by block->id().  These are only meaningful after
  // we do dataflow.
  StateVector<Block,BlockState> blockStates;
  jit::vector<IRInstruction*> reStores;
  // Used to prevent cycles in find_candidate_store
  StateVector<Block,uint32_t> seenStores;
  uint32_t seenStoreId{0};
  bool needsReflow{false};
  bool adjustedInlineCalls{false};

  // We can't safely remove InlineReturn instructions, once we've moved or
  // killed InlineCall instructions we handle InlineReturns in a final dataflow
  // analysis that tracks inline frames.
  IdSet<IRInstruction> deadInlineReturns;
};

// Block-local environment.
struct Local {
  explicit Local(Global& global)
    : global(global)
  {}

  Global& global;

  ALocBits antLoc;     // Copied to BlockAnalysis::antLoc
  ALocBits mayLoad;    // Things that may be read in the block
  ALocBits mayStore;   // Things that may be written in the block
  ALocBits avlLoc;     // Copied to BlockAnalysis::avlLoc
  ALocBits delLoc;     // Copied to BlockAnalysis::delLoc

  ALocBits reStores;

  bool containsCall{false}; // If there's a Call instruction in this block
};

//////////////////////////////////////////////////////////////////////

using jit::show;

const char* show(StoreKey::Where w) {
  switch (w) {
    case StoreKey::In:  return "In";
    case StoreKey::Out: return "Out";
  }
  not_reached();
}

std::string show(TrackedStore ts) {
  if (ts.isUnseen()) return "U";
  if (ts.isBad()) return "B";
  if (auto i = ts.instruction()) return folly::sformat("I{}", i->id());
  if (auto i = ts.processed()) return folly::sformat("I*{}", i->id());
  if (auto b = ts.block()) return folly::sformat("P{}", b->id());
  if (auto b = ts.pending()) return folly::sformat("P*{}", b->id());
  not_reached();
}

bool srcsCanSpanCall(const IRInstruction& inst) {
  if (inst.is(StFrameCtx)) return true;
  for (auto i = inst.numSrcs(); i--; ) {
    auto const src = inst.src(i);
    if (!src->isA(TStkPtr) &&
        !src->isA(TFramePtr) &&
        !src->inst()->is(DefConst)) return false;
  }
  return true;
}

folly::Optional<uint32_t> pure_store_bit(Local& env, AliasClass acls) {
  if (auto const meta = env.global.ainfo.find(canonicalize(acls))) {
    return meta->index;
  }
  return folly::none;
}

void set_movable_store(Local& env, uint32_t bit, IRInstruction& inst) {
  env.global.trackedStoreMap[
    StoreKey { inst.block(), StoreKey::Out, bit }].set(&inst);
}

bool isDead(Local& env, int bit) {
  return env.antLoc[bit];
}

bool isDeadSet(Local& env, const ALocBits& bits) {
  return (~env.antLoc & bits).none();
}

bool removeDead(Local& env, IRInstruction& inst, bool trash) {
  BISECTOR(store_delete);
  if (!store_delete.go()) return false;

  FTRACE(4, "      dead (removed)\n");

  IRInstruction* dbgInst = nullptr;
  if (trash && RuntimeOption::EvalHHIRGenerateAsserts) {
    switch (inst.op()) {
    case StStk:
      dbgInst = env.global.unit.gen(
        DbgTrashStk,
        inst.bcctx(),
        IRSPRelOffsetData { inst.extra<StStk>()->offset },
        inst.src(0)
      );
      break;
    case StMem:
      dbgInst = env.global.unit.gen(DbgTrashMem, inst.bcctx(), inst.src(0));
      break;
    default:
      dbgInst = nullptr;
      break;
    }
  }

  if (inst.is(InlineCall)) env.global.adjustedInlineCalls = true;

  auto block = inst.block();
  auto pos = block->erase(&inst);
  if (dbgInst) {
    block->insert(pos, dbgInst);
    return false;
  }
  return true;
}

void addLoadSet(Local& env, const ALocBits& bits) {
  FTRACE(4, "           load: {}\n", show(bits));
  env.mayLoad |= bits;
  env.antLoc &= ~bits;
}

void addAllLoad(Local& env) {
  FTRACE(4, "           load: -1\n");
  env.mayLoad.set();
  env.antLoc.reset();
}

void mustStore(Local& env, int bit) {
  FTRACE(4, "      mustStore: {}\n", bit);
  if (!env.antLoc[bit] &&
      !env.mayLoad[bit]) {
    env.delLoc[bit] = 1;
  }
  env.antLoc[bit] = 1;
  env.mayStore[bit] = 0;
  env.reStores[bit] = 0;
}

void mustStoreSet(Local& env, const ALocBits& bits) {
  FTRACE(4, "      mustStore: {}\n", show(bits));
  env.delLoc |= bits & ~(env.antLoc | env.mayLoad);
  env.antLoc |= bits;
  env.mayStore &= ~bits;
  env.reStores &= ~bits;
}

void killSet(Local& env, const ALocBits& bits) {
  FTRACE(4, "           kill: {}\n", show(bits));
  env.mayLoad &= ~bits;
  mustStoreSet(env, bits);
}

//////////////////////////////////////////////////////////////////////

void load(Local& env, AliasClass acls) {
  addLoadSet(env, env.global.ainfo.may_alias(canonicalize(acls)));
}

void mayStore(Local& env, AliasClass acls) {
  auto const canon = canonicalize(acls);
  auto const mayStore = env.global.ainfo.may_alias(canon);
  FTRACE(4, "       mayStore: {}\n", show(mayStore));
  env.mayStore |= mayStore;
  env.reStores &= ~mayStore;
}

void store(Local& env, AliasClass acls) {
  mayStore(env, acls);
  if (auto bit = pure_store_bit(env, acls)) {
    mustStore(env, *bit);
  } else {
    auto const it = env.global.ainfo.stack_ranges.find(canonicalize(acls));
    if (it != end(env.global.ainfo.stack_ranges)) {
      mustStoreSet(env, it->second);
    }
  }
};

void kill(Local& env, AliasClass acls) {
  auto const canon = canonicalize(acls);
  killSet(env, env.global.ainfo.expand(canon));
}

//////////////////////////////////////////////////////////////////////

void addPhiDeps(Local& env, uint32_t bit, SSATmp* dep) {
  if (dep->inst()->is(DefLabel) || dep->inst()->producesReference()) {
    FTRACE(2, "      B{} alters {} due to potential address change\n",
           dep->inst()->block()->id(), bit);
    env.global.blk2Aloc[dep->inst()->block()][bit] = 1;
    return;
  }

  for (auto src : dep->inst()->srcs()) {
    addPhiDeps(env, bit, src);
  }
}

//////////////////////////////////////////////////////////////////////

void visit(Local& env, IRInstruction& inst) {
  auto const effects = memory_effects(inst);
  FTRACE(3, "    {}\n"
         "      {}\n",
         inst.toString(),
         show(effects));

  auto const doPureStore = [&] (PureStore l) {
    if (auto bit = pure_store_bit(env, l.dst)) {
      if (l.dep) addPhiDeps(env, *bit, l.dep);
      if (isDead(env, *bit)) {
        if (!removeDead(env, inst, true)) {
          mayStore(env, l.dst);
          mustStore(env, *bit);
        }
        return;
      }
      if (!env.antLoc[*bit] &&
          !env.mayLoad[*bit] &&
          !env.mayStore[*bit] &&
          (!env.containsCall ||
           srcsCanSpanCall(inst))) {
        env.avlLoc[*bit] = 1;
        set_movable_store(env, *bit, inst);
      }
      mayStore(env, l.dst);
      mustStore(env, *bit);
      if (!l.value || l.value->inst()->block() != inst.block()) return;
      auto const le = memory_effects(*l.value->inst());
      auto pl = boost::get<PureLoad>(&le);
      if (!pl) return;
      auto lbit = pure_store_bit(env, pl->src);
      if (!lbit || *lbit != *bit) return;
      /*
       * The source of the store is a load from the same address, which is
       * also in this block. If there's no interference, we can kill this
       * store. We won't know until we see the load though.
       */
      if (env.global.reStores.size() <= *bit) {
        env.global.reStores.resize(*bit + 1);
      }
      env.global.reStores[*bit] = &inst;
      env.reStores[*bit] = 1;
      return;
    }
    mayStore(env, l.dst);
  };

  match<void>(
    effects,
    [&] (IrrelevantEffects) {
      switch (inst.op()) {
      case AssertLoc:
        load(env, ALocal { inst.src(0), inst.extra<AssertLoc>()->locId });
        return;
      case AssertStk:
        load(env, AStack { inst.src(0), inst.extra<AssertStk>()->offset, 1 });
        return;
      default:
        return;
      }
    },
    [&] (UnknownEffects)    { addAllLoad(env); env.mayStore.set(); },
    [&] (PureLoad l) {
      if (auto bit = pure_store_bit(env, l.src)) {
        if (env.reStores[*bit]) {
          auto const st = memory_effects(*env.global.reStores[*bit]);
          auto const pst = boost::get<PureStore>(&st);
          if (pst && pst->value && pst->value == inst.dst()) {
            FTRACE(4, "Killing self-store: {}\n",
                   env.global.reStores[*bit]->toString());
            removeDead(env, *env.global.reStores[*bit], false);
            env.reStores[*bit] = 0;
          }
        }
      }
      load(env, l.src);
    },
    [&] (GeneralEffects l)  {
      load(env, l.loads);
      mayStore(env, l.stores);
      kill(env, l.kills);
    },

    [&] (ReturnEffects l) {
      // Return from the main function.  Locations other than the frame and
      // stack (e.g. object properties and whatnot) are always live on a
      // function return---so mark everything read before we start killing
      // things.
      addAllLoad(env);
      killSet(env, env.global.ainfo.all_local);
      kill(env, l.kills);
    },

    [&] (ExitEffects l) {
      load(env, l.live);
      kill(env, l.kills);
    },

    [&] (PureInlineCall l) {
      doPureStore(PureStore { l.base, l.fp });
      load(env, l.actrec);
    },

    [&] (PureInlineReturn l) {
      if (auto bit = pure_store_bit(env, l.base)) {
        if (isDead(env, *bit)) env.global.deadInlineReturns.add(inst);
        mayStore(env, l.base);
        mustStore(env, *bit);
        return;
      }
      mayStore(env, l.base);
    },

    /*
     * Call instructions can potentially read any heap location, but we can be
     * more precise about everything else.
     */
    [&] (CallEffects l) {
      env.containsCall = true;

      store(env, l.outputs);
      load(env, AHeapAny);
      load(env, l.locals);
      load(env, l.inputs);
      store(env, l.actrec);
      kill(env, l.kills);
    },

    [&] (PureStore l) { doPureStore(l); }
  );
}

void block_visit(Local& env, Block* block) {
  FTRACE(2, "  visiting B{}\n", block->id());
  assertx(!block->instrs().empty());

  auto prev = std::prev(block->instrs().end());
  for (;;) {
    auto it = prev;
    if (it != block->instrs().begin()) {
      prev = std::prev(it);
      visit(env, *it);
    } else {
      visit(env, *it);
      break;
    }
  }
}

//////////////////////////////////////////////////////////////////////

struct BlockAnalysis {
  ALocBits antLoc;
  ALocBits alteredAnt;
  ALocBits alteredAvl;
  ALocBits avlLoc;
  ALocBits delLoc;
  bool containsCall;
};

BlockAnalysis analyze_block(Global& genv, Block* block) {
  auto env = Local { genv };
  env.antLoc.reset(); // During this first analysis pass, we need to pretend
                      // none of the stores is anticipated (because we don't yet
                      // know). We may still remove some stores during this
                      // visit if they are locally proven dead, however.
  block_visit(env, block);
  return BlockAnalysis {
    env.antLoc,
    env.mayLoad,
    // Note that we unset must-store bits in env.mayStore, so including
    // env.antLoc and env.delLoc is required for correctness here.
    env.mayLoad | env.mayStore | env.antLoc | env.delLoc,
    env.avlLoc,
    env.delLoc,
    env.containsCall
  };
}

void find_all_stores(Global& genv, Block* blk, uint32_t id,
                     jit::vector<IRInstruction*>& stores,
                     jit::hash_set<void*>& seen) {
  ITRACE(7, "find_all_stores: {} B{}\n", id, blk->id());
  Trace::Indent _i;
  blk->forEachPred(
    [&](Block* pred) {
      if (!seen.insert(pred).second) {
        ITRACE(7, "find_all_stores: {} B{} skipping pred B{}\n",
               id, blk->id(), pred->id());
        return;
      }
      ITRACE(7, "find_all_stores: {} B{} processing pred B{}\n",
             id, blk->id(), pred->id());
      auto& pst =
        genv.trackedStoreMap[StoreKey { pred, StoreKey::Out, id }];
      IRInstruction* inst;
      if ((inst = pst.instruction()) != nullptr ||
          (inst = pst.processed()) != nullptr) {
        if (seen.insert(inst).second) {
          ITRACE(7, "find_all_stores: {} B{} pred B{}: adding {}\n",
                 id, blk->id(), pred->id(), inst->toString());
          stores.push_back(inst);
        } else {
          ITRACE(7, "find_all_stores: {} B{} pred B{}: dropping {}\n",
                 id, blk->id(), pred->id(), inst->toString());
        }
        return;
      }
      Block* b;
      if ((b = pst.block()) != nullptr ||
          (b = pst.pending()) != nullptr) {
        if (b != pred && seen.count(b)) {
          ITRACE(7,
                 "find_all_stores: {} B{} pred B{} previously processed B{}\n",
                 id, blk->id(), pred->id(), b->id());
          return;
        }
        ITRACE(7, "find_all_stores: {} B{} pred B{} recur to B{}\n",
               id, blk->id(), pred->id(), b->id());
        return find_all_stores(genv, b, id, stores, seen);
      }
      always_assert(false);
    }
  );
}

IRInstruction* resolve_ts(Global& genv, Block* blk,
                          StoreKey::Where w, uint32_t id);

void resolve_cycle(Global& genv, TrackedStore& ts, Block* blk, uint32_t id) {
  genv.needsReflow = true;
  jit::vector<IRInstruction*> stores;
  jit::hash_set<void*> seen;
  // find all the stores, so we can determine
  // whether a phi is actually required for each
  // src (also, we need a candidate store to clone)

  seen.insert(blk);

  ITRACE(7, "resolve_cycle - store id {}:\n", id);
  Trace::Indent _i;

  find_all_stores(genv, blk, id, stores, seen);
  always_assert(stores.size() > 0);
  if (Trace::moduleEnabled(TRACEMOD, 7)) {
    for (auto const DEBUG_ONLY st : stores) {
      ITRACE(7, "  - {}\n", st->toString());
    }
  }
  auto const cand = stores[0];
  if (stores.size() == 1) {
    ts.set(cand);
    return;
  }
  jit::vector<uint32_t> srcsToPhi;
  for (uint32_t i = 0; i < cand->numSrcs(); i++) {
    SSATmp* prev = nullptr;
    for (auto const st : stores) {
      auto const si = st->src(i);
      if (prev && prev != si) {
        srcsToPhi.push_back(i);
        break;
      }
      prev = si;
    }
  }
  if (!srcsToPhi.size()) {
    // the various stores all had the same inputs
    // so nothing to do.
    ts.set(cand);
    return;
  }
  bool needsProcessed = false;
  auto const inst = genv.unit.clone(cand);
  for (auto i : srcsToPhi) {
    auto t = TBottom;
    for (auto const st : stores) {
      t |= st->src(i)->type();
    }
    if (t.admitsSingleVal()) {
      inst->setSrc(i, genv.unit.cns(t));
      continue;
    }
    needsProcessed = true;
    // create a Mov; we'll use its dst as the src of the store, and
    // when we eventually create the phi, we'll set its dst as the src
    // of the Mov (this allows us to avoid creating a new phi if
    // there's already a suitable one there). We also set the Mov's
    // src to be its dst, so that we can identify it as needing to be
    // fixed up in resolve_flat.
    auto mv = genv.unit.gen(Mov, blk->front().bcctx(), cand->src(i));
    blk->prepend(mv);
    inst->setSrc(i, mv->dst());
    mv->setSrc(0, mv->dst());
    mv->dst()->setType(t);
    ITRACE(7, "  + created {} for {}\n", mv->toString(), inst->toString());
  }
  if (needsProcessed) {
    ts.setProcessed(inst);
  } else {
    ts.set(inst);
  }
}

IRInstruction* resolve_flat(Global& genv, Block* blk, uint32_t id,
                            TrackedStore& ts) {
  ts.setPending(blk);

  jit::vector<IRInstruction*> stores;
  blk->forEachPred([&](Block* pred) {
      stores.push_back(resolve_ts(genv, pred, StoreKey::Out, id));
    });
  always_assert(stores.size() > 0);
  if (auto const rep = ts.instruction()) {
    ITRACE(7, "resolve_flat: returning {}\n", rep->toString());
    return rep;
  }

  auto const cand = stores[0];
  if (stores.size() == 1) return cand;
  assertx(blk->numPreds() == stores.size());
  auto& preds = blk->preds();
  jit::vector<SSATmp*> newSrcs;
  jit::vector<uint32_t> srcsToPhi;
  for (uint32_t i = 0; i < cand->numSrcs(); i++) {
    bool same = true;
    SSATmp* temp = nullptr;
    jit::hash_map<Block*, SSATmp*> phiInputs;
    uint32_t j = 0;
    for (auto const& edge : preds) {
      auto const st = stores[j++];
      auto si = st->src(i);
      if (!si->inst()->is(DefConst) && si->type().admitsSingleVal()) {
        si = genv.unit.cns(si->type());
      }
      phiInputs[edge.from()] = si;
      if (temp == nullptr) temp = si;
      if (si != temp) same = false;
    }
    if (!same) {
      srcsToPhi.push_back(i);
      newSrcs.push_back(insertPhi(genv.unit, blk, phiInputs));
    } else if (ts.processed()) {
      // even if we don't need a phi, resolve_cycle might have thought
      // we did; if so, we still need to fix up the input.
      auto const mv = ts.processed()->src(i)->inst();
      if (mv->is(Mov) && mv->src(0) == mv->dst()) {
        srcsToPhi.push_back(i);
        newSrcs.push_back(temp);
      }
    }
  }

  if (auto rep = ts.processed()) {
    if (Trace::moduleEnabled(TRACEMOD, 7)) {
      ITRACE(7, "resolve_flat: fixing {}\n", rep->toString());
      for (auto const DEBUG_ONLY st : stores) {
        ITRACE(7, "    - {}\n", st->toString());
      }
    }
    // the replacement was constructed during the recursive
    // walk. Just need to hook up the new phis.
    for (uint32_t ix = 0; ix < srcsToPhi.size(); ix++) {
      auto i = srcsToPhi[ix];
      auto src = rep->src(i);
      auto mv = src->inst();
      always_assert(mv->is(Mov) && mv->src(0) == mv->dst());
      mv->setSrc(0, newSrcs[ix]);
      retypeDests(mv, &genv.unit);
    }
    return rep;
  }

  auto rep = genv.unit.clone(cand);
  for (uint32_t ix = 0; ix < srcsToPhi.size(); ix++) {
    auto i = srcsToPhi[ix];
    rep->setSrc(i, newSrcs[ix]);
  }

  return rep;
}

IRInstruction* resolve_ts(Global& genv, Block* blk,
                          StoreKey::Where w, uint32_t id) {
  auto& ts = genv.trackedStoreMap[StoreKey { blk, w, id }];

  ITRACE(7, "resolve_ts: B{}:{} store:{} ts:{}\n",
         blk->id(), show(w), id, show(ts));
  Trace::Indent _i;

  if (ts.pending()) {
    always_assert(ts.pending() == blk);
    resolve_cycle(genv, ts, blk, id);
    assertx(ts.instruction() || ts.processed());
  }

  if (auto inst = ts.instruction()) {
    ITRACE(7, "-> inst: {}\n", inst->toString());
    return inst;
  }
  if (auto inst = ts.processed()) {
    ITRACE(7, "-> proc: {}\n", inst->toString());
    return inst;
  }

  always_assert(ts.block());
  if (w != StoreKey::In || blk != ts.block()) {
    ITRACE(7, "direct recur: B{}:{} -> B{}:In\n",
           blk->id(), show(w), ts.block()->id());
    ts.set(resolve_ts(genv, ts.block(), StoreKey::In, id));
  } else {
    ts.set(resolve_flat(genv, blk, id, ts));
  }
  auto const rep = ts.instruction();

  ITRACE(7, "-> {} (resolve_ts B{}, store {})\n",
         rep->toString(), blk->id(), id);
  return rep;
}

void optimize_block_pre(Global& genv, Block* block,
                        const jit::vector<BlockAnalysis>& blockAnalysis) {
  auto& state = genv.blockStates[block];
  auto const& transfer = blockAnalysis[state.id];
  auto insertBits = state.ppIn & ~state.antIn &
    (~state.ppOut | transfer.alteredAvl);
  auto const deleteBits =
      (state.antOut & transfer.delLoc) | (state.ppOut & transfer.avlLoc);

  if (deleteBits.any()) {
    FTRACE(1,
           "Optimize B{}:\n"
           "  Delete: {}\n",
           block->id(),
           show(deleteBits));

    auto env = Local { genv };
    env.antLoc = deleteBits;
    block_visit(env, block);
  }
  if (insertBits.any()) {
    // warning: if you turn off any stores using this bisector, you
    // need to disable the corresponding deletes (or simpler, all
    // deletes) using store_delete.
    BISECTOR(store_insert);
    FTRACE(1,
           "Optimize B{}:\n"
           "  Insert: {}\n",
           block->id(),
           show(insertBits));

    bitset_for_each_set(
      insertBits,
      [&](size_t i){
        if (!store_insert.go()) return;
        auto const cinst = resolve_ts(genv, block, StoreKey::In, i);
        FTRACE(1, " Inserting store {}: {}\n", i, cinst->toString());
        auto const inst = genv.unit.clone(cinst);
        block->prepend(inst);
        if (inst->is(InlineCall)) genv.adjustedInlineCalls = true;
      }
    );
  }
}

void optimize_block(Global& genv, Block* block) {
  auto& state = genv.blockStates[block];
  auto env = Local { genv };
  FTRACE(1,
         "Optimize B{}:\n"
         "  antOut: {}\n",
         block->id(),
         show(state.antOut));
  env.antLoc = state.antOut;
  block_visit(env, block);
}

enum class Direction { Forward, Backward };
template <Direction dir, typename IFunc, typename UFunc, typename RFunc>
void compute_dataflow(Global& genv,
                      const jit::vector<BlockAnalysis>& blockAnalysis,
                      IFunc init,
                      UFunc update,
                      RFunc reschedule) {
  using sorter = typename std::conditional<dir == Direction::Forward,
                                           std::less<PostOrderId>,
                                           std::greater<PostOrderId>>::type;

  auto incompleteQ =
    dataflow_worklist<PostOrderId, sorter>(genv.unit.numBlocks());

  for (auto poId = uint32_t{0}; poId < genv.poBlockList.size(); ++poId) {
    auto const blk = genv.poBlockList[poId];
    auto& state = genv.blockStates[blk];
    init(blk, state);
    incompleteQ.push(poId);
  }

  while (!incompleteQ.empty()) {
    auto const poId = incompleteQ.pop();
    auto const blk  = genv.poBlockList[poId];

    auto& state          = genv.blockStates[blk];
    auto const& transfer = blockAnalysis[poId];
    assertx(state.id == poId);

    if (!update(blk, transfer, state)) continue;

    /*
     * Update the predecessors / successors if things change
     */
    auto doblock = [&] (Block* other) {
      FTRACE(4, "   -> {}\n", other->id());
      auto& otherState = genv.blockStates[other];

      if (reschedule(state, otherState, incompleteQ)) {
        incompleteQ.push(otherState.id);
      }
    };
    if (dir == Direction::Forward) {
      blk->forEachSucc(doblock);
    } else {
      blk->forEachPred(doblock);
    }
  }
}

/*
 * Iterate on the antIn/antOut states until we reach a fixed point.
 */
void compute_anticipated(Global& genv,
                         const jit::vector<BlockAnalysis>& blockAnalysis) {
  FTRACE(4, "Iterating Anticipated\n");
  compute_dataflow<Direction::Backward>(
    genv, blockAnalysis,
    /*
     * Important note: every block starts with a full antOut set,
     * including blocks that are exiting the region.  When an HHIR
     * region is exited, there's always some instruction we can use
     * to indicate via memory_effects what may be read (e.g.
     * EndCatch, RetCtrl, ReqBindJmp, etc).  When we start iterating,
     * we'll appropriately mark things as read based on these.
     */
    [](Block*, BlockState& state) { state.antOut.set(); },
    [](Block* blk, const BlockAnalysis& transfer, BlockState& state) {
      state.antIn = transfer.antLoc |
        (state.antOut & ~transfer.alteredAnt);
      state.pAntIn = transfer.antLoc |
        (state.pAntOut & ~transfer.alteredAnt);
      FTRACE(4,
             "  block B{}\n"
             "    antIn   : {}\n"
             "    pAntIn  : {}\n"
             "    antLoc  : {}\n"
             "    altered : {}\n"
             "    antOut  : {}\n"
             "    pAntOut : {}\n",
             blk->id(),
             show(state.antIn),
             show(state.pAntIn),
             show(transfer.antLoc),
             show(transfer.alteredAnt),
             show(state.antOut),
             show(state.pAntOut));
      return true;
    },
    [](const BlockState& state, BlockState& pred,
       dataflow_worklist<PostOrderId>&) {
      auto const oldAntOut = pred.antOut;
      auto const oldPAntOut = pred.pAntOut;
      pred.antOut &= state.antIn;
      pred.pAntOut |= state.pAntIn;
      return pred.antOut != oldAntOut || pred.pAntOut != oldPAntOut;
    });
}

TrackedStore find_candidate_store_helper(Global& genv, TrackedStore ts,
                                         uint32_t id) {
  auto block = ts.block();
  assertx(block);
  TrackedStore ret;
  if (genv.seenStores[block] == genv.seenStoreId) return ret;

  // look for a candidate in the immediate predecessors
  block->forEachPred([&](Block* pred) {
      if (ret.isBad() || ret.instruction()) return;
      auto const pred_ts =
        genv.trackedStoreMap[StoreKey { pred, StoreKey::Out, id }];
      if (pred_ts.isBad() || pred_ts.instruction()) {
        ret = pred_ts;
        return;
      }
    });

  if (ret.isBad() || ret.instruction()) return ret;

  genv.seenStores[block] = genv.seenStoreId;
  // recursively search the predecessors
  block->forEachPred([&](Block* pred) {
      if (ret.isBad() || ret.instruction()) return;
      auto const pred_ts =
        genv.trackedStoreMap[StoreKey { pred, StoreKey::Out, id }];
      if (!pred_ts.block()) {
        always_assert_flog(pred_ts.isUnseen(),
                           "pred_ts: {}", pred_ts.toString());
        return;
      }
      ret = find_candidate_store_helper(genv, pred_ts, id);
    });

  return ret;
}

/*
 * Find a candidate store instruction; any one will do, because
 * compatibility between stores is transitive.
 */
TrackedStore find_candidate_store(Global& genv, TrackedStore ts, uint32_t id) {
  if (!ts.block()) return ts;
  genv.seenStoreId++;
  return find_candidate_store_helper(genv, ts, id);
}

TrackedStore combine_ts(Global& genv, uint32_t id,
                        TrackedStore s1,
                        TrackedStore s2, Block* succ) {
  if (s1.same(s2)) return s1;
  if (s1.isUnseen() || s2.isBad()) return s2;
  if (s2.isUnseen() || s1.isBad()) return s1;

  enum class Compat { Same, Compat, Bad };
  auto compat = [](TrackedStore store1, TrackedStore store2) {
    auto i1 = store1.instruction();
    auto i2 = store2.instruction();
    assertx(i1 && i2);
    if (i1->op() != i2->op()) return Compat::Bad;
    if (i1->numSrcs() != i2->numSrcs()) return Compat::Bad;
    for (auto i = i1->numSrcs(); i--; ) {
      // Ptr and Lval types are imcompatible, as one requires two register,
      // while the other requires only one.  This stops us from phiing the
      // two types together to elimnate a store.
      auto const& t1 = i1->src(i)->type();
      auto const& t2 = i2->src(i)->type();
      if ((t1.maybe(TPtrToCell) && t2.maybe(TLvalToCell)) ||
          (t2.maybe(TPtrToCell) && t1.maybe(TLvalToCell))) {
        return Compat::Bad;
      }
    }
    for (auto i = i1->numSrcs(); i--; ) {
      if (i1->src(i) != i2->src(i)) return Compat::Compat;
    }
    return Compat::Same;
  };

  auto trackedBlock = [&]() {
    return TrackedStore(succ, id);
  };

  if (s1.block() || s2.block()) {
    auto c1 = find_candidate_store(genv, s1, id);
    auto c2 = find_candidate_store(genv, s2, id);
    if (c1.instruction() && c2.instruction() &&
        compat(c1, c2) != Compat::Bad) {
      return trackedBlock();
    }
    return TrackedStore::BadVal();
  }

  switch (compat(s1, s2)) {
    case Compat::Same:   return s1;
    case Compat::Compat: return trackedBlock();
    case Compat::Bad:    break;
  }
  return TrackedStore::BadVal();
}

/*
 * Compute a TrackedStore for succ by merging its preds.
 */
TrackedStore recompute_ts(Global& genv, uint32_t id, Block* succ) {
  TrackedStore ret;
  succ->forEachPred([&](Block* pred) {
      auto const ts =
        genv.trackedStoreMap[StoreKey { pred, StoreKey::Out, id }];
      ret = combine_ts(genv, id, ts, ret, succ);
    });
  return ret;
}

/*
 * Compute a starting point for ppIn/ppOut based on global availability.
 * At the same time, compute exactly which stores are available at each
 * point. This is essentially the bitvector implementation of global
 * availability, but disallowing the cases where we can't find/create
 * a suitable store.
 */
void compute_available_stores(
  Global& genv, const jit::vector<BlockAnalysis>& blockAnalysis) {

  FTRACE(4, "\nIterating Available Stores\n");
  compute_dataflow<Direction::Forward>(
    genv, blockAnalysis,
    [](Block* blk, BlockState& state) {
      if (blk->numPreds()) state.ppIn.set();
    },
    [&](Block* blk, const BlockAnalysis& transfer, BlockState& state) {
      state.ppOut = transfer.avlLoc | (state.ppIn & ~transfer.alteredAvl);
      auto propagate = state.ppOut & ~transfer.avlLoc;
      bitset_for_each_set(
        propagate,
        [&](uint32_t i) {
          auto const& tsIn =
            genv.trackedStoreMap[StoreKey { blk, StoreKey::In, i }];
          auto& tsOut =
            genv.trackedStoreMap[StoreKey { blk, StoreKey::Out, i }];
          assertx(!tsIn.isUnseen());

          // Prevent tmps from spanning calls by not propagating tracked stores
          // thru any block which contains a call. The exception is if the store
          // only uses constants as those do not create a problem.
          if (transfer.containsCall &&
              (!tsIn.instruction() || !srcsCanSpanCall(*tsIn.instruction()))) {
            tsOut.setBad();
          } else {
            tsOut = tsIn;
          }

          if (tsOut.isBad()) {
            state.ppOut[i] = false;
          }
        }
      );
      auto showv DEBUG_ONLY = [&] (StoreKey::Where w, const ALocBits& pp) {
        std::string r;
        bitset_for_each_set(
          pp,
          [&](uint32_t i) {
            auto& ts = genv.trackedStoreMap[StoreKey { blk, w, i}];
            folly::format(&r, " {}:{}", i, show(ts));
          }
        );
        return r;
      };
      FTRACE(4,
             "  block B{}\n"
             "    ppIn     : {}\n"
             "    ppInV    : {}\n"
             "    avlLoc   : {}\n"
             "    altered  : {}\n"
             "    ppOut    : {}\n"
             "    ppOutV   : {}\n",
             blk->id(),
             show(state.ppIn),
             showv(StoreKey::In, state.ppIn),
             show(transfer.avlLoc),
             show(transfer.alteredAvl),
             show(state.ppOut),
             showv(StoreKey::Out, state.ppOut));
      return true;
    },
    [&](const BlockState& state, BlockState& succState,
        dataflow_worklist<PostOrderId, std::less<PostOrderId>>&) {
      auto const oldPpIn = succState.ppIn;
      succState.ppIn &= state.ppOut;
      bool changed = succState.ppIn != oldPpIn;
      auto blk = genv.poBlockList[state.id];
      auto succ = genv.poBlockList[succState.id];
      bitset_for_each_set(
        succState.ppIn,
        [&](uint32_t i) {
          auto& ts =
            genv.trackedStoreMap[StoreKey { blk, StoreKey::Out, i }];
          auto& tsSucc =
            genv.trackedStoreMap[StoreKey { succ, StoreKey::In, i }];
          auto tsNew = succ->numPreds() == 1 ? ts : recompute_ts(genv, i, succ);
          if (!tsNew.same(tsSucc)) {
            changed = true;
            assertx(tsNew >= tsSucc);
            tsSucc = tsNew;
            if (tsSucc.isBad()) {
              succState.ppIn[i] = false;
            }
          }
        }
      );
      return changed;
    });
}

/*
 * Iterate on the ppIn/ppOut states until we reach a fixed point.
 */
void compute_placement_possible(
  Global& genv, const jit::vector<BlockAnalysis>& blockAnalysis) {

  compute_available_stores(genv, blockAnalysis);

  FTRACE(4, "\nIterating Placement Possible\n");
  compute_dataflow<Direction::Forward>(
    genv, blockAnalysis,
    [](Block* blk, BlockState& state) {
      if (!blk->numPreds()) state.ppIn.reset();
      if (!blk->numSuccs()) state.ppOut.reset();
    },
    [&](Block* blk, const BlockAnalysis& transfer, BlockState& state) {
      auto trace = [&] () {
        FTRACE(4,
               "  block B{}\n"
               "    ppIn :  {}\n"
               "    avlLoc: {}\n"
               "    altLoc: {}\n"
               "    ppOut:  {}\n",
               blk->id(),
               show(state.ppIn),
               show(transfer.avlLoc),
               show(transfer.alteredAvl),
               show(state.ppOut));
      };

      if (!blk->numSuccs()) {
        trace();
        // ppOut is empty for a block with no succs (since placement
        // is definitely not possible in any of its successors), and
        // its initialized to empty, so nothing to do.
        //
        // Note that this is not just an optimization - the update
        // below would produce the wrong results.
        return false;
      }

      state.ppOut = transfer.avlLoc | (state.ppIn & ~transfer.alteredAvl);
      auto restrictBits = state.pAntOut;

      blk->forEachSucc([&] (Block* succ) {
          auto const& succState = genv.blockStates[succ];
          restrictBits &= succState.ppIn | succState.antIn;
        });

      state.ppOut &= restrictBits;
      trace();
      return true;
    },
    [&](const BlockState& state, BlockState& succState,
        dataflow_worklist<PostOrderId, std::less<PostOrderId>>& incompleteQ) {
      auto const oldPPIn = succState.ppIn;
      succState.ppIn &= state.ppOut;

      if (succState.ppIn == oldPPIn) return false;
      FTRACE(4, "   reprocessing succ {}\n",
             genv.poBlockList[succState.id]->id());
      auto const ssppa = succState.ppIn | succState.antIn;
      if (ssppa != (oldPPIn | succState.antIn)) {
        // this change might affect this successor's
        // predecessors; reschedule them if necessary
        auto succ = genv.poBlockList[succState.id];
        auto blk = genv.poBlockList[state.id];
        succ->forEachPred([&] (Block* pred) {
            if (pred == blk) return; // can't be affected
            auto const& psState = genv.blockStates[pred];
            if ((psState.ppOut & ssppa) != psState.ppOut) {
              FTRACE(4, "   reprocessing succ's pred {}\n", pred->id());
              incompleteQ.push(psState.id);
            }
          });
      }
      return true;
    });
}

void fix_inlined_call(Global& genv, IRInstruction* call, SSATmp* fp) {
  // Nothing to do if the frame hasn't changed.
  if (fp == call->src(1)) return;

  auto const origFp = call->src(1);
  auto const extra = call->extra<Call>();
  auto const callOffset = extra->callOffset;
  assertx(origFp->inst()->is(BeginInlining));

  // Adjust the fp and callOffset to reflect the caller frame for this call.
  auto const sk = call->marker().fixupSk();
  extra->callOffset = sk.offset() - sk.func()->base();
  call->setSrc(1, fp);

  // If we've already inserted a SyncReturnBC instruction there's no need
  // to insert any more.
  if (extra->hasInlFixup) return;
  extra->hasInlFixup = true;

  auto const catchBlock = call->taken();
  auto it = catchBlock->skipHeader();

  auto syncInst = genv.unit.gen(
    SyncReturnBC,
    it->bcctx(),
    SyncReturnBCData{callOffset, extra->spOffset + extra->numInputs()},
    call->src(0),
    origFp
  );
  catchBlock->insert(it, syncInst);
}

struct FPState {
  PC catchPC;      // PC at dominating BeginCatch or nullptr
  int exitDepth;   // number of live frames at end of block
  SSATmp* entry;   // live fp at start of block
  SSATmp* exit;    // live fp at end of block
  SSATmp* catchFP; // live fp at dominating BeginCatch or nullptr
};

void append_inline_returns(Global& genv, Block* b, SSATmp* start, SSATmp* end) {
  auto const it = b->backIter();
  for (; start != end; start = start->inst()->src(1)) {
    assertx(start->inst()->is(BeginInlining));

    auto const sInst = start->inst();
    auto const next = start->inst()->src(1);
    auto const nInst = next->inst();
    auto const startOff = sInst->extra<BeginInlining>()->spOffset.offset;
    auto const nextOff = [&] {
      if (next->inst()->is(BeginInlining)) {
        return nInst->extra<BeginInlining>()->spOffset;
      }
      auto const sp = sInst->src(0);
      assertx(sp->inst()->is(DefRegSP, DefFrameRelSP));

      // The IRSPRelOffset of `next` relative to `sp` is the same as the
      // FPInvOffset of `sp` relative to `next`.
      auto const off = sp->inst()->extra<FPInvOffsetData>()->offset;
      return IRSPRelOffset{off.offset};
    }().offset;

    /*
     * IRSPRel offsets get smaller the further down the stack they are, and
     * start will by definition always be below next. The offset computed by
     * nextOff - startOff will therefore be > 0. We want the FPRelOff, relative
     * to start of next. FPRel offsets are positive above the fp they are
     * are relative to ane negative below.
     *
     * +---------------------------+
     * |            ...            |
     * |                           |
     * +---------------------------+
     * |                           |
     * +---------------------------+ <- sp ----+--+
     * |                           |           |  |
     * +---------------------------+  nextOff >|  |
     * |                           |           |  |
     * +---------------------------+ <- next --+--|-+
     * |                           |              | |
     * +---------------------------+    startOff >| |< nextOff - startOff
     * |                           |              | |
     * +---------------------------+ <- start ----+-+
     * |                           |
     * |            ...            |
     * +---------------------------+
     */
    auto const callerFpOff = FPRelOffset{nextOff - startOff};

    auto returnInst = genv.unit.gen(
      InlineReturn,
      it->bcctx(),
      FPRelOffsetData { callerFpOff },
      start,
      next
    );
    b->insert(it, returnInst);
  }
}

void adjust_inline_marker(IRInstruction& inst, SSATmp* fp) {
  if (inst.marker().fp() == fp) return;
  if (!inst.mayRaiseError() && !inst.is(BeginCatch)) return;
  auto const curFp = inst.marker().fp();
  assertx(curFp->inst()->is(BeginInlining));
  auto const curOff = curFp->inst()->extra<BeginInlining>()->spOffset.offset;
  auto const newOff = [&] {
    if (fp->inst()->is(BeginInlining)) {
      return fp->inst()->extra<BeginInlining>()->spOffset.offset;
    }
    assertx(fp->inst()->is(DefFP, DefFuncEntryFP));
    auto const defSP = curFp->inst()->src(0)->inst();
    return defSP->extra<FPInvOffsetData>()->offset.offset;
  }();

  // Compute the difference in spoffset between the current and the previous
  // marker fp.
  auto const spAdj = newOff - curOff;

  // Find the source key for the last inlined call from a published frame.
  auto const callSK = [&] {
    auto next = curFp;
    for (;next->inst()->src(1) != fp; next = next->inst()->src(1)) {
      assertx(next->inst()->is(BeginInlining));
    }
    return next->inst()->marker().sk();
  }();

  inst.marker() = inst.marker().adjustFP(fp)
                               .adjustSP(inst.marker().spOff() + spAdj)
                               .adjustFixupSK(callSK);
}

void insert_eager_sync(Global& genv, IRInstruction& endCatch) {
  auto const block = endCatch.block();
  auto sync = genv.unit.gen(
    EagerSyncVMRegs,
    endCatch.bcctx(),
    IRSPRelOffsetData { endCatch.extra<EndCatch>()->offset },
    endCatch.src(0),
    endCatch.src(1)
  );
  block->insert(--block->end(), sync);
}

void fix_inline_frames(Global& genv) {
  using ECM = EndCatchData::CatchMode;
  StateVector<Block,FPState> blockState{
    genv.unit, FPState{nullptr, 0, nullptr, nullptr, nullptr}
  };
  const BlockList rpoBlocks{genv.poBlockList.rbegin(), genv.poBlockList.rend()};
  auto const rpoIDs = numberBlocks(genv.unit, rpoBlocks);
  dataflow_worklist<uint32_t> incompleteQ(rpoBlocks.size());
  DEBUG_ONLY std::unordered_map<SSATmp*,SSATmp*> parentFPs;

  for (auto rpoId = uint32_t{0}; rpoId < rpoBlocks.size(); ++rpoId) {
    incompleteQ.push(rpoId);
  }

  while (!incompleteQ.empty()) {
    auto const rpoId = incompleteQ.pop();
    auto const blk = rpoBlocks[rpoId];
    auto& state = blockState[blk];

    state.entry = nullptr;
    bool needFixup = false;
    blk->forEachPred([&] (Block* pred) {
      auto const& bs = blockState[pred];
      needFixup |= bs.exit && state.entry && state.entry != bs.exit;
      if (bs.catchPC) state.catchPC = bs.catchPC;
      if (bs.catchFP) state.catchFP = bs.catchFP;

      if (!bs.exit) return;
      if (!state.entry || bs.exitDepth < state.exitDepth) {
        state.entry = bs.exit;
        state.exitDepth = bs.exitDepth;
      }
    });

    // We split critical edges so it should be safe to insert InlineReturns into
    // predecessors in this case.
    if (needFixup) {
      assertx(blk->numPreds() > 1);
      blk->forEachPred([&] (Block* pred) {
        assertx(pred->numSuccs() == 1);
        auto& bs = blockState[pred];
        if (bs.exitDepth > state.exitDepth) {
          append_inline_returns(genv, pred, bs.exit, state.entry);
          bs.exit = state.entry;
        }
      });
    }

    auto fp = state.entry;
    folly::Optional<IRSPRelOffset> lastSync;
    for (auto& inst : *blk) {
      adjust_inline_marker(inst, fp);

      if (inst.is(InlineReturn)) {
        if (fp == inst.src(0)) {
          // If we didn't elide the InlineCall paired with this return we can't
          // have removed any of its parent calls as they should each depend on
          // each other.
          assertx(inst.src(1) == parentFPs[fp]);
          state.exitDepth--;

          fp = inst.src(1);
          continue;
        }

        // The InlineCall was removed, we can remove the InlineReturn, determine
        // if the InlineReturn needs to be converted to an InlineCall. This
        // can happen when the InlineCall was killed or moved but the store for
        // the InlineReturn is still live.
        auto const parent = inst.src(1)->inst();
        if (genv.deadInlineReturns[inst] || fp == parent->dst()) {
          inst.convertToNop();
          continue;
        }
        assertx(parent->is(BeginInlining));

        InlineCallData data;
        data.syncVmpc = state.catchPC;
        data.spOffset = parent->extra<BeginInlining>()->spOffset;
        genv.unit.replace(&inst, InlineCall, data, parent->dst(), fp);
        // fallthrough to the InlineCall logic
      }

      if (inst.is(InlineCall)) {
        fp = inst.src(0);
        inst.extra<InlineCall>()->syncVmpc = state.catchPC;
        state.exitDepth++;
      }

      if (debug && inst.is(BeginInlining)) parentFPs[inst.dst()] = inst.src(1);
      if (inst.is(DefFP, DefFuncEntryFP)) fp = inst.dst();
      if (inst.is(Call)) fix_inlined_call(genv, &inst, fp);
      if (inst.is(CallBuiltin)) inst.setSrc(0, fp);

      if (inst.is(EagerSyncVMRegs)) {
        inst.setSrc(0, fp);
        lastSync = inst.extra<EagerSyncVMRegs>()->offset;
      }

      if (inst.is(BeginCatch)) {
        state.catchPC = inst.marker().sk().pc();
        state.catchFP = fp;
      }
      if (inst.is(EndCatch)) {
        if (state.catchFP != fp &&
            inst.extra<EndCatch>()->mode != ECM::CallCatch &&
            lastSync != inst.extra<EndCatch>()->offset) {
          insert_eager_sync(genv, inst);
        }
      }

      // This isn't a correctness problem but it may save us a register
      if (inst.is(CheckSurpriseFlags) && inst.src(0)->isA(TFramePtr)) {
        if (fp->inst()->marker().resumeMode() == ResumeMode::None) {
          inst.setSrc(0, fp);
        }
      }
    }

    if (state.exit && state.exit != fp) {
      blk->forEachSucc([&] (Block* succ) {
        incompleteQ.push(rpoIDs[succ]);
      });
    }
    state.exit = fp;
  }
}

//////////////////////////////////////////////////////////////////////

}

void optimizeStores(IRUnit& unit) {
  PassTracer tracer{&unit, Trace::hhir_store, "optimizeStores"};
  Timer t(Timer::optimize_stores, unit.logEntry().get_pointer());

  // This isn't required for correctness, but it may allow removing stores that
  // otherwise we would leave alone.
  splitCriticalEdges(unit);

  /*
   * Global state for this pass, visible while processing any block.
   */
  auto genv = Global { unit };
  auto const& poBlockList = genv.poBlockList;
  if (genv.ainfo.locations.size() == 0) {
    FTRACE(1, "no memory accesses to possibly optimize\n");
    return;
  }
  FTRACE(1, "\nLocations:\n{}\n", show(genv.ainfo));

  /*
   * Initialize the block state structures.
   */
  for (auto poId = uint32_t{0}; poId < poBlockList.size(); ++poId) {
    genv.blockStates[poBlockList[poId]].id = poId;
  }

  /*
   * Analyze each block to compute its transfer function.
   *
   * The blockAnalysis vector is indexed by post order id.
   */
  auto const blockAnalysis = [&] () -> jit::vector<BlockAnalysis> {
    auto ret = jit::vector<BlockAnalysis>{};
    ret.reserve(unit.numBlocks());
    for (auto poId : poBlockList) {
      ret.push_back(analyze_block(genv, poId));
    }
    for (auto& elm : genv.blk2Aloc) {
      auto poId = genv.blockStates[elm.first].id;
      auto& ba = ret[poId];
      ba.antLoc &= ~elm.second;
      ba.alteredAnt |= elm.second;
      ba.alteredAvl |= elm.second;
    }
    genv.blk2Aloc.clear();
    return ret;
  }();

  FTRACE(2, "\nTransfer functions:\n{}\n",
    [&]() -> std::string {
      auto ret = std::string{};
      for (auto poId = uint32_t{0}; poId < poBlockList.size(); ++poId) {
        auto& analysis = blockAnalysis[poId];
        folly::format(
          &ret,
          " B{}\n"
          "   antLoc    : {}\n"
          "   alteredAnt: {}\n"
          "   alteredAvl: {}\n"
          "   avlLoc    : {}\n"
          "   delLoc    : {}\n",
          poBlockList[poId]->id(),
          show(analysis.antLoc),
          show(analysis.alteredAnt),
          show(analysis.alteredAvl),
          show(analysis.avlLoc),
          show(analysis.delLoc)
        );
      }
      return ret;
    }()
  );

  compute_anticipated(genv, blockAnalysis);

  if (!RuntimeOption::EvalHHIRStorePRE) {
    /*
     * We've reached a fixed point.
     * Delete redundant stores
     */
    FTRACE(2, "\nFixed point:\n{}\n",
           [&]() -> std::string {
             auto ret = std::string{};
             for (auto poId = uint32_t{0}; poId < poBlockList.size(); ++poId) {
               auto const blk = poBlockList[poId];
               auto const& state = genv.blockStates[blk];
               folly::format(
                 &ret,
                 " B{: <3}: antOut {}\n",
                 blk->id(), show(state.antOut)
               );
             }
             return ret;
           }()
          );
    for (auto& block : poBlockList) {
      optimize_block(genv, block);
    }
    return;
  }

  compute_placement_possible(genv, blockAnalysis);

  for (auto& block : poBlockList) {
    optimize_block_pre(genv, block, blockAnalysis);
  }

  if (genv.adjustedInlineCalls) {
    fix_inline_frames(genv);
  }

  if (genv.needsReflow) {
    reflowTypes(genv.unit);
  }
}

//////////////////////////////////////////////////////////////////////

}}
