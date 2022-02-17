// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use ffi::{
    BumpSliceMut,
    Maybe::{self, *},
    Pair, Slice, Str,
};
use hhvm_hhbc_defs_ffi::ffi::{FCallArgsFlags, FatalOp, InitPropOp, IsTypeOp};
use iterator::IterId;
use label::Label;
use local::Local;

#[derive(Clone, Copy, Debug, Eq, PartialEq)]
#[repr(C)]
pub enum CheckStarted {
    IgnoreStarted,
    CheckStarted,
}

#[derive(Clone, Copy, Debug, Eq, PartialEq)]
#[repr(C)]
pub enum FreeIterator {
    IgnoreIter,
    FreeIter,
}

/// see runtime/base/repo-auth-type.h
pub type RepoAuthType<'arena> = Str<'arena>;

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
pub type ConstId<'arena> = hhbc_id::r#const::ConstType<'arena>;
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

#[derive(Clone, Copy, Debug, Eq, PartialEq)]
#[repr(C)]
pub enum SpecialClsRef {
    Static,
    Self_,
    Parent,
}

#[derive(Clone, Copy, Debug, Eq, PartialEq)]
#[repr(C)]
pub enum MemberOpMode {
    ModeNone,
    Warn,
    Define,
    Unset,
    InOut,
}

#[derive(Clone, Copy, Debug, Eq, PartialEq)]
#[repr(C)]
pub enum QueryOp {
    CGet,
    CGetQuiet,
    Isset,
    InOut,
}

#[derive(Clone, Copy, Debug, Eq, PartialEq)]
#[repr(C)]
pub enum CollectionType {
    Vector,
    Map,
    Set,
    Pair,
    ImmVector,
    ImmMap,
    ImmSet,
    Dict,
    Array,
    Keyset,
    Vec,
}

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
pub enum InstructBasic {
    Nop,
    EntryNop,
    PopC,
    PopU,
    Dup,
}

#[derive(Clone, Copy, Debug, Eq, PartialEq)]
#[repr(C)]
pub enum TypeStructResolveOp {
    Resolve,
    DontResolve,
}

#[derive(Clone, Copy, Debug, Eq, PartialEq)]
#[repr(C)]
pub enum ReadonlyOp {
    Readonly,
    Mutable,
    Any,
    CheckROCOW,
    CheckMutROCOW,
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
pub enum IsLogAsDynamicCallOp {
    LogAsDynamicCall,
    DontLogAsDynamicCall,
}

#[derive(Clone, Debug)]
#[repr(C)]
pub enum InstructLitConst<'arena> {
    Null,
    True,
    False,
    NullUninit,
    Int(i64),
    Double(Str<'arena>),
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
}

#[derive(Clone, Debug)]
#[repr(C)]
pub enum InstructOperator<'arena> {
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
}

#[derive(Clone, Copy, Debug, Eq, PartialEq)]
#[repr(C)]
pub enum SwitchKind {
    Bounded,
    Unbounded,
}

#[derive(Clone, Debug)]
#[repr(C)]
pub enum InstructControlFlow<'arena> {
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
}

impl InstructControlFlow<'_> {
    pub fn targets(&self) -> &[Label] {
        match self {
            Self::Jmp(x) | Self::JmpNS(x) | Self::JmpZ(x) | Self::JmpNZ(x) => {
                std::slice::from_ref(x)
            }
            Self::Switch { targets, .. } | Self::SSwitch { targets, .. } => targets.as_ref(),
            Self::RetC | Self::RetCSuspended | Self::RetM(_) | Self::Throw => &[],
        }
    }

    pub fn targets_mut(&mut self) -> &mut [Label] {
        match self {
            Self::Jmp(x) | Self::JmpNS(x) | Self::JmpZ(x) | Self::JmpNZ(x) => {
                std::slice::from_mut(x)
            }
            Self::Switch { targets, .. } | Self::SSwitch { targets, .. } => targets.as_mut(),
            Self::RetC | Self::RetCSuspended | Self::RetM(_) | Self::Throw => &mut [],
        }
    }
}

#[derive(Clone, Debug)]
#[repr(C)]
pub enum InstructSpecialFlow {
    Continue(isize),
    Break(isize),
}

#[derive(Clone, Debug)]
#[repr(C)]
pub enum InstructGet<'arena> {
    CGetL(Local<'arena>),
    CGetQuietL(Local<'arena>),
    CGetL2(Local<'arena>),
    CUGetL(Local<'arena>),
    PushL(Local<'arena>),
    CGetG,
    CGetS(ReadonlyOp),
    ClassGetC,
    ClassGetTS,
}

#[derive(Clone, Debug)]
#[repr(C)]
pub enum InstructIsset<'arena> {
    IssetC,
    IssetL(Local<'arena>),
    IssetG,
    IssetS,
    IsUnsetL(Local<'arena>),
    IsTypeC(IsTypeOp),
    IsTypeL(Local<'arena>, IsTypeOp),
}

#[derive(Clone, Copy, Debug, Eq, PartialEq)]
#[repr(C)]
pub enum SetRangeOp {
    Forward,
    Reverse,
}

#[derive(Clone, Copy, Debug, Eq, PartialEq)]
#[repr(C)]
pub enum EqOp {
    PlusEqual,
    MinusEqual,
    MulEqual,
    ConcatEqual,
    DivEqual,
    PowEqual,
    ModEqual,
    AndEqual,
    OrEqual,
    XorEqual,
    SlEqual,
    SrEqual,
    PlusEqualO,
    MinusEqualO,
    MulEqualO,
}

#[derive(Clone, Copy, Debug, Eq, PartialEq)]
#[repr(C)]
pub enum IncDecOp {
    PreInc,
    PostInc,
    PreDec,
    PostDec,
    PreIncO,
    PostIncO,
    PreDecO,
    PostDecO,
}

#[derive(Clone, Debug)]
#[repr(C)]
pub enum InstructMutator<'arena> {
    SetL(Local<'arena>),
    /// PopL is put in mutators since it behaves as SetL + PopC
    PopL(Local<'arena>),
    SetG,
    SetS(ReadonlyOp),
    SetOpL(Local<'arena>, EqOp),
    SetOpG(EqOp),
    SetOpS(EqOp),
    IncDecL(Local<'arena>, IncDecOp),
    IncDecG(IncDecOp),
    IncDecS(IncDecOp),
    UnsetL(Local<'arena>),
    UnsetG,
    CheckProp(PropId<'arena>),
    InitProp(PropId<'arena>, InitPropOp),
}

#[derive(Clone, Debug, Eq, PartialEq)]
#[repr(C)]
pub enum ObjNullFlavor {
    NullThrows,
    NullSafe,
}

#[derive(Clone, Debug)]
#[repr(C)]
pub enum InstructCall<'arena> {
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
        flavor: ObjNullFlavor,
    },
    FCallObjMethodD {
        fcall_args: FcallArgs<'arena>,
        flavor: ObjNullFlavor,
        method: MethodId<'arena>,
    },
}

#[derive(Clone, Debug)]
#[repr(C)]
pub enum InstructNew<'arena> {
    NewObj,
    NewObjR,
    NewObjD(ClassId<'arena>),
    NewObjRD(ClassId<'arena>),
    NewObjS(SpecialClsRef),
}

impl<'arena> InstructCall<'arena> {
    pub fn fcall_args(&self) -> &FcallArgs<'arena> {
        match self {
            Self::FCallClsMethod { fcall_args, .. }
            | Self::FCallClsMethodD { fcall_args, .. }
            | Self::FCallClsMethodS { fcall_args, .. }
            | Self::FCallClsMethodSD { fcall_args, .. }
            | Self::FCallCtor(fcall_args)
            | Self::FCallFunc(fcall_args)
            | Self::FCallFuncD { fcall_args, .. }
            | Self::FCallObjMethod { fcall_args, .. }
            | Self::FCallObjMethodD { fcall_args, .. } => fcall_args,
        }
    }

    pub fn fcall_args_mut(&mut self) -> &mut FcallArgs<'arena> {
        match self {
            Self::FCallClsMethod { fcall_args, .. }
            | Self::FCallClsMethodD { fcall_args, .. }
            | Self::FCallClsMethodS { fcall_args, .. }
            | Self::FCallClsMethodSD { fcall_args, .. }
            | Self::FCallCtor(fcall_args)
            | Self::FCallFunc(fcall_args)
            | Self::FCallFuncD { fcall_args, .. }
            | Self::FCallObjMethod { fcall_args, .. }
            | Self::FCallObjMethodD { fcall_args, .. } => fcall_args,
        }
    }

    pub fn targets(&self) -> &[Label] {
        self.fcall_args().targets()
    }

    pub fn targets_mut(&mut self) -> &mut [Label] {
        self.fcall_args_mut().targets_mut()
    }
}

#[derive(Clone, Debug)]
#[repr(C)]
pub enum InstructBase<'arena> {
    BaseGC(StackIndex, MemberOpMode),
    BaseGL(Local<'arena>, MemberOpMode),
    BaseSC(StackIndex, StackIndex, MemberOpMode, ReadonlyOp),
    BaseL(Local<'arena>, MemberOpMode, ReadonlyOp),
    BaseC(StackIndex, MemberOpMode),
    BaseH,
    Dim(MemberOpMode, MemberKey<'arena>),
}

#[derive(Clone, Debug)]
#[repr(C)]
pub enum InstructFinal<'arena> {
    QueryM(NumParams, QueryOp, MemberKey<'arena>),
    SetM(NumParams, MemberKey<'arena>),
    IncDecM(NumParams, IncDecOp, MemberKey<'arena>),
    SetOpM(NumParams, EqOp, MemberKey<'arena>),
    UnsetM(NumParams, MemberKey<'arena>),
    SetRangeM(NumParams, isize, SetRangeOp),
}

#[derive(Clone, Debug)]
#[repr(C)]
pub enum InstructIterator<'arena> {
    IterInit(IterArgs<'arena>, Label),
    IterNext(IterArgs<'arena>, Label),
    IterFree(IterId),
}

impl InstructIterator<'_> {
    pub fn targets(&self) -> &[Label] {
        match self {
            Self::IterInit(_, target) | Self::IterNext(_, target) => std::slice::from_ref(target),
            Self::IterFree(_) => &[],
        }
    }

    pub fn targets_mut(&mut self) -> &mut [Label] {
        match self {
            Self::IterInit(_, target) | Self::IterNext(_, target) => std::slice::from_mut(target),
            Self::IterFree(_) => &mut [],
        }
    }
}

#[derive(Clone, Debug)]
#[repr(C)]
pub enum InstructIncludeEvalDefine {
    Incl,
    InclOnce,
    Req,
    ReqOnce,
    ReqDoc,
    Eval,
}

#[derive(Clone, Copy, Debug, Eq, PartialEq)]
#[repr(C)]
pub enum BareThisOp {
    Notice,
    NoNotice,
    NeverNull,
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

#[derive(Clone, Copy, Debug, Eq, PartialEq)]
#[repr(C)]
pub enum OpSilence {
    Start,
    End,
}

#[derive(Clone, Debug)]
#[repr(C)]
pub enum InstructMisc<'arena> {
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
    Self_,
    Parent,
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
    Silence(Local<'arena>, OpSilence),
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
}

impl InstructMisc<'_> {
    pub fn targets(&self) -> &[Label] {
        match self {
            Self::MemoGet(target, _) => std::slice::from_ref(target),
            Self::MemoGetEager(targets, _) => targets,

            // Make sure new variants with branch target Labels are handled above
            // before adding items to this catch-all.
            Self::This
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
            | Self::Self_
            | Self::Parent
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
            | Self::SetImplicitContextByValue => &[],
        }
    }

    pub fn targets_mut(&mut self) -> &mut [Label] {
        match self {
            Self::MemoGet(target, _) => std::slice::from_mut(target),
            Self::MemoGetEager(targets, _) => targets,

            // Make sure new variants with branch target Labels are handled above
            // before adding items to this catch-all.
            Self::This
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
            | Self::Self_
            | Self::Parent
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
            | Self::SetImplicitContextByValue => &mut [],
        }
    }
}

#[derive(Clone, Debug)]
#[repr(C)]
pub enum GenCreationExecution {
    CreateCont,
    ContEnter,
    ContRaise,
    Yield,
    YieldK,
    ContCheck(CheckStarted),
    ContValid,
    ContKey,
    ContGetReturn,
    ContCurrent,
}

#[derive(Clone, Debug)]
#[repr(C)]
pub enum AsyncFunctions<'arena> {
    WHResult,
    Await,
    AwaitAll(Maybe<Pair<Local<'arena>, isize>>),
}

#[derive(Clone, Copy, Debug, Eq, PartialEq)]
#[repr(C)]
pub enum InstructTry {
    TryCatchBegin,
    TryCatchMiddle,
    TryCatchEnd,
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
    Basic(InstructBasic),
    Iterator(InstructIterator<'arena>),
    LitConst(InstructLitConst<'arena>),
    Op(InstructOperator<'arena>),
    ContFlow(InstructControlFlow<'arena>),
    SpecialFlow(InstructSpecialFlow),
    Call(InstructCall<'arena>),
    New(InstructNew<'arena>),
    Misc(InstructMisc<'arena>),
    Get(InstructGet<'arena>),
    Mutator(InstructMutator<'arena>),
    Isset(InstructIsset<'arena>),
    Base(InstructBase<'arena>),
    Final(InstructFinal<'arena>),
    Label(Label),
    Try(InstructTry),
    Comment(Str<'arena>),
    SrcLoc(SrcLoc),
    Async(AsyncFunctions<'arena>),
    Generator(GenCreationExecution),
    IncludeEvalDefine(InstructIncludeEvalDefine),
}

impl Instruct<'_> {
    /// Return a slice of labels for the conditional branch targets of this instruction.
    /// This excludes the Label in an ILabel instruction, which is not a conditional branch.
    pub fn targets(&self) -> &[Label] {
        match self {
            Self::Call(x) => x.targets(),
            Self::ContFlow(x) => x.targets(),
            Self::Iterator(x) => x.targets(),
            Self::Misc(x) => x.targets(),

            // Make sure new variants with branch target Labels are handled above
            // before adding items to this catch-all.
            Self::Basic(_)
            | Self::LitConst(_)
            | Self::Op(_)
            | Self::SpecialFlow(_)
            | Self::New(_)
            | Self::Get(_)
            | Self::Mutator(_)
            | Self::Isset(_)
            | Self::Base(_)
            | Self::Final(_)
            | Self::Label(_)
            | Self::Try(_)
            | Self::Comment(_)
            | Self::SrcLoc(_)
            | Self::Async(_)
            | Self::Generator(_)
            | Self::IncludeEvalDefine(_) => &[],
        }
    }

    /// Return a mutable slice of labels for the conditional branch targets of this instruction.
    /// This excludes the Label in an ILabel instruction, which is not a conditional branch.
    pub fn targets_mut(&mut self) -> &mut [Label] {
        match self {
            Self::Call(x) => x.targets_mut(),
            Self::ContFlow(x) => x.targets_mut(),
            Self::Iterator(x) => x.targets_mut(),
            Self::Misc(x) => x.targets_mut(),

            // Make sure new variants with branch target Labels are handled above
            // before adding items to this catch-all.
            Self::Basic(_)
            | Self::LitConst(_)
            | Self::Op(_)
            | Self::SpecialFlow(_)
            | Self::New(_)
            | Self::Get(_)
            | Self::Mutator(_)
            | Self::Isset(_)
            | Self::Base(_)
            | Self::Final(_)
            | Self::Label(_)
            | Self::Try(_)
            | Self::Comment(_)
            | Self::SrcLoc(_)
            | Self::Async(_)
            | Self::Generator(_)
            | Self::IncludeEvalDefine(_) => &mut [],
        }
    }
}
