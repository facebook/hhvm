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
#include "runtime/ext/ext_continuation.h"
#include "runtime/vm/translator/hopt/hhbctranslator.h"
#include "runtime/vm/translator/translator-x64.h"
#include "runtime/vm/stats.h"
#include "runtime/vm/unit.h"
#include "runtime/vm/runtime.h"
#include "runtime/vm/translator/hopt/irfactory.h"

using namespace HPHP::VM::Transl;

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

void HhbcTranslator::refineType(SSATmp* tmp, Type::Tag type) {
  // If type is more refined than tmp's type, reset tmp's type to type
  IRInstruction* inst = tmp->getInstruction();
  if (Type::isMoreRefined(type, tmp->getType())) {
    // If tmp is incref or move, then chase down its src
    Opcode opc = inst->getOpcode();
    if (opc == Mov || opc == IncRef) {
      refineType(inst->getSrc(0), type);
      tmp->setType(outputType(inst));
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

SSATmp* HhbcTranslator::pop(Type::Tag type) {
  SSATmp* opnd = m_evalStack.pop();
  if (opnd == NULL) {
    uint32 stackOff = m_stackDeficit;
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
    pop(Type::Gen);
  }
}

// type is the type expected on the stack.
void HhbcTranslator::popDecRef(Type::Tag type) {
  if (SSATmp* src = m_evalStack.pop()) {
    m_tb->genDecRef(src);
    return;
  }

  uint32 stackOff = m_stackDeficit;
  m_tb->genDecRefStack(type, stackOff);
  m_stackDeficit++;
}

// We don't know what type description to expect for the stack
// locations before index, so we use a generic type when popping the
// intermediate values.  If it ends up creating a new LdStack,
// refineType during a later pop() or top() will fix up the type to
// the known type.
void HhbcTranslator::extendStack(uint32 index, Type::Tag type) {
  if (index == 0) {
    push(pop(type));
    return;
  }

  SSATmp* tmp = pop(Type::Gen);
  extendStack(index - 1, type);
  push(tmp);
}

SSATmp* HhbcTranslator::top(Type::Tag type, uint32 index) {
  SSATmp* tmp = m_evalStack.top(index);
  if (!tmp) {
    extendStack(index, type);
    tmp = m_evalStack.top(index);
  }
  assert(tmp);
  refineType(tmp, type);
  return tmp;
}

void HhbcTranslator::replace(uint32 index, SSATmp* tmp) {
  m_evalStack.replace(index, tmp);
}

void HhbcTranslator::setBcOff(Offset newOff, bool lastBcOff) {
  if (newOff != m_bcOff || m_bcOff == m_startBcOff) {
    m_bcOff = newOff;
    m_tb->genMarker(m_bcOff,
      m_tb->getSpOffset() + m_evalStack.numCells() - m_stackDeficit);
  }
  m_lastBcOff = lastBcOff;
}

void HhbcTranslator::emitPrint() {
  TRACE(3, "%u: Print\n", m_bcOff);
  Type::Tag type = topC()->getType();
  if (type == Type::Int || type == Type::Bool || type == Type::Null ||
      Type::isString(type)) {
    // the print helpers decref their arg, so don't decref pop'ed value
    m_tb->genPrint(popC());
    push(m_tb->genDefConst<int64>(1));
  } else {
    spillStack();
    popC();
    emitInterpOne(Type::Int);
  }
}

void HhbcTranslator::emitUnboxRAux() {
  if (Type::isUnboxed(top(Type::Gen)->getType())) {
    return; // top of stack already unboxed, nothing to do
  }
  Trace* exitTrace = getExitTrace();
  SSATmp* tmp = popR();
  pushIncRef(m_tb->genUnbox(tmp, exitTrace));
  m_tb->genDecRef(tmp);
}

void HhbcTranslator::emitUnboxR() {
  TRACE(3, "%u: UnboxR\n", m_bcOff);
  emitUnboxRAux();
}

void HhbcTranslator::emitThis() {
  TRACE(3, "%u: This\n", m_bcOff);
  pushIncRef(m_tb->genLdThis(getExitSlowTrace()));
}

void HhbcTranslator::emitCheckThis() {
  TRACE(3, "%u: CheckThis\n", m_bcOff);
  m_tb->genLdThis(getExitSlowTrace());
}

void HhbcTranslator::emitBareThis(int notice) {
  TRACE(3, "%u: BareThis %d\n", m_bcOff, notice);
  // We just exit the trace in the case $this is null. Before exiting
  // the trace, we could also push null onto the stack and raise a
  // notice if the notice argument is set. By exiting the trace when
  // $this is null, we can be sure in the rest of the trace that we
  // have the this object on top of the stack, and we can eliminate
  // further null checks of this.
  pushIncRef(m_tb->genLdThis(getExitSlowTrace()));
}

void HhbcTranslator::emitArray(int arrayId) {
  TRACE(3, "%u: Array %d\n", m_bcOff, arrayId);
  ArrayData* ad = lookupArrayId(arrayId);
  push(m_tb->genDefConst<const ArrayData*>(ad));
}

void HhbcTranslator::emitNewArray(int capacity) {
  TRACE(3, "%u: NewArray %d\n", m_bcOff, capacity);
  if (capacity == 0) {
    ArrayData* ad = HphpArray::GetStaticEmptyArray();
    push(m_tb->genDefConst<const ArrayData*>(ad));
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
  TRACE(3, "%u: NewTuple %d\n", m_bcOff, numArgs);
  SSATmp* sp = spillStack();
  for (int i = 0; i < numArgs; i++) popC();
  push(m_tb->genNewTuple(numArgs, sp));
}

void HhbcTranslator::emitArrayAdd() {
  TRACE(3, "%u: ArrayAdd\n", m_bcOff);
  Type::Tag type1 = topC(0)->getType();
  Type::Tag type2 = topC(1)->getType();
  if (type1 != Type::Arr || type2 != Type::Arr) {
    // This happens when we have a prior spillstack that optimizes away
    // its spilled values because they were already on the stack. This
    // prevents us from getting to type of the SSATmps popped from the
    // eval stack. Most likely we had an interpone before this
    // instruction.
    spillStack();
    popC();
    popC();
    emitInterpOne(Type::Arr);
    return;
  }
  SSATmp* tr = popC();
  SSATmp* tl = popC();
  // the ArrrayAdd helper decrefs its args, so don't decref pop'ed values
  // TODO task 1805916: verify that ArrayAdd increfs its result
  push(m_tb->genArrayAdd(tl, tr));
}

void HhbcTranslator::emitAddElemC() {
  TRACE(3, "%u: AddElemC\n", m_bcOff);
  SSATmp* val = popC();
  SSATmp* key = popC();
  SSATmp* arr = popC();
  // the AddElem helper decrefs its args, so don't decref pop'ed values
  // TODO task 1805916: verify that AddElem increfs its result
  push(m_tb->genAddElem(arr, key, val));
}

void HhbcTranslator::emitAddNewElemC() {
  TRACE(3, "%u: AddNewElemC\n", m_bcOff);
  SSATmp* val = popC();
  SSATmp* arr = popC();
  // the AddNewElem helper decrefs its args, so don't decref pop'ed values
  // TODO task 1805916: verify that NewElem increfs its result
  push(m_tb->genAddNewElem(arr, val));
}

void HhbcTranslator::emitCns(uint32 id) {
  spillStack();
  emitInterpOneOrPunt(Type::Cell);
}

void HhbcTranslator::emitDefCns(uint32 id) {
  TRACE(3, "%u: DefCns %d\n", m_bcOff, id);
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
//  m_tb->genDefCls(lookupPreClassId(cid), getCurUnit()->at(after));
  spillStack();
  emitInterpOneOrPunt(Type::None);
}

void HhbcTranslator::emitDefFunc(int fid) {
//  m_tb->genDefFunc(lookupFuncId(fid));
  spillStack();
  emitInterpOneOrPunt(Type::None);
}

void HhbcTranslator::emitSelf() {
  Class* clss = getCurClass();
  if (RuntimeOption::RepoAuthoritative &&
     (clss->preClass()->attrs() & AttrUnique)) {
    push(m_tb->genDefConst<const Class*>(clss));
  } else {
    push(m_tb->genLdFuncCls(m_tb->genLdCurFuncPtr()));
  }

}

void HhbcTranslator::emitParent() {
  Class* clss = getCurClass()->parent();
  // if in repo authorative mode & the class is unique, then we can
  // just burn the class ref into an immediate
  if (RuntimeOption::RepoAuthoritative &&
     (clss->preClass()->attrs() & AttrUnique)) {
    push(m_tb->genDefConst<const Class*>(clss));
  } else {
    SSATmp* className = m_tb->genDefConst<const StringData*>(clss->name());
    push(m_tb->genLdCls(className));
  }
}

void HhbcTranslator::emitString(int strId) {
  TRACE(3, "%u: String %d\n", m_bcOff, strId);
  push(m_tb->genDefConst<const StringData*>(lookupStringId(strId)));
}

void HhbcTranslator::emitInt(int64 val) {
  TRACE(3, "%u: Int %lld\n", m_bcOff, val);
  push(m_tb->genDefConst<int64>(val));
}

void HhbcTranslator::emitDouble(double val) {
  TRACE(3, "%u: Double %f\n", m_bcOff, val);
  push(m_tb->genDefConst<double>(val));
}

void HhbcTranslator::emitNull() {
  TRACE(3, "%u: Null\n", m_bcOff);
  push(m_tb->genDefNull());
}

void HhbcTranslator::emitTrue() {
  TRACE(3, "%u: True\n", m_bcOff);
  push(m_tb->genDefConst<bool>(true));
}

void HhbcTranslator::emitFalse() {
  TRACE(3, "%u: False\n", m_bcOff);
  push(m_tb->genDefConst<bool>(false));
}

void HhbcTranslator::emitUninitLoc(uint32 id) {
  TRACE(3, "%u: UninitLoc\n", m_bcOff);
  m_tb->genInitLoc(id, m_tb->genDefUninit());
}

void HhbcTranslator::emitInitThisLoc(int32 id) {
  TRACE(3, "%u: InitThisLoc %d\n", m_bcOff, id);
  SSATmp* tmpThis = m_tb->genLdThis(getExitSlowTrace());
  m_tb->genInitLoc(id, m_tb->genIncRef(tmpThis));
}

void HhbcTranslator::emitCGetL(int32 id) {
  TRACE(3, "%u: CGetL %d\n", m_bcOff, id);
  Trace* exitTrace = getExitTrace();
  pushIncRef(emitLdLocWarn(id, exitTrace));
}

void HhbcTranslator::emitCGetL2(int32 id) {
  TRACE(3, "%u: CGetL2 %d\n", m_bcOff, id);
  Trace* exitTrace = getExitTrace();
  SSATmp* oldTop = pop(Type::Gen);
  pushIncRef(emitLdLocWarn(id, exitTrace));
  push(oldTop);
}

void HhbcTranslator::emitVGetL(int32 id) {
  TRACE(3, "%u: VGetL %d\n", m_bcOff, id);
  pushIncRef(m_tb->genBoxLoc(id));
}

void HhbcTranslator::emitVGetG(const StringData* name) {
  // helper function BoxedGlobalCache::lookupCreate
  spillStack();
  popC();
  emitInterpOneOrPunt(Type::Gen);
}

void HhbcTranslator::emitUnsetN() {
  // No reason to punt, translator-x64 does emitInterpOne as well
  spillStack();
  popC();
  emitInterpOne(Type::None);
}

void HhbcTranslator::emitUnsetG() {
  // No reason to punt, translator-x64 does emitInterpOne as well
  spillStack();
  popC();
  emitInterpOne(Type::None);
}

void HhbcTranslator::emitUnsetL(int32 id) {
  TRACE(3, "%u: UnsetL %d\n", m_bcOff, id);
  m_tb->genBindLoc(id, m_tb->genDefUninit());
}

void HhbcTranslator::emitBindL(int32 id) {
  TRACE(3, "%u: BindL %d\n", m_bcOff, id);
  SSATmp* src = popV();
  if (m_curFunc->isPseudoMain()) {
    // in pseudo mains, the value of locals could change in functions
    // called explicitly (or implicitly via exceptions or destructors)
    // so we need to incref eagerly in case one of these functions
    // changes the value of our local and makes src dead.
    pushIncRef(src);
  }
  m_tb->genBindLoc(id, src);
  if (!m_curFunc->isPseudoMain()) {
    pushIncRef(src);
  }
}

void HhbcTranslator::emitSetL(int32 id) {
  TRACE(3, "%u: SetL %d\n", m_bcOff, id);
  Trace* exitTrace = getExitTrace();
  SSATmp* src = popC();
  // Note we can't use the same trick as emitBindL in which we
  // move the incref to after the store because the stored location
  // might contain a ref, which could then be modified by the decref
  // inserted after the stloc
  push(m_tb->genStLoc(id, src, true, true, exitTrace));
}

void HhbcTranslator::emitIncDecL(bool pre, bool inc, uint32 id) {
  // Handle only integer inc/dec for now
  Trace* exitTrace = getExitSlowTrace();
  m_tb->genGuardLoc(id, Type::Int, exitTrace);
  SSATmp* src = m_tb->genLdLoc(id);
  SSATmp* res = emitIncDec(pre, inc, src);
  m_tb->genStLoc(id, res, false, false, NULL);
}

// only handles integer inc/dec
SSATmp* HhbcTranslator::emitIncDec(bool pre, bool inc, SSATmp* src) {
  assert(src->getType() == Type::Int);
  SSATmp* one = m_tb->genDefConst<int64>(1);
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

static bool isSupportedBinaryArith(Type::Tag type1, Type::Tag type2) {
  return ((type1 == Type::Int || type1 == Type::Bool) &&
          (type2 == Type::Int || type2 == Type::Bool));

}

void HhbcTranslator::emitSetOpL(Opcode subOpc, uint32 id) {
  TRACE(3, "%u: SetOpL %d\n", m_bcOff, id);
  Trace* exitTrace = getExitTrace();
  SSATmp* loc = emitLdLocWarn(id, exitTrace);
  SSATmp* val = popC();
  SSATmp* result;
  if (subOpc == Concat) {
    // The concat helpers decref their args, so don't decref pop'ed values
    // and don't decref the old value held in the local. The concat helpers
    // also incref their results, which will be consumed by the stloc. We
    // need an extra incref for the push onto the stack.
    result = m_tb->genConcat(loc, val);
    pushIncRef(m_tb->genStLoc(id, result, false, true, exitTrace));
  } else if (isSupportedBinaryArith(loc->getType(), val->getType())) {
    result = m_tb->gen(subOpc, loc, val);
    push(m_tb->genStLoc(id, result, true, true, exitTrace));
  } else {
    PUNT(SetOpL);
  }
}

void HhbcTranslator::emitClassExists(const StringData* clsName) {
  spillStack();
  popC();
  popC();
  emitInterpOneOrPunt(Type::Bool);
}

void HhbcTranslator::emitInterfaceExists(const StringData* ifaceName) {
  emitClassExists(ifaceName);
}

void HhbcTranslator::emitTraitExists(const StringData* traitName) {
  emitClassExists(traitName);
}

void HhbcTranslator::emitStaticLocInit(uint32 varId, uint32 listStrId) {
  spillStack();
  popC();
  emitInterpOneOrPunt(Type::None);
}

void HhbcTranslator::emitReqDoc(const StringData* name) {
  PUNT(ReqDoc);
// Can't interp one req instructions because their interp one
// function changes the pc, sp, and fp.
//  spillStack();
//  popC();
//  emitInterpOne(Type::Cell);
}

void HhbcTranslator::emitReqMod(const StringData* name) {
//  PUNT(ReqMod);
  emitReqDoc(name);
}

void HhbcTranslator::emitReqSrc(const StringData* name) {
//  PUNT(ReqSrc);
  emitReqDoc(name);
}

template<class Lambda>
SSATmp* HhbcTranslator::emitIterInitCommon(int offset, Lambda genFunc) {
  SSATmp* src = popC();
  Type::Tag type = src->getType();
  if (type != Type::Arr && type != Type::Obj) {
    PUNT(IterInit);
  }
  spillStack();
  SSATmp* res = genFunc(src);
  return emitJmpCondHelper(offset, true, res);
}

void HhbcTranslator::emitIterInit(uint32 iterId,
                                    int offset,
                                    uint32 valLocalId) {
  emitIterInitCommon(offset, [=] (SSATmp* src) {
    return m_tb->genIterInit(src, iterId, valLocalId);
  });
}

void HhbcTranslator::emitIterInitK(uint32 iterId,
                                     int offset,
                                     uint32 valLocalId,
                                     uint32 keyLocalId) {
  emitIterInitCommon(offset, [=] (SSATmp* src) {
    return m_tb->genIterInitK(src, iterId, valLocalId, keyLocalId);
  });
}

void HhbcTranslator::emitIterNext(uint32 iterId,
                                    int offset,
                                    uint32 valLocalId) {
  SSATmp* res = m_tb->genIterNext(iterId, valLocalId);
  emitJmpCondHelper(offset, false, res);
}

void HhbcTranslator::emitIterNextK(uint32 iterId,
                                     int offset,
                                     uint32 valLocalId,
                                     uint32 keyLocalId) {
  SSATmp* res = m_tb->genIterNextK(iterId, valLocalId, keyLocalId);
  emitJmpCondHelper(offset, false, res);
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

  SSATmp* cont = m_tb->genCreateCont(getArgs, origFunc, genFunc);

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
      SSATmp* loc = m_tb->genIncRef(m_tb->genLdLoc(i));
      m_tb->genStMem(locals, cellsToBytes(genLocals - params[i] - 1), loc,
                     true);
    }
    if (fillThis) {
      assert(thisId != kInvalidId);
      m_tb->genFillContThis(cont, locals, cellsToBytes(genLocals - thisId - 1));
    }
  } else {
    m_tb->genFillContLocals(origFunc, genFunc, cont);
  }

  push(cont);
}

void HhbcTranslator::emitContEnter(int32 returnBcOffset) {
  spillStack();

  SSATmp* cont = m_tb->genLdThis(NULL);
  SSATmp* contAR = m_tb->genLdRaw(cont, RawMemSlot::ContARPtr, Type::StkPtr);

  SSATmp* func = m_tb->genLdARFuncPtr(contAR, m_tb->genDefConst<int64>(0));
  SSATmp* funcBody = m_tb->genLdRaw(func, RawMemSlot::FuncBody, Type::TCA);

  m_tb->genContEnter(contAR, funcBody, returnBcOffset);

  // We shouldn't need to change vmsp here
  assert(m_stackDeficit == 0);
}

void HhbcTranslator::emitContExit() {
  SSATmp* retAddr = m_tb->genLdRetAddr();
  // Despite the name, this doesn't actually free the AR; it updates the
  // hardware fp and returns the old one
  SSATmp* fp = m_tb->genFreeActRec();

  SSATmp* sp;
  if (m_stackDeficit) {
    // Adjust the hardware sp before leaving.
    // XXX This returns a ptrToGen
    sp = m_tb->genLdStackAddr(m_stackDeficit);
  } else {
    sp = m_tb->getSp();
  }

  m_tb->genRetCtrl(sp, fp, retAddr);
  m_hasRet = true;
}

void HhbcTranslator::emitUnpackCont() {
  m_tb->genLinkContVarEnv();
  SSATmp* cont = m_tb->genLdAssertedLoc(0, Type::Obj);
  push(m_tb->genLdRaw(cont, RawMemSlot::ContLabel, Type::Int));
}

void HhbcTranslator::emitPackCont(int64 labelId) {
  m_tb->genUnlinkContVarEnv();
  SSATmp* cont = m_tb->genLdAssertedLoc(0, Type::Obj);
  m_tb->genSetPropCell(cont, CONTOFF(m_value), popC());
  m_tb->genStRaw(cont, RawMemSlot::ContLabel,
                 m_tb->genDefConst<int64>(labelId), 0);
}

void HhbcTranslator::emitContReceive() {
  SSATmp* cont = m_tb->genLdAssertedLoc(0, Type::Obj);
  m_tb->genContRaiseCheck(cont, getExitTrace());
  SSATmp* valOffset = m_tb->genDefConst<int64>(CONTOFF(m_received));
  SSATmp* value = m_tb->genLdProp(cont, valOffset, Type::Cell, NULL);
  value = m_tb->genIncRef(value);
  push(value);
}

void HhbcTranslator::emitContRaised() {
  SSATmp* cont = m_tb->genLdAssertedLoc(0, Type::Obj);
  m_tb->genContRaiseCheck(cont, getExitTrace());
}

void HhbcTranslator::emitContDone() {
  SSATmp* cont = m_tb->genLdAssertedLoc(0, Type::Obj);
  m_tb->genStRaw(cont, RawMemSlot::ContDone, m_tb->genDefConst<bool>(true), 0);
  m_tb->genSetPropCell(cont, CONTOFF(m_value), m_tb->genDefNull());
}

void HhbcTranslator::emitContNext() {
  SSATmp* cont = m_tb->genLdThis(NULL);
  m_tb->genContPreNext(cont, getExitTrace());
  m_tb->genSetPropCell(cont, CONTOFF(m_received), m_tb->genDefUninit());
}

void HhbcTranslator::emitContSendImpl(bool raise) {
  SSATmp* cont = m_tb->genLdThis(NULL);
  m_tb->genContStartedCheck(cont, getExitTrace());
  m_tb->genContPreNext(cont, getExitTrace());

  SSATmp* value = m_tb->genLdAssertedLoc(0, Type::Cell);
  value = m_tb->genIncRef(value);
  m_tb->genSetPropCell(cont, CONTOFF(m_received), value);
  if (raise) {
    m_tb->genStRaw(cont, RawMemSlot::ContShouldThrow,
                   m_tb->genDefConst<bool>(true), 0);
  }
}

void HhbcTranslator::emitContSend() {
  emitContSendImpl(false);
}

void HhbcTranslator::emitContRaise() {
  emitContSendImpl(true);
}

void HhbcTranslator::emitContValid() {
  SSATmp* cont = m_tb->genLdThis(NULL);
  SSATmp* done = m_tb->genLdRaw(cont, RawMemSlot::ContDone, Type::Bool);
  push(m_tb->genNot(done));
}

void HhbcTranslator::emitContCurrent() {
  SSATmp* cont = m_tb->genLdThis(NULL);
  m_tb->genContStartedCheck(cont, getExitTrace());
  SSATmp* offset = m_tb->genDefConst<int64>(CONTOFF(m_value));
  SSATmp* value = m_tb->genLdProp(cont, offset, Type::Cell, NULL);
  value = m_tb->genIncRef(value);
  push(value);
}

void HhbcTranslator::emitContStopped() {
  SSATmp* cont = m_tb->genLdThis(NULL);
  m_tb->genStRaw(cont, RawMemSlot::ContRunning, m_tb->genDefConst<bool>(false),
                 0);
}

void HhbcTranslator::emitContHandle() {
  // No reason to punt, translator-x64 does emitInterpOne as well
  spillStack();
  popC();
  emitInterpOne(Type::None);
}

void HhbcTranslator::emitStrlen() {
  Type::Tag inType = topC()->getType();

  if (Type::isString(inType)) {
    SSATmp* input = popC();
    if (input->isConst()) {
      // static string; fold its strlen operation
      push(m_tb->genDefConst<int64>(input->getValStr()->size()));
    } else {
      push(m_tb->genLdRaw(input, RawMemSlot::StrLen, Type::Int));
      m_tb->genDecRef(input);
    }
  } else if (Type::isNull(inType)) {
    popC();
    push(m_tb->genDefConst<int64>(0));
  } else if (inType == Type::Bool) {
    // strlen(true) == 1, strlen(false) == 0.
    push(m_tb->genConvToInt(popC()));
  } else {
    spillStack();
    popC();
    emitInterpOneOrPunt(Type::Gen);
  }
}

void HhbcTranslator::emitIncStat(int32 counter, int32 value, bool force) {
  TRACE(3, "%u: IncStat %d %d\n", m_bcOff, counter, value);
  if (Stats::enabled() || force) {
    m_tb->genIncStat(counter, value, force);
  }
}

SSATmp* HhbcTranslator::getClsPropAddr(const Class* cls,
                                       const StringData* propName) {
  SSATmp* clsTmp = popA();
  SSATmp* prop = popC();
  SSATmp* clsName;
  if (propName) {
    prop = m_tb->genDefConst<const StringData*>(propName);
  }
  if (!cls) {
    PUNT(ClsPropAddr_noCls);
  }
  if (getCurFunc()->cls() != cls) {
    PUNT(ClsPropAddr_clsNE);
  }
  if (!prop->isConst() || prop->getType() != Type::StaticStr) {
    PUNT(ClsPropAddr_noProp);
  }
  if (cls) {
    const StringData* clsNameStr = cls->preClass()->name();
    clsName = m_tb->genDefConst<const StringData*>(clsNameStr);
  } else {
    IRInstruction* clsInst = clsTmp->getInstruction();
    if (clsInst->getOpcode() == LdCls &&
        clsInst->getSrc(0)->getType() == Type::StaticStr) {
      clsName = clsInst->getSrc(0);
    } else {
      PUNT(clsPropAddr_noClassName);
    }
  }
  return m_tb->genLdClsPropAddr(clsTmp, clsName, prop);
}

void HhbcTranslator::decRefPropAddr(SSATmp* propAddr) {
  SSATmp* prop = propAddr->getInstruction()->getSrc(1);
  m_tb->genDecRef(prop);
}


void HhbcTranslator::emitIncDecS(bool pre, bool inc) {
  Trace* exitTrace = getExitSlowTrace();
  SSATmp* propAddr = getClsPropAddr(NULL);
  emitIncDecMem(pre, inc, propAddr, exitTrace);
  decRefPropAddr(propAddr);
}

void HhbcTranslator::emitIncDecProp(bool pre, bool inc, int offset,
                                    bool isPropOnStack) {
  TRACE(3, "%u: IncDecProp %d\n", m_bcOff, offset);
  Trace* exitTrace = getExitSlowTrace();
  if (isPropOnStack) {
    UNUSED SSATmp* prop = popC();
  }
  SSATmp* obj = popC();
  if (obj->getType() != Type::Obj) {
    PUNT(IncDecProp_nonobj);
  }
  SSATmp* propOffset = m_tb->genDefConst<int64>(offset);
  SSATmp* src = m_tb->genLdProp(obj, propOffset, Type::Int, exitTrace);
  // do the add and store back
  SSATmp* res = emitIncDec(pre, inc, src);
  m_tb->genStProp(obj, propOffset, res, false);
  m_tb->genDecRef(obj);
  m_tb->genDecRef(src);
}

void HhbcTranslator::emitSetProp(int offset, bool isPropOnStack) {
  // object + offset
  TRACE(3, "%u: SetProp %d\n", m_bcOff, offset);
  Trace* exitTrace = getExitSlowTrace();
  SSATmp* src = popC();
  if (isPropOnStack) {
    UNUSED SSATmp* prop = popC();
  }
  SSATmp* obj = popC();
  if (obj->getType() != Type::Obj) {
    PUNT(SetProp_nonobj);
  }
  SSATmp* propOffset = m_tb->genDefConst<int64>(offset);
  SSATmp* prevValue;
  if (m_unboxPtrs) {
    SSATmp* propPtr = m_tb->genUnboxPtr(m_tb->genLdPropAddr(obj, propOffset));
    prevValue = m_tb->genLdMem(propPtr, Type::Cell, exitTrace);
    m_tb->genStMem(propPtr, src, true);
  } else {
     prevValue = m_tb->genLdProp(obj, propOffset, Type::Cell, exitTrace);
     m_tb->genStProp(obj, propOffset, src, true);
  }
  pushIncRef(src);
  m_tb->genDecRef(prevValue);
  m_tb->genDecRef(obj);
}

void HhbcTranslator::emitMInstr(const NormalizedInstruction& ni) {
  VectorTranslator(ni, *this).emit();
}

void HhbcTranslator::emitCGetProp(LocationCode locCode,
                                  int offset,
                                  bool isPropOnStack,
                                  Type::Tag resultType,
                                  bool isInferedType) {
  TRACE(3, "%u: CGetM %d\n", m_bcOff, offset);

  Trace* exitTrace1 = getExitSlowTrace();
  Trace* exitTrace2 = getExitSlowTrace();
  if (isPropOnStack) {
    UNUSED SSATmp* prop = popC();
  }
  SSATmp* obj;
  switch (locCode) {
    case LH:
      obj = m_tb->genLdThis(NULL);
      break;
    case LC:
      obj = popC();
      break;
    default:
      PUNT(CGetProp_unsupportedLocation);
  }
  if (obj->getType() != Type::Obj) {
    PUNT(CGetProp_nonobj);
  }
  SSATmp* propOffset = m_tb->genDefConst<int64>(offset);
  SSATmp* val;

  if (isInferedType) {
    assert(Type::isStaticallyKnownUnboxed(resultType));
    val = m_tb->genLdProp(obj, propOffset, resultType, NULL);
  } else {
    if (resultType == Type::None) {
      // result type not predicted
      resultType = Type::Cell;
    } else {
      assert(Type::isStaticallyKnownUnboxed(resultType));
    }
    // This code is currently correct, but once we enable type
    // prediction for CGetM, we should exit normally to a trace
    // executes the CGet as cell type (including the incref of the
    // result and decref of the obj) and exits to the next bytecode
    if (m_unboxPtrs) {
      SSATmp* propPtr = m_tb->genUnboxPtr(m_tb->genLdPropAddr(obj, propOffset));
      val = m_tb->genLdMem(propPtr, resultType, exitTrace1);
    } else {
      val = m_tb->genLdProp(obj, propOffset, resultType, exitTrace1);
    }
    m_tb->genCheckInit(val, exitTrace2);
  }
  pushIncRef(val);
  if (locCode != LH) {
    m_tb->genDecRef(obj);
  }
}

/*
 * IssetH: return true if var is not uninit and !is_null(var)
 * Unboxes var if necessary when var is not uninit.
 */
void HhbcTranslator::emitIssetL(int32 id) {
  TRACE(3, "%u: IssetL %d\n", m_bcOff, id);

  Type::Tag trackedType = m_tb->getLocalType(id);
  // guards should ensure we have type info at this point
  assert(trackedType != Type::None);
  if (trackedType == Type::Uninit) {
    push(m_tb->genDefConst<bool>(false));
  } else {
    Trace* exitTrace = getExitTrace();
    SSATmp* ld = m_tb->genLdLocAsCell(id, exitTrace);
    push(m_tb->gen(IsNType, Type::Null, ld));
  }
}

void HhbcTranslator::emitIssetS() {
  TRACE(3, "%u: IssetS\n", m_bcOff);
  SSATmp* propAddr = getClsPropAddr(NULL);
  push(m_tb->genQueryOp(IsSet, propAddr));
  decRefPropAddr(propAddr);
}

/*
 * EmptyL: return true if var is uninit or !var
 */
void HhbcTranslator::emitEmptyL(int32 id) {
  TRACE(3, "%u: EmptyL %d\n", m_bcOff, id);

  Type::Tag trackedType = m_tb->getLocalType(id);
  assert(trackedType != Type::None);
  if (trackedType == Type::Uninit) {
    push(m_tb->genDefConst<bool>(true));
  } else {
    Trace* exitTrace = getExitTrace();
    SSATmp* ld = m_tb->genLdLocAsCell(id, exitTrace);
    push(m_tb->genNot(m_tb->genConvToBool(ld)));
  }
}

void HhbcTranslator::emitEmptyS() {
  TRACE(3, "%u: EmptyS\n", m_bcOff);
  Trace* exitTrace = getExitSlowTrace();
  SSATmp* propAddr = getClsPropAddr(NULL);
  SSATmp* ld = m_tb->genLdMem(propAddr, Type::Cell, exitTrace);
  push(m_tb->genNot(m_tb->genConvToBool(ld)));
  decRefPropAddr(propAddr);
}

template<Type::Tag T>
void HhbcTranslator::emitIsTypeC() {
  TRACE(3, "%u: Is%sC\n", m_bcOff, Type::Strings[T]);
  SSATmp* src = popC();
  push(m_tb->gen(IsType, T, src));
  m_tb->genDecRef(src);
}

template<Type::Tag T>
void HhbcTranslator::emitIsTypeL(int id) {
  TRACE(3, "%u: Is%sH\n", m_bcOff, Type::Strings[T]);
  Trace* exitTrace = getExitTrace();
  push(m_tb->gen(IsType, T, emitLdLocWarn(id, exitTrace)));
}

void HhbcTranslator::emitIsNullL(int id)   { emitIsTypeL<Type::Null>(id);}
void HhbcTranslator::emitIsArrayL(int id)  { emitIsTypeL<Type::Arr>(id); }
void HhbcTranslator::emitIsStringL(int id) { emitIsTypeL<Type::Str>(id); }
void HhbcTranslator::emitIsObjectL(int id) { emitIsTypeL<Type::Obj>(id); }
void HhbcTranslator::emitIsIntL(int id)    { emitIsTypeL<Type::Int>(id); }
void HhbcTranslator::emitIsBoolL(int id)   { emitIsTypeL<Type::Bool>(id);}
void HhbcTranslator::emitIsDoubleL(int id) { emitIsTypeL<Type::Dbl>(id); }
void HhbcTranslator::emitIsNullC()   { emitIsTypeC<Type::Null>();}
void HhbcTranslator::emitIsArrayC()  { emitIsTypeC<Type::Arr>(); }
void HhbcTranslator::emitIsStringC() { emitIsTypeC<Type::Str>(); }
void HhbcTranslator::emitIsObjectC() { emitIsTypeC<Type::Obj>(); }
void HhbcTranslator::emitIsIntC()    { emitIsTypeC<Type::Int>(); }
void HhbcTranslator::emitIsBoolC()   { emitIsTypeC<Type::Bool>();}
void HhbcTranslator::emitIsDoubleC() { emitIsTypeC<Type::Dbl>(); }

void HhbcTranslator::emitPopC() {
  TRACE(3, "%u: PopC\n", m_bcOff);
  popDecRef(Type::Cell);
}

void HhbcTranslator::emitPopV() {
  TRACE(3, "%u: PopV\n", m_bcOff);
  popDecRef(Type::BoxedCell);
}

void HhbcTranslator::emitPopR() {
  TRACE(3, "%u: PopR\n", m_bcOff);
  popDecRef(Type::Gen);
}

void HhbcTranslator::emitDup() {
  TRACE(3, "%u: Dup\n", m_bcOff);
  pushIncRef(topC());
}

void HhbcTranslator::emitJmp(int32 offset, bool breakTracelet) {
  TRACE(3, "%u: Jmp %d\n", m_bcOff, offset);
  spillStack(); //  spill early since every path will need it
  // If surprise flags are set, exit trace and handle surprise
  bool backward = (offset - (int32)m_bcOff) < 0;
  if (backward) {
    Trace* exit = getExitSlowTrace();
    m_tb->genExitWhenSurprised(exit);
  }
  if (!breakTracelet) return;
  m_tb->genJmp(getExitTrace(offset));
}

SSATmp* HhbcTranslator::emitJmpCondHelper(int32 offset,
                                         bool negate,
                                         SSATmp* src) {
  Trace* target = NULL;
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

void HhbcTranslator::emitJmpZ(int32 offset) {
  TRACE(3, "%u: JmpZ %d\n", m_bcOff, offset);
  SSATmp* src = popC();
  emitJmpCondHelper(offset, true, src);
}

void HhbcTranslator::emitJmpNZ(int32 offset) {
  TRACE(3, "%u: JmpNZ %d\n", m_bcOff, offset);
  SSATmp* src = popC();
  emitJmpCondHelper(offset, false, src);
}

void HhbcTranslator::emitCmp(Opcode opc) {
  TRACE(3, "%u: Cmp %s\n", m_bcOff, opcodeName(opc));

  if (cmpOpTypesMayReenter(opc, topC(0)->getType(), topC(1)->getType())) {
    spillStack();
  }
  // src2 opc src1
  SSATmp* src1 = popC();
  SSATmp* src2 = popC();
  push(m_tb->genCmp(opc, src2, src1));
  m_tb->genDecRef(src2);
  m_tb->genDecRef(src1);
}

void HhbcTranslator::emitClsCnsD(int32 cnsNameStrId, int32 clsNameStrId) {
  // This bytecode re-enters if there is no class with the given name
  // and can throw a fatal error.
  const StringData* cnsNameStr = lookupStringId(cnsNameStrId);
  const StringData* clsNameStr = lookupStringId(clsNameStrId);
  TRACE(3, "%u: ClsCnsD %s::%s\n", m_bcOff, clsNameStr->data(),
        cnsNameStr->data());
  Trace* exitTrace = getExitSlowTrace();
  SSATmp* cnsNameTmp = m_tb->genDefConst(cnsNameStr);
  SSATmp* clsNameTmp = m_tb->genDefConst(clsNameStr);
  SSATmp* cns = m_tb->gen(LdClsCns, Type::Cell, cnsNameTmp, clsNameTmp);
  m_tb->genCheckInit(cns, exitTrace);
  push(cns);
}

void HhbcTranslator::emitFPassR() {
  TRACE(3, "%u: FPassR\n", m_bcOff);
  emitUnboxRAux();
}

void HhbcTranslator::emitFPassCOp() {
  TRACE(3, "%u: FPassCOp\n", m_bcOff);
}

void HhbcTranslator::emitNativeImpl() {
  TRACE(3, "%u: NativeImpl\n", m_bcOff);
  m_tb->genNativeImpl();
  SSATmp* retAddr = m_tb->genLdRetAddr();
  SSATmp* sp = m_tb->genRetAdjustStack();
  SSATmp* fp = m_tb->genFreeActRec();    // updates fp
  m_tb->genRetCtrl(sp, fp, retAddr);

  // Flag that this trace has a Ret instruction so no ExitTrace is needed
  m_hasRet = true;
}

void HhbcTranslator::emitFPushCtor(int32 numParams) {
  TRACE(3, "%u: FPushFuncCtor %d\n", m_bcOff, numParams);
  SSATmp* cls = popA();
  spillStack();
  SSATmp* newObj = m_tb->genNewObj(numParams, cls);
  m_fpiStack.push(newObj);
}

void HhbcTranslator::emitFPushCtorD(int32 numParams, int32 classNameStrId) {
  const StringData* className = lookupStringId(classNameStrId);
  TRACE(3, "%u: FPushFuncCtorD %d %s\n", m_bcOff, numParams, className->data());
  spillStack();
  SSATmp* newObj = m_tb->genNewObj(numParams, className);
  m_fpiStack.push(newObj);
}

void HhbcTranslator::emitFPushFuncD(int32 numParams, int32 funcId) {
  TRACE(3, "%u: FPushFuncD %d %d\n", m_bcOff, numParams, funcId);
  const NamedEntityPair& nep = lookupNamedEntityPairId(funcId);
  const StringData* name = nep.first;
  const Func* func       = Unit::lookupFunc(nep.second, name);
  // Translation is only supported if function lookup succeeds
  if (!func) {
    PUNT(FPushFuncDNull);
  }
  func->validate();

  const bool immutable = func->isNameBindingImmutable(getCurUnit());

  if (immutable) {
    spillStack();  // LdFixedFunc can reenter
  }
  SSATmp* ssaFunc = immutable ? m_tb->genDefConst<const Func*>(func)
                              : m_tb->gen(LdFixedFunc,
                                          m_tb->genDefConst(name));
  SSATmp* actRec  = m_tb->genDefActRec(ssaFunc,
                                       m_tb->genDefNull(),
                                       numParams,
                                       nullptr);
  m_evalStack.push(actRec);
  spillStack(); // TODO(#2036900)
  m_fpiStack.push(immutable ? actRec : ssaFunc);
}

void HhbcTranslator::emitFPushFunc(int32 numParams) {
  TRACE(3, "%u: FPushFuncD %d\n", m_bcOff, numParams);
  // input must be a string or an object implementing __invoke();
  // otherwise fatal
  SSATmp* funcName = popC();
  if (!Type::isString(funcName->getType())) {
    PUNT(FPushFunc_not_Str);
  }

  spillStack(); // LdFunc can reenter
  SSATmp* func = m_tb->gen(LdFunc, funcName);
  m_fpiStack.push(func);
  m_evalStack.push(m_tb->genDefActRec(func,
                                      m_tb->genDefNull(),
                                      numParams,
                                      nullptr));
  spillStack(); // TODO(#2036900)
}

void HhbcTranslator::emitFPushObjMethodD(int32 numParams,
                                         int32 methodNameStrId,
                                         const Class* baseClass) {
  const StringData* methodName = lookupStringId(methodNameStrId);
  TRACE(3, "%u: FPushObjMethodD %s %d\n",
        m_bcOff,
        methodName->data(),
        numParams);
  bool magicCall = false;
  SSATmp* funcTmp = NULL;
  const Func* func = HPHP::VM::Transl::lookupImmutableMethod(baseClass,
                                                             methodName,
                                                             magicCall,
                                                         /* staticLookup: */
                                                             false);
  SSATmp* objOrCls = popC();

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
          SSATmp* clsTmp = m_tb->gen(LdObjClass, objOrCls);
          funcTmp = m_tb->genLdClsMethod(clsTmp, func->methodSlot());
          if (res == MethodLookup::MethodFoundNoThis) {
            m_tb->genDecRef(objOrCls);
            objOrCls = clsTmp;
          }
        }
      } else {
        func = NULL; // force lookup
      }
    }
  }

  if (func != NULL && funcTmp == NULL) {
    if (func->attrs() & AttrStatic) {
      assert(baseClass);  // This assert may be too strong, but be aggressive
      // static function: store base class into this slot instead of obj
      // and decref the obj that was pushed as the this pointer since
      // the obj won't be in the actrec and thus MethodCache::lookup won't
      // decref it
      m_tb->genDecRef(objOrCls);
      objOrCls = m_tb->genDefConst<const Class*>(baseClass);
    }
  }

  const StringData* invName = nullptr;
  if (!funcTmp) {
    funcTmp = func ? m_tb->genDefConst<const Func*>(func)
                   : m_tb->genDefNull();
    if (func && magicCall) {
      invName = methodName;
    }
  }
  SSATmp* actRec = m_tb->genDefActRec(funcTmp,
                                      objOrCls,
                                      numParams,
                                      invName);
  m_evalStack.push(actRec);
  spillStack(); // TODO(#2036900)
  if (!func) {
    SSATmp* sp = spillStack();
    SSATmp* meth = m_tb->genLdObjMethod(methodName, sp);
    m_fpiStack.push(meth);
  } else {
    m_fpiStack.push(actRec);
  }
}

void HhbcTranslator::emitFPushClsMethodD(int32 numParams,
                                         int32 methodNameStrId,
                                         int32 clssNamedEntityPairId,
                                         bool mightNotBeStatic) {

  const StringData* methodName = lookupStringId(methodNameStrId);
  const NamedEntityPair& np = lookupNamedEntityPairId(clssNamedEntityPairId);
  UNUSED const StringData* className = np.first;
  TRACE(3, "%u: FPushClsMethodD %s::%s %d\n", m_bcOff, className->data(),
        methodName->data(), numParams);
  const Class* baseClass = Unit::lookupClass(np.second);
  bool magicCall = false;
  const Func* func = HPHP::VM::Transl::lookupImmutableMethod(baseClass,
                                                             methodName,
                                                             magicCall,
                                                         /* staticLookup: */
                                                             true);
  spillStack();
  SSATmp* objOrCls = m_tb->genDefNull();
  if (func) {
    if (!mightNotBeStatic) { // definitely static
      // static function: store base class into the m_cls/m_this slot
      objOrCls = m_tb->genDefConst<const Class*>(baseClass);
    } else if (m_tb->isThisAvailable()) {
      // 'this' pointer is available, so use it.
      objOrCls = m_tb->genIncRef(m_tb->genLdThis(NULL));
    } else {
      // might be a non-static call
      // generate code that tests at runtime whether to use
      // this pointer or class
      PUNT(FPushClsMethodD_MightNotBeStatic);
      assert(0);
    }
  } else {
    // lookup static method & class in the target cache
    Trace* exitTrace = getExitSlowTrace();
    const StringData* className = np.first;
    SSATmp* funcClassTmp = m_tb->genLdClsMethodCache(
                              m_tb->genDefConst(className),
                              m_tb->genDefConst(methodName),
                              m_tb->genDefConst(np.second),
                              exitTrace);
    SSATmp* actRec = m_tb->genDefActRec(funcClassTmp,
                                        m_tb->genDefNull(),
                                        numParams,
                                        nullptr);
    m_evalStack.push(actRec);
    spillStack(); // TODO(#2036900)
    m_fpiStack.push(actRec);
    return;
  }
  SSATmp* actRec = m_tb->genDefActRec(
    m_tb->genDefConst<const Func*>(func),
    objOrCls,
    numParams,
    func && magicCall ? methodName : nullptr);
  m_evalStack.push(actRec);
  spillStack(); // TODO(#2036900)
  m_fpiStack.push(actRec);
}

void HhbcTranslator::emitFCall(uint32_t numParams,
                               Offset returnBcOffset,
                               const Func* callee) {
  // pop the actrec or func from FPI stack
  SSATmp* actRecOrFunc = m_fpiStack.pop();

  // pop the incoming parameters to the call
  SSATmp* params[numParams];
  for (uint32 i = 0; i < numParams; i++) {
    params[numParams - i - 1] = popF();
  }

  SSATmp* func = callee ? m_tb->genDefConst<const Func*>(callee) : nullptr;
  SSATmp* actRec = spillStack();
  if (!func) {
    if (actRecOrFunc && actRecOrFunc->getType() == Type::FuncPtr) {
      func = actRecOrFunc;
    } else {
      func = m_tb->genDefNull();
    }
  }

  m_tb->genCall(actRec,
                returnBcOffset,
                func,
                numParams,
                params);
}

void HhbcTranslator::emitRet(SSATmp* retVal, Trace* exitTrace,
                             bool freeInline) {
  const Func* curFunc = getCurFunc();
  bool mayUseVV = (curFunc->attrs() & AttrMayUseVV);
  bool mayHaveThis = (curFunc->isPseudoMain() ||
                      (curFunc->isMethod() && !curFunc->isStatic()));

  m_tb->genExitWhenSurprised(exitTrace);
  if (mayUseVV) {
    // Note: this has to be the first thing, because we cannot bail after
    //       we start decRefing locs because then there'll be no corresponding
    //       bytecode boundaries until the end of RetC
    m_tb->genReleaseVVOrExit(exitTrace);
  }
  if (mayHaveThis) {
    m_tb->genDecRefThis();
  }

  SSATmp* sp;
  SSATmp* retAddr;
  if (freeInline) {
    for (int id = curFunc->numLocals() - 1; id >= 0; --id) {
      /*
       * TODO(#1980291): this doesn't correctly handle
       * debug_backtrace.
       */
      m_tb->genDecRefLoc(id);
    }
    retAddr = m_tb->genLdRetAddr();
    m_tb->genRetVal(retVal);
    sp = m_tb->genRetAdjustStack();
  } else {
    sp = m_tb->genGenericRetDecRefs(retVal, curFunc->numLocals());
    retAddr = m_tb->genLdRetAddr();
    m_tb->genRetVal(retVal);
  }

  // Free ActRec, and return control to caller.
  SSATmp* fp = m_tb->genFreeActRec();
  m_tb->genRetCtrl(sp, fp, retAddr);

  // Flag that this trace has a Ret instruction, so that no ExitTrace is needed
  m_hasRet = true;
}

void HhbcTranslator::emitRetC(bool freeInline) {
  Trace* exit = getExitSlowTrace();
  emitRet(popC(), exit, freeInline);
}

void HhbcTranslator::emitRetV(bool freeInline) {
  Trace* exit = getExitSlowTrace();
  emitRet(popV(), exit, freeInline);
}

void HhbcTranslator::setThisAvailable() {
  m_tb->setThisAvailable();
}

void HhbcTranslator::guardTypeLocal(uint32 locId, Type::Tag type) {
  checkTypeLocal(locId, type);
  m_typeGuards.push_back(TypeGuard(TypeGuard::Local, locId, type));
}

void HhbcTranslator::checkTypeLocal(uint32 locId, Type::Tag type) {
  m_tb->genGuardLoc(locId, type, getExitTrace());
}

void HhbcTranslator::assertTypeLocal(uint32 localIndex, Type::Tag type) {
  m_tb->genAssertLoc(localIndex, type);
}

Trace* HhbcTranslator::guardTypeStack(uint32 stackIndex,
                                      Type::Tag type,
                                      Trace* nextTrace) {
  if (nextTrace == NULL) {
    nextTrace = getGuardExit();
  }
  m_tb->genGuardStk(stackIndex, type, nextTrace);
  m_typeGuards.push_back(TypeGuard(TypeGuard::Stack, stackIndex, type));

  return nextTrace;
}

void HhbcTranslator::checkTypeTopOfStack(Type::Tag type,
                                         Offset nextByteCode) {
  Trace* exitTrace = getExitTrace(nextByteCode);
  SSATmp* tmp = m_evalStack.top();
  if (!tmp) {
    FTRACE(1, "checkTypeTopOfStack: no tmp: {}\n", Type::Strings[type]);
    m_tb->genGuardStk(0, type, exitTrace);
    push(pop(type));
  } else {
    FTRACE(1, "checkTypeTopOfStack: generating GuardType for {}\n",
           Type::Strings[type]);
    m_evalStack.pop();
    tmp = m_tb->genGuardType(tmp, type, exitTrace);
    push(tmp);
  }
}

void HhbcTranslator::assertTypeStack(uint32 stackIndex, Type::Tag type) {
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

Trace* HhbcTranslator::guardRefs(int64               entryArDelta,
                                 const vector<bool>& mask,
                                 const vector<bool>& vals,
                                 Trace*              exitTrace) {
  if (exitTrace == NULL) {
    exitTrace = getGuardExit();
  }

  int32_t actRecOff = cellsToBytes(entryArDelta);
  SSATmp* funcPtr = m_tb->genLdARFuncPtr(m_tb->getSp(),
                                        m_tb->genDefConst<int64>(actRecOff));
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

    SSATmp* mask64Tmp   = m_tb->genLdConst<int64>(mask64);
    SSATmp* vals64Tmp   = m_tb->genLdConst<int64>(vals64);
    SSATmp* firstBitNum = m_tb->genDefConst<int64>(i);

    m_tb->genGuardRefs(funcPtr, nParams, bitsPtr, firstBitNum,
                      mask64Tmp, vals64Tmp, exitTrace);
  }

  return exitTrace;
}

void HhbcTranslator::emitVerifyParamType(int32 paramId,
                                         const StringData* constraintClsName) {
  /* It's currently better to interpOne all of these, since the slow path punts
   * for the entire tracelet */
  spillStack();
  emitInterpOneOrPunt(Type::None);
  return;

  assert(!constraintClsName || constraintClsName->isStatic());
  const Func* func = getCurFunc();
  const Class* constraint = (constraintClsName ?
                             Unit::lookupClass(constraintClsName) : NULL);
  const TypeConstraint& tc = func->params()[paramId].typeConstraint();
  const StringData* tcTypeName = tc.typeName();

  // VerifyParamType does not remove the parameter from the stack
  TRACE(3, "%u: VerifyParamType %s\n", m_bcOff, tcTypeName->data());
  Trace* exitTrace2 = getExitSlowTrace();
  // XXX Should verify param type unbox?
  SSATmp* param = m_tb->genLdAssertedLoc(paramId, Type::Gen);
  if (param->getType() != Type::Obj) {
    PUNT(VerifyParamType_nonobj);
  }
  SSATmp* objClass = m_tb->gen(LdObjClass, param);

  if (tc.isObject()) {
    if (!(param->getType() == Type::Obj ||
          (param->getType() == Type::Null && tc.nullable()))) {
      spillStack();
      emitInterpOne(Type::None);
      return;
    }
  } else {
    if (!tc.check(frame_local(curFrame() /* FIXME */, paramId), getCurFunc())) {
      spillStack();
      emitInterpOne(Type::None);
      return;
    }
  }

  m_tb->genVerifyParamType(objClass,
                          m_tb->genDefConst<const StringData*>(tcTypeName),
                          constraint,
                          exitTrace2);
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
  if (!src->isA(Type::Obj)) {
    // If it's a Cell, it might still be an object.  All other types
    // push false.
    if (!Type::isMoreRefined(src->getType(), Type::Cell)) {
      PUNT(InstanceOfD_Cell);
    }
    push(m_tb->genDefConst(false));
    m_tb->genDecRef(src);
    return;
  }

  SSATmp* objClass     = m_tb->gen(LdObjClass, src);
  SSATmp* ssaClassName = m_tb->genDefConst(className);

  Class::initInstanceBits();
  const bool haveBit = Class::haveInstanceBit(className);

  Class* const maybeCls = Unit::lookupClass(className);
  const bool isNormalClass = maybeCls &&
                             !(maybeCls->attrs() &
                               (AttrTrait | AttrInterface));
  const bool isUnique = RuntimeOption::RepoAuthoritative &&
                        maybeCls && (maybeCls->attrs() & AttrUnique);

  /*
   * If the class is unique or a parent of the current context, we
   * don't need to load it out of target cache because it must
   * already exist and be defined.
   *
   * Otherwise, we only use LdCachedClass---instanceof with an
   * undefined class doesn't invoke autoload.
   */
  SSATmp* checkClass =
    isUnique || (maybeCls && getCurClass() &&
                  getCurClass()->classof(maybeCls))
      ? m_tb->genDefConst(maybeCls)
      : m_tb->gen(LdCachedClass, ssaClassName);

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

void HhbcTranslator::emitCastBool() {
  SSATmp* src = popC();
  push(m_tb->genConvToBool(src));
  m_tb->genDecRef(src);
}

void HhbcTranslator::emitCastInt() {
  SSATmp* src = popC();
  push(m_tb->genConvToInt(src));
  m_tb->genDecRef(src);
}

void HhbcTranslator::emitCastDouble() {
  SSATmp* src = popC();
  push(m_tb->genConvToDbl(src));
  m_tb->genDecRef(src);
}

void HhbcTranslator::emitCastString() {
  SSATmp* src = popC();
  Type::Tag fromType = src->getType();
  if (fromType == Type::Cell) {
    PUNT(CastString_Cell);
  } else if (fromType == Type::Obj) {
    // call the toString helper on object
    PUNT(CastString_Obj);
  } else {
    // for int to string conversion, this calls a helper that returns
    // a string with ref count of 0.
    pushIncRef(m_tb->genConvToStr(src));
  }
  m_tb->genDecRef(src);
}

void HhbcTranslator::emitCastArray() {
  SSATmp* src = popC();
  pushIncRef(m_tb->genConvToArr(src));
  m_tb->genDecRef(src);
}

void HhbcTranslator::emitCastObject() {
  SSATmp* src = popC();
  pushIncRef(m_tb->genConvToObj(src));
  m_tb->genDecRef(src);
}

static
bool isSupportedAGet(SSATmp* classSrc, const StringData* className) {
  Type::Tag srcType = classSrc->getType();
  return (srcType == Type::Obj ||
          className != NULL ||
          (Type::isString(srcType) && classSrc->isConst()));
}

void HhbcTranslator::emitAGet(SSATmp* classSrc) {
  Type::Tag srcType = classSrc->getType();
  if (Type::isString(srcType)) {
    push(m_tb->genLdCls(classSrc));
  } else if (srcType == Type::Obj) {
    push(m_tb->gen(LdObjClass, classSrc));
  } else {
    assert(0);
  }
}

void HhbcTranslator::emitAGetC(const StringData* clsName) {
  if (isSupportedAGet(topC(), clsName)) {
    SSATmp* src = popC();
    if (clsName != NULL) {
      src = m_tb->genDefConst<const StringData*>(clsName);
    }
    emitAGet(src);
    m_tb->genDecRef(src);
  } else {
    spillStack();
    popC();
    emitInterpOne(Type::ClassPtr);
  }
}

void HhbcTranslator::emitAGetL(int id, const StringData* clsName) {
  Trace* exitTrace = getExitTrace();
  SSATmp* src = m_tb->genLdLocAsCell(id, exitTrace);
  if (isSupportedAGet(src, clsName)) {
    if (clsName != NULL) {
      src = m_tb->genDefConst<const StringData*>(clsName);
    }
    emitAGet(src);
  } else {
    spillStack();
    emitInterpOne(Type::ClassPtr);
  }
}

void HhbcTranslator::emitCGetS(const Class* cls,
                               const StringData* propName,
                               Type::Tag resultType,
                               bool isInferedType) {

  TRACE(3, "%u: CGetS\n", m_bcOff);
  Trace* exitTrace = getExitSlowTrace();
  SSATmp* propPtr = getClsPropAddr(cls, propName);
  SSATmp* val;
  if (isInferedType) {
    assert(Type::isStaticallyKnownUnboxed(resultType));
    val = m_tb->genLdMem(propPtr, resultType, NULL);
  } else {
    if (resultType == Type::None) {
      // result type not predicted
      resultType = Type::Cell;
    } else {
      assert(Type::isStaticallyKnownUnboxed(resultType));
    }
    // Type is predicted, so take slow exit if check fails to avoid repeating
    // this situation.
    // This code is currently correct, but once we enable type
    // prediction for CGetS, we should exit normally to a trace that
    // executes the CGetS as cell type (including the incref of the
    // result) and exits to the next bytecode.
    val = m_tb->genLdMem(m_unboxPtrs ? m_tb->genUnboxPtr(propPtr) : propPtr,
                        resultType,
                        exitTrace);
  }
  pushIncRef(val);
  decRefPropAddr(propPtr);
}

void HhbcTranslator::emitVGetS() {
  TRACE(3, "%u: VGetS\n", m_bcOff);
  PUNT(VGetS);
}

void HhbcTranslator::emitCGetG(const StringData* name,
                               Type::Tag resultType, bool isInferedType) {
  spillStack();
  popC();
  if (isInferedType) {
    assert(Type::isUnboxed(resultType) &&
           resultType != Type::Cell &&
           resultType != Type::None);
  } else {
    assert(resultType == Type::None); // Type prediction shouldn't happen
    resultType = Type::Cell;
  }
  emitInterpOneOrPunt(resultType);
}

void HhbcTranslator::emitSetG() {
  spillStack();
  popC();
  popC();
  emitInterpOneOrPunt(Type::Cell);
}

void HhbcTranslator::emitSetS(const Class* cls, const StringData* propName) {
  Trace* exitTrace = getExitSlowTrace();
  SSATmp* src = popC();
  SSATmp* propAddrToDecRef = getClsPropAddr(cls, propName);
  SSATmp* propPtr = m_unboxPtrs ? m_tb->genUnboxPtr(propAddrToDecRef)
                                : propAddrToDecRef;
  SSATmp* prevValue = m_tb->genLdMem(propPtr, Type::Cell, exitTrace);
  pushIncRef(src);
  m_tb->genStMem(propPtr, src, true);
  m_tb->genDecRef(prevValue);
  decRefPropAddr(propAddrToDecRef);
}

void HhbcTranslator::emitBinaryArith(Opcode opc) {
  bool isBitOp = (opc == OpAnd || opc == OpOr || opc == OpXor);
  Type::Tag type1 = topC(0)->getType();
  Type::Tag type2 = topC(1)->getType();
  if (isSupportedBinaryArith(type1, type2)) {
    SSATmp* tr = popC();
    SSATmp* tl = popC();
    push(m_tb->gen(opc, tl, tr));
  } else if (isBitOp && (type1 == Type::Obj || type2 == Type::Obj)) {
    // raise fatal
    spillStack();
    popC();
    popC();
    emitInterpOne(Type::Cell);
  } else {
    spillStack();
    popC();
    popC();
    Type::Tag type = Type::Int;
    if (isBitOp) {
      if (Type::isString(type1) && Type::isString(type2)) {
        type = Type::Str;
      } else if ((!Type::isStaticallyKnown(type1) &&
                  (!Type::isStaticallyKnown(type2)
                   || Type::isString(type2)))
                 ||
                 (!Type::isStaticallyKnown(type2)
                  && Type::isString(type1))) {
        // both types might be strings, but can't tell
        type = Type::Cell;
      } else {
        type = Type::Int;
      }
    } else {
      // either an int or a dbl, but can't tell
      type = Type::Cell;
    }
    emitInterpOne(type);
  }
}

void HhbcTranslator::emitNot() {
  TRACE(3, "%u: Not\n", m_bcOff);
  SSATmp* src = popC();
  push(m_tb->genNot(m_tb->genConvToBool(src)));
  m_tb->genDecRef(src);
}

void HhbcTranslator::emitAdd() {
  TRACE(3, "%u: Add\n", m_bcOff);
  emitBinaryArith(OpAdd);
}
void HhbcTranslator::emitSub() {
  TRACE(3, "%u: Sub\n", m_bcOff);
  emitBinaryArith(OpSub);
}
void HhbcTranslator::emitMul() {
  TRACE(3, "%u: Mul\n", m_bcOff);
  emitBinaryArith(OpMul);
}
void HhbcTranslator::emitBitNot() {
  TRACE(3, "%u: BitNot\n", m_bcOff);
  Type::Tag srcType = topC()->getType();
  if (srcType == Type::Int) {
    SSATmp* src = popC();
    SSATmp* ones = m_tb->genDefConst<int64>(~0);
    push(m_tb->genXor(src, ones));
  } else if (srcType == Type::Null || srcType == Type::Bool ||
             srcType == Type::Arr || srcType == Type::Obj) {
    // raise fatal
    spillStack();
    popC();
    emitInterpOne(Type::Cell);
  } else {
    spillStack();
    popC();
    Type::Tag resultType = Type::Int;
    if (Type::isString(srcType)) {
      resultType = Type::Str;
    } else if (!Type::isStaticallyKnown(srcType)) {
      resultType = Type::Cell;
    }
    emitInterpOne(resultType);
  }
}
void HhbcTranslator::emitBitAnd() {
  TRACE(3, "%u: BitAnd\n", m_bcOff);
  emitBinaryArith(OpAnd);
}
void HhbcTranslator::emitBitOr() {
  TRACE(3, "%u: BitOr\n", m_bcOff);
  emitBinaryArith(OpOr);
}
void HhbcTranslator::emitBitXor() {
  TRACE(3, "%u: BitXor\n", m_bcOff);
  emitBinaryArith(OpXor);
}
void HhbcTranslator::emitXor() {
  TRACE(3, "%u: Xor\n", m_bcOff);
  SSATmp* btr = popC();
  SSATmp* btl = popC();
  SSATmp* tr = m_tb->genConvToBool(btr);
  SSATmp* tl = m_tb->genConvToBool(btl);
  push(m_tb->genConvToBool(m_tb->genXor(tl, tr)));
  m_tb->genDecRef(btl);
  m_tb->genDecRef(btr);
}

void HhbcTranslator::emitInterpOne(Type::Tag type, Trace* target /* = NULL */) {
  if (0) {
    m_tb->genTraceEnd(m_bcOff, TraceExitType::SlowNoProgress);
    // TODO need to push something on the stack...
    // Better approach here is to modify the code gen to generate an exit
    // when it sees interp one
  } else {
    m_tb->genInterpOne(m_bcOff, m_stackDeficit, type, target);
    m_stackDeficit = 0;
  }
}

void HhbcTranslator::emitInterpOneOrPunt(Type::Tag type,
                                         Trace* target /* = NULL */) {
  if (RuntimeOption::EvalIRPuntDontInterp) {
    PUNT(PuntDontInterp);
  } else {
    emitInterpOne(type, target);
  }
}

Trace* HhbcTranslator::getGuardExit() {
  assert(m_bcOff == -1 || m_bcOff == m_startBcOff);
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
    if (elem->getType() == Type::ActRec) {
      /*
       * For register allocation purposes, the SpillStack instruction
       * should count as a use of each of the SSATmps coming into the
       * a DefActRec.
       */
      auto* inst = elem->getInstruction();
      assert(inst->getNumSrcs() == 5);
      ret.push_back(inst->getSrc(1)); // func
      ret.push_back(inst->getSrc(2)); // objOrCls
      ret.push_back(inst->getSrc(3)); // numArgs
      ret.push_back(inst->getSrc(4)); // invName
    }
  }
  return ret;
}

/*
 * Generates an exit trace which will continue the VM execution at the given
 * nextByteCode (defaults to the current m_bcOff) without using HHIR.
 * This should be used in situations that HHIR cannot handle -- ideally only in
 * slow paths.
 */
Trace* HhbcTranslator::getExitSlowTrace() {
  std::vector<SSATmp*> stackValues = getSpillValues();
  return m_tb->getExitSlowTrace(m_bcOff,
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
    targetBcOff = m_bcOff != -1 ? m_bcOff : m_startBcOff;
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
Trace* HhbcTranslator::getExitTrace(uint32 targetBcOff, uint32 notTakenBcOff) {
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

SSATmp* HhbcTranslator::loadStackAddr(int32 offset) {
  // You're almost certainly doing it wrong if you want to get the address of a
  // stack cell that's in m_evalStack.
  assert(offset >= (int32)m_evalStack.numCells());
  return m_tb->genLdStackAddr(
    offset + m_stackDeficit - m_evalStack.numCells());
}

//
// This is a wrapper to TraceBuilder::genLdLoc() that also emits the
// RaiseUninitWarning if the local is uninitialized
//
SSATmp* HhbcTranslator::emitLdLocWarn(uint32 id,
                                      Trace* target) {
  SSATmp* locVal = m_tb->genLdLocAsCell(id, target);

  if (locVal->getType() == Type::Uninit) {
    spillStack();
    m_tb->genRaiseUninitWarning(id);
    return m_tb->genDefNull();
  }

  return locVal;
}

void HhbcTranslator::end(int nextPc) {
  if (m_hasRet) return;

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
