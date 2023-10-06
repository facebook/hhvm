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

/* The OPCODES macro is used in many places across HHVM and HackC. Keeping it
 * in this file allows us to keep includes lightweight.
 */

namespace HPHP {

//  name             immediates        inputs           outputs     flags
#define OPCODES \
  O(Nop,             NA,               NOV,             NOV,        NF) \
  O(BreakTraceHint,  NA,               NOV,             NOV,        NF) \
  O(PopC,            NA,               ONE(CV),         NOV,        NF) \
  O(PopU,            NA,               ONE(UV),         NOV,        NF) \
  O(PopU2,           NA,               TWO(CV,UV),      ONE(CV),    NF) \
  O(PopL,            ONE(LA),          ONE(CV),         NOV,        NF) \
  O(Dup,             NA,               ONE(CV),         TWO(CV,CV), NF) \
  O(CGetCUNop,       NA,               ONE(CUV),        ONE(CV),    NF) \
  O(UGetCUNop,       NA,               ONE(CUV),        ONE(UV),    NF) \
  O(Null,            NA,               NOV,             ONE(CV),    NF) \
  O(NullUninit,      NA,               NOV,             ONE(UV),    NF) \
  O(True,            NA,               NOV,             ONE(CV),    NF) \
  O(False,           NA,               NOV,             ONE(CV),    NF) \
  O(FuncCred,        NA,               NOV,             ONE(CV),    NF) \
  O(Int,             ONE(I64A),        NOV,             ONE(CV),    NF) \
  O(Double,          ONE(DA),          NOV,             ONE(CV),    NF) \
  O(String,          ONE(SA),          NOV,             ONE(CV),    NF) \
  O(Dict,            ONE(AA),          NOV,             ONE(CV),    NF) \
  O(Keyset,          ONE(AA),          NOV,             ONE(CV),    NF) \
  O(Vec,             ONE(AA),          NOV,             ONE(CV),    NF) \
  O(NewDictArray,    ONE(IVA),         NOV,             ONE(CV),    NF) \
  O(NewStructDict,   ONE(VSA),         SMANY,           ONE(CV),    NF) \
  O(NewVec,          ONE(IVA),         CMANY,           ONE(CV),    NF) \
  O(NewKeysetArray,  ONE(IVA),         CMANY,           ONE(CV),    NF) \
  O(AddElemC,        NA,               THREE(CV,CV,CV), ONE(CV),    NF) \
  O(AddNewElemC,     NA,               TWO(CV,CV),      ONE(CV),    NF) \
  O(NewCol,          ONE(OA(CollectionType)),                           \
                                       NOV,             ONE(CV),    NF) \
  O(NewPair,         NA,               TWO(CV,CV),      ONE(CV),    NF) \
  O(ColFromArray,    ONE(OA(CollectionType)),                           \
                                       ONE(CV),         ONE(CV),    NF) \
  O(CnsE,            ONE(SA),          NOV,             ONE(CV),    NF) \
  O(ClsCns,          ONE(SA),          ONE(CV),         ONE(CV),    NF) \
  O(ClsCnsD,         TWO(SA,SA),       NOV,             ONE(CV),    NF) \
  O(ClsCnsL,         ONE(LA),          ONE(CV),         ONE(CV),    NF) \
  O(ClassName,       NA,               ONE(CV),         ONE(CV),    NF) \
  O(LazyClassFromClass, NA,            ONE(CV),         ONE(CV),    NF) \
  O(EnumClassLabelName, NA,            ONE(CV),         ONE(CV),    NF) \
  O(File,            NA,               NOV,             ONE(CV),    NF) \
  O(Dir,             NA,               NOV,             ONE(CV),    NF) \
  O(Method,          NA,               NOV,             ONE(CV),    NF) \
  O(Concat,          NA,               TWO(CV,CV),      ONE(CV),    NF) \
  O(ConcatN,         ONE(IVA),         CMANY,           ONE(CV),    NF) \
  O(Add,             NA,               TWO(CV,CV),      ONE(CV),    NF) \
  O(Sub,             NA,               TWO(CV,CV),      ONE(CV),    NF) \
  O(Mul,             NA,               TWO(CV,CV),      ONE(CV),    NF) \
  O(Div,             NA,               TWO(CV,CV),      ONE(CV),    NF) \
  O(Mod,             NA,               TWO(CV,CV),      ONE(CV),    NF) \
  O(Pow,             NA,               TWO(CV,CV),      ONE(CV),    NF) \
  O(Not,             NA,               ONE(CV),         ONE(CV),    NF) \
  O(Same,            NA,               TWO(CV,CV),      ONE(CV),    NF) \
  O(NSame,           NA,               TWO(CV,CV),      ONE(CV),    NF) \
  O(Eq,              NA,               TWO(CV,CV),      ONE(CV),    NF) \
  O(Neq,             NA,               TWO(CV,CV),      ONE(CV),    NF) \
  O(Lt,              NA,               TWO(CV,CV),      ONE(CV),    NF) \
  O(Lte,             NA,               TWO(CV,CV),      ONE(CV),    NF) \
  O(Gt,              NA,               TWO(CV,CV),      ONE(CV),    NF) \
  O(Gte,             NA,               TWO(CV,CV),      ONE(CV),    NF) \
  O(Cmp,             NA,               TWO(CV,CV),      ONE(CV),    NF) \
  O(BitAnd,          NA,               TWO(CV,CV),      ONE(CV),    NF) \
  O(BitOr,           NA,               TWO(CV,CV),      ONE(CV),    NF) \
  O(BitXor,          NA,               TWO(CV,CV),      ONE(CV),    NF) \
  O(BitNot,          NA,               ONE(CV),         ONE(CV),    NF) \
  O(Shl,             NA,               TWO(CV,CV),      ONE(CV),    NF) \
  O(Shr,             NA,               TWO(CV,CV),      ONE(CV),    NF) \
  O(CastBool,        NA,               ONE(CV),         ONE(CV),    NF) \
  O(CastInt,         NA,               ONE(CV),         ONE(CV),    NF) \
  O(CastDouble,      NA,               ONE(CV),         ONE(CV),    NF) \
  O(CastString,      NA,               ONE(CV),         ONE(CV),    NF) \
  O(CastDict,        NA,               ONE(CV),         ONE(CV),    NF) \
  O(CastKeyset,      NA,               ONE(CV),         ONE(CV),    NF) \
  O(CastVec,         NA,               ONE(CV),         ONE(CV),    NF) \
  O(DblAsBits,       NA,               ONE(CV),         ONE(CV),    NF) \
  O(InstanceOf,      NA,               TWO(CV,CV),      ONE(CV),    NF) \
  O(InstanceOfD,     ONE(SA),          ONE(CV),         ONE(CV),    NF) \
  O(IsLateBoundCls,  NA,               ONE(CV),         ONE(CV),    NF) \
  O(IsTypeStructC,   ONE(OA(TypeStructResolveOp)),                      \
                                       TWO(CV,CV),      ONE(CV),    NF) \
  O(ThrowAsTypeStructException,                                         \
                     NA,               TWO(CV,CV),      NOV,        TF) \
  O(CombineAndResolveTypeStruct,                                        \
                     ONE(IVA),         CMANY,           ONE(CV),    NF) \
  O(Select,          NA,               THREE(CV,CV,CV), ONE(CV),    NF) \
  O(Print,           NA,               ONE(CV),         ONE(CV),    NF) \
  O(Clone,           NA,               ONE(CV),         ONE(CV),    NF) \
  O(Exit,            NA,               ONE(CV),         ONE(CV),    TF) \
  O(Fatal,           ONE(OA(FatalOp)), ONE(CV),         NOV,        TF) \
  O(Enter,           ONE(BA),          NOV,             NOV,        CF_TF) \
  O(Jmp,             ONE(BA),          NOV,             NOV,        CF_TF) \
  O(JmpZ,            ONE(BA),          ONE(CV),         NOV,        CF) \
  O(JmpNZ,           ONE(BA),          ONE(CV),         NOV,        CF) \
  O(Switch,          THREE(OA(SwitchKind),I64A,BLA),                    \
                                       ONE(CV),         NOV,        CF_TF) \
  O(SSwitch,         ONE(SLA),         ONE(CV),         NOV,        CF_TF) \
  O(RetC,            NA,               ONE(CV),         NOV,        CF_TF) \
  O(RetM,            ONE(IVA),         CMANY,           NOV,        CF_TF) \
  O(RetCSuspended,   NA,               ONE(CV),         NOV,        CF_TF) \
  O(Throw,           NA,               ONE(CV),         NOV,        CF_TF) \
  O(CGetL,           ONE(NLA),         NOV,             ONE(CV),    NF) \
  O(CGetQuietL,      ONE(LA),          NOV,             ONE(CV),    NF) \
  O(CUGetL,          ONE(LA),          NOV,             ONE(CUV),   NF) \
  O(CGetL2,          ONE(NLA),         ONE(CV),         TWO(CV,CV), NF) \
  O(PushL,           ONE(LA),          NOV,             ONE(CV),    NF) \
  O(CGetG,           NA,               ONE(CV),         ONE(CV),    NF) \
  O(CGetS,           ONE(OA(ReadonlyOp)),                               \
                     TWO(CV,CV),      ONE(CV),    NF) \
  O(ClassGetC,       NA,               ONE(CV),         ONE(CV),    NF) \
  O(ClassGetTS,      NA,               ONE(CV),         TWO(CV,CV), NF) \
  O(GetMemoKeyL,     ONE(NLA),         NOV,             ONE(CV),    NF) \
  O(AKExists,        NA,               TWO(CV,CV),      ONE(CV),    NF) \
  O(IssetL,          ONE(LA),          NOV,             ONE(CV),    NF) \
  O(IssetG,          NA,               ONE(CV),         ONE(CV),    NF) \
  O(IssetS,          NA,               TWO(CV,CV),      ONE(CV),    NF) \
  O(IsUnsetL,        ONE(LA),          NOV,             ONE(CV),    NF) \
  O(IsTypeC,         ONE(OA(IsTypeOp)),ONE(CV),         ONE(CV),    NF) \
  O(IsTypeL,         TWO(NLA,                                           \
                       OA(IsTypeOp)),  NOV,             ONE(CV),    NF) \
  O(AssertRATL,      TWO(ILA,RATA),    NOV,             NOV,        NF) \
  O(AssertRATStk,    TWO(IVA,RATA),    NOV,             NOV,        NF) \
  O(SetL,            ONE(LA),          ONE(CV),         ONE(CV),    NF) \
  O(SetG,            NA,               TWO(CV,CV),      ONE(CV),    NF) \
  O(SetS,            ONE(OA(ReadonlyOp)),                               \
                                       THREE(CV,CV,CV), ONE(CV),    NF) \
  O(SetOpL,          TWO(LA,                                            \
                       OA(SetOpOp)),   ONE(CV),         ONE(CV),    NF) \
  O(SetOpG,          ONE(OA(SetOpOp)), TWO(CV,CV),      ONE(CV),    NF) \
  O(SetOpS,          ONE(OA(SetOpOp)), THREE(CV,CV,CV), ONE(CV),    NF) \
  O(IncDecL,         TWO(NLA, OA(IncDecOp)),                            \
                                       NOV,             ONE(CV),    NF) \
  O(IncDecG,         ONE(OA(IncDecOp)),ONE(CV),         ONE(CV),    NF) \
  O(IncDecS,         ONE(OA(IncDecOp)),TWO(CV,CV),      ONE(CV),    NF) \
  O(UnsetL,          ONE(LA),          NOV,             NOV,        NF) \
  O(UnsetG,          NA,               ONE(CV),         NOV,        NF) \
                                                                        \
  O(ResolveFunc,     ONE(SA),          NOV,             ONE(CV),    NF) \
  O(ResolveMethCaller,ONE(SA),         NOV,             ONE(CV),    NF) \
  O(ResolveRFunc,    ONE(SA),          ONE(CV),         ONE(CV),    NF) \
  O(ResolveClsMethod,ONE(SA),          ONE(CV),         ONE(CV),    NF) \
  O(ResolveClsMethodD,                                                  \
                     TWO(SA,SA),       NOV,             ONE(CV),    NF) \
  O(ResolveClsMethodS,                                                  \
                     TWO(OA(SpecialClsRef),SA),                         \
                                       NOV,             ONE(CV),    NF) \
  O(ResolveRClsMethod,                                                  \
                     ONE(SA),          TWO(CV,CV),      ONE(CV),    NF) \
  O(ResolveRClsMethodD,                                                 \
                     TWO(SA,SA),       ONE(CV),         ONE(CV),    NF) \
  O(ResolveRClsMethodS,                                                 \
                     TWO(OA(SpecialClsRef),SA),                         \
                                       ONE(CV),         ONE(CV),    NF) \
  O(ResolveClass,    ONE(SA),          NOV,             ONE(CV),    NF) \
  O(LazyClass,       ONE(SA),          NOV,             ONE(CV),    NF) \
  O(EnumClassLabel,  ONE(SA),          NOV,             ONE(CV),    NF) \
  O(NewObj,          NA,               ONE(CV),         ONE(CV),    NF) \
  O(NewObjD,         ONE(SA),          NOV,             ONE(CV),    NF) \
  O(NewObjS,         ONE(OA(SpecialClsRef)),                            \
                                       NOV,             ONE(CV),    NF) \
  O(LockObj,         NA,               ONE(CV),         ONE(CV),    NF) \
  O(FCallClsMethod,  THREE(FCA,SA,OA(IsLogAsDynamicCallOp)),            \
                                       FCALL(2, 0),     FCALL,      CF) \
  O(FCallClsMethodM, FOUR(FCA,SA,OA(IsLogAsDynamicCallOp),SA),          \
                                       FCALL(1, 0),     FCALL,      CF) \
  O(FCallClsMethodD, THREE(FCA,SA,SA), FCALL(0, 0),     FCALL,      CF) \
  O(FCallClsMethodS, THREE(FCA,SA,OA(SpecialClsRef)),                   \
                                       FCALL(1, 0),     FCALL,      CF) \
  O(FCallClsMethodSD,FOUR(FCA,SA,OA(SpecialClsRef),SA),                 \
                                       FCALL(0, 0),     FCALL,      CF) \
  O(FCallCtor,       TWO(FCA,SA),      FCALL(0, 1),     FCALL,      CF) \
  O(FCallFunc,       ONE(FCA),         FCALL(1, 0),     FCALL,      CF) \
  O(FCallFuncD,      TWO(FCA,SA),      FCALL(0, 0),     FCALL,      CF) \
  O(FCallObjMethod,  THREE(FCA,SA,OA(ObjMethodOp)),                     \
                                       FCALL(1, 1),     FCALL,      CF) \
  O(FCallObjMethodD, FOUR(FCA,SA,OA(ObjMethodOp),SA),                   \
                                       FCALL(0, 1),     FCALL,      CF) \
  O(IterInit,        TWO(ITA,BA),      ONE(CV),         NOV,        CF) \
  O(LIterInit,       THREE(ITA,LA,BA), NOV,             NOV,        CF) \
  O(IterNext,        TWO(ITA,BA),      NOV,             NOV,        CF) \
  O(LIterNext,       THREE(ITA,LA,BA), NOV,             NOV,        CF) \
  O(IterFree,        ONE(IA),          NOV,             NOV,        NF) \
  O(LIterFree,       TWO(IA,LA),       NOV,             NOV,        NF) \
  O(Incl,            NA,               ONE(CV),         ONE(CV),    NF) \
  O(InclOnce,        NA,               ONE(CV),         ONE(CV),    NF) \
  O(Req,             NA,               ONE(CV),         ONE(CV),    NF) \
  O(ReqOnce,         NA,               ONE(CV),         ONE(CV),    NF) \
  O(ReqDoc,          NA,               ONE(CV),         ONE(CV),    NF) \
  O(Eval,            NA,               ONE(CV),         ONE(CV),    NF) \
  O(This,            NA,               NOV,             ONE(CV),    NF) \
  O(BareThis,        ONE(OA(BareThisOp)),                               \
                                       NOV,             ONE(CV),    NF) \
  O(CheckThis,       NA,               NOV,             NOV,        NF) \
  O(ChainFaults,     NA,               TWO(CV,CV),      ONE(CV),    NF) \
  O(OODeclExists,    ONE(OA(OODeclExistsOp)),                           \
                                       TWO(CV,CV),      ONE(CV),    NF) \
  O(VerifyOutType,   ONE(ILA),         ONE(CV),         ONE(CV),    NF) \
  O(VerifyParamType, ONE(ILA),         ONE(CV),         ONE(CV),    NF) \
  O(VerifyParamTypeTS, ONE(ILA),       ONE(CV),         NOV,        NF) \
  O(VerifyRetTypeC,  NA,               ONE(CV),         ONE(CV),    NF) \
  O(VerifyRetTypeTS, NA,               TWO(CV,CV),      ONE(CV),    NF) \
  O(VerifyRetNonNullC, NA,             ONE(CV),         ONE(CV),    NF) \
  O(SelfCls,         NA,               NOV,             ONE(CV),    NF) \
  O(ParentCls,       NA,               NOV,             ONE(CV),    NF) \
  O(LateBoundCls,    NA,               NOV,             ONE(CV),    NF) \
  O(RecordReifiedGeneric, NA,          ONE(CV),         ONE(CV),    NF) \
  O(CheckClsReifiedGenericMismatch, NA, ONE(CV),        NOV,        NF) \
  O(ClassHasReifiedGenerics, NA,       ONE(CV),         ONE(CV),    NF) \
  O(GetClsRGProp,    NA,               ONE(CV),         ONE(CV),    NF) \
  O(HasReifiedParent, NA,              ONE(CV),         ONE(CV),    NF) \
  O(CheckClsRGSoft,  NA,               ONE(CV),         NOV,        NF) \
  O(NativeImpl,      NA,               NOV,             NOV,        CF_TF) \
  O(CreateCl,        TWO(IVA,SA),      CUMANY,          ONE(CV),    NF) \
  O(CreateCont,      NA,               NOV,             ONE(CV),    CF) \
  O(ContEnter,       NA,               ONE(CV),         ONE(CV),    CF) \
  O(ContRaise,       NA,               ONE(CV),         ONE(CV),    CF) \
  O(Yield,           NA,               ONE(CV),         ONE(CV),    CF) \
  O(YieldK,          NA,               TWO(CV,CV),      ONE(CV),    CF) \
  O(ContCheck,       ONE(OA(ContCheckOp)), NOV,         NOV,        NF) \
  O(ContValid,       NA,               NOV,             ONE(CV),    NF) \
  O(ContKey,         NA,               NOV,             ONE(CV),    NF) \
  O(ContCurrent,     NA,               NOV,             ONE(CV),    NF) \
  O(ContGetReturn,   NA,               NOV,             ONE(CV),    NF) \
  O(WHResult,        NA,               ONE(CV),         ONE(CV),    NF) \
  O(SetImplicitContextByValue,                                          \
                     NA,               ONE(CV),         ONE(CV),    NF) \
  O(VerifyImplicitContextState,                                         \
                     NA,               NOV,             NOV,        NF) \
  O(CreateSpecialImplicitContext,                                       \
                     NA,               TWO(CV,CV),      ONE(CV),    NF) \
  O(Await,           NA,               ONE(CV),         ONE(CV),    CF) \
  O(AwaitAll,        ONE(LAR),         NOV,             ONE(CV),    CF) \
  O(Idx,             NA,               THREE(CV,CV,CV), ONE(CV),    NF) \
  O(ArrayIdx,        NA,               THREE(CV,CV,CV), ONE(CV),    NF) \
  O(ArrayMarkLegacy,    NA,            TWO(CV,CV),      ONE(CV),    NF) \
  O(ArrayUnmarkLegacy,  NA,            TWO(CV,CV),      ONE(CV),    NF) \
  O(CheckProp,       ONE(SA),          NOV,             ONE(CV),    NF) \
  O(InitProp,        TWO(SA, OA(InitPropOp)),                           \
                                       ONE(CV),         NOV,        NF) \
  O(Silence,         TWO(LA,OA(SilenceOp)),                             \
                                       NOV,             NOV,        NF) \
  O(ThrowNonExhaustiveSwitch, NA,      NOV,             NOV,        NF) \
  O(RaiseClassStringConversionNotice,                                   \
                              NA,      NOV,             NOV,        NF) \
  O(BaseGC,          TWO(IVA, OA(MOpMode)),                             \
                                       NOV,             NOV,        NF) \
  O(BaseGL,          TWO(LA, OA(MOpMode)),                              \
                                       NOV,             NOV,        NF) \
  O(BaseSC,          FOUR(IVA, IVA, OA(MOpMode), OA(ReadonlyOp)),       \
                                       NOV,             NOV,        NF) \
  O(BaseL,           THREE(NLA, OA(MOpMode), OA(ReadonlyOp)),           \
                                       NOV,             NOV,        NF) \
  O(BaseC,           TWO(IVA, OA(MOpMode)),                             \
                                       NOV,             NOV,        NF) \
  O(BaseH,           NA,               NOV,             NOV,        NF) \
  O(Dim,             TWO(OA(MOpMode), KA),                              \
                                       NOV,             NOV,        NF) \
  O(QueryM,          THREE(IVA, OA(QueryMOp), KA),                      \
                                       MFINAL,          ONE(CV),    NF) \
  O(SetM,            TWO(IVA, KA),     C_MFINAL(1),     ONE(CV),    NF) \
  O(SetRangeM,       THREE(IVA, IVA, OA(SetRangeOp)),                   \
                                       C_MFINAL(3),     NOV,        NF) \
  O(IncDecM,         THREE(IVA, OA(IncDecOp), KA),                      \
                                       MFINAL,          ONE(CV),    NF) \
  O(SetOpM,          THREE(IVA, OA(SetOpOp), KA),                       \
                                       C_MFINAL(1),     ONE(CV),    NF) \
  O(UnsetM,          TWO(IVA, KA),     MFINAL,          NOV,        NF) \
  O(MemoGet,         TWO(BA, LAR),     NOV,             ONE(CV),    CF) \
  O(MemoGetEager,    THREE(BA, BA, LAR),                                \
                                       NOV,             ONE(CV),    CF) \
  O(MemoSet,         ONE(LAR),         ONE(CV),         ONE(CV),    NF) \
  O(MemoSetEager,    ONE(LAR),         ONE(CV),         ONE(CV),    NF)

}
