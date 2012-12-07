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

#include "ir.h"
#include "linearscan.h"
#include "cse.h"
#include "simplifier.h"
#include <string.h>
#include <runtime/base/string_data.h>
#include <runtime/vm/runtime.h>
#include "runtime/vm/translator/targetcache.h"
#include <util/trace.h>

using namespace HPHP::VM::Transl::TargetCache;

namespace HPHP {
namespace VM {
namespace JIT{

const char* OpcodeStrings[] = {
#define OPC(name, hasDst, canCSE, essential, effects, native, consRef,  \
            prodRef, mayModRefs, rematerializable, error)               \
  #name,
  IR_OPCODES
  #undef OPC
};

/*
 * Has Dest indicates the instruction produces an SSATmp
 * which is generally the resulting value, but occassionally
 * a heap reference to help order operations, e.g., AddElem
 */
const int HasDst[] = {
#define OPC(name, hasDst, canCSE, essential, effects, native, consRef,  \
            prodRef, mayModRefs, rematerializable, error)               \
  hasDst,
  IR_OPCODES
  #undef OPC
};
bool IRInstruction::hasDst(Opcode opc) {
  return HasDst[opc];
}

/*
 * Can CSE this instruction, often not true for instructions
 * which have side-effects such as modifying heap memory.
 */
const int CanCSE[] = {
#define OPC(name, hasDst, canCSE, essential, effects, native, consRef, \
            prodRef, mayModRefs, rematerializable, error)              \
  canCSE,
  IR_OPCODES
  #undef OPC
};

bool IRInstruction::canCSE(Opcode opc) {
  // make sure that instructions that are CSE'able can't produce a
  // reference count or consume reference counts.
  ASSERT(!CanCSE[opc] || !producesReference(opc));
  ASSERT(!CanCSE[opc] || !consumesReferences(opc));
  return CanCSE[opc];
}

/*
 * HasMemEffects indicates the instruction has side effects on memory.
 */
const int HasMemEffects[] = {
#define OPC(name, hasDst, canCSE, essential, effects, native, consRef, \
            prodRef, mayModRefs, rematerializable, error)              \
  effects,
  IR_OPCODES
  #undef OPC
};
bool IRInstruction::hasMemEffects(Opcode opc) {
  return HasMemEffects[opc];
}

/*
 * HasMemEffects indicates the instruction has side effects on memory.
 */
const int IsNative[] = {
#define OPC(name, hasDst, canCSE, essential, effects, native, consRef, \
            prodRef, mayModRefs, rematerializable, error)              \
  native,
  IR_OPCODES
  #undef OPC
};
bool IRInstruction::isNative(Opcode opc) {
  return IsNative[opc];
}

/*
 * ConsumesReferences indicates the instruction decrefs its source operands.
 */
const int ConsumesReferences[] = {
#define OPC(name, hasDst, canCSE, essential, effects, native, consRef,  \
            prodRef, mayModRefs, rematerializable, error)               \
  consRef,
  IR_OPCODES
  #undef OPC
};
bool IRInstruction::consumesReferences(Opcode opc) {
  return ConsumesReferences[opc];
}
// Returns whether the instruction decrefs Operand <srcNo>.
bool IRInstruction::consumesReference(int srcNo) const {
  if (!consumesReferences()) {
    return false;
  }
  // Special case StMem, StMemNT, StProp, and StPropNT.
  // These instructions only consume the value operand.
  if ((m_op == StMem || m_op == StMemNT) && srcNo == 0) {
    // StMem[NT] <pointer>, <value>
    return false;
  }
  if ((m_op == StProp || m_op == StPropNT) && (srcNo == 0 || srcNo == 1)) {
    // StProp[NT] <base>, <offset>, <value>
    return false;
  }
  return true;
}

/*
 * ProducesReference indicates the instruction has incref'ed its destination
 */
const int ProducesReference[] = {
#define OPC(name, hasDst, canCSE, essential, effects, native, consRef, \
            prodRef, mayModRefs, rematerializable, error)              \
  prodRef,
  IR_OPCODES
  #undef OPC
};
bool IRInstruction::producesReference(Opcode opc) {
  return ProducesReference[opc];
}

const int MayModifyRefs[] = {
#define OPC(name, hasDst, canCSE, essential, effects, native, consRef, \
            prodRef, mayModRefs, rematerializable, error)              \
  mayModRefs,
  IR_OPCODES
  #undef OPC
};

bool IRInstruction::mayModifyRefs(Opcode opc) {
  return MayModifyRefs[opc];
}

const int Rematerializable[] = {
#define OPC(name, hasDst, canCSE, essential, effects, native, consRef, \
            prodRef, mayModRefs, rematerializable, error)              \
  rematerializable,
  IR_OPCODES
  #undef OPC
};

bool IRInstruction::isRematerializable(Opcode opc) {
  return Rematerializable[opc];
}

Opcode queryNegateTable[] = {
  OpLte,        // OpGt
  OpLt,         // OpGte
  OpGte,        // OpLt
  OpGt,         // OpLte
  OpNeq,        // OpEq
  OpEq,         // OpNeq
  OpNSame,      // OpSame
  OpSame,       // OpNSame
  NInstanceOfD, // InstanceOfD
  InstanceOfD,  // NInstanceOfD
  IsNSet,       // IsSet
  IsNType,      // IsType
  IsSet,        // IsNSet
  IsType        // IsNType
};

Opcode queryCommuteTable[] = {
  OpLt,         // OpGt
  OpLte,        // OpGte
  OpGt,         // OpLt
  OpGte,        // OpLte
  OpEq,         // OpEq
  OpNeq,        // OpNeq
  OpSame,       // OpSame
  OpNSame       // OpNSame
};

const char* Type::Strings[(int)Type::TAG_ENUM_COUNT] = {
    #define IRT(type, name)  name,
    IR_TYPES
    #undef IRT
};

TraceExitType::ExitType getExitType(Opcode opc) {
  ASSERT(opc >= ExitTrace && opc <= ExitGuardFailure);
  return (TraceExitType::ExitType)(opc - ExitTrace);
}

Opcode getExitOpcode(TraceExitType::ExitType type) {
  return (Opcode)(ExitTrace + type);
}

bool isRefCounted(SSATmp* tmp) {
  if (!Type::isRefCounted(tmp->getType())) {
    return false;
  }
  IRInstruction* inst = tmp->getInstruction();
  Opcode opc = inst->getOpcode();
  if (opc == DefConst || opc == LdConst || opc == LdClsCns) {
    return false;
  }
  return true;
}

IRInstruction* IRInstruction::clone(IRFactory* factory) {
  return factory->cloneInstruction(this);
}

IRInstruction* ExtendedInstruction::clone(IRFactory* factory) {
  return factory->cloneInstruction(this);
}

IRInstruction* ConstInstruction::clone(IRFactory* factory) {
  return factory->cloneInstruction(this);
}

IRInstruction* TypeInstruction::clone(IRFactory* factory) {
  return factory->cloneInstruction(this);
}

IRInstruction* LabelInstruction::clone(IRFactory* factory) {
  return factory->cloneInstruction(this);
}

SSATmp* IRInstruction::getSrc(uint32 i) const {
  ASSERT(i < getNumSrcs());
  if (i < NUM_FIXED_SRCS) {
    return m_srcs[i];
  }
  return getExtendedSrc(i - NUM_FIXED_SRCS);
}
void IRInstruction::setSrc(uint32 i, SSATmp* newSrc) {
  ASSERT(i < getNumSrcs());
  if (i < NUM_FIXED_SRCS) {
    m_srcs[i] = newSrc;
    return;
  }
  setExtendedSrc(i - NUM_FIXED_SRCS, newSrc);
}
bool IRInstruction::equals(IRInstruction* inst) const {
  if (m_op != inst->m_op ||
      m_type != inst->m_type ||
      m_numSrcs != inst->m_numSrcs) {
    return false;
  }
  for (uint32 i = 0; i < getNumSrcs(); i++) {
    if (getSrc(i) != inst->getSrc(i)) {
      return false;
    }
  }
  // TODO: check label for ControlFlowInstructions?
  return true;
}
uint32 IRInstruction::hash() {
  return CSEHash::instHash(m_op, m_type, m_srcs[0], m_srcs[1]);
}

SSATmp* IRInstruction::getExtendedSrc(uint32 i) const {
  ASSERT(0);
  return NULL;
}
void IRInstruction::setExtendedSrc(uint32 i, SSATmp* newSrc) {
  ASSERT(0);
}

void IRInstruction::printOpcode(std::ostream& ostream) {
  ostream << OpcodeStrings[m_op];
}

void IRInstruction::printDst(std::ostream& ostream) {
  if (m_dst) {
    m_dst->SSATmp::print(ostream, true);
    ostream << " = ";
  }
}

void IRInstruction::printSrc(std::ostream& ostream, uint32 i) {
  SSATmp* src = getSrc(i);
  if (src != NULL) {
    if (m_id != 0 && !src->isConst() && src->getLastUseId() == m_id) {
      ostream << "~";
    }
    src->print(ostream);
  } else {
    ostream << "!!!NULL @ " << i;
  }
}

void IRInstruction::printSrcs(std::ostream& ostream) {
  bool first = true;
  for (uint32 i = 0; i < m_numSrcs; i++) {
    if (!first) {
      ostream << ", ";
    } else {
      ostream << " ";
      first = false;
    }
    printSrc(ostream, i);
  }
}

void IRInstruction::print(std::ostream& ostream) {
  if (m_id != 0) {
    ostream << m_id << ": ";
  }
  printDst(ostream);
  bool isStMem = m_op == StMem || m_op == StMemNT || m_op == StRaw;
  bool isLdMem = m_op == LdMemNR || m_op == LdRaw;
  if (isStMem || m_op == StLoc || isLdMem) {
    if (isLdMem) {
      ostream << OpcodeStrings[m_op] << " ";
    }
    ostream << "[";
    printSrc(ostream, 0);
    SSATmp* offset = getSrc(1);
    if ((isStMem || isLdMem) &&
        (!offset->isConst() || offset->getConstValAsInt() != 0)) {
      ostream << " + ";
      printSrc(ostream, 1);
    }
    Type::Tag type = isStMem ? getSrc(2)->getType() : m_type;
    ostream << "]:" << Type::Strings[type];
    if (!isLdMem) {
      ASSERT(getNumSrcs() > 1);
      ostream << " = " <<  OpcodeStrings[m_op] << " ";
      printSrc(ostream, isStMem ? 2 : 1);
    }
  } else {
    printOpcode(ostream);
    printSrcs(ostream);
  }
  if (m_label) {
    ostream << ", ";
    m_label->print(ostream);
  }
  if (m_tca) {
    ostream << ", ";
    if (m_tca == kIRDirectJccJmpActive) {
      ostream << "JccJmp_Exit ";
    }
    else
    if (m_tca == kIRDirectJccActive) {
      ostream << "Jcc_Exit ";
    }
    else
    if (m_tca == kIRDirectGuardActive) {
      ostream << "Guard_Exit ";
    }
    else {
      ostream << (void*)m_tca;
    }
  }
}

void IRInstruction::print() {
  print(std::cerr);
  std::cerr << std::endl;
}

void ExtendedInstruction::initExtendedSrcs(IRFactory& irFactory,
                                           uint32 nOpnds,
                                           SSATmp** opnds) {
  uint32 offset = m_numSrcs;
  m_numSrcs += nOpnds;
  if (m_numSrcs > NUM_FIXED_SRCS) {
    m_extendedSrcs =
      new (irFactory.arena()) SSATmp*[m_numSrcs - NUM_FIXED_SRCS];
  }
  for (uint32 i = offset; i < m_numSrcs; i++) {
    setSrc(i, opnds[i - offset]);
  }
}

void ExtendedInstruction::initExtendedSrcs(IRFactory& irFactory,
                                           SSATmp* src,
                                           uint32 nOpnds,
                                           SSATmp** opnds) {
  uint32 offset = m_numSrcs;
  m_numSrcs += nOpnds + 1;
  if (m_numSrcs > NUM_FIXED_SRCS) {
    m_extendedSrcs =
      new (irFactory.arena()) SSATmp*[m_numSrcs - NUM_FIXED_SRCS];
  }
  setSrc(offset, src);
  for (uint32 i = offset + 1; i < m_numSrcs; i++) {
    setSrc(i, opnds[i - (offset + 1)]);
  }
}

SSATmp* ExtendedInstruction::getExtendedSrc(uint32 i) const {
  return m_extendedSrcs[i];
}

void ExtendedInstruction::setExtendedSrc(uint32 i, SSATmp* newSrc) {
  m_extendedSrcs[i] = newSrc;
}

void ExtendedInstruction::appendExtendedSrc(IRFactory& irFactory,
                                            SSATmp* src) {
  // create larger array and add input
  int i = 0;
  SSATmp** extendedSrcs = m_extendedSrcs;
  m_extendedSrcs =
    new (irFactory.arena()) SSATmp*[m_numSrcs + 1 - NUM_FIXED_SRCS];
  for (i = 0; i < (int)m_numSrcs - (int)NUM_FIXED_SRCS; i++) {
    m_extendedSrcs[i] = extendedSrcs[i];
  }
  m_extendedSrcs[i] = src;
  m_numSrcs++;
}

void ConstInstruction::printConst(std::ostream& ostream) {
  switch (m_type) {
    case Type::Int:
      ostream << m_intVal;
      break;
    case Type::Dbl:
      ostream << m_dblVal;
      break;
    case Type::Bool:
      ostream << (m_boolVal ? "true" : "false");
      break;
    case Type::Str:
    case Type::StaticStr:
      ostream << "\"" << m_strVal->data() << "\"";
      break;
    case Type::Arr:
    {
      if (isEmptyArray()) {
        ostream << "array()";
      } else {
        ostream << "Array(" << (void*)m_arrVal << ")";
      }
      break;
    }
    case Type::Home:
      m_local->print(ostream);
      break;
    case Type::Null:
      ostream << "Null";
      break;
    case Type::Uninit:
      ostream << "Unin";
      break;
    case Type::FuncRef:
      ostream << "Func(" << (m_func ? m_func->fullName()->data() : "0") << ")";
      break;
    case Type::ClassRef:
      ostream << "Class(" << (m_clss ? m_clss->name()->data() : "0") << ")";
      break;
    case Type::FuncClassRef:
      ASSERT(false /* ConstInstruction does not hold both func* and class* */);
      break;
    case Type::None:
      ostream << "None:" << m_intVal;
      break;
    default:
      not_reached();
  }
}

bool TypeInstruction::equals(IRInstruction* inst) const {
  if (!this->IRInstruction::equals(inst)) {
    return false;
  }
  return m_srcType == ((TypeInstruction*)inst)->m_srcType;
}

uint32 TypeInstruction::hash() {
  return
    CSEHash::instHash(m_op, m_type, m_srcs[0], m_srcs[1], (int64)m_srcType);
}

bool ConstInstruction::equals(IRInstruction* inst) const {
  if (!this->IRInstruction::equals(inst)) {
    return false;
  }
  return m_intVal == ((ConstInstruction*)inst)->m_intVal;
}
uint32 ConstInstruction::hash() {
  if (m_type == Type::Str) {
    return CSEHash::instHash(m_op, m_type, m_srcs[0], m_srcs[1],
                             (void*)m_strVal);
  } else if (m_type == Type::Home) {
    return CSEHash::instHash(m_op, m_type, m_srcs[0], m_srcs[1],
                             (void*)m_local);
  } else if (m_type == Type::FuncRef) {
    return CSEHash::instHash(m_op, m_type, m_srcs[0], m_srcs[1],
                             (void*)m_func);
  } else if (m_type == Type::ClassRef) {
    return CSEHash::instHash(m_op, m_type, m_srcs[0], m_srcs[1],
                             (void*)m_clss);
  } else if (m_type == Type::FuncClassRef) {
    ASSERT(false /* ConstInstruction does not hold both func* and class* */);
    return CSEHash::instHash(m_op, m_type, m_srcs[0], m_srcs[1], (void*)m_func);
  }
  return CSEHash::instHash(m_op, m_type, m_srcs[0], m_srcs[1], m_intVal);
}

void ConstInstruction::print(std::ostream& ostream) {
  this->IRInstruction::print(ostream);
  ostream << " ";
  printConst(ostream);
}

bool LabelInstruction::equals(IRInstruction* inst) const {
  ASSERT(0);
  return false;
}

uint32 LabelInstruction::hash() {
  ASSERT(0);
  return 0;
}

// Thread chain of patch locations using the 4 byte space in each jmp/jcc
void LabelInstruction::prependPatchAddr(TCA patchAddr) {
  ssize_t diff = getPatchAddr() ? ((TCA)patchAddr - (TCA)getPatchAddr()) : 0;
  ASSERT(deltaFits(diff, sz::dword));
  *(int*)(patchAddr) = (int)diff;
  m_patchAddr = patchAddr;
}

void* LabelInstruction::getPatchAddr() {
  return m_patchAddr;
}

void LabelInstruction::print(std::ostream& ostream) {
  if (m_op == DefLabel) {
    ostream << "L";
  } else if (m_op == Marker) {
    ostream << "--- bc";
  }
  ostream << m_labelId << ":";
  if (m_op == Marker) {
    ostream << ", spOff: " << m_stackOff;
  }
}

void TypeInstruction::print(std::ostream& ostream) {
  printDst(ostream);
  printOpcode(ostream);
  ostream << Type::Strings[getSrcType()];
  printSrcs(ostream);
}

bool SSATmp::isAssignedReg(uint32 index) const {
  return LinearScan::regNameAsInt(m_regs[index]) < LinearScan::NumRegs;
}

bool SSATmp::isAssignedMmxReg(uint32 index) const {
  int loc = LinearScan::regNameAsInt(m_regs[index]);
  return loc >= LinearScan::FirstMmxReg &&
         loc < LinearScan::FirstMmxReg + LinearScan::NumMmxRegs;
}

bool SSATmp::isAssignedSpillLoc(uint32 index) const {
  return LinearScan::regNameAsInt(m_regs[index]) >= LinearScan::FirstSpill;
}

uint32 SSATmp::getNumRegs() const {
  uint32 i;
  for (i = 0; i < kMaxNumRegs && m_regs[i] != reg::noreg; ++i);
  return i;
}

uint32 SSATmp::getSpillLoc(uint32 index) const {
  ASSERT(isAssignedSpillLoc(index));
  return LinearScan::regNameAsInt(m_regs[index]) - LinearScan::FirstSpill;
}

void SSATmp::setSpillLoc(uint32 spillLoc, uint32 index) {
  m_regs[index] = (RegNumber)(spillLoc + LinearScan::FirstSpill);
}

RegNumber SSATmp::getMmxReg(uint32 index) const {
  ASSERT(isAssignedMmxReg(index));
  return (RegNumber)(LinearScan::regNameAsInt(m_regs[index]) -
                           LinearScan::FirstMmxReg);
}

void SSATmp::setMmxReg(RegNumber mmxReg, uint32 index) {
  ASSERT(LinearScan::regNameAsInt(mmxReg) < (int32)LinearScan::NumMmxRegs);
  m_regs[index] = (RegNumber)(LinearScan::regNameAsInt(mmxReg) +
                                           LinearScan::FirstMmxReg);
}

bool SSATmp::getConstValAsBool()   {
  ASSERT(isConst());
  return ((ConstInstruction*)m_inst)->getValAsBool();
}
int64 SSATmp::getConstValAsInt() {
  ASSERT(isConst());
  return ((ConstInstruction*)m_inst)->getValAsInt();
}
int64 SSATmp::getConstValAsRawInt() {
  ASSERT(isConst());
  return ((ConstInstruction*)m_inst)->getValAsRawInt();
}
double SSATmp::getConstValAsDbl() {
  ASSERT(isConst());
  return ((ConstInstruction*)m_inst)->getValAsDbl();
}
const StringData* SSATmp::getConstValAsStr() {
  ASSERT(isConst());
  return ((ConstInstruction*)m_inst)->getValAsStr();
}
const ArrayData* SSATmp::getConstValAsArr() {
  ASSERT(isConst());
  return ((ConstInstruction*)m_inst)->getValAsArr();
}
const Func* SSATmp::getConstValAsFunc() {
  ASSERT(isConst());
  return ((ConstInstruction*)m_inst)->getValAsFunc();
}
const Class* SSATmp::getConstValAsClass() {
  ASSERT(isConst());
  return ((ConstInstruction*)m_inst)->getValAsClass();
}
const Local* SSATmp::getConstValAsLocal() {
  ASSERT(isConst());
  return ((ConstInstruction*)m_inst)->getLocal();
}
uintptr_t SSATmp::getConstValAsBits() {
  ASSERT(isConst());
  return ((ConstInstruction*)m_inst)->getValAsBits();
}

void SSATmp::setTCA(TCA tca) {
  getInstruction()->setTCA(tca);
}
TCA SSATmp::getTCA() {
  return getInstruction()->getTCA();
}

void SSATmp::print(std::ostream& ostream, bool printLastUse) {
  if (m_inst->isDefConst()) {
    ((ConstInstruction*)m_inst)->printConst(ostream);
    return;
  }
  ostream << "t" << m_id;
  if (printLastUse && m_lastUseId != 0) {
    ostream << "@" << m_lastUseId << "#" << m_useCount;
  }
  if (m_regs[0] != reg::noreg) {
    ostream << "(";
    LinearScan::printLoc(ostream, LinearScan::regNameAsInt(m_regs[0]));
    if (m_regs[1] != reg::noreg) {
      ostream << ", ";
      LinearScan::printLoc(ostream, LinearScan::regNameAsInt(m_regs[1]));
    }
    ostream << ")";
  }
  ostream << ":" << Type::Strings[m_inst->getType()];
}

void SSATmp::print() {
  print(std::cerr);
  std::cerr << std::endl;
}

IRInstruction* IRFactory::cloneInstruction(IRInstruction* inst) {
  return new (m_arena) IRInstruction(inst);
}

ExtendedInstruction* IRFactory::cloneInstruction(ExtendedInstruction* inst) {
  return new (m_arena) ExtendedInstruction(*this, inst);
}

ConstInstruction* IRFactory::cloneInstruction(ConstInstruction* inst) {
  return new (m_arena) ConstInstruction(inst);
}

TypeInstruction* IRFactory::cloneInstruction(TypeInstruction* inst) {
  return new (m_arena) TypeInstruction(inst);
}

LabelInstruction* IRFactory::cloneInstruction(LabelInstruction* inst) {
  return new (m_arena) LabelInstruction(inst);
}

IRInstruction* IRFactory::guardRefs(SSATmp*           funcPtr,
                                    SSATmp*           nParams,
                                    SSATmp*           bitsPtr,
                                    SSATmp*           firstBitNum,
                                    SSATmp*           mask64,
                                    SSATmp*           vals64,
                                    LabelInstruction* exitLabel) {

  SSATmp* args[3] = {firstBitNum, mask64, vals64};

  return new (m_arena) ExtendedInstruction(*this, GuardRefs, Type::None,
                                           funcPtr, nParams, bitsPtr,
                                           (sizeof(args) / sizeof(SSATmp*)),
                                           args, exitLabel);
}

IRInstruction* IRFactory::ldLoc(SSATmp* home) {
  ASSERT(home->getType() == Type::Home);
  return new (m_arena) IRInstruction(LdLoc, Type::Cell, home);
}

IRInstruction* IRFactory::ldLoc(SSATmp*           home,
                                Type::Tag         type,
                                LabelInstruction* typeFailLabel) {
  ASSERT(home->getType() == Type::Home);
  return new (m_arena) IRInstruction(LdLoc, type, home, typeFailLabel);
}

ConstInstruction* IRFactory::defConst(int64 val) {
  return new (m_arena) ConstInstruction(DefConst, val);
}

LabelInstruction* IRFactory::defLabel() {
  return new (m_arena) LabelInstruction(m_nextLabelId++);
}

LabelInstruction* IRFactory::marker(uint32 bcOff, const Func* f, int32 spOff) {
  return new (m_arena) LabelInstruction(Marker, bcOff, f, spOff);
}

IRInstruction* IRFactory::decRefLoc(SSATmp* home, LabelInstruction* exit) {
  return new (m_arena) IRInstruction(DecRefLoc, Type::None, home, exit);
}

IRInstruction* IRFactory::decRefStack(Type::Tag type,
                                      SSATmp* sp,
                                      SSATmp* index,
                                      LabelInstruction* exit) {
  return new (m_arena) IRInstruction(DecRefStack, type, sp, index, exit);
}

IRInstruction* IRFactory::decRefThis(SSATmp* fp, LabelInstruction* exit) {
  return new (m_arena) IRInstruction(DecRefThis, Type::None, fp, exit);
}

IRInstruction* IRFactory::decRefLocalsThis(SSATmp* fp, SSATmp* numLocals) {
  return new (m_arena) IRInstruction(DecRefLocalsThis, Type::None, fp,
                                     numLocals);
}

IRInstruction* IRFactory::decRefLocals(SSATmp* fp, SSATmp* numLocals) {
  return new (m_arena) IRInstruction(DecRefLocals, Type::None, fp, numLocals);
}

IRInstruction* IRFactory::decRef(SSATmp* tmp, LabelInstruction* exit) {
  return new (m_arena) IRInstruction(DecRef, tmp->getType(), tmp, exit);
}

IRInstruction* IRFactory::incRef(SSATmp* obj) {
  return new (m_arena) IRInstruction(IncRef, obj->getType(), obj);
}

IRInstruction* IRFactory::allocActRec(SSATmp* stackPtr,
                                      SSATmp* framePtr,
                                      SSATmp* func,
                                      SSATmp* objOrCls,
                                      SSATmp* numArgs,
                                      SSATmp* magicName) {
  SSATmp* args[4] = { func, objOrCls, numArgs, magicName };
  return new (m_arena) ExtendedInstruction(*this, AllocActRec,
                                           Type::SP, stackPtr, framePtr,
                                           (sizeof(args) / sizeof(SSATmp*)),
                                           args);

}

IRInstruction* IRFactory::freeActRec(SSATmp* framePtr) {
  return new (m_arena) IRInstruction(FreeActRec, Type::SP, framePtr);
}

IRInstruction* IRFactory::call(SSATmp* actRec,
                               SSATmp* returnBcOffset,
                               SSATmp* func,
                               uint32 numArgs,
                               SSATmp** args) {
  return new (m_arena) ExtendedInstruction(*this, Call, Type::SP, actRec,
                                           returnBcOffset, func,
                                           numArgs, args);
}

IRInstruction* IRFactory::spillStack(SSATmp* sp,
                                     SSATmp* stackAdjustment,
                                     uint32 numTmps,
                                     SSATmp** tmps,
                                     bool allocActRec) {
  Opcode opc = allocActRec ? SpillStackAllocAR : SpillStack;
  return new (m_arena) ExtendedInstruction(*this,
                                           opc,
                                           Type::SP,
                                           sp,
                                           stackAdjustment,
                                           numTmps,
                                           tmps);
}

IRInstruction* IRFactory::exitTrace(TraceExitType::ExitType exitType,
                                    SSATmp* func,
                                    SSATmp* pc,
                                    SSATmp* sp,
                                    SSATmp* fp) {
  SSATmp* args[2] = { sp, fp };
  return new (m_arena) ExtendedInstruction(*this,
                                           getExitOpcode(exitType),
                                           Type::None,
                                           func,
                                           pc,
                                           (sizeof(args) / sizeof(SSATmp*)),
                                           args);
}

IRInstruction* IRFactory::exitTrace(TraceExitType::ExitType exitType,
                                    SSATmp* func,
                                    SSATmp* pc,
                                    SSATmp* sp,
                                    SSATmp* fp,
                                    SSATmp* notTakenPC) {
  SSATmp* args[3] = { sp, fp, notTakenPC };
  return new (m_arena) ExtendedInstruction(*this,
                                           getExitOpcode(exitType),
                                           Type::None,
                                           func,
                                           pc,
                                           (sizeof(args) / sizeof(SSATmp*)),
                                           args);
}

IRInstruction* IRFactory::retVal(SSATmp* fp, SSATmp* val) {
  return new (m_arena) IRInstruction(RetVal, Type::SP, fp, val);
}

IRInstruction* IRFactory::retVal(SSATmp* fp) {
  return new (m_arena) IRInstruction(RetVal, Type::SP, fp);
}

IRInstruction* IRFactory::retCtrl(SSATmp* sp,
                                  SSATmp* fp,
                                  SSATmp* retAddr) {
  return new (m_arena) ExtendedInstruction(*this, RetCtrl, Type::None,
                                           sp, fp, 1, &retAddr);
}

IRInstruction* IRFactory::spill(SSATmp* src) {
  return new (m_arena) IRInstruction(Spill, src->getType(), src);
}

IRInstruction* IRFactory::reload(SSATmp* slot) {
  return new (m_arena) IRInstruction(Reload, slot->getType(), slot);
}

IRInstruction* IRFactory::allocSpill(SSATmp* numSlots) {
  return new (m_arena) IRInstruction(AllocSpill, Type::None, numSlots);
}

IRInstruction* IRFactory::freeSpill(SSATmp* numSlots) {
  return new (m_arena) IRInstruction(FreeSpill, Type::None, numSlots);
}

#ifdef DEBUG

extern "C" {
#include "../tools/xed2-intel64/include/xed-interface.h"
}

static void error(std::string msg) {
  fprintf(stderr, "Error: %s\n", msg.c_str());
  exit(1);
}


#define MAX_INSTR_ASM_LEN 128
xed_state_t xed_state;

static const xed_syntax_enum_t s_xed_syntax =
  getenv("HHVM_ATT_DISAS") ? XED_SYNTAX_ATT : XED_SYNTAX_INTEL;

void printInstructions(xed_uint8_t* codeStartAddr,
                       xed_uint8_t* codeEndAddr,
                       bool printAddr) {
  char codeStr[MAX_INSTR_ASM_LEN];
  xed_uint8_t *frontier;
  xed_decoded_inst_t xedd;
  uint64 ip;

  // Decode and print each instruction
  for (frontier = codeStartAddr, ip = (uint64)codeStartAddr;
       frontier < codeEndAddr;
      ) {
    xed_decoded_inst_zero_set_mode(&xedd, &xed_state);
    xed_decoded_inst_set_input_chip(&xedd, XED_CHIP_INVALID);
    xed_error_enum_t xed_error = xed_decode(&xedd, frontier, 15);
    if (xed_error != XED_ERROR_NONE) error("disasm error: xed_decode failed");

    // Get disassembled instruction in codeStr
    if (!xed_format_context(s_xed_syntax, &xedd, codeStr,
                            MAX_INSTR_ASM_LEN, ip, NULL)) {
      error("disasm error: xed_format_context failed");
    }

    if (printAddr) printf("0x%08llx: ", ip);
    uint32 instrLen = xed_decoded_inst_get_length(&xedd);
    if (false) { // print encoding, like in objdump
      unsigned posi = 0;
      for (; posi < instrLen; ++posi) {
        printf("%02x ", (uint8_t)frontier[posi]);
      }
      for (; posi < 16; ++posi) {
        printf("   ");
      }
    }
    printf("%s\n", codeStr);
    frontier += instrLen;
    ip       += instrLen;


  }
}
#endif

void Trace::print(std::ostream& ostream, bool printAsm,
                  bool isExit /* = false */) {
#ifdef DEBUG
  xed_state_init(&xed_state, XED_MACHINE_MODE_LONG_64,
                 XED_ADDRESS_WIDTH_64b, XED_ADDRESS_WIDTH_64b);
  xed_tables_init();
#endif

  IRInstruction::Iterator it;
  for (it = m_instructionList.begin();
       it != m_instructionList.end();
       ) {
    IRInstruction* inst = *it;
    it++;
    if (inst->getOpcode() == Marker) {
      inst->print(std::cout);
      std::cout << std::endl;
      if (isExit) continue; // don't print bytecode
      LabelInstruction* markerInst = (LabelInstruction*)inst;
      uint32 bcOffset = markerInst->getLabelId();
      const Func* func = markerInst->getFunc();
      if (func != NULL) {
        Unit* unit = func->unit();
        unit->prettyPrint(std::cout, bcOffset, bcOffset+1);
        continue;
      }
    }
    inst->print(std::cout);
    std::cout << std::endl;
    if (!printAsm) {
      continue;
    }
    uint8* asmAddr = (uint8*)inst->getAsmAddr();
    if (asmAddr == NULL) {
      continue;
    }
    // Find the next instruction that has an non-NULL asm address.
    IRInstruction::Iterator nextHasAsmAddr = it;
    while (nextHasAsmAddr != m_instructionList.end() &&
           (*nextHasAsmAddr)->getAsmAddr() == NULL) {
      ++nextHasAsmAddr;
    }
    uint8* endAsm;
    if (nextHasAsmAddr != m_instructionList.end()) {
      endAsm = (uint8*)(*nextHasAsmAddr)->getAsmAddr();
    } else {
      endAsm = m_lastAsmAddress;
    }
    if (asmAddr != endAsm) {
      // print out the assembly
      std::cout << std::endl;
#ifdef DEBUG
      printInstructions(asmAddr, endAsm, true);
      std::cout << std::endl;
#endif
    }
  }

  bool firstExitTracePrinted = false;
  for (List::iterator it = m_exitTraces.begin();
       it != m_exitTraces.end();
       it++) {
    Trace* exitTrace = *it;
    if (!firstExitTracePrinted) {
      firstExitTracePrinted = true;
      // print out any extra code in astubs
      if (m_firstAstubsAddress < exitTrace->m_firstAsmAddress) {
#ifdef DEBUG
        std::cout << "AStubs: " << std::endl;
        printInstructions(m_firstAstubsAddress,
                          exitTrace->m_firstAsmAddress,
                          true);
#endif
        std::cout << std::endl;
      }

    }
    std::cout << "\n-------  Exit Trace  -------\n";
    exitTrace->print(std::cout, printAsm, true);
  }
}

void Trace::print() {
  print(std::cout, true /* printAsm */);
}

void resetIdsAux(Trace* trace) {
  IRInstruction::Iterator it;
  IRInstruction::List instructionList = trace->getInstructionList();
  for (it = instructionList.begin();
       it != instructionList.end();
       it++) {
    IRInstruction* inst = *it;
    inst->setId(0);
    SSATmp* dst = inst->getDst();
    if (dst) {
      dst->setLastUseId(0);
      dst->setUseCount(0);
      dst->setSpillSlot(-1);
    }
  }
}

/*
 * Clears the IRInstructions' ids, and the SSATmps' use count and last use id
 * for the given trace and all its exit traces.
 */
void resetIds(Trace* trace) {
  resetIdsAux(trace);
  Trace::List& exitTraces = trace->getExitTraces();
  for (Trace::Iterator it = exitTraces.begin();
       it != exitTraces.end();
       it++) {
    resetIdsAux(*it);
  }
}

uint32 numberInstructions(Trace* trace,
                          uint32 nextId,
                          bool followControlFlow) {
  for (auto* inst : trace->getInstructionList()) {
    if (SSATmp* dst = inst->getDst()) {
      // Initialize this value for register spilling.
      dst->setSpillSlot(-1);
    }
    if (inst->getOpcode() == Marker) {
      continue; // don't number markers
    }
    uint32 id = nextId++;
    inst->setId(id);
    for (uint32 i=0; i < inst->getNumSrcs(); i++) {
      SSATmp* tmp = inst->getSrc(i);
      tmp->setLastUseId(id);
      tmp->incUseCount();
    }
    // This eagerly follows control flow edges to exit traces, so
    // if no more than 1 instruction should branch to a exit trace's label
    // otherwise that exit trace will be incorrectly processed more than once.
    // The net result is that the exit trace's instruction ids will be greater
    // than the ids of the instruction that can branch to it, which
    // is exactly the invariant we want for linear scan register allocation
    // and other analyses.
    if (followControlFlow && inst->isControlFlowInstruction()) {
      LabelInstruction* label = inst->getLabel();
      if (label) {
        nextId = numberInstructions(label->getTrace(), nextId);
      }
    }
  }
  return nextId;
}

/*
 * Returns true if a label is unreachable -- that is, if a label's id is 0
 * because numbering never visited it.
 */
static bool labelIsUnreachable(const Trace* trace) {
  return trace->getLabel()->getId() == 0;
}

/*
 * Assigns ids to instructions, sets the use count and last use id for the
 * SSA tmps, and removes unreachable exit traces. This sets us up for
 * linear scan register allocation.
 */
void numberInstructions(Trace* trace) {
  resetIds(trace);
  numberInstructions(trace, 1, true);
  // any exit trace with a label whose id is 0 is unreachable and
  // can be removed
  trace->getExitTraces().remove_if(labelIsUnreachable);
}

}}}

