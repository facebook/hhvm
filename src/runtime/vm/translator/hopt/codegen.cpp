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

#include <string.h>
#include "ir.h"
#include "linearscan.h"
#include "codegen.h"
#include "runtime/ext/ext_continuation.h"
#include "runtime/base/comparisons.h"
#include "runtime/base/complex_types.h"
#include "runtime/base/types.h"
#include "runtime/vm/bytecode.h"
#include "runtime/vm/runtime.h"
#include <runtime/base/runtime_option.h>
#include "runtime/base/string_data.h"
#include "runtime/base/array/hphp_array.h"
#include "runtime/vm/stats.h"
#include "runtime/vm/translator/types.h"
#include "runtime/vm/translator/translator.h"
#include "runtime/vm/translator/translator-x64.h"
#include "runtime/vm/translator/targetcache.h"
#include "runtime/vm/translator/translator-inline.h"
#include "runtime/vm/translator/x64-util.h"
#include "util/trace.h"

using HPHP::DataType;
using HPHP::TypedValue;
using HPHP::VM::Transl::TCA;

// emitDispDeref --
// emitDeref --
//
//   Helpers for common cell operations.
//
//   Dereference the var or home in the cell whose address lives in src
//   into dest.
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

using namespace Transl::reg;

static const HPHP::Trace::Module TRACEMOD = HPHP::Trace::tx64;

using Transl::rVmSp;
using Transl::rVmFp;

void cgPunt(const char* _file, int _line, const char* _func) {
  if (RuntimeOption::EvalDumpIR) {
    std::cout << "--------- CG_PUNT " << _file <<
                 "  " << _line << "  " << _func
                 << "\n";
  }
  throw FailedCodeGen(_file, _line, _func);
}

struct CycleInfo {
  int node;
  int length;
};

struct MoveInfo {
  enum Kind { Move, Xchg };

  MoveInfo(Kind kind, PhysReg reg1, PhysReg reg2):
      m_kind(kind), m_reg1(reg1), m_reg2(reg2) {}

  MoveInfo(Kind kind, int reg1, int reg2):
      m_kind(kind),
      m_reg1(reg1),
      m_reg2(reg2) {}

  Kind m_kind;
  PhysReg m_reg1, m_reg2;
};

template <int N>
void doRegMoves(int (&moves)[N], int rTmp,
                std::vector<MoveInfo>& howTo) {
  ASSERT(howTo.empty());
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
        ASSERT(false);
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

ArgDesc::ArgDesc(SSATmp* tmp, bool val) : m_imm(-1) {
  if (tmp->getInstruction()->isDefConst()) {
    m_srcReg = InvalidReg;
    if (val) {
      m_imm = tmp->getConstValAsBits();
    } else {
      m_imm = Type::toDataType(tmp->getType());
    }
    m_kind = Imm;
    return;
  }
  if (tmp->getType() == Type::Null || tmp->getType() == Type::Uninit) {
    m_srcReg = InvalidReg;
    if (val) {
      m_imm = 0;
    } else {
      m_imm = Type::toDataType(tmp->getType());
    }
    m_kind = Imm;
    return;
  }
  if (val || tmp->numNeededRegs() > 1) {
    auto reg = tmp->getReg(val ? 0 : 1);
    ASSERT(reg != InvalidReg);
    m_imm = 0;
    m_kind = Reg;
    m_srcReg = reg;
    return;
  }
  m_srcReg = InvalidReg;
  m_imm = Type::toDataType(tmp->getType());
  m_kind = Imm;
}

Address ArgDesc::genCode(CodeGenerator::Asm& a) const {
  Address start = a.code.frontier;
  switch (m_kind) {
    case Reg:
      a.    movq   (m_srcReg, m_dstReg);
      TRACE(3, "[counter] 1 reg move in ArgDesc::genCode\n");
      break;
    case Imm:
      emitImmReg(a, m_imm, m_dstReg);
      break;
    case Addr:
      a.    lea    (m_srcReg[m_imm.l()], m_dstReg);
      break;
  }
  return start;
}

void IRInstruction::genCode(CodeGenerator* codeGenerator) {
  m_asmAddr = codeGenerator->cgInst(this);
}

const Func* CodeGenerator::getCurrFunc() {
  return HPHP::VM::Transl::curFunc();
}

Address CodeGenerator::cgBreakpoint(X64Assembler &a) {
  Address start = a.code.frontier;
  a.int3();
  return start;
}

Address CodeGenerator::cgDefConst(IRInstruction* inst) {
  return NULL;
}

Address CodeGenerator::cgLdHome(IRInstruction* inst) {
  return NULL;
}

Address CodeGenerator::cgDefFP(IRInstruction* inst) {
  return NULL;
}

Address CodeGenerator::cgDefSP(IRInstruction* inst) {
  return NULL;
}

Address CodeGenerator::cgDefLabel(IRInstruction* inst) {
  LabelInstruction* label = (LabelInstruction*)inst;
  Address labelAddr = m_as.code.frontier;
  void* list = label->getPatchAddr();
  while (list != NULL) {
    int* toPatch   = (int*)list;
    int diffToNext = *toPatch;
    ssize_t diff = labelAddr - ((Address)list + 4);
    ASSERT(deltaFits(diff, sz::dword));
    *toPatch = (int)diff; // patch the jump address
    if (diffToNext == 0) {
      break;
    }
    void* next = (TCA)list - diffToNext;
    list = next;
  }
  return labelAddr;
}

Address CodeGenerator::cgMarker(IRInstruction* inst) {
  return cgDefLabel(inst);
}

Address CodeGenerator::cgJmpInstanceOfD(IRInstruction* inst) {
  CG_PUNT(JmpInstanceOfD);
  return NULL;
}

Address CodeGenerator::cgJmpNInstanceOfD(IRInstruction* inst) {
  CG_PUNT(JmpNInstanceOfD);
  return NULL;
}

Address CodeGenerator::cgJmpIsSet(IRInstruction* inst) {
  CG_PUNT(JmpIsSet);
  return NULL;
}

Address CodeGenerator::cgJmpIsType(IRInstruction* inst) {
  CG_PUNT(JmpIsType);
  return NULL;
}

Address CodeGenerator::cgJmpIsNSet(IRInstruction* inst) {
  CG_PUNT(JmpIsNSet);
  return NULL;
}

Address CodeGenerator::cgJmpIsNType(IRInstruction* inst) {
  CG_PUNT(JmpIsNType);
  return NULL;
}

Address CodeGenerator::cgNInstanceOfD(IRInstruction* inst) {
  CG_PUNT(NInstanceOfD);
  return NULL;
}

Address CodeGenerator::cgIsSet(IRInstruction* inst) {
  CG_PUNT(IsSet);
  return NULL;
}

Address CodeGenerator::cgIsNSet(IRInstruction* inst) {
  CG_PUNT(IsNSet);
  return NULL;
}

Address CodeGenerator::cgIsNType(IRInstruction* inst) {
  CG_PUNT(IsNType);
  return NULL;
}

Address CodeGenerator::cgLdThisNc(IRInstruction* inst) {
  CG_PUNT(LdThisNc);
  return NULL;
}

Address CodeGenerator::cgLdCurFuncPtr(IRInstruction* inst) {
  CG_PUNT(LdCurFuncPtr);
  return NULL;
}

Address CodeGenerator::cgLdFuncCls(IRInstruction* inst) {
  CG_PUNT(LdFuncCls);
  return NULL;
}

Address CodeGenerator::cgInst(IRInstruction* inst) {
  Opcode opc = inst->getOpcode();
  switch (opc) {
  // Autogenerate dispatch for each IR instruction in ir.h

#define OPC(name, flags)                        \
  case name : return cg ## name (inst);
  IR_OPCODES
#undef OPC

    default:
      std::cerr << "CodeGenerator: unimplemented support for opcode " <<
        opcodeName(opc) << '\n';
      ASSERT(0);
      return NULL;
  }
}

ConditionCode cmpOpToCC[JmpNSame - JmpGt + 1] = {
  CC_G,  // OpGt
  CC_GE, // OpGte
  CC_L,
  CC_LE,
  CC_E,
  CC_NE,
  CC_E,  // OpSame
  CC_NE  // OpNSame
};

Address CodeGenerator::emitFwdJcc(ConditionCode cc, LabelInstruction* label) {
  Address start = m_as.code.frontier;
  m_as.jcc(cc, m_as.code.frontier);
  TCA immPtr = m_as.code.frontier - 4;
  label->prependPatchAddr(immPtr);
  return start;
}

Address CodeGenerator::emitFwdJmp(Asm& as, LabelInstruction* label) {
  Address start = as.code.frontier;
  as.jmp(as.code.frontier);
  TCA immPtr = as.code.frontier - 4;
  label->prependPatchAddr(immPtr);
  return start;
}

// Patch with service request EMIT_BIND_JMP
Address CodeGenerator::emitSmashableFwdJmp(LabelInstruction* label,
                                           SSATmp* toSmash) {
  Address start = m_as.code.frontier;
  if (toSmash) {
    m_tx64->prepareForSmash(m_as, TranslatorX64::kJmpLen);
    Address tca = emitFwdJmp(label);
    toSmash->setTCA(tca);
    ASSERT(false);  // TODO looks like this path is unused
  } else {
    emitFwdJmp(label);
  }
  return start;
}

// Patch with servie request REQ_BIND_JMPCC_FIRST/SECOND
Address CodeGenerator::emitSmashableFwdJccAtEnd(ConditionCode cc,
                                              LabelInstruction* label,
                                              SSATmp* toSmash) {
  Address start = m_as.code.frontier;
  if (toSmash) {
    m_tx64->prepareForSmash(m_as, TranslatorX64::kJmpLen +
                                  TranslatorX64::kJmpccLen);
    Address tcaJcc = emitFwdJcc(cc, label);
    emitFwdJmp(label);
    toSmash->setTCA(tcaJcc);
  } else {
    emitFwdJcc(cc, label);
  }
  return start;
}

// Patch with service request REQ_BIND_JCC
Address CodeGenerator::emitSmashableFwdJcc(ConditionCode cc,
                                           LabelInstruction* label,
                                           SSATmp* toSmash) {
  Address start = m_as.code.frontier;
  ASSERT(toSmash);

  m_tx64->prepareForSmash(m_as, TranslatorX64::kJmpccLen);
  Address tcaJcc = emitFwdJcc(cc, label);
  toSmash->setTCA(tcaJcc);
  return start;
}

Address CodeGenerator::emitFwdJmp(LabelInstruction* label) {
  Address start = m_as.code.frontier;
  m_as.jmp(m_as.code.frontier);
  TCA immPtr = m_as.code.frontier - 4;
  label->prependPatchAddr(immPtr);
  return start;
}

Address CodeGenerator::cgJcc(IRInstruction* inst) {
  Address start = m_as.code.frontier;
  SSATmp* src1  = inst->getSrc(0);
  SSATmp* src2  = inst->getSrc(1);
  Opcode opc = inst->getOpcode();
  ConditionCode cc = cmpOpToCC[opc - JmpGt];
  LabelInstruction* label = inst->getLabel();
  Type::Tag src1Type = src1->getType();
  Type::Tag src2Type = src2->getType();

  // can't generate CMP instructions correctly for anything that isn't
  // a bool or an int, and we can't mix the two types because
  // -1 == true in PHP, but not in HHIR binary representation
  if (!((src1Type == Type::Int && src2Type == Type::Int) ||
        (src1Type == Type::Bool && src2Type == Type::Bool) ||
        (src1Type == Type::ClassRef && src2Type == Type::ClassRef))) {
    CG_PUNT(cgJcc);
  }
  if (src1Type == Type::ClassRef && src2Type == Type::ClassRef) {
    ASSERT(opc == JmpSame || opc == JmpNSame);
  }
  auto srcReg1 = src1->getReg();
  auto srcReg2 = src2->getReg();

  // Note: when both src1 and src2 are constants, we should transform the
  // branch into an unconditional jump earlier in the IR.
  if (src1->isConst()) {
    // TODO: use compare with immediate or make sure simplifier
    // canonicalizes this so that constant is src2
    srcReg1 = rScratch;
    m_as.mov_imm64_reg(src1->getConstValAsRawInt(), srcReg1);
  }
  if (src2->isConst()) {
    m_as.cmp_imm64_reg64(src2->getConstValAsRawInt(), srcReg1);
  } else {
    // Note the reverse syntax in the assembler.
    // This cmp will compute srcReg1 - srcReg2
    if (src1Type == Type::Bool) {
      m_as.    cmpb (Reg8(int(srcReg2)), Reg8(int(srcReg1)));
    } else {
      m_as.cmp_reg64_reg64(srcReg2, srcReg1);
    }
  }
  SSATmp* toSmash = inst->getTCA() == kIRDirectJccJmpActive ?
                                      inst->getDst() : NULL;
  emitSmashableFwdJccAtEnd(cc, label, toSmash);
  return start;
}

Address CodeGenerator::cgJmpGt(IRInstruction* inst) {
  return cgJcc(inst);
}

Address CodeGenerator::cgJmpGte(IRInstruction* inst) {
  return cgJcc(inst);
}

Address CodeGenerator::cgJmpLt(IRInstruction* inst) {
  return cgJcc(inst);
}

Address CodeGenerator::cgJmpLte(IRInstruction* inst) {
  return cgJcc(inst);
}

Address CodeGenerator::cgJmpEq(IRInstruction* inst) {
  return cgJcc(inst);
}

Address CodeGenerator::cgJmpNeq(IRInstruction* inst) {
  return cgJcc(inst);
}

Address CodeGenerator::cgJmpSame(IRInstruction* inst) {
  return cgJcc(inst);
}

Address CodeGenerator::cgJmpNSame(IRInstruction* inst) {
  return cgJcc(inst);
}

Address CodeGenerator::cgCallHelper(Asm& a,
                                    TCA addr,
                                    PhysReg dstReg,
                                    SyncOptions sync,
                                    ArgGroup& args) {
  ASSERT(int(args.size()) <= kNumRegisterArgs);
  Address start = a.code.frontier;

  // We don't want to include the dst register defined by this
  // instruction when saving the caller-saved registers.
  auto const regsToSave = (m_curInst->getLiveOutRegs() & kCallerSaved)
      .remove(PhysReg(dstReg));
  PhysRegSaverParity<1> regSaver(a, regsToSave);

  // Assign registers to the arguments
  for (size_t i = 0; i < args.size(); i++) {
    args[i].setDstReg(argNumToRegName[i]);
  }

  // First schedule arg moves
  for (size_t i = 0; i < args.size(); ++i) {
    // We don't support memory-to-register moves currently.
    ASSERT(args[i].getKind() == ArgDesc::Reg ||
           args[i].getKind() == ArgDesc::Imm);
  }
  // Handle register-to-register moves.
  int moves[kNumX64Regs];
  memset(moves, -1, sizeof moves);
  for (size_t i = 0; i < args.size(); ++i) {
    if (args[i].getKind() == ArgDesc::Reg) {
      moves[int(args[i].getDstReg())] = int(args[i].getSrcReg());
    }
  }
  std::vector<MoveInfo> howTo;
  doRegMoves(moves, int(reg::rScratch), howTo);
  for (size_t i = 0; i < howTo.size(); ++i) {
    if (howTo[i].m_kind == MoveInfo::Move) {
      a.    movq   (howTo[i].m_reg1, howTo[i].m_reg2);
    } else {
      a.    xchgq  (howTo[i].m_reg1, howTo[i].m_reg2);
    }
    if (m_curTrace->isMain()) {
      if (m_curInst->isNative()) {
        TRACE(3, "[counter] 1 reg move in cgCallHelper\n");
      }
    }
  }
  if (m_curInst->isNative()) {
    int numBetweenCaller = 0;
    for (size_t i = 0; i < howTo.size(); ++i) {
      if (kCallerSaved.contains(howTo[i].m_reg1) &&
          kCallerSaved.contains(howTo[i].m_reg2)) {
        ++numBetweenCaller;
      }
    }
    if (numBetweenCaller > 0) {
      TRACE(3, "[counter] %d moves are between caller-saved regs\n",
            numBetweenCaller);
    }
  }
  // Handle const-to-register moves.
  for (size_t i = 0; i < args.size(); ++i) {
    if (args[i].getKind() == ArgDesc::Imm) {
      a.    movq   (args[i].getImm(), args[i].getDstReg());
    }
  }

  // do the call; may use a trampoline
  m_tx64->emitCall(a, addr);

  // HHIR:TODO this only does required part of TranslatorX64::recordCallImpl()
  // Better to have improved SKTRACE'n by calling recordStubCall,
  // recordReentrantCall, or recordReentrantStubCall as appropriate
  if (sync == kSyncPoint) {
    recordSyncPoint(a);
  }
  // grab the return value if any
  if (dstReg != InvalidReg && dstReg != reg::rax) {
    a.mov_reg64_reg64(reg::rax, dstReg);
    if (m_curTrace->isMain()) {
      if (m_curInst->isNative()) {
        TRACE(3, "[counter] 1 reg move in cgCallHelper\n");
      }
    }
  }
  return start;
}

Address CodeGenerator::cgCallHelper(Asm& a,
                                    TCA addr,
                                    SSATmp* dst,
                                    SyncOptions sync,
                                    ArgGroup& args) {
  auto dstReg = dst == NULL ? InvalidReg : dst->getReg();
  return cgCallHelper(a, addr, dstReg, sync, args);
}


Address CodeGenerator::cgMov(IRInstruction* inst) {
  Address start = m_as.code.frontier;
  SSATmp* dst   = inst->getDst();
  SSATmp* src   = inst->getSrc(0);
  auto dstReg = dst->getReg();
  auto srcReg = src->getReg();
  if (dstReg != srcReg) {
    m_as.mov_reg64_reg64(srcReg, dstReg);
    if (m_curTrace->isMain()) {
      TRACE(3, "[counter] 1 reg move in cgMov\n");
    }
  }
  return start;
}

#define GEN_UNARY_INT_OP(INSTR, OPER) do {                            \
  if (src->getType() != Type::Int && src->getType() != Type::Bool) {  \
    ASSERT(0); CG_PUNT(INSTR);                                        \
  }                                                                   \
  auto dstReg = dst->getReg();                                        \
  auto srcReg = src->getReg();                                        \
  ASSERT(dstReg != InvalidReg);                                       \
  /* Integer operations require 64-bit representations */             \
  if (src->getType() == Type::Bool && !src->isConst()) {              \
    m_as.and_imm64_reg64(0xff, srcReg);                               \
  }                                                                   \
  /* const source */                                                  \
  if (src->isConst()) {                                               \
    m_as.mov_imm64_reg(OPER src->getConstValAsRawInt(), dstReg);      \
    break;                                                            \
  }                                                                   \
  ASSERT(srcReg != InvalidReg);                                       \
  if (dstReg != srcReg) {                                             \
    m_as.mov_reg64_reg64(srcReg, dstReg);                             \
    if (m_curTrace->isMain()) {                                       \
      TRACE(3, "[counter] 1 reg move in UNARY_INT_OP\n");             \
    }                                                                 \
  }                                                                   \
  m_as. INSTR (r64(dstReg));                                          \
} while (0)

Address CodeGenerator::cgNotWork(SSATmp* dst, SSATmp* src) {
  Address start = m_as.code.frontier;

  GEN_UNARY_INT_OP(not, ~);

  return start;
}

Address CodeGenerator::cgNegateWork(SSATmp* dst, SSATmp* src) {
  Address start = m_as.code.frontier;

  GEN_UNARY_INT_OP(neg, -);

  return start;
}

Address CodeGenerator::cgNegate(IRInstruction* inst) {
  return cgNegateWork(inst->getDst(), inst->getSrc(0));
}

#define GEN_COMMUTATIVE_INT_OP(INSTR, OPER) do {                        \
  if (!(src1->getType() == Type::Int || src1->getType() == Type::Bool) || \
      !(src2->getType() == Type::Int || src2->getType() == Type::Bool)) { \
    CG_PUNT(INSTR);                                                     \
  }                                                                     \
  auto dstReg  = dst->getReg();                                         \
  auto src1Reg = src1->getReg();                                        \
  auto src2Reg = src2->getReg();                                        \
  /* Integer operations require 64-bit representations */               \
  if (src1->getType() == Type::Bool && !src1->isConst()) {              \
    m_as.and_imm64_reg64(0xff, src1Reg);                                \
  }                                                                     \
  if (src2->getType() == Type::Bool && !src2->isConst()) {              \
    m_as.and_imm64_reg64(0xff, src2Reg);                                \
  }                                                                     \
  /* 2 consts */                                                        \
  if (src1->isConst() && src2->isConst()) {                             \
    int64 src1Const = src1->getConstValAsRawInt();                      \
    int64 src2Const = src2->getConstValAsRawInt();                      \
    m_as.mov_imm64_reg(src1Const OPER src2Const, dstReg);               \
  /* 1 const, 1 reg */                                                  \
  } else if (src1->isConst() || src2->isConst()) {                      \
    int64 srcConst = (src1->isConst() ? src1 : src2)->getConstValAsRawInt(); \
    auto srcReg = (src1->isConst() ? src2Reg : src1Reg);                \
    if (srcReg == dstReg) {                                             \
      m_as. INSTR ## _imm64_reg64(srcConst, dstReg);                    \
    } else {                                                            \
    /* TODO: use lea when possible */                                   \
      m_as.mov_imm64_reg(srcConst, dstReg);                             \
      m_as. INSTR ##_reg64_reg64(srcReg, dstReg);                       \
    }                                                                   \
  /* both src1 and src2 are regs */                                     \
  } else {                                                              \
    if (dstReg != src1Reg && dstReg != src2Reg) {                       \
      m_as.mov_reg64_reg64(src1Reg, dstReg);                            \
      if (m_curTrace->isMain()) {                                       \
        TRACE(3, "[counter] 1 reg move in COMMUTATIVE_INT_OP\n");       \
      }                                                                 \
      m_as. INSTR ## _reg64_reg64(src2Reg, dstReg);                     \
    } else {                                                            \
      if (dstReg == src1Reg) {                                          \
        m_as. INSTR ## _reg64_reg64(src2Reg, dstReg);                   \
      } else {                                                          \
        ASSERT(dstReg == src2Reg);                                      \
        m_as. INSTR ## _reg64_reg64(src1Reg, dstReg);                   \
      }                                                                 \
    }                                                                   \
  }                                                                     \
  } while (0)

#define GEN_NON_COMMUTATIVE_INT_OP(INSTR, OPER) do {                    \
  if (!(src1->getType() == Type::Int || src1->getType() == Type::Bool) || \
      !(src2->getType() == Type::Int || src2->getType() == Type::Bool)) { \
    CG_PUNT(INSTR);                                                     \
  }                                                                     \
  auto dstReg  = dst->getReg();                                         \
  auto src1Reg = src1->getReg();                                        \
  auto src2Reg = src2->getReg();                                        \
  /* Integer operations require 64-bit representations */               \
  if (src1->getType() == Type::Bool && !src1->isConst()) {              \
    m_as.and_imm64_reg64(0xff, src1Reg);                                \
  }                                                                     \
  if (src2->getType() == Type::Bool && !src2->isConst()) {              \
    m_as.and_imm64_reg64(0xff, src2Reg);                                \
  }                                                                     \
  /* 2 consts */                                                        \
  if (src1->isConst() && src2->isConst()) {                             \
    int64 src1Const = src1->getConstValAsRawInt();                      \
    int64 src2Const = src2->getConstValAsRawInt();                      \
    m_as.mov_imm64_reg(src1Const OPER src2Const, dstReg);               \
  /* 1 const, 1 reg */                                                  \
  } else if (src2->isConst()) {                                         \
    int64 src2Const = src2->getConstValAsRawInt();                      \
    if (src1Reg != dstReg) {                                            \
      m_as.mov_reg64_reg64(src1Reg, dstReg);                            \
      if (m_curTrace->isMain()) {                                       \
        TRACE(3, "[counter] 1 reg move in NON_COMMUTATIVE_INT_OP\n");   \
      }                                                                 \
    }                                                                   \
    m_as. INSTR ## _imm64_reg64(src2Const, dstReg);                     \
  } else if (src1->isConst()) {                                         \
    int64 src1Const = src1->getConstValAsRawInt();                      \
    if (dstReg != src2Reg) {                                            \
      m_as.mov_imm64_reg(src1Const, dstReg);                            \
      m_as. INSTR ## _reg64_reg64(src2Reg, dstReg);                     \
    } else {                                                            \
      m_as.mov_imm64_reg(src1Const, reg::rScratch);                     \
      m_as. INSTR ## _reg64_reg64(src2Reg, reg::rScratch);              \
      m_as.mov_reg64_reg64(reg::rScratch, dstReg);                      \
      if (m_curTrace->isMain()) {                                       \
        TRACE(3, "[counter] 1 reg move in NON_COMMUTATIVE_INT_OP\n");   \
      }                                                                 \
    }                                                                   \
  /* both src1 and src2 are regs */                                     \
  } else {                                                              \
    if (dstReg != src1Reg && dstReg != src2Reg) {                       \
      m_as.mov_reg64_reg64(src1Reg, dstReg);                            \
      if (m_curTrace->isMain()) {                                       \
        TRACE(3, "[counter] 1 reg move in NON_COMMUTATIVE_INT_OP\n");   \
      }                                                                 \
      m_as. INSTR ## _reg64_reg64(src2Reg, dstReg);                     \
    } else {                                                            \
      if (dstReg == src1Reg) {                                          \
        m_as. INSTR ## _reg64_reg64(src2Reg, dstReg);                   \
      } else {                                                          \
        ASSERT(dstReg == src2Reg);                                      \
        m_as.mov_reg64_reg64(src1Reg, reg::rScratch);                   \
        if (m_curTrace->isMain()) {                                     \
          TRACE(3, "[counter] 1 reg move in NON_COMMUTATIVE_INT_OP\n"); \
        }                                                               \
        m_as. INSTR ## _reg64_reg64(src2Reg, reg::rScratch);            \
        m_as.mov_reg64_reg64(reg::rScratch, dstReg);                    \
        if (m_curTrace->isMain()) {                                     \
          TRACE(3, "[counter] 1 reg move in NON_COMMUTATIVE_INT_OP\n"); \
        }                                                               \
      }                                                                 \
    }                                                                   \
  }                                                                     \
  } while (0)

Address CodeGenerator::cgOpAdd(IRInstruction* inst) {
  Address start = m_as.code.frontier;
  SSATmp* dst   = inst->getDst();
  SSATmp* src1  = inst->getSrc(0);
  SSATmp* src2  = inst->getSrc(1);

  GEN_COMMUTATIVE_INT_OP(add, +);

  return start;
}

Address CodeGenerator::cgOpSub(IRInstruction* inst) {
  Address start = m_as.code.frontier;
  SSATmp* dst   = inst->getDst();
  SSATmp* src1  = inst->getSrc(0);
  SSATmp* src2  = inst->getSrc(1);

  if (src1->isConst() && src1->getConstValAsInt() == 0)
    return cgNegateWork(dst, src2);

  GEN_NON_COMMUTATIVE_INT_OP(sub, -);

  return start;
}

Address CodeGenerator::cgOpAnd(IRInstruction* inst) {
  Address start = m_as.code.frontier;
  SSATmp* dst   = inst->getDst();
  SSATmp* src1  = inst->getSrc(0);
  SSATmp* src2  = inst->getSrc(1);

  GEN_COMMUTATIVE_INT_OP(and, &);

  return start;
}

Address CodeGenerator::cgOpOr(IRInstruction* inst) {
  Address start = m_as.code.frontier;
  SSATmp* dst   = inst->getDst();
  SSATmp* src1  = inst->getSrc(0);
  SSATmp* src2  = inst->getSrc(1);

  GEN_COMMUTATIVE_INT_OP(or, |);

  return start;
}

Address CodeGenerator::cgOpXor(IRInstruction* inst) {
  SSATmp* dst   = inst->getDst();
  SSATmp* src1  = inst->getSrc(0);
  SSATmp* src2  = inst->getSrc(1);
  if (src2->isConst() && src2->getType() == Type::Int &&
      src2->getConstValAsInt() == ~0L) {
    return cgNotWork(dst, src1);
  }

  Address start = m_as.code.frontier;

  GEN_COMMUTATIVE_INT_OP(xor, ^);

  return start;
}

Address CodeGenerator::cgOpMul(IRInstruction* inst) {
  Address start = m_as.code.frontier;
  SSATmp* dst   = inst->getDst();
  SSATmp* src1  = inst->getSrc(0);
  SSATmp* src2  = inst->getSrc(1);

  GEN_COMMUTATIVE_INT_OP(imul, *);

  return start;
}

// Runtime helpers
int64 strToBoolHelper(const StringData *s) {
  return s->toBoolean();
}

int64 strToIntHelper(const StringData* s) {
  return s->toInt64();
}

int64 arrToBoolHelper(const ArrayData *a) {
  return a->size() != 0;
}

int64 objToBoolHelper(const ObjectData *o) {
  return o->o_toBoolean();
}

int64 cellToBoolHelper(DataType kind, Value value) {
  if (IS_NULL_TYPE(kind)) {
    return 0;
  }

  if (kind <= KindOfInt64) {
    return value.m_data.num ? 1 : 0;
  }

  switch (kind) {
    case KindOfDouble:  return value.m_data.dbl != 0;
    case KindOfStaticString:
    case KindOfString:  return value.m_data.pstr->toBoolean();
    case KindOfArray:   return value.m_data.parr->size() != 0;
    case KindOfObject:  return value.m_data.pobj->o_toBoolean();
    default:
      ASSERT(false);
      break;
  }
  return 0;
}

const StringData* intToStringHelper(int64 n) {
  // This returns a string with ref count of 0, so be sure
  // to incref it immediately, or dispose of it when done
  StringData* s = buildStringData(n);
  return s;
}

///////////////////////////////////////////////////////////////////////////////
// Comparison Operators
///////////////////////////////////////////////////////////////////////////////

#define DISPATCHER(name)                                                      \
  int64 ccmp_ ## name (StringData* a1, StringData* a2)                        \
    { return name(a1, a2); }                                                  \
  int64 ccmp_ ## name (StringData* a1, int64 a2)                              \
    { return name(a1, a2); }                                                  \
  int64 ccmp_ ## name (StringData* a1, ObjectData* a2)                        \
    { return name(a1, Object(a2)); }                                          \
  int64 ccmp_ ## name (ObjectData* a1, ObjectData* a2)                        \
    { return name(Object(a1), Object(a2)); }                                  \
  int64 ccmp_ ## name (ObjectData* a1, int64 a2)                              \
    { return name(Object(a1), a2); }                                          \
  int64 ccmp_ ## name (ArrayData* a1, ArrayData* a2)                          \
    { return name(Array(a1), Array(a2)); }

DISPATCHER(same)
DISPATCHER(equal)
DISPATCHER(more)
DISPATCHER(less)

#undef DISPATCHER

template <typename A, typename B>
inline int64 ccmp_nsame(A a, B b) { return !ccmp_same(a, b); }

template <typename A, typename B>
inline int64 ccmp_nequal(A a, B b) { return !ccmp_equal(a, b); }

template <typename A, typename B>
inline int64 ccmp_lte(A a, B b) { return !ccmp_more(a, b); }

template <typename A, typename B>
inline int64 ccmp_gte(A a, B b) { return !ccmp_less(a, b); }

#define CG_OP_CMP(inst, setter, name)                                         \
  cgOpCmpHelper(inst, &Asm:: setter, ccmp_ ## name, ccmp_ ## name,            \
                ccmp_ ## name, ccmp_ ## name, ccmp_ ## name, ccmp_ ## name)

// SRON - string, resource, object, or number
static bool typeIsSRON(Type::Tag t) {
  return Type::isString(t)
      || t == Type::Obj // encompases object and resource
      || t == Type::Int
      || t == Type::Dbl
      ;
}

Address CodeGenerator::cgOpCmpHelper(
          IRInstruction* inst,
          void (Asm::*setter)(Reg8),
          int64 (*str_cmp_str)(StringData*, StringData*),
          int64 (*str_cmp_int)(StringData*, int64),
          int64 (*str_cmp_obj)(StringData*, ObjectData*),
          int64 (*obj_cmp_obj)(ObjectData*, ObjectData*),
          int64 (*obj_cmp_int)(ObjectData*, int64),
          int64 (*arr_cmp_arr)( ArrayData*, ArrayData*)
        ) {
  Address start = m_as.code.frontier;
  SSATmp* dst   = inst->getDst();
  SSATmp* src1  = inst->getSrc(0);
  SSATmp* src2  = inst->getSrc(1);

  Type::Tag type1 = src1->getType();
  Type::Tag type2 = src2->getType();

  auto src1Reg = src1->getReg();
  auto src2Reg = src2->getReg();
  auto dstReg  = dst ->getReg();

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
  if (Type::isString(type1) && Type::isString(type2)) {
    ArgGroup args;
    args.ssa(src1).ssa(src2);
    cgCallHelper(m_as, (TCA)str_cmp_str,  dst, kSyncPoint, args);
  }

  /////////////////////////////////////////////////////////////////////////////
  // case 2: bool/null cmp anything
  // simplifyCmp has converted all args to bool
  else if (type1 == Type::Bool && type2 == Type::Bool) {
    if (src2->isConst()) {
      m_as.    cmpb (src2->getConstValAsBool(), Reg8(int(src1Reg)));
    } else {
      m_as.    cmpb (Reg8(int(src2Reg)), Reg8(int(src1Reg)));
    }
    (m_as.*setter)(rbyte(dstReg));
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
        m_as.cmp_imm64_reg64(src2->getConstValAsInt(), src1Reg);
      } else {
        m_as.cmp_reg64_reg64(src2Reg, src1Reg);
      }
      (m_as.*setter)(rbyte(dstReg));
    }

    else if (type1 == Type::Dbl || type2 == Type::Dbl) {
      CG_PUNT(cgOpCmpHelper_Dbl);
    }

    else if (Type::isString(type1)) {
      // string cmp string is dealy with in case 1
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

    // we should never get here
    else {
      CG_PUNT(cgOpCmpHelper_xx);
    }
  }

  /////////////////////////////////////////////////////////////////////////////
  // case 5: array cmp array
  else if (type1 == Type::Arr && type2 == Type::Arr) {
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

  return start;
}

Address CodeGenerator::cgOpEq(IRInstruction* inst) {
  return CG_OP_CMP(inst, sete, equal);
}

Address CodeGenerator::cgOpNeq(IRInstruction* inst) {
  return CG_OP_CMP(inst, setne, nequal);
}

Address CodeGenerator::cgOpSame(IRInstruction* inst) {
  return CG_OP_CMP(inst, sete, same);
}

Address CodeGenerator::cgOpNSame(IRInstruction* inst) {
  return CG_OP_CMP(inst, setne, nsame);
}

Address CodeGenerator::cgOpLt(IRInstruction* inst) {
  return CG_OP_CMP(inst, setl, less);
}

Address CodeGenerator::cgOpGt(IRInstruction* inst) {
  return CG_OP_CMP(inst, setg, more);
}

Address CodeGenerator::cgOpLte(IRInstruction* inst) {
  return CG_OP_CMP(inst, setle, lte);
}

Address CodeGenerator::cgOpGte(IRInstruction* inst) {
  return CG_OP_CMP(inst, setge, gte);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

Address CodeGenerator::cgIsType(IRInstruction* inst) {
  Address start = m_as.code.frontier;
  UNUSED Type::Tag type = ((TypeInstruction*)inst)->getSrcType();
  UNUSED SSATmp* dst = inst->getDst();
  UNUSED SSATmp* src = inst->getSrc(0);

  CG_PUNT(IsType);

  return start;
}

Address CodeGenerator::cgConv(IRInstruction* inst) {
  Type::Tag toType   = inst->getType();
  Type::Tag fromType = inst->getSrc(0)->getType();
  SSATmp* dst = inst->getDst();
  SSATmp* src = inst->getSrc(0);
  Address start = m_as.code.frontier;

  auto dstReg = dst->getReg();
  auto srcReg = src->getReg();

  bool srcIsConst = src->isConst();

  if (toType == Type::Int) {
    if (fromType == Type::Bool) {
      // Bool -> Int is just a move
      if (srcIsConst) {
        int64 constVal = src->getConstValAsRawInt();
        if (constVal == 0) {
          m_as.xor_reg64_reg64(dstReg, dstReg);
        } else {
          m_as.mov_imm64_reg(1, dstReg);
        }
      } else if (srcReg != dstReg) {
        m_as.movzbl (rbyte(srcReg), r32(dstReg));
      } else {
        // srcReg == dstReg
        m_as.and_imm64_reg64(0xff, dstReg);
      }
      return start;
    }
    if (Type::isString(fromType)) {
      if (src->isConst()) {
        auto val = src->getConstValAsStr()->toInt64();
        m_as.mov_imm64_reg(val, dstReg);
      } else {
        ArgGroup args;
        args.ssa(src);
        cgCallHelper(m_as, (TCA)strToIntHelper, dst, kNoSyncPoint, args);
      }
      return start;
    }
  }

  if (toType == Type::Bool) {
    if (fromType == Type::Null || fromType == Type::Uninit) {
      // Uninit/Null -> Bool (false)
      m_as.xor_reg64_reg64(dstReg, dstReg);
    } else if (fromType == Type::Bool) {
      // Bool -> Bool (nop!)
      if (srcIsConst) {
        int64 constVal = src->getConstValAsRawInt();
        if (constVal == 0) {
          m_as.xor_reg64_reg64(dstReg, dstReg);
        } else {
          m_as.mov_imm64_reg(1, dstReg);
        }
      } else if (srcReg != dstReg) {
        m_as.mov_reg64_reg64(srcReg, dstReg);
      }
    } else if (fromType == Type::Int) {
      // Int -> Bool
      if (src->isConst()) {
        int64 constVal = src->getConstValAsInt();
        if (constVal == 0) {
          m_as.xor_reg64_reg64(dstReg, dstReg);
        } else {
          m_as.mov_imm64_reg(1, dstReg);
        }
      } else if (dstReg == srcReg) {
        m_as.test_reg64_reg64(dstReg, dstReg);
        m_as.setne(rbyte(dstReg));
      } else {
        m_as.xor_reg64_reg64(dstReg, dstReg);
        m_as.test_reg64_reg64(srcReg, srcReg);
        m_as.setne(rbyte(dstReg));
      }
    } else {
      TCA helper = NULL;
      ArgGroup args;
      if (fromType == Type::Cell) {
        // Cell -> Bool
        args.type(src);
        helper = (TCA)cellToBoolHelper;
      } else if (Type::isString(fromType)) {
        // Str -> Bool
        helper = (TCA)strToBoolHelper;
      } else if (fromType == Type::Arr) {
        // Arr -> Bool
        helper = (TCA)arrToBoolHelper;
      } else if (fromType == Type::Obj) {
        // Obj -> Bool
        helper = (TCA)objToBoolHelper;
      } else {
        // Dbl -> Bool
        CG_PUNT(Conv_Dbl_Bool);
      }
      args.ssa(src);
      cgCallHelper(m_as, helper, dst, kNoSyncPoint, args);
    }
    return start;
  }

  if (Type::isString(toType)) {
    if (fromType == Type::Int) {
      // Int -> Str
      ArgGroup args;
      args.ssa(src);
      cgCallHelper(m_as, (TCA)intToStringHelper, dst, kNoSyncPoint, args);
    } else if (fromType == Type::Bool) {
      // Bool -> Str
      m_as.testb(Reg8(int(srcReg)), Reg8(int(srcReg)));
      m_as.mov_imm64_reg((uint64)StringData::GetStaticString(""),
                         dstReg);
      m_as.mov_imm64_reg((uint64)StringData::GetStaticString("1"),
                         rScratch);
      m_as.cmov_reg64_reg64(CC_NZ, rScratch, dstReg);
    } else {
      CG_PUNT(Conv_toString);
    }
    return start;
  }
  // TODO: Add handling of conversions
  CG_PUNT(Conv);

  return start;
}

Address CodeGenerator::cgInstanceOfD(IRInstruction* inst) {
  Address start = m_as.code.frontier;
  UNUSED SSATmp* dst   = inst->getDst();
  UNUSED SSATmp* src1  = inst->getSrc(0);
  UNUSED SSATmp* src2  = inst->getSrc(1);

  CG_PUNT(InstanceOfD);

  return start;
}

Address CodeGenerator::cgUnboxPtr(IRInstruction* inst) {
  Address start = m_as.code.frontier;
  SSATmp* dst   = inst->getDst();
  SSATmp* src   = inst->getSrc(0);

  auto srcReg = src->getReg();
  auto dstReg = dst->getReg();

  ASSERT(srcReg != InvalidReg);
  ASSERT(dstReg != InvalidReg);

  if (srcReg != dstReg) {
    m_as.mov_reg64_reg64(srcReg, dstReg);
  }
  emitDerefIfVariant(m_as, PhysReg(dstReg));
  return start;
}

Address CodeGenerator::cgUnbox(IRInstruction* inst) {
  Address start = m_as.code.frontier;
  SSATmp* dst   = inst->getDst();
  SSATmp* src   = inst->getSrc(0);
  LabelInstruction* typeFailLabel = inst->getLabel();
  bool genIncRef = true;

  if (typeFailLabel != NULL) {
    CG_PUNT(Unbox);
  }

  Type::Tag dstType = dst->getType();
  Type::Tag srcType = src->getType();

  ASSERT(!Type::isUnboxed(srcType));
  ASSERT(Type::isUnboxed(dstType));
  if (dstType == Type::Cell) {
    CG_PUNT(Unbox_Cell);
  }
  auto srcReg = src->getReg();
  auto dstReg = dst->getReg();
  if (Type::isBoxed(srcType)) {
    emitDeref(m_as, PhysReg(srcReg), PhysReg(dstReg));
  } else if (srcType == Type::Gen) {
    CG_PUNT(Unbox_Gen);
    // XXX The following is wrong becuase it over-writes srcReg
    emitDerefIfVariant(m_as, PhysReg(srcReg));
    m_as.mov_reg64_reg64(srcReg, dstReg);
    if (m_curTrace->isMain()) {
      TRACE(3, "[counter] 1 reg move in cgUnbox\n");
    }
  } else {
    ASSERT(false);
    CG_PUNT(Unbox);
  }
  if (genIncRef && Type::isRefCounted(dstType) &&
      dst->getReg() != InvalidReg) {
    cgIncRefWork(dstType, dst, dst);
  }
  return start;
}

Address CodeGenerator::cgLdFixedFunc(IRInstruction* inst) {
  // Note: We may not need 2 opcodes for LdFixedFunc versus
  // LdFunc. We can look whether methodName is a string constant
  // and generate different code here.
  Address start = m_as.code.frontier;
  SSATmp* dst   = inst->getDst();
  SSATmp* methodName = inst->getSrc(0);
  SSATmp* actRec     = inst->getSrc(1);

  ASSERT(methodName->isConst() && methodName->getType() == Type::StaticStr);
  auto dstReg = dst->getReg();
  if (dstReg == InvalidReg) {
    // happens if LdFixedFunc and FCall not in same trace
    dstReg = rScratch;
    dst->setReg(dstReg, 0);
  }
  auto actRecReg = actRec->getReg();
  ASSERT(actRecReg != InvalidReg);
  using namespace TargetCache;
  const StringData* name = methodName->getConstValAsStr();
  CacheHandle ch = allocFixedFunction(name);
  size_t funcCacheOff = ch + offsetof(FixedFuncCache, m_func);
  m_as.load_reg64_disp_reg64(rVmTl, funcCacheOff, dstReg);
  m_as.test_reg64_reg64(dstReg, dstReg);
  // jz off to the helper call in astubs
  m_as.jcc(CC_E, m_astubs.code.frontier);
  // this helper tries the autoload map, and fatals on failure
  cgCallHelper(m_astubs, (TCA)FixedFuncCache::lookupUnknownFunc,
               dst, kSyncPoint, ArgGroup().immPtr(name));
  m_astubs.jmp(m_as.code.frontier);
  // save func ptr in actrec
  m_as.store_reg64_disp_reg64(dstReg, AROFF(m_func), actRecReg);
  return start;
}

Address CodeGenerator::cgLdFunc(IRInstruction* inst) {
  Address start = m_as.code.frontier;
  SSATmp* dst   = inst->getDst();
  SSATmp* methodName = inst->getSrc(0);
  SSATmp* actRec     = inst->getSrc(1);

  auto actRecReg = actRec->getReg();
  ASSERT(actRecReg != InvalidReg);
  TargetCache::CacheHandle ch = TargetCache::FuncCache::alloc();
  auto dstReg = dst->getReg();
  if (dstReg == InvalidReg) {
    // this happens if LdFixedFunc and FCAll are not in the same trace
    // TODO: try to get rax instead to avoid a move after the call
    dstReg = rScratch;
  }
  // raises an error if function not found
  cgCallHelper(m_as, (TCA)FuncCache::lookup, dstReg, kSyncPoint,
               ArgGroup().imm(ch)
                         .ssa(methodName));
  m_as.store_reg64_disp_reg64(dstReg, AROFF(m_func), actRecReg);
  return start;
}

Address CodeGenerator::cgLdObjMethod(IRInstruction* inst) {
  SSATmp* dst        = inst->getDst();
  SSATmp* methodName = inst->getSrc(0);
  SSATmp* actRec     = inst->getSrc(1);

  CacheHandle ch = MethodCache::alloc();
  // raises an error if function not found
  Address start = cgCallHelper(m_as,
                               (TCA)MethodCache::lookup,
                               dst,
                               kSyncPoint,
                               ArgGroup().imm(ch)
                                         .ssa(actRec)
                                         .ssa(methodName));
  // TODO: Don't allocate a register to the destination of this
  // instruction as its no longer used
  return start;
}

Address CodeGenerator::cgLdObjClass(IRInstruction* inst) {
  Address start = m_as.code.frontier;
  SSATmp* dst   = inst->getDst();
  SSATmp* obj   = inst->getSrc(0);

  // TODO:MP assert copied from translatorx64. Update and make it work
  // ASSERT(obj->getType() == Type::Obj);
  auto dstReg = dst->getReg();
  auto objReg = obj->getReg();
  m_as.load_reg64_disp_reg64(objReg, ObjectData::getVMClassOffset(), dstReg);

  return start;
}

Address CodeGenerator::cgLdCachedClass(IRInstruction* inst) {
  Address start = m_as.code.frontier;
  SSATmp* dst   = inst->getDst();
  SSATmp* className = inst->getSrc(0);
  ASSERT(className->isConst() && className->getType() == Type::StaticStr);

  auto dstReg = dst->getReg();
  const StringData* classNameString = className->getConstValAsStr();
  TargetCache::allocKnownClass(classNameString);
  TargetCache::CacheHandle ch = TargetCache::allocKnownClass(classNameString);
  m_as.load_reg64_disp_reg64(rVmTl, ch, dstReg);

  return start;
}

Address CodeGenerator::cgRetVal(IRInstruction* inst) {
  Address start = m_as.code.frontier;
  SSATmp* dstSp = inst->getDst();
  SSATmp* fp    = inst->getSrc(0);
  SSATmp* val   = inst->getNumSrcs() > 1 ? inst->getSrc(1) : NULL;

  auto dstSpReg = dstSp->getReg();
  auto fpReg    =    fp->getReg();

  if (val) {
    // Store return value at the top of the caller's eval stack
    // (a) Store the type
    if (Type::isStaticallyKnown(val->getType())) {
      DataType valDataType = Type::toDataType(val->getType());
      m_as.store_imm32_disp_reg(valDataType, AROFF(m_r) + TVOFF(m_type), fpReg);
    } else {
      auto typeReg = val->getReg(1);
      m_as.store_reg32_disp_reg64(typeReg, AROFF(m_r) + TVOFF(m_type), fpReg);
    }

    // (b) Store the actual value (not necessary when storing Null)
    if (val->getType() != Type::Null) {
      if (val->getInstruction()->isDefConst()) {
        int64 intVal = val->getConstValAsRawInt();
        m_as.store_imm64_disp_reg64(intVal,  AROFF(m_r) + TVOFF(m_data), fpReg);
      } else {
        auto valReg = val->getReg();
        if (val->getType() == Type::Bool) {
          // BOOL BYTE
          m_as.and_imm64_reg64(0xff, valReg);
        }
        m_as.store_reg64_disp_reg64(valReg,  AROFF(m_r) + TVOFF(m_data), fpReg);
      }
    }
  }
  // Adjust Stack Pointer
  m_as.lea_reg64_disp_reg64 (fpReg, AROFF(m_r), dstSpReg);

  return start;
}

Address CodeGenerator::cgLdRetAddr(IRInstruction* inst) {
  Address start = m_as.code.frontier;
  auto fpReg = inst->getSrc(0)->getReg(0);
  ASSERT(fpReg != InvalidReg);
  m_as.push(fpReg[AROFF(m_savedRip)]);
  return start;
}

void checkStack(Cell* sp, int numElems) {
  Cell* firstSp = sp + numElems;
  for (Cell* c=sp; c < firstSp; c++) {
    TypedValue* tv = (TypedValue*)c;
    ASSERT(tvIsPlausible(tv));
    DataType t = tv->m_type;
    if (IS_REFCOUNTED_TYPE(t)) {
      ASSERT(tv->m_data.pstr->getCount() > 0);
    }
  }
}

void checkStackAR(Cell* sp, int numElems) {
  checkStack(sp+3, numElems); // skip over the actrec
}

Address CodeGenerator::emitCheckStack(CodeGenerator::Asm& as,
                                      SSATmp* sp,
                                      uint32 numElems,
                                      bool allocActRec) {
  if (allocActRec) {
    return cgCallHelper(as, (TCA)checkStackAR, InvalidReg, kNoSyncPoint,
                        ArgGroup().ssa(sp).imm(numElems));
  } else {
    return cgCallHelper(as, (TCA)checkStack, InvalidReg, kNoSyncPoint,
                        ArgGroup().ssa(sp).imm(numElems));
  }
}

void checkCell(Cell* base, uint32 index) {
  TypedValue* tv = (TypedValue*)(base + index);
  always_assert(tvIsPlausible(tv));
  DataType t = tv->m_type;
  if (IS_REFCOUNTED_TYPE(t)) {
    always_assert(tv->m_data.pstr->getCount() > 0);
  }
}

Address CodeGenerator::emitCheckCell(CodeGenerator::Asm& as,
                                     SSATmp* sp,
                                     uint32 index) {
  return cgCallHelper(as, (TCA)checkCell, InvalidReg, kNoSyncPoint,
                      ArgGroup().ssa(sp).imm(index));
}

void checkFrame(ActRec* fp, Cell* sp, bool checkLocals) {
  const Func* func = fp->m_func;
  if (fp->hasVarEnv()) {
    ASSERT(fp->getVarEnv()->getCfp() == fp);
  }
  // TODO: validate this pointer from actrec
  int numLocals = func->numLocals();
  DEBUG_ONLY Cell* firstSp = ((Cell*)fp) - func->numSlotsInFrame();
  ASSERT(sp <= firstSp || func->isGenerator());
  if (checkLocals) {
    int numParams = func->numParams();
    for (int i=0; i < numLocals; i++) {
      if (i >= numParams && func->isGenerator() && i < func->numNamedLocals()) {
        continue;
      }
      TypedValue* tv = frame_local(fp, i);
      ASSERT(tvIsPlausible(tv));
      DataType t = tv->m_type;
      if (IS_REFCOUNTED_TYPE(t)) {
        ASSERT(tv->m_data.pstr->getCount() > 0);
      }
    }
  }
  // We unfortunately can't do the same kind of check for the stack
  // because it may contain ActRecs.
#if 0
  for (Cell* c=sp; c < firstSp; c++) {
    TypedValue* tv = (TypedValue*)c;
    ASSERT(tvIsPlausible(tv));
    DataType t = tv->m_type;
    if (IS_REFCOUNTED_TYPE(t)) {
      ASSERT(tv->m_data.pstr->getCount() > 0);
    }
  }
#endif
}

void traceRet(ActRec* fp, Cell* sp, void* rip) {
  if (rip == TranslatorX64::Get()->getCallToExit()) {
    return;
  }
  checkFrame(fp, sp, false);
  checkStack(sp, 1); // check return value
}

void CodeGenerator::emitTraceRet(CodeGenerator::Asm& a) {
  // call to a trace function
  // ld return ip from native stack into rdx
  a.    loadq(MemoryRef(DispReg(rsp)), rdx);
  a.    movq (rVmFp, rdi);
  a.    movq (rVmSp, rsi);
  // do the call; may use a trampoline
  m_tx64->emitCall(a, TCA(traceRet));
}

Address CodeGenerator::cgRetCtrl(IRInstruction* inst) {
  Address start = m_as.code.frontier;
  SSATmp* sp    = inst->getSrc(0);
  SSATmp* fp    = inst->getSrc(1);

  // Make sure rVmFp and rVmSp are set appropriately
  if (sp->getReg() != rVmSp) {
    if (m_curTrace->isMain()) {
      TRACE(3, "[counter] 1 reg move in cgRetCtrl\n");
    }
    m_as.mov_reg64_reg64(sp->getReg(), rVmSp);
  }
  if (fp->getReg() != rVmFp) {
    if (m_curTrace->isMain()) {
      TRACE(3, "[counter] 1 reg move in cgRetCtrl\n");
    }
    m_as.mov_reg64_reg64(fp->getReg(), rVmFp);
  }

  if (RuntimeOption::EvalHHIRGenerateAsserts) {
    emitTraceRet(m_as);
  }
  // Return control to caller
  m_as.ret();
  m_as.ud2();
  return start;
}

Address CodeGenerator::cgFreeActRec(IRInstruction* inst) {
  Address start = m_as.code.frontier;
  SSATmp* outFp = inst->getDst();
  SSATmp* inFp  = inst->getSrc(0);

  auto  inFpReg =  inFp->getReg();
  auto outFpReg = outFp->getReg();

  m_as.load_reg64_disp_reg64(inFpReg, AROFF(m_savedRbp), outFpReg);

  return start;
}

Address CodeGenerator::cgAllocSpill(IRInstruction* inst) {
  Address start = m_as.code.frontier;
  SSATmp* numSlots = inst->getSrc(0);

  ASSERT(numSlots->isConst());
  int64 n = numSlots->getConstValAsInt();
  ASSERT(n >= 0 && n % 2 == 0);
  if (n > 0) {
    m_as.sub_imm32_reg64(sizeof(uint64) * n, reg::rsp);
  }
  return start;
}

Address CodeGenerator::cgFreeSpill(IRInstruction* inst) {
  Address start = m_as.code.frontier;
  SSATmp* numSlots = inst->getSrc(0);

  ASSERT(numSlots->isConst());
  int64 n = numSlots->getConstValAsInt();
  ASSERT(n >= 0 && n % 2 == 0);
  if (n > 0) {
    m_as.add_imm32_reg64(sizeof(uint64) * n, reg::rsp);
  }
  return start;
}

Address CodeGenerator::cgSpill(IRInstruction* inst) {
  Address start = m_as.code.frontier;
  SSATmp* dst   = inst->getDst();
  SSATmp* src   = inst->getSrc(0);

  ASSERT(dst->numNeededRegs() == src->numNeededRegs());
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
                                      sizeof(uint64) * sinfo.mem(),
                                      reg::rsp);
      break;
    }
  }
  return start;
}

Address CodeGenerator::cgReload(IRInstruction* inst) {
  Address start = m_as.code.frontier;
  SSATmp* dst   = inst->getDst();
  SSATmp* src   = inst->getSrc(0);

  ASSERT(dst->numNeededRegs() == src->numNeededRegs());
  for (int locIndex = 0; locIndex < src->numNeededRegs(); ++locIndex) {
    auto dstReg = dst->getReg(locIndex);

    auto sinfo = src->getSpillInfo(locIndex);
    switch (sinfo.type()) {
    case SpillInfo::MMX:
      m_as.    mov_mmx_reg64(sinfo.mmx(), dstReg);
      break;
    case SpillInfo::Memory:
      m_as.    load_reg64_disp_reg64(reg::rsp,
                                     sizeof(uint64) * sinfo.mem(),
                                     dstReg);
      break;
    }
  }
  return start;
}

Address CodeGenerator::cgStPropWork(IRInstruction* inst, bool genTypeStore) {
  Address start = m_as.code.frontier;
  SSATmp* obj   = inst->getSrc(0);
  SSATmp* prop  = inst->getSrc(1);
  SSATmp* src   = inst->getSrc(2);

  auto objReg = obj->getReg();
  if (prop->isConst()) {
    int64 offset = prop->getConstValAsInt();
    cgStore(objReg, offset, src, genTypeStore);
  } else {
    CG_PUNT(StProp);
  }
  return start;
}
Address CodeGenerator::cgStProp(IRInstruction* inst) {
  return cgStPropWork(inst, true);
}
Address CodeGenerator::cgStPropNT(IRInstruction* inst) {
  return cgStPropWork(inst, false);
}

Address CodeGenerator::cgStMemWork(IRInstruction* inst, bool genStoreType) {
  SSATmp* addr = inst->getSrc(0);
  SSATmp* offset  = inst->getSrc(1);
  SSATmp* src  = inst->getSrc(2);
  return cgStore(addr->getReg(), offset->getConstValAsInt(),
                 src, genStoreType);
}
Address CodeGenerator::cgStMem(IRInstruction* inst) {
  return cgStMemWork(inst, true);
}
Address CodeGenerator::cgStMemNT(IRInstruction* inst) {
  return cgStMemWork(inst, false);
}

Address CodeGenerator::cgStRefWork(IRInstruction* inst, bool genStoreType) {
  SSATmp* addr = inst->getSrc(0);
  SSATmp* src  = inst->getSrc(1);
  SSATmp* dest = inst->getDst();

  Address start = cgStore(addr->getReg(), 0, src, genStoreType);

  auto destReg = dest->getReg();
  auto addrReg = addr->getReg();

  if (destReg != InvalidReg && destReg != addrReg) {
    m_as.mov_reg64_reg64(addrReg, destReg);
    TRACE(3, "[counter] 1 reg move in cgStRefWork\n");
  }
  return start;
}

Address CodeGenerator::cgStRef(IRInstruction* inst) {
  return cgStRefWork(inst, true);
}
Address CodeGenerator::cgStRefNT(IRInstruction* inst) {
  return cgStRefWork(inst, false);
}

Address CodeGenerator::cgStLocWork(IRInstruction* inst, bool genStoreType) {
  SSATmp* addr = inst->getSrc(0);
  SSATmp* src  = inst->getSrc(1);

  Address start = m_as.code.frontier;
  // TODO: reuse getLocalRegOffset() here
  IRInstruction* addrInst = addr->getInstruction();
  ASSERT(addrInst->getOpcode() == LdHome);
  ConstInstruction* homeInstr = (ConstInstruction*)addrInst;
  auto baseReg = homeInstr->getSrc(0)->getReg();
  int64_t index = homeInstr->getLocal().getId();
  cgStore(baseReg, -((index + 1) * sizeof(Cell)), src, genStoreType);
  return start;
}
Address CodeGenerator::cgStLoc(IRInstruction* inst) {
  return cgStLocWork(inst, true);
}
Address CodeGenerator::cgStLocNT(IRInstruction* inst) {
  return cgStLocWork(inst, false);
}

Address CodeGenerator::cgExitTrace(IRInstruction* inst) {
  SSATmp* func = inst->getSrc(0);
  SSATmp* pc   = inst->getSrc(1);
  SSATmp* sp   = inst->getSrc(2);
  SSATmp* fp   = inst->getSrc(3);
  SSATmp* notTakenPC = NULL;
  SSATmp* toSmash = NULL;
  ASSERT(pc->isConst() && inst->getNumSrcs() <= 6);

  TraceExitType::ExitType exitType = getExitType(inst->getOpcode());
  if (exitType == TraceExitType::Normal && inst->getNumSrcs() == 5) {
    // Unconditional trace exit
    toSmash    = inst->getSrc(4);
    ASSERT(toSmash);
  } else if (exitType == TraceExitType::NormalCc) {
    // Exit at trace end  which is the target of a conditional branch
    notTakenPC = inst->getSrc(4);
    ASSERT(notTakenPC->isConst());
    if (inst->getNumSrcs() == 6) {
      toSmash    = inst->getSrc(5);
      ASSERT(toSmash);
    }
  }
  using namespace HPHP::VM::Transl;

  Asm& outputAsm = m_as; // Note: m_as is the same as m_atubs for Exit Traces,
  // unless exit trace was moved to end of main trace

  Address start = outputAsm.code.frontier;
  if (sp->getReg() != rVmSp) {
    if (m_curTrace->isMain()) {
      TRACE(3, "[counter] 1 reg move in cgExitTrace\n");
    }
    outputAsm.mov_reg64_reg64(sp->getReg(), rVmSp);
  }
  if (fp->getReg() != rVmFp) {
    if (m_curTrace->isMain()) {
      TRACE(3, "[counter] 1 reg move in cgExitTrace\n");
    }
    outputAsm.mov_reg64_reg64(fp->getReg(), rVmFp);
  }

  // Get the SrcKey for the dest
  SrcKey  destSK(func->getConstValAsFunc(), pc->getConstValAsInt());

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
        ConditionCode  cc  = cmpOpToCC[opc - JmpGt];
        uint64_t     taken = pc->getConstValAsInt();
        uint64_t  notTaken = notTakenPC->getConstValAsInt();

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
        m_tx64->emitBindJmp(outputAsm, destSK, REQ_BIND_JMP);
      }
      break;
    case TraceExitType::Normal:
      {
        TCA smashAddr = toSmash ? toSmash->getTCA() : NULL;
        if (smashAddr) {
          ASSERT(smashAddr != kIRDirectJmpInactive);
          if (smashAddr != kIRDirectJccJmpActive) {
            // kIRDirectJccJmpActive only needs NormalCc exit in astubs

            m_tx64->emitServiceReq(TranslatorX64::SRFlags::SRInline,
                                   REQ_BIND_JMP, 2,
                                   smashAddr,
                                   uint64_t(destSK.offset()));

          }
        } else {
          ASSERT(smashAddr == kIRDirectJmpInactive);
          m_tx64->emitBindJmp(outputAsm, destSK, REQ_BIND_JMP);
        }
      }
      break;
    case TraceExitType::Slow:
      m_tx64->emitBindJmp(outputAsm, destSK, REQ_BIND_JMP_NO_IR);
      break;
    case TraceExitType::SlowNoProgress:
      m_tx64->emitReqRetransNoIR(outputAsm, destSK);
      break;
    case TraceExitType::GuardFailure:
      SrcRec* destSR = m_tx64->getSrcRec(destSK);
      m_tx64->emitFallbackUncondJmp(outputAsm, *destSR);
      break;
  }
  return start;
}

Address CodeGenerator::cgExitTraceCc(IRInstruction* inst) {
  return cgExitTrace(inst);
}

Address CodeGenerator::cgExitSlow(IRInstruction* inst) {
  return cgExitTrace(inst);
}

Address CodeGenerator::cgExitSlowNoProgress(IRInstruction* inst) {
  return cgExitTrace(inst);
}

Address CodeGenerator::cgExitGuardFailure(IRInstruction* inst) {
  return cgExitTrace(inst);
}

static void emitAssertFlagsNonNegative(CodeGenerator::Asm& as) {
  TCA patch = as.code.frontier;
  as.jcc8(CC_GE, patch);
  as.int3();
  as.patchJcc8(patch, as.code.frontier);
}

static void emitAssertRefCount(CodeGenerator::Asm& as, PhysReg base) {
  as.cmp_imm32_disp_reg32(HPHP::RefCountStaticValue,
                          TVOFF(_count),
                          base);
  TCA patch = as.code.frontier;
  as.jcc8(CC_BE, patch);
  as.int3();
  as.patchJcc8(patch, as.code.frontier);
}

static void emitIncRef(CodeGenerator::Asm& as, PhysReg base) {
  if (RuntimeOption::EvalHHIRGenerateAsserts) {
    emitAssertRefCount(as, base);
  }
  // emit incref
  as.add_imm32_disp_reg32(1, TVOFF(_count), base);
  if (RuntimeOption::EvalHHIRGenerateAsserts) {
    // Assert that the ref count is greater than zero
    emitAssertFlagsNonNegative(as);
  }
}

Address CodeGenerator::cgIncRefWork(Type::Tag type, SSATmp* dst, SSATmp* src) {
  ASSERT(Type::isRefCounted(type));
  auto base = src->getReg(0);
  Address start = m_as.code.frontier;
  if (type == Type::Obj || Type::isBoxed(type)) {
    emitIncRef(m_as, base);
    auto dstReg = dst->getReg(0);
    if (dstReg != InvalidReg && dstReg != base) {
      m_as.mov_reg64_reg64(base, dstReg);
      if (m_curTrace->isMain()) {
        TRACE(3, "[counter] 1 reg move in cgIncRefWork\n");
      }
    }
    return start;
  } else {
    // Type::Cell, Type::Gen, Type::String, or Type::Arr
    TCA patch1 = NULL;
    // TODO: Should be able to merge Gen case with Cell
    if (type == Type::Gen) {
      CG_PUNT(cgIncRef_Gen); // for now
      // may be variant or cell
      m_as.cmp_imm32_disp_reg32(KindOfRefCountThreshold, TVOFF(m_type), base);
      patch1 = m_as.code.frontier;
      m_as.jcc8(CC_LE, patch1);
    }
    if (type == Type::Cell) {
      auto typ = src->getReg(1);
      m_as.cmp_imm32_reg32(KindOfRefCountThreshold, typ);
      patch1 = m_as.code.frontier;
      m_as.jcc8(CC_LE, patch1);
    }
    // check against static
    m_as.cmp_imm32_disp_reg32(RefCountStaticValue, TVOFF(_count), base);
    TCA patch2 = m_as.code.frontier;
    m_as.jcc8(CC_E, patch2);
    // emit incref
    emitIncRef(m_as, base);
    if (patch1) {
      m_as.patchJcc8(patch1, m_as.code.frontier);
    }
    m_as.patchJcc8(patch2, m_as.code.frontier);
    auto dstValueReg = dst->getReg(0);
    if (type == Type::Cell) {
      auto srcTypeReg = src->getReg(1);
      auto dstTypeReg = dst->getReg(1);
      // Be careful here. We need to move values from one pair
      // of registers to another.
      // dstValueReg = base
      // dstTypeReg = srcTypeReg
      if (dstValueReg != InvalidReg && base != dstValueReg) {
        if (srcTypeReg == dstValueReg) {
          // use the scratch reg to avoid clobbering srcTypeReg
          m_as.mov_reg64_reg64(srcTypeReg, rScratch);
          if (m_curTrace->isMain()) {
            TRACE(3, "[counter] 1 reg move in cgIncRefWork\n");
          }
          srcTypeReg = rScratch;
        }
        m_as.mov_reg64_reg64(base, dstValueReg);
        if (m_curTrace->isMain()) {
          TRACE(3, "[counter] 1 reg move in cgIncRefWork\n");
        }
      }
      if (dstTypeReg != InvalidReg && srcTypeReg != dstTypeReg) {
        m_as.mov_reg64_reg64(srcTypeReg, dstTypeReg);
        if (m_curTrace->isMain()) {
          TRACE(3, "[counter] 1 reg move in cgIncRefWork\n");
        }
      }
    } else {
      if (dstValueReg != InvalidReg && dstValueReg != base) {
        m_as.mov_reg64_reg64(base, dstValueReg);
        if (m_curTrace->isMain()) {
          TRACE(3, "[counter] 1 reg move in cgIncRefWork\n");
        }
      }
    }
  }
  return start;
}
Address CodeGenerator::cgIncRef(IRInstruction* inst) {
  Type::Tag type = inst->getType();
  SSATmp* dst    = inst->getDst();
  SSATmp* src    = inst->getSrc(0);

  if (m_curTrace->isMain()) {
    TRACE(3, "[counter] 1 IncRef in main traces\n");
  }
  return cgIncRefWork(type, dst, src);
}

Address CodeGenerator::cgDecRefStack(IRInstruction* inst) {
  Type::Tag type = inst->getType();
  SSATmp* sp = inst->getSrc(0);
  SSATmp* index= inst->getSrc(1);
  LabelInstruction* exit = inst->getLabel();
  return cgDecRefMem(type, sp->getReg(),
                     index->getConstValAsInt() * sizeof(Cell), exit);
}

Address CodeGenerator::cgDecRefThis(IRInstruction* inst) {
  Address start = m_as.code.frontier;
  SSATmp* fp    = inst->getSrc(0);
  LabelInstruction* exit = inst->getLabel();
  auto fpReg = fp->getReg();
  auto scratchReg = rScratch;

  // Load AR->m_this into rScratch
  m_as.load_reg64_disp_reg64(fpReg, AROFF(m_this), scratchReg);
  // Currently we need to store zero back to m_this in case a local
  // destructor does debug_backtrace.
  m_as.store_imm64_disp_reg64(0, AROFF(m_this), fpReg);

  // In pseudo-mains, emit check for presence of m_this
  TCA patch1 = NULL;
  if (curFunc()->isPseudoMain()) {
    m_as.test_reg64_reg64(scratchReg, scratchReg);
    patch1 = m_as.code.frontier;
    m_as.jcc8(CC_Z, patch1);
  }

  // Check if this is available and we're not in a static context instead
  m_as.test_imm32_reg64(1, scratchReg);
  TCA patch2 = m_as.code.frontier;
  m_as.jcc8(CC_NZ, patch2);

  cgDecRefStaticType(Type::Obj, scratchReg, exit, true);

  m_as.patchJcc8(patch2, m_as.code.frontier);
  if (patch1) {
    m_as.patchJcc8(patch1, m_as.code.frontier);
  }
  return start;
}

Address CodeGenerator::cgDecRefLoc(IRInstruction* inst) {
  Type::Tag type = inst->getType();
  SSATmp*   addr = inst->getSrc(0);
  LabelInstruction* exit = inst->getLabel(); // Can be NULL

  IRInstruction* addrInst = addr->getInstruction();

  ASSERT (addrInst->getOpcode() == LdHome);

  ConstInstruction* homeInstr = (ConstInstruction*)addrInst;
  auto fpReg = homeInstr->getSrc(0)->getReg();
  int64_t index = homeInstr->getLocal().getId();

  return cgDecRefMem(type, fpReg, -((index + 1) * sizeof(Cell)), exit);
}

static void
frame_free_locals(ActRec* fp, int numLocals) {
  ASSERT(Transl::tx64->stateIsDirty());
  using namespace Transl;
#ifdef DEBUG
  VMRegAnchor _;
  ASSERT(vmfp() == (Cell*)fp);
#endif
  // At return-time, we know that the eval stack is empty except
  // for the return value.
  TRACE(1, "frame_free_locals: updated fp to %p\n", fp);
  frame_free_locals_inl(fp, numLocals);
}

static void
frame_free_locals_no_this(ActRec* fp, int numLocals) {
  ASSERT(Transl::tx64->stateIsDirty());
  using namespace Transl;
#ifdef DEBUG
  VMRegAnchor _;
  ASSERT(vmfp() == (Cell*)fp);
#endif
  // At return-time, we know that the eval stack is empty except
  // for the return value.
  TRACE(1, "frame_free_locals_no_this: updated fp to %p\n", fp);
  frame_free_locals_no_this_inl(fp, numLocals);
}

Address CodeGenerator::cgDecRefLocals(IRInstruction* inst) {
  SSATmp* fp = inst->getSrc(0);
  SSATmp* numLocals = inst->getSrc(1);

  return cgCallHelper(m_as, (TCA)frame_free_locals_no_this, InvalidReg,
                      kSyncPoint, ArgGroup().ssa(fp).ssa(numLocals));
}

Address CodeGenerator::cgDecRefLocalsThis(IRInstruction* inst) {
  SSATmp* fp = inst->getSrc(0);
  SSATmp* numLocals = inst->getSrc(1);

  return cgCallHelper(m_as, (TCA)frame_free_locals, InvalidReg, kSyncPoint,
                      ArgGroup().ssa(fp).ssa(numLocals));
}

Address CodeGenerator::getDtor(DataType type) {
  switch (type) {
    case KindOfString  : return (Address)tv_release_str;
    case KindOfArray   : return (Address)tv_release_arr;
    case KindOfObject  : return (Address)tv_release_obj;
    case KindOfRef     : return (Address)tv_release_ref;
    default: not_reached();
  }
}

static void
tv_release_generic(TypedValue* tv) {
  ASSERT(VM::Transl::tx64->stateIsDirty());
  ASSERT(tv->m_type >= KindOfString && tv->m_type <= KindOfRef);
  g_destructors[typeToDestrIndex(tv->m_type)](tv->m_data.pref);
}

static void
tv_release_typed(RefData* pv, DataType dt) {
  ASSERT(VM::Transl::tx64->stateIsDirty());
  ASSERT(dt >= KindOfString && dt <= KindOfRef);
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
Address CodeGenerator::cgCheckStaticBit(Type::Tag type,
                                        PhysReg reg,
                                        bool regIsCount) {
  if (!Type::needsStaticBitCheck(type)) return NULL;

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
Address CodeGenerator::cgCheckStaticBitAndDecRef(Type::Tag type,
                                                 PhysReg dataReg,
                                                 LabelInstruction* exit) {
  ASSERT(Type::isRefCounted(type));

  Address patchStaticCheck = NULL;
  const auto scratchReg = rScratch;

  bool canUseScratch = dataReg != scratchReg;

  // TODO: run experiments to check whether the 'if' code sequence
  // is any better than the 'else' branch below; otherwise, always
  // use the 'else' code sequence
  if (Type::needsStaticBitCheck(type) && canUseScratch) {
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
                                             int64 offset) {

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
Address CodeGenerator::cgDecRefStaticType(Type::Tag type,
                                          PhysReg dataReg,
                                          LabelInstruction* exit,
                                          bool genZeroCheck) {
  ASSERT(type != Type::Cell && type != Type::Gen);

  if (!Type::isRefCounted(type)) {
    return NULL;
  }

  Address start = m_as.code.frontier;

  // Check for RefCountStaticValue if needed, do the actual DecRef,
  // and leave flags set based on the subtract result, which is
  // tested below
  Address patchStaticCheck;
  if (genZeroCheck) {
    patchStaticCheck = cgCheckStaticBitAndDecRef(type, dataReg, exit);
  } else {
    // Set exit as NULL so that the code doesn't jump to error checking.
    patchStaticCheck = cgCheckStaticBitAndDecRef(type, dataReg, NULL);
  }

  // If not exiting on count down to zero, emit the zero-check and
  // release call
  if (genZeroCheck && exit == NULL) {
    // Emit jump to m_astubs (to call release) if count got down to
    // zero
    Address patch = m_as.code.frontier;
    ConditionCode cc = (&m_as == &m_astubs) ? CC_NE : CC_E;
    m_as.jcc(cc, m_astubs.code.frontier);
    // Emit the call to release in m_astubs
    cgCallHelper(m_astubs, getDtor(Type::toDataType(type)),
                 InvalidReg, kSyncPoint,
                 ArgGroup().reg(dataReg));
    if (&m_as == &m_astubs) {
      m_as.patchJcc(patch, m_as.code.frontier);
    } else {
      m_astubs.jmp(m_as.code.frontier);
    }
  }
  if (patchStaticCheck) {
    m_as.patchJcc8(patchStaticCheck, m_as.code.frontier);
  }

  return start;
}

//
// Generates dec-ref of a typed value with dynamic (statically unknown) type,
// when the type is stored in typeReg.
//
Address CodeGenerator::cgDecRefDynamicType(PhysReg typeReg,
                                           PhysReg dataReg,
                                           LabelInstruction* exit,
                                           bool genZeroCheck) {
  Address start = m_as.code.frontier;

  // Emit check for ref-counted type
  Address patchTypeCheck = cgCheckRefCountedType(typeReg);

  // Emit check for RefCountStaticValue and the actual DecRef
  Address patchStaticCheck;
  if (genZeroCheck) {
    patchStaticCheck = cgCheckStaticBitAndDecRef(Type::Cell, dataReg, exit);
  } else {
    patchStaticCheck = cgCheckStaticBitAndDecRef(Type::Cell, dataReg, NULL);
  }

  // If not exiting on count down to zero, emit the zero-check and release call
  if (genZeroCheck && exit == NULL) {
    // Emit jump to m_astubs (to call release) if count got down to zero
    Address patch = m_as.code.frontier;
    ConditionCode cc = (&m_as == &m_astubs) ? CC_NE : CC_E;
    m_as.jcc(cc, m_astubs.code.frontier);
    // Emit call to release in m_astubs
    cgCallHelper(m_astubs, getDtorTyped(), InvalidReg, kSyncPoint,
                 ArgGroup().reg(dataReg).reg(typeReg));
    if (&m_as == &m_astubs) {
      m_as.patchJcc(patch, m_as.code.frontier);
    } else {
      m_astubs.jmp(m_as.code.frontier);
    }
  }
  // Patch checks to jump around the DecRef
  if (patchTypeCheck)   m_as.patchJcc8(patchTypeCheck,   m_as.code.frontier);
  if (patchStaticCheck) m_as.patchJcc8(patchStaticCheck, m_as.code.frontier);

  return start;
}

//
// Generates dec-ref of a typed value with dynamic (statically
// unknown) type, when all we have is the baseReg and offset of
// the typed value. This method assumes that baseReg is not the
// scratch register.
//
Address CodeGenerator::cgDecRefDynamicTypeMem(PhysReg baseReg,
                                              int64 offset,
                                              LabelInstruction* exit) {
  Address start = m_as.code.frontier;
  auto scratchReg = rScratch;

  ASSERT(baseReg != scratchReg);

  // Emit check for ref-counted type
  Address patchTypeCheck = cgCheckRefCountedType(baseReg, offset);
  if (exit == NULL && RuntimeOption::EvalHHIRGenericDtorHelper) {
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
        m_as.lea(baseReg[offset], rdi);
        m_tx64->emitCall(m_as, m_tx64->m_dtorGenericStub);
      }
      recordSyncPoint(m_as);
    }
    if (patchTypeCheck) {
      m_as.patchJcc8(patchTypeCheck, m_as.code.frontier);
    }
    return start;
  }
  // Load m_data into the scratch reg
  m_as.load_reg64_disp_reg64(baseReg, offset + TVOFF(m_data), scratchReg);

  // Emit check for RefCountStaticValue and the actual DecRef
  Address patchStaticCheck = cgCheckStaticBitAndDecRef(Type::Cell, scratchReg,
                                                       exit);

  // If not exiting on count down to zero, emit the zero-check and release call
  if (exit == NULL) {
    // Emit jump to m_astubs (to call release) if count got down to
    // zero
    Address patch = m_as.code.frontier;
    ConditionCode cc = (&m_as == &m_astubs) ? CC_NE : CC_E;
    m_as.jcc(cc, m_astubs.code.frontier);

    m_astubs.lea_reg64_disp_reg64(baseReg, offset, scratchReg);

    // Emit call to release in m_astubs
    cgCallHelper(m_astubs, getDtorGeneric(), InvalidReg, kSyncPoint,
                 ArgGroup().reg(scratchReg));
    if (&m_as == &m_astubs) {
      m_as.patchJcc(patch, m_as.code.frontier);
    } else {
      m_astubs.jmp(m_as.code.frontier);
    }
  }

  // Patch checks to jump around the DecRef
  if (patchTypeCheck)   m_as.patchJcc8(patchTypeCheck,   m_as.code.frontier);
  if (patchStaticCheck) m_as.patchJcc8(patchStaticCheck, m_as.code.frontier);

  return start;
}

//
// Generates the dec-ref of a typed value in memory address [baseReg + offset].
// This handles cases where type is either static or dynamic.
//
Address CodeGenerator::cgDecRefMem(Type::Tag type,
                                   PhysReg baseReg,
                                   int64 offset,
                                   LabelInstruction* exit) {
  Address start = m_as.code.frontier;
  auto scratchReg = rScratch;
  ASSERT(baseReg != scratchReg);

  if (Type::isStaticallyKnown(type)) {
    if (Type::isRefCounted(type)) {
      m_as.load_reg64_disp_reg64(baseReg, offset, scratchReg);
      cgDecRefStaticType(type, scratchReg, exit, true);
    }
  } else {
    // The type is dynamic, but we don't have two registers available
    // to load the type and the data.
    cgDecRefDynamicTypeMem(baseReg, offset, exit);
  }

  return start;
}

Address CodeGenerator::cgDecRefWork(IRInstruction* inst, bool genZeroCheck) {
  Address start = m_as.code.frontier;
  SSATmp* src   = inst->getSrc(0);
  if (!isRefCounted(src)) {
    return start;
  }
  LabelInstruction* exit = inst->getLabel();
  Type::Tag type = src->getType();
  if (Type::isStaticallyKnown(type)) {
    cgDecRefStaticType(type, src->getReg(), exit, genZeroCheck);
  } else {
    cgDecRefDynamicType(src->getReg(1),
                        src->getReg(0),
                        exit,
                        genZeroCheck);
  }
  return start;
}

Address CodeGenerator::cgDecRef(IRInstruction *inst) {
  // DecRef may bring the count to zero, and run the destructor.
  // Generate code for this.
  return cgDecRefWork(inst, true);
}

Address CodeGenerator::cgDecRefNZ(IRInstruction* inst) {
  // DecRefNZ cannot bring the count to zero.
  // Therefore, we don't generate zero-checking code.
  return cgDecRefWork(inst, false);
}

Address CodeGenerator::cgAllocActRec1(IRInstruction* inst) {
  Address start = m_as.code.frontier;
  SSATmp* dst   = inst->getDst();
  SSATmp* sp    = inst->getSrc(0);

  int actRecAdjustment = -(int)sizeof(ActRec);
  auto spReg = sp->getReg();
  auto dstReg = dst->getReg();
  if (spReg != dstReg) {
    m_as.lea_reg64_disp_reg64(spReg, actRecAdjustment, dstReg);
  } else {
    m_as.add_imm32_reg64(actRecAdjustment, dstReg);
  }
  return start;
}

Address CodeGenerator::cgAllocActRec6(SSATmp* dst,
                                      SSATmp* sp,
                                      SSATmp* fp,
                                      SSATmp* func,
                                      SSATmp* objOrCls,
                                      SSATmp* nArgs,
                                      SSATmp* magicName)  {
  Address start = m_as.code.frontier;
  DEBUG_ONLY bool setThis  = true;

  int actRecAdjustment = -(int)sizeof(ActRec);
  auto spReg = sp->getReg();
  // actRec->m_this
  if (objOrCls->getType() == Type::ClassRef) {
    // store class
    if (objOrCls->isConst()) {
      m_as.store_imm64_disp_reg64(uintptr_t(objOrCls->getConstValAsClass()) | 1,
                                  actRecAdjustment + AROFF(m_this),
                                  spReg);
    } else {
      Reg64 clsPtrReg = objOrCls->getReg();
      m_as.movq  (clsPtrReg, rScratch);
      m_as.orq   (1, rScratch);
      m_as.storeq(rScratch, spReg[actRecAdjustment + AROFF(m_this)]);
    }
  } else if (objOrCls->getType() == Type::Obj) {
    // store this pointer
    m_as.store_reg64_disp_reg64(objOrCls->getReg(),
                                actRecAdjustment + AROFF(m_this),
                                spReg);
  } else {
    ASSERT(objOrCls->getType() == Type::Null);
    // no obj or class; this happens in FPushFunc
    int offset_m_this = actRecAdjustment + AROFF(m_this);
    // When func is Type::FuncClassRef, m_this/m_cls will be initialized below
    if (!func->isConst() && func->getType() == Type::FuncClassRef) {
      // m_this is unioned with m_cls and will be initialized below
      setThis = false;
    } else {
      m_as.store_imm64_disp_reg64(0, offset_m_this, spReg);
    }
  }
  // actRec->m_invName
  ASSERT(magicName->isConst());
  // ActRec::m_invName is encoded as a pointer with bit kInvNameBit
  // set to distinguish it from m_varEnv and m_extrArgs
  uintptr_t invName =
    (magicName->getType() == Type::Null
      ? 0
      : (uintptr_t(magicName->getConstValAsStr()) | ActRec::kInvNameBit));
  m_as.store_imm64_disp_reg64(invName,
                              actRecAdjustment + AROFF(m_invName),
                              spReg);
  // actRec->m_func  and possibly actRec->m_cls
  // Note m_cls is unioned with m_this and may overwrite previous value
  if (func->getType() == Type::Null) {
    ASSERT(func->isConst());
  } else if (func->isConst()) {
    // TODO: have register allocator materialize constants
    const Func* f = func->getConstValAsFunc();
    m_as. mov_imm64_reg((uint64)f, rScratch);
    m_as.store_reg64_disp_reg64(rScratch,
                                actRecAdjustment + AROFF(m_func),
                                spReg);
    if (func->getType() == Type::FuncClassRef) {
      // Fill in m_cls if provided with both func* and class*
      fprintf(stderr, "cgAllocActRec: const func->isFuncClassRef()\n");
      CG_PUNT(cgAllocActRec6);
    }
  } else {
    int offset_m_func = actRecAdjustment + AROFF(m_func);
    m_as.store_reg64_disp_reg64(func->getReg(0),
                                offset_m_func,
                                spReg);
    if (func->getType() == Type::FuncClassRef) {
      int offset_m_cls = actRecAdjustment + AROFF(m_cls);
      m_as.store_reg64_disp_reg64(func->getReg(1),
                                  offset_m_cls,
                                  spReg);
      setThis = true; /* m_this and m_cls are in a union */
    }
  }
  ASSERT(setThis);
  // actRec->m_savedRbp
  m_as.store_reg64_disp_reg64(fp->getReg(),
                              actRecAdjustment + AROFF(m_savedRbp),
                              spReg);

  // actRec->m_numArgsAndCtorFlag
  ASSERT(nArgs->isConst());
  m_as.store_imm32_disp_reg(nArgs->getConstValAsInt(),
                            actRecAdjustment + AROFF(m_numArgsAndCtorFlag),
                            spReg);

  auto dstReg = dst->getReg();
  if (spReg != dstReg) {
    m_as.lea_reg64_disp_reg64(spReg, actRecAdjustment, dstReg);
  } else {
    m_as.add_imm32_reg64(actRecAdjustment, dstReg);
  }
  return start;
}

Address CodeGenerator::cgAllocActRec5(SSATmp* dst,
                                      SSATmp* sp,
                                      SSATmp* fp,
                                      SSATmp* func,
                                      SSATmp* objOrCls,
                                      SSATmp* nArgs)  {
  Address start = m_as.code.frontier;

  int actRecAdjustment = -(int)sizeof(ActRec);
  m_as.store_reg64_disp_reg64(func->getReg(),
                              actRecAdjustment + AROFF(m_func),
                              sp->getReg());
  m_as.store_reg64_disp_reg64(fp->getReg(),
                              actRecAdjustment + AROFF(m_savedRbp),
                              sp->getReg());

  ASSERT(nArgs->isConst());
  m_as.store_imm32_disp_reg(nArgs->getConstValAsInt(),
                            actRecAdjustment + AROFF(m_numArgsAndCtorFlag),
                            sp->getReg());

  // XXX TODO: store the this or late bound class
  auto dstReg = dst->getReg();
  auto spReg = sp->getReg();
  if (spReg != dstReg) {
    m_as.lea_reg64_disp_reg64(spReg, actRecAdjustment, dstReg);
  } else {
    m_as.add_imm32_reg64(actRecAdjustment, dstReg);
  }
  // XXX TODO: This and varenv
  return start;
}

Address CodeGenerator::cgAllocActRec(IRInstruction* inst) {
  uint32 numSrcs = inst->getNumSrcs();
  uint32 numExtendedSrcs = inst->getNumExtendedSrcs();

  if (numSrcs == 1) {
    return cgAllocActRec1(inst);
  } else {
    SSATmp* dst   = inst->getDst();
    SSATmp* src1  = inst->getSrc(0);
    SSATmp* src2  = inst->getSrc(1);
    SSATmp* src3  = inst->getSrc(2);
    SSATmp* src4  = inst->getSrc(3);

    if (numExtendedSrcs == 4) {
      return cgAllocActRec6(dst,
                            src1,             // sp
                            src2,             // fp
                            src3,             // func
                            src4,             // objOrCls
                            inst->getSrc(4),  // numParams
                            inst->getSrc(5)); // magicName
    } else if (numExtendedSrcs == 3) {
      return cgAllocActRec5(dst,
                            src1,
                            src2,
                            src3,
                            src4,
                            inst->getSrc(4));
    } else {
      ASSERT(numExtendedSrcs == 2);
      return cgAllocActRec5(dst,
                            src1,
                            src2,
                            src3, /* extendedSrcs[0] */
                            NULL,
                            src4 /* extendedSrcs[1] */);
    }
  }
  ASSERT(false);
  return NULL;
}

ActRec*
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

ActRec*
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

Address CodeGenerator::cgNewObj(IRInstruction* inst) {
  Address start = m_as.code.frontier;
  SSATmp* dst   = inst->getDst();
  SSATmp* numParams = inst->getSrc(0);
  SSATmp* clsName = inst->getSrc(1);
  SSATmp* sp    = inst->getSrc(2);
  SSATmp* fp    = inst->getSrc(3);

  if (Type::isString(clsName->getType())) {
    const StringData* classNameString = clsName->getConstValAsStr();
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
  return start;
}

Address CodeGenerator::cgNewArray(IRInstruction* inst) {
  Address start = m_as.code.frontier;
  SSATmp* dst   = inst->getDst();
  SSATmp* cap   = inst->getSrc(0);
  ArgGroup args;
  args.ssa(cap);
  cgCallHelper(m_as, (TCA)new_array, dst, kNoSyncPoint, args);
  return start;
}

Address CodeGenerator::cgNewTuple(IRInstruction* inst) {
  Address start = m_as.code.frontier;
  SSATmp* dst = inst->getDst();
  SSATmp* numArgs = inst->getSrc(0);
  SSATmp* sp = inst->getSrc(1);
  ArgGroup args;
  args.ssa(numArgs).ssa(sp);
  cgCallHelper(m_as, (TCA) new_tuple, dst, kNoSyncPoint, args);
  return start;
}

Address CodeGenerator::cgCall(IRInstruction* inst) {

  Address start   = m_as.code.frontier;
  SSATmp* actRec  = inst->getSrc(0);
  SSATmp* returnBcOffset = inst->getSrc(1);
  int32  numArgs = inst->getNumExtendedSrcs();
  ASSERT(numArgs > 0);
  SSATmp** args   = ((ExtendedInstruction*)inst)->getExtendedSrcs();

  SSATmp* func = args[0];
  // skip over func
  args++;  numArgs--;

  auto spReg = actRec->getReg();
  // put all outgoing arguments onto the VM stack
  int64 adjustment = (-(int64)numArgs) * sizeof(Cell);
  for (int32 i = 0; i < numArgs; i++) {
    cgStore(spReg, -(i+1) * sizeof(Cell), args[i]);
  }
  // store the return bytecode offset into the outgoing actrec
  uint64 returnBc = returnBcOffset->getConstValAsInt();
  m_as.store_imm32_disp_reg(returnBc, AROFF(m_soff), spReg);
  if (adjustment != 0) {
    m_as.add_imm32_reg64(adjustment, spReg);
  }

  SrcKey srcKey = SrcKey(m_lastMarker->getFunc(), m_lastMarker->getLabelId());
  bool isImmutable = (func->isConst() && func->getType() != Type::Null);
  const Func* funcd = isImmutable ? func->getConstValAsFunc() : NULL;
  int32_t adjust = m_tx64->emitBindCall(srcKey, funcd, numArgs);
  if (adjust) {
    m_as.addq (adjust, rVmSp);
  }

  if (RuntimeOption::EvalHHIRGenerateAsserts) {
    emitCheckStack(m_as, inst->getDst(), 1, false);
  }
  return start;
}

Address CodeGenerator::cgSpillStackWork(IRInstruction* inst, bool allocActRec) {
  Address start = m_as.code.frontier;
  SSATmp* dst   = inst->getDst();
  SSATmp* sp    = inst->getSrc(0);
  SSATmp* spAdjustment = inst->getSrc(1);
  uint32   numSpill  = inst->getNumExtendedSrcs();
  SSATmp** spillSrcs = ((ExtendedInstruction*)inst)->getExtendedSrcs();

  auto dstReg = dst->getReg();
  auto spReg = sp->getReg();
  int64 adjustment =
    (spAdjustment->getConstValAsInt() - numSpill) * sizeof(Cell);
  for (uint32 i = 0; i < numSpill; i++) {
    cgStore(spReg, (i * sizeof(Cell)) + adjustment, spillSrcs[i]);
  }
  if (allocActRec) {
    adjustment -= (3 * sizeof(Cell)); // XXX replace with symbolic constant
  }
  if (adjustment != 0) {
    if (dstReg != spReg) {
      m_as.lea_reg64_disp_reg64(spReg, adjustment, dstReg);
    } else {
      m_as.add_imm32_reg64(adjustment, dstReg);
    }
  } else if (dstReg != spReg) {
    m_as.mov_reg64_reg64(spReg, dstReg);
    if (m_curTrace->isMain()) {
      TRACE(3, "[counter] 1 reg move in cgSpillStackWork\n");
    }
  }
  if (false) {
    emitCheckStack(m_as, dst, numSpill, allocActRec);
  }
  if (RuntimeOption::EvalHHIRGenerateAsserts) {
    for (uint32 i = 0; i < numSpill; i++) {
      if (spillSrcs[i]->getType() != Type::Gen) {
        emitCheckCell(m_as, dst, i + (allocActRec ? 3 : 0));
      }
    }
  }
  return start;
}

Address CodeGenerator::cgSpillStack(IRInstruction* inst) {
  return cgSpillStackWork(inst, false);
}
Address CodeGenerator::cgSpillStackAllocAR(IRInstruction* inst) {
  return cgSpillStackWork(inst, true);
}

Address CodeGenerator::cgNativeImpl(IRInstruction* inst) {
  Address start = m_as.code.frontier;
  SSATmp* func  = inst->getSrc(0);
  SSATmp* fp    = inst->getSrc(1);
  ASSERT(func->isConst());
  ASSERT(func->getType() == Type::FuncRef);
  BuiltinFunction builtinFuncPtr = func->getConstValAsFunc()->builtinFuncPtr();
  auto fpReg = fp->getReg();
  if (fpReg != argNumToRegName[0]) {
    m_as.mov_reg64_reg64(fpReg, argNumToRegName[0]);
    if (m_curTrace->isMain()) {
      TRACE(3, "[counter] 1 reg move in cgNativeImpl\n");
    }
  }
  m_as.call((TCA)builtinFuncPtr);
  recordSyncPoint(m_as);
  return start;
}

Address CodeGenerator::cgLdThis(IRInstruction* inst) {
  Address start = m_as.code.frontier;
  SSATmp* dst   = inst->getDst();
  SSATmp* src   = inst->getSrc(0);
  LabelInstruction* label = inst->getLabel();
  // mov dst, [fp + 0x20]
  auto dstReg = dst->getReg();

  // the destination of LdThis could be dead but
  // the instruction itself still useful because
  // of the checks that it does (if it has a label).
  // So we need to make sure there is a dstReg for this
  // instruction.
  if (dstReg != InvalidReg) {
    // instruction's result is not dead
    m_as.load_reg64_disp_reg64(src->getReg(),
                               AROFF(m_this),
                               dstReg);
  }

  if (label != NULL) {
    // we need to perform its checks
    if (curFunc()->cls() == NULL) {
      // test dst, dst
      // jz label

      if (dstReg == InvalidReg) {
        dstReg = reg::rScratch;
        m_as.load_reg64_disp_reg64(src->getReg(),
                                   AROFF(m_this),
                                   dstReg);
      }
      m_as.test_reg64_reg64(dstReg, dstReg);
      emitFwdJcc(CC_Z, label);
    }
    // test dst, 0x01
    // jnz label
    if (dstReg == InvalidReg) {
      // TODO: Could also use a 32-bit test here
      m_as.test_imm64_disp_reg64(1, AROFF(m_this), src->getReg());
      emitFwdJcc(CC_NZ, label);
    } else {
      m_as.test_imm32_reg64(1, dstReg);
      emitFwdJcc(CC_NZ, label);
    }
#if 0
    // TODO: Move this to be the code generated for a new instruction for
    // raising fatal
    m_as.jcc(CC_NZ, m_astubs.code.frontier);
    m_astubs.call((TCA)fatalNullThis);
    recordSyncPoint(m_astubs);
#endif
  }
  return start;
}

Address CodeGenerator::cgLdConst(IRInstruction* inst) {
  Address start = m_as.code.frontier;
  SSATmp* dst   = inst->getDst();
  int64 constVal= ((ConstInstruction*)inst)->getValAsInt();

  auto dstReg = dst->getReg();
  ASSERT(dstReg != InvalidReg);

  if (constVal == 0) {
    m_as.xor_reg64_reg64(dstReg, dstReg);
  } else {
    m_as.mov_imm64_reg(constVal, dstReg);
  }

  return start;
}

Address CodeGenerator::cgLdVarEnv(IRInstruction* inst) {
  Address start = m_as.code.frontier;
  SSATmp* dst   = inst->getDst();
  SSATmp* src   = inst->getSrc(0);

  ASSERT(!(src->isConst()));
  ASSERT(!(dst->isConst()));

  auto srcReg = src->getReg();
  auto dstReg = dst->getReg();

  m_as.load_reg64_disp_reg64(srcReg, AROFF(m_varEnv), dstReg);

  return start;
}

Address CodeGenerator::cgLdARFuncPtr(IRInstruction* inst) {
  Address start = m_as.code.frontier;
  SSATmp* dst   = inst->getDst();
  SSATmp* baseAddr = inst->getSrc(0);
  SSATmp* offset   = inst->getSrc(1);

  auto dstReg  = dst->getReg();
  auto baseReg = baseAddr->getReg();

  ASSERT(offset->isConst());

  m_as.load_reg64_disp_reg64(baseReg,
                           offset->getConstValAsInt() + AROFF(m_func),
                           dstReg);

  return start;
}

static int getNativeTypeSize(Type::Tag type) {
  switch (type) {
    case Type::Int:
    case Type::FuncRef:
      return sz::qword;
    case Type::Bool:
      return sz::byte;
    default:
      not_implemented();
  }
}

Address CodeGenerator::cgLdRaw(IRInstruction* inst) {
  Address start  = m_as.code.frontier;
  SSATmp* dest   = inst->getDst();
  SSATmp* addr   = inst->getSrc(0);
  SSATmp* offset = inst->getSrc(1);

  ASSERT(!(dest->isConst()));

  Reg64 addrReg = addr->getReg();
  PhysReg destReg = dest->getReg();

  if (addr->isConst()) {
    addrReg = rScratch;
    m_as.movq (addr->getConstValAsRawInt(), addrReg);
  }

  if (offset->isConst()) {
    ASSERT(offset->getType() == Type::Int);
    int64 kind = offset->getConstValAsInt();
    RawMemSlot& slot = RawMemSlot::Get(RawMemSlot::Kind(kind));
    int ldSize = slot.getSize();
    int64 off = slot.getOffset();
    if (ldSize == sz::qword) {
      m_as.loadq (addrReg[off], destReg);
    } else if (ldSize == sz::dword) {
      m_as.loadl (addrReg[off], r32(destReg));
    } else {
      ASSERT(ldSize == sz::byte);
      m_as.loadzbl (addrReg[off], r32(destReg));
    }
  } else {
    int ldSize = getNativeTypeSize(dest->getType());
    Reg64 offsetReg = r64(offset->getReg());
    if (ldSize == sz::qword) {
      m_as.loadq (addrReg[offsetReg], destReg);
    } else {
      // Not yet supported by our assembler
      ASSERT(ldSize == sz::byte);
      not_implemented();
    }
  }

  return start;
}

Address CodeGenerator::cgStRaw(IRInstruction* inst) {
  Address start = m_as.code.frontier;
  auto baseReg = inst->getSrc(0)->getReg();
  int64 kind = inst->getSrc(1)->getConstValAsInt();
  SSATmp* value = inst->getSrc(2);

  RawMemSlot& slot = RawMemSlot::Get(RawMemSlot::Kind(kind));
  int stSize = slot.getSize();
  int64 off = slot.getOffset();
  if (value->isConst()) {

    if (stSize == sz::qword) {
      m_as.store_imm64_disp_reg64(value->getConstValAsInt(),
                                  off,
                                  baseReg);
    } else if (stSize == sz::dword) {
      m_as.store_imm32_disp_reg(value->getConstValAsInt(),
                                  off,
                                  baseReg);
    } else {
      ASSERT(stSize == sz::byte);
      m_as.store_imm8_disp_reg(value->getConstValAsBool(),
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
      ASSERT(stSize == sz::byte);
      not_implemented();
    }
  }

  return start;
}

// If label is set and type is not Gen, this method generates a check
// that bails to the label if the loaded typed value doesn't match type.
Address CodeGenerator::cgLoadTypedValue(Type::Tag type,
                                        SSATmp* dst,
                                        PhysReg base,
                                        int64_t off,
                                        LabelInstruction* label,
                                        IRInstruction* inst) {
  ASSERT(type == dst->getType());
  ASSERT(!Type::isStaticallyKnown(type));
  Address start = m_as.code.frontier;
  auto valueDstReg = dst->getReg(0);
  auto typeDstReg = dst->getReg(1);
  if (valueDstReg == InvalidReg && typeDstReg == InvalidReg &&
      (label == NULL || type == Type::Gen)) {
    // a dead load
    ASSERT(typeDstReg == InvalidReg);
    return start;
  }
  bool useScratchReg = (base == typeDstReg && valueDstReg != reg::noreg);
  if (useScratchReg) {
    // Save base to rScratch, because base will be overwritten.
    m_as.mov_reg64_reg64(base, reg::rScratch);
    if (m_curTrace->isMain()) {
      TRACE(3, "[counter] 1 reg move in cgLoadTypedValue\n");
    }
  }

  // Check type if needed
  if (label) {
    cgGuardType(type, base, off, label, inst);
  }

  // Load type if it's not dead
  if (typeDstReg != reg::noreg) {
    m_as.load_reg64_disp_reg32(base, off + TVOFF(m_type), typeDstReg);
  }

  // Load value if it's not dead
  if (valueDstReg != reg::noreg) {
    if (useScratchReg) {
      m_as.load_reg64_disp_reg64(reg::rScratch, off + TVOFF(m_data), valueDstReg);
    } else {
      m_as.load_reg64_disp_reg64(base, off + TVOFF(m_data), valueDstReg);
    }
  }
  return start;
}


Address CodeGenerator::cgStoreTypedValue(PhysReg base,
                                         int64_t off,
                                         SSATmp* src) {
  Address start = m_as.code.frontier;
  ASSERT(!Type::isStaticallyKnown(src->getType()));
  m_as.store_reg64_disp_reg64(src->getReg(0),
                              off + TVOFF(m_data),
                              base);
  // store the type
  m_as.store_reg32_disp_reg64(src->getReg(1),
                              off + TVOFF(m_type),
                              base);
  return start;
}

// checkNotVar: If true, also emit check that loaded type is not Variant
// checkNotUninit: If true, also emit check that loaded type is not Uninit
Address CodeGenerator::cgStore(PhysReg base,
                               int64_t off,
                               SSATmp* src,
                               bool genStoreType) {
  Type::Tag type = src->getType();
  Address start = m_as.code.frontier;
  if (!Type::isStaticallyKnown(type)) {
    return cgStoreTypedValue(base, off, src);
  }
  if (type == Type::Uninit || type == Type::Null) {
    // no need to store a value for null or uninit
    if (genStoreType) {
      m_as.store_imm32_disp_reg(Type::toDataType(type),
                                off + TVOFF(m_type),
                                base);
    }
    return start;
  }
  ASSERT(type != Type::Home);
  if (src->isConst()) {
    if (type == Type::Bool) {
      m_as.store_imm64_disp_reg64((int64)src->getConstValAsBool(),
                                  off + TVOFF(m_data),
                                  base);
    } else if (type == Type::Int) {
      m_as.store_imm64_disp_reg64(src->getConstValAsInt(),
                                  off + TVOFF(m_data),
                                  base);
    } else if (type == Type::Dbl) {
      CG_PUNT(cgStore_Dbl); // not handled yet!
    } else if (type == Type::Arr) {
      m_as.store_imm64_disp_reg64((int64)src->getConstValAsArr(),
                                  off + TVOFF(m_data),
                                  base);
    } else {
      ASSERT(type == Type::StaticStr);
      m_as.store_imm64_disp_reg64((int64)src->getConstValAsStr(),
                                  off + TVOFF(m_data),
                                  base);
    }
  } else {
    if (type == Type::Bool) {
      // BOOL BYTE
      m_as.and_imm64_reg64(0xff, src->getReg());
    }
    if (type != Type::Null && type != Type::Uninit) {
      // no need to store any value for null or uninit
      m_as.store_reg64_disp_reg64(src->getReg(),
                                  off + TVOFF(m_data),
                                  base);
    }
  }
  // store the type
  if (genStoreType) {
    m_as.store_imm32_disp_reg(Type::toDataType(type),
                              off + TVOFF(m_type),
                              base);
  }
  return start;
}

void CodeGenerator::emitGuardOrFwdJcc(IRInstruction*    inst,
                                      ConditionCode     cc,
                                      LabelInstruction* label) {
  if (inst && inst->getTCA() == kIRDirectGuardActive) {
    if (RuntimeOption::EvalDumpIR) {
      m_tx64->prepareForSmash(m_as, TranslatorX64::kJmpccLen);
      inst->setTCA(m_as.code.frontier);
    }
    // Get the SrcKey for the dest
    SrcKey  destSK(getCurrFunc(), m_curTrace->getBcOff());
    SrcRec* destSR = m_tx64->getSrcRec(destSK);
    m_tx64->emitFallbackCondJmp(m_as, *destSR, cc);
  } else {
    emitFwdJcc(cc, label);
  }
}

Address CodeGenerator::cgLoad(Type::Tag type,
                              SSATmp* dst,
                              PhysReg base,
                              int64_t off,
                              LabelInstruction* label,
                              IRInstruction* inst) {
  Address start = m_as.code.frontier;
  if (!Type::isStaticallyKnown(type)) {
    return cgLoadTypedValue(type, dst, base, off, label, inst);
  }
  if (label != NULL && type != Type::Home) {
    cgGuardType(type, base, off, label, inst);
  }
  if (type == Type::Uninit || type == Type::Null) {
    return start; // these are constants
  }
  auto dstReg = dst->getReg();
  if (dstReg != InvalidReg) {
    // if dstReg == InvalidReg then the value of this load is dead
    if (type == Type::Bool) {
      m_as.load_reg64_disp_reg32(base, off + TVOFF(m_data),  dstReg);
    } else {
      m_as.load_reg64_disp_reg64(base, off + TVOFF(m_data),  dstReg);
    }
  }
  return start;
}

Address CodeGenerator::cgLdPropNR(IRInstruction* inst) {
  Address   start = m_as.code.frontier;
  Type::Tag type  = inst->getType();
  SSATmp*   dst   = inst->getDst();
  SSATmp*   obj   = inst->getSrc(0);
  SSATmp*   prop  = inst->getSrc(1);
  LabelInstruction* label = inst->getLabel();

  PhysReg objReg = obj->getReg();
  if (prop->isConst()) {
    int64 offset = prop->getConstValAsInt();
    cgLoad(type, dst, objReg, offset, label);
  } else {
    CG_PUNT(LdPropNR);
  }
  return start;
}

Address CodeGenerator::cgLdMemNR(IRInstruction * inst) {
  Address   start = m_as.code.frontier;
  Type::Tag type  = inst->getType();
  SSATmp*   dst   = inst->getDst();
  SSATmp*   addr  = inst->getSrc(0);
  int64 offset    = inst->getSrc(1)->getConstValAsInt();
  LabelInstruction* label = inst->getLabel();

  cgLoad(type, dst, addr->getReg(), offset, label);
  return start;
}

Address CodeGenerator::cgLdRefNR(IRInstruction* inst) {
  Address   start = m_as.code.frontier;
  Type::Tag type  = inst->getType();
  SSATmp*   dst   = inst->getDst();
  SSATmp*   addr  = inst->getSrc(0);
  LabelInstruction* label = inst->getLabel();

  cgLoad(type, dst, addr->getReg(), 0, label);
  return start;
}

void CodeGenerator::recordSyncPoint(Asm& as) {
  ASSERT(m_lastMarker);
  Offset stackOff = m_lastMarker->getStackOff();
  Offset pcOff = m_lastMarker->getLabelId() - getCurrFunc()->base();
  m_tx64->recordSyncPoint(as, pcOff, stackOff);

}

Address CodeGenerator::cgRaiseUninitWarning(IRInstruction* inst) {
  Address start = m_as.code.frontier;
  SSATmp* home  = inst->getSrc(0);
  ConstInstruction* homeInst = (ConstInstruction*)home->getInstruction();
  int64_t index = homeInst->getLocal().getId();
  const StringData* name = getCurrFunc()->localVarName(index);
  cgCallHelper(m_as,
               (TCA)HPHP::VM::Transl::raiseUndefVariable,
               (SSATmp*)NULL,
               kSyncPoint,
               ArgGroup().immPtr(name));
  return start;
}

static void getLocalRegOffset(SSATmp* src, PhysReg& reg, int64& off) {
  ConstInstruction* homeInstr =
    dynamic_cast<ConstInstruction*>(src->getInstruction());
  ASSERT(homeInstr && homeInstr->getOpcode() == LdHome);
  reg = homeInstr->getSrc(0)->getReg();
  int64 index = homeInstr->getLocal().getId();
  off = -cellsToBytes(index + 1);
}

Address CodeGenerator::cgLdLoc(IRInstruction* inst) {
  Address start = m_as.code.frontier;
  Type::Tag         type  = inst->getType();
  SSATmp*           dst   = inst->getDst();
  LabelInstruction* label = inst->getLabel();

  PhysReg fpReg;
  int64 offset;
  getLocalRegOffset(inst->getSrc(0), fpReg, offset);
  ASSERT(fpReg == HPHP::VM::Transl::reg::rbp);
  cgLoad(type, dst, fpReg, offset, label, inst);
  return start;
}

Address CodeGenerator::cgLdLocAddr(IRInstruction* inst) {
  Address start = m_as.code.frontier;
  PhysReg fpReg;
  int64 offset;
  getLocalRegOffset(inst->getSrc(0), fpReg, offset);
  m_as.lea_reg64_disp_reg64(fpReg, offset,
                            inst->getDst()->getReg());
  return start;
}

Address CodeGenerator::cgLdStackAddr(IRInstruction* inst) {
  Address start = m_as.code.frontier;
  m_as.lea_reg64_disp_reg64(inst->getSrc(0)->getReg(),
                            cellsToBytes(inst->getSrc(1)->getConstValAsInt()),
                            inst->getDst()->getReg());
  return start;
}

Address CodeGenerator::cgLdStack(IRInstruction* inst) {
  Type::Tag type = inst->getType();
  SSATmp* dst    = inst->getDst();
  SSATmp* sp     = inst->getSrc(0);
  SSATmp* index  = inst->getSrc(1);
  LabelInstruction* label = inst->getLabel();

  ASSERT(index->isConst());
  return cgLoad(type,
                dst,
                sp->getReg(),
                index->getConstValAsInt()*sizeof(Cell),
                // no need to worry about boxed types if loading a cell
                type == Type::Cell ? NULL : label,
                inst);
}

Address CodeGenerator::cgGuardType(Type::Tag         type,
                                   PhysReg           baseReg,
                                   int64_t           offset,
                                   LabelInstruction* label,
                                   IRInstruction*    inst) {
  ASSERT(label);
  Address start = m_as.code.frontier;
  int64_t typeOffset = offset + TVOFF(m_type);
  ConditionCode cc;

  switch (type) {
    case Type::StaticStr     :
    case Type::Str           : {
      m_as.test_imm32_disp_reg32(KindOfStringBit,        typeOffset, baseReg);
      cc = CC_Z;
      break;
    }
    case Type::UncountedInit : {
      m_as.test_imm32_disp_reg32(KindOfUncountedInitBit, typeOffset, baseReg);
      cc = CC_Z;
      break;
    }
    case Type::Uncounted     : {
      m_as.cmp_imm32_disp_reg32(KindOfRefCountThreshold, typeOffset, baseReg);
      cc = CC_G;
      break;
    }
    case Type::Cell          : {
      m_as.cmp_imm32_disp_reg32(KindOfRef,               typeOffset, baseReg);
      cc = CC_GE;
      break;
    }
    case Type::Gen : {
      return start; // nothing to check
    }
    default: {
      DataType dataType = Type::toDataType(type);
      ASSERT(dataType >= KindOfUninit);
      m_as.cmp_imm32_disp_reg32(dataType,                typeOffset, baseReg);
      cc = CC_NZ;
      break;
    }
  }

  emitGuardOrFwdJcc(inst, cc, label);

  return start;
}

Address CodeGenerator::cgGuardStk(IRInstruction* inst) {
  Type::Tag          type = inst->getType();
  SSATmp*              sp = inst->getSrc(0);
  SSATmp*           index = inst->getSrc(1);
  LabelInstruction* label = inst->getLabel();

  ASSERT(index->isConst());

  return cgGuardType(type,
                     sp->getReg(0),
                     index->getConstValAsInt() * sizeof(Cell),
                     label,
                     inst);
}

Address CodeGenerator::cgGuardLoc(IRInstruction* inst) {
  Type::Tag          type = inst->getType();
  SSATmp*           index = inst->getSrc(0);
  LabelInstruction* label = inst->getLabel();
  PhysReg fpReg;
  int64 offset;
  getLocalRegOffset(index, fpReg, offset);
  ASSERT(fpReg == HPHP::VM::Transl::reg::rbp);

  return cgGuardType(type, fpReg, offset, label, inst);
}


Address CodeGenerator::cgGuardType(IRInstruction* inst) {
  Address   start = m_as.code.frontier;
  UNUSED Type::Tag type = inst->getType();
  SSATmp*   dst   = inst->getDst();
  SSATmp*   src   = inst->getSrc(0);
  LabelInstruction* label = inst->getLabel();
  auto dstReg = dst->getReg(0);
  auto srcValueReg = src->getReg(0);
  auto srcTypeReg = src->getReg(1);
  ASSERT(srcTypeReg != InvalidReg);

  // compare srcTypeReg with type
  DataType dataType = Type::toDataType(type);
  ConditionCode cc;
  if (IS_STRING_TYPE(dataType)) {
    m_as.test_imm32_reg32(KindOfStringBit, srcTypeReg);
    cc = CC_Z;
  } else {
    m_as.cmp_imm32_reg32(dataType, srcTypeReg);
    cc = CC_NE;
  }
  emitFwdJcc(cc, label);

  if (srcValueReg != dstReg) {
    m_as.mov_reg64_reg64(srcValueReg, dstReg);
  }

  return start;
}

Address CodeGenerator::cgGuardRefs(IRInstruction* inst) {
  Address start = m_as.code.frontier;
  ASSERT(inst->getNumExtendedSrcs() == 4);
  SSATmp* funcPtrTmp = inst->getSrc(0);
  SSATmp* nParamsTmp = inst->getSrc(1);
  SSATmp* bitsPtrTmp = inst->getSrc(2);
  SSATmp* firstBitNumTmp = inst->getSrc(3);
  SSATmp* mask64Tmp  = inst->getSrc(4);
  SSATmp* vals64Tmp  = inst->getSrc(5);
  LabelInstruction* exitLabel = inst->getLabel();

  // Get values in place
  ASSERT(funcPtrTmp->getType() == Type::FuncRef);
  auto funcPtrReg = funcPtrTmp->getReg();
  ASSERT(funcPtrReg != InvalidReg);

  ASSERT(nParamsTmp->getType() == Type::Int);
  auto nParamsReg = nParamsTmp->getReg();
  ASSERT(nParamsReg != InvalidReg);

  ASSERT(bitsPtrTmp->getType() == Type::Int);
  auto bitsPtrReg = bitsPtrTmp->getReg();
  ASSERT(bitsPtrReg != InvalidReg);

  ASSERT(firstBitNumTmp->isConst() && firstBitNumTmp->getType() == Type::Int);
  uint32 firstBitNum = (uint32)(firstBitNumTmp->getConstValAsInt());

  ASSERT(mask64Tmp->getType() == Type::Int);
  ASSERT(mask64Tmp->getInstruction()->getOpcode() == LdConst);
  auto mask64Reg = mask64Tmp->getReg();
  ASSERT(mask64Reg != InvalidReg);
  int64 mask64 = mask64Tmp->getConstValAsInt();

  ASSERT(vals64Tmp->getType() == Type::Int);
  ASSERT(vals64Tmp->getInstruction()->getOpcode() == LdConst);
  auto vals64Reg = vals64Tmp->getReg();
  ASSERT(vals64Reg != InvalidReg);
  int64 vals64 = vals64Tmp->getConstValAsInt();


  // Actually generate code

  auto bitsValReg = rScratch;
  TCA patchEndOuterIf = NULL;
  TCA patchEndInnerIf = NULL;

  // If few enought args...
  m_as.cmp_imm32_reg32(firstBitNum + 1, nParamsReg);
  TCA patchCmpParams = m_as.code.frontier;
  m_as.jcc8(CC_L, patchCmpParams);
  {
    //   Load the bit values in bitValReg:
    //     bitsValReg <- [bitsValPtr + (firstBitNum / 64)]
    m_as.load_reg64_disp_reg64(bitsPtrReg, sizeof(uint64) * (firstBitNum / 64),
                             bitsValReg);
    //     bitsValReg <- bitsValReg & mask64
    m_as.and_reg64_reg64(mask64Reg, bitsValReg);

    //   If bitsValReg != vals64Reg, then goto Exit
    m_as.cmp_reg64_reg64(bitsValReg, vals64Reg);
    emitFwdJcc(CC_NE, exitLabel);

    //   Goto End:
    patchEndOuterIf = m_as.code.frontier;
    m_as.jmp8(patchEndOuterIf);
  }
  // Else
  {
    m_as.patchJcc8(patchCmpParams, m_as.code.frontier);

    // Don't emit this following code if none of the check will be generated
    // TODO: optimize jumps when only one of the cases will generate a check
    if (vals64 != 0 || mask64 != vals64) {
      //   If not special builtin...
      m_as.test_imm32_disp_reg32(AttrVariadicByRef,
                               Func::attrsOff(),
                               funcPtrReg);
      TCA patchCmpAttrVariadicByRef = m_as.code.frontier;
      m_as.jcc8(CC_NZ, patchCmpAttrVariadicByRef);

      //     If vals64Reg != 0, goto Exit
      {
        // Original:
        // m_as.test_reg64_reg64(vals64Reg, vals64Reg);
        // emitFwdJcc(CC_NE, exitLabel);
        // New version:
        if (vals64 != 0) {
          emitFwdJmp(exitLabel);
        } else {
          patchEndInnerIf = m_as.code.frontier;
          m_as.jmp8(patchEndInnerIf);
        }
      }
      //   Else: it's a special builtins that has reffiness for extra args
      {
        m_as.patchJcc8(patchCmpAttrVariadicByRef, m_as.code.frontier);
        //     If the additional args don't match the expectation, goto Exit
        // Original:
        //m_as.cmp_imm32_reg64((signed int)(-1ull & mask), vals64Reg);
        //emitFwdJcc(CC_NE, exitLabel);
        // New version:
        if (mask64 != vals64) {
          emitFwdJmp(exitLabel);
        }
      }
    }
  }
  // End;

  // Patch jumps to End
  if (patchEndOuterIf) m_as.patchJmp8(patchEndOuterIf, m_as.code.frontier);
  if (patchEndInnerIf) m_as.patchJmp8(patchEndInnerIf, m_as.code.frontier);

  return start;
}

Address CodeGenerator::cgLdPropAddr(IRInstruction* inst) {
  Address start = m_as.code.frontier;
  SSATmp*   dst   = inst->getDst();
  SSATmp*   obj   = inst->getSrc(0);
  SSATmp*   prop  = inst->getSrc(1);

  ASSERT(prop->isConst() && prop->getType() == Type::Int);

  auto dstReg = dst->getReg();
  auto objReg = obj->getReg();

  ASSERT(objReg != InvalidReg);
  ASSERT(dstReg != InvalidReg);

  int64 offset = prop->getConstValAsInt();
  m_as.lea_reg64_disp_reg64(objReg, offset, dstReg);

  return start;
}

// Copied from translator-x64.cpp
static const char* getContextName() {
  Class* ctx = arGetContextClass(curFrame());
  return ctx ? ctx->name()->data() : ":anonymous:";
}

Address CodeGenerator::cgLdClsMethod(IRInstruction* inst) {
  Address start = m_as.code.frontier;
  SSATmp* dst   = inst->getDst();
  SSATmp* cls   = inst->getSrc(0);
  SSATmp* mSlot = inst->getSrc(1);

  ASSERT(cls->getType() == Type::ClassRef);
  ASSERT(mSlot->isConst() && mSlot->getType() == Type::Int);
  uint64 mSlotInt64 = mSlot->getConstValAsRawInt();
  // We're going to multiply mSlotVal by sizeof(Func*) and use
  // it as a 32-bit offset (methOff) below.
  if (mSlotInt64 > (std::numeric_limits<uint32_t>::max() / sizeof(Func*))) {
    CG_PUNT(cgLdClsMethod_large_offset);
  }
  int32 mSlotVal = (uint32) mSlotInt64;

  Reg64 dstReg = dst->getReg();
  ASSERT(dstReg != InvalidReg);

  Reg64 clsReg = cls->getReg();
  if (clsReg == InvalidReg) {
    CG_PUNT(LdClsMethod);
  }

  Offset vecOff  = Class::getMethodsOffset() + Class::MethodMap::vecOff();
  int32  methOff = mSlotVal * sizeof(Func*);
  m_as.loadq(clsReg[vecOff],  dstReg);
  m_as.loadq(dstReg[methOff], dstReg);

  return start;
}

Address CodeGenerator::cgLdClsMethodCache(IRInstruction* inst) {
  if (inst->getNumSrcs() < 3) {
    CG_PUNT(LdClsMethodCache);
  }
  Address start = m_as.code.frontier;
  SSATmp* dst   = inst->getDst();
  SSATmp* className  = inst->getSrc(0);
  SSATmp* methodName = inst->getSrc(1);
  SSATmp* baseClass  = inst->getSrc(2);
  LabelInstruction* label = inst->getLabel();

  // Stats::emitInc(a, Stats::TgtCache_StaticMethodHit);
  const StringData*  cls    = className->getConstValAsStr();
  const StringData*  method = methodName->getConstValAsStr();
  const NamedEntity* ne     = (NamedEntity*)baseClass->getConstValAsRawInt();
  TargetCache::CacheHandle ch =
    TargetCache::StaticMethodCache::alloc(cls, method, getContextName());
  auto funcDestReg  = dst->getReg(0);
  auto classDestReg = dst->getReg(1);


  // Attempt to retrieve the func* and class* from cache
  m_as.load_reg64_disp_reg64(rVmTl, ch, funcDestReg);
  m_as.test_reg64_reg64(funcDestReg, funcDestReg);
  // May have retrieved a NULL from the cache
  m_as.load_reg64_disp_reg64(rVmTl,
                             ch + offsetof(TargetCache::StaticMethodCache,
                                           m_cls),
                             classDestReg);

  // handle case where method is not entered in the cache
  m_as.jcc(CC_E, m_astubs.code.frontier);
  {
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
    m_astubs.jnz(m_as.code.frontier);
    // StaticMethodCache::lookupIR() returned NULL, bail
    emitFwdJmp(m_astubs, label);
  }
  return start;
}

Address CodeGenerator::cgLdClsPropAddr(IRInstruction* inst) {
  using namespace Transl::TargetCache;
  Address start = m_as.code.frontier;
  SSATmp* dst   = inst->getDst();
  SSATmp* cls   = inst->getSrc(0);
  SSATmp* clsName = inst->getSrc(1);
  SSATmp* prop  = inst->getSrc(2);

  if (clsName->isConst() && clsName->getType() == Type::StaticStr  &&
      prop->isConst() && prop->getType() == Type::StaticStr &&
      cls->getType() == Type::ClassRef) {

    const StringData* propName = prop->getConstValAsStr();
    auto dstReg = dst->getReg();
    auto srcReg = cls->getReg();
    const StringData* clsNameString = clsName->getConstValAsStr();
    string sds(Util::toLower(clsNameString->data()) + ":" +
               string(propName->data(), propName->size()));
    StringData sd(sds.c_str(), sds.size(), AttachLiteral);
    CacheHandle ch = SPropCache::alloc(&sd);

    auto tmpReg = dstReg == srcReg ? PhysReg(rScratch) : dstReg;
    m_as.load_reg64_disp_reg64(rVmTl, ch, tmpReg);
    m_as.test_reg64_reg64(tmpReg, tmpReg);
    // jz off to the helper call in astubs
    m_as.jcc(CC_E, m_astubs.code.frontier);
    if (tmpReg != dstReg) {
      m_as.mov_reg64_reg64(tmpReg, dstReg);
    }
    // this helper can raise an invalid property access error
    cgCallHelper(m_astubs, (TCA)SPropCache::lookup, dstReg, kSyncPoint,
                 ArgGroup().imm(ch).ssa(cls).ssa(prop));
    // TODO make the lookup helper return null on an error, and bail out
    // of the trace if it returns null
    m_astubs.jmp(m_as.code.frontier);
  } else {
    CG_PUNT(LdClsPropAddr);
  }

  return start;
}

Address CodeGenerator::cgLdCls(IRInstruction* inst) {
  // TODO: See TranslatorX64::emitKnownClassCheck for recent changes

  Address start = m_as.code.frontier;
  SSATmp* dst   = inst->getDst();
  SSATmp* className = inst->getSrc(0);

  ASSERT(className->isConst() && className->getType() == Type::StaticStr);
  auto dstReg = dst->getReg();
  const StringData* classNameString = className->getConstValAsStr();
  TargetCache::CacheHandle ch = TargetCache::allocKnownClass(classNameString);
  m_as.load_reg64_disp_reg64(rVmTl, ch, dstReg);
  m_as.test_reg64_reg64(dstReg, dstReg);
  // jz off to the helper call in astubs
  m_as.jcc(CC_E, m_astubs.code.frontier);
  {
    m_astubs.lea_reg64_disp_reg64(rVmTl, ch, dstReg);
    // Passing only two arguments to lookupKnownClass, since the
    // third is ignored in the checkOnly==false case.
    ArgGroup args;
    args.reg(dstReg)
      .ssa(className);
    // this helper can raise an undefined class error
    cgCallHelper(m_astubs,
                 (TCA)TargetCache::lookupKnownClass<false>,
                 dst,
                 kSyncPoint,
                 args);
    m_astubs.jmp(m_as.code.frontier);
  }
  return start;
}

Address CodeGenerator::cgLdClsCns(IRInstruction* inst) {
  Address start = m_as.code.frontier;

  Type::Tag type  = inst->getType();
  SSATmp*   dst   = inst->getDst();
  SSATmp*   cnsName = inst->getSrc(0);
  SSATmp*   cls   = inst->getSrc(1); /* May be a string or classref */
  LabelInstruction* label = inst->getLabel();

  ASSERT(cnsName->isConst() && cnsName->getType() == Type::StaticStr);
  ASSERT(cls->isConst() && cls->getType() == Type::StaticStr);

  StringData* fullName = StringData::GetStaticString(
    Util::toLower(cls->getConstValAsStr()->data()) + "::" +
    cnsName->getConstValAsStr()->data());

  TargetCache::CacheHandle ch = TargetCache::allocClassConstant(fullName);
  // note that we bail from the trace if the target cache entry is empty
  // for this class constant or if the type assertion fails.
  // TODO: handle the slow case helper call.
  cgLoad(type, dst, rVmTl, ch,
         // no need to worry about boxed types if loading a cell
         type == Type::Cell ? NULL : label);
  // The following checks that the target cache entry is valid (not Uninit).
  // If type is known, the cgLoad above already exits if the entry is invalid.
  if (type == Type::Cell) {
    cgCheckUninit(dst, label); // slow path helper call
  }
  return start;
}

Address CodeGenerator::cgJmpZeroHelper(IRInstruction* inst,
                                       ConditionCode cc) {
  Address start = m_as.code.frontier;
  SSATmp* src   = inst->getSrc(0);
  LabelInstruction* label = inst->getLabel();
  SSATmp* toSmash = inst->getTCA() == kIRDirectJccJmpActive ?
                                      inst->getDst() : NULL;

  auto srcReg = src->getReg();
  if (src->isConst()) {
    bool valIsZero = src->getConstValAsRawInt() == 0;
    if ((cc == CC_Z  && valIsZero) ||
        (cc == CC_NZ && !valIsZero)) {
      emitSmashableFwdJmp(label, toSmash);
    } else {
      // Fall through to next bytecode, disable DirectJmp
      inst->setTCA(kIRDirectJmpInactive);
    }
    return start;
  }
  if (src->getType() == Type::Bool) {
    m_as.testb(Reg8(int(srcReg)), Reg8(int(srcReg)));
  } else {
    m_as.test_reg64_reg64(srcReg, srcReg);
  }
  emitSmashableFwdJccAtEnd(cc, label, toSmash);
  return start;
}

Address CodeGenerator::cgJmpZero(IRInstruction* inst) {
  return cgJmpZeroHelper(inst, CC_Z);
}

Address CodeGenerator::cgJmpNZero(IRInstruction* inst) {
  return cgJmpZeroHelper(inst, CC_NZ);
}

Address CodeGenerator::cgJmp_(IRInstruction* inst) {
  Address start = m_as.code.frontier;
  LabelInstruction* label = inst->getLabel();

  // removed when moving trace exit to main trace in eliminateDeadCode
  ASSERT(false);
  emitFwdJmp(label);
  return start;
}

Address CodeGenerator::cgCheckUninit(SSATmp* src, LabelInstruction* label) {
  Address start = m_as.code.frontier;
  auto typeReg = src->getReg(1);

  ASSERT(label);
  ASSERT(typeReg != InvalidReg);

  if (HPHP::KindOfUninit == 0) {
    m_as.test_reg32_reg32(typeReg, typeReg);
  } else {
    m_as.cmp_imm32_reg32(HPHP::KindOfUninit, typeReg);
  }
  emitFwdJcc(CC_Z, label);
  return start;
}
Address CodeGenerator::cgCheckUninit(IRInstruction* inst) {
  return cgCheckUninit(inst->getSrc(0), inst->getLabel());
}

Address CodeGenerator::cgExitWhenSurprised(IRInstruction* inst) {
  Address start = m_as.code.frontier;
  LabelInstruction* label = inst->getLabel();

  m_tx64->emitTestSurpriseFlags(m_as);
  emitFwdJcc(CC_NZ, label);
  return start;
}

Address CodeGenerator::cgExitOnVarEnv(IRInstruction* inst) {
  Address start = m_as.code.frontier;
  SSATmp* fp    = inst->getSrc(0);
  LabelInstruction* label = inst->getLabel();

  ASSERT(!(fp->isConst()));

  auto fpReg = fp->getReg();
  m_as.cmp_imm64_disp_reg64(0, AROFF(m_varEnv), fpReg);
  emitFwdJcc(CC_NE, label);
  return start;
}

Address CodeGenerator::cgBox(IRInstruction* inst) {
  Address start = m_as.code.frontier;
  SSATmp* dst   = inst->getDst();
  SSATmp* src   = inst->getSrc(0);
  Type::Tag type = src->getType();
  if (type == Type::Cell) {
    cgCallHelper(m_as, (TCA)tvBoxHelper, dst, kNoSyncPoint,
                 ArgGroup().type(src)
                           .ssa(src));
  } else if (type < Type::Cell) {
    cgCallHelper(m_as, (TCA)tvBoxHelper, dst, kNoSyncPoint,
                 ArgGroup().imm(Type::toDataType(src->getType()))
                           .ssa(src));
  } else {
    ASSERT(0); // can't have any other type!
  }
  return start;
}


Address CodeGenerator::cgPrint(IRInstruction* inst) {
  Address start = m_as.code.frontier;
  SSATmp* arg   = inst->getSrc(0);
  void* fptr = NULL;
  switch (arg->getType()) {
    case Type::Str: case Type::StaticStr:
      fptr = (void*)print_string;
      break;
    case Type::Int:
      fptr = (void*)print_int;
      break;
    case Type::Bool:
      fptr = (void*)print_boolean;
      break;
    case Type::Null: // nothing to do
      return start;
    default:
      ASSERT(0); // unsupported
//      CG_PUNT(cgPrint);
      return start;
  }
  return cgCallHelper(m_as, (TCA)fptr, InvalidReg, kNoSyncPoint,
                      ArgGroup().ssa(arg));
}

ArrayData* addElemIntKeyHelper(ArrayData* ad,
                               int64 key,
                               uintptr_t value,
                               DataType valueType) {
  TypedValue tv;
  tv.m_type = valueType;
  tv._count = 1;
  tv.m_data.num = value;
  // this does not re-enter
  // change to array_setm_ik1_v0, which decrefs the value
  return array_setm_ik1_v0(0, ad, key, &tv);
}

ArrayData* addElemStringKeyHelper(ArrayData* ad,
                                  StringData* key,
                                  uintptr_t value,
                                  DataType valueType) {
  TypedValue tv;
  tv.m_type = valueType;
  tv._count = 1;
  tv.m_data.num = value;
  // this does not re-enter
  // change to array_setm_s0k1_v0, which decrefs both the key & value
//  return array_setm_sk1_v(0, ad, key, &tv);
  return array_setm_s0k1_v0(0, ad, key, &tv);
}

Address CodeGenerator::cgAddElem(IRInstruction* inst) {
  Address start = m_as.code.frontier;
  SSATmp* dst   = inst->getDst();
  SSATmp* arr   = inst->getSrc(0);
  SSATmp* key   = inst->getSrc(1);
  SSATmp* val   = inst->getSrc(2);

  Type::Tag keyType = key->getType();
  // TODO: Double check that these helpers don't re-enter
  if (keyType == Type::Int) {
    // decrefs the value but not the key
    cgCallHelper(m_as, (TCA)addElemIntKeyHelper, dst, kNoSyncPoint,
                 ArgGroup().ssa(arr)
                           .ssa(key)
                           .ssa(val)
                           .type(val));
  } else if (keyType == Type::Str || keyType == Type::StaticStr) {
    // decrefs the value but not the key
    cgCallHelper(m_as, (TCA)addElemStringKeyHelper, dst, kNoSyncPoint,
                 ArgGroup().ssa(arr)
                           .ssa(key)
                           .ssa(val)
                           .type(val));
  } else {
    CG_PUNT(AddElem);
  }

  return start;
}

ArrayData* addNewElemHelper(ArrayData* ad, uintptr_t data, DataType type) {
  TypedValue tv;
  tv.m_type = type;
  tv._count = 1;
  tv.m_data.num = data;
  // this does not re-enter
  return array_setm_wk1_v0(NULL, ad, &tv);
}

Address CodeGenerator::cgAddNewElem(IRInstruction* inst) {
  Address start = m_as.code.frontier;
  UNUSED SSATmp* dst   = inst->getDst();
  UNUSED SSATmp* arr   = inst->getSrc(0);
  UNUSED SSATmp* val   = inst->getSrc(1);

  // decrefs value
  cgCallHelper(m_as, (TCA)addNewElemHelper, dst, kNoSyncPoint,
               ArgGroup().ssa(arr)
                         .ssa(val)
                         .type(val));
  return start;
}

Address CodeGenerator::cgDefCns(IRInstruction* inst) {
  Address start = m_as.code.frontier;
  UNUSED SSATmp* dst     = inst->getDst();
  UNUSED SSATmp* cnsName = inst->getSrc(0);
  UNUSED SSATmp* val     = inst->getSrc(1);
  using namespace TargetCache;
  UNUSED CacheHandle ch = allocConstant((StringData*)cnsName->getConstValAsStr());
#if 0
  // ALIA:TODO
  // XXX second param is an inout pointer to a Ref, so we need to pass
  // the pointer to a stack slot
  if (RuntimeOption::RepoAuthoritative) {
    EMIT_CALL3(a, defCnsHelper<false>, IMM(ch), A(i.inputs[0]->location),
               IMM((uint64)name));
  } else {
    EMIT_CALL4(a, defCnsHelper<true>, IMM(ch), A(i.inputs[0]->location),
               IMM((uint64)name), IMM(allocCnsBit(name)));
  }
#endif

  CG_PUNT(DefCns);

  return start;
}

Address CodeGenerator::cgConcat(IRInstruction* inst) {
  Address start = m_as.code.frontier;
  SSATmp* dst   = inst->getDst();
  SSATmp* tl    = inst->getSrc(0);
  SSATmp* tr    = inst->getSrc(1);

  Type::Tag lType = tl->getType();
  Type::Tag rType = tr->getType();
  // We have specialized helpers for concatenating two strings, a
  // string and an int, and an int and a string.
  void* fptr = NULL;
  if (Type::isString(lType) && Type::isString(rType)) {
    fptr = (void*)concat_ss;
  } else if (Type::isString(lType) && rType == Type::Int) {
    fptr = (void*)concat_si;
  } else if (lType == Type::Int && Type::isString(rType)) {
    fptr = (void*)concat_is;
  }
  if (fptr) {
    cgCallHelper(m_as, (TCA)fptr, dst, kNoSyncPoint,
                 ArgGroup().ssa(tl)
                           .ssa(tr));
  } else {
    if (lType >= Type::Obj || rType >= Type::Obj) {
      CG_PUNT(cgConcat);
    }
    cgCallHelper(m_as, (TCA)concat, dst, kNoSyncPoint,
                 ArgGroup().type(lType)
                           .ssa(tl)
                           .type(rType)
                           .ssa(tr));
  }

  return start;
}

Address CodeGenerator::cgArrayAdd(IRInstruction* inst) {
  SSATmp* dst   = inst->getDst();
  SSATmp* src1  = inst->getSrc(0);
  SSATmp* src2  = inst->getSrc(1);
  // TODO: Double check that array_add is not re-entrant
  return cgCallHelper(m_as, (TCA)array_add, dst, kNoSyncPoint,
                      ArgGroup().ssa(src1)
                                .ssa(src2));
}

Address CodeGenerator::cgDefCls(IRInstruction* inst) {
  Address start = m_as.code.frontier;
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

  EMIT_CALL2(a, m_tx64->m_defClsHelper, IMM((uint64)c), IMM((uint64)after));
#endif
  CG_PUNT(DefCls);

  return start;
}

Address CodeGenerator::cgInterpOne(IRInstruction* inst) {
  Address start = m_as.code.frontier;

  SSATmp* fp = inst->getSrc(0);
  SSATmp* sp = inst->getSrc(1);
  SSATmp* pcOffTmp  = inst->getSrc(2);
  SSATmp* spAdjustmentTmp = inst->getSrc(3);
  SSATmp* resultTypeTmp = inst->getSrc(4);
  LabelInstruction* label = inst->getLabel();

  ASSERT(pcOffTmp->isConst());
  ASSERT(spAdjustmentTmp->isConst());
  ASSERT(resultTypeTmp->isConst());
  ASSERT(fp->getType() == Type::SP);
  ASSERT(sp->getType() == Type::SP);

  int64 pcOff = pcOffTmp->getConstValAsInt();

  void* interpOneHelper =
    interpOneEntryPoints[*(getCurrFunc()->unit()->at(pcOff))];

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
    ASSERT(targetTrace);
    uint32 targetBcOff = targetTrace->getBcOff();
    // compare the pc with the target bc offset
    m_as.cmp_imm64_disp_reg64(targetBcOff,
                              offsetof(VMExecutionContext, m_pc),
                              dstReg);
//    emitFwdJcc(CC_E, label);
  }
  auto newSpReg = inst->getDst()->getReg();
  DEBUG_ONLY auto spReg = sp->getReg();
  int64 spAdjustment = spAdjustmentTmp->getConstValAsInt();
  Type::Tag resultType = (Type::Tag)resultTypeTmp->getConstValAsInt();
  int64 adjustment =
    (spAdjustment - (resultType == Type::None ? 0 : 1)) * sizeof(Cell);
  ASSERT(newSpReg == spReg);
  if (adjustment != 0) {
    m_as.add_imm32_reg64(adjustment, newSpReg);
  }
  return start;
}

Address CodeGenerator::cgDefFunc(IRInstruction* inst) {
  Address start = m_as.code.frontier;
  SSATmp* dst   = inst->getDst();
  SSATmp* func  = inst->getSrc(0);
  cgCallHelper(m_as, (TCA)defFuncHelper, dst, kSyncPoint,
               ArgGroup().ssa(func));
  return start;
}

Address CodeGenerator::cgCreateCont(IRInstruction* inst) {
  Address start = m_as.code.frontier;
  auto helper = curFunc()->isNonClosureMethod() ?
    VMExecutionContext::createContinuation<true> :
    VMExecutionContext::createContinuation<false>;
  cgCallHelper(m_as, (TCA)helper,
               inst->getDst(), kNoSyncPoint,
               ArgGroup().ssas(inst, 0, 4));
  return start;
}

Address CodeGenerator::cgFillContLocals(IRInstruction* inst) {
  Address start = m_as.code.frontier;
  cgCallHelper(m_as, (TCA)VMExecutionContext::fillContinuationVars,
               InvalidReg, kNoSyncPoint,
               ArgGroup().ssas(inst, 0, 4));
  return start;
}

Address CodeGenerator::cgFillContThis(IRInstruction* inst) {
  Address start = m_as.code.frontier;
  SSATmp* cont = inst->getSrc(0);
  auto baseReg = inst->getSrc(1)->getReg();
  int64 offset = inst->getSrc(2)->getConstValAsInt();
  auto scratch = rScratch;

  m_as.load_reg64_disp_reg64(cont->getReg(), CONTOFF(m_obj), scratch);
  m_as.test_reg64_reg64(scratch, scratch);
  Address jmp = m_as.code.frontier;
  m_as.jz8(jmp); // jz no_this
  {
    m_as.add_imm32_disp_reg32(1, TVOFF(_count), scratch);
    m_as.store_reg64_disp_reg64(scratch, offset + TVOFF(m_data), baseReg);
    m_as.store_imm32_disp_reg(KindOfObject, offset + TVOFF(m_type), baseReg);
  }
  // no_this:
  m_as.patchJcc8(jmp, m_as.code.frontier);

  return start;
}

Address CodeGenerator::emitContVarEnvHelperCall(SSATmp* fp, TCA helper) {
  Address start = m_as.code.frontier;
  auto scratch = rScratch;

  m_as.  loadq (fp->getReg()[AROFF(m_varEnv)], scratch);
  m_as.  testq (scratch, scratch);
  m_as.  jnz   (m_astubs.code.frontier);
  {
    cgCallHelper(m_astubs, helper, InvalidReg, kNoSyncPoint,
                 ArgGroup().ssa(fp));
    m_astubs.jmp(m_as.code.frontier);
  }

  return start;
}

Address CodeGenerator::cgUnlinkContVarEnv(IRInstruction* inst) {
  return emitContVarEnvHelperCall(
    inst->getSrc(0),
    (TCA)VMExecutionContext::packContVarEnvLinkage);
}

Address CodeGenerator::cgLinkContVarEnv(IRInstruction* inst) {
  return emitContVarEnvHelperCall(
    inst->getSrc(0),
    (TCA)VMExecutionContext::unpackContVarEnvLinkage);
}

Address CodeGenerator::cgContRaiseCheck(IRInstruction* inst) {
  Address start = m_as.code.frontier;
  SSATmp* cont = inst->getSrc(0);
  m_as.test_imm32_disp_reg32(0x1, CONTOFF(m_should_throw),
                             cont->getReg());
  emitFwdJcc(CC_NZ, inst->getLabel());
  return start;
}

Address CodeGenerator::cgContPreNext(IRInstruction* inst) {
  Address start = m_as.code.frontier;
  auto contReg = inst->getSrc(0)->getReg();

  const Offset doneOffset = CONTOFF(m_done);
  CT_ASSERT((doneOffset + 1) == CONTOFF(m_running));
  // Check m_done and m_running at the same time
  m_as.test_imm32_disp_reg32(0x0101, doneOffset, contReg);
  emitFwdJcc(CC_NZ, inst->getLabel());

  // ++m_index
  m_as.add_imm64_disp_reg64(0x1, CONTOFF(m_index), contReg);
  // m_running = true
  m_as.store_imm8_disp_reg(0x1, CONTOFF(m_running), contReg);
  return start;
}

Address CodeGenerator::cgContStartedCheck(IRInstruction* inst) {
  Address start = m_as.code.frontier;
  m_as.cmp_imm64_disp_reg64(0, CONTOFF(m_index),
                            inst->getSrc(0)->getReg());
  emitFwdJcc(CC_L, inst->getLabel());
  return start;
}

Address CodeGenerator::cgIncStat(IRInstruction *inst) {
  Address start = m_as.code.frontier;
  Stats::emitInc(m_as, Stats::StatCounter(inst->getSrc(0)->getConstValAsInt()),
                 inst->getSrc(1)->getConstValAsInt());
  return start;
}

Address CodeGenerator::cgAssertRefCount(IRInstruction* inst) {
  Address start = m_as.code.frontier;
  SSATmp* src = inst->getSrc(0);
  auto srcReg = src->getReg();
  emitAssertRefCount(m_as, srcReg);
  return start;
}

void traceCallback(ActRec* fp, Cell* sp, int64 pcOff, void* rip) {
#if 0
  const Func* func = fp->m_func;
  std::cout << func->fullName()->data()
            << " " << pcOff
            << " " << rip
            << std::endl;
#endif
  checkFrame(fp, sp, true);
}

void CodeGenerator::emitTraceCall(CodeGenerator::Asm& as, int64 pcOff) {
  // call to a trace function
  as.mov_imm64_reg((int64_t)as.code.frontier, reg::rcx);
  as.mov_reg64_reg64(rVmFp, reg::rdi);
  as.mov_reg64_reg64(rVmSp, reg::rsi);
  as.mov_imm64_reg(pcOff, reg::rdx);
  // do the call; may use a trampoline
  m_tx64->emitCall(as, (TCA)traceCallback);
}

void CodeGenerator::cgTrace(Trace* trace, vector<TransBCMapping>* bcMap) {
  bool firstMarkerSeen = false;
  m_curTrace = trace;
  trace->setFirstAsmAddress(m_as.code.frontier);
  trace->setFirstAstubsAddress(m_astubs.code.frontier);
  if (RuntimeOption::EvalHHIRGenerateAsserts && trace->isMain()) {
    emitTraceCall(m_as, trace->getBcOff());
  }
  IRInstruction::Iterator it;
  IRInstruction::List instructionList = trace->getInstructionList();
  for (it = instructionList.begin();
       it != instructionList.end();
       it++) {
    IRInstruction* inst = *it;
    if (inst->getOpcode() == Marker) {
      if (!firstMarkerSeen) {
        firstMarkerSeen = true;
        // This will be generated right after the tracelet guards
        if (RuntimeOption::EvalJitTransCounters && m_tx64 &&
            trace->isMain()) {
          m_tx64->emitTransCounterInc(m_as);
        }
      }
      m_lastMarker = (LabelInstruction*)inst;
      if (m_tx64 && m_tx64->isTransDBEnabled() && bcMap) {
        bcMap->push_back((TransBCMapping){Offset(m_lastMarker->getLabelId()),
              m_as.code.frontier,
              m_astubs.code.frontier});
      }
    }
    m_curInst = inst;
    inst->genCode(this);
  }
  trace->setLastAsmAddress(m_as.code.frontier);
  TRACE(3, "[counter] %lu bytes of code generated\n",
        trace->getLastAsmAddress() - trace->getFirstAsmAddress());
  if (trace->isMain()) {
    TRACE(3, "[counter] %lu bytes of code generated in main traces\n",
          trace->getLastAsmAddress() - trace->getFirstAsmAddress());
  }
}

void genCodeForTrace(Trace* trace,
                     CodeGenerator::Asm& as,
                     CodeGenerator::Asm& astubs,
                     IRFactory* irFactory,
                     vector<TransBCMapping>* bcMap,
                     Transl::TranslatorX64* tx64) {
  // select instructions for the trace and its exits
  CodeGenerator cgMain(as, astubs, tx64);
  cgMain.cgTrace(trace, bcMap);
  CodeGenerator cgExits(astubs, astubs, tx64);
  Trace::List& exitTraces = trace->getExitTraces();
  for (Trace::Iterator it = exitTraces.begin();
       it != exitTraces.end();
       it++) {
    cgExits.cgTrace(*it, NULL);
  }
}

}}}
