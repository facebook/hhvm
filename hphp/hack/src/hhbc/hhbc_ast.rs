// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use hhbc_id_rust as hhbc_id;
use label_rust as label;

use iterator::Id as IterId;
use runtime::TypedValue;

extern crate bitflags;
use bitflags::bitflags;

#[derive(Clone, Copy, Debug, Eq, PartialEq)]
pub enum CheckStarted {
    IgnoreStarted,
    CheckStarted,
}

#[derive(Clone, Copy, Debug, Eq, PartialEq)]
pub enum FreeIterator {
    IgnoreIter,
    FreeIter,
}

/// see runtime/base/repo-auth-type.h
pub type RepoAuthType = String;

#[derive(Clone, Debug)]
pub enum ParamId {
    ParamUnnamed(isize),
    ParamNamed(String),
}

pub type ParamNum = isize;

pub type StackIndex = isize;

pub type RecordNum = isize;

pub type TypedefNum = isize;

pub type ClassNum = isize;

pub type ConstNum = isize;

// TODO(hrust) re-think about these lifetimes

pub type ClassId = hhbc_id::class::Type<'static>;

pub type FunctionId = hhbc_id::function::Type<'static>;

pub type MethodId = hhbc_id::method::Type<'static>;

pub type ConstId = hhbc_id::r#const::Type<'static>;

pub type PropId = hhbc_id::prop::Type<'static>;

pub type NumParams = usize;

pub type ByRefs = Vec<bool>;

bitflags! {
    pub struct FcallFlags: u8 {
        const HAS_UNPACK =                  0b0001;
        const HAS_GENERICS =                0b0010;
        const LOCK_WHILE_UNWINDING =        0b0100;
    }
}
impl Default for FcallFlags {
    fn default() -> FcallFlags {
        FcallFlags::empty()
    }
}

#[derive(Clone, Debug)]
pub struct FcallArgs(
    pub FcallFlags,
    pub NumParams,
    pub NumParams,
    pub ByRefs,
    pub Option<label::Label>,
    pub Option<String>,
);

// ported from instruction_sequence in OCaml
impl FcallArgs {
    pub fn new(
        flags: FcallFlags,
        num_rets: usize,
        inouts: Vec<bool>,
        async_eager_label: Option<label::Label>,
        num_args: usize,
        context: Option<String>,
    ) -> FcallArgs {
        if !inouts.is_empty() && inouts.len() != num_args {
            panic!("length of by_refs must be either zero or num_args");
        }
        FcallArgs(
            flags,
            num_args,
            num_rets,
            inouts,
            async_eager_label,
            context,
        )
    }
}

#[derive(Clone, Debug)]
pub struct IterArgs {
    pub iter_id: IterId,
    pub key_id: Option<local::Type>,
    pub val_id: local::Type,
}

pub type ClassrefId = isize;

/// Conventionally this is "A_" followed by an integer
pub type AdataId = String;

pub type ParamLocations = Vec<isize>;

#[derive(Clone, Copy, Debug, Eq, PartialEq)]
pub enum SpecialClsRef {
    Static,
    Self_,
    Parent,
}

#[derive(Clone, Copy, Debug, Eq, PartialEq)]
pub enum MemberOpMode {
    ModeNone,
    Warn,
    Define,
    Unset,
    InOut,
}

#[derive(Clone, Copy, Debug, Eq, PartialEq)]
pub enum QueryOp {
    CGet,
    CGetQuiet,
    Isset,
    InOut,
}

#[derive(Clone, Copy, Debug, Eq, PartialEq)]
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

#[derive(Clone, Copy, Debug, Eq, PartialEq)]
pub enum FatalOp {
    Parse,
    Runtime,
    RuntimeOmitFrame,
}

#[derive(Clone, Debug)]
pub enum MemberKey {
    EC(StackIndex, ReadOnlyOp),
    EL(local::Type, ReadOnlyOp),
    ET(String, ReadOnlyOp),
    EI(i64, ReadOnlyOp),
    PC(StackIndex, ReadOnlyOp),
    PL(local::Type, ReadOnlyOp),
    PT(PropId, ReadOnlyOp),
    QT(PropId, ReadOnlyOp),
    W,
}

#[derive(Clone, Copy, Debug, Eq, PartialEq)]
pub enum InstructBasic {
    Nop,
    EntryNop,
    PopC,
    PopU,
    Dup,
}
#[derive(Clone, Copy, Debug, Eq, PartialEq)]
pub enum TypestructResolveOp {
    Resolve,
    DontResolve,
}

#[derive(Clone, Copy, Debug, Eq, PartialEq)]
pub enum ReadOnlyOp {
    ReadOnly,
    Mutable,
    Any,
}

#[derive(Clone, Copy, Debug, Eq, PartialEq)]
pub enum HasGenericsOp {
    NoGenerics,
    MaybeGenerics,
    HasGenerics,
}

#[derive(Clone, Copy, Debug, Eq, PartialEq)]
pub enum IsLogAsDynamicCallOp {
    LogAsDynamicCall,
    DontLogAsDynamicCall,
}

#[derive(Clone, Debug)]
pub enum InstructLitConst {
    Null,
    True,
    False,
    NullUninit,
    Int(i64),
    Double(String),
    String(String),
    LazyClass(ClassId),
    /// Pseudo instruction that will get translated into appropraite literal
    /// bytecode, with possible reference to .adata *)
    TypedValue(TypedValue),
    Vec(AdataId),
    Dict(AdataId),
    Keyset(AdataId),
    /// capacity hint
    NewDictArray(isize),
    NewStructDict(Vec<String>),
    NewVec(isize),
    NewKeysetArray(isize),
    NewPair,
    NewRecord(ClassId, Vec<String>),
    AddElemC,
    AddNewElemC,
    NewCol(CollectionType),
    ColFromArray(CollectionType),
    CnsE(ConstId),
    ClsCns(ConstId),
    ClsCnsD(ConstId, ClassId),
    ClsCnsL(local::Type),
    File,
    Dir,
    Method,
    FuncCred,
}

#[derive(Clone, Debug)]
pub enum InstructOperator {
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
    InstanceOfD(ClassId),
    IsLateBoundCls,
    IsTypeStructC(TypestructResolveOp),
    ThrowAsTypeStructException,
    CombineAndResolveTypeStruct(isize),
    Print,
    Clone,
    Exit,
    Fatal(FatalOp),
    ResolveFunc(FunctionId),
    ResolveRFunc(FunctionId),
    ResolveMethCaller(FunctionId),
    ResolveObjMethod,
    ResolveClsMethod(MethodId),
    ResolveClsMethodD(ClassId, MethodId),
    ResolveClsMethodS(SpecialClsRef, MethodId),
    ResolveRClsMethod(MethodId),
    ResolveRClsMethodD(ClassId, MethodId),
    ResolveRClsMethodS(SpecialClsRef, MethodId),
    ResolveClass(ClassId),
}

#[derive(Clone, Copy, Debug, Eq, PartialEq)]
pub enum Switchkind {
    Bounded,
    Unbounded,
}

#[derive(Clone, Debug)]
pub enum InstructControlFlow {
    Jmp(label::Label),
    JmpNS(label::Label),
    JmpZ(label::Label),
    JmpNZ(label::Label),
    /// bounded, base, offset vector
    Switch(Switchkind, isize, Vec<label::Label>),
    /// litstr id / offset vector
    SSwitch(Vec<(String, label::Label)>),
    RetC,
    RetCSuspended,
    RetM(NumParams),
    Throw,
}

#[derive(Clone, Debug)]
pub enum InstructSpecialFlow {
    Continue(isize),
    Break(isize),
    Goto(String),
}

#[derive(Clone, Debug)]
pub enum InstructGet {
    CGetL(local::Type),
    CGetQuietL(local::Type),
    CGetL2(local::Type),
    CUGetL(local::Type),
    PushL(local::Type),
    CGetG,
    CGetS(ReadOnlyOp),
    ClassGetC,
    ClassGetTS,
}

#[derive(Clone, Copy, Debug, Eq, PartialEq)]
pub enum IstypeOp {
    OpNull,
    OpBool,
    OpInt,
    OpDbl,
    OpStr,
    OpObj,
    OpRes,
    OpScalar,
    /// Int or Dbl or Str or Bool
    OpKeyset,
    OpDict,
    OpVec,
    OpArrLike,
    /// Arr or Vec or Dict or Keyset *)
    OpClsMeth,
    OpFunc,
    OpLegacyArrLike,
    OpClass,
}

#[derive(Clone, Debug)]
pub enum InstructIsset {
    IssetC,
    IssetL(local::Type),
    IssetG,
    IssetS,
    IsUnsetL(local::Type),
    IsTypeC(IstypeOp),
    IsTypeL(local::Type, IstypeOp),
}

#[derive(Clone, Copy, Debug, Eq, PartialEq)]
pub enum SetrangeOp {
    Forward,
    Reverse,
}

#[derive(Clone, Copy, Debug, Eq, PartialEq)]
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
pub enum IncdecOp {
    PreInc,
    PostInc,
    PreDec,
    PostDec,
    PreIncO,
    PostIncO,
    PreDecO,
    PostDecO,
}

#[derive(Clone, Copy, Debug, Eq, PartialEq)]
pub enum InitpropOp {
    Static,
    NonStatic,
}

#[derive(Clone, Debug)]
pub enum InstructMutator {
    SetL(local::Type),
    /// PopL is put in mutators since it behaves as SetL + PopC
    PopL(local::Type),
    SetG,
    SetS(ReadOnlyOp),
    SetOpL(local::Type, EqOp),
    SetOpG(EqOp),
    SetOpS(EqOp, ReadOnlyOp),
    IncDecL(local::Type, IncdecOp),
    IncDecG(IncdecOp),
    IncDecS(IncdecOp, ReadOnlyOp),
    UnsetL(local::Type),
    UnsetG,
    CheckProp(PropId),
    InitProp(PropId, InitpropOp),
}

#[derive(Clone, Debug, Eq, PartialEq)]
pub enum ObjNullFlavor {
    NullThrows,
    NullSafe,
}

#[derive(Clone, Debug)]
pub enum InstructCall {
    NewObj,
    NewObjR,
    NewObjD(ClassId),
    NewObjRD(ClassId),
    NewObjS(SpecialClsRef),
    FCall(FcallArgs),
    FCallClsMethod(FcallArgs, IsLogAsDynamicCallOp),
    FCallClsMethodD(FcallArgs, ClassId, MethodId),
    FCallClsMethodS(FcallArgs, SpecialClsRef),
    FCallClsMethodSD(FcallArgs, SpecialClsRef, MethodId),
    FCallCtor(FcallArgs),
    FCallFunc(FcallArgs),
    FCallFuncD(FcallArgs, FunctionId),
    FCallObjMethod(FcallArgs, ObjNullFlavor),
    FCallObjMethodD(FcallArgs, ObjNullFlavor, MethodId),
}

#[derive(Clone, Debug)]
pub enum InstructBase {
    BaseGC(StackIndex, MemberOpMode),
    BaseGL(local::Type, MemberOpMode),
    BaseSC(StackIndex, StackIndex, MemberOpMode, ReadOnlyOp),
    BaseL(local::Type, MemberOpMode),
    BaseC(StackIndex, MemberOpMode),
    BaseH,
    Dim(MemberOpMode, MemberKey),
}

#[derive(Clone, Debug)]
pub enum InstructFinal {
    QueryM(NumParams, QueryOp, MemberKey),
    SetM(NumParams, MemberKey),
    IncDecM(NumParams, IncdecOp, MemberKey),
    SetOpM(NumParams, EqOp, MemberKey),
    UnsetM(NumParams, MemberKey),
    SetRangeM(NumParams, isize, SetrangeOp, ReadOnlyOp),
}

#[derive(Clone, Debug)]
pub enum InstructIterator {
    IterInit(IterArgs, label::Label),
    IterNext(IterArgs, label::Label),
    IterFree(IterId),
}

#[derive(Clone, Debug)]
pub enum InstructIncludeEvalDefine {
    Incl,
    InclOnce,
    Req,
    ReqOnce,
    ReqDoc,
    Eval,
}

#[derive(Clone, Copy, Debug, Eq, PartialEq)]
pub enum BareThisOp {
    Notice,
    NoNotice,
    NeverNull,
}

#[derive(Clone, Copy, Debug, Eq, PartialEq)]
pub enum ClassKind {
    Class,
    Interface,
    Trait,
}

#[derive(Clone, Copy, Debug, Eq, PartialEq)]
pub enum OpSilence {
    Start,
    End,
}

#[derive(Clone, Debug)]
pub enum InstructMisc {
    This,
    BareThis(BareThisOp),
    CheckThis,
    FuncNumArgs,
    ChainFaults,
    OODeclExists(ClassKind),
    VerifyParamType(ParamId),
    VerifyParamTypeTS(ParamId),
    VerifyOutType(ParamId),
    VerifyRetTypeC,
    VerifyRetTypeTS,
    Self_,
    Parent,
    LateBoundCls,
    ClassName,
    RecordReifiedGeneric,
    CheckReifiedGenericMismatch,
    NativeImpl,
    AKExists,
    CreateCl(NumParams, ClassNum),
    Idx,
    ArrayIdx,
    ArrayMarkLegacy,
    ArrayUnmarkLegacy,
    AssertRATL(local::Type, RepoAuthType),
    AssertRATStk(StackIndex, RepoAuthType),
    BreakTraceHint,
    Silence(local::Type, OpSilence),
    GetMemoKeyL(local::Type),
    CGetCUNop,
    UGetCUNop,
    MemoGet(label::Label, Option<(local::Type, isize)>),
    MemoGetEager(label::Label, label::Label, Option<(local::Type, isize)>),
    MemoSet(Option<(local::Type, isize)>),
    MemoSetEager(Option<(local::Type, isize)>),
    LockObj,
    ThrowNonExhaustiveSwitch,
    RaiseClassStringConversionWarning,
}

#[derive(Clone, Debug)]
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
pub enum AsyncFunctions {
    WHResult,
    Await,
    AwaitAll(Option<(local::Type, isize)>),
}

#[derive(Clone, Copy, Debug, Eq, PartialEq)]
pub enum InstructTry {
    TryCatchBegin,
    TryCatchMiddle,
    TryCatchEnd,
}

#[derive(Clone, Debug)]
pub struct Srcloc {
    pub line_begin: isize,
    pub col_begin: isize,
    pub line_end: isize,
    pub col_end: isize,
}

#[derive(Clone, Debug)]
pub enum Instruct {
    IBasic(InstructBasic),
    IIterator(InstructIterator),
    ILitConst(InstructLitConst),
    IOp(InstructOperator),
    IContFlow(InstructControlFlow),
    ISpecialFlow(InstructSpecialFlow),
    ICall(InstructCall),
    IMisc(InstructMisc),
    IGet(InstructGet),
    IMutator(InstructMutator),
    IIsset(InstructIsset),
    IBase(InstructBase),
    IFinal(InstructFinal),
    ILabel(label::Label),
    ITry(InstructTry),
    IComment(String),
    ISrcLoc(Srcloc),
    IAsync(AsyncFunctions),
    IGenerator(GenCreationExecution),
    IIncludeEvalDefine(InstructIncludeEvalDefine),
}
