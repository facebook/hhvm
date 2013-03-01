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

#include "runtime/vm/translator/hopt/codegen.h"

#include <string.h>

#include "folly/ScopeGuard.h"
#include "util/trace.h"
#include "util/util.h"

#include "runtime/base/array/hphp_array.h"
#include "runtime/base/comparisons.h"
#include "runtime/base/complex_types.h"
#include "runtime/base/runtime_option.h"
#include "runtime/base/string_data.h"
#include "runtime/base/types.h"
#include "runtime/ext/ext_continuation.h"
#include "runtime/vm/bytecode.h"
#include "runtime/vm/runtime.h"
#include "runtime/vm/stats.h"
#include "runtime/vm/translator/targetcache.h"
#include "runtime/vm/translator/translator-inline.h"
#include "runtime/vm/translator/translator-x64.h"
#include "runtime/vm/translator/translator.h"
#include "runtime/vm/translator/types.h"
#include "runtime/vm/translator/x64-util.h"
#include "runtime/vm/translator/hopt/ir.h"
#include "runtime/vm/translator/hopt/linearscan.h"
#include "runtime/vm/translator/hopt/nativecalls.h"

using HPHP::DataType;
using HPHP::TypedValue;
using HPHP::VM::Transl::TCA;
using namespace HPHP::VM::Transl::TargetCache;

// emitDispDeref --
// emitDeref --
//
//   Helpers for common cell operations.
//
//   Dereference the cell whose address lives in src into dest.
/*static */void
emitDispDeref(X64Assembler &a, PhysReg src, int disp, PhysReg dest) {
  a.    load_reg64_disp_reg64(src, disp + TVOFF(m_data), dest);
}

/*static*/ void
emitDeref(X64Assembler &a, PhysReg src, PhysReg dest) {
  emitDispDeref(a, src, 0, dest);
}

/* static */ void
emitDerefIfVariant(X64Assembler &a, PhysReg reg) {
  a.cmp_imm32_disp_reg32(HPHP::KindOfRef, TVOFF(m_type), reg);
  a.cload_reg64_disp_reg64(CC_Z, reg, 0, reg);
}

namespace HPHP {
namespace VM {
namespace JIT {

namespace {

//////////////////////////////////////////////////////////////////////

using namespace Util;
using namespace Transl::reg;

static const HPHP::Trace::Module TRACEMOD = HPHP::Trace::hhir;

using Transl::rVmSp;
using Transl::rVmFp;

const int64_t kTypeShiftBits = sizeof(int32_t) * CHAR_BIT;
uint64_t toDataTypeForCall(Type type) {
  return (uint64_t)type.toDataType() << (sizeof(int32_t) * CHAR_BIT);
}

int64_t spillSlotsToSize(int n) {
  return n * sizeof(int64_t);
}

void cgPunt(const char* file, int line, const char* func) {
  if (RuntimeOption::EvalDumpIR) {
    HPHP::Trace::trace("--------- CG_PUNT %s %d %s\n", file, line, func);
  }
  throw FailedCodeGen(file, line, func);
}

#define CG_PUNT(instr) do {                     \
  if (tx64) {                                   \
    cgPunt( __FILE__, __LINE__, #instr);        \
  }                                             \
} while(0)

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
    if (cycles[i].length == 2) {
      int v = cycles[i].node;
      int w = moves[v];
      howTo.push_back(MoveInfo(MoveInfo::Xchg, w, v));
    } else if (cycles[i].length == 3) {
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

ConditionCode cmpOpToCC(Opcode opc) {
  switch (opc) {
  case OpGt:                return CC_G;
  case OpGte:               return CC_GE;
  case OpLt:                return CC_L;
  case OpLte:               return CC_LE;
  case OpEq:                return CC_E;
  case OpNeq:               return CC_NE;
  case OpSame:              return CC_E;
  case OpNSame:             return CC_NE;
  case InstanceOf:          return CC_NZ;
  case NInstanceOf:         return CC_Z;
  case InstanceOfBitmask:   return CC_NZ;
  case NInstanceOfBitmask:  return CC_Z;
  case IsType:              return CC_NZ;
  case IsNType:             return CC_Z;
  default:                  always_assert(0);
  }
}

const char* getContextName(Class* ctx) {
  return ctx ? ctx->name()->data() : ":anonymous:";
}

} // unnamed namespace

//////////////////////////////////////////////////////////////////////

ArgDesc::ArgDesc(SSATmp* tmp, bool val) : m_imm(-1), m_zeroExtend(false) {
  if (tmp->getType() == Type::None) {
    assert(val);
    m_kind = None;
    return;
  }
  if (tmp->getInstruction()->getOpcode() == DefConst) {
    m_srcReg = InvalidReg;
    if (val) {
      m_imm = tmp->getValBits();
    } else {
      m_imm = toDataTypeForCall(tmp->getType());
    }
    m_kind = Imm;
    return;
  }
  if (tmp->getType().isNull()) {
    m_srcReg = InvalidReg;
    if (val) {
      m_imm = 0;
    } else {
      m_imm = toDataTypeForCall(tmp->getType());
    }
    m_kind = Imm;
    return;
  }
  if (val || tmp->numNeededRegs() > 1) {
    auto reg = tmp->getReg(val ? 0 : 1);
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
  m_imm = toDataTypeForCall(tmp->getType());
  m_kind = Imm;
}

const Func* CodeGenerator::getCurFunc() const {
  return m_state.lastMarker ? m_state.lastMarker->func :
         m_curTrace->front()->getFunc();
}

Address CodeGenerator::cgInst(IRInstruction* inst) {
  Opcode opc = inst->getOpcode();
  auto const start = m_as.code.frontier;
  switch (opc) {
#define O(name, dsts, srcs, flags)                                \
  case name: cg ## name (inst);                                   \
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

NOOP_OPCODE(DefConst)
NOOP_OPCODE(DefFP)
NOOP_OPCODE(DefSP)
NOOP_OPCODE(Marker)
NOOP_OPCODE(AssertLoc)
NOOP_OPCODE(DefActRec)
NOOP_OPCODE(AssertStk)
NOOP_OPCODE(Nop)
NOOP_OPCODE(DefLabel)

CALL_OPCODE(AddElemStrKey)
CALL_OPCODE(AddElemIntKey)
CALL_OPCODE(AddNewElem)
CALL_OPCODE(ArrayAdd)
CALL_OPCODE(Box)
CALL_OPCODE(CreateCont)
CALL_OPCODE(FillContLocals)
CALL_OPCODE(LdObjMethod)
CALL_OPCODE(NewArray)
CALL_OPCODE(NewTuple)
CALL_OPCODE(PrintStr)
CALL_OPCODE(PrintInt)
CALL_OPCODE(PrintBool)
CALL_OPCODE(RaiseUninitWarning)
CALL_OPCODE(PropX)
CALL_OPCODE(CGetProp)
CALL_OPCODE(SetProp)
CALL_OPCODE(CGetElem)
CALL_OPCODE(SetElem)
CALL_OPCODE(BaseG)
CALL_OPCODE(DbgAssertPtr)
CALL_OPCODE(LdSwitchDblIndex);
CALL_OPCODE(LdSwitchStrIndex);
CALL_OPCODE(LdSwitchObjIndex);

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

// Patch with service request EMIT_BIND_JMP
Address CodeGenerator::emitSmashableFwdJmp(Block* target, SSATmp* toSmash) {
  Address start = m_as.code.frontier;
  if (toSmash) {
    m_tx64->prepareForSmash(m_as, TranslatorX64::kJmpLen);
    Address tca = emitFwdJmp(target);
    toSmash->setTCA(tca);
    //assert(false);  // TODO(#2012072): this path is supposed to be unused
  } else {
    emitFwdJmp(target);
  }
  return start;
}

// Patch with service request REQ_BIND_JMPCC_FIRST/SECOND
Address CodeGenerator::emitSmashableFwdJccAtEnd(ConditionCode cc, Block* target,
                                                SSATmp* toSmash) {
  Address start = m_as.code.frontier;
  if (toSmash) {
    m_tx64->prepareForSmash(m_as, TranslatorX64::kJmpLen +
                                  TranslatorX64::kJmpccLen);
    Address tcaJcc = emitFwdJcc(cc, target);
    emitFwdJmp(target);
    toSmash->setTCA(tcaJcc);
  } else {
    emitFwdJcc(cc, target);
  }
  return start;
}

void CodeGenerator::emitJccDirectExit(IRInstruction* inst,
                                      ConditionCode cc) {
  if (cc == CC_None) return;
  SSATmp* toSmash = inst->getTCA() == kIRDirectJccJmpActive
    ? inst->getDst() : nullptr;
  emitSmashableFwdJccAtEnd(cc, inst->getTaken(), toSmash);
}

// Patch with service request REQ_BIND_JCC
Address CodeGenerator::emitSmashableFwdJcc(ConditionCode cc, Block* target,
                                           SSATmp* toSmash) {
  Address start = m_as.code.frontier;
  assert(toSmash);

  m_tx64->prepareForSmash(m_as, TranslatorX64::kJmpccLen);
  Address tcaJcc = emitFwdJcc(cc, target);
  toSmash->setTCA(tcaJcc);
  return start;
}

void emitLoadImm(CodeGenerator::Asm& as, int64_t val, PhysReg dstReg) {
  as.emitImmReg(val, dstReg);
}

void emitMovRegReg(CodeGenerator::Asm& as, PhysReg srcReg, PhysReg dstReg) {
  if (srcReg != dstReg) as.movq(srcReg, dstReg);
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

static void zeroExtendIfBool(X64Assembler& as, const SSATmp* src) {
  if (src->isA(Type::Bool)) {
    auto reg = src->getReg();
    if (reg != InvalidReg) {
      // zero-extend the bool from a byte to a quad
      // note: movzbl actually extends the value to 64 bits.
      as.movzbl(rbyte(reg), r32(reg));
    }
  }
}

static void prepBinaryXmmOp(X64Assembler& a, const SSATmp* l, const SSATmp* r) {
  auto intoXmm = [&](const SSATmp* ssa, RegXMM xmm) {
    RegNumber src(ssa->getReg());
    if (ssa->getReg() == InvalidReg) {
      src = rScratch;
      assert(ssa->isConst());
      a.mov_imm64_reg(ssa->getValBits(), rScratch);
    }
    if (ssa->isA(Type::Int | Type::Bool)) {
      // Expand non-const bools to 64-bit.
      // Consts are already moved into src as 64-bit values above.
      if (!ssa->isConst()) zeroExtendIfBool(a, ssa);
      // cvtsi2sd doesn't modify the high bits of its target, which can
      // cause false dependencies to prevent register renaming from kicking
      // in. Break the dependency chain by zeroing out the destination reg.
      a.  pxor_xmm_xmm(xmm, xmm);
      a.  cvtsi2sd_reg64_xmm(src, xmm);
    } else {
      a.  mov_reg64_xmm(src, xmm);
    }
  };
  intoXmm(l, xmm0);
  intoXmm(r, xmm1);
}

static void doubleCmp(X64Assembler& a, RegXMM xmm0, RegXMM xmm1) {
  a.    ucomisd_xmm_xmm(xmm0, xmm1);
  Label notPF;
  a.    jnp8(notPF);
  // PF means the doubles were unordered. We treat this as !equal, so
  // clear ZF.
  a.    or_imm32_reg64(1, rScratch);
asm_label(a, notPF);
}

void CodeGenerator::cgJcc(IRInstruction* inst) {
  SSATmp* src1  = inst->getSrc(0);
  SSATmp* src2  = inst->getSrc(1);
  Opcode opc = inst->getOpcode();
  ConditionCode cc = cmpOpToCC(queryJmpToQueryOp(opc));
  Type src1Type = src1->getType();
  Type src2Type = src2->getType();

  // can't generate CMP instructions correctly for anything that isn't
  // a bool or a numeric, and we can't mix bool/numerics because
  // -1 == true in PHP, but not in HHIR binary representation
  if (!((src1Type == Type::Int && src2Type == Type::Int) ||
        ((src1Type == Type::Int || src1Type == Type::Dbl) &&
         (src2Type == Type::Int || src2Type == Type::Dbl)) ||
        (src1Type == Type::Bool && src2Type == Type::Bool) ||
        (src1Type == Type::Cls && src2Type == Type::Cls))) {
    CG_PUNT(cgJcc);
  }
  if (src1Type == Type::Dbl || src2Type == Type::Dbl) {
    prepBinaryXmmOp(m_as, src1, src2);
    doubleCmp(m_as, xmm0, xmm1);
  } else {
    if (src1Type == Type::Cls && src2Type == Type::Cls) {
      assert(opc == JmpSame || opc == JmpNSame);
    }
    auto srcReg1 = src1->getReg();
    auto srcReg2 = src2->getReg();

    // Note: when both src1 and src2 are constants, we should transform the
    // branch into an unconditional jump earlier in the IR.
    if (src1->isConst()) {
      // TODO: use compare with immediate or make sure simplifier
      // canonicalizes this so that constant is src2
      srcReg1 = rScratch;
      m_as.mov_imm64_reg(src1->getValRawInt(), srcReg1);
    }
    if (src2->isConst()) {
      m_as.cmp_imm64_reg64(src2->getValRawInt(), srcReg1);
    } else {
      // Note the reverse syntax in the assembler.
      // This cmp will compute srcReg1 - srcReg2
      if (src1Type == Type::Bool) {
        m_as.    cmpb (Reg8(int(srcReg2)), Reg8(int(srcReg1)));
      } else {
        m_as.cmp_reg64_reg64(srcReg2, srcReg1);
      }
    }
  }

  emitJccDirectExit(inst, cc);
}

void CodeGenerator::cgJmpGt   (IRInstruction* inst) { cgJcc(inst); }
void CodeGenerator::cgJmpGte  (IRInstruction* inst) { cgJcc(inst); }
void CodeGenerator::cgJmpLt   (IRInstruction* inst) { cgJcc(inst); }
void CodeGenerator::cgJmpLte  (IRInstruction* inst) { cgJcc(inst); }
void CodeGenerator::cgJmpEq   (IRInstruction* inst) { cgJcc(inst); }
void CodeGenerator::cgJmpNeq  (IRInstruction* inst) { cgJcc(inst); }
void CodeGenerator::cgJmpSame (IRInstruction* inst) { cgJcc(inst); }
void CodeGenerator::cgJmpNSame(IRInstruction* inst) { cgJcc(inst); }

/**
 * Once the arg sources and dests are all assigned; emit moves and exchanges
 * to put all the args in desired registers.
 */
typedef Transl::X64Assembler Asm;
static void shuffleArgs(Asm& a, ArgGroup& args) {
  // First schedule arg moves
  for (size_t i = 0; i < args.size(); ++i) {
    // We don't support memory-to-register moves currently.
    assert(args[i].getKind() == ArgDesc::Reg ||
           args[i].getKind() == ArgDesc::TypeReg ||
           args[i].getKind() == ArgDesc::Imm ||
           args[i].getKind() == ArgDesc::Addr ||
           args[i].getKind() == ArgDesc::None);
  }
  // Handle register-to-register moves.
  int moves[kNumX64Regs];
  ArgDesc* argDescs[kNumX64Regs];
  memset(moves, -1, sizeof moves);
  memset(argDescs, 0, sizeof argDescs);
  for (size_t i = 0; i < args.size(); ++i) {
    auto kind = args[i].getKind();
    if (!(kind == ArgDesc::Reg  ||
          kind == ArgDesc::Addr ||
          kind == ArgDesc::TypeReg)) {
      continue;
    }
    auto dstReg = args[i].getDstReg();
    auto srcReg = args[i].getSrcReg();
    if (dstReg == srcReg) {
      // Ignore register-to-register moves whose src and dst registers
      // are the same. But emit the code for load-effective-address
      // operations whose src and dst registers are the same as
      // doRegMoves won't handle those.
      if (kind == ArgDesc::Addr) {
        // an lea whose src and dest regs are the same
        a.    lea    (srcReg[args[i].getImm().q()], dstReg);
      }
      continue;
    }
    moves[int(dstReg)] = int(srcReg);
    argDescs[int(dstReg)] = &args[i];
  }
  std::vector<MoveInfo> howTo;
  doRegMoves(moves, int(reg::rScratch), howTo);
  for (size_t i = 0; i < howTo.size(); ++i) {
    if (howTo[i].m_kind == MoveInfo::Move) {
      if (howTo[i].m_reg2 == reg::rScratch) {
        a.      movq   (howTo[i].m_reg1, howTo[i].m_reg2);
      } else {
        ArgDesc* argDesc = argDescs[int(howTo[i].m_reg2)];
        if (argDesc->getKind() == ArgDesc::Reg ||
            argDesc->getKind() == ArgDesc::TypeReg) {
          if (argDesc->isZeroExtend()) {
            a.    movzbl (rbyte(howTo[i].m_reg1), r32(howTo[i].m_reg2));
          } else {
            a.    movq   (howTo[i].m_reg1, howTo[i].m_reg2);
          }
        } else {
          assert(argDesc->getKind() == ArgDesc::Addr);
          a.    lea    (howTo[i].m_reg1[argDesc->getImm().q()],
                        howTo[i].m_reg2);
        }
      }
    } else {
      a.    xchgq  (howTo[i].m_reg1, howTo[i].m_reg2);
      ArgDesc* argDesc2 = argDescs[int(howTo[i].m_reg2)];
      if (argDesc2->getKind() == ArgDesc::Addr) {
        a.  addq   (argDesc2->getImm(), howTo[i].m_reg2);
      } else if (argDesc2->isZeroExtend()) {
        a.  movzbl (rbyte(howTo[i].m_reg2), r32(howTo[i].m_reg2));
      }
      ArgDesc* argDesc1 = argDescs[int(howTo[i].m_reg1)];
      if (argDesc1->getKind() == ArgDesc::Addr) {
        a.  addq   (argDesc1->getImm(), howTo[i].m_reg1);
      } else if (argDesc1->isZeroExtend()) {
        a.  movzbl (rbyte(howTo[i].m_reg1), r32(howTo[i].m_reg1));
      }
    }
  }
  // Handle const-to-register moves and type shifting
  for (size_t i = 0; i < args.size(); ++i) {
    if (args[i].getKind() == ArgDesc::Imm) {
      emitLoadImm(a, args[i].getImm().q(), args[i].getDstReg());
    } else if (args[i].getKind() == ArgDesc::TypeReg) {
      a.    shlq   (kTypeShiftBits, args[i].getDstReg());
    } else if (RuntimeOption::EvalHHIRGenerateAsserts &&
               args[i].getKind() == ArgDesc::None) {
      emitLoadImm(a, 0xbadbadbadbadbad, args[i].getDstReg());
    }
  }
}

void CodeGenerator::cgCallNative(IRInstruction* inst) {
  using namespace NativeCalls;
  Opcode opc = inst->getOpcode();
  always_assert(CallMap::hasInfo(opc));

  const CallInfo& info = CallMap::getInfo(opc);
  ArgGroup argGroup;
  for (auto const& arg : info.args) {
    SSATmp* src = inst->getSrc(arg.srcIdx);
    switch (arg.type) {
      case SSA:
        argGroup.ssa(src);
        break;
      case TV:
        argGroup.valueType(src);
        break;
      case VecKeyS:
        argGroup.vectorKeyS(src);
        break;
      case VecKeyIS:
        argGroup.vectorKeyIS(src);
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
  cgCallHelper(m_as,
               addr,
               info.dest ? inst->getDst() : nullptr,
               info.sync,
               argGroup);
}

void CodeGenerator::cgCallHelper(Asm& a,
                                 TCA addr,
                                 SSATmp* dst,
                                 SyncOptions sync,
                                 ArgGroup& args) {
  PhysReg dstReg0 = InvalidReg;
  PhysReg dstReg1 = InvalidReg;
  if (dst) {
    dstReg0 = dst->getReg(0);
    dstReg1 = dst->getReg(1);
  }
  return cgCallHelper(a, Transl::Call(addr), dstReg0, dstReg1, sync, args);
}

void CodeGenerator::cgCallHelper(Asm& a,
                                 TCA addr,
                                 PhysReg dstReg,
                                 SyncOptions sync,
                                 ArgGroup& args) {
  cgCallHelper(a, Transl::Call(addr), dstReg, InvalidReg, sync, args);
}

void CodeGenerator::cgCallHelper(Asm& a,
                                 const Transl::Call& call,
                                 PhysReg dstReg,
                                 SyncOptions sync,
                                 ArgGroup& args) {
  cgCallHelper(a, call, dstReg, InvalidReg, sync, args);
}

void CodeGenerator::cgCallHelper(Asm& a,
                                 const Transl::Call& call,
                                 PhysReg dstReg0,
                                 PhysReg dstReg1,
                                 SyncOptions sync,
                                 ArgGroup& args) {
  assert(int(args.size()) <= kNumRegisterArgs);
  assert(m_curInst->isNative());

  // Save the register that are live at the point after this IR instruction.
  // However, don't save the destination registers that will be overwritten
  // by this call.
  RegSet regsToSave = (m_curInst->getLiveOutRegs() & kCallerSaved).
                      remove(dstReg0).remove(dstReg1);
  PhysRegSaverParity<1> regSaver(a, regsToSave);

  // Assign registers to the arguments
  for (size_t i = 0; i < args.size(); i++) {
    args[i].setDstReg(argNumToRegName[i]);
  }

  shuffleArgs(a, args);

  // do the call; may use a trampoline
  m_tx64->emitCall(a, call);

  // HHIR:TODO this only does required part of TranslatorX64::recordCallImpl()
  // Better to have improved SKTRACE'n by calling recordStubCall,
  // recordReentrantCall, or recordReentrantStubCall as appropriate
  if (sync != kNoSyncPoint) {
    recordSyncPoint(a, sync);
  }

  // assume if dstReg1 is needed that the result is a TypedValue passed
  // by value.  In that case we need to rightshift the packed m_type
  // enum to occupy the low 32bits of the dest register.
  if (dstReg1 != InvalidReg) {
    // dstReg1 contains m_type and _count but we're expecting just the
    // type in the lower 32 bits, so shift the 2nd result register.
    a.      shrq (kTypeShiftBits, reg::rdx);
  }

  // safely copy the return value (rax:rdx) to (dstReg0:dstReg1)
  shuffle2(a, reg::rax, reg::rdx, dstReg0, dstReg1);
}

void CodeGenerator::cgMov(IRInstruction* inst) {
  assert(!inst->getSrc(0)->hasReg(1)); // TODO: t2082361: handle Gen & Cell
  SSATmp* dst   = inst->getDst();
  SSATmp* src   = inst->getSrc(0);
  auto dstReg = dst->getReg();
  if (!src->hasReg(0)) {
    assert(src->isConst());
    if (src->getType() == Type::Bool) {
      emitLoadImm(m_as, (int64_t)src->getValBool(), dstReg);
    } else {
      emitLoadImm(m_as, src->getValRawInt(), dstReg);
    }
  } else {
    auto srcReg = src->getReg();
    emitMovRegReg(m_as, srcReg, dstReg);
  }
}

template<class OpInstr, class Oper>
void CodeGenerator::cgUnaryIntOp(SSATmp* dst,
                                 SSATmp* src,
                                 OpInstr instr,
                                 Oper oper) {
  if (src->getType() != Type::Int && src->getType() != Type::Bool) {
    assert(0); CG_PUNT(UnaryIntOp);
  }
  auto dstReg = dst->getReg();
  auto srcReg = src->getReg();
  assert(dstReg != InvalidReg);
  auto& a = m_as;

  // Integer operations require 64-bit representations
  zeroExtendIfBool(a, src);

  if (srcReg != InvalidReg) {
    emitMovRegReg(a, srcReg, dstReg);
    (a.*instr)   (dstReg);
  } else {
    assert(src->isConst());
    emitLoadImm(a, oper(src->getValRawInt()), dstReg);
  }
}

void CodeGenerator::cgNotWork(SSATmp* dst, SSATmp* src) {
  cgUnaryIntOp(dst, src, &Asm::not, [](int64_t i) { return ~i; });
}

void CodeGenerator::cgNegateWork(SSATmp* dst, SSATmp* src) {
  cgUnaryIntOp(dst, src, &Asm::neg, [](int64_t i) { return -i; });
}

void CodeGenerator::cgNegate(IRInstruction* inst) {
  cgNegateWork(inst->getDst(), inst->getSrc(0));
}

template<class Oper>
void CodeGenerator::cgBinaryIntOp(IRInstruction* inst,
                                  void (Asm::*instrIR)(Immed, Reg64),
                                  void (Asm::*instrRR)(Reg64, Reg64),
                                  Oper oper,
                                  Commutativity commuteFlag) {
  const SSATmp* dst   = inst->getDst();
  const SSATmp* src1  = inst->getSrc(0);
  const SSATmp* src2  = inst->getSrc(1);
  if (!(src1->isA(Type::Bool) || src1->isA(Type::Int)) ||
      !(src2->isA(Type::Bool) || src2->isA(Type::Int))) {
    CG_PUNT(cgBinaryIntOp);
  }

  bool const commutative = commuteFlag == Commutative;
  auto const dstReg      = dst->getReg();
  auto const src1Reg     = src1->getReg();
  auto const src2Reg     = src2->getReg();
  auto& a                = m_as;

  // Extend booleans: integer operations require 64-bit
  // representations.
  zeroExtendIfBool(m_as, src1);
  zeroExtendIfBool(m_as, src2);

  // Two registers.
  if (src1Reg != InvalidReg && src2Reg != InvalidReg) {
    if (dstReg == src1Reg) {
      (a.*instrRR)  (src2Reg, dstReg);
    } else if (dstReg == src2Reg) {
      if (commutative) {
        (a.*instrRR)(src1Reg, dstReg);
      } else {
        a.  movq    (src1Reg, rScratch);
        (a.*instrRR)(src2Reg, rScratch);
        a.  movq    (rScratch, dstReg);
      }
    } else {
      emitMovRegReg(a, src1Reg, dstReg);
      (a.*instrRR) (src2Reg, dstReg);
    }
    return;
  }

  // Two immediates.
  if (src1Reg == InvalidReg && src2Reg == InvalidReg) {
    assert(src1->isConst() && src2->isConst());
    int64_t value = oper(src1->getValRawInt(),
                       src2->getValRawInt());
    emitLoadImm(a, value, dstReg);
    return;
  }

  // One register, and one immediate.

  if (commutative) {
    int64_t immed = (src2Reg == InvalidReg
                     ? src2 : src1)->getValRawInt();
    auto srcReg = src2Reg == InvalidReg ? src1Reg : src2Reg;
    if (srcReg == dstReg) {
      (a.*instrIR) (immed, dstReg);
    } else {
      emitLoadImm(a, immed, dstReg);
      (a.*instrRR) (srcReg, dstReg);
    }
    return;
  }

  // NonCommutative:

  if (src1Reg == InvalidReg) {
    if (dstReg == src2Reg) {
      emitLoadImm(a, src1->getValRawInt(), rScratch);
      (a.*instrRR) (src2Reg, rScratch);
      a.    movq   (rScratch, dstReg);
    } else {
      emitLoadImm(a, src1->getValRawInt(), dstReg);
      (a.*instrRR) (src2Reg, dstReg);
    }
    return;
  }

  assert(src2Reg == InvalidReg);
  emitMovRegReg(a, src1Reg, dstReg);
  (a.*instrIR) (src2->getValRawInt(), dstReg);
}

template<class Oper>
void CodeGenerator::cgBinaryOp(IRInstruction* inst,
                               void (Asm::*instrIR)(Immed, Reg64),
                               void (Asm::*instrRR)(Reg64, Reg64),
                               void (Asm::*fpInstr)(RegXMM, RegXMM),
                               Oper oper,
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
    prepBinaryXmmOp(m_as, src1, src2);
    (m_as.*fpInstr)(xmm1, xmm0);
    m_as.    mov_xmm_reg64(xmm0, dst->getReg());
    return;
  }
  cgBinaryIntOp(inst, instrIR, instrRR, oper, commuteFlag);
}

bool CodeGenerator::emitIncDecHelper(SSATmp* dst, SSATmp* src1, SSATmp* src2,
                                     void(Asm::*emitFunc)(Reg64)) {
  if (src1->getReg() != InvalidReg &&
      dst ->getReg() != InvalidReg &&
      // src2 == 1:
      src2->isConst() && src2->isA(Type::Int) && src2->getValInt() == 1) {
    emitMovRegReg(m_as, src1->getReg(), dst->getReg());
    (m_as.*emitFunc)(dst->getReg());
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
             &Asm::addsd_xmm_xmm,
             std::plus<int64_t>(),
             Commutative);
}

void CodeGenerator::cgOpSub(IRInstruction* inst) {
  SSATmp* dst   = inst->getDst();
  SSATmp* src1  = inst->getSrc(0);
  SSATmp* src2  = inst->getSrc(1);

  if (emitDec(dst, src1, src2)) return;

  if (src1->isConst() && src1->isA(Type::Int) && src1->getValInt() == 0 &&
      !src2->isA(Type::Dbl)) {
    return cgNegateWork(dst, src2);
  }

  cgBinaryOp(inst,
             &Asm::subq,
             &Asm::subq,
             &Asm::subsd_xmm_xmm,
             std::minus<int64_t>(),
             NonCommutative);
}

void CodeGenerator::cgOpAnd(IRInstruction* inst) {
  cgBinaryIntOp(inst,
                &Asm::andq,
                &Asm::andq,
                [] (int64_t a, int64_t b) { return a & b; },
                Commutative);
}

void CodeGenerator::cgOpOr(IRInstruction* inst) {
  cgBinaryIntOp(inst,
                &Asm::orq,
                &Asm::orq,
                [] (int64_t a, int64_t b) { return a | b; },
                Commutative);
}

void CodeGenerator::cgOpXor(IRInstruction* inst) {
  SSATmp* dst   = inst->getDst();
  SSATmp* src1  = inst->getSrc(0);
  SSATmp* src2  = inst->getSrc(1);
  if (src2->isConst() && src2->getType() == Type::Int &&
      src2->getValInt() == ~0L) {
    return cgNotWork(dst, src1);
  }

  cgBinaryIntOp(inst,
                &Asm::xorq,
                &Asm::xorq,
                [] (int64_t a, int64_t b) { return a ^ b; },
                Commutative);
}

void CodeGenerator::cgOpMul(IRInstruction* inst) {
  cgBinaryOp(inst,
             &Asm::imul,
             &Asm::imul,
             &Asm::mulsd_xmm_xmm,
             std::multiplies<int64_t>(),
             Commutative);
}

// Runtime helpers

HOT_FUNC_VM static int64_t arrToBoolHelper(const ArrayData *a) {
  return a->size() != 0;
}

HOT_FUNC_VM static int64_t cellToBoolHelper(TypedValue tv) {
  if (IS_NULL_TYPE(tv.m_type)) {
    return 0;
  }

  if (tv.m_type <= KindOfInt64) {
    return tv.m_data.num ? 1 : 0;
  }

  switch (tv.m_type) {
    case KindOfDouble:  return tv.m_data.dbl != 0;
    case KindOfStaticString:
    case KindOfString:  return tv.m_data.pstr->toBoolean();
    case KindOfArray:   return tv.m_data.parr->size() != 0;
    case KindOfObject:  return tv.m_data.pobj->o_toBoolean();
    default:
      assert(false);
      break;
  }
  return 0;
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

  Type type1 = src1->getType();
  Type type2 = src2->getType();

  auto src1Reg = src1->getReg();
  auto src2Reg = src2->getReg();
  auto dstReg  = dst ->getReg();

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
    ArgGroup args;
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
        prepBinaryXmmOp(m_as, src1, src2);
        doubleCmp(m_as, xmm0, xmm1);
        setFromFlags();
      } else {
        CG_PUNT(cgOpCmpHelper_Dbl);
      }
    }

    else if (type1.isString()) {
      // string cmp string is dealt with in case 1
      // string cmp double is punted above

      if (type2 == Type::Int) {
        ArgGroup args;
        args.ssa(src1).ssa(src2);
        cgCallHelper(m_as, (TCA)str_cmp_int,  dst, kSyncPoint, args);
      } else if (type2 == Type::Obj) {
        ArgGroup args;
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
        ArgGroup args;
        args.ssa(src1).ssa(src2);
        cgCallHelper(m_as, (TCA)obj_cmp_obj,  dst, kSyncPoint, args);
      } else if (type2 == Type::Int) {
        ArgGroup args;
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
    ArgGroup args;
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
  ConditionCode cc;

  assert(!type.subtypeOf(Type::Cls));

  if (type.isString()) {
    m_as.testl(KindOfStringBit, src);
    cc = CC_NZ;
  } else if (type == Type::UncountedInit) {
    m_as.testl(KindOfUncountedInitBit, src);
    cc = CC_NZ;
  } else if (type == Type::Uncounted) {
    m_as.cmpl(KindOfRefCountThreshold, src);
    cc = CC_LE;
  } else if (type == Type::Cell) {
    m_as.cmpl(KindOfRef, src);
    cc = CC_L;
  } else if (type == Type::Gen) {
    return CC_None; // nothing to check
  } else {
    DataType dataType = type.toDataType();
    assert(dataType == KindOfRef ||
           (dataType >= KindOfUninit && dataType <= KindOfObject));
    m_as.cmpl(dataType, src);
    cc = CC_Z;
  }
  return negate ? ccNegate(cc) : cc;
}

template<class OpndType>
ConditionCode CodeGenerator::emitTypeTest(IRInstruction* inst, OpndType src,
                                          bool negate) {
  return emitTypeTest(inst->getTypeParam(), src, negate);
}

void CodeGenerator::emitSetCc(IRInstruction* inst, ConditionCode cc) {
  if (cc == CC_None) return;
  m_as.setcc(cc, rbyte(inst->getDst()->getReg()));
}

ConditionCode CodeGenerator::emitIsTypeTest(IRInstruction* inst, bool negate) {
  if (inst->getTypeParam().subtypeOf(Type::Obj)) {
    // Task #2094715: Handle isType tests for Type::Obj, which
    // requires checking that checked object is not a resource
    CG_PUNT(cgIsTypeObject);
  }

  SSATmp*  src = inst->getSrc(0);
  if (src->isA(Type::PtrToGen)) {
    PhysReg base = src->getReg();
    return emitTypeTest(inst, base[TVOFF(m_type)], negate);
  }
  assert(src->isA(Type::Gen));
  assert(!src->isConst());
  PhysReg srcReg = src->getReg(1); // type register
  return emitTypeTest(inst, r32(srcReg), negate);
}

template<class OpndType>
void CodeGenerator::emitGuardType(OpndType src, IRInstruction* inst) {
  emitGuardOrFwdJcc(inst, emitTypeTest(inst, src, true));
}

void CodeGenerator::cgGuardTypeCell(PhysReg           baseReg,
                                    int64_t           offset,
                                    IRInstruction*    inst) {
  emitGuardType(baseReg[offset + TVOFF(m_type)], inst);
}

void CodeGenerator::cgIsTypeMemCommon(IRInstruction* inst, bool negate) {
  emitSetCc(inst, emitIsTypeTest(inst, negate));
}

void CodeGenerator::cgIsTypeCommon(IRInstruction* inst, bool negate) {
  emitSetCc(inst, emitIsTypeTest(inst, negate));
}

void CodeGenerator::cgJmpIsTypeCommon(IRInstruction* inst, bool negate) {
  emitJccDirectExit(inst, emitIsTypeTest(inst, negate));
}

void CodeGenerator::cgIsType(IRInstruction* inst) {
  cgIsTypeCommon(inst, false);
}

void CodeGenerator::cgIsNType(IRInstruction* inst) {
  cgIsTypeCommon(inst, true);
}

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

HOT_FUNC_VM static bool instanceOfHelperIFace(const Class* objClass,
                                              const Class* testClass) {
  return testClass && objClass->classof(testClass->preClass());
}

void CodeGenerator::emitInstanceCheck(IRInstruction* inst, PhysReg dstReg) {
  const bool ifaceHint = inst->getSrc(2)->getValBool();
  cgCallHelper(m_as,
               TCA(ifaceHint ? instanceOfHelperIFace : instanceOfHelper),
               dstReg,
               kNoSyncPoint,
               ArgGroup()
                 .ssa(inst->getSrc(0))
                 .ssa(inst->getSrc(1)));
}

void CodeGenerator::cgInstanceOf(IRInstruction* inst) {
  emitInstanceCheck(inst, inst->getDst()->getReg());
}

void CodeGenerator::cgNInstanceOf(IRInstruction* inst) {
  // TODO(#2058865): having NInstanceOf is no better than InstanceOf
  // followed by boolean Not opcode.
  emitInstanceCheck(inst, inst->getDst()->getReg());
  auto& a = m_as;
  a.    testb   (al, al);
  a.    setz    (al);
}

void CodeGenerator::cgJmpInstanceOf(IRInstruction* inst) {
  auto& a = m_as;
  emitInstanceCheck(inst, rax);
  a.    testb   (al, al);
  emitJccDirectExit(inst, CC_NZ);
}

void CodeGenerator::cgJmpNInstanceOf(IRInstruction* inst) {
  auto& a = m_as;
  emitInstanceCheck(inst, rax);
  a.    testb  (al, al);
  emitJccDirectExit(inst, CC_Z);
}

/*
 * Check instanceof using instance bitmasks.
 *
 * Note it's not necessary to check whether the test class is defined:
 * if it doesn't exist than the candidate can't be an instance of it
 * and will fail this check.
 */
void CodeGenerator::emitInstanceBitmaskCheck(IRInstruction* inst) {
  auto const rObjClass     = inst->getSrc(0)->getReg(0);
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
  a.    setnz  (rbyte(inst->getDst()->getReg()));
}

void CodeGenerator::cgNInstanceOfBitmask(IRInstruction* inst) {
  auto& a = m_as;
  emitInstanceBitmaskCheck(inst);
  a.    setz   (rbyte(inst->getDst()->getReg()));
}

void CodeGenerator::cgJmpInstanceOfBitmask(IRInstruction* inst) {
  emitInstanceBitmaskCheck(inst);
  emitJccDirectExit(inst, CC_NZ);
}

void CodeGenerator::cgJmpNInstanceOfBitmask(IRInstruction* inst) {
  emitInstanceBitmaskCheck(inst);
  emitJccDirectExit(inst, CC_Z);
}

/*
 * Check instanceof using the superclass vector on the end of the
 * Class entry.
 */
void CodeGenerator::cgExtendsClass(IRInstruction* inst) {
  auto const rObjClass     = inst->getSrc(0)->getReg();
  auto const testClass     = inst->getSrc(1)->getValClass();
  auto rTestClass          = inst->getSrc(1)->getReg();
  auto const rdst          = rbyte(inst->getDst()->getReg());
  auto& a = m_as;

  Label out;
  Label notExact;
  Label falseLabel;

  if (rTestClass == InvalidReg) { // TODO(#2031606)
    rTestClass = rScratch; // careful below about asm-x64 smashing this
    emitLoadImm(a, (int64_t)testClass, rTestClass);
  }

  // Test if it is the exact same class.  TODO(#2044801): we should be
  // doing this control flow at the IR level.
  if (!(testClass->attrs() & AttrAbstract)) {
    if (Class::alwaysLowMem()) {
      a.  cmpl   (r32(rTestClass), r32(rObjClass));
    } else {
      a.  cmpq   (rTestClass, rObjClass);
    }
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
  if (Class::alwaysLowMem()) {
    a.  cmpl   (r32(rTestClass), rObjClass[vecOffset]);
  } else {
    a.  cmpq   (rTestClass, rObjClass[vecOffset]);
  }
  a.    sete   (rdst);
  a.    jmp8   (out);

asm_label(a, falseLabel);
  a.    xorl   (r32(rdst), r32(rdst));

asm_label(a, out);
}

HOT_FUNC_VM
ArrayData* new_singleton_array_helper(TypedValue value) {
  // tvCastToArrayInPlace overwrites value and thus decrements the ref count
  // of any counted object that value refers to.
  // The code calling cgConv (emitCastArray) does not expect that ref count
  // to be decreased and thus emits a dec ref following the call to this helper.
  // emitCastArray could (and probably should) be modified to not do that
  // which means that an inc ref dec ref pair can be eliminated. Unfortunately
  // doing that triggers a test failure. See Task 2160031.
  tvRefcountedIncRef(&value);
  tvCastToArrayInPlace(&value);
  return value.m_data.parr;
}

void CodeGenerator::cgConv(IRInstruction* inst) {
  Type toType   = inst->getTypeParam();
  Type fromType = inst->getSrc(0)->getType();
  SSATmp* dst = inst->getDst();
  SSATmp* src = inst->getSrc(0);

  auto dstReg = dst->getReg();
  auto srcReg = src->getReg();

  bool srcIsConst = src->isConst();

  if (toType == Type::Int) {
    if (fromType == Type::Bool) {
      // Bool -> Int is just a move
      if (srcIsConst) {
        int64_t constVal = src->getValRawInt();
        if (constVal == 0) {
          m_as.xor_reg64_reg64(dstReg, dstReg);
        } else {
          m_as.mov_imm64_reg(1, dstReg);
        }
      } else {
        m_as.movzbl (rbyte(srcReg), r32(dstReg));
      }
      return;
    }
    if (fromType.isString()) {
      if (src->isConst()) {
        auto val = src->getValStr()->toInt64();
        m_as.mov_imm64_reg(val, dstReg);
      } else {
        ArgGroup args;
        args.ssa(src).imm(10);
        cgCallHelper(m_as,
                     Transl::Call(getMethodPtr(&StringData::toInt64)),
                     dstReg, kNoSyncPoint, args);
      }
      return;
    }
  }

  if (toType == Type::Bool) {
    if (fromType.isNull()) {
      // Uninit/Null -> Bool (false)
      m_as.xor_reg64_reg64(dstReg, dstReg);
    } else if (fromType == Type::Bool) {
      // Bool -> Bool (nop!)
      if (srcIsConst) {
        int64_t constVal = src->getValRawInt();
        if (constVal == 0) {
          m_as.xor_reg64_reg64(dstReg, dstReg);
        } else {
          m_as.mov_imm64_reg(1, dstReg);
        }
      } else {
        emitMovRegReg(m_as, srcReg, dstReg);
      }
    } else if (fromType == Type::Int) {
      // Int -> Bool
      if (src->isConst()) {
        int64_t constVal = src->getValInt();
        if (constVal == 0) {
          m_as.xor_reg64_reg64(dstReg, dstReg);
        } else {
          m_as.mov_imm64_reg(1, dstReg);
        }
      } else {
        m_as.test_reg64_reg64(srcReg, srcReg);
        m_as.setne(rbyte(dstReg));
      }
    } else {
      Transl::Call helper(nullptr);
      ArgGroup args;
      args.ssa(src);
      if (fromType == Type::Cell) {
        // Cell -> Bool
        args.type(src);
        helper = Transl::Call((TCA)cellToBoolHelper);
      } else if (fromType.isString()) {
        // Str -> Bool
        helper = Transl::Call(getMethodPtr(&StringData::toBoolean));
      } else if (fromType.isArray()) {
        // Arr -> Bool
        helper = Transl::Call((TCA)arrToBoolHelper);
      } else if (fromType == Type::Obj) {
        // Obj -> Bool
        helper = Transl::Call(getMethodPtr(&ObjectData::o_toBoolean));
      } else {
        // Dbl -> Bool
        CG_PUNT(Conv_Dbl_Bool);
      }
      cgCallHelper(m_as, helper, dstReg, kNoSyncPoint, args);
    }
    return;
  }

  if (toType.isString()) {
    if (fromType == Type::Int) {
      // Int -> Str
      ArgGroup args;
      args.ssa(src);
      StringData*(*fPtr)(int64_t) = buildStringData;
      cgCallHelper(m_as, (TCA)fPtr,
                   dst, kNoSyncPoint, args);
    } else if (fromType == Type::Bool) {
      // Bool -> Str
      m_as.testb(Reg8(int(srcReg)), Reg8(int(srcReg)));
      m_as.mov_imm64_reg((uint64_t)StringData::GetStaticString(""),
                         dstReg);
      m_as.mov_imm64_reg((uint64_t)StringData::GetStaticString("1"),
                         rScratch);
      m_as.cmov_reg64_reg64(CC_NZ, rScratch, dstReg);
    } else {
      CG_PUNT(Conv_toString);
    }
    return;
  }

  if (toType.isArray()) {
    ArgGroup args;
    args.ssa(src);
    args.type(src);
    ArrayData*(*fPtr)(TypedValue) = new_singleton_array_helper;
    cgCallHelper(m_as, (TCA)fPtr, dst, kNoSyncPoint, args);
    return;
  }

  // TODO: Add handling of conversions
  CG_PUNT(Conv);
}

void CodeGenerator::cgUnboxPtr(IRInstruction* inst) {
  SSATmp* dst   = inst->getDst();
  SSATmp* src   = inst->getSrc(0);

  auto srcReg = src->getReg();
  auto dstReg = dst->getReg();

  assert(srcReg != InvalidReg);
  assert(dstReg != InvalidReg);

  emitMovRegReg(m_as, srcReg, dstReg);
  emitDerefIfVariant(m_as, PhysReg(dstReg));
}

void CodeGenerator::cgUnbox(IRInstruction* inst) {
  SSATmp* dst     = inst->getDst();
  SSATmp* src     = inst->getSrc(0);
  auto dstValReg  = dst->getReg(0);
  auto dstTypeReg = dst->getReg(1);
  auto srcValReg  = src->getReg(0);
  auto srcTypeReg = src->getReg(1);

  assert(src->getType().equals(Type::Gen));
  assert(dst->getType().notBoxed());


  // cmpq   KindOfRef, srcTypeReg
  // mov    srcTypeReg --> dstTypeReg
  //
  // -- srcTypeReg is now dead
  // -- if srcValReg == dstTypeReg, then use scratch for dstTypeReg; otherwise,
  // -- the next load will kill srcValReg
  //
  // cload.z [srcValReg + m_type] --> dstTypeReg
  // mov    srcValReg --> dstValReg (potentially move up 1 instruction)
  // cload.z [srcValReg + m_data] --> dstValReg

  PhysReg tmpDstTypeReg = srcValReg == dstTypeReg ? PhysReg(rScratch)
                                                  : dstTypeReg;

  m_as.   cmpq(HPHP::KindOfRef, srcTypeReg);
  emitMovRegReg(m_as, srcTypeReg, tmpDstTypeReg);
  m_as.   cload_reg64_disp_reg64(CC_Z, srcValReg, TVOFF(m_type), tmpDstTypeReg);
  emitMovRegReg(m_as, srcValReg, dstValReg);
  m_as.   cload_reg64_disp_reg64(CC_Z, srcValReg, TVOFF(m_data), dstValReg);
  emitMovRegReg(m_as, tmpDstTypeReg, dstTypeReg);
}

void CodeGenerator::cgLdFixedFunc(IRInstruction* inst) {
  SSATmp* dst        = inst->getDst();
  SSATmp* methodName = inst->getSrc(0);

  using namespace TargetCache;
  const StringData* name = methodName->getValStr();
  CacheHandle ch = allocFixedFunction(name);
  size_t funcCacheOff = ch + offsetof(FixedFuncCache, m_func);

  auto dstReg = dst->getReg();
  if (dstReg == InvalidReg) {
    // happens if LdFixedFunc and FCall not in same trace
    m_as.   cmpq(0, rVmTl[funcCacheOff]);
  } else {
    m_as.   loadq (rVmTl[funcCacheOff], dstReg);
    m_as.   testq (dstReg, dstReg);
  }
  // jz off to the helper call in astubs
  unlikelyIfBlock(CC_Z, [&] {
    // this helper tries the autoload map, and fatals on failure
    cgCallHelper(m_astubs, (TCA)FixedFuncCache::lookupUnknownFunc,
                 dstReg, kSyncPoint, ArgGroup().immPtr(name));
  });
}

void CodeGenerator::cgLdFunc(IRInstruction* inst) {
  SSATmp*        dst = inst->getDst();
  SSATmp* methodName = inst->getSrc(0);

  TargetCache::CacheHandle ch = TargetCache::FuncCache::alloc();
  // raises an error if function not found
  cgCallHelper(m_as, (TCA)FuncCache::lookup, dst->getReg(), kSyncPoint,
               ArgGroup().imm(ch).ssa(methodName));
}

static void emitLdObjClass(CodeGenerator::Asm& a,
                           PhysReg objReg,
                           PhysReg dstReg) {
  a.loadq  (objReg[ObjectData::getVMClassOffset()], dstReg);
}

void CodeGenerator::cgLdObjClass(IRInstruction* inst) {
  auto dstReg = inst->getDst()->getReg();
  auto objReg = inst->getSrc(0)->getReg();

  emitLdObjClass(m_as, objReg, dstReg);
}

void CodeGenerator::cgRetVal(IRInstruction* inst) {
  auto const rFp    = inst->getSrc(0)->getReg();
  auto* const val   = inst->getSrc(1);
  auto& a = m_as;

  // Store return value at the top of the caller's eval stack
  // (a) Store the type
  if (val->getType().needsReg()) {
    a.    storel (r32(val->getReg(1)), rFp[AROFF(m_r) + TVOFF(m_type)]);
  } else {
    a.    storel (val->getType().toDataType(),
                  rFp[AROFF(m_r) + TVOFF(m_type)]);
  }

  // (b) Store the actual value (not necessary when storing Null)
  if (val->getType().isNull()) return;
  if (val->getInstruction()->getOpcode() == DefConst) {
    a.    storeq (val->getValRawInt(),
                  rFp[AROFF(m_r) + TVOFF(m_data)]);
  } else {
    zeroExtendIfBool(m_as, val);
    a.    storeq (val->getReg(), rFp[AROFF(m_r) + TVOFF(m_data)]);
  }
}

void CodeGenerator::cgRetAdjustStack(IRInstruction* inst) {
  auto const rFp   = inst->getSrc(0)->getReg();
  auto const dstSp = inst->getDst()->getReg();
  auto& a = m_as;
  a.    lea   (rFp[AROFF(m_r)], dstSp);
}

void CodeGenerator::cgLdRetAddr(IRInstruction* inst) {
  auto fpReg = inst->getSrc(0)->getReg(0);
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
  DEBUG_ONLY Cell* firstSp = ((Cell*)fp) - func->numSlotsInFrame();
  assert(sp <= firstSp || func->isGenerator());
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
  checkFrame(fp, sp, false);
  assertTv(sp); // check return value
}

void CodeGenerator::emitTraceRet(CodeGenerator::Asm& a) {
  // call to a trace function
  // ld return ip from native stack into rdx
  a.    loadq (*rsp, rdx);
  a.    movq  (rVmFp, rdi);
  a.    movq  (rVmSp, rsi);
  // do the call; may use a trampoline
  m_tx64->emitCall(a, TCA(traceRet));
}

void CodeGenerator::cgRetCtrl(IRInstruction* inst) {
  SSATmp* sp = inst->getSrc(0);
  SSATmp* fp = inst->getSrc(1);

  // Make sure rVmFp and rVmSp are set appropriately
  emitMovRegReg(m_as, sp->getReg(), rVmSp);
  emitMovRegReg(m_as, fp->getReg(), rVmFp);

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
  dest = m_tx64->emitServiceReq(TranslatorX64::SRFlags::SRNone,
                                REQ_BIND_ADDR,
                                2ull,
                                &dest,
                                offset);
}

void CodeGenerator::cgJmpSwitchDest(IRInstruction* inst) {
  JmpSwitchData* data = inst->getExtra<JmpSwitchDest>();
  SSATmp* index       = inst->getSrc(0);
  auto indexReg       = index->getReg();

  if (!index->isConst()) {
    if (data->bounded) {
      if (data->base) {
        m_as.  subq(data->base, indexReg);
      }
      m_as.    cmpq(data->cases - 2, indexReg);
      m_tx64->prepareForSmash(m_as, TranslatorX64::kJmpccLen);
      TCA def = m_tx64->emitServiceReq(REQ_BIND_JMPCC_SECOND, 3,
                                   m_as.code.frontier, data->defaultOff, CC_AE);
      m_as.    jae(def);
    }

    TCA* table = m_tx64->m_globalData.alloc<TCA>(sizeof(TCA), data->cases);
    TCA afterLea = m_as.code.frontier + TranslatorX64::kLeaRipLen;
    ptrdiff_t diff = (TCA)table - afterLea;
    assert(deltaFits(diff, sz::dword));
    m_as.   lea(rip[diff], rScratch);
    assert(m_as.code.frontier == afterLea);
    m_as.   jmp(rScratch[indexReg*8]);

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
               ArgGroup()
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
               ArgGroup()
                 .valueType(inst->getSrc(0))
                 .immPtr(strtab)
                 .imm(data->numCases)
                 .immPtr(jmptab));
}

void CodeGenerator::cgFreeActRec(IRInstruction* inst) {
  SSATmp* outFp = inst->getDst();
  SSATmp* inFp  = inst->getSrc(0);

  auto  inFpReg =  inFp->getReg();
  auto outFpReg = outFp->getReg();

  m_as.load_reg64_disp_reg64(inFpReg, AROFF(m_savedRbp), outFpReg);
}

void CodeGenerator::cgAllocSpill(IRInstruction* inst) {
  SSATmp* numSlots = inst->getSrc(0);

  assert(numSlots->isConst());
  int64_t n = numSlots->getValInt();
  assert(n >= 0 && n % 2 == 0);
  if (n > 0) {
    m_as.sub_imm32_reg64(spillSlotsToSize(n), reg::rsp);
  }
}

void CodeGenerator::cgFreeSpill(IRInstruction* inst) {
  SSATmp* numSlots = inst->getSrc(0);

  assert(numSlots->isConst());
  int64_t n = numSlots->getValInt();
  assert(n >= 0 && n % 2 == 0);
  if (n > 0) {
    m_as.add_imm32_reg64(spillSlotsToSize(n), reg::rsp);
  }
}

void CodeGenerator::cgSpill(IRInstruction* inst) {
  SSATmp* dst   = inst->getDst();
  SSATmp* src   = inst->getSrc(0);

  assert(dst->numNeededRegs() == src->numNeededRegs());
  for (int locIndex = 0; locIndex < src->numNeededRegs(); ++locIndex) {
    auto srcReg = src->getReg(locIndex);

    // We do not need to mask booleans, since the IR will reload the spill
    auto sinfo = dst->getSpillInfo(locIndex);
    switch (sinfo.type()) {
    case SpillInfo::MMX:
      m_as.    mov_reg64_mmx(srcReg, sinfo.mmx());
      break;
    case SpillInfo::Memory:
      m_as.    store_reg64_disp_reg64(srcReg,
                                      sizeof(uint64_t) * sinfo.mem(),
                                      reg::rsp);
      break;
    }
  }
}

void CodeGenerator::cgReload(IRInstruction* inst) {
  SSATmp* dst   = inst->getDst();
  SSATmp* src   = inst->getSrc(0);

  assert(dst->numNeededRegs() == src->numNeededRegs());
  for (int locIndex = 0; locIndex < src->numNeededRegs(); ++locIndex) {
    auto dstReg = dst->getReg(locIndex);

    auto sinfo = src->getSpillInfo(locIndex);
    switch (sinfo.type()) {
    case SpillInfo::MMX:
      m_as.    mov_mmx_reg64(sinfo.mmx(), dstReg);
      break;
    case SpillInfo::Memory:
      m_as.    load_reg64_disp_reg64(reg::rsp,
                                     sizeof(uint64_t) * sinfo.mem(),
                                     dstReg);
      break;
    }
  }
}

void CodeGenerator::cgStPropWork(IRInstruction* inst, bool genTypeStore) {
  SSATmp* obj   = inst->getSrc(0);
  SSATmp* prop  = inst->getSrc(1);
  SSATmp* src   = inst->getSrc(2);
  cgStore(obj->getReg(), prop->getValInt(), src, genTypeStore);
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
  cgStore(addr->getReg(), offset->getValInt(), src, genStoreType);
}
void CodeGenerator::cgStMem(IRInstruction* inst) {
  cgStMemWork(inst, true);
}
void CodeGenerator::cgStMemNT(IRInstruction* inst) {
  cgStMemWork(inst, false);
}

void CodeGenerator::cgStRefWork(IRInstruction* inst, bool genStoreType) {
  auto destReg = inst->getDst()->getReg();
  auto addrReg = inst->getSrc(0)->getReg();
  SSATmp* src  = inst->getSrc(1);
  cgStore(addrReg, 0, src, genStoreType);
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
  cgStore(inst->getSrc(0)->getReg(),
          getLocalOffset(inst->getExtra<StLoc>()->locId),
          inst->getSrc(1),
          true /* store type */);
}

void CodeGenerator::cgStLocNT(IRInstruction* inst) {
  cgStore(inst->getSrc(0)->getReg(),
          getLocalOffset(inst->getExtra<StLocNT>()->locId),
          inst->getSrc(1),
          false /* store type */);
}

void CodeGenerator::cgSyncVMRegs(IRInstruction* inst) {
  emitMovRegReg(m_as, inst->getSrc(0)->getReg(), rVmFp);
  emitMovRegReg(m_as, inst->getSrc(1)->getReg(), rVmSp);
}

void CodeGenerator::cgExitTrace(IRInstruction* inst) {
  SSATmp* func = inst->getSrc(0);
  SSATmp* pc   = inst->getSrc(1);
  SSATmp* sp   = inst->getSrc(2);
  SSATmp* fp   = inst->getSrc(3);
  SSATmp* notTakenPC = nullptr;
  SSATmp* toSmash = nullptr;
  assert(pc->isConst() && inst->getNumSrcs() <= 6);

  TraceExitType::ExitType exitType = getExitType(inst->getOpcode());
  if (exitType == TraceExitType::Normal && inst->getNumSrcs() == 5) {
    // Unconditional trace exit
    toSmash    = inst->getSrc(4);
    assert(toSmash);
  } else if (exitType == TraceExitType::NormalCc) {
    // Exit at trace end which is the target of a conditional branch
    notTakenPC = inst->getSrc(4);
    assert(notTakenPC->isConst());
    if (inst->getNumSrcs() == 6) {
      toSmash    = inst->getSrc(5);
      assert(toSmash);
    }
  }
  using namespace HPHP::VM::Transl;

  Asm& a = m_as; // Note: m_as is the same as m_atubs for Exit Traces,
  // unless exit trace was moved to end of main trace

  emitMovRegReg(a, sp->getReg(), rVmSp);
  emitMovRegReg(a, fp->getReg(), rVmFp);

  // Get the SrcKey for the dest
  SrcKey  destSK(func->getValFunc(), pc->getValInt());

  switch(exitType) {
    case TraceExitType::NormalCc:
      if (toSmash) {
        TCA smashAddr = toSmash->getTCA();
        if (smashAddr == kIRDirectJmpInactive) {
          // The jump in the main trace has been optimized away
          // this exit trace is no longer needed
          break;
        }
        // Patch the original jcc;jmp, don't emit another
        IRInstruction* jcc = toSmash->getInstruction();
        Opcode         opc = jcc->getOpcode();
        ConditionCode  cc  = cmpOpToCC(queryJmpToQueryOp(opc));
        uint64_t     taken = pc->getValInt();
        uint64_t  notTaken = notTakenPC->getValInt();

        m_astubs.setcc(cc, rbyte(serviceReqArgRegs[4]));
        m_tx64-> emitServiceReq(TranslatorX64::SRFlags::SRInline,
                                REQ_BIND_JMPCC_FIRST,
                                4ull,
                                smashAddr,
                                taken,
                                notTaken,
                                uint64_t(cc));
      } else {
        // NormalCc exit but not optimized to jcc directly to destination
        m_tx64->emitBindJmp(a, destSK, REQ_BIND_JMP);
      }
      break;
    case TraceExitType::Normal:
      {
        TCA smashAddr = toSmash ? toSmash->getTCA() : nullptr;
        if (smashAddr) {
          assert(smashAddr != kIRDirectJmpInactive);
          if (smashAddr != kIRDirectJccJmpActive) {
            // kIRDirectJccJmpActive only needs NormalCc exit in astubs

            m_tx64->emitServiceReq(TranslatorX64::SRFlags::SRInline,
                                   REQ_BIND_JMP, 2,
                                   smashAddr,
                                   uint64_t(destSK.offset()));

          }
        } else {
          assert(smashAddr == kIRDirectJmpInactive);
          m_tx64->emitBindJmp(a, destSK, REQ_BIND_JMP);
        }
      }
      break;
    case TraceExitType::Slow:
    case TraceExitType::SlowNoProgress:
      if (RuntimeOption::EnableInstructionCounts ||
          HPHP::Trace::moduleEnabled(HPHP::Trace::stats, 3)) {
        Stats::emitInc(m_as,
                       Stats::opcodeToIRPostStatCounter(
                         Op(*getCurFunc()->unit()->at(destSK.m_offset))),
                       -1,
                       Transl::CC_None,
                       true);
      }

      if (HPHP::Trace::moduleEnabled(HPHP::Trace::punt, 1)) {
        VM::Op op = (VM::Op)*func->getValFunc()->unit()->at(destSK.m_offset);
        std::string name = folly::format(
          "exitSlow{}-{}",
          exitType == TraceExitType::SlowNoProgress ? "-np" : "",
          VM::opcodeToName(op)).str();
        m_tx64->emitRecordPunt(a, name);
      }
      if (exitType == TraceExitType::Slow) {
        m_tx64->emitBindJmp(a, destSK, REQ_BIND_JMP_NO_IR);
      } else {
        m_tx64->emitReqRetransNoIR(a, destSK);
      }
      break;
    case TraceExitType::GuardFailure:
      SrcRec* destSR = m_tx64->getSrcRec(destSK);
      m_tx64->emitFallbackUncondJmp(a, *destSR);
      break;
  }
}

void CodeGenerator::cgExitTraceCc(IRInstruction* inst) {
  cgExitTrace(inst);
}

void CodeGenerator::cgExitSlow(IRInstruction* inst) {
  cgExitTrace(inst);
}

void CodeGenerator::cgExitSlowNoProgress(IRInstruction* inst) {
  cgExitTrace(inst);
}

void CodeGenerator::cgExitGuardFailure(IRInstruction* inst) {
  cgExitTrace(inst);
}

static void emitAssertFlagsNonNegative(CodeGenerator::Asm& as) {
  ifThen(as, CC_NGE, [&] { as.int3(); });
}

static void emitAssertRefCount(CodeGenerator::Asm& as, PhysReg base) {
  as.cmpl(HPHP::RefCountStaticValue, base[TVOFF(_count)]);
  ifThen(as, CC_NBE, [&] { as.int3(); });
}

static void emitIncRef(CodeGenerator::Asm& as, PhysReg base) {
  if (RuntimeOption::EvalHHIRGenerateAsserts) {
    emitAssertRefCount(as, base);
  }
  // emit incref
  as.addl(1, base[TVOFF(_count)]);
  if (RuntimeOption::EvalHHIRGenerateAsserts) {
    // Assert that the ref count is greater than zero
    emitAssertFlagsNonNegative(as);
  }
}

void CodeGenerator::cgIncRefWork(Type type, SSATmp* src) {
  assert(type.maybeCounted());
  auto increfMaybeStatic = [&] {
    auto base = src->getReg(0);
    if (!type.needsStaticBitCheck()) {
      emitIncRef(m_as, base);
    } else {
      m_as.cmpl(RefCountStaticValue, base[TVOFF(_count)]);
      ifThen(m_as, CC_NE, [&] { emitIncRef(m_as, base); });
    }
  };

  if (type.isKnownDataType()) {
    assert(IS_REFCOUNTED_TYPE(type.toDataType()));
    increfMaybeStatic();
  } else {
    m_as.cmpl(KindOfRefCountThreshold, r32(src->getReg(1)));
    ifThen(m_as, CC_NLE, [&] { increfMaybeStatic(); });
  }
}

void CodeGenerator::cgIncRef(IRInstruction* inst) {
  SSATmp* dst    = inst->getDst();
  SSATmp* src    = inst->getSrc(0);
  Type type = src->getType();

  if (m_curTrace->isMain()) {
    TRACE(3, "[counter] 1 IncRef in main traces\n");
  }
  cgIncRefWork(type, src);
  shuffle2(m_as, src->getReg(0), src->getReg(1),
           dst->getReg(0), dst->getReg(1));
}

void CodeGenerator::cgDecRefStack(IRInstruction* inst) {
  Type type = inst->getTypeParam();
  SSATmp* sp     = inst->getSrc(0);
  SSATmp* index  = inst->getSrc(1);
  cgDecRefMem(type,
              sp->getReg(),
              index->getValInt() * sizeof(Cell),
              nullptr);
}

void CodeGenerator::cgDecRefThis(IRInstruction* inst) {
  SSATmp* fp    = inst->getSrc(0);
  Block* exit   = inst->getTaken();
  auto fpReg = fp->getReg();
  auto scratchReg = rScratch;

  // Load AR->m_this into rScratch
  m_as.loadq(fpReg[AROFF(m_this)], scratchReg);

  auto decrefIfAvailable = [&] {
    // Check if this is available and we're not in a static context instead
    m_as.testb(1, rbyte(scratchReg));
    ifThen(m_as, CC_Z, [&] {
      // Currently we need to store zero back to m_this in case a local
      // destructor does debug_backtrace.
      m_as.storeq(0, fpReg[AROFF(m_this)]);
      cgDecRefStaticType(Type::Obj, scratchReg, exit, true);
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
              inst->getSrc(0)->getReg(),
              getLocalOffset(inst->getExtra<DecRefLoc>()->locId),
              inst->getTaken());
}

void CodeGenerator::cgGenericRetDecRefs(IRInstruction* inst) {
  auto const rFp       = inst->getSrc(0)->getReg();
  auto const retVal    = inst->getSrc(1);
  auto const numLocals = inst->getSrc(2)->getValInt();
  auto const rDest     = inst->getDst()->getReg();
  auto& a = m_as;

  RegSet retvalRegs;
  for (int i = 0; i < retVal->numAllocatedRegs(); ++i) {
    retvalRegs.add(retVal->getReg(i));
  }
  assert(retvalRegs.size() <= 2);

  /*
   * The generic decref helpers preserve these two registers.
   *
   * XXX/TODO: we ideally shouldn't be moving the return value into
   * these regs and then back; it'd be better to precolor allocation
   * this way, or failing that remap the return value SSATmp with a
   * separate instruction.  This scheme also won't work for us once we
   * want hackIR to support handling surprise flags, because
   * setprofile will look on the VM stack for the return value.
   */
  auto spillRegs = RegSet(r14).add(r15);

  /*
   * Since we're making a call using a custom ABI to the generic
   * decref helper, it's important that our src and dest registers are
   * allocated to the registers we expect, and that no other SSATmp's
   * are still allocated to registers at this time.
   */
  const auto UNUSED expectedLiveRegs = RegSet(rFp).add(rDest) | retvalRegs;
  assert((m_curInst->getLiveOutRegs() - expectedLiveRegs).empty());
  assert(rFp == rVmFp &&
         "free locals helper assumes the frame pointer is rVmFp");
  assert(rDest == rVmSp &&
         "free locals helper adjusts rVmSp, which must be our dst reg");

  if (!numLocals) {
    a.  lea   (rFp[AROFF(m_r)], rDest);
    return;
  }

  // Remove overlap so we don't move registers that are already in the
  // saved set.
  auto intersectedRegs = spillRegs & retvalRegs;
  spillRegs -= intersectedRegs;
  retvalRegs -= intersectedRegs;

  auto grabPair = [&] (std::pair<PhysReg,PhysReg>& out) {
    assert(!retvalRegs.empty() && !spillRegs.empty());
    retvalRegs.findFirst(out.first);
    spillRegs.findFirst(out.second);
    retvalRegs.remove(out.first);
    spillRegs.remove(out.second);
  };

  auto savePairA = std::make_pair(InvalidReg, InvalidReg);
  auto savePairB = std::make_pair(InvalidReg, InvalidReg);
  if (!retvalRegs.empty()) {
    grabPair(savePairA);
    if (!retvalRegs.empty()) {
      grabPair(savePairB);
    }
  }

  if (savePairA.first != InvalidReg) {
    a.  movq   (savePairA.first, savePairA.second);
    if (savePairB.first != InvalidReg) {
      a.movq   (savePairB.first, savePairB.second);
    }
  }

  auto const target = numLocals > kNumFreeLocalsHelpers
    ? m_tx64->m_freeManyLocalsHelper
    : m_tx64->m_freeLocalsHelpers[numLocals - 1];

  a.    subq   (0x8, rsp);  // For parity; callee does retq $0x8.
  a.    lea    (rFp[-numLocals * sizeof(TypedValue)], rVmSp);
  a.    call   (target);
  recordSyncPoint(a);

  if (savePairA.first != InvalidReg) {
    a.  movq   (savePairA.second, savePairA.first);
    if (savePairB.first != InvalidReg) {
      a.movq   (savePairB.second, savePairB.first);
    }
  }
}

static void
tv_release_generic(TypedValue* tv) {
  assert(VM::Transl::tx64->stateIsDirty());
  assert(tv->m_type >= KindOfString && tv->m_type <= KindOfRef);
  g_destructors[typeToDestrIndex(tv->m_type)](tv->m_data.pref);
}

static void
tv_release_typed(RefData* pv, DataType dt) {
  assert(VM::Transl::tx64->stateIsDirty());
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
    m_as.cmp_imm32_disp_reg32(RefCountStaticValue, TVOFF(_count), reg);
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
  const auto scratchReg = rScratch;

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
    m_as.load_reg64_disp_reg32(dataReg, TVOFF(_count), scratchReg);

    // Check for RefCountStaticValue
    patchStaticCheck = cgCheckStaticBit(type, scratchReg,
                                        true /* reg has _count */);

    // Decrement count and store it back in memory.
    // If there's an exit, emit jump to it when _count would get down to 0
    m_as.sub_imm32_reg32(1, scratchReg);
    if (exit) {
      emitFwdJcc(CC_E, exit);
    }
    if (RuntimeOption::EvalHHIRGenerateAsserts) {
      // Assert that the ref count is greater than zero
      emitAssertFlagsNonNegative(m_as);
    }
    m_as.store_reg32_disp_reg64(scratchReg, TVOFF(_count), dataReg);

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
      m_as.cmp_imm32_disp_reg32(1, TVOFF(_count), dataReg);
      emitFwdJcc(CC_E, exit);
    }
    if (RuntimeOption::EvalHHIRGenerateAsserts) {
      emitAssertRefCount(m_as, dataReg);
    }

    // Decrement _count
    m_as.sub_imm32_disp_reg32(1, TVOFF(_count), dataReg);

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

  m_as.cmp_imm32_reg32(KindOfRefCountThreshold, typeReg);

  Address addrToPatch =  m_as.code.frontier;
  m_as.jcc8(CC_LE, addrToPatch);

  return addrToPatch;
}

Address CodeGenerator::cgCheckRefCountedType(PhysReg baseReg,
                                             int64_t offset) {

  m_as.cmp_imm32_disp_reg32(KindOfRefCountThreshold,
                            offset + TVOFF(m_type),
                            baseReg);

  Address addrToPatch =  m_as.code.frontier;
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
    unlikelyIfBlock(CC_Z, [&] {
      // Emit the call to release in m_astubs
      cgCallHelper(m_astubs, m_tx64->getDtorCall(type.toDataType()),
                   InvalidReg, InvalidReg, kSyncPoint, ArgGroup().reg(dataReg));
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
    unlikelyIfBlock(CC_Z, [&] {
      // Emit call to release in m_astubs
      cgCallHelper(m_astubs, getDtorTyped(), InvalidReg, kSyncPoint,
                   ArgGroup().reg(dataReg).reg(typeReg));
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
  auto scratchReg = rScratch;

  assert(baseReg != scratchReg);

  // Emit check for ref-counted type
  Address patchTypeCheck = cgCheckRefCountedType(baseReg, offset);
  if (exit == nullptr && RuntimeOption::EvalHHIRGenericDtorHelper) {
    {
      // This PhysRegSaverParity saves rdi redundantly if
      // !m_curInst->getLiveOutRegs().contains(rdi), but its
      // necessary to maintain stack alignment. We can do better
      // by making the helpers adjust the stack for us in the cold
      // path, which calls the destructor.
      PhysRegSaverParity<0> regSaver(m_as, RegSet(rdi));
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
  m_as.load_reg64_disp_reg64(baseReg, offset + TVOFF(m_data), scratchReg);

  // Emit check for RefCountStaticValue and the actual DecRef
  Address patchStaticCheck = cgCheckStaticBitAndDecRef(Type::Cell, scratchReg,
                                                       exit);

  // If not exiting on count down to zero, emit the zero-check and release call
  if (exit == nullptr) {
    // Emit jump to m_astubs (to call release) if count got down to zero
    unlikelyIfBlock(CC_Z, [&] {
      // Emit call to release in m_astubs
      m_astubs.lea_reg64_disp_reg64(baseReg, offset, scratchReg);
      cgCallHelper(m_astubs, getDtorGeneric(), InvalidReg, kSyncPoint,
                   ArgGroup().reg(scratchReg));
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
  auto scratchReg = rScratch;
  assert(baseReg != scratchReg);

  if (type.needsReg()) {
    // The type is dynamic, but we don't have two registers available
    // to load the type and the data.
    cgDecRefDynamicTypeMem(baseReg, offset, exit);
  } else if (type.maybeCounted()) {
    m_as.load_reg64_disp_reg64(baseReg, offset, scratchReg);
    cgDecRefStaticType(type, scratchReg, exit, true);
  }
}

void CodeGenerator::cgDecRefMem(IRInstruction* inst) {
  assert(inst->getSrc(0)->getType().isPtr());
  cgDecRefMem(inst->getTypeParam(),
              inst->getSrc(0)->getReg(),
              inst->getSrc(1)->getValInt(),
              inst->getTaken());
}

void CodeGenerator::cgDecRefWork(IRInstruction* inst, bool genZeroCheck) {
  SSATmp* src   = inst->getSrc(0);
  if (!isRefCounted(src)) return;
  Block* exit = inst->getTaken();
  Type type = src->getType();
  if (type.isKnownDataType()) {
    cgDecRefStaticType(type, src->getReg(), exit, genZeroCheck);
  } else {
    cgDecRefDynamicType(src->getReg(1),
                        src->getReg(0),
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

void CodeGenerator::emitSpillActRec(SSATmp* sp,
                                    int64_t spOffset,
                                    SSATmp* defAR) {
  if (debug) {
    // Ensure srcs of defAR are still live, since we use their registers.
    for (SSATmp* UNUSED src : defAR->getInstruction()->getSrcs()) {
      assert(src->getInstruction()->getOpcode() == DefConst ||
             src->getLastUseId() >= m_curInst->getId());
    }
  }
  auto* defInst     = defAR->getInstruction();
  SSATmp* fp        = defInst->getSrc(0);
  SSATmp* func      = defInst->getSrc(1);
  SSATmp* objOrCls  = defInst->getSrc(2);
  SSATmp* nArgs     = defInst->getSrc(3);
  SSATmp* magicName = defInst->getSrc(4);

  DEBUG_ONLY bool setThis = true;

  auto spReg = sp->getReg();
  // actRec->m_this
  if (objOrCls->isA(Type::Cls)) {
    // store class
    if (objOrCls->isConst()) {
      m_as.store_imm64_disp_reg64(uintptr_t(objOrCls->getValClass()) | 1,
                                  spOffset + AROFF(m_this),
                                  spReg);
    } else {
      Reg64 clsPtrReg = objOrCls->getReg();
      m_as.movq  (clsPtrReg, rScratch);
      m_as.orq   (1, rScratch);
      m_as.storeq(rScratch, spReg[spOffset + AROFF(m_this)]);
    }
  } else if (objOrCls->isA(Type::Obj)) {
    // store this pointer
    m_as.store_reg64_disp_reg64(objOrCls->getReg(),
                                spOffset + AROFF(m_this),
                                spReg);
  } else if (objOrCls->isA(Type::Ctx)) {
    // Stores either a this pointer or a Cctx -- statically unknown.
    Reg64 objOrClsPtrReg = objOrCls->getReg();
    m_as.storeq(objOrClsPtrReg, spReg[spOffset + AROFF(m_this)]);
  } else {
    assert(objOrCls->isA(Type::InitNull));
    // no obj or class; this happens in FPushFunc
    int offset_m_this = spOffset + AROFF(m_this);
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
  assert(magicName->isConst());
  // ActRec::m_invName is encoded as a pointer with bit kInvNameBit
  // set to distinguish it from m_varEnv and m_extrArgs
  uintptr_t invName =
    (magicName->getType().isNull()
      ? 0
      : (uintptr_t(magicName->getValStr()) | ActRec::kInvNameBit));
  m_as.store_imm64_disp_reg64(invName,
                              spOffset + AROFF(m_invName),
                              spReg);
  // actRec->m_func  and possibly actRec->m_cls
  // Note m_cls is unioned with m_this and may overwrite previous value
  if (func->getType().isNull()) {
    assert(func->isConst());
  } else if (func->isConst()) {
    // TODO: have register allocator materialize constants
    const Func* f = func->getValFunc();
    m_as. mov_imm64_reg((uint64_t)f, rScratch);
    m_as.store_reg64_disp_reg64(rScratch,
                                spOffset + AROFF(m_func),
                                spReg);
    if (func->isA(Type::FuncCtx)) {
      // Fill in m_cls if provided with both func* and class*
      CG_PUNT(cgAllocActRec);
    }
  } else {
    int offset_m_func = spOffset + AROFF(m_func);
    m_as.store_reg64_disp_reg64(func->getReg(0),
                                offset_m_func,
                                spReg);
    if (func->isA(Type::FuncCtx)) {
      int offset_m_cls = spOffset + AROFF(m_cls);
      m_as.store_reg64_disp_reg64(func->getReg(1),
                                  offset_m_cls,
                                  spReg);
      setThis = true; /* m_this and m_cls are in a union */
    }
  }
  assert(setThis);
  // actRec->m_savedRbp
  m_as.store_reg64_disp_reg64(fp->getReg(),
                              spOffset + AROFF(m_savedRbp),
                              spReg);

  // actRec->m_numArgsAndCtorFlag
  assert(nArgs->isConst());
  m_as.store_imm32_disp_reg(nArgs->getValInt(),
                            spOffset + AROFF(m_numArgsAndCtorFlag),
                            spReg);
}

HOT_FUNC_VM static ActRec*
cgNewInstanceHelper(Class* cls,
                    int numArgs,
                    Cell* sp,
                    ActRec* prevAr) {
  Cell* obj = sp - 1; // this is where the newly allocated object will go
  ActRec* ar = (ActRec*)(uintptr_t(obj) - sizeof(ActRec));

  Instance* newObj = newInstanceHelper(cls, numArgs, ar, prevAr);
  // store obj into the stack
  obj->m_data.pobj = newObj;
  obj->m_type = KindOfObject;
  return ar;
}

HOT_FUNC_VM static ActRec*
cgNewInstanceHelperCached(CacheHandle cacheHandle,
                          const StringData* clsName,
                          int numArgs,
                          Cell* sp,
                          ActRec* prevAr) {
  Cell* obj = sp - 1; // this is where the newly allocated object will go
  ActRec* ar = (ActRec*)(uintptr_t(obj) - sizeof(ActRec));

  Instance* newObj = newInstanceHelperCached((Class**)handleToPtr(cacheHandle),
                                             clsName,
                                             numArgs,
                                             ar,
                                             prevAr);
  // store obj into the stack
  obj->m_data.pobj = newObj;
  obj->m_type = KindOfObject;
  return ar;
}

void CodeGenerator::cgNewObj(IRInstruction* inst) {
  SSATmp* dst   = inst->getDst();
  SSATmp* numParams = inst->getSrc(0);
  SSATmp* clsName = inst->getSrc(1);
  SSATmp* sp    = inst->getSrc(2);
  SSATmp* fp    = inst->getSrc(3);

  if (clsName->isString()) {
    const StringData* classNameString = clsName->getValStr();
    CacheHandle ch = allocKnownClass(classNameString);
    ArgGroup args;
    args.imm(ch)
      .ssa(clsName)
      .ssa(numParams)
      .ssa(sp)
      .ssa(fp);
    cgCallHelper(m_as, (TCA)cgNewInstanceHelperCached, dst, kSyncPoint, args);
  } else {
    ArgGroup args;
    args.ssa(clsName).ssa(numParams).ssa(sp).ssa(fp);
    cgCallHelper(m_as, (TCA)cgNewInstanceHelper, dst, kSyncPoint, args);
  }
}

void CodeGenerator::cgCall(IRInstruction* inst) {
  SSATmp* actRec         = inst->getSrc(0);
  SSATmp* returnBcOffset = inst->getSrc(1);
  SSATmp* func           = inst->getSrc(2);
  SrcRange args          = inst->getSrcs().subpiece(3);
  int32_t numArgs          = args.size();

  auto spReg = actRec->getReg();
  // put all outgoing arguments onto the VM stack
  int64_t adjustment = (-(int64_t)numArgs) * sizeof(Cell);
  for (int32_t i = 0; i < numArgs; i++) {
    cgStore(spReg, -(i + 1) * sizeof(Cell), args[i]);
  }
  // store the return bytecode offset into the outgoing actrec
  uint64_t returnBc = returnBcOffset->getValInt();
  m_as.store_imm32_disp_reg(returnBc, AROFF(m_soff), spReg);
  if (adjustment != 0) {
    m_as.add_imm32_reg64(adjustment, spReg);
  }

  assert(m_state.lastMarker);
  SrcKey srcKey = SrcKey(m_state.lastMarker->func, m_state.lastMarker->bcOff);
  bool isImmutable = (func->isConst() && !func->getType().isNull());
  const Func* funcd = isImmutable ? func->getValFunc() : nullptr;
  assert(&m_as == &m_tx64->getAsm());
  int32_t adjust = m_tx64->emitBindCall(srcKey, funcd, numArgs);
  if (adjust) {
    m_as.addq (adjust, rVmSp);
  }
}

void CodeGenerator::cgCastStk(IRInstruction *inst) {
  Type type      = inst->getTypeParam();
  SSATmp* sp     = inst->getSrc(0);
  SSATmp* off    = inst->getSrc(1);
  uint32_t offset  = off->getValInt();
  PhysReg spReg  = sp->getReg();

  ArgGroup args;
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
               kSyncPoint, args);
}

void CodeGenerator::cgCallBuiltin(IRInstruction* inst) {
  SSATmp* f             = inst->getSrc(0);
  auto args             = inst->getSrcs().subpiece(1);
  int32_t numArgs         = args.size();
  SSATmp* dst           = inst->getDst();
  auto dstReg           = dst->getReg(0);
  auto dstType          = dst->getReg(1);
  Type returnType       = inst->getTypeParam();

  const Func* func = f->getValFunc();

  PhysReg returnBase = rsp;
  int returnOffset = HHIR_MISOFF(tvBuiltinReturn);

  // Load args into registers
  ArgGroup callArgs;
  callArgs.ssas(inst, 1, numArgs);

  // Call Builtin
  BuiltinFunction nativeFuncPtr = func->nativeFuncPtr();
  cgCallHelper(m_as,
              (TCA)nativeFuncPtr,
              dstReg,
              kSyncPoint,
              callArgs);

  if (dstReg == InvalidReg) {
    return;
  }
  // load return value from builtin
  // for primitive return types (int, bool, etc), the return value
  // is already in dstReg (the builtin call returns in rax). For return
  // by reference (String, Object, Array, etc), the builtin writes the
  // return value into MInstrState::tvBuiltinReturn TV, from where it
  // has to be tested and copied.
  if (returnType.isSimpleType()) {
    return;
  }
  if (returnType.isReferenceType()) {
    m_as.   loadq (returnBase[returnOffset], dstReg);
    emitLoadImm(m_as, returnType.toDataType(), dstType);
    emitLoadImm(m_as, KindOfNull, rScratch);
    m_as.   testq (dstReg, dstReg);
    m_as.   cmov_reg64_reg64 (CC_Z, rScratch, dstType);
    return;
  }
  if (returnType.subtypeOf(Type::Cell)
      || returnType.subtypeOf(Type::BoxedCell)) {
    m_as.   loadl (returnBase[returnOffset + TVOFF(m_type)], r32(dstType));
    m_as.   loadq (returnBase[returnOffset], dstReg);
    emitLoadImm(m_as, KindOfNull, rScratch);
    static_assert(KindOfUninit == 0, "CallBuiltin needs update for KindOfUninit");
    m_as.   testq  (dstType, dstType);
    m_as.   cmov_reg64_reg64 (CC_Z, rScratch, dstType);
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
  auto const dstReg       = dst->getReg();
  auto const spReg        = sp->getReg();
  auto const spillCells   = spillValueCells(inst);

  int64_t adjustment = (spDeficit - spillCells) * sizeof(Cell);
  for (uint32_t i = 0, cellOff = 0; i < numSpillSrcs; ++i) {
    const int64_t offset = cellOff * sizeof(Cell) + adjustment;
    if (spillVals[i]->getType() == Type::ActRec) {
      emitSpillActRec(sp, offset, spillVals[i]);
      cellOff += kNumActRecCells;
      i += kSpillStackActRecExtraArgs;
    } else {
      auto* val = spillVals[i];
      auto* inst = val->getInstruction();
      while (inst->isPassthrough()) {
        inst = inst->getPassthroughValue()->getInstruction();
      }
      // If our value came from a LdStack on the same sp and offset,
      // we don't need to spill it.
      if (inst->getOpcode() == LdStack && inst->getSrc(0) == sp &&
          inst->getSrc(1)->getValInt() * sizeof(Cell) == offset) {
        FTRACE(1, "{}: Not spilling spill value {} from {}\n",
               __func__, i, inst->toString());
      } else {
        cgStore(spReg, offset, val);
      }
      ++cellOff;
    }
  }
  if (adjustment != 0) {
    if (dstReg != spReg) {
      m_as.lea_reg64_disp_reg64(spReg, adjustment, dstReg);
    } else {
      m_as.add_imm32_reg64(adjustment, dstReg);
    }
  } else {
    emitMovRegReg(m_as, spReg, dstReg);
  }
}

void CodeGenerator::cgNativeImpl(IRInstruction* inst) {
  SSATmp* func  = inst->getSrc(0);
  SSATmp* fp    = inst->getSrc(1);

  assert(func->isConst());
  assert(func->getType() == Type::Func);

  BuiltinFunction builtinFuncPtr = func->getValFunc()->builtinFuncPtr();
  emitMovRegReg(m_as, fp->getReg(), argNumToRegName[0]);
  m_as.call((TCA)builtinFuncPtr);
  recordSyncPoint(m_as);
}

void CodeGenerator::cgLdThis(IRInstruction* inst) {
  SSATmp* dst   = inst->getDst();
  SSATmp* src   = inst->getSrc(0);
  Block* label = inst->getTaken();
  // mov dst, [fp + 0x20]
  auto dstReg = dst->getReg();

  // the destination of LdThis could be dead but the instruction
  // itself still useful because of the checks that it does (if it has
  // a label).  So we need to make sure there is a dstReg for this
  // instruction.
  if (dstReg != InvalidReg) {
    // instruction's result is not dead
    m_as.loadq(src->getReg()[AROFF(m_this)], dstReg);
  }
  if (label == NULL) return;  // no need to perform its checks
  if (dstReg != InvalidReg) {
    // test 0x01, dst
    m_as.testb(1, rbyte(dstReg));
  } else {
    m_as.testb(1, src->getReg()[AROFF(m_this)]);
  }
  // jnz label
  emitFwdJcc(CC_NZ, label);
}

static void emitLdClsCctx(CodeGenerator::Asm& a,
                          PhysReg srcReg,
                          PhysReg dstReg) {
  emitMovRegReg(a, srcReg, dstReg);
  a.    subq(1, dstReg);
}

void CodeGenerator::cgLdClsCtx(IRInstruction* inst) {
  PhysReg srcReg = inst->getSrc(0)->getReg();
  PhysReg dstReg = inst->getDst()->getReg();
  // Context could be either a this object or a class ptr
  m_as.   testb(1, rbyte(srcReg));
  ifThenElse(CC_NZ,
             [&] { emitLdClsCctx(m_as, srcReg, dstReg);  }, // ctx is a class
             [&] { emitLdObjClass(m_as, srcReg, dstReg); }  // ctx is this ptr
            );
}

void CodeGenerator::cgLdClsCctx(IRInstruction* inst) {
  PhysReg srcReg = inst->getSrc(0)->getReg();
  PhysReg dstReg = inst->getDst()->getReg();
  emitLdClsCctx(m_as, srcReg, dstReg);
}

void CodeGenerator::cgLdCtx(IRInstruction* inst) {
  PhysReg dstReg = inst->getDst()->getReg();
  PhysReg srcReg = inst->getSrc(0)->getReg();
  if (dstReg != InvalidReg) {
    m_as.loadq(srcReg[AROFF(m_this)], dstReg);
  }
}

void CodeGenerator::cgLdCctx(IRInstruction* inst) {
  return cgLdCtx(inst);
}

void CodeGenerator::cgLdConst(IRInstruction* inst) {
  auto const dstReg   = inst->getDst()->getReg();
  auto const val      = inst->getExtra<LdConst>()->as<uintptr_t>();
  if (dstReg == InvalidReg) return;
  emitLoadImm(m_as, val, dstReg);
}

void CodeGenerator::cgLdARFuncPtr(IRInstruction* inst) {
  SSATmp* dst   = inst->getDst();
  SSATmp* baseAddr = inst->getSrc(0);
  SSATmp* offset   = inst->getSrc(1);

  auto dstReg  = dst->getReg();
  auto baseReg = baseAddr->getReg();

  assert(offset->isConst());

  m_as.load_reg64_disp_reg64(baseReg,
                           offset->getValInt() + AROFF(m_func),
                           dstReg);
}

void CodeGenerator::cgLdContLocalsPtr(IRInstruction* inst) {
  auto rCont = inst->getSrc(0)->getReg();
  auto rLocals = inst->getDst()->getReg();
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

  Reg64 addrReg = addr->getReg();
  PhysReg destReg = dest->getReg();

  if (addr->isConst()) {
    addrReg = rScratch;
    emitLoadImm(m_as, addr->getValRawInt(), addrReg);
  }

  if (offset->isConst()) {
    assert(offset->getType() == Type::Int);
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
    int ldSize = getNativeTypeSize(dest->getType());
    Reg64 offsetReg = r64(offset->getReg());
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
  auto baseReg = inst->getSrc(0)->getReg();
  int64_t kind = inst->getSrc(1)->getValInt();
  SSATmp* value = inst->getSrc(2);

  RawMemSlot& slot = RawMemSlot::Get(RawMemSlot::Kind(kind));
  int stSize = slot.getSize();
  int64_t off = slot.getOffset();

  if (value->isConst()) {
    if (stSize == sz::qword) {
      m_as.store_imm64_disp_reg64(value->getValInt(),
                                  off,
                                  baseReg);
    } else if (stSize == sz::dword) {
      m_as.store_imm32_disp_reg(value->getValInt(),
                                  off,
                                  baseReg);
    } else {
      assert(stSize == sz::byte);
      m_as.store_imm8_disp_reg(value->getValBool(),
                               off,
                               baseReg);
    }
  } else {
    if (stSize == sz::qword) {
      m_as.store_reg64_disp_reg64(value->getReg(),
                                  off,
                                  baseReg);
    } else if (stSize == sz::dword) {
      m_as.store_reg32_disp_reg64(value->getReg(),
                                  off,
                                  baseReg);
    } else {
      // not supported by our assembler yet
      assert(stSize == sz::byte);
      not_implemented();
    }
  }
}

// If label is set and type is not Gen, this method generates a check
// that bails to the label if the loaded typed value doesn't match type.
void CodeGenerator::cgLoadTypedValue(PhysReg base,
                                     int64_t off,
                                     IRInstruction* inst) {
  Block* label = inst->getTaken();
  Type type   = inst->getTypeParam();
  SSATmp* dst = inst->getDst();

  assert(type == dst->getType());
  assert(type.needsReg());
  auto valueDstReg = dst->getReg(0);
  auto typeDstReg = dst->getReg(1);
  if (valueDstReg == InvalidReg && typeDstReg == InvalidReg &&
      (label == nullptr || type == Type::Gen)) {
    // a dead load
    return;
  }
  bool useScratchReg = (base == typeDstReg && valueDstReg != InvalidReg);
  if (useScratchReg) {
    // Save base to rScratch, because base will be overwritten.
    m_as.mov_reg64_reg64(base, reg::rScratch);
  }

  // Load type if it's not dead
  if (typeDstReg != InvalidReg) {
    m_as.load_reg64_disp_reg32(base, off + TVOFF(m_type), typeDstReg);
    if (label) {
      // Check type needed
      emitGuardType(r32(typeDstReg), inst);
    }
  } else if (label) {
    // Check type needed
    cgGuardTypeCell(base, off, inst);
  }

  // Load value if it's not dead
  if (valueDstReg == InvalidReg) return;

  if (useScratchReg) {
    m_as.load_reg64_disp_reg64(reg::rScratch, off + TVOFF(m_data), valueDstReg);
  } else {
    m_as.load_reg64_disp_reg64(base, off + TVOFF(m_data), valueDstReg);
  }
}

void CodeGenerator::cgStoreTypedValue(PhysReg base,
                                      int64_t off,
                                      SSATmp* src) {
  assert(src->getType().needsReg());
  m_as.store_reg64_disp_reg64(src->getReg(0),
                              off + TVOFF(m_data),
                              base);
  // store the type
  m_as.store_reg32_disp_reg64(src->getReg(1),
                              off + TVOFF(m_type),
                              base);
}

void CodeGenerator::cgStore(PhysReg base,
                            int64_t off,
                            SSATmp* src,
                            bool genStoreType) {
  Type type = src->getType();
  if (type.needsReg()) {
    cgStoreTypedValue(base, off, src);
    return;
  }
  // store the type
  if (genStoreType) {
    m_as.store_imm32_disp_reg(type.toDataType(),
                              off + TVOFF(m_type),
                              base);
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
    m_as.store_imm64_disp_reg64(val, off + TVOFF(m_data), base);
  } else {
    zeroExtendIfBool(m_as, src);
    m_as.store_reg64_disp_reg64(src->getReg(),
                                off + TVOFF(m_data),
                                base);
  }
}

void CodeGenerator::emitGuardOrFwdJcc(IRInstruction* inst, ConditionCode cc) {
  if (cc == CC_None) return;
  Block* label = inst->getTaken();
  if (inst && inst->getTCA() == kIRDirectGuardActive) {
    if (RuntimeOption::EvalDumpIR) {
      m_tx64->prepareForSmash(m_as, TranslatorX64::kJmpccLen);
      inst->setTCA(m_as.code.frontier);
    }
    // Get the SrcKey for the dest
    SrcKey  destSK(getCurFunc(), m_curTrace->getBcOff());
    SrcRec* destSR = m_tx64->getSrcRec(destSK);
    m_tx64->emitFallbackCondJmp(m_as, *destSR, cc);
  } else {
    emitFwdJcc(cc, label);
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
    cgGuardTypeCell(base, off, inst);
  }
  if (type.isNull()) return; // these are constants
  auto dstReg = inst->getDst()->getReg();
  // if dstReg == InvalidReg then the value of this load is dead
  if (dstReg == InvalidReg) return;

  if (type == Type::Bool) {
    m_as.load_reg64_disp_reg32(base, off + TVOFF(m_data),  dstReg);
  } else {
    m_as.load_reg64_disp_reg64(base, off + TVOFF(m_data),  dstReg);
  }
}

void CodeGenerator::cgLdProp(IRInstruction* inst) {
  cgLoad(inst->getSrc(0)->getReg(), inst->getSrc(1)->getValInt(), inst);
}

void CodeGenerator::cgLdMem(IRInstruction * inst) {
  cgLoad(inst->getSrc(0)->getReg(), inst->getSrc(1)->getValInt(), inst);
}

void CodeGenerator::cgLdRef(IRInstruction* inst) {
  cgLoad(inst->getSrc(0)->getReg(), 0, inst);
}

void CodeGenerator::recordSyncPoint(Asm& as,
                                    SyncOptions sync /* = kSyncPoint */) {
  assert(m_state.lastMarker);
  assert(sync != kNoSyncPoint);
  Offset stackOff = m_state.lastMarker->stackOff - (sync - kSyncPoint);
  Offset pcOff = m_state.lastMarker->bcOff - m_state.lastMarker->func->base();
  m_tx64->recordSyncPoint(as, pcOff, stackOff);
}

void CodeGenerator::cgLdAddr(IRInstruction* inst) {
  auto base = inst->getSrc(0)->getReg();
  int64_t offset = inst->getSrc(1)->getValInt();
  m_as.lea (base[offset], inst->getDst()->getReg());
}

void CodeGenerator::cgLdLoc(IRInstruction* inst) {
  cgLoad(inst->getSrc(0)->getReg(),
         getLocalOffset(inst->getExtra<LdLoc>()->locId),
         inst);
}

void CodeGenerator::cgLdLocAddr(IRInstruction* inst) {
  auto const fpReg  = inst->getSrc(0)->getReg();
  auto const offset = getLocalOffset(inst->getExtra<LdLocAddr>()->locId);
  if (inst->getDst()->hasReg()) {
    m_as.lea(fpReg[offset], inst->getDst()->getReg());
  }
}

void CodeGenerator::cgLdStackAddr(IRInstruction* inst) {
  auto base = inst->getSrc(0)->getReg();
  int64_t offset = cellsToBytes(inst->getSrc(1)->getValInt());
  m_as.lea (base[offset], inst->getDst()->getReg());
}

void CodeGenerator::cgLdStack(IRInstruction* inst) {
  assert(inst->getTaken() == nullptr);
  cgLoad(inst->getSrc(0)->getReg(),
         cellsToBytes(inst->getSrc(1)->getValInt()),
         inst);
}

void CodeGenerator::cgGuardStk(IRInstruction* inst) {
  cgGuardTypeCell(inst->getSrc(0)->getReg(),
                  cellsToBytes(inst->getSrc(1)->getValInt()),
                  inst);
}

void CodeGenerator::cgGuardLoc(IRInstruction* inst) {
  cgGuardTypeCell(inst->getSrc(0)->getReg(),
                  getLocalOffset(inst->getExtra<GuardLoc>()->locId),
                  inst);
}

void CodeGenerator::cgDefMIStateBase(IRInstruction* inst) {
  assert(inst->getDst()->getType() == Type::PtrToCell);
  assert(inst->getDst()->getReg() == rsp);
}

void CodeGenerator::cgGuardType(IRInstruction* inst) {
  Type      type  = inst->getTypeParam();
  SSATmp*   dst   = inst->getDst();
  SSATmp*   src   = inst->getSrc(0);
  Block*    label = inst->getTaken();
  auto dstReg = dst->getReg(0);
  auto srcValueReg = src->getReg(0);
  auto srcTypeReg = src->getReg(1);
  assert(srcTypeReg != InvalidReg);

  // compare srcTypeReg with type
  DataType dataType = type.toDataType();
  ConditionCode cc;
  if (IS_STRING_TYPE(dataType)) {
    m_as.test_imm32_reg32(KindOfStringBit, srcTypeReg);
    cc = CC_Z;
  } else {
    m_as.cmp_imm32_reg32(dataType, srcTypeReg);
    cc = CC_NE;
  }
  emitFwdJcc(cc, label);

  if (dstReg != InvalidReg) {
    emitMovRegReg(m_as, srcValueReg, dstReg);
  }
}

void CodeGenerator::cgGuardRefs(IRInstruction* inst) {
  assert(inst->getNumSrcs() == 6);

  SSATmp* funcPtrTmp = inst->getSrc(0);
  SSATmp* nParamsTmp = inst->getSrc(1);
  SSATmp* bitsPtrTmp = inst->getSrc(2);
  SSATmp* firstBitNumTmp = inst->getSrc(3);
  SSATmp* mask64Tmp  = inst->getSrc(4);
  SSATmp* vals64Tmp  = inst->getSrc(5);
  Block* exitLabel   = inst->getTaken();

  // Get values in place
  assert(funcPtrTmp->getType() == Type::Func);
  auto funcPtrReg = funcPtrTmp->getReg();
  assert(funcPtrReg != InvalidReg);

  assert(nParamsTmp->getType() == Type::Int);
  auto nParamsReg = nParamsTmp->getReg();
  assert(nParamsReg != InvalidReg);

  assert(bitsPtrTmp->getType() == Type::Int);
  auto bitsPtrReg = bitsPtrTmp->getReg();
  assert(bitsPtrReg != InvalidReg);

  assert(firstBitNumTmp->isConst() && firstBitNumTmp->getType() == Type::Int);
  uint32_t firstBitNum = (uint32_t)(firstBitNumTmp->getValInt());

  assert(mask64Tmp->getType() == Type::Int);
  assert(mask64Tmp->getInstruction()->getOpcode() == LdConst);
  auto mask64Reg = mask64Tmp->getReg();
  assert(mask64Reg != InvalidReg);
  int64_t mask64 = mask64Tmp->getValInt();

  assert(vals64Tmp->getType() == Type::Int);
  assert(vals64Tmp->getInstruction()->getOpcode() == LdConst);
  auto vals64Reg = vals64Tmp->getReg();
  assert(vals64Reg != InvalidReg);
  int64_t vals64 = vals64Tmp->getValInt();

  auto thenBody = [&] {
    auto bitsValReg = rScratch;
    //   Load the bit values in bitValReg:
    //     bitsValReg <- [bitsValPtr + (firstBitNum / 64)]
    m_as.load_reg64_disp_reg64(bitsPtrReg, sizeof(uint64_t) * (firstBitNum / 64),
                               bitsValReg);
    //     bitsValReg <- bitsValReg & mask64
    m_as.and_reg64_reg64(mask64Reg, bitsValReg);

    //   If bitsValReg != vals64Reg, then goto Exit
    m_as.cmp_reg64_reg64(bitsValReg, vals64Reg);
    emitFwdJcc(CC_NE, exitLabel);
  };

  // If few enough args...
  m_as.cmp_imm32_reg32(firstBitNum + 1, nParamsReg);
  if (vals64 == 0 && mask64 == 0) {
    ifThen(m_as, CC_NL, thenBody);
  } else if (vals64 != 0 && vals64 != mask64) {
    emitFwdJcc(CC_L, exitLabel);
    thenBody();
  } else if (vals64 != 0) {
    ifThenElse(CC_NL, thenBody, /* else */ [&] {
      //   If not special builtin...
      m_as.test_imm32_disp_reg32(AttrVariadicByRef, Func::attrsOff(), funcPtrReg);
      emitFwdJcc(CC_Z, exitLabel);
    });
  } else {
    ifThenElse(CC_NL, thenBody, /* else */ [&] {
      m_as.test_imm32_disp_reg32(AttrVariadicByRef, Func::attrsOff(), funcPtrReg);
      emitFwdJcc(CC_NZ, exitLabel);
    });
  }
}

void CodeGenerator::cgLdPropAddr(IRInstruction* inst) {
  SSATmp*   dst   = inst->getDst();
  SSATmp*   obj   = inst->getSrc(0);
  SSATmp*   prop  = inst->getSrc(1);

  assert(prop->isConst() && prop->getType() == Type::Int);

  auto dstReg = dst->getReg();
  auto objReg = obj->getReg();

  assert(objReg != InvalidReg);
  assert(dstReg != InvalidReg);

  int64_t offset = prop->getValInt();
  m_as.lea_reg64_disp_reg64(objReg, offset, dstReg);
}

void CodeGenerator::cgLdClsMethod(IRInstruction* inst) {
  SSATmp* dst   = inst->getDst();
  SSATmp* cls   = inst->getSrc(0);
  SSATmp* mSlot = inst->getSrc(1);

  assert(cls->getType() == Type::Cls);
  assert(mSlot->isConst() && mSlot->getType() == Type::Int);
  uint64_t mSlotInt64 = mSlot->getValRawInt();
  // We're going to multiply mSlotVal by sizeof(Func*) and use
  // it as a 32-bit offset (methOff) below.
  if (mSlotInt64 > (std::numeric_limits<uint32_t>::max() / sizeof(Func*))) {
    CG_PUNT(cgLdClsMethod_large_offset);
  }
  int32_t mSlotVal = (uint32_t) mSlotInt64;

  Reg64 dstReg = dst->getReg();
  assert(dstReg != InvalidReg);

  Reg64 clsReg = cls->getReg();
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
  auto funcDestReg  = dst->getReg(0);
  auto classDestReg = dst->getReg(1);

  assert(funcDestReg != InvalidReg && classDestReg != InvalidReg);
  // Attempt to retrieve the func* and class* from cache
  m_as.load_reg64_disp_reg64(rVmTl, ch, funcDestReg);
  m_as.load_reg64_disp_reg64(rVmTl,
                             ch + offsetof(TargetCache::StaticMethodCache,
                                           m_cls),
                             classDestReg);
  m_as.test_reg64_reg64(funcDestReg, funcDestReg);
  // May have retrieved a NULL from the cache
  // handle case where method is not entered in the cache
  unlikelyIfBlock(CC_E, [&] {
    if (false) { // typecheck
      const UNUSED Func* f = StaticMethodCache::lookupIR(ch, ne, cls, method);
    }
    // can raise an error if class is undefined
    cgCallHelper(m_astubs,
                 (TCA)StaticMethodCache::lookupIR,
                 funcDestReg,
                 kSyncPoint,
                 ArgGroup().imm(ch)            // Handle ch
                           .immPtr(ne)         // NamedEntity* np.second
                           .immPtr(cls)        // className
                           .immPtr(method)     // methodName
    );
    // recordInstrCall is done in cgCallHelper
    m_astubs.test_reg64_reg64(funcDestReg, funcDestReg);
    m_astubs.load_reg64_disp_reg64(rVmTl,
                                   ch + offsetof(TargetCache::StaticMethodCache,
                                                 m_cls),
                                   classDestReg);
    // if StaticMethodCache::lookupIR() returned NULL, jmp to label
    emitFwdJcc(m_astubs, CC_Z, label);
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
 * This method is similar to emitGetCtxFwdCallWithThis above,
 * but whether or not the callee is a static method is uknown at JIT time,
 * and that is determined dynamically by looking up into the StaticMethodFCache.
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
  PhysReg destCtxReg = inst->getDst()->getReg(0);
  SSATmp*  srcCtxTmp = inst->getSrc(0);
  const Func* callee = inst->getSrc(1)->getValFunc();
  bool      withThis = srcCtxTmp->isA(Type::Obj);

  // Eagerly move src into the dest reg
  emitMovRegReg(m_as, srcCtxTmp->getReg(0), destCtxReg);

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
  PhysReg         funcDestReg = inst->getDst()->getReg(0);
  PhysReg          destCtxReg = inst->getDst()->getReg(1);
  const Class*            cls = inst->getSrc(0)->getValClass();
  const StringData*  methName = inst->getSrc(1)->getValStr();
  SSATmp*           srcCtxTmp = inst->getSrc(2);
  PhysReg           srcCtxReg = srcCtxTmp->getReg(0);
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
  unlikelyIfBlock(CC_E, [&] {
    if (false) { // typecheck
      const UNUSED Func* f = StaticMethodFCache::lookupIR(ch, cls, methName);
    }
    cgCallHelper(m_astubs,
                 (TCA)StaticMethodFCache::lookupIR,
                 funcDestReg,
                 kSyncPoint,
                 ArgGroup().imm(ch)
                           .immPtr(cls)
                           .immPtr(methName));
    // If entry found in target cache, jump back to m_as.
    // Otherwise, bail to exit label
    m_astubs.testq(funcDestReg, funcDestReg);
    emitFwdJcc(m_astubs, CC_Z, exitLabel);
  });

  auto t = srcCtxTmp->getType();
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

  auto dstReg = dst->getReg();
  // Cls is live in the slow path call to lookupIR, so we have to be
  // careful not to clobber it before the branch to slow path. So
  // use the scratch register as a temporary destination if cls is
  // assigned the same register as the dst register.
  auto tmpReg = dstReg;
  if (dstReg == InvalidReg || dstReg == cls->getReg()) {
    tmpReg = PhysReg(rScratch);
  }

  // Could be optimized to cmp against zero when !label && dstReg == InvalidReg
  m_as.loadq(rVmTl[ch], tmpReg);
  m_as.testq(tmpReg, tmpReg);
  unlikelyIfBlock(CC_E, [&] {
    cgCallHelper(m_astubs,
                 target ? (TCA)SPropCache::lookupIR<false>
                        : (TCA)SPropCache::lookupIR<true>, // raise on error
                 tmpReg,
                 kSyncPoint, // could re-enter to initialize properties
                 ArgGroup().imm(ch).ssa(cls).ssa(propName).ssa(cxt));
    if (target) {
      m_astubs.testq(tmpReg, tmpReg);
      emitFwdJcc(m_astubs, CC_Z, target);
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
  auto dstReg = dst->getReg();
  if (dstReg == InvalidReg && target) {
    // result is unused but this instruction was not eliminated
    // because its essential
    dstReg = rScratch;
  }
  cgCallHelper(m_as,
               target ? (TCA)SPropCache::lookupSProp<false>
                      : (TCA)SPropCache::lookupSProp<true>, // raise on error
               dstReg,
               kSyncPoint, // could re-enter to initialize properties
               ArgGroup().ssa(cls).ssa(prop).ssa(cxt));
  if (target) {
    m_as.testq(dstReg, dstReg);
    emitFwdJcc(m_as, CC_Z, target);
  }
}

void CodeGenerator::cgLdCachedClass(IRInstruction* inst) {
  const StringData* classNameString = inst->getSrc(0)->getValStr();
  auto ch = TargetCache::allocKnownClass(classNameString);
  m_as.  loadq  (rVmTl[ch], inst->getDst()->getReg());
}

void CodeGenerator::cgLdClsCached(IRInstruction* inst) {
  SSATmp* dst = inst->getDst();
  SSATmp* className = inst->getSrc(0);
  // Note the redundancy with LdCachedClass above...
  const StringData* classNameString = className->getValStr();
  auto ch = TargetCache::allocKnownClass(classNameString);
  auto dstReg = dst->getReg();
  if (dstReg == InvalidReg) {
    m_as. cmpq   (0, rVmTl[ch]);
  } else {
    m_as.  loadq  (rVmTl[ch], dstReg);
    m_as.  testq  (dstReg, dstReg);
  }
  unlikelyIfBlock(CC_E, [&] {
    // Passing only two arguments to lookupKnownClass, since the
    // third is ignored in the checkOnly==false case.
    cgCallHelper(m_astubs,
                 (TCA)TargetCache::lookupKnownClass<false>,
                 dst,
                 kSyncPoint,
                 ArgGroup().addr(rVmTl, intptr_t(ch)).ssa(className));
  });
}

void CodeGenerator::cgLdCls(IRInstruction* inst) {
  SSATmp* dst       = inst->getDst();
  SSATmp* className = inst->getSrc(0);

  CacheHandle ch = ClassCache::alloc();
  cgCallHelper(m_as, (TCA)ClassCache::lookup, dst, kSyncPoint,
               ArgGroup().imm(ch).ssa(className));
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
  assert(cnsName->isConst() && cnsName->getType() == Type::StaticStr);
  assert(cls->isConst() && cls->getType() == Type::StaticStr);

  StringData* fullName = fullConstName(cls, cnsName);
  TargetCache::CacheHandle ch = TargetCache::allocClassConstant(fullName);

  auto rTvPtr = rScratch; // Cell* ptr to slot in target cache
  m_as.lea(rVmTl[ch], rTvPtr);

  ArgGroup args;
  args.reg(rTvPtr)
      .immPtr(Unit::GetNamedEntity(cls->getValStr()))
      .immPtr(cls->getValStr())
      .immPtr(cnsName->getValStr());

  cgCallHelper(m_as, TCA(TargetCache::lookupClassConstantTv),
               inst->getDst(), kSyncPoint, args);
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
  CArrRef arr = obj->o_toArray();
  int64_t res = ak_exist_string_helper(key, arr.get());
  return res;
}

HOT_FUNC_VM
static int64_t ak_exist_int_obj(int64_t key, ObjectData* obj) {
  CArrRef arr = obj->o_toArray();
  bool res = arr.get()->exists(key);
  return res;
}

void CodeGenerator::cgAKExists(IRInstruction* inst) {
  SSATmp* arr = inst->getSrc(0);
  SSATmp* key = inst->getSrc(1);

  if (key->getType().isNull()) {
    if (arr->isA(Type::Arr)) {
      cgCallHelper(m_as,
                   (TCA)ak_exist_string,
                   inst->getDst(),
                   kNoSyncPoint,
                   ArgGroup().immPtr(empty_string.get()).ssa(arr));
    } else {
      m_as.mov_imm64_reg(0, inst->getDst()->getReg());
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
               ArgGroup().ssa(key).ssa(arr));
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
  auto dstReg = inst->getDst()->getReg();
  cgCallHelper(m_as, (TCA)ldGblAddrHelper, dstReg, kNoSyncPoint,
               ArgGroup().ssa(inst->getSrc(0)));
  m_as.testq(dstReg, dstReg);
  emitFwdJcc(CC_Z, inst->getTaken());
}

void CodeGenerator::cgLdGblAddrDef(IRInstruction* inst) {
  cgCallHelper(m_as, (TCA)ldGblAddrDefHelper, inst->getDst(), kNoSyncPoint,
               ArgGroup().ssa(inst->getSrc(0)));
}

void CodeGenerator::cgJmpZeroHelper(IRInstruction* inst,
                                    ConditionCode cc) {
  SSATmp* src   = inst->getSrc(0);

  auto srcReg = src->getReg();
  if (src->isConst()) {
    bool valIsZero = src->getValRawInt() == 0;
    if ((cc == CC_Z  && valIsZero) ||
        (cc == CC_NZ && !valIsZero)) {
      // assert(false) here after new simplifier pass, t2019643
      // For now, materialize the test condition and use a Jcc
      m_as.xor_reg64_reg64(rScratch, rScratch);
      m_as.test_reg64_reg64(rScratch, rScratch);
      cc = CC_Z;
      // Update the instr opcode since cgExitTrace uses it
      // to determine correct cc for service request.
      inst->setOpcode(JmpZero);
    } else {
      // Fall through to next bytecode, disable DirectJmp
      inst->setTCA(kIRDirectJmpInactive);
      return;
    }
  } else {
    if (src->getType() == Type::Bool) {
      m_as.testb(Reg8(int(srcReg)), Reg8(int(srcReg)));
    } else {
      m_as.test_reg64_reg64(srcReg, srcReg);
    }
  }

  emitJccDirectExit(inst, cc);
}

void CodeGenerator::cgJmpZero(IRInstruction* inst) {
  cgJmpZeroHelper(inst, CC_Z);
}

void CodeGenerator::cgJmpNZero(IRInstruction* inst) {
  cgJmpZeroHelper(inst, CC_NZ);
}

void CodeGenerator::cgJmp_(IRInstruction* inst) {
  assert(inst->getNumSrcs() == inst->getTaken()->getLabel()->getNumDsts());
  if (unsigned n = inst->getNumSrcs()) {
    // Parallel-copy sources to the label's destination registers.
    // TODO: t2040286: this only works if all destinations fit in registers.
    SrcRange srcs = inst->getSrcs();
    DstRange dsts = inst->getTaken()->getLabel()->getDsts();
    ArgGroup args;
    for (unsigned i = 0, j = 0; i < n; i++) {
      assert(srcs[i]->getType().subtypeOf(dsts[i].getType()));
      SSATmp *dst = &dsts[i], *src = srcs[i];
      if (dst->getReg(0) == InvalidReg) continue; // dst is unused.
      // first dst register
      args.ssa(src);
      args[j++].setDstReg(dst->getReg(0));
      // second dst register, if any
      if (dst->numNeededRegs() == 2) {
        if (src->numNeededRegs() < 2) {
          // src has known data type, but dst doesn't - pass immediate type
          assert(src->getType().isKnownDataType());
          args.imm(src->getType().toDataType());
        } else {
          // pass src's second register
          assert(src->getReg(1) != InvalidReg);
          args.reg(src->getReg(1));
        }
        args[j++].setDstReg(dst->getReg(1));
      }
    }
    shuffleArgs(m_as, args);
  }
  emitFwdJmp(inst->getTaken());
}

void CodeGenerator::cgJmpIndirect(IRInstruction* inst) {
  m_as.jmp(inst->getSrc(0)->getReg());
}

void CodeGenerator::cgCheckInit(IRInstruction* inst) {
  Block* label = inst->getTaken();
  if (!label) {
    return;
  }
  SSATmp* src = inst->getSrc(0);

  // TODO: This optimization is redundant wrt simplifier. Remove it once we can
  // simplifier as a separate pass.
  Type type = src->getType();
  if (type.isInit()) {
    // Unnecessary CheckInit
    return;
  }

  auto typeReg = src->getReg(1);
  assert(label);
  assert(typeReg != InvalidReg);

  if (HPHP::KindOfUninit == 0) {
    m_as.test_reg32_reg32(typeReg, typeReg);
  } else {
    m_as.cmp_imm32_reg32(HPHP::KindOfUninit, typeReg);
  }
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

  auto fpReg = fp->getReg();
  m_as.    cmpq   (0, fpReg[AROFF(m_varEnv)]);
  emitFwdJcc(CC_NE, label);
}

void CodeGenerator::cgReleaseVVOrExit(IRInstruction* inst) {
  auto* const label = inst->getTaken();
  auto const rFp = inst->getSrc(0)->getReg();

  m_as.    cmpq   (0, rFp[AROFF(m_varEnv)]);
  unlikelyIfBlock(CC_NZ, [&] {
    m_astubs.    testl  (ActRec::kExtraArgsBit, rFp[AROFF(m_varEnv)]);
    emitFwdJcc(m_astubs, CC_Z, label);
    cgCallHelper(
      m_astubs,
      TCA(static_cast<void (*)(ActRec*)>(ExtraArgs::deallocate)),
      nullptr,
      kSyncPoint,
      ArgGroup().reg(rFp)
    );
  });
}

void CodeGenerator::cgBoxPtr(IRInstruction* inst) {
  SSATmp* dst  = inst->getDst();
  SSATmp* addr = inst->getSrc(0);
  auto base    = addr->getReg();
  auto dstReg  = dst->getReg();
  emitMovRegReg(m_as, base, dstReg);
  ConditionCode cc = emitTypeTest(Type::BoxedCell, base[TVOFF(m_type)], true);
  ifThen(m_as, cc, [&] {
    cgCallHelper(m_as, (TCA)tvBox, dstReg, kNoSyncPoint, ArgGroup().ssa(addr));
  });
}

void CodeGenerator::cgDefCns(IRInstruction* inst) {
  UNUSED SSATmp* dst     = inst->getDst();
  UNUSED SSATmp* cnsName = inst->getSrc(0);
  UNUSED SSATmp* val     = inst->getSrc(1);
  using namespace TargetCache;
  UNUSED CacheHandle ch = allocConstant((StringData*)cnsName->getValStr());
#if 0
  // ALIA:TODO
  // XXX second param is an inout pointer to a Ref, so we need to pass
  // the pointer to a stack slot
  if (RuntimeOption::RepoAuthoritative) {
    EMIT_CALL3(a, defCnsHelper<false>, IMM(ch), A(i.inputs[0]->location),
               IMM((uint64_t)name));
  } else {
    EMIT_CALL4(a, defCnsHelper<true>, IMM(ch), A(i.inputs[0]->location),
               IMM((uint64_t)name), IMM(allocCnsBit(name)));
  }
#endif

  CG_PUNT(DefCns);
}

// TODO: Kill this #2031980
static StringData* concat_value(TypedValue tv1, TypedValue tv2) {
  return concat(tv1.m_type, tv1.m_data.num, tv2.m_type, tv2.m_data.num);
}

void CodeGenerator::cgConcat(IRInstruction* inst) {
  SSATmp* dst   = inst->getDst();
  SSATmp* tl    = inst->getSrc(0);
  SSATmp* tr    = inst->getSrc(1);

  Type lType = tl->getType();
  Type rType = tr->getType();
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
                 ArgGroup().ssa(tl)
                           .ssa(tr));
  } else {
    if (lType.subtypeOf(Type::Obj) || lType.needsReg() ||
        rType.subtypeOf(Type::Obj) || rType.needsReg()) {
      CG_PUNT(cgConcat);
    }
    cgCallHelper(m_as, (TCA)concat_value, dst, kNoSyncPoint,
                 ArgGroup().valueType(tl).valueType(tr));
  }
}

void CodeGenerator::cgDefCls(IRInstruction* inst) {
  UNUSED SSATmp* dst   = inst->getDst();
  UNUSED SSATmp* preClass    = inst->getSrc(0);
  UNUSED SSATmp* opcodeAfter = inst->getSrc(1);

  // XXX we need to compute the stack ptr to pass to the m_defClsHelper function
  // in register rax
#if 0
  // ALIA:TODO
  /*
     compute the corrected stack ptr as a pseudo-param to m_defClsHelper
     which it will store in g_vmContext, in case of fatals, or __autoload
  */
  a.   lea_reg64_disp_reg64(rVmSp, -cellsToBytes(i.stackOff), rax);

  EMIT_CALL2(a, m_tx64->m_defClsHelper, IMM((uint64_t)c), IMM((uint64_t)after));
#endif
  CG_PUNT(DefCls);
}

void CodeGenerator::cgInterpOne(IRInstruction* inst) {
  SSATmp* fp = inst->getSrc(0);
  SSATmp* sp = inst->getSrc(1);
  SSATmp* pcOffTmp  = inst->getSrc(2);
  SSATmp* spAdjustmentTmp = inst->getSrc(3);
  Type resultType = inst->getTypeParam();
  Block* label = inst->getTaken();

  assert(pcOffTmp->isConst());
  assert(spAdjustmentTmp->isConst());
  assert(fp->getType() == Type::StkPtr);
  assert(sp->getType() == Type::StkPtr);

  int64_t pcOff = pcOffTmp->getValInt();

  void* interpOneHelper =
    interpOneEntryPoints[*(getCurFunc()->unit()->at(pcOff))];

  auto dstReg = InvalidReg;
  if (label) {
    dstReg = rScratch;
  }
  cgCallHelper(m_as, (TCA)interpOneHelper, dstReg, kSyncPoint,
               ArgGroup().ssa(fp).ssa(sp).imm(pcOff));
  if (label) {
    // compare the pc in the returned execution context with the
    // bytecode offset of the label
    Trace* targetTrace = label->getTrace();
    assert(targetTrace);
    uint32_t targetBcOff = targetTrace->getBcOff();
    // compare the pc with the target bc offset
    m_as.cmp_imm64_disp_reg64(targetBcOff,
                              offsetof(VMExecutionContext, m_pc),
                              dstReg);
//    emitFwdJcc(CC_E, label);
  }
  auto newSpReg = inst->getDst()->getReg();
  DEBUG_ONLY auto spReg = sp->getReg();
  int64_t spAdjustment = spAdjustmentTmp->getValInt();
  int64_t adjustment =
    (spAdjustment - (resultType == Type::None ? 0 : 1)) * sizeof(Cell);
  assert(newSpReg == spReg);
  if (adjustment != 0) {
    m_as.add_imm32_reg64(adjustment, newSpReg);
  }
}

void CodeGenerator::cgDefFunc(IRInstruction* inst) {
  SSATmp* dst   = inst->getDst();
  SSATmp* func  = inst->getSrc(0);
  cgCallHelper(m_as, (TCA)defFuncHelper, dst, kSyncPoint,
               ArgGroup().ssa(func));
}

void CodeGenerator::cgFillContThis(IRInstruction* inst) {
  SSATmp* cont = inst->getSrc(0);
  auto baseReg = inst->getSrc(1)->getReg();
  int64_t offset = inst->getSrc(2)->getValInt();
  auto scratch = rScratch;
  auto contReg = cont->getReg();

  m_as.loadq(contReg[CONTOFF(m_obj)], scratch);
  m_as.testq(scratch, scratch);
  ifThen(m_as, CC_NZ, [&] {
    m_as.addl(1, scratch[TVOFF(_count)]);
    m_as.storeq(scratch, baseReg[offset + TVOFF(m_data)]);
    m_as.storel(KindOfObject, baseReg[offset + TVOFF(m_type)]);
  });
}

void CodeGenerator::cgContEnter(IRInstruction* inst) {
  SSATmp* contAR = inst->getSrc(0);
  SSATmp* addr = inst->getSrc(1);
  SSATmp* returnOff = inst->getSrc(2);
  auto contARReg = contAR->getReg();

  m_as.  storel (returnOff->getValInt(), contARReg[AROFF(m_soff)]);
  m_as.  storeq (rVmFp, contARReg[AROFF(m_savedRbp)]);
  m_as.  movq   (contARReg, rStashedAR);

  m_as.  call   (addr->getReg());
}

void CodeGenerator::emitContVarEnvHelperCall(SSATmp* fp, TCA helper) {
  auto scratch = rScratch;

  m_as.  loadq (fp->getReg()[AROFF(m_varEnv)], scratch);
  m_as.  testq (scratch, scratch);
  unlikelyIfBlock(CC_NZ, [&] {
    cgCallHelper(m_astubs, helper, InvalidReg, kNoSyncPoint,
                 ArgGroup().ssa(fp));
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
                             cont->getReg());
  emitFwdJcc(CC_NZ, inst->getTaken());
}

void CodeGenerator::cgContPreNext(IRInstruction* inst) {
  auto contReg = inst->getSrc(0)->getReg();

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
                            inst->getSrc(0)->getReg());
  emitFwdJcc(CC_L, inst->getTaken());
}

void CodeGenerator::cgIterNextK(IRInstruction* inst) {
  cgIterNextCommon(inst, true);
}

void CodeGenerator::cgIterNext(IRInstruction* inst) {
  cgIterNextCommon(inst, false);
}

void CodeGenerator::cgIterNextCommon(IRInstruction* inst, bool isNextK) {
  PhysReg fpReg = inst->getSrc(0)->getReg();
  ArgGroup args;
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
  PhysReg fpReg = inst->getSrc(0)->getReg();
  int64_t  offset = getIterOffset(inst->getSrc(1));
  cgCallHelper(m_as, (TCA)iterFreeHelper, InvalidReg, kSyncPoint,
               ArgGroup().addr(fpReg, offset));
}

void CodeGenerator::cgIterInitK(IRInstruction* inst) {
  cgIterInitCommon(inst, true);
}

void CodeGenerator::cgIterInitCommon(IRInstruction* inst, bool isInitK) {
  PhysReg        fpReg = inst->getSrc(1)->getReg();
  int64_t     iterOffset = getIterOffset(inst->getSrc(2));
  int64_t valLocalOffset = getLocalOffset(inst->getSrc(3));
  SSATmp*          src = inst->getSrc(0);
  ArgGroup args;
  args.addr(fpReg, iterOffset).ssa(src);
  if (src->isArray()) {
    args.addr(fpReg, valLocalOffset);
    if (isInitK) {
      args.addr(fpReg, getLocalOffset(inst->getSrc(4)));
    }
    TCA helperAddr = isInitK ? (TCA)new_iter_array_key : (TCA)new_iter_array;
    cgCallHelper(m_as, helperAddr, inst->getDst(), kSyncPoint, args);
  } else {
    assert(src->getType() == Type::Obj);
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

void CodeGenerator::cgDbgAssertRefCount(IRInstruction* inst) {
  emitAssertRefCount(m_as, inst->getSrc(0)->getReg());
}

void traceCallback(ActRec* fp, Cell* sp, int64_t pcOff, void* rip) {
#if 0
  const Func* func = fp->m_func;
  std::cout << func->fullName()->data()
            << " " << pcOff
            << " " << rip
            << std::endl;
#endif
  checkFrame(fp, sp, true);
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

void patchJumps(Asm& as, CodegenState& state, Block* block) {
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
    if (inst->getOpcode() == Marker) {
      if (!m_state.firstMarkerSeen) {
        m_state.firstMarkerSeen = true;
        // This will be generated right after the tracelet guards
        if (RuntimeOption::EvalJitTransCounters && m_tx64 &&
            block->getTrace()->isMain()) {
          m_tx64->emitTransCounterInc(m_as);
        }
      }
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
    }
  }
}

void cgTrace(Trace* trace, Asm& amain, Asm& astubs, Transl::TranslatorX64* tx64,
             vector<TransBCMapping>* bcMap, CodegenState& state) {
  state.firstMarkerSeen = false;
  state.lastMarker = nullptr;
  TCA traceStart = amain.code.frontier;
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
    CodeGenerator cg(trace, *as, astubs, tx64, state);
    cg.cgBlock(block, bcMap);
    if (Block* next = block->getNext()) {
      // if there's a fallthrough block and it's not the next thing going
      // into this assembler, then emit a jump to it.
      if (it == end || next != *it || as != chooseAs(next)) {
        CodeGenerator::emitFwdJmp(*as, next, state);
      }
    }
    if (state.asmInfo) {
      state.asmInfo->asmRanges[block] = TcaRange(asmStart, as->code.frontier);
      if (as != &astubs) {
        state.asmInfo->astubRanges[block] = TcaRange(astubsStart,
                                                     astubs.code.frontier);
      }
    }
  }
  size_t UNUSED traceSize = amain.code.frontier - traceStart;
  TRACE(3, "[counter] %lu bytes of code generated\n", traceSize);
  if (trace->isMain()) {
    TRACE(3, "[counter] %lu bytes of code generated in main traces\n",
          traceSize);
  }
}

// select instructions for the trace and its exits
void genCodeForTrace(Trace* trace,
                     CodeGenerator::Asm& as,
                     CodeGenerator::Asm& astubs,
                     IRFactory* irFactory,
                     vector<TransBCMapping>* bcMap,
                     Transl::TranslatorX64* tx64,
                     AsmInfo* asmInfo) {
  assert(trace->isMain());
  CodegenState state(irFactory, asmInfo);
  cgTrace(trace, as, astubs, tx64, bcMap, state);
  for (Trace* exit : trace->getExitTraces()) {
    cgTrace(exit, astubs, astubs, tx64, nullptr, state);
  }
}

}}}
