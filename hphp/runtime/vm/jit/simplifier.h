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

#include "hphp/runtime/vm/jit/bc-marker.h"
#include "hphp/runtime/vm/jit/containers.h"
#include "hphp/runtime/vm/jit/ir-opcode.h"

namespace HPHP { namespace jit {

struct IRInstruction;
struct IRUnit;
struct SSATmp;

//////////////////////////////////////////////////////////////////////

/*
 * Module that handles state-independent optimizations.
 *
 * Specifically, the optimizations in this module should be those that
 * we can do based only on chasing the use-def chain.  Instructions
 * can be modified in place or replaced with new instructions as
 * needed.
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
    jit::vector<IRInstruction*> instrs;
    SSATmp* dst;
  };
  Result simplify(const IRInstruction*, bool typesMightRelax);

private:
  SSATmp* simplifyWork(const IRInstruction*);

#define SIMPLIFY_INSTRS                         \
  X(Shl)                                        \
  X(Shr)                                        \
  X(AbsDbl)                                     \
  X(AssertNonNull)                              \
  X(BoxPtr)                                     \
  X(CallBuiltin)                                \
  X(CastStk)                                    \
  X(Ceil)                                       \
  X(CheckInit)                                  \
  X(CheckPackedArrayBounds)                     \
  X(CoerceCellToBool)                           \
  X(CoerceCellToDbl)                            \
  X(CoerceCellToInt)                            \
  X(CoerceStk)                                  \
  X(ConcatCellCell)                             \
  X(ConcatStrStr)                               \
  X(ConvArrToBool)                              \
  X(ConvArrToDbl)                               \
  X(ConvArrToInt)                               \
  X(ConvBoolToArr)                              \
  X(ConvBoolToDbl)                              \
  X(ConvBoolToInt)                              \
  X(ConvBoolToStr)                              \
  X(ConvCellToBool)                             \
  X(ConvCellToDbl)                              \
  X(ConvCellToInt)                              \
  X(ConvCellToObj)                              \
  X(ConvCellToStr)                              \
  X(ConvClsToCctx)                              \
  X(ConvDblToArr)                               \
  X(ConvDblToBool)                              \
  X(ConvDblToInt)                               \
  X(ConvDblToStr)                               \
  X(ConvIntToArr)                               \
  X(ConvIntToBool)                              \
  X(ConvIntToDbl)                               \
  X(ConvIntToStr)                               \
  X(ConvObjToBool)                              \
  X(ConvStrToArr)                               \
  X(ConvStrToBool)                              \
  X(ConvStrToDbl)                               \
  X(ConvStrToInt)                               \
  X(Count)                                      \
  X(CountArray)                                 \
  X(DecRef)                                     \
  X(DecRefNZ)                                   \
  X(DecRefStack)                                \
  X(DivDbl)                                     \
  X(Floor)                                      \
  X(GetCtxFwdCall)                              \
  X(IncRef)                                     \
  X(IncRefCtx)                                  \
  X(IsNType)                                    \
  X(IsScalarType)                               \
  X(IsType)                                     \
  X(IsWaitHandle)                               \
  X(LdClsCtx)                                   \
  X(LdClsName)                                  \
  X(LdCtx)                                      \
  X(LdObjClass)                                 \
  X(LdObjInvoke)                                \
  X(LdPackedArrayElem)                          \
  X(LdStack)                                    \
  X(LdStackAddr)                                \
  X(Mov)                                        \
  X(SpillStack)                                 \
  X(TakeStack)                                  \
  X(UnboxPtr)                                   \
  X(JmpGt)                                      \
  X(JmpGte)                                     \
  X(JmpLt)                                      \
  X(JmpLte)                                     \
  X(JmpEq)                                      \
  X(JmpNeq)                                     \
  X(JmpGtInt)                                   \
  X(JmpGteInt)                                  \
  X(JmpLtInt)                                   \
  X(JmpLteInt)                                  \
  X(JmpEqInt)                                   \
  X(JmpNeqInt)                                  \
  X(JmpSame)                                    \
  X(JmpNSame)                                   \
  X(JmpZero)                                    \
  X(JmpNZero)                                   \
  X(OrInt)                                      \
  X(AddInt)                                     \
  X(SubInt)                                     \
  X(MulInt)                                     \
  X(AddDbl)                                     \
  X(SubDbl)                                     \
  X(MulDbl)                                     \
  X(Mod)                                        \
  X(AndInt)                                     \
  X(XorInt)                                     \
  X(XorBool)                                    \
  X(AddIntO)                                    \
  X(SubIntO)                                    \
  X(MulIntO)                                    \
  X(Gt)                                         \
  X(Gte)                                        \
  X(Lt)                                         \
  X(Lte)                                        \
  X(Eq)                                         \
  X(Neq)                                        \
  X(GtInt)                                      \
  X(GteInt)                                     \
  X(LtInt)                                      \
  X(LteInt)                                     \
  X(EqInt)                                      \
  X(NeqInt)                                     \
  X(Same)                                       \
  X(NSame)                                      \
  /* */

  /*
   * Individual simplification routines return nullptr if they don't
   * want to change anything, or they can call gen any number of times
   * to produce a different IR sequence, returning the thing gen'd
   * that should be used as the value of the simplified instruction
   * sequence.
   */
#define X(x) SSATmp* simplify##x(const IRInstruction*);
  SIMPLIFY_INSTRS
#undef X

private:
  bool validate(SSATmp*, const IRInstruction*) const;
  SSATmp* cmpImpl(Opcode opName, const IRInstruction*, SSATmp*, SSATmp*);
  SSATmp* condJmpImpl(const IRInstruction*);
  SSATmp* queryJmpImpl(const IRInstruction*);
  SSATmp* isTypeImpl(const IRInstruction*);
  SSATmp* convToArrImpl(const IRInstruction*);
  SSATmp* decRefImpl(const IRInstruction*);
  template <class Oper>
  SSATmp* constImpl(SSATmp*, SSATmp*, Oper op);
  SSATmp* xorTrueImpl(SSATmp*);
  template<class Oper>
  SSATmp* commutativeImpl(SSATmp* src1,
                          SSATmp* src2,
                          Opcode opcode,
                          Oper op);
  template<class OutOper, class InOper>
  SSATmp* distributiveImpl(SSATmp* src1,
                           SSATmp* src2,
                           Opcode outcode,
                           Opcode incode,
                           OutOper outop,
                           InOper inop);
  template<class Oper>
  SSATmp* shiftImpl(const IRInstruction*, Oper op);
  template<class Oper>
  SSATmp* roundImpl(const IRInstruction*, Oper);

private:
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
  jit::stack<const IRInstruction*> m_insts;
  jit::vector<IRInstruction*> m_newInsts;
};

//////////////////////////////////////////////////////////////////////

struct StackValueInfo {
  explicit StackValueInfo(SSATmp*);
  explicit StackValueInfo(IRInstruction*, Type);

  SSATmp* value;   // may be nullptr
  Type knownType;  // the type of the value, for when value is nullptr
  bool spansCall;  // whether the tmp's definition was above a call
  IRInstruction* typeSrc; // the instruction that gave us knownType
};

std::string show(const StackValueInfo&);

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
jit::vector<SSATmp*> collectStackValues(SSATmp* sp, uint32_t stackDepth);

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
const SSATmp* canonical(const SSATmp*);
SSATmp* canonical(SSATmp*);

/*
 * Assuming sp is the VM stack pointer either from inside an FPI region or an
 * inlined call, find the SpillFrame instruction that defined the current
 * frame. Returns nullptr if the frame can't be found.
 */
IRInstruction* findSpillFrame(SSATmp* sp);

//////////////////////////////////////////////////////////////////////

}}

#endif
