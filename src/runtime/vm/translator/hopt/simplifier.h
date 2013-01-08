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

#ifndef _SIMPLIFIER_H_
#define _SIMPLIFIER_H_

#include "ir.h"
#include "cse.h"

namespace HPHP {
namespace VM {
namespace JIT {

class TraceBuilder;

class Simplifier {
public:
  Simplifier(TraceBuilder* t);
  SSATmp* simplifyInst(IRInstruction* inst);
  SSATmp* simplifyInst(Opcode opc, Type::Tag type,
                       SSATmp* src1, SSATmp* src2);
  SSATmp* simplifyInst(Opcode opc, Type::Tag type,
                       SSATmp* src1, SSATmp* src2,
                       uint32 numExtendedSrcs, SSATmp** extendedSrcs);
  SSATmp* simplifyInst(Opcode opc, Type::Tag type,
                       SSATmp* src, Type::Tag srcType);
  SSATmp* simplifyInst(Opcode opc, Type::Tag type,
                       SSATmp* src1, SSATmp* src2,
                       LabelInstruction *label);
  static void copyProp(IRInstruction* tmp);

  SSATmp* simplifyMov(SSATmp* src);
  SSATmp* simplifyNot(SSATmp* src);
  SSATmp* simplifyAdd(SSATmp* src1, SSATmp* src2);
  SSATmp* simplifySub(SSATmp* src1, SSATmp* src2);
  SSATmp* simplifyMul(SSATmp* src1, SSATmp* src2);
  SSATmp* simplifyAnd(SSATmp* src1, SSATmp* src2);
  SSATmp* simplifyOr(SSATmp* src1, SSATmp* src2);
  SSATmp* simplifyXor(SSATmp* src1, SSATmp* src2);
  SSATmp* simplifyGt(SSATmp* src1, SSATmp* src2);
  SSATmp* simplifyGte(SSATmp* src1, SSATmp* src2);
  SSATmp* simplifyLt(SSATmp* src1, SSATmp* src2);
  SSATmp* simplifyLte(SSATmp* src1, SSATmp* src2);
  SSATmp* simplifyEq(SSATmp* src1, SSATmp* src2);
  SSATmp* simplifyNeq(SSATmp* src1, SSATmp* src2);
  SSATmp* simplifySame(SSATmp* src1, SSATmp* src2);
  SSATmp* simplifyNSame(SSATmp* src1, SSATmp* src2);
  SSATmp* simplifyIsType(Type::Tag type, SSATmp* src);
  SSATmp* simplifyConcat(SSATmp* src1, SSATmp* src2);
  SSATmp* simplifyConv(Type::Tag toType, SSATmp* src);
  SSATmp* simplifyLdObjClass(SSATmp* src);
  SSATmp* simplifyLdCachedClass(SSATmp* src);
  SSATmp* simplifyInstanceOfD(SSATmp* src1, SSATmp* src2, bool negate);
  SSATmp* simplifyIsSet(SSATmp* src, bool negate);
  SSATmp* simplifyUnbox(Type::Tag, SSATmp* src);
  SSATmp* simplifyUnbox(Type::Tag, SSATmp* src, LabelInstruction* failLabel);
  SSATmp* simplifyJcc(Opcode opc, Type::Tag type,
                      SSATmp* src1, SSATmp* src2,
                      LabelInstruction* label);
  SSATmp* simplifyJmpZ(SSATmp* src, LabelInstruction* label);
  SSATmp* simplifyJmpNz(SSATmp* src, LabelInstruction* label);
  SSATmp* simplifyLdClsPropAddr(SSATmp* cls, SSATmp* clsName, SSATmp* propName);
  SSATmp* simplifyAllocActRec(SSATmp* src1, SSATmp* src2, SSATmp* src3,
                              SSATmp* src4, SSATmp* src5);
private:
  SSATmp* genDefInt(int64 val);
  SSATmp* genDefBool(bool val);
  SSATmp* genLdClsPropAddr(SSATmp* cls, SSATmp* clsName, SSATmp* propName);
  SSATmp* simplifyCmp(Opcode opName, SSATmp* src1, SSATmp* src2);
  TraceBuilder* m_tb;

};

}}} // namespace HPHP::VM::JIT

#endif // _SIMPLIFIER_H_
