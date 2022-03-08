// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use crate::{
    AdataId, BareThisOp, BumpSliceMut, ClassId, ClassNum, CollectionType, ConstId, ContCheckOp,
    FatalOp, FcallArgs, FunctionId, IncDecOp, InitPropOp, IsLogAsDynamicCallOp, IsTypeOp, IterArgs,
    IterId, Label, Local, LocalRange, MOpMode, MemberKey, MethodId, NumParams, OODeclExistsOp,
    ObjMethodOp, ParamId, PropId, QueryMOp, ReadonlyOp, RepoAuthType, SetOpOp, SetRangeOp,
    SilenceOp, Slice, SpecialClsRef, StackIndex, Str, SwitchKind, Targets, TypeStructResolveOp,
};

#[derive(Clone, Debug)]
#[repr(C)]
pub enum Opcodes<'arena> {
    AKExists,
    Add,
    AddElemC,
    AddNewElemC,
    AddO,
    ArrayIdx,
    ArrayMarkLegacy,
    ArrayUnmarkLegacy,
    AssertRATL(Local<'arena>, RepoAuthType<'arena>),
    AssertRATStk(StackIndex, RepoAuthType<'arena>),
    Await,
    AwaitAll(LocalRange),
    BareThis(BareThisOp),
    BaseC(StackIndex, MOpMode),
    BaseGC(StackIndex, MOpMode),
    BaseGL(Local<'arena>, MOpMode),
    BaseH,
    BaseL(Local<'arena>, MOpMode, ReadonlyOp),
    BaseSC(StackIndex, StackIndex, MOpMode, ReadonlyOp),
    BitAnd,
    BitNot,
    BitOr,
    BitXor,
    BreakTraceHint,
    CGetCUNop,
    CGetG,
    CGetL(Local<'arena>),
    CGetL2(Local<'arena>),
    CGetQuietL(Local<'arena>),
    CGetS(ReadonlyOp),
    CUGetL(Local<'arena>),
    CastBool,
    CastDict,
    CastDouble,
    CastInt,
    CastKeyset,
    CastString,
    CastVec,
    ChainFaults,
    CheckProp(PropId<'arena>),
    CheckReifiedGenericMismatch,
    CheckThis,
    ClassGetC,
    ClassGetTS,
    ClassName,
    Clone,
    ClsCns(ConstId<'arena>),
    ClsCnsD(ConstId<'arena>, ClassId<'arena>),
    ClsCnsL(Local<'arena>),
    Cmp,
    CnsE(ConstId<'arena>),
    ColFromArray(CollectionType),
    CombineAndResolveTypeStruct(u32),
    Concat,
    ConcatN(u32),
    ContCheck(ContCheckOp),
    ContCurrent,
    ContEnter,
    ContGetReturn,
    ContKey,
    ContRaise,
    ContValid,
    CreateCl(NumParams, ClassNum),
    CreateCont,
    DblAsBits,
    Dict(AdataId<'arena>),
    Dim(MOpMode, MemberKey<'arena>),
    Dir,
    Div,
    Double(f64),
    Dup,
    EntryNop,
    Eq,
    Eval,
    Exit,
    FCallClsMethod(FcallArgs<'arena>, Str<'arena>, IsLogAsDynamicCallOp),
    FCallClsMethodD(
        FcallArgs<'arena>,
        Str<'arena>,
        ClassId<'arena>,
        MethodId<'arena>,
    ),
    FCallClsMethodS(FcallArgs<'arena>, Str<'arena>, SpecialClsRef),
    FCallClsMethodSD(
        FcallArgs<'arena>,
        Str<'arena>,
        SpecialClsRef,
        MethodId<'arena>,
    ),
    FCallCtor(FcallArgs<'arena>, Str<'arena>),
    FCallFunc(FcallArgs<'arena>),
    FCallFuncD(FcallArgs<'arena>, FunctionId<'arena>),
    FCallObjMethod(FcallArgs<'arena>, Str<'arena>, ObjMethodOp),
    FCallObjMethodD(
        FcallArgs<'arena>,
        Str<'arena>,
        ObjMethodOp,
        MethodId<'arena>,
    ),
    False,
    Fatal(FatalOp),
    File,
    FuncCred,
    GetMemoKeyL(Local<'arena>),
    Gt,
    Gte,
    Idx,
    IncDecG(IncDecOp),
    IncDecL(Local<'arena>, IncDecOp),
    IncDecM(StackIndex, IncDecOp, MemberKey<'arena>),
    IncDecS(IncDecOp),
    Incl,
    InclOnce,
    InitProp(PropId<'arena>, InitPropOp),
    InstanceOf,
    InstanceOfD(ClassId<'arena>),
    Int(i64),
    IsLateBoundCls,
    IsTypeC(IsTypeOp),
    IsTypeL(Local<'arena>, IsTypeOp),
    IsTypeStructC(TypeStructResolveOp),
    IsUnsetL(Local<'arena>),
    IssetG,
    IssetL(Local<'arena>),
    IssetS,
    IterFree(IterId),
    IterInit(IterArgs<'arena>, Label),
    IterNext(IterArgs<'arena>, Label),
    Jmp(Label),
    JmpNS(Label),
    JmpNZ(Label),
    JmpZ(Label),
    Keyset(AdataId<'arena>),
    LIterFree(IterId, Local<'arena>),
    LIterInit(IterArgs<'arena>, Local<'arena>, Label),
    LIterNext(IterArgs<'arena>, Local<'arena>, Label),
    LateBoundCls,
    LazyClass(ClassId<'arena>),
    LazyClassFromClass,
    LockObj,
    Lt,
    Lte,
    MemoGet(Label, LocalRange),
    MemoGetEager([Label; 2], LocalRange),
    MemoSet(LocalRange),
    MemoSetEager(LocalRange),
    Method,
    Mod,
    Mul,
    MulO,
    NSame,
    NativeImpl,
    Neq,
    NewCol(CollectionType),
    NewDictArray(u32),
    NewKeysetArray(u32),
    NewObj,
    NewObjD(ClassId<'arena>),
    NewObjR,
    NewObjRD(ClassId<'arena>),
    NewObjS(SpecialClsRef),
    NewPair,
    NewStructDict(Slice<'arena, Str<'arena>>),
    NewVec(u32),
    Nop,
    Not,
    Null,
    NullUninit,
    OODeclExists(OODeclExistsOp),
    ParentCls,
    PopC,
    PopL(Local<'arena>),
    PopU,
    PopU2,
    Pow,
    Print,
    PushL(Local<'arena>),
    QueryM(StackIndex, QueryMOp, MemberKey<'arena>),
    RaiseClassStringConversionWarning,
    RecordReifiedGeneric,
    Req,
    ReqDoc,
    ReqOnce,
    ResolveClass(ClassId<'arena>),
    ResolveClsMethod(MethodId<'arena>),
    ResolveClsMethodD(ClassId<'arena>, MethodId<'arena>),
    ResolveClsMethodS(SpecialClsRef, MethodId<'arena>),
    ResolveFunc(FunctionId<'arena>),
    ResolveMethCaller(FunctionId<'arena>),
    ResolveRClsMethod(MethodId<'arena>),
    ResolveRClsMethodD(ClassId<'arena>, MethodId<'arena>),
    ResolveRClsMethodS(SpecialClsRef, MethodId<'arena>),
    ResolveRFunc(FunctionId<'arena>),
    RetC,
    RetCSuspended,
    RetM(StackIndex),
    /// String switch
    SSwitch {
        /// One string for each case.
        cases: BumpSliceMut<'arena, Str<'arena>>,

        /// One Label for each case, congruent to cases.
        targets: BumpSliceMut<'arena, Label>,
    },
    Same,
    Select,
    SelfCls,
    SetG,
    SetImplicitContextByValue,
    SetL(Local<'arena>),
    SetM(StackIndex, MemberKey<'arena>),
    SetOpG(SetOpOp),
    SetOpL(Local<'arena>, SetOpOp),
    SetOpM(StackIndex, SetOpOp, MemberKey<'arena>),
    SetOpS(SetOpOp),
    SetRangeM(StackIndex, u32, SetRangeOp),
    SetS(ReadonlyOp),
    Shl,
    Shr,
    Silence(Local<'arena>, SilenceOp),
    String(Str<'arena>),
    Sub,
    SubO,
    /// Integer switch
    Switch(SwitchKind, i64, BumpSliceMut<'arena, Label>),
    This,
    Throw,
    ThrowAsTypeStructException,
    ThrowNonExhaustiveSwitch,
    True,
    UGetCUNop,
    UnsetG,
    UnsetL(Local<'arena>),
    UnsetM(StackIndex, MemberKey<'arena>),
    Vec(AdataId<'arena>),
    VerifyOutType(ParamId<'arena>),
    VerifyParamType(ParamId<'arena>),
    VerifyParamTypeTS(ParamId<'arena>),
    VerifyRetNonNullC,
    VerifyRetTypeC,
    VerifyRetTypeTS,
    WHResult,
    Yield,
    YieldK,
}

impl<'arena> Targets for Opcodes<'arena> {
    fn targets(&self) -> &[Label] {
        match self {
            Opcodes::FCallClsMethod(fca, _, _) => fca.targets(),
            Opcodes::FCallClsMethodD(fca, _, _, _) => fca.targets(),
            Opcodes::FCallClsMethodS(fca, _, _) => fca.targets(),
            Opcodes::FCallClsMethodSD(fca, _, _, _) => fca.targets(),
            Opcodes::FCallCtor(fca, _) => fca.targets(),
            Opcodes::FCallFunc(fca) => fca.targets(),
            Opcodes::FCallFuncD(fca, _) => fca.targets(),
            Opcodes::FCallObjMethod(fca, _, _) => fca.targets(),
            Opcodes::FCallObjMethodD(fca, _, _, _) => fca.targets(),
            Opcodes::IterInit(_, target2) => std::slice::from_ref(target2),
            Opcodes::IterNext(_, target2) => std::slice::from_ref(target2),
            Opcodes::Jmp(target1) => std::slice::from_ref(target1),
            Opcodes::JmpNS(target1) => std::slice::from_ref(target1),
            Opcodes::JmpNZ(target1) => std::slice::from_ref(target1),
            Opcodes::JmpZ(target1) => std::slice::from_ref(target1),
            Opcodes::LIterInit(_, _, target3) => std::slice::from_ref(target3),
            Opcodes::LIterNext(_, _, target3) => std::slice::from_ref(target3),
            Opcodes::MemoGet(target1, _) => std::slice::from_ref(target1),
            Opcodes::MemoGetEager(target1, _) => target1,
            Opcodes::SSwitch { cases: _, targets } => targets.as_ref(),
            Opcodes::Switch(_, _, targets) => targets.as_ref(),

            // Make sure new variants with branch target Labels are handled above
            // before adding items to this catch-all.
            Opcodes::AKExists
            | Opcodes::Add
            | Opcodes::AddElemC
            | Opcodes::AddNewElemC
            | Opcodes::AddO
            | Opcodes::ArrayIdx
            | Opcodes::ArrayMarkLegacy
            | Opcodes::ArrayUnmarkLegacy
            | Opcodes::AssertRATL(..)
            | Opcodes::AssertRATStk(..)
            | Opcodes::Await
            | Opcodes::AwaitAll(..)
            | Opcodes::BareThis(..)
            | Opcodes::BaseC(..)
            | Opcodes::BaseGC(..)
            | Opcodes::BaseGL(..)
            | Opcodes::BaseH
            | Opcodes::BaseL(..)
            | Opcodes::BaseSC(..)
            | Opcodes::BitAnd
            | Opcodes::BitNot
            | Opcodes::BitOr
            | Opcodes::BitXor
            | Opcodes::BreakTraceHint
            | Opcodes::CGetCUNop
            | Opcodes::CGetG
            | Opcodes::CGetL(..)
            | Opcodes::CGetL2(..)
            | Opcodes::CGetQuietL(..)
            | Opcodes::CGetS(..)
            | Opcodes::CUGetL(..)
            | Opcodes::CastBool
            | Opcodes::CastDict
            | Opcodes::CastDouble
            | Opcodes::CastInt
            | Opcodes::CastKeyset
            | Opcodes::CastString
            | Opcodes::CastVec
            | Opcodes::ChainFaults
            | Opcodes::CheckProp(..)
            | Opcodes::CheckReifiedGenericMismatch
            | Opcodes::CheckThis
            | Opcodes::ClassGetC
            | Opcodes::ClassGetTS
            | Opcodes::ClassName
            | Opcodes::Clone
            | Opcodes::ClsCns(..)
            | Opcodes::ClsCnsD(..)
            | Opcodes::ClsCnsL(..)
            | Opcodes::Cmp
            | Opcodes::CnsE(..)
            | Opcodes::ColFromArray(..)
            | Opcodes::CombineAndResolveTypeStruct(..)
            | Opcodes::Concat
            | Opcodes::ConcatN(..)
            | Opcodes::ContCheck(..)
            | Opcodes::ContCurrent
            | Opcodes::ContEnter
            | Opcodes::ContGetReturn
            | Opcodes::ContKey
            | Opcodes::ContRaise
            | Opcodes::ContValid
            | Opcodes::CreateCl(..)
            | Opcodes::CreateCont
            | Opcodes::DblAsBits
            | Opcodes::Dict(..)
            | Opcodes::Dim(..)
            | Opcodes::Dir
            | Opcodes::Div
            | Opcodes::Double(..)
            | Opcodes::Dup
            | Opcodes::EntryNop
            | Opcodes::Eq
            | Opcodes::Eval
            | Opcodes::Exit
            | Opcodes::False
            | Opcodes::Fatal(..)
            | Opcodes::File
            | Opcodes::FuncCred
            | Opcodes::GetMemoKeyL(..)
            | Opcodes::Gt
            | Opcodes::Gte
            | Opcodes::Idx
            | Opcodes::IncDecG(..)
            | Opcodes::IncDecL(..)
            | Opcodes::IncDecM(..)
            | Opcodes::IncDecS(..)
            | Opcodes::Incl
            | Opcodes::InclOnce
            | Opcodes::InitProp(..)
            | Opcodes::InstanceOf
            | Opcodes::InstanceOfD(..)
            | Opcodes::Int(..)
            | Opcodes::IsLateBoundCls
            | Opcodes::IsTypeC(..)
            | Opcodes::IsTypeL(..)
            | Opcodes::IsTypeStructC(..)
            | Opcodes::IsUnsetL(..)
            | Opcodes::IssetG
            | Opcodes::IssetL(..)
            | Opcodes::IssetS
            | Opcodes::IterFree(..)
            | Opcodes::Keyset(..)
            | Opcodes::LIterFree(..)
            | Opcodes::LateBoundCls
            | Opcodes::LazyClass(..)
            | Opcodes::LazyClassFromClass
            | Opcodes::LockObj
            | Opcodes::Lt
            | Opcodes::Lte
            | Opcodes::MemoSet(..)
            | Opcodes::MemoSetEager(..)
            | Opcodes::Method
            | Opcodes::Mod
            | Opcodes::Mul
            | Opcodes::MulO
            | Opcodes::NSame
            | Opcodes::NativeImpl
            | Opcodes::Neq
            | Opcodes::NewCol(..)
            | Opcodes::NewDictArray(..)
            | Opcodes::NewKeysetArray(..)
            | Opcodes::NewObj
            | Opcodes::NewObjD(..)
            | Opcodes::NewObjR
            | Opcodes::NewObjRD(..)
            | Opcodes::NewObjS(..)
            | Opcodes::NewPair
            | Opcodes::NewStructDict(..)
            | Opcodes::NewVec(..)
            | Opcodes::Nop
            | Opcodes::Not
            | Opcodes::Null
            | Opcodes::NullUninit
            | Opcodes::OODeclExists(..)
            | Opcodes::ParentCls
            | Opcodes::PopC
            | Opcodes::PopL(..)
            | Opcodes::PopU
            | Opcodes::PopU2
            | Opcodes::Pow
            | Opcodes::Print
            | Opcodes::PushL(..)
            | Opcodes::QueryM(..)
            | Opcodes::RaiseClassStringConversionWarning
            | Opcodes::RecordReifiedGeneric
            | Opcodes::Req
            | Opcodes::ReqDoc
            | Opcodes::ReqOnce
            | Opcodes::ResolveClass(..)
            | Opcodes::ResolveClsMethod(..)
            | Opcodes::ResolveClsMethodD(..)
            | Opcodes::ResolveClsMethodS(..)
            | Opcodes::ResolveFunc(..)
            | Opcodes::ResolveMethCaller(..)
            | Opcodes::ResolveRClsMethod(..)
            | Opcodes::ResolveRClsMethodD(..)
            | Opcodes::ResolveRClsMethodS(..)
            | Opcodes::ResolveRFunc(..)
            | Opcodes::RetC
            | Opcodes::RetCSuspended
            | Opcodes::RetM(..)
            | Opcodes::Same
            | Opcodes::Select
            | Opcodes::SelfCls
            | Opcodes::SetG
            | Opcodes::SetImplicitContextByValue
            | Opcodes::SetL(..)
            | Opcodes::SetM(..)
            | Opcodes::SetOpG(..)
            | Opcodes::SetOpL(..)
            | Opcodes::SetOpM(..)
            | Opcodes::SetOpS(..)
            | Opcodes::SetRangeM(..)
            | Opcodes::SetS(..)
            | Opcodes::Shl
            | Opcodes::Shr
            | Opcodes::Silence(..)
            | Opcodes::String(..)
            | Opcodes::Sub
            | Opcodes::SubO
            | Opcodes::This
            | Opcodes::Throw
            | Opcodes::ThrowAsTypeStructException
            | Opcodes::ThrowNonExhaustiveSwitch
            | Opcodes::True
            | Opcodes::UGetCUNop
            | Opcodes::UnsetG
            | Opcodes::UnsetL(..)
            | Opcodes::UnsetM(..)
            | Opcodes::Vec(..)
            | Opcodes::VerifyOutType(..)
            | Opcodes::VerifyParamType(..)
            | Opcodes::VerifyParamTypeTS(..)
            | Opcodes::VerifyRetNonNullC
            | Opcodes::VerifyRetTypeC
            | Opcodes::VerifyRetTypeTS
            | Opcodes::WHResult
            | Opcodes::Yield
            | Opcodes::YieldK => &[],
        }
    }

    fn targets_mut(&mut self) -> &mut [Label] {
        match self {
            Opcodes::FCallClsMethod(fca, _, _) => fca.targets_mut(),
            Opcodes::FCallClsMethodD(fca, _, _, _) => fca.targets_mut(),
            Opcodes::FCallClsMethodS(fca, _, _) => fca.targets_mut(),
            Opcodes::FCallClsMethodSD(fca, _, _, _) => fca.targets_mut(),
            Opcodes::FCallCtor(fca, _) => fca.targets_mut(),
            Opcodes::FCallFunc(fca) => fca.targets_mut(),
            Opcodes::FCallFuncD(fca, _) => fca.targets_mut(),
            Opcodes::FCallObjMethod(fca, _, _) => fca.targets_mut(),
            Opcodes::FCallObjMethodD(fca, _, _, _) => fca.targets_mut(),
            Opcodes::IterInit(_, target2) => std::slice::from_mut(target2),
            Opcodes::IterNext(_, target2) => std::slice::from_mut(target2),
            Opcodes::Jmp(target1) => std::slice::from_mut(target1),
            Opcodes::JmpNS(target1) => std::slice::from_mut(target1),
            Opcodes::JmpNZ(target1) => std::slice::from_mut(target1),
            Opcodes::JmpZ(target1) => std::slice::from_mut(target1),
            Opcodes::LIterInit(_, _, target3) => std::slice::from_mut(target3),
            Opcodes::LIterNext(_, _, target3) => std::slice::from_mut(target3),
            Opcodes::MemoGet(target1, _) => std::slice::from_mut(target1),
            Opcodes::MemoGetEager(target1, _) => target1,
            Opcodes::SSwitch { cases: _, targets } => targets.as_mut(),
            Opcodes::Switch(_, _, targets) => targets.as_mut(),

            // Make sure new variants with branch target Labels are handled above
            // before adding items to this catch-all.
            Opcodes::AKExists
            | Opcodes::Add
            | Opcodes::AddElemC
            | Opcodes::AddNewElemC
            | Opcodes::AddO
            | Opcodes::ArrayIdx
            | Opcodes::ArrayMarkLegacy
            | Opcodes::ArrayUnmarkLegacy
            | Opcodes::AssertRATL(..)
            | Opcodes::AssertRATStk(..)
            | Opcodes::Await
            | Opcodes::AwaitAll(..)
            | Opcodes::BareThis(..)
            | Opcodes::BaseC(..)
            | Opcodes::BaseGC(..)
            | Opcodes::BaseGL(..)
            | Opcodes::BaseH
            | Opcodes::BaseL(..)
            | Opcodes::BaseSC(..)
            | Opcodes::BitAnd
            | Opcodes::BitNot
            | Opcodes::BitOr
            | Opcodes::BitXor
            | Opcodes::BreakTraceHint
            | Opcodes::CGetCUNop
            | Opcodes::CGetG
            | Opcodes::CGetL(..)
            | Opcodes::CGetL2(..)
            | Opcodes::CGetQuietL(..)
            | Opcodes::CGetS(..)
            | Opcodes::CUGetL(..)
            | Opcodes::CastBool
            | Opcodes::CastDict
            | Opcodes::CastDouble
            | Opcodes::CastInt
            | Opcodes::CastKeyset
            | Opcodes::CastString
            | Opcodes::CastVec
            | Opcodes::ChainFaults
            | Opcodes::CheckProp(..)
            | Opcodes::CheckReifiedGenericMismatch
            | Opcodes::CheckThis
            | Opcodes::ClassGetC
            | Opcodes::ClassGetTS
            | Opcodes::ClassName
            | Opcodes::Clone
            | Opcodes::ClsCns(..)
            | Opcodes::ClsCnsD(..)
            | Opcodes::ClsCnsL(..)
            | Opcodes::Cmp
            | Opcodes::CnsE(..)
            | Opcodes::ColFromArray(..)
            | Opcodes::CombineAndResolveTypeStruct(..)
            | Opcodes::Concat
            | Opcodes::ConcatN(..)
            | Opcodes::ContCheck(..)
            | Opcodes::ContCurrent
            | Opcodes::ContEnter
            | Opcodes::ContGetReturn
            | Opcodes::ContKey
            | Opcodes::ContRaise
            | Opcodes::ContValid
            | Opcodes::CreateCl(..)
            | Opcodes::CreateCont
            | Opcodes::DblAsBits
            | Opcodes::Dict(..)
            | Opcodes::Dim(..)
            | Opcodes::Dir
            | Opcodes::Div
            | Opcodes::Double(..)
            | Opcodes::Dup
            | Opcodes::EntryNop
            | Opcodes::Eq
            | Opcodes::Eval
            | Opcodes::Exit
            | Opcodes::False
            | Opcodes::Fatal(..)
            | Opcodes::File
            | Opcodes::FuncCred
            | Opcodes::GetMemoKeyL(..)
            | Opcodes::Gt
            | Opcodes::Gte
            | Opcodes::Idx
            | Opcodes::IncDecG(..)
            | Opcodes::IncDecL(..)
            | Opcodes::IncDecM(..)
            | Opcodes::IncDecS(..)
            | Opcodes::Incl
            | Opcodes::InclOnce
            | Opcodes::InitProp(..)
            | Opcodes::InstanceOf
            | Opcodes::InstanceOfD(..)
            | Opcodes::Int(..)
            | Opcodes::IsLateBoundCls
            | Opcodes::IsTypeC(..)
            | Opcodes::IsTypeL(..)
            | Opcodes::IsTypeStructC(..)
            | Opcodes::IsUnsetL(..)
            | Opcodes::IssetG
            | Opcodes::IssetL(..)
            | Opcodes::IssetS
            | Opcodes::IterFree(..)
            | Opcodes::Keyset(..)
            | Opcodes::LIterFree(..)
            | Opcodes::LateBoundCls
            | Opcodes::LazyClass(..)
            | Opcodes::LazyClassFromClass
            | Opcodes::LockObj
            | Opcodes::Lt
            | Opcodes::Lte
            | Opcodes::MemoSet(..)
            | Opcodes::MemoSetEager(..)
            | Opcodes::Method
            | Opcodes::Mod
            | Opcodes::Mul
            | Opcodes::MulO
            | Opcodes::NSame
            | Opcodes::NativeImpl
            | Opcodes::Neq
            | Opcodes::NewCol(..)
            | Opcodes::NewDictArray(..)
            | Opcodes::NewKeysetArray(..)
            | Opcodes::NewObj
            | Opcodes::NewObjD(..)
            | Opcodes::NewObjR
            | Opcodes::NewObjRD(..)
            | Opcodes::NewObjS(..)
            | Opcodes::NewPair
            | Opcodes::NewStructDict(..)
            | Opcodes::NewVec(..)
            | Opcodes::Nop
            | Opcodes::Not
            | Opcodes::Null
            | Opcodes::NullUninit
            | Opcodes::OODeclExists(..)
            | Opcodes::ParentCls
            | Opcodes::PopC
            | Opcodes::PopL(..)
            | Opcodes::PopU
            | Opcodes::PopU2
            | Opcodes::Pow
            | Opcodes::Print
            | Opcodes::PushL(..)
            | Opcodes::QueryM(..)
            | Opcodes::RaiseClassStringConversionWarning
            | Opcodes::RecordReifiedGeneric
            | Opcodes::Req
            | Opcodes::ReqDoc
            | Opcodes::ReqOnce
            | Opcodes::ResolveClass(..)
            | Opcodes::ResolveClsMethod(..)
            | Opcodes::ResolveClsMethodD(..)
            | Opcodes::ResolveClsMethodS(..)
            | Opcodes::ResolveFunc(..)
            | Opcodes::ResolveMethCaller(..)
            | Opcodes::ResolveRClsMethod(..)
            | Opcodes::ResolveRClsMethodD(..)
            | Opcodes::ResolveRClsMethodS(..)
            | Opcodes::ResolveRFunc(..)
            | Opcodes::RetC
            | Opcodes::RetCSuspended
            | Opcodes::RetM(..)
            | Opcodes::Same
            | Opcodes::Select
            | Opcodes::SelfCls
            | Opcodes::SetG
            | Opcodes::SetImplicitContextByValue
            | Opcodes::SetL(..)
            | Opcodes::SetM(..)
            | Opcodes::SetOpG(..)
            | Opcodes::SetOpL(..)
            | Opcodes::SetOpM(..)
            | Opcodes::SetOpS(..)
            | Opcodes::SetRangeM(..)
            | Opcodes::SetS(..)
            | Opcodes::Shl
            | Opcodes::Shr
            | Opcodes::Silence(..)
            | Opcodes::String(..)
            | Opcodes::Sub
            | Opcodes::SubO
            | Opcodes::This
            | Opcodes::Throw
            | Opcodes::ThrowAsTypeStructException
            | Opcodes::ThrowNonExhaustiveSwitch
            | Opcodes::True
            | Opcodes::UGetCUNop
            | Opcodes::UnsetG
            | Opcodes::UnsetL(..)
            | Opcodes::UnsetM(..)
            | Opcodes::Vec(..)
            | Opcodes::VerifyOutType(..)
            | Opcodes::VerifyParamType(..)
            | Opcodes::VerifyParamTypeTS(..)
            | Opcodes::VerifyRetNonNullC
            | Opcodes::VerifyRetTypeC
            | Opcodes::VerifyRetTypeTS
            | Opcodes::WHResult
            | Opcodes::Yield
            | Opcodes::YieldK => &mut [],
        }
    }
}
