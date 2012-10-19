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

#include "hhbctranslator.h"
#include "runtime/ext/ext_continuation.h"
#include "runtime/vm/translator/translator-x64.h"
#include <util/trace.h>
#include "runtime/vm/unit.h"
#include "runtime/vm/runtime.h"

#define HHIR_UNIMPLEMENTED(op)                          \
  do {                                                  \
    throw FailedIRGen(__FILE__, __LINE__, #op);         \
  } while (0)

using namespace HPHP::VM::Transl;

namespace HPHP {
namespace VM {
namespace JIT {

static const HPHP::Trace::Module TRACEMOD = HPHP::Trace::hhir;

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
  m_evalStack.push(tmp);
  return tmp;
}

void HhbcTranslator::refineType(SSATmp* tmp, Type::Tag type) {
  // If type is more refined that tmp's type, reset tmp's type to type
  IRInstruction* inst = tmp->getInstruction();
  if (Type::isMoreRefined(type, inst->getType())) {
    inst->setType(type);
    // If tmp is incref or move, then chase down its src
    Opcode opc = inst->getOpcode();
    if (opc == Mov || opc == IncRef) {
      refineType(inst->getSrc(0), type);
    } else {
      // at this point, we have no business refining the type of any
      // instructions other than the following
      ASSERT (opc == LdLoc   || opc == LdStack  ||
              opc == LdMemNR || opc == LdPropNR ||
              opc == LdRefNR || opc == LdClsCns);
    }
  }
}

SSATmp* HhbcTranslator::pop(Type::Tag type, Trace* exitTrace) {
  SSATmp* opnd = m_evalStack.pop();
  if (opnd == NULL) {
    uint32 stackOff = m_stackDeficit;
    m_stackDeficit++;
    // we use an exit trace in case a later genGuardType adds a
    // guardType to this instruction
    return m_tb.genLdStack(stackOff, type, exitTrace);
  }
  if (exitTrace == NULL) {
    // if we have an exit trace label, then we are guarding that the
    // top has a particular type and we should not use refineType
    // to refine tmp's type.
    refineType(opnd, type);
  }
  return opnd;
}

// type is the type expected on the stack.
void HhbcTranslator::popDecRef(Type::Tag type, Trace* exitTrace) {
  SSATmp* src = m_evalStack.pop();
  if (src == NULL) {
    uint32 stackOff = m_stackDeficit;
    m_tb.genDecRefStack(type, stackOff, exitTrace);
    m_stackDeficit++;
    return;
  } else {
    m_tb.genDecRef(src);
  }
}

void HhbcTranslator::extendStack(uint32 index,
                                 Type::Tag type,
                                 Trace* exitTrace) {
  ASSERT(index != (uint32)-1);
  // We don't know what type description to expect, so we use a generic
  // type here. If this ends up pushing a ldStack, refineType
  // will later fix up the type to the expected type.
  SSATmp* tmp;
  if (index != 0) {
    tmp = pop(Type::Gen, exitTrace);
    extendStack(index-1, type, exitTrace);
  } else {
    tmp = pop(type, exitTrace);
  }
  push(tmp);
}

SSATmp* HhbcTranslator::top(Type::Tag type, uint32 index) {
  ASSERT(index != (uint32)-1);
  SSATmp* tmp = m_evalStack.top(index);
  if (!tmp) {
    extendStack(index);
    tmp = m_evalStack.top(index);
  }
  ASSERT(tmp);
  refineType(tmp, type);
  return tmp;
}

void HhbcTranslator::replace(uint32 index, SSATmp* tmp) {
  ASSERT(index < m_evalStack.numElems());
  m_evalStack.replace(index, tmp);
}

void HhbcTranslator::setBcOff(uint32 newOff, bool lastBcOff) {
  if (newOff != m_bcOff || m_firstBcOff) {
    if (m_bcOff != 0xffffffff) {
      m_firstBcOff = false;
    }
    m_bcOff = newOff;
    m_tb.genMarker(m_bcOff,
                   m_tb.getSpOffset() + m_evalStack.numElems() - m_stackDeficit
                  );
  }
  m_lastBcOff = lastBcOff;
}

void HhbcTranslator::emitPrint() {
  TRACE(3, "%u: Print\n", m_bcOff);
  Type::Tag type = topC()->getType();
  if (type == Type::Int || type == Type::Bool || type == Type::Null ||
      Type::isString(type)) {
    // the print helpers decref their arg, so don't decref pop'ed value
    m_tb.genPrint(popC());
    push(m_tb.genDefConst<int64>(1));
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
  Trace* exitTrace = getExitSlowTrace();
  SSATmp* tmp = popR();
  pushIncRef(m_tb.genUnbox(tmp, Type::Cell, exitTrace));
  m_tb.genDecRef(tmp);
}

void HhbcTranslator::emitUnboxR() {
  TRACE(3, "%u: UnboxR\n", m_bcOff);
  emitUnboxRAux();
}

void HhbcTranslator::emitThis() {
  TRACE(3, "%u: This\n", m_bcOff);
  pushIncRef(m_tb.genLdThis(getExitSlowTrace()));
}

void HhbcTranslator::emitCheckThis() {
  TRACE(3, "%u: CheckThis\n", m_bcOff);
  m_tb.genLdThis(getExitSlowTrace());
}

void HhbcTranslator::emitArray(int arrayId) {
  TRACE(3, "%u: Array %d\n", m_bcOff, arrayId);
  ArrayData* ad = lookupArrayId(arrayId);
  push(m_tb.genDefConst<const ArrayData*>(ad));
}

void HhbcTranslator::emitNewArray() {
  TRACE(3, "%u: NewArray\n", m_bcOff);
  ArrayData* ad = HphpArray::GetStaticEmptyArray();
  push(m_tb.genDefConst<const ArrayData*>(ad));
}

void HhbcTranslator::emitNewTuple(int numArgs) {
  spillStack();
  for (int i = 0; i < numArgs; i++) popC();
  emitInterpOneOrPunt(Type::Arr);
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
  push(m_tb.genArrayAdd(tl, tr));
}

void HhbcTranslator::emitAddElemC() {
  TRACE(3, "%u: AddElemC\n", m_bcOff);
  SSATmp* val = popC();
  SSATmp* key = popC();
  SSATmp* arr = popC();
  // the AddElem helper decrefs its args, so don't decref pop'ed values
  // TODO task 1805916: verify that AddElem increfs its result
  push(m_tb.genAddElem(arr, key, val));
}

void HhbcTranslator::emitAddNewElemC() {
  TRACE(3, "%u: AddNewElemC\n", m_bcOff);
  SSATmp* val = popC();
  SSATmp* arr = popC();
  // the AddNewElem helper decrefs its args, so don't decref pop'ed values
  // TODO task 1805916: verify that NewElem increfs its result
  push(m_tb.genAddNewElem(arr, val));
}

void HhbcTranslator::emitCns(uint32 id) {
  spillStack();
  emitInterpOneOrPunt(Type::Cell);
}

void HhbcTranslator::emitDefCns(uint32 id) {
  TRACE(3, "%u: DefCns %d\n", m_bcOff, id);
  StringData* name = lookupStringId(id);
  SSATmp* val = popC();
  push(m_tb.genDefCns(name, val));
}

void HhbcTranslator::emitConcat() {
  SSATmp* tr = popC();
  SSATmp* tl = popC();
  // the concat helpers decref their args, so don't decref pop'ed values
  push(m_tb.genConcat(tl, tr));
}

void HhbcTranslator::emitDefCls(int cid, Offset after) {
//  m_tb.genDefCls(lookupPreClassId(cid), getCurUnit()->at(after));
  spillStack();
  emitInterpOneOrPunt(Type::None);
}

void HhbcTranslator::emitDefFunc(int fid) {
//  m_tb.genDefFunc(lookupFuncId(fid));
  spillStack();
  emitInterpOneOrPunt(Type::None);
}

void HhbcTranslator::emitSelf() {
  Class* clss = getCurClass();
  if (RuntimeOption::RepoAuthoritative &&
     (clss->preClass()->attrs() & AttrUnique)) {
    push(m_tb.genDefConst<const Class*>(clss));
  } else {
    push(m_tb.genLdFuncCls(m_tb.genLdCurFuncPtr()));
  }

}

void HhbcTranslator::emitParent() {
  Class* clss = getCurClass()->parent();
  // if in repo authorative mode & the class is unique, then we can
  // just burn the class ref into an immediate
  if (RuntimeOption::RepoAuthoritative &&
     (clss->preClass()->attrs() & AttrUnique)) {
    push(m_tb.genDefConst<const Class*>(clss));
  } else {
    SSATmp* className = m_tb.genDefConst<const StringData*>(clss->name());
    push(m_tb.genLdCls(className));
  }
}

void HhbcTranslator::emitString(int strId) {
  TRACE(3, "%u: String %d\n", m_bcOff, strId);
  push(m_tb.genDefConst<const StringData*>(lookupStringId(strId)));
}

void HhbcTranslator::emitInt(int64 val) {
  TRACE(3, "%u: Int %lld\n", m_bcOff, val);
  push(m_tb.genDefConst<int64>(val));
}

void HhbcTranslator::emitDouble(double val) {
  TRACE(3, "%u: Double %f\n", m_bcOff, val);
  push(m_tb.genDefConst<double>(val));
}

void HhbcTranslator::emitNull() {
  TRACE(3, "%u: Null\n", m_bcOff);
  push(m_tb.genDefNull());
}

void HhbcTranslator::emitTrue() {
  TRACE(3, "%u: True\n", m_bcOff);
  push(m_tb.genDefConst<bool>(true));
}

void HhbcTranslator::emitFalse() {
  TRACE(3, "%u: False\n", m_bcOff);
  push(m_tb.genDefConst<bool>(false));
}

void HhbcTranslator::emitUninitLoc(uint32 id) {
  TRACE(3, "%u: UninitLoc\n", m_bcOff);
  m_tb.genInitLoc(id, m_tb.genDefUninit());
}

void HhbcTranslator::emitInitThisLoc(int32 id) {
  TRACE(3, "%u: InitThisLoc %d\n", m_bcOff, id);
  SSATmp* tmpThis = m_tb.genLdThis(getExitSlowTrace());
  m_tb.genInitLoc(id, m_tb.genIncRef(tmpThis));
}

void HhbcTranslator::emitCGetL(int32 id) {
  TRACE(3, "%u: CGetL %d\n", m_bcOff, id);
  Trace* exitTrace = getExitSlowTrace();
  pushIncRef(emitLdLocWarn(id, Type::Cell, exitTrace));
}

void HhbcTranslator::emitCGetL2(int32 id) {
  TRACE(3, "%u: CGetL2 %d\n", m_bcOff, id);
  Trace* exitTrace = getExitSlowTrace();
  SSATmp* oldTop = popC();
  pushIncRef(emitLdLocWarn(id, Type::Cell, exitTrace));
  push(oldTop);
}

void HhbcTranslator::emitVGetL(int32 id) {
  TRACE(3, "%u: VGetL %d\n", m_bcOff, id);
  pushIncRef(m_tb.genBoxLoc(id));
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
  m_tb.genBindLoc(id, m_tb.genDefUninit());
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
  m_tb.genBindLoc(id, src);
  if (!m_curFunc->isPseudoMain()) {
    pushIncRef(src);
  }
}

void HhbcTranslator::emitSetL(int32 id) {
  TRACE(3, "%u: SetL %d\n", m_bcOff, id);
  Trace* exitTrace = getExitSlowTrace();
  SSATmp* src = popC();
  // Note we can't use the same trick as emitBindL in which we
  // move the incref to after the store because the stored location
  // might contain a ref, which could then be modified by the decref
  // inserted after the stloc
  push(m_tb.genStLoc(id, src, true, true, exitTrace));
}

void HhbcTranslator::emitIncDecL(bool pre, bool inc, uint32 id) {
  Trace* exitTrace = getExitSlowTrace();
  // Handle only integer inc/dec for now
  SSATmp* src = emitLdLocWarn(id, Type::Int, exitTrace);
  SSATmp* res = emitIncDec(pre, inc, src);
  m_tb.genStLoc(id, res, false, false, NULL);
}

// only handles integer inc/dec
SSATmp* HhbcTranslator::emitIncDec(bool pre, bool inc, SSATmp* src) {
  ASSERT(src->getType() == Type::Int);
  SSATmp* one = m_tb.genDefConst<int64>(1);
  SSATmp* res = inc ? m_tb.genAdd(src, one) : m_tb.genSub(src, one);
  // no incref necessary on push since result is an int
  push(pre ? res : src);
  return res;
}

void HhbcTranslator::emitIncDecMem(bool pre,
                                   bool inc,
                                   SSATmp* propAddr,
                                   Trace* exitTrace) {
  // Handle only integer inc/dec for now
  SSATmp* src = m_tb.genLdMem(propAddr, Type::Int, exitTrace);
  // do the add and store back
  SSATmp* res = emitIncDec(pre, inc, src);
  // don't gen a dec ref or type store
  m_tb.genStMem(propAddr, res, false);
}

void HhbcTranslator::emitSetOpL(Opcode subOpc, uint32 id) {
  spillStack();
  popC();
  emitInterpOneOrPunt(Type::Cell);
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

void HhbcTranslator::emitIterInit(uint32 iterVarId, int offset) {
  PUNT(IterInit);
// Need to model control flow in interpone to make it work for IterInit
//  spillStack();
//  popC();
//  emitInterpOne(getExitTrace(offset));
//  spillStack();

}

void HhbcTranslator::emitIterKey(uint32 iterVarId) {
  spillStack();
  emitInterpOneOrPunt(Type::Cell);
}

void HhbcTranslator::emitIterNext(uint32 iterVarId, int offset) {
  PUNT(IterNext);
// Need to model control flow in interpone to make it work for IterNext
//  spillStack();
//  emitInterpOne(getExitTrace(offset));
}

void HhbcTranslator::emitIterValueC(uint32 iterVarId) {
  spillStack();
  emitInterpOneOrPunt(Type::Cell);
}

// continuations
SSATmp* HhbcTranslator::getContLocals(SSATmp* cont) {
  /* Using this before iterating over each of the locals allows us to save code
   * space: even with only one local the net effect is 3 bytes saved, with up
   * to 6 more bytes save for each additional local. */
  return m_tb.genLdPropAddr(cont, m_tb.genDefConst<int64>(
                              c_Continuation::localsOffset()));
}

void HhbcTranslator::emitCreateCont(bool getArgs,
                                    Id funNameStrId) {
  /* Runtime-determined slow path punts to TranslatorX64 for now */
  m_tb.genExitOnVarEnv(getExitSlowTrace());

  const StringData* genName = lookupStringId(funNameStrId);
  const Func* origFunc = curFunc();
  const Func* genFunc = origFunc->getGeneratorBody(genName);
  int origLocals = origFunc->numNamedLocals();
  int genLocals = genFunc->numNamedLocals() - 1;

  SSATmp* cont = m_tb.genCreateCont(getArgs, origFunc, genFunc);

  TranslatorX64::ContParamMap params;
  if (origLocals <= TranslatorX64::kMaxInlineContLocals &&
      TranslatorX64::mapContParams(params, origFunc, genFunc)) {
    static const StringData* thisStr = StringData::GetStaticString("this");
    Id thisId = kInvalidId;
    bool fillThis = origFunc->isNonClosureMethod() && !origFunc->isStatic() &&
      ((thisId = genFunc->lookupVarId(thisStr)) != kInvalidId) &&
      (origFunc->lookupVarId(thisStr) == kInvalidId);
    SSATmp* locals = getContLocals(cont);
    for (int i = 0; i < origLocals; ++i) {
      SSATmp* loc = m_tb.genIncRef(m_tb.genLdLoc(i, Type::Gen, NULL));
      m_tb.genStMem(locals, cellsToBytes(genLocals - params[i]), loc, true);
    }
    if (fillThis) {
      ASSERT(thisId != kInvalidId);
      m_tb.genFillContThis(cont, locals, cellsToBytes(genLocals - thisId));
    }
  } else {
    m_tb.genFillContLocals(origFunc, genFunc, cont);
  }

  push(cont);
}

void HhbcTranslator::emitUnpackCont() {
  int nCopy = curFunc()->numNamedLocals() - 1;
  if (nCopy > TranslatorX64::kMaxInlineContLocals) {
    SSATmp* locals = m_tb.genLdLocAddr(nCopy);
    spillStack();
    push(m_tb.genUnpackCont(m_tb.genLdLoc(0), locals));
    for (int i = 0; debug && i < nCopy; ++i) {
      ASSERT(m_tb.getLocalValue(nCopy - i) == NULL);
    }
    return;
  }

  SSATmp* cont = m_tb.genLdLoc(0, Type::Obj, NULL);
  m_tb.genExitOnContVars(cont, getExitSlowTrace());

  SSATmp* locals = getContLocals(cont);
  SSATmp* uninit = m_tb.genDefUninit();
  for (int i = 0; i < nCopy; ++i) {
    int contOffset = cellsToBytes(i);
    SSATmp* val = m_tb.genLdMem(locals, contOffset, Type::Gen, NULL);
    m_tb.genStMem(locals, contOffset, uninit, true);
    m_tb.genInitLoc(nCopy - i, val);
  }

  SSATmp* offset = m_tb.genDefConst<int64>(CONTOFF(m_label));
  push(m_tb.genLdRaw(cont, offset, Type::Int));
}

void HhbcTranslator::emitPackCont(int32 labelId) {
  int nCopy = curFunc()->numNamedLocals() - 1;
  if (nCopy > TranslatorX64::kMaxInlineContLocals) {
    spillStack();
    m_tb.genPackCont(m_tb.genLdLoc(0, Type::Obj, NULL),
                     loadStackAddr(0),
                     labelId,
                     curFunc());
    popC();
    return;
  }

  m_tb.genExitOnVarEnv(getExitSlowTrace());

  SSATmp* cont = m_tb.genLdLoc(0, Type::Obj, NULL);
  SSATmp* uninit = m_tb.genDefUninit();
  SSATmp* locals = getContLocals(cont);
  for (int i = 0; i < nCopy; ++i) {
    int locId = nCopy - i;
    SSATmp* local = m_tb.genLdLoc(locId, Type::Gen, NULL);
    m_tb.genInitLoc(locId, uninit);
    m_tb.genStMem(locals, cellsToBytes(i), local, true);
  }

  m_tb.genSetPropCell(cont, CONTOFF(m_value), popC());
  m_tb.genStRaw(cont, CONTOFF(m_label), m_tb.genDefConst<int64>(labelId));
}

void HhbcTranslator::emitContReceive() {
  SSATmp* cont = m_tb.genLdLoc(0, Type::Obj, NULL);
  m_tb.genContRaiseCheck(cont, getExitSlowTrace());

  SSATmp* valOffset = m_tb.genDefConst<int64>(CONTOFF(m_received));
  SSATmp* value = m_tb.genLdProp(cont, valOffset, Type::Cell, NULL);
  value = m_tb.genIncRef(value);
  push(value);
}

void HhbcTranslator::emitContRaised() {
  SSATmp* cont = m_tb.genLdLoc(0, Type::Obj, NULL);
  m_tb.genContRaiseCheck(cont, getExitSlowTrace());
}

void HhbcTranslator::emitContDone() {
  SSATmp* cont = m_tb.genLdLoc(0, Type::Obj, NULL);
  m_tb.genStRaw(cont, CONTOFF(m_done), m_tb.genDefConst<bool>(true));
}

void HhbcTranslator::emitContNext() {
  SSATmp* cont = m_tb.genLdThis(NULL);
  m_tb.genContPreNext(cont, getExitSlowTrace());
  m_tb.genSetPropCell(cont, CONTOFF(m_received), m_tb.genDefUninit());
}

void HhbcTranslator::emitContSendImpl(bool raise) {
  SSATmp* cont = m_tb.genLdThis(NULL);
  m_tb.genContStartedCheck(cont, getExitSlowTrace());
  m_tb.genContPreNext(cont, getExitSlowTrace());

  SSATmp* value = m_tb.genLdLoc(0, Type::Cell, NULL);
  value = m_tb.genIncRef(value);
  m_tb.genSetPropCell(cont, CONTOFF(m_received), value);
  if (raise) {
    m_tb.genStRaw(cont, CONTOFF(m_should_throw), m_tb.genDefConst<bool>(true));
  }
}

void HhbcTranslator::emitContSend() {
  emitContSendImpl(false);
}

void HhbcTranslator::emitContRaise() {
  emitContSendImpl(true);
}

void HhbcTranslator::emitContValid() {
  SSATmp* cont = m_tb.genLdThis(NULL);
  SSATmp* done =
    m_tb.genLdRaw(cont, m_tb.genDefConst<int64>(CONTOFF(m_done)), Type::Bool);
  push(m_tb.genNot(done));
}

void HhbcTranslator::emitContCurrent() {
  SSATmp* cont = m_tb.genLdThis(NULL);
  m_tb.genContStartedCheck(cont, getExitSlowTrace());
  SSATmp* offset = m_tb.genDefConst<int64>(CONTOFF(m_value));
  SSATmp* value = m_tb.genLdProp(cont, offset, Type::Cell, NULL);
  value = m_tb.genIncRef(value);
  push(value);
}

void HhbcTranslator::emitContStopped() {
  SSATmp* cont = m_tb.genLdThis(NULL);
  m_tb.genStRaw(cont, CONTOFF(m_running), m_tb.genDefConst<bool>(false));
}

void HhbcTranslator::emitContHandle() {
  // No reason to punt, translator-x64 does emitInterpOne as well
  spillStack();
  popC();
  emitInterpOne(Type::None);
}

SSATmp* HhbcTranslator::getClsPropAddr(const Class* cls,
                                       const StringData* propName) {
  SSATmp* clsTmp = popA();
  SSATmp* prop = popC();
  SSATmp* clsName;
  if (propName) {
    prop = m_tb.genDefConst<const StringData*>(propName);
  }
  // TODO: fallback to interpone if we decide to punt
  if (!cls || curFunc()->cls() != cls) {
    PUNT(ClsPropAddr_noCls);
  }
  if (!prop->isConst() || prop->getType() != Type::StaticStr) {
    PUNT(ClsPropAddr_noProp);
  }
  if (cls) {
    const StringData* clsNameStr = cls->preClass()->name();
    clsName = m_tb.genDefConst<const StringData*>(clsNameStr);
  } else {
    IRInstruction* clsInst = clsTmp->getInstruction();
    if (clsInst->getOpcode() == LdCls &&
        clsInst->getSrc(0)->getType() == Type::StaticStr) {
      clsName = clsInst->getSrc(0);
    } else {
      PUNT(clsPropAddr_noClassName);
    }
  }
  return m_tb.genLdClsPropAddr(clsTmp, clsName, prop);
}

void HhbcTranslator::decRefPropAddr(SSATmp* propAddr) {
  SSATmp* prop = propAddr->getInstruction()->getSrc(1);
  m_tb.genDecRef(prop);
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
  SSATmp* propOffset = m_tb.genDefConst<int64>(offset);
  SSATmp* src = m_tb.genLdProp(obj, propOffset, Type::Int, exitTrace);
  // do the add and store back
  SSATmp* res = emitIncDec(pre, inc, src);
  m_tb.genStProp(obj, propOffset, res, false);
  m_tb.genDecRef(obj);
  m_tb.genDecRef(src);
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
  SSATmp* propOffset = m_tb.genDefConst<int64>(offset);
  SSATmp* prevValue;
  if (m_unboxPtrs) {
    SSATmp* propPtr = m_tb.genUnboxPtr(m_tb.genLdPropAddr(obj, propOffset));
    prevValue = m_tb.genLdMem(propPtr, Type::Cell, exitTrace);
    m_tb.genStMem(propPtr, src, true);
  } else {
     prevValue = m_tb.genLdProp(obj, propOffset, Type::Cell, exitTrace);
     m_tb.genStProp(obj, propOffset, src, true);
  }
  pushIncRef(src);
  m_tb.genDecRef(prevValue);
  m_tb.genDecRef(obj);
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
      obj = m_tb.genLdThis(NULL);
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
  SSATmp* propOffset = m_tb.genDefConst<int64>(offset);
  SSATmp* val;

  if (isInferedType) {
    ASSERT(Type::isStaticallyKnownUnboxed(resultType));
    val = m_tb.genLdProp(obj, propOffset, resultType, NULL);
  } else {
    if (resultType == Type::None) {
      // result type not predicted
      resultType = Type::Cell;
    } else {
      ASSERT(Type::isStaticallyKnownUnboxed(resultType));
    }
    // This code is currently correct, but once we enable type
    // prediction for CGetM, we should exit normally to a trace
    // executes the CGet as cell type (including the incref of the
    // result and decref of the obj) and exits to the next bytecode
    if (m_unboxPtrs) {
      SSATmp* propPtr = m_tb.genUnboxPtr(m_tb.genLdPropAddr(obj, propOffset));
      val = m_tb.genLdMem(propPtr, resultType, exitTrace1);
    } else {
      val = m_tb.genLdProp(obj, propOffset, resultType, exitTrace1);
    }
    m_tb.genCheckUninit(val, exitTrace2);
  }
  pushIncRef(val);
  if (locCode != LH) {
    m_tb.genDecRef(obj);
  }
}

/*
 * IssetH: return true if var is not uninit and !is_null(var)
 * Unboxes var if necessary when var is not uninit.
 */
void HhbcTranslator::emitIssetL(int32 id) {
  TRACE(3, "%u: IssetL %d\n", m_bcOff, id);

  Type::Tag trackedType = m_tb.getLocalType(id);
  // guards should ensure we have type info at this point
  ASSERT(trackedType != Type::None);
  if (trackedType == Type::Uninit) {
    push(m_tb.genDefConst<bool>(false));
  } else {
    Trace* exitTrace = getExitSlowTrace();
    SSATmp* ld = m_tb.genLdLoc(id, Type::Cell, exitTrace);
    push(m_tb.genNot(m_tb.genIsType<Type::Null>(ld)));
  }
}

void HhbcTranslator::emitIssetS() {
  TRACE(3, "%u: IssetS\n", m_bcOff);
  SSATmp* propAddr = getClsPropAddr(NULL);
  push(m_tb.genQueryOp(IsSet, propAddr));
  decRefPropAddr(propAddr);
}

/*
 * EmptyL: return true if var is uninit or !var
 */
void HhbcTranslator::emitEmptyL(int32 id) {
  TRACE(3, "%u: EmptyL %d\n", m_bcOff, id);

  Type::Tag trackedType = m_tb.getLocalType(id);
  ASSERT(trackedType != Type::None);
  if (trackedType == Type::Uninit) {
    push(m_tb.genDefConst<bool>(true));
  } else {
    Trace* exitTrace = getExitSlowTrace();
    SSATmp* ld = m_tb.genLdLoc(id, Type::Cell, exitTrace);
    push(m_tb.genNot(m_tb.genConvToBool(ld)));
  }
}

void HhbcTranslator::emitEmptyS() {
  TRACE(3, "%u: EmptyS\n", m_bcOff);
  Trace* exitTrace = getExitSlowTrace();
  SSATmp* propAddr = getClsPropAddr(NULL);
  SSATmp* ld = m_tb.genLdMem(propAddr, Type::Cell, exitTrace);
  push(m_tb.genNot(m_tb.genConvToBool(ld)));
  decRefPropAddr(propAddr);
}

template<Type::Tag T>
void HhbcTranslator::emitIsTypeC() {
  TRACE(3, "%u: Is%sC\n", m_bcOff, Type::Strings[T]);
  SSATmp* src = popC();
  push(m_tb.genIsType<T>(src));
  m_tb.genDecRef(src);
}

template<Type::Tag T>
void HhbcTranslator::emitIsTypeL(int id) {
  TRACE(3, "%u: Is%sH\n", m_bcOff, Type::Strings[T]);
  Trace* exitTrace = getExitSlowTrace();
  push(m_tb.genIsType<T>(emitLdLocWarn(id, Type::Cell, exitTrace)));
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
  popDecRef(Type::Cell, NULL);
}

void HhbcTranslator::emitPopV() {
  TRACE(3, "%u: PopV\n", m_bcOff);
  popDecRef(Type::BoxedCell, NULL);
}

void HhbcTranslator::emitPopR() {
  TRACE(3, "%u: PopR\n", m_bcOff);
  popDecRef(Type::Gen, NULL);
}

void HhbcTranslator::emitDup() {
  TRACE(3, "%u: Dup\n", m_bcOff);
  pushIncRef(topC());
}

Trace* HhbcTranslator::emitJmp(int32 offset) {
  TRACE(3, "%u: Jmp %d\n", m_bcOff, offset);
  spillStack(); //  spill early since every path will need it
  // If surprise flags are set, exit trace and handle surprise
  bool backward = (offset - (int32)m_bcOff) < 0;
  if (backward) {
    Trace* exit = getExitSlowTrace();
    m_tb.genExitWhenSurprised(exit);
  }

  Trace* target = getExitTrace(offset);
  return m_tb.genJmp(target);
}

Trace* HhbcTranslator::emitJmpCondHelper(int32 offset, bool negate) {
  SSATmp* src = popC();
  Trace* target = NULL;
  if (m_lastBcOff) {
    // Spill everything on main trace if all paths will exit
    spillStack();
    uint32 nextTrace = getBcOffNextTrace();
    target = getExitTrace(offset, nextTrace);
  } else {
    target = getExitTrace(offset);
  }
  SSATmp* boolSrc = m_tb.genConvToBool(src);
  m_tb.genDecRef(src);
  return m_tb.genJmpCond(boolSrc, target, negate);
}

Trace* HhbcTranslator::emitJmpZ(int32 offset) {
  TRACE(3, "%u: JmpZ %d\n", m_bcOff, offset);
  return emitJmpCondHelper(offset, true);
}

Trace* HhbcTranslator::emitJmpNZ(int32 offset) {
  TRACE(3, "%u: JmpNZ %d\n", m_bcOff, offset);
  return emitJmpCondHelper(offset, false);
}

void HhbcTranslator::emitCmp(Opcode opc) {
  TRACE(3, "%u: Cmp %s\n", m_bcOff, OpcodeStrings[opc]);
  SSATmp* src1 = popC();
  SSATmp* src2 = popC();
  // src2 opc src1
  push(m_tb.genCmp(opc, src2, src1));
  m_tb.genDecRef(src2);
  m_tb.genDecRef(src1);
}

void HhbcTranslator::emitClsCnsD(int32 cnsNameStrId, int32 clsNameStrId) {
  // This bytecode re-enters if there is no class with the given name
  // and can throw a fatal error.
  const StringData* cnsNameStr = lookupStringId(cnsNameStrId);
  const StringData* clsNameStr = lookupStringId(clsNameStrId);
  TRACE(3, "%u: ClsCnsD %s::%s\n", m_bcOff, clsNameStr->data(),
        cnsNameStr->data());
  Trace* exitTrace = getExitSlowTrace();
  SSATmp* cnsNameTmp = m_tb.genDefConst<const StringData*>(cnsNameStr);
  SSATmp* clsNameTmp = m_tb.genDefConst<const StringData*>(clsNameStr);
  push(m_tb.genLdClsCns(cnsNameTmp, clsNameTmp, exitTrace));
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
#if 1
  m_tb.genNativeImpl();
  SSATmp* retAddr = m_tb.genLdRetAddr();
  SSATmp* sp = m_tb.genRetVal();        // updates sp
  SSATmp* fp = m_tb.genFreeActRec();    // updates fp
  m_tb.genRetCtrl(sp, fp, retAddr);
#else
  spillStack();
  emitInterpOne();
#endif

  // Flag that this trace has a Ret instruction so no ExitTrace is needed
  this->m_hasRet = true;
}

void HhbcTranslator::emitFPushCtor(int32 numParams) {
  TRACE(3, "%u: FPushFuncCtor %d\n", m_bcOff, numParams);
//  PUNT(FPushCtor);
  SSATmp* cls = popA();
  spillStack();
  SSATmp* newObj = m_tb.genNewObj(numParams, cls);
  m_fpiStack.push(newObj);
}

void HhbcTranslator::emitFPushCtorD(int32 numParams, int32 classNameStrId) {
  const StringData* className = lookupStringId(classNameStrId);
  TRACE(3, "%u: FPushFuncCtorD %d %s\n", m_bcOff, numParams, className->data());
  spillStack();
  SSATmp* newObj = m_tb.genNewObj(numParams, className);
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
  spillStack();
  if (func->isNameBindingImmutable(getCurUnit())) {
    // func can't change
    m_fpiStack.push(m_tb.genAllocActRec(func,
                                        m_tb.genDefNull(),
                                        numParams,
                                        NULL));
  } else {
    SSATmp* actRec = m_tb.genAllocActRec(m_tb.genDefNull(),
                                         m_tb.genDefNull(),
                                         numParams,
                                         NULL);
    m_fpiStack.push(m_tb.genLdFixedFunc(name, actRec));
  }
}

void HhbcTranslator::emitFPushFunc(int32 numParams) {
  TRACE(3, "%u: FPushFuncD %d\n", m_bcOff, numParams);
  // input must be a string or an object implementing __invoke();
  // otherwise fatal
  SSATmp* funcName = popC();
  if (!Type::isString(funcName->getType())) {
    PUNT(FPushFunc_not_Str);
  }
  spillStack();
  SSATmp* actRec = m_tb.genAllocActRec(m_tb.genDefNull(),
                                       m_tb.genDefNull(),
                                       numParams,
                                       NULL);
  m_fpiStack.push(m_tb.genLdFunc(funcName, actRec));
}

void HhbcTranslator::emitFPushContFunc() {
  Class* genClass = curFrame()->getThis()->getVMClass();
  ASSERT(genClass == SystemLib::s_MethodContinuationClass ||
         genClass == SystemLib::s_FunctionContinuationClass);
  bool isMethod = genClass == SystemLib::s_MethodContinuationClass;

  SSATmp* cont = m_tb.genLdThis(NULL);
  SSATmp* funcOffset = m_tb.genDefConst<int64>(CONTOFF(m_vmFunc));
  SSATmp* thiz = isMethod ? m_tb.genLdContThisOrCls(cont) : m_tb.genDefNull();
  SSATmp* func = m_tb.genLdRaw(cont, funcOffset, Type::FuncRef);
  m_fpiStack.push(m_tb.genAllocActRec(func, thiz, 1, NULL));
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
  const Func* func = HPHP::VM::Transl::lookupImmutableMethod(baseClass,
                                                             methodName,
                                                             magicCall,
                                                         /* staticLookup: */
                                                             false);
  SSATmp* objOrCls = popC();
  if (func) {
    if (func->attrs() & AttrStatic) {
      ASSERT(baseClass);  // This assert may be too strong, but be aggressive
      // static function: store base class into this slot instead of obj
      // and decref the obj that was pushed as the this pointer since
      // the obj won't be in the actrec and thus MethodCache::lookup won't
      // decref it
      m_tb.genDecRef(objOrCls);
      objOrCls = m_tb.genDefConst<const Class*>(baseClass);
    }
  }
  spillStack();
  SSATmp* actRec = m_tb.genAllocActRec(func,
                                       objOrCls,
                                       numParams,
                                       (func && magicCall ? methodName : NULL));
  if (!func) {
    // lookup the function
    SSATmp* meth = m_tb.genLdObjMethod(methodName, actRec);
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
  SSATmp* objOrCls = m_tb.genDefNull();
  if (func) {
    if (!mightNotBeStatic) { // definitely static
      // static function: store base class into the m_cls/m_this slot
      objOrCls = m_tb.genDefConst<const Class*>(baseClass);
    } else if (m_tb.isThisAvailable()) {
      // 'this' pointer is available, so use it.
      objOrCls = m_tb.genIncRef(m_tb.genLdThis(NULL));
    } else {
      // might be a non-static call
      // generate code that tests at runtime whether to use
      // this pointer or class
      PUNT(MightNotBeStatic);
#if 0
      // XXX TODO: Need to represent this in the IR
      ScratchReg rClsScratch(m_regMap);
      PhysReg rCls = *rClsScratch;
      a.    load_reg64_disp_reg64(rVmFp, AROFF(m_cls), rCls);
      a.    test_imm32_reg64(1, rCls);
      {
        IfElseBlock<CC_NZ> ifThis(a);
        // rCls is holding $this. We should pass it to the callee
        emitIncRef(rCls, KindOfObject);
        emitVStackStore(a, i, rCls, clsOff);
        ifThis.Else();
        emitVStackStoreImm(a, i, uintptr_t(baseClass)|1, clsOff);
      }
#endif
      ASSERT(0);
    }
  } else {
    // lookup static method & class in the target cache
    Trace* exitTrace = getExitSlowTrace();
    const StringData* className = np.first;
    SSATmp* funcClassTmp = m_tb.genLdClsMethod(
                              m_tb.genDefConst<const StringData*>(className),
                              m_tb.genDefConst<const StringData*>(methodName),
    /*TODO: NamedEntity* */   m_tb.genDefConst<int64>((uintptr_t)np.second),
                              exitTrace);
    SSATmp* actRec = m_tb.genAllocActRec(funcClassTmp, numParams);
    m_fpiStack.push(actRec);
    return;
  }
  SSATmp* tmp = m_tb.genAllocActRec(func,
                                    objOrCls,
                                    numParams,
                                    (func && magicCall ? methodName : NULL));
  m_fpiStack.push(tmp);
}

void HhbcTranslator::emitFCallAux(uint32 numParams,
                                  uint32 returnBcOffset,
                                  const Func* callee) {
  // pop the actrec or func from FPI stack
  SSATmp* actRecOrFunc = m_fpiStack.pop();
  // pop the incoming parameters to the call
  SSATmp* params[numParams];
  for (uint32 i = 0; i < numParams; i++) {
    params[numParams - i - 1] = popF();
  }
  SSATmp* func = callee ? m_tb.genDefConst<const Func*>(callee) : NULL;
  bool allocActRec;
  if (actRecOrFunc == NULL) {
    // this fcall and its FPush are in seperate traces; pop and throw
    // away the cells corresponding to the act rec on the stack
    allocActRec = true;
    uint32 numActRecCells = (sizeof(ActRec) / sizeof(Cell));
    for (uint32 i = 0; i < numActRecCells; i++) {
      pop();
    }
  } else {
    allocActRec = false;
    if (!func && actRecOrFunc->getType() == Type::FuncRef) {
      // we had a func on the FPI stack
      func = actRecOrFunc;
    }
    // there may have been another call between the allocActRec
    // and this call, so we need to spill stack below
  }
  if (!func) {
    func = m_tb.genDefNull();
  }
  SSATmp* actRec = spillStack(allocActRec);
  m_tb.genCall(actRec,
               returnBcOffset,
               func,
               numParams,
               params);

}

void HhbcTranslator::emitFCall(uint32 numParams, uint32 returnBcOffset) {
  TRACE(3, "%u: FCall %u %u\n", m_bcOff, numParams, returnBcOffset);
  emitFCallAux(numParams, returnBcOffset, NULL);
}

void HhbcTranslator::emitFCallD(uint32 numParams,
                                const Func* callee,
                                uint32 returnBcOffset) {
  TRACE(3, "%u: FCallD %s %u %u\n", m_bcOff,
        callee->fullName()->data(), numParams, returnBcOffset);
  emitFCallAux(numParams, returnBcOffset, callee);
}

// Bytecode instructions RetC/RetV get broken into the IR instructions:
//    ExitWhenSurprised
//    ExitOnVarEnv
//    DecRef(loc), for each loc in curFunc
//    DecRef(this), if curFunc is a non-static method
//    retAddr = LdRetAddr
//    sp = RetVal
//    fp = FreeActRec
//    RetCtrl retAddr

void HhbcTranslator::emitRet(SSATmp* retVal, Trace* exitTrace) {
  const Func* curFunc = getCurFunc();
  bool mayUseVV = (curFunc->attrs() & AttrMayUseVV);
  bool mayHaveThis = (curFunc->isPseudoMain() ||
                      (curFunc->isMethod() && !curFunc->isStatic()));
  bool freeInline = freeLocalsInline();

  if (freeInline) {

    // If surprise flags are set, exit trace and handle surprise
    m_tb.genExitWhenSurprised(exitTrace);

    if (mayUseVV) {
      // Emit code to bail to an exit if frame has VarEnv
      // Note: this has to be the first thing, because we cannot bail after
      //       we start decRefing locs because then there'll be no corresponding
      //       bytecode boundaries until the end of RetC
      m_tb.genExitOnVarEnv(exitTrace);
    }

    // decref refcounted locals
    for (int id = 0; id < curFunc->numLocals(); id++) {
      m_tb.genDecRefLoc(id);
    }

    // decref $this
    if (mayHaveThis) {
      // TODO: tx64 breaks apart the cases of (isMethod && !isStatic)
      // and isPseudoMain.
      m_tb.genDecRefThis();
    }

  } else {

    // Emit call to frame_free_locals / frame_free_locals_no_this helper
    // to free locals and This (if needed)
    if (mayHaveThis) {
      m_tb.genDecRefLocalsThis(curFunc->numLocals());
    } else {
      m_tb.genDecRefLocals(curFunc->numLocals());
    }
  }

  // Pass the return value to caller, free ActRec, and return control to caller
  SSATmp* retAddr = m_tb.genLdRetAddr();
  SSATmp* sp = m_tb.genRetVal(retVal);  // updates sp
  SSATmp* fp = m_tb.genFreeActRec();    // updates fp
  m_tb.genRetCtrl(sp, fp, retAddr);

  // Flag that this trace has a Ret instruction, so that no ExitTrace is needed
  this->m_hasRet = true;
}

void HhbcTranslator::emitRetC() {
  Trace* exit = getExitSlowTrace();
  emitRet(popC(), exit);
}

void HhbcTranslator::emitRetV() {
  Trace* exit = getExitSlowTrace();
  emitRet(popV(), exit);
}

void HhbcTranslator::setThisAvailable() {
  m_tb.setThisAvailable();
}

Trace* HhbcTranslator::guardTypeLocal(uint32 localIndex,
                                      Type::Tag type,
                                      Trace* nextTrace) {
  if (nextTrace == NULL) {
    nextTrace = getGuardExit();
  }
  m_tb.genLdLoc(localIndex, type, nextTrace);
  return nextTrace;
}

// Similar to guardTypeLocal, but this takes a exit instead of
// a guard exit.
void HhbcTranslator::checkTypeLocal(uint32 localIndex, Type::Tag type) {
  Trace* exitTrace = getExitTrace(m_bcOff);
  m_tb.killLocalValue(localIndex);
  m_tb.genLdLoc(localIndex, type, exitTrace);
}

void HhbcTranslator::assertTypeLocal(uint32 localIndex, Type::Tag type) {
  m_tb.genLdLoc(localIndex, type, NULL);
}

void HhbcTranslator::checkTypeStackAux(uint32 stackIndex,
                                       Type::Tag type,
                                       Trace* nextTrace) {
  ASSERT(stackIndex != (uint32)-1);
  SSATmp* tmp = m_evalStack.top(stackIndex);
  if (!tmp) {
    extendStack(stackIndex, type, nextTrace);
    tmp = m_evalStack.top(stackIndex);
  }
  ASSERT(tmp);
  tmp = m_tb.genGuardType(tmp, type, nextTrace);
  replace(stackIndex, tmp);
}


Trace* HhbcTranslator::guardTypeStack(uint32 stackIndex,
                                      Type::Tag type,
                                      Trace* nextTrace) {
  if (nextTrace == NULL) {
    nextTrace = getGuardExit();
  }
  checkTypeStackAux(stackIndex, type, nextTrace);
  return nextTrace;
}

void HhbcTranslator::checkTypeStack(uint32 stackIndex,
                                    Type::Tag type,
                                    Offset nextByteCode) {
  Trace* exitTrace = getExitTrace(nextByteCode);
  checkTypeStackAux(stackIndex, type, exitTrace);
}

void HhbcTranslator::assertTypeStack(uint32 stackIndex, Type::Tag type) {
  ASSERT(stackIndex != (uint32)-1);
  // top() generates the LdStack if necessary, and sets 'type' accordingly
  SSATmp* tmp = top(type, stackIndex);
  replace(stackIndex, tmp);
}

Trace* HhbcTranslator::guardRefs(int64               entryArDelta,
                                 const vector<bool>& mask,
                                 const vector<bool>& vals,
                                 Trace*              exitTrace) {
  if (exitTrace == NULL) {
    exitTrace = getGuardExit();
  }

  int32_t actRecOff = cellsToBytes(entryArDelta);
  SSATmp* funcPtr = m_tb.genLdARFuncPtr(m_tb.getSp(),
                                        m_tb.genDefConst<int64>(actRecOff));
  SSATmp* nParams =
    m_tb.genLdRaw(funcPtr, m_tb.genDefConst<int64>(Func::numParamsOff()),
                  Type::Int);
  SSATmp* bitsPtr =
    m_tb.genLdRaw(funcPtr, m_tb.genDefConst<int64>(Func::refBitVecOff()),
                  Type::Int);

  for (unsigned i = 0; i < mask.size(); i += 64) {
    ASSERT(i < vals.size());

    uint64_t mask64 = TranslatorX64::packBitVec(mask, i);
    if (mask64 == 0) {
      continue;
    }

    uint64_t vals64 = TranslatorX64::packBitVec(vals, i);

    SSATmp* mask64Tmp   = m_tb.genLdConst<int64>(mask64);
    SSATmp* vals64Tmp   = m_tb.genLdConst<int64>(vals64);
    SSATmp* firstBitNum = m_tb.genDefConst<int64>(i);

    m_tb.genGuardRefs(funcPtr, nParams, bitsPtr, firstBitNum,
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

  ASSERT(!constraintClsName || constraintClsName->isStatic());
  const Func* func = getCurFunc();
  const Class* constraint = (constraintClsName ?
                             Unit::lookupClass(constraintClsName) : NULL);
  const TypeConstraint& tc = func->params()[paramId].typeConstraint();
  const StringData* tcTypeName = tc.typeName();

  // VerifyParamType does not remove the parameter from the stack
  TRACE(3, "%u: VerifyParamType %s\n", m_bcOff, tcTypeName->data());
  Trace* exitTrace1 = getExitSlowTrace();
  Trace* exitTrace2 = getExitSlowTrace();
  // XXX Should verify param type unbox?
  SSATmp* param = m_tb.genLdLoc(paramId, Type::Gen, exitTrace1);
  if (param->getType() != Type::Obj) {
    PUNT(VerifyParamType_nonobj);
  }
  SSATmp* objClass = m_tb.genLdObjClass(param);

  if (tc.isObject()) {
    if (!(param->getType() == Type::Obj ||
          (param->getType() == Type::Null && tc.nullable()))) {
      spillStack();
      emitInterpOne(Type::None);
      return;
    }
  } else {
    if (!tc.check(frame_local(curFrame(), paramId), curFunc())) {
      spillStack();
      emitInterpOne(Type::None);
      return;
    }
  }

  m_tb.genVerifyParamType(objClass,
                          m_tb.genDefConst<const StringData*>(tcTypeName),
                          constraint,
                          exitTrace2);
}

void HhbcTranslator::emitInstanceOfD(int classNameStrId) {
  const StringData* className = lookupStringId(classNameStrId);
  TRACE(3, "%u: InstanceOfD %s\n", m_bcOff, className->data());
  SSATmp* src = popC();
  push(m_tb.genInstanceOfD(src,
                           m_tb.genDefConst<const StringData*>(className)));
  m_tb.genDecRef(src);
}

void HhbcTranslator::emitCastBool() {
  SSATmp* src = popC();
  push(m_tb.genConvToBool(src));
  m_tb.genDecRef(src);
}

void HhbcTranslator::emitCastInt() {
  SSATmp* src = popC();
  push(m_tb.genConvToInt(src));
  m_tb.genDecRef(src);
}

void HhbcTranslator::emitCastDouble() {
  SSATmp* src = popC();
  push(m_tb.genConvToDbl(src));
  m_tb.genDecRef(src);
}

// TODO for emitCastString, castAray, and castObject, double check that the
// helpers already don't incref their return values
void HhbcTranslator::emitCastString() {
  SSATmp* src = popC();
  Type::Tag fromType = src->getType();
  if (fromType == Type::Cell) {
    PUNT(CastString);
  } else if (fromType == Type::Obj) {
    // call the toString helper on object
    PUNT(CastString);
  } else {
    // for int to string conversion, this calls a helper that returns
    // a string with ref count of 0.
    pushIncRef(m_tb.genConvToStr(src));
  }
  m_tb.genDecRef(src);
}

void HhbcTranslator::emitCastArray() {
  SSATmp* src = popC();
  pushIncRef(m_tb.genConvToArr(src));
  m_tb.genDecRef(src);
}

void HhbcTranslator::emitCastObject() {
  SSATmp* src = popC();
  pushIncRef(m_tb.genConvToObj(src));
  m_tb.genDecRef(src);
}

static
bool isSupportedAGet(SSATmp* classSrc, const StringData* className) {
  Type::Tag srcType = classSrc->getType();
  return (srcType == Type::Obj ||
          className != NULL ||
          // TODO: Remove this isConst() check once the code gen
          // supports helper call for non-constant strings
          (Type::isString(srcType) && classSrc->isConst()));
}

void HhbcTranslator::emitAGet(SSATmp* classSrc) {
  Type::Tag srcType = classSrc->getType();
  if (Type::isString(srcType)) {
    push(m_tb.genLdCls(classSrc));
  } else if (srcType == Type::Obj) {
    push(m_tb.genLdObjClass(classSrc));
  } else {
    ASSERT(0);
  }
}

void HhbcTranslator::emitAGetC(const StringData* clsName) {
  if (isSupportedAGet(topC(), clsName)) {
    SSATmp* src = popC();
    if (clsName != NULL) {
      src = m_tb.genDefConst<const StringData*>(clsName);
    }
    emitAGet(src);
    m_tb.genDecRef(src);
  } else {
    spillStack();
    popC();
    emitInterpOne(Type::ClassRef);
  }
}

void HhbcTranslator::emitAGetL(int id, const StringData* clsName) {
  Trace* exitTrace = getExitSlowTrace();
  SSATmp* src = m_tb.genLdLoc(id, Type::Cell, exitTrace);
  if (isSupportedAGet(src, clsName)) {
    if (clsName != NULL) {
      src = m_tb.genDefConst<const StringData*>(clsName);
    }
    emitAGet(src);
  } else {
    spillStack();
    emitInterpOne(Type::ClassRef);
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
    ASSERT(Type::isStaticallyKnownUnboxed(resultType));
    val = m_tb.genLdMem(propPtr, resultType, NULL);
  } else {
    if (resultType == Type::None) {
      // result type not predicted
      resultType = Type::Cell;
    } else {
      ASSERT(Type::isStaticallyKnownUnboxed(resultType));
    }
    // This code is currently correct, but once we enable type
    // prediction for CGetS, we should exit normally to a trace that
    // executes the CGetS as cell type (including the incref of the
    // result) and exits to the next bytecode.
    val = m_tb.genLdMem(m_unboxPtrs ? m_tb.genUnboxPtr(propPtr) : propPtr,
                        resultType,
                        exitTrace);
  }
  pushIncRef(val);
  decRefPropAddr(propPtr);
}

// TODO
void HhbcTranslator::emitVGetS() {
  TRACE(3, "%u: VGetS\n", m_bcOff);
  SSATmp* propAddr = getClsPropAddr(NULL);
// TODO
//  push(m_tb.genLdMem());
  PUNT(VGetS);
  decRefPropAddr(propAddr);
}

void HhbcTranslator::emitCGetG(const StringData* name,
                               Type::Tag resultType, bool isInferedType) {
  spillStack();
  popC();
  if (isInferedType) {
    ASSERT(Type::isUnboxed(resultType) &&
           resultType != Type::Cell &&
           resultType != Type::None);
  } else {
    ASSERT(resultType == Type::None); // Type prediction shouldn't happen
    resultType = Type::Cell;
  }
  // TODO: Consider type inference once we support CGetG translation
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
  SSATmp* propPtr = m_unboxPtrs ? m_tb.genUnboxPtr(propAddrToDecRef)
                                : propAddrToDecRef;
  SSATmp* prevValue = m_tb.genLdMem(propPtr, Type::Cell, exitTrace);
  pushIncRef(src);
  m_tb.genStMem(propPtr, src, true);
  m_tb.genDecRef(prevValue);
  decRefPropAddr(propAddrToDecRef);
}

void HhbcTranslator::emitBinaryArith(Opcode opc, bool isBitOp /* = false */) {
  Type::Tag type1 = topC(0)->getType();
  Type::Tag type2 = topC(1)->getType();
  if ((type1 == Type::Int || type1 == Type::Bool) &&
      (type2 == Type::Int || type2 == Type::Bool)) {
    SSATmp* tr = popC();
    SSATmp* tl = popC();
    push(m_tb.genIntegerOp(opc, tl, tr));
  } else if (isBitOp && (type1 == Type::Obj || type1 == Type::Obj)) {
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
  push(m_tb.genNot(src));
  m_tb.genDecRef(src);
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
    SSATmp* ones = m_tb.genDefConst<int64>(~0);
    push(m_tb.genXor(src, ones));
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
  emitBinaryArith(OpAnd, true);
}
void HhbcTranslator::emitBitOr() {
  TRACE(3, "%u: BitOr\n", m_bcOff);
  emitBinaryArith(OpOr, true);
}
void HhbcTranslator::emitBitXor() {
  TRACE(3, "%u: BitXor\n", m_bcOff);
  emitBinaryArith(OpXor, true);
}
void HhbcTranslator::emitXor() {
  TRACE(3, "%u: Xor\n", m_bcOff);
  SSATmp* btr = popC();
  SSATmp* btl = popC();
  SSATmp* tr = m_tb.genConvToBool(btr);
  SSATmp* tl = m_tb.genConvToBool(btl);
  push(m_tb.genConvToBool(m_tb.genXor(tl, tr)));
  m_tb.genDecRef(btl);
  m_tb.genDecRef(btr);
}

void HhbcTranslator::emitInterpOne(Type::Tag type, Trace* target /* = NULL */) {
  if (0) {
    m_tb.genTraceEnd(m_bcOff, TraceExitType::SlowNoProgress);
    // TODO need to push something on the stack...
    // Better approach here is to modify the code gen to generate an exit
    // when it sees interp one
  } else {
    m_tb.genInterpOne(m_bcOff, m_stackDeficit, type, target);
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
  if (!m_firstBcOff) {
    // we're in the middle of a trace, so just return a trace exit
    return getExitTrace(m_bcOff);
  }
  // stack better be empty since we're at the start of the trace
  ASSERT((m_evalStack.numElems() - m_stackDeficit) == 0);
  return m_tb.getExitGuardFailureTrace();
}

Trace* HhbcTranslator::getExitSlowTrace(Offset nextByteCode /* = -1 */) {
  uint32 numStackElems = m_evalStack.numElems();
  SSATmp* stackValues[numStackElems];
  for (uint32 i = 0; i < numStackElems; i++) {
    stackValues[i] = m_evalStack.top(i);
  }
  return m_tb.getExitSlowTrace(nextByteCode == -1 ? m_bcOff : nextByteCode,
                               m_stackDeficit,
                               numStackElems,
                               numStackElems ? stackValues : 0);
}

// generates a trace that can be a target of a control flow instruction
// at the current bytecode offset
Trace* HhbcTranslator::getExitTrace(uint32 targetBcOff) {
  uint32 numStackElems = m_evalStack.numElems();
  SSATmp* stackValues[numStackElems];
  for (uint32 i = 0; i < numStackElems; i++) {
    stackValues[i] = m_evalStack.top(i);
  }
  return m_tb.genExitTrace(targetBcOff,
                           m_stackDeficit,
                           numStackElems,
                           numStackElems ? stackValues :
                                           (SSATmp**) NULL,
                           TraceExitType::Normal);
}

// generates a trace exit that can be the target of a conditional
// control flow instruction at the current bytecode offset
Trace* HhbcTranslator::getExitTrace(uint32 targetBcOff, uint32 notTakenBcOff) {
  uint32 numStackElems = m_evalStack.numElems();
  SSATmp* stackValues[numStackElems];
  for (uint32 i = 0; i < numStackElems; i++) {
    stackValues[i] = m_evalStack.top(i);
  }
  return m_tb.genExitTrace(targetBcOff,
                           m_stackDeficit,
                           numStackElems,
                           numStackElems ? stackValues :
                                           (SSATmp**) NULL,
                           TraceExitType::NormalCc,
                           notTakenBcOff);
}

SSATmp* HhbcTranslator::spillStack(bool allocActRec) {
  uint32 numStackElems = m_evalStack.numElems();
  SSATmp* stackValues[numStackElems];
  for (uint32 i = 0; i < numStackElems; i++) {
    stackValues[i] = m_evalStack.pop();
  }
  SSATmp* curSp = m_tb.getSp();

  // remove spilled values that are already on the stack
  // at the right offset
  if (false) {
    for (int32 i = numStackElems - 1; i >= 0; i--) {
      IRInstruction* inst = stackValues[i]->getInstruction();
      if (inst->getOpcode() != LdStack) {
        break;
      }
      // first check that the ldstack uses the same sp as the current
      // sp value
      if (inst->getSrc(0) != curSp) {
        break;
      }
      // now check the stack offset
      int64 stackOffset = inst->getSrc(1)->getConstValAsInt();
      if (allocActRec) {
        // if there is an AR on the stack, then we need to adjust
        // the stack offset by its size
        stackOffset -= (sizeof(ActRec) / sizeof(Cell));
      }
      if (stackOffset != i) {
        break;
      }
      // XXX it's better not to eliminate these LdStack here but rather
      // not generate code for them and not allocate registers to their
      // ldstacks src (by not incrementing their use counts). This is
      // because getStackValue uses these SpillStack instructions to
      // propagate type and value information.  When checking to see if
      // the stack offsets are the same, we should also chase down
      // increfs.

      // remove the operand by decrementing the number of operands
      numStackElems--;
      m_stackDeficit--; // compensate
    }
  }

  SSATmp* sp = m_tb.genSpillStack(m_stackDeficit,
                                  numStackElems,
                                  numStackElems ? stackValues : 0,
                                  allocActRec);
  m_stackDeficit = 0;
  return sp;
}

SSATmp* HhbcTranslator::loadStackAddr(int32 offset) {
  // You're almost certainly doing it wrong if you want to get the address of a
  // stack cell that's in m_evalStack.
  ASSERT(offset >= (int32)m_evalStack.numElems());
  return m_tb.genLdStackAddr(offset + m_stackDeficit - m_evalStack.numElems());
}

//
// This is a wrapper to TraceBuilder::genLdLoc() that also emits the
// RaiseUninitWarning if the local is uninitialized
//
SSATmp* HhbcTranslator::emitLdLocWarn(uint32 id,
                                      Type::Tag type,
                                      Trace* target) {
  SSATmp* locVal = m_tb.genLdLoc(id, type, target);

  if (locVal->getType() == Type::Uninit) {
    spillStack();
    m_tb.genRaiseUninitWarning(id);
    return m_tb.genDefNull();
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
  m_tb.genTraceEnd(nextPc);
}

}}} // namespace HPHP::VM::JIT
