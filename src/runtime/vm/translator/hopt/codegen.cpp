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
#include "runtime/base/tv_macros.h"
#include "runtime/base/array/hphp_array.h"
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

// from translator-x64.h
typedef register_name_t PhysReg; // XXX; should use asm-x64.h version
static const PhysReg InvalidReg = reg::noreg;

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

static const HPHP::Trace::Module TRACEMOD = HPHP::Trace::tx64;

using Transl::rVmSp;
using Transl::rVmFp;

// from traslator-x64.h
// The x64 C ABI.
const register_name_t CodeGenerator::argNumToRegName[] = {
  reg::rdi, reg::rsi, reg::rdx, reg::rcx, reg::r8, reg::r9
};

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

  MoveInfo(Kind kind, register_name_t reg1, register_name_t reg2):
      m_kind(kind), m_reg1(reg1), m_reg2(reg2) {}

  MoveInfo(Kind kind, int reg1, int reg2):
      m_kind(kind),
      m_reg1((register_name_t)reg1),
      m_reg2((register_name_t)reg2) {}

  Kind m_kind;
  register_name_t m_reg1, m_reg2;
};

template <int N>
void doRegMoves(int moves[N], int rTmp,
                std::vector<MoveInfo> &howTo) {
  ASSERT(howTo.empty());
  int outDegree[N];
  CycleInfo cycles[N];
  int numCycles = 0;
  // Iterate over the nodes filling in outDegree[] and cycles[] as we go
  {
    int index[N];
    for (int node = 0; node < N; ++node) {
      // If a node's source is itself, its a nop
      if (moves[node] == node) moves[node] = LinearScan::regNameAsInt(reg::noreg);
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

ArgDesc::ArgDesc(SSATmp* tmp, bool val) {
  if (tmp->getInstruction()->isDefConst()) {
    m_srcReg = reg::noreg;
    if (val) {
      m_imm = tmp->getConstValAsBits();
    } else {
      m_imm = Type::toDataType(tmp->getType());
    }
    m_kind = Imm;
    return;
  }
  if (tmp->getType() == Type::Null || tmp->getType() == Type::Uninit) {
    m_srcReg = reg::noreg;
    if (val) {
      m_imm = 0;
    } else {
      m_imm = Type::toDataType(tmp->getType());
    }
    m_kind = Imm;
    return;
  }
  if (val || tmp->getNumAssignedLocs() > 1) {
    register_name_t reg = tmp->getAssignedLoc(val ? 0 : 1);
    ASSERT(reg != reg::noreg);
    m_imm = 0;
    m_kind = Reg;
    m_srcReg = reg;
    return;
  }
  m_srcReg = reg::noreg;
  m_imm = Type::toDataType(tmp->getType());
  m_kind = Imm;
}

Address ArgDesc::genCode(CodeGenerator::Asm& as) const {
  Address start = as.code.frontier;
  switch (m_kind) {
    case Reg:
      as.mov_reg64_reg64(m_srcReg, m_dstReg);
      TRACE(3, "[counter] 1 reg move in ArgDesc::genCode\n");
      break;
    case Imm:
      emitImmReg(as, m_imm, m_dstReg);
      break;
    case Addr:
      as.lea_reg64_disp_reg64(m_srcReg, m_imm, m_dstReg);
      break;
  }
  return start;
}


void IRInstruction::genCode(CodeGenerator* codeGenerator) {
  m_asmAddr = codeGenerator->cgInst(this);
}

void TypeInstruction::genCode(CodeGenerator* codeGenerator) {
  m_asmAddr = codeGenerator->cgInst(this);
}

void ConstInstruction::genCode(CodeGenerator* codeGenerator) {
  ASSERT(m_op == LdConst);
  m_asmAddr = codeGenerator->cgLdConst(this);
}

void LabelInstruction::genCode(CodeGenerator* codeGenerator) {
  m_asmAddr = codeGenerator->cgLabel((Opcode)m_op, this);
}

void ExtendedInstruction::genCode(CodeGenerator* codeGenerator) {
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
  return m_as.code.frontier;
}

Address CodeGenerator::cgMarker(IRInstruction* inst) {
  return m_as.code.frontier;
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

#define OPC(name, hasDst, canCSE, essential, effects, native, consRef,  \
            prodRef, mayModRefs, rematerializable, error)               \
  case name : return cg ## name (inst);
  IR_OPCODES
  #undef OPC

    default:
      std::cerr << "CodeGenerator: unimplemented support for opcode " <<
        OpcodeStrings[opc] << std::endl;
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
  if (!((src1Type == Type::Int && src2Type == Type::Int  ) ||
        (src1Type == Type::Bool && src2Type == Type::Bool) ||
        (src1Type == Type::ClassRef && src2Type == Type::ClassRef))) {
    CG_PUNT(cgJcc);
  }
  if (src1Type == Type::ClassRef && src2Type == Type::ClassRef) {
    ASSERT(opc == JmpSame || opc == JmpNSame);
  }
  register_name_t srcReg1 = src1->getAssignedLoc();
  register_name_t srcReg2 = src2->getAssignedLoc();

  // Note: when both src1 and src2 are constants, we should transform the
  // branch into an unconditional jump earlier in the IR.
  if (src1->isConst()) {
    // TODO: use compare with immediate or make sure simplifier
    // canonicalizes this so that constant is src2
    srcReg1 = LinearScan::rScratch;
    m_as.mov_imm64_reg(src1->getConstValAsRawInt(), srcReg1);
  }
  if (src2->isConst()) {
    m_as.cmp_imm64_reg64(src2->getConstValAsRawInt(), srcReg1);
  } else {
    // Note the reverse syntax in the assembler.
    // This cmp will compute srcReg1 - srcReg2
    m_as.cmp_reg64_reg64(srcReg2, srcReg1);
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

void saveLiveOutRegs(CodeGenerator::Asm& as, int regSaveMask) {
  if (!regSaveMask) {
    return; // nothing to save
  }
  // save all live registers on the native stack
  for (int i = 0; i < LinearScan::NumCallerSavedRegs; i++) {
    register_name_t j = LinearScan::getCallerSavedReg(i);
    if (regSaveMask & LinearScan::getRegMask(j)) {
      as.pushr(j);
    }
  }
  return;
}

/*
 * return true if we pushed an extra reg to maintain 16byte stack alignment
 */
static
bool saveLiveOutRegsAligned(CodeGenerator::Asm& as, int regSaveMask) {
  if (!regSaveMask) {
    return 0; // nothing to save
  }
  // save all live registers on the native stack
  int count = 0;
  register_name_t lastSaved = reg::noreg;
  for (int i = 0; i < LinearScan::NumCallerSavedRegs; i++) {
    register_name_t j = LinearScan::getCallerSavedReg(i);
    if (regSaveMask & LinearScan::getRegMask(j)) {
      as.pushr(j);
      ++count; lastSaved = j;
    }
  }
  bool extraPush = count & 0x1;
  if (extraPush) {
    as.pushr(lastSaved);
  }
  return extraPush;
}

void restoreLiveOutRegs(CodeGenerator::Asm& as, int regSaveMask) {
  if (!regSaveMask) {
    return; // nothing to restore
  }
  // restore live caller-saved registers from native stack
  for (int i = LinearScan::NumCallerSavedRegs-1; i >= 0; i--) {
    register_name_t j = LinearScan::getCallerSavedReg(i);
    if (regSaveMask & LinearScan::getRegMask(j)) {
      as.popr(j);
    }
  }
}

void restoreLiveOutRegsAligned(CodeGenerator::Asm& as,
                               int regSaveMask,
                               bool extraPop) {
  if (!regSaveMask) {
    return; // nothing to restore
  }
  // restore live caller-saved registers from native stack
  for (int i = LinearScan::NumCallerSavedRegs-1; i >= 0; i--) {
    register_name_t j = LinearScan::getCallerSavedReg(i);
    if (regSaveMask & LinearScan::getRegMask(j)) {
      if (extraPop) {
        as.popr(j);
        extraPop = false;
      }
      as.popr(j);
    }
  }
}

int CodeGenerator::getLiveOutRegsToSave(register_name_t dstReg) {
  int regSaveMask =
    LinearScan::CallerSavedRegMask & m_curInst->getLiveOutRegs();
  // Don't include the dst register defined by this instruction
  // when saving the caller-saved registers
  if (dstReg != reg::noreg) {
    regSaveMask &= ~LinearScan::getRegMask(dstReg); // subtract out dstReg
  }
  return regSaveMask;
}

Address CodeGenerator::cgCallHelper(Asm& a,
                                    TCA addr,
                                    register_name_t dstReg,
                                    bool doRecordSyncPoint,
                                    ArgGroup& args) {

  ASSERT(int(args.size()) <= LinearScan::NumCallerSavedRegs);
  Address start = a.code.frontier;
  int regSaveMask = getLiveOutRegsToSave(dstReg);
  bool extraPop = saveLiveOutRegsAligned(a, regSaveMask);
  if (m_curInst->isNative()) {
    int nPushes = __builtin_popcount(regSaveMask) + extraPop;
    if (nPushes > 0) {
      TRACE(3, "[counter] %d pushes inserted before a native\n", nPushes);
    }
  }
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
  int moves[LinearScan::NumRegs];
  memset(moves, -1, sizeof moves);
  for (size_t i = 0; i < args.size(); ++i) {
    if (args[i].getKind() == ArgDesc::Reg) {
      moves[LinearScan::regNameAsInt(args[i].getDstReg())] =
        LinearScan::regNameAsInt(args[i].getSrcReg());
    }
  }
  std::vector<MoveInfo> howTo;
  doRegMoves<LinearScan::NumRegs>(moves, LinearScan::regNameAsInt(reg::rScratch),
                                  howTo);
  for (size_t i = 0; i < howTo.size(); ++i) {
    if (howTo[i].m_kind == MoveInfo::Move) {
      a.mov_reg64_reg64(howTo[i].m_reg1, howTo[i].m_reg2);
    } else {
      a.xchg_reg64_reg64(howTo[i].m_reg1, howTo[i].m_reg2);
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
      if (LinearScan::isCallerSavedReg(howTo[i].m_reg1) &&
          LinearScan::isCallerSavedReg(howTo[i].m_reg2)) {
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
      a.mov_imm64_reg((int64_t)args[i].getImm(), args[i].getDstReg());
    }
  }

  // do the call; may use a trampoline
  m_tx64->emitCall(a, addr);

  // HHIR:TODO this only does required part of TranslatorX64::recordCallImpl()
  // Better to have improved SKTRACE'n by calling recordStubCall,
  // recordReentrantCall, or recordReentrantStubCall as appropriate
  if (doRecordSyncPoint) {
    recordSyncPoint(a);
  }
  // grab the return value if any
  if (dstReg != reg::noreg && dstReg != reg::rax) {
    a.mov_reg64_reg64(reg::rax, dstReg);
    if (m_curTrace->isMain()) {
      if (m_curInst->isNative()) {
        TRACE(3, "[counter] 1 reg move in cgCallHelper\n");
      }
    }
  }
  restoreLiveOutRegsAligned(a, regSaveMask, extraPop);
  return start;
}

Address CodeGenerator::cgCallHelper(Asm& a,
                                    TCA addr,
                                    SSATmp* dst,
                                    bool doRecordSyncPoint,
                                    ArgGroup& args) {
  register_name_t dstReg = dst == NULL ? reg::noreg : dst->getAssignedLoc();
  return cgCallHelper(a, addr, dstReg, doRecordSyncPoint, args);
}


Address CodeGenerator::cgMov(IRInstruction* inst) {
  Address start = m_as.code.frontier;
  SSATmp* dst   = inst->getDst();
  SSATmp* src   = inst->getSrc(0);
  register_name_t dstReg  = dst->getAssignedLoc();
  register_name_t srcReg = src->getAssignedLoc();
  if (dstReg != srcReg) {
    m_as.mov_reg64_reg64(srcReg, dstReg);
    if (m_curTrace->isMain()) {
      TRACE(3, "[counter] 1 reg move in cgMov\n");
    }
  }
  return start;
}

#define GEN_UNARY_INT_OP(INSTR, OPER) do {                              \
  if (src->getType() != Type::Int && src->getType() != Type::Bool) {    \
    ASSERT(0); CG_PUNT(INSTR);                                          \
  }                                                                     \
  register_name_t dstReg = dst->getAssignedLoc();                       \
  register_name_t srcReg = src->getAssignedLoc();                       \
  ASSERT(dstReg != reg::noreg);                                         \
  /* const source */                                                    \
  if (src->isConst()) {                                                 \
    m_as.mov_imm64_reg(OPER src->getConstValAsRawInt(), dstReg);        \
    break;                                                              \
  }                                                                     \
  ASSERT(srcReg != reg::noreg);                                         \
  if (dstReg != srcReg) {                                               \
    m_as.mov_reg64_reg64(srcReg, dstReg);                               \
    if (m_curTrace->isMain()) {                                         \
      TRACE(3, "[counter] 1 reg move in UNARY_INT_OP\n");               \
    }                                                                   \
  }                                                                     \
  m_as. INSTR ## _reg64(dstReg);                                        \
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

#define GEN_COMMUTATIVE_INT_OP(INSTR, OPER) do {                            \
  if (!(src1->getType() == Type::Int || src1->getType() == Type::Bool) ||   \
      !(src2->getType() == Type::Int || src2->getType() == Type::Bool)) {   \
    CG_PUNT(INSTR);                                                         \
  }                                                                         \
  register_name_t dstReg  = dst->getAssignedLoc();                          \
  register_name_t src1Reg = src1->getAssignedLoc();                         \
  register_name_t src2Reg = src2->getAssignedLoc();                         \
  /* 2 consts */                                                            \
  if (src1->isConst() && src2->isConst()) {                                 \
    int64 src1Const = src1->getConstValAsRawInt();                          \
    int64 src2Const = src2->getConstValAsRawInt();                          \
    m_as.mov_imm64_reg(src1Const OPER src2Const, dstReg);                   \
  /* 1 const, 1 reg */                                                      \
  } else if (src1->isConst() || src2->isConst()) {                          \
    int64 srcConst = (src1->isConst() ? src1 : src2)->getConstValAsRawInt();\
    register_name_t srcReg = (src1->isConst() ? src2Reg : src1Reg);         \
    if (srcReg == dstReg) {                                                 \
      m_as. INSTR ## _imm64_reg64(srcConst, dstReg);                        \
    } else {                                                                \
    /* TODO: use lea when possible */                                       \
      m_as.mov_imm64_reg(srcConst, dstReg);                                 \
      m_as. INSTR ##_reg64_reg64(srcReg, dstReg);                           \
    }                                                                       \
  /* both src1 and src2 are regs */                                         \
  } else {                                                                  \
    if (dstReg != src1Reg && dstReg != src2Reg) {                           \
      m_as.mov_reg64_reg64(src1Reg, dstReg);                                \
      if (m_curTrace->isMain()) {                                           \
        TRACE(3, "[counter] 1 reg move in COMMUTATIVE_INT_OP\n");           \
      }                                                                     \
      m_as. INSTR ## _reg64_reg64(src2Reg, dstReg);                         \
    } else {                                                                \
      if (dstReg == src1Reg) {                                              \
        m_as. INSTR ## _reg64_reg64(src2Reg, dstReg);                       \
      } else {                                                              \
        ASSERT(dstReg == src2Reg);                                          \
        m_as. INSTR ## _reg64_reg64(src1Reg, dstReg);                       \
      }                                                                     \
    }                                                                       \
  }                                                                         \
  } while (0)

#define GEN_NON_COMMUTATIVE_INT_OP(INSTR, OPER) do {                      \
  if (!(src1->getType() == Type::Int || src1->getType() == Type::Bool) || \
      !(src2->getType() == Type::Int || src2->getType() == Type::Bool)) { \
    CG_PUNT(INSTR);                                                       \
  }                                                                       \
  register_name_t dstReg  = dst->getAssignedLoc();                        \
  register_name_t src1Reg = src1->getAssignedLoc();                       \
  register_name_t src2Reg = src2->getAssignedLoc();                       \
  /* 2 consts */                                                          \
  if (src1->isConst() && src2->isConst()) {                               \
    int64 src1Const = src1->getConstValAsRawInt();                        \
    int64 src2Const = src2->getConstValAsRawInt();                        \
    m_as.mov_imm64_reg(src1Const OPER src2Const, dstReg);                 \
  /* 1 const, 1 reg */                                                    \
  } else if (src2->isConst()) {                                           \
    int64 src2Const = src2->getConstValAsRawInt();                        \
    if (src1Reg != dstReg) {                                              \
      m_as.mov_reg64_reg64(src1Reg, dstReg);                              \
      if (m_curTrace->isMain()) {                                         \
        TRACE(3, "[counter] 1 reg move in NON_COMMUTATIVE_INT_OP\n");     \
      }                                                                   \
    }                                                                     \
    m_as. INSTR ## _imm64_reg64(src2Const, dstReg);                       \
  } else if (src1->isConst()) {                                           \
    int64 src1Const = src1->getConstValAsRawInt();                        \
    if (dstReg != src2Reg) {                                              \
      m_as.mov_imm64_reg(src1Const, dstReg);                              \
      m_as. INSTR ## _reg64_reg64(src2Reg, dstReg);                       \
    } else {                                                              \
      m_as.mov_imm64_reg(src1Const, reg::rScratch);                       \
      m_as. INSTR ## _reg64_reg64(src2Reg, reg::rScratch);                \
      m_as.mov_reg64_reg64(reg::rScratch, dstReg);                        \
      if (m_curTrace->isMain()) {                                         \
        TRACE(3, "[counter] 1 reg move in NON_COMMUTATIVE_INT_OP\n");     \
      }                                                                   \
    }                                                                     \
  /* both src1 and src2 are regs */                                       \
  } else {                                                                \
    if (dstReg != src1Reg && dstReg != src2Reg) {                         \
      m_as.mov_reg64_reg64(src1Reg, dstReg);                              \
      if (m_curTrace->isMain()) {                                         \
        TRACE(3, "[counter] 1 reg move in NON_COMMUTATIVE_INT_OP\n");     \
      }                                                                   \
      m_as. INSTR ## _reg64_reg64(src2Reg, dstReg);                       \
    } else {                                                              \
      if (dstReg == src1Reg) {                                            \
        m_as. INSTR ## _reg64_reg64(src2Reg, dstReg);                     \
      } else {                                                            \
        ASSERT(dstReg == src2Reg);                                        \
        m_as.mov_reg64_reg64(src1Reg, reg::rScratch);                     \
        if (m_curTrace->isMain()) {                                       \
          TRACE(3, "[counter] 1 reg move in NON_COMMUTATIVE_INT_OP\n");   \
        }                                                                 \
        m_as. INSTR ## _reg64_reg64(src2Reg, reg::rScratch);              \
        m_as.mov_reg64_reg64(reg::rScratch, dstReg);                      \
        if (m_curTrace->isMain()) {                                       \
          TRACE(3, "[counter] 1 reg move in NON_COMMUTATIVE_INT_OP\n");   \
        }                                                                 \
      }                                                                   \
    }                                                                     \
  }                                                                       \
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

Address CodeGenerator::cgOpGt(IRInstruction* inst) {
  Address start = m_as.code.frontier;
  UNUSED SSATmp* dst   = inst->getDst();
  UNUSED SSATmp* src1  = inst->getSrc(0);
  UNUSED SSATmp* src2  = inst->getSrc(1);

  CG_PUNT(Gt);

  return start;
}

Address CodeGenerator::cgOpGte(IRInstruction* inst) {
  Address start = m_as.code.frontier;
  UNUSED SSATmp* dst   = inst->getDst();
  UNUSED SSATmp* src1  = inst->getSrc(0);
  UNUSED SSATmp* src2  = inst->getSrc(1);

  CG_PUNT(Gte);

  return start;
}

Address CodeGenerator::cgOpLt(IRInstruction* inst) {
  Address start = m_as.code.frontier;
  UNUSED SSATmp* dst   = inst->getDst();
  UNUSED SSATmp* src1  = inst->getSrc(0);
  UNUSED SSATmp* src2  = inst->getSrc(1);

  CG_PUNT(Lt);

  return start;
}

Address CodeGenerator::cgOpLte(IRInstruction* inst) {
  Address start = m_as.code.frontier;
  UNUSED SSATmp* dst   = inst->getDst();
  UNUSED SSATmp* src1  = inst->getSrc(0);
  UNUSED SSATmp* src2  = inst->getSrc(1);

  CG_PUNT(Lte);

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


// eq - is it = or !=
// same - is it == or ===
template <bool eq, bool same>
int64 cgStringEqHelper2(const StringData* s1, const StringData* s2) {
  if (!same) {
    int64 s1i, s2i;
    double  s1d, s2d;
    auto nt1 = s1->isNumericWithVal(s1i, s1d, false);
    if (nt1 != KindOfNull) {
      auto nt2 = s2->isNumericWithVal(s2i, s2d, false);
      if (nt2 != KindOfNull) {
        if (nt1 == KindOfInt64 && nt2 == KindOfInt64) {
          return eq ? s1i == s2i : s1i != s2i;
        } else if (nt1 == KindOfInt64) {
          return eq ? s1i == s2d : s1i != s2d;
        } else if (nt2 == KindOfInt64) {
          return eq ? s1d == s2i : s1d != s2i;
        }
        return eq ? s1d == s2d : s1d != s2d;
      }
      return eq ? false : true;
    }
  }
  bool ret = s1->same(s2);
  return eq ? ret : !ret;
}

int64 cgStringEqHelper(const StringData* s1, const StringData* s2) {
  return cgStringEqHelper2<true, false>(s1, s2);
}

int64 cgStringNeqHelper(const StringData* s1, const StringData* s2) {
  return cgStringEqHelper2<false, false>(s1, s2);
}

int64 cgStringSameHelper(const StringData* s1, const StringData* s2) {
  return cgStringEqHelper2<true, true>(s1, s2);
}

int64 cgStringNSameHelper(const StringData* s1, const StringData* s2) {
  return cgStringEqHelper2<false, true>(s1, s2);
}

int64 cgIntEqStringHelper(const StringData* s, int64 i) {
  int64 si;
  double sd;
  auto st = s->isNumericWithVal(si, sd, true);

  if (st == KindOfDouble) {
    return i == sd;
  }
  if (st == KindOfNull) si = 0;
  return i == si;
}

int64 cgIntNeqStringHelper(const StringData* s, int64 i) {
  return !cgIntEqStringHelper(s, i);
}

Address CodeGenerator::cgOpEqHelper(IRInstruction* inst, bool eq) {
  Address start = m_as.code.frontier;
  UNUSED SSATmp* dst   = inst->getDst();
  UNUSED SSATmp* src1  = inst->getSrc(0);
  UNUSED SSATmp* src2  = inst->getSrc(1);

  Type::Tag type1 = src1->getType();
  Type::Tag type2 = src2->getType();

  register_name_t dstReg = dst->getAssignedLoc();

  if (type1 == type2 &&
      (type1 == Type::Int ||
       type1 == Type::Bool)) {
    bool c1 = src1->isConst();
    bool c2 = src2->isConst();

    if (c1 && c2) {
      bool val = src1->getConstValAsRawInt() == src2->getConstValAsRawInt();
      m_as.mov_imm64_reg(val == eq, dstReg);
    } else {
      bool dstRegIsBool = false;
      if (c1 || c2) {
        auto srcReg = c1 ? src2->getAssignedLoc() : src1->getAssignedLoc();
        if (srcReg == dstReg && type1 == Type::Bool) dstRegIsBool = true;
        auto imm = c1 ? src1->getConstValAsRawInt()
                      : src2->getConstValAsRawInt();
        m_as.cmp_imm64_reg64(imm, srcReg);
      } else {
        auto src1Reg = src1->getAssignedLoc();
        auto src2Reg = src2->getAssignedLoc();
        if ((src1Reg == dstReg || src2Reg == dstReg) &&
            type1 == Type::Bool) {
          dstRegIsBool = true;
        }
        m_as.cmp_reg64_reg64(src1Reg, src2Reg);
      }
      if (eq) m_as.sete (dstReg);
      else    m_as.setne(dstReg);
      if (!dstRegIsBool) m_as.and_imm64_reg64(0xff, dstReg);
    }
  } else if ((type1 == Type::Bool && type2 == Type::Int) ||
             (type2 == Type::Bool && type1 == Type::Int)) {
    if (type2 == Type::Bool && type1 == Type::Int) {
      std::swap(src1, src2);
      std::swap(type1, type2);
    }
    // type1 is bool and type2 is int

    bool c1 = src1->isConst();
    bool c2 = src2->isConst();

    if (c1 && c2) {
      bool val =
        src1->getConstValAsBool() == (bool)src2->getConstValAsRawInt();
      m_as.mov_imm64_reg(val == eq, dstReg);
    } else if (c1) {
      bool val = src1->getConstValAsBool();
      auto src2Reg = src2->getAssignedLoc();
      m_as.test_reg64_reg64(src2Reg, src2Reg);
      if (eq == val) m_as.setnz(dstReg);
      else           m_as.setz (dstReg);
      m_as.and_imm64_reg64(0xff, dstReg);
    } else if (c2) {
      auto src1Reg = src1->getAssignedLoc();
      auto val = src2->getConstValAsRawInt();

      if (dstReg == src1Reg) {
        if (eq == (bool)val) ; // do nothing
        else m_as.xor_imm64_reg64(0x1, dstReg);
      } else {
        m_as.mov_reg64_reg64(src1Reg, dstReg);
        if (eq != (bool)val) m_as.xor_imm64_reg64(0x1, dstReg);
      }
    } else {
      auto src1Reg = src1->getAssignedLoc();
      auto src2Reg = src2->getAssignedLoc();

      if (dstReg == src1Reg) {
        m_as.mov_reg64_reg64(src1Reg, reg::rScratch);
        m_as.test_reg64_reg64(src2Reg, src2Reg);
        if (eq) m_as.setz (dstReg);
        else    m_as.setnz(dstReg);
        m_as.xor_reg64_reg64(reg::rScratch, dstReg);
      } else {
        m_as.test_reg64_reg64(src2Reg, src2Reg);
        if (eq) m_as.setz (dstReg);
        else    m_as.setnz(dstReg);
        m_as.xor_reg64_reg64(src1Reg, dstReg);
        m_as.and_imm64_reg64(0xff, dstReg);
      }
    }
  } else if (Type::isString(type1) && Type::isString(type2)) {
    ArgGroup args;
    args.ssa(src1).ssa(src2);
    if (eq) cgCallHelper(m_as, (TCA)cgStringEqHelper,  dst, true, args);
    else    cgCallHelper(m_as, (TCA)cgStringNeqHelper, dst, true, args);
  } else if ((type1 == Type::Int && Type::isString(type2)) ||
             (type2 == Type::Int && Type::isString(type1))) {
    if (type1 == Type::Int) {
      std::swap(type1, type2);
      std::swap(src1, src2);
      assert(false); // due to canonicalization, this should never happen
    }
    // type1 is now String, type2 is Int

    if (src1->isConst() && src2->isConst()) {
      // this should be dealt with in the simplifier
      if (eq) CG_PUNT(Eq_const_int_const_string);
      else    CG_PUNT(Neq_const_int_const_string);
    } else if (src1->isConst()) {
      auto str = src1->getConstValAsStr();
      int64 si;
      double sd;
      auto st = str->isNumericWithVal(si, sd, true);

      if (st == KindOfDouble) {
        ArgGroup args;
        args.ssa(src1).ssa(src2);

        if (eq) cgCallHelper(m_as, (TCA)cgIntEqStringHelper,  dst, true, args);
        else    cgCallHelper(m_as, (TCA)cgIntNeqStringHelper, dst, true, args);
      } else {
        if (st == KindOfNull) {
          si = 0;
        }
        m_as.cmp_imm64_reg64(si, src2->getAssignedLoc());
        if (eq) m_as.setz (dstReg);
        else    m_as.setnz(dstReg);
        m_as.and_imm64_reg64(0xff, dstReg);
      }
    } else {
      ArgGroup args;
      args.ssa(src1).ssa(src2);

      if (eq) cgCallHelper(m_as, (TCA)cgIntEqStringHelper,  dst, true, args);
      else    cgCallHelper(m_as, (TCA)cgIntNeqStringHelper, dst, true, args);
    }
  } else if (type1 == Type::Obj && type2 == Type::Obj) {
    if (eq) CG_PUNT(Eq_obj);
    else    CG_PUNT(Neq_obj);
  } else if (type1 == Type::Arr && type2 == Type::Arr) {
    if (eq) CG_PUNT(Eq_arr);
    else    CG_PUNT(Neq_arr);
  } else {
    if (eq) CG_PUNT(Eq);
    else    CG_PUNT(Neq);
  }

  return start;
}

Address CodeGenerator::cgOpEq(IRInstruction* inst) {
  return cgOpEqHelper(inst, true);
}

Address CodeGenerator::cgOpNeq(IRInstruction* inst) {
  return cgOpEqHelper(inst, false);
}

Address CodeGenerator::cgOpSameHelper(IRInstruction* inst, bool eq) {
  Address start = m_as.code.frontier;
  UNUSED SSATmp* dst   = inst->getDst();
  UNUSED SSATmp* src1  = inst->getSrc(0);
  UNUSED SSATmp* src2  = inst->getSrc(1);

  Type::Tag type1 = src1->getType();
  Type::Tag type2 = src2->getType();


  if (Type::isString(type1) && Type::isString(type2)) {
    ArgGroup args;
    args.ssa(src1).ssa(src2);
    if (eq) cgCallHelper(m_as, (TCA)cgStringSameHelper,  dst, true, args);
    else    cgCallHelper(m_as, (TCA)cgStringNSameHelper, dst, true, args);
  } else if (type1 == Type::Obj && type2 == Type::Obj) {
    register_name_t dstReg = dst->getAssignedLoc();
    // Objects cannot be const
    auto src1Reg = src1->getAssignedLoc();
    auto src2Reg = src2->getAssignedLoc();
    m_as.cmp_reg64_reg64(src1Reg, src2Reg);
    if (eq) m_as.sete (dstReg);
    else    m_as.setne(dstReg);
    m_as.and_imm64_reg64(0xff, dstReg);
  } else {
    if (eq) CG_PUNT(Same);
    else    CG_PUNT(NSame);
  }

  return start;
}

Address CodeGenerator::cgOpSame(IRInstruction* inst) {
  return cgOpSameHelper(inst, true);
}

Address CodeGenerator::cgOpNSame(IRInstruction* inst) {
  return cgOpSameHelper(inst, false);
}

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

  register_name_t dstReg = dst->getAssignedLoc();
  register_name_t srcReg = src->getAssignedLoc();

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
        m_as.mov_reg64_reg64(srcReg, dstReg);
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
        cgCallHelper(m_as, (TCA)strToIntHelper, dst, false, args);
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
        m_as.setne(dstReg);
        m_as.and_imm64_reg64(0xff, dstReg);
      } else {
        m_as.xor_reg64_reg64(dstReg, dstReg);
        m_as.test_reg64_reg64(srcReg, srcReg);
        m_as.setne(dstReg);
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
        CG_PUNT(Conv);
      }
      args.ssa(src);
      cgCallHelper(m_as, helper, dst, false, args);
    }
    return start;
  }

  if (Type::isString(toType)) {
    if (fromType == Type::Int) {
      // Int -> Str
      ArgGroup args;
      args.ssa(src);
      cgCallHelper(m_as, (TCA)intToStringHelper, dst, false, args);
    } else if (fromType == Type::Bool) {
      // Bool -> Str
      m_as.test_reg64_reg64(srcReg, srcReg);
      m_as.mov_imm64_reg((uint64)StringData::GetStaticString(""),
                         dstReg);
      m_as.mov_imm64_reg((uint64)StringData::GetStaticString("1"),
                         LinearScan::rScratch);
      m_as.cmov_reg64_reg64(CC_NZ, LinearScan::rScratch, dstReg);
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

  register_name_t srcReg = src->getAssignedLoc();
  register_name_t dstReg = dst->getAssignedLoc();

  ASSERT(srcReg != reg::noreg);
  ASSERT(dstReg != reg::noreg);

  if (srcReg != dstReg) {
    m_as.mov_reg64_reg64(srcReg, dstReg);
  }
  emitDerefIfVariant(m_as, dstReg);
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
    CG_PUNT(Unbox);
  }
  register_name_t srcReg = src->getAssignedLoc();
  register_name_t dstReg = dst->getAssignedLoc();
  if (Type::isBoxed(srcType)) {
    emitDeref(m_as, srcReg, dstReg);
  } else if (srcType == Type::Gen) {
    CG_PUNT(Unbox);
    // XXX The following is wrong becuase it over-writes srcReg
    emitDerefIfVariant(m_as, srcReg);
    m_as.mov_reg64_reg64(srcReg, dstReg);
    if (m_curTrace->isMain()) {
      TRACE(3, "[counter] 1 reg move in cgUnbox\n");
    }
  } else {
    ASSERT(false);
    CG_PUNT(Unbox);
  }
  if (genIncRef && Type::isRefCounted(dstType) &&
      dst->getAssignedLoc() != reg::noreg) {
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
  register_name_t dstReg = dst->getAssignedLoc();
  if (dstReg == reg::noreg) {
    // happens if LdFixedFunc and FCAll not in same trace
    dstReg = LinearScan::rScratch;
    dst->setAssignedLoc(dstReg, 0);
  }
  register_name_t actRecReg = actRec->getAssignedLoc();
  ASSERT(actRecReg != reg::noreg);
  using namespace TargetCache;
  const StringData* name = methodName->getConstValAsStr();
  CacheHandle ch = allocFixedFunction(name);
  size_t funcCacheOff = ch + offsetof(FixedFuncCache, m_func);
  m_as.load_reg64_disp_reg64(LinearScan::rTlPtr, funcCacheOff, dstReg);
  m_as.test_reg64_reg64(dstReg, dstReg);
  // jz off to the helper call in astubs
  m_as.jcc(CC_E, m_astubs.code.frontier);
  // this helper simply raises an error
  cgCallHelper(m_astubs, (TCA)FixedFuncCache::lookupFailed, dst, true,
               ArgGroup().immPtr(name));
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

  register_name_t actRecReg = actRec->getAssignedLoc();
  ASSERT(actRecReg != reg::noreg);
  TargetCache::CacheHandle ch = TargetCache::FuncCache::alloc();
  register_name_t dstReg = dst->getAssignedLoc();
  if (dstReg == reg::noreg) {
    // this happens if LdFixedFunc and FCAll are not in the same trace
    // TODO: try to get rax instead to avoid a move after the call
    dstReg = LinearScan::rScratch;
  }
  // raises an error if function not found
  cgCallHelper(m_as, (TCA)FuncCache::lookup, dstReg, true,
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
                               true,
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
  register_name_t dstReg = dst->getAssignedLoc();
  register_name_t objReg = obj->getAssignedLoc();
  m_as.load_reg64_disp_reg64(objReg, ObjectData::getVMClassOffset(), dstReg);

  return start;
}

Address CodeGenerator::cgLdCachedClass(IRInstruction* inst) {
  Address start = m_as.code.frontier;
  SSATmp* dst   = inst->getDst();
  SSATmp* className = inst->getSrc(0);
  ASSERT(className->isConst() && className->getType() == Type::StaticStr);

  register_name_t dstReg = dst->getAssignedLoc();
  const StringData* classNameString = className->getConstValAsStr();
  TargetCache::allocKnownClass(classNameString);
  TargetCache::CacheHandle ch = TargetCache::allocKnownClass(classNameString);
  m_as.load_reg64_disp_reg64(LinearScan::rTlPtr, ch, dstReg);

  return start;
}

Address CodeGenerator::cgRetVal(IRInstruction* inst) {
  Address start = m_as.code.frontier;
  SSATmp* dstSp = inst->getDst();
  SSATmp* fp    = inst->getSrc(0);
  SSATmp* val   = inst->getNumSrcs() > 1 ? inst->getSrc(1) : NULL;

  register_name_t dstSpReg = dstSp->getAssignedLoc();
  register_name_t fpReg    =    fp->getAssignedLoc();

  if (val) {
    // Store return value at the top of the caller's eval stack
    // (a) Store the type
    if (val->getType() != Type::Gen && val->getType() != Type::Cell) {
      DataType valDataType = Type::toDataType(val->getType());
      m_as.store_imm32_disp_reg(valDataType, AROFF(m_r) + TVOFF(m_type), fpReg);
    } else {
      register_name_t typeReg = val->getAssignedLoc(1);
      m_as.store_reg32_disp_reg64(typeReg, AROFF(m_r) + TVOFF(m_type), fpReg);
    }

    // (b) Store the actual value (not necessary when storing Null)
    if (val->getType() != Type::Null) {
      if (val->getInstruction()->isDefConst()) {
        int64 intVal = val->getConstValAsRawInt();
        m_as.store_imm64_disp_reg64(intVal,  AROFF(m_r) + TVOFF(m_data), fpReg);
      } else {
        register_name_t valReg = val->getAssignedLoc();
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
  SSATmp* retAddr = inst->getDst();
  SSATmp* fp    = inst->getSrc(0);

  register_name_t retAddrReg = retAddr->getAssignedLoc(0);
  ASSERT(retAddrReg != reg::noreg);

  register_name_t fpReg = fp->getAssignedLoc(0);
  ASSERT(fpReg != reg::noreg);

  m_as.load_reg64_disp_reg64(fpReg, AROFF(m_savedRip), retAddrReg);

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
    return cgCallHelper(as, (TCA)checkStackAR, reg::noreg, false,
                        ArgGroup().ssa(sp).imm(numElems));
  } else {
    return cgCallHelper(as, (TCA)checkStack, reg::noreg, false,
                        ArgGroup().ssa(sp).imm(numElems));
  }
}

void checkCell(Cell* base, uint32 index) {
  TypedValue* tv = (TypedValue*)(base + index);
  assert(tvIsPlausible(tv));
  DataType t = tv->m_type;
  if (IS_REFCOUNTED_TYPE(t)) {
    assert(tv->m_data.pstr->getCount() > 0);
  }
}

Address CodeGenerator::emitCheckCell(CodeGenerator::Asm& as,
                                     SSATmp* sp,
                                     uint32 index) {
  return cgCallHelper(as, (TCA)checkCell, reg::noreg, false,
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

void CodeGenerator::emitTraceRet(CodeGenerator::Asm& as,
                                 register_name_t retAddrReg) {
  as.pushr(retAddrReg);
  // call to a trace function
  as.mov_reg64_reg64(retAddrReg, reg::rdx);
  as.mov_reg64_reg64(rVmFp, reg::rdi);
  as.mov_reg64_reg64(rVmSp, reg::rsi);
  // do the call; may use a trampoline
  m_tx64->emitCall(as, (TCA)traceRet);
  as.popr(retAddrReg);
}

Address CodeGenerator::cgRetCtrl(IRInstruction* inst) {
  Address start = m_as.code.frontier;
  SSATmp* sp    = inst->getSrc(0);
  SSATmp* fp    = inst->getSrc(1);
  SSATmp* retAddr = inst->getSrc(2);


  register_name_t retAddrReg = retAddr->getAssignedLoc();

  // Make sure rVmFp and rVmSp are set appropriately
  if (sp->getAssignedLoc() != LinearScan::rVmSP) {
    if (m_curTrace->isMain()) {
      TRACE(3, "[counter] 1 reg move in cgRetCtrl\n");
    }
    m_as.mov_reg64_reg64(sp->getAssignedLoc(), LinearScan::rVmSP);
  }
  if (fp->getAssignedLoc() != LinearScan::rVmFP) {
    if (m_curTrace->isMain()) {
      TRACE(3, "[counter] 1 reg move in cgRetCtrl\n");
    }
    m_as.mov_reg64_reg64(fp->getAssignedLoc(), LinearScan::rVmFP);
  }

  emitTraceRet(m_as, retAddrReg);
  // Return control to caller
  m_as.jmp_reg(retAddrReg);
  m_as.ud2();
  return start;
}

Address CodeGenerator::cgFreeActRec(IRInstruction* inst) {
  Address start = m_as.code.frontier;
  SSATmp* outFp = inst->getDst();
  SSATmp* inFp  = inst->getSrc(0);

  register_name_t  inFpReg =  inFp->getAssignedLoc();
  register_name_t outFpReg = outFp->getAssignedLoc();

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

  ASSERT(dst->getNumAssignedLocs() == src->getNumAssignedLocs());
  for (uint32 locIndex = 0; locIndex < src->getNumAssignedLocs(); ++locIndex) {
    register_name_t srcReg = src->getAssignedLoc(locIndex);
    if (dst->isAssignedMmxReg(locIndex)) {
      m_as.mov_reg64_mmx(srcReg, dst->getMmxReg(locIndex));
    } else {
      m_as.store_reg64_disp_reg64(srcReg,
                                  sizeof(uint64) * dst->getSpillLoc(locIndex),
                                  reg::rsp);
    }
  }
  return start;
}

Address CodeGenerator::cgReload(IRInstruction* inst) {
  Address start = m_as.code.frontier;
  SSATmp* dst   = inst->getDst();
  SSATmp* src   = inst->getSrc(0);

  ASSERT(dst->getNumAssignedLocs() == src->getNumAssignedLocs());
  for (uint32 locIndex = 0; locIndex < src->getNumAssignedLocs(); ++locIndex) {
    register_name_t dstReg = dst->getAssignedLoc(locIndex);
    if (src->isAssignedMmxReg(locIndex)) {
      m_as.mov_mmx_reg64(src->getMmxReg(locIndex), dstReg);
    } else {
      m_as.load_reg64_disp_reg64(reg::rsp,
                                 sizeof(uint64) * src->getSpillLoc(locIndex),
                                 dstReg);
    }
  }
  return start;
}

Address CodeGenerator::cgStPropWork(IRInstruction* inst, bool genTypeStore) {
  Address start = m_as.code.frontier;
  SSATmp* obj   = inst->getSrc(0);
  SSATmp* prop  = inst->getSrc(1);
  SSATmp* src   = inst->getSrc(2);

  register_name_t objReg = obj->getAssignedLoc();
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
  return cgStore(addr->getAssignedLoc(), offset->getConstValAsInt(),
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

  Address start = cgStore(addr->getAssignedLoc(), 0, src, genStoreType);

  register_name_t destReg = dest->getAssignedLoc();
  register_name_t addrReg = addr->getAssignedLoc();

  if (destReg != reg::noreg && destReg != addrReg) {
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
  register_name_t baseReg = homeInstr->getSrc(0)->getAssignedLoc();
  int64_t index = homeInstr->getLocal()->getId();
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
  if (sp->getAssignedLoc() != LinearScan::rVmSP) {
    if (m_curTrace->isMain()) {
      TRACE(3, "[counter] 1 reg move in cgExitTrace\n");
    }
    outputAsm.mov_reg64_reg64(sp->getAssignedLoc(), LinearScan::rVmSP);
  }
  if (fp->getAssignedLoc() != LinearScan::rVmFP) {
    if (m_curTrace->isMain()) {
      TRACE(3, "[counter] 1 reg move in cgExitTrace\n");
    }
    outputAsm.mov_reg64_reg64(fp->getAssignedLoc(), LinearScan::rVmFP);
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

        m_astubs.setcc(cc, serviceReqArgRegs[4]);
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

static
void emitAssertRefCount(CodeGenerator::Asm& as, register_name_t base) {
  as.cmp_imm32_disp_reg32(HPHP::RefCountStaticValue,
                          TVOFF(_count),
                          base);
  TCA patch = as.code.frontier;
  as.jcc8(CC_BE, patch);
  as.int3();
  as.patchJcc8(patch, as.code.frontier);
}

static
void emitIncRef(CodeGenerator::Asm& as, register_name_t base) {
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
  register_name_t base = src->getAssignedLoc(0);
  Address start = m_as.code.frontier;
  if (type == Type::Obj || Type::isBoxed(type)) {
    emitIncRef(m_as, base);
    register_name_t dstReg = dst->getAssignedLoc(0);
    if (dstReg != reg::noreg && dstReg != base) {
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
      CG_PUNT(cgIncRef); // for now
      // may be variant or cell
      m_as.cmp_imm32_disp_reg32(KindOfRefCountThreshold, TVOFF(m_type), base);
      patch1 = m_as.code.frontier;
      m_as.jcc8(CC_LE, patch1);
    }
    if (type == Type::Cell) {
      register_name_t typ = src->getAssignedLoc(1);
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
    register_name_t dstValueReg = dst->getAssignedLoc(0);
    if (type == Type::Cell) {
      register_name_t srcTypeReg = src->getAssignedLoc(1);
      register_name_t dstTypeReg = dst->getAssignedLoc(1);
      // Be careful here. We need to move values from one pair
      // of registers to another.
      // dstValueReg = base
      // dstTypeReg = srcTypeReg
      if (dstValueReg != reg::noreg && base != dstValueReg) {
        if (srcTypeReg == dstValueReg) {
          // use the scratch reg to avoid clobbering srcTypeReg
          m_as.mov_reg64_reg64(srcTypeReg, LinearScan::rScratch);
          if (m_curTrace->isMain()) {
            TRACE(3, "[counter] 1 reg move in cgIncRefWork\n");
          }
          srcTypeReg = LinearScan::rScratch;
        }
        m_as.mov_reg64_reg64(base, dstValueReg);
        if (m_curTrace->isMain()) {
          TRACE(3, "[counter] 1 reg move in cgIncRefWork\n");
        }
      }
      if (dstTypeReg != reg::noreg && srcTypeReg != dstTypeReg) {
        m_as.mov_reg64_reg64(srcTypeReg, dstTypeReg);
        if (m_curTrace->isMain()) {
          TRACE(3, "[counter] 1 reg move in cgIncRefWork\n");
        }
      }
    } else {
      if (dstValueReg != reg::noreg && dstValueReg != base) {
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
  return cgDecRefMem(type, sp->getAssignedLoc(),
                     index->getConstValAsInt() * sizeof(Cell), exit);
}

Address CodeGenerator::cgDecRefThis(IRInstruction* inst) {
  Address start = m_as.code.frontier;
  SSATmp* fp    = inst->getSrc(0);
  LabelInstruction* exit = inst->getLabel();
  register_name_t fpReg = fp->getAssignedLoc();
  register_name_t scratchReg = LinearScan::rScratch;

  // Load AR->m_this into rScratch
  m_as.load_reg64_disp_reg64(fpReg, AROFF(m_this), scratchReg);

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
  register_name_t fpReg = homeInstr->getSrc(0)->getAssignedLoc();
  int64_t index = homeInstr->getLocal()->getId();

  return cgDecRefMem(type, fpReg, -((index + 1) * sizeof(Cell)), exit);
}

Address CodeGenerator::cgDecRefLocals(IRInstruction* inst) {
  SSATmp* fp = inst->getSrc(0);
  SSATmp* numLocals = inst->getSrc(1);

  return cgCallHelper(m_as, (TCA)frame_free_locals_no_this, reg::noreg, true,
                      ArgGroup().ssa(fp).ssa(numLocals));
}

Address CodeGenerator::cgDecRefLocalsThis(IRInstruction* inst) {
  SSATmp* fp = inst->getSrc(0);
  SSATmp* numLocals = inst->getSrc(1);

  return cgCallHelper(m_as, (TCA)frame_free_locals, reg::noreg, true,
                      ArgGroup().ssa(fp).ssa(numLocals));
}

Address CodeGenerator::getDtor(DataType type) {
  switch (type) {
    case KindOfString  : return (Address)tv_release_str;
    case KindOfArray   : return (Address)tv_release_arr;
    case KindOfObject  : return (Address)tv_release_obj;
    case KindOfRef     : return (Address)tv_release_ref;
    default: assert(0);
  }
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
                                        register_name_t reg,
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
                                                 register_name_t dataReg,
                                                 LabelInstruction* exit) {
  ASSERT(Type::isRefCounted(type));

  Address patchStaticCheck = NULL;
  const register_name_t scratchReg = LinearScan::rScratch;

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
Address CodeGenerator::cgCheckRefCountedType(register_name_t typeReg) {

  m_as.cmp_imm32_reg32(KindOfRefCountThreshold, typeReg);

  Address addrToPatch =  m_as.code.frontier;
  m_as.jcc8(CC_LE, addrToPatch);

  return addrToPatch;
}

Address CodeGenerator::cgCheckRefCountedType(register_name_t baseReg,
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
                                          register_name_t dataReg,
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
                 reg::noreg, true,
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
Address CodeGenerator::cgDecRefDynamicType(register_name_t typeReg,
                                           register_name_t dataReg,
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
    cgCallHelper(m_astubs, getDtorTyped(), reg::noreg, true,
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
Address CodeGenerator::cgDecRefDynamicTypeMem(register_name_t baseReg,
                                              int64 offset,
                                              LabelInstruction* exit) {
  Address start = m_as.code.frontier;
  register_name_t scratchReg = LinearScan::rScratch;

  ASSERT(baseReg != scratchReg);

  // Emit check for ref-counted type
  Address patchTypeCheck = cgCheckRefCountedType(baseReg, offset);

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
    cgCallHelper(m_astubs, getDtorGeneric(), reg::noreg, true,
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
                                   register_name_t baseReg,
                                   int64 offset,
                                   LabelInstruction* exit) {
  Address start = m_as.code.frontier;
  register_name_t scratchReg = LinearScan::rScratch;
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


Address patchLabel(LabelInstruction* label, Address labelAddr) {
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

Address CodeGenerator::cgDecRefWork(IRInstruction* inst, bool genZeroCheck) {
  Address start = m_as.code.frontier;
  SSATmp* src   = inst->getSrc(0);
  if (!isRefCounted(src)) {
    return start;
  }
  LabelInstruction* exit = inst->getLabel();
  Type::Tag type = src->getType();
  if (Type::isStaticallyKnown(type)) {
    cgDecRefStaticType(type, src->getAssignedLoc(), exit, genZeroCheck);
  } else {
    cgDecRefDynamicType(src->getAssignedLoc(1),
                        src->getAssignedLoc(0),
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
  register_name_t spReg = sp->getAssignedLoc();
  register_name_t dstReg = dst->getAssignedLoc();
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
  register_name_t spReg = sp->getAssignedLoc();
  // actRec->m_this
  if (objOrCls->getType() == Type::ClassRef) {
    // store class
    m_as.store_imm64_disp_reg64(uintptr_t(objOrCls->getConstValAsClass()) | 1,
                                actRecAdjustment + AROFF(m_this),
                                spReg);
  } else if (objOrCls->getType() == Type::Obj) {
    // store this pointer
    m_as.store_reg64_disp_reg64(objOrCls->getAssignedLoc(),
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
  // ActRec::m_invName is encoded as a pointer with bottom bit set to 1
  // to distinguish it from m_varEnv and m_extrArgs
  uintptr_t invName = (magicName->getType() == Type::Null ?
                       0 : (uintptr_t(magicName->getConstValAsStr()) | 1));
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
    m_as. mov_imm64_reg((uint64)f, LinearScan::rScratch);
    m_as.store_reg64_disp_reg64(LinearScan::rScratch,
                                actRecAdjustment + AROFF(m_func),
                                spReg);
    if (func->getType() == Type::FuncClassRef) {
      // Fill in m_cls if provided with both func* and class*
      fprintf(stderr, "cgAllocActRec: const func->isFuncClassRef()\n");
      CG_PUNT(cgAllocActRec6);
    }
  } else {
    int offset_m_func = actRecAdjustment + AROFF(m_func);
    m_as.store_reg64_disp_reg64(func->getAssignedLoc(0),
                                offset_m_func,
                                spReg);
    if (func->getType() == Type::FuncClassRef) {
      int offset_m_cls = actRecAdjustment + AROFF(m_cls);
      m_as.store_reg64_disp_reg64(func->getAssignedLoc(1),
                                  offset_m_cls,
                                  spReg);
      setThis = true; /* m_this and m_cls are in a union */
    }
  }
  ASSERT(setThis);
  // actRec->m_savedRbp
  m_as.store_reg64_disp_reg64(fp->getAssignedLoc(),
                              actRecAdjustment + AROFF(m_savedRbp),
                              spReg);

  // actRec->m_numArgsAndCtorFlag
  ASSERT(nArgs->isConst());
  m_as.store_imm32_disp_reg(nArgs->getConstValAsInt(),
                            actRecAdjustment + AROFF(m_numArgsAndCtorFlag),
                            spReg);

  register_name_t dstReg = dst->getAssignedLoc();
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
  m_as.store_reg64_disp_reg64(func->getAssignedLoc(),
                              actRecAdjustment + AROFF(m_func),
                              sp->getAssignedLoc());
  m_as.store_reg64_disp_reg64(fp->getAssignedLoc(),
                              actRecAdjustment + AROFF(m_savedRbp),
                              sp->getAssignedLoc());

  ASSERT(nArgs->isConst());
  m_as.store_imm32_disp_reg(nArgs->getConstValAsInt(),
                            actRecAdjustment + AROFF(m_numArgsAndCtorFlag),
                            sp->getAssignedLoc());

  // XXX TODO: store the this or late bound class
  register_name_t dstReg = dst->getAssignedLoc();
  register_name_t spReg = sp->getAssignedLoc();
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
    cgCallHelper(m_as, (TCA)cgNewInstanceHelperCached, dst, true, args);
  } else {
    ArgGroup args;
    args.ssa(clsName).ssa(numParams).ssa(sp).ssa(fp);
    cgCallHelper(m_as, (TCA)cgNewInstanceHelper, dst, true, args);
  }
  return start;
}

Address CodeGenerator::cgCall(IRInstruction* inst) {

  Address start   = m_as.code.frontier;
  SSATmp* actRec  = inst->getSrc(0);
  SSATmp* returnBcOffset = inst->getSrc(1);
  uint32  numArgs = inst->getNumExtendedSrcs();
  ASSERT(numArgs > 0);
  SSATmp** args   = ((ExtendedInstruction*)inst)->getExtendedSrcs();

  SSATmp* func = args[0];
  // skip over func
  args++;  numArgs--;

  register_name_t spReg = actRec->getAssignedLoc();
  // put all outgoing arguments onto the VM stack
  int64 adjustment = (-(int64)numArgs) * sizeof(Cell);
  for (uint32 i = 0; i < numArgs; i++) {
    cgStore(spReg, -(i+1) * sizeof(Cell), args[i]);
  }
  // store the return IP into the outgoing actrec; this is patched below
  StoreImmPatcher retIp(m_as, (uint64_t)m_as.code.frontier, reg::rScratch,
                        AROFF(m_savedRip), spReg);
  // store the return bytecode offset into the outgoing actrec
  uint64 returnBc = returnBcOffset->getConstValAsInt();
  m_as.store_imm32_disp_reg(returnBc, AROFF(m_soff), spReg);
  if (adjustment != 0) {
    m_as.add_imm32_reg64(adjustment, spReg);
  }

  // Stash callee's rVmFp into rStashedAR for the callee's prologue
  if (numArgs == 0) {
    m_as.mov_reg64_reg64(LinearScan::rVmSP, LinearScan::rStashedAR);
    if (m_curTrace->isMain()) {
      TRACE(3, "[counter] 1 reg move in cgCall\n");
    }
  } else {
    m_as.lea_reg64_disp_reg64(LinearScan::rVmSP,
                            cellsToBytes(numArgs),
                            LinearScan::rStashedAR);
  }

  // HHIR:TODO SrcKey(func->getConstValAsFunc(), bcOff /*pc*/);
  SrcKey srcKey = SrcKey(m_lastMarker->getFunc(), m_lastMarker->getLabelId());
  bool isImmutable = (func->isConst() && func->getType() != Type::Null);
  const Func* funcd = isImmutable ? func->getConstValAsFunc() : NULL;
  m_tx64->emitBindCallHelper(LinearScan::rStashedAR, srcKey,
                             funcd, numArgs, isImmutable);

  // Patch the immediate in the code which defines the return address
  retIp.patch((uint64)m_as.code.frontier);
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

  register_name_t dstReg = dst->getAssignedLoc();
  register_name_t spReg = sp->getAssignedLoc();
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
  register_name_t fpReg = fp->getAssignedLoc();
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
  register_name_t dstReg = dst->getAssignedLoc();

  // the destination of LdThis could be dead but
  // the instruction itself still useful because
  // of the checks that it does (if it has a label).
  // So we need to make sure there is a dstReg for this
  // instruction.
  if (dstReg != reg::noreg) {
    // instruction's result is not dead
    m_as.load_reg64_disp_reg64(src->getAssignedLoc(),
                               AROFF(m_this),
                               dstReg);
  }

  if (label != NULL) {
    // we need to perform its checks
    if (curFunc()->cls() == NULL) {
      // test dst, dst
      // jz label

      if (dstReg == reg::noreg) {
        dstReg = reg::rScratch;
        m_as.load_reg64_disp_reg64(src->getAssignedLoc(),
                                   AROFF(m_this),
                                   dstReg);
      }
      m_as.test_reg64_reg64(dstReg, dstReg);
      emitFwdJcc(CC_Z, label);
    }
    // test dst, 0x01
    // jnz label
    if (dstReg == reg::noreg) {
      // TODO: Could also use a 32-bit test here
      m_as.test_imm64_disp_reg64(1, AROFF(m_this), src->getAssignedLoc());
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

  register_name_t dstReg = dst->getAssignedLoc();
  ASSERT(dstReg != reg::noreg);

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

  register_name_t srcReg = src->getAssignedLoc();
  register_name_t dstReg = dst->getAssignedLoc();

  m_as.load_reg64_disp_reg64(srcReg, AROFF(m_varEnv), dstReg);

  return start;
}

Address CodeGenerator::cgLdARFuncPtr(IRInstruction* inst) {
  Address start = m_as.code.frontier;
  SSATmp* dst   = inst->getDst();
  SSATmp* baseAddr = inst->getSrc(0);
  SSATmp* offset   = inst->getSrc(1);

  register_name_t dstReg  = dst->getAssignedLoc();
  register_name_t baseReg = baseAddr->getAssignedLoc();

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

  ASSERT(!(addr->isConst()));
  ASSERT(!(dest->isConst()));

  register_name_t addrReg = addr->getAssignedLoc();
  register_name_t destReg = dest->getAssignedLoc();

  int ldSize = getNativeTypeSize(dest->getType());
  if (offset->isConst()) {
    ASSERT(offset->getType() == Type::Int);
    if (ldSize == sz::qword) {
      m_as.load_reg64_disp_reg64(addrReg, offset->getConstValAsInt(), destReg);
    } else {
      ASSERT(ldSize == sz::byte);
      m_as.loadzxb_reg64_disp_reg64(addrReg, offset->getConstValAsInt(),
                                    destReg);
    }
  } else {
    register_name_t offsetReg = offset->getAssignedLoc();
    if (ldSize == sz::qword) {
      m_as.load_reg64_disp_index_reg64(addrReg, 0, offsetReg, destReg);
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
  register_name_t baseReg = inst->getSrc(0)->getAssignedLoc();
  int64 offset = inst->getSrc(1)->getConstValAsInt();
  SSATmp* value = inst->getSrc(2);

  int stSize = getNativeTypeSize(value->getType());
  if (value->isConst()) {
    if (stSize == sz::qword) {
      m_as.store_imm64_disp_reg64(value->getConstValAsInt(),
                                  offset,
                                  baseReg);
    } else {
      ASSERT(stSize == sz::byte);
      m_as.store_imm8_disp_reg(value->getConstValAsBool(),
                               offset,
                               baseReg);
    }
  } else {
    if (stSize == sz::qword) {
      m_as.store_reg64_disp_reg64(value->getAssignedLoc(),
                                  offset,
                                  baseReg);
    } else {
      // not supported by our assembler yet
      ASSERT(stSize == sz::byte);
      not_implemented();
    }
  }

  return start;
}

// if label is set and type is Cell, then cgLoadCell will generate
// a check that bails to the label if the loaded type is boxed.
Address CodeGenerator::cgLoadCell(Type::Tag type,
                                  SSATmp* dst,
                                  register_name_t base,
                                  int64_t off,
                                  LabelInstruction* label) {
  ASSERT(dst->getType() == Type::Cell || dst->getType() == Type::Gen);
  Address start = m_as.code.frontier;
  register_name_t valueDstReg = dst->getAssignedLoc(0);
  register_name_t typeDstReg = dst->getAssignedLoc(1);
  if (valueDstReg == reg::noreg) {
    // a dead load
    ASSERT(typeDstReg == reg::noreg);
    return start;
  }
  if (base == typeDstReg) {
    // Save base to rScratch, because base will be overwritten.
    m_as.mov_reg64_reg64(base, reg::rScratch);
    if (m_curTrace->isMain()) {
      TRACE(3, "[counter] 1 reg move in cgLoadCell\n");
    }
  }
  m_as.load_reg64_disp_reg32(base, off + TVOFF(m_type), typeDstReg);
  // Do not use rScratch any more before loading the value.
  if (label && type == Type::Cell) {
    // if we have a label and the type is Cell, then generate a guard
    // that bails if the local's value is a ref
    ASSERT(label);
    m_as.cmp_imm32_reg32(HPHP::KindOfRef, typeDstReg);
    emitFwdJcc(CC_GE, label);
  }
  if (base == typeDstReg) {
    m_as.load_reg64_disp_reg64(reg::rScratch, off + TVOFF(m_data), valueDstReg);
  } else {
    m_as.load_reg64_disp_reg64(base, off + TVOFF(m_data), valueDstReg);
  }
  return start;
}


Address CodeGenerator::cgStoreCell(register_name_t base,
                                   int64_t off,
                                   SSATmp* src) {
  Address start = m_as.code.frontier;
  ASSERT(src->getType() == Type::Cell || src->getType() == Type::Gen);
  m_as.store_reg64_disp_reg64(src->getAssignedLoc(0),
                              off + TVOFF(m_data),
                              base);
  // store the type
  m_as.store_reg32_disp_reg64(src->getAssignedLoc(1),
                              off + TVOFF(m_type),
                              base);
  return start;
}

// checkNotVar: If true, also emit check that loaded type is not Variant
// checkNotUninit: If true, also emit check that loaded type is not Uninit
Address CodeGenerator::cgStore(register_name_t base,
                               int64_t off,
                               SSATmp* src,
                               bool genStoreType) {
  Type::Tag type = src->getType();
  Address start = m_as.code.frontier;
  if (type == Type::Cell || type == Type::Gen) {
    return cgStoreCell(base, off, src);
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
  if (type == Type::Home) {
    IRInstruction* addrInst = src->getInstruction();
    if (addrInst->getOpcode() == LdHome) {
      // do an lea of the home
      ConstInstruction* homeInstr = (ConstInstruction*)addrInst;
      register_name_t baseReg = homeInstr->getSrc(0)->getAssignedLoc();
      int64_t index = homeInstr->getLocal()->getId();
      register_name_t tmpReg = reg::rScratch;
      m_as.lea_reg64_disp_reg64(baseReg, -((index + 1)*sizeof(Cell)), tmpReg);
      m_as.store_reg64_disp_reg64(tmpReg, off + TVOFF(m_data), base);
    } else {
      // deal with the general case of addrInst being ldStack
      ASSERT(addrInst->getOpcode() == LdStack);
      m_as.store_reg64_disp_reg64(src->getAssignedLoc(),
                                  off + TVOFF(m_data),
                                  base);
    }
  } else if (src->isConst()) {
    if (type == Type::Bool) {
      m_as.store_imm64_disp_reg64((int64)src->getConstValAsBool(),
                                  off + TVOFF(m_data),
                                  base);
    } else if (type == Type::Int) {
      m_as.store_imm64_disp_reg64(src->getConstValAsInt(),
                                  off + TVOFF(m_data),
                                  base);
    } else if (type == Type::Dbl) {
      CG_PUNT(cgStore); // not handled yet!
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
    if (type != Type::Null && type != Type::Uninit) {
      // no need to store any value for null or uninit
      m_as.store_reg64_disp_reg64(src->getAssignedLoc(),
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

Address CodeGenerator::cgLoad(Type::Tag type,
                              SSATmp* dst,
                              register_name_t base,
                              int64_t off,
                              LabelInstruction* label) {
  Address start = m_as.code.frontier;
  if (type == Type::Cell || type == Type::Gen) {
    return cgLoadCell(type, dst, base, off, label);
  }
  if (label != NULL && type != Type::Home) {
    // generate a guard for the type
    // see Translator.cpp checkType
    DataType dataType = Type::toDataType(type);
    if (IS_STRING_TYPE(dataType)) {
      // Note: this assumes String and StaticString are types 6 and 7
      m_as.load_reg64_disp_reg32(base,
                                 off + TVOFF(m_type),
                                 LinearScan::rScratch);
      m_as.and_imm32_reg32((signed char)(0xfe), LinearScan::rScratch);
      m_as.cmp_imm32_reg32(6, LinearScan::rScratch);
    } else {
      m_as.cmp_imm32_disp_reg32(dataType, off + TVOFF(m_type), base);
    }
    emitFwdJcc(CC_NE, label);
  }
  if (type == Type::Uninit || type == Type::Null) {
    return start; // these are constants
  }
  register_name_t dstReg = dst->getAssignedLoc();
  if (dstReg != reg::noreg) {
    // if dstReg == reg::noreg then the value of this load is dead
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

  register_name_t objReg = obj->getAssignedLoc();
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

  cgLoad(type, dst, addr->getAssignedLoc(), offset, label);
  return start;
}

Address CodeGenerator::cgLdRefNR(IRInstruction* inst) {
  Address   start = m_as.code.frontier;
  Type::Tag type  = inst->getType();
  SSATmp*   dst   = inst->getDst();
  SSATmp*   addr  = inst->getSrc(0);
  LabelInstruction* label = inst->getLabel();

  cgLoad(type, dst, addr->getAssignedLoc(), 0, label);
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
  int64_t index = homeInst->getLocal()->getId();
  const StringData* name = getCurrFunc()->localVarName(index);
  cgCallHelper(m_as,
               (TCA)HPHP::VM::Transl::raiseUndefVariable,
               (SSATmp*)NULL,
               true,
               ArgGroup().immPtr(name));
  return start;
}

static void getLocalRegOffset(SSATmp* src, register_name_t& reg, int64& off) {
  ConstInstruction* homeInstr =
    dynamic_cast<ConstInstruction*>(src->getInstruction());
  ASSERT(homeInstr && homeInstr->getOpcode() == LdHome);
  reg = homeInstr->getSrc(0)->getAssignedLoc();
  int64 index = homeInstr->getLocal()->getId();
  off = -cellsToBytes(index + 1);
}

Address CodeGenerator::cgLdLoc(IRInstruction* inst) {
  Address start = m_as.code.frontier;
  Type::Tag         type = inst->getType();
  SSATmp*           dst  = inst->getDst();
  LabelInstruction* label     = inst->getLabel();

  register_name_t fpReg;
  int64 offset;
  getLocalRegOffset(inst->getSrc(0), fpReg, offset);
  cgLoad(type, dst, fpReg, offset, label);
  return start;
}

Address CodeGenerator::cgLdLocAddr(IRInstruction* inst) {
  Address start = m_as.code.frontier;
  register_name_t fpReg;
  int64 offset;
  getLocalRegOffset(inst->getSrc(0), fpReg, offset);
  m_as.lea_reg64_disp_reg64(fpReg, offset,
                            inst->getDst()->getAssignedLoc());
  return start;
}

Address CodeGenerator::cgLdStackAddr(IRInstruction* inst) {
  Address start = m_as.code.frontier;
  m_as.lea_reg64_disp_reg64(inst->getSrc(0)->getAssignedLoc(),
                            cellsToBytes(inst->getSrc(1)->getConstValAsInt()),
                            inst->getDst()->getAssignedLoc());
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
                sp->getAssignedLoc(),
                index->getConstValAsInt()*sizeof(Cell),
                // no need to worry about boxed types if loading a cell
                type == Type::Cell ? NULL : label);
}

Address CodeGenerator::cgGuardType(IRInstruction* inst) {
  Address   start = m_as.code.frontier;
  UNUSED Type::Tag type = inst->getType();
  SSATmp*   dst   = inst->getDst();
  SSATmp*   src   = inst->getSrc(0);
  LabelInstruction* label = inst->getLabel();
  register_name_t dstReg = dst->getAssignedLoc(0);
  register_name_t srcValueReg = src->getAssignedLoc(0);
  register_name_t srcTypeReg = src->getAssignedLoc(1);
  ASSERT(srcTypeReg != reg::noreg);

  // compare srcTypeReg with type
  DataType dataType = Type::toDataType(type);
  if (IS_STRING_TYPE(dataType)) {
    // Note: this assumes String and StaticString are types 6 and 7
    // TODO: Delete this mov if srcTypeReg is not live out
    m_as.mov_reg64_reg64(srcTypeReg, LinearScan::rScratch);
    m_as.and_imm32_reg32((signed char)(0xfe), LinearScan::rScratch);
    m_as.cmp_imm32_reg32(6, LinearScan::rScratch);
  } else {
    m_as.cmp_imm32_reg32(dataType, srcTypeReg);
  }
  emitFwdJcc(CC_NE, label);

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
  register_name_t funcPtrReg = funcPtrTmp->getAssignedLoc();
  ASSERT(funcPtrReg != reg::noreg);

  ASSERT(nParamsTmp->getType() == Type::Int);
  register_name_t nParamsReg = nParamsTmp->getAssignedLoc();
  ASSERT(nParamsReg != reg::noreg);

  ASSERT(bitsPtrTmp->getType() == Type::Int);
  register_name_t bitsPtrReg = bitsPtrTmp->getAssignedLoc();
  ASSERT(bitsPtrReg != reg::noreg);

  ASSERT(firstBitNumTmp->isConst() && firstBitNumTmp->getType() == Type::Int);
  uint32 firstBitNum = (uint32)(firstBitNumTmp->getConstValAsInt());

  ASSERT(mask64Tmp->getType() == Type::Int);
  ASSERT(mask64Tmp->getInstruction()->getOpcode() == LdConst);
  register_name_t mask64Reg = mask64Tmp->getAssignedLoc();
  ASSERT(mask64Reg != reg::noreg);
  int64 mask64 = mask64Tmp->getConstValAsInt();

  ASSERT(vals64Tmp->getType() == Type::Int);
  ASSERT(vals64Tmp->getInstruction()->getOpcode() == LdConst);
  register_name_t vals64Reg = vals64Tmp->getAssignedLoc();
  ASSERT(vals64Reg != reg::noreg);
  int64 vals64 = vals64Tmp->getConstValAsInt();


  // Actually generate code

  register_name_t bitsValReg = LinearScan::rScratch;
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

  register_name_t dstReg = dst->getAssignedLoc();
  register_name_t objReg = obj->getAssignedLoc();

  ASSERT(objReg != reg::noreg);
  ASSERT(dstReg != reg::noreg);

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
  if (inst->getNumSrcs() < 3) {
    CG_PUNT(LdClsMethod);
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
  register_name_t funcDestReg  = dst->getAssignedLoc(0);
  register_name_t classDestReg = dst->getAssignedLoc(1);


  // Attempt to retrieve the func* and class* from cache
  m_as.load_reg64_disp_reg64(LinearScan::rTlPtr, ch, funcDestReg);
  m_as.test_reg64_reg64(funcDestReg, funcDestReg);
  // May have retrieved a NULL from the cache
  m_as.load_reg64_disp_reg64(LinearScan::rTlPtr,
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
                 true,
                 ArgGroup().imm(ch)            // Handle ch
                           .immPtr(ne)         // NamedEntity* np.second
                           .immPtr(cls)        // className
                           .immPtr(method)     // methodName
    );
    // recordInstrCall is done in cgCallHelper
    m_astubs.test_reg64_reg64(funcDestReg, funcDestReg);
    m_astubs.load_reg64_disp_reg64(LinearScan::rTlPtr,
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
    register_name_t dstReg = dst->getAssignedLoc();
    register_name_t srcReg = cls->getAssignedLoc();
    const StringData* clsNameString = clsName->getConstValAsStr();
    string sds(Util::toLower(clsNameString->data()) + ":" +
               string(propName->data(), propName->size()));
    StringData sd(sds.c_str(), sds.size(), AttachLiteral);
    CacheHandle ch = SPropCache::alloc(&sd);

    register_name_t tmpReg = dstReg == srcReg ? LinearScan::rScratch
                                              : dstReg;
    m_as.load_reg64_disp_reg64(LinearScan::rTlPtr, ch, tmpReg);
    m_as.test_reg64_reg64(tmpReg, tmpReg);
    // jz off to the helper call in astubs
    m_as.jcc(CC_E, m_astubs.code.frontier);
    if (tmpReg != dstReg) {
      m_as.mov_reg64_reg64(tmpReg, dstReg);
    }
    // this helper can raise an invalid property access error
    cgCallHelper(m_astubs, (TCA)SPropCache::lookup, dstReg, true,
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
  register_name_t dstReg = dst->getAssignedLoc();
  const StringData* classNameString = className->getConstValAsStr();
  TargetCache::CacheHandle ch = TargetCache::allocKnownClass(classNameString);
  m_as.load_reg64_disp_reg64(LinearScan::rTlPtr, ch, dstReg);
  m_as.test_reg64_reg64(dstReg, dstReg);
  // jz off to the helper call in astubs
  m_as.jcc(CC_E, m_astubs.code.frontier);
  {
    m_astubs.lea_reg64_disp_reg64(LinearScan::rTlPtr, ch, dstReg);
    // Passing only two arguments to lookupKnownClass, since the
    // third is ignored in the checkOnly==false case.
    ArgGroup args;
    args.reg(dstReg)
      .ssa(className);
    // this helper can raise an undefined class error
    cgCallHelper(m_astubs,
                 (TCA)TargetCache::lookupKnownClass<false>,
                 dst,
                 true,
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
  cgLoad(type, dst, LinearScan::rTlPtr, ch,
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

  register_name_t srcReg = src->getAssignedLoc();
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
    m_as.test_reg32_reg32(srcReg, srcReg);
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
  register_name_t typeReg = src->getAssignedLoc(1);

  ASSERT(label);
  ASSERT(typeReg != reg::noreg);

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

  CT_ASSERT(sizeof(RequestInjectionData::conditionFlags) == 8);

#if 0
  // ALIA:TODO
  m_as.test_imm64_disp_reg64(-1,
                             TargetCache::kConditionFlagsOff,
                             LinearScan::rTlPtr);
  emitFwdJcc(CC_NZ, label);
#else
  m_as.cmp_imm64_disp_reg64(0,
                            TargetCache::kConditionFlagsOff,
                            LinearScan::rTlPtr);
  emitFwdJcc(CC_NE, label);
#endif
  return start;
}

Address CodeGenerator::cgExitOnVarEnv(IRInstruction* inst) {
  Address start = m_as.code.frontier;
  SSATmp* fp    = inst->getSrc(0);
  LabelInstruction* label = inst->getLabel();

  ASSERT(!(fp->isConst()));

  register_name_t fpReg = fp->getAssignedLoc();
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
    cgCallHelper(m_as, (TCA)tvBoxHelper, dst, false,
                 ArgGroup().type(src)
                           .ssa(src));
  } else if (type < Type::Cell) {
    cgCallHelper(m_as, (TCA)tvBoxHelper, dst, false,
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
  return cgCallHelper(m_as, (TCA)fptr, reg::noreg, false,
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
  Type::Tag valType = val->getType();
  // TODO: Double check that these helpers don't re-enter
  if (keyType == Type::Int && valType == Type::Int) {
    cgCallHelper(m_as, (TCA)array_setm_ik1_iv, dst, false,
                 ArgGroup().imm(0)
                           .ssa(arr)
                           .ssa(key)
                           .ssa(val));
  } else if (keyType == Type::Int) {
    // decrefs the value but not the key
    cgCallHelper(m_as, (TCA)addElemIntKeyHelper, dst, false,
                 ArgGroup().ssa(arr)
                           .ssa(key)
                           .ssa(val)
                           .type(val));
  } else if (keyType == Type::Str || keyType == Type::StaticStr) {
    // decrefs the value but not the key
    cgCallHelper(m_as, (TCA)addElemStringKeyHelper, dst, false,
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
  cgCallHelper(m_as, (TCA)addNewElemHelper, dst, false,
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
    cgCallHelper(m_as, (TCA)fptr, dst, false,
                 ArgGroup().ssa(tl)
                           .ssa(tr));
  } else {
    if (lType >= Type::Obj || rType >= Type::Obj) {
      CG_PUNT(cgConcat);
    }
    cgCallHelper(m_as, (TCA)concat, dst, false,
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
  return cgCallHelper(m_as, (TCA)array_add, dst, false,
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

  register_name_t dstReg = reg::noreg;
  if (label) {
    dstReg = LinearScan::rScratch;
  }
  cgCallHelper(m_as, (TCA)interpOneHelper, dstReg, true,
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
  register_name_t newSpReg = inst->getDst()->getAssignedLoc();
  DEBUG_ONLY register_name_t spReg = sp->getAssignedLoc();
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
  cgCallHelper(m_as, (TCA)defFuncHelper, dst, true,
               ArgGroup().ssa(func));
  return start;
}

Address CodeGenerator::cgLdContThisOrCls(IRInstruction* inst) {
  not_reached();
}

Address CodeGenerator::cgCreateCont(IRInstruction* inst) {
  Address start = m_as.code.frontier;
  auto helper = curFunc()->isNonClosureMethod() ?
    VMExecutionContext::createContinuation<true> :
    VMExecutionContext::createContinuation<false>;
  cgCallHelper(m_as, (TCA)helper,
               inst->getDst(), false,
               ArgGroup().ssa(inst->getSrc(0))
                         .ssa(inst->getSrc(1))
                         .ssa(inst->getSrc(2))
                         .ssa(inst->getSrc(3)));
  return start;
}

Address CodeGenerator::cgFillContLocals(IRInstruction* inst) {
  Address start = m_as.code.frontier;
  cgCallHelper(m_as, (TCA)VMExecutionContext::fillContinuationVars,
               reg::noreg, false,
               ArgGroup().ssa(inst->getSrc(0))
                         .ssa(inst->getSrc(1))
                         .ssa(inst->getSrc(2))
                         .ssa(inst->getSrc(3)));
  return start;
}

Address CodeGenerator::cgFillContThis(IRInstruction* inst) {
  Address start = m_as.code.frontier;
  SSATmp* cont = inst->getSrc(0);
  register_name_t baseReg = inst->getSrc(1)->getAssignedLoc();
  int64 offset = inst->getSrc(2)->getConstValAsInt();
  register_name_t scratch = LinearScan::rScratch;

  m_as.load_reg64_disp_reg64(cont->getAssignedLoc(), CONTOFF(m_obj), scratch);
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

Address CodeGenerator::cgUnpackCont(IRInstruction* inst) {
  not_reached();
}

Address CodeGenerator::cgExitOnContVars(IRInstruction* inst) {
  not_reached();
}

Address CodeGenerator::cgPackCont(IRInstruction* inst) {
  not_reached();
}

Address CodeGenerator::cgContRaiseCheck(IRInstruction* inst) {
  Address start = m_as.code.frontier;
  SSATmp* cont = inst->getSrc(0);
  m_as.test_imm32_disp_reg32(0x1, CONTOFF(m_should_throw),
                             cont->getAssignedLoc());
  emitFwdJcc(CC_NZ, inst->getLabel());
  return start;
}

Address CodeGenerator::cgContPreNext(IRInstruction* inst) {
  Address start = m_as.code.frontier;
  register_name_t contReg = inst->getSrc(0)->getAssignedLoc();

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
                            inst->getSrc(0)->getAssignedLoc());
  emitFwdJcc(CC_L, inst->getLabel());
  return start;
}

Address CodeGenerator::cgAssertRefCount(IRInstruction* inst) {
  Address start = m_as.code.frontier;
  SSATmp* src = inst->getSrc(0);
  register_name_t srcReg = src->getAssignedLoc();
  emitAssertRefCount(m_as, srcReg);
  return start;
}

Address CodeGenerator::cgLabel(Opcode opc, LabelInstruction* label) {
  return patchLabel(label, m_as.code.frontier);
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

void CodeGenerator::cgTrace(Trace* trace) {
  m_curTrace = trace;
  trace->setFirstAsmAddress(m_as.code.frontier);
  trace->setFirstAstubsAddress(m_astubs.code.frontier);
  if (trace->isMain()) {
    emitTraceCall(m_as, trace->getBcOff());
  }
  IRInstruction::Iterator it;
  IRInstruction::List instructionList = trace->getInstructionList();
  for (it = instructionList.begin();
       it != instructionList.end();
       it++) {
    IRInstruction* inst = *it;
    if (inst->getOpcode() == Marker) {
      m_lastMarker = (LabelInstruction*)inst;
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

void assignRegsForTrace(Trace* trace,
                        IRFactory* irFactory,
                        TraceBuilder* traceBuilder) {
  LinearScan ls(irFactory, traceBuilder);
  ls.allocRegsToTrace(trace);
}

void genCodeForTrace(Trace* trace,
                     CodeGenerator::Asm& as,
                     CodeGenerator::Asm& astubs,
                     IRFactory* irFactory,
                     Transl::TranslatorX64* tx64 /* =NULL */) {
  // select instructions for the trace and its exits
  CodeGenerator cgMain(as, astubs, tx64);
  cgMain.cgTrace(trace);
  CodeGenerator cgExits(astubs, astubs, tx64);
  Trace::List& exitTraces = trace->getExitTraces();
  for (Trace::Iterator it = exitTraces.begin();
       it != exitTraces.end();
       it++) {
    cgExits.cgTrace(*it);
  }
}

void genCodeForTrace(Trace* trace, IRFactory* irFactory) {
  static const size_t aSize = 512 << 20;
  static const size_t astubsSize = 512 << 20;
  uint8_t *base = allocSlab(aSize + astubsSize);
  CodeGenerator::Asm as;
  CodeGenerator::Asm astubs;
  as.init(base, aSize);
  astubs.init(base + aSize, astubsSize);
  genCodeForTrace(trace, as, astubs, irFactory, NULL);
}

}}}
