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
    return m_tb->genLdStack(stackOff, type);
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
    m_tb->genDecRef(src);
    return;
  }

  uint32_t stackOff = m_stackDeficit;
  m_tb->genDecRefStack(type, stackOff);
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
      m_tb->gen(
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
  auto calleeFP  = m_tb->gen(DefInlineFP, BCOffset(returnBcOffset), calleeSP);

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
  m_tb->gen(Marker, marker);
}

void HhbcTranslator::profileFunctionEntry(const char* category) {
  static const bool enabled = Stats::enabledAny() &&
                              getenv("HHVM_STATS_FUNCENTRY");
  if (!enabled) return;

  m_tb->gen(
    IncStatGrouped,
    cns(StringData::GetStaticString("FunctionEntry")),
    cns(StringData::GetStaticString(category)),
    cns(1)
  );
}

void HhbcTranslator::profileInlineFunctionShape(const std::string& str) {
  m_tb->gen(
    IncStatGrouped,
    cns(StringData::GetStaticString("InlineShape")),
    cns(StringData::GetStaticString(str)),
    cns(1)
  );
}

void HhbcTranslator::profileSmallFunctionShape(const std::string& str) {
  m_tb->gen(
    IncStatGrouped,
    cns(StringData::GetStaticString("SmallFunctions")),
    cns(StringData::GetStaticString(str)),
    cns(1)
  );
}

void HhbcTranslator::profileFailedInlShape(const std::string& str) {
  m_tb->gen(
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
      m_tb->gen(op, cell);
    }
    push(m_tb->genDefConst<int64_t>(1));
  } else {
    emitInterpOne(Type::Int, 1);
  }
}

void HhbcTranslator::emitUnboxRAux() {
  Block* exit = getExitTrace()->front();
  SSATmp* srcBox = popR();
  SSATmp* unboxed = m_tb->gen(Unbox, exit, srcBox);
  if (unboxed == srcBox) {
    // If the Unbox ended up being a noop, don't bother refcounting
    push(unboxed);
  } else {
    pushIncRef(unboxed);
    m_tb->genDecRef(srcBox);
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
  pushIncRef(m_tb->genLdThis(getExitSlowTrace()));
}

void HhbcTranslator::emitCheckThis() {
  if (!getCurClass()) {
    emitInterpOne(Type::None, 0); // will throw a fatal
    return;
  }
  m_tb->genLdThis(getExitSlowTrace());
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
  pushIncRef(m_tb->genLdThis(getExitSlowTrace()));
}

void HhbcTranslator::emitArray(int arrayId) {
  ArrayData* ad = lookupArrayId(arrayId);
  push(m_tb->genDefConst(ad));
}

void HhbcTranslator::emitNewArray(int capacity) {
  if (capacity == 0) {
    ArrayData* ad = HphpArray::GetStaticEmptyArray();
    push(m_tb->genDefConst(ad));
  } else {
    push(m_tb->genNewArray(capacity));
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
  push(m_tb->genNewTuple(numArgs, sp));
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
  push(m_tb->gen(ArrayAdd, tl, tr));
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

  push(m_tb->gen(op, arr, key, val));
}

void HhbcTranslator::emitAddNewElemC() {
  SSATmp* val = popC();
  SSATmp* arr = popC();
  // the AddNewElem helper decrefs its args, so don't decref pop'ed values
  // TODO task 1805916: verify that NewElem increfs its result
  push(m_tb->gen(AddNewElem, arr, val));
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
  SSATmp* cnsNameTmp = m_tb->genDefConst(name);
  const TypedValue* tv = Unit::lookupPersistentCns(name);
  SSATmp* result = nullptr;
  Type cnsType = Type::Cell;
  if (tv) {
    switch (tv->m_type) {
      case KindOfUninit:
        // a dynamic system constant. always a slow lookup
        result = m_tb->gen(LookupCns, cnsType, cnsNameTmp);
        break;
      case KindOfBoolean:
        result = m_tb->genDefConst((bool)tv->m_data.num);
        break;
      case KindOfInt64:
        result = m_tb->genDefConst(tv->m_data.num);
        break;
      case KindOfDouble:
        result = m_tb->genDefConst(tv->m_data.dbl);
        break;
      case KindOfString:
      case KindOfStaticString:
        result = m_tb->genDefConst(tv->m_data.pstr);
        break;
      default:
        not_reached();
    }
  } else {
    spillStack(); // do this on main trace so we update stack tracking once.
    SSATmp* c1 = m_tb->gen(LdCns, cnsType, cnsNameTmp);
    result = m_tb->cond(
      getCurFunc(),
      [&] (Block* taken) { // branch
        m_tb->gen(CheckInit, taken, c1);
      },
      [&] { // Next: LdCns hit in TC
        return c1;
      },
      [&] { // Taken: miss in TC, do lookup & init
        m_tb->hint(Block::Unlikely);
        return m_tb->gen(LookupCns, cnsType, cnsNameTmp);
      }
    );
  }
  push(result);
}

void HhbcTranslator::emitDefCns(uint32_t id) {
  StringData* name = lookupStringId(id);
  SSATmp* val = popC();
  push(m_tb->genDefCns(name, val));
}

void HhbcTranslator::emitConcat() {
  SSATmp* tr = popC();
  SSATmp* tl = popC();
  // the concat helpers decref their args, so don't decref pop'ed values
  push(m_tb->genConcat(tl, tr));
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
  push(m_tb->gen(LdClsCtx, m_tb->genLdCtx(getCurFunc())));
}

void HhbcTranslator::emitSelf() {
  Class* clss = getCurClass();
  if (clss == nullptr) {
    emitInterpOne(Type::Cls, 0);
  } else {
    push(m_tb->genDefConst(clss));
  }
}

void HhbcTranslator::emitParent() {
  auto const clss = getCurClass();
  if (clss == nullptr || clss->parent() == nullptr) {
    emitInterpOne(Type::Cls, 0);
  } else {
    push(m_tb->genDefConst(clss->parent()));
  }
}

void HhbcTranslator::emitString(int strId) {
  push(m_tb->genDefConst(lookupStringId(strId)));
}

void HhbcTranslator::emitInt(int64_t val) {
  push(m_tb->genDefConst(val));
}

void HhbcTranslator::emitDouble(double val) {
  push(m_tb->genDefConst(val));
}

void HhbcTranslator::emitNullUninit() {
  push(m_tb->genDefUninit());
}

void HhbcTranslator::emitNull() {
  push(m_tb->genDefInitNull());
}

void HhbcTranslator::emitTrue() {
  push(m_tb->genDefConst(true));
}

void HhbcTranslator::emitFalse() {
  push(m_tb->genDefConst(false));
}

void HhbcTranslator::emitUninitLoc(uint32_t id) {
  m_tb->genInitLoc(id, m_tb->genDefUninit());
}

void HhbcTranslator::emitInitThisLoc(int32_t id) {
  if (!getCurClass()) {
    // Do nothing if this is null
    return;
  }
  SSATmp* tmpThis = m_tb->genLdThis(getExitSlowTrace());
  m_tb->genInitLoc(id, m_tb->genIncRef(tmpThis));
}

void HhbcTranslator::emitCGetL(int32_t id) {
  Trace* exitTrace = getExitTrace();
  pushIncRef(emitLdLocWarn(id, exitTrace));
}

void HhbcTranslator::emitCGetL2(int32_t id) {
  Trace* exitTrace = getExitTrace();
  SSATmp* oldTop = pop(Type::Gen);
  pushIncRef(emitLdLocWarn(id, exitTrace));
  push(oldTop);
}

void HhbcTranslator::emitVGetL(int32_t id) {
  pushIncRef(m_tb->genBoxLoc(id));
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
  m_tb->genBindLoc(id, m_tb->genDefUninit());
}

void HhbcTranslator::emitBindL(int32_t id) {
  SSATmp* src = popV();
  if (getCurFunc()->isPseudoMain()) {
    // in pseudo mains, the value of locals could change in functions
    // called explicitly (or implicitly via exceptions or destructors)
    // so we need to incref eagerly in case one of these functions
    // changes the value of our local and makes src dead.
    pushIncRef(src);
  }
  m_tb->genBindLoc(id, src);
  if (!getCurFunc()->isPseudoMain()) {
    pushIncRef(src);
  }
}

void HhbcTranslator::emitSetL(int32_t id) {
  Trace* exitTrace = getExitTrace();
  SSATmp* src = popC();
  // Note we can't use the same trick as emitBindL in which we
  // move the incref to after the store because the stored location
  // might contain a ref, which could then be modified by the decref
  // inserted after the stloc
  push(m_tb->genStLoc(id, src, true, true, exitTrace));
}

void HhbcTranslator::emitIncDecL(bool pre, bool inc, uint32_t id) {
  // Handle only integer inc/dec for now
  Trace* exitTrace = getExitSlowTrace();
  SSATmp* src = m_tb->genLdLocAsCell(id, exitTrace);
  if (src->isA(Type::Bool)) {
    // inc/dec of a bool is a no-op
    push(src);
  } else {
    SSATmp* res = emitIncDec(pre, inc, src);
    m_tb->genStLoc(id, res, false, false, nullptr);
  }
}

// only handles integer or double inc/dec
SSATmp* HhbcTranslator::emitIncDec(bool pre, bool inc, SSATmp* src) {
  assert(src->isA(Type::Int) || src->isA(Type::Dbl));
  SSATmp* one = src->isA(Type::Int) ? m_tb->genDefConst(1)
                                    : m_tb->genDefConst(1.0);
  SSATmp* res = inc ? m_tb->genAdd(src, one) : m_tb->genSub(src, one);
  // no incref necessary on push since result is an int
  push(pre ? res : src);
  return res;
}

void HhbcTranslator::emitIncDecMem(bool pre,
                                   bool inc,
                                   SSATmp* propAddr,
                                   Trace* exitTrace) {
  // Handle only integer inc/dec for now
  SSATmp* src = m_tb->genLdMem(propAddr, Type::Int, exitTrace);
  // do the add and store back
  SSATmp* res = emitIncDec(pre, inc, src);
  // don't gen a dec ref or type store
  m_tb->genStMem(propAddr, res, false);
}

static bool isSupportedBinaryArith(Opcode opc,
                                   Type t1,
                                   Type t2) {
  switch (opc) {
    // Opcodes supporting FP
    case OpAdd:
    case OpSub:
    case OpMul: return (t1.subtypeOf(Type::Int | Type::Bool | Type::Dbl) &&
                        t2.subtypeOf(Type::Int | Type::Bool | Type::Dbl));

    default:    return (t1.subtypeOf(Type::Int | Type::Bool) &&
                        t2.subtypeOf(Type::Int | Type::Bool));
  }
}

void HhbcTranslator::emitSetOpL(Opcode subOpc, uint32_t id) {
  Trace* exitTrace = getExitTrace();
  SSATmp* loc = emitLdLocWarn(id, exitTrace);
  if (subOpc == Concat) {
    // The concat helpers decref their args, so don't decref pop'ed values
    // and don't decref the old value held in the local. The concat helpers
    // also incref their results, which will be consumed by the stloc. We
    // need an extra incref for the push onto the stack.
    SSATmp* val = popC();
    SSATmp* result = m_tb->genConcat(loc, val);
    pushIncRef(m_tb->genStLoc(id, result, false, true, exitTrace));
  } else if (isSupportedBinaryArith(subOpc,
                                    loc->type(),
                                    topC()->type())) {
    SSATmp* val = popC();
    Type resultType = Type::binArithResultType(loc->type(),
                                               val->type());
    SSATmp* result = m_tb->gen(subOpc, resultType, loc, val);
    push(m_tb->genStLoc(id, result, true, true, exitTrace));
  } else {
    PUNT(SetOpL);
  }
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
    box = m_tb->gen(StaticLocInit, cns(name), m_tb->getFp(), value);
  } else {
    SSATmp* ch =
      m_tb->genDefConst(TargetCache::allocStatic(), Type::CacheHandle);
    SSATmp* cachedBox = nullptr;
    box = m_tb->cond(getCurFunc(),
      [&](Block* taken) {
        // Careful: cachedBox is only ok to use in the 'next' branch.
        cachedBox = m_tb->gen(LdStaticLocCached, taken, ch);
      },
      [&] { // next: The local is already initialized
        return m_tb->gen(IncRef, cachedBox);
      },
      [&] { // taken: We missed in the cache
        m_tb->hint(Block::Unlikely);
        return m_tb->gen(StaticLocInitCached,
                         cns(name), m_tb->getFp(), value, ch);
      }
    );
  }
  m_tb->gen(StLoc, LocalId(locId), m_tb->getFp(), box);
  m_tb->gen(DecRef, value);
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
    return m_tb->genIterInit(src, iterId, valLocalId);
  });
}

void HhbcTranslator::emitIterInitK(uint32_t iterId,
                                   int offset,
                                   uint32_t valLocalId,
                                   uint32_t keyLocalId) {
  emitIterInitCommon(offset, [=] (SSATmp* src) {
    return m_tb->genIterInitK(src, iterId, valLocalId, keyLocalId);
  });
}

void HhbcTranslator::emitIterNext(uint32_t iterId,
                                  int offset,
                                  uint32_t valLocalId) {
  SSATmp* res = m_tb->genIterNext(iterId, valLocalId);
  emitJmpCondHelper(offset, false, res);
}

void HhbcTranslator::emitIterNextK(uint32_t iterId,
                                   int offset,
                                   uint32_t valLocalId,
                                   uint32_t keyLocalId) {
  SSATmp* res = m_tb->genIterNextK(iterId, valLocalId, keyLocalId);
  emitJmpCondHelper(offset, false, res);
}

void HhbcTranslator::emitIterFree(uint32_t iterId) {
  m_tb->genIterFree(iterId);
}

void HhbcTranslator::emitCreateCont(bool getArgs,
                                    Id funNameStrId) {
  /* Runtime-determined slow path punts to TranslatorX64 for now */
  m_tb->genExitOnVarEnv(getExitSlowTrace());

  const StringData* genName = lookupStringId(funNameStrId);
  const Func* origFunc = getCurFunc();
  const Func* genFunc = origFunc->getGeneratorBody(genName);
  int origLocals = origFunc->numLocals();
  int genLocals = genFunc->numLocals();

  TCA helper = origFunc->isMethod() ?
    (TCA)&VMExecutionContext::createContinuation<true> :
    (TCA)&VMExecutionContext::createContinuation<false>;
  SSATmp* cont = m_tb->gen(CreateCont, cns(helper), m_tb->getFp(),
                           cns(getArgs), cns(origFunc), cns(genFunc));

  TranslatorX64::ContParamMap params;
  if (origLocals <= TranslatorX64::kMaxInlineContLocals &&
      TranslatorX64::mapContParams(params, origFunc, genFunc)) {
    static const StringData* thisStr = StringData::GetStaticString("this");
    Id thisId = kInvalidId;
    bool fillThis = origFunc->isNonClosureMethod() && !origFunc->isStatic() &&
      ((thisId = genFunc->lookupVarId(thisStr)) != kInvalidId) &&
      (origFunc->lookupVarId(thisStr) == kInvalidId);
    SSATmp* locals = m_tb->gen(LdContLocalsPtr, cont);
    for (int i = 0; i < origLocals; ++i) {
      SSATmp* loc = m_tb->genIncRef(m_tb->genLdAssertedLoc(i, Type::Gen));
      m_tb->genStMem(locals, cellsToBytes(genLocals - params[i] - 1), loc,
                     true);
    }
    if (fillThis) {
      assert(thisId != kInvalidId);
      m_tb->gen(FillContThis, cont, locals,
                cns(cellsToBytes(genLocals - thisId - 1)));
    }
  } else {
    m_tb->gen(FillContLocals, m_tb->getFp(), cns(origFunc),
      cns(genFunc), cont);
  }

  push(cont);
}

void HhbcTranslator::emitContEnter(int32_t returnBcOffset) {
  spillStack();

  assert(getCurClass());
  SSATmp* cont = m_tb->genLdThis(nullptr);
  SSATmp* contAR = m_tb->genLdRaw(cont, RawMemSlot::ContARPtr, Type::FramePtr);

  SSATmp* func = m_tb->genLdARFuncPtr(contAR, m_tb->genDefConst<int64_t>(0));
  SSATmp* funcBody = m_tb->genLdRaw(func, RawMemSlot::ContEntry, Type::TCA);

  m_tb->gen(
    ContEnter,
    contAR,
    funcBody,
    cns(returnBcOffset),
    m_tb->getFp()
  );
  assert(m_stackDeficit == 0);
}

void HhbcTranslator::emitContExitImpl() {
  SSATmp* retAddr = m_tb->genLdRetAddr();
  // Despite the name, this doesn't actually free the AR; it updates the
  // hardware fp and returns the old one
  SSATmp* fp = m_tb->gen(FreeActRec, m_tb->getFp());

  SSATmp* sp;
  if (m_stackDeficit) {
    // Adjust the hardware sp before leaving.
    // XXX This returns a ptrToGen
    sp = m_tb->genLdStackAddr(m_stackDeficit);
  } else {
    sp = m_tb->getSp();
  }

  m_tb->genRetCtrl(sp, fp, retAddr);
  m_hasExit = true;
}

void HhbcTranslator::emitContExit() {
  m_tb->genExitWhenSurprised(getExitSlowTrace());
  emitContExitImpl();
}

void HhbcTranslator::emitUnpackCont() {
  m_tb->genLinkContVarEnv();
  SSATmp* cont = m_tb->genLdAssertedLoc(0, Type::Obj);
  push(m_tb->genLdRaw(cont, RawMemSlot::ContLabel, Type::Int));
}

void HhbcTranslator::emitPackCont(int64_t labelId) {
  m_tb->genUnlinkContVarEnv();
  SSATmp* cont = m_tb->genLdAssertedLoc(0, Type::Obj);
  m_tb->genSetPropCell(cont, CONTOFF(m_value), popC());
  m_tb->genStRaw(cont, RawMemSlot::ContLabel,
                 m_tb->genDefConst(labelId));
}

void HhbcTranslator::emitContReceive() {
  SSATmp* cont = m_tb->genLdAssertedLoc(0, Type::Obj);
  m_tb->genContRaiseCheck(cont, getExitSlowTrace());
  SSATmp* valOffset = m_tb->genDefConst<int64_t>(CONTOFF(m_received));
  push(m_tb->genLdProp(cont, valOffset, Type::Cell, nullptr));
  m_tb->genStProp(cont, valOffset, m_tb->genDefUninit(), true);
}

void HhbcTranslator::emitContRetC() {
  SSATmp* cont = m_tb->genLdAssertedLoc(0, Type::Obj);
  m_tb->genExitWhenSurprised(getExitSlowTrace());
  m_tb->genStRaw(cont, RawMemSlot::ContDone, m_tb->genDefConst(true));
  m_tb->genSetPropCell(cont, CONTOFF(m_value), popC());

  // transfer control
  emitContExitImpl();
}

void HhbcTranslator::emitContNext() {
  assert(getCurClass());
  SSATmp* cont = m_tb->genLdThis(nullptr);
  m_tb->genContPreNext(cont, getExitSlowTrace());
  m_tb->genSetPropCell(cont, CONTOFF(m_received), m_tb->genDefInitNull());
}

void HhbcTranslator::emitContSendImpl(bool raise) {
  assert(getCurClass());
  SSATmp* cont = m_tb->genLdThis(nullptr);
  m_tb->genContStartedCheck(cont, getExitSlowTrace());
  m_tb->genContPreNext(cont, getExitSlowTrace());

  SSATmp* value = m_tb->genLdAssertedLoc(0, Type::Cell);
  value = m_tb->genIncRef(value);
  m_tb->genSetPropCell(cont, CONTOFF(m_received), value);
  if (raise) {
    m_tb->genStRaw(cont, RawMemSlot::ContShouldThrow,
                   m_tb->genDefConst(true));
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
  SSATmp* cont = m_tb->genLdThis(nullptr);
  SSATmp* done = m_tb->genLdRaw(cont, RawMemSlot::ContDone, Type::Bool);
  push(m_tb->genNot(done));
}

void HhbcTranslator::emitContCurrent() {
  assert(getCurClass());
  SSATmp* cont = m_tb->genLdThis(nullptr);
  m_tb->genContStartedCheck(cont, getExitSlowTrace());
  SSATmp* offset = m_tb->genDefConst<int64_t>(CONTOFF(m_value));
  SSATmp* value = m_tb->genLdProp(cont, offset, Type::Cell, nullptr);
  value = m_tb->genIncRef(value);
  push(value);
}

void HhbcTranslator::emitContStopped() {
  assert(getCurClass());
  SSATmp* cont = m_tb->genLdThis(nullptr);
  m_tb->genStRaw(cont, RawMemSlot::ContRunning, m_tb->genDefConst(false));
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
      push(m_tb->genDefConst<int64_t>(input->getValStr()->size()));
    } else {
      push(m_tb->genLdRaw(input, RawMemSlot::StrLen, Type::Int));
      m_tb->genDecRef(input);
    }
  } else if (inType.isNull()) {
    popC();
    push(m_tb->genDefConst<int64_t>(0));
  } else if (inType == Type::Bool) {
    // strlen(true) == 1, strlen(false) == 0.
    push(m_tb->gen(ConvBoolToInt, popC()));
  } else {
    emitInterpOne(Type::Int | Type::InitNull, 1);
  }
}

void HhbcTranslator::emitIncStat(int32_t counter, int32_t value, bool force) {
  if (Stats::enabled() || force) {
    m_tb->genIncStat(counter, value, force);
  }
}

SSATmp* HhbcTranslator::getStrName(const StringData* knownName) {
  SSATmp* name = popC();
  assert(name->isA(Type::Str) || knownName);
  if (!name->isConst() || !name->isA(Type::Str)) {
    if (knownName) {
      // The SSATmp on the evaluation stack was not a string constant,
      // but the bytecode translator somehow knew the name statically.
      name = m_tb->genDefConst(knownName);
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
  SSATmp* addr = m_tb->gen(LdClsPropAddr,
                           block,
                           clsTmp,
                           prop,
                           m_tb->genDefConst(getCurClass()));
  m_tb->genDecRef(prop); // safe to do early because prop is a string
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
  SSATmp* addr = m_tb->gen(LdGblAddr, block, name);
  m_tb->genDecRef(name);
  return addr;
}

SSATmp* HhbcTranslator::emitLdGblAddrDef(const StringData* gblName) {
  return m_tb->gen(LdGblAddrDef, getStrName(gblName));
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
  Type trackedType = m_tb->getLocalType(id);
  // guards should ensure we have type info at this point
  assert(trackedType != Type::None);
  if (trackedType.subtypeOf(Type::Uninit)) {
    push(m_tb->genDefConst(false));
  } else {
    Trace* exitTrace = getExitTrace();
    SSATmp* ld = m_tb->genLdLocAsCell(id, exitTrace);
    push(m_tb->gen(IsNType, Type::Null, ld));
  }
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
  Type trackedType = m_tb->getLocalType(id);
  assert(trackedType != Type::None);
  if (trackedType == Type::Uninit) {
    push(m_tb->genDefConst(true));
  } else {
    Trace* exitTrace = getExitTrace();
    SSATmp* ld = m_tb->genLdLocAsCell(id, exitTrace);
    push(m_tb->genNot(m_tb->genConvToBool(ld)));
  }
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
  push(m_tb->gen(IsType, t, src));
  m_tb->genDecRef(src);
}

void HhbcTranslator::emitIsTypeL(Type t, int id) {
  Trace* exitTrace = getExitTrace();
  push(m_tb->gen(IsType, t, emitLdLocWarn(id, exitTrace)));
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
    m_tb->genExitWhenSurprised(getExitSlowTrace());
  }
  if (!breakTracelet) return;
  m_tb->genJmp(getExitTrace(offset));
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
  SSATmp* boolSrc = m_tb->genConvToBool(src);
  m_tb->genDecRef(src);
  return m_tb->genJmpCond(boolSrc, target, negate);
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
  push(m_tb->genCmp(opc, src2, src1));
  m_tb->genDecRef(src2);
  m_tb->genDecRef(src1);
}

void HhbcTranslator::emitClsCnsD(int32_t cnsNameStrId, int32_t clsNameStrId) {
  // This bytecode re-enters if there is no class with the given name
  // and can throw a fatal error.
  const StringData* cnsNameStr = lookupStringId(cnsNameStrId);
  const StringData* clsNameStr = lookupStringId(clsNameStrId);
  SSATmp* cnsNameTmp = m_tb->genDefConst(cnsNameStr);
  SSATmp* clsNameTmp = m_tb->genDefConst(clsNameStr);
  if (0) {
    // TODO: 2068502 pick one of these two implementations and remove the other.
    Trace* exitTrace = getExitSlowTrace();
    SSATmp* cns = m_tb->gen(LdClsCns, Type::Cell, cnsNameTmp, clsNameTmp);
    m_tb->gen(CheckInit, m_tb->getFirstBlock(exitTrace), cns);
    push(cns);
  } else {
    // if-then-else
    // todo: t2068502: refine the type? hhbc spec says null|bool|int|dbl|str
    //       and, str should always be static-str.
    exceptionBarrier(); // do on main trace so we update stack tracking once.
    Type cnsType = Type::Cell;
    SSATmp* c1 = m_tb->gen(LdClsCns, cnsType, cnsNameTmp, clsNameTmp);
    SSATmp* result = m_tb->cond(getCurFunc(),
      [&] (Block* taken) { // branch
        m_tb->gen(CheckInit, taken, c1);
      },
      [&] { // Next: LdClsCns hit in TC
        return c1;
      },
      [&] { // Taken: miss in TC, do lookup & init
        m_tb->hint(Block::Unlikely);
        return m_tb->gen(LookupClsCns, cnsType, cnsNameTmp, clsNameTmp);
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

  push(m_tb->gen(AKExists, arr, key));
  m_tb->genDecRef(arr);
  m_tb->genDecRef(key);
}

void HhbcTranslator::emitFPassR() {
  emitUnboxRAux();
}

void HhbcTranslator::emitFPassCOp() {
}

void HhbcTranslator::emitFPassV() {
  Block* exit = getExitTrace()->front();
  SSATmp* tmp = popV();
  pushIncRef(m_tb->gen(Unbox, exit, tmp));
  m_tb->genDecRef(tmp);
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
      ctx = m_tb->gen(LdCtx, m_tb->getFp(), cns(curFunc));
      ctx = m_tb->gen(GetCtxFwdCall, ctx, cns(callee));
    } else {
      ctx = getClsMethodCtx(callee, cls);
    }
    if (!TargetCache::isPersistentHandle(cls->m_cachedOffset)) {
      // The miss path is complicated and rare. Punt for now.
      m_tb->gen(LdClsCachedSafe, getExitSlowTrace(), cns(cls->name()));
    }
  } else {
    ctx = m_tb->genDefInitNull();
    if (!TargetCache::isPersistentHandle(callee->getCachedOffset())) {
      // The miss path is complicated and rare. Punt for now.
      func = m_tb->gen(LdFuncCachedSafe, getExitSlowTrace(),
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
  m_tb->genNativeImpl();
  SSATmp* sp = m_tb->genRetAdjustStack();
  SSATmp* retAddr = m_tb->genLdRetAddr();
  SSATmp* fp = m_tb->gen(FreeActRec, m_tb->getFp());
  m_tb->genRetCtrl(sp, fp, retAddr);

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
    returnSp = m_tb->gen(StashGeneratorSP, m_tb->getSp());
  }

  m_fpiStack.emplace(returnSp, m_tb->getSpOffset());

  ActRecInfo info;
  info.numArgs = numArgs;
  info.invName = invName;
  m_tb->gen(
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
  m_tb->genNewObj(numParams, cls);
  m_fpiStack.emplace(nullptr, 0);
}

void HhbcTranslator::emitFPushCtorD(int32_t numParams, int32_t classNameStrId) {
  const StringData* className = lookupStringId(classNameStrId);
  exceptionBarrier();
  m_fpiStack.emplace(nullptr, 0);

  // If constructor is the generated 86ctor, no need to call it.
  if (RuntimeOption::RepoAuthoritative &&
      numParams == 0) {
    const Class* cls = Unit::lookupUniqueClass(className);
    if (cls &&
        (cls->attrs() & AttrUnique) &&
        Func::isSpecial(cls->getCtor()->name())) {
      // This optimization is only safe if the FCall is in the same
      // tracelet.  Luckily that is always the case: since this
      // optimization only applies when numParams==0, there will be
      // nothing between the FPushCtorD and the FCall.
      m_tb->genNewObjNoCtorCached(className);
      push(m_tb->genDefNull());
      return;
    }
  }
  m_tb->genNewObjCached(numParams, className);
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

  auto const closure = m_tb->gen(
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
    emitFPushFunc(numParams, m_tb->genDefConst(name));
    return;
  }
  func->validate();

  const bool immutable = func->isNameBindingImmutable(getCurUnit());

  if (!immutable) {
    exceptionBarrier();  // LdFuncCached can reenter
  }
  SSATmp* ssaFunc = immutable ? m_tb->genDefConst(func)
                              : m_tb->gen(LdFuncCached, m_tb->genDefConst(name));
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
  emitFPushActRec(m_tb->gen(LdFunc, funcName),
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
          SSATmp* clsTmp = m_tb->gen(LdObjClass, obj);
          SSATmp* funcTmp = m_tb->genLdClsMethod(clsTmp, func->methodSlot());
          if (res == MethodLookup::MethodFoundNoThis) {
            m_tb->genDecRef(obj);
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
      m_tb->genDecRef(obj);
      objOrCls = m_tb->genDefConst(baseClass);
    }
    emitFPushActRec(m_tb->genDefConst(func),
                    objOrCls,
                    numParams,
                    magicCall ? methodName : nullptr);
  } else {
    emitFPushActRec(m_tb->genDefNull(),
                    obj,
                    numParams,
                    nullptr);
    auto const actRec = spillStack();
    auto const objCls = m_tb->gen(LdObjClass, obj);
    m_tb->gen(LdObjMethod,
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
    return m_tb->gen(LdCls, cns(cls->name()), cns(getCurClass()));
  } else if (m_tb->isThisAvailable()) {
    // might not be a static call and $this is available, so we know it's
    // definitely not static
    assert(getCurClass());
    return m_tb->genIncRef(m_tb->genLdThis(nullptr));
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
    emitFPushActRec(m_tb->genDefConst(func),
                    objOrCls,
                    numParams,
                    func && magicCall ? methodName : nullptr);
  } else {
    // lookup static method & class in the target cache
    Trace* exitTrace = getExitSlowTrace();
    SSATmp* funcClassTmp =
      m_tb->genLdClsMethodCache(m_tb->genDefConst(className),
                                m_tb->genDefConst(methodName),
                                m_tb->genDefConst(np.second),
                                exitTrace);
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
  SSATmp* curCtxTmp = m_tb->genLdCtx(getCurFunc());
  if (func) {
    SSATmp*   funcTmp = m_tb->genDefConst(func);
    SSATmp* newCtxTmp = m_tb->gen(GetCtxFwdCall, curCtxTmp, funcTmp);

    emitFPushActRec(funcTmp, newCtxTmp, numParams,
                    (magicCall ? methName : nullptr));

  } else {
    SSATmp* funcCtxTmp = m_tb->gen(LdClsMethodFCache, exitBlock,
                                   m_tb->genDefConst(cls),
                                   m_tb->genDefConst(methName),
                                   curCtxTmp);
    emitFPushActRec(funcCtxTmp,
                    m_tb->genDefInitNull(),
                    numParams,
                    (magicCall ? methName : nullptr));
  }
}

void HhbcTranslator::emitFCallArray() {
  PUNT(FCallArray); // can't interpret one because of control flow
}

void HhbcTranslator::emitFCall(uint32_t numParams,
                               Offset returnBcOffset,
                               const Func* callee) {
  // pop the incoming parameters to the call
  SSATmp* params[numParams];
  for (uint32_t i = 0; i < numParams; i++) {
    params[numParams - i - 1] = popF();
  }
  auto actRec = spillStack();
  auto func = callee ? cns(callee) : m_tb->genDefNull();
  m_tb->genCall(actRec,
                returnBcOffset,
                func,
                numParams,
                params);

  if (!m_fpiStack.empty()) {
    m_fpiStack.pop();
  }
}

void HhbcTranslator::emitFCallBuiltin(uint32_t numArgs,
                                      uint32_t numNonDefault, int32_t funcId) {
  const NamedEntityPair& nep = lookupNamedEntityPairId(funcId);
  const StringData* name = nep.first;
  const Func* callee = Unit::lookupFunc(nep.second, name);

  callee->validate();

  // spill args to stack. We need to spill these for two resons:
  // 1. some of the arguments may be passed by reference, for which
  //    case we will generate LdStackAddr() (see below).
  // 2. type conversions of the arguments (using tvCast* helpers)
  //    may throw an exception, so we need to have the VM stack
  //    in a clean state at that point.
  exceptionBarrier();
  // Convert types if needed
  for (int i = 0; i < numNonDefault; i++) {
    const Func::ParamInfo& pi = callee->params()[i];
    switch (pi.builtinType()) {
      case KindOfBoolean:
      case KindOfInt64:
      case KindOfArray:
      case KindOfObject:
      case KindOfString:
        m_tb->genCastStk(numArgs - i - 1,
                         Type::fromDataType(pi.builtinType(), KindOfInvalid));
        break;
      case KindOfDouble: not_reached();
      case KindOfUnknown: break;
      default:            not_reached();
    }
  }

  // pass arguments for call
  SSATmp* args[numArgs + 1];

  for (int i = numArgs - 1; i >= 0; i--) {
    const Func::ParamInfo& pi = callee->params()[i];
    switch (pi.builtinType()) {
      case KindOfBoolean:
      case KindOfInt64:
        args[i] = top(Type::fromDataType(pi.builtinType(), KindOfInvalid),
                      numArgs - i - 1);
        break;
      case KindOfDouble: assert(false);
      default:
        args[i] = loadStackAddr(numArgs - i - 1);
        break;
    }
  }
  // generate call and set return type
  SSATmp* func = m_tb->genDefConst<const Func*>(callee);
  Type type = Type::fromDataTypeWithRef(callee->returnType(),
                       (callee->attrs() & ClassInfo::IsReference));
  SSATmp* ret = m_tb->genCallBuiltin(func, type, numArgs, args);

  // decref and free args
  for (int i = 0; i < numArgs; i++) {
    SSATmp* arg = popR();
    if (i >= numArgs - numNonDefault) {
      m_tb->genDecRef(arg);
    }
  }

  // push return value
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
  FTRACE(1, "]]] end inlining: {}\n", getCurFunc()->fullName()->data());
  m_bcStateStack.pop_back();
  m_fpiStack.pop();
  push(retVal);

  emitMarker();
}

/*
 * In case retVal comes from a local, the logic below tweaks the code
 * so that retVal is DecRef'd and the corresponding local's SSATmp is
 * returned. This enables the ref-count optimization to eliminate the
 * IncRef/DecRef pair in the main trace.
 */
SSATmp* HhbcTranslator::emitDecRefLocalsInline(SSATmp* retVal) {
  SSATmp* retValSrcLoc = nullptr;
  Opcode  retValSrcOpc = Nop; // Nop flags the ref-count opt is impossible
  IRInstruction* retValSrcInstr = retVal->inst();
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
      m_tb->genDecRef(retVal);
    } else {
      m_tb->genDecRefThis();
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
      m_tb->genDecRef(retVal);
    } else {
      m_tb->genDecRefLoc(id);
    }
  }

  return retValSrcLoc ? retValSrcLoc : retVal;
}

void HhbcTranslator::emitRet(Type type, bool freeInline) {
  if (isInlining()) {
    return emitRetFromInlined(type);
  }

  const Func* curFunc = getCurFunc();
  bool mayUseVV = (curFunc->attrs() & AttrMayUseVV);

  m_tb->genExitWhenSurprised(getExitSlowTrace());
  if (mayUseVV) {
    // Note: this has to be the first thing, because we cannot bail after
    //       we start decRefing locs because then there'll be no corresponding
    //       bytecode boundaries until the end of RetC
    m_tb->genReleaseVVOrExit(getExitSlowTrace());
  }
  SSATmp* retVal = pop(type);

  SSATmp* sp;
  if (freeInline) {
    SSATmp* useRet = emitDecRefLocalsInline(retVal);
    m_tb->genRetVal(useRet);
    sp = m_tb->genRetAdjustStack();
  } else {
    if (mayHaveThis(curFunc)) {
      m_tb->genDecRefThis();
    }
    sp = m_tb->genGenericRetDecRefs(retVal, curFunc->numLocals());
    m_tb->genRetVal(retVal);
  }

  // Free ActRec, and return control to caller.
  SSATmp* retAddr = m_tb->genLdRetAddr();
  SSATmp* fp = m_tb->gen(FreeActRec, m_tb->getFp());
  m_tb->genRetCtrl(sp, fp, retAddr);

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
  SSATmp* ssabase = m_tb->genDefConst(base);
  SSATmp* ssatargets = m_tb->genDefConst(nTargets);

  Offset defaultOff = bcOff() + iv.vec32()[iv.size() - 1];
  Offset zeroOff = 0;
  if (base <= 0 && (base + nTargets) > 0) {
    zeroOff = bcOff() + iv.vec32()[0 - base];
  } else {
    zeroOff = defaultOff;
  }

  if (type.subtypeOf(Type::Null)) {
    m_tb->genJmp(getExitTrace(zeroOff));
    return;
  } else if (type.subtypeOf(Type::Bool)) {
    Offset nonZeroOff = bcOff() + iv.vec32()[iv.size() - 2];
    m_tb->genJmpCond(switchVal, getExitTrace(nonZeroOff), false);
    m_tb->genJmp(getExitTrace(zeroOff));
    return;
  } else if (type.subtypeOf(Type::Int)) {
    // No special treatment needed
    index = switchVal;
  } else if (type.subtypeOf(Type::Dbl)) {
    // switch(Double|String|Obj)Helper do bounds-checking for us, so
    // we need to make sure the default case is in the jump table,
    // and don't emit our own bounds-checking code
    bounded = false;
    index = m_tb->gen(LdSwitchDblIndex,
                      switchVal, ssabase, ssatargets);
  } else if (type.subtypeOf(Type::Str)) {
    bounded = false;
    index = m_tb->gen(LdSwitchStrIndex,
                      switchVal, ssabase, ssatargets);
  } else if (type.subtypeOf(Type::Obj)) {
    // switchObjHelper can throw exceptions and reenter the VM
    if (type.subtypeOf(Type::Obj)) {
      exceptionBarrier();
    }
    bounded = false;
    index = m_tb->gen(LdSwitchObjIndex,
                      switchVal, ssabase, ssatargets);
  } else if (type.subtypeOf(Type::Arr)) {
    m_tb->genDecRef(switchVal);
    m_tb->genJmp(getExitTrace(defaultOff));
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
  m_tb->gen(SyncVMRegs, m_tb->getFp(), stack);

  m_tb->gen(JmpSwitchDest, data, index);
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

  SSATmp* dest = m_tb->gen(fastPath ? LdSSwitchDestFast
                                    : LdSSwitchDestSlow,
                           data,
                           testVal);
  m_tb->genDecRef(testVal);
  auto const stack = spillStack();
  m_tb->gen(SyncVMRegs, m_tb->getFp(), stack);
  m_tb->gen(JmpIndirect, dest);
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
  m_tb->genGuardLoc(locId, type, getExitTrace());
}

void HhbcTranslator::assertTypeLocal(uint32_t localIndex, Type type) {
  m_tb->genAssertLoc(localIndex, type, false);
}

void HhbcTranslator::overrideTypeLocal(uint32_t localIndex, Type type) {
  // if changing the inner type of a boxed local, also drop the
  // information about inner types for any other boxed local
  if (type.isBoxed()) {
    m_tb->dropLocalRefsInnerTypes();
  }
  m_tb->genAssertLoc(localIndex, type, true);
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
  m_tb->genGuardStk(stackIndex, type, nextTrace);
  m_typeGuards.push_back(TypeGuard(TypeGuard::Stack, stackIndex, type));

  return nextTrace;
}

void HhbcTranslator::checkTypeTopOfStack(Type type,
                                         Offset nextByteCode) {
  Trace* exitTrace = getExitTrace(nextByteCode);
  SSATmp* tmp = m_evalStack.top();
  if (!tmp) {
    FTRACE(1, "checkTypeTopOfStack: no tmp: {}\n", type.toString());
    m_tb->genGuardStk(0, type, exitTrace);
    push(pop(type));
  } else {
    FTRACE(1, "checkTypeTopOfStack: generating GuardType for {}\n",
           type.toString());
    m_evalStack.pop();
    tmp = m_tb->genGuardType(tmp, type, exitTrace);
    push(tmp);
  }
}

void HhbcTranslator::assertTypeStack(uint32_t stackIndex, Type type) {
  SSATmp* tmp = m_evalStack.top(stackIndex);
  if (!tmp) {
    m_tb->genAssertStk(stackIndex, type);
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
      m_tb->genLdLoc(guard.getIndex());
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
  SSATmp* funcPtr = m_tb->genLdARFuncPtr(m_tb->getSp(),
                                         m_tb->genDefConst<int64_t>(actRecOff));
  SSATmp* nParams = m_tb->genLdRaw(funcPtr, RawMemSlot::FuncNumParams,
                                  Type::Int);
  SSATmp* bitsPtr = m_tb->genLdRaw(funcPtr, RawMemSlot::FuncRefBitVec,
                                  Type::Int);

  for (unsigned i = 0; i < mask.size(); i += 64) {
    assert(i < vals.size());

    uint64_t mask64 = TranslatorX64::packBitVec(mask, i);
    if (mask64 == 0) {
      continue;
    }

    uint64_t vals64 = TranslatorX64::packBitVec(vals, i);

    SSATmp* mask64Tmp   = m_tb->genLdConst<int64_t>(mask64);
    SSATmp* vals64Tmp   = m_tb->genLdConst<int64_t>(vals64);
    SSATmp* firstBitNum = m_tb->genDefConst<int64_t>(i);

    m_tb->genGuardRefs(funcPtr, nParams, bitsPtr, firstBitNum,
                      mask64Tmp, vals64Tmp, exitTrace);
  }

  return exitTrace;
}

void HhbcTranslator::emitVerifyParamType(int32_t paramId) {
  const Func* func = getCurFunc();
  const TypeConstraint& tc = func->params()[paramId].typeConstraint();
  SSATmp* locVal = m_tb->genLdLoc(paramId);
  Type locType = locVal->type().unbox();
  assert(locType.isKnownDataType());

  if (tc.nullable() && locType.isNull()) {
    return;
  }
  if (tc.isCallable()) {
    exceptionBarrier();
    locVal = m_tb->gen(Unbox, getExitTrace(), locVal);
    m_tb->gen(VerifyParamCallable, locVal, cns(paramId));
    return;
  }

  // For non-object guards, we rely on what we know from the tracelet
  // guards and never have to do runtime checks.
  if (!tc.isObjectOrTypedef()) {
    if (!tc.checkPrimitive(locType.toDataType())) {
      exceptionBarrier();
      m_tb->gen(VerifyParamFail, cns(paramId));
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
      m_tb->gen(VerifyParamFail, cns(paramId));
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
                                       : m_tb->gen(LdClsCachedSafe, cns(clsName));
  locVal = m_tb->gen(Unbox, getExitTrace(), locVal);
  SSATmp* objClass = m_tb->gen(LdObjClass, locVal);
  if (haveBit || classIsUniqueNormalClass(knownConstraint)) {
    SSATmp* isInstance = haveBit
      ? m_tb->gen(InstanceOfBitmask, objClass, cns(clsName))
      : m_tb->gen(ExtendsClass, objClass, constraint);
    exceptionBarrier();
    m_tb->ifThen(getCurFunc(),
      [&](Block* taken) {
        m_tb->gen(JmpZero, taken, isInstance);
      },
      [&] { // taken: the param type does not match
        m_tb->hint(Block::Unlikely);
        m_tb->gen(VerifyParamFail, cns(paramId));
      }
    );
  } else {
    exceptionBarrier();
    m_tb->gen(VerifyParamCls,
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
    push(m_tb->genDefConst(false));
    m_tb->genDecRef(src);
    return;
  }

  SSATmp* objClass     = m_tb->gen(LdObjClass, src);
  SSATmp* ssaClassName = m_tb->genDefConst(className);

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
      ? m_tb->genDefConst(maybeCls)
      : m_tb->gen(LdClsCachedSafe, ssaClassName);

  push(
    haveBit ? m_tb->gen(InstanceOfBitmask,
                        objClass,
                        ssaClassName) :
    isUnique && isNormalClass ? m_tb->gen(ExtendsClass,
                                          objClass,
                                          checkClass) :
    m_tb->gen(InstanceOf,
              objClass,
              checkClass,
              m_tb->genDefConst(maybeCls && !isNormalClass))
  );
  m_tb->genDecRef(src);
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
    push(m_tb->genDefConst(HphpArray::GetStaticEmptyArray()));
  } else if (fromType.isBool()) {
    push(m_tb->gen(ConvBoolToArr, src));
  } else if (fromType.isDbl()) {
    push(m_tb->gen(ConvDblToArr, src));
  } else if (fromType.isInt()) {
    push(m_tb->gen(ConvIntToArr, src));
  } else if (fromType.isString()) {
    push(m_tb->gen(ConvStrToArr, src));
  } else if (fromType.isObj()) {
    push(m_tb->gen(ConvObjToArr, src));
  } else {
    push(m_tb->gen(ConvCellToArr, src));
  }
}

void HhbcTranslator::emitCastBool() {
  SSATmp* src = popC();
  push(m_tb->genConvToBool(src));
  m_tb->genDecRef(src);
}

void HhbcTranslator::emitCastDouble() {
  SSATmp* src = popC();
  Type fromType = src->type();
  if (fromType.isDbl()) {
    push(src);
  } else if (fromType.isNull()) {
    push(m_tb->genDefConst(0.0));
  } else if (fromType.isArray()) {
    push(m_tb->gen(ConvArrToDbl, src));
    m_tb->genDecRef(src);
  } else if (fromType.isBool()) {
    push(m_tb->gen(ConvBoolToDbl, src));
  } else if (fromType.isInt()) {
    push(m_tb->gen(ConvIntToDbl, src));
  } else if (fromType.isString()) {
    push(m_tb->gen(ConvStrToDbl, src));
  } else if (fromType.isObj()) {
    exceptionBarrier();
    push(m_tb->gen(ConvObjToDbl, src));
  } else {
    exceptionBarrier(); // may throw
    push(m_tb->gen(ConvCellToDbl, src));
  }
}

void HhbcTranslator::emitCastInt() {
  SSATmp* src = popC();
  Type fromType = src->type();
  if (fromType.isInt()) {
    push(src);
  } else if (fromType.isNull()) {
    push(m_tb->genDefConst(0));
  } else if (fromType.isArray()) {
    push(m_tb->gen(ConvArrToInt, src));
    m_tb->genDecRef(src);
  } else if (fromType.isBool()) {
    push(m_tb->gen(ConvBoolToInt, src));
  } else if (fromType.isDbl()) {
    push(m_tb->gen(ConvDblToInt, src));
  } else if (fromType.isString()) {
    push(m_tb->gen(ConvStrToInt, src));
    m_tb->genDecRef(src);
  } else if (fromType.isObj()) {
    exceptionBarrier();
    push(m_tb->gen(ConvObjToInt, src));
  } else {
    exceptionBarrier();
    push(m_tb->gen(ConvCellToInt, src));
  }
}

void HhbcTranslator::emitCastObject() {
  SSATmp* src = popC();
  Type srcType = src->type();
  if (srcType.isObj()) {
    push(src);
  } else {
    push(m_tb->gen(ConvCellToObj, src));
  }
}

void HhbcTranslator::emitCastString() {
  SSATmp* src = popC();
  Type fromType = src->type();
  if (fromType.isString()) {
    push(src);
  } else if (fromType.isNull()) {
    push(m_tb->genDefConst(StringData::GetStaticString("")));
  } else if (fromType.isArray()) {
    push(m_tb->genDefConst(StringData::GetStaticString("Array")));
    m_tb->genDecRef(src);
  } else if (fromType.isBool()) {
    push(m_tb->gen(ConvBoolToStr, src));
  } else if (fromType.isDbl()) {
    push(m_tb->gen(ConvDblToStr, src));
  } else if (fromType.isInt()) {
    push(m_tb->gen(ConvIntToStr, src));
  } else if (fromType.isObj()) {
    exceptionBarrier();
    push(m_tb->gen(ConvObjToStr, src));
  } else {
    exceptionBarrier();
    push(m_tb->gen(ConvCellToStr, src));
  }
}

static
bool isSupportedAGet(SSATmp* classSrc, const StringData* clsName) {
  return (classSrc->isA(Type::Obj) || classSrc->isA(Type::Str) || clsName);
}

void HhbcTranslator::emitAGet(SSATmp* classSrc, const StringData* clsName) {
  if (classSrc->isA(Type::Str)) {
    push(m_tb->gen(LdCls, classSrc, m_tb->genDefConst(getCurClass())));
  } else if (classSrc->isA(Type::Obj)) {
    push(m_tb->gen(LdObjClass, classSrc));
  } else if (clsName) {
    push(m_tb->gen(LdCls,
                   m_tb->genDefConst(clsName),
                   m_tb->genDefConst(getCurClass())));
  } else {
    not_reached();
  }
}

void HhbcTranslator::emitAGetC(const StringData* clsName) {
  if (isSupportedAGet(topC(), clsName)) {
    SSATmp* src = popC();
    emitAGet(src, clsName);
    m_tb->genDecRef(src);
  } else {
    emitInterpOne(Type::Cls, 1);
  }
}

void HhbcTranslator::emitAGetL(int id, const StringData* clsName) {
  SSATmp* src = m_tb->genLdLocAsCell(id, getExitTrace());
  if (isSupportedAGet(src, clsName)) {
    emitAGet(src, clsName);
  } else {
    PUNT(AGetL); // need to teach interpone about local uses
  }
}

SSATmp* HhbcTranslator::unboxPtr(SSATmp* ptr) {
  return m_tb->genUnboxPtr(ptr);
}

void HhbcTranslator::emitBindMem(SSATmp* ptr, SSATmp* src) {
  SSATmp* prevValue = m_tb->genLdMem(ptr, ptr->type().deref(), NULL);
  pushIncRef(src);
  m_tb->genStMem(ptr, src, true);
  if (isRefCounted(src) && src->type().canRunDtor()) {
    Block* exitBlock = getExitTrace(getNextSrcKey().offset())->front();
    Block::iterator markerInst = exitBlock->skipLabel();
    exitBlock->insert(++markerInst, m_irFactory.gen(DecRef, prevValue));
    m_tb->gen(DecRefNZOrBranch, exitBlock, prevValue);
  } else {
    m_tb->genDecRef(prevValue);
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
  emitBindMem(unboxPtr(ptr), src);
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
  pushIncRef(m_tb->genLdMem(m_tb->gen(BoxPtr, ptr), Type::BoxedCell, NULL));
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
                          return m_tb->gen(IsNTypeMem, Type::Null,
                                          m_tb->genUnboxPtr(ptr));
                        },
                        [&] { // Taken
                          return m_tb->genDefConst(false);
                        }
  );
  push(result);
}

void HhbcTranslator::emitEmptyMem(SSATmp* ptr) {
  SSATmp* ld = m_tb->genLdMem(unboxPtr(ptr), Type::Cell, nullptr);
  push(m_tb->genNot(m_tb->genConvToBool(ld)));
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
                          SSATmp* ld = m_tb->genLdMem(unboxPtr(ptr), Type::Cell,
                                                      nullptr);
                          return m_tb->genNot(m_tb->genConvToBool(ld));
                        },
                        [&] { // Taken
                          return m_tb->genDefConst(true);
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
                                      ? m_tb->getFirstBlock(getExitSlowTrace())
                                      : nullptr);
  if (!isInferedType) ptr = unboxPtr(ptr);
  pushIncRef(m_tb->genLdMem(ptr, resultType, exit));
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
    push(m_tb->gen(opc, Type::binArithResultType(type1, type2), tl, tr));
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
  push(m_tb->genNot(m_tb->genConvToBool(src)));
  m_tb->genDecRef(src);
}

#define BINOP(Opp) \
void HhbcTranslator::emit ## Opp() {     \
    emitBinaryArith(Op ## Opp);          \
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
  exitSpillValues.push_back(m_tb->genDefConst(false));
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
  m_tb->gen(JmpZero, exit, r);
  push(m_tb->gen(OpMod, Type::Int, l, r));
}

void HhbcTranslator::emitBitNot() {
  Type srcType = topC()->type();
  if (srcType == Type::Int) {
    SSATmp* src = popC();
    SSATmp* ones = m_tb->genDefConst<int64_t>(~0);
    push(m_tb->genXor(src, ones));
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
  SSATmp* tr = m_tb->genConvToBool(btr);
  SSATmp* tl = m_tb->genConvToBool(btl);
  push(m_tb->genConvToBool(m_tb->genXor(tl, tr)));
  m_tb->genDecRef(btl);
  m_tb->genDecRef(btr);
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
  m_tb->genInterpOne(bcOff(), numPopped - numPushed, type);
  m_stackDeficit = 0;
}

void HhbcTranslator::emitInterpOneCF(int numPopped) {
  exceptionBarrier();
  // discard the top elements of the stack, which are consumed by this instr
  discard(numPopped);
  assert(numPopped == m_stackDeficit);
  m_tb->gen(InterpOneCF, m_tb->getFp(), m_tb->getSp(),
            m_tb->genDefConst(bcOff()));
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
  std::vector<SSATmp*> stackValues = getSpillValues();
  m_evalStack.clear();
  SSATmp* sp = m_tb->genSpillStack(m_stackDeficit,
                                   stackValues.size(),
                                   stackValues.size() ? &stackValues[0] : 0);
  m_stackDeficit = 0;
  return sp;
}

void HhbcTranslator::exceptionBarrier() {
  auto const sp = spillStack();
  m_tb->gen(ExceptionBarrier, sp);
}

SSATmp* HhbcTranslator::loadStackAddr(int32_t offset) {
  // You're almost certainly doing it wrong if you want to get the address of a
  // stack cell that's in m_evalStack.
  assert(offset >= (int32_t)m_evalStack.numCells());
  return m_tb->genLdStackAddr(
    offset + m_stackDeficit - m_evalStack.numCells());
}

//
// This is a wrapper to TraceBuilder::genLdLoc() that also emits the
// RaiseUninitLoc if the local is uninitialized
//
SSATmp* HhbcTranslator::emitLdLocWarn(uint32_t id,
                                      Trace* target) {
  SSATmp* locVal = m_tb->genLdLocAsCell(id, target);

  if (locVal->type().subtypeOf(Type::Uninit)) {
    exceptionBarrier();
    m_tb->genRaiseUninitLoc(id);
    return m_tb->genDefInitNull();
  }

  return locVal;
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
