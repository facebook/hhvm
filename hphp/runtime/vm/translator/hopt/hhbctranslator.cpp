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
#include "runtime/vm/translator/hopt/hhbctranslator.h"

#include "util/trace.h"
#include "runtime/ext/ext_closure.h"
#include "runtime/ext/ext_continuation.h"
#include "runtime/vm/translator/translator-runtime.h"
#include "runtime/vm/translator/translator-x64.h"
#include "runtime/base/stats.h"
#include "runtime/vm/unit.h"
#include "runtime/vm/runtime.h"
#include "runtime/vm/translator/hopt/irfactory.h"

// Include last to localize effects to this file
#include "util/assert_throw.h"

namespace HPHP {
namespace VM {
namespace JIT {

TRACE_SET_MOD(hhir);

using namespace HPHP::VM::Transl;

ArrayData* HhbcTranslator::lookupArrayId(int arrId) {
  return getCurUnit()->lookupArrayId(arrId);
}

StringData* HhbcTranslator::lookupStringId(int strId) {
  return getCurUnit()->lookupLitstrId(strId);
}

Func* HhbcTranslator::lookupFuncId(int funcId) {
  return getCurUnit()->lookupFuncId(funcId);
}

PreClass* HhbcTranslator::lookupPreClassId(int preClassId) {
  return getCurUnit()->lookupPreClassId(preClassId);
}

const NamedEntityPair& HhbcTranslator::lookupNamedEntityPairId(int id) {
  return getCurUnit()->lookupNamedEntityPairId(id);
}

SSATmp* HhbcTranslator::push(SSATmp* tmp) {
  assert(tmp);
  m_evalStack.push(tmp);
  return tmp;
}

void HhbcTranslator::refineType(SSATmp* tmp, Type type) {
  // If type is more refined than tmp's type, reset tmp's type to type
  IRInstruction* inst = tmp->inst();
  if (type.strictSubtypeOf(tmp->type())) {
    // If tmp is incref or move, then chase down its src
    Opcode opc = inst->op();
    if (opc == Mov || opc == IncRef) {
      refineType(inst->getSrc(0), type);
      tmp->setType(outputType(inst));
    } else if (tmp->type().isNull() && type.isNull()) {
      // Refining Null to Uninit or InitNull is supported
      tmp->setType(type);
    } else {
      // At this point, we have no business refining the type of any
      // instructions other than the following, which all control
      // their destination type via a type parameter.
      //
      // FIXME: I think most of these shouldn't be possible still
      // (except LdStack?).
      //
      // XXX These are possible once we remove the inferred/predicted
      // type from emitCGetProp etc in HhbcTranslator. We need to
      // delete label on these instructions if this is due to an
      // assertType and also handled LdClsCns.
      // TODO(#2035446): fix this for LdClsCns
      assert(opc == LdLoc || opc == LdStack  ||
             opc == LdMem || opc == LdProp ||
             opc == LdRef);
      inst->setTypeParam(type);
      tmp->setType(type);
      assert(outputType(inst) == type);
    }
  }
}

SSATmp* HhbcTranslator::pop(Type type) {
  SSATmp* opnd = m_evalStack.pop();
  if (opnd == nullptr) {
    uint32_t stackOff = m_stackDeficit;
    m_stackDeficit++;
    return gen(LdStack, type, StackOffset(stackOff), m_tb->getSp());
  }

  // Refine the type of the temp given the information we have from
  // `type'.  This case can occur if we did an extendStack() and
  // didn't know the type of the intermediate values yet (see below).
  refineType(opnd, type);
  return opnd;
}

void HhbcTranslator::discard(unsigned n) {
  for (unsigned i = 0; i < n; ++i) {
    pop(Type::Gen | Type::Cls);
  }
}

// type is the type expected on the stack.
void HhbcTranslator::popDecRef(Type type) {
  if (SSATmp* src = m_evalStack.pop()) {
    gen(DecRef, src);
    return;
  }

  gen(DecRefStack, StackOffset(m_stackDeficit), type, m_tb->getSp());
  m_stackDeficit++;
}

// We don't know what type description to expect for the stack
// locations before index, so we use a generic type when popping the
// intermediate values.  If it ends up creating a new LdStack,
// refineType during a later pop() or top() will fix up the type to
// the known type.
void HhbcTranslator::extendStack(uint32_t index, Type type) {
  if (index == 0) {
    push(pop(type));
    return;
  }

  SSATmp* tmp = pop(Type::Gen | Type::Cls);
  extendStack(index - 1, type);
  push(tmp);
}

SSATmp* HhbcTranslator::top(Type type, uint32_t index) {
  SSATmp* tmp = m_evalStack.top(index);
  if (!tmp) {
    extendStack(index, type);
    tmp = m_evalStack.top(index);
  }
  assert(tmp);
  refineType(tmp, type);
  return tmp;
}

void HhbcTranslator::replace(uint32_t index, SSATmp* tmp) {
  m_evalStack.replace(index, tmp);
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
 *     fp2   = DefInlineFP<returnBc> sp2
 *     sp4   = ReDefSP<numLocals> fp2
 *
 *         // ... callee body ...
 *
 *           = InlineReturn fp2
 *
 *   sp5  = ReDefSP<returnOffset> fp0 sp1
 *
 * The rest of the code then depends on sp5, and not any of the StkPtr
 * tree going through the callee body.  The sp5 tmp has the same view
 * of the stack as sp1 did, which represents what the stack looks like
 * before the return address is pushed but after the activation record
 * is popped.
 *
 * In DCE we attempt to remove the SpillFrame/InlineReturn/DefInlineFP
 * instructions if they aren't needed.
 */
void HhbcTranslator::beginInlining(unsigned numParams,
                                   const Func* target,
                                   Offset returnBcOffset) {
  assert(!m_fpiStack.empty() &&
    "Inlining does not support calls with the FPush* in a different Tracelet");
  assert(!target->isGenerator() && "Generator stack handling not implemented");

  FTRACE(1, "[[[ begin inlining: {}\n", target->fullName()->data());

  {
    static const bool enabled = Stats::enabledAny() &&
                                getenv("HHVM_STATS_INLINEFUNC");
    if (enabled) {
      gen(
        IncStatGrouped,
        cns(StringData::GetStaticString("HHIRInline")),
        cns(target->fullName()),
        cns(1)
      );
    }
  }

  SSATmp* params[numParams];
  for (unsigned i = 0; i < numParams; ++i) {
    params[numParams - i - 1] = popF();
  }

  auto prevSP    = m_fpiStack.top().first;
  auto prevSPOff = m_fpiStack.top().second;
  auto calleeSP  = spillStack();
  auto calleeFP  = gen(DefInlineFP, BCOffset(returnBcOffset), calleeSP);

  m_bcStateStack.emplace_back(target->base(), target);
  m_tb->beginInlining(target, calleeFP, calleeSP, prevSP, prevSPOff);
  profileFunctionEntry("Inline");

  for (unsigned i = 0; i < numParams; ++i) {
    m_tb->setLocalValue(i, params[i]);
  }
  for (unsigned i = numParams; i < target->numLocals(); ++i) {
    /*
     * Here we need to be generating hopefully-dead stores to
     * initialize non-parameter locals to KindOfUnknownin case we have
     * to leave the trace.
     */
    always_assert(0 && "unimplemented");
    m_tb->setLocalValue(i, m_tb->genDefUninit());
  }

  emitMarker();
}

bool HhbcTranslator::isInlining() const {
  return m_bcStateStack.size() > 1;
}

void HhbcTranslator::emitMarker() {
  int32_t stackOff = m_tb->getSpOffset() +
    m_evalStack.numCells() - m_stackDeficit;

  FTRACE(2, "emitMarker: bc {} sp {} fn {}\n",
         bcOff(), stackOff, getCurFunc()->fullName()->data());

  MarkerData marker;
  marker.bcOff     = bcOff();
  marker.func      = getCurFunc();
  marker.stackOff  = stackOff;
  gen(Marker, marker);
}

void HhbcTranslator::profileFunctionEntry(const char* category) {
  static const bool enabled = Stats::enabledAny() &&
                              getenv("HHVM_STATS_FUNCENTRY");
  if (!enabled) return;

  gen(
    IncStatGrouped,
    cns(StringData::GetStaticString("FunctionEntry")),
    cns(StringData::GetStaticString(category)),
    cns(1)
  );
}

void HhbcTranslator::profileInlineFunctionShape(const std::string& str) {
  gen(
    IncStatGrouped,
    cns(StringData::GetStaticString("InlineShape")),
    cns(StringData::GetStaticString(str)),
    cns(1)
  );
}

void HhbcTranslator::profileSmallFunctionShape(const std::string& str) {
  gen(
    IncStatGrouped,
    cns(StringData::GetStaticString("SmallFunctions")),
    cns(StringData::GetStaticString(str)),
    cns(1)
  );
}

void HhbcTranslator::profileFailedInlShape(const std::string& str) {
  gen(
    IncStatGrouped,
    cns(StringData::GetStaticString("FailedInl")),
    cns(StringData::GetStaticString(str)),
    cns(1)
  );
}

void HhbcTranslator::setBcOff(Offset newOff, bool lastBcOff) {
  if (isInlining()) assert(!lastBcOff);

  if (newOff != bcOff()) {
    m_bcStateStack.back().bcOff = newOff;
    emitMarker();
  }
  m_lastBcOff = lastBcOff;
}

void HhbcTranslator::emitPrint() {
  Type type = topC()->type();
  if (type.subtypeOfAny(Type::Int, Type::Bool, Type::Null, Type::Str)) {
    auto const cell = popC();

    Opcode op;
    if (type.isString()) {
      op = PrintStr;
    } else if (type.subtypeOf(Type::Int)) {
      op = PrintInt;
    } else if (type.subtypeOf(Type::Bool)) {
      op = PrintBool;
    } else {
      assert(type.isNull());
      op = Nop;
    }
    // the print helpers decref their arg, so don't decref pop'ed value
    if (op != Nop) {
      gen(op, cell);
    }
    push(cns(1));
  } else {
    emitInterpOne(Type::Int, 1);
  }
}

void HhbcTranslator::emitUnboxRAux() {
  Block* exit = getExitTrace()->front();
  SSATmp* srcBox = popR();
  SSATmp* unboxed = gen(Unbox, exit, srcBox);
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

void HhbcTranslator::emitThis() {
  if (!getCurClass()) {
    emitInterpOne(Type::Obj, 0); // will throw a fatal
    return;
  }
  pushIncRef(gen(LdThis, getExitSlowTrace(), m_tb->getFp()));
}

void HhbcTranslator::emitCheckThis() {
  if (!getCurClass()) {
    emitInterpOne(Type::None, 0); // will throw a fatal
    return;
  }
  gen(LdThis, getExitSlowTrace(), m_tb->getFp());
}

void HhbcTranslator::emitBareThis(int notice) {
  // We just exit the trace in the case $this is null. Before exiting
  // the trace, we could also push null onto the stack and raise a
  // notice if the notice argument is set. By exiting the trace when
  // $this is null, we can be sure in the rest of the trace that we
  // have the this object on top of the stack, and we can eliminate
  // further null checks of this.
  if (!getCurClass()) {
    emitInterpOne(Type::InitNull, 0); // will raise notice and push null
    return;
  }
  pushIncRef(gen(LdThis, getExitSlowTrace(), m_tb->getFp()));
}

void HhbcTranslator::emitArray(int arrayId) {
  push(cns(lookupArrayId(arrayId)));
}

void HhbcTranslator::emitNewArray(int capacity) {
  if (capacity == 0) {
    push(cns(HphpArray::GetStaticEmptyArray()));
  } else {
    push(gen(NewArray, cns(capacity)));
  }
}

void HhbcTranslator::emitNewTuple(int numArgs) {
  // The new_tuple helper function needs array values passed to it
  // via the stack.  We use spillStack() to flush the eval stack and
  // obtain a pointer to the topmost item; if over-flushing becomes
  // a problem then we should refactor the NewTuple opcode to take
  // its values directly as SSA operands.
  SSATmp* sp = spillStack();
  for (int i = 0; i < numArgs; i++) popC();
  push(gen(NewTuple, cns(numArgs), sp));
}

void HhbcTranslator::emitArrayAdd() {
  Type type1 = topC(0)->type();
  Type type2 = topC(1)->type();
  if (!type1.isArray() || !type2.isArray()) {
    // This happens when we have a prior spillstack that optimizes away
    // its spilled values because they were already on the stack. This
    // prevents us from getting to type of the SSATmps popped from the
    // eval stack. Most likely we had an interpone before this
    // instruction.
    emitInterpOne(Type::Arr, 2);
    return;
  }
  SSATmp* tr = popC();
  SSATmp* tl = popC();
  // the ArrrayAdd helper decrefs its args, so don't decref pop'ed values
  // TODO task 1805916: verify that ArrayAdd increfs its result
  push(gen(ArrayAdd, tl, tr));
}

void HhbcTranslator::emitAddElemC() {
  SSATmp* val = popC();
  SSATmp* key = popC();
  SSATmp* arr = popC();
  // the AddElem* instructions decrefs their args, so don't decref
  // pop'ed values. TODO task 1805916: verify that AddElem* increfs
  // their result
  auto kt = key->type();
  Opcode op;
  if (kt.subtypeOf(Type::Int)) {
    op = AddElemIntKey;
  } else if (kt.isString()) {
    op = AddElemStrKey;
  } else {
    PUNT(AddElem-NonIntNonStr);
  }

  push(gen(op, arr, key, val));
}

void HhbcTranslator::emitAddNewElemC() {
  SSATmp* val = popC();
  SSATmp* arr = popC();
  // the AddNewElem helper decrefs its args, so don't decref pop'ed values
  // TODO task 1805916: verify that NewElem increfs its result
  push(gen(AddNewElem, arr, val));
}

void HhbcTranslator::emitNewCol(int type, int numElems) {
  emitInterpOneOrPunt(Type::Obj, 0);
}

void HhbcTranslator::emitColAddElemC() {
  emitInterpOneOrPunt(Type::Obj, 3);
}

void HhbcTranslator::emitColAddNewElemC() {
  emitInterpOneOrPunt(Type::Obj, 2);
}

void HhbcTranslator::emitCns(uint32_t id) {
  StringData* name = curUnit()->lookupLitstrId(id);
  SSATmp* cnsNameTmp = cns(name);
  const TypedValue* tv = Unit::lookupPersistentCns(name);
  SSATmp* result = nullptr;
  Type cnsType = Type::Cell;
  if (tv) {
    switch (tv->m_type) {
      case KindOfUninit:
        // a dynamic system constant. always a slow lookup
        result = gen(LookupCns, cnsType, cnsNameTmp);
        break;
      case KindOfBoolean:
        result = cns((bool)tv->m_data.num);
        break;
      case KindOfInt64:
        result = cns(tv->m_data.num);
        break;
      case KindOfDouble:
        result = cns(tv->m_data.dbl);
        break;
      case KindOfString:
      case KindOfStaticString:
        result = cns(tv->m_data.pstr);
        break;
      default:
        not_reached();
    }
  } else {
    spillStack(); // do this on main trace so we update stack tracking once.
    SSATmp* c1 = gen(LdCns, cnsType, cnsNameTmp);
    result = m_tb->cond(
      getCurFunc(),
      [&] (Block* taken) { // branch
        gen(CheckInit, taken, c1);
      },
      [&] { // Next: LdCns hit in TC
        return c1;
      },
      [&] { // Taken: miss in TC, do lookup & init
        m_tb->hint(Block::Unlikely);
        return gen(LookupCns, cnsType, cnsNameTmp);
      }
    );
  }
  push(result);
}

void HhbcTranslator::emitDefCns(uint32_t id) {
  StringData* name = lookupStringId(id);
  SSATmp* val = popC();
  push(gen(DefCns, cns(name), val));
}

void HhbcTranslator::emitConcat() {
  SSATmp* tr = popC();
  SSATmp* tl = popC();
  // the concat helpers decref their args, so don't decref pop'ed values
  push(gen(Concat, tl, tr));
}

void HhbcTranslator::emitDefCls(int cid, Offset after) {
  emitInterpOneOrPunt(Type::None, 0);
}

void HhbcTranslator::emitDefFunc(int fid) {
  emitInterpOneOrPunt(Type::None, 0);
}

void HhbcTranslator::emitLateBoundCls() {
  Class* clss = getCurClass();
  if (!clss) {
    // no static context class, so this will raise an error
    emitInterpOne(Type::Cls, 0);
    return;
  }
  auto const ctx = gen(LdCtx, m_tb->getFp(), cns(getCurFunc()));
  push(gen(LdClsCtx, ctx));
}

void HhbcTranslator::emitSelf() {
  Class* clss = getCurClass();
  if (clss == nullptr) {
    emitInterpOne(Type::Cls, 0);
  } else {
    push(cns(clss));
  }
}

void HhbcTranslator::emitParent() {
  auto const clss = getCurClass();
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
  push(m_tb->genDefUninit());
}

void HhbcTranslator::emitNull() {
  push(m_tb->genDefInitNull());
}

void HhbcTranslator::emitTrue() {
  push(cns(true));
}

void HhbcTranslator::emitFalse() {
  push(cns(false));
}

void HhbcTranslator::emitInitThisLoc(int32_t id) {
  if (!getCurClass()) {
    // Do nothing if this is null
    return;
  }
  SSATmp* tmpThis = gen(LdThis, getExitSlowTrace(), m_tb->getFp());
  gen(StLoc, LocalId(id), m_tb->getFp(), gen(IncRef, tmpThis));
}

void HhbcTranslator::emitCGetL(int32_t id) {
  Trace* exitTrace = getExitTrace();
  pushIncRef(ldLocInnerWarn(id, exitTrace));
}

void HhbcTranslator::emitCGetL2(int32_t id) {
  Trace* exitTrace = getExitTrace();
  SSATmp* oldTop = pop(Type::Gen);
  pushIncRef(ldLocInnerWarn(id, exitTrace));
  push(oldTop);
}

void HhbcTranslator::emitVGetL(int32_t id) {
  auto value = ldLoc(id);
  if (!value->type().isBoxed()) {
    if (value->isA(Type::Uninit)) {
      value = m_tb->genDefInitNull();
    }
    value = gen(Box, value);
    gen(StLoc, LocalId(id), m_tb->getFp(), value);
  }
  pushIncRef(value);
}

void HhbcTranslator::emitUnsetN() {
  // No reason to punt, translator-x64 does emitInterpOne as well
  emitInterpOne(Type::None, 1);
}

void HhbcTranslator::emitUnsetG(const StringData* gblName) {
  // No reason to punt, translator-x64 does emitInterpOne as well
  emitInterpOne(Type::None, 1);
}

void HhbcTranslator::emitUnsetL(int32_t id) {
  auto const prev = ldLoc(id);
  gen(StLoc, LocalId(id), m_tb->getFp(), m_tb->genDefUninit());
  gen(DecRef, prev);
}

void HhbcTranslator::emitBindL(int32_t id) {
  auto const newValue = popV();
  // XXX: does this cause user-visible semantic differences?  What
  // about the stack being different when we re-enter the dtor?  What
  // is this for?
  if (getCurFunc()->isPseudoMain()) {
    // in pseudo mains, the value of locals could change in functions
    // called explicitly (or implicitly via exceptions or destructors)
    // so we need to incref eagerly in case one of these functions
    // changes the value of our local and makes src dead.
    pushIncRef(newValue);
  }
  auto const oldValue = ldLoc(id);
  gen(StLoc, LocalId(id), m_tb->getFp(), newValue);
  gen(DecRef, oldValue);
  if (!getCurFunc()->isPseudoMain()) {
    pushIncRef(newValue);
  }
}

void HhbcTranslator::emitSetL(int32_t id) {
  auto const exitTrace = getExitTrace();
  auto const src = popC();
  push(stLoc(id, exitTrace, src));
}

void HhbcTranslator::emitIncDecL(bool pre, bool inc, uint32_t id) {
  Trace* exitTrace = getExitTrace();
  auto const src = ldLocInner(id, exitTrace);

  // Inc/Dec of a bool is a no-op.
  if (src->isA(Type::Bool)) {
    push(src);
    return;
  }

  auto const res = emitIncDec(pre, inc, src);
  stLoc(id, exitTrace, res);
}

// only handles integer or double inc/dec
SSATmp* HhbcTranslator::emitIncDec(bool pre, bool inc, SSATmp* src) {
  assert(src->isA(Type::Int) || src->isA(Type::Dbl));
  SSATmp* one = src->isA(Type::Int) ? cns(1) : cns(1.0);
  SSATmp* res = inc ? gen(OpAdd, src, one) : gen(OpSub, src, one);
  // no incref necessary on push since result is an int
  push(pre ? res : src);
  return res;
}

void HhbcTranslator::emitIncDecMem(bool pre,
                                   bool inc,
                                   SSATmp* propAddr,
                                   Trace* exitTrace) {
  // Handle only integer inc/dec for now
  SSATmp* src = gen(LdMem, Type::Int, exitTrace, propAddr, cns(0));
  // do the add and store back
  SSATmp* res = emitIncDec(pre, inc, src);
  // don't gen a dec ref or type store
  gen(StMemNT, propAddr, cns(0), res);
}

static bool isSupportedBinaryArith(Opcode opc, Type t1, Type t2) {
  switch (opc) {
  case OpAdd:
  case OpSub:
  case OpMul: return t1.subtypeOfAny(Type::Int, Type::Bool, Type::Dbl) &&
                     t2.subtypeOfAny(Type::Int, Type::Bool, Type::Dbl);

  default:    return t1.subtypeOfAny(Type::Int, Type::Bool) &&
                     t2.subtypeOfAny(Type::Int, Type::Bool);
  }
}

void HhbcTranslator::emitSetOpL(Opcode subOpc, uint32_t id) {
  auto const exitTrace = getExitTrace();
  auto const loc       = ldLocInnerWarn(id, exitTrace);

  if (subOpc == Concat) {
    /*
     * The concat helpers decref their args, so don't decref pop'ed values
     * and don't decref the old value held in the local. The concat helpers
     * also incref their results, which will be consumed by the stloc. We
     * need an extra incref for the push onto the stack.
     */
    auto const val    = popC();
    auto const result = gen(Concat, loc, val);
    pushIncRef(stLocNRC(id, exitTrace, result));
    return;
  }

  if (isSupportedBinaryArith(subOpc, loc->type(), topC()->type())) {
    auto const val    = popC();
    auto const result = gen(subOpc, loc, val);
    push(stLoc(id, exitTrace, result));
    return;
  }

  PUNT(SetOpL);
}

void HhbcTranslator::emitClassExists(const StringData* clsName) {
  emitInterpOneOrPunt(Type::Bool, 2);
}

void HhbcTranslator::emitInterfaceExists(const StringData* ifaceName) {
  emitClassExists(ifaceName);
}

void HhbcTranslator::emitTraitExists(const StringData* traitName) {
  emitClassExists(traitName);
}

void HhbcTranslator::emitStaticLocInit(uint32_t locId, uint32_t litStrId) {
  const StringData* name = lookupStringId(litStrId);
  SSATmp* value = popC();
  SSATmp* box;

  // Closures and generators from closures don't satisfy the "one static per
  // source location" rule that the inline fastpath requires
  if (getCurFunc()->isClosureBody() || getCurFunc()->isGeneratorFromClosure()) {
    box = gen(StaticLocInit, cns(name), m_tb->getFp(), value);
  } else {
    SSATmp* ch = cns(TargetCache::allocStatic(), Type::CacheHandle);
    SSATmp* cachedBox = nullptr;
    box = m_tb->cond(getCurFunc(),
      [&](Block* taken) {
        // Careful: cachedBox is only ok to use in the 'next' branch.
        cachedBox = gen(LdStaticLocCached, taken, ch);
      },
      [&] { // next: The local is already initialized
        return gen(IncRef, cachedBox);
      },
      [&] { // taken: We missed in the cache
        m_tb->hint(Block::Unlikely);
        return gen(StaticLocInitCached,
                         cns(name), m_tb->getFp(), value, ch);
      }
    );
  }
  gen(StLoc, LocalId(locId), m_tb->getFp(), box);
  gen(DecRef, value);
}

void HhbcTranslator::emitReqDoc(const StringData* name) {
  PUNT(ReqDoc);
}

template<class Lambda>
SSATmp* HhbcTranslator::emitIterInitCommon(int offset, Lambda genFunc) {
  SSATmp* src = popC();
  Type type = src->type();
  if (!type.isArray() && type != Type::Obj) {
    PUNT(IterInit);
  }
  spillStack();
  SSATmp* res = genFunc(src);
  return emitJmpCondHelper(offset, true, res);
}

void HhbcTranslator::emitIterInit(uint32_t iterId,
                                  int offset,
                                  uint32_t valLocalId) {
  emitIterInitCommon(offset, [=] (SSATmp* src) {
    return gen(
      IterInit,
      Type::Bool,
      src,
      m_tb->getFp(),
      cns(iterId),
      cns(valLocalId)
    );
  });
}

void HhbcTranslator::emitIterInitK(uint32_t iterId,
                                   int offset,
                                   uint32_t valLocalId,
                                   uint32_t keyLocalId) {
  emitIterInitCommon(offset, [=] (SSATmp* src) {
    return gen(
      IterInitK,
      Type::Bool,
      src,
      m_tb->getFp(),
      cns(iterId),
      cns(valLocalId),
      cns(keyLocalId)
    );
  });
}

void HhbcTranslator::emitIterNext(uint32_t iterId,
                                  int offset,
                                  uint32_t valLocalId) {
  SSATmp* res = gen(
    IterNext,
    Type::Bool,
    m_tb->getFp(),
    cns(iterId),
    cns(valLocalId)
  );
  emitJmpCondHelper(offset, false, res);
}

void HhbcTranslator::emitIterNextK(uint32_t iterId,
                                   int offset,
                                   uint32_t valLocalId,
                                   uint32_t keyLocalId) {
  SSATmp* res = gen(
    IterNextK,
    Type::Bool,
    m_tb->getFp(),
    cns(iterId),
    cns(valLocalId),
    cns(keyLocalId)
  );
  emitJmpCondHelper(offset, false, res);
}

void HhbcTranslator::emitIterFree(uint32_t iterId) {
  gen(IterFree, m_tb->getFp(), cns(iterId));
}

void HhbcTranslator::emitCreateCont(bool getArgs,
                                    Id funNameStrId) {
  gen(ExitOnVarEnv, getExitSlowTrace()->front(), m_tb->getFp());

  auto const genName = lookupStringId(funNameStrId);
  auto const origFunc = getCurFunc();
  auto const genFunc = origFunc->getGeneratorBody(genName);
  auto const origLocals = origFunc->numLocals();
  auto const genLocals = genFunc->numLocals();

  auto const cont = gen(
    CreateCont,
    cns(
      origFunc->isMethod() ?
        (TCA)&VMExecutionContext::createContinuation<true> :
        (TCA)&VMExecutionContext::createContinuation<false>
    ),
    m_tb->getFp(),
    cns(getArgs),
    cns(origFunc),
    cns(genFunc)
  );

  TranslatorX64::ContParamMap params;
  if (origLocals <= TranslatorX64::kMaxInlineContLocals &&
      TranslatorX64::mapContParams(params, origFunc, genFunc)) {
    static auto const thisStr = StringData::GetStaticString("this");
    Id thisId = kInvalidId;
    const bool fillThis = origFunc->isNonClosureMethod() &&
      !origFunc->isStatic() &&
      ((thisId = genFunc->lookupVarId(thisStr)) != kInvalidId) &&
      (origFunc->lookupVarId(thisStr) == kInvalidId);

    SSATmp* locals = gen(LdContLocalsPtr, cont);
    for (int i = 0; i < origLocals; ++i) {
      // We must generate an AssertLoc because we don't have tracelet
      // guards on the object type in these outer generator functions.
      gen(AssertLoc, Type::Gen, LocalId(i), m_tb->getFp());
      auto const loc = gen(IncRef, ldLoc(i));
      gen(
        StMem,
        locals,
        cns(cellsToBytes(genLocals - params[i] - 1)),
        loc
      );
    }
    if (fillThis) {
      assert(thisId != kInvalidId);
      gen(FillContThis, cont, locals,
                cns(cellsToBytes(genLocals - thisId - 1)));
    }
  } else {
    gen(FillContLocals, m_tb->getFp(), cns(origFunc),
      cns(genFunc), cont);
  }

  push(cont);
}

void HhbcTranslator::emitContEnter(int32_t returnBcOffset) {
  spillStack();

  assert(getCurClass());
  SSATmp* cont = gen(LdThis, m_tb->getFp());
  SSATmp* contAR = gen(
    LdRaw, Type::FramePtr, cont, cns(RawMemSlot::ContARPtr)
  );

  SSATmp* func = gen(LdARFuncPtr, contAR, cns(0));
  SSATmp* funcBody = gen(
    LdRaw, Type::TCA, func, cns(RawMemSlot::ContEntry)
  );

  gen(
    ContEnter,
    contAR,
    funcBody,
    cns(returnBcOffset),
    m_tb->getFp()
  );
  assert(m_stackDeficit == 0);
}

void HhbcTranslator::emitContExitImpl() {
  auto const retAddr = gen(LdRetAddr, m_tb->getFp());
  auto const fp = gen(FreeActRec, m_tb->getFp());
  auto const sp = spillStack();
  gen(RetCtrl, sp, fp, retAddr);
  m_hasExit = true;
}

void HhbcTranslator::emitContExit() {
  gen(ExitWhenSurprised, getExitSlowTrace());
  emitContExitImpl();
}

void HhbcTranslator::emitUnpackCont() {
  gen(LinkContVarEnv, m_tb->getFp());
  gen(AssertLoc, Type::Obj, LocalId(0), m_tb->getFp());
  auto const cont = ldLoc(0);
  push(gen(LdRaw, Type::Int, cont, cns(RawMemSlot::ContLabel)));
}

void HhbcTranslator::emitPackCont(int64_t labelId) {
  gen(UnlinkContVarEnv, m_tb->getFp());
  gen(AssertLoc, Type::Obj, LocalId(0), m_tb->getFp());
  auto const cont = ldLoc(0);
  auto const newVal = popC();
  auto const oldValue = gen(LdProp, Type::Cell, cont, cns(CONTOFF(m_value)));
  gen(StProp, cont, cns(CONTOFF(m_value)), newVal);
  gen(DecRef, oldValue);
  gen(
    StRaw, cont, cns(RawMemSlot::ContLabel), cns(labelId)
  );
}

void HhbcTranslator::emitContReceive() {
  gen(AssertLoc, Type::Obj, LocalId(0), m_tb->getFp());
  auto const cont = ldLoc(0);
  gen(ContRaiseCheck, getExitSlowTrace(), cont);
  auto const valOffset = cns(CONTOFF(m_received));
  push(gen(LdProp, Type::Cell, cont, valOffset));
  gen(StProp, cont, valOffset, m_tb->genDefUninit());
}

void HhbcTranslator::emitContRetC() {
  gen(AssertLoc, Type::Obj, LocalId(0), m_tb->getFp());
  auto const cont = ldLoc(0);
  gen(ExitWhenSurprised, getExitSlowTrace());
  gen(
    StRaw, cont, cns(RawMemSlot::ContDone), cns(true)
  );
  auto const newVal = popC();
  auto const oldVal = gen(LdProp, Type::Cell, cont, cns(CONTOFF(m_value)));
  gen(StProp, cont, cns(CONTOFF(m_value)), newVal);
  gen(DecRef, oldVal);

  // transfer control
  emitContExitImpl();
}

void HhbcTranslator::emitContNext() {
  assert(getCurClass());
  SSATmp* cont = gen(LdThis, m_tb->getFp());
  gen(ContPreNext, getExitSlowTrace(), cont);

  auto const oldVal = gen(LdProp, Type::Cell, cont, cns(CONTOFF(m_received)));
  gen(StProp, cont, cns(CONTOFF(m_received)), m_tb->genDefInitNull());
  gen(DecRef, oldVal);
}

void HhbcTranslator::emitContSendImpl(bool raise) {
  assert(getCurClass());
  SSATmp* cont = gen(LdThis, m_tb->getFp());
  gen(ContStartedCheck, getExitSlowTrace(), cont);
  gen(ContPreNext, getExitSlowTrace(), cont);

  gen(AssertLoc, Type::Cell, LocalId(0), m_tb->getFp());
  auto const newVal = gen(IncRef, ldLoc(0));
  auto const oldVal = gen(LdProp, Type::Cell, cont, cns(CONTOFF(m_received)));
  gen(StProp, cont, cns(CONTOFF(m_received)), newVal);
  gen(DecRef, oldVal);
  if (raise) {
    gen(
      StRaw, cont, cns(RawMemSlot::ContShouldThrow), cns(true)
    );
  }
}

void HhbcTranslator::emitContSend() {
  emitContSendImpl(false);
}

void HhbcTranslator::emitContRaise() {
  emitContSendImpl(true);
}

void HhbcTranslator::emitContValid() {
  assert(getCurClass());
  SSATmp* cont = gen(LdThis, m_tb->getFp());
  SSATmp* done = gen(
    LdRaw, Type::Bool, cont, cns(RawMemSlot::ContDone)
  );
  push(gen(OpNot, done));
}

void HhbcTranslator::emitContCurrent() {
  assert(getCurClass());
  SSATmp* cont = gen(LdThis, m_tb->getFp());
  gen(ContStartedCheck, getExitSlowTrace(), cont);
  SSATmp* offset = cns(CONTOFF(m_value));
  SSATmp* value = gen(LdProp, Type::Cell, cont, offset);
  value = gen(IncRef, value);
  push(value);
}

void HhbcTranslator::emitContStopped() {
  assert(getCurClass());
  SSATmp* cont = gen(LdThis, m_tb->getFp());
  gen(
    StRaw, cont, cns(RawMemSlot::ContRunning), cns(false)
  );
}

void HhbcTranslator::emitContHandle() {
  emitInterpOneCF(1);
}

void HhbcTranslator::emitStrlen() {
  Type inType = topC()->type();

  if (inType.isString()) {
    SSATmp* input = popC();
    if (input->isConst()) {
      // static string; fold its strlen operation
      push(cns(input->getValStr()->size()));
    } else {
      push(gen(LdRaw, Type::Int, input, cns(RawMemSlot::StrLen)));
      gen(DecRef, input);
    }
  } else if (inType.isNull()) {
    popC();
    push(cns(0));
  } else if (inType == Type::Bool) {
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

void HhbcTranslator::emitIncTransCounter() {
  m_tb->gen(IncTransCounter);
}

SSATmp* HhbcTranslator::getStrName(const StringData* knownName) {
  SSATmp* name = popC();
  assert(name->isA(Type::Str) || knownName);
  if (!name->isConst() || !name->isA(Type::Str)) {
    if (knownName) {
      // The SSATmp on the evaluation stack was not a string constant,
      // but the bytecode translator somehow knew the name statically.
      name = cns(knownName);
    }
  } else {
    assert(!knownName || knownName->same(name->getValStr()));
  }
  return name;
}

SSATmp* HhbcTranslator::emitLdClsPropAddrOrExit(const StringData* propName,
                                                Block* block) {
  if (!block) exceptionBarrier();

  SSATmp* clsTmp = popA();
  SSATmp* prop = getStrName(propName);
  SSATmp* addr = gen(LdClsPropAddr,
                           block,
                           clsTmp,
                           prop,
                           cns(getCurClass()));
  gen(DecRef, prop); // safe to do early because prop is a string
  return addr;
}

bool HhbcTranslator::checkSupportedClsProp(const StringData* propName,
                                           Type resultType,
                                           int stkIndex) {
  if (topC(stkIndex + 1)->isA(Type::Str) || propName) {
    return true;
  }
  emitInterpOne(resultType, stkIndex + 2);
  return false;
}

bool HhbcTranslator::checkSupportedGblName(const StringData* gblName,
                                           Type resultType,
                                           int stkIndex) {
  if (topC(stkIndex)->isA(Type::Str) || gblName) {
    return true;
  }
  emitInterpOne(resultType, stkIndex + 1);
  return false;
}

SSATmp* HhbcTranslator::emitLdGblAddr(const StringData* gblName, Block* block) {
  SSATmp* name = getStrName(gblName);
  // Note: Once we use control flow to implement IssetG/EmptyG, we can
  // use a LdGblAddr helper that decrefs name for us
  SSATmp* addr = gen(LdGblAddr, block, name);
  gen(DecRef, name);
  return addr;
}

SSATmp* HhbcTranslator::emitLdGblAddrDef(const StringData* gblName) {
  return gen(LdGblAddrDef, getStrName(gblName));
}

void HhbcTranslator::emitIncDecS(bool pre, bool inc) {
  if (!checkSupportedClsProp(nullptr, Type::Cell, 0)) return;
  Trace* exit = getExitSlowTrace();
  emitIncDecMem(pre, inc, emitLdClsPropAddr(nullptr), exit);
}

void HhbcTranslator::emitMInstr(const NormalizedInstruction& ni) {
  VectorTranslator(ni, *this).emit();
}

/*
 * IssetH: return true if var is not uninit and !is_null(var)
 * Unboxes var if necessary when var is not uninit.
 */
void HhbcTranslator::emitIssetL(int32_t id) {
  auto const exitTrace = getExitTrace();
  auto const ld = ldLocInner(id, exitTrace);
  push(gen(IsNType, Type::Null, ld));
}

void HhbcTranslator::emitIssetG(const StringData* gblName) {
  emitIsset(gblName,
            &HhbcTranslator::checkSupportedGblName,
            &HhbcTranslator::emitLdGblAddr);
}

void HhbcTranslator::emitIssetS(const StringData* propName) {
  emitIsset(propName,
            &HhbcTranslator::checkSupportedClsProp,
            &HhbcTranslator::emitLdClsPropAddrOrExit);
}

void HhbcTranslator::emitEmptyL(int32_t id) {
  auto const exitTrace = getExitTrace();
  auto const ld = ldLocInner(id, exitTrace);
  push(gen(OpNot, gen(ConvCellToBool, ld)));
}

void HhbcTranslator::emitEmptyG(const StringData* gblName) {
  emitEmpty(gblName,
            &HhbcTranslator::checkSupportedGblName,
            &HhbcTranslator::emitLdGblAddr);
}

void HhbcTranslator::emitEmptyS(const StringData* propName) {
  emitEmpty(propName,
            &HhbcTranslator::checkSupportedClsProp,
            &HhbcTranslator::emitLdClsPropAddrOrExit);
}

void HhbcTranslator::emitIsTypeC(Type t) {
  SSATmp* src = popC();
  push(gen(IsType, t, src));
  gen(DecRef, src);
}

void HhbcTranslator::emitIsTypeL(Type t, int id) {
  Trace* exitTrace = getExitTrace();
  push(gen(IsType, t, ldLocInnerWarn(id, exitTrace)));
}

void HhbcTranslator::emitIsNullL(int id)   { emitIsTypeL(Type::Null, id);}
void HhbcTranslator::emitIsArrayL(int id)  { emitIsTypeL(Type::Arr, id); }
void HhbcTranslator::emitIsStringL(int id) { emitIsTypeL(Type::Str, id); }
void HhbcTranslator::emitIsObjectL(int id) { emitIsTypeL(Type::Obj, id); }
void HhbcTranslator::emitIsIntL(int id)    { emitIsTypeL(Type::Int, id); }
void HhbcTranslator::emitIsBoolL(int id)   { emitIsTypeL(Type::Bool, id);}
void HhbcTranslator::emitIsDoubleL(int id) { emitIsTypeL(Type::Dbl, id); }
void HhbcTranslator::emitIsNullC()   { emitIsTypeC(Type::Null);}
void HhbcTranslator::emitIsArrayC()  { emitIsTypeC(Type::Arr); }
void HhbcTranslator::emitIsStringC() { emitIsTypeC(Type::Str); }
void HhbcTranslator::emitIsObjectC() { emitIsTypeC(Type::Obj); }
void HhbcTranslator::emitIsIntC()    { emitIsTypeC(Type::Int); }
void HhbcTranslator::emitIsBoolC()   { emitIsTypeC(Type::Bool);}
void HhbcTranslator::emitIsDoubleC() { emitIsTypeC(Type::Dbl); }

void HhbcTranslator::emitPopC() {
  popDecRef(Type::Cell);
}

void HhbcTranslator::emitPopV() {
  popDecRef(Type::BoxedCell);
}

void HhbcTranslator::emitPopR() {
  popDecRef(Type::Gen);
}

void HhbcTranslator::emitDup() {
  pushIncRef(topC());
}

void HhbcTranslator::emitJmp(int32_t offset,
                             bool  breakTracelet,
                             bool  noSurprise) {
  // If surprise flags are set, exit trace and handle surprise
  bool backward = (offset - (int32_t)bcOff()) < 0;
  if (backward && !noSurprise) {
    gen(ExitWhenSurprised, getExitSlowTrace());
  }
  if (!breakTracelet) return;
  gen(Jmp_, getExitTrace(offset));
}

SSATmp* HhbcTranslator::emitJmpCondHelper(int32_t offset,
                                         bool negate,
                                         SSATmp* src) {
  Trace* target = nullptr;
  if (m_lastBcOff) {
    // Spill everything on main trace if all paths will exit
    spillStack();
    target = getExitTrace(offset, getBcOffNextTrace());
  } else {
    target = getExitTrace(offset);
  }
  auto const boolSrc = gen(ConvCellToBool, src);
  gen(DecRef, src);
  return gen(negate ? JmpZero : JmpNZero, target, boolSrc);
}

void HhbcTranslator::emitJmpZ(int32_t offset) {
  SSATmp* src = popC();
  emitJmpCondHelper(offset, true, src);
}

void HhbcTranslator::emitJmpNZ(int32_t offset) {
  SSATmp* src = popC();
  emitJmpCondHelper(offset, false, src);
}

void HhbcTranslator::emitCmp(Opcode opc) {
  if (cmpOpTypesMayReenter(opc, topC(0)->type(), topC(1)->type())) {
    exceptionBarrier();
  }
  // src2 opc src1
  SSATmp* src1 = popC();
  SSATmp* src2 = popC();
  push(gen(opc, src2, src1));
  gen(DecRef, src2);
  gen(DecRef, src1);
}

void HhbcTranslator::emitClsCnsD(int32_t cnsNameStrId, int32_t clsNameStrId) {
  // This bytecode re-enters if there is no class with the given name
  // and can throw a fatal error.
  const StringData* cnsNameStr = lookupStringId(cnsNameStrId);
  const StringData* clsNameStr = lookupStringId(clsNameStrId);
  SSATmp* cnsNameTmp = cns(cnsNameStr);
  SSATmp* clsNameTmp = cns(clsNameStr);
  if (0) {
    // TODO: 2068502 pick one of these two implementations and remove the other.
    Trace* exitTrace = getExitSlowTrace();
    SSATmp* cns = gen(LdClsCns, Type::Cell, cnsNameTmp, clsNameTmp);
    gen(CheckInit, exitTrace, cns);
    push(cns);
  } else {
    // if-then-else
    // todo: t2068502: refine the type? hhbc spec says null|bool|int|dbl|str
    //       and, str should always be static-str.
    exceptionBarrier(); // do on main trace so we update stack tracking once.
    Type cnsType = Type::Cell;
    SSATmp* c1 = gen(LdClsCns, cnsType, cnsNameTmp, clsNameTmp);
    SSATmp* result = m_tb->cond(getCurFunc(),
      [&] (Block* taken) { // branch
        gen(CheckInit, taken, c1);
      },
      [&] { // Next: LdClsCns hit in TC
        return c1;
      },
      [&] { // Taken: miss in TC, do lookup & init
        m_tb->hint(Block::Unlikely);
        return gen(LookupClsCns, cnsType, cnsNameTmp, clsNameTmp);
      }
    );
    push(result);
  }
}

void HhbcTranslator::emitAKExists() {
  SSATmp* arr = popC();
  SSATmp* key = popC();

  if (!arr->isA(Type::Arr) && !arr->isA(Type::Obj)) {
    PUNT(AKExists_badArray);
  }
  if (!key->isString() && !key->isA(Type::Int) && !key->isA(Type::Null)) {
    PUNT(AKExists_badKey);
  }

  push(gen(AKExists, arr, key));
  gen(DecRef, arr);
  gen(DecRef, key);
}

void HhbcTranslator::emitFPassR() {
  emitUnboxRAux();
}

void HhbcTranslator::emitFPassCOp() {
}

void HhbcTranslator::emitFPassV() {
  Block* exit = getExitTrace()->front();
  SSATmp* tmp = popV();
  pushIncRef(gen(Unbox, exit, tmp));
  gen(DecRef, tmp);
}

void HhbcTranslator::emitFPushCufOp(VM::Op op, Class* cls, StringData* invName,
                                    const Func* callee, int numArgs) {
  const Func* curFunc = getCurFunc();
  const bool safe = op == OpFPushCufSafe;
  const bool forward = op == OpFPushCufF;

  if (!callee) {
    SSATmp* callable = topC(safe ? 1 : 0);
    // The most common type for the callable in this case is Arr. We
    // can't really do better than the interpreter here, so punt.
    SPUNT(StringData::GetStaticString(
            folly::format("FPushCuf-{}",
                          callable->type().toString()).str())
          ->data());
  }

  SSATmp* ctx;
  SSATmp* safeFlag = cns(true); // This is always true until the slow exits
                                // below are implemented
  SSATmp* func = cns(callee);
  if (cls) {
    if (forward) {
      ctx = gen(LdCtx, m_tb->getFp(), cns(curFunc));
      ctx = gen(GetCtxFwdCall, ctx, cns(callee));
    } else {
      ctx = getClsMethodCtx(callee, cls);
    }
    if (!TargetCache::isPersistentHandle(cls->m_cachedOffset)) {
      // The miss path is complicated and rare. Punt for now.
      gen(LdClsCachedSafe, getExitSlowTrace(), cns(cls->name()));
    }
  } else {
    ctx = m_tb->genDefInitNull();
    if (!TargetCache::isPersistentHandle(callee->getCachedOffset())) {
      // The miss path is complicated and rare. Punt for now.
      func = gen(LdFuncCachedSafe, getExitSlowTrace(),
                       cns(callee->name()));
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
  gen(NativeImpl, cns(getCurFunc()), m_tb->getFp());
  SSATmp* sp = gen(RetAdjustStack, m_tb->getFp());
  SSATmp* retAddr = gen(LdRetAddr, m_tb->getFp());
  SSATmp* fp = gen(FreeActRec, m_tb->getFp());
  gen(RetCtrl, sp, fp, retAddr);

  // Flag that this trace has a Ret instruction so no ExitTrace is needed
  m_hasExit = true;
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

  /*
   * XXX. In a generator, we can't use ReDefSP to restore the stack
   * pointer from the frame pointer if we inline the callee.  (This is
   * because we don't really pay attention to usedefs for allocating
   * registers to stack pointers, and rVmFp and rVmSp are not related
   * to each other in a generator frame.)
   *
   * Instead, save it somewhere so we can move it back after.  This
   * instruction will be dce'd if we don't inline the callee.
   *
   * TODO(#2288359): freeing up the special-ness of %rbx should
   * allow us to avoid this sort of thing.
   */
  if (getCurFunc()->isGenerator()) {
    returnSp = gen(StashGeneratorSP, m_tb->getSp());
  }

  m_fpiStack.emplace(returnSp, m_tb->getSpOffset());

  ActRecInfo info;
  info.numArgs = numArgs;
  info.invName = invName;
  gen(
    SpillFrame,
    info,
    // Using actualStack instead of returnSp so SpillFrame still gets
    // the src in rVmSp.  (TODO(#2288359).)
    actualStack,
    m_tb->getFp(),
    func,
    objOrClass
  );
  assert(m_stackDeficit == 0);
}

void HhbcTranslator::emitFPushCtor(int32_t numParams) {
  SSATmp* cls = popA();
  exceptionBarrier();
  gen(NewObj, cls, cns(numParams), m_tb->getSp(), m_tb->getFp());
  m_fpiStack.emplace(nullptr, 0);
}

void HhbcTranslator::emitFPushCtorD(int32_t numParams, int32_t classNameStrId) {
  const StringData* className = lookupStringId(classNameStrId);
  exceptionBarrier();

  const Class* cls = Unit::lookupUniqueClass(className);
  bool uniqueCls = classIsUnique(cls);
  bool persistentCls = classIsPersistent(cls);

  const Func* func = uniqueCls ? cls->getCtor() : nullptr;
  if (func && !(func->attrs() & AttrPublic)) {
    Class* ctx = arGetContextClass(curFrame());
    if (!ctx) {
      func = nullptr;
    } else if (ctx != cls) {
      if ((func->attrs() & AttrPrivate) ||
        !(ctx->classof(cls) || cls->classof(ctx))) {
        func = nullptr;
      }
    }
  }

  SSATmp* clss = nullptr;
  if (persistentCls) {
    clss = cns(cls);
  } else {
    clss = gen(LdClsCached, cns(className));
  }
  SSATmp* obj = gen(IncRef, gen(AllocObj, clss));
  push(obj);

  SSATmp* fn = nullptr;
  if (func) {
    fn = cns(func);
  } else {
    fn = gen(LdClsCtor, clss);
  }

  SSATmp* obj2 = gen(IncRef, obj);
  int32_t numArgsAndCtorFlag = numParams | (1 << 31);
  emitFPushActRec(fn, obj2, numArgsAndCtorFlag, nullptr);
}

/*
 * The CreateCl opcode is specified as not being allowed before the
 * class it creates exists, and closure classes are always unique.
 *
 * This means even if we're not in RepoAuthoritative mode, as long as
 * this code is reachable it will always use the same closure Class*,
 * so we can just burn it into the TC without using TargetCache.
 */
void HhbcTranslator::emitCreateCl(int32_t numParams, int32_t funNameStrId) {
  auto const sp = spillStack();
  auto const cls = Unit::lookupUniqueClass(lookupStringId(funNameStrId));
  assert(cls && (cls->attrs() & AttrUnique));

  auto const closure = gen(
    CreateCl,
    cns(cls),
    cns(numParams),
    m_tb->getFp(),
    sp
  );

  discard(numParams);
  push(closure);
}

void HhbcTranslator::emitFPushFuncD(int32_t numParams, int32_t funcId) {
  const NamedEntityPair& nep = lookupNamedEntityPairId(funcId);
  const StringData* name = nep.first;
  const Func* func       = Unit::lookupFunc(nep.second, name);
  if (!func) {
    // function lookup failed so just do the same as FPushFunc
    emitFPushFunc(numParams, cns(name));
    return;
  }
  func->validate();

  const bool immutable = func->isNameBindingImmutable(getCurUnit());

  if (!immutable) {
    exceptionBarrier();  // LdFuncCached can reenter
  }
  SSATmp* ssaFunc = immutable ? cns(func)
                              : gen(LdFuncCached, cns(name));
  emitFPushActRec(ssaFunc,
                  m_tb->genDefInitNull(),
                  numParams,
                  nullptr);
}

void HhbcTranslator::emitFPushFunc(int32_t numParams) {
  // input must be a string or an object implementing __invoke();
  // otherwise fatal
  SSATmp* funcName = popC();
  if (!funcName->isString()) {
    PUNT(FPushFunc_not_Str);
  }
  emitFPushFunc(numParams, funcName);
}

void HhbcTranslator::emitFPushFunc(int32_t numParams, SSATmp* funcName) {
  exceptionBarrier(); // LdFunc can reenter
  emitFPushActRec(gen(LdFunc, funcName),
                  m_tb->genDefInitNull(),
                  numParams,
                  nullptr);
}

void HhbcTranslator::emitFPushObjMethodD(int32_t numParams,
                                         int32_t methodNameStrId,
                                         const Class* baseClass) {
  const StringData* methodName = lookupStringId(methodNameStrId);
  bool magicCall = false;
  const Func* func = HPHP::VM::Transl::lookupImmutableMethod(baseClass,
                                                             methodName,
                                                             magicCall,
                                                         /* staticLookup: */
                                                             false);
  SSATmp* obj = popC();
  SSATmp* objOrCls = obj;

  if (!func) {
    if (baseClass && !(baseClass->attrs() & AttrInterface)) {
      MethodLookup::LookupResult res =
        g_vmContext->lookupObjMethod(func, baseClass, methodName, false);
      if ((res == MethodLookup::MethodFoundWithThis ||
           res == MethodLookup::MethodFoundNoThis) &&
          !func->isAbstract()) {
        /*
         * If we found the func in baseClass, then either:
         *  a) its private, and this is always going to be the
         *     called function. This case is handled further down.
         * OR
         *  b) any derived class must have a func that matches in staticness
         *     and is at least as accessible (and in particular, you can't
         *     override a public/protected method with a private method).
         *     In this case, we emit code to dynamically lookup the method
         *     given the Object and the method slot, which is the same as func's.
         */
        if (!(func->attrs() & AttrPrivate)) {
          SSATmp* clsTmp = gen(LdObjClass, obj);
          SSATmp* funcTmp = gen(
            LdClsMethod, clsTmp, cns(func->methodSlot())
          );
          if (res == MethodLookup::MethodFoundNoThis) {
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
    if (func->attrs() & AttrStatic) {
      assert(baseClass);  // This assert may be too strong, but be aggressive
      // static function: store base class into this slot instead of obj
      // and decref the obj that was pushed as the this pointer since
      // the obj won't be in the actrec and thus MethodCache::lookup won't
      // decref it
      gen(DecRef, obj);
      objOrCls = cns(baseClass);
    }
    emitFPushActRec(cns(func),
                    objOrCls,
                    numParams,
                    magicCall ? methodName : nullptr);
  } else {
    emitFPushActRec(m_tb->genDefNull(),
                    obj,
                    numParams,
                    nullptr);
    auto const actRec = spillStack();
    auto const objCls = gen(LdObjClass, obj);
    gen(LdObjMethod,
              objCls,
              cns(methodName),
              actRec);
  }
}

SSATmp* HhbcTranslator::getClsMethodCtx(const Func* callee, const Class* cls) {
  bool mightNotBeStatic = false;
  assert(callee);
  if (!(callee->attrs() & AttrStatic) &&
      !(getCurFunc()->attrs() & AttrStatic) &&
      getCurClass() &&
      getCurClass()->classof(cls)) {
    mightNotBeStatic = true;
  }

  if (!mightNotBeStatic) {
    // static function: ctx is just the Class*. LdCls will simplify to a
    // DefConst or LdClsCached.
    return gen(LdCls, cns(cls->name()), cns(getCurClass()));
  } else if (m_tb->isThisAvailable()) {
    // might not be a static call and $this is available, so we know it's
    // definitely not static
    assert(getCurClass());
    return gen(IncRef, gen(LdThis, m_tb->getFp()));
  } else {
    // might be a non-static call. we have to inspect the func at runtime
    PUNT(getClsMethodCtx-MightNotBeStatic);
  }
}

void HhbcTranslator::emitFPushClsMethodD(int32_t numParams,
                                         int32_t methodNameStrId,
                                         int32_t clssNamedEntityPairId) {

  const StringData* methodName = lookupStringId(methodNameStrId);
  const NamedEntityPair& np = lookupNamedEntityPairId(clssNamedEntityPairId);
  const StringData* className = np.first;
  const Class* baseClass = Unit::lookupUniqueClass(np.second);
  bool magicCall = false;
  const Func* func = HPHP::VM::Transl::lookupImmutableMethod(baseClass,
                                                             methodName,
                                                             magicCall,
                                                         /* staticLookup: */
                                                             true);
  if (func) {
    SSATmp* objOrCls = getClsMethodCtx(func, baseClass);
    emitFPushActRec(cns(func),
                    objOrCls,
                    numParams,
                    func && magicCall ? methodName : nullptr);
  } else {
    // lookup static method & class in the target cache
    Trace* exitTrace = getExitSlowTrace();
    SSATmp* funcClassTmp =
      gen(LdClsMethodCache,
                exitTrace,
                cns(className),
                cns(methodName),
                cns(np.second));
    emitFPushActRec(funcClassTmp,
                    m_tb->genDefInitNull(),
                    numParams,
                    nullptr);
  }
}

void HhbcTranslator::emitFPushClsMethodF(int32_t           numParams,
                                         const Class*      cls,
                                         const StringData* methName) {

  assert(cls);
  assert(methName && methName->isStatic());

  Block* exitBlock = getExitSlowTrace()->front();

  UNUSED SSATmp* clsVal  = popC();
  UNUSED SSATmp* methVal = popC();

  bool magicCall = false;
  const Func* func = lookupImmutableMethod(cls, methName, magicCall,
                                           true /* staticLookup */);
  SSATmp* curCtxTmp = gen(LdCtx, m_tb->getFp(), cns(getCurFunc()));
  if (func) {
    SSATmp*   funcTmp = cns(func);
    SSATmp* newCtxTmp = gen(GetCtxFwdCall, curCtxTmp, funcTmp);

    emitFPushActRec(funcTmp, newCtxTmp, numParams,
                    (magicCall ? methName : nullptr));

  } else {
    SSATmp* funcCtxTmp = gen(LdClsMethodFCache, exitBlock,
                                   cns(cls),
                                   cns(methName),
                                   curCtxTmp);
    emitFPushActRec(funcCtxTmp,
                    m_tb->genDefInitNull(),
                    numParams,
                    (magicCall ? methName : nullptr));
  }
}

void HhbcTranslator::emitFCallArray(const Offset pcOffset,
                                    const Offset after) {
  SSATmp* stack = spillStack();
  gen(CallArray, CallArrayData(pcOffset, after), stack);
}

void HhbcTranslator::emitFCall(uint32_t numParams,
                               Offset returnBcOffset,
                               const Func* callee) {
  SSATmp* params[numParams + 3];
  std::memset(params, 0, sizeof params);
  for (uint32_t i = 0; i < numParams; i++) {
    params[numParams + 3 - i - 1] = popF();
  }
  params[0] = spillStack();
  params[1] = cns(returnBcOffset);
  params[2] = callee ? cns(callee) : m_tb->genDefNull();
  SSATmp** decayedPtr = params;
  gen(Call, std::make_pair(numParams + 3, decayedPtr));

  if (!m_fpiStack.empty()) {
    m_fpiStack.pop();
  }
}

void HhbcTranslator::emitFCallBuiltin(uint32_t numArgs,
                                      uint32_t numNonDefault,
                                      int32_t funcId) {
  const NamedEntityPair& nep = lookupNamedEntityPairId(funcId);
  const StringData* name = nep.first;
  const Func* callee = Unit::lookupFunc(nep.second, name);

  callee->validate();

  // spill args to stack. We need to spill these for two resons:
  // 1. some of the arguments may be passed by reference, for which
  //    case we will pass a stack address.
  // 2. type conversions of the arguments (using tvCast* helpers)
  //    may throw an exception, so we need to have the VM stack
  //    in a clean state at that point.
  exceptionBarrier();

  // Convert types if needed.
  for (int i = 0; i < numNonDefault; i++) {
    const Func::ParamInfo& pi = callee->params()[i];
    switch (pi.builtinType()) {
      case KindOfBoolean:
      case KindOfInt64:
      case KindOfArray:
      case KindOfObject:
      case KindOfString:
        gen(
          CastStk,
          Type::fromDataType(pi.builtinType(), KindOfInvalid),
          StackOffset(numArgs - i - 1),
          m_tb->getSp()
        );
        break;
      case KindOfDouble: not_reached();
      case KindOfUnknown: break;
      default:            not_reached();
    }
  }

  // Pass arguments for CallBuiltin.
  SSATmp* args[numArgs + 1];
  args[0] = cns(callee);
  for (int i = numArgs - 1; i >= 0; i--) {
    const Func::ParamInfo& pi = callee->params()[i];
    switch (pi.builtinType()) {
      case KindOfBoolean:
      case KindOfInt64:
        args[i + 1] = top(Type::fromDataType(pi.builtinType(), KindOfInvalid),
                          numArgs - i - 1);
        break;
      case KindOfDouble: assert(false);
      default:
        args[i + 1] = ldStackAddr(numArgs - i - 1);
        break;
    }
  }

  // Generate call and set return type
  SSATmp** decayedPtr = args;
  auto const ret = gen(
    CallBuiltin,
    Type::fromDataTypeWithRef(callee->returnType(),
                              (callee->attrs() & ClassInfo::IsReference)),
    std::make_pair(numArgs + 1, decayedPtr)
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

static bool mayHaveThis(const Func* func) {
  return func->isPseudoMain() || (func->isMethod() && !func->isStatic());
}

void HhbcTranslator::emitRetFromInlined(Type type) {
  SSATmp* retVal = pop(type);

  assert(!(getCurFunc()->attrs() & AttrMayUseVV));
  assert(!getCurFunc()->isPseudoMain());
  assert(!m_fpiStack.empty());

  emitDecRefLocalsInline(retVal);

  /*
   * Pop the ActRec and restore the stack and frame pointers.  It's
   * important that this does endInlining before pushing the return
   * value so stack offsets are properly tracked.
   */
  m_tb->endInlining();
  // after end of inlining, m_stackDeficit should be set to
  // 0, and eval stack should be empty
  assert(m_evalStack.numCells() == 0);
  m_stackDeficit = 0;
  FTRACE(1, "]]] end inlining: {}\n", getCurFunc()->fullName()->data());
  m_bcStateStack.pop_back();
  m_fpiStack.pop();
  push(retVal);

  emitMarker();
}

SSATmp* HhbcTranslator::emitDecRefLocalsInline(SSATmp* retVal) {
  SSATmp* retValSrcLoc = nullptr;
  Opcode  retValSrcOpc = Nop; // Nop flags the ref-count opt is impossible
  IRInstruction* retValSrcInstr = retVal->inst();

  /*
   * In case retVal comes from a local, the logic below tweaks the code
   * so that retVal is DecRef'd and the corresponding local's SSATmp is
   * returned. This enables the ref-count optimization to eliminate the
   * IncRef/DecRef pair in the main trace.
   */
  if (retValSrcInstr->op() == IncRef) {
    retValSrcLoc = retValSrcInstr->getSrc(0);
    retValSrcOpc = retValSrcLoc->inst()->op();
    if (retValSrcOpc != LdLoc && retValSrcOpc != LdThis) {
      retValSrcLoc = nullptr;
      retValSrcOpc = Nop;
    }
  }

  if (mayHaveThis(getCurFunc())) {
    if (retValSrcLoc && retValSrcOpc == LdThis) {
      gen(DecRef, retVal);
    } else {
      gen(DecRefThis, m_tb->getFp());
    }
  }

  /*
   * Note: this is currently off for isInlining() because the shuffle
   * was preventing a decref elimination due to ordering.  Currently
   * we don't inline anything with parameters, though, so it doesn't
   * matter.  This will need to be revisted then.
   */
  int retValLocId = (!isInlining() && retValSrcLoc && retValSrcOpc == LdLoc) ?
    retValSrcLoc->inst()->getExtra<LocalId>()->locId : -1;
  for (int id = getCurFunc()->numLocals() - 1; id >= 0; --id) {
    if (retValLocId == id) {
      gen(DecRef, retVal);
      continue;
    }
    gen(DecRefLoc, Type::Gen, LocalId(id), m_tb->getFp());
  }

  return retValSrcLoc ? retValSrcLoc : retVal;
}

void HhbcTranslator::emitRet(Type type, bool freeInline) {
  if (isInlining()) {
    return emitRetFromInlined(type);
  }

  const Func* curFunc = getCurFunc();
  bool mayUseVV = (curFunc->attrs() & AttrMayUseVV);

  gen(ExitWhenSurprised, getExitSlowTrace());
  if (mayUseVV) {
    // Note: this has to be the first thing, because we cannot bail after
    //       we start decRefing locs because then there'll be no corresponding
    //       bytecode boundaries until the end of RetC
    gen(ReleaseVVOrExit, getExitSlowTrace(), m_tb->getFp());
  }
  SSATmp* retVal = pop(type);

  SSATmp* sp;
  if (freeInline) {
    SSATmp* useRet = emitDecRefLocalsInline(retVal);
    gen(RetVal, m_tb->getFp(), useRet);
    sp = gen(RetAdjustStack, m_tb->getFp());
  } else {
    if (mayHaveThis(curFunc)) {
      gen(DecRefThis, m_tb->getFp());
    }
    sp = gen(
      GenericRetDecRefs, m_tb->getFp(), retVal, cns(curFunc->numLocals())
    );
    gen(RetVal, m_tb->getFp(), retVal);
  }

  // Free ActRec, and return control to caller.
  SSATmp* retAddr = gen(LdRetAddr, m_tb->getFp());
  SSATmp* fp = gen(FreeActRec, m_tb->getFp());
  gen(RetCtrl, sp, fp, retAddr);

  // Flag that this trace has a Ret instruction, so that no ExitTrace is needed
  m_hasExit = true;
}

void HhbcTranslator::emitSwitch(const ImmVector& iv,
                                int64_t base,
                                bool bounded) {
  int nTargets = bounded ? iv.size() - 2 : iv.size();

  SSATmp* const switchVal = popC();
  Type type = switchVal->type();
  assert(IMPLIES(!type.equals(Type::Int), bounded));
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

  if (type.subtypeOf(Type::Null)) {
    gen(Jmp_, getExitTrace(zeroOff));
    return;
  } else if (type.subtypeOf(Type::Bool)) {
    Offset nonZeroOff = bcOff() + iv.vec32()[iv.size() - 2];
    gen(JmpNZero, getExitTrace(nonZeroOff), switchVal);
    gen(Jmp_, getExitTrace(zeroOff));
    return;
  } else if (type.subtypeOf(Type::Int)) {
    // No special treatment needed
    index = switchVal;
  } else if (type.subtypeOf(Type::Dbl)) {
    // switch(Double|String|Obj)Helper do bounds-checking for us, so
    // we need to make sure the default case is in the jump table,
    // and don't emit our own bounds-checking code
    bounded = false;
    index = gen(LdSwitchDblIndex,
                      switchVal, ssabase, ssatargets);
  } else if (type.subtypeOf(Type::Str)) {
    bounded = false;
    index = gen(LdSwitchStrIndex,
                      switchVal, ssabase, ssatargets);
  } else if (type.subtypeOf(Type::Obj)) {
    // switchObjHelper can throw exceptions and reenter the VM
    if (type.subtypeOf(Type::Obj)) {
      exceptionBarrier();
    }
    bounded = false;
    index = gen(LdSwitchObjIndex,
                      switchVal, ssabase, ssatargets);
  } else if (type.subtypeOf(Type::Arr)) {
    gen(DecRef, switchVal);
    gen(Jmp_, getExitTrace(defaultOff));
    return;
  } else {
    PUNT(Switch-UnknownType);
  }

  std::vector<Offset> targets(iv.size());
  for (int i = 0; i < iv.size(); i++) {
    targets[i] = bcOff() + iv.vec32()[i];
  }

  JmpSwitchData data;
  data.func        = getCurFunc();
  data.base        = base;
  data.bounded     = bounded;
  data.cases       = iv.size();
  data.defaultOff  = defaultOff;
  data.targets     = &targets[0];

  auto const stack = spillStack();
  gen(SyncVMRegs, m_tb->getFp(), stack);

  gen(JmpSwitchDest, data, index);
  m_hasExit = true;
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
        return getCurUnit()->lookupLitstrId(item.str)->isNumeric();
      }
    );

  // The slow path can throw exceptions and reenter the VM.
  if (!fastPath) exceptionBarrier();

  SSATmp* const testVal = popC();
  assert(bcOff() != -1);

  std::vector<LdSSwitchData::Elm> cases(numCases);
  for (int i = 0; i < numCases; ++i) {
    auto const& kv = iv.strvec()[i];
    cases[i].str  = getCurUnit()->lookupLitstrId(kv.str);
    cases[i].dest = bcOff() + kv.dest;
  }

  LdSSwitchData data;
  data.func       = getCurFunc();
  data.numCases   = numCases;
  data.cases      = &cases[0];
  data.defaultOff = bcOff() + iv.strvec()[iv.size() - 1].dest;

  SSATmp* dest = gen(fastPath ? LdSSwitchDestFast
                                    : LdSSwitchDestSlow,
                           data,
                           testVal);
  gen(DecRef, testVal);
  auto const stack = spillStack();
  gen(SyncVMRegs, m_tb->getFp(), stack);
  gen(JmpIndirect, dest);
  m_hasExit = true;
}

void HhbcTranslator::emitRetC(bool freeInline) {
  emitRet(Type::Cell, freeInline);
}

void HhbcTranslator::emitRetV(bool freeInline) {
  emitRet(Type::BoxedCell, freeInline);
}

void HhbcTranslator::setThisAvailable() {
  m_tb->setThisAvailable();
}

void HhbcTranslator::guardTypeLocal(uint32_t locId, Type type) {
  checkTypeLocal(locId, type);
  m_typeGuards.push_back(TypeGuard(TypeGuard::Local, locId, type));
}

void HhbcTranslator::checkTypeLocal(uint32_t locId, Type type) {
  gen(GuardLoc, type, getExitTrace(), LocalId(locId), m_tb->getFp());
}

void HhbcTranslator::assertTypeLocal(uint32_t locId, Type type) {
  gen(AssertLoc, type, LocalId(locId), m_tb->getFp());
}

void HhbcTranslator::overrideTypeLocal(uint32_t locId, Type type) {
  gen(OverrideLoc, type, LocalId(locId), m_tb->getFp());
}

Trace* HhbcTranslator::guardTypeStack(uint32_t stackIndex,
                                      Type type,
                                      Trace* nextTrace) {
  if (type.subtypeOf(Type::Cls)) {
    // Should not generate guards for class; instead assert their type
    assertTypeStack(stackIndex, type);
    return nextTrace;
  }
  if (nextTrace == nullptr) {
    nextTrace = getGuardExit();
  }
  gen(GuardStk, type, nextTrace, StackOffset(stackIndex), m_tb->getSp());
  m_typeGuards.push_back(TypeGuard(TypeGuard::Stack, stackIndex, type));

  return nextTrace;
}

void HhbcTranslator::checkTypeTopOfStack(Type type,
                                         Offset nextByteCode) {
  Trace* exitTrace = getExitTrace(nextByteCode);
  SSATmp* tmp = m_evalStack.top();
  if (!tmp) {
    FTRACE(1, "checkTypeTopOfStack: no tmp: {}\n", type.toString());
    gen(GuardStk, type, exitTrace, StackOffset(0), m_tb->getSp());
    push(pop(type));
  } else {
    FTRACE(1, "checkTypeTopOfStack: generating GuardType for {}\n",
           type.toString());
    m_evalStack.pop();
    tmp = gen(GuardType, type, exitTrace, tmp);
    push(tmp);
  }
}

void HhbcTranslator::assertTypeStack(uint32_t stackIndex, Type type) {
  SSATmp* tmp = m_evalStack.top(stackIndex);
  if (!tmp) {
    gen(AssertStk, type, StackOffset(stackIndex), m_tb->getSp());
    return;
  }

  /*
   * We already had a value in flight---refine the type in case it
   * allows generating better code.  This is safe because in this path
   * we know the value is *actually* this type due to static analysis
   * (not based on guards).
   */
  refineType(tmp, type);
}

void HhbcTranslator::emitLoadDeps() {
  for (auto& guard : m_typeGuards) {
    switch (guard.getKind()) {
    case TypeGuard::Local:
      ldLoc(guard.getIndex());
      break;
    case TypeGuard::Stack:
      break;
    default:
      assert(false); // iterator guards should not happen
    }
  }
}

Trace* HhbcTranslator::guardRefs(int64_t               entryArDelta,
                                 const vector<bool>& mask,
                                 const vector<bool>& vals,
                                 Trace*              exitTrace) {
  if (exitTrace == nullptr) {
    exitTrace = getGuardExit();
  }

  int32_t actRecOff = cellsToBytes(entryArDelta);
  SSATmp* funcPtr = gen(LdARFuncPtr, m_tb->getSp(), cns(actRecOff));
  SSATmp* nParams = gen(
    LdRaw, Type::Int, funcPtr, cns(RawMemSlot::FuncNumParams)
  );
  SSATmp* bitsPtr = gen(
    LdRaw, Type::Int, funcPtr, cns(RawMemSlot::FuncRefBitVec)
  );

  for (unsigned i = 0; i < mask.size(); i += 64) {
    assert(i < vals.size());

    uint64_t mask64 = TranslatorX64::packBitVec(mask, i);
    if (mask64 == 0) {
      continue;
    }
    uint64_t vals64 = TranslatorX64::packBitVec(vals, i);

    gen(
      GuardRefs,
      exitTrace,
      funcPtr,
      nParams,
      bitsPtr,
      cns(i),
      m_tb->genLdConst(mask64),
      m_tb->genLdConst(vals64)
    );
  }

  return exitTrace;
}

void HhbcTranslator::emitVerifyParamType(int32_t paramId) {
  const Func* func = getCurFunc();
  const TypeConstraint& tc = func->params()[paramId].typeConstraint();
  auto locVal = ldLoc(paramId);
  Type locType = locVal->type().unbox();
  assert(locType.isKnownDataType());

  if (tc.nullable() && locType.isNull()) {
    return;
  }
  if (tc.isCallable()) {
    exceptionBarrier();
    locVal = gen(Unbox, getExitTrace(), locVal);
    gen(VerifyParamCallable, locVal, cns(paramId));
    return;
  }

  // For non-object guards, we rely on what we know from the tracelet
  // guards and never have to do runtime checks.
  if (!tc.isObjectOrTypedef()) {
    if (!tc.checkPrimitive(locType.toDataType())) {
      exceptionBarrier();
      gen(VerifyParamFail, cns(paramId));
      return;
    }
    return;
  }

  /*
   * If the parameter is an object, we check the object in one of
   * various ways (similar to instance of).  If the parameter is not
   * an object, it still might pass the VerifyParamType if the
   * constraint is a typedef.
   *
   * For now we just interp that case.
   */
  if (!locType.isObj()) {
    emitInterpOneOrPunt(Type::None, 0);
    return;
  }

  const StringData* clsName;
  const Class* knownConstraint = nullptr;
  if (!tc.isSelf() && !tc.isParent()) {
    clsName = tc.typeName();
    knownConstraint = Unit::lookupClass(clsName);
  } else {
    if (tc.isSelf()) {
      tc.selfToClass(getCurFunc(), &knownConstraint);
    } else if (tc.isParent()) {
      tc.parentToClass(getCurFunc(), &knownConstraint);
    }
    if (knownConstraint) {
      clsName = knownConstraint->preClass()->name();
    } else {
      // The hint was self or parent and there's no corresponding
      // class for the current func. This typehint will always fail.
      exceptionBarrier();
      gen(VerifyParamFail, cns(paramId));
      return;
    }
  }
  assert(clsName);
  // We can only burn in the Class* if it's unique or in the
  // inheritance hierarchy of our context. It's ok if the class isn't
  // defined yet - all paths below are tolerant of a null constraint.
  if (!classIsUniqueOrCtxParent(knownConstraint)) knownConstraint = nullptr;

  Class::initInstanceBits();
  bool haveBit = Class::haveInstanceBit(clsName);
  SSATmp* constraint = knownConstraint ? cns(knownConstraint)
                                       : gen(LdClsCachedSafe, cns(clsName));
  locVal = gen(Unbox, getExitTrace(), locVal);
  SSATmp* objClass = gen(LdObjClass, locVal);
  if (haveBit || classIsUniqueNormalClass(knownConstraint)) {
    SSATmp* isInstance = haveBit
      ? gen(InstanceOfBitmask, objClass, cns(clsName))
      : gen(ExtendsClass, objClass, constraint);
    exceptionBarrier();
    m_tb->ifThen(getCurFunc(),
      [&](Block* taken) {
        gen(JmpZero, taken, isInstance);
      },
      [&] { // taken: the param type does not match
        m_tb->hint(Block::Unlikely);
        gen(VerifyParamFail, cns(paramId));
      }
    );
  } else {
    exceptionBarrier();
    gen(VerifyParamCls,
              objClass,
              constraint,
              cns(paramId),
              cns(uintptr_t(&tc)));
  }
}

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
    push(cns(false));
    gen(DecRef, src);
    return;
  }

  SSATmp* objClass     = gen(LdObjClass, src);
  SSATmp* ssaClassName = cns(className);

  Class::initInstanceBits();
  const bool haveBit = Class::haveInstanceBit(className);

  Class* const maybeCls = Unit::lookupUniqueClass(className);
  const bool isNormalClass = classIsUniqueNormalClass(maybeCls);
  const bool isUnique = classIsUnique(maybeCls);

  /*
   * If the class is unique or a parent of the current context, we
   * don't need to load it out of target cache because it must
   * already exist and be defined.
   *
   * Otherwise, we only use LdClsCachedSafe---instanceof with an
   * undefined class doesn't invoke autoload.
   */
  SSATmp* checkClass =
    isUnique || (maybeCls && getCurClass() &&
                  getCurClass()->classof(maybeCls))
      ? cns(maybeCls)
      : gen(LdClsCachedSafe, ssaClassName);

  push(
    haveBit ? gen(InstanceOfBitmask,
                        objClass,
                        ssaClassName) :
    isUnique && isNormalClass ? gen(ExtendsClass,
                                          objClass,
                                          checkClass) :
    gen(InstanceOf,
              objClass,
              checkClass,
              cns(maybeCls && !isNormalClass))
  );
  gen(DecRef, src);
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

  SSATmp* src = popC();
  Type fromType = src->type();
  if (fromType.isArray()) {
    push(src);
  } else if (fromType.isNull()) {
    push(cns(HphpArray::GetStaticEmptyArray()));
  } else if (fromType.isBool()) {
    push(gen(ConvBoolToArr, src));
  } else if (fromType.isDbl()) {
    push(gen(ConvDblToArr, src));
  } else if (fromType.isInt()) {
    push(gen(ConvIntToArr, src));
  } else if (fromType.isString()) {
    push(gen(ConvStrToArr, src));
  } else if (fromType.isObj()) {
    push(gen(ConvObjToArr, src));
  } else {
    push(gen(ConvCellToArr, src));
  }
}

void HhbcTranslator::emitCastBool() {
  auto const src = popC();
  push(gen(ConvCellToBool, src));
  gen(DecRef, src);
}

void HhbcTranslator::emitCastDouble() {
  SSATmp* src = popC();
  Type fromType = src->type();
  if (fromType.isDbl()) {
    push(src);
  } else if (fromType.isNull()) {
    push(cns(0.0));
  } else if (fromType.isArray()) {
    push(gen(ConvArrToDbl, src));
    gen(DecRef, src);
  } else if (fromType.isBool()) {
    push(gen(ConvBoolToDbl, src));
  } else if (fromType.isInt()) {
    push(gen(ConvIntToDbl, src));
  } else if (fromType.isString()) {
    push(gen(ConvStrToDbl, src));
  } else if (fromType.isObj()) {
    exceptionBarrier();
    push(gen(ConvObjToDbl, src));
  } else {
    exceptionBarrier(); // may throw
    push(gen(ConvCellToDbl, src));
  }
}

void HhbcTranslator::emitCastInt() {
  SSATmp* src = popC();
  Type fromType = src->type();
  if (fromType.isInt()) {
    push(src);
  } else if (fromType.isNull()) {
    push(cns(0));
  } else if (fromType.isArray()) {
    push(gen(ConvArrToInt, src));
    gen(DecRef, src);
  } else if (fromType.isBool()) {
    push(gen(ConvBoolToInt, src));
  } else if (fromType.isDbl()) {
    push(gen(ConvDblToInt, src));
  } else if (fromType.isString()) {
    push(gen(ConvStrToInt, src));
    gen(DecRef, src);
  } else if (fromType.isObj()) {
    exceptionBarrier();
    push(gen(ConvObjToInt, src));
  } else {
    exceptionBarrier();
    push(gen(ConvCellToInt, src));
  }
}

void HhbcTranslator::emitCastObject() {
  SSATmp* src = popC();
  Type srcType = src->type();
  if (srcType.isObj()) {
    push(src);
  } else {
    push(gen(ConvCellToObj, src));
  }
}

void HhbcTranslator::emitCastString() {
  SSATmp* src = popC();
  Type fromType = src->type();
  if (fromType.isString()) {
    push(src);
  } else if (fromType.isNull()) {
    push(cns(StringData::GetStaticString("")));
  } else if (fromType.isArray()) {
    push(cns(StringData::GetStaticString("Array")));
    gen(DecRef, src);
  } else if (fromType.isBool()) {
    push(gen(ConvBoolToStr, src));
  } else if (fromType.isDbl()) {
    push(gen(ConvDblToStr, src));
  } else if (fromType.isInt()) {
    push(gen(ConvIntToStr, src));
  } else if (fromType.isObj()) {
    exceptionBarrier();
    push(gen(ConvObjToStr, src));
  } else {
    exceptionBarrier();
    push(gen(ConvCellToStr, src));
  }
}

static
bool isSupportedAGet(SSATmp* classSrc, const StringData* clsName) {
  return (classSrc->isA(Type::Obj) || classSrc->isA(Type::Str) || clsName);
}

void HhbcTranslator::emitAGet(SSATmp* classSrc, const StringData* clsName) {
  if (classSrc->isA(Type::Str)) {
    push(gen(LdCls, classSrc, cns(getCurClass())));
  } else if (classSrc->isA(Type::Obj)) {
    push(gen(LdObjClass, classSrc));
  } else if (clsName) {
    push(gen(LdCls, cns(clsName), cns(getCurClass())));
  } else {
    not_reached();
  }
}

void HhbcTranslator::emitAGetC(const StringData* clsName) {
  if (isSupportedAGet(topC(), clsName)) {
    SSATmp* src = popC();
    emitAGet(src, clsName);
    gen(DecRef, src);
  } else {
    emitInterpOne(Type::Cls, 1);
  }
}

void HhbcTranslator::emitAGetL(int id, const StringData* clsName) {
  auto const src = ldLocInner(id, getExitTrace());
  if (isSupportedAGet(src, clsName)) {
    emitAGet(src, clsName);
  } else {
    PUNT(AGetL); // need to teach interpone about local uses
  }
}

void HhbcTranslator::emitBindMem(SSATmp* ptr, SSATmp* src) {
  SSATmp* prevValue = gen(LdMem, ptr->type().deref(), ptr, cns(0));
  pushIncRef(src);
  gen(StMem, ptr, cns(0), src);
  if (isRefCounted(src) && src->type().canRunDtor()) {
    Block* exitBlock = getExitTrace(getNextSrcKey().offset())->front();
    Block::iterator markerInst = exitBlock->skipLabel();
    exitBlock->insert(++markerInst, m_irFactory.gen(DecRef, prevValue));
    gen(DecRefNZOrBranch, exitBlock, prevValue);
  } else {
    gen(DecRef, prevValue);
  }
}

template<class CheckSupportedFun, class EmitLdAddrFun>
void HhbcTranslator::emitBind(const StringData* name,
                              CheckSupportedFun checkSupported,
                              EmitLdAddrFun emitLdAddr) {
  if (!(this->*checkSupported)(name, topC(0)->type(), 1)) return;
  SSATmp* src = popV();
  emitBindMem((this->*emitLdAddr)(name), src);
}

void HhbcTranslator::emitSetMem(SSATmp* ptr, SSATmp* src) {
  emitBindMem(gen(UnboxPtr, ptr), src);
}

template<class CheckSupportedFun, class EmitLdAddrFun>
void HhbcTranslator::emitSet(const StringData* name,
                             CheckSupportedFun checkSupported,
                             EmitLdAddrFun emitLdAddr) {
  if (!(this->*checkSupported)(name, topC(0)->type(), 1)) return;
  SSATmp* src = popC();
  emitSetMem((this->*emitLdAddr)(name), src);
}

void HhbcTranslator::emitVGetMem(SSATmp* ptr) {
  pushIncRef(
    gen(LdMem, Type::BoxedCell, gen(BoxPtr, ptr), cns(0))
  );
}

template<class CheckSupportedFun, class EmitLdAddrFun>
void HhbcTranslator::emitVGet(const StringData* name,
                              CheckSupportedFun checkSupported,
                              EmitLdAddrFun emitLdAddr) {
  if (!(this->*checkSupported)(name, Type::BoxedCell, 0)) return;
  emitVGetMem((this->*emitLdAddr)(name));
}

template<class CheckSupportedFun, class EmitLdAddrFun>
void HhbcTranslator::emitIsset(const StringData* name,
                               CheckSupportedFun checkSupported,
                               EmitLdAddrFun emitLdAddr) {
  if (!(this->*checkSupported)(name, Type::Bool, 0)) return;
  SSATmp* ptr = nullptr;
  SSATmp* result = m_tb->cond(getCurFunc(),
                        [&] (Block* taken) { // branch
                          ptr = (this->*emitLdAddr)(name, taken);
                        },
                        [&] { // Next: property or global is defined
                          return gen(IsNTypeMem, Type::Null,
                                           gen(UnboxPtr, ptr));
                        },
                        [&] { // Taken
                          return cns(false);
                        }
  );
  push(result);
}

void HhbcTranslator::emitEmptyMem(SSATmp* ptr) {
  SSATmp* ld = gen(LdMem, Type::Cell, gen(UnboxPtr, ptr), cns(0));
  push(gen(OpNot, gen(ConvCellToBool, ld)));
}

template<class CheckSupportedFun, class EmitLdAddrFun>
void HhbcTranslator::emitEmpty(const StringData* name,
                               CheckSupportedFun checkSupported,
                               EmitLdAddrFun emitLdAddr) {
  if (!(this->*checkSupported)(name, Type::Bool, 0)) return;
  SSATmp* ptr = nullptr;
  SSATmp* result = m_tb->cond(getCurFunc(),
                        [&] (Block* taken) {
                          ptr = (this->*emitLdAddr)(name, taken);
                        },
                        [&] { // Next: property or global is defined
                          SSATmp* ld = gen(
                            LdMem,
                            Type::Cell,
                            gen(UnboxPtr, ptr),
                            cns(0)
                          );
                          return gen(OpNot, gen(ConvCellToBool, ld));
                        },
                        [&] { // Taken
                          return cns(true);
                        }
  );
  push(result);
}

void HhbcTranslator::emitBindG(const StringData* gblName) {
  emitBind(gblName,
           &HhbcTranslator::checkSupportedGblName,
           &HhbcTranslator::emitLdGblAddrDef);
}

void HhbcTranslator::emitBindS(const StringData* propName) {
  emitBind(propName,
           &HhbcTranslator::checkSupportedClsProp,
           &HhbcTranslator::emitLdClsPropAddr);
}

void HhbcTranslator::emitVGetG(const StringData* gblName) {
  emitVGet(gblName,
           &HhbcTranslator::checkSupportedGblName,
           &HhbcTranslator::emitLdGblAddrDef);
}

void HhbcTranslator::emitVGetS(const StringData* propName) {
  emitVGet(propName,
           &HhbcTranslator::checkSupportedClsProp,
           &HhbcTranslator::emitLdClsPropAddr);
}

void HhbcTranslator::emitSetG(const StringData* gblName) {
  emitSet(gblName,
          &HhbcTranslator::checkSupportedGblName,
          &HhbcTranslator::emitLdGblAddrDef);
}

void HhbcTranslator::emitSetS(const StringData* propName) {
  emitSet(propName,
          &HhbcTranslator::checkSupportedClsProp,
          &HhbcTranslator::emitLdClsPropAddr);
}

static Type getResultType(Type resultType, bool isInferedType) {
  assert(!isInferedType || resultType.isKnownUnboxedDataType());
  if (resultType.equals(Type::None)) {
    // result type neither predicted nor inferred
    return Type::Cell;
  }
  assert(resultType.isKnownUnboxedDataType());
  return resultType;
}

template<class CheckSupportedFun, class EmitLdAddrFun>
void HhbcTranslator::emitCGet(const StringData* name,
                              Type resultType,
                              bool isInferedType,
                              bool exitOnFailure,
                              CheckSupportedFun checkSupported,
                              EmitLdAddrFun emitLdAddr) {
  resultType = getResultType(resultType, isInferedType);
  if (!(this->*checkSupported)(name, resultType, 0)) return;
  Trace* exit = (isInferedType || resultType.equals(Type::Cell))
                ? nullptr : getExitSlowTrace();
  SSATmp* ptr = (this->*emitLdAddr)(name,
                                    exitOnFailure
                                      ? getExitSlowTrace()->front()
                                      : nullptr);
  if (!isInferedType) ptr = gen(UnboxPtr, ptr);
  pushIncRef(gen(LdMem, resultType, exit, ptr, cns(0)));
}

void HhbcTranslator::emitCGetG(const StringData* gblName,
                               Type resultType,
                               bool isInferedType) {
  emitCGet(gblName, resultType, isInferedType, true,
           &HhbcTranslator::checkSupportedGblName,
           &HhbcTranslator::emitLdGblAddr);
}

void HhbcTranslator::emitCGetS(const StringData* propName,
                               Type resultType,
                               bool isInferedType) {
  emitCGet(propName, resultType, isInferedType, false,
           &HhbcTranslator::checkSupportedClsProp,
           &HhbcTranslator::emitLdClsPropAddrOrExit);
}

void HhbcTranslator::emitBinaryArith(Opcode opc) {
  bool isBitOp = (opc == OpAnd || opc == OpOr || opc == OpXor);
  Type type1 = topC(0)->type();
  Type type2 = topC(1)->type();
  if (isSupportedBinaryArith(opc, type1, type2)) {
    SSATmp* tr = popC();
    SSATmp* tl = popC();
    push(gen(opc, tl, tr));
  } else if (isBitOp && (type1 == Type::Obj || type2 == Type::Obj)) {
    // raise fatal
    emitInterpOne(Type::Cell, 2);
  } else {
    Type type = Type::Int;
    if (isBitOp) {
      if (type1.isString() && type2.isString()) {
        type = Type::Str;
      } else if ((type1.needsReg() && (type2.needsReg() || type2.isString()))
                 || (type2.needsReg() && type1.isString())) {
        // both types might be strings, but can't tell
        type = Type::Cell;
      } else {
        type = Type::Int;
      }
    } else {
      // either an int or a dbl, but can't tell
      type = Type::Cell;
    }
    emitInterpOne(type, 2);
  }
}

void HhbcTranslator::emitNot() {
  SSATmp* src = popC();
  push(gen(OpNot, gen(ConvCellToBool, src)));
  gen(DecRef, src);
}

#define BINOP(Opp) \
void HhbcTranslator::emit ## Opp() {  \
  emitBinaryArith(Op ## Opp);         \
}

BINOP(Add)
BINOP(Sub)
BINOP(Mul)
BINOP(BitAnd)
BINOP(BitOr)
BINOP(BitXor)

#undef BINOP

void HhbcTranslator::emitDiv() {
  emitInterpOne(Type::Cell, 2);
}

void HhbcTranslator::emitMod() {
  // XXX: Disabled until t2299606 is fixed
  PUNT(emitMod);

  auto tl = topC(1)->type();
  auto tr = topC(0)->type();
  auto isInty = [&](Type t) {
    return t.subtypeOf(Type::Null | Type::Bool | Type::Int);
  };
  if (!(isInty(tl) && isInty(tr))) {
    emitInterpOne(Type::Cell, 2);
    return;
  }
  SSATmp* r = popC();
  SSATmp* l = popC();
  // Exit path spills an additional false
  auto exitSpillValues = getSpillValues();
  exitSpillValues.push_back(cns(false));
  // Generate an exit for the rare case that r is zero
  auto exit =
    m_tb->ifThenExit(
      getCurFunc(),
      m_stackDeficit,
      exitSpillValues,
      [&](IRFactory* irf, Trace* t) {
        // Dividing by zero. Interpreting will raise a notice and
        // produce the boolean false. Punch out here and resume after
        // the Mod instruction; this should be rare.
        m_tb->genFor(t, RaiseWarning,
                     cns(StringData::GetStaticString(
                         Strings::DIVISION_BY_ZERO)));
      },
      getNextSrcKey().offset() /* exitBcOff */,
      bcOff()
    );
  gen(JmpZero, exit, r);
  push(gen(OpMod, l, r));
}

void HhbcTranslator::emitBitNot() {
  Type srcType = topC()->type();
  if (srcType == Type::Int) {
    SSATmp* src = popC();
    SSATmp* ones = cns(~uint64_t(0));
    push(gen(OpXor, src, ones));
  } else if (srcType.subtypeOf(Type::Null | Type::Bool | Type::Arr | Type::Obj)) {
    // raise fatal
    emitInterpOne(Type::Cell, 1);
  } else {
    Type resultType = Type::Int;
    if (srcType.isString()) {
      resultType = Type::Str;
    } else if (srcType.needsReg()) {
      resultType = Type::Cell;
    }
    emitInterpOne(resultType, 1);
  }
}

void HhbcTranslator::emitXor() {
  SSATmp* btr = popC();
  SSATmp* btl = popC();
  SSATmp* tr = gen(ConvCellToBool, btr);
  SSATmp* tl = gen(ConvCellToBool, btl);
  push(gen(ConvCellToBool, gen(OpXor, tl, tr)));
  gen(DecRef, btl);
  gen(DecRef, btr);
}

/**
 * Emit InterpOne instruction.
 *   - 'type' is the return type of the value the instruction pushes on
 *            the stack if any (or Type:None if none)
 *   - 'numPopped' is the number of cells that this instruction pops
 *   - 'numExtraPushed' is the number of cells this instruction pushes on
 *            the stack, in addition to the cell corresponding to 'type'
 */
void HhbcTranslator::emitInterpOne(Type type, int numPopped,
                                   int numExtraPushed) {
  exceptionBarrier();
  // discard the top elements of the stack, which are consumed by this instr
  discard(numPopped);
  assert(numPopped == m_stackDeficit);
  int numPushed = (type == Type::None ? 0 : 1) + numExtraPushed;
  gen(
    InterpOne,
    type,
    m_tb->getFp(),
    m_tb->getSp(),
    cns(bcOff()),
    cns(numPopped - numPushed)
  );
  m_stackDeficit = 0;
}

void HhbcTranslator::emitInterpOneCF(int numPopped) {
  exceptionBarrier();
  // discard the top elements of the stack, which are consumed by this instr
  discard(numPopped);
  assert(numPopped == m_stackDeficit);
  gen(InterpOneCF, m_tb->getFp(), m_tb->getSp(), cns(bcOff()));
  m_stackDeficit = 0;
  m_hasExit = true;
}

void HhbcTranslator::emitInterpOneOrPunt(Type type, int numPopped,
                                         int numExtraPushed) {
  if (RuntimeOption::EvalIRPuntDontInterp) {
    Op op = *(Op*)(getCurUnit()->entry() + bcOff());
    const char* name = StringData::GetStaticString(
      std::string("PuntDontInterp-") + opcodeToName(op))->data();
    SPUNT(name);
  } else {
    emitInterpOne(type, numPopped, numExtraPushed);
  }
}

Trace* HhbcTranslator::getGuardExit() {
  assert(bcOff() == -1 || bcOff() == m_startBcOff);
  assert(!isInlining());
  // stack better be empty since we're at the start of the trace
  assert((m_evalStack.numCells() - m_stackDeficit) == 0);
  return m_exitGuardFailureTrace;
}

/*
 * Get SSATmps representing all the information on the virtual eval
 * stack in preparation for a spill or exit trace.
 *
 * Doesn't actually remove these values from the eval stack.
 */
std::vector<SSATmp*> HhbcTranslator::getSpillValues() const {
  std::vector<SSATmp*> ret;
  ret.reserve(m_evalStack.size());
  for (int i = 0; i < m_evalStack.size(); ++i) {
    SSATmp* elem = m_evalStack.top(i);
    ret.push_back(elem);
  }
  return ret;
}

/*
 * Generates an exit trace which will continue execution without HHIR.
 * This should be used in situations that HHIR cannot handle -- ideally
 * only in slow paths.
 */
Trace* HhbcTranslator::getExitSlowTrace() {
  auto stackValues = getSpillValues();
  return m_tb->getExitSlowTrace(bcOff(),
                                m_stackDeficit,
                                stackValues.size(),
                                stackValues.size() ? &stackValues[0] : 0);
}

/*
 * Generates an exit trace for the given targetBcOff
 * (defaults to the current offset).
 * The exit trace returned will be linked to a translation starting at
 * targetBcOff, which will be a retranslation of the same tracelet if this
 * exit is taken before executing any bytecode instruction of the current
 * tracelet.
 */
Trace* HhbcTranslator::getExitTrace(Offset targetBcOff /* = -1 */) {
  if (targetBcOff == -1) {
    targetBcOff = bcOff() != -1 ? bcOff() : m_startBcOff;
  }
  if (targetBcOff == m_startBcOff) {
    return m_exitGuardFailureTrace;
  }

  std::vector<SSATmp*> stackValues = getSpillValues();
  return m_tb->genExitTrace(targetBcOff,
                            m_stackDeficit,
                            stackValues.size(),
                            stackValues.size() ? &stackValues[0] : nullptr,
                            TraceExitType::Normal);
}

/*
 * Generates a trace exit that can be the target of a conditional
 * control flow instruction at the current bytecode offset.
 */
Trace* HhbcTranslator::getExitTrace(uint32_t targetBcOff,
                                    uint32_t notTakenBcOff) {
  std::vector<SSATmp*> stackValues = getSpillValues();
  return m_tb->genExitTrace(targetBcOff,
                           m_stackDeficit,
                           stackValues.size(),
                           stackValues.size() ? &stackValues[0] : nullptr,
                           TraceExitType::NormalCc,
                           notTakenBcOff);
}

SSATmp* HhbcTranslator::spillStack() {
  auto ssaArgs = getSpillValues();
  ssaArgs.insert(
    ssaArgs.begin(),
    { m_tb->getSp(), cns(int64_t(m_stackDeficit)) }
  );
  m_evalStack.clear();
  m_stackDeficit = 0;
  return gen(SpillStack, std::make_pair(ssaArgs.size(), &ssaArgs[0]));
}

void HhbcTranslator::exceptionBarrier() {
  auto const sp = spillStack();
  gen(ExceptionBarrier, sp);
}

SSATmp* HhbcTranslator::ldStackAddr(int32_t offset) {
  // You're almost certainly doing it wrong if you want to get the address of a
  // stack cell that's in m_evalStack.
  assert(offset >= (int32_t)m_evalStack.numCells());
  return gen(
    LdStackAddr,
    Type::PtrToGen,
    StackOffset(offset + m_stackDeficit - m_evalStack.numCells()),
    m_tb->getSp()
  );
}

SSATmp* HhbcTranslator::ldLoc(uint32_t locId) {
  return gen(
    LdLoc,
    Type::Gen,
    LocalId(locId),
    m_tb->getFp()
  );
}

SSATmp* HhbcTranslator::ldLocAddr(uint32_t locId) {
  return gen(
    LdLocAddr,
    Type::PtrToGen,
    LocalId(locId),
    m_tb->getFp()
  );
}

/*
 * Load a local, and if it's boxed dereference to get the inner cell.
 *
 * Note: For boxed values, this will generate a LdRef instruction which
 *       takes the given exit trace in case the inner type doesn't match
 *       the tracked type for this local.  This check may be optimized away
 *       if we can determine that the inner type must match the tracked type.
 */
SSATmp* HhbcTranslator::ldLocInner(uint32_t locId, Trace* exitTrace) {
  auto loc = ldLoc(locId);
  assert((loc->type().isBoxed() || loc->type().notBoxed()) &&
         "Currently we don't handle traces where locals are maybeBoxed");
  return loc->type().isBoxed()
    ? gen(LdRef, loc->type().innerType(), exitTrace, loc)
    : loc;
}

/*
 * This is a wrapper to ldLocInner that also emits the RaiseUninitLoc
 * if the local is uninitialized
 */
SSATmp* HhbcTranslator::ldLocInnerWarn(uint32_t id, Trace* target) {
  auto const locVal = ldLocInner(id, target);

  if (locVal->type().subtypeOf(Type::Uninit)) {
    exceptionBarrier();
    gen(RaiseUninitLoc, cns(getCurFunc()->localVarName(id)));
    return m_tb->genDefInitNull();
  }

  return locVal;
}

/*
 * Store to a local, if it's boxed set the value on the inner cell.
 *
 * Returns the value that was stored to the local, after incrementing
 * its reference count.
 *
 * Pre: !newVal->type().isBoxed() && !newVal->type().maybeBoxed()
 * Pre: exitTrace != nullptr if the local may be boxed
 */
SSATmp* HhbcTranslator::stLocImpl(uint32_t id,
                                  Trace* exitTrace,
                                  SSATmp* newVal,
                                  bool doRefCount) {
  assert(!newVal->type().maybeBoxed());

  auto const oldLoc = ldLoc(id);
  assert((oldLoc->type().isBoxed() || oldLoc->type().notBoxed()) &&
         "We don't support maybeBoxed locals right now");

  if (oldLoc->type().notBoxed()) {
    gen(StLoc, LocalId(id), m_tb->getFp(), newVal);
    auto const ret = doRefCount ? gen(IncRef, newVal) : newVal;
    if (doRefCount) {
      gen(DecRef, oldLoc);
    }
    return ret;
  }

  // It's important that the IncRef happens after the LdRef, since the
  // LdRef is also a guard on the inner type and may side-exit.
  assert(exitTrace);
  auto const innerCell = gen(
    LdRef, oldLoc->type().innerType(), exitTrace, oldLoc
  );
  auto const ret = doRefCount ? gen(IncRef, newVal) : newVal;
  gen(StRef, oldLoc, newVal);
  if (doRefCount) {
    gen(DecRef, innerCell);
  }

  return ret;
}

SSATmp* HhbcTranslator::stLoc(uint32_t id, Trace* exit, SSATmp* newVal) {
  const bool doRefCount = true;
  return stLocImpl(id, exit, newVal, doRefCount);
}

SSATmp* HhbcTranslator::stLocNRC(uint32_t id, Trace* exit, SSATmp* newVal) {
  const bool doRefCount = false;
  return stLocImpl(id, exit, newVal, doRefCount);
}

void HhbcTranslator::end(int nextPc) {
  if (m_hasExit) return;

  if (nextPc >= getCurFunc()->past()) {
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
  spillStack();
  m_tb->genTraceEnd(nextPc);
}

}}} // namespace HPHP::VM::JIT
