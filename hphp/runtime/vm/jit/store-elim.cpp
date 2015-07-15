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
#include "hphp/runtime/vm/jit/opt.h"

#include <iterator>
#include <string>

#include <folly/Format.h>
#include <folly/Optional.h>
#include <folly/ScopeGuard.h>

#include "hphp/util/bisector.h"
#include "hphp/util/match.h"
#include "hphp/util/trace.h"
#include "hphp/util/dataflow-worklist.h"

#include "hphp/runtime/vm/jit/alias-analysis.h"
#include "hphp/runtime/vm/jit/cfg.h"
#include "hphp/runtime/vm/jit/containers.h"
#include "hphp/runtime/vm/jit/ir-unit.h"
#include "hphp/runtime/vm/jit/memory-effects.h"
#include "hphp/runtime/vm/jit/mutation.h"
#include "hphp/runtime/vm/jit/pass-tracer.h"
#include "hphp/runtime/vm/jit/state-vector.h"

/*
  This implements partial-redundancy elimination for stores.

  The basic algorithm follows Morel & Renvoise "Global Optimization
  by Suppression of Partial Redundancies". That paper talks about
  redundancy of expressions, so we have to "reverse" everything
  to apply it to stores (Anticipated <=> Available, In <=> Out).
  In addition, there are two forms of (non)-transparency for stores.
  A (possible) use of the result of a candidate store prevents the
  candidate from being moved past the use, and also prevents the candidate
  from being killed by a later store. This is very similar to transparency
  of expressions. But in addition, a possibly interfering store following a
  candidate store prevents the candidate from moving past the store, but does
  not prevent the candidate from being killed by a later store. We split
  these out into alteredAnt, and alteredAvl. This also affects local
  availabilty; a store may not be available in the sense that it can be moved
  to the end of the block (because of a possible conflicting write), but it may
  still be available in the sense that it can be killed by a later store. We
  split this out into AvlLoc (can be moved to the end), and DelLoc (can be
  deleted if its anticipated out).

  Finally, we make some simplifications to the CONST term in M & R to
  reduce the number of bitvectors involved, and reduce the bidirectionality
  of the data flow equations.

  The final equations are:

   Local bitvectors (contents of BlockAnalysis):
    antLoc     : A store can be moved to the start of its block, without
                 changing the meaning of the program
    avlLoc     : A store can be moved to the end of its block, without changing
                 the meaning of the program
    delLoc     : A store can be killed if it would be redundant at the end of
                 the block (even though it may not be legal to move it there)

    alteredAnt : prevents a store from being moved forward through this block,
                 but does not prevent it being killed by a store in a later
                 block.
    alteredAvl : prevents a store from being moved forward through this block,
                 and prevents it from being killed by a later store.

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
      block to the exit of the region. We can place a store at the end of a
      block if its AvlLoc (by definition) or if its PPIn and "transparent" to
      the block; but since we're actually going to be inserting at the start of
      blocks, there's no point marking a block PPOut unless the store is PPIn
      or its redundant in each of the following blocks.

      Note that we could use 1s instead of AvlIn/AvlOut, but convergence would
      be slower, and we would still need to deal with stores where the
      bitvectors say they're available, but we can't construct a suitable
      store).

    Insert = PPIn & ~AntIn & (~PPOut | alteredAvl);
      We want to insert if its possible, and the store is not redundant, and we
      can't push the store any later.

    Delete = (AntOut & DelLoc) | (PPOut & AvlLoc);
      AntOut & DelLoc means its redundant; we'll just kill it.
      PPOut => PPIn | AntIn in each successor, meaning that for each successor
      (recursively) we can either push the store into that successor, or its
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
      paths to this point they're not compatible in some way
      (usually SpillFrame vs StStk).

    - Pending and Processed are used while building phis to handle
      cycles of phis.

  For the Phi case, we just need to ensure that we can differentiate
  Phis in the same block for different ALocations; this is just used
  for the hash() and same() methods for the spillFrameMap.
*/
struct TrackedStore {
  enum Kind {
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
  IRInstruction* processed() {
    return kind() == Processed ? static_cast<IRInstruction*>(m_ptr.ptr()) :
      nullptr;
  }
  bool isUnseen() const {
    return kind() == Unseen;
  }
  bool isBad() const {
    return kind() == Bad;
  }
  bool isPhi() const {
    return kind() >= Phi && kind() != Bad;
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
  friend bool operator>=(const TrackedStore& a, const TrackedStore& b) {
    return a.kind() >= b.kind();
  }
  size_t hash() const {
    return m_ptr.tag() + intptr_t(m_ptr.ptr());
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

struct TrackedStoreHashCmp {
  size_t operator()(const TrackedStore& ts) const {
    return ts.hash();
  }
  bool operator()(const TrackedStore& ts1, const TrackedStore& ts2) const {
    return ts1.same(ts2);
  }
};

using SpillFrameMap = hphp_hash_map<TrackedStore, ALocBits,
                                    TrackedStoreHashCmp, TrackedStoreHashCmp>;

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
  {}

  IRUnit& unit;
  BlockList poBlockList;
  AliasAnalysis ainfo;

  // We keep an entry for each tracked SpillFrame in this map, so we
  // can check its ALocBits
  SpillFrameMap spillFrameMap;
  MovableStoreMap trackedStoreMap;
  // Block states are indexed by block->id().  These are only meaningful after
  // we do dataflow.
  StateVector<Block,BlockState> blockStates;
  jit::vector<IRInstruction*> reStores;
  bool needsReflow{false};
};

// Block-local environment.
struct Local {
  Global& global;

  ALocBits antLoc;     // Copied to BlockAnalysis::antLoc
  ALocBits mayLoad;    // Things that may be read in the block
  ALocBits mayStore;   // Things that may be written in the block
  ALocBits avlLoc;     // Copied to BlockAnalysis::avlLoc
  ALocBits delLoc;     // Copied to BlockAnalysis::delLoc

  ALocBits reStores;
};

//////////////////////////////////////////////////////////////////////

using jit::show;
std::string show(TrackedStore ts) {
  if (ts.isUnseen()) return "U";
  if (ts.isBad()) return "B";
  if (auto i = ts.instruction()) return folly::sformat("I{}", i->id());
  if (auto b = ts.block()) return folly::sformat("P{}", b->id());
  not_reached();
}

const ALocBits* findSpillFrame(Global& genv, const TrackedStore& ts) {
  auto it = genv.spillFrameMap.find(ts);
  if (it == genv.spillFrameMap.end()) return nullptr;
  return &it->second;
}

void set_movable_store(Local& env, uint32_t bit, IRInstruction& inst) {
  env.global.trackedStoreMap[
    StoreKey { inst.block(), StoreKey::Out, bit }].set(&inst);
}

void set_movable_spill_frame(Local& env, const ALocBits& bits,
                             IRInstruction& inst) {
  TrackedStore ts { &inst };
  for (uint32_t i = env.global.ainfo.count(); i--; ) {
    if (bits[i]) {
      env.global.trackedStoreMap[
        StoreKey { inst.block(), StoreKey::Out, i }] = ts;
    }
  }
  env.global.spillFrameMap[ts] = bits;
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
        inst.marker(),
        IRSPOffsetData { inst.extra<StStk>()->offset },
        inst.src(0)
      );
      break;
    case SpillFrame:
      dbgInst = env.global.unit.gen(
        DbgTrashFrame,
        inst.marker(),
        IRSPOffsetData { inst.extra<SpillFrame>()->spOffset },
        inst.src(0)
      );
      break;
    case StMem:
      dbgInst = env.global.unit.gen(DbgTrashMem, inst.marker(), inst.src(0));
      break;
    default:
      dbgInst = nullptr;
      break;
    }
  }

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

void mustStore(Local& env, AliasClass acls) {
  auto const canon = canonicalize(acls);
  auto const kill = env.global.ainfo.expand(canon);
  mayStore(env, acls);
  mustStoreSet(env, kill);
}

folly::Optional<uint32_t> pure_store_bit(Local& env, AliasClass acls) {
  if (auto const meta = env.global.ainfo.find(canonicalize(acls))) {
    return meta->index;
  }
  return folly::none;
}

void visit(Local& env, IRInstruction& inst) {
  auto const effects = memory_effects(inst);
  FTRACE(3, "    {}\n"
            "      {}\n",
            inst.toString(),
            show(effects));

  match<void>(
    effects,
    [&] (IrrelevantEffects) {},
    [&] (UnknownEffects)    { addAllLoad(env); env.mayStore.set(); },
    [&] (PureLoad l)        {
      if (auto bit = pure_store_bit(env, l.src)) {
        if (env.reStores[*bit]) {
          FTRACE(4, "Killing self-store: {}\n",
                 env.global.reStores[*bit]->toString());
          removeDead(env, *env.global.reStores[*bit], false);
        }
      }
      load(env, l.src);
    },
    [&] (GeneralEffects l)  { load(env, l.loads);
                              mayStore(env, l.stores);
                              mustStore(env, l.kills); },

    [&] (ReturnEffects l) {
      if (inst.is(InlineReturn)) {
        // Returning from an inlined function.  This adds nothing to gen, but
        // kills frame and stack locations for the callee.
        auto const fp = inst.src(0);
        auto const killSet = env.global.ainfo.per_frame_bits[fp];
        mustStoreSet(env, killSet);
        mustStore(env, l.kills);
        return;
      }

      // Return from the main function.  Locations other than the frame and
      // stack (e.g. object properties and whatnot) are always live on a
      // function return---so mark everything read before we start killing
      // things.
      addAllLoad(env);
      mustStoreSet(env, env.global.ainfo.all_frame);
      mustStore(env, l.kills);
    },

    [&] (ExitEffects l) {
      load(env, l.live);
      mustStore(env, l.kills);
    },

    /*
     * Call instructions potentially throw, even though we don't (yet) have
     * explicit catch traces for them, which means it counts as possibly
     * reading any local, on any frame---if it enters the unwinder it could
     * read them.
     */
    [&] (CallEffects l) {
      load(env, AHeapAny);
      load(env, AFrameAny);  // Not necessary for some builtin calls, but it
                             // depends which builtin...
      load(env, l.stack);
      mustStore(env, l.kills);
    },

    [&] (PureStore l) {
      if (auto bit = pure_store_bit(env, l.dst)) {
        if (isDead(env, *bit)) {
          if (!removeDead(env, inst, true)) {
            mayStore(env, l.dst);
            mustStore(env, *bit);
          }
          return;
        }
        if (!env.antLoc[*bit] &&
            !env.mayLoad[*bit] &&
            !env.mayStore[*bit]) {
          env.avlLoc[*bit] = 1;
          set_movable_store(env, *bit, inst);
        }
        mayStore(env, l.dst);
        mustStore(env, *bit);
        if (l.value->inst()->block() != inst.block()) return;
        auto const le = memory_effects(*l.value->inst());
        auto pl = boost::get<PureLoad>(&le);
        if (!pl) return;
        auto lbit = pure_store_bit(env, pl->src);
        if (!lbit || *lbit != *bit) return;
        /*
         * The source of the store is a load from
         * the same address, which is also in this
         * block. If there's no interference, we
         * can kill this store. We won't know until
         * we see the load though.
         */
        if (env.global.reStores.size() <= *bit) {
          env.global.reStores.resize(*bit + 1);
        }
        env.global.reStores[*bit] = &inst;
        env.reStores[*bit] = 1;
        return;
      }
      mayStore(env, l.dst);
    },

    [&] (PureSpillFrame l) {
      auto const it = env.global.ainfo.stack_ranges.find(canonicalize(l.stk));
      if (it != end(env.global.ainfo.stack_ranges)) {
        // If all the bits corresponding to the stack range are dead, we can
        // eliminate this instruction.  We can also count all of them as
        // redefined.
        if (isDeadSet(env, it->second)) {
          if (removeDead(env, inst, true)) return;
        } else {
          auto avlLoc = it->second & ~(env.antLoc | env.mayLoad |
                                       env.mayStore);
          if (avlLoc == it->second) {
            set_movable_spill_frame(env, avlLoc, inst);
            env.avlLoc |= avlLoc;
          }
        }
        mayStore(env, l.stk);
        mustStoreSet(env, it->second);
        return;
      }
      mayStore(env, l.stk);
    }
  );
}

void block_visit(Local& env, Block* block) {
  FTRACE(2, "  visiting B{}\n", block->id());
  auto it = block->instrs().end();
  --it;
  for (;;) {
    folly::Optional<Block::iterator> prev;
    if (it != block->instrs().begin()) {
      prev = std::prev(it);
    }
    visit(env, *it);
    if (!prev) break;
    it = *prev;
  }
}

//////////////////////////////////////////////////////////////////////

struct BlockAnalysis {
  ALocBits antLoc;
  ALocBits alteredAnt;
  ALocBits alteredAvl;
  ALocBits avlLoc;
  ALocBits delLoc;
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
    env.mayLoad | env.mayStore | env.antLoc | env.delLoc,
    env.avlLoc,
    env.delLoc
  };
}

void find_all_stores(Global& genv, Block* blk, uint32_t id,
                     jit::vector<IRInstruction*>& stores,
                     jit::hash_set<Block*>& seen) {
  if (!seen.insert(blk).second) return;
  blk->forEachPred([&](Block* pred) {
      auto& pst = genv.trackedStoreMap[StoreKey { pred, StoreKey::Out, id }];
      IRInstruction* inst;
      if ((inst = pst.instruction()) != nullptr ||
          (inst = pst.processed()) != nullptr) {
        stores.push_back(inst);
        return;
      }
      Block* b;
      if ((b = pst.block()) != nullptr ||
          (b = pst.pending()) != nullptr) {
        find_all_stores(genv, b, id, stores, seen);
        return;
      }
      always_assert(false);
    });
}

IRInstruction* resolve_ts(Global& genv, Block* blk,
                          StoreKey::Where w, uint32_t id,
                          const ALocBits** sfp = nullptr);

IRInstruction* resolve_cycle(Global& genv, Block* blk, uint32_t id) {
  genv.needsReflow = true;
  jit::vector<IRInstruction*> stores;
  jit::hash_set<Block*> seen;
  // find all the stores, so we can determine
  // whether a phi is actually required for each
  // src (also, we need a candidate store to clone)
  find_all_stores(genv, blk, id, stores, seen);
  always_assert(stores.size() > 0);
  auto cand = stores[0];
  if (stores.size() == 1) {
    return cand;
  }
  jit::vector<uint32_t> srcsToPhi;
  for (uint32_t i = 0; i < cand->numSrcs(); i++) {
    SSATmp* prev = nullptr;
    for (auto& st : stores) {
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
    return cand;
  }
  auto inst = genv.unit.clone(cand);
  for (auto i : srcsToPhi) {
    // create a Mov; we'll use its dst as the src of the store,
    // and when we eventually create the phi, we'll set its
    // dst as the src of the Mov (this allows us to avoid
    // creating a new phi if there's already a suitable one
    // there).
    auto mv = genv.unit.gen(Mov, blk->front().marker(), cand->src(i));
    blk->prepend(mv);
    inst->setSrc(i, mv->dst());
  }
  return inst;
}

IRInstruction* resolve_flat(Global& genv, Block* blk, uint32_t id,
                            TrackedStore& ts) {
  ts.setPending(blk);

  jit::vector<IRInstruction*> stores;
  blk->forEachPred([&](Block* pred) {
      stores.push_back(resolve_ts(genv, pred, StoreKey::Out, id));
    });
  always_assert(stores.size() > 0);
  if (auto rep = ts.instruction()) return rep;

  auto cand = stores[0];
  if (stores.size() == 1) return cand;
  jit::vector<SSATmp*> newSrcs;
  jit::vector<uint32_t> srcsToPhi;
  for (uint32_t i = 0; i < cand->numSrcs(); i++) {
    bool same = true;
    std::vector<SSATmp*> phiInputs;
    for (auto& st : stores) {
      auto const si = st->src(i);
      phiInputs.push_back(si);
      if (si != phiInputs[0]) same = false;
    }
    if (!same) {
      srcsToPhi.push_back(i);
      newSrcs.push_back(insertPhi(genv.unit, blk, phiInputs));
    }
  }

  if (auto rep = ts.processed()) {
    // the replacement was constructed during the recursive
    // walk. Just need to hook up the new phis.
    for (uint32_t ix = 0; ix < srcsToPhi.size(); ix++) {
      auto i = srcsToPhi[ix];
      auto src = rep->src(i);
      auto mv = src->inst();
      always_assert(mv->is(Mov));
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
                          StoreKey::Where w, uint32_t id,
                          const ALocBits** sfp) {
  auto& ts = genv.trackedStoreMap[StoreKey { blk, w, id }];
  auto sf = findSpillFrame(genv, ts);
  if (sfp) *sfp = sf;
  if (auto inst = ts.instruction()) return inst;
  if (auto inst = ts.processed()) return inst;

  auto rep = [&]() {
    if (ts.pending()) {
      always_assert(ts.pending() == blk);
      ts.setProcessed(resolve_cycle(genv, blk, id));
      return ts.processed();
    }

    always_assert(ts.block());
    if (w != StoreKey::In || blk != ts.block()) {
      ts.set(resolve_ts(genv, ts.block(), StoreKey::In, id));
    } else {
      ts.set(resolve_flat(genv, blk, id, ts));
    }
    return ts.instruction();
  }();

  if (sf) genv.spillFrameMap[ts] = *sf;
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

    for (uint32_t i = 0; i < genv.ainfo.count(); i++) {
      if (insertBits[i] && store_insert.go()) {
        const ALocBits* sf = nullptr;
        auto const cinst = resolve_ts(genv, block, StoreKey::In, i, &sf);
        if (sf) {
          always_assert((state.ppIn & *sf) == *sf);
          insertBits &= ~*sf;
        }
        FTRACE(1, " Inserting store {}: {}\n", i, cinst->toString());
        auto const inst = genv.unit.clone(cinst);
        block->prepend(inst);
      }
    }
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

/*
 * Find a candidate store instruction; any one will do, because
 * compatibility between stores is transitive.
 */
TrackedStore find_candidate_store(Global& genv, TrackedStore ts, uint32_t id) {
  while (true) {
    auto block = ts.block();
    if (!block) return ts;
    ts.reset();
    block->forEachPred([&](Block* pred) {
        if (ts.isBad()) return;
        auto const pred_ts =
          genv.trackedStoreMap[StoreKey { pred, StoreKey::Out, id }];
        if (pred_ts.isBad() || pred_ts.instruction() || !ts.instruction()) {
          // given a choice of blocks, choose the one with higher
          // post-order id to avoid cycles
          if (!ts.block() || !pred_ts.block() ||
              genv.blockStates[ts.block()].id <
              genv.blockStates[pred_ts.block()].id) {
            ts = pred_ts;
          }
        }
      });
  }
}

TrackedStore combine_ts(Global& genv, uint32_t id,
                        TrackedStore s1,
                        TrackedStore s2, Block* succ) {
  if (s1.same(s2)) return s1;
  if (s1.isUnseen() || s2.isBad()) return s2;
  if (s2.isUnseen() || s1.isBad()) return s1;

  enum class Compat { Same, Compat, Bad };
  auto compat = [](TrackedStore s1, TrackedStore s2) {
    auto i1 = s1.instruction();
    auto i2 = s2.instruction();
    assert(i1 && i2);
    if (i1->op() != i2->op()) return Compat::Bad;
    if (i1->numSrcs() != i2->numSrcs()) return Compat::Bad;
    for (auto i = i1->numSrcs(); i--; ) {
      if (i1->src(i) != i2->src(i)) return Compat::Compat;
    }
    return Compat::Same;
  };

  auto sf1 = findSpillFrame(genv, s1);
  auto sf2 = findSpillFrame(genv, s2);

  // t7621182 Don't merge different spillframes for now,
  // because code-gen doesn't handle phi'ing an Obj and a
  // Cls.
  if (sf1 || sf2) {
    // we already know they aren't the same
    return TrackedStore::BadVal();
  }

  if (!sf1 != !sf2 || (sf1 && *sf1 != *sf2)) {
    // They need to both be spill frames affecting
    // the same addresses, or both not be spill frames.
    return TrackedStore::BadVal();
  }

  auto trackedBlock = [&]() {
    auto ret = TrackedStore(succ, id);
    if (sf1) genv.spillFrameMap[ret] = *sf1;
    return ret;
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
 * Given a TrackedStore s1 in blk, and s2 in succ,
 * figure out the new TrackedStore to replace s2.
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
      if (propagate.any()) {
        for (uint32_t i = genv.ainfo.count(); i--; ) {
          if (!propagate[i]) continue;
          auto const& tsIn =
            genv.trackedStoreMap[StoreKey { blk, StoreKey::In, i }];
          auto& tsOut =
            genv.trackedStoreMap[StoreKey { blk, StoreKey::Out, i }];
          assert(!tsIn.isUnseen());
          tsOut = tsIn;
          if (tsOut.isBad()) {
            state.ppOut[i] = 0;
          } else if (auto sf = findSpillFrame(genv, tsIn)) {
            if ((propagate & *sf) != *sf) {
              state.ppOut &= ~*sf;
            }
          }
        }
      }
      auto showv DEBUG_ONLY = [&] (StoreKey::Where w, const ALocBits& pp) {
        std::string r;
        for (uint32_t i = genv.ainfo.count(); i--; ) {
          if (pp[i]) {
            auto& ts = genv.trackedStoreMap[StoreKey { blk, w, i}];
            folly::format(&r, " {}:{}", i, show(ts));
          }
        }
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
      if (succState.ppIn.any()) {
        auto blk = genv.poBlockList[state.id];
        auto succ = genv.poBlockList[succState.id];
        for (uint32_t i = genv.ainfo.count(); i--; ) {
          if (!succState.ppIn[i]) continue;
          auto& ts =
            genv.trackedStoreMap[StoreKey { blk, StoreKey::Out, i }];
          auto& tsSucc =
            genv.trackedStoreMap[StoreKey { succ, StoreKey::In, i }];
          auto sf = findSpillFrame(genv, ts);
          if (sf) {
            if ((succState.ppIn & *sf) != *sf) {
              changed = true;
              tsSucc.setBad();
              succState.ppIn &= ~*sf;
              continue;
            }
          }
          auto tsNew = succ->numPreds() == 1 ? ts : recompute_ts(genv, i, succ);
          if (!tsNew.same(tsSucc)) {
            changed = true;
            assert(tsNew >= tsSucc);
            tsSucc = tsNew;
            if (tsSucc.isBad()) {
              if (sf) {
                succState.ppIn &= ~*sf;
              } else {
                succState.ppIn[i] = false;
              }
            }
          }
        }
      }
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

      auto bitsToClear = state.ppOut & ~restrictBits;
      if (bitsToClear.any()) {
        // ensure that if we clear any spill frame bits, we clear
        // them all.
        for (uint32_t i = genv.ainfo.count(); i--; ) {
          if (!bitsToClear[i]) continue;
          auto const ts =
            genv.trackedStoreMap[StoreKey { blk, StoreKey::Out, i }];
          if (auto sf = findSpillFrame(genv, ts)) {
            restrictBits &= ~*sf;
          }
        }
        state.ppOut &= restrictBits;
      }
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

//////////////////////////////////////////////////////////////////////

}

void optimizeStores(IRUnit& unit) {
  PassTracer tracer{&unit, Trace::hhir_store, "optimizeStores"};

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
    for (auto id = uint32_t{0}; id < poBlockList.size(); ++id) {
      ret.push_back(analyze_block(genv, poBlockList[id]));
    }
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
  if (genv.needsReflow) {
    reflowTypes(genv.unit);
  }
}

//////////////////////////////////////////////////////////////////////

}}
