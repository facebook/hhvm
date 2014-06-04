/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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

#include "hphp/runtime/base/smart-containers.h"
#include "hphp/runtime/vm/jit/cse.h"
#include "hphp/runtime/vm/jit/ir.h"

namespace HPHP {  namespace JIT {

//////////////////////////////////////////////////////////////////////

/*
 * Module that handles state-independent optimizations.
 *
 * Specifically, the optimizations in this module should be those that
 * we can do based only on chasing the use-def chain.  Instructions
 * can be modified in place or replaced with new instructions as
 * needed.
 *
 * The Simplifier recursively invokes itself, so that all instructions
 * returned from simplify() have been fully simplified themselves.
 *
 * The line of separation between these two modules is essentially
 * about who needs to know about tracked state.  If an optimization is
 * completely stateless (e.g. strength reduction, constant folding,
 * etc) it goes in here, otherwise it goes in IRBuilder or some
 * other pass.
 */
struct Simplifier {
  explicit Simplifier(IRUnit& unit) : m_unit(unit) {}

  Simplifier(const Simplifier&) = delete;
  Simplifier& operator=(const Simplifier&) = delete;

  /*
   * Simplify performs a number of optimizations.
   *
   * In general, the simplifier transforms one instruction into zero or more
   * instructions. The pair represents the zero or more instructions to replace
   * the input with, plus the SSATmp* to use instead of the input instruction's
   * dst (if any).
   */
  struct Result {
    smart::vector<IRInstruction*> instrs;
    SSATmp* dst;
  };
  Result simplify(const IRInstruction*, bool typesMightRelax);

private:
  SSATmp* simplifyWork(const IRInstruction*);

  /*
   * Individual simplification routines return nullptr if they don't
   * want to change anything, or they can call gen any number of times
   * to produce a different IR sequence, returning the thing gen'd
   * that should be used as the value of the simplified instruction
   * sequence.
   */
  SSATmp* simplifyMov(SSATmp* src);
  SSATmp* simplifyNot(SSATmp* src);
  SSATmp* simplifyAbsDbl(const IRInstruction* inst);
  SSATmp* simplifyAddInt(SSATmp* src1, SSATmp* src2);
  SSATmp* simplifySubInt(SSATmp* src1, SSATmp* src2);
  SSATmp* simplifyMulInt(SSATmp* src1, SSATmp* src2);
  SSATmp* simplifyAddDbl(SSATmp* src1, SSATmp* src2);
  SSATmp* simplifySubDbl(SSATmp* src1, SSATmp* src2);
  SSATmp* simplifyMulDbl(SSATmp* src1, SSATmp* src2);
  SSATmp* simplifyAddIntO(SSATmp* src1, SSATmp* src2);
  SSATmp* simplifySubIntO(SSATmp* src1, SSATmp* src2);
  SSATmp* simplifyMulIntO(SSATmp* src1, SSATmp* src2);
  SSATmp* simplifyMod(SSATmp* src1, SSATmp* src2);
  SSATmp* simplifyDivDbl(const IRInstruction* inst);
  SSATmp* simplifyAndInt(SSATmp* src1, SSATmp* src2);
  SSATmp* simplifyOrInt(SSATmp* src1, SSATmp* src2);
  SSATmp* simplifyXorInt(SSATmp* src1, SSATmp* src2);
  SSATmp* simplifyShl(const IRInstruction* inst);
  SSATmp* simplifyShr(const IRInstruction* inst);
  SSATmp* simplifyXorBool(SSATmp* src1, SSATmp* src2);
  SSATmp* simplifyXorTrue(SSATmp* src);
  SSATmp* simplifyGt(SSATmp* src1, SSATmp* src2);
  SSATmp* simplifyGte(SSATmp* src1, SSATmp* src2);
  SSATmp* simplifyLt(SSATmp* src1, SSATmp* src2);
  SSATmp* simplifyLte(SSATmp* src1, SSATmp* src2);
  SSATmp* simplifyEq(SSATmp* src1, SSATmp* src2);
  SSATmp* simplifyNeq(SSATmp* src1, SSATmp* src2);
  SSATmp* simplifySame(SSATmp* src1, SSATmp* src2);
  SSATmp* simplifyNSame(SSATmp* src1, SSATmp* src2);
  SSATmp* simplifyIsType(const IRInstruction*);
  SSATmp* simplifyIsScalarType(const IRInstruction*);
  SSATmp* simplifyConcatCellCell(const IRInstruction*);
  SSATmp* simplifyConcatStrStr(SSATmp* src1, SSATmp* src2);
  SSATmp* simplifyConvToArr(const IRInstruction*);
  SSATmp* simplifyConvArrToBool(const IRInstruction*);
  SSATmp* simplifyConvDblToBool(const IRInstruction*);
  SSATmp* simplifyConvIntToBool(const IRInstruction*);
  SSATmp* simplifyConvStrToBool(const IRInstruction*);
  SSATmp* simplifyConvToDbl(const IRInstruction*);
  SSATmp* simplifyConvArrToDbl(const IRInstruction*);
  SSATmp* simplifyConvBoolToDbl(const IRInstruction*);
  SSATmp* simplifyConvIntToDbl(const IRInstruction*);
  SSATmp* simplifyConvStrToDbl(const IRInstruction*);
  SSATmp* simplifyConvArrToInt(const IRInstruction*);
  SSATmp* simplifyConvBoolToInt(const IRInstruction*);
  SSATmp* simplifyConvDblToInt(const IRInstruction*);
  SSATmp* simplifyConvObjToInt(const IRInstruction*);
  SSATmp* simplifyConvStrToInt(const IRInstruction*);
  SSATmp* simplifyConvBoolToStr(const IRInstruction*);
  SSATmp* simplifyConvDblToStr(const IRInstruction*);
  SSATmp* simplifyConvIntToStr(const IRInstruction*);
  SSATmp* simplifyConvCellToBool(const IRInstruction*);
  SSATmp* simplifyConvCellToStr(const IRInstruction*);
  SSATmp* simplifyConvCellToInt(const IRInstruction*);
  SSATmp* simplifyConvCellToDbl(const IRInstruction*);
  SSATmp* simplifyFloor(const IRInstruction*);
  SSATmp* simplifyCeil(const IRInstruction*);
  SSATmp* simplifyUnboxPtr(const IRInstruction*);
  SSATmp* simplifyBoxPtr(const IRInstruction*);
  SSATmp* simplifyCheckInit(const IRInstruction* inst);
  SSATmp* simplifyDecRef(const IRInstruction* inst);
  SSATmp* simplifyIncRef(const IRInstruction* inst);
  SSATmp* simplifyIncRefCtx(const IRInstruction* inst);
  SSATmp* simplifyLdCls(const IRInstruction* inst);
  SSATmp* simplifyLdClsPropAddr(const IRInstruction*);
  SSATmp* simplifyLdCtx(const IRInstruction*);
  SSATmp* simplifyLdClsCtx(const IRInstruction*);
  SSATmp* simplifyGetCtxFwdCall(const IRInstruction* inst);
  SSATmp* simplifyConvClsToCctx(const IRInstruction* inst);
  SSATmp* simplifySpillStack(const IRInstruction* inst);
  SSATmp* simplifyCmp(Opcode opName, const IRInstruction* inst,
                      SSATmp* src1, SSATmp* src2);
  SSATmp* simplifyCondJmp(const IRInstruction*);
  SSATmp* simplifyQueryJmp(const IRInstruction*);
  SSATmp* simplifyExitOnVarEnv(const IRInstruction*);
  SSATmp* simplifyCastStk(const IRInstruction*);
  SSATmp* simplifyCoerceStk(const IRInstruction*);
  SSATmp* simplifyLdStack(const IRInstruction*);
  SSATmp* simplifyTakeStack(const IRInstruction*);
  SSATmp* simplifyLdStackAddr(const IRInstruction*);
  SSATmp* simplifyDecRefStack(const IRInstruction*);
  SSATmp* simplifyLdLoc(const IRInstruction*);
  SSATmp* simplifyAssertNonNull(const IRInstruction*);
  SSATmp* simplifyCallBuiltin(const IRInstruction*);
  SSATmp* simplifyConvObjToBool(const IRInstruction*);
  SSATmp* simplifyIsWaitHandle(const IRInstruction*);
  SSATmp* simplifyCount(const IRInstruction*);
  SSATmp* simplifyCountArray(const IRInstruction*);
  SSATmp* simplifyCountCollection(const IRInstruction*);

  template <class Oper>
  SSATmp* simplifyConst(SSATmp* src1, SSATmp* src2, Oper op);

  template <class Oper>
  SSATmp* simplifyCommutative(SSATmp* src1,
                              SSATmp* src2,
                              Opcode opcode,
                              Oper op);

  template <class OutOper, class InOper>
  SSATmp* simplifyDistributive(SSATmp* src1,
                               SSATmp* src2,
                               Opcode outcode,
                               Opcode incode,
                               OutOper outop,
                               InOper inop);

  template<class Oper>
  SSATmp* simplifyShift(SSATmp* src1, SSATmp* src2, Oper op);
  template<class Oper> SSATmp* simplifyRoundCommon(const IRInstruction*, Oper);

  SSATmp* simplifyCheckPackedArrayBounds(const IRInstruction*);
  SSATmp* simplifyLdPackedArrayElem(const IRInstruction*);

  bool typeMightRelax(SSATmp* src) const;

private: // makeInstruction forwarders
  template<class... Args> SSATmp* cns(Args&&...);
  template<class... Args> SSATmp* gen(Opcode op, Args&&...);
  template<class... Args> SSATmp* gen(Opcode op, BCMarker marker, Args&&...);

private:
  IRUnit& m_unit;
  bool m_typesMightRelax;

  // The current instruction being simplified is always at
  // m_insts.top(). This has to be a stack instead of just a pointer
  // because simplify is reentrant.
  smart::stack<const IRInstruction*> m_insts;
  smart::vector<IRInstruction*> m_newInsts;
};

//////////////////////////////////////////////////////////////////////

struct StackValueInfo {
  explicit StackValueInfo(SSATmp* value)
    : value(value)
    , knownType(value->type())
    , spansCall(false)
    , typeSrc(value->inst())
  {
    ITRACE(5, "{} created\n", show());
  }

  explicit StackValueInfo(IRInstruction* inst, Type type)
    : value(nullptr)
    , knownType(type)
    , spansCall(false)
    , typeSrc(inst)
  {
    ITRACE(5, "{} created\n", show());
  }

  std::string show() const {
    std::string out = "StackValueInfo {";

    if (value) {
      out += value->inst()->toString();
    } else {
      folly::toAppend(knownType.toString(), " from ", typeSrc->toString(),
                      &out);
    }

    if (spansCall) out += ", spans call";
    out += "}";

    return out;
  }

  SSATmp* value;   // may be nullptr
  Type knownType;  // the type of the value, for when value is nullptr
  bool spansCall;  // whether the tmp's definition was above a call
  IRInstruction* typeSrc; // the instruction that gave us knownType

 private:
  TRACE_SET_MOD(hhir);
};

/*
 * Track down a value or type using the StkPtr chain.
 *
 * The spansCall parameter tracks whether the returned value's
 * lifetime on the stack spans a call.  This search bottoms out on
 * hitting either the initial DefSP instruction (failure), or some
 * instruction that produced a view of the stack with the requested
 * value.
 */
StackValueInfo getStackValue(SSATmp* stack, uint32_t index);

/*
 * Return this list of all values that are known to be on the stack
 * given the particular depth.
 *
 * This function is used for computing available value for
 * DecRef->DecRefNZ conversions in IRBuilder.
 */
smart::vector<SSATmp*> collectStackValues(SSATmp* sp, uint32_t stackDepth);

/*
 * Propagate very simple copies on the given instruction.
 * Specifically, Movs.
 *
 * More complicated copy-propagation is performed in the Simplifier.
 */
void copyProp(IRInstruction*);

/*
 * Returns the canonical version of the given value by tracing through any
 * passthrough instructions (Mov, CheckType, etc...).
 */
const SSATmp* canonical(const SSATmp* tmp);
SSATmp* canonical(SSATmp* tmp);

/*
 * Assuming sp is the VM stack pointer either from inside an FPI region or an
 * inlined call, find the SpillFrame instruction that defined the current
 * frame. Returns nullptr if the frame can't be found.
 */
IRInstruction* findSpillFrame(SSATmp* sp);

/*
 * Given an instruction defining a frame pointer, chase backwards in the
 * definition chain looking for the first DefFP or DefInlineFP. Never returns
 * nullptr.
 */
const IRInstruction* frameRoot(const IRInstruction* inst);
IRInstruction* frameRoot(IRInstruction* inst);

//////////////////////////////////////////////////////////////////////

}}

#endif
