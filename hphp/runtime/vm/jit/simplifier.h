/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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

class TraceBuilder;

//////////////////////////////////////////////////////////////////////

/*
 * Module that handles state-independent optimizations.
 *
 * Specifically, the optimizations in this module should be those that
 * we can do based only on chasing the use-def chain.  Instructions
 * can be modified in place or replaced with new instructions as
 * needed.
 *
 * The Simplifier recursively invokes TraceBuilder, which can call
 * back into it.  It's used both during our initial gen-time
 * optimizations and in the TraceBuilder::reoptimize pass.
 *
 * The line of separation between these two modules is essentially
 * about who needs to know about tracked state.  If an optimization is
 * completely stateless (e.g. strength reduction, constant folding,
 * etc) it goes in here, otherwise it goes in TraceBuilder or some
 * other pass.
 */
struct Simplifier {
  explicit Simplifier(TraceBuilder& t) : m_tb(t) {}

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
  SSATmp* simplifyAbsInt(IRInstruction* inst);
  SSATmp* simplifyAbsDbl(IRInstruction* inst);
  SSATmp* simplifyAdd(SSATmp* src1, SSATmp* src2);
  SSATmp* simplifySub(SSATmp* src1, SSATmp* src2);
  SSATmp* simplifyMul(SSATmp* src1, SSATmp* src2);
  SSATmp* simplifyMod(SSATmp* src1, SSATmp* src2);
  SSATmp* simplifyDivDbl(IRInstruction* inst);
  SSATmp* simplifyBitAnd(SSATmp* src1, SSATmp* src2);
  SSATmp* simplifyBitOr(SSATmp* src1, SSATmp* src2);
  SSATmp* simplifyBitXor(SSATmp* src1, SSATmp* src2);
  SSATmp* simplifyShl(IRInstruction* inst);
  SSATmp* simplifyShr(IRInstruction* inst);
  SSATmp* simplifyLogicXor(SSATmp* src1, SSATmp* src2);
  SSATmp* simplifyGt(SSATmp* src1, SSATmp* src2);
  SSATmp* simplifyGte(SSATmp* src1, SSATmp* src2);
  SSATmp* simplifyLt(SSATmp* src1, SSATmp* src2);
  SSATmp* simplifyLte(SSATmp* src1, SSATmp* src2);
  SSATmp* simplifyEq(SSATmp* src1, SSATmp* src2);
  SSATmp* simplifyNeq(SSATmp* src1, SSATmp* src2);
  SSATmp* simplifySame(SSATmp* src1, SSATmp* src2);
  SSATmp* simplifyNSame(SSATmp* src1, SSATmp* src2);
  SSATmp* simplifyIsType(IRInstruction*);
  SSATmp* simplifyIsScalarType(IRInstruction*);
  SSATmp* simplifyJmpIsType(IRInstruction*);
  SSATmp* simplifyConcatCellCell(IRInstruction*);
  SSATmp* simplifyConcatStrStr(SSATmp* src1, SSATmp* src2);
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
  SSATmp* simplifyConvCellToBool(IRInstruction*);
  SSATmp* simplifyConvCellToStr(IRInstruction*);
  SSATmp* simplifyConvCellToInt(IRInstruction*);
  SSATmp* simplifyConvCellToDbl(IRInstruction*);
  SSATmp* simplifyFloor(IRInstruction*);
  SSATmp* simplifyCeil(IRInstruction*);
  SSATmp* simplifyUnbox(IRInstruction*);
  SSATmp* simplifyUnboxPtr(IRInstruction*);
  SSATmp* simplifyCheckInit(IRInstruction* inst);
  SSATmp* simplifyPrint(IRInstruction* inst);
  SSATmp* simplifyDecRef(IRInstruction* inst);
  SSATmp* simplifyIncRef(IRInstruction* inst);
  SSATmp* simplifyIncRefCtx(IRInstruction* inst);
  SSATmp* simplifyCheckType(IRInstruction* inst);
  SSATmp* simplifyAssertType(IRInstruction* inst);
  SSATmp* simplifyCheckStk(IRInstruction* inst);
  SSATmp* simplifyLdCls(IRInstruction* inst);
  SSATmp* simplifyLdClsPropAddr(IRInstruction*);
  SSATmp* simplifyLdCtx(IRInstruction*);
  SSATmp* simplifyLdClsCtx(IRInstruction*);
  SSATmp* simplifyGetCtxFwdCall(IRInstruction* inst);
  SSATmp* simplifyConvClsToCctx(IRInstruction* inst);
  SSATmp* simplifySpillStack(IRInstruction* inst);
  SSATmp* simplifyCall(IRInstruction* inst);
  SSATmp* simplifyCmp(Opcode opName, IRInstruction* inst,
                      SSATmp* src1, SSATmp* src2);
  SSATmp* simplifyCondJmp(IRInstruction*);
  SSATmp* simplifyQueryJmp(IRInstruction*);
  SSATmp* simplifyExitOnVarEnv(IRInstruction*);
  SSATmp* simplifyCastStk(IRInstruction*);
  SSATmp* simplifyCoerceStk(IRInstruction*);
  SSATmp* simplifyAssertStk(IRInstruction*);
  SSATmp* simplifyLdStack(IRInstruction*);
  SSATmp* simplifyTakeStack(IRInstruction*);
  SSATmp* simplifyLdStackAddr(IRInstruction*);
  SSATmp* simplifyDecRefStack(IRInstruction*);
  SSATmp* simplifyDecRefLoc(IRInstruction*);
  SSATmp* simplifyLdLoc(IRInstruction*);
  SSATmp* simplifyStRef(IRInstruction*);
  SSATmp* simplifyAssertNonNull(IRInstruction*);


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
  template<class Oper> SSATmp* simplifyRoundCommon(IRInstruction*, Oper);

  SSATmp* simplifyCheckPackedArrayBounds(IRInstruction*);

private: // tracebuilder forwarders
  template<class... Args> SSATmp* cns(Args&&...);
  template<class... Args> SSATmp* gen(Opcode op, Args&&...);
  template<class... Args> SSATmp* gen(Opcode op, BCMarker marker, Args&&...);

private:
  TraceBuilder& m_tb;

  // The current instruction being simplified is always at
  // m_insts.top(). This has to be a stack instead of just a pointer
  // because simplify is reentrant.
  smart::stack<const IRInstruction*> m_insts;
};

//////////////////////////////////////////////////////////////////////

struct StackValueInfo {
  explicit StackValueInfo(SSATmp* value)
    : value(value)
    , knownType(value->type())
    , spansCall(false)
    , typeSrc(value->inst())
  {
    TRACE(5, "%s created\n", show().c_str());
  }

  explicit StackValueInfo(IRInstruction* inst, Type type)
    : value(nullptr)
    , knownType(type)
    , spansCall(false)
    , typeSrc(inst)
  {
    TRACE(5, "%s created\n", show().c_str());
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
 * If the typeParam of inst isn't a subtype of oldType, filter out the
 * parts of the typeParam that aren't in oldType and return
 * true. Otherwise, return false.
 */
bool filterAssertType(IRInstruction* inst, Type oldType);

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
 * DecRef->DecRefNZ conversions in tracebuilder.
 */
smart::vector<SSATmp*> collectStackValues(SSATmp* sp, uint32_t stackDepth);

/*
 * Propagate very simple copies on the given instruction.
 * Specifically, Movs, and also IncRefs of non-refcounted types.
 *
 * More complicated copy-propagation is performed in the Simplifier.
 */
void copyProp(IRInstruction*);

/*
 * Checks if property propName of class clsTmp, called from context class ctx,
 * can be accessed via the static property cache.
 */
bool canUseSPropCache(SSATmp* clsTmp,
                      const StringData* propName,
                      const Class* ctx);


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
 * definition chain looking for a PassFP, returning it if found. If a
 * DefInlineFP or DefFP is found before a PassFP, returns nullptr.
 */
IRInstruction* findPassFP(IRInstruction* inst);

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
