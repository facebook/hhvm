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

#ifndef incl_HPHP_VM_IRFACTORY_H_
#define incl_HPHP_VM_IRFACTORY_H_

#include "util/arena.h"
#include "runtime/vm/translator/hopt/ir.h"

namespace HPHP { namespace VM { namespace JIT {

class IRFactory {
public:
  IRFactory()
    : m_nextLabelId(0)
    , m_nextOpndId(0)
  {}

  IRInstruction* cloneInstruction(IRInstruction* inst);
  ExtendedInstruction* cloneInstruction(ExtendedInstruction* inst);
  ConstInstruction* cloneInstruction(ConstInstruction* inst);
  TypeInstruction* cloneInstruction(TypeInstruction* inst);
  LabelInstruction* cloneInstruction(LabelInstruction* inst);

  IRInstruction* guardRefs(SSATmp* funcPtr,
                           SSATmp* nParams,
                           SSATmp* bitsPtr,
                           SSATmp* firstBitNum,
                           SSATmp* mask64,
                           SSATmp* vals64,
                           LabelInstruction* exitLabel = NULL);

  IRInstruction* ldLoc(SSATmp* home);
  IRInstruction* ldLoc(SSATmp* home,
                       Type::Tag type,
                       LabelInstruction*);
  ConstInstruction* defConst(int64 val);
  IRInstruction* verifyParamType(SSATmp* src,
                                 SSATmp* tc, SSATmp* constraint,
                                 LabelInstruction*);
  IRInstruction* spillStack(SSATmp* sp,
                            SSATmp* stackAdjustment,
                            uint32 numOpnds,
                            SSATmp** opnds,
                            bool allocActRec = false);
  IRInstruction* exitTrace(TraceExitType::ExitType,
                           SSATmp* func,
                           SSATmp* pc,
                           SSATmp* sp,
                           SSATmp* fp);
  IRInstruction* exitTrace(TraceExitType::ExitType,
                           SSATmp* func,
                           SSATmp* pc,
                           SSATmp* sp,
                           SSATmp* fp,
                           SSATmp* notTakenPC);
  IRInstruction* allocActRec(SSATmp* stkPtr,
                             SSATmp* framePtr,
                             SSATmp* func,
                             SSATmp* objOrCls,
                             SSATmp* numArgs,
                             SSATmp* magicName);
  IRInstruction* freeActRec(SSATmp* framePtr);
  IRInstruction* call(SSATmp* actRec,
                      SSATmp* returnBcOffset,
                      SSATmp* func,
                      uint32 numArgs,
                      SSATmp** args);
  IRInstruction* incRef(SSATmp* obj);
  IRInstruction* decRef(SSATmp* obj, LabelInstruction* dtorLabel);
  LabelInstruction* defLabel();
  LabelInstruction* marker(uint32 bcOff, const Func* func, int32 spOff);
  IRInstruction* decRefLoc(SSATmp* home, LabelInstruction* exit);
  IRInstruction* decRefStack(Type::Tag type,
                             SSATmp* sp,
                             SSATmp* index,
                             LabelInstruction* exit);
  IRInstruction* decRefThis(SSATmp* fp, LabelInstruction* exit);

  template<Type::Tag T> TypeInstruction* isType(SSATmp* src) {
    return new TypeInstruction(IsType, T, Type::Bool, src);
  }
  IRInstruction* spill(SSATmp* src);
  IRInstruction* reload(SSATmp* slot);
  IRInstruction* allocSpill(SSATmp* numSlots);
  IRInstruction* freeSpill(SSATmp* numSlots);

  SSATmp* getSSATmp(IRInstruction* inst) {
    SSATmp* tmp = new (m_arena) SSATmp(m_nextOpndId++, inst);
    inst->setDst(tmp);
    return tmp;
  }

  uint32 getNumSSATmps() { return m_nextOpndId; }
  Arena& arena() { return m_arena; }

private:
  uint32 m_nextLabelId;
  uint32 m_nextOpndId;

  // SSATmp and IRInstruction objects are allocated here.
  Arena m_arena;
};

}}}

#endif
