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

#include <folly/Optional.h>

#include "hphp/util/trace.h"
#include "hphp/runtime/ext/ext_closure.h"
#include "hphp/runtime/base/stats.h"
#include "hphp/runtime/vm/unit.h"
#include "hphp/runtime/vm/instance-bits.h"
#include "hphp/runtime/vm/runtime.h"
#include "hphp/runtime/vm/jit/code-gen-helpers.h"
#include "hphp/runtime/vm/jit/ir-unit.h"
#include "hphp/runtime/vm/jit/mc-generator.h"
#include "hphp/runtime/vm/jit/normalized-instruction.h"
#include "hphp/runtime/vm/jit/punt.h"
#include "hphp/runtime/vm/jit/target-profile.h"
#include "hphp/runtime/vm/jit/translator-inline.h"
#include "hphp/runtime/vm/jit/translator-runtime.h"
#include "hphp/runtime/vm/jit/analysis.h"
#include "hphp/runtime/base/packed-array-defs.h"

#include "hphp/runtime/vm/jit/hhbc-translator-internal.h"
// Include last to localize effects to this file
#include "hphp/util/assert-throw.h"

namespace HPHP { namespace jit {

TRACE_SET_MOD(hhir);

//////////////////////////////////////////////////////////////////////

HhbcTranslator::HhbcTranslator(TransContext ctx)
  : m_context(ctx)
  , m_unit(ctx)
  , m_irb(new IRBuilder(m_unit, BCMarker { ctx.srcKey(),
                                           ctx.initSpOffset,
                                           ctx.transID }))
  , m_bcStateStack { ctx.srcKey() }
  , m_lastBcOff{false}
  , m_mode{IRGenMode::Trace}
{
  updateMarker();
  auto const fp = gen(DefFP);
  gen(DefSP, StackOffset{ctx.initSpOffset}, fp);
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
  auto const key = SrcKey {
    target,
    target->getEntryForNumArgs(numParams),
    false
  };
  m_bcStateStack.emplace_back(key);
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

  m_bcStateStack.back().setOffset(newOff);
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
  auto const value  = gen(LdRef, Type::InitCell, box);
  auto const isnull = gen(IsType, Type::InitNull, value);
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
    if (auto newCap = PackedArray::getMaxCapInPlaceFast(capacity)) {
      assert(newCap > static_cast<uint32_t>(capacity));
      capacity = newCap;
    }
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
  if (auto newCap = PackedArray::getMaxCapInPlaceFast(capacity)) {
    assert(newCap > static_cast<uint32_t>(capacity));
    capacity = newCap;
  }
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
  auto const ldPMExit = makeExit();
  auto const ld = ldLocInner(id, ldrefExit, ldPMExit, DataTypeSpecific);

  SSATmp* arr;

  if (ld->isA(Type::Arr)) {
    arr = gen(NewLikeArray, ld, cns(capacity));
  } else {
    capacity = (capacity ? capacity : MixedArray::SmallSize);
    arr = gen(NewArray, cns(capacity));
  }
  push(arr);
}

SSATmp* HhbcTranslator::touchArgsSpillStackAndPopArgs(int numArgs) {
  // Before the spillStack() we touch all of the incoming stack
  // arguments so that they are available to later optimizations via
  // getStackValue().
  for (int i = 0; i < numArgs; i++) topC(i, DataTypeGeneric);
  SSATmp* sp = spillStack();
  for (int i = 0; i < numArgs; i++) popC(DataTypeGeneric);
  return sp;
}

void HhbcTranslator::emitNewPackedArray(uint32_t numArgs) {
  auto const extra = PackedArrayData { numArgs };
  if (numArgs > kPackedCapCodeThreshold) {
    // The NewPackedArray opcode's helper needs array values passed to it
    // via the stack.  We use spillStack() to flush the eval stack and
    // obtain a pointer to the topmost item; if over-flushing becomes
    // a problem then we should refactor the NewPackedArray opcode to
    // take its values directly as SSA operands.
    //
    // We only emit NewPackedArray when the array literal is too large for the
    // normal inline AllocNewPackedArray/InitPackedArray IR nodes to handle.
    SSATmp* sp = touchArgsSpillStackAndPopArgs(numArgs);
    push(gen(NewPackedArray, extra, sp));
    return;
  }

  SSATmp* array = gen(AllocPackedArray, extra);
  if (numArgs > kMaxUnrolledInitArray) {
    SSATmp* sp = touchArgsSpillStackAndPopArgs(numArgs);
    gen(InitPackedArrayLoop, extra, array, sp);
    push(array);
    return;
  }

  for (int i = 0; i < numArgs; ++i) {
    gen(InitPackedArray, IndexData{ numArgs - i - 1 }, array, popC());
  }
  push(array);
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
  auto const oldLoc = ldLoc(id, ldrefExit, DataTypeCountness);
  auto const tmpThis = gen(LdThis, makeExitSlow(), m_irb->fp());
  gen(IncRef, tmpThis);
  genStLocal(id, m_irb->fp(), tmpThis);
  gen(DecRef, oldLoc);
}

void HhbcTranslator::emitCGetL(int32_t id) {
  auto ldrefExit = makeExit();
  auto ldPMExit = makePseudoMainExit();
  // Mimic hhbc guard relaxation for now.
  auto cat = curSrcKey().op() == OpFPassL ? DataTypeSpecific
                                          : DataTypeCountnessInit;
  pushIncRef(ldLocInnerWarn(id, ldrefExit, ldPMExit, cat));
}

void HhbcTranslator::emitPushL(uint32_t id) {
  assertTypeLocal(id, Type::InitCell);
  auto* locVal = ldLoc(id, makeExit(), DataTypeGeneric);
  push(locVal);
  genStLocal(id, m_irb->fp(), cns(Type::Uninit));
}

void HhbcTranslator::emitCGetL2(int32_t id) {
  auto ldrefExit = makeExit();
  auto ldPMExit = makePseudoMainExit();
  auto catchBlock = makeCatch();
  SSATmp* oldTop = pop(Type::StackElem);
  auto val = ldLocInnerWarn(
    id,
    ldrefExit,
    ldPMExit,
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

  auto const ldPMExit = makePseudoMainExit();
  auto const newValue = popV();
  // Note that the IncRef must happen first, for correctness in a
  // pseudo-main: the destructor could decref the value again after
  // we've stored it into the local.
  pushIncRef(newValue);
  auto const oldValue = ldLoc(id, ldPMExit, DataTypeSpecific);
  genStLocal(id, m_irb->fp(), newValue);
  gen(DecRef, oldValue);
}

void HhbcTranslator::emitSetL(int32_t id) {
  auto const ldrefExit = makeExit();
  auto const ldPMExit = makePseudoMainExit();

  // since we're just storing the value in a local, this function doesn't care
  // about the type of the value. stLoc needs to IncRef the value so it may
  // constrain it further.
  auto const src = popC(DataTypeGeneric);
  pushStLoc(id, ldrefExit, ldPMExit, src);
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

  auto const ldPMExit = makePseudoMainExit();
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
  auto const oldValue = ldLoc(locId, ldPMExit, DataTypeSpecific);
  genStLocal(locId, m_irb->fp(), box);
  gen(DecRef, oldValue);
  // We don't need to decref value---it's a bytecode invariant that
  // our Cell was not ref-counted.
}

void HhbcTranslator::emitStaticLoc(uint32_t locId, uint32_t litStrId) {
  if (inPseudoMain()) PUNT(StaticLoc);

  auto const ldPMExit = makePseudoMainExit();
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
  auto const oldValue = ldLoc(locId, ldPMExit, DataTypeGeneric);
  genStLocal(locId, m_irb->fp(), box);
  gen(DecRef, oldValue);
  push(res);
}


void HhbcTranslator::emitIncStat(int32_t counter, int32_t value, bool force) {
  if (Stats::enabled() || force) {
    gen(IncStat, cns(counter), cns(value), cns(force));
  }
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
  auto const ldPMExit = makePseudoMainExit();
  auto const ld = ldLocInner(id, ldrefExit, ldPMExit, DataTypeSpecific);
  push(gen(IsNType, Type::Null, ld));
}

void HhbcTranslator::emitEmptyL(int32_t id) {
  auto const ldrefExit = makeExit();
  auto const ldPMExit = makePseudoMainExit();
  auto const ld = ldLocInner(id, ldrefExit, ldPMExit, DataTypeSpecific);
  push(gen(XorBool, gen(ConvCellToBool, ld), cns(true)));
}

void HhbcTranslator::emitIsTypeC(DataType t) {
  SSATmp* src = popC(DataTypeSpecific);
  if (t == KindOfObject) {
    push(optimizedCallIsObject(src));
  } else {
    push(gen(IsType, Type(t), src));
  }
  gen(DecRef, src);
}

void HhbcTranslator::emitIsTypeL(uint32_t id, DataType t) {
  auto const ldrefExit = makeExit();
  auto const ldPMExit = makePseudoMainExit();
  auto const val =
    ldLocInnerWarn(id, ldrefExit, ldPMExit, DataTypeSpecific);
  if (t == KindOfObject) {
    push(optimizedCallIsObject(val));
  } else {
    push(gen(IsType, Type(t), val));
  }
}

void HhbcTranslator::emitIsScalarL(int id) {
  auto const ldrefExit = makeExit();
  auto const ldPMExit = makePseudoMainExit();
  auto const src = ldLocInner(id, ldrefExit, ldPMExit, DataTypeSpecific);
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

void HhbcTranslator::jmpImpl(int32_t offset, JmpFlags flags) {
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
  const bool backward = static_cast<uint32_t>(offset) <= bcOff();
  if (backward) {
    auto const exit = makeExitSlow();
    gen(CheckSurpriseFlags, exit);
  }
  jmpImpl(offset, flags);
}

void HhbcTranslator::emitJmpNS(int32_t offset, JmpFlags flags) {
  jmpImpl(offset, flags);
}

void HhbcTranslator::jmpCondHelper(int32_t taken,
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
  jmpCondHelper(taken, true, flags, src);
}

void HhbcTranslator::emitJmpNZ(Offset taken, JmpFlags flags) {
  auto const src = popC();
  jmpCondHelper(taken, false, flags, src);
}

// Return a constant SSATmp representing a static value held in a
// TypedValue.  The TypedValue may be a non-scalar, but it must have a
// static value.
SSATmp* HhbcTranslator::staticTVCns(const TypedValue* tv) {
  switch (tv->m_type) {
    case KindOfNull:          return cns(Type::InitNull);
    case KindOfBoolean:       return cns(!!tv->m_data.num);
    case KindOfInt64:         return cns(tv->m_data.num);
    case KindOfDouble:        return cns(tv->m_data.dbl);
    case KindOfStaticString:
    case KindOfString:        return cns(tv->m_data.pstr);
    case KindOfArray:         return cns(tv->m_data.parr);

    case KindOfUninit:
    case KindOfObject:
    case KindOfResource:
    case KindOfRef:
    case KindOfClass:
      break;
  }
  always_assert(false);
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
  if (RDS::isPersistentHandle(cls->sPropInitHandle())) return;
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

  // Make sure our property init vectors are all set up.
  bool props = cls->pinitVec().size() > 0;
  bool sprops = cls->numStaticProperties() > 0;
  assert((props || sprops) == cls->needInitialization());
  if (cls->needInitialization()) {
    if (props) emitInitProps(cls, makeCatch());
    if (sprops) emitInitSProps(cls, makeCatch());
  }

  /*
   * Allocate the object.  This must happen after we do sinits for consistency
   * with the interpreter about o_id assignments.  Also, the prop
   * initialization above can throw, so we don't want to have the object
   * allocated already.
   */
  auto const ssaObj = gen(NewInstanceRaw, ClassData(cls));

  // Initialize the properties
  gen(InitObjProps, ClassData(cls), ssaObj);

  // Call a custom initializer if one exists
  if (cls->callsCustomInstanceInit()) {
    return registerObj(gen(CustomInstanceInit, ssaObj));
  }

  return registerObj(ssaObj);
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

void HhbcTranslator::emitNameA() {
  push(gen(LdClsName, popA()));
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
  gen(JmpSSwitchDest, dest);
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
 */
void HhbcTranslator::emitProfiledGuard(Type type,
                                       ProfGuard kind,
                                       int32_t id, // locId or stackOff
                                       Block* checkExit /* = nullptr */) {
  auto doGuard = [&] (Type type) {
    switch (kind) {
    case ProfGuard::CheckLoc:
      gen(CheckLoc, type, LocalId(id), checkExit, m_irb->fp());
      return;
    case ProfGuard::GuardLoc:
      gen(GuardLoc, type, LocalId(id), m_irb->fp(), m_irb->sp());
      return;
    case ProfGuard::CheckStk:
    case ProfGuard::GuardStk:
      {
        // Adjust 'id' to get an offset from the current m_irb->sp().
        auto const adjOff =
          static_cast<int32_t>(id + m_irb->stackDeficit()) -
          static_cast<int32_t>(m_irb->evalStack().numCells());
        if (kind == ProfGuard::CheckStk) {
          gen(CheckStk, type, StackOffset { adjOff }, checkExit, m_irb->sp());
          return;
        }
        gen(GuardStk, type, StackOffset { adjOff }, m_irb->sp(), m_irb->fp());
        return;
      }
    }
  };

  auto loadAddr = [&]() -> SSATmp* {
    switch (kind) {
    case ProfGuard::CheckLoc:
    case ProfGuard::GuardLoc:
      return ldLocAddr(id);
    case ProfGuard::CheckStk:
    case ProfGuard::GuardStk:
      return ldStackAddr(id);
    }
    not_reached();
  };

  // We really do want to check for exact type equality here: if type
  // is StaticStr there's nothing for us to do, and we don't support
  // guarding on CountedStr.
  if (!RuntimeOption::EvalJitPGOStringSpec ||
      type != Type::Str ||
      (mcg->tx().mode() != TransKind::Profile &&
       mcg->tx().mode() != TransKind::Optimize)) {
    return doGuard(type);
  }

  auto const profileKey = [&] {
    switch (kind) {
    case ProfGuard::CheckLoc:
    case ProfGuard::GuardLoc:
      return makeStaticString(folly::to<std::string>("Loc", id));
    case ProfGuard::CheckStk:
    case ProfGuard::GuardStk:
      // Note that for stacks we are using a profiling key on the unadjusted
      // index (index from top of virtual stack).
      return makeStaticString(folly::to<std::string>("Stk", id));
    }
    not_reached();
  }();
  TargetProfile<StrProfile> profile(m_context, m_irb->marker(), profileKey);

  if (profile.profiling()) {
    doGuard(Type::Str);
    gen(ProfileStr, ProfileStrData { profileKey }, loadAddr());
    return;
  }

  if (profile.optimizing()) {
    auto const data = profile.data(StrProfile::reduce);
    auto const total = data.total();

    if (data.staticStr == total) doGuard(Type::StaticStr);
    else                         doGuard(Type::Str);
    return;
  }

  // TransLive: just do a normal guard.
  doGuard(Type::Str);
}

void HhbcTranslator::guardTypeLocal(uint32_t locId, Type type, bool outerOnly) {
  if (!type.isBoxed()) {
    emitProfiledGuard(type, ProfGuard::GuardLoc, locId);
    return;
  }

  emitProfiledGuard(Type::BoxedInitCell, ProfGuard::GuardLoc, locId);
  gen(HintLocInner,
      type & Type::BoxedInitCell,
      LocalId { locId },
      m_irb->fp());

  if (!outerOnly && type.isBoxed() && type.unbox() < Type::Cell) {
    auto const ldrefExit = makeExit();
    auto const ldPMExit = makePseudoMainExit();
    auto const val = ldLoc(locId, ldPMExit, DataTypeSpecific);
    gen(CheckRefInner, m_irb->predictedInnerType(locId), ldrefExit, val);
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
  if (!type.isBoxed()) {
    emitProfiledGuard(type, ProfGuard::CheckLoc, locId, makeExit(dest));
    return;
  }

  emitProfiledGuard(Type::BoxedInitCell, ProfGuard::CheckLoc, locId,
    makeExit(dest));
  gen(HintLocInner,
      type & Type::BoxedInitCell,
      LocalId { locId },
      m_irb->fp());
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
  assert(type.isBoxed() || type.notBoxed());

  if (!type.isBoxed()) {
    emitProfiledGuard(type, ProfGuard::GuardStk, stackIndex);
    return;
  }

  emitProfiledGuard(Type::BoxedInitCell, ProfGuard::GuardStk, stackIndex);
  m_irb->constrainStack(stackIndex, DataTypeSpecific);
  gen(HintStkInner, type & Type::BoxedInitCell, stackOff, m_irb->sp());

  if (!outerOnly && type.isBoxed() && type.unbox() < Type::Cell) {
    auto stk = gen(LdStack, Type::BoxedInitCell, stackOff, m_irb->sp());
    gen(CheckRefInner,
        getStackInnerTypePrediction(m_irb->sp(), stackIndex),
        makeExit(),
        stk);
  }
}

void HhbcTranslator::checkTypeStack(uint32_t idx, Type type, Offset dest) {
  assert(type <= Type::Gen);

  if (type.isBoxed()) {
    spillStack(); // don't bother with the case that it's not spilled.
    auto const exit = makeExit(dest);
    auto const soff = StackOffset { static_cast<int32_t>(idx) };
    emitProfiledGuard(Type::BoxedInitCell, ProfGuard::CheckStk, idx, exit);
    m_irb->constrainStack(idx, DataTypeSpecific);
    gen(HintStkInner, type & Type::BoxedInitCell, soff, m_irb->sp());
    return;
  }

  auto const exit = makeExit(dest);
  if (idx < m_irb->evalStack().size()) {
    FTRACE(1, "checkTypeStack({}): generating CheckType for {}\n",
           idx, type.toString());
    // CheckType only cares about its input type if the simplifier does
    // something with it and that's handled if and when it happens.
    auto const tmp = top(DataTypeGeneric, idx);
    assert(tmp);
    m_irb->evalStack().replace(idx, gen(CheckType, type, exit, tmp));
    return;
  }
  FTRACE(1, "checkTypeStack({}): no tmp: {}\n", idx, type.toString());
  // Just like CheckType, CheckStk only cares about its input type if the
  // simplifier does something with it.
  emitProfiledGuard(type, ProfGuard::CheckStk, idx, exit);
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
        if (stackVal.knownType.isBoxed() &&
            !(stackVal.predictedInner <= Type::Bottom)) {
          return ldRefReturn(stackVal.predictedInner.unbox()).box();
        }
        return stackVal.knownType;
      }
    } break;
    case Location::Local: {
      auto l = loc.offset;
      auto ty = m_irb->localType(l, DataTypeGeneric);
      if (!ty.isBoxed()) {
        return ty;
      }
      return m_irb->predictedInnerType(l).box();
    } break;
    case Location::Litstr:
      return Type::cns(curUnit()->lookupLitstrId(loc.offset));
    case Location::Litint:
      return Type::cns(loc.offset);
    case Location::This:
      // Don't specialize $this for cloned closures which may have been re-bound
      if (curFunc()->hasForeignThis()) return Type::Obj;
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

void HhbcTranslator::refCheckHelper(int64_t entryArDelta,
                                    const std::vector<bool>& mask,
                                    const std::vector<bool>& vals,
                                    Offset dest /* = -1 */) {
  int32_t actRecOff = cellsToBytes(entryArDelta +
                                   m_irb->stackDeficit() -
                                   m_irb->evalStack().size());
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
    } else if (!nParams || nParams->isConst()) {
      nParams = gen(LdFuncNumParams, funcPtr);
    }

    uint64_t vals64 = packBitVec(vals, i);
    if (dest == -1) {
      gen(GuardRefs, funcPtr, nParams, cns(i), cns(mask64), cns(vals64),
          m_irb->fp(), m_irb->sp());
    } else {
      gen(CheckRefs, makeExit(dest), funcPtr, nParams, cns(i),
          cns(mask64), cns(vals64));
    }
  }
}

void HhbcTranslator::guardRefs(int64_t entryArDelta,
                               const std::vector<bool>& mask,
                               const std::vector<bool>& vals) {
  refCheckHelper(entryArDelta, mask, vals);
}

void HhbcTranslator::checkRefs(int64_t entryArDelta,
                               const std::vector<bool>& mask,
                               const std::vector<bool>& vals,
                               Offset dest) {
  refCheckHelper(entryArDelta, mask, vals, dest);
}

void HhbcTranslator::endGuards() {
  gen(EndGuards);
}

void HhbcTranslator::emitVerifyTypeImpl(int32_t const id) {
  const bool isReturnType = (id == HPHP::TypeConstraint::ReturnId);
  if (isReturnType && !RuntimeOption::EvalCheckReturnTypeHints) return;

  auto const ldPMExit = makePseudoMainExit();
  auto func = curFunc();
  auto const& tc = isReturnType ? func->returnTypeConstraint()
                                : func->params()[id].typeConstraint;
  auto val = isReturnType ? topR() : ldLoc(id, ldPMExit, DataTypeSpecific);
  assert(val->type().isBoxed() || val->type().notBoxed());

  auto const valType = [&]() -> Type {
    if (!val->type().isBoxed()) return val->type();
    if (isReturnType) PUNT(VerifyReturnTypeBoxed);
    auto const pred = m_irb->predictedInnerType(id);
    gen(CheckRefInner, pred, makeExit(), val);
    val = gen(LdRef, pred, val);
    return pred;
  }();

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
            || td->any
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

const StaticString s_WaitHandle("HH\\WaitHandle");

SSATmp* HhbcTranslator::emitInstanceOfDImpl(SSATmp* src,
                                            const StringData* className) {
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
    return cns(res);
  }

  if (s_WaitHandle.get()->isame(className)) {
    return gen(IsWaitHandle, src);
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
    return gen(InstanceOfIface, objClass, ssaClassName);
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

  return
      haveBit ? gen(InstanceOfBitmask, objClass, ssaClassName)
    : isUnique && isNormalClass ? gen(ExtendsClass, objClass, checkClass)
    : gen(InstanceOf, objClass, checkClass);
}

void HhbcTranslator::emitInstanceOfD(int classNameStrId) {
  const StringData* className = lookupStringId(classNameStrId);
  SSATmp* src = popC();

  push(emitInstanceOfDImpl(src, className));
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

SSATmp* HhbcTranslator::ldCls(Block* catchBlock, SSATmp* className) {
  assert(className->isA(Type::Str));
  if (className->isConst()) {
    if (auto const cls = Unit::lookupClass(className->strVal())) {
      if (classIsPersistentOrCtxParent(cls)) return cns(cls);
    }
    return gen(LdClsCached, catchBlock, className);
  }
  return gen(LdCls, catchBlock, className, cns(curClass()));
}

void HhbcTranslator::emitAGet(SSATmp* classSrc, Block* catchBlock) {
  if (classSrc->isA(Type::Str)) {
    push(ldCls(catchBlock, classSrc));
    return;
  }
  always_assert(classSrc->isA(Type::Obj));
  push(gen(LdObjClass, classSrc));
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
  auto const ldPMExit = makePseudoMainExit();

  auto const src = ldLocInner(id, ldrefExit, ldPMExit, DataTypeSpecific);
  if (isSupportedAGet(src)) {
    emitAGet(src, makeCatch());
  } else {
    PUNT(AGetL); // need to teach interpone about local uses
  }
}

void HhbcTranslator::emitBindMem(SSATmp* ptr, SSATmp* src) {
  auto const prevValue = gen(LdMem, ptr->type().deref(), ptr, cns(0));
  pushIncRef(src);
  gen(StMem, ptr, cns(0), src);
  gen(DecRef, prevValue);
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

  auto const ptrTy = convertToType(repoTy).ptr(Ptr::SProp);

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
      base = gen(LdClsPropAddrKnown, Type::PtrToSPropCell, cns(ctx),
        cns(propName));
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
    static_assert(std::is_unsigned<typeof(locId)>::value,
                  "locId should be unsigned");
    assert(locId < curFunc()->numLocals());
    return m_irb->localType(locId, DataTypeSpecific);
  };

  auto boxed = [&] (Type t) -> Type {
    if (t.equals(Type::Gen)) return t;
    assert(t.isBoxed() || t.notBoxed());
    checkTypeType = t.isBoxed() ? t : boxType(t); // inner type is predicted
    return Type::BoxedInitCell;
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
    case OutVUnknown:    return Type::BoxedInitCell;

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
    // Relax the type to something guardable.  For InterpOne we don't bother to
    // keep track of specialized types or inner-ref types.  (And note that for
    // psuedomains we may in fact have to guard on the local type after this.)
    locals.emplace_back(id, t.relaxToGuardable());
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
      setImmLocType(0, locType.isBoxed() ? Type::BoxedInitCell : stackType);
      break;
    }

    case OpStaticLocInit:
      setImmLocType(0, Type::BoxedInitCell);
      break;

    case OpInitThisLoc:
      setImmLocType(0, Type::Cell);
      break;

    case OpSetL: {
      auto locType = m_irb->localType(localInputId(inst), DataTypeSpecific);
      auto stackType = topType(0);
      // SetL preserves reffiness of a local.
      setImmLocType(0, locType.isBoxed() ? Type::BoxedInitCell : stackType);
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
          auto const baseType = m_irb->localType(
            base.offset, DataTypeSpecific
          ).ptr(Ptr::Frame);
          auto const isUnset = inst.op() == OpUnsetM;
          auto const isProp = mcodeIsProp(inst.immVecM[0]);

          if (isUnset && isProp) break;
          auto op = isProp ? SetProp : isUnset ? UnsetElem : SetWithRefElem;
          MInstrEffects effects(op, baseType);
          if (effects.baseValChanged) {
            auto const ty = effects.baseType.deref();
            assert(ty.isBoxed() || ty.notBoxed());
            setLocType(base.offset, ty.isBoxed() ? Type::BoxedInitCell : ty);
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
      setImmLocType(2, Type::BoxedInitCell);
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
        setImmLocType(0, locType.isBoxed() ? Type::BoxedInitCell : Type::Cell);
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
    auto const localValue = m_irb->localValue(i, DataTypeGeneric);
    auto const localTy = localValue ? localValue->type()
                                    : m_irb->localType(i, DataTypeGeneric);
    auto str = localValue ? localValue->inst()->toString()
                          : localTy.toString();
    if (localTy.isBoxed()) {
      auto const pred = m_irb->predictedInnerType(i);
      if (!pred.subtypeOf(Type::Bottom)) {
        str += folly::sformat(" (predict inner: {})", pred.toString());
      }
    }
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
  auto const curBcOff = bcOff();
  auto const currentMarker = makeMarker(curBcOff);
  m_irb->evalStack().swap(stackValues);
  SCOPE_EXIT {
    m_bcStateStack.back().setOffset(curBcOff);
    m_irb->evalStack().swap(stackValues);
  };

  auto exitMarker = makeMarker(targetBcOff);

  auto const exit = m_irb->makeExit();
  BlockPusher tp(*m_irb,
                 flag == ExitFlag::DelayedMarker ? currentMarker : exitMarker,
                 exit);

  if (flag != ExitFlag::DelayedMarker) {
    m_bcStateStack.back().setOffset(targetBcOff);
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
    m_bcStateStack.back().setOffset(targetBcOff);
  }

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

    stack = gen(interpOp, idata, makeCatchNoSpill(), stack, m_irb->fp());

    if (!changesPC) {
      // If the op changes PC, InterpOneCF handles getting to the right place
      gen(SyncABIRegs, m_irb->fp(), stack);
      gen(ReqBindJmp, ReqBindJmpData(interpSk.advanced().offset()));
    }
    return exit;
  }

  gen(SyncABIRegs, m_irb->fp(), stack);

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
 * Returns an IR block corresponding to the given bytecode offset. If the block
 * starts with a DefLabel expecting a StkPtr, this function will return an
 * intermediate block that passes the current sp.
 */
Block* HhbcTranslator::getBlock(Offset offset) {
  // If hasBlock returns true, then IRUnit already has a block for that offset
  // and makeBlock will just return it.  This will be the proper successor
  // block set by Translator::setSuccIRBlocks.  Otherwise, the given offset
  // doesn't belong to the region, so we just create an exit block.

  if (!m_irb->hasBlock(offset)) return makeExit(offset);

  auto block = m_irb->makeBlock(offset);
  if (!block->empty()) {
    auto& label = block->front();
    if (label.is(DefLabel) && label.numDsts() > 0 &&
        label.dst(0)->isA(Type::StkPtr)) {
      auto middle = m_unit.defBlock();
      ITRACE(2, "getBlock returning B{} to pass sp to B{}\n",
             middle->id(), block->id());
      BlockPusher bp(*m_irb, label.marker(), middle);
      gen(Jmp, block, m_irb->sp());
      return middle;
    }
  }

  return block;
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

SSATmp* HhbcTranslator::ldStackAddr(int32_t offset) {
  m_irb->constrainStack(offset, DataTypeSpecific);
  // You're almost certainly doing it wrong if you want to get the address of a
  // stack cell that's in m_irb->evalStack().
  assert(offset >= (int32_t)m_irb->evalStack().numCells());
  return gen(
    LdStackAddr,
    Type::PtrToStkGen,
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
    if (type.isBoxed()) {
      gen(CheckRefInner, inner, exit, val);
      return gen(LdRef, inner, val);
    }
    return val;
  }

  return m_irb->cond(
    0,
    [&](Block* taken) {
      return gen(CheckType, Type::BoxedCell, taken, val);
    },
    [&](SSATmp* box) { // Next: val is a ref
      m_irb->constrainValue(box, DataTypeCountness);
      gen(CheckRefInner, inner, exit, box);
      return gen(LdRef, inner, box);
    },
    [&] { // Taken: val is unboxed
      return gen(AssertType, Type::Cell, val);
    });
}

SSATmp* HhbcTranslator::ldLoc(uint32_t locId, Block* exit, TypeConstraint tc) {
  assert(IMPLIES(exit == nullptr, !inPseudoMain()));

  auto const opStr = inPseudoMain() ? "LdLocPseudoMain" : "LdLoc";
  m_irb->constrainLocal(locId, tc, opStr);

  if (inPseudoMain()) {
    auto const type = m_irb->localType(locId, tc).relaxToGuardable();
    assert(!type.isSpecialized());
    assert(type == type.dropConstVal());

    // We don't support locals being type Gen, so if we ever get into such a
    // case, we need to punt.
    if (type == Type::Gen) PUNT(LdGbl-Gen);
    return gen(LdLocPseudoMain, type, exit, LocalId(locId), m_irb->fp());
  }

  return gen(LdLoc, Type::Gen, LocalId(locId), m_irb->fp());
}

SSATmp* HhbcTranslator::ldLocAddr(uint32_t locId) {
  m_irb->constrainLocal(locId, DataTypeSpecific, "LdLocAddr");
  return gen(LdLocAddr, Type::PtrToFrameGen, LocalId(locId), m_irb->fp());
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
                                   Block* ldPMExit,
                                   TypeConstraint constraint) {
  // We only care if the local is KindOfRef or not. DataTypeCountness
  // gets us that.
  auto const loc = ldLoc(locId, ldPMExit, DataTypeCountness);
  assert((loc->type().isBoxed() || loc->type().notBoxed()) &&
         "Currently we don't handle traces where locals are maybeBoxed");

  if (!loc->type().isBoxed()) {
    m_irb->constrainValue(loc, constraint);
    return loc;
  }

  auto const predTy = m_irb->predictedInnerType(locId);
  gen(CheckRefInner, predTy, ldrefExit, loc);
  return gen(LdRef, predTy, loc);
}

/*
 * This is a wrapper to ldLocInner that also emits the RaiseUninitLoc if the
 * local is uninitialized. The catchBlock argument may be provided if the
 * caller requires the catch trace to be generated at a point earlier than when
 * it calls this function.
 */
SSATmp* HhbcTranslator::ldLocInnerWarn(uint32_t id,
                                       Block* ldrefExit,
                                       Block* ldPMExit,
                                       TypeConstraint constraint,
                                       Block* catchBlock /* = nullptr */) {
  if (!catchBlock) catchBlock = makeCatch();
  auto const locVal = ldLocInner(id, ldrefExit, ldPMExit, constraint);
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
                                  Block* ldPMExit,
                                  SSATmp* newVal,
                                  bool decRefOld,
                                  bool incRefNew) {
  assert(!newVal->type().maybeBoxed());

  auto const cat = decRefOld ? DataTypeCountness : DataTypeGeneric;
  auto const oldLoc = ldLoc(id, ldPMExit, cat);
  assert(oldLoc->type().isBoxed() || oldLoc->type().notBoxed());

  if (oldLoc->type().notBoxed()) {
    genStLocal(id, m_irb->fp(), newVal);
    if (incRefNew) gen(IncRef, newVal);
    if (decRefOld) gen(DecRef, oldLoc);
    return newVal;
  }

  // It's important that the IncRef happens after the guard on the inner type
  // of the ref, since it may side-exit.
  auto const predTy = m_irb->predictedInnerType(id);

  // We may not have a ldrefExit, but if so we better not be loading the inner
  // ref.
  if (ldrefExit == nullptr) always_assert(!decRefOld);
  if (ldrefExit != nullptr) {
    gen(CheckRefInner, predTy, ldrefExit, oldLoc);
  }
  auto const innerCell = decRefOld ? gen(LdRef, predTy, oldLoc) : nullptr;
  gen(StRef, oldLoc, newVal);
  if (incRefNew) gen(IncRef, newVal);
  if (decRefOld) {
    gen(DecRef, innerCell);
    m_irb->constrainValue(oldLoc, DataTypeCountness);
  }

  return newVal;
}

SSATmp* HhbcTranslator::pushStLoc(uint32_t id,
                                  Block* ldrefExit,
                                  Block* ldPMExit,
                                  SSATmp* newVal) {
  constexpr bool decRefOld = true;
  constexpr bool incRefNew = true;
  SSATmp* ret = stLocImpl(
    id,
    ldrefExit,
    ldPMExit,
    newVal,
    decRefOld,
    incRefNew
  );

  m_irb->constrainValue(ret, DataTypeCountness);
  return push(ret);
}

SSATmp* HhbcTranslator::stLoc(uint32_t id,
                              Block* ldrefExit,
                              Block* ldPMExit,
                              SSATmp* newVal) {
  constexpr bool decRefOld = true;
  constexpr bool incRefNew = false;
  return stLocImpl(id, ldrefExit, ldPMExit, newVal, decRefOld, incRefNew);
}

SSATmp* HhbcTranslator::stLocNRC(uint32_t id,
                                 Block* ldrefExit,
                                 Block* ldPMExit,
                                 SSATmp* newVal) {
  constexpr bool decRefOld = false;
  constexpr bool incRefNew = false;
  return stLocImpl(id, ldrefExit, ldPMExit, newVal, decRefOld, incRefNew);
}

SSATmp* HhbcTranslator::genStLocal(uint32_t id, SSATmp* fp, SSATmp* newVal) {
  return gen(
    inPseudoMain() ? StLocPseudoMain : StLoc,
    LocalId(id),
    fp,
    newVal
  );
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
  jmpImpl(next, nextIsMerge ? JmpFlagNextIsMerge : JmpFlagNone);
}

bool HhbcTranslator::inPseudoMain() const {
  return curFunc()->isPseudoMain();
}

}}
