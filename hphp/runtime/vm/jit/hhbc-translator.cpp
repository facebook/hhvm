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
#include "hphp/runtime/vm/jit/hhbc-translator.h"
#include <algorithm>
#include <vector>

#include "folly/CpuId.h"
#include "folly/Optional.h"

#include "hphp/util/trace.h"
#include "hphp/runtime/ext/ext_closure.h"
#include "hphp/runtime/ext/ext_generator.h"
#include "hphp/runtime/ext/asio/wait_handle.h"
#include "hphp/runtime/ext/asio/async_function_wait_handle.h"
#include "hphp/runtime/base/stats.h"
#include "hphp/runtime/vm/repo.h"
#include "hphp/runtime/vm/repo-global-data.h"
#include "hphp/runtime/vm/unit.h"
#include "hphp/runtime/vm/instance-bits.h"
#include "hphp/runtime/vm/runtime.h"
#include "hphp/runtime/vm/jit/code-gen-helpers.h"
#include "hphp/runtime/vm/jit/ir-unit.h"
#include "hphp/runtime/vm/jit/normalized-instruction.h"
#include "hphp/runtime/vm/jit/translator-inline.h"
#include "hphp/runtime/vm/jit/mc-generator.h"
#include "hphp/runtime/vm/jit/target-profile.h"

// Include last to localize effects to this file
#include "hphp/util/assert-throw.h"

namespace HPHP { namespace jit {

TRACE_SET_MOD(hhir);

//////////////////////////////////////////////////////////////////////

namespace {

bool classIsUnique(const Class* cls) {
  return RuntimeOption::RepoAuthoritative && cls && (cls->attrs() & AttrUnique);
}

bool classIsUniqueNormalClass(const Class* cls) {
  return classIsUnique(cls) && isNormalClass(cls);
}

bool classIsUniqueInterface(const Class* cls) {
  return classIsUnique(cls) && isInterface(cls);
}

}

//////////////////////////////////////////////////////////////////////

HhbcTranslator::HhbcTranslator(TransContext context)
  : m_context(context)
  , m_unit(context)
  , m_irb(new IRBuilder(context.initSpOffset, m_unit, context.func))
  , m_bcStateStack { BcState { context.initBcOffset,
                               context.resumed,
                               context.func } }
  , m_lastBcOff{false}
  , m_mode{IRGenMode::Trace}
{
  updateMarker();
  auto const fp = gen(DefFP);
  gen(DefSP, StackOffset{context.initSpOffset}, fp);
}

void HhbcTranslator::setGenMode(IRGenMode mode) {
  m_mode = mode;
}

bool HhbcTranslator::classIsUniqueOrCtxParent(const Class* cls) const {
  if (!cls) return false;
  if (classIsUnique(cls)) return true;
  if (!curClass()) return false;
  return curClass()->classof(cls);
}

bool HhbcTranslator::classIsPersistentOrCtxParent(const Class* cls) const {
  if (!cls) return false;
  if (classHasPersistentRDS(cls)) return true;
  if (!curClass()) return false;
  return curClass()->classof(cls);
}

SrcKey HhbcTranslator::nextSrcKey() const {
  SrcKey srcKey = curSrcKey();
  srcKey.advance(curFunc()->unit());
  return srcKey;
}

Offset HhbcTranslator::nextBcOff() const {
  return nextSrcKey().offset();
}

ArrayData* HhbcTranslator::lookupArrayId(int arrId) {
  return curUnit()->lookupArrayId(arrId);
}

StringData* HhbcTranslator::lookupStringId(int strId) {
  return curUnit()->lookupLitstrId(strId);
}

Func* HhbcTranslator::lookupFuncId(int funcId) {
  return curUnit()->lookupFuncId(funcId);
}

PreClass* HhbcTranslator::lookupPreClassId(int preClassId) {
  return curUnit()->lookupPreClassId(preClassId);
}

const NamedEntityPair& HhbcTranslator::lookupNamedEntityPairId(int id) {
  return curUnit()->lookupNamedEntityPairId(id);
}

const NamedEntity* HhbcTranslator::lookupNamedEntityId(int id) {
  return curUnit()->lookupNamedEntityId(id);
}

SSATmp* HhbcTranslator::push(SSATmp* tmp) {
  assert(tmp);
  FTRACE(2, "HhbcTranslator pushing {}\n", *tmp->inst());
  m_irb->evalStack().push(tmp);
  return tmp;
}

SSATmp* HhbcTranslator::pushIncRef(SSATmp* tmp, TypeConstraint tc) {
  m_irb->constrainValue(tmp, tc);
  gen(IncRef, tmp);
  return push(tmp);
}

SSATmp* HhbcTranslator::pop(Type type, TypeConstraint tc) {
  SSATmp* opnd = m_irb->evalStack().pop();
  m_irb->constrainValue(opnd, tc);

  if (opnd == nullptr) {
    uint32_t stackOff = m_irb->stackDeficit();
    m_irb->incStackDeficit();
    m_irb->constrainStack(stackOff, tc);

    // pop() is usually called with Cell or Gen. Don't rely
    // on the simplifier to get a better type for the LdStack.
    auto const info = getStackValue(m_irb->sp(), stackOff);
    type = std::min(type, info.knownType);

    auto value = gen(LdStack, type, StackOffset(stackOff), m_irb->sp());
    FTRACE(2, "HhbcTranslator popping {}\n", *value->inst());
    return value;
  }

  FTRACE(2, "HhbcTranslator popping {}\n", *opnd->inst());
  return opnd;
}

void HhbcTranslator::discard(unsigned n) {
  for (unsigned i = 0; i < n; ++i) {
    pop(Type::StackElem, DataTypeGeneric); // don't care about the values
  }
}

// type is the type expected on the stack.
void HhbcTranslator::popDecRef(Type type, TypeConstraint tc) {
  if (SSATmp* src = m_irb->evalStack().pop()) {
    m_irb->constrainValue(src, tc);
    gen(DecRef, src);
    return;
  }

  m_irb->constrainStack(m_irb->stackDeficit(), tc);
  gen(DecRefStack, StackOffset(m_irb->stackDeficit()), type, m_irb->sp());
  m_irb->incStackDeficit();
}

// We don't know what type description to expect for the stack
// locations before index, so we use a generic type when popping the
// intermediate values.  If it ends up creating a new LdStack,
// refineType during a later pop() or top() will fix up the type to
// the known type.
void HhbcTranslator::extendStack(uint32_t index, Type type) {
  // DataTypeGeneric is used in here because nobody's actually looking at the
  // values, we're just inserting LdStacks into the eval stack to be consumed
  // elsewhere.
  if (index == 0) {
    push(pop(type, DataTypeGeneric));
    return;
  }

  SSATmp* tmp = pop(Type::StackElem, DataTypeGeneric);
  extendStack(index - 1, type);
  push(tmp);
}

SSATmp* HhbcTranslator::top(TypeConstraint tc, uint32_t index) const {
  SSATmp* tmp = m_irb->evalStack().top(index);
  if (!tmp) return nullptr;
  m_irb->constrainValue(tmp, tc);
  return tmp;
}

SSATmp* HhbcTranslator::top(Type type, uint32_t index,
                            TypeConstraint constraint) {
  SSATmp* tmp = top(constraint, index);
  if (!tmp) {
    extendStack(index, type);
    tmp = top(constraint, index);
  }
  assert(tmp);
  return tmp;
}

void HhbcTranslator::replace(uint32_t index, SSATmp* tmp) {
  m_irb->evalStack().replace(index, tmp);
}

Type HhbcTranslator::topType(uint32_t idx, TypeConstraint constraint) const {
  FTRACE(5, "Asking for type of stack elem {}\n", idx);
  if (idx < m_irb->evalStack().size()) {
    return top(constraint, idx)->type();
  } else {
    auto absIdx = idx - m_irb->evalStack().size() + m_irb->stackDeficit();
    auto stkVal = getStackValue(m_irb->sp(), absIdx);
    m_irb->constrainStack(absIdx, constraint);
    return stkVal.knownType;
  }
}

size_t HhbcTranslator::spOffset() const {
  return m_irb->spOffset() + m_irb->evalStack().size() - m_irb->stackDeficit();
}

/*
 * When doing gen-time inlining, we set up a series of IR instructions
 * that looks like this:
 *
 *   fp0  = DefFP
 *   sp0  = DefSP<offset>
 *
 *   // ... normal stuff happens ...
 *   // sp_pre = some SpillStack, or maybe the DefSP
 *
 *   // FPI region:
 *     sp1   = SpillStack sp_pre, ...
 *     sp2   = SpillFrame sp1, ...
 *     // ... possibly more spillstacks due to argument expressions
 *     sp3   = SpillStack sp2, -argCount
 *     fp2   = DefInlineFP<func,retBC,retSP> sp2 sp1
 *     sp4   = ReDefSP<spOffset,spansCall> sp1 fp2
 *
 *         // ... callee body ...
 *
 *           = InlineReturn fp2
 *
 * [ sp5  = ReDefSP<spOffset,spansCall> sp1 fp0 ]
 *
 * The rest of the code then depends on sp5, and not any of the StkPtr
 * tree going through the callee body.  The sp5 tmp has the same view
 * of the stack as sp1 did, which represents what the stack looks like
 * before the return address is pushed but after the activation record
 * is popped.
 *
 * In DCE we attempt to remove the SpillFrame, InlineReturn, and
 * DefInlineFP instructions if they aren't needed.
 *
 * ReDefSP takes sp1, the stack pointer from before the inlined frame.
 * This SSATmp may be used for determining stack types in the
 * simplifier, or stack values if the inlined body doesn't contain a
 * call---these instructions both take an extradata `spansCall' which
 * is true iff a Call occured anywhere between the the definition of
 * its first argument and itself.
 */
void HhbcTranslator::beginInlining(unsigned numParams,
                                   const Func* target,
                                   Offset returnBcOffset,
                                   Type retTypePred) {
  assert(!m_fpiStack.empty() &&
    "Inlining does not support calls with the FPush* in a different Tracelet");
  assert(returnBcOffset >= 0 && "returnBcOffset before beginning of caller");
  assert(curFunc()->base() + returnBcOffset < curFunc()->past() &&
         "returnBcOffset past end of caller");

  FTRACE(1, "[[[ begin inlining: {}\n", target->fullName()->data());

  SSATmp* params[numParams];
  for (unsigned i = 0; i < numParams; ++i) {
    params[numParams - i - 1] = popF();
  }

  auto const prevSP    = m_fpiStack.top().first;
  auto const prevSPOff = m_fpiStack.top().second;
  auto const calleeSP  = spillStack();

  DefInlineFPData data;
  data.target   = target;
  data.retBCOff = returnBcOffset;
  data.retSPOff = prevSPOff;
  data.retTypePred = retTypePred;

  // Push state and update the marker before emitting any instructions so
  // they're all given markers in the callee.
  m_bcStateStack.emplace_back(BcState { target->getEntryForNumArgs(numParams),
                                        false,
                                        target});
  updateMarker();

  always_assert_flog(
    findSpillFrame(calleeSP),
    "Couldn't find SpillFrame for inlined call on sp {}."
    " Was the FPush instruction interpreted?\n{}",
    *calleeSP->inst(), m_irb->unit()
  );

  auto const calleeFP = gen(DefInlineFP, data, calleeSP, prevSP, m_irb->fp());
  gen(
    ReDefSP,
    ReDefSPData {
      target->numLocals(),
      false /* spansCall; calls in FPI regions are not inline
             * candidates currently */
    },
    m_irb->sp(),
    m_irb->fp()
  );

  profileFunctionEntry("Inline");

  for (unsigned i = 0; i < numParams; ++i) {
    genStLocal(i, calleeFP, params[i]);
  }
  for (unsigned i = numParams; i < target->numLocals(); ++i) {
    /*
     * Here we need to be generating hopefully-dead stores to
     * initialize non-parameter locals to KindOfUninit in case we have
     * to leave the trace.
     */
    genStLocal(i, calleeFP, cns(Type::Uninit));
  }

  m_fpiActiveStack.push(std::move(m_fpiStack.top()));
  m_fpiStack.pop();
}

bool HhbcTranslator::isInlining() const {
  return m_bcStateStack.size() > 1;
}

int HhbcTranslator::inliningDepth() const {
  return m_bcStateStack.size() - 1;
}

BCMarker HhbcTranslator::makeMarker(Offset bcOff) {
  int32_t stackOff = m_irb->spOffset() +
    m_irb->evalStack().numCells() - m_irb->stackDeficit();

  FTRACE(2, "makeMarker: bc {} sp {} fn {}\n",
         bcOff, stackOff, curFunc()->fullName()->data());

  return BCMarker {
    SrcKey { curFunc(), bcOff, resumed() },
    stackOff,
    m_profTransID
  };
}

void HhbcTranslator::updateMarker() {
  m_irb->setMarker(makeMarker(bcOff()));
}

void HhbcTranslator::profileFunctionEntry(const char* category) {
  static const bool enabled = Stats::enabledAny() &&
                              getenv("HHVM_STATS_FUNCENTRY");
  if (!enabled) return;

  gen(
    IncStatGrouped,
    cns(makeStaticString("FunctionEntry")),
    cns(makeStaticString(category)),
    cns(1)
  );
}

void HhbcTranslator::profileInlineFunctionShape(const std::string& str) {
  gen(
    IncStatGrouped,
    cns(makeStaticString("InlineShape")),
    cns(makeStaticString(str)),
    cns(1)
  );
}

void HhbcTranslator::profileSmallFunctionShape(const std::string& str) {
  gen(
    IncStatGrouped,
    cns(makeStaticString("SmallFunctions")),
    cns(makeStaticString(str)),
    cns(1)
  );
}

void HhbcTranslator::profileFailedInlShape(const std::string& str) {
  gen(
    IncStatGrouped,
    cns(makeStaticString("FailedInl")),
    cns(makeStaticString(str)),
    cns(1)
  );
}

void HhbcTranslator::setProfTransID(TransID id) {
  m_profTransID = id;
}

void HhbcTranslator::setBcOff(Offset newOff, bool lastBcOff) {
  always_assert_log(
    IMPLIES(isInlining(), !lastBcOff),
    [&] {
      return folly::format("Tried to end trace while inlining:\n{}",
                           unit()).str();
    }
  );

  m_bcStateStack.back().bcOff = newOff;
  updateMarker();
  m_lastBcOff = lastBcOff;
}

void HhbcTranslator::emitSingletonSProp(const Func* func,
                                        const Op* clsOp,
                                        const Op* propOp) {
  assert(*clsOp == Op::String);
  assert(*propOp == Op::String);

  TransFlags trflags;
  trflags.noinlineSingleton = true;

  auto exitBlock = makeExit(trflags);
  auto catchBlock = makeCatch();

  // Pull the class and property names.
  auto const unit = func->unit();
  auto const clsName  = unit->lookupLitstrId(getImmPtr(clsOp,  0)->u_SA);
  auto const propName = unit->lookupLitstrId(getImmPtr(propOp, 0)->u_SA);

  // Make sure we have a valid class.
  auto const cls = Unit::lookupClass(clsName);
  if (UNLIKELY(!classHasPersistentRDS(cls))) {
    PUNT(SingletonSProp-Persistent);
  }

  // Make sure the sprop is accessible from the singleton method's context.
  bool visible, accessible;
  cls->findSProp(func->cls(), propName, visible, accessible);

  if (UNLIKELY(!visible || !accessible)) {
    PUNT(SingletonSProp-Accessibility);
  }

  // Look up the static property.
  auto const sprop   = ldClsPropAddrKnown(catchBlock, cns(cls), cns(propName));
  auto const unboxed = gen(UnboxPtr, sprop);
  auto const value   = gen(LdMem, unboxed->type().deref(), unboxed, cns(0));

  // Side exit if the static property is null.
  auto isnull = gen(IsType, Type::Null, value);
  gen(JmpNZero, exitBlock, isnull);

  // Return the singleton.
  pushIncRef(value);
}

void HhbcTranslator::emitSingletonSLoc(const Func* func, const Op* op) {
  assert(*op == Op::StaticLocInit);

  TransFlags trflags;
  trflags.noinlineSingleton = true;

  auto exit = makeExit(trflags);
  auto const name = func->unit()->lookupLitstrId(getImmPtr(op, 1)->u_SA);

  // Side exit if the static local is uninitialized.
  auto const box = gen(LdStaticLocCached, StaticLocName { func, name });
  gen(CheckStaticLocInit, exit, box);

  // Side exit if the static local is null.
  auto value = gen(LdRef, Type::Cell, exit, box);
  auto isnull = gen(IsType, Type::Null, value);
  gen(JmpNZero, exit, isnull);

  // Return the singleton.
  pushIncRef(value);
}

void HhbcTranslator::emitPrint() {
  Type type = topC()->type();
  if (type.subtypeOfAny(Type::Int, Type::Bool, Type::Null, Type::Str)) {
    auto catchBlock = makeCatch();
    auto const cell = popC();

    Opcode op;
    if (type <= Type::Str) {
      op = PrintStr;
    } else if (type <= Type::Int) {
      op = PrintInt;
    } else if (type <= Type::Bool) {
      op = PrintBool;
    } else {
      assert(type <= Type::Null);
      op = Nop;
    }
    // the print helpers decref their arg, so don't decref pop'ed value
    if (op != Nop) {
      gen(op, catchBlock, cell);
    }
    push(cns(1));
  } else {
    emitInterpOne(Type::Int, 1);
  }
}

void HhbcTranslator::emitUnboxRAux() {
  Block* exit = makeExit();
  SSATmp* srcBox = popR();
  SSATmp* unboxed = unbox(srcBox, exit);
  if (unboxed == srcBox) {
    // If the Unbox ended up being a noop, don't bother refcounting
    push(unboxed);
  } else {
    pushIncRef(unboxed);
    gen(DecRef, srcBox);
  }
}

void HhbcTranslator::emitUnboxR() {
  emitUnboxRAux();
}

void HhbcTranslator::emitUnbox() {
  Block* exit = makeExit();
  SSATmp* srcBox = popV();
  SSATmp* unboxed = unbox(srcBox, exit);
  pushIncRef(unboxed);
  gen(DecRef, srcBox);
}

void HhbcTranslator::emitThis() {
  pushIncRef(gen(LdThis, makeExitNullThis(), m_irb->fp()));
}

void HhbcTranslator::emitCheckThis() {
  gen(LdThis, makeExitNullThis(), m_irb->fp());
}

void HhbcTranslator::emitRB(Trace::RingBufferType t, SrcKey sk, int level) {
  if (!Trace::moduleEnabledRelease(Trace::ringbuffer, level)) return;

  gen(RBTrace, RBTraceData(t, sk));
}

void HhbcTranslator::emitRB(Trace::RingBufferType t, const StringData* msg,
                            int level) {
  if (!Trace::moduleEnabledRelease(Trace::ringbuffer, level)) return;

  gen(RBTrace, RBTraceData(t, msg));
}

void HhbcTranslator::emitRB(Trace::RingBufferType t, std::string msg,
                            int level) {
  emitRB(t, makeStaticString(msg), level);
}

void HhbcTranslator::emitDbgAssertRetAddr() {
  gen(DbgAssertRetAddr);
}

void HhbcTranslator::emitBareThis(int notice) {
  // We just exit the trace in the case $this is null. Before exiting
  // the trace, we could also push null onto the stack and raise a
  // notice if the notice argument is set. By exiting the trace when
  // $this is null, we can be sure in the rest of the trace that we
  // have the this object on top of the stack, and we can eliminate
  // further null checks of this.
  if (!curClass()) {
    emitInterpOne(Type::InitNull, 0); // will raise notice and push null
    return;
  }
  if (notice == static_cast<int>(BareThisOp::NeverNull)) {
    setThisAvailable();
  }
  pushIncRef(gen(LdThis, makeExitSlow(), m_irb->fp()));
}

void HhbcTranslator::emitArray(int arrayId) {
  push(cns(lookupArrayId(arrayId)));
}

void HhbcTranslator::emitNewArray(int capacity) {
  if (capacity == 0) {
    push(cns(staticEmptyArray()));
  } else {
    push(gen(NewArray, cns(capacity)));
  }
}

void HhbcTranslator::emitNewMixedArray(int capacity) {
  if (capacity == 0) {
    push(cns(staticEmptyArray()));
  } else {
    push(gen(NewMixedArray, cns(capacity)));
  }
}

void HhbcTranslator::emitNewVArray(int capacity) {
  // TODO(t4757263) staticEmptyArray() for VArray
  push(gen(NewVArray, cns(capacity)));
}

void HhbcTranslator::emitNewMIArray(int capacity) {
  // TODO(t4757263) staticEmptyArray() for IntMap
  push(gen(NewMIArray, cns(capacity)));
}

void HhbcTranslator::emitNewMSArray(int capacity) {
  // TODO(t4757263) staticEmptyArray() for StrMap
  push(gen(NewMSArray, cns(capacity)));
}

void HhbcTranslator::emitNewLikeArrayL(int id, int capacity) {
  auto const ldrefExit = makeExit();
  auto const ldgblExit = makeExit();
  auto const ld = ldLocInner(id, ldrefExit, ldgblExit, DataTypeSpecific);

  SSATmp* arr;

  if (ld->isA(Type::Arr)) {
    arr = gen(NewLikeArray, ld, cns(capacity));
  } else {
    capacity = (capacity ? capacity : MixedArray::SmallSize);
    arr = gen(NewArray, cns(capacity));
  }
  push(arr);
}

void HhbcTranslator::emitNewPackedArray(int numArgs) {
  // The NewPackedArray opcode's helper needs array values passed to it
  // via the stack.  We use spillStack() to flush the eval stack and
  // obtain a pointer to the topmost item; if over-flushing becomes
  // a problem then we should refactor the NewPackedArray opcode to
  // take its values directly as SSA operands.
  //
  // Before the spillStack() we touch all of the incoming stack
  // arguments so that they are available to later optimizations via
  // getStackValue().
  for (int i = 0; i < numArgs; i++) topC(i, DataTypeGeneric);
  SSATmp* sp = spillStack();
  for (int i = 0; i < numArgs; i++) popC(DataTypeGeneric);
  push(gen(NewPackedArray, cns(numArgs), sp));
}

void HhbcTranslator::emitNewStructArray(uint32_t numArgs, StringData** keys) {
  // The NewPackedArray opcode's helper needs array values passed to it
  // via the stack.  We use spillStack() to flush the eval stack and
  // obtain a pointer to the topmost item; if over-flushing becomes
  // a problem then we should refactor the NewPackedArray opcode to
  // take its values directly as SSA operands.
  SSATmp* sp = spillStack();
  for (int i = 0; i < numArgs; i++) popC(DataTypeGeneric);
  NewStructData extra;
  extra.numKeys = numArgs;
  extra.keys = new (m_unit.arena()) StringData*[numArgs];
  memcpy(extra.keys, keys, numArgs * sizeof(*keys));
  push(gen(NewStructArray, extra, sp));
}

void HhbcTranslator::emitArrayAdd() {
  if (!topC(0)->isA(Type::Arr) || !topC(1)->isA(Type::Arr)) {
    // This happens when we have a prior spillstack that optimizes away
    // its spilled values because they were already on the stack. This
    // prevents us from getting to type of the SSATmps popped from the
    // eval stack. Most likely we had an interpone before this
    // instruction.
    emitInterpOne(Type::Arr, 2);
    return;
  }

  auto catchBlock = makeCatch();
  SSATmp* tr = popC();
  SSATmp* tl = popC();
  // The ArrayAdd helper decrefs its args, so don't decref pop'ed values.
  push(gen(ArrayAdd, catchBlock, tl, tr));
}

void HhbcTranslator::emitAddElemC() {
  // This is just to peek at the type; it'll be consumed for real down below and
  // we don't want to constrain it if we're just going to InterpOne.
  auto kt = topC(1, DataTypeGeneric)->type();
  Opcode op;
  if (kt <= Type::Int) {
    op = AddElemIntKey;
  } else if (kt <= Type::Str) {
    op = AddElemStrKey;
  } else {
    emitInterpOne(Type::Arr, 3);
    return;
  }

  // val is teleported from the stack to the array, so we don't have to do any
  // refcounting.
  auto const catchBlock = makeCatch();
  auto const val = popC(DataTypeGeneric);
  auto const key = popC();
  auto const arr = popC();
  // The AddElem* instructions decref their args, so don't decref pop'ed
  // values.
  push(gen(op, catchBlock, arr, key, val));
}

void HhbcTranslator::emitAddNewElemC() {
  if (!topC(1)->isA(Type::Arr)) {
    return emitInterpOne(Type::Arr, 2);
  }

  auto const catchBlock = makeCatch();
  auto const val = popC();
  auto const arr = popC();
  // The AddNewElem helper decrefs its args, so don't decref pop'ed values.
  push(gen(AddNewElem, catchBlock, arr, val));
}

void HhbcTranslator::emitNewCol(int type, int size) {
  push(gen(NewCol, cns(type), cns(size)));
}

void HhbcTranslator::emitClone() {
  if (!topC()->isA(Type::Obj)) PUNT(Clone-NonObj);
  auto const catchTrace = makeCatch();
  auto const obj        = popC();
  push(gen(Clone, catchTrace, obj));
  gen(DecRef, obj);
}

void HhbcTranslator::emitColAddElemC() {
  if (!topC(2)->isA(Type::Obj)) {
    return emitInterpOne(Type::Obj, 3);
  }
  if (!topC(1, DataTypeGeneric)->type().subtypeOfAny(Type::Int, Type::Str)) {
    emitInterpOne(Type::Obj, 3);
    return;
  }

  auto* catchBlock = makeCatch();
  auto const val = popC();
  auto const key = popC();
  auto const coll = popC();
  push(gen(ColAddElemC, catchBlock, coll, key, val));
  gen(DecRef, key);
}

void HhbcTranslator::emitColAddNewElemC() {
  if (!topC(1)->isA(Type::Obj)) {
    return emitInterpOne(Type::Obj, 2);
  }

  auto* catchBlock = makeCatch();
  auto const val = popC();
  auto const coll = popC();
  // The AddNewElem helper decrefs its args, so don't decref pop'ed values.
  push(gen(ColAddNewElemC, catchBlock, coll, val));
}

void HhbcTranslator::emitCnsCommon(uint32_t id,
                                   uint32_t fallback,
                                   bool error) {
  assert(fallback == kInvalidId || !error);
  StringData* name = curUnit()->lookupLitstrId(id);
  SSATmp* cnsNameTmp = cns(name);
  const TypedValue* tv = Unit::lookupPersistentCns(name);
  SSATmp* result = nullptr;

  SSATmp* fallbackNameTmp = nullptr;
  if (fallback != kInvalidId) {
    StringData* fallbackName = curUnit()->lookupLitstrId(fallback);
    fallbackNameTmp = cns(fallbackName);
  }
  if (tv) {
    if (tv->m_type == KindOfUninit) {
      // KindOfUninit is a dynamic system constant. always a slow
      // lookup.
      assert(!fallbackNameTmp);
      if (error) {
        result = gen(LookupCnsE, makeCatch(), cnsNameTmp);
      } else {
        result = gen(LookupCns, makeCatch(), cnsNameTmp);
      }
    } else {
      result = staticTVCns(tv);
    }
  } else {
    SSATmp* c1 = gen(LdCns, cnsNameTmp);
    result = m_irb->cond(
      1,
      [&] (Block* taken) { // branch
        gen(CheckInit, taken, c1);
      },
      [&] { // Next: LdCns hit in TC
        return c1;
      },
      [&] { // Taken: miss in TC, do lookup & init
        m_irb->hint(Block::Hint::Unlikely);
        // We know that c1 is Uninit in this branch but we have to encode this
        // in the IR.
        gen(AssertType, Type::Uninit, c1);

        if (fallbackNameTmp) {
          return gen(LookupCnsU, makeCatch(),
                     cnsNameTmp, fallbackNameTmp);
        }
        if (error) {
          return gen(LookupCnsE, makeCatch(), cnsNameTmp);
        }
        return gen(LookupCns, makeCatch(), cnsNameTmp);
      });
  }
  push(result);
}

void HhbcTranslator::emitCns(uint32_t id) {
  emitCnsCommon(id, kInvalidId, false);
}

void HhbcTranslator::emitCnsE(uint32_t id) {
  emitCnsCommon(id, kInvalidId, true);
}

void HhbcTranslator::emitCnsU(uint32_t id, uint32_t fallbackId) {
  emitCnsCommon(id, fallbackId, false);
}

void HhbcTranslator::emitDefCns(uint32_t id) {
  emitInterpOne(Type::Bool, 1);
}

void HhbcTranslator::emitConcat() {
  auto const catchBlock = makeCatch();
  SSATmp* tr = popC();
  SSATmp* tl = popC();
  // Concat consumes only first ref, never second
  push(gen(ConcatCellCell, catchBlock, tl, tr));
  // so we need to consume second ref ourselves
  gen(DecRef, tr);
}

void HhbcTranslator::emitConcatN(int n) {
  if (n == 2) return emitConcat();

  auto const catchBlock = makeCatch();

  SSATmp* t1 = popC();
  SSATmp* t2 = popC();
  SSATmp* t3 = popC();

  if (!t1->isA(Type::Str) ||
      !t2->isA(Type::Str) ||
      !t3->isA(Type::Str)) {
    PUNT(ConcatN);
  }

  if (n == 3) {
    push(gen(ConcatStr3, catchBlock, t3, t2, t1));
    gen(DecRef, t2);
    gen(DecRef, t1);

  } else if (n == 4) {
    SSATmp* t4 = popC();
    if (!t4->isA(Type::Str)) PUNT(ConcatN);

    push(gen(ConcatStr4, catchBlock, t4, t3, t2, t1));
    gen(DecRef, t3);
    gen(DecRef, t2);
    gen(DecRef, t1);

  } else {
    not_reached();
  }
}

void HhbcTranslator::emitDefCls(int cid, Offset after) {
  emitInterpOne(0);
}

void HhbcTranslator::emitDefFunc(int fid) {
  emitInterpOne(0);
}

void HhbcTranslator::emitLateBoundCls() {
  Class* clss = curClass();
  if (!clss) {
    // no static context class, so this will raise an error
    emitInterpOne(Type::Cls, 0);
    return;
  }
  auto const ctx = gen(LdCtx, FuncData(curFunc()), m_irb->fp());
  push(gen(LdClsCtx, ctx));
}

void HhbcTranslator::emitSelf() {
  Class* clss = curClass();
  if (clss == nullptr) {
    emitInterpOne(Type::Cls, 0);
  } else {
    push(cns(clss));
  }
}

void HhbcTranslator::emitParent() {
  auto const clss = curClass();
  if (clss == nullptr || clss->parent() == nullptr) {
    emitInterpOne(Type::Cls, 0);
  } else {
    push(cns(clss->parent()));
  }
}

void HhbcTranslator::emitString(int strId) {
  push(cns(lookupStringId(strId)));
}

void HhbcTranslator::emitInt(int64_t val) {
  push(cns(val));
}

void HhbcTranslator::emitDouble(double val) {
  push(cns(val));
}

void HhbcTranslator::emitNullUninit() {
  push(cns(Type::Uninit));
}

void HhbcTranslator::emitNull() {
  push(cns(Type::InitNull));
}

void HhbcTranslator::emitTrue() {
  push(cns(true));
}

void HhbcTranslator::emitFalse() {
  push(cns(false));
}

void HhbcTranslator::emitDir() {
  push(cns(curUnit()->dirpath()));
}

void HhbcTranslator::emitFile() {
  push(cns(curUnit()->filepath()));
}

void HhbcTranslator::emitInitThisLoc(int32_t id) {
  if (!curClass()) {
    // Do nothing if this is null
    return;
  }
  auto const ldrefExit = makeExit();
  auto const tmpThis = gen(LdThis, makeExitSlow(), m_irb->fp());
  gen(IncRef, tmpThis);
  auto const oldLoc = ldLoc(id, ldrefExit, DataTypeCountness);
  genStLocal(id, m_irb->fp(), tmpThis);
  gen(DecRef, oldLoc);
}

void HhbcTranslator::emitCGetL(int32_t id) {
  auto ldrefExit = makeExit();
  auto ldgblExit = makePseudoMainExit();
  // Mimic hhbc guard relaxation for now.
  auto cat = curSrcKey().op() == OpFPassL ? DataTypeSpecific
                                          : DataTypeCountnessInit;
  pushIncRef(ldLocInnerWarn(id, ldrefExit, ldgblExit, cat));
}

void HhbcTranslator::emitFPassL(int32_t id) {
  auto ldrefExit = makeExit();
  auto ldgblExit = makePseudoMainExit();
  pushIncRef(ldLocInnerWarn(id, ldrefExit, ldgblExit, DataTypeSpecific));
}

void HhbcTranslator::emitPushL(uint32_t id) {
  assertTypeLocal(id, Type::InitCell);
  auto* locVal = ldLoc(id, makeExit(), DataTypeGeneric);
  push(locVal);
  genStLocal(id, m_irb->fp(), cns(Type::Uninit));
}

void HhbcTranslator::emitCGetL2(int32_t id) {
  auto ldrefExit = makeExit();
  auto ldgblExit = makePseudoMainExit();
  auto catchBlock = makeCatch();
  SSATmp* oldTop = pop(Type::StackElem);
  auto val = ldLocInnerWarn(
    id,
    ldrefExit,
    ldgblExit,
    DataTypeCountnessInit,
    catchBlock
  );
  pushIncRef(val);
  push(oldTop);
}

void HhbcTranslator::emitVGetL(int32_t id) {
  auto value = ldLoc(id, makeExit(), DataTypeCountnessInit);
  auto const t = value->type();
  always_assert(t.isBoxed() || t.notBoxed());

  if (t.notBoxed()) {
    if (value->isA(Type::Uninit)) {
      value = cns(Type::InitNull);
    }
    value = gen(Box, value);
    genStLocal(id, m_irb->fp(), value);
  }
  pushIncRef(value);
}

void HhbcTranslator::emitUnsetL(int32_t id) {
  auto const prev = ldLoc(id, makeExit(), DataTypeCountness);
  genStLocal(id, m_irb->fp(), cns(Type::Uninit));
  gen(DecRef, prev);
}

void HhbcTranslator::emitBindL(int32_t id) {
  if (inPseudoMain()) {
    emitInterpOne(Type::BoxedCell, 1);
    return;
  }

  auto const ldgblExit = makePseudoMainExit();
  auto const newValue = popV();
  // Note that the IncRef must happen first, for correctness in a
  // pseudo-main: the destructor could decref the value again after
  // we've stored it into the local.
  pushIncRef(newValue);
  auto const oldValue = ldLoc(id, ldgblExit, DataTypeSpecific);
  genStLocal(id, m_irb->fp(), newValue);
  gen(DecRef, oldValue);
}

void HhbcTranslator::emitSetL(int32_t id) {
  auto const ldrefExit = makeExit();
  auto const ldgblExit = makePseudoMainExit();

  // since we're just storing the value in a local, this function doesn't care
  // about the type of the value. stLoc needs to IncRef the value so it may
  // constrain it further.
  auto const src = popC(DataTypeGeneric);
  pushStLoc(id, ldrefExit, ldgblExit, src);
}

void HhbcTranslator::emitIncDecL(bool pre, bool inc, bool over, uint32_t id) {
  auto const ldrefExit = makeExit();
  auto const ldgblExit = makePseudoMainExit();
  auto const src = ldLocInnerWarn(
    id,
    ldrefExit,
    ldgblExit,
    DataTypeSpecific
  );

  if (src->isA(Type::Bool)) {
    push(src);
    return;
  }

  if (src->type().subtypeOfAny(Type::Arr, Type::Obj)) {
    pushIncRef(src);
    return;
  }

  if (src->isA(Type::Null)) {
    push(inc && pre ? cns(1) : src);
    if (inc) {
      stLoc(id, ldrefExit, ldgblExit, cns(1));
    }
    return;
  }

  if (!src->type().subtypeOfAny(Type::Int, Type::Dbl)) {
    PUNT(IncDecL);
  }

  auto const res = emitIncDec(pre, inc, over, src);
  stLoc(id, ldrefExit, ldgblExit, res);
}

// only handles integer or double inc/dec
SSATmp* HhbcTranslator::emitIncDec(bool pre, bool inc, bool over, SSATmp* src) {
  assert(src->isA(Type::Int) || src->isA(Type::Dbl));

  Opcode op;

  if (src->isA(Type::Dbl)) {
    op = inc ? AddDbl : SubDbl;
  } else if (!over) {
    op = inc ? AddInt : SubInt;
  } else {
    op = inc ? AddIntO : SubIntO;
  }

  SSATmp* one = src->isA(Type::Int) ? cns(1) : cns(1.0);
  SSATmp* res = nullptr;

  if (op == AddIntO || op == SubIntO) {
    auto spills = peekSpillValues();
    auto const exit = makeExitImpl(
      bcOff(),
      ExitFlag::Interp,
      spills,
      CustomExit{}
    );
    res = gen(op, exit, src, one);
  } else {
    res = gen(op, src, one);
  }

  // no incref necessary on push since result is an int
  push(pre ? res : src);
  return res;
}

#define BINARY_ARITH          \
  AOP(Add, AddInt, AddDbl)    \
  AOP(Sub, SubInt, SubDbl)    \
  AOP(Mul, MulInt, MulDbl)    \
  AOP(AddO, AddIntO, AddDbl)  \
  AOP(SubO, SubIntO, SubDbl)  \
  AOP(MulO, MulIntO, MulDbl)  \

#define BINARY_BITOP  \
  BOP(BitAnd, AndInt) \
  BOP(BitOr,  OrInt)  \
  BOP(BitXor, XorInt) \

static bool areBinaryArithTypesSupported(Op op, Type t1, Type t2) {
  auto checkArith = [](Type ty) {
    return ty.subtypeOfAny(Type::Int, Type::Bool, Type::Dbl);
  };
  auto checkBitOp = [](Type ty) {
    return ty.subtypeOfAny(Type::Int, Type::Bool);
  };

  switch (op) {
  #define AOP(OP, OPI, OPD) \
    case Op::OP: return checkArith(t1) && checkArith(t2);
  BINARY_ARITH
  #undef AOP
  #define BOP(OP, OPI) \
    case Op::OP: return checkBitOp(t1) && checkBitOp(t2);
  BINARY_BITOP
  #undef BOP
  default: not_reached();
  }
}

Opcode intArithOp(Op op) {
  switch (op) {
    #define AOP(OP, OPI, OPD) case Op::OP: return OPI;
    BINARY_ARITH
    #undef AOP
    default: not_reached();
  }
}

Opcode dblArithOp(Op op) {
  switch (op) {
    #define AOP(OP, OPI, OPD) case Op::OP: return OPD;
    BINARY_ARITH
    #undef AOP
    default: not_reached();
  }
}

Opcode bitOp(Op op) {
  switch (op) {
    #define BOP(OP, OPI) case Op::OP: return OPI;
    BINARY_BITOP
    #undef BOP
    default: not_reached();
  }
}

bool isBitOp(Op op) {
  switch (op) {
    #define BOP(OP, OPI) case Op::OP: return true;
    BINARY_BITOP
    #undef BOP
    default: return false;
  }
}

SSATmp* HhbcTranslator::promoteBool(SSATmp* src) {
  // booleans in arithmetic and bitwise operations get cast to ints
  return src->isA(Type::Bool) ? gen(ConvBoolToInt, src) : src;
}

Opcode HhbcTranslator::promoteBinaryDoubles(Op op,
                                            SSATmp*& src1,
                                            SSATmp*& src2) {
  auto type1 = src1->type();
  auto type2 = src2->type();

  Opcode opc = intArithOp(op);
  if (type1 <= Type::Dbl) {
    opc = dblArithOp(op);
    if (type2 <= Type::Int) {
      src2 = gen(ConvIntToDbl, src2);
    }
  } else if (type2 <= Type::Dbl) {
    opc = dblArithOp(op);
    src1 = gen(ConvIntToDbl, src1);
  }
  return opc;
}

void HhbcTranslator::emitSetOpL(Op subOp, uint32_t id) {
  // Needs to modify locals after doing effectful operations like
  // ConcatCellCell, so we can't guard on their types.
  if (inPseudoMain()) PUNT(SetOpL-PseudoMain);

  // Null guard block for globals because we always punt on pseudomains
  auto const ldgblExit = nullptr;

  /*
   * Handle array addition first because we don't want to bother with
   * boxed locals.
   */
  bool isAdd = (subOp == Op::Add || subOp == Op::AddO);
  if (isAdd && (m_irb->localType(id, DataTypeSpecific) <= Type::Arr) &&
      topC()->isA(Type::Arr)) {
    /*
     * ArrayAdd decrefs its sources and returns a new array with
     * refcount == 1. That covers the local, so incref once more for
     * the stack.
     */
    auto const catchBlock = makeCatch();
    auto const loc    = ldLoc(id, ldgblExit, DataTypeSpecific);
    auto const val    = popC();
    auto const result = gen(ArrayAdd, catchBlock, loc, val);
    genStLocal(id, m_irb->fp(), result);
    pushIncRef(result);
    return;
  }

  auto const ldrefExit = makeExit();
  auto loc = ldLocInnerWarn(id, ldrefExit, ldgblExit, DataTypeGeneric);

  if (subOp == Op::Concat) {
    /*
     * The concat helpers incref their results, which will be consumed by
     * the stloc. We need an extra incref for the push onto the stack.
     */
    auto const catchBlock = makeCatch();
    auto const val    = popC();
    m_irb->constrainValue(loc, DataTypeSpecific);
    auto const result = gen(ConcatCellCell, catchBlock, loc, val);

    // Null exit block for 'ldrefExit' because this is a local that we've
    // already guarded against in the upper ldLocInnerWarn, and we can't run
    // any guards since ConcatCellCell can have effects.
    pushIncRef(stLocNRC(id, nullptr, ldgblExit, result));

    // ConcatCellCell does not DecRef its second argument,
    // so we need to do it here
    gen(DecRef, val);
    return;
  }

  if (areBinaryArithTypesSupported(subOp, loc->type(), topC()->type())) {
    auto val = popC();
    m_irb->constrainValue(loc, DataTypeSpecific);
    loc = promoteBool(loc);
    val = promoteBool(val);
    Opcode opc;
    if (isBitOp(subOp)) {
      opc = bitOp(subOp);
    } else {
      opc = promoteBinaryDoubles(subOp, loc, val);
    }

    SSATmp* result = nullptr;
    if (opc == AddIntO || opc == SubIntO || opc == MulIntO) {
      auto spillValues = peekSpillValues();
      spillValues.push_back(val);
      auto const exit = makeExitImpl(
        bcOff(),
        ExitFlag::Interp,
        spillValues,
        CustomExit{}
      );
      result = gen(opc, exit, loc, val);
    } else {
      result = gen(opc, loc, val);
    }
    pushStLoc(id, ldrefExit, ldgblExit, result);
    return;
  }

  PUNT(SetOpL);
}

void HhbcTranslator::emitOODeclExists(unsigned char ucsubop) {
  auto const subop = static_cast<OODeclExistsOp>(ucsubop);
  auto const catchTrace = makeCatch();

  auto const tAutoload = popC();
  auto const tCls = popC();

  assert(tCls->isA(Type::Str)); // result of CastString
  assert(tAutoload->isA(Type::Bool)); // result of CastBool

  ClassKind kind;
  switch (subop) {
    case OODeclExistsOp::Class : kind = ClassKind::Class; break;
    case OODeclExistsOp::Trait : kind = ClassKind::Trait; break;
    case OODeclExistsOp::Interface : kind = ClassKind::Interface; break;
  }

  push(gen(OODeclExists, catchTrace, ClassKindData { kind }, tCls, tAutoload));
  gen(DecRef, tCls);
}

void HhbcTranslator::emitStaticLocInit(uint32_t locId, uint32_t litStrId) {
  if (inPseudoMain()) PUNT(StaticLocInit);

  auto const ldgblExit = makePseudoMainExit();
  auto const name  = lookupStringId(litStrId);
  auto const value = popC();

  // Closures and generators from closures don't satisfy the "one static per
  // source location" rule that the inline fastpath requires
  auto const box = [&]{
    if (curFunc()->isClosureBody()) {
      return gen(ClosureStaticLocInit, cns(name), m_irb->fp(), value);
    }

    auto const cachedBox =
      gen(LdStaticLocCached, StaticLocName { curFunc(), name });
    m_irb->ifThen(
      [&] (Block* taken) {
        gen(CheckStaticLocInit, taken, cachedBox);
      },
      [&] {
        m_irb->hint(Block::Hint::Unlikely);
        gen(StaticLocInitCached, cachedBox, value);
      }
    );
    return cachedBox;
  }();
  gen(IncRef, box);
  auto const oldValue = ldLoc(locId, ldgblExit, DataTypeSpecific);
  genStLocal(locId, m_irb->fp(), box);
  gen(DecRef, oldValue);
  // We don't need to decref value---it's a bytecode invariant that
  // our Cell was not ref-counted.
}

void HhbcTranslator::emitStaticLoc(uint32_t locId, uint32_t litStrId) {
  if (inPseudoMain()) PUNT(StaticLoc);

  auto const ldgblExit = makePseudoMainExit();
  auto const name = lookupStringId(litStrId);

  auto const box = curFunc()->isClosureBody() ?
    gen(ClosureStaticLocInit, cns(name), m_irb->fp(), cns(Type::Uninit)) :
    gen(LdStaticLocCached, StaticLocName { curFunc(), name });

  auto const res = m_irb->cond(
    0,
    [&] (Block* taken) {
      gen(CheckStaticLocInit, taken, box);
    },
    [&] { // Next: the static local is already initialized
      return cns(true);
    },
    [&] { // Taken: need to initialize the static local
      /*
       * Even though this path is "cold", we're not marking it
       * unlikely because the size of the instructions this will
       * generate is about 10 bytes, which is not much larger than the
       * 5 byte jump to acold would be.
       *
       * One note about StaticLoc: we're literally always going to
       * generate a fallthrough trace here that is cold (the code that
       * initializes the static local).  TODO(#2894612).
       */
      gen(StaticLocInitCached, box, cns(Type::InitNull));
      return cns(false);
    });
  gen(IncRef, box);
  auto const oldValue = ldLoc(locId, ldgblExit, DataTypeGeneric);
  genStLocal(locId, m_irb->fp(), box);
  gen(DecRef, oldValue);
  push(res);
}

template<class Lambda>
void HhbcTranslator::emitIterInitCommon(int offset, JmpFlags jmpFlags,
                                           Lambda genFunc,
                                           bool invertCond) {
  auto const src = popC();
  auto const type = src->type();
  if (!type.subtypeOfAny(Type::Arr, Type::Obj)) PUNT(IterInit);
  auto const res = genFunc(src);
  emitJmpCondHelper(offset, !invertCond, jmpFlags, res);
}

template<class Lambda>
void HhbcTranslator::emitMIterInitCommon(int offset, JmpFlags jmpFlags,
                                            Lambda genFunc) {
  auto exit = makeExit();

  SSATmp* src = topV();
  Type type = src->type();

  assert(type.isBoxed());
  m_irb->constrainValue(gen(LdRef, type.innerType(), exit, src),
                        DataTypeSpecific);
  SSATmp* res = genFunc(src);
  SSATmp* out = popV();
  gen(DecRef, out);
  emitJmpCondHelper(offset, true, jmpFlags, res);
}

void HhbcTranslator::emitIterInit(uint32_t iterId,
                                  int offset,
                                  uint32_t valLocalId,
                                  bool invertCond,
                                  JmpFlags jmpFlags) {
  auto catchBlock = makeCatch();
  emitIterInitCommon(offset, jmpFlags, [&] (SSATmp* src) {
      return gen(IterInit,
                 Type::Bool,
                 catchBlock,
                 IterData(iterId, -1, valLocalId),
                 src,
                 m_irb->fp());
    },
    invertCond);
}

void HhbcTranslator::emitIterInitK(uint32_t iterId,
                                   int offset,
                                   uint32_t valLocalId,
                                   uint32_t keyLocalId,
                                   bool invertCond,
                                   JmpFlags jmpFlags) {
  auto catchBlock = makeCatch();
  emitIterInitCommon(offset, jmpFlags, [&] (SSATmp* src) {
      return gen(IterInitK,
                 Type::Bool,
                 catchBlock,
                 IterData(iterId, keyLocalId, valLocalId),
                 src,
                 m_irb->fp());
    },
    invertCond);
}

void HhbcTranslator::emitIterNext(uint32_t iterId,
                                  int offset,
                                  uint32_t valLocalId,
                                  bool invertCond,
                                  JmpFlags jmpFlags) {
  SSATmp* res = gen(
    IterNext,
    Type::Bool,
    makeCatch(),
    IterData(iterId, -1, valLocalId),
    m_irb->fp()
  );
  emitJmpCondHelper(offset, invertCond, jmpFlags, res);
}

void HhbcTranslator::emitIterNextK(uint32_t iterId,
                                   int offset,
                                   uint32_t valLocalId,
                                   uint32_t keyLocalId,
                                   bool invertCond,
                                   JmpFlags jmpFlags) {
  SSATmp* res = gen(
    IterNextK,
    Type::Bool,
    makeCatch(),
    IterData(iterId, keyLocalId, valLocalId),
    m_irb->fp()
  );
  emitJmpCondHelper(offset, invertCond, jmpFlags, res);
}

void HhbcTranslator::emitWIterInit(uint32_t iterId,
                                   int offset,
                                   uint32_t valLocalId,
                                   bool invertCond,
                                   JmpFlags jmpFlags) {
  auto catchBlock = makeCatch();
  emitIterInitCommon(
    offset, jmpFlags, [&] (SSATmp* src) {
      return gen(WIterInit,
                 Type::Bool,
                 catchBlock,
                 IterData(iterId, -1, valLocalId),
                 src,
                 m_irb->fp());
    },
    invertCond);
}

void HhbcTranslator::emitWIterInitK(uint32_t iterId,
                                    int offset,
                                    uint32_t valLocalId,
                                    uint32_t keyLocalId,
                                    bool invertCond,
                                    JmpFlags jmpFlags) {
  auto catchBlock = makeCatch();
  emitIterInitCommon(
    offset, jmpFlags, [&] (SSATmp* src) {
      return gen(WIterInitK,
                 Type::Bool,
                 catchBlock,
                 IterData(iterId, keyLocalId, valLocalId),
                 src,
                 m_irb->fp());
    },
    invertCond);
}

void HhbcTranslator::emitWIterNext(uint32_t iterId,
                                   int offset,
                                   uint32_t valLocalId,
                                   bool invertCond,
                                   JmpFlags jmpFlags) {
  SSATmp* res = gen(
    WIterNext,
    Type::Bool,
    makeCatch(),
    IterData(iterId, -1, valLocalId),
    m_irb->fp()
  );
  emitJmpCondHelper(offset, invertCond, jmpFlags, res);
}

void HhbcTranslator::emitWIterNextK(uint32_t iterId,
                                    int offset,
                                    uint32_t valLocalId,
                                    uint32_t keyLocalId,
                                    bool invertCond,
                                    JmpFlags jmpFlags) {
  SSATmp* res = gen(
    WIterNextK,
    Type::Bool,
    makeCatch(),
    IterData(iterId, keyLocalId, valLocalId),
    m_irb->fp()
  );
  emitJmpCondHelper(offset, invertCond, jmpFlags, res);
}

void HhbcTranslator::emitMIterInit(uint32_t iterId,
                                   int offset,
                                   uint32_t valLocalId,
                                   JmpFlags jmpFlags) {
  auto catchBlock = makeCatch();
  emitMIterInitCommon(offset, jmpFlags, [&] (SSATmp* src) {
    return gen(
      MIterInit,
      Type::Bool,
      catchBlock,
      IterData(iterId, -1, valLocalId),
      src,
      m_irb->fp()
    );
  });
}

void HhbcTranslator::emitMIterInitK(uint32_t iterId,
                                    int offset,
                                    uint32_t valLocalId,
                                    uint32_t keyLocalId,
                                    JmpFlags jmpFlags) {
  auto catchBlock = makeCatch();
  emitMIterInitCommon(offset, jmpFlags, [&] (SSATmp* src) {
    return gen(
      MIterInitK,
      Type::Bool,
      catchBlock,
      IterData(iterId, keyLocalId, valLocalId),
      src,
      m_irb->fp()
    );
  });
}

void HhbcTranslator::emitMIterNext(uint32_t iterId,
                                   int offset,
                                   uint32_t valLocalId,
                                   JmpFlags jmpFlags) {
  SSATmp* res = gen(
    MIterNext,
    Type::Bool,
    IterData(iterId, -1, valLocalId),
    m_irb->fp()
  );
  emitJmpCondHelper(offset, false, jmpFlags, res);
}

void HhbcTranslator::emitMIterNextK(uint32_t iterId,
                                    int offset,
                                    uint32_t valLocalId,
                                    uint32_t keyLocalId,
                                    JmpFlags jmpFlags) {
  SSATmp* res = gen(
    MIterNextK,
    Type::Bool,
    IterData(iterId, keyLocalId, valLocalId),
    m_irb->fp()
  );
  emitJmpCondHelper(offset, false, jmpFlags, res);
}

void HhbcTranslator::emitIterFree(uint32_t iterId) {
  gen(IterFree, IterId(iterId), m_irb->fp());
}

void HhbcTranslator::emitMIterFree(uint32_t iterId) {
  gen(MIterFree, IterId(iterId), m_irb->fp());
}

void HhbcTranslator::emitDecodeCufIter(uint32_t iterId, int offset,
                                       JmpFlags jmpFlags) {
  auto catchBlock = makeCatch();
  SSATmp* src = popC();
  Type type = src->type();
  if (type.subtypeOfAny(Type::Arr, Type::Str, Type::Obj)) {
    SSATmp* res = gen(DecodeCufIter, Type::Bool,
                      IterId(iterId), catchBlock, src, m_irb->fp());
    gen(DecRef, src);
    emitJmpCondHelper(offset, true, jmpFlags, res);
  } else {
    gen(DecRef, src);
    emitJmpImpl(offset, JmpFlagEndsRegion, nullptr);
  }
}

void HhbcTranslator::emitCIterFree(uint32_t iterId) {
  gen(CIterFree, IterId(iterId), m_irb->fp());
}

void HhbcTranslator::emitIterBreak(const ImmVector& iv,
                                   uint32_t offset,
                                   bool endsRegion) {
  int iterIndex;
  for (iterIndex = 0; iterIndex < iv.size(); iterIndex += 2) {
    IterKind iterKind = (IterKind)iv.vec32()[iterIndex];
    Id       iterId   = iv.vec32()[iterIndex + 1];
    switch (iterKind) {
      case KindOfIter:  gen(IterFree,  IterId(iterId), m_irb->fp()); break;
      case KindOfMIter: gen(MIterFree, IterId(iterId), m_irb->fp()); break;
      case KindOfCIter: gen(CIterFree, IterId(iterId), m_irb->fp()); break;
    }
  }

  if (!endsRegion) return;
  gen(Jmp, makeExit(offset));
}

void HhbcTranslator::emitCreateCont(Offset resumeOffset) {
  assert(!resumed());
  assert(curFunc()->isGenerator());

  if (curFunc()->isAsyncGenerator()) PUNT(CreateCont-AsyncGenerator);

  // Create the Generator object. CreateCont takes care of copying local
  // variables and iterators.
  auto const func = curFunc();
  auto const resumeSk = SrcKey(func, resumeOffset, true);
  auto const resumeAddr = gen(LdBindAddr, LdBindAddrData(resumeSk));
  auto const cont = gen(CreateCont, m_irb->fp(), cns(func->numSlotsInFrame()),
                        resumeAddr, cns(resumeOffset));

  // Teleport local variables into the generator.
  SSATmp* contAR = gen(LdContActRec, Type::PtrToGen, cont);

  // Call the FunctionSuspend hook and put the return value on the stack so that
  // the unwinder would decref it.
  emitRetSurpriseCheck(contAR, nullptr, makeCatch({cont}), false);

  // Grab caller info from ActRec, free ActRec, store the return value
  // and return control to the caller.
  gen(StRetVal, m_irb->fp(), cont);
  SSATmp* retAddr = gen(LdRetAddr, m_irb->fp());
  SSATmp* sp = gen(RetAdjustStack, m_irb->fp());
  SSATmp* fp = gen(FreeActRec, m_irb->fp());
  gen(RetCtrl, RetCtrlData(false), sp, fp, retAddr);
}

void HhbcTranslator::emitContEnter(Offset returnOffset) {
  assert(curClass());
  assert(curClass()->classof(c_AsyncGenerator::classof()) ||
         curClass()->classof(c_Generator::classof()));
  assert(curFunc()->contains(returnOffset));

  // Load generator's FP and resume address.
  auto genObj = gen(LdThis, m_irb->fp());
  auto genFp = gen(LdContActRec, Type::FramePtr, genObj);
  auto resumeAddr =
    gen(LdContArRaw, RawMemData{RawMemData::ContResumeAddr}, genFp);

  // Make sure function enter hook is called if needed.
  auto exitSlow = makeExitSlow();
  gen(CheckSurpriseFlags, exitSlow);

  // Exit to interpreter if resume address is not known.
  resumeAddr = gen(CheckNonNull, exitSlow, resumeAddr);

  // Sync stack.
  auto const sp = spillStack();

  // Enter generator.
  auto returnBcOffset = returnOffset - curFunc()->base();
  gen(ContEnter, sp, m_irb->fp(), genFp, resumeAddr, cns(returnBcOffset));
}

void HhbcTranslator::emitYieldReturnControl(Block* catchBlock) {
  // Push return value of next()/send()/raise().
  push(cns(Type::InitNull));

  auto const sp = spillStack();
  emitRetSurpriseCheck(m_irb->fp(), nullptr, catchBlock, true);

  auto const retAddr = gen(LdRetAddr, m_irb->fp());
  auto const fp = gen(FreeActRec, m_irb->fp());

  gen(RetCtrl, RetCtrlData(true), sp, fp, retAddr);
}

void HhbcTranslator::emitYieldImpl(Offset resumeOffset) {
  // Resumable::setResumeAddr(resumeAddr, resumeOffset)
  auto const resumeSk = SrcKey(curFunc(), resumeOffset, true);
  auto const resumeAddr = gen(LdBindAddr, LdBindAddrData(resumeSk));
  gen(StContArRaw, RawMemData{RawMemData::ContResumeAddr}, m_irb->fp(),
      resumeAddr);
  gen(StContArRaw, RawMemData{RawMemData::ContResumeOffset}, m_irb->fp(),
      cns(resumeOffset));

  // Set yielded value.
  auto const oldValue = gen(LdContArValue, Type::Cell, m_irb->fp());
  gen(StContArValue, m_irb->fp(), popC(DataTypeGeneric)); // teleporting value
  gen(DecRef, oldValue);

  // Set state from Running to Started.
  gen(StContArRaw, RawMemData{RawMemData::ContState}, m_irb->fp(),
      cns(BaseGenerator::State::Started));
}

void HhbcTranslator::emitYield(Offset resumeOffset) {
  assert(resumed());
  assert(curFunc()->isGenerator());

  if (curFunc()->isAsyncGenerator()) PUNT(Yield-AsyncGenerator);

  auto catchBlock = makeCatchNoSpill();
  emitYieldImpl(resumeOffset);

  // take a fast path if this generator has no yield k => v;
  if (curFunc()->isPairGenerator()) {
    // this needs optimization
    auto const idx =
      gen(LdContArRaw, RawMemData{RawMemData::ContIndex}, m_irb->fp());
    auto const newIdx = gen(AddInt, idx, cns(1));
    gen(StContArRaw, RawMemData{RawMemData::ContIndex}, m_irb->fp(), newIdx);

    auto const oldKey = gen(LdContArKey, Type::Cell, m_irb->fp());
    gen(StContArKey, m_irb->fp(), newIdx);
    gen(DecRef, oldKey);
  } else {
    // we're guaranteed that the key is an int
    gen(ContArIncKey, m_irb->fp());
  }

  // transfer control
  emitYieldReturnControl(catchBlock);
}

void HhbcTranslator::emitYieldK(Offset resumeOffset) {
  assert(resumed());
  assert(curFunc()->isGenerator());

  if (curFunc()->isAsyncGenerator()) PUNT(YieldK-AsyncGenerator);

  auto catchBlock = makeCatchNoSpill();
  emitYieldImpl(resumeOffset);

  auto const newKey = popC();
  auto const oldKey = gen(LdContArKey, Type::Cell, m_irb->fp());
  gen(StContArKey, m_irb->fp(), newKey);
  gen(DecRef, oldKey);

  auto const keyType = newKey->type();
  if (keyType <= Type::Int) {
    gen(ContArUpdateIdx, m_irb->fp(), newKey);
  }

  // transfer control
  emitYieldReturnControl(catchBlock);
}

void HhbcTranslator::emitContCheck(bool checkStarted) {
  assert(curClass());
  assert(curClass()->classof(c_AsyncGenerator::classof()) ||
         curClass()->classof(c_Generator::classof()));
  SSATmp* cont = gen(LdThis, m_irb->fp());
  gen(ContPreNext, makeExitSlow(), cont, cns(checkStarted));
}

void HhbcTranslator::emitContValid() {
  assert(curClass());
  SSATmp* cont = gen(LdThis, m_irb->fp());
  push(gen(ContValid, cont));
}

void HhbcTranslator::emitContKey() {
  assert(curClass());
  SSATmp* cont = gen(LdThis, m_irb->fp());
  gen(ContStartedCheck, makeExitSlow(), cont);
  SSATmp* offset = cns(CONTOFF(m_key));
  SSATmp* value = gen(LdProp, Type::Cell, cont, offset);
  pushIncRef(value);
}

void HhbcTranslator::emitContCurrent() {
  assert(curClass());
  SSATmp* cont = gen(LdThis, m_irb->fp());
  gen(ContStartedCheck, makeExitSlow(), cont);
  SSATmp* offset = cns(CONTOFF(m_value));
  SSATmp* value = gen(LdProp, Type::Cell, cont, offset);
  pushIncRef(value);
}

void HhbcTranslator::emitAwaitE(SSATmp* child, Block* catchBlock,
                                Offset resumeOffset, int numIters) {
  assert(curFunc()->isAsync());
  assert(!resumed());
  assert(child->isA(Type::Obj));

  // Create the AsyncFunctionWaitHandle object. CreateAFWH takes care of
  // copying local variables and iterators.
  auto const func = curFunc();
  auto const resumeSk = SrcKey(func, resumeOffset, true);
  auto const resumeAddr = gen(LdBindAddr, LdBindAddrData(resumeSk));
  auto const waitHandle =
    gen(CreateAFWH, catchBlock, m_irb->fp(), cns(func->numSlotsInFrame()),
        resumeAddr, cns(resumeOffset),
        child);

  SSATmp* asyncAR = gen(LdAFWHActRec, Type::PtrToGen, waitHandle);

  // Call the FunctionSuspend hook and put the AsyncFunctionWaitHandle
  // on the stack so that the unwinder would decref it.
  push(waitHandle);
  emitRetSurpriseCheck(asyncAR, nullptr, makeCatch(), false);
  discard(1);

  // Grab caller info from ActRec, free ActRec, store the return value
  // and return control to the caller.
  gen(StRetVal, m_irb->fp(), waitHandle);
  SSATmp* retAddr = gen(LdRetAddr, m_irb->fp());
  SSATmp* sp = gen(RetAdjustStack, m_irb->fp());
  SSATmp* fp = gen(FreeActRec, m_irb->fp());
  gen(RetCtrl, RetCtrlData(false), sp, fp, retAddr);
}

void HhbcTranslator::emitAwaitR(SSATmp* child, Block* catchBlock,
                                Offset resumeOffset) {
  assert(curFunc()->isAsync());
  assert(resumed());
  assert(child->isA(Type::Obj));

  // Prepare child for establishing dependency.
  gen(AFWHPrepareChild, catchBlock, m_irb->fp(), child);

  // Suspend the async function.
  auto const resumeSk = SrcKey(curFunc(), resumeOffset, true);
  auto const resumeAddr = gen(LdBindAddr, LdBindAddrData(resumeSk));
  gen(StAsyncArRaw, RawMemData{RawMemData::AsyncResumeAddr}, m_irb->fp(),
      resumeAddr);
  gen(StAsyncArRaw, RawMemData{RawMemData::AsyncResumeOffset}, m_irb->fp(),
      cns(resumeOffset));

  // Set up the dependency.
  gen(AFWHBlockOn, m_irb->fp(), child);

  // Transfer control back to the scheduler.
  auto const sp = spillStack();
  push(cns(Type::InitNull));
  emitRetSurpriseCheck(m_irb->fp(), nullptr, makeCatch(), true);
  popC();

  auto const retAddr = gen(LdRetAddr, m_irb->fp());
  auto const fp = gen(FreeActRec, m_irb->fp());

  gen(RetCtrl, RetCtrlData(true), sp, fp, retAddr);
}

void HhbcTranslator::emitAwait(Offset resumeOffset, int numIters) {
  assert(curFunc()->isAsync());

  if (curFunc()->isAsyncGenerator()) PUNT(Await-AsyncGenerator);

  auto const catchBlock = makeCatch();
  auto const exitSlow   = makeExitSlow();

  if (!topC()->isA(Type::Obj)) PUNT(Await-NonObject);

  auto const child = popC();
  gen(JmpZero, exitSlow, gen(IsWaitHandle, child));

  // cns() would ODR-use these
  auto const kSucceeded = c_WaitHandle::STATE_SUCCEEDED;
  auto const kFailed    = c_WaitHandle::STATE_FAILED;

  auto const state = gen(LdWHState, child);
  gen(JmpEq, exitSlow, state, cns(kFailed));

  m_irb->ifThenElse(
    [&] (Block* taken) {
      gen(JmpEq, taken, state, cns(kSucceeded));
    },
    [&] { // Next: the wait handle is not finished, we need to suspend
      if (resumed()) {
        emitAwaitR(child, catchBlock, resumeOffset);
      } else {
        emitAwaitE(child, catchBlock, resumeOffset, numIters);
      }
    },
    [&] { // Taken: retrieve the result from the wait handle
      auto const res = gen(LdWHResult, child);
      gen(IncRef, res);
      gen(DecRef, child);
      push(res);
    }
  );
}

void HhbcTranslator::emitStrlen() {
  Type inType = topC()->type();

  if (inType <= Type::Str) {
    SSATmp* input = popC();
    if (input->isConst()) {
      // static string; fold its strlen operation
      push(cns(input->strVal()->size()));
    } else {
      push(gen(LdRaw, RawMemData{RawMemData::StrLen}, input));
      gen(DecRef, input);
    }
  } else if (inType <= Type::Null) {
    popC();
    push(cns(0));
  } else if (inType <= Type::Bool) {
    // strlen(true) == 1, strlen(false) == 0.
    push(gen(ConvBoolToInt, popC()));
  } else {
    emitInterpOne(Type::Int | Type::InitNull, 1);
  }
}

void HhbcTranslator::emitIncStat(int32_t counter, int32_t value, bool force) {
  if (Stats::enabled() || force) {
    gen(IncStat, cns(counter), cns(value), cns(force));
  }
}

void HhbcTranslator::emitIdx() {
  Type keyType = topC(1, DataTypeGeneric)->type();
  SSATmp* base = topC(2, DataTypeGeneric);
  Type baseType = base->type();

  if (baseType <= Type::Arr &&
      (keyType <= Type::Int || keyType <= Type::Str)) {
    emitArrayIdx();
  } else {
    emitIdxCommon(GenericIdx, makeCatch());
  }
}

// NOTE: #3233688 talks about making an idx fast path for collections and
// that is where this function will be used and make more sense. It's only
// called once now.
void HhbcTranslator::emitIdxCommon(Opcode opc, Block* catchBlock) {
  SSATmp* def = popC(DataTypeSpecific);
  SSATmp* key = popC(DataTypeSpecific);
  SSATmp* arr = popC(DataTypeSpecific);
  push(gen(opc, catchBlock, arr, key, def));
  gen(DecRef, arr);
  gen(DecRef, key);
  gen(DecRef, def);
}

void HhbcTranslator::emitArrayIdx() {
  // These types are just used to decide what to do; once we know what we're
  // actually doing we constrain the values with the popC()s later on in this
  // function.
  Type keyType = topC(1, DataTypeGeneric)->type();
  Type arrType = topC(2, DataTypeGeneric)->type();

  if (!(arrType <= Type::Arr)) {
    // raise fatal
    emitInterpOne(Type::Cell, 3);
    return;
  }

  if (keyType <= Type::Null) {
    SSATmp* def = popC(DataTypeGeneric); // def is just pushed back on the stack
    SSATmp* key = popC();
    SSATmp* arr = popC();

    // if the key is null it will not be found so just return the default
    push(def);
    gen(DecRef, arr);
    gen(DecRef, key);
    return;
  }
  if (!(keyType <= Type::Int || keyType <= Type::Str)) {
    emitInterpOne(Type::Cell, 3);
    return;
  }

  SSATmp* def = popC(DataTypeGeneric); // a helper will decref it but the
                                       // translated code doesn't care about
                                       // the type
  SSATmp* key = popC();
  SSATmp* arr = popC();

  KeyType arrayKeyType;
  bool checkForInt;
  bool converted;
  checkStrictlyInteger(key, arrayKeyType, checkForInt, converted);

  TCA opFunc;
  if (checkForInt) {
    opFunc = (TCA)&arrayIdxSi;
  } else if (KeyType::Int == arrayKeyType) {
    if (converted) {
      opFunc = (TCA)&arrayIdxIc;
    } else {
      opFunc = (TCA)&arrayIdxI;
    }
  } else {
    assert(KeyType::Str == arrayKeyType);
    opFunc = (TCA)&arrayIdxS;
  }

  push(gen(ArrayIdx, cns(opFunc), arr, key, def));
  gen(DecRef, arr);
  gen(DecRef, key);
  gen(DecRef, def);
}

void HhbcTranslator::emitIncTransCounter() {
  m_irb->gen(IncTransCounter);
}

void HhbcTranslator::emitIncProfCounter(TransID transId) {
  m_irb->gen(IncProfCounter, TransIDData(transId));
}

void HhbcTranslator::emitCheckCold(TransID transId) {
  m_irb->gen(CheckCold, makeExitOpt(transId), TransIDData(transId));
}

void HhbcTranslator::emitMInstr(const NormalizedInstruction& ni) {
  if (inPseudoMain()) {
    emitInterpOne(ni);
    return;
  }
  MInstrTranslator(ni, *this).emit();
}

/*
 * IssetL: return true if var is not uninit and !is_null(var)
 * Unboxes var if necessary when var is not uninit.
 */
void HhbcTranslator::emitIssetL(int32_t id) {
  auto const ldrefExit = makeExit();
  auto const ldgblExit = makePseudoMainExit();
  auto const ld = ldLocInner(id, ldrefExit, ldgblExit, DataTypeSpecific);
  push(gen(IsNType, Type::Null, ld));
}

void HhbcTranslator::emitEmptyL(int32_t id) {
  auto const ldrefExit = makeExit();
  auto const ldgblExit = makePseudoMainExit();
  auto const ld = ldLocInner(id, ldrefExit, ldgblExit, DataTypeSpecific);
  push(gen(XorBool, gen(ConvCellToBool, ld), cns(true)));
}

void HhbcTranslator::emitIsTypeC(DataType t) {
  SSATmp* src = popC(DataTypeSpecific);
  push(gen(IsType, Type(t), src));
  gen(DecRef, src);
}

void HhbcTranslator::emitIsTypeL(uint32_t id, DataType t) {
  auto const ldrefExit = makeExit();
  auto const ldgblExit = makePseudoMainExit();
  auto const val =
    ldLocInnerWarn(id, ldrefExit, ldgblExit, DataTypeSpecific);
  push(gen(IsType, Type(t), val));
}

void HhbcTranslator::emitIsScalarL(int id) {
  auto const ldrefExit = makeExit();
  auto const ldgblExit = makePseudoMainExit();
  SSATmp* src = ldLocInner(id, ldrefExit, ldgblExit, DataTypeSpecific);
  push(gen(IsScalarType, src));
}

void HhbcTranslator::emitIsScalarC() {
  SSATmp* src = popC();
  push(gen(IsScalarType, src));
  gen(DecRef, src);
}

void HhbcTranslator::emitPopA() { popA(); }

void HhbcTranslator::emitPopC() {
  popDecRef(Type::Cell, DataTypeGeneric);
}

void HhbcTranslator::emitPopV() {
  popDecRef(Type::BoxedCell, DataTypeGeneric);
}

void HhbcTranslator::emitPopR() {
  popDecRef(Type::Gen, DataTypeGeneric);
}

void HhbcTranslator::emitDup() {
  pushIncRef(topC());
}

void HhbcTranslator::emitJmpImpl(int32_t offset,
                                 JmpFlags flags,
                                 Block* catchBlock) {
  // If surprise flags are set, exit trace and handle surprise
  bool backward = static_cast<uint32_t>(offset) <= bcOff();
  if (backward && catchBlock) {
    emitJmpSurpriseCheck(catchBlock);
  }
  if (genMode() == IRGenMode::CFG) {
    if (flags & JmpFlagNextIsMerge) {
      exceptionBarrier();
    }
    auto target = getBlock(offset);
    assert(target != nullptr);
    gen(Jmp, target);
    return;
  }
  if (!(flags & JmpFlagEndsRegion)) return;
  gen(Jmp, makeExit(offset));
}

void HhbcTranslator::emitJmp(int32_t offset, JmpFlags flags) {
  emitJmpImpl(offset, flags,
              flags & JmpFlagSurprise ? makeCatch() : nullptr);
}

void HhbcTranslator::emitJmpCondHelper(int32_t taken,
                                       bool negate,
                                       JmpFlags flags,
                                       SSATmp* src) {
  if (flags & JmpFlagEndsRegion) {
    spillStack();
  }
  if (genMode() == IRGenMode::CFG && (flags & JmpFlagNextIsMerge)) {
    // Before jumping to a merge point we have to ensure that the
    // stack pointer is sync'ed.  Without an ExceptionBarrier the
    // SpillStack can be removed by DCE (especially since merge points
    // start with a DefSP to block SP-chain walking).
    exceptionBarrier();
  }
  auto const target = getBlock(taken);
  assert(target != nullptr);
  auto const boolSrc = gen(ConvCellToBool, src);
  gen(DecRef, src);
  gen(negate ? JmpZero : JmpNZero, target, boolSrc);
}

void HhbcTranslator::emitJmpZ(Offset taken, JmpFlags flags) {
  auto const src = popC();
  emitJmpCondHelper(taken, true, flags, src);
}

void HhbcTranslator::emitJmpNZ(Offset taken, JmpFlags flags) {
  auto const src = popC();
  emitJmpCondHelper(taken, false, flags, src);
}

/*
 * True if comparison may throw or reenter.
 *
 * 1. Objects compared with strings may involve calling a user-defined
 * __toString function.
 * 2. Array comparisons can throw if recursion is detected.
 */
bool cmpOpTypesMayReenter(Type t0, Type t1) {
  assert(!t0.equals(Type::Gen) && !t1.equals(Type::Gen));
  return (t0.maybe(Type::Obj) && t1.maybe(Type::Str)) ||
         (t0.maybe(Type::Str) && t1.maybe(Type::Obj)) ||
         (t0.maybe(Type::Obj) && t1.maybe(Type::Obj)) ||
         (t0.maybe(Type::Arr) && t1.maybe(Type::Arr));
}

Opcode matchReentrantCmp(Opcode opc) {
  switch (opc) {
  case Gt:  return GtX;
  case Gte: return GteX;
  case Lt:  return LtX;
  case Lte: return LteX;
  case Eq:  return EqX;
  case Neq: return NeqX;
  default:  return opc;
  }
}

void HhbcTranslator::emitCmp(Opcode opc) {
  Block* catchBlock = nullptr;
  Opcode opc2 = matchReentrantCmp(opc);
  // if the comparison operator could re-enter, convert it to the re-entrant
  // form and add the required catch block.
  // TODO #3446092 un-overload these opcodes.
  if (cmpOpTypesMayReenter(topC(0)->type(), topC(1)->type()) && opc2 != opc) {
    catchBlock = makeCatch();
    opc = opc2;
  }
  // src2 opc src1
  SSATmp* src1 = popC();
  SSATmp* src2 = popC();
  push(gen(opc, catchBlock, src2, src1));
  gen(DecRef, src2);
  gen(DecRef, src1);
}

// Return a constant SSATmp representing a static value held in a
// TypedValue.  The TypedValue may be a non-scalar, but it must have a
// static value.
SSATmp* HhbcTranslator::staticTVCns(const TypedValue* tv) {
  switch (tv->m_type) {
  case KindOfNull:         return cns(Type::InitNull);
  case KindOfBoolean:      return cns(!!tv->m_data.num);
  case KindOfInt64:        return cns(tv->m_data.num);
  case KindOfString:
  case KindOfStaticString: return cns(tv->m_data.pstr);
  case KindOfDouble:       return cns(tv->m_data.dbl);
  case KindOfArray:        return cns(tv->m_data.parr);
  default:                 always_assert(0);
  }
}

void HhbcTranslator::emitClsCnsD(int32_t cnsNameId, int32_t clsNameId,
                                 Type outPred) {
  auto const clsNameStr = lookupStringId(clsNameId);
  auto const cnsNameStr = lookupStringId(cnsNameId);
  auto const clsCnsName = ClsCnsName { clsNameStr, cnsNameStr };

  // If we have to side exit, do the RDS lookup before chaining to
  // another Tracelet so forward progress still happens.
  auto catchBlock = makeCatchNoSpill();
  auto const sideExit = makeSideExit(
    nextBcOff(),
    [&] {
      return gen(LookupClsCns, catchBlock, clsCnsName);
    }
  );

  /*
   * If the class is already defined in this request, and this
   * constant is a scalar constant, we can just compile it to a
   * literal.
   *
   * We need to guard at runtime that the class is defined in this
   * request and has the Class* we expect.  If the class is persistent
   * or a parent of the current context, we don't need the guard.
   */
  if (auto const cls = Unit::lookupClass(clsNameStr)) {
    Slot ignore;
    auto const tv = cls->cnsNameToTV(cnsNameStr, ignore);
    if (tv && tv->m_type != KindOfUninit) {
      if (!classIsPersistentOrCtxParent(cls)) {
        gen(CheckDefinedClsEq, CheckDefinedClsData{clsNameStr, cls}, sideExit);
      }
      push(staticTVCns(tv));
      return;
    }
  }

  auto guardType = Type::UncountedInit;
  if (outPred.strictSubtypeOf(guardType)) guardType = outPred;
  auto const cns = gen(LdClsCns, sideExit, clsCnsName, guardType);
  push(cns);
}

void HhbcTranslator::emitAKExists() {
  SSATmp* arr = popC();
  SSATmp* key = popC();

  if (!arr->isA(Type::Arr) && !arr->isA(Type::Obj)) {
    PUNT(AKExists_badArray);
  }
  if (!key->isA(Type::Str) && !key->isA(Type::Int) && !key->isA(Type::Null)) {
    PUNT(AKExists_badKey);
  }

  push(gen(AKExists, arr, key));
  gen(DecRef, arr);
  gen(DecRef, key);
}

void HhbcTranslator::emitFPassR() {
  emitUnboxRAux();
}

void HhbcTranslator::emitFPassV() {
  Block* exit = makeExit();
  SSATmp* tmp = popV();
  pushIncRef(gen(LdRef, exit, tmp->type().innerType(), tmp));
  gen(DecRef, tmp);
}

void HhbcTranslator::emitFPushCufIter(int32_t numParams,
                                      int32_t itId) {
  auto sp = spillStack();
  m_fpiStack.emplace(sp, m_irb->spOffset());
  gen(CufIterSpillFrame,
      FPushCufData(numParams, itId),
      sp, m_irb->fp());
}

static const Func* findCuf(Op op,
                           SSATmp* callable,
                           Class* ctx,
                           Class*& cls,
                           StringData*& invName,
                           bool& forward) {
  cls = nullptr;
  invName = nullptr;

  const StringData* str =
    callable->isA(Type::Str) && callable->isConst() ? callable->strVal()
                                                    : nullptr;
  const ArrayData* arr =
    callable->isA(Type::Arr) && callable->isConst() ? callable->arrVal()
                                                    : nullptr;

  StringData* sclass = nullptr;
  StringData* sname = nullptr;
  if (str) {
    Func* f = Unit::lookupFunc(str);
    if (f) return f;
    String name(const_cast<StringData*>(str));
    int pos = name.find("::");
    if (pos <= 0 || pos + 2 >= name.size() ||
        name.find("::", pos + 2) != String::npos) {
      return nullptr;
    }
    sclass = makeStaticString(name.substr(0, pos).get());
    sname = makeStaticString(name.substr(pos + 2).get());
  } else if (arr) {
    if (arr->size() != 2) return nullptr;
    const Variant& e0 = arr->get(int64_t(0), false);
    const Variant& e1 = arr->get(int64_t(1), false);
    if (!e0.isString() || !e1.isString()) return nullptr;
    sclass = e0.getStringData();
    sname = e1.getStringData();
    String name(sname);
    if (name.find("::") != String::npos) return nullptr;
  } else {
    return nullptr;
  }

  if (sclass->isame(s_self.get())) {
    if (!ctx) return nullptr;
    cls = ctx;
    forward = true;
  } else if (sclass->isame(s_parent.get())) {
    if (!ctx || !ctx->parent()) return nullptr;
    cls = ctx->parent();
    forward = true;
  } else if (sclass->isame(s_static.get())) {
    return nullptr;
  } else {
    cls = Unit::lookupUniqueClass(sclass);
    if (!cls) return nullptr;
  }

  bool magicCall = false;
  const Func* f = lookupImmutableMethod(cls, sname, magicCall,
                                        /* staticLookup = */ true, ctx);
  if (!f || (forward && !ctx->classof(f->cls()))) {
    /*
     * To preserve the invariant that the lsb class
     * is an instance of the context class, we require
     * that f's class is an instance of the context class.
     * This is conservative, but without it, we would need
     * a runtime check to decide whether or not to forward
     * the lsb class
     */
    return nullptr;
  }
  if (magicCall) invName = sname;
  return f;
}

bool HhbcTranslator::emitFPushCufArray(SSATmp* callable, int32_t numParams) {
  if (!callable->isA(Type::Arr)) return false;

  auto callableInst = callable->inst();
  if (!callableInst->is(NewPackedArray)) return false;

  auto callableSize = callableInst->src(0);
  if (!callableSize->isConst() ||
      callableSize->intVal() != 2) {
    return false;
  }

  auto method = getStackValue(m_irb->sp(), 0).value;
  auto object = getStackValue(m_irb->sp(), 1).value;
  if (!method || !object) return false;

  if (!method->isConst(Type::Str) ||
      strstr(method->strVal()->data(), "::") != nullptr) {
    return false;
  }

  if (!object->isA(Type::Obj)) {
    if (!object->type().equals(Type::Cell)) return false;
    // This is probably an object, and we just haven't guarded on
    // the type.  Do so now.
    auto exit = makeExit();
    object = gen(CheckType, Type::Obj, exit, object);
  }
  m_irb->constrainValue(object, DataTypeSpecific);

  popC();

  gen(IncRef, object);
  emitFPushObjMethodCommon(object,
                           method->strVal(),
                           numParams,
                           false /* shouldFatal */,
                           callable);
  gen(DecRef, callable);
  return true;
}

// FPushCuf when the callee is not known at compile time.
void HhbcTranslator::emitFPushCufUnknown(Op op, int32_t numParams) {
  if (op != Op::FPushCuf) {
    PUNT(emitFPushCufUnknown-nonFPushCuf);
  }

  if (topC()->isA(Type::Obj)) {
    return emitFPushFuncObj(numParams);
  }

  if (!topC()->type().subtypeOfAny(Type::Arr, Type::Str)) {
    PUNT(emitFPushCufUnknown);
  }

  // Peek at the top of the stack before deciding to pop it.
  auto const callable = topC();
  if (emitFPushCufArray(callable, numParams)) return;

  popC();

  emitFPushActRec(
    cns(Type::Nullptr),
    cns(Type::Nullptr),
    numParams,
    nullptr
  );
  auto const actRec = spillStack();

  /*
   * This is a similar case to lookup for functions in FPushFunc or
   * FPushObjMethod.  We can throw in a weird situation where the
   * ActRec is already on the stack, but this bytecode isn't done
   * executing yet.  See arPreliveOverwriteCells for details about why
   * we need this marker.
   */
  updateMarker();

  auto const opcode = callable->isA(Type::Arr) ? LdArrFPushCuf
                                               : LdStrFPushCuf;
  gen(opcode, makeCatch({callable}, 1), callable, actRec, m_irb->fp());
  gen(DecRef, callable);
}

void HhbcTranslator::emitFPushCufOp(Op op, int32_t numArgs) {
  const bool safe = op == OpFPushCufSafe;
  bool forward = op == OpFPushCufF;
  SSATmp* callable = topC(safe ? 1 : 0);

  Class* cls = nullptr;
  StringData* invName = nullptr;
  auto const callee = findCuf(op, callable, curClass(), cls, invName, forward);
  if (!callee) return emitFPushCufUnknown(op, numArgs);

  SSATmp* ctx;
  SSATmp* safeFlag = cns(true); // This is always true until the slow exits
                                // below are implemented
  SSATmp* func = cns(callee);
  if (cls) {
    auto const exitSlow = makeExitSlow();
    if (!RDS::isPersistentHandle(cls->classHandle())) {
      // The miss path is complicated and rare.  Punt for now.  This
      // must be checked before we IncRef the context below, because
      // the slow exit will want to do that same IncRef via InterpOne.
      gen(LdClsCachedSafe, exitSlow, cns(cls->name()));
    }

    if (forward) {
      ctx = gen(LdCtx, FuncData(curFunc()), m_irb->fp());
      ctx = gen(GetCtxFwdCall, ctx, cns(callee));
    } else {
      ctx = genClsMethodCtx(callee, cls);
    }
  } else {
    ctx = cns(Type::Nullptr);
    if (!RDS::isPersistentHandle(callee->funcHandle())) {
      // The miss path is complicated and rare. Punt for now.
      func = gen(
        LdFuncCachedSafe, LdFuncCachedData(callee->name()), makeExitSlow()
      );
    }
  }

  SSATmp* defaultVal = safe ? popC() : nullptr;
  popDecRef(Type::Cell); // callable
  if (safe) {
    push(defaultVal);
    push(safeFlag);
  }

  emitFPushActRec(func, ctx, numArgs, invName);
}

void HhbcTranslator::emitNativeImpl() {
  if (isInlining()) return emitNativeImplInlined();

  gen(NativeImpl, m_irb->fp());
  SSATmp* sp = gen(RetAdjustStack, m_irb->fp());
  SSATmp* retAddr = gen(LdRetAddr, m_irb->fp());
  SSATmp* fp = gen(FreeActRec, m_irb->fp());
  gen(RetCtrl, RetCtrlData(false), sp, fp, retAddr);
}

void HhbcTranslator::emitFPushActRec(SSATmp* func,
                                     SSATmp* objOrClass,
                                     int32_t numArgs,
                                     const StringData* invName) {
  /*
   * Before allocating an ActRec, we do a spillStack so we'll have a
   * StkPtr that represents what the stack will look like after the
   * ActRec is popped.
   */
  auto actualStack = spillStack();
  auto returnSp = actualStack;

  m_fpiStack.emplace(returnSp, m_irb->spOffset());

  ActRecInfo info;
  info.numArgs = numArgs;
  info.invName = invName;
  gen(
    SpillFrame,
    info,
    // Using actualStack instead of returnSp so SpillFrame still gets
    // the src in rVmSp.  (TODO(#2288359).)
    actualStack,
    func,
    objOrClass
  );
  assert(m_irb->stackDeficit() == 0);
}

void HhbcTranslator::emitFPushCtorCommon(SSATmp* cls,
                                         SSATmp* obj,
                                         const Func* func,
                                         int32_t numParams) {
  push(obj);
  auto const fn = [&] {
    if (func) return cns(func);
    /*
      Without the updateMarker, the catch trace will write
      obj onto the stack, but the VMRegAnchor will setup the
      stack as it was before the FPushCtor*, which (for
      FPushCtorD at least) won't include obj
    */
    updateMarker();
    return gen(LdClsCtor, makeCatch(), cls);
  }();
  gen(IncRef, obj);
  auto numArgsAndFlags = ActRec::encodeNumArgs(numParams, false, false, true);
  emitFPushActRec(fn, obj, numArgsAndFlags, nullptr);
}

void HhbcTranslator::emitFPushCtor(int32_t numParams) {
  auto catchBlock = makeCatch();
  SSATmp* cls = popA();
  SSATmp* obj = gen(AllocObj, catchBlock, cls);
  gen(IncRef, obj);
  emitFPushCtorCommon(cls, obj, nullptr, numParams);
}

static bool canInstantiateClass(const Class* cls) {
  return cls && isNormalClass(cls) && !isAbstract(cls);
}

void HhbcTranslator::emitInitProps(const Class* cls, Block* catchBlock) {
  cls->initPropHandle();
  m_irb->ifThen(
    [&](Block* taken) {
      gen(CheckInitProps, taken, ClassData(cls));
    },
    [&] {
      m_irb->hint(Block::Hint::Unlikely);
      gen(InitProps, catchBlock, ClassData(cls));
    }
  );
}

void HhbcTranslator::emitInitSProps(const Class* cls, Block* catchBlock) {
  cls->initSPropHandles();
  m_irb->ifThen(
    [&](Block* taken) {
      gen(CheckInitSProps, taken, ClassData(cls));
    },
    [&] {
      m_irb->hint(Block::Hint::Unlikely);
      gen(InitSProps, catchBlock, ClassData(cls));
    }
  );
}

SSATmp* HhbcTranslator::emitAllocObjFast(const Class* cls) {
  auto registerObj = [this, cls](SSATmp* obj) {
    if (RuntimeOption::EnableObjDestructCall && cls->getDtor()) {
      gen(RegisterLiveObj, obj);
    }
    return obj;
  };

  // If it's an extension class with a custom instance initializer,
  // that init function does all the work.
  if (cls->instanceCtor()) {
    return registerObj(gen(ConstructInstance, makeCatch(), ClassData(cls)));
  }

  // First, make sure our property init vectors are all set up
  bool props = cls->pinitVec().size() > 0;
  bool sprops = cls->numStaticProperties() > 0;
  assert((props || sprops) == cls->needInitialization());
  if (cls->needInitialization()) {
    if (props) emitInitProps(cls, makeCatch());
    if (sprops) emitInitSProps(cls, makeCatch());
  }

  // Next, allocate the object
  auto const ssaObj = gen(NewInstanceRaw, ClassData(cls));

  // Initialize the properties
  gen(InitObjProps, ClassData(cls), ssaObj);

  // Call a custom initializer if one exists
  if (cls->callsCustomInstanceInit()) {
    return registerObj(gen(CustomInstanceInit, ssaObj));
  }

  return registerObj(ssaObj);
}

void HhbcTranslator::emitFPushCtorD(int32_t numParams, int32_t classNameStrId) {
  const StringData* className = lookupStringId(classNameStrId);

  const Class* cls = Unit::lookupUniqueClass(className);
  bool uniqueCls = classIsUnique(cls);
  bool persistentCls = classHasPersistentRDS(cls);
  bool canInstantiate = canInstantiateClass(cls);
  bool fastAlloc =
    persistentCls &&
    canInstantiate &&
    !cls->callsCustomInstanceInit();

  const Func* func = uniqueCls ? cls->getCtor() : nullptr;
  if (func && !(func->attrs() & AttrPublic)) {
    Class* ctx = curClass();
    if (!ctx) {
      func = nullptr;
    } else if (ctx != cls) {
      if ((func->attrs() & AttrPrivate) ||
        !(ctx->classof(cls) || cls->classof(ctx))) {
        func = nullptr;
      }
    }
  }

  auto ssaCls = persistentCls ? cns(cls)
                              : gen(LdClsCached, makeCatch(), cns(className));
  if (!ssaCls->isConst() && uniqueCls) {
    // If the Class is unique but not persistent, it's safe to use it as a
    // const after the LdClsCached, which will throw if the class can't be
    // defined.
    ssaCls = cns(cls);
  }

  auto const obj = fastAlloc ? emitAllocObjFast(cls)
                             : gen(AllocObj, makeCatch(), ssaCls);
  gen(IncRef, obj);
  emitFPushCtorCommon(ssaCls, obj, func, numParams);
}

const StaticString s_uuinvoke("__invoke");

/*
 * The CreateCl opcode is specified as not being allowed before the
 * class it creates exists, and closure classes are always unique.
 *
 * This means even if we're not in RepoAuthoritative mode, as long as
 * this code is reachable it will always use the same closure Class*,
 * so we can just burn it into the TC without using RDS.
 */
void HhbcTranslator::emitCreateCl(int32_t numParams, int32_t funNameStrId) {
  auto const cls = Unit::lookupUniqueClass(lookupStringId(funNameStrId));
  auto const invokeFunc = cls->lookupMethod(s_uuinvoke.get());
  auto const clonedFunc = invokeFunc->cloneAndSetClass(curClass());
  assert(cls && (cls->attrs() & AttrUnique));

  auto const closure = emitAllocObjFast(cls);
  gen(IncRef, closure);

  auto const ctx = [&]{
    if (!curClass()) return cns(nullptr);
    auto const ldctx = gen(LdCtx, FuncData(curFunc()), m_irb->fp());
    if (invokeFunc->attrs() & AttrStatic) {
      return gen(ConvClsToCctx, gen(LdClsCtx, ldctx));
    }
    gen(IncRefCtx, ldctx);
    return ldctx;
  }();
  gen(StClosureCtx, closure, ctx);
  gen(StClosureFunc, FuncData(clonedFunc), closure);

  SSATmp* args[numParams];
  for (int32_t i = 0; i < numParams; ++i) {
    args[numParams - i - 1] = popF();
  }

  int32_t propId = 0;
  for (; propId < numParams; ++propId) {
    gen(
      StClosureArg,
      PropByteOffset(cls->declPropOffset(propId)),
      closure,
      args[propId]
    );
  }

  // Closure static variables are per instance, and need to start
  // uninitialized.  After numParams use vars, the remaining instance
  // properties hold any static locals.
  assert(cls->numDeclProperties() ==
      clonedFunc->numStaticLocals() + numParams);
  for (int32_t numDeclProperties = cls->numDeclProperties();
      propId < numDeclProperties;
      ++propId) {
    gen(
      StClosureArg,
      PropByteOffset(cls->declPropOffset(propId)),
      closure,
      cns(Type::Uninit)
    );
  }

  push(closure);
}

void HhbcTranslator::emitFPushFuncCommon(const Func* func,
                                         const StringData* name,
                                         const StringData* fallback,
                                         int32_t numParams) {
  if (func) {
    func->validate();
    if (func->isNameBindingImmutable(curUnit())) {
      emitFPushActRec(cns(func),
                      cns(Type::Nullptr),
                      numParams,
                      nullptr);
      return;
    }
  }

  // LdFuncCached can throw
  auto const catchBlock = makeCatch();
  auto const ssaFunc = fallback
    ? gen(LdFuncCachedU,
          LdFuncCachedUData { name, fallback },
          catchBlock)
    : gen(LdFuncCached,
          LdFuncCachedData { name },
          catchBlock);
  emitFPushActRec(ssaFunc,
                  cns(Type::Nullptr),
                  numParams,
                  nullptr);
}

void HhbcTranslator::emitFPushFuncD(int32_t numParams, int32_t funcId) {
  const NamedEntityPair& nep = lookupNamedEntityPairId(funcId);
  const StringData* name = nep.first;
  const Func* func       = Unit::lookupFunc(nep.second);
  emitFPushFuncCommon(func, name, nullptr, numParams);
}

void HhbcTranslator::emitFPushFuncU(int32_t numParams,
                                    int32_t funcId,
                                    int32_t fallbackFuncId) {
  const NamedEntityPair& nep = lookupNamedEntityPairId(funcId);
  const StringData* name     = nep.first;
  const Func* func           = Unit::lookupFunc(nep.second);
  const NamedEntityPair& fallbackNep = lookupNamedEntityPairId(fallbackFuncId);
  const StringData* fallbackName     = fallbackNep.first;
  emitFPushFuncCommon(func, name, fallbackName, numParams);
}

void HhbcTranslator::emitFPushFunc(int32_t numParams) {
  if (topC()->isA(Type::Obj)) {
    return emitFPushFuncObj(numParams);
  }

  if (topC()->isA(Type::Arr)) {
    return emitFPushFuncArr(numParams);
  }

  if (!topC()->isA(Type::Str)) {
    PUNT(FPushFunc_not_Str);
  }

  auto const catchBlock = makeCatch();
  auto const funcName = popC();
  emitFPushActRec(gen(LdFunc, catchBlock, funcName),
                  cns(Type::Nullptr),
                  numParams,
                  nullptr);
}

void HhbcTranslator::emitFPushFuncObj(int32_t numParams) {
  auto const slowExit = makeExitSlow();
  auto const obj      = popC();
  auto const cls      = gen(LdObjClass, obj);
  auto const func     = gen(LdObjInvoke, slowExit, cls);
  emitFPushActRec(func, obj, numParams, nullptr);
}

void HhbcTranslator::emitFPushFuncArr(int32_t numParams) {
  auto const thisAR = m_irb->fp();

  auto const arr    = popC();
  emitFPushActRec(
    cns(Type::Nullptr),
    cns(Type::Nullptr),
    numParams,
    nullptr);
  auto const actRec = spillStack();

  // This is special. We need to move the stackpointer incase LdArrFuncCtx
  // calls a destructor. Otherwise it would clobber the ActRec we just
  // pushed.
  updateMarker();

  gen(LdArrFuncCtx, makeCatch({arr}, 1), arr, actRec, thisAR);
  gen(DecRef, arr);
}

void HhbcTranslator::emitFPushObjMethodCommon(SSATmp* obj,
                                              const StringData* methodName,
                                              int32_t numParams,
                                              bool shouldFatal,
                                              SSATmp* extraSpill) {
  SSATmp* objOrCls = obj;
  const Class* baseClass = nullptr;
  if (obj->type().isSpecialized()) {
    auto cls = obj->type().getClass();
    if (!m_irb->constrainValue(obj, TypeConstraint(cls).setWeak())) {
      // If we know the class without having to specialize a guard any further,
      // use it.
      baseClass = cls;
    }
  }

  bool magicCall = false;
  const Func* func = lookupImmutableMethod(baseClass,
                                           methodName,
                                           magicCall,
                                           /* staticLookup: */
                                           false,
                                           curClass());

  if (!func) {
    if (baseClass && !(baseClass->attrs() & AttrInterface)) {
      LookupResult res =
        g_context->lookupObjMethod(func, baseClass, methodName, curClass(),
                                     false);
      if (res == LookupResult::MethodFoundWithThis ||
          /*
           * TODO(#4455926): We don't allow vtable-style dispatch of
           * abstract static methods, but not for any real reason
           * here.  It should be able to work, but needs further
           * testing to be enabled.
           */
          (res == LookupResult::MethodFoundNoThis && !func->isAbstract())) {
        /*
         * If we found the func in baseClass, then either:
         *  a) its private, and this is always going to be the
         *     called function. This case is handled further down.
         * OR
         *  b) any derived class must have a func that matches in staticness
         *     and is at least as accessible (and in particular, you can't
         *     override a public/protected method with a private method).  In
         *     this case, we emit code to dynamically lookup the method given
         *     the Object and the method slot, which is the same as func's.
         */
        if (!(func->attrs() & AttrPrivate)) {
          auto const clsTmp = gen(LdObjClass, obj);
          auto const funcTmp = gen(
            LdClsMethod, clsTmp, cns(-(func->methodSlot() + 1))
          );
          if (res == LookupResult::MethodFoundNoThis) {
            gen(DecRef, obj);
            objOrCls = clsTmp;
          }
          emitFPushActRec(funcTmp, objOrCls, numParams,
                          magicCall ? methodName : nullptr);
          return;
        }
      } else {
        // method lookup did not find anything
        func = nullptr; // force lookup
      }
    }
  }

  if (func != nullptr) {
    /*
     * static function: store base class into this slot instead of obj
     * and decref the obj that was pushed as the this pointer since
     * the obj won't be in the actrec and thus MethodCache::lookup won't
     * decref it
     *
     * static closure body: we still need to pass the object instance
     * for the closure prologue to properly do its dispatch (and
     * extract use vars).  It will decref it and put the class on the
     * actrec before entering the "real" cloned closure body.
     */
    if (func->attrs() & AttrStatic && !func->isClosureBody()) {
      assert(baseClass);
      gen(DecRef, obj);
      objOrCls = cns(baseClass);
    }
    emitFPushActRec(cns(func),
                    objOrCls,
                    numParams,
                    magicCall ? methodName : nullptr);
    return;
  }

  fpushObjMethodUnknown(obj, methodName, numParams, shouldFatal, extraSpill);
}

// Pushing for object method when we don't know the Func* statically.
void HhbcTranslator::fpushObjMethodUnknown(SSATmp* obj,
                                           const StringData* methodName,
                                           int32_t numParams,
                                           bool shouldFatal,
                                           SSATmp* extraSpill) {
  spillStack();
  emitFPushActRec(cns(Type::Nullptr),  // Will be set by LdObjMethod
                  obj,
                  numParams,
                  nullptr);
  auto const actRec = spillStack();
  auto const objCls = gen(LdObjClass, obj);

  // This is special. We need to move the stackpointer in case
  // LdObjMethod calls a destructor. Otherwise it would clobber the
  // ActRec we just pushed.
  updateMarker();
  Block* catchBlock;
  if (extraSpill) {
    /*
     * If LdObjMethod throws, it nulls out the ActRec (since the unwinder
     * will attempt to destroy it as if it were cells), and then writes
     * obj into the last entry, since we need it to be destroyed.
     * If we have another object to destroy, we should write it in
     * the first - so pop 1 cell, then push extraSpill.
     */
    std::vector<SSATmp*> spill{extraSpill};
    catchBlock = makeCatch(spill, 1);
  } else {
    catchBlock = makeCatchNoSpill();
  }
  gen(LdObjMethod,
      LdObjMethodData { methodName, shouldFatal },
      catchBlock,
      objCls,
      actRec);
}

void HhbcTranslator::emitFPushObjMethodD(int32_t numParams,
                                         int32_t methodNameStrId) {
  auto const obj = popC();
  if (!obj->isA(Type::Obj)) PUNT(FPushObjMethodD-nonObj);
  auto const methodName = lookupStringId(methodNameStrId);
  emitFPushObjMethodCommon(obj, methodName, numParams, true /* shouldFatal */);
}

SSATmp* HhbcTranslator::genClsMethodCtx(const Func* callee, const Class* cls) {
  bool mustBeStatic = true;

  if (!(callee->attrs() & AttrStatic) &&
      !(curFunc()->attrs() & AttrStatic) &&
      curClass()) {
    if (curClass()->classof(cls)) {
      // In this case, it might not be static, but we can be sure
      // we're going to forward $this if thisAvailable.
      mustBeStatic = false;
    } else if (cls->classof(curClass())) {
      // Unlike the above, we might be calling down to a subclass that
      // is not related to the current instance.  To know whether this
      // call forwards $this requires a runtime type check, so we have
      // to punt instead of trying the thisAvailable path below.
      PUNT(getClsMethodCtx-PossibleStaticRelatedCall);
    }
  }

  if (mustBeStatic) {
    // static function: ctx is just the Class*. LdCls will simplify to a
    // DefConst or LdClsCached.
    return gen(LdCls, makeCatch(), cns(cls->name()), cns(curClass()));
  }
  if (m_irb->thisAvailable()) {
    // might not be a static call and $this is available, so we know it's
    // definitely not static
    assert(curClass());
    auto this_ = gen(LdThis, m_irb->fp());
    gen(IncRef, this_);
    return this_;
  }
  // might be a non-static call. we have to inspect the func at runtime
  PUNT(getClsMethodCtx-MightNotBeStatic);
}

void HhbcTranslator::emitFPushClsMethodD(int32_t numParams,
                                         int32_t methodNameStrId,
                                         int32_t clssNamedEntityPairId) {

  auto const methodName = lookupStringId(methodNameStrId);
  auto const& np        = lookupNamedEntityPairId(clssNamedEntityPairId);
  auto const className  = np.first;
  auto const baseClass  = Unit::lookupUniqueClass(np.second);
  bool magicCall        = false;

  if (auto const func = lookupImmutableMethod(baseClass,
                                              methodName,
                                              magicCall,
                                              true /* staticLookup */,
                                              curClass())) {
    auto const objOrCls = genClsMethodCtx(func, baseClass);
    emitFPushActRec(cns(func),
                    objOrCls,
                    numParams,
                    func && magicCall ? methodName : nullptr);
    return;
  }

  auto const slowExit = makeExitSlow();
  auto const data = ClsMethodData{className, methodName, np.second};

  // Look up the Func* in the targetcache. If it's not there, try the slow
  // path. If that fails, slow exit.
  auto const func = m_irb->cond(
    0,
    [&] (Block* taken) {
      return gen(CheckNonNull, taken, gen(LdClsMethodCacheFunc, data));
    },
    [&] (SSATmp* func) { // next
      return func;
    },
    [&] { // taken
      m_irb->hint(Block::Hint::Unlikely);
      auto result = gen(LookupClsMethodCache, makeCatch(), data,
                        m_irb->fp());
      return gen(CheckNonNull, slowExit, result);
    }
  );
  auto const clsCtx = gen(LdClsMethodCacheCls, data);

  emitFPushActRec(func,
                  clsCtx,
                  numParams,
                  nullptr);
}

void HhbcTranslator::emitFPushClsMethod(int32_t numParams) {
  auto const clsVal  = popA();
  auto const methVal = popC();

  if (!methVal->isA(Type::Str) || !clsVal->isA(Type::Cls)) {
    PUNT(FPushClsMethod-unknownType);
  }

  if (methVal->isConst() && clsVal->inst()->op() == LdClsCctx) {
    /*
     * Optimize FPushClsMethod when the method is a known static
     * string and the input class is the context.  The common bytecode
     * pattern here is LateBoundCls ; FPushClsMethod.
     *
     * This logic feels like it belongs in the simplifier, but the
     * generated code for this case is pretty different, since we
     * don't need the pre-live ActRec trick.
     */
    auto const cls = curClass();
    const Func* func;
    auto res =
      g_context->lookupClsMethod(func,
                                   cls,
                                   methVal->strVal(),
                                   nullptr,
                                   cls,
                                   false);
    if (res == LookupResult::MethodFoundNoThis) {
      auto funcTmp = gen(LdClsMethod, clsVal, cns(-(func->methodSlot() + 1)));
      emitFPushActRec(funcTmp, clsVal, numParams, nullptr);
      return;
    }
  }

  emitFPushActRec(cns(Type::Nullptr),
                  cns(Type::Nullptr),
                  numParams,
                  nullptr);
  auto const actRec = spillStack();

  /*
   * Similar to FPushFunc/FPushObjMethod, we have an incomplete ActRec
   * on the stack and must handle that properly if we throw.
   */
  updateMarker();

  gen(LookupClsMethod, makeCatch({methVal, clsVal}), clsVal, methVal, actRec,
      m_irb->fp());
  gen(DecRef, methVal);
}

void HhbcTranslator::emitFPushClsMethodF(int32_t numParams) {
  auto const exitBlock = makeExitSlow();

  auto classTmp = top(Type::Cls);
  auto methodTmp = topC(1, DataTypeGeneric);
  assert(classTmp->isA(Type::Cls));
  if (!classTmp->isConst() || !methodTmp->isConst(Type::Str)) {
    PUNT(FPushClsMethodF-unknownClassOrMethod);
  }
  m_irb->constrainValue(methodTmp, DataTypeSpecific);

  auto const cls = classTmp->clsVal();
  auto const methName = methodTmp->strVal();

  bool magicCall = false;
  auto const vmfunc = lookupImmutableMethod(cls,
                                            methName,
                                            magicCall,
                                            true /* staticLookup */,
                                            curClass());
  auto const catchBlock = vmfunc ? nullptr : makeCatch();
  discard(2);

  auto const curCtxTmp = gen(LdCtx, FuncData(curFunc()), m_irb->fp());
  if (vmfunc) {
    auto const funcTmp = cns(vmfunc);
    auto const newCtxTmp = gen(GetCtxFwdCall, curCtxTmp, funcTmp);
    emitFPushActRec(funcTmp, newCtxTmp, numParams,
                    (magicCall ? methName : nullptr));
    return;
  }

  auto const data = ClsMethodData{cls->name(), methName};
  auto const funcTmp = m_irb->cond(
    0,
    [&](Block* taken) {
      return gen(CheckNonNull, taken, gen(LdClsMethodFCacheFunc, data));
    },
    [&](SSATmp* func) { // next
      return func;
    },
    [&] { // taken
      m_irb->hint(Block::Hint::Unlikely);
      auto result = gen(LookupClsMethodFCache, catchBlock, data,
                        cns(cls), m_irb->fp());
      return gen(CheckNonNull, exitBlock, result);
    }
  );
  auto const ctx = gen(GetCtxFwdCallDyn, data, curCtxTmp);

  emitFPushActRec(funcTmp,
                  ctx,
                  numParams,
                  magicCall ? methName : nullptr);
}

void HhbcTranslator::emitFCallArray(const Offset pcOffset,
                                    const Offset after,
                                    bool destroyLocals) {
  auto const stack = spillStack();
  gen(CallArray, CallArrayData { pcOffset, after, destroyLocals }, stack);
}

void HhbcTranslator::emitFCall(uint32_t numParams,
                               Offset returnBcOffset,
                               const Func* callee,
                               bool destroyLocals) {
  if (RuntimeOption::EvalRuntimeTypeProfile) {
    for (auto i = uint32_t{0}; i < numParams; ++i) {
      auto const val = topF(numParams - i - 1);
      if (callee != nullptr) {
        gen(TypeProfileFunc, TypeProfileData(i), val, cns(callee));
      } else  {
        auto const func = gen(LdARFuncPtr, m_irb->sp(), cns(0));
        gen(TypeProfileFunc, TypeProfileData(i), val, func);
      }
    }
  }

  /*
   * Figure out if we know where we're going already (if a prologue
   * was already generated, we don't need to do a whole bind call
   * thing again).
   *
   * We're skipping magic calls right now because 'callee' will be set
   * to __call in some cases (with 86ctor) where we shouldn't really
   * call that function (arguable bug in annotation).
   *
   * TODO(#4357498): This is currently disabled, because we haven't
   * set things up properly to be able to eagerly bind.  Because
   * code-gen can punt, the code there needs to delay adding these
   * smash locations until after we know the translation isn't punted.
   */
  auto const knownPrologue = [&]() -> TCA {
    if (false) {
      if (!callee || callee->isMagic()) return nullptr;
      auto const prologueIndex =
        numParams <= callee->numNonVariadicParams()
          ? numParams
          : callee->numNonVariadicParams() + 1;
      TCA ret;
      if (!mcg->checkCachedPrologue(callee, prologueIndex, ret)) {
        return nullptr;
      }
      return ret;
    }
    return nullptr;
  }();

  auto const sp = spillStack();
  gen(
    Call,
    CallData {
      numParams,
      returnBcOffset,
      callee,
      destroyLocals,
      knownPrologue
    },
    sp,
    m_irb->fp()
  );
  if (!m_fpiStack.empty()) {
    m_fpiStack.pop();
  }
}

void HhbcTranslator::emitNameA() {
  push(gen(LdClsName, popA()));
}

const StaticString
  s_count("count"),
  s_ini_get("ini_get"),
  s_get_class("get_class"),
  s_get_called_class("get_called_class");

SSATmp* HhbcTranslator::optimizedCallCount() {
  auto const mode = top(Type::Int, 0);
  auto const val = top(Type::Gen, 1);

  // Bail if we're trying to do a recursive count()
  if (!mode->isConst(0)) return nullptr;

  return gen(Count, makeCatch(), val);
}

SSATmp* HhbcTranslator::optimizedCallIniGet() {
  // Only generate the optimized version if the argument passed in is a
  // static string with a constant literal value so we can get the string value
  // at JIT time.
  Type argType = topType(0);
  if (!(argType <= Type::StaticStr) || !argType.isConst()) {
      return nullptr;
  }

  // We can only optimize settings that are system wide since user level
  // settings can be overridden during the execution of a request.
  std::string settingName = top(Type::Str, 0)->strVal()->toCppString();
  IniSetting::Mode mode = IniSetting::PHP_INI_NONE;
  if (!IniSetting::GetMode(settingName, mode) ||
      !(mode & IniSetting::PHP_INI_SYSTEM)) {
    return nullptr;
  }

  Variant value;
  IniSetting::Get(settingName, value);
  // ini_get() is now enhanced to return more than strings
  // Only return a string, get out of here if we are something
  // else like an array
  if (value.isString()) {
    return cns(makeStaticString(value.toString()));
  }
  return nullptr;
}

SSATmp* HhbcTranslator::optimizedCallGetClass(uint32_t numNonDefault) {
  auto const curCls = curClass();
  auto const curName = [&] {
    return curCls != nullptr ? cns(curCls->name()) : nullptr;
  };

  if (numNonDefault == 0) return curName();
  assert(numNonDefault == 1);

  auto const val = topC(0);
  if (val->isA(Type::Null)) return curName();

  if (val->isA(Type::Obj)) return gen(LdClsName, gen(LdObjClass, val));

  return nullptr;
}

SSATmp* HhbcTranslator::optimizedCallGetCalledClass() {
  if (!curClass()) return nullptr;

  auto const ctx = gen(LdCtx, FuncData(curFunc()), m_irb->fp());
  auto const cls = gen(LdClsCtx, ctx);
  return gen(LdClsName, cls);
}

bool HhbcTranslator::optimizedFCallBuiltin(const Func* func,
                                           uint32_t numArgs,
                                           uint32_t numNonDefault) {
  SSATmp* res = nullptr;
  switch (numArgs) {
    case 0:
      if (func->name()->isame(s_get_called_class.get())) {
        res = optimizedCallGetCalledClass();
      }
      break;
    case 1:
      if (func->name()->isame(s_ini_get.get())) res = optimizedCallIniGet();
      else if (func->name()->isame(s_get_class.get())) {
        res = optimizedCallGetClass(numNonDefault);
      }
      break;
    case 2:
      if (func->name()->isame(s_count.get())) res = optimizedCallCount();
      break;
    default: break;
  }

  if (res == nullptr) return false;

  // Decref and free args
  for (int i = 0; i < numArgs; i++) {
    auto const arg = popR();
    if (i >= numArgs - numNonDefault) {
      gen(DecRef, arg);
    }
  }

  push(res);
  return true;
}

void HhbcTranslator::emitFCallBuiltinCoerce(const Func* callee,
                                            uint32_t numArgs,
                                            uint32_t numNonDefault,
                                            bool destroyLocals) {
  /*
   * Spill args to stack.  Some of the arguments may be passed by
   * reference, for which case we will pass a stack address.
   *
   * The CallBuiltin instruction itself doesn't depend on the stack
   * pointer, but if any of its args were passed via pointers to the
   * stack it will indirectly depend on it.
   */
  spillStack();

  // Convert types if needed.
  for (int i = 0; i < numNonDefault; i++) {
    auto const& pi = callee->params()[i];
    switch (pi.builtinType) {
    case KindOfBoolean:
    case KindOfInt64:
    case KindOfDouble:
    case KindOfArray:
    case KindOfObject:
    case KindOfResource:
    case KindOfString:
      gen(CoerceStk,
          Type(pi.builtinType),
          StackOffset(numArgs - i - 1),
          makeExitSlow(),
          m_irb->sp());
      break;
    case KindOfUnknown:
      break;
    default:
      not_reached();
    }
  }

  // Pass arguments for CallBuiltin.
  SSATmp* args[numArgs];
  for (int i = numArgs - 1; i >= 0; i--) {
    auto const& pi = callee->params()[i];
    switch (pi.builtinType) {
    case KindOfBoolean:
    case KindOfInt64:
    case KindOfDouble:
      args[i] = top(Type(pi.builtinType),
                        numArgs - i - 1);
      break;
    default:
      args[i] = ldStackAddr(numArgs - i - 1, DataTypeSpecific);
      break;
    }
  }

  // Generate call and set return type
  auto const retDt = callee->returnType();
  auto retType = retDt == KindOfUnknown ? Type::Cell : Type(retDt);
  if (callee->attrs() & ClassInfo::IsReference) retType = retType.box();

  SSATmp** const decayedPtr = &args[0];
  auto const ret = gen(
    CallBuiltin,
    retType,
    CallBuiltinData { callee, destroyLocals },
    makeCatch(),
    std::make_pair(numArgs, decayedPtr)
  );

  // Decref and free args
  for (int i = 0; i < numArgs; i++) {
    auto const arg = popR();
    if (i >= numArgs - numNonDefault) {
      gen(DecRef, arg);
    }
  }

  push(ret);
}

void HhbcTranslator::emitFCallBuiltin(uint32_t numArgs,
                                      uint32_t numNonDefault,
                                      int32_t funcId,
                                      bool destroyLocals) {
  const NamedEntity* ne = lookupNamedEntityId(funcId);
  const Func* callee = Unit::lookupFunc(ne);

  callee->validate();

  if (optimizedFCallBuiltin(callee, numArgs, numNonDefault)) return;

  if (callee->isParamCoerceMode()) {
    return emitFCallBuiltinCoerce(callee, numArgs, numNonDefault,
                                  destroyLocals);
  }

  emitBuiltinCall(callee,
                  numArgs,
                  numNonDefault,
                  nullptr,  /* no this */
                  false,    /* not inlining */
                  false,    /* not inlining constructor */
                  destroyLocals,
                  [&](uint32_t) { return popC(); });
}

template<class GetArg>
void HhbcTranslator::emitBuiltinCall(const Func* callee,
                                     uint32_t numArgs,
                                     uint32_t numNonDefault,
                                     SSATmp* paramThis,
                                     bool inlining,
                                     bool wasInliningConstructor,
                                     bool destroyLocals,
                                     GetArg getArg) {
  // Collect the parameter locals---we'll need them later.  Also
  // determine which ones will need to be passed through the eval
  // stack.
  jit::vector<SSATmp*> paramSSAs(numArgs);
  jit::vector<bool> paramThroughStack(numArgs);
  jit::vector<bool> paramNeedsConversion(numArgs);
  auto numParamsThroughStack = uint32_t{0};
  for (auto i = uint32_t{0}; i < numArgs; ++i) {
    // Fill in paramSSAs in reverse, since they may come from popC's.
    auto const offset = numArgs - i - 1;
    paramSSAs[offset] = getArg(offset);

    auto const& pi = callee->params()[offset];
    switch (pi.builtinType) {
      case KindOfBoolean:
      case KindOfInt64:
      case KindOfDouble:
        paramThroughStack[offset] = false;
        break;
      default:
        ++numParamsThroughStack;
        paramThroughStack[offset] = true;
        break;
    }

    paramNeedsConversion[offset] = offset < numNonDefault
                                   && pi.builtinType != KindOfUnknown;
  }

  // For the same reason that we have to IncRef the locals above, we
  // need to grab one on the $this.
  if (paramThis) gen(IncRef, paramThis);

  if (inlining) emitEndInlinedCommon();   /////// leaving inlined function

  /*
   * Everything that needs to be on the stack gets spilled now.
   *
   * Note: right here we should eventually be handling the possibility
   * that we need to coerce parameters.  But right now any situation
   * requiring that is disabled in shouldIRInline.
   */
  if (numParamsThroughStack != 0 || !inlining) {
    for (auto i = uint32_t{0}; i < numArgs; ++i) {
      if (paramThroughStack[i]) {
        push(paramSSAs[i]);
      }
    }
    // We're going to do ldStackAddrs on these, so the stack must be
    // materialized:
    spillStack();
    // This marker update is to make sure rbx points to the bottom of
    // our stack when we enter our catch trace.  The catch trace
    // twiddles the VM registers directly on the execution context to
    // make the unwinder understand the situation, however.
    updateMarker();
  }

  /*
   * We have an unusual situation if we raise an exception:
   *
   * The unwinder is going to see our PC as equal to the FCallD for
   * the call to this NativeImpl instruction.  This means the PC will
   * be inside the FPI region for the call, so it'll try to pop an
   * ActRec.
   *
   * Meanwhile, we've just exited the inlined callee (and its frame
   * was hopefully removed by dce), and then pushed any of our
   * by-reference arguments on the eval stack.  So, if we throw, we
   * need to pop anything we pushed, put down a fake ActRec, and then
   * eagerly sync VM regs to represent that stack depth.
   */
  auto const makeUnusualCatch = [&] { return makeCatchImpl([&] {
        // TODO(#4323657): this is generating generic DecRefs at the time
        // of this writing---probably we're not handling the stack chain
        // correctly in a catch block.
        for (auto i = uint32_t{0}; i < numArgs; ++i) {
          if (paramThroughStack[i]) {
            popDecRef(Type::Gen);
          } else {
            gen(DecRef, paramSSAs[i]);
          }
        }
        if (inlining) {
          emitFPushActRec(cns(callee),
                          paramThis ? paramThis : cns(Type::Nullptr),
                          ActRec::encodeNumArgs(numArgs,
                                                false /* localsDecRefd */,
                                                false /* resumed */,
                                                wasInliningConstructor),
                          nullptr);
        }
        for (auto i = uint32_t{0}; i < numArgs; ++i) {
          // TODO(#4313939): it's not actually necessary to push these
          // nulls.
          push(cns(Type::InitNull));
        }
        auto const stack = spillStack();
        gen(SyncABIRegs, m_irb->fp(), stack);
        gen(EagerSyncVMRegs, m_irb->fp(), stack);
        return stack;
      }); };

  /*
   * Prepare the actual arguments to the CallBuiltin instruction.  If
   * any of the parameters need type conversions, we need to handle
   * that too.
   */
  auto const cbNumArgs = numArgs + (paramThis ? 1 : 0);
  SSATmp* args[cbNumArgs];
  {
    auto argIdx   = uint32_t{0};
    auto stackIdx = uint32_t{0};

    if (paramThis) args[argIdx++] = paramThis;
    for (auto i = uint32_t{0}; i < numArgs; ++i) {
      if (!paramThroughStack[i]) {
        if (paramNeedsConversion[i]) {
          auto const ty = Type(callee->params()[i].builtinType);
          auto const oldVal = paramSSAs[i];
          paramSSAs[i] = [&] {
            if (ty <= Type::Int) {
              return gen(ConvCellToInt, makeUnusualCatch(), oldVal);
            }
            if (ty <= Type::Dbl) {
              return gen(ConvCellToDbl, makeUnusualCatch(), oldVal);
            }
            always_assert(ty <= Type::Bool);  // or will be passed by stack
            return gen(ConvCellToBool, oldVal);
          }();
          gen(DecRef, oldVal);
        }
        args[argIdx++] = paramSSAs[i];
        continue;
      }

      auto const offset = numParamsThroughStack - stackIdx - 1;
      if (paramNeedsConversion[i]) {
        Type t(callee->params()[i].builtinType);
        if (callee->params()[i].builtinType == KindOfObject &&
            callee->methInfo()->parameters[i]->valueLen > 0) {
          t = Type::NullableObj;
        }
        gen(CastStk,
            makeUnusualCatch(),
            t,
            StackOffset { static_cast<int32_t>(offset) },
            m_irb->sp());
      }

      args[argIdx++] = ldStackAddr(offset, DataTypeSpecific);
      ++stackIdx;
    }

    assert(stackIdx == numParamsThroughStack);
    assert(argIdx == cbNumArgs);
  }

  // Make the actual call.
  auto const retType = [&] {
    auto const retDt = callee->returnType();
    auto const ret = retDt == KindOfUnknown ? Type::Cell : Type(retDt);
    return callee->attrs() & ClassInfo::IsReference ? ret.box() : ret;
  }();
  SSATmp** decayedPtr = &args[0];
  auto const ret = gen(
    CallBuiltin,
    retType,
    CallBuiltinData { callee, destroyLocals },
    makeUnusualCatch(),
    std::make_pair(cbNumArgs, decayedPtr)
  );

  // Pop the stack params and push the return value.
  if (paramThis) gen(DecRef, paramThis);
  for (auto i = uint32_t{0}; i < numParamsThroughStack; ++i) {
    popDecRef(Type::Gen);
  }
  push(ret);
}

void HhbcTranslator::emitEndInlinedCommon() {
  assert(!m_fpiActiveStack.empty());
  assert(!curFunc()->isPseudoMain());

  assert(!resumed());

  emitDecRefLocalsInline();

  if (curFunc()->mayHaveThis()) {
    gen(DecRefThis, m_irb->fp());
  }

  /*
   * Pop the ActRec and restore the stack and frame pointers.  It's
   * important that this does endInlining before pushing the return
   * value so stack offsets are properly tracked.
   */
  gen(InlineReturn, m_irb->fp());

  // Return to the caller function.  Careful between here and the
  // updateMarker() below, where the caller state isn't entirely set up.
  m_bcStateStack.pop_back();
  m_fpiActiveStack.pop();

  updateMarker();
  gen(
    ReDefSP,
    ReDefSPData {
      m_irb->spOffset(),
      m_irb->inlinedFrameSpansCall()
    },
    m_irb->sp(),
    m_irb->fp()
  );

  /*
   * After the end of inlining, we are restoring to a previously
   * defined stack that we know is entirely materialized (i.e. in
   * memory), so stackDeficit needs to be slammed to zero.
   *
   * The push of the return value in the caller of this function is
   * not yet materialized.
   */
  assert(m_irb->evalStack().numCells() == 0);
  m_irb->clearStackDeficit();

  FTRACE(1, "]]] end inlining: {}\n", curFunc()->fullName()->data());
}

/*
 * When we're inlining a NativeImpl opcode, we know this is the only
 * opcode in the callee method body (bytecode invariant).  So in
 * order to make sure we can eliminate the SpillFrame, we do the
 * CallBuiltin instruction after we've left the inlined frame.
 *
 * We may need to pass some arguments to the builtin through the
 * stack (e.g. if it takes const Variant&'s)---these are spilled to
 * the stack after leaving the callee.
 *
 * To make this work, we need to do some weird things in the catch
 * trace.  ;)
 */
void HhbcTranslator::emitNativeImplInlined() {
  auto const callee = curFunc();
  assert(callee->nativeFuncPtr());

  // Figure out if this inlined function was for an FPushCtor.  We'll
  // need this creating the unwind block blow.
  auto const wasInliningConstructor = [&]() -> bool {
    auto const sframe = findSpillFrame(m_irb->sp());
    assert(sframe);
    return sframe->extra<ActRecInfo>()->isFromFPushCtor();
  }();

  bool const instanceMethod = callee->isMethod() &&
                                !(callee->attrs() & AttrStatic);
  // Collect the parameter locals---we'll need them later.  Also
  // determine which ones will need to be passed through the eval
  // stack.
  auto const numArgs = callee->numParams();
  auto const paramThis = instanceMethod ? gen(LdThis, m_irb->fp())
                                        : nullptr;

  emitBuiltinCall(callee,
                  numArgs,
                  numArgs,  /* numNonDefault */
                  paramThis,
                  true,     /* inlining */
                  wasInliningConstructor,
                  false,    /* destroyLocals */
                  [&](uint32_t i) {
                    auto ret = ldLoc(i, nullptr, DataTypeSpecific);
                    gen(IncRef, ret);
                    return ret;
                  });
}

void HhbcTranslator::emitRetFromInlined(Type type) {
  auto const retVal = pop(type, DataTypeGeneric);
  // Before we leave the inlined frame, grab a type prediction from
  // our DefInlineFP.
  auto const retPred = m_irb->fp()->inst()->extra<DefInlineFP>()->retTypePred;
  emitEndInlinedCommon();
  push(retVal);
  if (retPred < retVal->type()) { // TODO: this if statement shouldn't
                                  // be here, because check type
                                  // resolves to the intersection of
                                  // the two types
    // If we had a predicted output type that's useful, check that here.
    checkTypeStack(0, retPred, curSrcKey().advanced().offset());
  }
}

void HhbcTranslator::emitDecRefLocalsInline() {
  for (int id = curFunc()->numLocals() - 1; id >= 0; --id) {
    gen(DecRefLoc, Type::Gen, LocalId(id), m_irb->fp());
  }
}

void HhbcTranslator::emitRet(Type type, bool freeInline) {
  auto const func = curFunc();
  if (func->attrs() & AttrMayUseVV) {
    // Note: this has to be the first thing, because we cannot bail after
    //       we start decRefing locs because then there'll be no corresponding
    //       bytecode boundaries until the end of RetC
    gen(ReleaseVVOrExit, makeExitSlow(), m_irb->fp());
  }

  // Pop the return value. Since it will be teleported to its place in memory,
  // we don't care about the type.
  auto catchBlock = makeCatch();
  SSATmp* retVal = pop(type, func->isGenerator() ? DataTypeSpecific
                                                 : DataTypeGeneric);

  // Free local variables.
  if (freeInline) {
    emitDecRefLocalsInline();
    for (unsigned i = 0; i < func->numLocals(); ++i) {
      m_irb->constrainLocal(i, DataTypeCountness, "inlined RetC/V");
    }
  } else {
    gen(GenericRetDecRefs, m_irb->fp());
  }

  // Free $this.
  if (func->mayHaveThis()) {
    gen(DecRefThis, m_irb->fp());
  }

  // Call the FunctionReturn hook and put the return value on the stack so that
  // the unwinder would decref it.
  emitRetSurpriseCheck(m_irb->fp(), retVal, catchBlock, false);

  // In async function, wrap the return value into succeeded StaticWaitHandle.
  if (!resumed() && func->isAsyncFunction()) {
    retVal = gen(CreateSSWH, retVal);
  }

  // Type profile return value.
  if (RuntimeOption::EvalRuntimeTypeProfile) {
    gen(TypeProfileFunc, TypeProfileData(-1), retVal, cns(func));
  }

  SSATmp* sp;
  SSATmp* resumableObj = nullptr;
  if (!resumed()) {
    // Store the return value.
    gen(StRetVal, m_irb->fp(), retVal);

    // Free ActRec.
    sp = gen(RetAdjustStack, m_irb->fp());
  } else if (func->isAsyncFunction()) {
    // Load the parent chain.
    auto parentChain = gen(LdAsyncArParentChain, m_irb->fp());

    // Mark the async function as succeeded.
    auto succeeded = c_WaitHandle::toKindState(
        c_WaitHandle::Kind::AsyncFunction, c_WaitHandle::STATE_SUCCEEDED);
    gen(StAsyncArRaw, RawMemData{RawMemData::AsyncState}, m_irb->fp(),
        cns(succeeded));

    // Store the return value.
    gen(StAsyncArResult, m_irb->fp(), retVal);

    // Unblock parents.
    gen(ABCUnblock, parentChain);

    // Sync SP.
    sp = spillStack();

    // Get the AsyncFunctionWaitHandle.
    resumableObj = gen(LdResumableArObj, m_irb->fp());
  } else if (func->isNonAsyncGenerator()) {
    // Clear generator's key and value.
    auto const oldKey = gen(LdContArKey, Type::Cell, m_irb->fp());
    gen(StContArKey, m_irb->fp(), cns(Type::InitNull));
    gen(DecRef, oldKey);

    auto const oldValue = gen(LdContArValue, Type::Cell, m_irb->fp());
    gen(StContArValue, m_irb->fp(), cns(Type::InitNull));
    gen(DecRef, oldValue);

    // Mark generator as finished.
    gen(StContArRaw, RawMemData{RawMemData::ContState}, m_irb->fp(),
        cns(BaseGenerator::State::Done));

    // Push return value of next()/send()/raise().
    push(cns(Type::InitNull));

    // Sync SP.
    sp = spillStack();
  } else {
    not_reached();
  }

  // Grab caller info from ActRec.
  SSATmp* retAddr = gen(LdRetAddr, m_irb->fp());
  SSATmp* fp = gen(FreeActRec, m_irb->fp());

  // Drop reference to this resumable. The reference to the object storing
  // the frame is implicitly owned by the execution. TakeRef is used to inform
  // the refcount optimizer about this fact.
  if (resumableObj != nullptr) {
    gen(TakeRef, resumableObj);
    gen(DecRef, resumableObj);
  }

  // Return control to the caller.
  gen(RetCtrl, RetCtrlData(false), sp, fp, retAddr);
}

void HhbcTranslator::emitRetC(bool freeInline) {
  if (curFunc()->isAsyncGenerator()) PUNT(RetC-AsyncGenerator);

  if (isInlining()) {
    assert(!resumed());
    emitRetFromInlined(Type::Cell);
  } else {
    emitRet(Type::Cell, freeInline);
  }
}

void HhbcTranslator::emitRetV(bool freeInline) {
  assert(!resumed());
  assert(!curFunc()->isResumable());
  if (isInlining()) {
    emitRetFromInlined(Type::BoxedCell);
  } else {
    emitRet(Type::BoxedCell, freeInline);
  }
}

void HhbcTranslator::emitJmpSurpriseCheck(Block* catchBlock) {
  m_irb->ifThen([&](Block* taken) {
                 gen(CheckSurpriseFlags, taken);
               },
               [&] {
                 m_irb->hint(Block::Hint::Unlikely);
                 gen(SurpriseHook, catchBlock);
               });
}

void HhbcTranslator::emitRetSurpriseCheck(SSATmp* fp, SSATmp* retVal,
                                          Block* catchBlock,
                                          bool suspendingResumed) {
  emitRB(Trace::RBTypeFuncExit, curFunc()->fullName());
  m_irb->ifThen([&](Block* taken) {
                 gen(CheckSurpriseFlags, taken);
               },
               [&] {
                 m_irb->hint(Block::Hint::Unlikely);
                 if (retVal != nullptr) {
                   gen(FunctionReturnHook, RetCtrlData(suspendingResumed),
                       catchBlock, fp, retVal);
                 } else {
                   gen(FunctionSuspendHook, RetCtrlData(suspendingResumed),
                       catchBlock, fp, cns(suspendingResumed));
                 }
               });
}

void HhbcTranslator::emitSwitch(const ImmVector& iv,
                                int64_t base,
                                bool bounded) {
  int nTargets = bounded ? iv.size() - 2 : iv.size();

  auto catchBlock = topC()->isA(Type::Obj) ? makeCatch() : nullptr;
  SSATmp* const switchVal = popC();
  Type type = switchVal->type();
  assert(IMPLIES(!(type <= Type::Int), bounded));
  assert(IMPLIES(bounded, iv.size() > 2));
  SSATmp* index;
  SSATmp* ssabase = cns(base);
  SSATmp* ssatargets = cns(nTargets);

  Offset defaultOff = bcOff() + iv.vec32()[iv.size() - 1];
  Offset zeroOff = 0;
  if (base <= 0 && (base + nTargets) > 0) {
    zeroOff = bcOff() + iv.vec32()[0 - base];
  } else {
    zeroOff = defaultOff;
  }

  if (type <= Type::Null) {
    gen(Jmp, makeExit(zeroOff));
    return;
  }
  if (type <= Type::Bool) {
    Offset nonZeroOff = bcOff() + iv.vec32()[iv.size() - 2];
    gen(JmpNZero, makeExit(nonZeroOff), switchVal);
    gen(Jmp, makeExit(zeroOff));
    return;
  }

  if (type <= Type::Int) {
    // No special treatment needed
    index = switchVal;
  } else if (type <= Type::Dbl) {
    // switch(Double|String|Obj)Helper do bounds-checking for us, so
    // we need to make sure the default case is in the jump table,
    // and don't emit our own bounds-checking code
    bounded = false;
    index = gen(LdSwitchDblIndex, switchVal, ssabase, ssatargets);
  } else if (type <= Type::Str) {
    bounded = false;
    index = gen(LdSwitchStrIndex, switchVal, ssabase, ssatargets);
  } else if (type <= Type::Obj) {
    // switchObjHelper can throw exceptions and reenter the VM so we use the
    // catch block here.
    bounded = false;
    index = gen(LdSwitchObjIndex, catchBlock, switchVal, ssabase, ssatargets);
  } else if (type <= Type::Arr) {
    gen(DecRef, switchVal);
    gen(Jmp, makeExit(defaultOff));
    return;
  } else {
    PUNT(Switch-UnknownType);
  }

  std::vector<Offset> targets(iv.size());
  for (int i = 0; i < iv.size(); i++) {
    targets[i] = bcOff() + iv.vec32()[i];
  }

  JmpSwitchData data;
  data.base        = base;
  data.bounded     = bounded;
  data.cases       = iv.size();
  data.defaultOff  = defaultOff;
  data.targets     = &targets[0];

  auto const stack = spillStack();
  gen(SyncABIRegs, m_irb->fp(), stack);

  gen(JmpSwitchDest, data, index);
}

void HhbcTranslator::emitSSwitch(const ImmVector& iv) {
  const int numCases = iv.size() - 1;

  /*
   * We use a fast path translation with a hashtable if none of the
   * cases are numeric strings and if the input is actually a string.
   *
   * Otherwise we do a linear search through the cases calling string
   * conversion routines.
   */
  const bool fastPath =
    topC()->isA(Type::Str) &&
    std::none_of(iv.strvec(), iv.strvec() + numCases,
      [&](const StrVecItem& item) {
        return curUnit()->lookupLitstrId(item.str)->isNumeric();
      }
    );

  Block* catchBlock = nullptr;
  // The slow path can throw exceptions and reenter the VM.
  if (!fastPath) catchBlock = makeCatch();

  auto const testVal = popC();

  std::vector<LdSSwitchData::Elm> cases(numCases);
  for (int i = 0; i < numCases; ++i) {
    auto const& kv = iv.strvec()[i];
    cases[i].str  = curUnit()->lookupLitstrId(kv.str);
    cases[i].dest = bcOff() + kv.dest;
  }

  LdSSwitchData data;
  data.numCases   = numCases;
  data.cases      = &cases[0];
  data.defaultOff = bcOff() + iv.strvec()[iv.size() - 1].dest;

  SSATmp* dest = gen(fastPath ? LdSSwitchDestFast
                              : LdSSwitchDestSlow,
                     catchBlock,
                     data,
                     testVal);
  gen(DecRef, testVal);
  auto const stack = spillStack();
  gen(SyncABIRegs, m_irb->fp(), stack);
  gen(JmpIndirect, dest);
}

void HhbcTranslator::setThisAvailable() {
  m_irb->setThisAvailable();
}

/*
 * Emit a type guard, possibly using profiling information. Depending on the
 * current translation mode and type to be guarded, this function may emit
 * additional profiling code or modify the guarded type using previously
 * collected profiling information. Str -> StaticStr is the only supported
 * refinement for now.
 *
 * type: The original guard type.
 * location, id: Name and index used in a profile key like "Loc3" or "Stk0".
 * doGuard: Lambda which will be called exactly once to emit the actual guard.
 * loadAddr: Lambda which will be called up to once to get a pointer to the
 *           value being checked.
 */
template<typename G, typename L>
void HhbcTranslator::emitProfiledGuard(Type type, const char* location,
                                       int32_t id, G doGuard, L loadAddr) {
  // We really do want to check for exact equality here: if type is StaticStr
  // there's nothing for us to do, and we don't support guarding on CountedStr.
  if (type != Type::Str ||
      (mcg->tx().mode() != TransKind::Profile &&
       mcg->tx().mode() != TransKind::Optimize)) {
    return doGuard(type);
  }

  auto profileKey = makeStaticString(folly::to<std::string>(location, id));
  TargetProfile<StrProfile> profile(m_context, m_irb->marker(), profileKey);
  if (profile.profiling()) {
    doGuard(Type::Str);
    auto addr = loadAddr();
    m_irb->constrainValue(addr, DataTypeSpecific);
    gen(ProfileStr, ProfileStrData(profileKey), addr);
  } else if (profile.optimizing()) {
    auto const data = profile.data(StrProfile::reduce);
    auto const total = data.total();

    if (data.staticStr == total) doGuard(Type::StaticStr);
    else                         doGuard(Type::Str);
  } else {
    doGuard(Type::Str);
  }
}

void HhbcTranslator::guardTypeLocal(uint32_t locId, Type type, bool outerOnly) {
  emitProfiledGuard(
    type, "Loc", locId,
    [&](Type type) { gen(GuardLoc, type, LocalId(locId), m_irb->fp()); },
    [&] { return gen(LdLocAddr, Type::PtrToStr, LocalId(locId),
                     m_irb->fp()); }
  );

  if (!outerOnly && type.isBoxed() && type.unbox() < Type::Cell) {
    auto const ldrefExit = makeExit();
    auto const ldgblExit = makePseudoMainExit();
    auto const val = ldLoc(locId, ldgblExit, DataTypeSpecific);
    gen(LdRef, type.unbox(), ldrefExit, val);
  }
}

void HhbcTranslator::guardTypeLocation(const RegionDesc::Location& loc,
                                       Type type, bool outerOnly) {
  assert(type <= Type::Gen);
  typedef RegionDesc::Location::Tag T;
  switch (loc.tag()) {
    case T::Stack: guardTypeStack(loc.stackOffset(), type, outerOnly); break;
    case T::Local: guardTypeLocal(loc.localId(),     type, outerOnly); break;
  }
}

void HhbcTranslator::checkTypeLocal(uint32_t locId, Type type,
                                    Offset dest /* = -1 */) {
  emitProfiledGuard(
    type, "Loc", locId,
    [&](Type type) {
      gen(CheckLoc, type, LocalId(locId), makeExit(dest), m_irb->fp());
    },
    [&] { return gen(LdLocAddr, Type::PtrToStr, LocalId(locId),
                     m_irb->fp()); }
  );
}

void HhbcTranslator::assertTypeLocal(uint32_t locId, Type type) {
  gen(AssertLoc, type, LocalId(locId), m_irb->fp());
}

void HhbcTranslator::checkType(const RegionDesc::Location& loc,
                               Type type, Offset dest) {
  assert(type <= Type::Gen);
  typedef RegionDesc::Location::Tag T;
  switch (loc.tag()) {
    case T::Stack: checkTypeStack(loc.stackOffset(), type, dest); break;
    case T::Local: checkTypeLocal(loc.localId(), type, dest);     break;
  }
}

void HhbcTranslator::assertType(const RegionDesc::Location& loc,
                                        Type type) {
  assert(type <= Type::StackElem);
  typedef RegionDesc::Location::Tag T;
  switch (loc.tag()) {
    case T::Stack: assertTypeStack(loc.stackOffset(), type); break;
    case T::Local: assertTypeLocal(loc.localId(), type);     break;
  }
}

void HhbcTranslator::guardTypeStack(uint32_t stackIndex, Type type,
                                    bool outerOnly) {
  assert(type <= Type::Gen);
  assert(m_irb->evalStack().size() == 0);
  // This should only be called at the beginning of a trace, with a
  // clean stack
  assert(m_irb->stackDeficit() == 0);
  auto stackOff = StackOffset(stackIndex);

  emitProfiledGuard(
    type, "Stk", stackIndex,
    [&](Type type) { gen(GuardStk, type, stackOff, m_irb->sp()); },
    [&] { return gen(LdStackAddr, Type::PtrToStr, stackOff, m_irb->sp()); }
  );

  if (!outerOnly && type.isBoxed() && type.unbox() < Type::Cell) {
    auto stk = gen(LdStack, Type::BoxedCell, stackOff, m_irb->sp());
    m_irb->constrainValue(stk, DataTypeSpecific);
    gen(LdRef, type.unbox(), makeExit(), stk);
  }
}

void HhbcTranslator::checkTypeStack(uint32_t idx, Type type, Offset dest) {
  assert(type <= Type::Gen);
  auto exit = makeExit(dest);
  if (idx < m_irb->evalStack().size()) {
    FTRACE(1, "checkTypeStack({}): generating CheckType for {}\n",
           idx, type.toString());
    // CheckType only cares about its input type if the simplifier does
    // something with it and that's handled if and when it happens.
    SSATmp* tmp = top(DataTypeGeneric, idx);
    assert(tmp);
    m_irb->evalStack().replace(idx, gen(CheckType, type, exit, tmp));
  } else {
    FTRACE(1, "checkTypeStack({}): no tmp: {}\n", idx, type.toString());
    // Just like CheckType, CheckStk only cares about its input type if the
    // simplifier does something with it.

    auto const adjustedOffset =
      StackOffset(idx - m_irb->evalStack().size() + m_irb->stackDeficit());
    emitProfiledGuard(
      type, "Stk", idx,
      [&](Type t) {
        gen(CheckStk, type, exit, adjustedOffset, m_irb->sp());
      },
      [&] {
        return gen(LdStackAddr, Type::PtrToStr, adjustedOffset, m_irb->sp());
      }
    );
  }
}

void HhbcTranslator::checkTypeTopOfStack(Type type, Offset nextByteCode) {
  checkTypeStack(0, type, nextByteCode);
}

void HhbcTranslator::assertTypeStack(uint32_t idx, Type type) {
  if (idx < m_irb->evalStack().size()) {
    // We're asserting a new type so we don't care about the previous type.
    SSATmp* tmp = top(DataTypeGeneric, idx);
    assert(tmp);
    m_irb->evalStack().replace(idx, gen(AssertType, type, tmp));
  } else {
    gen(AssertStk, type,
        StackOffset(idx - m_irb->evalStack().size() + m_irb->stackDeficit()),
        m_irb->sp());
  }
}

/*
 * Returns the Type of the given location. All accesses to the stack and locals
 * use DataTypeGeneric so this function should only be used for inspecting
 * state; when the values are actually used they must be constrained further.
 */
Type HhbcTranslator::typeFromLocation(const Location& loc) {
  switch (loc.space) {
    case Location::Stack: {
      auto i = loc.offset;
      assert(i >= 0);
      if (i < m_irb->evalStack().size()) {
        return top(DataTypeGeneric, i)->type();
      } else {
        auto stackVal =
          getStackValue(m_irb->sp(),
                        i - m_irb->evalStack().size() + m_irb->stackDeficit());
        return stackVal.knownType;
      }
    } break;
    case Location::Local: {
      auto l = loc.offset;
      return m_irb->localType(l, DataTypeGeneric);
    } break;
    case Location::Litstr:
      return Type::cns(curUnit()->lookupLitstrId(loc.offset));
    case Location::Litint:
      return Type::cns(loc.offset);
    case Location::This:
      return Type::Obj.specialize(curFunc()->cls());

    default:
      always_assert(false && "Bad location in typeFromLocation");
  }
}

static uint64_t packBitVec(const std::vector<bool>& bits, unsigned i) {
  uint64_t retval = 0;
  assert(i % 64 == 0);
  assert(i < bits.size());
  while (i < bits.size()) {
    retval |= bits[i] << (i % 64);
    if ((++i % 64) == 0) {
      break;
    }
  }
  return retval;
}

void HhbcTranslator::guardRefs(int64_t entryArDelta,
                               const std::vector<bool>& mask,
                               const std::vector<bool>& vals) {
  int32_t actRecOff = cellsToBytes(entryArDelta);
  SSATmp* funcPtr = gen(LdARFuncPtr, m_irb->sp(), cns(actRecOff));
  SSATmp* nParams = nullptr;

  for (unsigned i = 0; i < mask.size(); i += 64) {
    assert(i < vals.size());

    uint64_t mask64 = packBitVec(mask, i);
    if (mask64 == 0) {
      continue;
    }

    if (i == 0) {
      nParams = cns(64);
    } else if (i == 64) {
      nParams = gen(LdRaw, RawMemData{RawMemData::FuncNumParams}, funcPtr);
    }

    uint64_t vals64 = packBitVec(vals, i);
    gen(
      GuardRefs,
      funcPtr,
      nParams,
      cns(i),
      cns(mask64),
      cns(vals64)
    );
  }
}

void HhbcTranslator::endGuards() {
  gen(EndGuards);
}

void HhbcTranslator::emitVerifyTypeImpl(int32_t id) {
  bool isReturnType = (id == HPHP::TypeConstraint::ReturnId);
  if (isReturnType && !RuntimeOption::EvalCheckReturnTypeHints) return;

  auto const ldgblExit = makePseudoMainExit();
  auto func = curFunc();
  auto const& tc = isReturnType ? func->returnTypeConstraint()
                                : func->params()[id].typeConstraint;
  auto* val = isReturnType ? topR() : ldLoc(id, ldgblExit, DataTypeSpecific);
  assert(val->type().isBoxed() || val->type().notBoxed());
  if (val->type().isBoxed()) {
    val = gen(LdRef, val->type().innerType(), makeExit(), val);
    m_irb->constrainValue(val, DataTypeSpecific);
  }
  auto const valType = val->type();

  if (!valType.isKnownDataType()) {
    if (!isReturnType) {
      // This is supposed to be impossible, but it does happen in a rare case
      // with the legacy region selector. Until it's figured out, punt in
      // release builds. t3412704
      assert_log(false,
      [&] {
        return folly::format("Bad type {} for local {}:\n\n{}\n",
                             valType, id, m_irb->unit().toString()).str();
      });
    }
    emitInterpOne(0);
    return;
  }

  if (tc.isTypeVar()) return;
  if (tc.isNullable() && valType.subtypeOf(Type::InitNull)) return;

  if (!isReturnType && tc.isArray() && !tc.isSoft() && !func->mustBeRef(id) &&
      valType <= Type::Obj) {
    PUNT(VerifyParamType-collectionToArray);
    return;
  }
  if (tc.isCallable()) {
    if (isReturnType) {
      gen(VerifyRetCallable, makeCatch(), val);
    } else {
      gen(VerifyParamCallable, makeCatch(), val, cns(id));
    }
    return;
  }

  // For non-object guards, we rely on what we know from the tracelet
  // guards and never have to do runtime checks.
  if (!tc.isObjectOrTypeAlias()) {
    if (!tc.checkPrimitive(valType.toDataType())) {
      if (isReturnType) {
        gen(VerifyRetFail, makeCatch(), val);
      } else {
        gen(VerifyParamFail, makeCatch(), cns(id));
      }
    }
    return;
  }
  // If val is not an object, it still might pass the type constraint
  // if the constraint is a typedef. For now we just interp that case.
  auto const typeName = tc.typeName();
  if (valType <= Type::Arr && interface_supports_array(typeName)) {
    return;
  }
  if (valType <= Type::Str && interface_supports_string(typeName)) {
    return;
  }
  if (valType <= Type::Int && interface_supports_int(typeName)) {
    return;
  }
  if (valType <= Type::Dbl && interface_supports_double(typeName)) {
    return;
  }
  if (!(valType <= Type::Obj)) {
    if (tc.isObjectOrTypeAlias()
        && RuntimeOption::RepoAuthoritative
        && !tc.isCallable()
        && tc.isPrecise()) {
      auto const td = tc.namedEntity()->getCachedTypeAlias();
      if (tc.namedEntity()->isPersistentTypeAlias() && td) {
        if ((td->nullable && valType <= Type::Null)
            || td->kind == KindOfAny
            || equivDataTypes(td->kind, valType.toDataType())) {
          m_irb->constrainValue(val, TypeConstraint(DataTypeSpecific));
          return;
        }
      }
    }
    emitInterpOne(0);
    return;
  }

  const StringData* clsName;
  const Class* knownConstraint = nullptr;
  if (!tc.isSelf() && !tc.isParent()) {
    clsName = tc.typeName();
    knownConstraint = Unit::lookupClass(clsName);
  } else {
    if (tc.isSelf()) {
      tc.selfToClass(curFunc(), &knownConstraint);
    } else if (tc.isParent()) {
      tc.parentToClass(curFunc(), &knownConstraint);
    }
    if (knownConstraint) {
      clsName = knownConstraint->preClass()->name();
    } else {
      // The hint was self or parent and there's no corresponding
      // class for the current func. This typehint will always fail.
      if (isReturnType) {
        gen(VerifyRetFail, makeCatch(), val);
      } else {
        gen(VerifyParamFail, makeCatch(), cns(id));
      }
      return;
    }
  }
  assert(clsName);

  // We can only burn in the Class* if it's unique or in the
  // inheritance hierarchy of our context. It's ok if the class isn't
  // defined yet - all paths below are tolerant of a null constraint.
  if (!classIsUniqueOrCtxParent(knownConstraint)) knownConstraint = nullptr;

  /*
   * If the local is a specialized object type and we don't have to constrain a
   * guard to get it, we can avoid emitting runtime checks if we know the thing
   * would pass. If we don't know, we still have to emit them because valType
   * might be a subtype of its specialized object type.
   */
  if (valType < Type::Obj) {
    auto const cls = valType.getClass();
    if (!m_irb->constrainValue(val, TypeConstraint(cls).setWeak()) &&
        ((knownConstraint && cls->classof(knownConstraint)) ||
         cls->name()->isame(clsName))) {
      return;
    }
  }

  InstanceBits::init();
  bool haveBit = InstanceBits::lookup(clsName) != 0;
  SSATmp* constraint = knownConstraint ? cns(knownConstraint)
                                       : gen(LdClsCachedSafe, cns(clsName));
  SSATmp* objClass = gen(LdObjClass, val);
  if (haveBit || classIsUniqueNormalClass(knownConstraint)) {
    SSATmp* isInstance = haveBit
      ? gen(InstanceOfBitmask, objClass, cns(clsName))
      : gen(ExtendsClass, objClass, constraint);
    m_irb->ifThen([&](Block* taken) {
        gen(JmpZero, taken, isInstance);
      },
      [&] { // taken: the param type does not match
        m_irb->hint(Block::Hint::Unlikely);
        if (isReturnType) {
          gen(VerifyRetFail, makeCatch(), val);
        } else {
          gen(VerifyParamFail, makeCatch(), cns(id));
        }
      }
    );
  } else {
    if (isReturnType) {
      gen(VerifyRetCls, makeCatch(), objClass, constraint,
          cns(uintptr_t(&tc)), val);
    } else {
      gen(VerifyParamCls, makeCatch(), objClass, constraint,
          cns(uintptr_t(&tc)), cns(id));
    }
  }
}

void HhbcTranslator::emitVerifyRetTypeC() {
  emitVerifyTypeImpl(HPHP::TypeConstraint::ReturnId);
}

void HhbcTranslator::emitVerifyRetTypeV() {
  emitVerifyTypeImpl(HPHP::TypeConstraint::ReturnId);
}

void HhbcTranslator::emitVerifyParamType(int32_t paramId) {
  emitVerifyTypeImpl(paramId);
}

const StaticString s_WaitHandle("WaitHandle");

void HhbcTranslator::emitInstanceOfD(int classNameStrId) {
  const StringData* className = lookupStringId(classNameStrId);
  SSATmp* src = popC();

  /*
   * InstanceOfD is always false if it's not an object.
   *
   * We're prepared to generate translations for known non-object
   * types, but if it's Gen/Cell we're going to PUNT because it's
   * natural to translate that case with control flow TODO(#2020251)
   */
  if (Type::Obj.strictSubtypeOf(src->type())) {
    PUNT(InstanceOfD_MaybeObj);
  }
  if (!src->isA(Type::Obj)) {
    bool res = ((src->isA(Type::Arr) && interface_supports_array(className))) ||
      (src->isA(Type::Str) && interface_supports_string(className)) ||
      (src->isA(Type::Int) && interface_supports_int(className)) ||
      (src->isA(Type::Dbl) && interface_supports_double(className));
    push(cns(res));
    gen(DecRef, src);
    return;
  }

  if (s_WaitHandle.get()->isame(className)) {
    push(gen(IsWaitHandle, src));
    gen(DecRef, src);
    return;
  }

  SSATmp* objClass     = gen(LdObjClass, src);
  SSATmp* ssaClassName = cns(className);

  InstanceBits::init();
  const bool haveBit = InstanceBits::lookup(className) != 0;

  Class* const maybeCls = Unit::lookupUniqueClass(className);
  const bool isNormalClass = classIsUniqueNormalClass(maybeCls);
  const bool isUnique = classIsUnique(maybeCls);

  /*
   * If the class is a unique interface, we can just hit the class's
   * interfaces map and call it a day.
   */
  if (!haveBit && classIsUniqueInterface(maybeCls)) {
    push(gen(InstanceOfIface, objClass, ssaClassName));
    gen(DecRef, src);
    return;
  }

  /*
   * If the class is unique or a parent of the current context, we
   * don't need to load it out of RDS because it must already exist
   * and be defined.
   *
   * Otherwise, we only use LdClsCachedSafe---instanceof with an
   * undefined class doesn't invoke autoload.
   */
  SSATmp* checkClass =
    isUnique || (maybeCls && curClass() && curClass()->classof(maybeCls))
      ? cns(maybeCls)
      : gen(LdClsCachedSafe, ssaClassName);

  push(
      haveBit ? gen(InstanceOfBitmask, objClass, ssaClassName)
    : isUnique && isNormalClass ? gen(ExtendsClass, objClass, checkClass)
    : gen(InstanceOf, objClass, checkClass)
  );
  gen(DecRef, src);
}

void HhbcTranslator::emitInstanceOf() {
  auto const t1 = popC();
  auto const t2 = popC(); // t2 instanceof t1

  if (t1->isA(Type::Obj) && t2->isA(Type::Obj)) {
    auto const c2 = gen(LdObjClass, t2);
    auto const c1 = gen(LdObjClass, t1);
    push(gen(InstanceOf, c2, c1));
    gen(DecRef, t2);
    gen(DecRef, t1);
    return;
  }

  if (!t1->isA(Type::Str)) PUNT(InstanceOf-NotStr);

  if (t2->isA(Type::Obj)) {
    auto const rds = gen(LookupClsRDSHandle, t1);
    auto const c1  = gen(DerefClsRDSHandle, rds);
    auto const c2  = gen(LdObjClass, t2);
    push(gen(InstanceOf, c2, c1));
    gen(DecRef, t2);
    gen(DecRef, t1);
    return;
  }

  push(
    t2->isA(Type::Arr) ? gen(InterfaceSupportsArr, t1) :
    t2->isA(Type::Int) ? gen(InterfaceSupportsInt, t1) :
    t2->isA(Type::Str) ? gen(InterfaceSupportsStr, t1) :
    t2->isA(Type::Dbl) ? gen(InterfaceSupportsDbl, t1) :
    cns(false)
  );
  gen(DecRef, t2);
  gen(DecRef, t1);
}

void HhbcTranslator::emitCastArray() {
  // Turns the castArray BC operation into a type specialized
  // IR operation. The IR operation might end up being simplified
  // into a constant, but if not, it simply turns into a helper
  // call when translated to machine code. The main benefit from
  // separate IR instructions is that they can have different flags,
  // principally to distinguish the instructions that (may) hold on to a
  // reference to argument, from instructions that do not.

  // In the future, if this instruction occurs in a hot trace,
  // it might be better to expand it into a series of primitive
  // IR instructions so that the object allocation is exposed to
  // the optimizer and becomes eligible for removal if it does not
  // escape the trace.

  auto catchBlock = makeCatch();
  SSATmp* src = popC();
  if (src->isA(Type::Arr)) {
    push(src);
  } else if (src->isA(Type::Null)) {
    push(cns(staticEmptyArray()));
  } else if (src->isA(Type::Bool)) {
    push(gen(ConvBoolToArr, src));
  } else if (src->isA(Type::Dbl)) {
    push(gen(ConvDblToArr, src));
  } else if (src->isA(Type::Int)) {
    push(gen(ConvIntToArr, src));
  } else if (src->isA(Type::Str)) {
    push(gen(ConvStrToArr, src));
  } else if (src->isA(Type::Obj)) {
    push(gen(ConvObjToArr, catchBlock, src));
  } else {
    push(gen(ConvCellToArr, catchBlock, src));
  }
}

void HhbcTranslator::emitCastBool() {
  auto const src = popC();
  push(gen(ConvCellToBool, src));
  gen(DecRef, src);
}

void HhbcTranslator::emitCastDouble() {
  auto const catchBlock = makeCatch();
  auto const src = popC();
  push(gen(ConvCellToDbl, catchBlock, src));
  gen(DecRef, src);
}

void HhbcTranslator::emitCastInt() {
  auto const catchBlock = makeCatch();
  auto const src = popC();
  push(gen(ConvCellToInt, catchBlock, src));
  gen(DecRef, src);
}

void HhbcTranslator::emitCastObject() {
  auto catchBlock = makeCatch();
  SSATmp* src = popC();
  push(gen(ConvCellToObj, catchBlock, src));
}

void HhbcTranslator::emitCastString() {
  auto const catchBlock = makeCatch();
  auto const src = popC();
  push(gen(ConvCellToStr, catchBlock, src));
  gen(DecRef, src);
}

static bool isSupportedAGet(SSATmp* classSrc) {
  return (classSrc->isA(Type::Obj) || classSrc->isA(Type::Str));
}

void HhbcTranslator::emitAGet(SSATmp* classSrc, Block* catchBlock) {
  if (classSrc->isA(Type::Str)) {
    push(gen(LdCls, catchBlock, classSrc, cns(curClass())));
  } else if (classSrc->isA(Type::Obj)) {
    push(gen(LdObjClass, classSrc));
  } else {
    not_reached();
  }
}

void HhbcTranslator::emitAGetC() {
  auto const name = topC();
  if (isSupportedAGet(name)) {
    auto catchBlock = makeCatch();
    popC();
    emitAGet(name, catchBlock);
    gen(DecRef, name);
  } else {
    emitInterpOne(Type::Cls, 1);
  }
}

void HhbcTranslator::emitAGetL(int id) {
  auto const ldrefExit = makeExit();
  auto const ldgblExit = makePseudoMainExit();

  auto const src = ldLocInner(id, ldrefExit, ldgblExit, DataTypeSpecific);
  if (isSupportedAGet(src)) {
    emitAGet(src, makeCatch());
  } else {
    PUNT(AGetL); // need to teach interpone about local uses
  }
}

void HhbcTranslator::emitBindMem(SSATmp* ptr, SSATmp* src) {
  SSATmp* prevValue = gen(LdMem, ptr->type().deref(), ptr, cns(0));

  pushIncRef(src);
  gen(StMem, ptr, cns(0), src);
  gen(DecRef, prevValue);
}

void HhbcTranslator::emitEmptyMem(SSATmp* ptr) {
  SSATmp* ld = gen(LdMem, Type::Cell, gen(UnboxPtr, ptr), cns(0));
  push(gen(XorBool, gen(ConvCellToBool, ld), cns(true)));
}

void HhbcTranslator::destroyName(SSATmp* name) {
  assert(name == topC());
  popDecRef(name->type());
}

SSATmp* HhbcTranslator::ldClsPropAddrKnown(Block* catchBlock,
                                           SSATmp* ssaCls,
                                           SSATmp* ssaName) {
  auto const cls = ssaCls->clsVal();

  auto const repoTy = [&] {
    if (!RuntimeOption::RepoAuthoritative) return RepoAuthType{};
    auto const slot = cls->lookupSProp(ssaName->strVal());
    return cls->staticPropRepoAuthType(slot);
  }();

  auto const ptrTy = convertToType(repoTy).ptr();

  emitInitSProps(cls, catchBlock);
  return gen(LdClsPropAddrKnown, ptrTy, ssaCls, ssaName);
}

SSATmp* HhbcTranslator::ldClsPropAddr(Block* catchBlock,
                                      SSATmp* ssaCls,
                                      SSATmp* ssaName,
                                      bool raise) {
  /*
   * We can use LdClsPropAddrKnown if either we know which property it is and
   * that it is visible && accessible, or we know it is a property on this
   * class itself.
   */
  bool const sPropKnown = [&] {
    if (!ssaName->isConst()) return false;
    auto const propName = ssaName->strVal();

    if (!ssaCls->isConst()) return false;
    auto const cls = ssaCls->clsVal();
    if (!classIsPersistentOrCtxParent(cls)) return false;

    bool visible, accessible;
    cls->findSProp(curClass(), propName, visible, accessible);
    return visible && accessible;
  }();

  if (sPropKnown) {
    return ldClsPropAddrKnown(catchBlock, ssaCls, ssaName);
  }

  if (raise) {
    return gen(LdClsPropAddrOrRaise, catchBlock,
               ssaCls, ssaName, cns(curClass()));
  } else {
    return gen(LdClsPropAddrOrNull, catchBlock,
               ssaCls, ssaName, cns(curClass()));
  }
}

void HhbcTranslator::emitCGetS() {
  auto const catchBlock  = makeCatch();
  auto const ssaPropName = topC(1);

  if (!ssaPropName->isA(Type::Str)) {
    PUNT(CGetS-PropNameNotString);
  }

  auto const ssaCls   = popA();
  auto const propAddr = ldClsPropAddr(catchBlock, ssaCls, ssaPropName, true);
  auto const unboxed  = gen(UnboxPtr, propAddr);
  auto const ldMem    = gen(LdMem, unboxed->type().deref(), unboxed, cns(0));

  destroyName(ssaPropName);
  pushIncRef(ldMem);
}

void HhbcTranslator::emitSetS() {
  auto const catchBlock  = makeCatch();
  auto const ssaPropName = topC(2);

  if (!ssaPropName->isA(Type::Str)) {
    PUNT(SetS-PropNameNotString);
  }

  auto const value    = popC(DataTypeCountness);
  auto const ssaCls   = popA();
  auto const propAddr = ldClsPropAddr(catchBlock, ssaCls, ssaPropName, true);
  auto const ptr      = gen(UnboxPtr, propAddr);

  destroyName(ssaPropName);
  emitBindMem(ptr, value);
}

void HhbcTranslator::emitVGetS() {
  auto const catchBlock  = makeCatch();
  auto const ssaPropName = topC(1);

  if (!ssaPropName->isA(Type::Str)) {
    PUNT(VGetS-PropNameNotString);
  }

  auto const ssaCls   = popA();
  auto const propAddr = ldClsPropAddr(catchBlock, ssaCls, ssaPropName, true);

  destroyName(ssaPropName);
  pushIncRef(gen(LdMem, Type::BoxedCell, gen(BoxPtr, propAddr), cns(0)));
}

void HhbcTranslator::emitBindS() {
  auto const catchBlock  = makeCatch();
  auto const ssaPropName = topC(2);

  if (!ssaPropName->isA(Type::Str)) {
    PUNT(BindS-PropNameNotString);
  }

  auto const value    = popV();
  auto const ssaCls   = popA();
  auto const propAddr = ldClsPropAddr(catchBlock, ssaCls, ssaPropName, true);

  destroyName(ssaPropName);
  emitBindMem(propAddr, value);
}

void HhbcTranslator::emitIssetS() {
  auto const catchBlock  = makeCatch();

  auto const ssaPropName = topC(1);
  if (!ssaPropName->isA(Type::Str)) {
    PUNT(IssetS-PropNameNotString);
  }
  auto const ssaCls = popA();

  auto const ret = m_irb->cond(
    0,
    [&] (Block* taken) {
      auto propAddr = ldClsPropAddr(catchBlock, ssaCls, ssaPropName, false);
      return gen(CheckNonNull, taken, propAddr);
    },
    [&] (SSATmp* ptr) { // Next: property or global exists
      return gen(IsNTypeMem, Type::Null, gen(UnboxPtr, ptr));
    },
    [&] { // Taken: LdClsPropAddr* returned Nullptr because it isn't defined
      return cns(false);
    });

  destroyName(ssaPropName);
  push(ret);
}

void HhbcTranslator::emitEmptyS() {
  auto const catchBlock  = makeCatch();

  auto const ssaPropName = topC(1);
  if (!ssaPropName->isA(Type::Str)) {
    PUNT(EmptyS-PropNameNotString);
  }

  auto const ssaCls = popA();
  auto const ret = m_irb->cond(
    0,
    [&] (Block* taken) {
      auto propAddr = ldClsPropAddr(catchBlock, ssaCls, ssaPropName, false);
      return gen(CheckNonNull, taken, propAddr);
    },
    [&] (SSATmp* ptr) {
      auto const unbox = gen(UnboxPtr, ptr);
      auto const val   = gen(LdMem, unbox->type().deref(), unbox, cns(0));
      return gen(XorBool, gen(ConvCellToBool, val), cns(true));
    },
    [&] { // Taken: LdClsPropAddr* returned Nullptr because it isn't defined
      return cns(true);
    });

  destroyName(ssaPropName);
  push(ret);
}

void HhbcTranslator::emitCGetG() {
  auto const exit = makeExitSlow();
  auto const name = topC();
  if (!name->isA(Type::Str)) PUNT(CGetG-NonStrName);
  auto const ptr = gen(LdGblAddr, exit, name);
  destroyName(name);
  pushIncRef(gen(LdMem, Type::Cell, gen(UnboxPtr, ptr), cns(0)));
}

void HhbcTranslator::emitVGetG() {
  auto const name = topC();
  if (!name->isA(Type::Str)) PUNT(VGetG-NonStrName);
  auto const ptr = gen(LdGblAddrDef, name);
  destroyName(name);
  pushIncRef(gen(LdMem, Type::BoxedCell, gen(BoxPtr, ptr), cns(0)));
}

void HhbcTranslator::emitBindG() {
  auto const name = topC(1);
  if (!name->isA(Type::Str)) PUNT(BindG-NameNotStr);
  auto const box = popV();
  auto const ptr = gen(LdGblAddrDef, name);
  destroyName(name);
  emitBindMem(ptr, box);
}

void HhbcTranslator::emitSetG() {
  auto const name = topC(1);
  if (!name->isA(Type::Str)) PUNT(SetG-NameNotStr);
  auto const value   = popC(DataTypeCountness);
  auto const unboxed = gen(UnboxPtr, gen(LdGblAddrDef, name));
  destroyName(name);
  emitBindMem(unboxed, value);
}

void HhbcTranslator::emitIssetG() {
  auto const name = topC(0);
  if (!name->isA(Type::Str)) PUNT(IssetG-NameNotStr);

  auto const ret = m_irb->cond(
    0,
    [&] (Block* taken) {
      return gen(LdGblAddr, taken, name);
    },
    [&] (SSATmp* ptr) { // Next: global exists
      return gen(IsNTypeMem, Type::Null, gen(UnboxPtr, ptr));
    },
    [&] { // Taken: global doesn't exist
      return cns(false);
    });
  destroyName(name);
  push(ret);
}

void HhbcTranslator::emitEmptyG() {
  auto const name = topC();
  if (!name->isA(Type::Str)) PUNT(EmptyG-NameNotStr);

  auto const ret = m_irb->cond(
    0,
    [&] (Block* taken) {
      return gen(LdGblAddr, taken, name);
    },
    [&] (SSATmp* ptr) { // Next: global exists
      auto const unboxed = gen(UnboxPtr, ptr);
      auto const val     = gen(LdMem, Type::Cell, unboxed, cns(0));
      return gen(XorBool, gen(ConvCellToBool, val), cns(true));
    },
    [&] { // Taken: global doesn't exist
      return cns(true);
    });
  destroyName(name);
  push(ret);
}

void HhbcTranslator::emitBinaryBitOp(Op op) {
  Type type2 = topC(0)->type();
  Type type1 = topC(1)->type();

  if (!areBinaryArithTypesSupported(op, type1, type2)) {
    PUNT(BunaryBitOp-Unsupported);
    return;
  }

  SSATmp* src2 = promoteBool(popC());
  SSATmp* src1 = promoteBool(popC());
  push(gen(bitOp(op), src1, src2));
}

void HhbcTranslator::emitBinaryArith(Op op) {
  Type type2 = topC(0)->type();
  Type type1 = topC(1)->type();

  if (!areBinaryArithTypesSupported(op, type1, type2)) {
    // either an int or a dbl, but can't tell
    PUNT(BinaryArith-Unsupported);
    return;
  }

  auto spillValues = peekSpillValues();
  SSATmp* src2 = promoteBool(popC());
  SSATmp* src1 = promoteBool(popC());
  Opcode opc = promoteBinaryDoubles(op, src1, src2);

  if (opc == AddIntO || opc == SubIntO || opc == MulIntO) {
    assert(src1->isA(Type::Int) && src2->isA(Type::Int));

    auto const exit = makeExitImpl(
      bcOff(),
      ExitFlag::Interp,
      spillValues,
      CustomExit{}
    );

    push(gen(opc, exit, src1, src2));
  } else {
    push(gen(opc, src1, src2));
  }
}

void HhbcTranslator::emitNot() {
  SSATmp* src = popC();
  push(gen(XorBool, gen(ConvCellToBool, src), cns(true)));
  gen(DecRef, src);
}

void HhbcTranslator::emitFloor() {
  // need SSE 4.1 support to use roundsd
  if (!folly::CpuId().sse41()) {
    PUNT(Floor);
  }

  auto catchBlock = makeCatch();
  auto val    = popC();
  auto dblVal = gen(ConvCellToDbl, catchBlock, val);
  gen(DecRef, val);
  push(gen(Floor, dblVal));
}

void HhbcTranslator::emitCeil() {
  // need SSE 4.1 support to use roundsd
  if (!folly::CpuId().sse41()) {
    PUNT(Ceil);
  }

  auto catchBlock = makeCatch();
  auto val = popC();
  auto dblVal = gen(ConvCellToDbl, catchBlock, val);
  gen(DecRef, val);
  push(gen(Ceil, dblVal));
}

void HhbcTranslator::emitCheckProp(Id propId) {
  StringData* propName = lookupStringId(propId);

  auto* cctx = gen(LdCctx, m_irb->fp());
  auto* cls = gen(LdClsCtx, cctx);
  auto* propInitVec = gen(LdClsInitData, cls);

  auto* ctx = curClass();
  auto idx = ctx->lookupDeclProp(propName);

  auto* curVal = gen(LdElem, propInitVec, cns(idx * sizeof(TypedValue)));
  push(gen(IsNType, Type::Uninit, curVal));
}

void HhbcTranslator::emitInitProp(Id propId, InitPropOp op) {
  StringData* propName = lookupStringId(propId);
  SSATmp* val = popC();

  auto* ctx = curClass();

  SSATmp* base;
  Slot idx = 0;

  switch(op) {
    case InitPropOp::Static:
      // For sinit, the context class is always the same as the late-bound
      // class, so we can just use curClass().
      base = gen(LdClsPropAddrKnown, Type::PtrToCell, cns(ctx), cns(propName));
      break;

    case InitPropOp::NonStatic: {
      // The above is not the case for pinit, so we need to load.
      auto* cctx = gen(LdCctx, m_irb->fp());
      auto* cls = gen(LdClsCtx, cctx);

      base = gen(LdClsInitData, cls);
      idx = ctx->lookupDeclProp(propName);
    } break;
  }

  gen(StElem, base, cns(idx * sizeof(TypedValue)), val);
}

void HhbcTranslator::emitSilence(Id localId, unsigned char ucsubop) {
  SilenceOp subop = static_cast<SilenceOp>(ucsubop);
  switch (subop) {
    case SilenceOp::Start: {
      // We assume that whatever is in the local is dead and doesn't need to be
      // refcounted before being overwritten.
      gen(AssertLoc, Type::Uncounted, LocalId(localId), m_irb->fp());
      auto level = gen(ZeroErrorLevel);
      gen(StLoc, LocalId(localId), m_irb->fp(), level);
      break;
    }
    case SilenceOp::End: {
      gen(AssertLoc, Type::Int, LocalId(localId), m_irb->fp());
      auto level = ldLoc(localId, makeExit(), DataTypeGeneric);
      gen(RestoreErrorLevel, level);
      break;
    }
  }
}


/*
 * Note: this is currently separate from convertToType(RepoAuthType)
 * for now, just because we don't want to enable every single type for
 * assertions yet.
 *
 * (Some of them currently regress performance, presumably because the
 * IR doesn't always handle the additional type information very well.
 * It is possibly a compile-time slowdown only, but we haven't
 * investigated yet.)
 */
folly::Optional<Type> HhbcTranslator::ratToAssertType(RepoAuthType rat) const {
  using T = RepoAuthType::Tag;
  switch (rat.tag()) {
  case T::Uninit:     return Type::Uninit;
  case T::InitNull:   return Type::InitNull;
  case T::Int:        return Type::Int;
  case T::Dbl:        return Type::Dbl;
  case T::Res:        return Type::Res;
  case T::Null:       return Type::Null;
  case T::Bool:       return Type::Bool;
  case T::Str:        return Type::Str;
  case T::Obj:        return Type::Obj;
  case T::SStr:       return Type::StaticStr;

  // These aren't enabled yet:
  case T::OptInt:
  case T::OptObj:
  case T::OptDbl:
  case T::OptBool:
  case T::OptSStr:
  case T::OptStr:
  case T::OptRes:
    return folly::none;

  case T::OptSArr:
  case T::OptArr:
    // TODO(#4205897): optional array types.
    return folly::none;

  case T::SArr:
    if (auto const arr = rat.array()) {
      return Type::StaticArr.specialize(arr);
    }
    return Type::StaticArr;
  case T::Arr:
    if (auto const arr = rat.array()) {
      return Type::Arr.specialize(arr);
    }
    return Type::Arr;

  case T::OptExactObj:
  case T::OptSubObj:
  case T::ExactObj:
  case T::SubObj:
    {
      auto ty = Type::Obj;
      auto const cls = Unit::lookupUniqueClass(rat.clsName());
      if (classIsUniqueOrCtxParent(cls)) {
        if (rat.tag() == T::OptExactObj || rat.tag() == T::ExactObj) {
          ty = ty.specializeExact(cls);
        } else {
          ty = ty.specialize(cls);
        }
      }
      if (rat.tag() == T::OptExactObj || rat.tag() == T::OptSubObj) {
        ty = ty | Type::InitNull;
      }
      return ty;
    }

  // We always know this at JIT time right now.
  case T::Cell:
  case T::Ref:
    return folly::none;

  case T::InitGen:
    // Should ideally be able to remove Uninit here.
    return folly::none;
  case T::Gen:
    return folly::none;

  // The JIT can't currently handle the exact information in these
  // type assertions in some cases:
  case T::InitUnc:    return folly::none;
  case T::Unc:        return folly::none;
  case T::InitCell:   return Type::Cell; // - Type::Uninit
  }
  not_reached();
}

void HhbcTranslator::emitAssertRATL(int32_t loc, RepoAuthType rat) {
  if (auto const t = ratToAssertType(rat)) {
    assertTypeLocal(loc, *t);
  }
}

void HhbcTranslator::emitAssertRATStk(int32_t offset, RepoAuthType rat) {
  if (auto const t = ratToAssertType(rat)) {
    assertTypeStack(offset, *t);
  }
}

void HhbcTranslator::emitAbs() {
  auto value = popC();

  if (value->isA(Type::Int)) {
    // compute integer absolute value ((src>>63) ^ src) - (src>>63)
    auto t1 = gen(Shr, value, cns(63));
    auto t2 = gen(XorInt, t1, value);
    push(gen(SubInt, t2, t1));
    return;
  }

  if (value->isA(Type::Dbl)) {
    push(gen(AbsDbl, value));
    return;
  }

  if (value->isA(Type::Arr)) {
    gen(DecRef, value);
    push(cns(false));
    return;
  }

  PUNT(Abs);
}

#define AOP(OP, OPI, OPD) \
  void HhbcTranslator::emit ## OP() { emitBinaryArith(Op::OP); }
BINARY_ARITH
#undef AOP

#define BOP(OP, OPI) \
  void HhbcTranslator::emit ## OP() { emitBinaryBitOp(Op::OP); }
BINARY_BITOP
#undef BOP

void HhbcTranslator::emitDiv() {
  auto divisorType  = topC(0)->type();
  auto dividendType = topC(1)->type();

  auto isNumeric = [&] (Type type) {
    return type.subtypeOfAny(Type::Int, Type::Dbl, Type::Bool);
  };

  // not going to bother with string division etc.
  if (!isNumeric(divisorType) || !isNumeric(dividendType)) {
    emitInterpOne(Type::UncountedInit, 2);
    return;
  }

  auto divisor  = topC(0);
  auto dividend = topC(1);

  // we can't codegen this but we may be able to special case it away
  if (!divisor->isA(Type::Dbl) && !dividend->isA(Type::Dbl)) {
    // TODO(#2570625): support integer-integer division, move this to simlifier:
    if (divisor->isConst()) {
      int64_t divisorVal;
      if (divisor->isA(Type::Int)) {
        divisorVal = divisor->intVal();
      } else {
        assert(divisor->isA(Type::Bool));
        divisorVal = divisor->boolVal();
      }

      if (divisorVal == 0) {
        auto catchBlock = makeCatch();
        popC();
        popC();
        gen(RaiseWarning, catchBlock,
            cns(makeStaticString(Strings::DIVISION_BY_ZERO)));
        push(cns(false));
        return;
      }

      if (dividend->isConst()) {
        int64_t dividendVal;
        if (dividend->isA(Type::Int)) {
          dividendVal = dividend->intVal();
        } else {
          assert(dividend->isA(Type::Bool));
          dividendVal = dividend->boolVal();
        }
        popC();
        popC();
        if (dividendVal == LLONG_MIN || dividendVal % divisorVal) {
          push(cns((double)dividendVal / divisorVal));
        } else {
          push(cns(dividendVal / divisorVal));
        }
        return;
      }
      /* fall through */
    }
    emitInterpOne(Type::UncountedInit, 2);
    return;
  }

  auto make_double = [&] (SSATmp* src) {
    if (src->isA(Type::Int)) {
      return gen(ConvIntToDbl, src);
    } else if (src->isA(Type::Bool)) {
      return gen(ConvBoolToDbl, src);
    }
    assert(src->isA(Type::Dbl));
    return src;
  };

  divisor  = make_double(popC());
  dividend = make_double(popC());

  // on division by zero we spill false and exit with a warning
  auto exitSpillValues = peekSpillValues();
  exitSpillValues.push_back(cns(false));

  auto const exit = makeExitWarn(nextBcOff(), exitSpillValues,
                                 makeStaticString(Strings::DIVISION_BY_ZERO));

  assert(divisor->isA(Type::Dbl) && dividend->isA(Type::Dbl));
  push(gen(DivDbl, exit, dividend, divisor));
}

void HhbcTranslator::emitMod() {
  auto catchBlock1 = makeCatch();
  auto catchBlock2 = makeCatch();
  SSATmp* btr = popC();
  SSATmp* btl = popC();
  SSATmp* tr = gen(ConvCellToInt, catchBlock1, btr);
  SSATmp* tl = gen(ConvCellToInt, catchBlock2, btl);

  // We only want to decref btr and btl if the ConvCellToInt operation gave us
  // a new value back.
  if (tr != btr) gen(DecRef, btr);
  if (tl != btl) gen(DecRef, btl);
  // Exit path spills an additional false
  auto exitSpillValues = peekSpillValues();
  exitSpillValues.push_back(cns(false));

  // Generate an exit for the rare case that r is zero.  Interpreting
  // will raise a notice and produce the boolean false.  Punch out
  // here and resume after the Mod instruction; this should be rare.
  auto const exit = makeExitWarn(nextBcOff(), exitSpillValues,
                                 makeStaticString(Strings::DIVISION_BY_ZERO));
  gen(JmpZero, exit, tr);

  // We unfortunately need to special-case r = -1 here. In two's
  // complement, trying to divide INT_MIN by -1 will cause an integer
  // overflow.
  if (tr->isConst()) {
    // This whole block only exists so m_irb->cond doesn't get mad when one
    // of the branches gets optimized out due to constant folding.
    if (tr->intVal() == -1LL) {
      push(cns(0));
    } else if (tr->intVal() == 0) {
      // mod by zero is undefined. don't emit opmod for it because
      // this could cause issues in simplifier/codegen
      // this should never get reached anyway, we just need to dump
      // something on the stack
      push(cns(false));
    } else {
      push(gen(Mod, tl, tr));
    }
    return;
  }

  // check for -1 (dynamic version)
  SSATmp *res = m_irb->cond(
    0,
    [&] (Block* taken) {
      SSATmp* negone = gen(Eq, tr, cns(-1));
      gen(JmpNZero, taken, negone);
    },
    [&] {
      return gen(Mod, tl, tr);
    },
    [&] {
      m_irb->hint(Block::Hint::Unlikely);
      return cns(0);
    });
  push(res);
}

void HhbcTranslator::emitPow() {
  emitInterpOne(Type::UncountedInit, 2);
}

void HhbcTranslator::emitSqrt() {
  auto const srcType = topC()->type();
  if (srcType <= Type::Int) {
    auto const src = gen(ConvIntToDbl, popC());
    push(gen(Sqrt, src));
    return;
  }

  if (srcType <= Type::Dbl) {
    auto const src = popC();
    push(gen(Sqrt, src));
    return;
  }

  emitInterpOne(Type::UncountedInit, 1);
}

void HhbcTranslator::emitBitNot() {
  auto const srcType = topC()->type();
  if (srcType <= Type::Int) {
    auto const src = popC();
    push(gen(XorInt, src, cns(-1)));
    return;
  }

  if (srcType <= Type::Dbl) {
    auto const src = gen(ConvDblToInt, popC());
    push(gen(XorInt, src, cns(-1)));
    return;
  }

  auto const resultType = srcType <= Type::Str ? Type::Str
                        : srcType.needsReg() ? Type::Cell
                        : Type::Int;
  emitInterpOne(resultType, 1);
}

void HhbcTranslator::emitXor() {
  SSATmp* btr = popC();
  SSATmp* btl = popC();
  SSATmp* tr = gen(ConvCellToBool, btr);
  SSATmp* tl = gen(ConvCellToBool, btl);
  push(gen(XorBool, tl, tr));
  gen(DecRef, btl);
  gen(DecRef, btr);
}

void HhbcTranslator::emitShl() {
  auto catch1 = makeCatch();
  auto catch2 = makeCatch();
  auto shiftAmount = popC();
  auto lhs         = popC();

  auto lhsInt         = gen(ConvCellToInt, catch1, lhs);
  auto shiftAmountInt = gen(ConvCellToInt, catch2, shiftAmount);

  push(gen(Shl, lhsInt, shiftAmountInt));
  gen(DecRef, lhs);
  gen(DecRef, shiftAmount);
}

void HhbcTranslator::emitShr() {
  auto catch1 = makeCatch();
  auto catch2 = makeCatch();
  auto shiftAmount = popC();
  auto lhs         = popC();

  auto lhsInt         = gen(ConvCellToInt, catch1, lhs);
  auto shiftAmountInt = gen(ConvCellToInt, catch2, shiftAmount);

  push(gen(Shr, lhsInt, shiftAmountInt));
  gen(DecRef, lhs);
  gen(DecRef, shiftAmount);
}

namespace {

Type arithOpResult(Type t1, Type t2) {
  if (!t1.isKnownDataType() || !t2.isKnownDataType()) {
    return Type::Cell;
  }

  auto both = t1 | t2;
  if (both.maybe(Type::Dbl)) return Type::Dbl;
  if (both.maybe(Type::Arr)) return Type::Arr;
  if (both.maybe(Type::Str)) return Type::Cell;
  return Type::Int;
}

Type arithOpOverResult(Type t1, Type t2) {
  if (t1 <= Type::Int && t2 <= Type::Int) {
    return Type::Int | Type::Dbl;
  }
  return arithOpResult(t1, t2);
}

Type bitOpResult(Type t1, Type t2) {
  if (!t1.isKnownDataType() || !t2.isKnownDataType()) {
    return Type::Cell;
  }

  auto both = t1 | t2;
  if (both <= Type::Str) return Type::Str;
  return Type::Int;
}

Type setOpResult(Type locType, Type valType, SetOpOp op) {
  switch (op) {
  case SetOpOp::PlusEqual:
  case SetOpOp::MinusEqual:
  case SetOpOp::MulEqual:    return arithOpResult(locType.unbox(), valType);
  case SetOpOp::PlusEqualO:
  case SetOpOp::MinusEqualO:
  case SetOpOp::MulEqualO:   return arithOpOverResult(locType.unbox(), valType);
  case SetOpOp::ConcatEqual: return Type::Str;
  case SetOpOp::PowEqual:
  case SetOpOp::DivEqual:
  case SetOpOp::ModEqual:    return Type::UncountedInit;
  case SetOpOp::AndEqual:
  case SetOpOp::OrEqual:
  case SetOpOp::XorEqual:    return bitOpResult(locType.unbox(), valType);
  case SetOpOp::SlEqual:
  case SetOpOp::SrEqual:     return Type::Int;
  }
  not_reached();
}

uint32_t localInputId(const NormalizedInstruction& inst) {
  switch (inst.op()) {
    case OpSetWithRefLM:
    case OpFPassL:
      return inst.imm[1].u_LA;

    default:
      return inst.imm[0].u_LA;
  }
}

}

folly::Optional<Type> HhbcTranslator::interpOutputType(
    const NormalizedInstruction& inst,
    folly::Optional<Type>& checkTypeType) const {
  using namespace jit::InstrFlags;
  auto localType = [&]{
    auto locId = localInputId(inst);
    assert(locId >= 0 && locId < curFunc()->numLocals());
    return m_irb->localType(locId, DataTypeSpecific);
  };
  auto boxed = [](Type t) {
    if (t.equals(Type::Gen)) return t;
    assert(t.isBoxed() || t.notBoxed());
    return t.isBoxed() ? t : boxType(t);
  };

  if (inst.outputPredicted) return Type::Gen;

  auto outFlag = getInstrInfo(inst.op()).type;
  if (outFlag == OutFInputL) {
    outFlag = inst.preppedByRef ? OutVInputL : OutCInputL;
  } else if (outFlag == OutFInputR) {
    outFlag = inst.preppedByRef ? OutVInput : OutCInput;
  }

  switch (outFlag) {
    case OutNull:        return Type::InitNull;
    case OutNullUninit:  return Type::Uninit;
    case OutString:      return Type::Str;
    case OutStringImm:   return Type::StaticStr;
    case OutDouble:      return Type::Dbl;
    case OutIsTypeL:
    case OutBoolean:
    case OutPredBool:
    case OutBooleanImm:  return Type::Bool;
    case OutInt64:       return Type::Int;
    case OutArray:       return Type::Arr;
    case OutArrayImm:    return Type::Arr; // Should be StaticArr: t2124292
    case OutObject:
    case OutThisObject:  return Type::Obj;
    case OutResource:    return Type::Res;

    case OutFDesc:       return folly::none;
    case OutUnknown:     return Type::Gen;

    case OutPred:
      checkTypeType = inst.outPred;
      // Returning inst.outPred from this function would turn the CheckStk
      // after the InterpOne into a nop.
      return Type::Gen;

    case OutCns:         return Type::Cell;
    case OutVUnknown:    return Type::BoxedCell;

    case OutSameAsInput: return topType(0);
    case OutVInput:      return boxed(topType(0));
    case OutVInputL:     return boxed(localType());
    case OutFInputL:
    case OutFInputR:     not_reached();

    case OutArith:       return arithOpResult(topType(0), topType(1));
    case OutArithO:      return arithOpOverResult(topType(0), topType(1));
    case OutBitOp:
      return bitOpResult(topType(0),
                         inst.op() == HPHP::OpBitNot ? Type::Bottom
                                                     : topType(1));
    case OutSetOp:      return setOpResult(localType(), topType(0),
                                           SetOpOp(inst.imm[1].u_OA));
    case OutIncDec: {
      auto ty = localType().unbox();
      return ty <= Type::Dbl ? ty : Type::Cell;
    }
    case OutStrlen:
      return topType(0) <= Type::Str ? Type::Int : Type::UncountedInit;
    case OutClassRef:   return Type::Cls;
    case OutFPushCufSafe: return folly::none;

    case OutNone:       return folly::none;

    case OutCInput: {
      auto ttype = topType(0);
      if (ttype.notBoxed()) return ttype;
      // All instructions that are OutCInput or OutCInputL cannot push uninit or
      // a ref, so only specific inner types need to be checked.
      if (ttype.unbox().strictSubtypeOf(Type::InitCell)) {
        checkTypeType = ttype.unbox();
      }
      return Type::Cell;
    }

    case OutCInputL: {
      auto ltype = localType();
      if (ltype.notBoxed()) return ltype;
      if (ltype.unbox().strictSubtypeOf(Type::InitCell)) {
        checkTypeType = ltype.unbox();
      }
      return Type::Cell;
    }
  }
  not_reached();
}

jit::vector<InterpOneData::LocalType>
HhbcTranslator::interpOutputLocals(const NormalizedInstruction& inst,
                                   bool& smashesAllLocals,
                                   folly::Optional<Type> pushedType) {
  using namespace jit::InstrFlags;
  if (!(getInstrInfo(inst.op()).out & Local)) return {};

  jit::vector<InterpOneData::LocalType> locals;
  auto setLocType = [&](uint32_t id, Type t) {
    // Relax the type for pseudomains so that we can actually guard on it.
    auto const type = inPseudoMain() ? t.relaxToGuardable() : t;
    locals.emplace_back(id, type);
  };
  auto setImmLocType = [&](uint32_t id, Type t) {
    setLocType(inst.imm[id].u_LA, t);
  };
  auto* func = curFunc();

  switch (inst.op()) {
    case OpSetN:
    case OpSetOpN:
    case OpIncDecN:
    case OpBindN:
    case OpVGetN:
    case OpUnsetN:
      smashesAllLocals = true;
      break;

    case OpSetOpL:
    case OpIncDecL: {
      assert(pushedType.hasValue());
      auto locType = m_irb->localType(localInputId(inst), DataTypeSpecific);
      assert(locType < Type::Gen);

      auto stackType = inst.outputPredicted ? inst.outPred : pushedType.value();
      setImmLocType(0, locType.isBoxed() ? stackType.box() : stackType);
      break;
    }

    case OpStaticLocInit:
      setImmLocType(0, Type::BoxedCell);
      break;

    case OpInitThisLoc:
      setImmLocType(0, Type::Cell);
      break;

    case OpSetL: {
      auto locType = m_irb->localType(localInputId(inst), DataTypeSpecific);
      auto stackType = topType(0);
      // SetL preserves reffiness of a local.
      setImmLocType(0, locType.isBoxed() ? boxType(stackType) : stackType);
      break;
    }
    case OpVGetL:
    case OpBindL: {
      assert(pushedType.hasValue());
      assert(pushedType->isBoxed());
      setImmLocType(0, pushedType.value());
      break;
    }

    case OpUnsetL:
    case OpPushL:
      setImmLocType(0, Type::Uninit);
      break;

    case OpSetM:
    case OpSetOpM:
    case OpBindM:
    case OpVGetM:
    case OpSetWithRefLM:
    case OpSetWithRefRM:
    case OpUnsetM:
    case OpFPassM:
    case OpIncDecM:
      switch (inst.immVec.locationCode()) {
        case LL: {
          auto const& mii = getMInstrInfo(inst.mInstrOp());
          auto const& base = inst.inputs[mii.valCount()]->location;
          assert(base.space == Location::Local);

          // MInstrEffects expects to be used in the context of a normally
          // translated instruction, not an interpOne. The two important
          // differences are that the base is normally a PtrTo* and we need to
          // supply an IR opcode representing the operation. SetWithRefElem is
          // used instead of SetElem because SetElem makes a few assumptions
          // about side exits that interpOne won't do.
          auto const baseType = m_irb->localType(base.offset,
                                                 DataTypeSpecific).ptr();
          auto const isUnset = inst.op() == OpUnsetM;
          auto const isProp = mcodeIsProp(inst.immVecM[0]);

          if (isUnset && isProp) break;
          auto op = isProp ? SetProp : isUnset ? UnsetElem : SetWithRefElem;
          MInstrEffects effects(op, baseType);
          if (effects.baseValChanged) {
            setLocType(base.offset, effects.baseType.deref());
          }
          break;
        }

        case LNL:
        case LNC:
          smashesAllLocals = true;
          break;

        default:
          break;
      }
      break;

    case OpMIterInitK:
    case OpMIterNextK:
      setImmLocType(3, Type::Cell);
    case OpMIterInit:
    case OpMIterNext:
      setImmLocType(2, Type::BoxedCell);
      break;

    case OpIterInitK:
    case OpWIterInitK:
    case OpIterNextK:
    case OpWIterNextK:
      setImmLocType(3, Type::Cell);
    case OpIterInit:
    case OpWIterInit:
    case OpIterNext:
    case OpWIterNext:
      setImmLocType(2, Type::Gen);
      break;

    case OpVerifyParamType: {
      auto paramId = inst.imm[0].u_LA;
      auto const& tc = func->params()[paramId].typeConstraint;
      auto locType = m_irb->localType(localInputId(inst), DataTypeSpecific);
      if (tc.isArray() && !tc.isSoft() && !func->mustBeRef(paramId) &&
          (locType <= Type::Obj || locType.maybeBoxed())) {
        setImmLocType(0, locType.isBoxed() ? Type::BoxedCell : Type::Cell);
      }
      break;
    }

    case OpSilence:
      if (static_cast<SilenceOp>(inst.imm[0].u_OA) == SilenceOp::Start) {
        setImmLocType(inst.imm[0].u_LA, Type::Int);
      }
      break;

    default:
      not_reached();
  }

  return locals;
}

void HhbcTranslator::emitInterpOne(const NormalizedInstruction& inst) {
  folly::Optional<Type> checkTypeType;
  auto stackType = interpOutputType(inst, checkTypeType);
  auto popped = getStackPopped(inst.pc());
  auto pushed = getStackPushed(inst.pc());
  FTRACE(1, "emitting InterpOne for {}, result = {}, popped {}, pushed {}\n",
         inst.toString(),
         stackType.hasValue() ? stackType->toString() : "<none>",
         popped, pushed);

  InterpOneData idata;
  auto locals = interpOutputLocals(inst, idata.smashesAllLocals, stackType);
  idata.nChangedLocals = locals.size();
  idata.changedLocals = locals.data();

  emitInterpOne(stackType, popped, pushed, idata);
  if (checkTypeType) {
    auto const out = getInstrInfo(inst.op()).out;
    auto const checkIdx = (out & InstrFlags::StackIns2) ? 2
                        : (out & InstrFlags::StackIns1) ? 1
                        : 0;
    checkTypeStack(checkIdx, *checkTypeType, inst.nextSk().offset());
  }
}

void HhbcTranslator::emitInterpOne(int popped) {
  InterpOneData idata;
  emitInterpOne(folly::none, popped, 0, idata);
}

void HhbcTranslator::emitInterpOne(Type outType, int popped) {
  InterpOneData idata;
  emitInterpOne(outType, popped, 1, idata);
}

void HhbcTranslator::emitInterpOne(folly::Optional<Type> outType, int popped,
                                   int pushed, InterpOneData& idata) {
  auto unit = curFunc()->unit();
  auto sp = spillStack();
  auto op = unit->getOpcode(bcOff());

  auto& iInfo = getInstrInfo(op);
  if (iInfo.type == jit::InstrFlags::OutFDesc) {
    m_fpiStack.emplace(sp, m_irb->spOffset());
  } else if (isFCallStar(op) && !m_fpiStack.empty()) {
    m_fpiStack.pop();
  }

  idata.bcOff = bcOff();
  idata.cellsPopped = popped;
  idata.cellsPushed = pushed;
  idata.opcode = op;

  auto const changesPC = opcodeChangesPC(idata.opcode);
  gen(changesPC ? InterpOneCF : InterpOne, outType,
      makeCatch(), idata, sp, m_irb->fp());
  assert(m_irb->stackDeficit() == 0);
}

std::string HhbcTranslator::showStack() const {
  std::ostringstream out;
  auto header = [&](const std::string& str) {
    out << folly::format("+{:-^82}+\n", str);
  };

  const int32_t frameCells = resumed() ? 0 : curFunc()->numSlotsInFrame();
  const int32_t stackDepth =
    m_irb->spOffset() + m_irb->evalStack().size()
    - m_irb->stackDeficit() - frameCells;
  auto spOffset = stackDepth;
  auto elem = [&](const std::string& str) {
    out << folly::format("| {:<80} |\n",
                         folly::format("{:>2}: {}",
                                       stackDepth - spOffset, str));
    assert(spOffset > 0);
    --spOffset;
  };

  auto fpi = curFunc()->findFPI(bcOff());
  auto checkFpi = [&]() {
    if (fpi && spOffset + frameCells == fpi->m_fpOff) {
      auto fpushOff = fpi->m_fpushOff;
      auto after = fpushOff + instrLen((Op*)curUnit()->at(fpushOff));
      std::ostringstream msg;
      msg << "ActRec from ";
      curUnit()->prettyPrint(msg, Unit::PrintOpts().range(fpushOff, after)
                                                   .noLineNumbers()
                                                   .indent(0)
                                                   .noFuncs());
      auto msgStr = msg.str();
      assert(msgStr.back() == '\n');
      msgStr.erase(msgStr.size() - 1);
      for (unsigned i = 0; i < kNumActRecCells; ++i) elem(msgStr);
      fpi = fpi->m_parentIndex != -1 ? &curFunc()->fpitab()[fpi->m_parentIndex]
                                     : nullptr;
      return true;
    }
    return false;
  };

  header(folly::format(" {} stack element(s); m_evalStack: ",
                       stackDepth).str());
  for (unsigned i = 0; i < m_irb->evalStack().size(); ++i) {
    while (checkFpi());
    SSATmp* value = top(DataTypeGeneric, i); // debug-only
    elem(value->inst()->toString());
  }

  header(" in-memory ");
  for (unsigned i = m_irb->stackDeficit(); spOffset > 0; ) {
    assert(i < curFunc()->maxStackCells());
    if (checkFpi()) {
      i += kNumActRecCells;
      continue;
    }

    auto stkVal = getStackValue(m_irb->sp(), i);
    std::ostringstream elemStr;
    if (stkVal.knownType == Type::StackElem) elem("unknown");
    else if (stkVal.value) elem(stkVal.value->inst()->toString());
    else elem(stkVal.knownType.toString());

    ++i;
  }
  header("");
  out << "\n";

  header(folly::format(" {} local(s) ", curFunc()->numLocals()).str());
  for (unsigned i = 0; i < curFunc()->numLocals(); ++i) {
    auto localValue = m_irb->localValue(i, DataTypeGeneric);
    auto str = localValue
      ? localValue->inst()->toString()
      : m_irb->localType(i, DataTypeGeneric).toString();
    out << folly::format("| {:<80} |\n",
                         folly::format("{:>2}: {}", i, str));
  }
  header("");
  return out.str();
}

/*
 * Get SSATmps representing all the information on the virtual eval
 * stack in preparation for a spill or exit trace. Top of stack will
 * be in the last element.
 *
 * Doesn't actually remove these values from the eval stack.
 */
std::vector<SSATmp*> HhbcTranslator::peekSpillValues() const {
  std::vector<SSATmp*> ret;
  ret.reserve(m_irb->evalStack().size());
  for (int i = m_irb->evalStack().size(); i--; ) {
    // DataTypeGeneric is used here because SpillStack just teleports the
    // values to memory.
    SSATmp* elem = top(DataTypeGeneric, i);
    ret.push_back(elem);
  }
  return ret;
}

Block* HhbcTranslator::makeExit(Offset targetBcOff /* = -1 */) {
  auto spillValues = peekSpillValues();
  return makeExit(targetBcOff, spillValues);
}

Block* HhbcTranslator::makeExit(TransFlags trflags) {
  auto spillValues = peekSpillValues();
  return makeExit(-1, spillValues, trflags);
}

Block* HhbcTranslator::makeExit(Offset targetBcOff,
                                std::vector<SSATmp*>& spillValues,
                                TransFlags trflags) {
  if (targetBcOff == -1) targetBcOff = bcOff();
  return makeExitImpl(targetBcOff, ExitFlag::JIT, spillValues,
                      CustomExit{}, trflags);
}

Block* HhbcTranslator::makePseudoMainExit(Offset targetBcOff /* = -1 */) {
  return inPseudoMain() ? makeExit(targetBcOff) : nullptr;
}

Block* HhbcTranslator::makeExitWarn(Offset targetBcOff,
                                    std::vector<SSATmp*>& spillValues,
                                    const StringData* warning) {
  assert(targetBcOff != -1);
  return makeExitImpl(targetBcOff, ExitFlag::JIT, spillValues,
    [&]() -> SSATmp* {
      gen(RaiseWarning, makeCatchNoSpill(), cns(warning));
      return nullptr;
    }
  );
}

Block* HhbcTranslator::makeExitError(SSATmp* msg, Block* catchBlock) {
  auto exit = m_irb->makeExit();
  BlockPusher bp(*m_irb, m_irb->marker(), exit);
  gen(RaiseError, catchBlock, msg);
  return exit;
}

Block* HhbcTranslator::makeExitNullThis() {
  return makeExitError(cns(makeStaticString(Strings::FATAL_NULL_THIS)),
                       makeCatch());
}

template<class ExitLambda>
Block* HhbcTranslator::makeSideExit(Offset targetBcOff, ExitLambda exit) {
  auto spillValues = peekSpillValues();
  return makeExitImpl(targetBcOff, ExitFlag::DelayedMarker, spillValues, exit);
}

Block* HhbcTranslator::makeExitSlow() {
  auto spillValues = peekSpillValues();
  return makeExitImpl(bcOff(), ExitFlag::Interp, spillValues, CustomExit{});
}

Block* HhbcTranslator::makeExitOpt(TransID transId) {
  Offset targetBcOff = bcOff();
  auto const exit = m_irb->makeExit();

  BCMarker exitMarker {
    SrcKey{ curFunc(), targetBcOff, resumed() },
    static_cast<int32_t>(m_irb->spOffset() +
                           m_irb->evalStack().size() - m_irb->stackDeficit()),
    m_profTransID
  };

  BlockPusher blockPusher(*m_irb, exitMarker, exit);

  SSATmp* stack = nullptr;
  if (m_irb->stackDeficit() != 0 || !m_irb->evalStack().empty()) {
    stack = spillStack();
  } else {
    stack = m_irb->sp();
  }

  gen(SyncABIRegs, m_irb->fp(), stack);
  gen(ReqRetranslateOpt, ReqRetransOptData(transId, targetBcOff));

  return exit;
}

Block* HhbcTranslator::makeExitImpl(Offset targetBcOff, ExitFlag flag,
                                    std::vector<SSATmp*>& stackValues,
                                    const CustomExit& customFn,
                                    TransFlags trflags) {
  Offset curBcOff = bcOff();
  BCMarker currentMarker = makeMarker(curBcOff);
  m_irb->evalStack().swap(stackValues);
  SCOPE_EXIT {
    m_bcStateStack.back().bcOff = curBcOff;
    m_irb->evalStack().swap(stackValues);
  };

  BCMarker exitMarker = makeMarker(targetBcOff);

  auto const exit = m_irb->makeExit();
  BlockPusher tp(*m_irb,
                 flag == ExitFlag::DelayedMarker ? currentMarker : exitMarker,
                 exit);

  if (flag != ExitFlag::DelayedMarker) {
    m_bcStateStack.back().bcOff = targetBcOff;
  }

  auto stack = spillStack();

  if (customFn) {
    stack = gen(ExceptionBarrier, stack);
    auto const customTmp = customFn();
    if (customTmp) {
      SSATmp* spill2[] = { stack, cns(0), customTmp };
      stack = gen(SpillStack,
                  std::make_pair(sizeof spill2 / sizeof spill2[0], spill2)
      );
      exitMarker.setSpOff(exitMarker.spOff() + 1);
    }
  }

  if (flag == ExitFlag::DelayedMarker) {
    m_irb->setMarker(exitMarker);
    m_bcStateStack.back().bcOff = targetBcOff;
  }

  gen(SyncABIRegs, m_irb->fp(), stack);

  if (flag == ExitFlag::Interp) {
    auto interpSk = SrcKey {curFunc(), targetBcOff, resumed()};
    auto pc = curUnit()->at(targetBcOff);
    auto changesPC = opcodeChangesPC(*reinterpret_cast<const Op*>(pc));
    auto interpOp = changesPC ? InterpOneCF : InterpOne;

    InterpOneData idata;
    idata.bcOff = targetBcOff;
    idata.cellsPopped = getStackPopped(pc);
    idata.cellsPushed = getStackPushed(pc);
    idata.opcode = *reinterpret_cast<const Op*>(pc);

    // This is deliberately ignoring anything the opcode might output on the
    // stack -- this Unit is about to end.
    gen(interpOp, idata, makeCatchNoSpill(), stack, m_irb->fp());

    if (!changesPC) {
      // If the op changes PC, InterpOneCF handles getting to the right place
      gen(ReqBindJmp, ReqBindJmpData(interpSk.advanced().offset()));
    }
    return exit;
  }

  if (!isInlining() &&
      curBcOff == m_context.initBcOffset &&
      targetBcOff == m_context.initBcOffset) {
    // Note that if we're inlining, then targetBcOff is in the inlined
    // func, while context.initBcOffset is in the outer func, so
    // bindJmp will always work (and there's no guarantee that there
    // is an anchor translation, so we must not use ReqRetranslate).
    gen(ReqRetranslate, ReqRetranslateData(trflags));
  } else {
    gen(ReqBindJmp, ReqBindJmpData(targetBcOff, trflags));
  }
  return exit;
}

/*
 * Create a catch block with a user-defined body (usually empty or a
 * SpillStack). Regardless of what body() does, it must return the current
 * stack pointer. This is a block to be invoked by the unwinder while unwinding
 * through a call to C++ from translated code. When attached to an instruction
 * as its taken field, code will be generated and the block will be registered
 * with the unwinder automatically.
 */
template<typename Body>
Block* HhbcTranslator::makeCatchImpl(Body body) {
  auto exit = m_irb->makeExit(Block::Hint::Unused);

  BlockPusher bp(*m_irb, makeMarker(bcOff()), exit);
  gen(BeginCatch);
  auto sp = body();
  gen(EndCatch, m_irb->fp(), sp);

  return exit;
}

/*
 * Create a catch block that spills the current state of the eval stack. The
 * incoming value of spillVals will be the top of the spilled stack: values in
 * the eval stack will be appended to spillVals to form the sources for the
 * SpillStack.
 */
Block* HhbcTranslator::makeCatch(std::vector<SSATmp*> spillVals,
                                 int64_t numPop) {
  return makeCatchImpl([&] {
    auto spills = peekSpillValues();
    spills.insert(spills.begin(), spillVals.begin(), spillVals.end());
    return emitSpillStack(m_irb->sp(), spills, numPop);
  });
}

/*
 * Create a catch block with no SpillStack. Some of our optimizations rely on
 * the ability to insert code on *every* path out of a trace, so we can't
 * simply elide the catch block in the cases that want an empty body.
 */
Block* HhbcTranslator::makeCatchNoSpill() {
  return makeCatchImpl([&] { return m_irb->sp(); });
}

/*
 * Returns an IR block corresponding to the given offset.
 */
Block* HhbcTranslator::getBlock(Offset offset) {
  // If hasBlock returns true, then IRUnit already has a block for
  // that offset and makeBlock will just return it.  This will be the
  // proper successor block set by Translator::setSuccIRBlocks.
  // Otherwise, the given offset doesn't belong to the region, so we
  // just create an exit block.

  return m_irb->hasBlock(offset) ? m_irb->makeBlock(offset)
                                 : makeExit(offset);
}

SSATmp* HhbcTranslator::emitSpillStack(SSATmp* sp,
                                       const std::vector<SSATmp*>& spillVals,
                                       int64_t extraOffset) {
  std::vector<SSATmp*> ssaArgs{
    sp, cns(int64_t(m_irb->stackDeficit() + extraOffset))
  };
  ssaArgs.insert(ssaArgs.end(), spillVals.rbegin(), spillVals.rend());

  auto args = std::make_pair(ssaArgs.size(), &ssaArgs[0]);
  return gen(SpillStack, args);
}

SSATmp* HhbcTranslator::spillStack() {
  auto newSp = emitSpillStack(m_irb->sp(), peekSpillValues());
  m_irb->evalStack().clear();
  m_irb->clearStackDeficit();
  return newSp;
}

void HhbcTranslator::prepareForSideExit() {
  spillStack();
}

void HhbcTranslator::exceptionBarrier() {
  auto const sp = spillStack();
  gen(ExceptionBarrier, sp);
}

SSATmp* HhbcTranslator::ldStackAddr(int32_t offset, TypeConstraint tc) {
  m_irb->constrainStack(offset, tc);
  // You're almost certainly doing it wrong if you want to get the address of a
  // stack cell that's in m_irb->evalStack().
  assert(offset >= (int32_t)m_irb->evalStack().numCells());
  return gen(
    LdStackAddr,
    Type::PtrToGen,
    StackOffset(offset + m_irb->stackDeficit() - m_irb->evalStack().numCells()),
    m_irb->sp()
  );
}

SSATmp* HhbcTranslator::unbox(SSATmp* val, Block* exit) {
  auto const type = val->type();
  // If we don't have an exit the LdRef can't be a guard.
  auto const inner = exit ? (type & Type::BoxedCell).innerType() : Type::Cell;

  if (type.isBoxed() || type.notBoxed()) {
    m_irb->constrainValue(val, DataTypeCountness);
    return type.isBoxed() ? gen(LdRef, inner, exit, val) : val;
  }

  return m_irb->cond(
    0,
    [&](Block* taken) {
      return gen(CheckType, Type::BoxedCell, taken, val);
    },
    [&](SSATmp* box) { // Next: val is a ref
      m_irb->constrainValue(box, DataTypeCountness);
      return gen(LdRef, inner, exit, box);
    },
    [&] { // Taken: val is unboxed
      return gen(AssertType, Type::Cell, val);
    });
}

SSATmp* HhbcTranslator::ldLoc(uint32_t locId, Block* exit, TypeConstraint tc) {
  assert(IMPLIES(exit == nullptr, !inPseudoMain()));

  auto const opStr = inPseudoMain() ? "LdGbl" : "LdLoc";
  m_irb->constrainLocal(locId, tc, opStr);

  if (inPseudoMain()) {
    auto const type = m_irb->localType(locId, tc).relaxToGuardable();
    assert(!type.isSpecialized());
    assert(type == type.dropConstVal());

    // We don't support locals being type Gen, so if we ever get into such a
    // case, we need to punt.
    if (type == Type::Gen) PUNT(LdGbl-Gen);
    return gen(LdGbl, type, exit, LocalId(locId), m_irb->fp());
  }

  return gen(LdLoc, Type::Gen, LocalId(locId), m_irb->fp());
}

SSATmp* HhbcTranslator::ldLocAddr(uint32_t locId, TypeConstraint tc) {
  m_irb->constrainLocal(locId, tc, "LdLocAddr");
  return gen(LdLocAddr, Type::PtrToGen, LocalId(locId), m_irb->fp());
}

/*
 * Load a local, and if it's boxed dereference to get the inner cell.
 *
 * Note: For boxed values, this will generate a LdRef instruction which
 *       takes the given exit trace in case the inner type doesn't match
 *       the tracked type for this local.  This check may be optimized away
 *       if we can determine that the inner type must match the tracked type.
 */
SSATmp* HhbcTranslator::ldLocInner(uint32_t locId,
                                   Block* ldrefExit,
                                   Block* ldgblExit,
                                   TypeConstraint constraint) {
  // We only care if the local is KindOfRef or not. DataTypeCountness
  // gets us that.
  auto loc = ldLoc(locId, ldgblExit, DataTypeCountness);
  assert((loc->type().isBoxed() || loc->type().notBoxed()) &&
         "Currently we don't handle traces where locals are maybeBoxed");

  auto value = loc->type().isBoxed()
    ? gen(LdRef, loc->type().innerType(), ldrefExit, loc)
    : loc;
  m_irb->constrainValue(value, constraint);
  return value;
}

/*
 * This is a wrapper to ldLocInner that also emits the RaiseUninitLoc if the
 * local is uninitialized. The catchBlock argument may be provided if the
 * caller requires the catch trace to be generated at a point earlier than when
 * it calls this function.
 */
SSATmp* HhbcTranslator::ldLocInnerWarn(uint32_t id,
                                       Block* ldrefExit,
                                       Block* ldgblExit,
                                       TypeConstraint constraint,
                                       Block* catchBlock /* = nullptr */) {
  if (!catchBlock) catchBlock = makeCatch();
  auto const locVal = ldLocInner(id, ldrefExit, ldgblExit, constraint);
  auto const varName = curFunc()->localVarName(id);

  auto warnUninit = [&] {
    if (varName != nullptr) {
      gen(RaiseUninitLoc, catchBlock, cns(varName));
    }
    return cns(Type::InitNull);
  };

  m_irb->constrainLocal(id, DataTypeCountnessInit, "ldLocInnerWarn");
  if (locVal->type() <= Type::Uninit) {
    return warnUninit();
  }

  if (locVal->type().maybe(Type::Uninit)) {
    // The local might be Uninit so we have to check at runtime.
    return m_irb->cond(
      0,
      [&](Block* taken) {
        gen(CheckInit, taken, locVal);
      },
      [&] { // Next: local is Init
        return locVal;
      },
      [&] { // Taken: local is Uninit
        return warnUninit();
      });
  }

  return locVal;
}

/*
 * Store to a local, if it's boxed set the value on the inner cell.
 *
 * Returns the value that was stored to the local. Assumes that 'newVal'
 * has already been incremented, with this Store consuming the
 * ref-count increment. If the caller of this function needs to
 * push the stored value on stack, it should set 'incRefNew' so that
 * 'newVal' will have its ref-count incremented.
 *
 * Pre: !newVal->type().isBoxed() && !newVal->type().maybeBoxed()
 * Pre: exit != nullptr if the local may be boxed
 */
SSATmp* HhbcTranslator::stLocImpl(uint32_t id,
                                  Block* ldrefExit,
                                  Block* ldgblExit,
                                  SSATmp* newVal,
                                  bool decRefOld,
                                  bool incRefNew) {
  assert(!newVal->type().maybeBoxed());

  auto const cat = decRefOld ? DataTypeCountness : DataTypeGeneric;
  auto const oldLoc = ldLoc(id, ldgblExit, cat);
  assert(oldLoc->type().isBoxed() || oldLoc->type().notBoxed());

  if (oldLoc->type().notBoxed()) {
    genStLocal(id, m_irb->fp(), newVal);
    if (incRefNew) gen(IncRef, newVal);
    if (decRefOld) gen(DecRef, oldLoc);
    return newVal;
  }

  // It's important that the IncRef happens after the LdRef, since the
  // LdRef is also a guard on the inner type and may side-exit.
  auto const innerCell = gen(
    LdRef, oldLoc->type().innerType(), ldrefExit, oldLoc
  );
  gen(StRef, oldLoc, newVal);
  if (incRefNew) gen(IncRef, newVal);
  if (decRefOld) {
    gen(DecRef, innerCell);
    m_irb->constrainValue(oldLoc, TypeConstraint(DataTypeCountness,
                                                 DataTypeCountness));
  }

  return newVal;
}

SSATmp* HhbcTranslator::pushStLoc(uint32_t id,
                                  Block* ldrefExit,
                                  Block* ldgblExit,
                                  SSATmp* newVal) {
  constexpr bool decRefOld = true;
  constexpr bool incRefNew = true;
  SSATmp* ret = stLocImpl(
    id,
    ldrefExit,
    ldgblExit,
    newVal,
    decRefOld,
    incRefNew
  );

  m_irb->constrainValue(ret, DataTypeCountness);
  return push(ret);
}

SSATmp* HhbcTranslator::stLoc(uint32_t id,
                              Block* ldrefExit,
                              Block* ldgblExit,
                              SSATmp* newVal) {
  constexpr bool decRefOld = true;
  constexpr bool incRefNew = false;
  return stLocImpl(id, ldrefExit, ldgblExit, newVal, decRefOld, incRefNew);
}

SSATmp* HhbcTranslator::stLocNRC(uint32_t id,
                                 Block* ldrefExit,
                                 Block* ldgblExit,
                                 SSATmp* newVal) {
  constexpr bool decRefOld = false;
  constexpr bool incRefNew = false;
  return stLocImpl(id, ldrefExit, ldgblExit, newVal, decRefOld, incRefNew);
}

SSATmp* HhbcTranslator::genStLocal(uint32_t id, SSATmp* fp, SSATmp* newVal) {
  return gen(inPseudoMain() ? StGbl : StLoc, LocalId(id), fp, newVal);
}

void HhbcTranslator::end() {
  auto const nextSk = curSrcKey().advanced(curUnit());
  end(nextSk.offset());
}

void HhbcTranslator::end(Offset nextPc) {
  if (nextPc >= curFunc()->past()) {
    // We have fallen off the end of the func's bytecodes. This happens
    // when the function's bytecodes end with an unconditional
    // backwards jump so that nextPc is out of bounds and causes an
    // assertion failure in unit.cpp. The common case for this comes
    // from the default value funclets, which are placed after the end
    // of the function, with an unconditional branch back to the start
    // of the function. So you should see this in any function with
    // default params.
    return;
  }
  setBcOff(nextPc, true);
  auto const sp = spillStack();
  gen(SyncABIRegs, m_irb->fp(), sp);
  gen(ReqBindJmp, ReqBindJmpData(nextPc));
}

void HhbcTranslator::endBlock(Offset next, bool nextIsMerge) {
  if (m_irb->hasBlock(next)) {
    emitJmpImpl(next,
                nextIsMerge ? JmpFlagNextIsMerge : JmpFlagNone,
                nullptr);
  }
}

void HhbcTranslator::checkStrictlyInteger(
    SSATmp*& key, KeyType& keyType, bool& checkForInt, bool& converted) {
  checkForInt = false;
  converted = false;
  if (key->isA(Type::Int)) {
    keyType = KeyType::Int;
  } else {
    assert(key->isA(Type::Str));
    keyType = KeyType::Str;
    if (key->isConst()) {
      int64_t i;
      if (key->strVal()->isStrictlyInteger(i)) {
        converted = true;
        keyType = KeyType::Int;
        key = cns(i);
      }
    } else {
      checkForInt = true;
    }
  }
}

bool HhbcTranslator::inPseudoMain() const {
  return Translator::liveFrameIsPseudoMain();
}

}}
