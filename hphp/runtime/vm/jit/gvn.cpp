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

#include "hphp/runtime/vm/jit/analysis.h"
#include "hphp/runtime/vm/jit/block.h"
#include "hphp/runtime/vm/jit/cfg.h"
#include "hphp/runtime/vm/jit/ir-instruction.h"
#include "hphp/runtime/vm/jit/ssa-tmp.h"
#include "hphp/runtime/vm/jit/state-vector.h"
#include <unordered_map>

namespace HPHP { namespace jit {

TRACE_SET_MOD(gvn);

//////////////////////////////////////////////////////////////////////

namespace {

struct ValueNumberMetadata {
  SSATmp* key;
  SSATmp* value;
};
typedef StateVector<SSATmp, ValueNumberMetadata> ValueNumberTable;

struct CongruenceHasher {
  explicit CongruenceHasher(const ValueNumberTable& vnTable)
    : m_vnTable(vnTable)
  {
  }

  size_t operator()(IRInstruction* inst) const {
    // Note: this doesn't take commutativity or associativity into account, but
    // it might be nice to do so for the opcodes where it makes sense.
    size_t result = static_cast<size_t>(inst->op());

    for (uint32_t i = 0; i < inst->numSrcs(); ++i) {
      auto src = inst->src(i);
      assert(m_vnTable[src].value);
      result = folly::hash::hash_128_to_64(
        result,
        reinterpret_cast<size_t>(m_vnTable[src].value)
      );
    }

    if (inst->hasExtra()) {
      result = folly::hash::hash_128_to_64(
        result,
        cseHashExtra(inst->op(), inst->rawExtra())
      );
    }

    if (inst->hasTypeParam()) {
      result = folly::hash::hash_128_to_64(result, inst->typeParam().hash());
    }

    return result;
  }

private:
  const ValueNumberTable& m_vnTable;
};

struct CongruenceComparator {
  explicit CongruenceComparator(const ValueNumberTable& vnTable)
    : m_vnTable(vnTable)
  {
  }

  bool operator()(IRInstruction* instA, IRInstruction* instB) const {
    // Note: this doesn't take commutativity or associativity into account, but
    // it might be nice to do so for the opcodes where it makes sense.
    if (instA->op() != instB->op()) return false;
    if (instA->numSrcs() != instB->numSrcs()) return false;
    if (instA->hasTypeParam() != instB->hasTypeParam()) return false;
    if (instA->hasTypeParam() && instA->typeParam() != instB->typeParam()) {
      return false;
    }

    for (uint32_t i = 0; i < instA->numSrcs(); ++i) {
      auto srcA = instA->src(i);
      auto srcB = instB->src(i);
      assert(m_vnTable[srcA].value);
      assert(m_vnTable[srcB].value);
      if (m_vnTable[srcA].value != m_vnTable[srcB].value) return false;
    }

    if (instA->hasExtra()) {
      assert(instB->hasExtra());
      if (!cseEqualsExtra(instA->op(), instA->rawExtra(), instB->rawExtra())) {
        return false;
      }
    }
    return true;
  }

private:
  const ValueNumberTable& m_vnTable;
};

typedef std::unordered_map<
  IRInstruction*, SSATmp*,
  CongruenceHasher,
  CongruenceComparator> NameTable;

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
  case ConvIntToArr:
  case ConvDblToBool:
  case ConvIntToBool:
  case ConvBoolToDbl:
  case ConvIntToDbl:
  case ConvBoolToInt:
  case ConvDblToInt:
  case ConvBoolToStr:
  case ConvClsToCctx:
  case Gt:
  case GtX:
  case Gte:
  case GteX:
  case Lt:
  case LtX:
  case Lte:
  case LteX:
  case Eq:
  case EqX:
  case Neq:
  case NeqX:
  case Same:
  case NSame:
  case GtInt:
  case GteInt:
  case LtInt:
  case LteInt:
  case EqInt:
  case NeqInt:
  case GtDbl:
  case GteDbl:
  case LtDbl:
  case LteDbl:
  case EqDbl:
  case NeqDbl:
  case InstanceOf:
  case InstanceOfIface:
  case ExtendsClass:
  case InstanceOfBitmask:
  case NInstanceOfBitmask:
  case InterfaceSupportsArr:
  case InterfaceSupportsStr:
  case InterfaceSupportsInt:
  case InterfaceSupportsDbl:
  case IsType:
  case IsNType:
  case IsScalarType:
  case IsWaitHandle:
  case ClsNeq:
  case LdStackAddr:
  case LdLocAddr:
  case LdRDSAddr:
  case LdCtx:
  case CastCtxThis:
  case LdCctx:
  case LdClsCtx:
  case LdClsCctx:
  case LdClsCtor:
  case DefConst:
  case LdCls:
  case LdClsCached:
  case LdClsInitData:
  case LookupClsRDSHandle:
  case LdClsMethod:
  case LdPropAddr:
  case LdClsPropAddrOrNull:
  case LdClsPropAddrOrRaise:
  case LdObjClass:
  case LdClsName:
  case LdARFuncPtr:
  case Mov:
  case LdContActRec:
  case LdAFWHActRec:
  case LdResumableArObj:
  case LdMIStateAddr:
    return true;
  default:
    return false;
  }
}

void initWithInstruction(IRInstruction* inst, ValueNumberTable& table) {
  // Each SSATmp starts out as the canonical name for itself.
  for (auto& dst : inst->dsts()) {
    table[&dst] = ValueNumberMetadata { &dst, &dst };
  }

  for (auto src : inst->srcs()) {
    table[src] = ValueNumberMetadata { src, src };
  }
}

bool visitDefLabel(
  IRInstruction* inst,
  ValueNumberTable& vnTable,
  ValueNumberTable& updates,
  NameTable& nameTable
) {
  assert(inst->is(DefLabel));
  bool changed = false;

  /*
   * We can forward value numbers through DefLabels if all of the predecessor
   * blocks agree on the value number of the things being joined. We look at
   * each dst separately, and there are two main cases we care about:
   *
   * x_2 = phi(x_1, x_1) => x_1
   * x_2 = phi(x_1, x_2) => x_1
   *
   * where x_1 and x_2 is the value number of the variable being joined.
   *
   * The first is the case where all the predecssors agree on the value number
   * of the inputs to the phi.
   *
   * The second case handles the situation where the phi uses the variable it's
   * defining (as in the case of a loop back edge), so obviously the value
   * number of the dst should be the other value number flowing into the phi.
   */
  for (uint32_t i = 0; i < inst->numDsts(); ++i) {
    auto dst = inst->dst(i);
    auto block = inst->block();
    // The forwardedDst is the first value number we see that's not dst.
    auto forwardedDst = dst;
    bool canForwardDst = true;

    for (auto& pred : block->preds()) {
      auto fromBlock = pred.from();
      auto& jmp = fromBlock->back();
      assert(jmp.is(Jmp));
      auto src = jmp.src(i);
      assert(vnTable[src].value);
      auto srcVN = vnTable[src].value;

      // Update forwardedDst if this is the first src we've seen that doesn't
      // match our original dst.
      if (forwardedDst == dst && srcVN != dst) {
        forwardedDst = srcVN;
      }

      // If we see either our original dst or the first non-matching dst, we're
      // okay so we keep going.
      if (srcVN == forwardedDst) continue;
      if (srcVN == dst) continue;

      // At this point, the value number does not match the other forwardedDst
      // we've seen (so it can't be case 1) and it doesn't match dst (so it
      // can't be case 2), so we have to give up.
      canForwardDst = false;
      break;
    }

    assert(vnTable[dst].value);
    if (canForwardDst && vnTable[dst].value != forwardedDst) {
      TRACE(2, "gvn: forwarded through DefLabel: %p (old) -> %p (new)\n",
        dst, forwardedDst);
      updates[dst] = ValueNumberMetadata { dst, forwardedDst };
      changed = true;
    }
  }

  return changed;
}

bool visitInstruction(
  IRInstruction* inst,
  ValueNumberTable& vnTable,
  ValueNumberTable& updates,
  NameTable& nameTable
) {
  if (isCallOp(inst->op())) nameTable.clear();

  if (inst->is(DefLabel)) {
    return visitDefLabel(inst, vnTable, updates, nameTable);
  }

  if (!supportsGVN(inst)) return false;

  assert(inst->numDsts() == 1);
  auto dst = inst->dst(0);
  assert(dst);

  SSATmp* temp;
  auto iter = nameTable.find(inst);
  if (iter == nameTable.end()) {
    temp = dst;
    nameTable.insert(std::make_pair(inst, dst));
  } else {
    temp = iter->second;
  }

  assert(temp);

  assert(vnTable[dst].value);
  if (temp != vnTable[dst].value) {
    updates[dst] = ValueNumberMetadata { dst, temp };
    TRACE(2, "\ngvn: instruction %s\n", inst->toString().c_str());
    TRACE(2, "gvn: updated value number for dst to dst of %s\n",
      temp->inst()->toString().c_str());
    return true;
  }
  return false;
}

bool visitBlock(
  Block* block,
  ValueNumberTable& vnTable,
  ValueNumberTable& updates,
  NameTable& nameTable
) {
  bool changed = false;
  for (auto& inst : *block) {
    changed = visitInstruction(&inst, vnTable, updates, nameTable) || changed;
  }
  return changed;
}

void applyLocalUpdates(ValueNumberTable& global, ValueNumberTable& local) {
  for (auto metadata : local) {
    if (!metadata.key) continue;
    global[metadata.key] = metadata;
  }
}

void runAnalysis(IRUnit& unit, BlockList& blocks, ValueNumberTable& vnTable) {
  for (auto block : blocks) {
    for (auto& inst : *block) {
      initWithInstruction(&inst, vnTable);
    }
  }

  bool changed = true;
  while (changed) {
    // We need a temporary table of updates which we apply after running this
    // iteration of the fixed point. If we change the global ValueNumberTable
    // during the pass, the hash values of the SSATmps will change which is
    // apparently a no-no for unordered_map.
    ValueNumberTable localUpdates(unit,
      ValueNumberMetadata { nullptr, nullptr });
    {
      CongruenceHasher hash(vnTable);
      CongruenceComparator pred(vnTable);
      NameTable nameTable(0, hash, pred);

      changed = false;
      for (auto block : blocks) {
        changed = visitBlock(
          block,
          vnTable,
          localUpdates,
          nameTable
        ) || changed;
      }
    }
    applyLocalUpdates(vnTable, localUpdates);
  }
}

bool canReplaceWith(
  IdomVector& idoms,
  SSATmp* dst,
  SSATmp* other
) {
  assert(other->type() <= dst->type());
  if (other->inst()->is(DefConst)) return true;
  auto const definingBlock = findDefiningBlock(other);
  if (!definingBlock) return false;
  return dominates(definingBlock, dst->inst()->block(), idoms);
}

void tryReplaceInstruction(
  IRUnit& unit,
  IdomVector& idoms,
  IRInstruction* inst,
  ValueNumberTable& table
) {
  for (uint32_t i = 0; i < inst->numSrcs(); ++i) {
    auto s = inst->src(i);
    auto valueNumber = table[s].value;
    if (valueNumber == s) continue;
    if (!valueNumber) continue;
    if (!canReplaceWith(idoms, s, valueNumber)) continue;
    TRACE(2, "\ngvn: instruction %s\n", inst->toString().c_str());
    TRACE(2, "gvn: replacing src %d with dst of %s\n",
      i, valueNumber->inst()->toString().c_str());
    inst->setSrc(i, valueNumber);
    if (valueNumber->inst()->producesReference(0)) {
      auto prevInst = valueNumber->inst();
      auto block = prevInst->block();
      auto iter = block->iteratorTo(prevInst);
      block->insert(++iter, unit.gen(IncRef, prevInst->marker(), valueNumber));
    }
  }
}

void replaceRedundantComputations(
  IRUnit& unit,
  IdomVector& idoms,
  BlockList& blocks,
  ValueNumberTable& table
) {
  for (auto block : blocks) {
    for (auto& inst : *block) {
      tryReplaceInstruction(unit, idoms, &inst, table);
    }
  }
}

} // namespace

/////////////////////////////////////////////////////////////////////////

void gvn(IRUnit& unit) {
  auto rpoBlocksWithIds = rpoSortCfgWithIds(unit);
  auto& rpoBlocks = rpoBlocksWithIds.blocks;
  auto dominators = findDominators(unit, rpoBlocksWithIds);
  ValueNumberTable vnTable(unit, ValueNumberMetadata { nullptr, nullptr });

  // This is an implementation of the RPO version of the global value numbering
  // algorithm presented in the 1996 paper "SCC-based Value Numbering" by
  // Cooper and Simpson.
  runAnalysis(unit, rpoBlocks, vnTable);
  replaceRedundantComputations(unit, dominators, rpoBlocks, vnTable);
}

} } // namespace HPHP::jit
