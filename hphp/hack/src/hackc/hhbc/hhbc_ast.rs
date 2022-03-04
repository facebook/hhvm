// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use ffi::{
    BumpSliceMut,
    Maybe::{self, *},
    Pair, Slice, Str,
};
use iterator::IterId;
use label::Label;
use local::Local;

/// see runtime/base/repo-auth-type.h
pub type RepoAuthType<'arena> = Str<'arena>;

/// Export these publicly so consumers of hhbc_ast don't have to know the
/// internal details about the ffi.
pub use hhvm_hhbc_defs_ffi::ffi::{
    BareThisOp, CollectionType, ContCheckOp, FCallArgsFlags, FatalOp, IncDecOp, InitPropOp,
    IsLogAsDynamicCallOp, IsTypeOp, MOpMode, ObjMethodOp, QueryMOp, ReadonlyOp, SetOpOp,
    SetRangeOp, SilenceOp, SpecialClsRef, SwitchKind, TypeStructResolveOp,
};

#[derive(Clone, Debug)]
#[repr(C)]
pub enum ParamId<'arena> {
    ParamUnnamed(isize),
    ParamNamed(Str<'arena>),
}

pub type ParamNum = isize;
pub type StackIndex = isize;
pub type RecordNum = isize;
pub type TypedefNum = isize;
pub type ClassNum = isize;
pub type ConstNum = isize;

pub type ClassId<'arena> = hhbc_id::class::ClassType<'arena>;
pub type FunctionId<'arena> = hhbc_id::function::FunctionType<'arena>;
pub type MethodId<'arena> = hhbc_id::method::MethodType<'arena>;
pub type ConstId<'arena> = hhbc_id::constant::ConstType<'arena>;
pub type PropId<'arena> = hhbc_id::prop::PropType<'arena>;

pub type NumParams = usize;
pub type ByRefs<'arena> = Slice<'arena, bool>;

#[derive(Clone, Debug)]
#[repr(C)]
pub struct FcallArgs<'arena> {
    pub flags: FCallArgsFlags,
    pub num_args: NumParams,
    pub num_rets: NumParams,
    pub inouts: ByRefs<'arena>,
    pub readonly: ByRefs<'arena>,
    pub async_eager_target: Maybe<Label>,
    pub context: Maybe<Str<'arena>>,
}

impl<'arena> FcallArgs<'arena> {
    pub fn new(
        flags: FCallArgsFlags,
        num_rets: usize,
        inouts: Slice<'arena, bool>,
        readonly: Slice<'arena, bool>,
        async_eager_target: Option<Label>,
        num_args: usize,
        context: Option<&'arena str>,
    ) -> FcallArgs<'arena> {
        assert!(
            (inouts.is_empty() || inouts.len() == num_args)
                && (readonly.is_empty() || readonly.len() == num_args),
            "length of by_refs must be either zero or num_args"
        );
        FcallArgs {
            flags,
            num_args,
            num_rets,
            inouts,
            readonly,
            async_eager_target: async_eager_target.into(),
            context: context.map(|s| Str::new(s.as_bytes())).into(),
        }
    }

    pub fn targets(&self) -> &[Label] {
        match &self.async_eager_target {
            Just(x) => std::slice::from_ref(x),
            Nothing => &[],
        }
    }

    pub fn targets_mut(&mut self) -> &mut [Label] {
        match &mut self.async_eager_target {
            Just(x) => std::slice::from_mut(x),
            Nothing => &mut [],
        }
    }
}

#[derive(Clone, Debug)]
#[repr(C)]
pub struct IterArgs<'arena> {
    pub iter_id: IterId,
    pub key_id: Maybe<Local<'arena>>,
    pub val_id: Local<'arena>,
}

pub type ClassrefId = isize;
/// Conventionally this is "A_" followed by an integer
pub type AdataId<'arena> = Str<'arena>;
pub type ParamLocations<'arena> = Slice<'arena, isize>;

#[derive(Clone, Copy, Debug)]
#[repr(C)]
pub enum MemberKey<'arena> {
    EC(StackIndex, ReadonlyOp),
    EL(Local<'arena>, ReadonlyOp),
    ET(Str<'arena>, ReadonlyOp),
    EI(i64, ReadonlyOp),
    PC(StackIndex, ReadonlyOp),
    PL(Local<'arena>, ReadonlyOp),
    PT(PropId<'arena>, ReadonlyOp),
    QT(PropId<'arena>, ReadonlyOp),
    W,
}

#[derive(Clone, Copy, Debug, Eq, PartialEq)]
#[repr(C)]
pub enum HasGenericsOp {
    NoGenerics,
    MaybeGenerics,
    HasGenerics,
}

#[derive(Clone, Copy, Debug, Eq, PartialEq)]
#[repr(C)]
pub enum ClassishKind {
    Class, // c.f. ast_defs::ClassishKind - may need Abstraction (Concrete, Abstract)
    Interface,
    Trait,
    Enum,
    EnumClass,
}
impl std::convert::From<oxidized::ast_defs::ClassishKind> for ClassishKind {
    fn from(k: oxidized::ast_defs::ClassishKind) -> Self {
        use oxidized::ast_defs;
        match k {
            ast_defs::ClassishKind::Cclass(_) => Self::Class,
            ast_defs::ClassishKind::Cinterface => Self::Interface,
            ast_defs::ClassishKind::Ctrait => Self::Trait,
            ast_defs::ClassishKind::Cenum => Self::Enum,
            ast_defs::ClassishKind::CenumClass(_) => Self::EnumClass,
        }
    }
}

#[derive(Clone, Copy, Debug, Eq, PartialEq)]
#[repr(C)]
pub enum Visibility {
    Private,
    Public,
    Protected,
    Internal,
}
impl std::convert::From<oxidized::ast_defs::Visibility> for Visibility {
    fn from(k: oxidized::ast_defs::Visibility) -> Self {
        use oxidized::ast_defs;
        match k {
            ast_defs::Visibility::Private => Self::Private,
            ast_defs::Visibility::Public => Self::Public,
            ast_defs::Visibility::Protected => Self::Protected,
            ast_defs::Visibility::Internal => Self::Internal,
        }
    }
}
impl AsRef<str> for Visibility {
    fn as_ref(&self) -> &str {
        match self {
            Self::Private => "private",
            Self::Public => "public",
            Self::Protected => "protected",
            Self::Internal => "internal",
        }
    }
}

#[derive(Clone, Debug)]
#[repr(C)]
pub struct SrcLoc {
    pub line_begin: isize,
    pub col_begin: isize,
    pub line_end: isize,
    pub col_end: isize,
}

#[derive(Clone, Debug)]
#[repr(C)]
pub enum Instruct<'arena> {
    Nop,
    EntryNop,
    PopC,
    PopU,
    Dup,
    TryCatchBegin,
    TryCatchMiddle,
    TryCatchEnd,
    IterInit(IterArgs<'arena>, Label),
    IterNext(IterArgs<'arena>, Label),
    IterFree(IterId),
    Null,
    True,
    False,
    NullUninit,
    Int(i64),
    Double(f64),
    String(Str<'arena>),
    LazyClass(ClassId<'arena>),
    /// Pseudo instruction that will get translated into appropraite literal
    /// bytecode, with possible reference to .adata *)
    TypedValue(runtime::TypedValue<'arena>),
    Vec(AdataId<'arena>),
    Dict(AdataId<'arena>),
    Keyset(AdataId<'arena>),
    /// capacity hint
    NewDictArray(isize),
    NewStructDict(Slice<'arena, Str<'arena>>),
    NewVec(isize),
    NewKeysetArray(isize),
    NewPair,
    NewRecord(ClassId<'arena>, Slice<'arena, Str<'arena>>),
    AddElemC,
    AddNewElemC,
    NewCol(CollectionType),
    ColFromArray(CollectionType),
    CnsE(ConstId<'arena>),
    ClsCns(ConstId<'arena>),
    ClsCnsD(ConstId<'arena>, ClassId<'arena>),
    ClsCnsL(Local<'arena>),
    File,
    Dir,
    Method,
    FuncCred,
    Concat,
    ConcatN(isize),
    Add,
    Sub,
    Mul,
    AddO,
    SubO,
    MulO,
    Div,
    Mod,
    Pow,
    Not,
    Same,
    NSame,
    Eq,
    Neq,
    Lt,
    Lte,
    Gt,
    Gte,
    Cmp,
    BitAnd,
    BitOr,
    BitXor,
    BitNot,
    Shl,
    Shr,
    CastBool,
    CastInt,
    CastDouble,
    CastString,
    CastVec,
    CastDict,
    CastKeyset,
    InstanceOf,
    InstanceOfD(ClassId<'arena>),
    IsLateBoundCls,
    IsTypeStructC(TypeStructResolveOp),
    ThrowAsTypeStructException,
    CombineAndResolveTypeStruct(isize),
    Print,
    Clone,
    Exit,
    Fatal(FatalOp),
    ResolveFunc(FunctionId<'arena>),
    ResolveRFunc(FunctionId<'arena>),
    ResolveMethCaller(FunctionId<'arena>),
    ResolveClsMethod(MethodId<'arena>),
    ResolveClsMethodD(ClassId<'arena>, MethodId<'arena>),
    ResolveClsMethodS(SpecialClsRef, MethodId<'arena>),
    ResolveRClsMethod(MethodId<'arena>),
    ResolveRClsMethodD(ClassId<'arena>, MethodId<'arena>),
    ResolveRClsMethodS(SpecialClsRef, MethodId<'arena>),
    ResolveClass(ClassId<'arena>),
    Jmp(Label),
    JmpNS(Label),
    JmpZ(Label),
    JmpNZ(Label),

    /// Integer switch
    Switch {
        kind: SwitchKind,
        base: isize,
        targets: BumpSliceMut<'arena, Label>,
    },

    /// String switch
    SSwitch {
        /// One string for each case.
        cases: BumpSliceMut<'arena, Str<'arena>>,

        /// One Label for each case, congruent to cases.
        targets: BumpSliceMut<'arena, Label>,
    },
    RetC,
    RetCSuspended,
    RetM(NumParams),
    Throw,
    Continue(isize),
    Break(isize),
    FCallClsMethod {
        fcall_args: FcallArgs<'arena>,
        log: IsLogAsDynamicCallOp,
    },
    FCallClsMethodD {
        fcall_args: FcallArgs<'arena>,
        class: ClassId<'arena>,
        method: MethodId<'arena>,
    },
    FCallClsMethodS {
        fcall_args: FcallArgs<'arena>,
        clsref: SpecialClsRef,
    },
    FCallClsMethodSD {
        fcall_args: FcallArgs<'arena>,
        clsref: SpecialClsRef,
        method: MethodId<'arena>,
    },
    FCallCtor(FcallArgs<'arena>),
    FCallFunc(FcallArgs<'arena>),
    FCallFuncD {
        fcall_args: FcallArgs<'arena>,
        func: FunctionId<'arena>,
    },
    FCallObjMethod {
        fcall_args: FcallArgs<'arena>,
        flavor: ObjMethodOp,
    },
    FCallObjMethodD {
        fcall_args: FcallArgs<'arena>,
        flavor: ObjMethodOp,
        method: MethodId<'arena>,
    },
    NewObj,
    NewObjR,
    NewObjD(ClassId<'arena>),
    NewObjRD(ClassId<'arena>),
    NewObjS(SpecialClsRef),
    This,
    BareThis(BareThisOp),
    CheckThis,
    FuncNumArgs,
    ChainFaults,
    OODeclExists(ClassishKind),
    VerifyParamType(ParamId<'arena>),
    VerifyParamTypeTS(ParamId<'arena>),
    VerifyOutType(ParamId<'arena>),
    VerifyRetTypeC,
    VerifyRetTypeTS,
    SelfCls,
    ParentCls,
    LateBoundCls,
    ClassName,
    LazyClassFromClass,
    RecordReifiedGeneric,
    CheckReifiedGenericMismatch,
    NativeImpl,
    AKExists,
    CreateCl(NumParams, ClassNum),
    Idx,
    ArrayIdx,
    ArrayMarkLegacy,
    ArrayUnmarkLegacy,
    AssertRATL(Local<'arena>, RepoAuthType<'arena>),
    AssertRATStk(StackIndex, RepoAuthType<'arena>),
    BreakTraceHint,
    Silence(Local<'arena>, SilenceOp),
    GetMemoKeyL(Local<'arena>),
    CGetCUNop,
    UGetCUNop,
    MemoGet(Label, Maybe<Pair<Local<'arena>, isize>>),
    MemoGetEager([Label; 2], Maybe<Pair<Local<'arena>, isize>>),
    MemoSet(Maybe<Pair<Local<'arena>, isize>>),
    MemoSetEager(Maybe<Pair<Local<'arena>, isize>>),
    LockObj,
    ThrowNonExhaustiveSwitch,
    RaiseClassStringConversionWarning,
    SetImplicitContextByValue,
    CGetL(Local<'arena>),
    CGetQuietL(Local<'arena>),
    CGetL2(Local<'arena>),
    CUGetL(Local<'arena>),
    PushL(Local<'arena>),
    CGetG,
    CGetS(ReadonlyOp),
    ClassGetC,
    ClassGetTS,
    SetL(Local<'arena>),
    PopL(Local<'arena>),
    SetG,
    SetS(ReadonlyOp),
    SetOpL(Local<'arena>, SetOpOp),
    SetOpG(SetOpOp),
    SetOpS(SetOpOp),
    IncDecL(Local<'arena>, IncDecOp),
    IncDecG(IncDecOp),
    IncDecS(IncDecOp),
    UnsetL(Local<'arena>),
    UnsetG,
    CheckProp(PropId<'arena>),
    InitProp(PropId<'arena>, InitPropOp),
    IssetL(Local<'arena>),
    IssetG,
    IssetS,
    IsUnsetL(Local<'arena>),
    IsTypeC(IsTypeOp),
    IsTypeL(Local<'arena>, IsTypeOp),
    BaseGC(StackIndex, MOpMode),
    BaseGL(Local<'arena>, MOpMode),
    BaseSC(StackIndex, StackIndex, MOpMode, ReadonlyOp),
    BaseL(Local<'arena>, MOpMode, ReadonlyOp),
    BaseC(StackIndex, MOpMode),
    BaseH,
    Dim(MOpMode, MemberKey<'arena>),
    QueryM(NumParams, QueryMOp, MemberKey<'arena>),
    SetM(NumParams, MemberKey<'arena>),
    IncDecM(NumParams, IncDecOp, MemberKey<'arena>),
    SetOpM(NumParams, SetOpOp, MemberKey<'arena>),
    UnsetM(NumParams, MemberKey<'arena>),
    SetRangeM(NumParams, isize, SetRangeOp),
    Label(Label),
    Comment(Str<'arena>),
    SrcLoc(SrcLoc),
    WHResult,
    Await,
    AwaitAll(Maybe<Pair<Local<'arena>, isize>>),
    CreateCont,
    ContEnter,
    ContRaise,
    Yield,
    YieldK,
    ContCheck(ContCheckOp),
    ContValid,
    ContKey,
    ContGetReturn,
    ContCurrent,
    Incl,
    InclOnce,
    Req,
    ReqOnce,
    ReqDoc,
    Eval,
}

impl Instruct<'_> {
    /// Return a slice of labels for the conditional branch targets of this instruction.
    /// This excludes the Label in an ILabel instruction, which is not a conditional branch.
    pub fn targets(&self) -> &[Label] {
        match self {
            Self::FCallClsMethod { fcall_args, .. }
            | Self::FCallClsMethodD { fcall_args, .. }
            | Self::FCallClsMethodS { fcall_args, .. }
            | Self::FCallClsMethodSD { fcall_args, .. }
            | Self::FCallCtor(fcall_args)
            | Self::FCallFunc(fcall_args)
            | Self::FCallFuncD { fcall_args, .. }
            | Self::FCallObjMethod { fcall_args, .. }
            | Self::FCallObjMethodD { fcall_args, .. } => fcall_args.targets(),
            Self::Jmp(x) | Self::JmpNS(x) | Self::JmpZ(x) | Self::JmpNZ(x) => {
                std::slice::from_ref(x)
            }
            Self::Switch { targets, .. } | Self::SSwitch { targets, .. } => targets.as_ref(),
            Self::IterInit(_, target) | Self::IterNext(_, target) => std::slice::from_ref(target),
            Self::MemoGet(target, _) => std::slice::from_ref(target),
            Self::MemoGetEager(targets, _) => targets,

            // Make sure new variants with branch target Labels are handled above
            // before adding items to this catch-all.
            Self::RetC
            | Self::This
            | Self::BareThis(_)
            | Self::CheckThis
            | Self::FuncNumArgs
            | Self::ChainFaults
            | Self::OODeclExists(_)
            | Self::VerifyParamType(_)
            | Self::VerifyParamTypeTS(_)
            | Self::VerifyOutType(_)
            | Self::VerifyRetTypeC
            | Self::VerifyRetTypeTS
            | Self::SelfCls
            | Self::ParentCls
            | Self::LateBoundCls
            | Self::ClassName
            | Self::LazyClassFromClass
            | Self::RecordReifiedGeneric
            | Self::CheckReifiedGenericMismatch
            | Self::NativeImpl
            | Self::AKExists
            | Self::CreateCl(_, _)
            | Self::Idx
            | Self::ArrayIdx
            | Self::ArrayMarkLegacy
            | Self::ArrayUnmarkLegacy
            | Self::AssertRATL(_, _)
            | Self::AssertRATStk(_, _)
            | Self::BreakTraceHint
            | Self::Silence(_, _)
            | Self::GetMemoKeyL(_)
            | Self::CGetCUNop
            | Self::UGetCUNop
            | Self::MemoSet(_)
            | Self::MemoSetEager(_)
            | Self::LockObj
            | Self::ThrowNonExhaustiveSwitch
            | Self::RaiseClassStringConversionWarning
            | Self::SetImplicitContextByValue
            | Self::RetCSuspended
            | Self::RetM(_)
            | Self::Throw
            | Self::Nop
            | Self::EntryNop
            | Self::PopC
            | Self::PopU
            | Self::Dup
            | Self::IterFree(_)
            | Self::Null
            | Self::True
            | Self::False
            | Self::NullUninit
            | Self::Int(_)
            | Self::Double(_)
            | Self::String(_)
            | Self::LazyClass(_)
            | Self::TypedValue(_)
            | Self::Vec(_)
            | Self::Dict(_)
            | Self::Keyset(_)
            | Self::NewDictArray(_)
            | Self::NewStructDict(_)
            | Self::NewVec(_)
            | Self::NewKeysetArray(_)
            | Self::NewPair
            | Self::NewRecord(_, _)
            | Self::AddElemC
            | Self::AddNewElemC
            | Self::NewCol(_)
            | Self::ColFromArray(_)
            | Self::CnsE(_)
            | Self::ClsCns(_)
            | Self::ClsCnsD(_, _)
            | Self::ClsCnsL(_)
            | Self::File
            | Self::Dir
            | Self::Method
            | Self::FuncCred
            | Self::Concat
            | Self::ConcatN(_)
            | Self::Add
            | Self::Sub
            | Self::Mul
            | Self::AddO
            | Self::SubO
            | Self::MulO
            | Self::Div
            | Self::Mod
            | Self::Pow
            | Self::Not
            | Self::Same
            | Self::NSame
            | Self::Eq
            | Self::Neq
            | Self::Lt
            | Self::Lte
            | Self::Gt
            | Self::Gte
            | Self::Cmp
            | Self::BitAnd
            | Self::BitOr
            | Self::BitXor
            | Self::BitNot
            | Self::Shl
            | Self::Shr
            | Self::CastBool
            | Self::CastInt
            | Self::CastDouble
            | Self::CastString
            | Self::CastVec
            | Self::CastDict
            | Self::CastKeyset
            | Self::InstanceOf
            | Self::InstanceOfD(_)
            | Self::IsLateBoundCls
            | Self::IsTypeStructC(_)
            | Self::ThrowAsTypeStructException
            | Self::CombineAndResolveTypeStruct(_)
            | Self::Print
            | Self::Clone
            | Self::Exit
            | Self::Fatal(_)
            | Self::ResolveFunc(_)
            | Self::ResolveRFunc(_)
            | Self::ResolveMethCaller(_)
            | Self::ResolveClsMethod(_)
            | Self::ResolveClsMethodD(_, _)
            | Self::ResolveClsMethodS(_, _)
            | Self::ResolveRClsMethod(_)
            | Self::ResolveRClsMethodD(_, _)
            | Self::ResolveRClsMethodS(_, _)
            | Self::ResolveClass(_)
            | Self::Continue(_)
            | Self::Break(_)
            | Self::NewObj
            | Self::NewObjR
            | Self::NewObjD(_)
            | Self::NewObjRD(_)
            | Self::NewObjS(_)
            | Self::CGetL(_)
            | Self::CGetQuietL(_)
            | Self::CGetL2(_)
            | Self::CUGetL(_)
            | Self::PushL(_)
            | Self::CGetG
            | Self::CGetS(_)
            | Self::ClassGetC
            | Self::ClassGetTS
            | Self::SetL(_)
            | Self::PopL(_)
            | Self::SetG
            | Self::SetS(_)
            | Self::SetOpL(_, _)
            | Self::SetOpG(_)
            | Self::SetOpS(_)
            | Self::IncDecL(_, _)
            | Self::IncDecG(_)
            | Self::IncDecS(_)
            | Self::UnsetL(_)
            | Self::UnsetG
            | Self::CheckProp(_)
            | Self::InitProp(_, _)
            | Self::IssetL(_)
            | Self::IssetG
            | Self::IssetS
            | Self::IsUnsetL(_)
            | Self::IsTypeC(_)
            | Self::IsTypeL(_, _)
            | Self::BaseGC(_, _)
            | Self::BaseGL(_, _)
            | Self::BaseSC(_, _, _, _)
            | Self::BaseL(_, _, _)
            | Self::BaseC(_, _)
            | Self::BaseH
            | Self::Dim(_, _)
            | Self::QueryM(_, _, _)
            | Self::SetM(_, _)
            | Self::IncDecM(_, _, _)
            | Self::SetOpM(_, _, _)
            | Self::UnsetM(_, _)
            | Self::SetRangeM(_, _, _)
            | Self::Label(_)
            | Self::TryCatchBegin
            | Self::TryCatchMiddle
            | Self::TryCatchEnd
            | Self::Comment(_)
            | Self::SrcLoc(_)
            | Self::WHResult
            | Self::Await
            | Self::AwaitAll(_)
            | Self::CreateCont
            | Self::ContEnter
            | Self::ContRaise
            | Self::Yield
            | Self::YieldK
            | Self::ContCheck(_)
            | Self::ContValid
            | Self::ContKey
            | Self::ContGetReturn
            | Self::ContCurrent
            | Self::Incl
            | Self::InclOnce
            | Self::Req
            | Self::ReqOnce
            | Self::ReqDoc
            | Self::Eval => &[],
        }
    }

    /// Return a mutable slice of labels for the conditional branch targets of this instruction.
    /// This excludes the Label in an ILabel instruction, which is not a conditional branch.
    pub fn targets_mut(&mut self) -> &mut [Label] {
        match self {
            Self::FCallClsMethod { fcall_args, .. }
            | Self::FCallClsMethodD { fcall_args, .. }
            | Self::FCallClsMethodS { fcall_args, .. }
            | Self::FCallClsMethodSD { fcall_args, .. }
            | Self::FCallCtor(fcall_args)
            | Self::FCallFunc(fcall_args)
            | Self::FCallFuncD { fcall_args, .. }
            | Self::FCallObjMethod { fcall_args, .. }
            | Self::FCallObjMethodD { fcall_args, .. } => fcall_args.targets_mut(),
            Self::Jmp(x) | Self::JmpNS(x) | Self::JmpZ(x) | Self::JmpNZ(x) => {
                std::slice::from_mut(x)
            }
            Self::Switch { targets, .. } | Self::SSwitch { targets, .. } => targets.as_mut(),
            Self::IterInit(_, target) | Self::IterNext(_, target) => std::slice::from_mut(target),
            Self::MemoGet(target, _) => std::slice::from_mut(target),
            Self::MemoGetEager(targets, _) => targets,

            // Make sure new variants with branch target Labels are handled above
            // before adding items to this catch-all.
            Self::RetC
            | Self::This
            | Self::BareThis(_)
            | Self::CheckThis
            | Self::FuncNumArgs
            | Self::ChainFaults
            | Self::OODeclExists(_)
            | Self::VerifyParamType(_)
            | Self::VerifyParamTypeTS(_)
            | Self::VerifyOutType(_)
            | Self::VerifyRetTypeC
            | Self::VerifyRetTypeTS
            | Self::SelfCls
            | Self::ParentCls
            | Self::LateBoundCls
            | Self::ClassName
            | Self::LazyClassFromClass
            | Self::RecordReifiedGeneric
            | Self::CheckReifiedGenericMismatch
            | Self::NativeImpl
            | Self::AKExists
            | Self::CreateCl(_, _)
            | Self::Idx
            | Self::ArrayIdx
            | Self::ArrayMarkLegacy
            | Self::ArrayUnmarkLegacy
            | Self::AssertRATL(_, _)
            | Self::AssertRATStk(_, _)
            | Self::BreakTraceHint
            | Self::Silence(_, _)
            | Self::GetMemoKeyL(_)
            | Self::CGetCUNop
            | Self::UGetCUNop
            | Self::MemoSet(_)
            | Self::MemoSetEager(_)
            | Self::LockObj
            | Self::ThrowNonExhaustiveSwitch
            | Self::RaiseClassStringConversionWarning
            | Self::SetImplicitContextByValue
            | Self::RetCSuspended
            | Self::RetM(_)
            | Self::Throw
            | Self::Nop
            | Self::EntryNop
            | Self::PopC
            | Self::PopU
            | Self::Dup
            | Self::IterFree(_)
            | Self::Null
            | Self::True
            | Self::False
            | Self::NullUninit
            | Self::Int(_)
            | Self::Double(_)
            | Self::String(_)
            | Self::LazyClass(_)
            | Self::TypedValue(_)
            | Self::Vec(_)
            | Self::Dict(_)
            | Self::Keyset(_)
            | Self::NewDictArray(_)
            | Self::NewStructDict(_)
            | Self::NewVec(_)
            | Self::NewKeysetArray(_)
            | Self::NewPair
            | Self::NewRecord(_, _)
            | Self::AddElemC
            | Self::AddNewElemC
            | Self::NewCol(_)
            | Self::ColFromArray(_)
            | Self::CnsE(_)
            | Self::ClsCns(_)
            | Self::ClsCnsD(_, _)
            | Self::ClsCnsL(_)
            | Self::File
            | Self::Dir
            | Self::Method
            | Self::FuncCred
            | Self::Concat
            | Self::ConcatN(_)
            | Self::Add
            | Self::Sub
            | Self::Mul
            | Self::AddO
            | Self::SubO
            | Self::MulO
            | Self::Div
            | Self::Mod
            | Self::Pow
            | Self::Not
            | Self::Same
            | Self::NSame
            | Self::Eq
            | Self::Neq
            | Self::Lt
            | Self::Lte
            | Self::Gt
            | Self::Gte
            | Self::Cmp
            | Self::BitAnd
            | Self::BitOr
            | Self::BitXor
            | Self::BitNot
            | Self::Shl
            | Self::Shr
            | Self::CastBool
            | Self::CastInt
            | Self::CastDouble
            | Self::CastString
            | Self::CastVec
            | Self::CastDict
            | Self::CastKeyset
            | Self::InstanceOf
            | Self::InstanceOfD(_)
            | Self::IsLateBoundCls
            | Self::IsTypeStructC(_)
            | Self::ThrowAsTypeStructException
            | Self::CombineAndResolveTypeStruct(_)
            | Self::Print
            | Self::Clone
            | Self::Exit
            | Self::Fatal(_)
            | Self::ResolveFunc(_)
            | Self::ResolveRFunc(_)
            | Self::ResolveMethCaller(_)
            | Self::ResolveClsMethod(_)
            | Self::ResolveClsMethodD(_, _)
            | Self::ResolveClsMethodS(_, _)
            | Self::ResolveRClsMethod(_)
            | Self::ResolveRClsMethodD(_, _)
            | Self::ResolveRClsMethodS(_, _)
            | Self::ResolveClass(_)
            | Self::Continue(_)
            | Self::Break(_)
            | Self::NewObj
            | Self::NewObjR
            | Self::NewObjD(_)
            | Self::NewObjRD(_)
            | Self::NewObjS(_)
            | Self::CGetL(_)
            | Self::CGetQuietL(_)
            | Self::CGetL2(_)
            | Self::CUGetL(_)
            | Self::PushL(_)
            | Self::CGetG
            | Self::CGetS(_)
            | Self::ClassGetC
            | Self::ClassGetTS
            | Self::SetL(_)
            | Self::PopL(_)
            | Self::SetG
            | Self::SetS(_)
            | Self::SetOpL(_, _)
            | Self::SetOpG(_)
            | Self::SetOpS(_)
            | Self::IncDecL(_, _)
            | Self::IncDecG(_)
            | Self::IncDecS(_)
            | Self::UnsetL(_)
            | Self::UnsetG
            | Self::CheckProp(_)
            | Self::InitProp(_, _)
            | Self::IssetL(_)
            | Self::IssetG
            | Self::IssetS
            | Self::IsUnsetL(_)
            | Self::IsTypeC(_)
            | Self::IsTypeL(_, _)
            | Self::BaseGC(_, _)
            | Self::BaseGL(_, _)
            | Self::BaseSC(_, _, _, _)
            | Self::BaseL(_, _, _)
            | Self::BaseC(_, _)
            | Self::BaseH
            | Self::Dim(_, _)
            | Self::QueryM(_, _, _)
            | Self::SetM(_, _)
            | Self::IncDecM(_, _, _)
            | Self::SetOpM(_, _, _)
            | Self::UnsetM(_, _)
            | Self::SetRangeM(_, _, _)
            | Self::Label(_)
            | Self::TryCatchBegin
            | Self::TryCatchMiddle
            | Self::TryCatchEnd
            | Self::Comment(_)
            | Self::SrcLoc(_)
            | Self::WHResult
            | Self::Await
            | Self::AwaitAll(_)
            | Self::CreateCont
            | Self::ContEnter
            | Self::ContRaise
            | Self::Yield
            | Self::YieldK
            | Self::ContCheck(_)
            | Self::ContValid
            | Self::ContKey
            | Self::ContGetReturn
            | Self::ContCurrent
            | Self::Incl
            | Self::InclOnce
            | Self::Req
            | Self::ReqOnce
            | Self::ReqDoc
            | Self::Eval => &mut [],
        }
    }
}
