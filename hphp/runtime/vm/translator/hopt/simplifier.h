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

#ifndef incl_HPHP_HHVM_HHIR_SIMPLIFIER_H_
#define incl_HPHP_HHVM_HHIR_SIMPLIFIER_H_

#include "runtime/vm/translator/hopt/cse.h"
#include "runtime/vm/translator/hopt/ir.h"

namespace HPHP {
namespace VM {
namespace JIT {

class TraceBuilder;

struct Simplifier {
  explicit Simplifier(TraceBuilder* t) : m_tb(t) {}

  /*
   * Simplify performs a number of optimizations.
   *
   * In many cases this may involve returning a SSATmp* that should be
   * used instead of the candidate instruction passed to this
   * function.  If this function returns nullptr, the candidate
   * instruction should still be used (but the call to simplify may
   * have changed it, also).
   */
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
  SSATmp* simplifyIsType(IRInstruction*);
  SSATmp* simplifyJmpIsType(IRInstruction*);
  SSATmp* simplifyConcat(SSATmp* src1, SSATmp* src2);
  SSATmp* simplifyConvToArr(IRInstruction*);
  SSATmp* simplifyConvArrToBool(IRInstruction*);
  SSATmp* simplifyConvDblToBool(IRInstruction*);
  SSATmp* simplifyConvIntToBool(IRInstruction*);
  SSATmp* simplifyConvStrToBool(IRInstruction*);
  SSATmp* simplifyConvToDbl(IRInstruction*);
  SSATmp* simplifyConvArrToDbl(IRInstruction*);
  SSATmp* simplifyConvBoolToDbl(IRInstruction*);
  SSATmp* simplifyConvIntToDbl(IRInstruction*);
  SSATmp* simplifyConvStrToDbl(IRInstruction*);
  SSATmp* simplifyConvArrToInt(IRInstruction*);
  SSATmp* simplifyConvBoolToInt(IRInstruction*);
  SSATmp* simplifyConvDblToInt(IRInstruction*);
  SSATmp* simplifyConvObjToInt(IRInstruction*);
  SSATmp* simplifyConvStrToInt(IRInstruction*);
  SSATmp* simplifyConvBoolToStr(IRInstruction*);
  SSATmp* simplifyConvDblToStr(IRInstruction*);
  SSATmp* simplifyConvIntToStr(IRInstruction*);
  SSATmp* simplifyUnbox(IRInstruction*);
  SSATmp* simplifyUnboxPtr(IRInstruction*);
  SSATmp* simplifyCheckInit(IRInstruction* inst);
  SSATmp* simplifyPrint(IRInstruction* inst);
  SSATmp* simplifyDecRef(IRInstruction* inst);
  SSATmp* simplifyIncRef(IRInstruction* inst);
  SSATmp* simplifyGuardType(IRInstruction* inst);
  SSATmp* simplifyLdThis(IRInstruction*);
  SSATmp* simplifyLdCls(IRInstruction* inst);
  SSATmp* simplifyLdClsPropAddr(IRInstruction*);
  SSATmp* simplifyLdCtx(IRInstruction*);
  SSATmp* simplifyLdClsCtx(IRInstruction*);
  SSATmp* simplifyGetCtxFwdCall(IRInstruction* inst);
  SSATmp* simplifySpillStack(IRInstruction* inst);
  SSATmp* simplifyCall(IRInstruction* inst);

private:
  SSATmp* genDefInt(int64_t val);
  SSATmp* genDefDbl(double val);
  SSATmp* genDefBool(bool val);
  SSATmp* simplifyCmp(Opcode opName, SSATmp* src1, SSATmp* src2);
  SSATmp* simplifyCondJmp(IRInstruction*);
  SSATmp* simplifyQueryJmp(IRInstruction*);

private:
  TraceBuilder* const m_tb;
};

/*
 * Propagate very simple copies on the given instruction.
 * Specifically, Movs, and also IncRefs of non-refcounted types.
 *
 * More complicated copy-propagation is performed in the Simplifier.
 */
void copyProp(IRInstruction*);

}}}

#endif
