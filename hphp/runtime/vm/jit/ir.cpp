/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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

#include "hphp/runtime/vm/jit/ir.h"

#include <algorithm>
#include <cstring>
#include <forward_list>
#include <sstream>
#include <type_traits>
#include <boost/algorithm/string.hpp>

#include "folly/Format.h"
#include "folly/Traits.h"

#include "hphp/util/trace.h"
#include "hphp/runtime/base/string-data.h"
#include "hphp/runtime/vm/runtime.h"
#include "hphp/runtime/base/stats.h"
#include "hphp/runtime/vm/jit/cse.h"
#include "hphp/runtime/vm/jit/ir-instruction.h"
#include "hphp/runtime/vm/jit/ir-factory.h"
#include "hphp/runtime/vm/jit/linear-scan.h"
#include "hphp/runtime/vm/jit/print.h"
#include "hphp/runtime/vm/jit/simplifier.h"
#include "hphp/runtime/vm/jit/ir-trace.h"

// Include last to localize effects to this file
#include "hphp/util/assert-throw.h"

namespace HPHP {  namespace JIT {

using namespace HPHP::Transl;

#define IRT(name, ...) const Type Type::name(Type::k##name);
IR_TYPES
#undef IRT

std::string Type::toString() const {
  // Try to find an exact match to a predefined type
# define IRT(name, ...) if (*this == name) return #name;
  IR_TYPES
# undef IRT

  if (strictSubtypeOf(Type::Obj)) {
    return folly::format("Obj<{}>", m_class->name()->data()).str();
  }

  if (canSpecializeArrayKind() && hasArrayKind()) {
    std::string typeName = [&] {
#     define IRT(name, ...) if (subtypeOf(name)) return #name;
      IR_TYPES
#     undef IRT
      not_reached();
    }();
    return folly::format("{}<{}>", typeName,
                         ArrayData::kindToString(m_arrayKind)).str();
  }

  // Concat all of the primitive types in the custom union type
  std::vector<std::string> types;
# define IRT(name, ...) if (name.subtypeOf(*this)) types.push_back(#name);
  IRT_PRIMITIVE
# undef IRT
  return folly::format("{{{}}}", folly::join('|', types)).str();
}

std::string Type::debugString(Type t) {
  return t.toString();
}

Type Type::fromString(const std::string& str) {
  static hphp_string_map<Type> types;
  static bool init = false;
  if (UNLIKELY(!init)) {
#   define IRT(name, ...) types[#name] = name;
    IR_TYPES
#   undef IRT
    init = true;
  }
  return mapGet(types, str, Type::None);
}

TRACE_SET_MOD(hhir);

namespace {

#define NF     0
#define C      CanCSE
#define E      Essential
#define N      CallsNative
#define PRc    ProducesRC
#define CRc    ConsumesRC
#define Refs   MayModifyRefs
#define Er     MayRaiseError
#define Mem    MemEffects
#define T      Terminal
#define P      Passthrough
#define K      KillsSources
#define StkFlags(f) HasStackVersion|(f)
#define MProp  MInstrProp
#define MElem  MInstrElem

#define ND        0
#define D(n)      HasDest
#define DofS(n)   HasDest
#define DUnbox(n) HasDest
#define DBox(n)   HasDest
#define DParam    HasDest
#define DArith    HasDest
#define DMulti    NaryDest
#define DSetElem  HasDest
#define DStk(x)   ModifiesStack|(x)
#define DPtrToParam HasDest
#define DBuiltin  HasDest
#define DSubtract(n,t) HasDest

struct {
  const char* name;
  uint64_t flags;
} OpInfo[] = {
#define O(name, dsts, srcs, flags)                    \
    { #name,                                          \
       (OpHasExtraData<name>::value ? HasExtra : 0) | \
       dsts | (flags)                                 \
    },
  IR_OPCODES
#undef O
  { 0 }
};

#undef NF
#undef C
#undef E
#undef PRc
#undef CRc
#undef Refs
#undef Er
#undef Mem
#undef T
#undef P
#undef K
#undef StkFlags
#undef MProp
#undef MElem

#undef ND
#undef D
#undef DofS
#undef DUnbox
#undef DBox
#undef DParam
#undef DArith
#undef DMulti
#undef DSetElem
#undef DStk
#undef DPtrToParam
#undef DBuiltin
#undef DSubtract

} // namespace

//////////////////////////////////////////////////////////////////////

const char* opcodeName(Opcode opcode) {
  return OpInfo[uint16_t(opcode)].name;
}

bool opcodeHasFlags(Opcode opcode, uint64_t flags) {
  return OpInfo[uint16_t(opcode)].flags & flags;
}

bool opHasExtraData(Opcode op) {
  return opcodeHasFlags(op, HasExtra);
}

Opcode getStackModifyingOpcode(Opcode opc) {
  assert(opcodeHasFlags(opc, HasStackVersion));
  opc = Opcode(uint64_t(opc) + 1);
  assert(opcodeHasFlags(opc, ModifiesStack));
  return opc;
}

const StringData* findClassName(SSATmp* cls) {
  assert(cls->isA(Type::Cls));

  if (cls->isConst()) {
    return cls->getValClass()->preClass()->name();
  }
  // Try to get the class name from a LdCls
  IRInstruction* clsInst = cls->inst();
  if (clsInst->op() == LdCls || clsInst->op() == LdClsCached) {
    SSATmp* clsName = clsInst->src(0);
    assert(clsName->isA(Type::Str));
    if (clsName->isConst()) {
      return clsName->getValStr();
    }
  }
  return nullptr;
}

bool isGuardOp(Opcode opc) {
  switch (opc) {
    case GuardLoc:
    case CheckLoc:
    case GuardStk:
    case CheckStk:
    case CheckType:
      return true;

    default:
      return false;
  }
}

bool isQueryOp(Opcode opc) {
  switch (opc) {
  case Gt:
  case Gte:
  case Lt:
  case Lte:
  case Eq:
  case Neq:
  case Same:
  case NSame:
  case InstanceOfBitmask:
  case NInstanceOfBitmask:
  case IsType:
  case IsNType:
    return true;
  default:
    return false;
  }
}

bool isCmpOp(Opcode opc) {
  switch (opc) {
  case Gt:
  case Gte:
  case Lt:
  case Lte:
  case Eq:
  case Neq:
  case Same:
  case NSame:
    return true;
  default:
    return false;
  }
}

bool isQueryJmpOp(Opcode opc) {
  switch (opc) {
  case JmpGt:
  case JmpGte:
  case JmpLt:
  case JmpLte:
  case JmpEq:
  case JmpNeq:
  case JmpSame:
  case JmpNSame:
  case JmpInstanceOfBitmask:
  case JmpNInstanceOfBitmask:
  case JmpIsType:
  case JmpIsNType:
  case JmpZero:
  case JmpNZero:
    return true;
  default:
    return false;
  }
}

Opcode queryToJmpOp(Opcode opc) {
  assert(isQueryOp(opc));
  switch (opc) {
  case Gt:                 return JmpGt;
  case Gte:                return JmpGte;
  case Lt:                 return JmpLt;
  case Lte:                return JmpLte;
  case Eq:                 return JmpEq;
  case Neq:                return JmpNeq;
  case Same:               return JmpSame;
  case NSame:              return JmpNSame;
  case InstanceOfBitmask:  return JmpInstanceOfBitmask;
  case NInstanceOfBitmask: return JmpNInstanceOfBitmask;
  case IsType:             return JmpIsType;
  case IsNType:            return JmpIsNType;
  default:                 always_assert(0);
  }
}

Opcode queryJmpToQueryOp(Opcode opc) {
  assert(isQueryJmpOp(opc));
  switch (opc) {
  case JmpGt:                 return Gt;
  case JmpGte:                return Gte;
  case JmpLt:                 return Lt;
  case JmpLte:                return Lte;
  case JmpEq:                 return Eq;
  case JmpNeq:                return Neq;
  case JmpSame:               return Same;
  case JmpNSame:              return NSame;
  case JmpInstanceOfBitmask:  return InstanceOfBitmask;
  case JmpNInstanceOfBitmask: return NInstanceOfBitmask;
  case JmpIsType:             return IsType;
  case JmpIsNType:            return IsNType;
  default:                    always_assert(0);
  }
}

Opcode jmpToReqBindJmp(Opcode opc) {
  switch (opc) {
  case JmpGt:                 return ReqBindJmpGt;
  case JmpGte:                return ReqBindJmpGte;
  case JmpLt:                 return ReqBindJmpLt;
  case JmpLte:                return ReqBindJmpLte;
  case JmpEq:                 return ReqBindJmpEq;
  case JmpNeq:                return ReqBindJmpNeq;
  case JmpSame:               return ReqBindJmpSame;
  case JmpNSame:              return ReqBindJmpNSame;
  case JmpInstanceOfBitmask:  return ReqBindJmpInstanceOfBitmask;
  case JmpNInstanceOfBitmask: return ReqBindJmpNInstanceOfBitmask;
  case JmpZero:               return ReqBindJmpZero;
  case JmpNZero:              return ReqBindJmpNZero;
  default:                    always_assert(0);
  }
}

Opcode negateQueryOp(Opcode opc) {
  assert(isQueryOp(opc));
  switch (opc) {
  case Gt:                  return Lte;
  case Gte:                 return Lt;
  case Lt:                  return Gte;
  case Lte:                 return Gt;
  case Eq:                  return Neq;
  case Neq:                 return Eq;
  case Same:                return NSame;
  case NSame:               return Same;
  case InstanceOfBitmask:   return NInstanceOfBitmask;
  case NInstanceOfBitmask:  return InstanceOfBitmask;
  case IsType:              return IsNType;
  case IsNType:             return IsType;
  default:                  always_assert(0);
  }
}

Opcode commuteQueryOp(Opcode opc) {
  assert(isQueryOp(opc));
  switch (opc) {
  case Gt:    return Lt;
  case Gte:   return Lte;
  case Lt:    return Gt;
  case Lte:   return Gte;
  case Eq:    return Eq;
  case Neq:   return Neq;
  case Same:  return Same;
  case NSame: return NSame;
  default:      always_assert(0);
  }
}

// Objects compared with strings may involve calling a user-defined
// __toString function.
bool cmpOpTypesMayReenter(Opcode op, Type t0, Type t1) {
  if (op == NSame || op == Same) return false;
  assert(!t0.equals(Type::Gen) && !t1.equals(Type::Gen));
  return (t0.maybe(Type::Obj) && t1.maybe(Type::Str)) ||
         (t0.maybe(Type::Str) && t1.maybe(Type::Obj));
}

bool isRefCounted(SSATmp* tmp) {
  return tmp->type().maybeCounted() && !tmp->isConst();
}

int32_t spillValueCells(IRInstruction* spillStack) {
  assert(spillStack->op() == SpillStack);
  int32_t numSrcs = spillStack->numSrcs();
  return numSrcs - 2;
}

bool isConvIntOrPtrToBool(IRInstruction* instr) {
  switch (instr->op()) {
    case ConvIntToBool:
      return true;
    case ConvCellToBool:
      return instr->src(0)->type().subtypeOfAny(
        Type::Func, Type::Cls, Type::FuncCls, Type::VarEnv, Type::TCA);
    default:
      return false;
  }
}

BlockList rpoSortCfg(IRTrace* trace, const IRFactory& factory) {
  assert(trace->isMain());
  BlockList blocks;
  blocks.reserve(factory.numBlocks());
  unsigned next_id = 0;
  postorderWalk(
    [&](Block* block) {
      block->setPostId(next_id++);
      blocks.push_back(block);
    },
    factory.numBlocks(),
    trace->front()
  );
  std::reverse(blocks.begin(), blocks.end());
  assert(blocks.size() <= factory.numBlocks());
  assert(next_id <= factory.numBlocks());
  return blocks;
}

bool isRPOSorted(const BlockList& blocks) {
  int id = 0;
  for (auto it = blocks.rbegin(); it != blocks.rend(); ++it) {
    if ((*it)->postId() != id++) return false;
  }
  return true;
}

/*
 * Find the immediate dominator of each block using Cooper, Harvey, and
 * Kennedy's "A Simple, Fast Dominance Algorithm", returned as a vector
 * of postorder ids, indexed by postorder id.
 */
IdomVector findDominators(const BlockList& blocks) {
  assert(isRPOSorted(blocks));

  // Calculate immediate dominators with the iterative two-finger algorithm.
  // When it terminates, idom[post-id] will contain the post-id of the
  // immediate dominator of each block.  idom[start] will be -1.  This is
  // the general algorithm but it will only loop twice for loop-free graphs.
  auto const num_blocks = blocks.size();
  IdomVector idom(num_blocks, -1);
  auto start = blocks.begin();
  int start_id = (*start)->postId();
  idom[start_id] = start_id;
  start++;
  for (bool changed = true; changed; ) {
    changed = false;
    // for each block after start, in reverse postorder
    for (auto it = start; it != blocks.end(); it++) {
      Block* block = *it;
      int b = block->postId();
      // new_idom = any already-processed predecessor
      auto edge_it = block->preds().begin();
      int new_idom = edge_it->from()->postId();
      while (idom[new_idom] == -1) new_idom = (++edge_it)->from()->postId();
      // for all other already-processed predecessors p of b
      for (auto& edge : block->preds()) {
        auto p = edge.from()->postId();
        if (p != new_idom && idom[p] != -1) {
          // find earliest common predecessor of p and new_idom
          // (higher postIds are earlier in flow and in dom-tree).
          int b1 = p, b2 = new_idom;
          do {
            while (b1 < b2) b1 = idom[b1];
            while (b2 < b1) b2 = idom[b2];
          } while (b1 != b2);
          new_idom = b1;
        }
      }
      if (idom[b] != new_idom) {
        idom[b] = new_idom;
        changed = true;
      }
    }
  }
  idom[start_id] = -1; // start has no idom.
  return idom;
}

bool dominates(const Block* b1, const Block* b2, const IdomVector& idoms) {
  int p1 = b1->postId();
  int p2 = b2->postId();
  for (int i = p2; i != -1; i = idoms[i]) {
    if (i == p1) return true;
  }
  return false;
}

DomChildren findDomChildren(const BlockList& blocks) {
  IdomVector idom = findDominators(blocks);
  DomChildren children(blocks.size(), BlockList());
  for (Block* block : blocks) {
    int idom_id = idom[block->postId()];
    if (idom_id != -1) children[idom_id].push_back(block);
  }
  return children;
}

bool hasInternalFlow(IRTrace* trace) {
  for (Block* block : trace->blocks()) {
    if (Block* taken = block->taken()) {
      if (taken->trace() == trace) return true;
    }
  }
  return false;
}

}}

