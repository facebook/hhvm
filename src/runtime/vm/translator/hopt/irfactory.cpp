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
#include "runtime/vm/translator/hopt/irfactory.h"

namespace HPHP { namespace VM { namespace JIT {

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

}}}
