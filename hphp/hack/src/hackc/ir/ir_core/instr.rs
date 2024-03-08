// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

use std::borrow::Cow;

use macros::HasLoc;
use macros::HasLocals;
use macros::HasOperands;
use newtype::newtype_int;
use smallvec::SmallVec;
use strum::Display;

use crate::AsTypeStructExceptionKind;
use crate::BareThisOp;
use crate::BlockId;
use crate::ClassGetCMode;
use crate::ClassName;
use crate::CollectionType;
use crate::ConstName;
use crate::ContCheckOp;
use crate::FCallArgsFlags;
use crate::FatalOp;
use crate::FunctionName;
use crate::GlobalId;
use crate::IncDecOp;
use crate::InitPropOp;
use crate::IsLogAsDynamicCallOp;
use crate::IsTypeOp;
use crate::IterId;
use crate::LocId;
use crate::MOpMode;
use crate::MethodId;
use crate::OODeclExistsOp;
use crate::ObjMethodOp;
use crate::PropId;
use crate::QueryMOp;
use crate::ReadonlyOp;
use crate::SetOpOp;
use crate::SetRangeOp;
use crate::SilenceOp;
use crate::SpecialClsRef;
use crate::SwitchKind;
use crate::TypeStructEnforceKind;
use crate::TypeStructResolveOp;
use crate::UnitBytesId;
use crate::ValueId;
use crate::VarId;

pub trait HasLoc {
    fn loc_id(&self) -> LocId;
}

pub trait HasLocals {
    fn locals(&self) -> &[LocalId];
}

pub trait HasOperands {
    fn operands(&self) -> &[ValueId];
    fn operands_mut(&mut self) -> &mut [ValueId];
}

pub trait HasEdges {
    fn edges(&self) -> &[BlockId];
    fn edges_mut(&mut self) -> &mut [BlockId];
}

pub trait CanThrow {
    fn can_throw(&self) -> bool;
}

/// Some of the Instr layouts are a little unnatural - but there are a few
/// important considerations:
///
///   1. All operands must be in a layout that can be returned as a single slice
///      by the operands() and operands_mut() methods.
///
///   2. All edges must be in a layout that can be returned as a single slice by
///      the edges() and edges_mut() methods.
///
///   3. The internal size of Instrs should be big enough that most of the
///      instructions have no external references but small enough that we're
///      not wasting a ton of space on every instruction.
///
/// FIXME: Right now the size is pretty big (56 bytes). For most instructions
/// the bulk of this seems to be from padding.
///
/// FIXME: Right now there are no types on these instructions - there really
/// should be. Q: Should the type be on the Instr itself or embedded in the
/// Func::instrs table?
#[derive(Clone, Debug, HasLoc, HasLocals, HasOperands, PartialEq, Eq)]
pub enum Instr {
    Call(Box<Call>),
    Hhbc(Hhbc),
    MemberOp(MemberOp),
    Special(Special),
    Terminator(Terminator),
}

impl Instr {
    pub fn is_param(&self) -> bool {
        matches!(self, Instr::Special(Special::Param))
    }

    pub fn is_terminal(&self) -> bool {
        matches!(self, Instr::Terminator(_))
    }

    pub fn is_tombstone(&self) -> bool {
        matches!(self, Instr::Special(Special::Tombstone))
    }

    pub fn is_select(&self) -> bool {
        matches!(self, Instr::Special(Special::Select(..)))
    }
}

impl HasEdges for Instr {
    fn edges(&self) -> &[BlockId] {
        match self {
            Instr::Terminator(t) => t.edges(),
            _ => &[],
        }
    }

    fn edges_mut(&mut self) -> &mut [BlockId] {
        match self {
            Instr::Terminator(t) => t.edges_mut(),
            _ => &mut [],
        }
    }
}

impl CanThrow for Instr {
    fn can_throw(&self) -> bool {
        match self {
            Instr::Call(call) => call.can_throw(),
            Instr::Hhbc(hhbc) => hhbc.can_throw(),
            Instr::MemberOp(_) => true,
            Instr::Special(sp) => sp.can_throw(),
            Instr::Terminator(t) => t.can_throw(),
        }
    }
}

#[derive(Clone, Debug, HasLoc, HasLocals, HasOperands, Display, PartialEq, Eq)]
pub enum Terminator {
    // This is an async call - it's a terminator with one edge for the async
    // return and one edge for the eager return. The async edge takes a single
    // argument which is the function's async value. The eager edge takes a
    // single argument which is the function's return value.
    CallAsync(Box<Call>, [BlockId; 2]),
    #[has_operands(none)]
    Enter(BlockId, LocId),
    Exit(ValueId, LocId),
    Fatal(ValueId, FatalOp, LocId),
    IterInit(IteratorArgs, ValueId),
    #[has_operands(none)]
    IterNext(IteratorArgs),
    #[has_operands(none)]
    Jmp(BlockId, LocId),
    JmpArgs(BlockId, Box<[ValueId]>, LocId),
    JmpOp {
        cond: ValueId,
        pred: Predicate,
        targets: [BlockId; 2],
        loc: LocId,
    },
    // Hmm - should this be the range or should we explicitly list all the
    // locals in range so there's no question...
    // Reasons to make a list:
    // - Then you have a slice of operands like every other instruction, no more
    //   special case.
    // Reason to not make a list:
    // - Lowering to hhbc will be harder because the locals may not be
    //   contiguous the way hhbc needs them.
    #[has_operands(none)]
    MemoGet(MemoGet),
    #[has_operands(none)]
    MemoGetEager(MemoGetEager),
    NativeImpl(LocId),
    Ret(ValueId, LocId),
    RetCSuspended(ValueId, LocId),
    RetM(Box<[ValueId]>, LocId),
    #[has_locals(none)]
    Switch {
        cond: ValueId,
        bounded: SwitchKind,
        base: i64,
        targets: Box<[BlockId]>,
        loc: LocId,
    },
    SSwitch {
        cond: ValueId,
        cases: Box<[UnitBytesId]>,
        targets: Box<[BlockId]>,
        loc: LocId,
    },
    Throw(ValueId, LocId),
    ThrowAsTypeStructException([ValueId; 2], AsTypeStructExceptionKind, LocId),
    Unreachable,
}

impl Terminator {
    // Some instructions produce an 'implied' value on some edges - for example
    // a CallAsync will pass a value to each of its edges, but a MemoGet will
    // only pass a value on a single edge. This returns the number of implied
    // args passed along the given edge.
    pub fn implied_arg_count(&self, target: BlockId) -> usize {
        match self {
            Terminator::CallAsync(call, edges) => {
                if edges[0] == target {
                    call.num_rets as usize
                } else if edges[1] == target {
                    1
                } else {
                    panic!("unknown target in instr");
                }
            }
            Terminator::MemoGet(get) => {
                if get.no_value_edge() == target {
                    0
                } else {
                    1
                }
            }
            Terminator::MemoGetEager(get) => {
                if get.no_value_edge() == target {
                    0
                } else {
                    1
                }
            }
            _ => 0,
        }
    }
}

impl HasEdges for Terminator {
    fn edges(&self) -> &[BlockId] {
        match self {
            Terminator::CallAsync(_, targets)
            | Terminator::IterInit(IteratorArgs { targets, .. }, _)
            | Terminator::IterNext(IteratorArgs { targets, .. })
            | Terminator::JmpOp { targets, .. } => targets,

            Terminator::Switch { targets, .. } | Terminator::SSwitch { targets, .. } => targets,

            Terminator::Exit(..)
            | Terminator::Fatal(..)
            | Terminator::NativeImpl(..)
            | Terminator::Ret(..)
            | Terminator::RetCSuspended(..)
            | Terminator::RetM(..)
            | Terminator::Throw(..)
            | Terminator::ThrowAsTypeStructException(..)
            | Terminator::Unreachable => &[],

            Terminator::Enter(bid, _)
            | Terminator::Jmp(bid, _)
            | Terminator::JmpArgs(bid, _, _) => std::slice::from_ref(bid),

            Terminator::MemoGet(get) => get.edges(),
            Terminator::MemoGetEager(get) => get.edges(),
        }
    }

    fn edges_mut(&mut self) -> &mut [BlockId] {
        match self {
            Terminator::CallAsync(_, targets)
            | Terminator::IterInit(IteratorArgs { targets, .. }, _)
            | Terminator::IterNext(IteratorArgs { targets, .. })
            | Terminator::JmpOp { targets, .. } => targets,

            Terminator::Switch { targets, .. } | Terminator::SSwitch { targets, .. } => targets,

            Terminator::Exit(..)
            | Terminator::Fatal(..)
            | Terminator::NativeImpl(..)
            | Terminator::Ret(..)
            | Terminator::RetCSuspended(..)
            | Terminator::RetM(..)
            | Terminator::Throw(..)
            | Terminator::ThrowAsTypeStructException(..)
            | Terminator::Unreachable => &mut [],

            Terminator::Enter(bid, _)
            | Terminator::Jmp(bid, _)
            | Terminator::JmpArgs(bid, _, _) => std::slice::from_mut(bid),

            Terminator::MemoGet(get) => get.edges_mut(),
            Terminator::MemoGetEager(get) => get.edges_mut(),
        }
    }
}

impl CanThrow for Terminator {
    fn can_throw(&self) -> bool {
        match self {
            Terminator::CallAsync(call, _) => call.can_throw(),

            Terminator::Enter(..)
            | Terminator::Exit(..)
            | Terminator::Jmp(..)
            | Terminator::JmpArgs(..)
            | Terminator::MemoGet(..)
            | Terminator::MemoGetEager(..)
            | Terminator::Ret(..)
            | Terminator::RetCSuspended(..)
            | Terminator::RetM(..) => false,

            Terminator::Fatal(..)
            | Terminator::IterInit(..)
            | Terminator::IterNext(..)
            | Terminator::JmpOp { .. }
            | Terminator::NativeImpl(..)
            | Terminator::Switch { .. }
            | Terminator::SSwitch { .. }
            | Terminator::Throw(..)
            | Terminator::ThrowAsTypeStructException(..)
            | Terminator::Unreachable => true,
        }
    }
}

#[derive(Clone, Debug, HasLocals, HasLoc, PartialEq, Eq)]
pub struct MemoGet {
    pub edges: [BlockId; 2],
    pub locals: Box<[LocalId]>,
    pub loc: LocId,
}

impl MemoGet {
    pub fn new(
        value_edge: BlockId,
        no_value_edge: BlockId,
        locals: &[LocalId],
        loc: LocId,
    ) -> Self {
        MemoGet {
            edges: [value_edge, no_value_edge],
            locals: locals.into(),
            loc,
        }
    }

    pub fn value_edge(&self) -> BlockId {
        self.edges[0]
    }
    pub fn no_value_edge(&self) -> BlockId {
        self.edges[1]
    }
}

impl HasEdges for MemoGet {
    fn edges(&self) -> &[BlockId] {
        &self.edges
    }

    fn edges_mut(&mut self) -> &mut [BlockId] {
        &mut self.edges
    }
}

#[derive(Clone, Debug, HasLoc, HasLocals, PartialEq, Eq)]
pub struct MemoGetEager {
    pub edges: [BlockId; 3],
    pub locals: Box<[LocalId]>,
    pub loc: LocId,
}

impl MemoGetEager {
    pub fn new(
        no_value_edge: BlockId,
        suspended_edge: BlockId,
        eager_edge: BlockId,
        locals: &[LocalId],
        loc: LocId,
    ) -> Self {
        MemoGetEager {
            edges: [no_value_edge, suspended_edge, eager_edge],
            locals: locals.into(),
            loc,
        }
    }

    pub fn no_value_edge(&self) -> BlockId {
        self.edges[0]
    }
    pub fn suspended_edge(&self) -> BlockId {
        self.edges[1]
    }
    pub fn eager_edge(&self) -> BlockId {
        self.edges[2]
    }
}

impl HasEdges for MemoGetEager {
    fn edges(&self) -> &[BlockId] {
        &self.edges
    }

    fn edges_mut(&mut self) -> &mut [BlockId] {
        &mut self.edges
    }
}

/// Hhbc variant parameters should be in the order:
/// ValueId (or [ValueId]), … constants …, LocId
#[derive(Clone, Debug, HasLoc, HasLocals, HasOperands, Display, PartialEq, Eq)]
pub enum Hhbc {
    AKExists([ValueId; 2], LocId),
    Add([ValueId; 2], LocId),
    AddElemC([ValueId; 3], LocId),
    AddNewElemC([ValueId; 2], LocId),
    ArrayIdx([ValueId; 3], LocId),
    ArrayMarkLegacy([ValueId; 2], LocId),
    ArrayUnmarkLegacy([ValueId; 2], LocId),
    Await(ValueId, LocId),
    #[has_operands(none)]
    AwaitAll(Box<[LocalId]>, LocId),
    #[has_operands(none)]
    BareThis(BareThisOp, LocId),
    BitAnd([ValueId; 2], LocId),
    BitNot(ValueId, LocId),
    BitOr([ValueId; 2], LocId),
    BitXor([ValueId; 2], LocId),
    CGetG(ValueId, LocId),
    CGetL(LocalId, LocId),
    CGetQuietL(LocalId, LocId),
    CGetS([ValueId; 2], ReadonlyOp, LocId),
    CUGetL(LocalId, LocId),
    CastBool(ValueId, LocId),
    CastDict(ValueId, LocId),
    CastDouble(ValueId, LocId),
    CastInt(ValueId, LocId),
    CastKeyset(ValueId, LocId),
    CastString(ValueId, LocId),
    CastVec(ValueId, LocId),
    ChainFaults([ValueId; 2], LocId),
    CheckClsReifiedGenericMismatch(ValueId, LocId),
    CheckClsRGSoft(ValueId, LocId),
    CheckProp(PropId, LocId),
    CheckThis(LocId),
    ClassGetC(ValueId, ClassGetCMode, LocId),
    ClassGetTS(ValueId, LocId),
    ClassHasReifiedGenerics(ValueId, LocId),
    ClassName(ValueId, LocId),
    Clone(ValueId, LocId),
    ClsCns(ValueId, ConstName, LocId),
    ClsCnsD(ConstName, ClassName, LocId),
    ClsCnsL(ValueId, LocalId, LocId),
    Cmp([ValueId; 2], LocId),
    CmpOp([ValueId; 2], CmpOp, LocId),
    ColFromArray(ValueId, CollectionType, LocId),
    CombineAndResolveTypeStruct(Box<[ValueId]>, LocId),
    Concat([ValueId; 2], LocId),
    ConcatN(Box<[ValueId]>, LocId),
    #[has_operands(none)]
    ContCheck(ContCheckOp, LocId),
    ConsumeL(LocalId, LocId),
    ContCurrent(LocId),
    ContEnter(ValueId, LocId),
    ContGetReturn(LocId),
    ContKey(LocId),
    ContRaise(ValueId, LocId),
    ContValid(LocId),
    CreateCl {
        operands: Box<[ValueId]>,
        clsid: ClassName,
        loc: LocId,
    },
    CreateCont(LocId),
    CreateSpecialImplicitContext([ValueId; 2], LocId),
    Div([ValueId; 2], LocId),
    EnumClassLabelName(ValueId, LocId),
    GetClsRGProp(ValueId, LocId),
    GetMemoKeyL(LocalId, LocId),
    HasReifiedParent(ValueId, LocId),
    Idx([ValueId; 3], LocId),
    #[has_operands(none)]
    IncDecL(LocalId, IncDecOp, LocId),
    IncDecS([ValueId; 2], IncDecOp, LocId),
    IncludeEval(IncludeEval),
    InitProp(ValueId, PropId, InitPropOp, LocId),
    InstanceOfD(ValueId, ClassName, LocId),
    IsLateBoundCls(ValueId, LocId),
    IsTypeC(ValueId, IsTypeOp, LocId),
    #[has_operands(none)]
    IsTypeL(LocalId, IsTypeOp, LocId),
    IsTypeStructC(
        [ValueId; 2],
        TypeStructResolveOp,
        TypeStructEnforceKind,
        LocId,
    ),
    IssetG(ValueId, LocId),
    IssetL(LocalId, LocId),
    IssetS([ValueId; 2], LocId),
    #[has_operands(none)]
    IterFree(IterId, LocId),
    LateBoundCls(LocId),
    LazyClassFromClass(ValueId, LocId),
    LockObj(ValueId, LocId),
    MemoSet(ValueId, Box<[LocalId]>, LocId),
    MemoSetEager(ValueId, Box<[LocalId]>, LocId),
    Modulo([ValueId; 2], LocId),
    Mul([ValueId; 2], LocId),
    NewDictArray(/* capacity hint */ u32, LocId),
    NewKeysetArray(Box<[ValueId]>, LocId),
    NewObj(ValueId, LocId),
    NewObjD(ClassName, LocId),
    #[has_locals(none)]
    #[has_operands(none)]
    NewObjS(SpecialClsRef, LocId),
    NewPair([ValueId; 2], LocId),
    NewStructDict(Box<[UnitBytesId]>, Box<[ValueId]>, LocId),
    NewVec(Box<[ValueId]>, LocId),
    Not(ValueId, LocId),
    #[has_locals(none)]
    OODeclExists([ValueId; 2], OODeclExistsOp, LocId),
    ParentCls(LocId),
    Pow([ValueId; 2], LocId),
    Print(ValueId, LocId),
    RaiseClassStringConversionNotice(LocId),
    RecordReifiedGeneric(ValueId, LocId),
    ResolveClass(ClassName, LocId),
    ResolveClsMethod(ValueId, MethodId, LocId),
    ResolveClsMethodD(ClassName, MethodId, LocId),
    #[has_locals(none)]
    #[has_operands(none)]
    ResolveClsMethodS(SpecialClsRef, MethodId, LocId),
    ResolveRClsMethod([ValueId; 2], MethodId, LocId),
    #[has_locals(none)]
    ResolveRClsMethodS(ValueId, SpecialClsRef, MethodId, LocId),
    ResolveFunc(FunctionName, LocId),
    ResolveRClsMethodD(ValueId, ClassName, MethodId, LocId),
    ResolveRFunc(ValueId, FunctionName, LocId),
    ResolveMethCaller(FunctionName, LocId),
    SelfCls(LocId),
    SetG([ValueId; 2], LocId),
    SetImplicitContextByValue(ValueId, LocId),
    SetL(ValueId, LocalId, LocId),
    SetOpL(ValueId, LocalId, SetOpOp, LocId),
    SetOpG([ValueId; 2], SetOpOp, LocId),
    SetOpS([ValueId; 3], SetOpOp, LocId),
    SetS([ValueId; 3], ReadonlyOp, LocId),
    Shl([ValueId; 2], LocId),
    Shr([ValueId; 2], LocId),
    #[has_operands(none)]
    Silence(LocalId, SilenceOp, LocId),
    Sub([ValueId; 2], LocId),
    This(LocId),
    ThrowNonExhaustiveSwitch(LocId),
    UnsetG(ValueId, LocId),
    UnsetL(LocalId, LocId),
    VerifyImplicitContextState(LocId),
    VerifyOutType(ValueId, LocalId, LocId),
    VerifyParamType(ValueId, LocalId, LocId),
    VerifyParamTypeTS(ValueId, LocalId, LocId),
    VerifyRetTypeC(ValueId, LocId),
    VerifyRetTypeTS([ValueId; 2], LocId),
    WHResult(ValueId, LocId),
    Yield(ValueId, LocId),
    YieldK([ValueId; 2], LocId),
}

impl CanThrow for Hhbc {
    fn can_throw(&self) -> bool {
        // TODO: Lazy... need to figure out which of these could
        // actually throw or not.
        true
    }
}

#[derive(Debug, HasLoc, Clone, PartialEq, Eq)]
pub enum BaseOp {
    // Get base from value. Has output of base value.
    BaseC {
        mode: MOpMode,
        loc: LocId,
    },
    // Get base from global name. Mutates global.
    BaseGC {
        mode: MOpMode,
        loc: LocId,
    },
    // Get base from $this. Cannot mutate $this.
    BaseH {
        loc: LocId,
    },
    // Get base from local. Mutates local.
    BaseL {
        mode: MOpMode,
        readonly: ReadonlyOp,
        loc: LocId,
    },
    // Get base from static property. Mutates static property.
    BaseSC {
        mode: MOpMode,
        readonly: ReadonlyOp,
        loc: LocId,
    },
    // Not allowed in HHBC - Get base from static property.
    BaseST {
        mode: MOpMode,
        readonly: ReadonlyOp,
        loc: LocId,
        prop: PropId,
    },
}

#[derive(Debug, HasLoc, Clone, PartialEq, Eq)]
pub struct IntermediateOp {
    pub key: MemberKey,
    pub mode: MOpMode,
    pub readonly: ReadonlyOp,
    pub loc: LocId,
}

#[derive(Debug, HasLoc, Clone, PartialEq, Eq)]
pub enum FinalOp {
    IncDecM {
        key: MemberKey,
        readonly: ReadonlyOp,
        inc_dec_op: IncDecOp,
        loc: LocId,
    },
    QueryM {
        key: MemberKey,
        readonly: ReadonlyOp,
        query_m_op: QueryMOp,
        loc: LocId,
    },
    SetM {
        key: MemberKey,
        readonly: ReadonlyOp,
        loc: LocId,
    },
    SetRangeM {
        sz: u32,
        set_range_op: SetRangeOp,
        loc: LocId,
    },
    SetOpM {
        key: MemberKey,
        readonly: ReadonlyOp,
        set_op_op: SetOpOp,
        loc: LocId,
    },
    UnsetM {
        key: MemberKey,
        readonly: ReadonlyOp,
        loc: LocId,
    },
}

impl FinalOp {
    pub fn key(&self) -> Option<&MemberKey> {
        match self {
            FinalOp::IncDecM { key, .. }
            | FinalOp::QueryM { key, .. }
            | FinalOp::SetM { key, .. }
            | FinalOp::SetOpM { key, .. }
            | FinalOp::UnsetM { key, .. } => Some(key),
            FinalOp::SetRangeM { .. } => None,
        }
    }

    pub fn is_write(&self) -> bool {
        match self {
            FinalOp::QueryM { .. } => false,
            FinalOp::IncDecM { .. }
            | FinalOp::SetM { .. }
            | FinalOp::SetOpM { .. }
            | FinalOp::UnsetM { .. }
            | FinalOp::SetRangeM { .. } => true,
        }
    }
}

#[derive(Debug, HasLoc, HasLocals, HasOperands, Clone, PartialEq, Eq)]
#[has_loc("base_op")]
pub struct MemberOp {
    pub operands: Box<[ValueId]>,
    pub locals: Box<[LocalId]>,
    pub base_op: BaseOp,
    pub intermediate_ops: Box<[IntermediateOp]>,
    pub final_op: FinalOp,
}

impl MemberOp {
    /// Return the number of values this MemberOp produces. This should match
    /// the number of `Select` Instrs that follow this Instr.
    pub fn num_values(&self) -> usize {
        let (rets_from_final, write_op) = match self.final_op {
            FinalOp::SetRangeM { .. } => (0, true),
            FinalOp::UnsetM { .. } => (0, true),
            FinalOp::IncDecM { .. } => (1, true),
            FinalOp::QueryM { .. } => (1, false),
            FinalOp::SetM { .. } => (1, true),
            FinalOp::SetOpM { .. } => (1, true),
        };

        let base_key_is_element_access = self.intermediate_ops.first().map_or_else(
            || self.final_op.key().map_or(true, |k| k.is_element_access()),
            |dim| dim.key.is_element_access(),
        );

        let rets_from_base = match self.base_op {
            BaseOp::BaseC { .. } => (base_key_is_element_access && write_op) as usize,
            BaseOp::BaseGC { .. }
            | BaseOp::BaseH { .. }
            | BaseOp::BaseL { .. }
            | BaseOp::BaseSC { .. }
            | BaseOp::BaseST { .. } => 0,
        };
        rets_from_base + rets_from_final
    }
}

impl CanThrow for MemberOp {
    fn can_throw(&self) -> bool {
        // TODO: Lazy... need to figure out which of these could actually throw
        // or not.
        true
    }
}

#[derive(Clone, Debug, PartialEq, Eq)]
pub enum CallDetail {
    // A::$b(42);
    // $a::$b(42);
    FCallClsMethod {
        log: IsLogAsDynamicCallOp,
    },
    // A::foo(42);
    FCallClsMethodD {
        clsid: ClassName,
        method: MethodId,
    },
    // $a::foo(42);
    FCallClsMethodM {
        method: MethodId,
        log: IsLogAsDynamicCallOp,
    },
    // self::$a();
    FCallClsMethodS {
        clsref: SpecialClsRef,
    },
    // self::foo();
    FCallClsMethodSD {
        clsref: SpecialClsRef,
        method: MethodId,
    },
    // new A(42);
    FCallCtor,
    // $a(42);
    FCallFunc,
    // foo(42);
    FCallFuncD {
        func: FunctionName,
    },
    // $a->$b(42);
    FCallObjMethod {
        flavor: ObjMethodOp,
    },
    // $a->foo(42);
    FCallObjMethodD {
        flavor: ObjMethodOp,
        method: MethodId,
    },
}

impl CallDetail {
    pub fn args<'a>(&self, operands: &'a [ValueId]) -> &'a [ValueId] {
        let len = operands.len();
        match self {
            CallDetail::FCallClsMethod { .. } => &operands[..len - 2],
            CallDetail::FCallFunc => &operands[..len - 1],
            CallDetail::FCallObjMethod { .. } => &operands[1..len - 1],
            CallDetail::FCallClsMethodM { .. } => &operands[..len - 1],
            CallDetail::FCallClsMethodS { .. } => &operands[..len - 1],
            CallDetail::FCallClsMethodD { .. } => operands,
            CallDetail::FCallClsMethodSD { .. } => operands,
            CallDetail::FCallCtor => &operands[1..],
            CallDetail::FCallFuncD { .. } => operands,
            CallDetail::FCallObjMethodD { .. } => &operands[1..],
        }
    }

    pub fn class(&self, operands: &[ValueId]) -> ValueId {
        let len = operands.len();
        match self {
            CallDetail::FCallClsMethod { .. } | CallDetail::FCallClsMethodM { .. } => {
                operands[len - 1]
            }
            CallDetail::FCallClsMethodD { .. }
            | CallDetail::FCallClsMethodS { .. }
            | CallDetail::FCallClsMethodSD { .. }
            | CallDetail::FCallCtor
            | CallDetail::FCallFunc
            | CallDetail::FCallFuncD { .. }
            | CallDetail::FCallObjMethod { .. }
            | CallDetail::FCallObjMethodD { .. } => {
                panic!("Cannot call 'class' on detail of type {:?}", self)
            }
        }
    }

    pub fn method(&self, operands: &[ValueId]) -> ValueId {
        let len = operands.len();
        match self {
            CallDetail::FCallClsMethod { .. } => operands[len - 2],
            CallDetail::FCallClsMethodS { .. } | CallDetail::FCallObjMethod { .. } => {
                operands[len - 1]
            }
            CallDetail::FCallClsMethodD { .. }
            | CallDetail::FCallClsMethodM { .. }
            | CallDetail::FCallClsMethodSD { .. }
            | CallDetail::FCallCtor
            | CallDetail::FCallFunc
            | CallDetail::FCallFuncD { .. }
            | CallDetail::FCallObjMethodD { .. } => {
                panic!("Cannot call 'method' on detail of type {:?}", self)
            }
        }
    }

    pub fn obj(&self, operands: &[ValueId]) -> ValueId {
        match self {
            CallDetail::FCallCtor
            | CallDetail::FCallObjMethodD { .. }
            | CallDetail::FCallObjMethod { .. } => operands[0],
            CallDetail::FCallClsMethod { .. }
            | CallDetail::FCallClsMethodD { .. }
            | CallDetail::FCallClsMethodM { .. }
            | CallDetail::FCallClsMethodS { .. }
            | CallDetail::FCallClsMethodSD { .. }
            | CallDetail::FCallFunc
            | CallDetail::FCallFuncD { .. } => {
                panic!("Cannot call 'obj' on detail of type {:?}", self)
            }
        }
    }

    pub fn target(&self, operands: &[ValueId]) -> ValueId {
        let len = operands.len();
        match self {
            CallDetail::FCallFunc => operands[len - 1],
            CallDetail::FCallCtor
            | CallDetail::FCallObjMethodD { .. }
            | CallDetail::FCallClsMethodD { .. }
            | CallDetail::FCallClsMethodM { .. }
            | CallDetail::FCallClsMethodS { .. }
            | CallDetail::FCallClsMethod { .. }
            | CallDetail::FCallObjMethod { .. }
            | CallDetail::FCallClsMethodSD { .. }
            | CallDetail::FCallFuncD { .. } => {
                panic!("Cannot call 'target' on detail of type {:?}", self)
            }
        }
    }
}

#[derive(Clone, Debug, HasLoc, HasLocals, HasOperands, PartialEq, Eq)]
#[has_locals(none)]
pub struct Call {
    pub operands: Box<[ValueId]>,
    pub context: UnitBytesId,
    pub detail: CallDetail,
    pub flags: FCallArgsFlags,
    pub num_rets: u32,
    /// For calls with inout parameters this is a sorted list of the indices
    /// of those parameters.
    pub inouts: Option<Box<[u32]>>,
    /// For calls with readonly parameters this is a sorted list of the indices
    /// of those parameters.
    pub readonly: Option<Box<[u32]>>,
    pub loc: LocId,
}

impl Call {
    pub fn args(&self) -> &[ValueId] {
        self.detail.args(&self.operands)
    }

    pub fn obj(&self) -> ValueId {
        self.detail.obj(&self.operands)
    }

    pub fn target(&self) -> ValueId {
        self.detail.target(&self.operands)
    }
}

impl CanThrow for Call {
    fn can_throw(&self) -> bool {
        // TODO: Lazy... need to figure out which of these could actually throw
        // or not.
        true
    }
}

#[derive(Copy, Clone, Debug, PartialEq, Eq)]
pub enum IncludeKind {
    Eval,
    Include,
    IncludeOnce,
    Require,
    RequireOnce,
    RequireOnceDoc,
}

#[derive(Clone, Debug, HasLoc, HasLocals, HasOperands, PartialEq, Eq)]
#[has_locals(none)]
pub struct IncludeEval {
    pub kind: IncludeKind,
    pub vid: ValueId,
    pub loc: LocId,
}

impl CanThrow for IncludeEval {
    fn can_throw(&self) -> bool {
        // TODO: Lazy... need to figure out which of these could actually throw
        // or not.
        true
    }
}

#[derive(Debug, Clone, PartialEq, Eq)]
pub enum MemberKey {
    // cell from stack as index
    //   $a[foo()]
    EC,
    // immediate
    //   $a[3]
    EI(i64),
    // local (stored in locals array on MemberOp) as index
    //   $a[$b]
    EL,
    // literal string as index
    //   $a["hello"]
    ET(UnitBytesId),
    // cell from stack as property
    //   $a->(foo())
    PC,
    // local (stored in locals array on MemberOp) as property
    //   $a->($b)
    PL,
    // literal string as property
    //   $a->hello
    PT(PropId),
    // nullsafe PT
    //   $a?->hello
    QT(PropId),
    // new element
    //   $a[]
    W,
}

impl MemberKey {
    pub fn is_element_access(&self) -> bool {
        match self {
            MemberKey::EC | MemberKey::EI(_) | MemberKey::EL | MemberKey::ET(_) | MemberKey::W => {
                true
            }
            MemberKey::PC | MemberKey::PL | MemberKey::PT(_) | MemberKey::QT(_) => false,
        }
    }
}

// The ValueId param is laid out so that we can chop off the optional args and
// always be able to produce a valid operands slice:
//   [optional: key_vid, required: base, optional: value]
//
// For accessors to this fixed-size data it's strongly recommended that it is ok
// to match [x, y, _] but never ok to refer to elems by number [1] -- for the
// latter case always use an accessor.
#[derive(Debug, HasLoc)]
pub struct MemberArgs([ValueId; 3], pub MemberKey, pub LocId);

impl MemberArgs {
    pub fn new(
        base: ValueId,
        (key, key_vid): (MemberKey, Option<ValueId>),
        value: Option<ValueId>,
        loc: LocId,
    ) -> Self {
        let key_vid = key_vid.unwrap_or_else(ValueId::none);
        let value = value.unwrap_or_else(ValueId::none);
        Self([key_vid, base, value], key, loc)
    }

    pub fn base(&self) -> ValueId {
        let Self([_, base, _], _, _) = *self;
        base
    }

    pub fn key(&self) -> &MemberKey {
        let Self(_, key, _) = self;
        key
    }

    pub fn key_vid(&self) -> Option<ValueId> {
        let Self([vid, _, _], _, _) = *self;
        if vid.is_none() { None } else { Some(vid) }
    }

    pub fn value(&self) -> Option<ValueId> {
        let Self([_, _, value], _, _) = *self;
        if value.is_none() { None } else { Some(value) }
    }
}

impl HasOperands for MemberArgs {
    fn operands(&self) -> &[ValueId] {
        let Self(ops, _, _) = self;
        let a = ops[0].is_none() as usize;
        let b = !ops[2].is_none() as usize + 2;
        &ops[a..b]
    }

    fn operands_mut(&mut self) -> &mut [ValueId] {
        let Self(ops, _, _) = self;
        let a = ops[0].is_none() as usize;
        let b = !ops[2].is_none() as usize + 2;
        &mut ops[a..b]
    }
}

impl CanThrow for MemberArgs {
    fn can_throw(&self) -> bool {
        // TODO: Lazy... need to figure out which of these could actually throw
        // or not.
        true
    }
}

#[derive(Copy, Clone, Debug, PartialEq, Eq)]
pub enum Predicate {
    NonZero,
    Zero,
}

#[derive(Clone, Debug, HasLoc, PartialEq, Eq)]
pub struct IteratorArgs {
    pub iter_id: IterId,
    // Stored as [ValueLid, KeyLid]
    pub locals: SmallVec<[LocalId; 2]>,
    pub targets: [BlockId; 2],
    pub loc: LocId,
}

impl IteratorArgs {
    pub fn new(
        iter_id: IterId,
        key_lid: Option<LocalId>,
        value_lid: LocalId,
        done_bid: BlockId,
        next_bid: BlockId,
        loc: LocId,
    ) -> Self {
        let mut locals = SmallVec::new();
        locals.push(value_lid);
        if let Some(key_lid) = key_lid {
            locals.push(key_lid);
        }
        IteratorArgs {
            iter_id,
            locals,
            targets: [done_bid, next_bid],
            loc,
        }
    }

    pub fn key_lid(&self) -> Option<LocalId> {
        self.locals.get(1).copied()
    }

    pub fn value_lid(&self) -> LocalId {
        self.locals[0]
    }

    pub fn done_bid(&self) -> BlockId {
        self.targets[0]
    }

    pub fn next_bid(&self) -> BlockId {
        self.targets[1]
    }
}

impl HasLocals for IteratorArgs {
    fn locals(&self) -> &[LocalId] {
        self.locals.as_slice()
    }
}

#[derive(Copy, Clone, Debug, PartialEq, Eq)]
pub enum CmpOp {
    Eq,
    Gt,
    Gte,
    Lt,
    Lte,
    NSame,
    Neq,
    Same,
}

/// Instructions used by the SSA pass.
#[derive(Clone, Debug, HasLoc, HasLocals, HasOperands, PartialEq, Eq)]
pub enum Tmp {
    SetVar(VarId, ValueId), // var, value
    GetVar(VarId),          // var
}

/// Instructions used during ir_to_bc.
#[derive(Clone, Debug, HasLoc, HasLocals, HasOperands, PartialEq, Eq)]
pub enum IrToBc {
    PopC,
    PopL(LocalId),
    PushL(LocalId),
    PushConstant(ValueId),
    PushUninit,
    UnsetL(LocalId),
}

/// Instructions used during conversions/textual.
#[derive(Clone, Debug, HasLoc, HasLocals, HasOperands, PartialEq, Eq)]
pub enum Textual {
    /// If the expression is not true then halt execution along this path.
    /// (this is what Textual calls 'prune')
    #[has_locals(none)]
    AssertTrue(ValueId, LocId),
    /// If the expression is true then halt execution along this path.
    /// (this is what Textual calls 'prune not')
    #[has_locals(none)]
    AssertFalse(ValueId, LocId),
    /// Special Deref marker for a variable.
    #[has_operands(none)]
    #[has_loc(none)]
    Deref(LocalId),
    /// Special call to a Hack builtin function (like `hack_string`) skipping
    /// the normal Hack function ABI.
    #[has_locals(none)]
    HackBuiltin {
        target: Cow<'static, str>,
        values: Box<[ValueId]>,
        loc: LocId,
    },
    #[has_operands(none)]
    #[has_loc(none)]
    #[has_locals(none)]
    LoadGlobal { id: GlobalId, is_const: bool },
    /// Literal String
    #[has_operands(none)]
    #[has_loc(none)]
    #[has_locals(none)]
    String(UnitBytesId),
}

impl Textual {
    pub fn deref(lid: LocalId) -> Instr {
        Instr::Special(Special::Textual(Textual::Deref(lid)))
    }
}

#[derive(Clone, Debug, HasLoc, HasLocals, HasOperands, PartialEq, Eq)]
pub enum Special {
    Copy(ValueId),
    IrToBc(IrToBc),
    Param,
    Select(ValueId, u32),
    Textual(Textual),
    // Used to build SSA.
    #[has_locals(none)]
    #[has_loc(none)]
    Tmp(Tmp),
    Tombstone,
}

impl CanThrow for Special {
    fn can_throw(&self) -> bool {
        // TODO: Lazy... need to figure out which of these could actually throw
        // or not.
        true
    }
}

impl Instr {
    pub fn call(call: Call) -> Instr {
        Instr::Call(Box::new(call))
    }

    pub fn simple_call(func: FunctionName, operands: &[ValueId], loc: LocId) -> Instr {
        Self::call(Call {
            operands: operands.into(),
            context: UnitBytesId::EMPTY,
            detail: CallDetail::FCallFuncD { func },
            flags: FCallArgsFlags::default(),
            num_rets: 0,
            inouts: None,
            readonly: None,
            loc,
        })
    }

    pub fn simple_method_call(
        method: MethodId,
        receiver: ValueId,
        operands: &[ValueId],
        loc: LocId,
    ) -> Instr {
        Self::call(Call {
            operands: std::iter::once(receiver)
                .chain(operands.iter().copied())
                .collect(),
            context: UnitBytesId::EMPTY,
            detail: CallDetail::FCallObjMethodD {
                flavor: ObjMethodOp::NullThrows,
                method,
            },
            flags: FCallArgsFlags::default(),
            num_rets: 0,
            inouts: None,
            readonly: None,
            loc,
        })
    }

    pub fn method_call_special(
        clsref: SpecialClsRef,
        method: MethodId,
        operands: &[ValueId],
        loc: LocId,
    ) -> Instr {
        Self::call(Call {
            operands: operands.into(),
            context: UnitBytesId::EMPTY,
            detail: CallDetail::FCallClsMethodSD { clsref, method },
            flags: FCallArgsFlags::default(),
            num_rets: 0,
            inouts: None,
            readonly: None,
            loc,
        })
    }

    pub fn jmp(bid: BlockId, loc: LocId) -> Instr {
        Instr::Terminator(Terminator::Jmp(bid, loc))
    }

    pub fn enter(bid: BlockId, loc: LocId) -> Instr {
        Instr::Terminator(Terminator::Enter(bid, loc))
    }

    pub fn jmp_args(bid: BlockId, args: &[ValueId], loc: LocId) -> Instr {
        let args = args.to_vec().into_boxed_slice();
        Instr::Terminator(Terminator::JmpArgs(bid, args, loc))
    }

    pub fn jmp_op(
        cond: ValueId,
        pred: Predicate,
        true_bid: BlockId,
        false_bid: BlockId,
        loc: LocId,
    ) -> Instr {
        Instr::Terminator(Terminator::JmpOp {
            cond,
            pred,
            targets: [true_bid, false_bid],
            loc,
        })
    }

    pub fn param() -> Instr {
        Instr::Special(Special::Param)
    }

    pub fn ret(vid: ValueId, loc: LocId) -> Instr {
        Instr::Terminator(Terminator::Ret(vid, loc))
    }

    pub fn tombstone() -> Instr {
        Instr::Special(Special::Tombstone)
    }

    pub fn copy(value: ValueId) -> Instr {
        Instr::Special(Special::Copy(value))
    }

    pub fn unreachable() -> Instr {
        Instr::Terminator(Terminator::Unreachable)
    }

    pub fn set_var(var: VarId, value: ValueId) -> Instr {
        Instr::Special(Special::Tmp(Tmp::SetVar(var, value)))
    }

    pub fn get_var(var: VarId) -> Instr {
        Instr::Special(Special::Tmp(Tmp::GetVar(var)))
    }
}

newtype_int!(UnnamedLocalId, u32, UnnamedLocalIdMap, UnnamedLocalIdSet);

/// Note: Unlike HHBC the IR named and unnamed are in distinct namespaces.
#[derive(Debug, Copy, Clone, Hash, Eq, PartialEq)]
pub enum LocalId {
    Named(UnitBytesId),
    // The UnnamedLocalIds are just for uniqueness - there are no requirements
    // for these IDs to be contiguous or in any particular order.
    Unnamed(UnnamedLocalId),
}

#[test]
fn check_sizes() {
    use static_assertions::assert_eq_size;
    assert_eq_size!(ValueId, u32);
}
