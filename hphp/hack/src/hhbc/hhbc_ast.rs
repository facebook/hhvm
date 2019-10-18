// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use hhbc_id_rust as hhbc_id;
use label_rust as label;
use local_rust as local;

use env::iterator::Iter;
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
        const SUPPORTS_ASYNC_EAGER_RETURN = 0b0100;
        const LOCK_WHILE_UNWINDING =        0b1000;
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
);

// ported from instruction_sequence in OCaml
impl FcallArgs {
    pub fn new(
        mut flags: Option<FcallFlags>,
        num_rets: Option<usize>,
        by_refs: Option<Vec<bool>>,
        async_eager_label: Option<label::Label>,
        num_args: usize,
    ) -> FcallArgs {
        let flags = *flags.get_or_insert_with(FcallFlags::default);
        let num_rets = num_rets.unwrap_or(1);
        let by_refs = by_refs.unwrap_or(vec![]);

        if !by_refs.is_empty() && by_refs.len() != num_args {
            panic!("length of by_refs must be either zero or num_args");
        }
        FcallArgs(flags, num_args, num_rets, by_refs, async_eager_label)
    }
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
    Empty,
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
}

#[derive(Clone, Copy, Debug, Eq, PartialEq)]
pub enum FatalOp {
    Parse,
    Runtime,
    RuntimeOmitFrame,
}

#[derive(Clone, Debug)]
pub enum MemberKey {
    EC(StackIndex),
    EL(local::Id),
    ET(String),
    EI(i64),
    PC(StackIndex),
    PL(local::Id),
    PT(PropId),
    QT(PropId),
}

#[derive(Clone, Copy, Debug, Eq, PartialEq)]
pub enum InstructBasic {
    Nop,
    EntryNop,
    PopC,
    PopV,
    PopU,
    Dup,
}

#[derive(Clone, Copy, Debug, Eq, PartialEq)]
pub enum TypestructResolveOp {
    Resolve,
    DontResolve,
}

#[derive(Clone, Copy, Debug, Eq, PartialEq)]
pub enum ClsMethResolveOp {
    Warn,
    NoWarn,
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

#[derive(Debug)]
pub enum InstructLitConst {
    Null,
    True,
    False,
    NullUninit,
    Int(i64),
    Double(String),
    String(String),
    /// Pseudo instruction that will get translated into appropraite literal
    /// bytecode, with possible reference to .adata *)
    TypedValue(TypedValue),
    Array(AdataId),
    Vec(AdataId),
    Dict(AdataId),
    Keyset(AdataId),
    NewArray(isize),
    /// capacity hint
    NewMixedArray(isize),
    /// capacity hint
    NewDictArray(isize),
    /// capacity hint
    NewDArray(isize),
    /// capacity hint
    NewLikeArrayL(local::Id, isize),
    /// capacity hint
    NewPackedArray(isize),
    NewStructArray(Vec<String>),
    NewStructDArray(Vec<String>),
    NewStructDict(Vec<String>),
    NewVArray(isize),
    NewVecArray(isize),
    NewKeysetArray(isize),
    NewPair,
    NewRecord(ClassId, Vec<String>),
    NewRecordArray(ClassId, Vec<String>),
    AddElemC,
    AddNewElemC,
    NewCol(CollectionType),
    ColFromArray(CollectionType),
    CnsE(ConstId),
    ClsCns(ConstId),
    ClsCnsD(ConstId, ClassId),
    File,
    Dir,
    Method,
    FuncCred,
}

#[derive(Clone, Debug)]
pub enum InstructOperator {
    Concat,
    ConcatN(isize),
    Abs,
    Add,
    Sub,
    Mul,
    AddO,
    SubO,
    MulO,
    Div,
    Mod,
    Pow,
    Sqrt,
    Xor,
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
    Floor,
    Ceil,
    CastBool,
    CastInt,
    CastDouble,
    CastString,
    CastArray,
    CastVec,
    CastDict,
    CastKeyset,
    CastVArray,
    CastDArray,
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
    ResolveObjMethod,
    ResolveClsMethod(ClsMethResolveOp),
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
    CGetL(local::Id),
    CGetQuietL(local::Id),
    CGetL2(local::Id),
    CUGetL(local::Id),
    PushL(local::Id),
    CGetG,
    CGetS,
    VGetL(local::Id),
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
    OpArr,
    OpObj,
    OpRes,
    OpScalar,
    /// Int or Dbl or Str or Bool
    OpKeyset,
    OpDict,
    OpVec,
    OpArrLike,
    /// Arr or Vec or Dict or Keyset *)
    OpVArray,
    OpDArray,
    OpClsMeth,
    OpFunc,
}

#[derive(Clone, Debug)]
pub enum InstructIsset {
    IssetC,
    IssetL(local::Id),
    IssetG,
    IssetS,
    EmptyL(local::Id),
    EmptyG,
    EmptyS,
    IsTypeC(IstypeOp),
    IsTypeL(local::Id, IstypeOp),
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
    SetL(local::Id),
    /// PopL is put in mutators since it behaves as SetL + PopC
    PopL(local::Type),
    SetG,
    SetS,
    SetOpL(local::Id, EqOp),
    SetOpG(EqOp),
    SetOpS(EqOp),
    IncDecL(local::Id, IncdecOp),
    IncDecG(IncdecOp),
    IncDecS(IncdecOp),
    UnsetL(local::Id),
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
    FCallBuiltin(NumParams, NumParams, NumParams, String),
    FCallClsMethod(FcallArgs, ParamLocations, IsLogAsDynamicCallOp),
    FCallClsMethodD(FcallArgs, ClassId, MethodId),
    FCallClsMethodS(FcallArgs, SpecialClsRef),
    FCallClsMethodSD(FcallArgs, SpecialClsRef, MethodId),
    FCallCtor(FcallArgs),
    FCallFunc(FcallArgs, ParamLocations),
    FCallFuncD(FcallArgs, FunctionId),
    FCallObjMethod(FcallArgs, ObjNullFlavor, ParamLocations),
    FCallObjMethodD(FcallArgs, ObjNullFlavor, MethodId),
}

#[derive(Clone, Debug)]
pub enum InstructBase {
    BaseGC(StackIndex, MemberOpMode),
    BaseGL(local::Id, MemberOpMode),
    BaseSC(StackIndex, StackIndex, MemberOpMode),
    BaseL(local::Id, MemberOpMode),
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
    SetRangeM(NumParams, SetrangeOp, isize),
}

#[derive(Clone, Copy, Debug, Eq, PartialEq)]
pub enum IterKind {
    Iter,
    LIter,
}

#[derive(Clone, Debug)]
pub enum InstructIterator {
    IterInit(Iter, label::Label, local::Id),
    IterInitK(Iter, label::Label, local::Id, local::Id),
    LIterInit(Iter, local::Id, label::Label, local::Id),
    LIterInitK(Iter, local::Id, label::Label, local::Id, local::Id),
    IterNext(Iter, label::Label, local::Id),
    IterNextK(Iter, label::Label, local::Id, local::Id),
    LIterNext(Iter, local::Id, label::Label, local::Id),
    LIterNextK(Iter, local::Id, label::Label, local::Id, local::Id),
    IterFree(Iter),
    LIterFree(Iter, local::Id),
    IterBreak(label::Label, Vec<(IterKind, Iter)>),
}

#[derive(Clone, Debug)]
pub enum InstructIncludeEvalDefine {
    Incl,
    InclOnce,
    Req,
    ReqOnce,
    ReqDoc,
    Eval,
    AliasCls(String, String),
    DefCls(ClassNum),
    DefClsNop(ClassNum),
    DefRecord(RecordNum),
    DefCns(ConstId),
    DefTypeAlias(TypedefNum),
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
    InitThisLoc(local::Id),
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
    AssertRATL(local::Id, RepoAuthType),
    AssertRATStk(StackIndex, RepoAuthType),
    BreakTraceHint,
    Silence(local::Id, OpSilence),
    GetMemoKeyL(local::Id),
    CGetCUNop,
    UGetCUNop,
    MemoGet(label::Label, Option<(local::Id, isize)>),
    MemoGetEager(label::Label, label::Label, Option<(local::Id, isize)>),
    MemoSet(Option<(local::Id, isize)>),
    MemoSetEager(Option<(local::Id, isize)>),
    LockObj,
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
pub enum GenDelegation {
    ContAssignDelegate(Iter),
    ContEnterDelegate,
    YieldFromDelegate(Iter, label::Label),
    ContUnsetDelegate(FreeIterator, Iter),
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

#[derive(Debug)]
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
    IGenDelegation(GenDelegation),
    IIncludeEvalDefine(InstructIncludeEvalDefine),
}
