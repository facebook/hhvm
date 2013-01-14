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

#ifndef incl_HHVM_HHIR_SIMPLIFIER_H_
#define incl_HHVM_HHIR_SIMPLIFIER_H_

#include "ir.h"
#include "cse.h"

namespace HPHP {
namespace VM {
namespace JIT {

class TraceBuilder;

struct Simplifier {
  explicit Simplifier(TraceBuilder* t) : m_tb(t) {}

  static void copyProp(IRInstruction* tmp);

  SSATmp* simplify(IRInstruction*);

private:
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
  SSATmp* simplifyInstanceOfD(SSATmp* src1, SSATmp* src2, bool negate);
  SSATmp* simplifyUnbox(Type::Tag, SSATmp* src);
  SSATmp* simplifyUnbox(Type::Tag, SSATmp* src, LabelInstruction* failLabel);
  SSATmp* simplifyLdClsPropAddr(SSATmp* cls, SSATmp* clsName, SSATmp* propName);
  SSATmp* simplifyAllocActRec(SSATmp* src1, SSATmp* src2, SSATmp* src3,
                              SSATmp* src4, SSATmp* src5);

private:
  SSATmp* genDefInt(int64 val);
  SSATmp* genDefBool(bool val);
  SSATmp* genLdClsPropAddr(SSATmp* cls, SSATmp* clsName, SSATmp* propName);
  SSATmp* simplifyCmp(Opcode opName, SSATmp* src1, SSATmp* src2);

private:
  TraceBuilder* const m_tb;
};

}}}

#endif
