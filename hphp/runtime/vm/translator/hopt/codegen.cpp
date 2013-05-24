/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
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

#include "hphp/runtime/vm/translator/hopt/codegen.h"

#include <string.h>

#include "folly/ScopeGuard.h"
#include "hphp/util/trace.h"
#include "hphp/util/util.h"

#include "hphp/runtime/base/array/hphp_array.h"
#include "hphp/runtime/base/comparisons.h"
#include "hphp/runtime/base/complex_types.h"
#include "hphp/runtime/base/runtime_option.h"
#include "hphp/runtime/base/string_data.h"
#include "hphp/runtime/base/types.h"
#include "hphp/runtime/ext/ext_closure.h"
#include "hphp/runtime/ext/ext_continuation.h"
#include "hphp/runtime/ext/ext_collections.h"
#include "hphp/runtime/vm/bytecode.h"
#include "hphp/runtime/vm/runtime.h"
#include "hphp/runtime/base/stats.h"
#include "hphp/runtime/vm/translator/targetcache.h"
#include "hphp/runtime/vm/translator/translator-inline.h"
#include "hphp/runtime/vm/translator/translator-x64.h"
#include "hphp/runtime/vm/translator/translator-x64-internal.h"
#include "hphp/runtime/vm/translator/translator.h"
#include "hphp/runtime/vm/translator/types.h"
#include "hphp/runtime/vm/translator/x64-util.h"
#include "hphp/runtime/vm/translator/hopt/ir.h"
#include "hphp/runtime/vm/translator/hopt/linearscan.h"
#include "hphp/runtime/vm/translator/hopt/nativecalls.h"
#include "hphp/runtime/vm/translator/hopt/print.h"

using HPHP::Transl::TCA;
using namespace HPHP::Transl::TargetCache;

namespace HPHP {
namespace JIT {

namespace {

//////////////////////////////////////////////////////////////////////

using namespace Util;
using namespace Transl::reg;

TRACE_SET_MOD(hhir);

/*
 * It's not normally ok to directly use tracelet abi registers in
 * codegen, unless you're directly dealing with an instruction that
 * does near-end-of-tracelet glue.  (Or also we sometimes use them
 * just for some static_assertions relating to calls to helpers from
 * tx64 that hardcode these registers.)
 */
using Transl::rVmFp;
using Transl::rVmSp;

const size_t kTypeWordOffset = (offsetof(TypedValue, m_type) % 8);
const size_t kTypeShiftBits = kTypeWordOffset * CHAR_BIT;

// left shift an immediate DataType, for type, to the correct position
// within one of the registers used to pass a TypedValue by value.
uint64_t toDataTypeForCall(Type type) {
  return uint64_t(type.toDataType()) << kTypeShiftBits;
}

int64_t spillSlotsToSize(int n) {
  return n * sizeof(int64_t);
}

void cgPunt(const char* file, int line, const char* func, uint32_t bcOff) {
  if (dumpIREnabled()) {
    HPHP::Trace::trace("--------- CG_PUNT %s %d %s  bcOff: %d \n",
                       file, line, func, bcOff);
  }
  throw FailedCodeGen(file, line, func, bcOff);
}

#define CG_PUNT(instr) cgPunt(__FILE__, __LINE__, #instr, m_curBcOff)

struct CycleInfo {
  int node;
  int length;
};

struct MoveInfo {
  enum Kind { Move, Xchg };

  MoveInfo(Kind kind, int reg1, int reg2):
      m_kind(kind), m_reg1(reg1), m_reg2(reg2) {}

  Kind m_kind;
  PhysReg m_reg1, m_reg2;
};

template <int N>
static bool cycleHasMMXReg(const CycleInfo& cycle,
                           const int (&moves)[N]) {
  int first = cycle.node;
  int node = first;
  do {
    if (PhysReg(node).isXMM()) return true;
    node = moves[node];
  } while (node != first);
  return false;
}

template <int N>
void doRegMoves(int (&moves)[N], int rTmp,
                std::vector<MoveInfo>& howTo) {
  assert(howTo.empty());
  int outDegree[N];
  CycleInfo cycles[N];
  int numCycles = 0;
  // Iterate over the nodes filling in outDegree[] and cycles[] as we go
  {
    int index[N];
    for (int node = 0; node < N; ++node) {
      // If a node's source is itself, its a nop
      if (moves[node] == node) moves[node] = -1;
      if (node == rTmp && moves[node] >= 0) {
        // ERROR: rTmp cannot be referenced in moves[].
        assert(false);
      }
      outDegree[node] = 0;
      index[node] = -1;
    }
    int nextIndex = 0;
    for (int startNode = 0; startNode < N; ++startNode) {
      // If startNode has not been visited yet, begin walking
      // a path from start node
      if (index[startNode] < 0) {
        int node = startNode;
pathloop:
        index[node] = nextIndex++;
        if (moves[node] >= 0) {
          int nextNode = moves[node];
          ++outDegree[nextNode];
          if (index[nextNode] < 0) {
            // If there is an edge from v to nextNode and nextNode has not been
            // visited, extend the current path to include nextNode and recurse
            node = nextNode;
            goto pathloop;
          }
          // There is an edge from v to nextNode but nextNode has already been
          // visited, check if nextNode is on the current path
          if (index[nextNode] >= index[startNode]) {
            // nextNode is on the current path so we've found a cycle
            int length = nextIndex - index[nextNode];
            CycleInfo ci = { nextNode, length };
            cycles[numCycles] = ci;
            ++numCycles;
          }
        }
      }
    }
  }
  // Handle all moves that aren't part of a cycle
  {
    int q[N];
    int qBack = 0;
    for (int node = 0; node < N; ++node) {
      if (outDegree[node] == 0) {
        q[qBack] = node;
        ++qBack;
      }
    }
    for (int i = 0; i < qBack; ++i) {
      int node = q[i];
      if (moves[node] >= 0) {
        int nextNode = moves[node];
        howTo.push_back(MoveInfo(MoveInfo::Move, nextNode, node));
        --outDegree[nextNode];
        if (outDegree[nextNode] == 0) {
          q[qBack] = nextNode;
          ++qBack;
        }
      }
    }
  }
  // Deal with any cycles we encountered
  for (int i = 0; i < numCycles; ++i) {
    // can't use xchg if one of the registers is MMX
    bool hasMMXReg = cycleHasMMXReg(cycles[i], moves);
    if (cycles[i].length == 2 && !hasMMXReg) {
      int v = cycles[i].node;
      int w = moves[v];
      howTo.push_back(MoveInfo(MoveInfo::Xchg, w, v));
    } else if (cycles[i].length == 3 && !hasMMXReg) {
      int v = cycles[i].node;
      int w = moves[v];
      howTo.push_back(MoveInfo(MoveInfo::Xchg, w, v));
      int x = moves[w];
      howTo.push_back(MoveInfo(MoveInfo::Xchg, x, w));
    } else {
      int v = cycles[i].node;
      howTo.push_back(MoveInfo(MoveInfo::Move, v, rTmp));
      int w = v;
      int x = moves[w];
      while (x != v) {
        howTo.push_back(MoveInfo(MoveInfo::Move, x, w));
        w = x;
        x = moves[w];
      }
      howTo.push_back(MoveInfo(MoveInfo::Move, rTmp, w));
    }
  }
}

const char* getContextName(Class* ctx) {
  return ctx ? ctx->name()->data() : ":anonymous:";
}

} // unnamed namespace

//////////////////////////////////////////////////////////////////////

ArgDesc::ArgDesc(SSATmp* tmp, const RegisterInfo& info, bool val)
  : m_imm(-1), m_zeroExtend(false), m_done(false) {
  if (tmp->type() == Type::None) {
    assert(val);
    m_kind = None;
    return;
  }
  if (tmp->inst()->op() == DefConst) {
    m_srcReg = InvalidReg;
    if (val) {
      m_imm = tmp->getValBits();
    } else {
      m_imm = toDataTypeForCall(tmp->type());
    }
    m_kind = Imm;
    return;
  }
  if (tmp->type().isNull()) {
    m_srcReg = InvalidReg;
    if (val) {
      m_imm = 0;
    } else {
      m_imm = toDataTypeForCall(tmp->type());
    }
    m_kind = Imm;
    return;
  }
  if (val || tmp->numNeededRegs() > 1) {
    auto reg = info.getReg(val ? 0 : 1);
    assert(reg != InvalidReg);
    m_imm = 0;

    // If val is false then we're passing tmp's type. TypeReg lets
    // CodeGenerator know that the value might require some massaging
    // to be in the right format for the call.
    m_kind = val ? Reg : TypeReg;
    // zero extend any boolean value that we pass to the helper in case
    // the helper expects it (e.g., as TypedValue)
    if (val && tmp->isA(Type::Bool)) m_zeroExtend = true;
    m_srcReg = reg;
    return;
  }
  m_srcReg = InvalidReg;
  m_imm = toDataTypeForCall(tmp->type());
  m_kind = Imm;
}

const Func* CodeGenerator::getCurFunc() const {
  always_assert(m_state.lastMarker &&
                "We shouldn't be looking for a func when we have no marker");
  return m_state.lastMarker->func;
}

/*
 * Select a scratch register to use in the given instruction, prefering the
 * lower registers which don't require a REX prefix.  The selected register
 * must not be any of the instructions inputs or outputs, and neither a register
 * that is alive across this instruction.
 */
PhysReg CodeGenerator::selectScratchReg(IRInstruction* inst) {
  static const RegSet kLowGPRegs = RegSet()
    | RegSet(reg::rax)
    | RegSet(reg::rcx)
    | RegSet(reg::rdx)
    | RegSet(reg::rsi)
    | RegSet(reg::rdi)
    ;
  RegSet liveRegs = m_state.liveRegs[inst];
  for (const auto& tmp : inst->getSrcs()) {
    liveRegs |= m_regs[tmp].getRegs();
  }
  for (const auto& tmp : inst->getDsts()) {
    liveRegs |= m_regs[tmp].getRegs();
  }
  PhysReg selectedReg;
  if ((kLowGPRegs - liveRegs).findFirst(selectedReg)) {
    return selectedReg;
  }
  return rCgGP;
}

Address CodeGenerator::cgInst(IRInstruction* inst) {
  Opcode opc = inst->op();
  auto const start = m_as.code.frontier;
  m_rScratch = selectScratchReg(inst);
  switch (opc) {
#define O(name, dsts, srcs, flags)                                \
  case name: FTRACE(7, "cg" #name "\n");                          \
             cg ## name (inst);                                   \
             return m_as.code.frontier == start ? nullptr : start;
  IR_OPCODES
#undef O

  default:
    assert(0);
    return nullptr;
  }
}

#define NOOP_OPCODE(opcode) \
  void CodeGenerator::cg##opcode(IRInstruction*) {}

#define PUNT_OPCODE(opcode) \
  void CodeGenerator::cg##opcode(IRInstruction*) { CG_PUNT(opcode); }

#define CALL_OPCODE(opcode) \
  void CodeGenerator::cg##opcode(IRInstruction* i) { cgCallNative(i); }

#define CALL_STK_OPCODE(opcode) \
  CALL_OPCODE(opcode)           \
  CALL_OPCODE(opcode ## Stk)

PUNT_OPCODE(DefCls)

NOOP_OPCODE(DefConst)
NOOP_OPCODE(DefFP)
NOOP_OPCODE(DefSP)
NOOP_OPCODE(AssertLoc)
NOOP_OPCODE(OverrideLoc)
NOOP_OPCODE(AssertStk)
NOOP_OPCODE(Nop)
NOOP_OPCODE(DefLabel)
NOOP_OPCODE(ExceptionBarrier)

CALL_OPCODE(AddElemStrKey)
CALL_OPCODE(AddElemIntKey)
CALL_OPCODE(AddNewElem)
CALL_OPCODE(ArrayAdd)
CALL_OPCODE(Box)

CALL_OPCODE(ConvBoolToArr);
CALL_OPCODE(ConvDblToArr);
CALL_OPCODE(ConvIntToArr);
CALL_OPCODE(ConvObjToArr);
CALL_OPCODE(ConvStrToArr);
CALL_OPCODE(ConvCellToArr);

CALL_OPCODE(ConvArrToBool);
CALL_OPCODE(ConvStrToBool);
CALL_OPCODE(ConvCellToBool);

CALL_OPCODE(ConvArrToDbl);
CALL_OPCODE(ConvObjToDbl);
CALL_OPCODE(ConvStrToDbl);
CALL_OPCODE(ConvCellToDbl);

CALL_OPCODE(ConvArrToInt);
CALL_OPCODE(ConvDblToInt);
CALL_OPCODE(ConvObjToInt);
CALL_OPCODE(ConvStrToInt);
CALL_OPCODE(ConvCellToInt);

CALL_OPCODE(ConvCellToObj);

CALL_OPCODE(ConvDblToStr);
CALL_OPCODE(ConvIntToStr);
CALL_OPCODE(ConvObjToStr);
CALL_OPCODE(ConvCellToStr);

CALL_OPCODE(CreateCont)
CALL_OPCODE(FillContLocals)
CALL_OPCODE(NewArray)
CALL_OPCODE(NewTuple)
CALL_OPCODE(AllocObj)
CALL_OPCODE(LdClsCtor);
CALL_OPCODE(CreateCl)
CALL_OPCODE(PrintStr)
CALL_OPCODE(PrintInt)
CALL_OPCODE(PrintBool)
CALL_OPCODE(DbgAssertPtr)
CALL_OPCODE(LdSwitchDblIndex)
CALL_OPCODE(LdSwitchStrIndex)
CALL_OPCODE(LdSwitchObjIndex)
CALL_OPCODE(VerifyParamCallable)
CALL_OPCODE(VerifyParamFail)
CALL_OPCODE(RaiseUninitLoc)
CALL_OPCODE(WarnNonObjProp)
CALL_OPCODE(ThrowNonObjProp)
CALL_OPCODE(RaiseUndefProp)
CALL_OPCODE(RaiseError)
CALL_OPCODE(RaiseWarning)
CALL_OPCODE(IncStatGrouped)
CALL_OPCODE(StaticLocInit)
CALL_OPCODE(StaticLocInitCached)
CALL_OPCODE(OpMod)
CALL_OPCODE(ArrayIdx)

// Vector instruction helpers
CALL_OPCODE(BaseG)
CALL_OPCODE(PropX)
CALL_STK_OPCODE(PropDX)
CALL_OPCODE(CGetProp)
CALL_STK_OPCODE(VGetProp)
CALL_STK_OPCODE(BindProp)
CALL_STK_OPCODE(SetProp)
CALL_OPCODE(UnsetProp)
CALL_STK_OPCODE(SetOpProp)
CALL_STK_OPCODE(IncDecProp)
CALL_OPCODE(EmptyProp)
CALL_OPCODE(IssetProp)
CALL_OPCODE(ElemX)
CALL_STK_OPCODE(ElemDX)
CALL_STK_OPCODE(ElemUX)
CALL_OPCODE(ArrayGet)
CALL_OPCODE(CGetElem)
CALL_STK_OPCODE(VGetElem)
CALL_STK_OPCODE(BindElem)
CALL_OPCODE(ArraySet)
CALL_OPCODE(ArraySetRef)
CALL_STK_OPCODE(SetElem)
CALL_STK_OPCODE(UnsetElem)
CALL_STK_OPCODE(SetOpElem)
CALL_STK_OPCODE(IncDecElem)
CALL_STK_OPCODE(SetNewElem)
CALL_STK_OPCODE(BindNewElem)
CALL_OPCODE(ArrayIsset)
CALL_OPCODE(IssetElem)
CALL_OPCODE(EmptyElem)

#undef NOOP_OPCODE
#undef PUNT_OPCODE

// Thread chain of patch locations using the 4 byte space in each jmp/jcc
void prependPatchAddr(CodegenState& state, Block* block, TCA patchAddr) {
  auto &patches = state.patches;
  ssize_t diff = patches[block] ? (patchAddr - (TCA)patches[block]) : 0;
  assert(deltaFits(diff, sz::dword));
  *(int32_t*)(patchAddr) = (int32_t)diff;
  patches[block] = patchAddr;
}

Address CodeGenerator::emitFwdJcc(Asm& a, ConditionCode cc, Block* target) {
  assert(target);
  Address start = a.code.frontier;
  a.jcc(cc, a.code.frontier);
  TCA immPtr = a.code.frontier - 4;
  prependPatchAddr(m_state, target, immPtr);
  return start;
}

Address CodeGenerator::emitFwdJmp(Asm& a, Block* target, CodegenState& state) {
  Address start = a.code.frontier;
  a.jmp(a.code.frontier);
  TCA immPtr = a.code.frontier - 4;
  prependPatchAddr(state, target, immPtr);
  return start;
}

Address CodeGenerator::emitFwdJmp(Asm& a, Block* target) {
  return emitFwdJmp(a, target, m_state);
}

Address CodeGenerator::emitFwdJcc(ConditionCode cc, Block* target) {
  return emitFwdJcc(m_as, cc, target);
}

Address CodeGenerator::emitFwdJmp(Block* target) {
  return emitFwdJmp(m_as, target);
}

void emitLoadImm(CodeGenerator::Asm& as, int64_t val, PhysReg dstReg) {
  as.emitImmReg(val, dstReg);
}

static void
emitMovRegReg(CodeGenerator::Asm& as, PhysReg srcReg, PhysReg dstReg) {
  assert(srcReg != InvalidReg);
  assert(dstReg != InvalidReg);

  if (srcReg == dstReg) return;

  if (srcReg.isGP()) {
    if (dstReg.isGP()) {                 // GP => GP
      as.movq(srcReg, dstReg);
    } else {                             // GP => MMX
      // This generates a movq x86 instruction, which zero extends
      // the 64-bit value in srcReg into a 128-bit XMM register
      as.mov_reg64_xmm(srcReg, dstReg);
    }
  } else {
    if (dstReg.isGP()) {                 // MMX => GP
      as.mov_xmm_reg64(srcReg, dstReg);
    } else {                             // MMX => MMX
      // This copies all 128 bits in XMM,
      // thus avoiding partial register stalls
      as.movdqa(srcReg, dstReg);
    }
  }
}

void CodeGenerator::emitLoadImm(CodeGenerator::Asm& as, int64_t val,
                                PhysReg dstReg) {
  assert(dstReg != InvalidReg);
  if (dstReg.isGP()) {
    as.emitImmReg(val, dstReg);
  } else {
    assert(dstReg.isXMM());
    if (val == 0) {
      as.pxor_xmm_xmm(dstReg, dstReg);
    } else {
      // Can't move immediate directly into XMM register, so use m_rScratch
      as.emitImmReg(val, m_rScratch);
      emitMovRegReg(as, m_rScratch, dstReg);
    }
  }
}

static void emitLea(CodeGenerator::Asm& as, MemoryRef mr, PhysReg dst) {
  if (dst == InvalidReg) return;
  if (mr.r.disp == 0) {
    emitMovRegReg(as, mr.r.base, dst);
  } else {
    as.lea(mr, dst);
  }
}

template<class Mem>
static void emitLoadReg(CodeGenerator::Asm& as, Mem mem, PhysReg reg) {
  assert(reg != InvalidReg);
  if (reg.isGP()) {
    as.loadq(mem, reg);
  } else {
    as.movsd(mem, reg);
  }
}

template<class Mem>
static void emitStoreReg(CodeGenerator::Asm& as, PhysReg reg, Mem mem) {
  assert(reg != InvalidReg);
  if (reg.isGP()) {
    as.storeq(reg, mem);
  } else {
    as.movsd(reg, mem);
  }
}

void shuffle2(CodeGenerator::Asm& a,
              PhysReg s0, PhysReg s1, PhysReg d0, PhysReg d1) {
  assert(s0 != s1);
  if (d0 == s1 && d1 != InvalidReg) {
    assert(d0 != d1);
    if (d1 == s0) {
      a.    xchgq (s1, s0);
    } else {
      a.    movq (s1, d1); // save s1 first; d1 != s0
      a.    movq (s0, d0);
    }
  } else {
    if (d0 != InvalidReg) emitMovRegReg(a, s0, d0); // d0 != s1
    if (d1 != InvalidReg) emitMovRegReg(a, s1, d1);
  }
}

static void zeroExtendBool(X64Assembler& as, const RegisterInfo& info) {
  auto reg = info.getReg();
  if (reg != InvalidReg) {
    // zero-extend the bool from a byte to a quad
    // note: movzbl actually extends the value to 64 bits.
    as.movzbl(rbyte(reg), r32(reg));
  }
}

static void zeroExtendIfBool(X64Assembler& as, const SSATmp* src,
                            const RegisterInfo& info) {
  if (src->isA(Type::Bool)) {
    zeroExtendBool(as, info);
  }
}

static int64_t convIntToDouble(int64_t i) {
  union {
    double  d;
    int64_t i;
  } u;
  u.d = double(i);
  return u.i;
}

/*
 * Returns a XMM register containing the value of SSATmp tmp,
 * which can be either a bool, an int, or a double.
 * If the value is already in a XMM register, simply returns it.
 * Otherwise, the value is moved into rCgXMM, which is returned.
 * If instructions to convert to a double at runtime are needed,
 * they're emitted in 'as'.
 */
PhysReg CodeGenerator::prepXMMReg(const SSATmp* tmp,
                                  X64Assembler& as,
                                  const RegAllocInfo& allocInfo,
                                  RegXMM rCgXMM) {
  assert(tmp->isA(Type::Bool) || tmp->isA(Type::Int) || tmp->isA(Type::Dbl));

  PhysReg reg = allocInfo[tmp].getReg();

  // Case 1: tmp is already in a XMM register
  if (reg.isXMM()) return reg;

  // Case 2: tmp is in a GP register
  if (reg != InvalidReg) {
    // Case 2.a: Dbl stored in GP reg
    if (tmp->isA(Type::Dbl)) {
      emitMovRegReg(as, reg, rCgXMM);
      return rCgXMM;
    }
    // Case 2.b: Bool or Int stored in GP reg
    assert(tmp->isA(Type::Bool) || tmp->isA(Type::Int));
    zeroExtendIfBool(as, tmp, allocInfo[tmp]);
    as.pxor_xmm_xmm(rCgXMM, rCgXMM);
    as.cvtsi2sd_reg64_xmm(reg, rCgXMM);
    return rCgXMM;
  }

  // Case 3: tmp is a constant
  assert(tmp->isConst());

  int64_t val = tmp->getValRawInt();
  if (!tmp->isA(Type::Dbl)) {
    assert(tmp->isA(Type::Bool | Type::Int));
    if (tmp->isA(Type::Bool)) val = val != 0;  // see task #2401790
    val = convIntToDouble(val);
  }
  emitLoadImm(as, val, m_rScratch);
  emitMovRegReg(as, m_rScratch, rCgXMM);
  return rCgXMM;
}

void CodeGenerator::doubleCmp(X64Assembler& a, RegXMM xmmReg0, RegXMM xmmReg1) {
  a.    ucomisd_xmm_xmm(xmmReg0, xmmReg1);
  Label notPF;
  a.    jnp8(notPF);
  // PF means the doubles were unordered. We treat this as !equal, so
  // clear ZF.
  a.    or_imm32_reg64(1, m_rScratch);
  asm_label(a, notPF);
}

static ConditionCode opToConditionCode(Opcode opc) {
  using namespace HPHP::Transl;

  switch (opc) {
  case JmpGt:                 return CC_G;
  case JmpGte:                return CC_GE;
  case JmpLt:                 return CC_L;
  case JmpLte:                return CC_LE;
  case JmpEq:                 return CC_E;
  case JmpNeq:                return CC_NE;
  case JmpSame:               return CC_E;
  case JmpNSame:              return CC_NE;
  case JmpInstanceOfBitmask:  return CC_NZ;
  case JmpNInstanceOfBitmask: return CC_Z;
  case JmpIsType:             return CC_NZ;
  case JmpIsNType:            return CC_Z;
  case JmpZero:               return CC_Z;
  case JmpNZero:              return CC_NZ;
  case ReqBindJmpGt:                 return CC_G;
  case ReqBindJmpGte:                return CC_GE;
  case ReqBindJmpLt:                 return CC_L;
  case ReqBindJmpLte:                return CC_LE;
  case ReqBindJmpEq:                 return CC_E;
  case ReqBindJmpNeq:                return CC_NE;
  case ReqBindJmpSame:               return CC_E;
  case ReqBindJmpNSame:              return CC_NE;
  case ReqBindJmpInstanceOfBitmask:  return CC_NZ;
  case ReqBindJmpNInstanceOfBitmask: return CC_Z;
  case ReqBindJmpZero:               return CC_Z;
  case ReqBindJmpNZero:              return CC_NZ;
  default:
    always_assert(0);
  }
}

void CodeGenerator::emitCompare(SSATmp* src1, SSATmp* src2) {
  auto const src1Type = src1->type();
  auto const src2Type = src2->type();

  // can't generate CMP instructions correctly for anything that isn't
  // a bool or a numeric, and we can't mix bool/numerics because
  // -1 == true in PHP, but not in HHIR binary representation
  if (!((src1Type == Type::Int && src2Type == Type::Int) ||
        ((src1Type == Type::Int || src1Type == Type::Dbl) &&
         (src2Type == Type::Int || src2Type == Type::Dbl)) ||
        (src1Type == Type::Bool && src2Type == Type::Bool) ||
        (src1Type == Type::Cls && src2Type == Type::Cls))) {
    CG_PUNT(emitCompare);
  }
  if (src1Type == Type::Dbl || src2Type == Type::Dbl) {
    PhysReg srcReg1 = prepXMMReg(src1, m_as, m_regs, rCgXMM0);
    PhysReg srcReg2 = prepXMMReg(src2, m_as, m_regs, rCgXMM1);
    assert(srcReg1 != rCgXMM1 && srcReg2 != rCgXMM0);
    doubleCmp(m_as, srcReg1, srcReg2);
  } else {
    auto srcReg1 = m_regs[src1].getReg();
    auto srcReg2 = m_regs[src2].getReg();

    // Note: when both src1 and src2 are constants, we should transform the
    // branch into an unconditional jump earlier in the IR.
    if (src1->isConst()) {
      // TODO: use compare with immediate or make sure simplifier
      // canonicalizes this so that constant is src2
      srcReg1 = m_rScratch;
      m_as.      mov_imm64_reg(src1->getValRawInt(), srcReg1);
    }
    if (src2->isConst()) {
      if (src1Type.subtypeOf(Type::Bool)) {
        m_as.    cmpb (src2->getValRawInt(), Reg8(int(srcReg1)));
      } else {
        m_as.    cmp_imm64_reg64(src2->getValRawInt(), srcReg1);
      }
    } else {
      // Note the reverse syntax in the assembler.
      // This cmp will compute srcReg1 - srcReg2
      if (src1Type.subtypeOf(Type::Bool)) {
        m_as.    cmpb (Reg8(int(srcReg2)), Reg8(int(srcReg1)));
      } else {
        m_as.    cmp_reg64_reg64(srcReg2, srcReg1);
      }
    }
  }
}

void CodeGenerator::emitReqBindJcc(ConditionCode cc,
                                   const ReqBindJccData* extra) {
  auto& a = m_as;
  assert(&m_as != &m_astubs &&
         "ReqBindJcc only makes sense outside of astubs");

  prepareForTestAndSmash(a, 0, kAlignJccAndJmp);
  auto const patchAddr = a.code.frontier;

  auto const jccStub = m_astubs.code.frontier;
  {
    auto& a = m_astubs;
    // TODO(#2404398): move the setcc into the generic stub code so we
    // don't need SRFlags::Persistent.
    a.    setcc  (cc, rbyte(serviceReqArgRegs[4]));
    m_tx64->emitServiceReq(
      SRFlags::Persistent,
      REQ_BIND_JMPCC_FIRST,
      4ull,
      patchAddr,
      uint64_t(extra->taken),
      uint64_t(extra->notTaken),
      uint64_t(cc)
    );
  }

  a.    jcc    (cc, jccStub);
  a.    jmp    (jccStub);
}

void CodeGenerator::cgJcc(IRInstruction* inst) {
  emitCompare(inst->getSrc(0), inst->getSrc(1));
  emitFwdJcc(opToConditionCode(inst->op()), inst->getTaken());
}

void CodeGenerator::cgReqBindJcc(IRInstruction* inst) {
  // TODO(#2404427): prepareForTestAndSmash?
  emitCompare(inst->getSrc(0), inst->getSrc(1));
  emitReqBindJcc(opToConditionCode(inst->op()),
                 inst->getExtra<ReqBindJccData>());
}

#define X(x)                                                              \
  void CodeGenerator::cgReqBind##x(IRInstruction* i) { cgReqBindJcc(i); } \
  void CodeGenerator::cg##x       (IRInstruction* i) { cgJcc(i);        }

X(JmpGt);
X(JmpGte);
X(JmpLt);
X(JmpLte);
X(JmpEq);
X(JmpNeq);
X(JmpSame);
X(JmpNSame);

#undef X


/**
 * Once the arg sources and dests are all assigned; emit moves and exchanges to
 * put all the args in desired registers. Any arguments that don't fit in
 * registers will be put on the stack. In addition to moves and exchanges,
 * shuffleArgs also handles adding lea-offsets for dest registers (dest = src +
 * lea-offset) and zero extending bools (dest = zeroExtend(src)).
 */
typedef Transl::X64Assembler Asm;
static int64_t shuffleArgs(Asm& a, ArgGroup& args) {
  // Compute the move/shuffle plan.
  int moves[kNumRegs];
  ArgDesc* argDescs[kNumRegs];
  memset(moves, -1, sizeof moves);
  memset(argDescs, 0, sizeof argDescs);
  for (size_t i = 0; i < args.numRegArgs(); ++i) {
    auto kind = args[i].getKind();
    if (!(kind == ArgDesc::Reg  ||
          kind == ArgDesc::Addr ||
          kind == ArgDesc::TypeReg)) {
      continue;
    }
    auto dstReg = args[i].getDstReg();
    auto srcReg = args[i].getSrcReg();
    if (dstReg != srcReg) {
      moves[int(dstReg)] = int(srcReg);
      argDescs[int(dstReg)] = &args[i];
    }
  }
  std::vector<MoveInfo> howTo;
  doRegMoves(moves, int(rCgGP), howTo);

  // Execute the plan
  for (size_t i = 0; i < howTo.size(); ++i) {
    if (howTo[i].m_kind == MoveInfo::Move) {
      if (howTo[i].m_reg2 == rCgGP) {
        emitMovRegReg(a, howTo[i].m_reg1, howTo[i].m_reg2);
      } else {
        ArgDesc* argDesc = argDescs[int(howTo[i].m_reg2)];
        ArgDesc::Kind kind = argDesc->getKind();
        if (kind == ArgDesc::Reg || kind == ArgDesc::TypeReg) {
          if (argDesc->isZeroExtend()) {
            assert(howTo[i].m_reg1.isGP());
            assert(howTo[i].m_reg2.isGP());
            a.    movzbl (rbyte(howTo[i].m_reg1), r32(howTo[i].m_reg2));
          } else {
            emitMovRegReg(a, howTo[i].m_reg1, howTo[i].m_reg2);
          }
        } else {
          assert(kind == ArgDesc::Addr);
          assert(howTo[i].m_reg1.isGP());
          assert(howTo[i].m_reg2.isGP());
          a.    lea    (howTo[i].m_reg1[argDesc->getImm().q()],
                        howTo[i].m_reg2);
        }
        if (kind != ArgDesc::TypeReg) {
          argDesc->markDone();
        }
      }
    } else {
      assert(howTo[i].m_reg1.isGP());
      assert(howTo[i].m_reg2.isGP());
      a.    xchgq  (howTo[i].m_reg1, howTo[i].m_reg2);
    }
  }
  // Handle const-to-register moves, type shifting,
  // load-effective address and zero extending for bools.
  // Ignore args that have been handled by the
  // move above.
  for (size_t i = 0; i < args.numRegArgs(); ++i) {
    if (!args[i].done()) {
      ArgDesc::Kind kind = args[i].getKind();
      PhysReg dst = args[i].getDstReg();
      assert(dst.isGP());
      if (kind == ArgDesc::Imm) {
        a.emitImmReg(args[i].getImm().q(), dst);
      } else if (kind == ArgDesc::TypeReg) {
        a.    shlq   (kTypeShiftBits, dst);
      } else if (kind == ArgDesc::Addr) {
        a.    addq   (args[i].getImm(), dst);
      } else if (args[i].isZeroExtend()) {
        a.    movzbl (rbyte(dst), r32(dst));
      } else if (RuntimeOption::EvalHHIRGenerateAsserts &&
                 kind == ArgDesc::None) {
        a.emitImmReg(0xbadbadbadbadbad, dst);
      }
    }
  }

  // Store any remaining arguments to the stack
  for (int i = args.numStackArgs() - 1; i >= 0; --i) {
    auto& arg = args.stk(i);
    auto srcReg = arg.getSrcReg();
    assert(arg.getDstReg() == InvalidReg);
    switch (arg.getKind()) {
      case ArgDesc::Reg:
        if (arg.isZeroExtend()) {
          a.  movzbl(rbyte(srcReg), r32(rCgGP));
          a.  push(rCgGP);
        } else {
          if (srcReg.isXMM()) {
            emitMovRegReg(a, srcReg, rCgGP);
            a.push(rCgGP);
          } else {
            a.push(srcReg);
          }
        }
        break;

      case ArgDesc::TypeReg:
        static_assert(kTypeWordOffset == 4 || kTypeWordOffset == 1,
                      "kTypeWordOffset value not supported");
        assert(srcReg.isGP());
        // x86 stacks grow down, so push higher offset items first
        if (kTypeWordOffset == 4) {
          a.  pushl(r32(srcReg));
          // 4 bytes of garbage:
          a.  pushl(eax);
        } else {
          // 4 bytes of garbage:
          a.  pushl(eax);
          // get the type in the right place in rCgGP before pushing it
          a.  movb (rbyte(srcReg), rbyte(rCgGP));
          a.  shll (CHAR_BIT, r32(rCgGP));
          a.  pushl(r32(rCgGP));
        }
        break;

      case ArgDesc::Imm:
        a.    emitImmReg(arg.getImm(), rCgGP);
        a.    push(rCgGP);
        break;

      case ArgDesc::Addr:
        not_implemented();

      case ArgDesc::None:
        a.    push(rax);
        if (RuntimeOption::EvalHHIRGenerateAsserts) {
          a.  storeq(0xbadbadbadbadbad, *rsp);
        }
        break;
    }
  }
  return args.numStackArgs() * sizeof(int64_t);
}

void CodeGenerator::cgCallNative(Asm& a, IRInstruction* inst) {
  using namespace NativeCalls;
  Opcode opc = inst->op();
  always_assert(CallMap::hasInfo(opc));

  const CallInfo& info = CallMap::getInfo(opc);
  ArgGroup argGroup(m_regs);
  for (auto const& arg : info.args) {
    SSATmp* src = inst->getSrc(arg.srcIdx);
    switch (arg.type) {
      case SSA:
        argGroup.ssa(src);
        break;
      case TV:
        argGroup.typedValue(src);
        break;
      case VecKeyS:
        argGroup.vectorKeyS(src);
        break;
      case VecKeyIS:
        argGroup.vectorKeyIS(src);
        break;
      case Immed:
        always_assert(0 && "We can't generate a native call for this");
        break;
    }
  }

  TCA addr = nullptr;
  switch (info.func.type) {
    case FPtr:
      addr = info.func.ptr;
      break;
    case FSSA:
      addr = inst->getSrc(info.func.srcIdx)->getValTCA();
      break;
  }
  cgCallHelper(a,
               addr,
               info.dest != DestType::None ? inst->getDst(0) : nullptr,
               info.sync,
               argGroup,
               info.dest);
}

void CodeGenerator::cgCallHelper(Asm& a,
                                 TCA addr,
                                 SSATmp* dst,
                                 SyncOptions sync,
                                 ArgGroup& args,
                                 DestType destType) {
  PhysReg dstReg0 = InvalidReg;
  PhysReg dstReg1 = InvalidReg;
  if (dst) {
    auto &info = m_regs[dst];
    dstReg0 = info.getReg(0);
    dstReg1 = info.getReg(1);
  }
  return cgCallHelper(a, Transl::Call(addr), dstReg0, dstReg1, sync, args,
                      destType);
}

void CodeGenerator::cgCallHelper(Asm& a,
                                 TCA addr,
                                 PhysReg dstReg,
                                 SyncOptions sync,
                                 ArgGroup& args,
                                 DestType destType) {
  cgCallHelper(a, Transl::Call(addr), dstReg, InvalidReg, sync, args, destType);
}

void CodeGenerator::cgCallHelper(Asm& a,
                                 const Transl::Call& call,
                                 PhysReg dstReg,
                                 SyncOptions sync,
                                 ArgGroup& args,
                                 DestType destType) {
  cgCallHelper(a, call, dstReg, InvalidReg, sync, args, destType);
}

void CodeGenerator::cgCallHelper(Asm& a,
                                 const Transl::Call& call,
                                 PhysReg dstReg0,
                                 PhysReg dstReg1,
                                 SyncOptions sync,
                                 ArgGroup& args,
                                 DestType destType) {
  cgCallHelper(a, call, dstReg0, dstReg1, sync, args,
               m_state.liveRegs[m_curInst], destType);
}

void CodeGenerator::cgCallHelper(Asm& a,
                                 const Transl::Call& call,
                                 PhysReg dstReg0,
                                 PhysReg dstReg1,
                                 SyncOptions sync,
                                 ArgGroup& args,
                                 RegSet toSave,
                                 DestType destType) {
  assert(m_curInst->isNative());

  // Save the caller-saved registers that are live across this
  // instruction. The number of regs to save and the number of args
  // being passed on the stack affect the parity of the PhysRegSaver,
  // so we use the generic version here.
  toSave = toSave & kCallerSaved;
  assert((toSave & RegSet().add(dstReg0).add(dstReg1)).empty());
  PhysRegSaverParity regSaver(1 + args.numStackArgs(), a, toSave);

  // Assign registers to the arguments then prepare them for the call.
  for (size_t i = 0; i < args.numRegArgs(); i++) {
    args[i].setDstReg(argNumToRegName[i]);
  }
  regSaver.bytesPushed(shuffleArgs(a, args));

  // do the call; may use a trampoline
  m_tx64->emitCall(a, call);
  if (sync != kNoSyncPoint) {
    recordSyncPoint(a, sync);
  }

  // copy the call result to the destination register(s)
  if (destType == DestType::TV) {
    // rax contains m_type and m_aux but we're expecting just the
    // type in the lower bits, so shift the type result register.
    auto rval = packed_tv ? reg::rdx : reg::rax;
    auto rtyp = packed_tv ? reg::rax : reg::rdx;
    if (kTypeShiftBits > 0) a.shrq(kTypeShiftBits, rtyp);
    shuffle2(a, rval, rtyp, dstReg0, dstReg1);
  } else if (destType == DestType::SSA) {
    // copy the single-register result to dstReg0
    assert(dstReg1 == InvalidReg);
    if (dstReg0 != InvalidReg) emitMovRegReg(a, reg::rax, dstReg0);
  } else {
    // void return type, no registers have values
    assert(dstReg0 == InvalidReg && dstReg1 == InvalidReg);
  }
}

/*
 * This doesn't really produce any code; it just keeps track of the current
 * bytecode offset.
 */
void CodeGenerator::cgMarker(IRInstruction* inst) {
  m_curBcOff = inst->getExtra<MarkerData>()->bcOff;
}

void CodeGenerator::cgMov(IRInstruction* inst) {
  assert(!m_regs[inst->getSrc(0)].hasReg(1));//TODO: t2082361: handle Gen & Cell
  SSATmp* dst   = inst->getDst();
  SSATmp* src   = inst->getSrc(0);
  auto dstReg = m_regs[dst].getReg();
  if (!m_regs[src].hasReg(0)) {
    assert(src->isConst());
    if (src->type() == Type::Bool) {
      emitLoadImm(m_as, (int64_t)src->getValBool(), dstReg);
    } else {
      emitLoadImm(m_as, src->getValRawInt(), dstReg);
    }
  } else {
    auto srcReg = m_regs[src].getReg();
    emitMovRegReg(m_as, srcReg, dstReg);
  }
}

template<class OpInstr, class Oper>
void CodeGenerator::cgUnaryIntOp(SSATmp* dst,
                                 SSATmp* src,
                                 OpInstr instr,
                                 Oper oper) {
  if (src->type() != Type::Int && src->type() != Type::Bool) {
    assert(0); CG_PUNT(UnaryIntOp);
  }
  auto dstReg = m_regs[dst].getReg();
  auto srcReg = m_regs[src].getReg();
  assert(dstReg != InvalidReg);
  auto& a = m_as;

  // Integer operations require 64-bit representations
  zeroExtendIfBool(a, src, m_regs[src]);

  if (srcReg != InvalidReg) {
    emitMovRegReg(a, srcReg, dstReg);
    (a.*instr)   (dstReg);
  } else {
    assert(src->isConst());
    emitLoadImm(a, oper(src->getValRawInt()), dstReg);
  }
}

void CodeGenerator::cgNegateWork(SSATmp* dst, SSATmp* src) {
  cgUnaryIntOp(dst, src, &Asm::neg, [](int64_t i) { return -i; });
}

inline static Reg8 convertToReg8(PhysReg reg) { return rbyte(reg); }
inline static Reg64 convertToReg64(PhysReg reg) { return reg; }

template<class Oper, class RegType>
void CodeGenerator::cgBinaryIntOp(IRInstruction* inst,
              void (Asm::*instrIR)(Immed, RegType),
              void (Asm::*instrRR)(RegType, RegType),
              void (Asm::*movInstr)(RegType, RegType),
              Oper oper,
              RegType (*convertReg)(PhysReg),
              Commutativity commuteFlag) {
  const SSATmp* dst   = inst->getDst();
  const SSATmp* src1  = inst->getSrc(0);
  const SSATmp* src2  = inst->getSrc(1);
  if (!(src1->isA(Type::Bool) || src1->isA(Type::Int)) ||
      !(src2->isA(Type::Bool) || src2->isA(Type::Int))) {
    CG_PUNT(cgBinaryIntOp);
  }

  bool const commutative = commuteFlag == Commutative;
  auto const dstReg      = m_regs[dst].getReg();
  auto const src1Reg     = m_regs[src1].getReg();
  auto const src2Reg     = m_regs[src2].getReg();
  auto& a                = m_as;

  auto const dstOpReg    = convertReg(dstReg);
  auto const src1OpReg   = convertReg(src1Reg);
  auto const src2OpReg   = convertReg(src2Reg);
  auto const rOpScratch  = convertReg(m_rScratch);

  // Two registers.
  if (src1Reg != InvalidReg && src2Reg != InvalidReg) {
    if (dstReg == src1Reg) {
      (a.*instrRR)  (src2OpReg, dstOpReg);
    } else if (dstReg == src2Reg) {
      if (commutative) {
        (a.*instrRR) (src1OpReg, dstOpReg);
      } else {
        (a.*movInstr)(src1OpReg, rOpScratch);
        (a.*instrRR) (src2OpReg, rOpScratch);
        (a.*movInstr)(rOpScratch, dstOpReg);
      }
    } else {
      emitMovRegReg(a, src1Reg, dstReg);
      (a.*instrRR) (src2OpReg, dstOpReg);
    }
    return;
  }

  // Two immediates.
  if (src1Reg == InvalidReg && src2Reg == InvalidReg) {
    assert(src1->isConst() && src2->isConst());
    int64_t value = oper(src1->getValRawInt(), src2->getValRawInt());
    emitLoadImm(a, value, dstReg);
    return;
  }

  // One register, and one immediate.
  if (commutative) {
    auto immedSrc = (src2Reg == InvalidReg ? src2 : src1);
    auto immed = immedSrc->getValRawInt();
    auto srcReg = m_regs[(src2Reg == InvalidReg ? src1 : src2)].getReg();
    if (srcReg == dstReg) {
      (a.*instrIR) (immed, dstOpReg);
    } else {
      emitLoadImm(a, immed, dstReg);
      (a.*instrRR) (convertReg(srcReg), dstOpReg);
    }
    return;
  }

  // NonCommutative:
  if (src1Reg == InvalidReg) {
    if (dstReg == src2Reg) {
      emitLoadImm(a, src1->getValRawInt(), m_rScratch);
      (a.*instrRR) (src2OpReg, rOpScratch);
      (a.*movInstr)(rOpScratch, dstOpReg);
    } else {
      emitLoadImm(a, src1->getValRawInt(), dstReg);
      (a.*instrRR) (src2OpReg, dstOpReg);
    }
    return;
  }

  assert(src2Reg == InvalidReg);
  emitMovRegReg(a, src1Reg, dstReg);
  (a.*instrIR) (src2->getValRawInt(), dstOpReg);
}

template<class Oper, class RegType>
void CodeGenerator::cgBinaryOp(IRInstruction* inst,
                 void (Asm::*instrIR)(Immed, RegType),
                 void (Asm::*instrRR)(RegType, RegType),
                 void (Asm::*movInstr)(RegType, RegType),
                 void (Asm::*fpInstr)(RegXMM, RegXMM),
                 Oper oper,
                 RegType (*convertReg)(PhysReg),
                 Commutativity commuteFlag) {
  const SSATmp* dst   = inst->getDst();
  const SSATmp* src1  = inst->getSrc(0);
  const SSATmp* src2  = inst->getSrc(1);
  if (!(src1->isA(Type::Bool) || src1->isA(Type::Int) || src1->isA(Type::Dbl))
      ||
      !(src2->isA(Type::Bool) || src2->isA(Type::Int) || src2->isA(Type::Dbl)) )
  {
    CG_PUNT(cgBinaryOp);
  }
  if (src1->isA(Type::Dbl) || src2->isA(Type::Dbl)) {
    PhysReg dstReg  = m_regs[dst].getReg();
    PhysReg resReg  = dstReg.isXMM() && dstReg != m_regs[src2].getReg() ?
                      dstReg : PhysReg(rCgXMM0);
    assert(resReg.isXMM());

    PhysReg srcReg1 = prepXMMReg(src1, m_as, m_regs, resReg);
    PhysReg srcReg2 = prepXMMReg(src2, m_as, m_regs, rCgXMM1);
    assert(srcReg1 != rCgXMM1 && srcReg2 != rCgXMM0);

    emitMovRegReg(m_as, srcReg1, resReg);

    (m_as.*fpInstr)(srcReg2, resReg);

    emitMovRegReg(m_as, resReg, dstReg);
    return;
  }
  cgBinaryIntOp(inst, instrIR, instrRR, movInstr,
                oper, convertReg, commuteFlag);
}

bool CodeGenerator::emitIncDecHelper(SSATmp* dst, SSATmp* src1, SSATmp* src2,
                                     void(Asm::*emitFunc)(Reg64)) {
  if (m_regs[src1].getReg() != InvalidReg &&
      m_regs[dst].getReg() != InvalidReg &&
      src1->isA(Type::Int) &&
      // src2 == 1:
      src2->isConst() && src2->isA(Type::Int) && src2->getValInt() == 1) {
    emitMovRegReg(m_as, m_regs[src1].getReg(), m_regs[dst].getReg());
    (m_as.*emitFunc)(m_regs[dst].getReg());
    return true;
  }
  return false;
}

/*
 * If src2 is 1, this generates dst = src1 + 1 using the "inc" x86 instruction.
 * The return value is whether or not the instruction could be generated.
 */
bool CodeGenerator::emitInc(SSATmp* dst, SSATmp* src1, SSATmp* src2) {
  return emitIncDecHelper(dst, src1, src2, &Asm::incq);
}

/*
 * If src2 is 1, this generates dst = src1 - 1 using the "dec" x86 instruction.
 * The return value is whether or not the instruction could be generated.
 */
bool CodeGenerator::emitDec(SSATmp* dst, SSATmp* src1, SSATmp* src2) {
  return emitIncDecHelper(dst, src1, src2, &Asm::decq);
}

void CodeGenerator::cgOpAdd(IRInstruction* inst) {
  SSATmp* dst  = inst->getDst();
  SSATmp* src1 = inst->getSrc(0);
  SSATmp* src2 = inst->getSrc(1);

  // Special cases: x = y + 1
  if (emitInc(dst, src1, src2) || emitInc(dst, src2, src1)) return;

  cgBinaryOp(inst,
             &Asm::addq,
             &Asm::addq,
             &Asm::movq,
             &Asm::addsd_xmm_xmm,
             std::plus<int64_t>(),
             &convertToReg64,
             Commutative);
}

void CodeGenerator::cgOpSub(IRInstruction* inst) {
  SSATmp* dst   = inst->getDst();
  SSATmp* src1  = inst->getSrc(0);
  SSATmp* src2  = inst->getSrc(1);

  if (emitDec(dst, src1, src2)) return;

  if (src1->isConst() && src1->isA(Type::Int) && src1->getValInt() == 0 &&
      src2->isA(Type::Int)) {
    cgNegateWork(dst, src2);
    return;
  }

  cgBinaryOp(inst,
             &Asm::subq,
             &Asm::subq,
             &Asm::movq,
             &Asm::subsd_xmm_xmm,
             std::minus<int64_t>(),
             &convertToReg64,
             NonCommutative);
}

void CodeGenerator::cgOpDiv(IRInstruction* inst) {
  not_implemented();
}

void CodeGenerator::cgOpBitAnd(IRInstruction* inst) {
  cgBinaryIntOp(inst,
                &Asm::andq,
                &Asm::andq,
                &Asm::movq,
                [] (int64_t a, int64_t b) { return a & b; },
                &convertToReg64,
                Commutative);
}

void CodeGenerator::cgOpBitOr(IRInstruction* inst) {
  cgBinaryIntOp(inst,
                &Asm::orq,
                &Asm::orq,
                &Asm::movq,
                [] (int64_t a, int64_t b) { return a | b; },
                &convertToReg64,
                Commutative);
}

void CodeGenerator::cgOpBitXor(IRInstruction* inst) {
  cgBinaryIntOp(inst,
                &Asm::xorq,
                &Asm::xorq,
                &Asm::movq,
                [] (int64_t a, int64_t b) { return a ^ b; },
                &convertToReg64,
                Commutative);
}

void CodeGenerator::cgOpBitNot(IRInstruction* inst) {
  cgUnaryIntOp(inst->getDst(),
               inst->getSrc(0),
               &Asm::not,
               [](int64_t i) { return ~i; });
}

void CodeGenerator::cgOpLogicXor(IRInstruction* inst) {
  cgBinaryIntOp(inst,
                &Asm::xorb,
                &Asm::xorb,
                &Asm::movb,
                [] (bool a, bool b) { return a ^ b; },
                &convertToReg8,
                Commutative);
}

void CodeGenerator::cgOpMul(IRInstruction* inst) {
  cgBinaryOp(inst,
             &Asm::imul,
             &Asm::imul,
             &Asm::movq,
             &Asm::mulsd_xmm_xmm,
             std::multiplies<int64_t>(),
             &convertToReg64,
             Commutative);
}

void CodeGenerator::cgOpNot(IRInstruction* inst) {
  auto const src = inst->getSrc(0);
  auto const dstReg = m_regs[inst->getDst()].getReg();
  auto& a = m_as;

  if (src->isConst()) {
    a.    movb   (!src->getValBool(), rbyte(dstReg));
  } else {
    if (dstReg != m_regs[src].getReg()) {
      a.  movb   (rbyte(m_regs[src].getReg()), rbyte(dstReg));
    }
    a.    xorb   (1, rbyte(dstReg));
  }
}

///////////////////////////////////////////////////////////////////////////////
// Comparison Operators
///////////////////////////////////////////////////////////////////////////////

#define DISPATCHER(name)                                                \
  HOT_FUNC_VM int64_t ccmp_ ## name (StringData* a1, StringData* a2)      \
  { return name(a1, a2); }                                              \
  HOT_FUNC_VM int64_t ccmp_ ## name (StringData* a1, int64_t a2)            \
  { return name(a1, a2); }                                              \
  HOT_FUNC_VM int64_t ccmp_ ## name (StringData* a1, ObjectData* a2)      \
  { return name(a1, Object(a2)); }                                      \
  HOT_FUNC_VM int64_t ccmp_ ## name (ObjectData* a1, ObjectData* a2)      \
  { return name(Object(a1), Object(a2)); }                              \
  HOT_FUNC_VM int64_t ccmp_ ## name (ObjectData* a1, int64_t a2)            \
  { return name(Object(a1), a2); }                                      \
  HOT_FUNC_VM int64_t ccmp_ ## name (ArrayData* a1, ArrayData* a2)        \
  { return name(Array(a1), Array(a2)); }

DISPATCHER(same)
DISPATCHER(equal)
DISPATCHER(more)
DISPATCHER(less)

#undef DISPATCHER

template <typename A, typename B>
inline int64_t ccmp_nsame(A a, B b) { return !ccmp_same(a, b); }

template <typename A, typename B>
inline int64_t ccmp_nequal(A a, B b) { return !ccmp_equal(a, b); }

template <typename A, typename B>
inline int64_t ccmp_lte(A a, B b) { return !ccmp_more(a, b); }

template <typename A, typename B>
inline int64_t ccmp_gte(A a, B b) { return !ccmp_less(a, b); }

#define CG_OP_CMP(inst, setter, name)                                         \
  cgOpCmpHelper(inst, &Asm:: setter, ccmp_ ## name, ccmp_ ## name,            \
                ccmp_ ## name, ccmp_ ## name, ccmp_ ## name, ccmp_ ## name)

// SRON - string, resource, object, or number
static bool typeIsSRON(Type t) {
  return t.isString()
      || t == Type::Obj // encompases object and resource
      || t == Type::Int
      || t == Type::Dbl
      ;
}

void CodeGenerator::cgOpCmpHelper(
          IRInstruction* inst,
          void (Asm::*setter)(Reg8),
          int64_t (*str_cmp_str)(StringData*, StringData*),
          int64_t (*str_cmp_int)(StringData*, int64_t),
          int64_t (*str_cmp_obj)(StringData*, ObjectData*),
          int64_t (*obj_cmp_obj)(ObjectData*, ObjectData*),
          int64_t (*obj_cmp_int)(ObjectData*, int64_t),
          int64_t (*arr_cmp_arr)( ArrayData*, ArrayData*)
        ) {
  SSATmp* dst   = inst->getDst();
  SSATmp* src1  = inst->getSrc(0);
  SSATmp* src2  = inst->getSrc(1);

  Type type1 = src1->type();
  Type type2 = src2->type();

  auto src1Reg = m_regs[src1].getReg();
  auto src2Reg = m_regs[src2].getReg();
  auto dstReg  = m_regs[dst].getReg();

  auto setFromFlags = [&] {
    (m_as.*setter)(rbyte(dstReg));
  };
  // It is possible that some pass has been done after simplification; if such
  // a pass invalidates our invariants, then just punt.

  // simplifyCmp has done const-const optimization
  //
  // If the types are the same and there is only one constant,
  // simplifyCmp has moved it to the right.
  if (src1->isConst()) {
    CG_PUNT(cgOpCmpHelper_const);
  }

  /////////////////////////////////////////////////////////////////////////////
  // case 1: null/string cmp string
  // simplifyCmp has converted the null to ""
  if (type1.isString() && type2.isString()) {
    ArgGroup args(m_regs);
    args.ssa(src1).ssa(src2);
    cgCallHelper(m_as, (TCA)str_cmp_str,  dst, kSyncPoint, args);
  }

  /////////////////////////////////////////////////////////////////////////////
  // case 2: bool/null cmp anything
  // simplifyCmp has converted all args to bool
  else if (type1 == Type::Bool && type2 == Type::Bool) {
    if (src2->isConst()) {
      m_as.    cmpb (src2->getValBool(), Reg8(int(src1Reg)));
    } else {
      m_as.    cmpb (Reg8(int(src2Reg)), Reg8(int(src1Reg)));
    }
    setFromFlags();
  }

  /////////////////////////////////////////////////////////////////////////////
  // case 3, 4, and 7: string/resource/object/number (sron) cmp sron
  // These cases must be amalgamated because Type::Obj can refer to an object
  //  or to a resource.
  // strings are canonicalized to the left, ints to the right
  else if (typeIsSRON(type1) && typeIsSRON(type2)) {
    // the common case: int cmp int
    if (type1 == Type::Int && type2 == Type::Int) {
      if (src2->isConst()) {
        m_as.cmp_imm64_reg64(src2->getValInt(), src1Reg);
      } else {
        m_as.cmp_reg64_reg64(src2Reg, src1Reg);
      }
      setFromFlags();
    }

    else if (type1 == Type::Dbl || type2 == Type::Dbl) {
      if ((type1 == Type::Dbl || type1 == Type::Int) &&
          (type2 == Type::Dbl || type2 == Type::Int)) {
        PhysReg srcReg1 = prepXMMReg(src1, m_as, m_regs, rCgXMM0);
        PhysReg srcReg2 = prepXMMReg(src2, m_as, m_regs, rCgXMM1);
        assert(srcReg1 != rCgXMM1 && srcReg2 != rCgXMM0);
        doubleCmp(m_as, srcReg1, srcReg2);
        setFromFlags();
      } else {
        CG_PUNT(cgOpCmpHelper_Dbl);
      }
    }

    else if (type1.isString()) {
      // string cmp string is dealt with in case 1
      // string cmp double is punted above

      if (type2 == Type::Int) {
        ArgGroup args(m_regs);
        args.ssa(src1).ssa(src2);
        cgCallHelper(m_as, (TCA)str_cmp_int,  dst, kSyncPoint, args);
      } else if (type2 == Type::Obj) {
        ArgGroup args(m_regs);
        args.ssa(src1).ssa(src2);
        cgCallHelper(m_as, (TCA)str_cmp_obj,  dst, kSyncPoint, args);
      } else {
        CG_PUNT(cgOpCmpHelper_sx);
      }
    }

    else if (type1 == Type::Obj) {
      // string cmp object/resource is dealt with above
      // object cmp double is punted above

      if (type2 == Type::Obj) {
        ArgGroup args(m_regs);
        args.ssa(src1).ssa(src2);
        cgCallHelper(m_as, (TCA)obj_cmp_obj,  dst, kSyncPoint, args);
      } else if (type2 == Type::Int) {
        ArgGroup args(m_regs);
        args.ssa(src1).ssa(src2);
        cgCallHelper(m_as, (TCA)obj_cmp_int,  dst, kSyncPoint, args);
      } else {
        CG_PUNT(cgOpCmpHelper_ox);
      }
    }

   else NOT_REACHED();
  }

  /////////////////////////////////////////////////////////////////////////////
  // case 5: array cmp array
  else if (type1.isArray() && type2.isArray()) {
    ArgGroup args(m_regs);
    args.ssa(src1).ssa(src2);
    cgCallHelper(m_as, (TCA)arr_cmp_arr,  dst, kSyncPoint, args);
  }

  /////////////////////////////////////////////////////////////////////////////
  // case 6: array cmp anything
  // simplifyCmp has already dealt with this case.

  /////////////////////////////////////////////////////////////////////////////
  else {
    // We have a type which is not a common type. It might be a cell or a box.
    CG_PUNT(cgOpCmpHelper_unimplemented);
  }
}

void CodeGenerator::cgOpEq(IRInstruction* inst) {
  CG_OP_CMP(inst, sete, equal);
}

void CodeGenerator::cgOpNeq(IRInstruction* inst) {
  CG_OP_CMP(inst, setne, nequal);
}

void CodeGenerator::cgOpSame(IRInstruction* inst) {
  CG_OP_CMP(inst, sete, same);
}

void CodeGenerator::cgOpNSame(IRInstruction* inst) {
  CG_OP_CMP(inst, setne, nsame);
}

void CodeGenerator::cgOpLt(IRInstruction* inst) {
  CG_OP_CMP(inst, setl, less);
}

void CodeGenerator::cgOpGt(IRInstruction* inst) {
  CG_OP_CMP(inst, setg, more);
}

void CodeGenerator::cgOpLte(IRInstruction* inst) {
  CG_OP_CMP(inst, setle, lte);
}

void CodeGenerator::cgOpGte(IRInstruction* inst) {
  CG_OP_CMP(inst, setge, gte);
}

///////////////////////////////////////////////////////////////////////////////
// Type check operators
///////////////////////////////////////////////////////////////////////////////

template<class OpndType>
ConditionCode CodeGenerator::emitTypeTest(Type type, OpndType src,
                                          bool negate) {
  assert(!type.subtypeOf(Type::Cls));
  ConditionCode cc;
  if (type.isString()) {
    emitTestTVType(m_as, KindOfStringBit, src);
    cc = CC_NZ;
  } else if (type.equals(Type::UncountedInit)) {
    emitTestTVType(m_as, KindOfUncountedInitBit, src);
    cc = CC_NZ;
  } else if (type.equals(Type::Uncounted)) {
    emitCmpTVType(m_as, KindOfRefCountThreshold, src);
    cc = CC_LE;
  } else if (type.equals(Type::Cell)) {
    emitCmpTVType(m_as, KindOfRef, src);
    cc = CC_L;
  } else if (type.equals(Type::Gen)) {
    return CC_None; // nothing to check
  } else {
    DataType dataType = type.toDataType();
    assert(dataType == KindOfRef ||
           (dataType >= KindOfUninit && dataType <= KindOfObject));
    emitCmpTVType(m_as, dataType, src);
    cc = CC_E;
  }
  return negate ? ccNegate(cc) : cc;
}

ConditionCode CodeGenerator::emitIsTypeTest(IRInstruction* inst, bool negate) {
  auto const src = inst->getSrc(0);

  if (inst->getTypeParam().equals(Type::Obj)) {
    auto const srcReg = m_regs[src].getReg();
    if (src->isA(Type::PtrToGen)) {
      emitTestTVType(m_as, KindOfObject, srcReg[TVOFF(m_type)]);
      TCA toPatch = m_as.code.frontier;
      m_as.   jne8(toPatch);  // 1

      // Get the ObjectData*
      emitDeref(m_as, srcReg, m_rScratch);
      m_as.   cmpq(SystemLib::s_resourceClass,
                   m_rScratch[ObjectData::getVMClassOffset()]);
      // 1:
      m_as.patchJcc8(toPatch, m_as.code.frontier);
    } else {
      // Cases where src isn't an Obj should have been simplified away
      if (!src->isA(Type::Obj)) {
        CG_PUNT(IsType-KnownWrongType);
      }
      m_as.   cmpq(SystemLib::s_resourceClass,
                   srcReg[ObjectData::getVMClassOffset()]);
    }
    // At this point, the flags say "equal" if is_object is false.
    return negate ? CC_E : CC_NE;
  }

  if (src->isA(Type::PtrToGen)) {
    PhysReg base = m_regs[src].getReg();
    return emitTypeTest(inst->getTypeParam(), base[TVOFF(m_type)], negate);
  }
  assert(src->isA(Type::Gen));
  assert(!src->isConst());

  PhysReg srcReg = m_regs[src].getReg(1); // type register
  if (srcReg == InvalidReg) {
    CG_PUNT(IsType-KnownType);
  }
  return emitTypeTest(inst->getTypeParam(), srcReg, negate);
}

template<class MemLoc>
void CodeGenerator::emitTypeCheck(Type type, MemLoc mem, Block* taken) {
  auto const negate = true;
  auto const cc = emitTypeTest(type, mem, negate);
  if (cc == CC_None) return;
  emitFwdJcc(cc, taken);
}

template<class MemLoc>
void CodeGenerator::emitTypeGuard(Type type, MemLoc mem) {
  auto const negate = true;
  auto const cc = emitTypeTest(type, mem, negate);
  if (cc == CC_None) return;

  auto const destSK = SrcKey(getCurFunc(), m_curTrace->getBcOff());
  auto const destSR = m_tx64->getSrcRec(destSK);
  m_tx64->emitFallbackCondJmp(m_as, *destSR, cc);
}

void CodeGenerator::emitSetCc(IRInstruction* inst, ConditionCode cc) {
  if (cc == CC_None) return;
  m_as.setcc(cc, rbyte(m_regs[inst->getDst()].getReg()));
}

void CodeGenerator::cgIsTypeMemCommon(IRInstruction* inst, bool negate) {
  emitSetCc(inst, emitIsTypeTest(inst, negate));
}

void CodeGenerator::cgIsTypeCommon(IRInstruction* inst, bool negate) {
  emitSetCc(inst, emitIsTypeTest(inst, negate));
}

void CodeGenerator::cgJmpIsTypeCommon(IRInstruction* inst, bool negate) {
  emitFwdJcc(emitIsTypeTest(inst, negate), inst->getTaken());
}

void CodeGenerator::cgIsType(IRInstruction* inst) {
  cgIsTypeCommon(inst, false);
}

void CodeGenerator::cgIsNType(IRInstruction* inst) {
  cgIsTypeCommon(inst, true);
}

// TODO(#2404341): remove JmpIs{N,}Type

void CodeGenerator::cgJmpIsType(IRInstruction* inst) {
  cgJmpIsTypeCommon(inst, false);
}

void CodeGenerator::cgJmpIsNType(IRInstruction* inst) {
  cgJmpIsTypeCommon(inst, true);
}

void CodeGenerator::cgIsTypeMem(IRInstruction* inst) {
  cgIsTypeMemCommon(inst, false);
}

void CodeGenerator::cgIsNTypeMem(IRInstruction* inst) {
  cgIsTypeMemCommon(inst, true);
}

///////////////////////////////////////////////////////////////////////////////

HOT_FUNC_VM static bool instanceOfHelper(const Class* objClass,
                                         const Class* testClass) {
  return testClass && objClass->classof(testClass);
}

void CodeGenerator::cgInstanceOf(IRInstruction* inst) {
  cgCallHelper(m_as,
               TCA(instanceOfHelper),
               inst->getDst(),
               kNoSyncPoint,
               ArgGroup(m_regs)
                 .ssa(inst->getSrc(0))
                 .ssa(inst->getSrc(1)));
}

/*
 * Check instanceof using instance bitmasks.
 *
 * Note it's not necessary to check whether the test class is defined:
 * if it doesn't exist than the candidate can't be an instance of it
 * and will fail this check.
 */
void CodeGenerator::emitInstanceBitmaskCheck(IRInstruction* inst) {
  auto const rObjClass     = m_regs[inst->getSrc(0)].getReg(0);
  auto const testClassName = inst->getSrc(1)->getValStr();
  auto& a = m_as;

  int offset;
  uint8_t mask;
  if (!Class::getInstanceBitMask(testClassName, offset, mask)) {
    always_assert(!"cgInstanceOfBitmask had no bitmask");
  }
  a.    testb  (int8_t(mask), rObjClass[offset]);
}

void CodeGenerator::cgInstanceOfBitmask(IRInstruction* inst) {
  auto& a = m_as;
  emitInstanceBitmaskCheck(inst);
  a.    setnz  (rbyte(m_regs[inst->getDst()].getReg()));
}

void CodeGenerator::cgNInstanceOfBitmask(IRInstruction* inst) {
  auto& a = m_as;
  emitInstanceBitmaskCheck(inst);
  a.    setz   (rbyte(m_regs[inst->getDst()].getReg()));
}

void CodeGenerator::cgJmpInstanceOfBitmask(IRInstruction* inst) {
  emitInstanceBitmaskCheck(inst);
  emitFwdJcc(CC_NZ, inst->getTaken());
}

void CodeGenerator::cgJmpNInstanceOfBitmask(IRInstruction* inst) {
  emitInstanceBitmaskCheck(inst);
  emitFwdJcc(CC_Z, inst->getTaken());
}

void CodeGenerator::cgReqBindJmpInstanceOfBitmask(IRInstruction* inst) {
  emitInstanceBitmaskCheck(inst);
  emitReqBindJcc(opToConditionCode(inst->op()),
                 inst->getExtra<ReqBindJccData>());
}

void CodeGenerator::cgReqBindJmpNInstanceOfBitmask(IRInstruction* inst) {
  emitInstanceBitmaskCheck(inst);
  emitReqBindJcc(opToConditionCode(inst->op()),
                 inst->getExtra<ReqBindJccData>());
}

/*
 * Check instanceof using the superclass vector on the end of the
 * Class entry.
 */
void CodeGenerator::cgExtendsClass(IRInstruction* inst) {
  auto const rObjClass     = m_regs[inst->getSrc(0)].getReg();
  auto const testClass     = inst->getSrc(1)->getValClass();
  auto rTestClass          = m_regs[inst->getSrc(1)].getReg();
  auto const rdst          = rbyte(m_regs[inst->getDst()].getReg());
  auto& a = m_as;

  Label out;
  Label notExact;
  Label falseLabel;

  if (rTestClass == InvalidReg) { // TODO(#2031606)
    rTestClass = m_rScratch; // careful below about asm-x64 smashing this
    emitLoadImm(a, (int64_t)testClass, rTestClass);
  }

  // Test if it is the exact same class.  TODO(#2044801): we should be
  // doing this control flow at the IR level.
  if (!(testClass->attrs() & AttrAbstract)) {
    a.    cmpq   (rTestClass, rObjClass);
    a.    jne8   (notExact);
    a.    movb   (1, rdst);
    a.    jmp8   (out);
  }

  auto const vecOffset = Class::classVecOff() +
    sizeof(Class*) * (testClass->classVecLen() - 1);

  // Check the length of the class vectors---if the candidate's is at
  // least as long as the potential base (testClass) it might be a
  // subclass.
asm_label(a, notExact);
  a.    cmpl   (testClass->classVecLen(),
                rObjClass[Class::classVecLenOff()]);
  a.    jb8    (falseLabel);

  // If it's a subclass, rTestClass must be at the appropriate index.
  a.    cmpq   (rTestClass, rObjClass[vecOffset]);
  a.    sete   (rdst);
  a.    jmp8   (out);

asm_label(a, falseLabel);
  a.    xorl   (r32(rdst), r32(rdst));

asm_label(a, out);
}

void CodeGenerator::cgConvDblToBool(IRInstruction* inst) {
  SSATmp* dst = inst->getDst();
  auto dstReg = m_regs[dst].getReg();
  assert(dstReg != InvalidReg);
  SSATmp* src = inst->getSrc(0);
  auto srcReg = m_regs[src].getReg();
  if (srcReg == InvalidReg) {
    assert(src->isConst());
    double constVal = src->getValDbl();
    if (constVal == 0.0) {
      m_as.xor_reg64_reg64(dstReg, dstReg);
    } else {
      m_as.mov_imm64_reg(1, dstReg);
    }
  } else {
    emitMovRegReg(m_as, srcReg, dstReg);
    m_as.shlq(1, dstReg); // 0.0 stays zero and -0.0 is now 0.0
    m_as.setne(rbyte(dstReg)); // lower byte becomes 1 if dstReg != 0
    m_as.movzbl(rbyte(dstReg), r32(dstReg));
  }
}

void CodeGenerator::cgConvIntToBool(IRInstruction* inst) {
  SSATmp* dst = inst->getDst();
  auto dstReg = m_regs[dst].getReg();
  assert(dstReg != InvalidReg);
  SSATmp* src = inst->getSrc(0);
  auto srcReg = m_regs[src].getReg();

  if (srcReg == InvalidReg) {
    assert(src->isConst());
    int64_t constVal = src->getValInt();
    if (constVal == 0) {
      m_as.xor_reg64_reg64(dstReg, dstReg);
    } else {
      m_as.mov_imm64_reg(1, dstReg);
    }
  } else {
    m_as.test_reg64_reg64(srcReg, srcReg);
    m_as.setne(rbyte(dstReg));
    m_as.movzbl(rbyte(dstReg), r32(dstReg));
  }
}

void CodeGenerator::emitConvBoolOrIntToDbl(IRInstruction* inst) {
  SSATmp* src = inst->getSrc(0);
  SSATmp* dst = inst->getDst();
  PhysReg dstReg = m_regs[dst].getReg();
  assert(src->isA(Type::Bool) || src->isA(Type::Int));
  assert(dstReg != InvalidReg);
  if (src->isConst()) {
    int64_t constVal = src->getValRawInt();
    if (src->isA(Type::Bool)) constVal = constVal != 0; // see task #2401790
    constVal = convIntToDouble(constVal);
    emitLoadImm(m_as, constVal, dstReg);
  } else {
    // cvtsi2sd doesn't modify the high bits of its target, which can
    // cause false dependencies to prevent register renaming from kicking
    // in. Break the dependency chain by zeroing out the XMM reg.
    PhysReg srcReg = m_regs[src].getReg();
    PhysReg xmmReg = dstReg.isXMM() ? dstReg : PhysReg(rCgXMM0);
    m_as.pxor_xmm_xmm(xmmReg, xmmReg);
    m_as.cvtsi2sd_reg64_xmm(srcReg, xmmReg);
    zeroExtendIfBool(m_as, src, m_regs[src]);
    emitMovRegReg(m_as, xmmReg, dstReg);
  }
}

void CodeGenerator::cgConvBoolToDbl(IRInstruction* inst) {
  emitConvBoolOrIntToDbl(inst);
}

void CodeGenerator::cgConvIntToDbl(IRInstruction* inst) {
  emitConvBoolOrIntToDbl(inst);
}

void CodeGenerator::cgConvBoolToInt(IRInstruction* inst) {
  SSATmp* dst = inst->getDst();
  auto dstReg = m_regs[dst].getReg();
  assert(dstReg != InvalidReg);
  SSATmp* src = inst->getSrc(0);
  auto srcReg = m_regs[src].getReg();
  assert(src->isConst() == (srcReg == InvalidReg));
  if (srcReg == InvalidReg) {
    int64_t constVal = src->getValRawInt();
    if (constVal == 0) {
      m_as.xor_reg64_reg64(dstReg, dstReg);
    } else {
      m_as.mov_imm64_reg(1, dstReg);
    }
  } else {
    m_as.movzbl(rbyte(srcReg), r32(dstReg));
  }
}

void CodeGenerator::cgConvBoolToStr(IRInstruction* inst) {
  SSATmp* dst = inst->getDst();
  auto dstReg = m_regs[dst].getReg();
  assert(dstReg != InvalidReg);
  SSATmp* src = inst->getSrc(0);
  auto srcReg = m_regs[src].getReg();
  assert(src->isConst() == (srcReg == InvalidReg));
  if (srcReg == InvalidReg) {
    auto constVal = src->getValBool();
    if (!constVal) {
      m_as.mov_imm64_reg((uint64_t)StringData::GetStaticString(""), dstReg);
    } else {
      m_as.mov_imm64_reg((uint64_t)StringData::GetStaticString("1"), dstReg);
    }
  } else {
    m_as.testb(Reg8(int(srcReg)), Reg8(int(srcReg)));
    m_as.mov_imm64_reg((uint64_t)StringData::GetStaticString(""), dstReg);
    m_as.mov_imm64_reg((uint64_t)StringData::GetStaticString("1"), m_rScratch);
    m_as.cmov_reg64_reg64(CC_NZ, m_rScratch, dstReg);
  }
}

void CodeGenerator::cgUnboxPtr(IRInstruction* inst) {
  SSATmp* dst   = inst->getDst();
  SSATmp* src   = inst->getSrc(0);

  auto srcReg = m_regs[src].getReg();
  auto dstReg = m_regs[dst].getReg();

  assert(srcReg != InvalidReg);
  assert(dstReg != InvalidReg);

  emitMovRegReg(m_as, srcReg, dstReg);
  emitDerefIfVariant(m_as, PhysReg(dstReg));
}

void CodeGenerator::cgUnbox(IRInstruction* inst) {
  SSATmp* dst     = inst->getDst();
  SSATmp* src     = inst->getSrc(0);
  auto dstValReg  = m_regs[dst].getReg(0);
  auto dstTypeReg = m_regs[dst].getReg(1);
  auto srcValReg  = m_regs[src].getReg(0);
  auto srcTypeReg = m_regs[src].getReg(1);

  assert(dstValReg != dstTypeReg);
  assert(src->type().equals(Type::Gen));
  assert(dst->type().notBoxed());

  emitCmpTVType(m_as, HPHP::KindOfRef, srcTypeReg);
  ifThenElse(CC_E, [&] {
    // srcTypeReg == KindOfRef; srcValReg is RefData*
    const size_t ref_tv_off = RefData::tvOffset();
    if (dstValReg != srcValReg) {
      emitLoadReg(m_as, srcValReg[ref_tv_off + TVOFF(m_data)], dstValReg);
      emitLoadTVType(m_as, srcValReg[ref_tv_off + TVOFF(m_type)],
                     r32(dstTypeReg));
    } else {
      emitLoadTVType(m_as, srcValReg[ref_tv_off + TVOFF(m_type)],
                     r32(dstTypeReg));
      m_as.loadq(srcValReg[ref_tv_off + TVOFF(m_data)], dstValReg);
    }
  }, [&] {
    // srcTypeReg != KindOfRef; copy src -> dst
    shuffle2(m_as, srcValReg, srcTypeReg, dstValReg, dstTypeReg);
  });
}

void CodeGenerator::cgLdFuncCachedCommon(IRInstruction* inst) {
  SSATmp* dst        = inst->getDst();
  SSATmp* methodName = inst->getSrc(0);

  const StringData* name = methodName->getValStr();
  CacheHandle ch = TargetCache::allocFixedFunction(name);
  size_t funcCacheOff = ch + offsetof(FixedFuncCache, m_func);

  auto dstReg = m_regs[dst].getReg();
  if (dstReg == InvalidReg) {
    // happens if LdFixedFunc and FCall not in same trace
    m_as.   cmpq(0, rVmTl[funcCacheOff]);
  } else {
    m_as.   loadq (rVmTl[funcCacheOff], dstReg);
    m_as.   testq (dstReg, dstReg);
  }
}

void CodeGenerator::cgLdFuncCached(IRInstruction* inst) {
  cgLdFuncCachedCommon(inst);
  // jz off to the helper call in astubs
  unlikelyIfBlock(CC_Z, [&] (Asm& a) {
    // this helper tries the autoload map, and fatals on failure
    cgCallNative(a, inst);
  });
}

void CodeGenerator::cgLdFuncCachedSafe(IRInstruction* inst) {
  cgLdFuncCachedCommon(inst);
  if (Block* taken = inst->getTaken()) {
    emitFwdJcc(m_as, CC_Z, taken);
  }
}

void CodeGenerator::cgLdFunc(IRInstruction* inst) {
  SSATmp*        dst = inst->getDst();
  SSATmp* methodName = inst->getSrc(0);

  TargetCache::CacheHandle ch = TargetCache::FuncCache::alloc();
  // raises an error if function not found
  cgCallHelper(m_as, (TCA)FuncCache::lookup, m_regs[dst].getReg(), kSyncPoint,
               ArgGroup(m_regs).imm(ch).ssa(methodName));
}

static void emitLdObjClass(CodeGenerator::Asm& a,
                           PhysReg objReg,
                           PhysReg dstReg) {
  a.loadq  (objReg[ObjectData::getVMClassOffset()], dstReg);
}

void CodeGenerator::cgLdObjClass(IRInstruction* inst) {
  auto dstReg = m_regs[inst->getDst()].getReg();
  auto objReg = m_regs[inst->getSrc(0)].getReg();

  emitLdObjClass(m_as, objReg, dstReg);
}

void CodeGenerator::cgLdObjMethod(IRInstruction *inst) {
  auto cls       = inst->getSrc(0);
  auto clsReg    = m_regs[cls].getReg();
  auto name      = inst->getSrc(1);
  auto actRec    = inst->getSrc(2);
  auto actRecReg = m_regs[actRec].getReg();
  CacheHandle handle = Transl::TargetCache::MethodCache::alloc();

  // lookup in the targetcache
  assert(MethodCache::kNumLines == 1);
  if (debug) {
    MethodCache::Pair p;
    static_assert(sizeof(p.m_value) == 8,
                  "MethodCache::Pair::m_value assumed to be 8 bytes");
    static_assert(sizeof(p.m_key) == 8,
                  "MethodCache::Pair::m_key assumed to be 8 bytes");
  }

  // preload handle->m_value
  m_as.loadq(rVmTl[handle + offsetof(MethodCache::Pair, m_value)], m_rScratch);
  m_as.cmpq (rVmTl[handle + offsetof(MethodCache::Pair, m_key)], clsReg);
  ifThenElse(CC_E, // if handle->key == cls
             [&] { // then actReg->m_func = handle->value
               m_as.storeq(m_rScratch, actRecReg[AROFF(m_func)]);
             },
             [&] { // else call slow path helper
               cgCallHelper(m_as, (TCA)methodCacheSlowPath, InvalidReg,
                            kSyncPoint,
                            ArgGroup(m_regs).addr(rVmTl, handle)
                                            .ssa(actRec)
                                            .ssa(name)
                                            .ssa(cls));
             });
}

void CodeGenerator::cgStRetVal(IRInstruction* inst) {
  auto  const rFp = m_regs[inst->getSrc(0)].getReg();
  auto* const val = inst->getSrc(1);
  cgStore(rFp, AROFF(m_r), val);
}

void CodeGenerator::cgRetAdjustStack(IRInstruction* inst) {
  auto const rFp   = m_regs[inst->getSrc(0)].getReg();
  auto const dstSp = m_regs[inst->getDst()].getReg();
  auto& a = m_as;
  a.    lea   (rFp[AROFF(m_r)], dstSp);
}

void CodeGenerator::cgLdRetAddr(IRInstruction* inst) {
  auto fpReg = m_regs[inst->getSrc(0)].getReg(0);
  assert(fpReg != InvalidReg);
  m_as.push(fpReg[AROFF(m_savedRip)]);
}

void checkFrame(ActRec* fp, Cell* sp, bool checkLocals) {
  const Func* func = fp->m_func;
  if (fp->hasVarEnv()) {
    assert(fp->getVarEnv()->getCfp() == fp);
  }
  // TODO: validate this pointer from actrec
  int numLocals = func->numLocals();
  assert(sp <= (Cell*)fp - func->numSlotsInFrame()
         || func->isGenerator());
  if (checkLocals) {
    int numParams = func->numParams();
    for (int i=0; i < numLocals; i++) {
      if (i >= numParams && func->isGenerator() && i < func->numNamedLocals()) {
        continue;
      }
      assert(checkTv(frame_local(fp, i)));
    }
  }
  // We unfortunately can't do the same kind of check for the stack
  // because it may contain ActRecs.
#if 0
  for (Cell* c=sp; c < firstSp; c++) {
    TypedValue* tv = (TypedValue*)c;
    assert(tvIsPlausible(tv));
    DataType t = tv->m_type;
    if (IS_REFCOUNTED_TYPE(t)) {
      assert(tv->m_data.pstr->getCount() > 0);
    }
  }
#endif
}

void traceRet(ActRec* fp, Cell* sp, void* rip) {
  if (rip == TranslatorX64::Get()->getCallToExit()) {
    return;
  }
  checkFrame(fp, sp, /*checkLocals*/ false);
  assert(sp <= (Cell*)fp || fp->m_func->isGenerator());
  // check return value if stack not empty
  if (sp < (Cell*)fp) assertTv(sp);
}

void CodeGenerator::emitTraceRet(CodeGenerator::Asm& a) {
  // call to a trace function
  a.    movq  (rVmFp, rdi);
  a.    movq  (rVmSp, rsi);
  a.    loadq (*rsp, rdx); // return ip from native stack
  // do the call; may use a trampoline
  m_tx64->emitCall(a, TCA(traceRet));
}

void CodeGenerator::cgRetCtrl(IRInstruction* inst) {
  SSATmp* sp = inst->getSrc(0);
  SSATmp* fp = inst->getSrc(1);

  // Make sure rVmFp and rVmSp are set appropriately
  emitMovRegReg(m_as, m_regs[sp].getReg(), rVmSp);
  emitMovRegReg(m_as, m_regs[fp].getReg(), rVmFp);

  // Return control to caller
  if (RuntimeOption::EvalHHIRGenerateAsserts) {
    emitTraceRet(m_as);
  }
  m_as.ret();
  if (RuntimeOption::EvalHHIRGenerateAsserts) {
    m_as.ud2();
  }
}

void CodeGenerator::emitReqBindAddr(const Func* func,
                                    TCA& dest,
                                    Offset offset) {
  dest = m_tx64->emitServiceReq(SRFlags::None,
                                REQ_BIND_ADDR,
                                2ull,
                                &dest,
                                offset);
}

void CodeGenerator::cgJmpSwitchDest(IRInstruction* inst) {
  JmpSwitchData* data = inst->getExtra<JmpSwitchDest>();
  SSATmp* index       = inst->getSrc(0);
  auto indexReg       = m_regs[index].getReg();

  if (!index->isConst()) {
    if (data->bounded) {
      if (data->base) {
        m_as.  subq(data->base, indexReg);
      }
      m_as.    cmpq(data->cases - 2, indexReg);
      prepareForSmash(m_as, kJmpccLen);
      TCA def = m_tx64->emitServiceReq(REQ_BIND_JMPCC_SECOND, 3,
                                   m_as.code.frontier, data->defaultOff, CC_AE);
      m_as.    jae(def);
    }

    TCA* table = m_tx64->m_globalData.alloc<TCA>(sizeof(TCA), data->cases);
    TCA afterLea = m_as.code.frontier + kLeaRipLen;
    ptrdiff_t diff = (TCA)table - afterLea;
    assert(deltaFits(diff, sz::dword));
    m_as.   lea(rip[diff], m_rScratch);
    assert(m_as.code.frontier == afterLea);
    m_as.   jmp(m_rScratch[indexReg*8]);

    for (int i = 0; i < data->cases; i++) {
      emitReqBindAddr(data->func, table[i], data->targets[i]);
    }
  } else {
    int64_t indexVal = index->getValInt();

    if (data->bounded) {
      indexVal -= data->base;
      if (indexVal >= data->cases - 2 || indexVal < 0) {
        m_tx64->emitBindJmp(m_as, SrcKey(data->func, data->defaultOff));
        return;
      }
    }
    m_tx64->emitBindJmp(m_as, SrcKey(data->func, data->targets[indexVal]));
  }
}

typedef FixedStringMap<TCA,true> SSwitchMap;

static TCA sswitchHelperFast(const StringData* val,
                             const SSwitchMap* table,
                             TCA* def) {
  TCA* dest = table->find(val);
  return dest ? *dest : *def;
}

void CodeGenerator::cgLdSSwitchDestFast(IRInstruction* inst) {
  auto data = inst->getExtra<LdSSwitchDestFast>();

  auto table = m_tx64->m_globalData.alloc<SSwitchMap>(64);
  table->init(data->numCases);
  for (int64_t i = 0; i < data->numCases; ++i) {
    table->add(data->cases[i].str, nullptr);
    TCA* addr = table->find(data->cases[i].str);
    emitReqBindAddr(data->func, *addr, data->cases[i].dest);
  }
  TCA* def = m_tx64->m_globalData.alloc<TCA>(sizeof(TCA), 1);
  emitReqBindAddr(data->func, *def, data->defaultOff);

  cgCallHelper(m_as,
               TCA(sswitchHelperFast),
               inst->getDst(),
               kNoSyncPoint,
               ArgGroup(m_regs)
                 .ssa(inst->getSrc(0))
                 .immPtr(table)
                 .immPtr(def));
}

static TCA sswitchHelperSlow(TypedValue typedVal,
                             const StringData** strs,
                             int numStrs,
                             TCA* jmptab) {
  TypedValue* cell = tvToCell(&typedVal);
  for (int i = 0; i < numStrs; ++i) {
    if (tvAsCVarRef(cell).equal(strs[i])) return jmptab[i];
  }
  return jmptab[numStrs]; // default case
}

void CodeGenerator::cgLdSSwitchDestSlow(IRInstruction* inst) {
  auto data = inst->getExtra<LdSSwitchDestSlow>();

  auto strtab = m_tx64->m_globalData.alloc<const StringData*>(
    sizeof(const StringData*), data->numCases);
  auto jmptab = m_tx64->m_globalData.alloc<TCA>(sizeof(TCA),
    data->numCases + 1);
  for (int i = 0; i < data->numCases; ++i) {
    strtab[i] = data->cases[i].str;
    emitReqBindAddr(data->func, jmptab[i], data->cases[i].dest);
  }
  emitReqBindAddr(data->func, jmptab[data->numCases], data->defaultOff);

  cgCallHelper(m_as,
               TCA(sswitchHelperSlow),
               inst->getDst(),
               kSyncPoint,
               ArgGroup(m_regs)
                 .typedValue(inst->getSrc(0))
                 .immPtr(strtab)
                 .imm(data->numCases)
                 .immPtr(jmptab));
}

/*
 * It'd be nice not to have the cgMov here (and just copy propagate
 * the source or something), but for now we're keeping it allocated to
 * rVmFp so inlined calls to C++ helpers that use the rbp chain to
 * find the caller's ActRec will work correctly.
 *
 * This instruction primarily exists to assist in optimizing away
 * unused activation records, so it's usually not going to happen
 * anyway.
 */
void CodeGenerator::cgDefInlineFP(IRInstruction* inst) {
  auto const fp       = m_regs[inst->getSrc(0)].getReg();
  auto const fakeRet  = m_tx64->getRetFromInlinedFrame();
  auto const retBCOff = inst->getExtra<DefInlineFP>()->retBCOff;

  m_as.    storeq (fakeRet, fp[AROFF(m_savedRip)]);
  m_as.    storel (retBCOff, fp[AROFF(m_soff)]);

  cgMov(inst);
}

void CodeGenerator::cgInlineReturn(IRInstruction* inst) {
  auto fpReg = m_regs[inst->getSrc(0)].getReg();
  assert(fpReg == rVmFp);
  m_as.    loadq  (fpReg[AROFF(m_savedRbp)], rVmFp);
}

void CodeGenerator::cgReDefSP(IRInstruction* inst) {
  // TODO(#2288359): this instruction won't be necessary (for
  // non-generator frames) when we don't track rVmSp independently
  // from rVmFp.  In generator frames we'll have to track offsets from
  // a DefGeneratorSP or something similar.
  auto fp  = m_regs[inst->getSrc(0)].getReg();
  auto dst = m_regs[inst->getDst()].getReg();
  auto off = -inst->getExtra<ReDefSP>()->offset * sizeof(Cell);
  emitLea(m_as, fp[off], dst);
}

void CodeGenerator::cgStashGeneratorSP(IRInstruction* inst) {
  cgMov(inst);
}

void CodeGenerator::cgReDefGeneratorSP(IRInstruction* inst) {
  cgMov(inst);
}

void CodeGenerator::cgFreeActRec(IRInstruction* inst) {
  m_as.loadq(m_regs[inst->getSrc(0)].getReg()[AROFF(m_savedRbp)],
             m_regs[inst->getDst()].getReg());
}

void CodeGenerator::cgSpill(IRInstruction* inst) {
  SSATmp* dst   = inst->getDst();
  SSATmp* src   = inst->getSrc(0);

  assert(dst->numNeededRegs() == src->numNeededRegs());
  for (int locIndex = 0; locIndex < m_regs[src].numAllocatedRegs();
       ++locIndex) {
    // We do not need to mask booleans, since the IR will reload the spill
    auto srcReg = m_regs[src].getReg(locIndex);
    auto sinfo = m_regs[dst].getSpillInfo(locIndex);
    if (m_regs[src].isFullXMM()) {
      m_as.movdqa(srcReg, reg::rsp[sinfo.offset()]);
    } else {
      int offset = sinfo.offset();
      if (locIndex == 0 || packed_tv) {
        emitStoreReg(m_as, srcReg, reg::rsp[offset]);
      } else {
        // Note that type field is shifted in memory
        assert(srcReg.isGP());
        offset += TVOFF(m_type) - (TVOFF(m_data) + sizeof(Value));
        emitStoreTVType(m_as, srcReg, reg::rsp[offset]);
      }
    }
  }
}

void CodeGenerator::cgReload(IRInstruction* inst) {
  SSATmp* dst   = inst->getDst();
  SSATmp* src   = inst->getSrc(0);

  assert(dst->numNeededRegs() == src->numNeededRegs());
  for (int locIndex = 0; locIndex < m_regs[dst].numAllocatedRegs();
       ++locIndex) {
    auto dstReg = m_regs[dst].getReg(locIndex);
    auto sinfo = m_regs[src].getSpillInfo(locIndex);
    if (m_regs[dst].isFullXMM()) {
      assert(dstReg.isXMM());
      m_as.movdqa(reg::rsp[sinfo.offset()], dstReg);
    } else {
      int offset = sinfo.offset();
      if (locIndex == 0 || packed_tv) {
        emitLoadReg(m_as, reg::rsp[offset], dstReg);
      } else {
        // Note that type field is shifted in memory
        offset += TVOFF(m_type) - (TVOFF(m_data) + sizeof(Value));
        assert(dstReg.isGP());
        emitLoadTVType(m_as, reg::rsp[offset], dstReg);
      }
    }
  }
}

void CodeGenerator::cgStPropWork(IRInstruction* inst, bool genTypeStore) {
  SSATmp* obj   = inst->getSrc(0);
  SSATmp* prop  = inst->getSrc(1);
  SSATmp* src   = inst->getSrc(2);
  cgStore(m_regs[obj].getReg(), prop->getValInt(), src, genTypeStore);
}
void CodeGenerator::cgStProp(IRInstruction* inst) {
  cgStPropWork(inst, true);
}
void CodeGenerator::cgStPropNT(IRInstruction* inst) {
  cgStPropWork(inst, false);
}

void CodeGenerator::cgStMemWork(IRInstruction* inst, bool genStoreType) {
  SSATmp* addr = inst->getSrc(0);
  SSATmp* offset  = inst->getSrc(1);
  SSATmp* src  = inst->getSrc(2);
  cgStore(m_regs[addr].getReg(), offset->getValInt(), src, genStoreType);
}
void CodeGenerator::cgStMem(IRInstruction* inst) {
  cgStMemWork(inst, true);
}
void CodeGenerator::cgStMemNT(IRInstruction* inst) {
  cgStMemWork(inst, false);
}

void CodeGenerator::cgStRefWork(IRInstruction* inst, bool genStoreType) {
  auto destReg = m_regs[inst->getDst()].getReg();
  auto addrReg = m_regs[inst->getSrc(0)].getReg();
  SSATmp* src  = inst->getSrc(1);
  cgStore(addrReg, RefData::tvOffset(), src, genStoreType);
  if (destReg != InvalidReg)  emitMovRegReg(m_as, addrReg, destReg);
}

void CodeGenerator::cgStRef(IRInstruction* inst) {
  cgStRefWork(inst, true);
}
void CodeGenerator::cgStRefNT(IRInstruction* inst) {
  cgStRefWork(inst, false);
}

static int64_t getLocalOffset(int64_t index) {
  return -cellsToBytes(index + 1);
}

static int64_t getLocalOffset(SSATmp* index) {
  return getLocalOffset(index->getValInt());
}

int CodeGenerator::getIterOffset(SSATmp* tmp) {
  const Func* func = getCurFunc();
  int64_t index = tmp->getValInt();
  return -cellsToBytes(((index + 1) * kNumIterCells + func->numLocals()));
}

void CodeGenerator::cgStLoc(IRInstruction* inst) {
  cgStore(m_regs[inst->getSrc(0)].getReg(),
          getLocalOffset(inst->getExtra<StLoc>()->locId),
          inst->getSrc(1),
          true /* store type */);
}

void CodeGenerator::cgStLocNT(IRInstruction* inst) {
  cgStore(m_regs[inst->getSrc(0)].getReg(),
          getLocalOffset(inst->getExtra<StLocNT>()->locId),
          inst->getSrc(1),
          false /* store type */);
}

void CodeGenerator::cgSyncABIRegs(IRInstruction* inst) {
  emitMovRegReg(m_as, m_regs[inst->getSrc(0)].getReg(), rVmFp);
  emitMovRegReg(m_as, m_regs[inst->getSrc(1)].getReg(), rVmSp);
}

void CodeGenerator::cgReqBindJmp(IRInstruction* inst) {
  m_tx64->emitBindJmp(
    m_as,
    SrcKey(getCurFunc(), inst->getExtra<ReqBindJmp>()->offset)
  );
}

static void emitExitNoIRStats(Asm& a,
                              TranslatorX64* tx64,
                              const Func* func,
                              SrcKey dest) {
  if (RuntimeOption::EnableInstructionCounts ||
      HPHP::Trace::moduleEnabled(HPHP::Trace::stats, 3)) {
    Stats::emitInc(a,
                   Stats::opcodeToIRPreStatCounter(
                     Op(*func->unit()->at(dest.m_offset))),
                   -1,
                   Transl::CC_None,
                   true);
  }

  if (HPHP::Trace::moduleEnabled(HPHP::Trace::punt, 1)) {
    auto const op = Op(*func->unit()->at(dest.m_offset));
    auto const name = folly::format(
      "exitSlow-{}",
      opcodeToName(op)
    ).str();
    tx64->emitRecordPunt(a, name);
  }
}

void CodeGenerator::cgReqBindJmpNoIR(IRInstruction* inst) {
  auto const dest = SrcKey(getCurFunc(),
                           inst->getExtra<ReqBindJmpNoIR>()->offset);
  emitExitNoIRStats(m_as, m_tx64, getCurFunc(), dest);
  m_tx64->emitBindJmp(m_as, dest, REQ_BIND_JMP_NO_IR);
}

void CodeGenerator::cgReqRetranslateNoIR(IRInstruction* inst) {
  auto const dest = SrcKey(getCurFunc(),
                           inst->getExtra<ReqRetranslateNoIR>()->offset);
  emitExitNoIRStats(m_as, m_tx64, getCurFunc(), dest);
  m_tx64->emitReqRetransNoIR(m_as, dest);
}

void CodeGenerator::cgReqRetranslate(IRInstruction* inst) {
  auto const destSK = SrcKey(getCurFunc(), m_curTrace->getBcOff());
  auto const destSR = m_tx64->getSrcRec(destSK);
  m_tx64->emitFallbackUncondJmp(m_as, *destSR);
}

static void emitAssertFlagsNonNegative(CodeGenerator::Asm& as) {
  ifThen(as, CC_NGE, [&] { as.ud2(); });
}

static void emitAssertRefCount(CodeGenerator::Asm& as, PhysReg base) {
  as.cmpl(HPHP::RefCountStaticValue, base[FAST_REFCOUNT_OFFSET]);
  ifThen(as, CC_NBE, [&] { as.ud2(); });
}

static void emitIncRef(CodeGenerator::Asm& as, PhysReg base) {
  if (RuntimeOption::EvalHHIRGenerateAsserts) {
    emitAssertRefCount(as, base);
  }
  // emit incref
  as.incl(base[FAST_REFCOUNT_OFFSET]);
  if (RuntimeOption::EvalHHIRGenerateAsserts) {
    // Assert that the ref count is greater than zero
    emitAssertFlagsNonNegative(as);
  }
}

void CodeGenerator::cgIncRefWork(Type type, SSATmp* src) {
  assert(type.maybeCounted());
  auto increfMaybeStatic = [&] {
    auto base = m_regs[src].getReg(0);
    if (!type.needsStaticBitCheck()) {
      emitIncRef(m_as, base);
    } else {
      m_as.cmpl(RefCountStaticValue, base[FAST_REFCOUNT_OFFSET]);
      ifThen(m_as, CC_NE, [&] { emitIncRef(m_as, base); });
    }
  };

  if (type.isKnownDataType()) {
    assert(IS_REFCOUNTED_TYPE(type.toDataType()));
    increfMaybeStatic();
  } else {
    m_as.cmpl(KindOfRefCountThreshold, r32(m_regs[src].getReg(1)));
    ifThen(m_as, CC_NLE, [&] { increfMaybeStatic(); });
  }
}

void CodeGenerator::cgIncRef(IRInstruction* inst) {
  SSATmp* dst    = inst->getDst();
  SSATmp* src    = inst->getSrc(0);
  Type type = src->type();

  cgIncRefWork(type, src);
  shuffle2(m_as, m_regs[src].getReg(0), m_regs[src].getReg(1),
           m_regs[dst].getReg(0), m_regs[dst].getReg(1));
}

void CodeGenerator::cgDecRefStack(IRInstruction* inst) {
  cgDecRefMem(inst->getTypeParam(),
              m_regs[inst->getSrc(0)].getReg(),
              cellsToBytes(inst->getExtra<DecRefStack>()->offset),
              nullptr);
}

void CodeGenerator::cgDecRefThis(IRInstruction* inst) {
  SSATmp* fp    = inst->getSrc(0);
  Block* exit   = inst->getTaken();
  auto fpReg = m_regs[fp].getReg();
  auto scratchReg = m_rScratch;

  // Load AR->m_this into m_rScratch
  m_as.loadq(fpReg[AROFF(m_this)], scratchReg);

  auto decrefIfAvailable = [&] {
    // Check if this is available and we're not in a static context instead
    m_as.testb(1, rbyte(scratchReg));
    ifThen(m_as, CC_Z, [&] {
      cgDecRefStaticType(
        Type::Obj,
        scratchReg,
        exit,
        true /* genZeroCheck */
      );
    });
  };

  if (getCurFunc()->isPseudoMain()) {
    // In pseudo-mains, emit check for presence of m_this
    m_as.testq(scratchReg, scratchReg);
    ifThen(m_as, CC_NZ, [&] { decrefIfAvailable(); });
  } else {
    decrefIfAvailable();
  }
}

void CodeGenerator::cgDecRefLoc(IRInstruction* inst) {
  cgDecRefMem(inst->getTypeParam(),
              m_regs[inst->getSrc(0)].getReg(),
              getLocalOffset(inst->getExtra<DecRefLoc>()->locId),
              inst->getTaken());
}

void CodeGenerator::cgGenericRetDecRefs(IRInstruction* inst) {
  auto const rFp       = m_regs[inst->getSrc(0)].getReg();
  auto const numLocals = inst->getSrc(1)->getValInt();
  auto const rDest     = m_regs[inst->getDst()].getReg();
  auto& a = m_as;

  assert(rFp == rVmFp &&
         "free locals helper assumes the frame pointer is rVmFp");
  assert(rDest == rVmSp &&
         "free locals helper adjusts rVmSp, which must be our dst reg");

  if (numLocals == 0) return;

  // The helpers called below use a special ABI, in which r13 is not saved.
  // So save r13 on the stack if it's live.
  bool saveR13 = m_state.liveRegs[inst].contains(r13);

  int stackAdjust = 8;
  if (saveR13)  {
    a.push(r13);
    stackAdjust = 16;
  }

  auto const target = numLocals > kNumFreeLocalsHelpers
    ? m_tx64->m_freeManyLocalsHelper
    : m_tx64->m_freeLocalsHelpers[numLocals - 1];

  a.subq(stackAdjust, rsp);  // For parity; callee does retq $0x8.
  a.lea(rFp[-numLocals * sizeof(TypedValue)], rDest);
  a.call(target);
  recordSyncPoint(a);

  if (saveR13) {
    a.addq(8, rsp);
    a.pop(r13);
  }
}

static void
tv_release_generic(TypedValue* tv) {
  assert(Transl::tx64->stateIsDirty());
  assert(tv->m_type >= KindOfString && tv->m_type <= KindOfRef);
  g_destructors[typeToDestrIndex(tv->m_type)](tv->m_data.pref);
}

static void
tv_release_typed(RefData* pv, DataType dt) {
  assert(Transl::tx64->stateIsDirty());
  assert(dt >= KindOfString && dt <= KindOfRef);
  g_destructors[typeToDestrIndex(dt)](pv);
}

Address CodeGenerator::getDtorGeneric() {
  return (Address)tv_release_generic;
}

Address CodeGenerator::getDtorTyped() {
  return (Address)tv_release_typed;
}

//
// This method generates code that checks the static bit and jumps if the bit
// is set.  If regIsCount is true, reg contains the _count field. Otherwise,
// it's assumed to contain m_data field.
//
// Return value: the address to be patched with the address to jump to in case
// the static bit is set. If the check is unnecessary, this method retuns NULL.
Address CodeGenerator::cgCheckStaticBit(Type type,
                                        PhysReg reg,
                                        bool regIsCount) {
  if (!type.needsStaticBitCheck()) return NULL;

  if (regIsCount) {
    // reg has the _count value
    m_as.cmp_imm32_reg32(RefCountStaticValue, reg);
  } else {
    // reg has the data pointer
    m_as.cmp_imm32_disp_reg32(RefCountStaticValue, FAST_REFCOUNT_OFFSET, reg);
  }

  Address addrToPatch = m_as.code.frontier;
  m_as.jcc8(CC_E, addrToPatch);
  return addrToPatch;
}


//
// Using the given dataReg, this method generates code that checks the static
// bit out of dataReg, and emits a DecRef if needed.
// NOTE: the flags are left with the result of the DecRef's subtraction,
//       which can then be tested immediately after this.
//
// Return value: the address to be patched if a RefCountedStaticValue check is
//               emitted; NULL otherwise.
//
Address CodeGenerator::cgCheckStaticBitAndDecRef(Type type,
                                                 PhysReg dataReg,
                                                 Block* exit) {
  assert(type.maybeCounted());

  Address patchStaticCheck = nullptr;
  const auto scratchReg = m_rScratch;

  bool canUseScratch = dataReg != scratchReg;

  // TODO: run experiments to check whether the 'if' code sequence
  // is any better than the 'else' branch below; otherwise, always
  // use the 'else' code sequence
  if (type.needsStaticBitCheck() && canUseScratch) {
    // If we need to check for static value, then load the _count into a
    // register to avoid doing two loads. The generated sequence is:
    //
    //     scratchReg = [dataReg + offset(_count)]
    //     if scratchReg == RefCountStaticValue then skip DecRef
    //     scratchReg = scratchReg - 1
    //     ( if exit != NULL, emit:
    //          jz exit
    //     )
    //     [dataReg + offset(_count)] = scratchReg

    if (RuntimeOption::EvalHHIRGenerateAsserts) {
      emitAssertRefCount(m_as, dataReg);
    }
    // Load _count in scratchReg
    m_as.loadl(dataReg[FAST_REFCOUNT_OFFSET], r32(scratchReg));

    // Check for RefCountStaticValue
    patchStaticCheck = cgCheckStaticBit(type, scratchReg,
                                        true /* reg has _count */);

    // Decrement count and store it back in memory.
    // If there's an exit, emit jump to it when _count would get down to 0
    m_as.decq(scratchReg);
    if (exit) {
      emitFwdJcc(CC_E, exit);
    }
    if (RuntimeOption::EvalHHIRGenerateAsserts) {
      // Assert that the ref count is greater than zero
      emitAssertFlagsNonNegative(m_as);
    }
    m_as.store_reg32_disp_reg64(scratchReg, FAST_REFCOUNT_OFFSET, dataReg);

  } else {
    // Can't use scratch reg, so emit code that operates directly in
    // memory. Compared to the sequence above, this will result in one
    // extra load, but it has the advantage of producing a instruction
    // sequence.
    //
    //     ( if needStaticBitCheck, emit :
    //           cmp [dataReg + offset(_count)], RefCountStaticValue
    //           je LabelAfterDecRef
    //     )
    //     ( if exit != NULL, emit:
    //           cmp [dataReg + offset(_count)], 1
    //           jz exit
    //     )
    //     sub [dataReg + offset(_count)], 1

    // If necessary, check for RefCountStaticValue
    patchStaticCheck = cgCheckStaticBit(type, dataReg,
                                        false /* passing dataReg */);

    // If there's an exit, emit jump to it if _count would get down to 0
    if (exit) {
      m_as.cmp_imm32_disp_reg32(1, FAST_REFCOUNT_OFFSET, dataReg);
      emitFwdJcc(CC_E, exit);
    }
    if (RuntimeOption::EvalHHIRGenerateAsserts) {
      emitAssertRefCount(m_as, dataReg);
    }

    // Decrement _count
    m_as.decl(dataReg[FAST_REFCOUNT_OFFSET]);

    if (RuntimeOption::EvalHHIRGenerateAsserts) {
      // Assert that the ref count is not less than zero
      emitAssertFlagsNonNegative(m_as);
    }
  }

  return patchStaticCheck;
}


//
// Returns the address to be patched with the address to jump to in case
// the type is not ref-counted.
//
Address CodeGenerator::cgCheckRefCountedType(PhysReg typeReg) {
  emitCmpTVType(m_as, KindOfRefCountThreshold, typeReg);
  Address addrToPatch = m_as.code.frontier;
  m_as.jcc8(CC_LE, addrToPatch);
  return addrToPatch;
}

Address CodeGenerator::cgCheckRefCountedType(PhysReg baseReg, int64_t offset) {
  emitCmpTVType(m_as, KindOfRefCountThreshold, baseReg[offset + TVOFF(m_type)]);
  Address addrToPatch = m_as.code.frontier;
  m_as.jcc8(CC_LE, addrToPatch);
  return addrToPatch;
}

//
// Generates dec-ref of a typed value with statically known type.
//
void CodeGenerator::cgDecRefStaticType(Type type,
                                       PhysReg dataReg,
                                       Block* exit,
                                       bool genZeroCheck) {
  assert(type != Type::Cell && type != Type::Gen);
  assert(type.isKnownDataType());

  if (type.notCounted()) return;

  // Check for RefCountStaticValue if needed, do the actual DecRef,
  // and leave flags set based on the subtract result, which is
  // tested below
  Address patchStaticCheck;
  if (genZeroCheck) {
    patchStaticCheck = cgCheckStaticBitAndDecRef(type, dataReg, exit);
  } else {
    // Set exit as NULL so that the code doesn't jump to error checking.
    patchStaticCheck = cgCheckStaticBitAndDecRef(type, dataReg, nullptr);
  }

  // If not exiting on count down to zero, emit the zero-check and
  // release call
  if (genZeroCheck && exit == nullptr) {
    // Emit jump to m_astubs (to call release) if count got down to zero
    unlikelyIfBlock(CC_Z, [&] (Asm& a) {
      // Emit the call to release in m_astubs
      cgCallHelper(a, m_tx64->getDtorCall(type.toDataType()),
                   InvalidReg, InvalidReg, kSyncPoint,
                   ArgGroup(m_regs).reg(dataReg));
    });
  }
  if (patchStaticCheck) {
    m_as.patchJcc8(patchStaticCheck, m_as.code.frontier);
  }
}

//
// Generates dec-ref of a typed value with dynamic (statically unknown) type,
// when the type is stored in typeReg.
//
void CodeGenerator::cgDecRefDynamicType(PhysReg typeReg,
                                        PhysReg dataReg,
                                        Block* exit,
                                        bool genZeroCheck) {
  // Emit check for ref-counted type
  Address patchTypeCheck = cgCheckRefCountedType(typeReg);

  // Emit check for RefCountStaticValue and the actual DecRef
  Address patchStaticCheck;
  if (genZeroCheck) {
    patchStaticCheck = cgCheckStaticBitAndDecRef(Type::Cell, dataReg, exit);
  } else {
    patchStaticCheck = cgCheckStaticBitAndDecRef(Type::Cell, dataReg, nullptr);
  }

  // If not exiting on count down to zero, emit the zero-check and release call
  if (genZeroCheck && exit == nullptr) {
    // Emit jump to m_astubs (to call release) if count got down to zero
    unlikelyIfBlock(CC_Z, [&] (Asm& a) {
      // Emit call to release in m_astubs
      cgCallHelper(a, getDtorTyped(), InvalidReg, kSyncPoint,
                   ArgGroup(m_regs).reg(dataReg).reg(typeReg));
    });
  }
  // Patch checks to jump around the DecRef
  if (patchTypeCheck)   m_as.patchJcc8(patchTypeCheck,   m_as.code.frontier);
  if (patchStaticCheck) m_as.patchJcc8(patchStaticCheck, m_as.code.frontier);
}

//
// Generates dec-ref of a typed value with dynamic (statically
// unknown) type, when all we have is the baseReg and offset of
// the typed value. This method assumes that baseReg is not the
// scratch register.
//
void CodeGenerator::cgDecRefDynamicTypeMem(PhysReg baseReg,
                                           int64_t offset,
                                           Block* exit) {
  auto scratchReg = m_rScratch;

  assert(baseReg != scratchReg);

  // Emit check for ref-counted type
  Address patchTypeCheck = cgCheckRefCountedType(baseReg, offset);
  if (exit == nullptr && RuntimeOption::EvalHHIRGenericDtorHelper) {
    {
      // This PhysRegSaverStub saves rdi redundantly if
      // !liveRegs[m_curInst].contains(rdi), but its
      // necessary to maintain stack alignment. We can do better
      // by making the helpers adjust the stack for us in the cold
      // path, which calls the destructor.
      PhysRegSaverStub regSaver(m_as, RegSet(rdi));

      /*
       * rVmSp is ok here because this is part of the special
       * ABI to m_irPopRHelper.  We're not using a secret dependency
       * on the frame or stack---we're only going to use that ABI if
       * we happen to have that register allocated for baseReg.
       */
      if (offset == 0 && baseReg == rVmSp) {
        // Decref'ing top of vm stack, very likely a popR
        m_tx64->emitCall(m_as, m_tx64->m_irPopRHelper);
      } else {
        if (baseReg == rsp) {
          // Because we just pushed %rdi, %rsp is 8 bytes below where
          // offset is expecting it to be.
          offset += sizeof(int64_t);
        }
        m_as.lea(baseReg[offset], rdi);
        m_tx64->emitCall(m_as, m_tx64->m_dtorGenericStub);
      }
      recordSyncPoint(m_as);
    }
    if (patchTypeCheck) {
      m_as.patchJcc8(patchTypeCheck, m_as.code.frontier);
    }
    return;
  }
  // Load m_data into the scratch reg
  m_as.loadq(baseReg[offset + TVOFF(m_data)], scratchReg);

  // Emit check for RefCountStaticValue and the actual DecRef
  Address patchStaticCheck = cgCheckStaticBitAndDecRef(Type::Cell, scratchReg,
                                                       exit);

  // If not exiting on count down to zero, emit the zero-check and release call
  if (exit == nullptr) {
    // Emit jump to m_astubs (to call release) if count got down to zero
    unlikelyIfBlock(CC_Z, [&] (Asm& a) {
      // Emit call to release in m_astubs
      a.lea(baseReg[offset], scratchReg);
      cgCallHelper(a, getDtorGeneric(), InvalidReg, kSyncPoint,
                   ArgGroup(m_regs).reg(scratchReg));
    });
  }

  // Patch checks to jump around the DecRef
  if (patchTypeCheck)   m_as.patchJcc8(patchTypeCheck,   m_as.code.frontier);
  if (patchStaticCheck) m_as.patchJcc8(patchStaticCheck, m_as.code.frontier);
}

//
// Generates the dec-ref of a typed value in memory address [baseReg + offset].
// This handles cases where type is either static or dynamic.
//
void CodeGenerator::cgDecRefMem(Type type,
                                PhysReg baseReg,
                                int64_t offset,
                                Block* exit) {
  auto scratchReg = m_rScratch;
  assert(baseReg != scratchReg);

  if (type.needsReg()) {
    // The type is dynamic, but we don't have two registers available
    // to load the type and the data.
    cgDecRefDynamicTypeMem(baseReg, offset, exit);
  } else if (type.maybeCounted()) {
    m_as.loadq(baseReg[offset + TVOFF(m_data)], scratchReg);
    cgDecRefStaticType(type, scratchReg, exit, true);
  }
}

void CodeGenerator::cgDecRefMem(IRInstruction* inst) {
  assert(inst->getSrc(0)->type().isPtr());
  cgDecRefMem(inst->getTypeParam(),
              m_regs[inst->getSrc(0)].getReg(),
              inst->getSrc(1)->getValInt(),
              inst->getTaken());
}

void CodeGenerator::cgDecRefWork(IRInstruction* inst, bool genZeroCheck) {
  SSATmp* src   = inst->getSrc(0);
  if (!isRefCounted(src)) return;
  Block* exit = inst->getTaken();
  Type type = src->type();
  if (type.isKnownDataType()) {
    cgDecRefStaticType(type, m_regs[src].getReg(), exit, genZeroCheck);
  } else {
    cgDecRefDynamicType(m_regs[src].getReg(1),
                        m_regs[src].getReg(0),
                        exit,
                        genZeroCheck);
  }
}

void CodeGenerator::cgDecRef(IRInstruction *inst) {
  // DecRef may bring the count to zero, and run the destructor.
  // Generate code for this.
  assert(!inst->getTaken());
  cgDecRefWork(inst, true);
}

void CodeGenerator::cgDecRefNZ(IRInstruction* inst) {
  // DecRefNZ cannot bring the count to zero.
  // Therefore, we don't generate zero-checking code.
  assert(!inst->getTaken());
  cgDecRefWork(inst, false);
}

void CodeGenerator::cgDecRefNZOrBranch(IRInstruction* inst) {
  assert(inst->getTaken());
  cgDecRefWork(inst, true);
}

void CodeGenerator::cgSpillFrame(IRInstruction* inst) {
  auto const sp        = inst->getSrc(0);
  auto const fp        = inst->getSrc(1);
  auto const func      = inst->getSrc(2);
  auto const objOrCls  = inst->getSrc(3);
  auto const magicName = inst->getExtra<SpillFrame>()->invName;
  auto const nArgs     = inst->getExtra<SpillFrame>()->numArgs;

  const int64_t spOffset = -kNumActRecCells * sizeof(Cell);

  DEBUG_ONLY bool setThis = true;

  auto spReg = m_regs[sp].getReg();
  // actRec->m_this
  if (objOrCls->isA(Type::Cls)) {
    // store class
    if (objOrCls->isConst()) {
      m_as.store_imm64_disp_reg64(uintptr_t(objOrCls->getValClass()) | 1,
                                  spOffset + int(AROFF(m_this)),
                                  spReg);
    } else {
      Reg64 clsPtrReg = m_regs[objOrCls].getReg();
      m_as.movq  (clsPtrReg, m_rScratch);
      m_as.orq   (1, m_rScratch);
      m_as.storeq(m_rScratch, spReg[spOffset + int(AROFF(m_this))]);
    }
  } else if (objOrCls->isA(Type::Obj)) {
    // store this pointer
    m_as.store_reg64_disp_reg64(m_regs[objOrCls].getReg(),
                                spOffset + int(AROFF(m_this)),
                                spReg);
  } else if (objOrCls->isA(Type::Ctx)) {
    // Stores either a this pointer or a Cctx -- statically unknown.
    Reg64 objOrClsPtrReg = m_regs[objOrCls].getReg();
    m_as.storeq(objOrClsPtrReg, spReg[spOffset + int(AROFF(m_this))]);
  } else {
    assert(objOrCls->isA(Type::InitNull));
    // no obj or class; this happens in FPushFunc
    int offset_m_this = spOffset + int(AROFF(m_this));
    // When func is either Type::FuncCls or Type::FuncCtx,
    // m_this/m_cls will be initialized below
    if (!func->isConst() && (func->isA(Type::FuncCtx))) {
      // m_this is unioned with m_cls and will be initialized below
      setThis = false;
    } else {
      m_as.store_imm64_disp_reg64(0, offset_m_this, spReg);
    }
  }
  // actRec->m_invName
  // ActRec::m_invName is encoded as a pointer with bit kInvNameBit
  // set to distinguish it from m_varEnv and m_extrArgs
  uintptr_t invName = !magicName
    ? 0
    : reinterpret_cast<uintptr_t>(magicName) | ActRec::kInvNameBit;
  m_as.store_imm64_disp_reg64(invName,
                              spOffset + int(AROFF(m_invName)),
                              spReg);
  // actRec->m_func  and possibly actRec->m_cls
  // Note m_cls is unioned with m_this and may overwrite previous value
  if (func->type().isNull()) {
    assert(func->isConst());
  } else if (func->isConst()) {
    const Func* f = func->getValFunc();
    m_as. mov_imm64_reg((uint64_t)f, m_rScratch);
    m_as.store_reg64_disp_reg64(m_rScratch,
                                spOffset + int(AROFF(m_func)),
                                spReg);
    if (func->isA(Type::FuncCtx)) {
      // Fill in m_cls if provided with both func* and class*
      CG_PUNT(cgAllocActRec);
    }
  } else {
    int offset_m_func = spOffset + int(AROFF(m_func));
    m_as.store_reg64_disp_reg64(m_regs[func].getReg(0),
                                offset_m_func,
                                spReg);
    if (func->isA(Type::FuncCtx)) {
      int offset_m_cls = spOffset + int(AROFF(m_cls));
      m_as.store_reg64_disp_reg64(m_regs[func].getReg(1),
                                  offset_m_cls,
                                  spReg);
      setThis = true; /* m_this and m_cls are in a union */
    }
  }
  assert(setThis);
  // actRec->m_savedRbp
  m_as.store_reg64_disp_reg64(m_regs[fp].getReg(),
                              spOffset + int(AROFF(m_savedRbp)),
                              spReg);

  // actRec->m_numArgsAndCtorFlag
  m_as.store_imm32_disp_reg(nArgs,
                            spOffset + int(AROFF(m_numArgsAndCtorFlag)),
                            spReg);

  emitAdjustSp(spReg,
               m_regs[inst->getDst()].getReg(),
               spOffset);
}

const Func* loadClassCtor(Class* cls) {
  const Func* f = cls->getCtor();
  if (UNLIKELY(!(f->attrs() & AttrPublic))) {
    VMRegAnchor _;
    UNUSED MethodLookup::LookupResult res =
      g_vmContext->lookupCtorMethod(f, cls, true /*raise*/);
    assert(res == MethodLookup::MethodFoundWithThis);
  }
  return f;
}

Instance* createClHelper(Class* cls, int numArgs, ActRec* ar, TypedValue* sp) {
  Instance* newObj = newInstance(cls);
  newObj->incRefCount();
  return static_cast<c_Closure*>(newObj)->init(numArgs, ar, sp);
}

void CodeGenerator::cgAllocObjFast(IRInstruction* inst) {
  const Class* cls = inst->getSrc(0)->getValClass();
  auto dstReg = m_regs[inst->getDst()].getReg();

  // First, make sure our property init vectors are all set up
  bool props = cls->pinitVec().size() > 0;
  bool sprops = cls->numStaticProperties() > 0;
  assert((props || sprops) == cls->needInitialization());
  if (cls->needInitialization()) {
    if (props) {
      cls->initPropHandle();
      m_as.testq(-1, rVmTl[cls->propHandle()]);
      unlikelyIfBlock(CC_Z, [&] (Asm& a) {
          cgCallHelper(a,
                       (TCA)getMethodPtr(&Class::initProps),
                       InvalidReg,
                       kSyncPoint,
                       ArgGroup(m_regs).imm((uint64_t)cls));
      });
    }
    if (sprops) {
      cls->initSPropHandle();
      m_as.testq(-1, rVmTl[cls->sPropHandle()]);
      unlikelyIfBlock(CC_Z, [&] (Asm& a) {
          cgCallHelper(a,
                       (TCA)getMethodPtr(&Class::initSProps),
                       InvalidReg,
                       kSyncPoint,
                       ArgGroup(m_regs).imm((uint64_t)cls));
      });
    }
  }

  // Next, allocate the object
  if (cls->instanceCtor()) {
    cgCallHelper(m_as,
                 (TCA)cls->instanceCtor(),
                 dstReg,
                 kSyncPoint,
                 ArgGroup(m_regs).imm((uint64_t)cls));
  } else {
    size_t size = Instance::sizeForNProps(cls->numDeclProperties());
    int allocator = object_alloc_size_to_index(size);
    assert(allocator != -1);
    cgCallHelper(m_as,
                 (TCA)getMethodPtr(&Instance::newInstanceRaw),
                 dstReg,
                 kSyncPoint,
                 ArgGroup(m_regs).imm((uint64_t)cls).imm(allocator));
  }

  // Set the attributes, if any
  int odAttrs = cls->getODAttrs();
  if (odAttrs) {
    // o_attribute is 16 bits but the fact that we're or-ing a mask makes it ok
    assert(!(odAttrs & 0xffff0000));
    m_as.orq(odAttrs, dstReg[ObjectData::attributeOff()]);
  }

  // Initialize the properties
  size_t nProps = cls->numDeclProperties();
  if (nProps > 0) {
    m_as.push(dstReg);
    m_as.subq(8, reg::rsp);
    if (cls->pinitVec().size() == 0) {
      // Fast case: copy from a known address in the Class
      ArgGroup args = ArgGroup(m_regs)
        .addr(dstReg, sizeof(ObjectData) + cls->builtinPropSize())
        .imm(int64_t(&cls->declPropInit()[0]))
        .imm(cellsToBytes(nProps));
      cgCallHelper(m_as,
                   (TCA)memcpy,
                   InvalidReg,
                   kNoSyncPoint,
                   args);
    } else {
      // Slower case: we have to load the src address from the targetcache
      auto rPropData = m_rScratch;
      // Load the Class's propInitVec from the targetcache
      m_as.loadq(rVmTl[cls->propHandle()], rPropData);
      // propData holds the PropInitVec. We want &(*propData)[0]
      m_as.loadq(rPropData[Class::PropInitVec::dataOff()], rPropData);
      if (!cls->hasDeepInitProps()) {
        ArgGroup args = ArgGroup(m_regs)
          .addr(dstReg, sizeof(ObjectData) + cls->builtinPropSize())
          .reg(rPropData)
          .imm(cellsToBytes(nProps));
        cgCallHelper(m_as,
                     (TCA)memcpy,
                     InvalidReg,
                     kNoSyncPoint,
                     args);
      } else {
        ArgGroup args = ArgGroup(m_regs)
          .addr(dstReg, sizeof(ObjectData) + cls->builtinPropSize())
          .reg(rPropData)
          .imm(nProps);
        cgCallHelper(m_as,
                     (TCA)deepInitHelper,
                     InvalidReg,
                     kNoSyncPoint,
                     args);
      }
    }
    m_as.addq(8, reg::rsp);
    m_as.pop(dstReg);
  }
  if (cls->callsCustomInstanceInit()) {
    // callCustomInstanceInit returns the instance in rax
    cgCallHelper(m_as,
                 (TCA)getMethodPtr(&Instance::callCustomInstanceInit),
                 dstReg,
                 kSyncPoint,
                 ArgGroup(m_regs).reg(dstReg));
  }
}

void CodeGenerator::cgInlineCreateCont(IRInstruction* inst) {
  auto const& data = *inst->getExtra<InlineCreateCont>();
  auto const helper = data.origFunc->isMethod()
    ? &VMExecutionContext::createContinuationHelper<true>
    : &VMExecutionContext::createContinuationHelper<false>;

  cgCallHelper(
    m_as,
    reinterpret_cast<TCA>(helper),
    inst->getDst(),
    kSyncPoint,
    ArgGroup(m_regs)
      .immPtr(data.origFunc)
      .immPtr(data.genFunc)
      .ssa(inst->getSrc(0))
      .immPtr(nullptr) // getArgs array
      // Deliberately ignoring frameStaticClass parameter, because
      // it's unused if we have a $this pointer, and we don't inline
      // functions with a null $this.
  );
  if (data.origFunc->isMethod()) {
    // We can't support a null $this.
    assert(inst->getSrc(0)->isA(Type::Obj));
  }
}

void CodeGenerator::cgCallArray(IRInstruction* inst) {
  Offset pc             = inst->getExtra<CallArray>()->pc;
  Offset after          = inst->getExtra<CallArray>()->after;

  ArgGroup args(m_regs);
  args.imm(pc).imm(after);

  // fCallArrayHelper makes the actual call by smashing its return address.
  cgCallHelper(m_as, (TCA)TranslatorX64::fCallArrayHelper,
               nullptr, kSyncPoint, args);
}

void CodeGenerator::cgCall(IRInstruction* inst) {
  SSATmp* actRec         = inst->getSrc(0);
  SSATmp* returnBcOffset = inst->getSrc(1);
  SSATmp* func           = inst->getSrc(2);
  SrcRange args          = inst->getSrcs().subpiece(3);
  int32_t numArgs        = args.size();

  auto spReg = m_regs[actRec].getReg();
  // put all outgoing arguments onto the VM stack
  int64_t adjustment = (-(int64_t)numArgs) * sizeof(Cell);
  for (int32_t i = 0; i < numArgs; i++) {
    // Type::None here means that the simplifier proved that the value
    // matches the value already in memory, thus the store is redundant.
    if (args[i]->type() != Type::None) {
      cgStore(spReg, -(i + 1) * sizeof(Cell), args[i]);
    }
  }
  // store the return bytecode offset into the outgoing actrec
  uint64_t returnBc = returnBcOffset->getValInt();
  m_as.store_imm32_disp_reg(returnBc, AROFF(m_soff), spReg);
  if (adjustment != 0) {
    m_as.add_imm32_reg64(adjustment, spReg);
  }

  assert(m_state.lastMarker);
  SrcKey srcKey = SrcKey(m_state.lastMarker->func, m_state.lastMarker->bcOff);
  bool isImmutable = (func->isConst() && !func->type().isNull());
  const Func* funcd = isImmutable ? func->getValFunc() : nullptr;
  assert(&m_as == &m_tx64->getAsm());
  int32_t adjust = m_tx64->emitBindCall(srcKey, funcd, numArgs);
  if (adjust) {
    m_as.addq (adjust, rVmSp);
  }
}

void CodeGenerator::cgCastStk(IRInstruction *inst) {
  Type type       = inst->getTypeParam();
  SSATmp* sp      = inst->getSrc(0);
  uint32_t offset = inst->getExtra<CastStk>()->offset;
  PhysReg spReg   = m_regs[sp].getReg();

  ArgGroup args(m_regs);
  args.addr(spReg, cellsToBytes(offset));

  TCA tvCastHelper;
  if (type.subtypeOf(Type::Bool)) {
    tvCastHelper = (TCA)tvCastToBooleanInPlace;
  } else if (type.subtypeOf(Type::Int)) {
    // if casting to integer, pass 10 as the base for the conversion
    args.imm(10);
    tvCastHelper = (TCA)tvCastToInt64InPlace;
  } else if (type.subtypeOf(Type::Dbl)) {
    tvCastHelper = (TCA)tvCastToDoubleInPlace;
  } else if (type.subtypeOf(Type::Arr)) {
    tvCastHelper = (TCA)tvCastToArrayInPlace;
  } else if (type.subtypeOf(Type::Str)) {
    tvCastHelper = (TCA)tvCastToStringInPlace;
  } else if (type.subtypeOf(Type::Obj)) {
    tvCastHelper = (TCA)tvCastToObjectInPlace;
  } else {
    not_reached();
  }
  cgCallHelper(m_as, tvCastHelper, nullptr,
               kSyncPoint, args, DestType::None);
}

void CodeGenerator::cgCallBuiltin(IRInstruction* inst) {
  SSATmp* f             = inst->getSrc(0);
  auto args             = inst->getSrcs().subpiece(1);
  int32_t numArgs         = args.size();
  SSATmp* dst           = inst->getDst();
  auto dstReg           = m_regs[dst].getReg(0);
  auto dstType          = m_regs[dst].getReg(1);
  Type returnType       = inst->getTypeParam();

  const Func* func = f->getValFunc();
  DataType funcReturnType = func->returnType();
  int returnOffset = HHIR_MISOFF(tvBuiltinReturn);

  if (TranslatorX64::eagerRecord(func)) {
    const uchar* pc = curUnit()->entry() + m_state.lastMarker->bcOff;
    // we have spilled all args to stack, so spDiff is 0
    m_tx64->emitEagerSyncPoint(m_as, pc, 0);
  }
  // RSP points to the MInstrState we need to use.
  // workaround the fact that rsp moves when we spill registers around call
  PhysReg misReg = m_rScratch;
  emitMovRegReg(m_as, reg::rsp, misReg);

  ArgGroup callArgs(m_regs);
  if (isCppByRef(funcReturnType)) {
    // first arg is pointer to storage for that return value
    if (isSmartPtrRef(funcReturnType)) {
      returnOffset += TVOFF(m_data);
    }
    // misReg is pointing to an MInstrState struct on the C stack.  Pass
    // the address of tvBuiltinReturn to the native function as the location
    // it can construct the return Array, String, Object, or Variant.
    callArgs.addr(misReg, returnOffset); // &misReg[returnOffset]
  }

  // non-pointer args are plain values passed by value.  String, Array,
  // Object, and Variant are passed by const&, ie a pointer to stack memory
  // holding the value, so expect PtrToT types for these.
  // Pointers to smartptr types (String, Array, Object) need adjusting to
  // point to &ptr->m_data.
  for (int i = 0; i < numArgs; i++) {
    const Func::ParamInfo& pi = func->params()[i];
    if (TVOFF(m_data) && isSmartPtrRef(pi.builtinType())) {
      assert(args[i]->type().isPtr() && m_regs[args[i]].getReg() != InvalidReg);
      callArgs.addr(m_regs[args[i]].getReg(), TVOFF(m_data));
    } else {
      callArgs.ssa(args[i]);
    }
  }

  // if the return value is returned by reference, we don't need the
  // return value from this call since we know where the value is.
  cgCallHelper(m_as, Transl::Call((TCA)func->nativeFuncPtr()),
               isCppByRef(funcReturnType) ? InvalidReg : dstReg,
               kSyncPoint, callArgs);

  // load return value from builtin
  // for primitive return types (int, bool), the return value
  // is already in dstReg (the builtin call returns in rax). For return
  // by reference (String, Object, Array, Variant), the builtin writes the
  // return value into MInstrState::tvBuiltinReturn TV, from where it
  // has to be tested and copied.
  if (dstReg == InvalidReg || returnType.isSimpleType()) {
    return;
  }
  // after the call, RSP is back pointing to MInstrState and rSratch
  // has been clobberred.
  misReg = rsp;

  if (returnType.isReferenceType()) {
    assert(isCppByRef(funcReturnType) && isSmartPtrRef(funcReturnType));
    // return type is String, Array, or Object; fold nullptr to KindOfNull
    m_as.   loadq (misReg[returnOffset], dstReg);
    emitLoadImm(m_as, returnType.toDataType(), dstType);
    emitLoadImm(m_as, KindOfNull, m_rScratch);
    m_as.   testq (dstReg, dstReg);
    m_as.   cmov_reg64_reg64 (CC_Z, m_rScratch, dstType);
    return;
  }
  if (returnType.subtypeOf(Type::Cell)
      || returnType.subtypeOf(Type::BoxedCell)) {
    // return type is Variant; fold KindOfUninit to KindOfNull
    assert(isCppByRef(funcReturnType) && !isSmartPtrRef(funcReturnType));
    assert(misReg != dstType);
    emitLoadTVType(m_as, misReg[returnOffset + TVOFF(m_type)], dstType);
    m_as.   loadq (misReg[returnOffset + TVOFF(m_data)], dstReg);
    emitLoadImm(m_as, KindOfNull, m_rScratch);
    static_assert(KindOfUninit == 0, "KindOfUninit must be 0 for test");
    m_as.   testb (rbyte(dstType), rbyte(dstType));
    m_as.   cmov_reg64_reg64 (CC_Z, m_rScratch, dstType);
    return;
  }
  not_reached();
}

void CodeGenerator::cgSpillStack(IRInstruction* inst) {
  SSATmp* dst             = inst->getDst();
  SSATmp* sp              = inst->getSrc(0);
  auto const spDeficit    = inst->getSrc(1)->getValInt();
  auto const spillVals    = inst->getSrcs().subpiece(2);
  auto const numSpillSrcs = spillVals.size();
  auto const dstReg       = m_regs[dst].getReg();
  auto const spReg        = m_regs[sp].getReg();
  auto const spillCells   = spillValueCells(inst);

  int64_t adjustment = (spDeficit - spillCells) * sizeof(Cell);
  for (uint32_t i = 0; i < numSpillSrcs; ++i) {
    const int64_t offset = i * sizeof(Cell) + adjustment;
    if (spillVals[i]->type() == Type::None) {
      // The simplifier detected that we're storing the same value
      // already in there.
      continue;
    }

    auto* val = spillVals[i];
    auto* inst = val->inst();
    while (inst->isPassthrough()) {
      inst = inst->getPassthroughValue()->inst();
    }
    // If our value came from a LdStack on the same sp and offset,
    // we don't need to spill it.
    if (inst->op() == LdStack && inst->getSrc(0) == sp &&
        inst->getExtra<LdStack>()->offset * sizeof(Cell) == offset) {
      FTRACE(1, "{}: Not spilling spill value {} from {}\n",
             __func__, i, inst->toString());
    } else {
      cgStore(spReg, offset, val);
    }
  }

  emitAdjustSp(spReg, dstReg, adjustment);
}

void CodeGenerator::emitAdjustSp(PhysReg spReg,
                                 PhysReg dstReg,
                                 int64_t adjustment /* bytes */) {
  if (adjustment != 0) {
    if (dstReg != spReg) {
      m_as.    lea   (spReg[adjustment], dstReg);
    } else {
      m_as.    addq  (adjustment, dstReg);
    }
  } else {
    emitMovRegReg(m_as, spReg, dstReg);
  }
}

void CodeGenerator::cgNativeImpl(IRInstruction* inst) {
  SSATmp* func  = inst->getSrc(0);
  SSATmp* fp    = inst->getSrc(1);

  assert(func->isConst());
  assert(func->type() == Type::Func);
  const Func* fn = func->getValFunc();

  BuiltinFunction builtinFuncPtr = func->getValFunc()->builtinFuncPtr();
  emitMovRegReg(m_as, m_regs[fp].getReg(), argNumToRegName[0]);
  if (TranslatorX64::eagerRecord(fn)) {
    m_tx64->emitEagerSyncPoint(m_as, fn->getEntry(), 0);
  }
  m_as.call((TCA)builtinFuncPtr);
  recordSyncPoint(m_as);
}

void CodeGenerator::cgLdThis(IRInstruction* inst) {
  SSATmp* dst   = inst->getDst();
  SSATmp* src   = inst->getSrc(0);
  Block* label = inst->getTaken();
  // mov dst, [fp + 0x20]
  auto dstReg = m_regs[dst].getReg();

  // the destination of LdThis could be dead but the instruction
  // itself still useful because of the checks that it does (if it has
  // a label).  So we need to make sure there is a dstReg for this
  // instruction.
  if (dstReg != InvalidReg) {
    // instruction's result is not dead
    m_as.loadq(m_regs[src].getReg()[AROFF(m_this)], dstReg);
  }
  if (label == NULL) return;  // no need to perform its checks
  if (dstReg != InvalidReg) {
    // test 0x01, dst
    m_as.testb(1, rbyte(dstReg));
  } else {
    m_as.testb(1, m_regs[src].getReg()[AROFF(m_this)]);
  }
  // jnz label
  emitFwdJcc(CC_NZ, label);
}

static void emitLdClsCctx(CodeGenerator::Asm& a,
                          PhysReg srcReg,
                          PhysReg dstReg) {
  emitMovRegReg(a, srcReg, dstReg);
  a.    decq(dstReg);
}

void CodeGenerator::cgLdClsCtx(IRInstruction* inst) {
  PhysReg srcReg = m_regs[inst->getSrc(0)].getReg();
  PhysReg dstReg = m_regs[inst->getDst()].getReg();
  // Context could be either a this object or a class ptr
  m_as.   testb(1, rbyte(srcReg));
  ifThenElse(CC_NZ,
             [&] { emitLdClsCctx(m_as, srcReg, dstReg);  }, // ctx is a class
             [&] { emitLdObjClass(m_as, srcReg, dstReg); }  // ctx is this ptr
            );
}

void CodeGenerator::cgLdClsCctx(IRInstruction* inst) {
  PhysReg srcReg = m_regs[inst->getSrc(0)].getReg();
  PhysReg dstReg = m_regs[inst->getDst()].getReg();
  emitLdClsCctx(m_as, srcReg, dstReg);
}

void CodeGenerator::cgLdCtx(IRInstruction* inst) {
  PhysReg dstReg = m_regs[inst->getDst()].getReg();
  PhysReg srcReg = m_regs[inst->getSrc(0)].getReg();
  if (dstReg != InvalidReg) {
    m_as.loadq(srcReg[AROFF(m_this)], dstReg);
  }
}

void CodeGenerator::cgLdCctx(IRInstruction* inst) {
  return cgLdCtx(inst);
}

void CodeGenerator::cgLdConst(IRInstruction* inst) {
  auto const dstReg   = m_regs[inst->getDst()].getReg();
  auto const val      = inst->getExtra<LdConst>()->as<uintptr_t>();
  if (dstReg == InvalidReg) return;
  emitLoadImm(m_as, val, dstReg);
}

void CodeGenerator::cgLdARFuncPtr(IRInstruction* inst) {
  SSATmp* dst   = inst->getDst();
  SSATmp* baseAddr = inst->getSrc(0);
  SSATmp* offset   = inst->getSrc(1);

  auto dstReg  = m_regs[dst].getReg();
  auto baseReg = m_regs[baseAddr].getReg();

  assert(offset->isConst());

  m_as.load_reg64_disp_reg64(baseReg,
                           offset->getValInt() + AROFF(m_func),
                           dstReg);
}

void CodeGenerator::cgLdContLocalsPtr(IRInstruction* inst) {
  auto rCont = m_regs[inst->getSrc(0)].getReg();
  auto rLocals = m_regs[inst->getDst()].getReg();
  m_as.  loadl  (rCont[CONTOFF(m_localsOffset)], r32(rLocals));
  m_as.  addq   (rCont, rLocals);
}

static int getNativeTypeSize(Type type) {
  if (type.subtypeOf(Type::Int | Type::Func)) return sz::qword;
  if (type.subtypeOf(Type::Bool))             return sz::byte;
  not_implemented();
}

void CodeGenerator::cgLdRaw(IRInstruction* inst) {
  SSATmp* dest   = inst->getDst();
  SSATmp* addr   = inst->getSrc(0);
  SSATmp* offset = inst->getSrc(1);

  assert(!(dest->isConst()));

  Reg64 addrReg = m_regs[addr].getReg();
  PhysReg destReg = m_regs[dest].getReg();

  if (addr->isConst()) {
    addrReg = m_rScratch;
    emitLoadImm(m_as, addr->getValRawInt(), addrReg);
  }

  if (offset->isConst()) {
    assert(offset->type() == Type::Int);
    int64_t kind = offset->getValInt();
    RawMemSlot& slot = RawMemSlot::Get(RawMemSlot::Kind(kind));
    int ldSize = slot.getSize();
    int64_t off = slot.getOffset();
    if (ldSize == sz::qword) {
      m_as.loadq (addrReg[off], destReg);
    } else if (ldSize == sz::dword) {
      m_as.loadl (addrReg[off], r32(destReg));
    } else {
      assert(ldSize == sz::byte);
      m_as.loadzbl (addrReg[off], r32(destReg));
    }
  } else {
    int ldSize = getNativeTypeSize(dest->type());
    Reg64 offsetReg = r64(m_regs[offset].getReg());
    if (ldSize == sz::qword) {
      m_as.loadq (addrReg[offsetReg], destReg);
    } else {
      // Not yet supported by our assembler
      assert(ldSize == sz::byte);
      not_implemented();
    }
  }
}

void CodeGenerator::cgStRaw(IRInstruction* inst) {
  auto baseReg = m_regs[inst->getSrc(0)].getReg();
  int64_t kind = inst->getSrc(1)->getValInt();
  SSATmp* value = inst->getSrc(2);

  RawMemSlot& slot = RawMemSlot::Get(RawMemSlot::Kind(kind));
  assert(value->type().equals(slot.type()));
  int stSize = slot.getSize();
  int64_t off = slot.getOffset();
  auto dest = baseReg[off];

  if (value->isConst()) {
    if (stSize == sz::qword) {
      m_as.storeq(value->getValRawInt(), dest);
    } else if (stSize == sz::dword) {
      m_as.storel(value->getValRawInt(), dest);
    } else {
      assert(stSize == sz::byte);
      m_as.storeb(value->getValBool(), dest);
    }
  } else {
    if (stSize == sz::qword) {
      m_as.storeq(r64(m_regs[value].getReg()), dest);
    } else if (stSize == sz::dword) {
      m_as.storel(r32(m_regs[value].getReg()), dest);
    } else {
      assert(stSize == sz::byte);
      m_as.storeb(rbyte(m_regs[value].getReg()), dest);
    }
  }
}

void CodeGenerator::cgLdStaticLocCached(IRInstruction* inst) {
  auto ch = inst->getSrc(0)->getValRawInt();
  auto outReg = m_regs[inst->getDst()].getReg();

  m_as.loadq (rVmTl[ch], outReg);
  m_as.testq (outReg, outReg);
  emitFwdJcc(m_as, CC_Z, inst->getTaken());
}

// If label is set and type is not Gen, this method generates a check
// that bails to the label if the loaded typed value doesn't match type.
void CodeGenerator::cgLoadTypedValue(PhysReg base,
                                     int64_t off,
                                     IRInstruction* inst) {
  Block* label = inst->getTaken();
  Type type   = inst->getTypeParam();
  SSATmp* dst = inst->getDst();

  assert(type == dst->type());
  assert(type.needsReg());
  auto valueDstReg = m_regs[dst].getReg(0);
  auto typeDstReg  = m_regs[dst].getReg(1);

  if (valueDstReg.isXMM()) {
    // Whole typed value is stored in single XMM reg valueDstReg
    assert(RuntimeOption::EvalHHIRAllocXMMRegs);
    assert(typeDstReg == InvalidReg);
    m_as.movdqa(base[off + TVOFF(m_data)], valueDstReg);
    return;
  }

  if (valueDstReg == InvalidReg && typeDstReg == InvalidReg &&
      (label == nullptr || type == Type::Gen)) {
    // a dead load
    return;
  }
  bool useScratchReg = (base == typeDstReg && valueDstReg != InvalidReg);
  if (useScratchReg) {
    // Save base to m_rScratch, because base will be overwritten.
    m_as.mov_reg64_reg64(base, m_rScratch);
  }

  // Load type if it's not dead
  if (typeDstReg != InvalidReg) {
    emitLoadTVType(m_as, base[off + TVOFF(m_type)], typeDstReg);
    if (label) {
      emitTypeCheck(inst->getTypeParam(), typeDstReg, inst->getTaken());
    }
  } else if (label) {
    emitTypeCheck(inst->getTypeParam(),
                  base[off + TVOFF(m_type)],
                  inst->getTaken());
  }

  // Load value if it's not dead
  if (valueDstReg == InvalidReg) return;

  if (useScratchReg) {
    m_as.loadq(m_rScratch[off + TVOFF(m_data)], valueDstReg);
  } else {
    m_as.loadq(base[off + TVOFF(m_data)], valueDstReg);
  }
}

void CodeGenerator::cgStoreTypedValue(PhysReg base,
                                      int64_t off,
                                      SSATmp* src) {
  assert(src->type().needsReg());
  auto srcReg0 = m_regs[src].getReg(0);
  auto srcReg1 = m_regs[src].getReg(1);
  if (srcReg0.isXMM()) {
    // Whole typed value is stored in single XMM reg srcReg0
    assert(RuntimeOption::EvalHHIRAllocXMMRegs);
    assert(srcReg1 == InvalidReg);
    m_as.movdqa(srcReg0, base[off + TVOFF(m_data)]);
    return;
  }
  m_as.storeq(srcReg0, base[off + TVOFF(m_data)]);
  emitStoreTVType(m_as, srcReg1, base[off + TVOFF(m_type)]);
}

void CodeGenerator::cgStore(PhysReg base,
                            int64_t off,
                            SSATmp* src,
                            bool genStoreType) {
  Type type = src->type();
  if (type.needsReg()) {
    cgStoreTypedValue(base, off, src);
    return;
  }
  // store the type
  if (genStoreType) {
    emitStoreTVType(m_as, type.toDataType(), base[off + TVOFF(m_type)]);
  }
  if (type.isNull()) {
    // no need to store a value for null or uninit
    return;
  }
  if (src->isConst()) {
    int64_t val = 0;
    if (type.subtypeOf(Type::Bool | Type::Int | Type::Dbl |
                       Type::Arr | Type::StaticStr | Type::Cls)) {
        val = src->getValBits();
    } else {
      not_reached();
    }
    m_as.storeq(val, base[off + TVOFF(m_data)]);
  } else {
    zeroExtendIfBool(m_as, src, m_regs[src]);
    emitStoreReg(m_as, m_regs[src].getReg(), base[off + TVOFF(m_data)]);
  }
}

void CodeGenerator::cgLoad(PhysReg base,
                           int64_t off,
                           IRInstruction* inst) {
  Type type = inst->getTypeParam();
  if (type.needsReg()) {
    return cgLoadTypedValue(base, off, inst);
  }
  Block* label = inst->getTaken();
  if (label != NULL) {
    emitTypeCheck(inst->getTypeParam(),
                  base[off + TVOFF(m_type)],
                  inst->getTaken());
  }
  if (type.isNull()) return; // these are constants
  auto dstReg = m_regs[inst->getDst()].getReg();
  // if dstReg == InvalidReg then the value of this load is dead
  if (dstReg == InvalidReg) return;

  if (type == Type::Bool) {
    m_as.load_reg64_disp_reg32(base, off + TVOFF(m_data),  dstReg);
  } else {
    emitLoadReg(m_as, base[off + TVOFF(m_data)],  dstReg);
  }
}

void CodeGenerator::cgLdProp(IRInstruction* inst) {
  cgLoad(m_regs[inst->getSrc(0)].getReg(), inst->getSrc(1)->getValInt(), inst);
}

void CodeGenerator::cgLdMem(IRInstruction * inst) {
  cgLoad(m_regs[inst->getSrc(0)].getReg(), inst->getSrc(1)->getValInt(), inst);
}

void CodeGenerator::cgLdRef(IRInstruction* inst) {
  cgLoad(m_regs[inst->getSrc(0)].getReg(), RefData::tvOffset(), inst);
}

void CodeGenerator::recordSyncPoint(Asm& as,
                                    SyncOptions sync /* = kSyncPoint */) {
  assert(m_state.lastMarker);

  Offset stackOff = m_state.lastMarker->stackOff;
  switch (sync) {
  case kSyncPointAdjustOne:
    stackOff -= 1;
    break;
  case kSyncPoint:
    break;
  case kNoSyncPoint:
    assert(0);
  }

  Offset pcOff = m_state.lastMarker->bcOff - m_state.lastMarker->func->base();

  FTRACE(3, "IR recordSyncPoint: {} {} {}\n", as.code.frontier, pcOff,
         stackOff);
  m_tx64->recordSyncPoint(as, pcOff, stackOff);
}

void CodeGenerator::cgLdAddr(IRInstruction* inst) {
  auto base = m_regs[inst->getSrc(0)].getReg();
  int64_t offset = inst->getSrc(1)->getValInt();
  m_as.lea (base[offset], m_regs[inst->getDst()].getReg());
}

void CodeGenerator::cgLdLoc(IRInstruction* inst) {
  cgLoad(m_regs[inst->getSrc(0)].getReg(),
         getLocalOffset(inst->getExtra<LdLoc>()->locId),
         inst);
}

void CodeGenerator::cgLdLocAddr(IRInstruction* inst) {
  auto const fpReg  = m_regs[inst->getSrc(0)].getReg();
  auto const offset = getLocalOffset(inst->getExtra<LdLocAddr>()->locId);
  if (m_regs[inst->getDst()].hasReg()) {
    m_as.lea(fpReg[offset], m_regs[inst->getDst()].getReg());
  }
}

void CodeGenerator::cgLdStackAddr(IRInstruction* inst) {
  auto const base   = m_regs[inst->getSrc(0)].getReg();
  auto const offset = cellsToBytes(inst->getExtra<LdStackAddr>()->offset);
  m_as.lea (base[offset], m_regs[inst->getDst()].getReg());
}

void CodeGenerator::cgLdStack(IRInstruction* inst) {
  assert(inst->getTaken() == nullptr);
  cgLoad(m_regs[inst->getSrc(0)].getReg(),
         cellsToBytes(inst->getExtra<LdStack>()->offset),
         inst);
}

void CodeGenerator::cgGuardStk(IRInstruction* inst) {
  auto const rSP = m_regs[inst->getSrc(0)].getReg();
  auto const off = cellsToBytes(inst->getExtra<GuardStk>()->offset) +
                     TVOFF(m_type);
  emitTypeGuard(inst->getTypeParam(), rSP[off]);
}

void CodeGenerator::cgCheckStk(IRInstruction* inst) {
  auto const rbase = m_regs[inst->getSrc(0)].getReg();
  auto const off   = cellsToBytes(inst->getExtra<CheckStk>()->offset) +
                       TVOFF(m_type);
  emitTypeCheck(inst->getTypeParam(), rbase[off], inst->getTaken());
}

void CodeGenerator::cgGuardLoc(IRInstruction* inst) {
  auto const rFP = m_regs[inst->getSrc(0)].getReg();
  auto const off = getLocalOffset(inst->getExtra<GuardLoc>()->locId) +
                     TVOFF(m_type);
  emitTypeGuard(inst->getTypeParam(), rFP[off]);
}

void CodeGenerator::cgCheckLoc(IRInstruction* inst) {
  auto const rbase = m_regs[inst->getSrc(0)].getReg();
  auto const off   = getLocalOffset(inst->getExtra<CheckLoc>()->locId) +
                       TVOFF(m_type);
  emitTypeCheck(inst->getTypeParam(), rbase[off], inst->getTaken());
}

template<class MemLoc>
void CodeGenerator::emitSideExitGuard(Type type, MemLoc mem, Offset taken) {
  auto const cc = emitTypeTest(type, mem, true /* negate */);
  auto const sk = SrcKey(getCurFunc(), taken);
  if (cc == CC_None) return;
  m_tx64->emitBindJcc(m_as, cc, sk, REQ_BIND_SIDE_EXIT);
}

void CodeGenerator::cgSideExitGuardLoc(IRInstruction* inst) {
  auto const fp    = m_regs[inst->getSrc(0)].getReg();
  auto const extra = inst->getExtra<SideExitGuardLoc>();
  emitSideExitGuard(inst->getTypeParam(),
                    fp[getLocalOffset(extra->checkedSlot) + TVOFF(m_type)],
                    extra->taken);
}

void CodeGenerator::cgSideExitGuardStk(IRInstruction* inst) {
  auto const sp    = m_regs[inst->getSrc(0)].getReg();
  auto const extra = inst->getExtra<SideExitGuardStk>();
  emitSideExitGuard(inst->getTypeParam(),
                    sp[cellsToBytes(extra->checkedSlot) + TVOFF(m_type)],
                    extra->taken);
}

void CodeGenerator::cgDefMIStateBase(IRInstruction* inst) {
  assert(inst->getDst()->type() == Type::PtrToCell);
  assert(m_regs[inst->getDst()].getReg() == rsp);
}

void CodeGenerator::cgCheckType(IRInstruction* inst) {
  auto const src   = inst->getSrc(0);
  auto const rType = m_regs[src].getReg(1);

  auto const cc = emitTypeTest(inst->getTypeParam(), rType, true);
  if (cc == CC_None) return;

  emitFwdJcc(cc, inst->getTaken());

  auto const dstReg = m_regs[inst->getDst()].getReg();
  if (dstReg != InvalidReg) {
    emitMovRegReg(m_as, m_regs[src].getReg(0), dstReg);
  }
}

void CodeGenerator::cgCheckTypeMem(IRInstruction* inst) {
  auto const reg = m_regs[inst->getSrc(0)].getReg();
  emitTypeCheck(inst->getTypeParam(), reg[TVOFF(m_type)], inst->getTaken());
}

void CodeGenerator::cgGuardRefs(IRInstruction* inst) {
  assert(inst->getNumSrcs() == 6);

  SSATmp* funcPtrTmp = inst->getSrc(0);
  SSATmp* nParamsTmp = inst->getSrc(1);
  SSATmp* bitsPtrTmp = inst->getSrc(2);
  SSATmp* firstBitNumTmp = inst->getSrc(3);
  SSATmp* mask64Tmp  = inst->getSrc(4);
  SSATmp* vals64Tmp  = inst->getSrc(5);

  // Get values in place
  assert(funcPtrTmp->type() == Type::Func);
  auto funcPtrReg = m_regs[funcPtrTmp].getReg();
  assert(funcPtrReg != InvalidReg);

  assert(nParamsTmp->type() == Type::Int);
  auto nParamsReg = m_regs[nParamsTmp].getReg();
  assert(nParamsReg != InvalidReg);

  assert(bitsPtrTmp->type() == Type::Int);
  auto bitsPtrReg = m_regs[bitsPtrTmp].getReg();
  assert(bitsPtrReg != InvalidReg);

  assert(firstBitNumTmp->isConst() && firstBitNumTmp->type() == Type::Int);
  uint32_t firstBitNum = (uint32_t)(firstBitNumTmp->getValInt());

  assert(mask64Tmp->type() == Type::Int);
  assert(mask64Tmp->inst()->op() == LdConst);
  auto mask64Reg = m_regs[mask64Tmp].getReg();
  assert(mask64Reg != InvalidReg);
  int64_t mask64 = mask64Tmp->getValInt();

  assert(vals64Tmp->type() == Type::Int);
  assert(vals64Tmp->inst()->op() == LdConst);
  auto vals64Reg = m_regs[vals64Tmp].getReg();
  assert(vals64Reg != InvalidReg);
  int64_t vals64 = vals64Tmp->getValInt();

  auto const destSK = SrcKey(getCurFunc(), m_curTrace->getBcOff());
  auto const destSR = m_tx64->getSrcRec(destSK);

  auto thenBody = [&] {
    auto bitsValReg = m_rScratch;
    //   Load the bit values in bitValReg:
    //     bitsValReg <- [bitsValPtr + (firstBitNum / 64)]
    m_as.load_reg64_disp_reg64(bitsPtrReg, sizeof(uint64_t) * (firstBitNum / 64),
                               bitsValReg);
    //     bitsValReg <- bitsValReg & mask64
    m_as.and_reg64_reg64(mask64Reg, bitsValReg);

    //   If bitsValReg != vals64Reg, then goto Exit
    m_as.cmp_reg64_reg64(bitsValReg, vals64Reg);
    m_tx64->emitFallbackCondJmp(m_as, *destSR, CC_NE);
  };

  // If few enough args...
  m_as.cmp_imm32_reg32(firstBitNum + 1, nParamsReg);
  if (vals64 == 0 && mask64 == 0) {
    ifThen(m_as, CC_NL, thenBody);
  } else if (vals64 != 0 && vals64 != mask64) {
    m_tx64->emitFallbackCondJmp(m_as, *destSR, CC_L);
    thenBody();
  } else if (vals64 != 0) {
    ifThenElse(CC_NL, thenBody, /* else */ [&] {
      //   If not special builtin...
      m_as.testl(AttrVariadicByRef, funcPtrReg[Func::attrsOff()]);
      m_tx64->emitFallbackCondJmp(m_as, *destSR, CC_Z);
    });
  } else {
    ifThenElse(CC_NL, thenBody, /* else */ [&] {
      m_as.testl(AttrVariadicByRef, funcPtrReg[Func::attrsOff()]);
      m_tx64->emitFallbackCondJmp(m_as, *destSR, CC_NZ);
    });
  }
}

void CodeGenerator::cgLdPropAddr(IRInstruction* inst) {
  SSATmp*   dst   = inst->getDst();
  SSATmp*   obj   = inst->getSrc(0);
  SSATmp*   prop  = inst->getSrc(1);

  assert(prop->isConst() && prop->type() == Type::Int);

  auto dstReg = m_regs[dst].getReg();
  auto objReg = m_regs[obj].getReg();

  assert(objReg != InvalidReg);
  assert(dstReg != InvalidReg);

  int64_t offset = prop->getValInt();
  m_as.lea_reg64_disp_reg64(objReg, offset, dstReg);
}

void CodeGenerator::cgLdClsMethod(IRInstruction* inst) {
  SSATmp* dst   = inst->getDst();
  SSATmp* cls   = inst->getSrc(0);
  SSATmp* mSlot = inst->getSrc(1);

  assert(cls->type() == Type::Cls);
  assert(mSlot->isConst() && mSlot->type() == Type::Int);
  uint64_t mSlotInt64 = mSlot->getValRawInt();
  // We're going to multiply mSlotVal by sizeof(Func*) and use
  // it as a 32-bit offset (methOff) below.
  if (mSlotInt64 > (std::numeric_limits<uint32_t>::max() / sizeof(Func*))) {
    CG_PUNT(cgLdClsMethod_large_offset);
  }
  int32_t mSlotVal = (uint32_t) mSlotInt64;

  Reg64 dstReg = m_regs[dst].getReg();
  assert(dstReg != InvalidReg);

  Reg64 clsReg = m_regs[cls].getReg();
  if (clsReg == InvalidReg) {
    CG_PUNT(LdClsMethod);
  }

  Offset vecOff  = Class::getMethodsOffset() + Class::MethodMap::vecOff();
  int32_t  methOff = mSlotVal * sizeof(Func*);
  m_as.loadq(clsReg[vecOff],  dstReg);
  m_as.loadq(dstReg[methOff], dstReg);
}

void CodeGenerator::cgLdClsMethodCache(IRInstruction* inst) {
  SSATmp* dst        = inst->getDst();
  SSATmp* className  = inst->getSrc(0);
  SSATmp* methodName = inst->getSrc(1);
  SSATmp* baseClass  = inst->getSrc(2);
  Block*  label      = inst->getTaken();

  // Stats::emitInc(a, Stats::TgtCache_StaticMethodHit);
  const StringData*  cls    = className->getValStr();
  const StringData*  method = methodName->getValStr();
  auto const ne             = baseClass->getValNamedEntity();
  TargetCache::CacheHandle ch =
    TargetCache::StaticMethodCache::alloc(cls,
                                          method,
                                          getContextName(getCurClass()));
  auto funcDestReg  = m_regs[dst].getReg(0);
  auto classDestReg = m_regs[dst].getReg(1);
  auto offsetof_func = offsetof(TargetCache::StaticMethodCache, m_func);
  auto offsetof_cls  = offsetof(TargetCache::StaticMethodCache, m_cls);

  assert(funcDestReg != InvalidReg && classDestReg != InvalidReg);
  // Attempt to retrieve the func* and class* from cache
  m_as.loadq(rVmTl[ch + offsetof_func], funcDestReg);
  m_as.loadq(rVmTl[ch + offsetof_cls], classDestReg);
  m_as.testq(funcDestReg, funcDestReg);
  // May have retrieved a NULL from the cache
  // handle case where method is not entered in the cache
  unlikelyIfBlock(CC_E, [&] (Asm& a) {
    if (false) { // typecheck
      const UNUSED Func* f = StaticMethodCache::lookupIR(ch, ne, cls, method);
    }
    // can raise an error if class is undefined
    cgCallHelper(a,
                 (TCA)StaticMethodCache::lookupIR,
                 funcDestReg,
                 kSyncPoint,
                 ArgGroup(m_regs).imm(ch)      // Handle ch
                           .immPtr(ne)         // NamedEntity* np.second
                           .immPtr(cls)        // className
                           .immPtr(method)     // methodName
    );
    // recordInstrCall is done in cgCallHelper
    a.testq(funcDestReg, funcDestReg);
    a.loadq(rVmTl[ch + offsetof_cls], classDestReg);
    // if StaticMethodCache::lookupIR() returned NULL, jmp to label
    emitFwdJcc(a, CC_Z, label);
  });
}

/**
 * Helper to emit getting the value for ActRec's m_this/m_cls slot
 * from a This pointer depending on whether the callee method is
 * static or not.
 */
void CodeGenerator::emitGetCtxFwdCallWithThis(PhysReg ctxReg,
                                              bool    staticCallee) {
  if (staticCallee) {
    // Load (this->m_cls | 0x1) into ctxReg.
    m_as.loadq(ctxReg[ObjectData::getVMClassOffset()], ctxReg);
    m_as.orq(1, ctxReg);
  } else {
    // Just incref $this.
    emitIncRef(m_as, ctxReg);
  }
}

/**
 * This method is similar to emitGetCtxFwdCallWithThis above, but
 * whether or not the callee is a static method is unknown at JIT
 * time, and that is determined dynamically by looking up into the
 * StaticMethodFCache.
 */
void CodeGenerator::emitGetCtxFwdCallWithThisDyn(PhysReg      destCtxReg,
                                                 PhysReg      thisReg,
                                                 CacheHandle& ch) {
  Label NonStaticCall, End;

  // thisReg is holding $this. Should we pass it to the callee?
  m_as.cmpl(1, rVmTl[ch + offsetof(StaticMethodFCache, m_static)]);
  m_as.jcc8(CC_NE, NonStaticCall);
  // If calling a static method...
  {
    // Load (this->m_cls | 0x1) into destCtxReg
    m_as.loadq(thisReg[ObjectData::getVMClassOffset()], destCtxReg);
    m_as.orq(1, destCtxReg);
    m_as.jmp8(End);
  }
  // Else: calling non-static method
  {
    asm_label(m_as, NonStaticCall);
    emitMovRegReg(m_as, thisReg, destCtxReg);
    emitIncRef(m_as, destCtxReg);
  }
  asm_label(m_as, End);
}

void CodeGenerator::cgGetCtxFwdCall(IRInstruction* inst) {
  PhysReg destCtxReg = m_regs[inst->getDst()].getReg(0);
  SSATmp*  srcCtxTmp = inst->getSrc(0);
  const Func* callee = inst->getSrc(1)->getValFunc();
  bool      withThis = srcCtxTmp->isA(Type::Obj);

  // Eagerly move src into the dest reg
  emitMovRegReg(m_as, m_regs[srcCtxTmp].getReg(0), destCtxReg);

  Label End;
  // If we don't know whether we have a This, we need to check dynamically
  if (!withThis) {
    m_as.testb(1, rbyte(destCtxReg));
    m_as.jcc8(CC_NZ, End);
  }

  // If we have a This pointer in destCtxReg, then select either This
  // or its Class based on whether callee is static or not
  emitGetCtxFwdCallWithThis(destCtxReg, (callee->attrs() & AttrStatic));

  asm_label(m_as, End);
}

void CodeGenerator::cgLdClsMethodFCache(IRInstruction* inst) {
  PhysReg         funcDestReg = m_regs[inst->getDst()].getReg(0);
  PhysReg          destCtxReg = m_regs[inst->getDst()].getReg(1);
  const Class*            cls = inst->getSrc(0)->getValClass();
  const StringData*  methName = inst->getSrc(1)->getValStr();
  SSATmp*           srcCtxTmp = inst->getSrc(2);
  PhysReg           srcCtxReg = m_regs[srcCtxTmp].getReg(0);
  Block*            exitLabel = inst->getTaken();
  const StringData*   clsName = cls->name();
  CacheHandle              ch = StaticMethodFCache::alloc(clsName, methName,
                                              getContextName(getCurClass()));

  assert(funcDestReg != InvalidReg && destCtxReg != InvalidReg);
  emitMovRegReg(m_as, srcCtxReg, destCtxReg);
  m_as.loadq(rVmTl[ch], funcDestReg);
  m_as.testq(funcDestReg, funcDestReg);

  Label End;

  // Handle case where method is not entered in the cache
  unlikelyIfBlock(CC_E, [&] (Asm& a) {
    const Func* (*lookup)(CacheHandle, const Class*, const StringData*) =
      StaticMethodFCache::lookupIR;
    // preserve destCtxReg across the call since it wouldn't be otherwise
    RegSet toSave = m_state.liveRegs[inst] | RegSet(destCtxReg);
    cgCallHelper(a, Transl::Call((TCA)lookup),
                 funcDestReg, InvalidReg,
                 kSyncPoint,
                 ArgGroup(m_regs).imm(ch)
                           .immPtr(cls)
                           .immPtr(methName),
                 toSave);
    // If entry found in target cache, jump back to m_as.
    // Otherwise, bail to exit label
    a.testq(funcDestReg, funcDestReg);
    emitFwdJcc(a, CC_Z, exitLabel);
  });

  auto t = srcCtxTmp->type();
  assert(!t.equals(Type::Cls));
  if (t.equals(Type::Cctx)) {
    return; // done: destCtxReg already has srcCtxReg
  } else if (t == Type::Obj) {
    // unconditionally run code produced by emitGetCtxFwdCallWithThisDyn below
    // break
  } else if (t == Type::Ctx) {
    // dynamically check if we have a This pointer and
    // call emitGetCtxFwdCallWithThisDyn below
    m_as.testb(1, rbyte(destCtxReg));
    m_as.jcc8(CC_NZ, End);
  } else {
    not_reached();
  }

  // If we have a 'this' pointer ...
  emitGetCtxFwdCallWithThisDyn(destCtxReg, destCtxReg, ch);

  asm_label(m_as, End);
}

void CodeGenerator::cgLdClsPropAddrCached(IRInstruction* inst) {
  using namespace Transl::TargetCache;
  SSATmp* dst      = inst->getDst();
  SSATmp* cls      = inst->getSrc(0);
  SSATmp* propName = inst->getSrc(1);
  SSATmp* clsName  = inst->getSrc(2);
  SSATmp* cxt      = inst->getSrc(3);
  Block* target    = inst->getTaken();

  const StringData* propNameString = propName->getValStr();
  const StringData* clsNameString  = clsName->getValStr();

  string sds(Util::toLower(clsNameString->data()) + ":" +
             string(propNameString->data(), propNameString->size()));
  StackStringData sd(sds.c_str(), sds.size(), AttachLiteral);
  CacheHandle ch = SPropCache::alloc(&sd);

  auto dstReg = m_regs[dst].getReg();
  // Cls is live in the slow path call to lookupIR, so we have to be
  // careful not to clobber it before the branch to slow path. So
  // use the scratch register as a temporary destination if cls is
  // assigned the same register as the dst register.
  auto tmpReg = dstReg;
  if (dstReg == InvalidReg || dstReg == m_regs[cls].getReg()) {
    tmpReg = PhysReg(m_rScratch);
  }

  // Could be optimized to cmp against zero when !label && dstReg == InvalidReg
  m_as.loadq(rVmTl[ch], tmpReg);
  m_as.testq(tmpReg, tmpReg);
  unlikelyIfBlock(CC_E, [&] (Asm& a) {
    cgCallHelper(a,
                 target ? (TCA)SPropCache::lookupIR<false>
                        : (TCA)SPropCache::lookupIR<true>, // raise on error
                 tmpReg,
                 kSyncPoint, // could re-enter to initialize properties
                 ArgGroup(m_regs).imm(ch).ssa(cls).ssa(propName).ssa(cxt));
    if (target) {
      a.testq(tmpReg, tmpReg);
      emitFwdJcc(a, CC_Z, target);
    }
  });
  if (dstReg != InvalidReg) {
    emitMovRegReg(m_as, tmpReg, dstReg);
  }
}

void CodeGenerator::cgLdClsPropAddr(IRInstruction* inst) {
  SSATmp* dst   = inst->getDst();
  SSATmp* cls   = inst->getSrc(0);
  SSATmp* prop  = inst->getSrc(1);
  SSATmp* cxt   = inst->getSrc(2);
  Block* target = inst->getTaken();
  auto dstReg = m_regs[dst].getReg();
  if (dstReg == InvalidReg && target) {
    // result is unused but this instruction was not eliminated
    // because its essential
    dstReg = m_rScratch;
  }
  cgCallHelper(m_as,
               target ? (TCA)SPropCache::lookupSProp<false>
                      : (TCA)SPropCache::lookupSProp<true>, // raise on error
               dstReg,
               kSyncPoint, // could re-enter to initialize properties
               ArgGroup(m_regs).ssa(cls).ssa(prop).ssa(cxt));
  if (target) {
    m_as.testq(dstReg, dstReg);
    emitFwdJcc(m_as, CC_Z, target);
  }
}

TargetCache::CacheHandle CodeGenerator::cgLdClsCachedCommon(
  IRInstruction* inst) {
  SSATmp* dst = inst->getDst();
  const StringData* className = inst->getSrc(0)->getValStr();
  auto ch = TargetCache::allocKnownClass(className);
  auto dstReg = m_regs[dst].getReg();
  if (dstReg == InvalidReg) {
    m_as. cmpq   (0, rVmTl[ch]);
  } else {
    m_as.  loadq  (rVmTl[ch], dstReg);
    m_as.  testq  (dstReg, dstReg);
  }

  return ch;
}

void CodeGenerator::cgLdClsCached(IRInstruction* inst) {
  auto ch = cgLdClsCachedCommon(inst);
  unlikelyIfBlock(CC_E, [&] (Asm& a) {
    // Passing only two arguments to lookupKnownClass, since the
    // third is ignored in the checkOnly==false case.
    cgCallHelper(a,
                 (TCA)TargetCache::lookupKnownClass<false>,
                 inst->getDst(),
                 kSyncPoint,
                 ArgGroup(m_regs).addr(rVmTl, intptr_t(ch)).ssas(inst, 0));
  });
}

void CodeGenerator::cgLdClsCachedSafe(IRInstruction* inst) {
  cgLdClsCachedCommon(inst);
  if (Block* taken = inst->getTaken()) {
    emitFwdJcc(CC_Z, taken);
  }
}

void CodeGenerator::cgLdCls(IRInstruction* inst) {
  SSATmp* dst       = inst->getDst();
  SSATmp* className = inst->getSrc(0);

  CacheHandle ch = ClassCache::alloc();
  cgCallHelper(m_as, (TCA)ClassCache::lookup, dst, kSyncPoint,
               ArgGroup(m_regs).imm(ch).ssa(className));
}

static StringData* fullConstName(SSATmp* cls, SSATmp* cnsName) {
  return StringData::GetStaticString(
      Util::toLower(cls->getValStr()->data()) + "::" +
      cnsName->getValStr()->data());
}

void CodeGenerator::cgLdClsCns(IRInstruction* inst) {
  SSATmp* cnsName = inst->getSrc(0);
  SSATmp* cls     = inst->getSrc(1);

  StringData* fullName = fullConstName(cls, cnsName);
  TargetCache::CacheHandle ch = TargetCache::allocClassConstant(fullName);
  // note that we bail from the trace if the target cache entry is empty
  // for this class constant or if the type assertion fails.
  // TODO: handle the slow case helper call.
  cgLoad(rVmTl, ch, inst);
}

void CodeGenerator::cgLookupClsCns(IRInstruction* inst) {
  SSATmp*   cnsName = inst->getSrc(0);
  SSATmp*   cls     = inst->getSrc(1);

  assert(inst->getTypeParam() == Type::Cell);
  assert(cnsName->isConst() && cnsName->type() == Type::StaticStr);
  assert(cls->isConst() && cls->type() == Type::StaticStr);

  StringData* fullName = fullConstName(cls, cnsName);
  TargetCache::CacheHandle ch = TargetCache::allocClassConstant(fullName);

  ArgGroup args(m_regs);
  args.addr(rVmTl, ch)
      .immPtr(Unit::GetNamedEntity(cls->getValStr()))
      .immPtr(cls->getValStr())
      .immPtr(cnsName->getValStr());

  cgCallHelper(m_as, TCA(TargetCache::lookupClassConstantTv),
               inst->getDst(), kSyncPoint, args, DestType::TV);
}

void CodeGenerator::cgLdCns(IRInstruction* inst) {
  const StringData* cnsName = inst->getSrc(0)->getValStr();

  TargetCache::CacheHandle ch = StringData::DefCnsHandle(cnsName, false);
  // Has an unlikely branch to a LookupCns
  cgLoad(rVmTl, ch, inst);
}

static TypedValue lookupCnsHelper(const TypedValue* tv, StringData* nm) {
  assert(tv->m_type == KindOfUninit);
  TypedValue *cns = nullptr;
  TypedValue c1;
  if (UNLIKELY(tv->m_data.pref != nullptr)) {
    ClassInfo::ConstantInfo* ci =
      (ClassInfo::ConstantInfo*)(void*)tv->m_data.pref;
    cns = const_cast<Variant&>(ci->getDeferredValue()).asTypedValue();
    tvReadCell(cns, &c1);
  } else {
    if (UNLIKELY(TargetCache::s_constants != nullptr)) {
      cns = TargetCache::s_constants->HphpArray::nvGet(nm);
    }
    if (!cns) {
      cns = Unit::loadCns(const_cast<StringData*>(nm));
    }
    if (UNLIKELY(!cns)) {
      raise_notice(Strings::UNDEFINED_CONSTANT, nm->data(), nm->data());
      c1.m_data.pstr = const_cast<StringData*>(nm);
      c1.m_type = KindOfStaticString;
    } else {
      c1.m_type = cns->m_type;
      c1.m_data = cns->m_data;
    }
  }
  return c1;
}

void CodeGenerator::cgLookupCns(IRInstruction* inst) {
  SSATmp* cnsNameTmp = inst->getSrc(0);

  assert(inst->getTypeParam() == Type::Cell);
  assert(cnsNameTmp->isConst() && cnsNameTmp->type() == Type::StaticStr);

  const StringData* cnsName = cnsNameTmp->getValStr();
  TargetCache::CacheHandle ch = StringData::DefCnsHandle(cnsName, false);

  ArgGroup args(m_regs);
  args.addr(rVmTl, ch)
      .immPtr(cnsName);

  cgCallHelper(m_as, TCA(lookupCnsHelper),
               inst->getDst(), kSyncPoint, args, DestType::TV);
}

HOT_FUNC_VM
static inline int64_t ak_exist_string_helper(StringData* key, ArrayData* arr) {
  int64_t n;
  if (key->isStrictlyInteger(n)) {
    return arr->exists(n);
  }
  return arr->exists(StrNR(key));
}

HOT_FUNC_VM
static int64_t ak_exist_string(StringData* key, ArrayData* arr) {
  int64_t res = ak_exist_string_helper(key, arr);
  return res;
}

HOT_FUNC_VM
static int64_t ak_exist_int(int64_t key, ArrayData* arr) {
  bool res = arr->exists(key);
  return res;
}

HOT_FUNC_VM
static int64_t ak_exist_string_obj(StringData* key, ObjectData* obj) {
  if (obj->isCollection()) {
    return collectionOffsetContains(obj, key);
  }
  CArrRef arr = obj->o_toArray();
  int64_t res = ak_exist_string_helper(key, arr.get());
  return res;
}

HOT_FUNC_VM
static int64_t ak_exist_int_obj(int64_t key, ObjectData* obj) {
  if (obj->isCollection()) {
    return collectionOffsetContains(obj, key);
  }
  CArrRef arr = obj->o_toArray();
  bool res = arr.get()->exists(key);
  return res;
}

void CodeGenerator::cgAKExists(IRInstruction* inst) {
  SSATmp* arr = inst->getSrc(0);
  SSATmp* key = inst->getSrc(1);

  if (key->type().isNull()) {
    if (arr->isA(Type::Arr)) {
      cgCallHelper(m_as,
                   (TCA)ak_exist_string,
                   inst->getDst(),
                   kNoSyncPoint,
                   ArgGroup(m_regs).immPtr(empty_string.get()).ssa(arr));
    } else {
      m_as.mov_imm64_reg(0, m_regs[inst->getDst()].getReg());
    }
    return;
  }

  TCA helper_func =
    arr->isA(Type::Obj)
    ? (key->isA(Type::Int) ? (TCA)ak_exist_int_obj : (TCA)ak_exist_string_obj)
    : (key->isA(Type::Int) ? (TCA)ak_exist_int : (TCA)ak_exist_string);

  cgCallHelper(m_as,
               helper_func,
               inst->getDst(),
               kNoSyncPoint,
               ArgGroup(m_regs).ssa(key).ssa(arr));
}

HOT_FUNC_VM static TypedValue* ldGblAddrHelper(StringData* name) {
  return g_vmContext->m_globalVarEnv->lookup(name);
}

HOT_FUNC_VM static TypedValue* ldGblAddrDefHelper(StringData* name) {
  TypedValue* r = g_vmContext->m_globalVarEnv->lookupAdd(name);
  decRefStr(name);
  return r;
}

void CodeGenerator::cgLdGblAddr(IRInstruction* inst) {
  auto dstReg = m_regs[inst->getDst()].getReg();
  cgCallHelper(m_as, (TCA)ldGblAddrHelper, dstReg, kNoSyncPoint,
               ArgGroup(m_regs).ssa(inst->getSrc(0)));
  m_as.testq(dstReg, dstReg);
  emitFwdJcc(CC_Z, inst->getTaken());
}

void CodeGenerator::cgLdGblAddrDef(IRInstruction* inst) {
  cgCallHelper(m_as, (TCA)ldGblAddrDefHelper, inst->getDst(), kNoSyncPoint,
               ArgGroup(m_regs).ssa(inst->getSrc(0)));
}

void CodeGenerator::emitTestZero(SSATmp* src) {
  auto& a = m_as;
  auto reg = m_regs[src].getReg();

  /*
   * If src is const, normally a earlier optimization pass should have
   * converted the thing testing this condition into something
   * unconditional.  So rather than supporting constants efficiently
   * here, we just materialize the value into a register.
   */
  if (reg == InvalidReg) {
    reg = m_rScratch;
    a.    movq   (src->getValBits(), reg);
  }

  if (src->isA(Type::Bool)) {
    a.    testb  (rbyte(reg), rbyte(reg));
  } else {
    a.    testq  (reg, reg);
  }
}

void CodeGenerator::cgJmpZero(IRInstruction* inst) {
  emitTestZero(inst->getSrc(0));
  emitFwdJcc(CC_Z, inst->getTaken());
}

void CodeGenerator::cgJmpNZero(IRInstruction* inst) {
  emitTestZero(inst->getSrc(0));
  emitFwdJcc(CC_NZ, inst->getTaken());
}

void CodeGenerator::cgReqBindJmpZero(IRInstruction* inst) {
  // TODO(#2404427): prepareForTestAndSmash?
  emitTestZero(inst->getSrc(0));
  emitReqBindJcc(CC_Z, inst->getExtra<ReqBindJmpZero>());
}

void CodeGenerator::cgReqBindJmpNZero(IRInstruction* inst) {
  // TODO(#2404427): prepareForTestAndSmash?
  emitTestZero(inst->getSrc(0));
  emitReqBindJcc(CC_NZ, inst->getExtra<ReqBindJmpNZero>());
}

void CodeGenerator::cgJmp_(IRInstruction* inst) {
  assert(inst->getNumSrcs() == inst->getTaken()->getLabel()->getNumDsts());
  if (unsigned n = inst->getNumSrcs()) {
    // Parallel-copy sources to the label's destination registers.
    // TODO: t2040286: this only works if all destinations fit in registers.
    SrcRange srcs = inst->getSrcs();
    DstRange dsts = inst->getTaken()->getLabel()->getDsts();
    ArgGroup args(m_regs);
    for (unsigned i = 0, j = 0; i < n; i++) {
      assert(srcs[i]->type().subtypeOf(dsts[i].type()));
      SSATmp *dst = &dsts[i], *src = srcs[i];
      // Currently, full XMM registers cannot be assigned to SSATmps
      // passed from to Jmp_ to DefLabel. If this changes, it'll require
      // teaching shuffleArgs() how to handle full XMM values.
      assert(!m_regs[src].isFullXMM() && !m_regs[dst].isFullXMM());
      if (m_regs[dst].getReg(0) == InvalidReg) continue; // dst is unused.
      // first dst register
      args.ssa(src);
      args[j++].setDstReg(m_regs[dst].getReg(0));
      // second dst register, if any
      if (dst->numNeededRegs() == 2) {
        if (src->numNeededRegs() < 2) {
          // src has known data type, but dst doesn't - pass immediate type
          assert(src->type().isKnownDataType());
          args.imm(src->type().toDataType());
        } else {
          // pass src's second register
          assert(m_regs[src].getReg(1) != InvalidReg);
          args.reg(m_regs[src].getReg(1));
        }
        args[j++].setDstReg(m_regs[dst].getReg(1));
      }
    }
    assert(args.numStackArgs() == 0 &&
           "Jmp_ doesn't support passing arguments on the stack yet.");
    shuffleArgs(m_as, args);
  }
  if (!m_state.noTerminalJmp_) {
    emitFwdJmp(inst->getTaken());
  }
}

void CodeGenerator::cgJmpIndirect(IRInstruction* inst) {
  m_as.jmp(m_regs[inst->getSrc(0)].getReg());
}

void CodeGenerator::cgCheckInit(IRInstruction* inst) {
  Block* label = inst->getTaken();
  assert(label);
  SSATmp* src = inst->getSrc(0);

  if (src->type().isInit()) return;

  auto typeReg = m_regs[src].getReg(1);
  assert(typeReg != InvalidReg);

  static_assert(KindOfUninit == 0, "cgCheckInit assumes KindOfUninit == 0");
  m_as.testb (rbyte(typeReg), rbyte(typeReg));
  emitFwdJcc(CC_Z, label);
}

void CodeGenerator::cgCheckInitMem(IRInstruction* inst) {
  Block* label = inst->getTaken();
  assert(label);
  SSATmp* base = inst->getSrc(0);
  int64_t offset = inst->getSrc(1)->getValInt();
  Type t = base->type().deref();
  if (t.isInit()) return;
  auto basereg = m_regs[base].getReg();
  emitCmpTVType(m_as, KindOfUninit, basereg[offset + TVOFF(m_type)]);
  emitFwdJcc(CC_Z, label);
}

void CodeGenerator::cgExitWhenSurprised(IRInstruction* inst) {
  Block* label = inst->getTaken();
  m_tx64->emitTestSurpriseFlags(m_as);
  emitFwdJcc(CC_NZ, label);
}

void CodeGenerator::cgExitOnVarEnv(IRInstruction* inst) {
  SSATmp* fp    = inst->getSrc(0);
  Block*  label = inst->getTaken();

  assert(!(fp->isConst()));

  auto fpReg = m_regs[fp].getReg();
  m_as.    cmpq   (0, fpReg[AROFF(m_varEnv)]);
  emitFwdJcc(CC_NE, label);
}

void CodeGenerator::cgReleaseVVOrExit(IRInstruction* inst) {
  auto* const label = inst->getTaken();
  auto const rFp = m_regs[inst->getSrc(0)].getReg();

  m_as.    cmpq   (0, rFp[AROFF(m_varEnv)]);
  unlikelyIfBlock(CC_NZ, [&] (Asm& a) {
    a.    testl  (ActRec::kExtraArgsBit, rFp[AROFF(m_varEnv)]);
    emitFwdJcc(a, CC_Z, label);
    cgCallHelper(
      a,
      TCA(static_cast<void (*)(ActRec*)>(ExtraArgs::deallocate)),
      nullptr,
      kSyncPoint,
      ArgGroup(m_regs).reg(rFp),
      DestType::None
    );
  });
}

void CodeGenerator::cgBoxPtr(IRInstruction* inst) {
  SSATmp* dst  = inst->getDst();
  SSATmp* addr = inst->getSrc(0);
  auto base    = m_regs[addr].getReg();
  auto dstReg  = m_regs[dst].getReg();
  emitMovRegReg(m_as, base, dstReg);
  auto const cc = emitTypeTest(Type::BoxedCell, base[TVOFF(m_type)], true);
  ifThen(m_as, cc, [&] {
    cgCallHelper(m_as, (TCA)tvBox, dstReg, kNoSyncPoint,
                 ArgGroup(m_regs).ssa(addr));
  });
}

void CodeGenerator::cgDefCns(IRInstruction* inst) {
  UNUSED SSATmp* dst     = inst->getDst();
  UNUSED SSATmp* cnsName = inst->getSrc(0);
  UNUSED SSATmp* val     = inst->getSrc(1);
  using namespace TargetCache;
  CG_PUNT(DefCns);
}

// TODO: Kill this #2031980
static StringData* concat_value(TypedValue tv1, TypedValue tv2) {
  return concat_tv(tv1.m_type, tv1.m_data.num, tv2.m_type, tv2.m_data.num);
}

void CodeGenerator::cgConcat(IRInstruction* inst) {
  SSATmp* dst   = inst->getDst();
  SSATmp* tl    = inst->getSrc(0);
  SSATmp* tr    = inst->getSrc(1);

  Type lType = tl->type();
  Type rType = tr->type();
  // We have specialized helpers for concatenating two strings, a
  // string and an int, and an int and a string.
  void* fptr = nullptr;
  if (lType.isString() && rType.isString()) {
    fptr = (void*)concat_ss;
  } else if (lType.isString() && rType == Type::Int) {
    fptr = (void*)concat_si;
  } else if (lType == Type::Int && rType.isString()) {
    fptr = (void*)concat_is;
  }
  if (fptr) {
    cgCallHelper(m_as, (TCA)fptr, dst, kNoSyncPoint,
                 ArgGroup(m_regs).ssa(tl).ssa(tr));
  } else {
    if (lType.subtypeOf(Type::Obj) || lType.needsReg() ||
        rType.subtypeOf(Type::Obj) || rType.needsReg()) {
      CG_PUNT(cgConcat);
    }
    cgCallHelper(m_as, (TCA)concat_value, dst, kNoSyncPoint,
                 ArgGroup(m_regs).typedValue(tl).typedValue(tr));
  }
}

void CodeGenerator::cgInterpOne(IRInstruction* inst) {
  SSATmp* fp = inst->getSrc(0);
  SSATmp* sp = inst->getSrc(1);
  SSATmp* pcOffTmp  = inst->getSrc(2);
  SSATmp* spAdjustmentTmp = inst->getSrc(3);
  int64_t pcOff = pcOffTmp->getValInt();

  auto opc = *(getCurFunc()->unit()->at(pcOff));
  void* interpOneHelper = interpOneEntryPoints[opc];

  auto dstReg = InvalidReg;
  cgCallHelper(m_as, (TCA)interpOneHelper, dstReg, kSyncPoint,
               ArgGroup(m_regs).ssa(fp).ssa(sp).imm(pcOff));

  auto newSpReg = m_regs[inst->getDst()].getReg();
  assert(newSpReg == m_regs[sp].getReg());

  int64_t spAdjustBytes = cellsToBytes(spAdjustmentTmp->getValInt());
  if (spAdjustBytes != 0) {
    m_as.addq(spAdjustBytes, newSpReg);
  }
}

void CodeGenerator::cgInterpOneCF(IRInstruction* inst) {
  SSATmp* fp = inst->getSrc(0);
  SSATmp* sp = inst->getSrc(1);
  int64_t pcOff = inst->getSrc(2)->getValInt();

  auto opc = *(getCurFunc()->unit()->at(pcOff));
  void* interpOneHelper = interpOneEntryPoints[opc];

  auto dstReg = InvalidReg;
  cgCallHelper(m_as, (TCA)interpOneHelper, dstReg, kSyncPoint,
               ArgGroup(m_regs).ssa(fp).ssa(sp).imm(pcOff));

  // The interpOne method returns a pointer to the current ExecutionContext
  // in rax.  Use it read the 'm_fp' and 'm_stack.m_top' fields into the
  // rVmFp and rVmSp registers.
  m_as.loadq(rax[offsetof(VMExecutionContext, m_fp)], rVmFp);
  m_as.loadq(rax[offsetof(VMExecutionContext, m_stack) +
                 Stack::topOfStackOffset()], rVmSp);

  m_tx64->emitServiceReq(SRFlags::EmitInA, REQ_RESUME, 0ull);
}

void CodeGenerator::cgDefFunc(IRInstruction* inst) {
  SSATmp* dst   = inst->getDst();
  SSATmp* func  = inst->getSrc(0);
  cgCallHelper(m_as, (TCA)defFuncHelper, dst, kSyncPoint,
               ArgGroup(m_regs).ssa(func), DestType::None);
}

void CodeGenerator::cgFillContThis(IRInstruction* inst) {
  SSATmp* cont = inst->getSrc(0);
  auto baseReg = m_regs[inst->getSrc(1)].getReg();
  int64_t offset = inst->getSrc(2)->getValInt();
  auto scratch = m_rScratch;
  auto contReg = m_regs[cont].getReg();

  m_as.loadq(contReg[CONTOFF(m_obj)], scratch);
  m_as.testq(scratch, scratch);
  ifThen(m_as, CC_NZ, [&] {
    m_as.addl(1, scratch[FAST_REFCOUNT_OFFSET]);
    m_as.storeq(scratch, baseReg[offset + TVOFF(m_data)]);
    emitStoreTVType(m_as, KindOfObject, baseReg[offset + TVOFF(m_type)]);
  });
}

void CodeGenerator::cgContEnter(IRInstruction* inst) {
  auto contAR = inst->getSrc(0);
  auto addr = inst->getSrc(1);
  auto returnOff = inst->getSrc(2);
  auto curFp = m_regs[inst->getSrc(3)].getReg();
  auto contARReg = m_regs[contAR].getReg();

  m_as.  storel (returnOff->getValInt(), contARReg[AROFF(m_soff)]);
  m_as.  storeq (curFp, contARReg[AROFF(m_savedRbp)]);
  m_as.  movq   (contARReg, rStashedAR);

  m_as.  call   (m_regs[addr].getReg());
}

void CodeGenerator::emitContVarEnvHelperCall(SSATmp* fp, TCA helper) {
  auto scratch = m_rScratch;

  m_as.  loadq (m_regs[fp].getReg()[AROFF(m_varEnv)], scratch);
  m_as.  testq (scratch, scratch);
  unlikelyIfBlock(CC_NZ, [&] (Asm& a) {
    cgCallHelper(a, helper, InvalidReg, kNoSyncPoint,
                 ArgGroup(m_regs).ssa(fp));
  });
}

void CodeGenerator::cgUnlinkContVarEnv(IRInstruction* inst) {
  emitContVarEnvHelperCall(
    inst->getSrc(0),
    (TCA)VMExecutionContext::packContVarEnvLinkage);
}

void CodeGenerator::cgLinkContVarEnv(IRInstruction* inst) {
  emitContVarEnvHelperCall(
    inst->getSrc(0),
    (TCA)VMExecutionContext::unpackContVarEnvLinkage);
}

void CodeGenerator::cgContRaiseCheck(IRInstruction* inst) {
  SSATmp* cont = inst->getSrc(0);
  m_as.test_imm32_disp_reg32(0x1, CONTOFF(m_should_throw),
                             m_regs[cont].getReg());
  emitFwdJcc(CC_NZ, inst->getTaken());
}

void CodeGenerator::cgContPreNext(IRInstruction* inst) {
  auto contReg = m_regs[inst->getSrc(0)].getReg();

  const Offset doneOffset = CONTOFF(m_done);
  static_assert((doneOffset + 1) == CONTOFF(m_running),
                "m_done should immediately precede m_running");
  // Check m_done and m_running at the same time
  m_as.test_imm32_disp_reg32(0x0101, doneOffset, contReg);
  emitFwdJcc(CC_NZ, inst->getTaken());

  // ++m_index
  m_as.add_imm64_disp_reg64(0x1, CONTOFF(m_index), contReg);
  // m_running = true
  m_as.store_imm8_disp_reg(0x1, CONTOFF(m_running), contReg);
}

void CodeGenerator::cgContStartedCheck(IRInstruction* inst) {
  m_as.cmp_imm64_disp_reg64(0, CONTOFF(m_index),
                            m_regs[inst->getSrc(0)].getReg());
  emitFwdJcc(CC_L, inst->getTaken());
}

void CodeGenerator::cgIterNextK(IRInstruction* inst) {
  cgIterNextCommon(inst, true);
}

void CodeGenerator::cgIterNext(IRInstruction* inst) {
  cgIterNextCommon(inst, false);
}

void CodeGenerator::cgIterNextCommon(IRInstruction* inst, bool isNextK) {
  PhysReg fpReg = m_regs[inst->getSrc(0)].getReg();
  ArgGroup args(m_regs);
  args.addr(fpReg, getIterOffset(inst->getSrc(1)))
      .addr(fpReg, getLocalOffset(inst->getSrc(2)));
  if (isNextK) {
    args.addr(fpReg, getLocalOffset(inst->getSrc(3)));
  }
  TCA helperAddr = isNextK ? (TCA)iter_next_key : (TCA)iter_next;
  cgCallHelper(m_as, helperAddr, inst->getDst(), kSyncPoint, args);
}

void CodeGenerator::cgIterInit(IRInstruction* inst) {
  cgIterInitCommon(inst, false);
}

void iterFreeHelper(Iter* iter) {
  iter->free();
}

void CodeGenerator::cgIterFree(IRInstruction* inst) {
  PhysReg fpReg = m_regs[inst->getSrc(0)].getReg();
  int64_t  offset = getIterOffset(inst->getSrc(1));
  cgCallHelper(m_as, (TCA)iterFreeHelper, InvalidReg, kSyncPoint,
               ArgGroup(m_regs).addr(fpReg, offset));
}

void CodeGenerator::cgIterInitK(IRInstruction* inst) {
  cgIterInitCommon(inst, true);
}

void CodeGenerator::cgIterInitCommon(IRInstruction* inst, bool isInitK) {
  PhysReg        fpReg = m_regs[inst->getSrc(1)].getReg();
  int64_t     iterOffset = getIterOffset(inst->getSrc(2));
  int64_t valLocalOffset = getLocalOffset(inst->getSrc(3));
  SSATmp*          src = inst->getSrc(0);
  ArgGroup args(m_regs);
  args.addr(fpReg, iterOffset).ssa(src);
  if (src->isArray()) {
    args.addr(fpReg, valLocalOffset);
    if (isInitK) {
      args.addr(fpReg, getLocalOffset(inst->getSrc(4)));
    }
    TCA helperAddr = isInitK ? (TCA)new_iter_array_key : (TCA)new_iter_array;
    cgCallHelper(m_as, helperAddr, inst->getDst(), kSyncPoint, args);
  } else {
    assert(src->type() == Type::Obj);
    args.imm(uintptr_t(getCurClass())).addr(fpReg, valLocalOffset);
    if (isInitK) {
      args.addr(fpReg, getLocalOffset(inst->getSrc(4)));
    } else {
      args.imm(0);
    }
    // new_iter_object decrefs its src object if it propagates an
    // exception out, so we use kSyncPointAdjustOne, which adjusts the
    // stack pointer by 1 stack element on an unwind, skipping over
    // the src object.
    cgCallHelper(m_as, (TCA)new_iter_object, inst->getDst(),
                 kSyncPointAdjustOne, args);
  }
}

void CodeGenerator::cgIncStat(IRInstruction *inst) {
  Stats::emitInc(m_as,
                 Stats::StatCounter(inst->getSrc(0)->getValInt()),
                 inst->getSrc(1)->getValInt(),
                 Transl::CC_None,
                 inst->getSrc(2)->getValBool());
}

void CodeGenerator::cgIncTransCounter(IRInstruction* inst) {
  m_tx64->emitTransCounterInc(m_as);
}

void CodeGenerator::cgDbgAssertRefCount(IRInstruction* inst) {
  emitAssertRefCount(m_as, m_regs[inst->getSrc(0)].getReg());
}

void traceCallback(ActRec* fp, Cell* sp, int64_t pcOff, void* rip) {
  if (HPHP::Trace::moduleEnabled(HPHP::Trace::hhirTracelets)) {
    FTRACE(0, "{} {} {}\n", fp->m_func->fullName()->data(), pcOff, rip);
  }
  checkFrame(fp, sp, /*checkLocals*/true);
}

void CodeGenerator::cgDbgAssertType(IRInstruction* inst) {
  ConditionCode cc = emitTypeTest(inst->getTypeParam(),
                                  m_regs[inst->getSrc(0)].getReg(1), true);
  ifThen(m_as, cc, [&] { m_as.ud2(); });
}

void CodeGenerator::cgVerifyParamCls(IRInstruction* inst) {
  SSATmp* objClass = inst->getSrc(0);
  assert(!objClass->isConst());
  auto objClassReg = m_regs[objClass].getReg();
  SSATmp* constraint = inst->getSrc(1);

  if (constraint->isConst()) {
    m_as.  cmpq(constraint->getValClass(), objClassReg);
  } else {
    m_as.  cmpq(m_regs[constraint].getReg(), objClassReg);
  }

  // The native call for this instruction is the slow path that does
  // proper subtype checking. The comparison above is just to
  // short-circuit the overhead when the Classes are an exact match.
  ifThen(m_as, CC_NE, [&]{ cgCallNative(inst); });
}

void CodeGenerator::emitTraceCall(CodeGenerator::Asm& as, int64_t pcOff,
                                  Transl::TranslatorX64* tx64) {
  // call to a trace function
  as.mov_imm64_reg((int64_t)as.code.frontier, reg::rcx);
  as.mov_reg64_reg64(rVmFp, reg::rdi);
  as.mov_reg64_reg64(rVmSp, reg::rsi);
  as.mov_imm64_reg(pcOff, reg::rdx);
  // do the call; may use a trampoline
  tx64->emitCall(as, (TCA)traceCallback);
}

static void patchJumps(Asm& as, CodegenState& state, Block* block) {
  void* list = state.patches[block];
  Address labelAddr = as.code.frontier;
  while (list) {
    int32_t* toPatch = (int32_t*)list;
    int32_t diffToNext = *toPatch;
    ssize_t diff = labelAddr - ((Address)list + sizeof(int32_t));
    *toPatch = safe_cast<int32_t>(diff); // patch the jump address
    if (diffToNext == 0) break;
    void* next = (TCA)list - diffToNext;
    list = next;
  }
}

void CodeGenerator::cgBlock(Block* block, vector<TransBCMapping>* bcMap) {
  for (IRInstruction& instr : *block) {
    IRInstruction* inst = &instr;
    if (inst->op() == Marker) {
      m_state.lastMarker = inst->getExtra<Marker>();
      if (m_tx64 && m_tx64->isTransDBEnabled() && bcMap) {
        bcMap->push_back((TransBCMapping){Offset(m_state.lastMarker->bcOff),
              m_as.code.frontier,
              m_astubs.code.frontier});
      }
    }
    m_curInst = inst;
    auto nuller = folly::makeGuard([&]{ m_curInst = nullptr; });
    auto* addr = cgInst(inst);
    if (m_state.asmInfo && addr) {
      m_state.asmInfo->instRanges[inst] = TcaRange(addr, m_as.code.frontier);
      m_state.asmInfo->asmRanges[block] =
        TcaRange(m_state.asmInfo->asmRanges[block].start(), m_as.code.frontier);
    }
  }
}

void cgTrace(Trace* trace, Asm& amain, Asm& astubs, Transl::TranslatorX64* tx64,
             vector<TransBCMapping>* bcMap, CodegenState& state) {
  state.lastMarker = nullptr;
  if (RuntimeOption::EvalHHIRGenerateAsserts && trace->isMain()) {
    CodeGenerator::emitTraceCall(amain, trace->getBcOff(), tx64);
  }
  auto chooseAs = [&](Block* b) {
    return b->getHint() != Block::Unlikely ? &amain : &astubs;
  };
  auto& blocks = trace->getBlocks();
  for (auto it = blocks.begin(), end = blocks.end(); it != end;) {
    Block* block = *it; ++it;
    Asm* as = chooseAs(block);
    TCA asmStart = as->code.frontier;
    TCA astubsStart = astubs.code.frontier;
    patchJumps(*as, state, block);

    // Grab the next block that will go into this assembler
    Block* nextThisAs = nullptr;
    for (auto next = it; next != end; ++next) {
      if (chooseAs(*next) == as) {
        nextThisAs = *next;
        break;
      }
    }

    // If the block ends with a Jmp_ to the next block for this
    // assembler, it doesn't need to actually emit a jmp.
    IRInstruction* last = block->back();
    state.noTerminalJmp_ =
      last->op() == Jmp_ && last->getTaken() == nextThisAs;

    CodeGenerator cg(trace, *as, astubs, tx64, state);
    if (state.asmInfo) {
      state.asmInfo->asmRanges[block] = TcaRange(asmStart, as->code.frontier);
    }
    cg.cgBlock(block, bcMap);
    Block* next = block->getNext();
    if (next && next != nextThisAs) {
      // if there's a fallthrough block and it's not the next thing
      // going into this assembler, then emit a jump to it.
      CodeGenerator::emitFwdJmp(*as, next, state);
    }
    if (state.asmInfo) {
      state.asmInfo->asmRanges[block] = TcaRange(asmStart, as->code.frontier);
      if (as != &astubs) {
        state.asmInfo->astubRanges[block] = TcaRange(astubsStart,
                                                     astubs.code.frontier);
      }
    }
  }
}

void CodeGenerator::print() const {
  JIT::print(std::cout, m_curTrace, &m_state.regs, m_state.lifetime,
             m_state.asmInfo);
}

/*
 * Compute and save registers that are live *across* each inst, not including
 * registers whose lifetimes end at inst, nor registers defined by inst.
 */
LiveRegs computeLiveRegs(const IRFactory* factory, const RegAllocInfo& regs,
                         Block* start_block) {
  StateVector<Block, RegSet> liveMap(factory, RegSet());
  LiveRegs live_regs(factory, RegSet());
  postorderWalk(
    [&](Block* block) {
      RegSet& live = liveMap[block];
      if (Block* taken = block->getTaken()) live = liveMap[taken];
      if (Block* next = block->getNext()) live |= liveMap[next];
      for (auto it = block->end(); it != block->begin(); ) {
        IRInstruction& inst = *--it;
        for (const SSATmp& dst : inst.getDsts()) {
          live -= regs[dst].getRegs();
        }
        live_regs[inst] = live;
        for (SSATmp* src : inst.getSrcs()) {
          live |= regs[src].getRegs();
        }
      }
    },
    factory->numBlocks(),
    start_block
  );
  return live_regs;
}

// select instructions for the trace and its exits
void genCodeForTrace(Trace* trace,
                     CodeGenerator::Asm& as,
                     CodeGenerator::Asm& astubs,
                     IRFactory* irFactory,
                     vector<TransBCMapping>* bcMap,
                     Transl::TranslatorX64* tx64,
                     const RegAllocInfo& regs,
                     const LifetimeInfo* lifetime,
                     AsmInfo* asmInfo) {
  assert(trace->isMain());
  LiveRegs live_regs = computeLiveRegs(irFactory, regs, trace->front());
  CodegenState state(irFactory, regs, live_regs, lifetime, asmInfo);
  cgTrace(trace, as, astubs, tx64, bcMap, state);
  for (Trace* exit : trace->getExitTraces()) {
    cgTrace(exit, astubs, astubs, tx64, nullptr, state);
  }
}

ALWAYS_INLINE
TypedValue& getDefaultIfNullCell(TypedValue* tv, TypedValue& def) {
  if (UNLIKELY(nullptr == tv)) {
    // refcount is already correct since def was never decrefed
    return def;
  }
  tvRefcountedDecRef(&def);
  TypedValue* ret = tvToCell(tv);
  tvRefcountedIncRef(ret);
  return *ret;
}

HOT_FUNC_VM
TypedValue arrayIdxS(ArrayData* a, StringData* key, TypedValue def) {
  return getDefaultIfNullCell(a->nvGet(key), def);
}

HOT_FUNC_VM
TypedValue arrayIdxSi(ArrayData* a, StringData* key, TypedValue def) {
  int64_t i;
  return UNLIKELY(key->isStrictlyInteger(i)) ?
         getDefaultIfNullCell(a->nvGet(i), def) :
         getDefaultIfNullCell(a->nvGet(key), def);
}

HOT_FUNC_VM
TypedValue arrayIdxI(ArrayData* a, int64_t key, TypedValue def) {
  return getDefaultIfNullCell(a->nvGet(key), def);
}

}}
