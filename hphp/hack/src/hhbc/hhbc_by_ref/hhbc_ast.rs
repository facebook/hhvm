// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use hhbc_by_ref_local as local;

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
pub type RepoAuthType<'arena> = &'arena str;

#[derive(Clone, Debug)]
pub enum ParamId<'arena> {
    ParamUnnamed(isize),
    ParamNamed(&'arena str),
}

pub type ParamNum = isize;
pub type StackIndex = isize;
pub type RecordNum = isize;
pub type TypedefNum = isize;
pub type ClassNum = isize;
pub type ConstNum = isize;

pub type ClassId<'arena> = hhbc_by_ref_hhbc_id::class::Type<'arena>;
pub type FunctionId<'arena> = hhbc_by_ref_hhbc_id::function::Type<'arena>;
pub type MethodId<'arena> = hhbc_by_ref_hhbc_id::method::Type<'arena>;
pub type ConstId<'arena> = hhbc_by_ref_hhbc_id::r#const::Type<'arena>;
pub type PropId<'arena> = hhbc_by_ref_hhbc_id::prop::Type<'arena>;

pub type NumParams = usize;
pub type ByRefs<'arena> = &'arena [bool];

bitflags::bitflags! {
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
pub struct FcallArgs<'arena>(
    pub FcallFlags,
    pub NumParams,
    pub NumParams,
    pub ByRefs<'arena>,
    pub Option<hhbc_by_ref_label::Label<'arena>>,
    pub Option<&'arena str>,
);

impl<'arena> FcallArgs<'arena> {
    pub fn new(
        _: &'arena bumpalo::Bump,
        flags: FcallFlags,
        num_rets: usize,
        inouts: &'arena [bool],
        async_eager_label: Option<hhbc_by_ref_label::Label<'arena>>,
        num_args: usize,
        context: Option<&'arena str>,
    ) -> FcallArgs<'arena> {
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
pub struct IterArgs<'arena> {
    pub iter_id: hhbc_by_ref_iterator::Id,
    pub key_id: Option<local::Type<'arena>>,
    pub val_id: local::Type<'arena>,
}

pub type ClassrefId = isize;
/// Conventionally this is "A_" followed by an integer
pub type AdataId<'arena> = &'arena str;
pub type ParamLocations<'arena> = &'arena [isize];

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

#[derive(Clone, Copy, Debug)]
pub enum MemberKey<'arena> {
    EC(StackIndex, ReadOnlyOp),
    EL(local::Type<'arena>, ReadOnlyOp),
    ET(&'arena str, ReadOnlyOp),
    EI(i64, ReadOnlyOp),
    PC(StackIndex, ReadOnlyOp),
    PL(local::Type<'arena>, ReadOnlyOp),
    PT(PropId<'arena>, ReadOnlyOp),
    QT(PropId<'arena>, ReadOnlyOp),
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
pub enum InstructLitConst<'arena> {
    Null,
    True,
    False,
    NullUninit,
    Int(i64),
    Double(&'arena str),
    String(&'arena str),
    LazyClass(ClassId<'arena>),
    /// Pseudo instruction that will get translated into appropraite literal
    /// bytecode, with possible reference to .adata *)
    TypedValue(hhbc_by_ref_runtime::TypedValue<'arena>),
    Vec(AdataId<'arena>),
    Dict(AdataId<'arena>),
    Keyset(AdataId<'arena>),
    /// capacity hint
    NewDictArray(isize),
    NewStructDict(&'arena [&'arena str]),
    NewVec(isize),
    NewKeysetArray(isize),
    NewPair,
    NewRecord(ClassId<'arena>, &'arena [&'arena str]),
    AddElemC,
    AddNewElemC,
    NewCol(CollectionType),
    ColFromArray(CollectionType),
    CnsE(ConstId<'arena>),
    ClsCns(ConstId<'arena>),
    ClsCnsD(ConstId<'arena>, ClassId<'arena>),
    ClsCnsL(local::Type<'arena>),
    File,
    Dir,
    Method,
    FuncCred,
}

#[derive(Clone, Debug)]
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
    IsTypeStructC(TypestructResolveOp),
    ThrowAsTypeStructException,
    CombineAndResolveTypeStruct(isize),
    Print,
    Clone,
    Exit,
    Fatal(FatalOp),
    ResolveFunc(FunctionId<'arena>),
    ResolveRFunc(FunctionId<'arena>),
    ResolveMethCaller(FunctionId<'arena>),
    ResolveObjMethod,
    ResolveClsMethod(MethodId<'arena>),
    ResolveClsMethodD(ClassId<'arena>, MethodId<'arena>),
    ResolveClsMethodS(SpecialClsRef, MethodId<'arena>),
    ResolveRClsMethod(MethodId<'arena>),
    ResolveRClsMethodD(ClassId<'arena>, MethodId<'arena>),
    ResolveRClsMethodS(SpecialClsRef, MethodId<'arena>),
    ResolveClass(ClassId<'arena>),
}

#[derive(Clone, Copy, Debug, Eq, PartialEq)]
pub enum Switchkind {
    Bounded,
    Unbounded,
}

#[derive(Clone, Debug)]
pub enum InstructControlFlow<'arena> {
    Jmp(hhbc_by_ref_label::Label<'arena>),
    JmpNS(hhbc_by_ref_label::Label<'arena>),
    JmpZ(hhbc_by_ref_label::Label<'arena>),
    JmpNZ(hhbc_by_ref_label::Label<'arena>),
    /// bounded, base, offset vector
    Switch(
        Switchkind,
        isize,
        bumpalo::collections::Vec<'arena, hhbc_by_ref_label::Label<'arena>>,
    ),
    /// litstr id / offset vector
    SSwitch(bumpalo::collections::Vec<'arena, (&'arena str, hhbc_by_ref_label::Label<'arena>)>),
    RetC,
    RetCSuspended,
    RetM(NumParams),
    Throw,
}

#[derive(Clone, Debug)]
pub enum InstructSpecialFlow<'arena> {
    Continue(isize),
    Break(isize),
    Goto(&'arena str),
}

#[derive(Clone, Debug)]
pub enum InstructGet<'arena> {
    CGetL(local::Type<'arena>),
    CGetQuietL(local::Type<'arena>),
    CGetL2(local::Type<'arena>),
    CUGetL(local::Type<'arena>),
    PushL(local::Type<'arena>),
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
pub enum InstructIsset<'arena> {
    IssetC,
    IssetL(local::Type<'arena>),
    IssetG,
    IssetS,
    IsUnsetL(local::Type<'arena>),
    IsTypeC(IstypeOp),
    IsTypeL(local::Type<'arena>, IstypeOp),
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
pub enum InstructMutator<'arena> {
    SetL(local::Type<'arena>),
    /// PopL is put in mutators since it behaves as SetL + PopC
    PopL(local::Type<'arena>),
    SetG,
    SetS(ReadOnlyOp),
    SetOpL(local::Type<'arena>, EqOp),
    SetOpG(EqOp),
    SetOpS(EqOp, ReadOnlyOp),
    IncDecL(local::Type<'arena>, IncdecOp),
    IncDecG(IncdecOp),
    IncDecS(IncdecOp, ReadOnlyOp),
    UnsetL(local::Type<'arena>),
    UnsetG,
    CheckProp(PropId<'arena>),
    InitProp(PropId<'arena>, InitpropOp),
}

#[derive(Clone, Debug, Eq, PartialEq)]
pub enum ObjNullFlavor {
    NullThrows,
    NullSafe,
}

#[derive(Clone, Debug)]
pub enum InstructCall<'arena> {
    NewObj,
    NewObjR,
    NewObjD(ClassId<'arena>),
    NewObjRD(ClassId<'arena>),
    NewObjS(SpecialClsRef),
    FCall(FcallArgs<'arena>),
    FCallClsMethod(FcallArgs<'arena>, IsLogAsDynamicCallOp),
    FCallClsMethodD(FcallArgs<'arena>, ClassId<'arena>, MethodId<'arena>),
    FCallClsMethodS(FcallArgs<'arena>, SpecialClsRef),
    FCallClsMethodSD(FcallArgs<'arena>, SpecialClsRef, MethodId<'arena>),
    FCallCtor(FcallArgs<'arena>),
    FCallFunc(FcallArgs<'arena>),
    FCallFuncD(FcallArgs<'arena>, FunctionId<'arena>),
    FCallObjMethod(FcallArgs<'arena>, ObjNullFlavor),
    FCallObjMethodD(FcallArgs<'arena>, ObjNullFlavor, MethodId<'arena>),
}

#[derive(Clone, Debug)]
pub enum InstructBase<'arena> {
    BaseGC(StackIndex, MemberOpMode),
    BaseGL(local::Type<'arena>, MemberOpMode),
    BaseSC(StackIndex, StackIndex, MemberOpMode, ReadOnlyOp),
    BaseL(local::Type<'arena>, MemberOpMode),
    BaseC(StackIndex, MemberOpMode),
    BaseH,
    Dim(MemberOpMode, MemberKey<'arena>),
}

#[derive(Clone, Debug)]
pub enum InstructFinal<'arena> {
    QueryM(NumParams, QueryOp, MemberKey<'arena>),
    SetM(NumParams, MemberKey<'arena>),
    IncDecM(NumParams, IncdecOp, MemberKey<'arena>),
    SetOpM(NumParams, EqOp, MemberKey<'arena>),
    UnsetM(NumParams, MemberKey<'arena>),
    SetRangeM(NumParams, isize, SetrangeOp, ReadOnlyOp),
}

#[derive(Clone, Debug)]
pub enum InstructIterator<'arena> {
    IterInit(IterArgs<'arena>, hhbc_by_ref_label::Label<'arena>),
    IterNext(IterArgs<'arena>, hhbc_by_ref_label::Label<'arena>),
    IterFree(hhbc_by_ref_iterator::Id),
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
pub enum InstructMisc<'arena> {
    This,
    BareThis(BareThisOp),
    CheckThis,
    FuncNumArgs,
    ChainFaults,
    OODeclExists(ClassKind),
    VerifyParamType(ParamId<'arena>),
    VerifyParamTypeTS(ParamId<'arena>),
    VerifyOutType(ParamId<'arena>),
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
    AssertRATL(local::Type<'arena>, RepoAuthType<'arena>),
    AssertRATStk(StackIndex, RepoAuthType<'arena>),
    BreakTraceHint,
    Silence(local::Type<'arena>, OpSilence),
    GetMemoKeyL(local::Type<'arena>),
    CGetCUNop,
    UGetCUNop,
    MemoGet(
        hhbc_by_ref_label::Label<'arena>,
        Option<(local::Type<'arena>, isize)>,
    ),
    MemoGetEager(
        hhbc_by_ref_label::Label<'arena>,
        hhbc_by_ref_label::Label<'arena>,
        Option<(local::Type<'arena>, isize)>,
    ),
    MemoSet(Option<(local::Type<'arena>, isize)>),
    MemoSetEager(Option<(local::Type<'arena>, isize)>),
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
pub enum AsyncFunctions<'arena> {
    WHResult,
    Await,
    AwaitAll(Option<(local::Type<'arena>, isize)>),
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
pub enum Instruct<'arena> {
    IBasic(InstructBasic),
    IIterator(InstructIterator<'arena>),
    ILitConst(InstructLitConst<'arena>),
    IOp(InstructOperator<'arena>),
    IContFlow(InstructControlFlow<'arena>),
    ISpecialFlow(InstructSpecialFlow<'arena>),
    ICall(InstructCall<'arena>),
    IMisc(InstructMisc<'arena>),
    IGet(InstructGet<'arena>),
    IMutator(InstructMutator<'arena>),
    IIsset(InstructIsset<'arena>),
    IBase(InstructBase<'arena>),
    IFinal(InstructFinal<'arena>),
    ILabel(hhbc_by_ref_label::Label<'arena>),
    ITry(InstructTry),
    IComment(&'arena str),
    ISrcLoc(Srcloc),
    IAsync(AsyncFunctions<'arena>),
    IGenerator(GenCreationExecution),
    IIncludeEvalDefine(InstructIncludeEvalDefine),
}
