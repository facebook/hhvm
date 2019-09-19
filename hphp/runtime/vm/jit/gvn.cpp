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

#include "hphp/runtime/vm/jit/analysis.h"
#include "hphp/runtime/vm/jit/block.h"
#include "hphp/runtime/vm/jit/cfg.h"
#include "hphp/runtime/vm/jit/containers.h"
#include "hphp/runtime/vm/jit/ir-instruction.h"
#include "hphp/runtime/vm/jit/ssa-tmp.h"
#include "hphp/runtime/vm/jit/state-vector.h"
#include "hphp/runtime/vm/jit/pass-tracer.h"
#include "hphp/runtime/vm/jit/timer.h"

#include "hphp/util/dataflow-worklist.h"

#include <sstream>
#include <folly/small_vector.h>

namespace HPHP { namespace jit {

TRACE_SET_MOD(hhir_gvn);

//////////////////////////////////////////////////////////////////////

namespace {

struct ValueNumberMetadata {
  SSATmp* key;
  SSATmp* value;
};

using ValueNumberTable = StateVector<SSATmp, ValueNumberMetadata>;
using DstIndex = int32_t;

struct CongruenceHasher {
  using KeyType = std::pair<IRInstruction*, DstIndex>;

  explicit CongruenceHasher(const ValueNumberTable& globalTable)
    : m_globalTable(&globalTable)
  {
  }

  size_t hashSharedImpl(IRInstruction* inst, size_t result) const {
    if (inst->hasExtra()) {
      result = folly::hash::hash_128_to_64(
        result,
        hashExtra(inst->op(), inst->rawExtra())
      );
    }

    if (inst->hasTypeParam()) {
      result = folly::hash::hash_128_to_64(result, inst->typeParam().hash());
    }

    return result;
  }

  size_t hashSrcs(KeyType key, size_t result) const {
    auto& table = *m_globalTable;
    auto inst = key.first;
    for (uint32_t i = 0; i < inst->numSrcs(); ++i) {
      auto src = canonical(inst->src(i));
      assertx(table[src].value);
      result = folly::hash::hash_128_to_64(
        result,
        reinterpret_cast<size_t>(table[src].value)
      );
    }
    return result;
  }

  size_t operator()(KeyType key) const {
    auto inst = key.first;

    // Note: this doesn't take commutativity or associativity into account, but
    // it might be nice to do so for the opcodes where it makes sense.
    size_t result = static_cast<size_t>(inst->op());
    result = hashSrcs(key, result);
    return hashSharedImpl(inst, result);
  }

private:
  const ValueNumberTable* m_globalTable;
};

struct CongruenceComparator {
  using KeyType = std::pair<IRInstruction*, DstIndex>;

  explicit CongruenceComparator(const ValueNumberTable& globalTable)
    : m_globalTable(&globalTable)
  {
  }

  bool compareSrcs(KeyType keyA, KeyType keyB) const {
    auto& table = *m_globalTable;
    auto instA = keyA.first;
    auto instB = keyB.first;

    for (uint32_t i = 0; i < instA->numSrcs(); ++i) {
      auto srcA = canonical(instA->src(i));
      auto srcB = canonical(instB->src(i));
      assertx(table[srcA].value);
      assertx(table[srcB].value);
      if (table[srcA].value != table[srcB].value) return false;
    }

    return true;
  }

  bool operator()(KeyType keyA, KeyType keyB) const {
    auto instA = keyA.first;
    auto instB = keyB.first;

    // Note: this doesn't take commutativity or associativity into account, but
    // it might be nice to do so for the opcodes where it makes sense.
    if (instA->op() != instB->op()) return false;
    if (instA->numSrcs() != instB->numSrcs()) return false;
    if (instA->hasTypeParam() != instB->hasTypeParam()) return false;
    if (instA->hasTypeParam() && instA->typeParam() != instB->typeParam()) {
      return false;
    }

    if (instA->hasExtra()) {
      assertx(instB->hasExtra());
      if (!equalsExtra(instA->op(), instA->rawExtra(), instB->rawExtra())) {
        return false;
      }
    }

    if (!compareSrcs(keyA, keyB)) {
      return false;
    }

    return true;
  }

private:
  const ValueNumberTable* m_globalTable;
};

using NameTable = jit::hash_map<
  std::pair<IRInstruction*, DstIndex>, SSATmp*,
  CongruenceHasher,
  CongruenceComparator
>;

struct GVNState {
  ValueNumberTable* localTable{nullptr};
  ValueNumberTable* globalTable{nullptr};
  NameTable* nameTable{nullptr};
};

bool supportsGVN(const IRInstruction* inst) {
  switch (inst->op()) {
  case AssertType:
  case AbsDbl:
  case AddInt:
  case SubInt:
  case MulInt:
  case AndInt:
  case AddDbl:
  case SubDbl:
  case MulDbl:
  case DivDbl:
  case DivInt:
  case Mod:
  case Sqrt:
  case OrInt:
  case XorInt:
  case Shl:
  case Shr:
  case Floor:
  case Ceil:
  case AddIntO:
  case SubIntO:
  case MulIntO:
  case XorBool:
  case ConvBoolToArr:
  case ConvDblToArr:
  case ConvFuncToArr:
  case ConvIntToArr:
  case ConvDblToBool:
  case ConvIntToBool:
  case ConvBoolToDbl:
  case ConvIntToDbl:
  case ConvBoolToInt:
  case ConvDblToInt:
  case ConvClsToCctx:
  case DblAsBits:
  case GtInt:
  case GteInt:
  case LtInt:
  case LteInt:
  case EqInt:
  case NeqInt:
  case CmpInt:
  case GtDbl:
  case GteDbl:
  case LtDbl:
  case LteDbl:
  case EqDbl:
  case NeqDbl:
  case CmpDbl:
  case GtStr:
  case GteStr:
  case LtStr:
  case LteStr:
  case EqStr:
  case NeqStr:
  case SameStr:
  case NSameStr:
  case CmpStr:
  case GtStrInt:
  case GteStrInt:
  case LtStrInt:
  case LteStrInt:
  case EqStrInt:
  case NeqStrInt:
  case CmpStrInt:
  case GtBool:
  case GteBool:
  case LtBool:
  case LteBool:
  case EqBool:
  case NeqBool:
  case CmpBool:
  case SameObj:
  case NSameObj:
  case SameVec:
  case NSameVec:
  case SameDict:
  case NSameDict:
  case EqKeyset:
  case NeqKeyset:
  case SameKeyset:
  case NSameKeyset:
  case GtRes:
  case GteRes:
  case LtRes:
  case LteRes:
  case EqRes:
  case NeqRes:
  case CmpRes:
  case EqCls:
  case EqFunc:
  case EqStrPtr:
  case InstanceOf:
  case InstanceOfIface:
  case InstanceOfIfaceVtable:
  case ExtendsClass:
  case InstanceOfBitmask:
  case NInstanceOfBitmask:
  case InterfaceSupportsArr:
  case InterfaceSupportsVec:
  case InterfaceSupportsDict:
  case InterfaceSupportsKeyset:
  case InterfaceSupportsStr:
  case InterfaceSupportsInt:
  case InterfaceSupportsDbl:
  case HasToString:
  case IsType:
  case IsNType:
  case IsWaitHandle:
  case IsCol:
  case IsDVArray:
  case LdRDSAddr:
  case LdCtx:
  case LdCctx:
  case LdClsCtx:
  case LdClsCctx:
  case LdClsCtor:
  case DefConst:
  case DefCls:
  case LdCls:
  case LdClsCached:
  case LdClsInitData:
  case LdClsFromClsMeth:
  case LdSmashableFunc:
  case LdFuncFromClsMeth:
  case LdFuncVecLen:
  case LdClsMethod:
  case LdIfaceMethod:
  case LdPropAddr:
  case LdClsPropAddrOrNull:
  case LdClsPropAddrOrRaise:
  case LdObjClass:
  case LdClsName:
  case LdARNumParams:
  case Mov:
  case LdContActRec:
  case LdAFWHActRec:
  case LdPackedArrayDataElemAddr:
  case OrdStr:
  case ChrInt:
  case CheckRange:
  case CountArrayFast:
  case CountVec:
  case CountDict:
  case CountKeyset:
  case Select:
  case StrictlyIntegerConv:
  case LookupSPropSlot:
  case ConvPtrToLval:
    return true;

  case SameArr:
  case NSameArr:
    return !RuntimeOption::EvalHackArrCompatDVCmpNotices;

  default:
    return false;
  }
}

void initWithInstruction(IRInstruction* inst, ValueNumberTable& table) {
  // Each SSATmp starts out as the canonical name for itself.
  for (auto dst : inst->dsts()) {
    table[dst] = ValueNumberMetadata { dst, dst };
  }

  for (auto src : inst->srcs()) {
    table[src] = ValueNumberMetadata { src, src };
  }
}

bool visitInstruction(
  GVNState& env,
  IRInstruction* inst
) {
  auto& globalTable = *env.globalTable;
  auto& localTable = *env.localTable;
  auto& nameTable = *env.nameTable;

  if (isCallOp(inst->op())) nameTable.clear();

  if (!supportsGVN(inst)) return false;

  bool changed = false;
  for (auto dstIdx = 0; dstIdx < inst->numDsts(); ++dstIdx) {
    auto dst = inst->dst(dstIdx);
    assertx(dst);

    auto result = nameTable.emplace(std::make_pair(inst, dstIdx), dst);
    SSATmp* temp = result.second ? dst : result.first->second;
    assertx(temp);

    assertx(globalTable[dst].value);
    if (temp != globalTable[dst].value) {
      localTable[dst] = ValueNumberMetadata { dst, temp };
      FTRACE(1,
        "instruction {}\n"
        "updated value number for dst to dst of {}\n",
        *inst,
        *temp->inst()
      );
      changed = true;
    }
  }
  return changed;
}

bool visitBlock(
  GVNState& env,
  Block* block
) {
  bool changed = false;
  for (auto& inst : *block) {
    changed = visitInstruction(env, &inst) || changed;
  }
  return changed;
}

void applyLocalUpdates(ValueNumberTable& local, ValueNumberTable& global) {
  for (auto metadata : local) {
    if (!metadata.key) continue;
    global[metadata.key] = metadata;
  }
}

void runAnalysis(
  GVNState& env,
  const IRUnit& unit,
  const BlockList& blocks
) {
  for (auto block : blocks) {
    for (auto& inst : *block) {
      initWithInstruction(&inst, *env.globalTable);
    }
  }

  bool changed = true;
  while (changed) {
    // We need a temporary table of updates which we apply after running this
    // iteration of the fixed point. If we change the global ValueNumberTable
    // during the pass, the hash values of the SSATmps will change which is
    // apparently a no-no for unordered_map.
    ValueNumberTable localTable(unit, ValueNumberMetadata{});
    env.localTable = &localTable;
    SCOPE_EXIT { env.localTable = nullptr; };
    {
      CongruenceHasher hash(*env.globalTable);
      CongruenceComparator pred(*env.globalTable);
      NameTable nameTable(0, hash, pred);
      env.nameTable = &nameTable;
      SCOPE_EXIT { env.nameTable = nullptr; };

      changed = false;
      for (auto block : blocks) {
        changed = visitBlock(env, block) || changed;
      }
    }
    applyLocalUpdates(localTable, *env.globalTable);
  }
}

/*
 * When gvn replaces an instruction that produces a refcount, we need
 * to IncRef the replacement value. Its not enough to IncRef it where
 * the replacement occurs (where a refcount would originally have been
 * produced), because the replacement value might not survive that
 * long. Instead, we need to IncRef it right after the canonical
 * instruction. However, in general there will be paths from the
 * canonical instruction that don't reach the replaced instruction, so
 * we'll need to insert DecRefs on those paths.
 *
 * As an example, given:
 *
 *        +------+
 *        | Orig |
 *        +------+
 *           |\
 *           | \
 *           |  \    +-----+
 *           |   --->|exit1|
 *           |       +-----+
 *           v
 *        +------+
 *        | Rep1 |
 *        +------+
 *           |\
 *           | \
 *           |  \    +-----+
 *           |   --->|exit2|
 *           |       +-----+
 *           v
 *        +------+
 *        | Rep2 |
 *        +------+
 *
 * Where Orig is a PRc instruction, and Rep1 and Rep2 are identical
 * instructions that are going to be replaced, we need to insert an
 * IncRef after Orig (to ensure its dst lives until Rep1), a DecRef at
 * exit1 (to keep the RefCounts balanced), an IncRef after Rep1 (to
 * ensure Orig's dst lives until Rep2), and a DecRef in exit2 (to keep
 * the RefCounts balanced).
 *
 * We do this by computing Availability, Anticipability and
 * Partial-Anticipability of the candidate instructions (Orig, Rep1
 * and Rep2 in the example). After each candidate instruction we
 * insert an IncRef if a candidate is Partially-Anticipated-out. If a
 * candidate is Available-in and not Partially-Anticipated-in, we
 * insert a DecRef if its Partially-Anticipated out of all
 * predecessors. This could fail to insert DecRefs if we don't split
 * critical edges - but in that case there simply wouldn't be anywhere
 * to insert the DecRef.
 */
constexpr uint32_t kMaxTrackedPrcs = 64;

struct PrcState {
  using Bits = std::bitset<kMaxTrackedPrcs>;

  uint32_t rpoId;

  /* local is the set of candidates in this block */
  Bits local;
  /* antIn = local | antOut */
  Bits antIn;
  /* pantIn = local | pantOut */
  Bits pantIn;
  /* antOut = Intersection<succs>(antIn) (empty if numSucc==0) */
  Bits antOut;
  /* pantOut = Union<succs>(pantIn) */
  Bits pantOut;
  /* avlIn = Intersection<preds>(avlOut) (empty if numPred==0) */
  Bits avlIn;
  /* avlOut = local | avlIn */
  Bits avlOut;
};

std::string show(const PrcState::Bits& bits) {
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

std::string show(const PrcState& state) {
  return folly::sformat(
    "   antIn   : {}\n"
    "   pantIn  : {}\n"
    "   avlIn   : {}\n"
    "   local   : {}\n"
    "   antOut  : {}\n"
    "   pantOut : {}\n"
    "   avlOut  : {}\n",
    show(state.antIn),
    show(state.pantIn),
    show(state.avlIn),
    show(state.local),
    show(state.antOut),
    show(state.pantOut),
    show(state.avlOut));
}

struct PrcEnv {
  PrcEnv(IRUnit& unit, const BlockList& rpoBlocks) :
      unit{unit}, rpoBlocks{rpoBlocks} {}

  IRUnit& unit;
  const BlockList& rpoBlocks;
  // The first element of each inner vector is the canonical SSATmp
  // and the remainder are the SSATmps that will be replaced.
  std::vector<std::vector<SSATmp*>> insertMap;
  std::vector<PrcState> states;
};

void insertIncRefs(PrcEnv& env) {
  auto antQ =
    dataflow_worklist<uint32_t, std::less<uint32_t>>(env.rpoBlocks.size());
  auto avlQ =
    dataflow_worklist<uint32_t, std::greater<uint32_t>>(env.rpoBlocks.size());

  env.states.resize(env.unit.numBlocks());
  for (uint32_t i = 0; i < env.rpoBlocks.size(); i++) {
    auto blk = env.rpoBlocks[i];
    auto& state = env.states[blk->id()];
    state.rpoId = i;
    if (blk->numSuccs()) state.antOut.set();
    if (blk->numPreds()) state.avlIn.set();
    antQ.push(i);
    avlQ.push(i);
  }

  auto id = 0;
  for (auto& v : env.insertMap) {
    for (auto const tmp : v) {
      auto const blk = tmp->inst()->block();
      auto& state = env.states[blk->id()];
      if (!state.local.test(id)) {
        state.local.set(id);
        continue;
      }
    }
    id++;
  }

  using Bits = PrcState::Bits;
  // compute anticipated
  do {
    auto const blk = env.rpoBlocks[antQ.pop()];
    auto& state = env.states[blk->id()];
    state.antIn = state.antOut | state.local;
    state.pantIn = state.pantOut | state.local;
    blk->forEachPred(
      [&] (Block* b) {
        auto& s = env.states[b->id()];
        auto const antOut = s.antOut & state.antIn;
        auto const pantOut = s.pantOut | state.pantIn;
        if (antOut != s.antOut || pantOut != s.pantOut) {
          s.antOut = antOut;
          s.pantOut = pantOut;
          antQ.push(s.rpoId);
        }
      }
    );
  } while (!antQ.empty());

  // compute available
  do {
    auto const blk = env.rpoBlocks[avlQ.pop()];
    auto& state = env.states[blk->id()];
    state.avlOut = state.avlIn | state.local;
    blk->forEachSucc(
      [&] (Block* b) {
        auto& s = env.states[b->id()];
        auto const avlIn = s.avlIn & state.avlOut;
        if (avlIn != s.avlIn) {
          s.avlIn = avlIn;
          avlQ.push(s.rpoId);
        }
      });
  } while (!avlQ.empty());

  for (auto blk : env.rpoBlocks) {
    auto& state = env.states[blk->id()];
    FTRACE(4,
           "InsertIncDecs: Blk(B{}) <- {}\n"
           "{}"
           "  ->{}\n",
           blk->id(),
           [&] {
             std::string ret;
             blk->forEachPred([&] (Block* pred) {
                 folly::format(&ret, " B{}", pred->id());
               });
             return ret;
           }(),
           show(state),
           [&] {
             std::string ret;
             blk->forEachSucc([&] (Block* succ) {
                 folly::format(&ret, " B{}", succ->id());
               });
             return ret;
           }());

    auto inc = state.local;
    for (auto inc_id = 0; inc.any(); inc >>= 1, inc_id++) {
      if (inc.test(0)) {
        auto const& tmps = env.insertMap[inc_id];
        auto insert = [&] (IRInstruction* inst) {
          FTRACE(3, "Inserting IncRef into B{}\n", blk->id());
          auto const iter = std::next(blk->iteratorTo(inst));
          blk->insert(iter, env.unit.gen(IncRef, inst->bcctx(), tmps[0]));
        };
        SSATmp* last = nullptr;
        // Insert an IncRef after every candidate in this block except
        // the last one (since we know for all but the last that its
        // successor is anticipated). Note that entries in tmps from
        // the same block are guaranteed to be in program order.
        for (auto const tmp : tmps) {
          if (tmp->inst()->block() != blk) continue;
          if (last) insert(last->inst());
          last = tmp;
        }
        // If it's partially anticipated out, insert an inc after the
        // last one too.
        always_assert(last);
        if (state.pantOut.test(inc_id)) insert(last->inst());
      }
    }
    auto dec = state.avlIn & ~state.pantIn;
    if (dec.any()) {
      blk->forEachPred(
        [&] (Block* pred) {
          auto& pstate = env.states[pred->id()];
          dec &= pstate.pantOut;
        });

      for (auto dec_id = 0; dec.any(); dec >>= 1, dec_id++) {
        if (dec.test(0)) {
          FTRACE(3, "Inserting DecRef into B{}\n", blk->id());
          auto const tmp = env.insertMap[dec_id][0];
          blk->prepend(env.unit.gen(DecRef, tmp->inst()->bcctx(),
                                    DecRefData{}, tmp));
        }
      }
    }
  }
}

using ActionMap = jit::fast_map<SSATmp*, std::vector<SSATmp*>>;

void tryReplaceInstruction(
  IRUnit& unit,
  const IdomVector& idoms,
  IRInstruction* inst,
  ValueNumberTable& table,
  ActionMap& actionMap
) {
  if (inst->hasDst()) {
    auto const dst = inst->dst();
    auto const valueNumber = table[dst].value;
    if (valueNumber &&
        valueNumber != dst &&
        is_tmp_usable(idoms, valueNumber, inst->block())) {
      if (inst->producesReference()) {
        auto& v = actionMap[valueNumber];
        if (!v.size()) v.push_back(valueNumber);
        v.push_back(dst);
      }
      if (!(valueNumber->type() <= dst->type())) {
        FTRACE(1,
               "replacing {} with AssertType({})\n",
               inst->toString(),
               dst->type().toString()
              );
        unit.replace(inst, AssertType, dst->type(), valueNumber);
      }
    }
  }

  for (uint32_t i = 0; i < inst->numSrcs(); ++i) {
    auto s = inst->src(i);
    auto valueNumber = table[s].value;
    if (valueNumber == s) continue;
    if (!valueNumber) continue;
    if (!is_tmp_usable(idoms, valueNumber, inst->block())) continue;

    // If the replacement is at least as refined as the source type,
    // just substitute it directly
    if (valueNumber->type() <= s->type()) {
      FTRACE(1,
             "instruction {}\n"
             "replacing src {} with dst of {}\n",
             *inst,
             i,
             *valueNumber->inst()
            );
      inst->setSrc(i, valueNumber);
    }
  }
}

void replaceRedundantComputations(
  IRUnit& unit,
  const IdomVector& idoms,
  const BlockList& blocks,
  ValueNumberTable& table
) {
  ActionMap actionMap;
  for (auto block : blocks) {
    for (auto& inst : *block) {
      tryReplaceInstruction(unit, idoms, &inst, table, actionMap);
    }
  }
  if (actionMap.empty()) return;
  PrcEnv env(unit, blocks);
  for (auto& elm : actionMap) {
    if (env.insertMap.size() == kMaxTrackedPrcs) {
      // This pretty much doesn't happen; when it does, we might be
      // over-increffing here - but thats not a big deal.
      auto const newTmp = elm.second[0];
      auto const block = newTmp->inst()->block();
      auto const iter = std::next(block->iteratorTo(newTmp->inst()));
      for (auto i = 1; i < elm.second.size(); i++) {
        block->insert(iter, unit.gen(IncRef, newTmp->inst()->bcctx(), newTmp));
      }
      continue;
    }
    env.insertMap.push_back(std::move(elm.second));
  }
  insertIncRefs(env);
}

} // namespace

/////////////////////////////////////////////////////////////////////////

void gvn(IRUnit& unit) {
  PassTracer tracer{&unit, Trace::hhir_gvn, "gvn"};
  Timer t(Timer::optimize_gvn, unit.logEntry().get_pointer());
  splitCriticalEdges(unit);

  GVNState state;
  auto const rpoBlocks = rpoSortCfg(unit);
  auto const idoms = findDominators(
    unit,
    rpoBlocks,
    numberBlocks(unit, rpoBlocks)
  );

  ValueNumberTable globalTable(unit, ValueNumberMetadata{});
  state.globalTable = &globalTable;

  // This is an implementation of the RPO version of the global value numbering
  // algorithm presented in the 1996 paper "SCC-based Value Numbering" by
  // Cooper and Simpson.
  runAnalysis(state, unit, rpoBlocks);
  replaceRedundantComputations(unit, idoms, rpoBlocks, globalTable);
  state.globalTable = nullptr;
}

}}
