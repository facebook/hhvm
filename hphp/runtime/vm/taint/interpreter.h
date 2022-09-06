/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
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

#pragma once

#ifdef HHVM_TAINT

#include "hphp/runtime/base/tv-val.h"
#include "hphp/runtime/vm/bytecode.h"
#include "hphp/runtime/vm/hhbc.h"
#include "hphp/runtime/vm/jit/types.h"

namespace HPHP {
namespace taint {

using jit::TCA;

void iopNop();
void iopBreakTraceHint();
void iopPopC();
void iopPopU();
void iopPopU2();
void iopPopL(tv_lval to);
void iopDup();
void iopCGetCUNop();
void iopUGetCUNop();
void iopNull();
void iopNullUninit();
void iopTrue();
void iopFalse();
void iopFuncCred();
void iopInt(int64_t imm);
void iopDouble(double imm);
void iopString(const StringData* s);
void iopDict(const ArrayData* a);
void iopKeyset(const ArrayData* a);
void iopVec(const ArrayData* a);
void iopNewDictArray(uint32_t capacity);
void iopNewStructDict(imm_array<int32_t> ids);
void iopNewVec(uint32_t n);
void iopNewKeysetArray(uint32_t n);
void iopAddElemC();
void iopAddNewElemC();
void iopNewCol(CollectionType cType);
void iopNewPair();
void iopColFromArray(CollectionType cType);
void iopCnsE(const StringData* s);
void iopClsCns(const StringData* clsCnsName);
void iopClsCnsD(const StringData* clsCnsName, Id classId);
void iopClsCnsL(tv_lval local);
void iopClassName();
void iopLazyClassFromClass();
void iopFile();
void iopDir();
void iopMethod();
void iopConcat();
void iopConcatN(uint32_t n);
void iopAdd();
void iopSub();
void iopMul();
void iopAddO();
void iopSubO();
void iopMulO();
void iopDiv();
void iopMod();
void iopPow();
void iopNot();
void iopSame();
void iopNSame();
void iopEq();
void iopNeq();
void iopLt();
void iopLte();
void iopGt();
void iopGte();
void iopCmp();
void iopBitAnd();
void iopBitOr();
void iopBitXor();
void iopBitNot();
void iopShl();
void iopShr();
void iopCastBool();
void iopCastInt();
void iopCastDouble();
void iopCastString();
void iopCastDict();
void iopCastKeyset();
void iopCastVec();
void iopDblAsBits();
void iopInstanceOf();
void iopInstanceOfD(Id id);
void iopIsLateBoundCls();
void iopIsTypeStructC(TypeStructResolveOp op);
void iopThrowAsTypeStructException();
void iopCombineAndResolveTypeStruct(uint32_t n);
void iopSelect();
void iopPrint();
void iopClone();
void iopExit();
void iopFatal(FatalOp kind_char);
void iopEnter(PC& pc, PC targetpc);
void iopJmp(PC& pc, PC targetpc);
void iopJmpZ(PC& pc, PC targetpc);
void iopJmpNZ(PC& pc, PC targetpc);
void iopSwitch(
    PC origpc,
    PC& pc,
    SwitchKind kind,
    int64_t base,
    imm_array<Offset> jmptab);
void iopSSwitch(PC origpc, PC& pc, imm_array<StrVecItem> jmptab);
void iopRetC(PC& pc);
void iopRetM(PC& pc, uint32_t numRet);
void iopRetCSuspended(PC& pc);
void iopThrow(PC& pc);
void iopCGetL(named_local_var fr);
void iopCGetQuietL(tv_lval fr);
void iopCUGetL(tv_lval fr);
void iopCGetL2(named_local_var fr);
void iopPushL(tv_lval locVal);
void iopCGetG();
void iopCGetS(ReadonlyOp op);
void iopClassGetC();
void iopClassGetTS();
void iopGetMemoKeyL(named_local_var loc);
void iopAKExists();
void iopIssetL(tv_lval val);
void iopIssetG();
void iopIssetS();
void iopIsUnsetL(tv_lval val);
void iopIsTypeC(IsTypeOp op);
void iopIsTypeL(named_local_var loc, IsTypeOp op);
void iopAssertRATL(local_var loc, RepoAuthType rat);
void iopAssertRATStk(uint32_t stkSlot, RepoAuthType rat);
void iopSetL(tv_lval to);
void iopSetG();
void iopSetS(ReadonlyOp op);
void iopSetOpL(tv_lval to, SetOpOp op);
void iopSetOpG(SetOpOp op);
void iopSetOpS(SetOpOp op);
void iopIncDecL(named_local_var fr, IncDecOp op);
void iopIncDecG(IncDecOp op);
void iopIncDecS(IncDecOp op);
void iopUnsetL(tv_lval loc);
void iopUnsetG();
void iopResolveFunc(Id id);
void iopResolveMethCaller(Id id);
void iopResolveRFunc(Id id);
void iopResolveClsMethod(const StringData* methName);
void iopResolveClsMethodD(Id classId, const StringData* methName);
void iopResolveClsMethodS(SpecialClsRef ref, const StringData* methName);
void iopResolveRClsMethod(const StringData* methName);
void iopResolveRClsMethodD(Id classId, const StringData* methName);
void iopResolveRClsMethodS(SpecialClsRef ref, const StringData* methName);
void iopResolveClass(Id id);
void iopLazyClass(Id id);
void iopNewObj();
void iopNewObjD(Id id);
void iopNewObjS(SpecialClsRef ref);
void iopLockObj();
TCA iopFCallClsMethod(
    bool retToJit,
    PC origpc,
    PC& pc,
    const FCallArgs& fca,
    const StringData*,
    IsLogAsDynamicCallOp op);
TCA iopFCallClsMethodD(
    bool retToJit,
    PC origpc,
    PC& pc,
    const FCallArgs& fca,
    const StringData*,
    Id classId,
    const StringData* methName);
TCA iopFCallClsMethodS(
    bool retToJit,
    PC origpc,
    PC& pc,
    const FCallArgs& fca,
    const StringData*,
    SpecialClsRef ref);
TCA iopFCallClsMethodSD(
    bool retToJit,
    PC origpc,
    PC& pc,
    const FCallArgs& fca,
    const StringData*,
    SpecialClsRef ref,
    const StringData* methName);
TCA iopFCallCtor(
    bool retToJit,
    PC origpc,
    PC& pc,
    const FCallArgs& fca,
    const StringData*);
TCA iopFCallFunc(bool retToJit, PC origpc, PC& pc, const FCallArgs& fca);
TCA iopFCallFuncD(
    bool retToJit,
    PC origpc,
    PC& pc,
    const FCallArgs& fca,
    Id id);
TCA iopFCallObjMethod(
    bool retToJit,
    PC origpc,
    PC& pc,
    const FCallArgs& fca,
    const StringData* name,
    ObjMethodOp op);
TCA iopFCallObjMethodD(
    bool retToJit,
    PC origpc,
    PC& pc,
    const FCallArgs& fca,
    const StringData*,
    ObjMethodOp op,
    const StringData* name);
void iopIterInit(PC& pc, const IterArgs& ita, PC targetpc);
void iopLIterInit(PC& pc, const IterArgs& ita, TypedValue* base, PC targetpc);
void iopIterNext(PC& pc, const IterArgs& ita, PC targetpc);
void iopLIterNext(PC& pc, const IterArgs& ita, TypedValue* base, PC targetpc);
void iopIterFree(Iter* it);
void iopLIterFree(Iter* it, tv_lval);
void iopIncl();
void iopInclOnce();
void iopReq();
void iopReqOnce();
void iopReqDoc();
void iopEval();
void iopThis();
void iopBareThis(BareThisOp bto);
void iopCheckThis();
void iopChainFaults();
void iopOODeclExists(OODeclExistsOp subop);
void iopVerifyImplicitContextState();
void iopVerifyOutType(uint32_t paramId);
void iopVerifyParamType(local_var param);
void iopVerifyParamTypeTS(local_var param);
void iopVerifyRetTypeC();
void iopVerifyRetTypeTS();
void iopVerifyRetNonNullC();
void iopSelfCls();
void iopParentCls();
void iopLateBoundCls();
void iopRecordReifiedGeneric();
void iopClassHasReifiedGenerics();
void iopGetClsRGProp();
void iopCheckClsReifiedGenericMismatch();
void iopHasReifiedParent();
void iopCheckClsRGSoft();
void iopNativeImpl(PC& pc);
void iopCreateCl(uint32_t numArgs, uint32_t clsIx);
void iopCreateCont(PC origpc, PC& pc);
void iopContEnter(PC origpc, PC& pc);
void iopContRaise(PC origpc, PC& pc);
void iopYield(PC origpc, PC& pc);
void iopYieldK(PC origpc, PC& pc);
void iopContCheck(ContCheckOp subop);
void iopContValid();
void iopContKey();
void iopContCurrent();
void iopContGetReturn();
void iopWHResult();
void iopAwait(PC origpc, PC& pc);
void iopAwaitAll(PC origpc, PC& pc, LocalRange locals);
void iopIdx();
void iopArrayIdx();
void iopArrayMarkLegacy();
void iopArrayUnmarkLegacy();
void iopCheckProp(const StringData* propName);
void iopSetImplicitContextByValue();
void iopInitProp(const StringData* propName, InitPropOp propOp);
void iopSilence(tv_lval loc, SilenceOp subop);
void iopThrowNonExhaustiveSwitch();
void iopRaiseClassStringConversionWarning();
void iopBaseGC(uint32_t idx, MOpMode mode);
void iopBaseGL(tv_lval loc, MOpMode mode);
void iopBaseSC(uint32_t keyIdx, uint32_t clsIdx, MOpMode mode, ReadonlyOp op);
void iopBaseL(named_local_var loc, MOpMode mode, ReadonlyOp op);
void iopBaseC(uint32_t idx, MOpMode);
void iopBaseH();
void iopDim(MOpMode mode, MemberKey mk);
void iopQueryM(uint32_t nDiscard, QueryMOp subop, MemberKey mk);
void iopSetM(uint32_t nDiscard, MemberKey mk);
void iopSetRangeM(uint32_t nDiscard, uint32_t size, SetRangeOp op);
void iopIncDecM(uint32_t nDiscard, IncDecOp subop, MemberKey mk);
void iopSetOpM(uint32_t nDiscard, SetOpOp subop, MemberKey mk);
void iopUnsetM(uint32_t nDiscard, MemberKey mk);
void iopMemoGet(PC& pc, PC notfound, LocalRange keys);
void iopMemoGetEager(PC& pc, PC notfound, PC suspended, LocalRange keys);
void iopMemoSet(LocalRange keys);
void iopMemoSetEager(LocalRange keys);

} // namespace taint
} // namespace HPHP

#endif
