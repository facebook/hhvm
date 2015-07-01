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
#include "hphp/runtime/vm/jit/containers.h"
#include "hphp/runtime/vm/jit/ir-instruction.h"
#include "hphp/runtime/vm/jit/ssa-tmp.h"
#include "hphp/runtime/vm/jit/state-vector.h"
#include "hphp/runtime/vm/jit/pass-tracer.h"
#include <unordered_map>

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
    : m_globalTable(globalTable)
  {
  }

  size_t hashDefLabel(KeyType key) const {
    auto inst = key.first;
    auto idx = key.second;

    // We use a set (instead of an unordered_set) because we want a fixed and
    // well-defined iteration order while we're accumulating the hash below.
    jit::set<SSATmp*> values;
    for (auto& pred : inst->block()->preds()) {
      auto fromBlock = pred.from();
      auto& jmp = fromBlock->back();
      auto src = canonical(jmp.src(idx));
      assertx(m_globalTable[src].value);
      values.emplace(m_globalTable[src].value);
    }

    auto result = static_cast<size_t>(inst->op());
    for (auto value : values) {
      result = folly::hash::hash_128_to_64(
        result,
        reinterpret_cast<size_t>(value)
      );
    }
    return hashSharedImpl(inst, result);
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
    auto inst = key.first;
    for (uint32_t i = 0; i < inst->numSrcs(); ++i) {
      auto src = canonical(inst->src(i));
      assertx(m_globalTable[src].value);
      result = folly::hash::hash_128_to_64(
        result,
        reinterpret_cast<size_t>(m_globalTable[src].value)
      );
    }
    return result;
  }

  size_t operator()(KeyType key) const {
    auto inst = key.first;

    if (inst->is(DefLabel)) return hashDefLabel(key);

    // Note: this doesn't take commutativity or associativity into account, but
    // it might be nice to do so for the opcodes where it makes sense.
    size_t result = static_cast<size_t>(inst->op());
    result = hashSrcs(key, result);
    return hashSharedImpl(inst, result);
  }

private:
  const ValueNumberTable& m_globalTable;
};

struct CongruenceComparator {
  using KeyType = std::pair<IRInstruction*, DstIndex>;

  explicit CongruenceComparator(const ValueNumberTable& globalTable)
    : m_globalTable(globalTable)
  {
  }

  bool compareDefLabelSrcs(KeyType keyA, KeyType keyB) const {
    auto instA = keyA.first;
    auto instB = keyB.first;
    auto idxA = keyA.second;
    auto idxB = keyB.second;

    assert(instA->op() == instB->op());
    assert(instA->is(DefLabel));

    jit::hash_set<SSATmp*> valuesA;
    jit::hash_set<SSATmp*> valuesB;

    auto fillValueSet = [&](
        IRInstruction* inst,
        int32_t idx,
        std::unordered_set<SSATmp*>& values
    ) {
      for (auto& pred : inst->block()->preds()) {
        auto fromBlock = pred.from();
        auto& jmp = fromBlock->back();
        auto src = canonical(jmp.src(idx));
        assertx(m_globalTable[src].value);
        values.emplace(m_globalTable[src].value);
      }
    };
    fillValueSet(instA, idxA, valuesA);
    fillValueSet(instB, idxB, valuesB);
    return valuesA == valuesB;
  }

  bool compareSrcs(KeyType keyA, KeyType keyB) const {
    auto instA = keyA.first;
    auto instB = keyB.first;

    for (uint32_t i = 0; i < instA->numSrcs(); ++i) {
      auto srcA = canonical(instA->src(i));
      auto srcB = canonical(instB->src(i));
      assertx(m_globalTable[srcA].value);
      assertx(m_globalTable[srcB].value);
      if (m_globalTable[srcA].value != m_globalTable[srcB].value) return false;
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

    if (instA->is(DefLabel)) {
      if (!compareDefLabelSrcs(keyA, keyB)) return false;
    } else if (!compareSrcs(keyA, keyB)) {
      return false;
    }

    return true;
  }

private:
  const ValueNumberTable& m_globalTable;
};

using NameTable = std::unordered_map<
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
  case Gte:
  case Lt:
  case Lte:
  case Eq:
  case Neq:
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
  case GtStr:
  case GteStr:
  case LtStr:
  case LteStr:
  case EqStr:
  case NeqStr:
  case SameStr:
  case NSameStr:
  case InstanceOf:
  case InstanceOfIface:
  case InstanceOfIfaceVtable:
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
  case LdStkAddr:
  case LdLocAddr:
  case LdRDSAddr:
  case LdCtx:
  case LdCctx:
  case CastCtxThis:
  case LdClsCtx:
  case LdClsCctx:
  case LdClsCtor:
  case DefConst:
  case LdCls:
  case LdClsCached:
  case LdClsInitData:
  case LookupClsRDSHandle:
  case LdClsMethod:
  case LdIfaceMethod:
  case LdPropAddr:
  case LdClsPropAddrOrNull:
  case LdClsPropAddrOrRaise:
  case LdObjClass:
  case LdClsName:
  case LdARFuncPtr:
  case LdARNumParams:
  case Mov:
  case LdContActRec:
  case LdAFWHActRec:
  case LdResumableArObj:
  case LdMIStateAddr:
  case LdPackedArrayElemAddr:
  case OrdStr:
  case DefLabel:
  case CheckRange:
  case CountArrayFast:
    return true;
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

void tryReplaceInstruction(
  IRUnit& unit,
  const IdomVector& idoms,
  IRInstruction* inst,
  ValueNumberTable& table
) {
  for (uint32_t i = 0; i < inst->numSrcs(); ++i) {
    auto s = inst->src(i);
    auto valueNumber = table[s].value;
    auto valueInst = valueNumber->inst();
    if (valueNumber == s) continue;
    if (!valueNumber) continue;
    if (!is_tmp_usable(idoms, valueNumber, inst->block())) continue;
    FTRACE(1,
      "instruction {}\n"
      "replacing src {} with dst of {}\n",
      *inst,
      i,
      *valueInst
    );
    inst->setSrc(i, valueNumber);
    if (valueInst->producesReference()) {
      auto block = valueInst->block();
      auto iter = block->iteratorTo(valueInst);
      block->insert(++iter, unit.gen(IncRef, valueInst->marker(), valueNumber));
    }
  }
}

void replaceRedundantComputations(
  IRUnit& unit,
  const IdomVector& idoms,
  const BlockList& blocks,
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
  PassTracer tracer{&unit, Trace::hhir_gvn, "gvn"};

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
