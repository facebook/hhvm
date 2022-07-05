// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

use crate::BlockId;
use crate::ClassId;
use crate::ConstId;
use crate::FunctionId;
use crate::LocId;
use crate::MethodId;
use crate::PropId;
use crate::UnitStringId;
use crate::ValueId;
use macros::HasLoc;
use macros::HasLocals;
use macros::HasOperands;
use newtype::newtype_int;
use smallvec::SmallVec;

// Re-export some types in from hhbc so users of `ir` don't have to figure out
// which random stuff to get from `ir` and which to get elsewhere.
pub use hhbc::BareThisOp;
pub use hhbc::ClassishKind;
pub use hhbc::CollectionType;
pub use hhbc::ContCheckOp;
pub use hhbc::FCallArgsFlags;
pub use hhbc::FatalOp;
pub use hhbc::IncDecOp;
pub use hhbc::InitPropOp;
pub use hhbc::IsLogAsDynamicCallOp;
pub use hhbc::IsTypeOp;
pub use hhbc::IterId;
pub use hhbc::MOpMode;
pub use hhbc::OODeclExistsOp;
pub use hhbc::ObjMethodOp;
pub use hhbc::QueryMOp;
pub use hhbc::ReadonlyOp;
pub use hhbc::SetOpOp;
pub use hhbc::SilenceOp;
pub use hhbc::SpecialClsRef;
pub use hhbc::SrcLoc;
pub use hhbc::TypeStructResolveOp;

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
///
#[derive(Debug, HasLoc, HasLocals, HasOperands)]
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

#[derive(Debug, HasLoc, HasLocals, HasOperands)]
pub enum Terminator {
    // This is an async call - it's a terminator with one edge for the async
    // return and one edge for the eager return. The async edge takes a single
    // argument which is the function's async value. The eager edge takes a
    // single argument which is the function's return value.
    CallAsync(Box<Call>, [BlockId; 2]),
    Exit(ValueId, LocId),
    Fatal(ValueId, FatalOp, LocId),
    IterInit(IteratorArgs, ValueId),
    #[has_operands(none)]
    IterNext(IteratorArgs),
    #[has_operands(none)]
    Jmp(BlockId, SurpriseCheck, LocId),
    JmpArgs(BlockId, SurpriseCheck, Box<[ValueId]>, LocId),
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
    // - Lowering to hhbc_ast will be harder because the locals may not be
    //   contiguous the way hhbc_ast needs them.
    #[has_operands(none)]
    MemoGet(MemoGet),
    #[has_operands(none)]
    MemoGetEager(MemoGetEager),
    NativeImpl(LocId),
    Ret(ValueId, LocId),
    RetCSuspended(ValueId, LocId),
    RetM(Box<[ValueId]>, LocId),
    SSwitch {
        cond: ValueId,
        cases: Box<[UnitStringId]>,
        targets: Box<[BlockId]>,
        loc: LocId,
    },
    Throw(ValueId, LocId),
    ThrowAsTypeStructException([ValueId; 2], LocId),
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

            Terminator::SSwitch { targets, .. } => targets,

            Terminator::Exit(..)
            | Terminator::Fatal(..)
            | Terminator::NativeImpl(..)
            | Terminator::Ret(..)
            | Terminator::RetCSuspended(..)
            | Terminator::RetM(..)
            | Terminator::Throw(..)
            | Terminator::ThrowAsTypeStructException(..)
            | Terminator::Unreachable => &[],

            Terminator::Jmp(bid, _, _) | Terminator::JmpArgs(bid, _, _, _) => {
                std::slice::from_ref(bid)
            }

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

            Terminator::SSwitch { targets, .. } => targets,

            Terminator::Exit(..)
            | Terminator::Fatal(..)
            | Terminator::NativeImpl(..)
            | Terminator::Ret(..)
            | Terminator::RetCSuspended(..)
            | Terminator::RetM(..)
            | Terminator::Throw(..)
            | Terminator::ThrowAsTypeStructException(..)
            | Terminator::Unreachable => &mut [],

            Terminator::Jmp(bid, _, _) | Terminator::JmpArgs(bid, _, _, _) => {
                std::slice::from_mut(bid)
            }

            Terminator::MemoGet(get) => get.edges_mut(),
            Terminator::MemoGetEager(get) => get.edges_mut(),
        }
    }
}

impl CanThrow for Terminator {
    fn can_throw(&self) -> bool {
        match self {
            Terminator::CallAsync(call, _) => call.can_throw(),

            Terminator::Exit(..)
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
            | Terminator::SSwitch { .. }
            | Terminator::Throw(..)
            | Terminator::ThrowAsTypeStructException(..)
            | Terminator::Unreachable => true,
        }
    }
}

#[derive(Debug, HasLocals, HasLoc)]
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

#[derive(Debug, HasLoc, HasLocals)]
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

#[derive(Debug, HasLoc, HasLocals, HasOperands)]
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
    CheckProp(PropId, LocId),
    CheckThis(LocId),
    ClassGetC(ValueId, LocId),
    ClassName(ValueId, LocId),
    Clone(ValueId, LocId),
    ClsCns(ValueId, ConstId, LocId),
    ClsCnsD(ConstId, ClassId, LocId),
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
        clsid: ClassId,
        loc: LocId,
    },
    CreateCont(LocId),
    Div([ValueId; 2], LocId),
    GetMemoKeyL(LocalId, LocId),
    Idx([ValueId; 3], LocId),
    #[has_operands(none)]
    IncDecL(LocalId, IncDecOp, LocId),
    IncDecS([ValueId; 2], IncDecOp, LocId),
    IncludeEval(IncludeEval),
    InitProp(ValueId, PropId, InitPropOp, LocId),
    InstanceOfD(ValueId, ClassId, LocId),
    IsLateBoundCls(ValueId, LocId),
    IsTypeC(ValueId, IsTypeOp, LocId),
    #[has_operands(none)]
    IsTypeL(LocalId, IsTypeOp, LocId),
    IsTypeStructC([ValueId; 2], TypeStructResolveOp, LocId),
    IssetL(LocalId, LocId),
    #[has_operands(none)]
    IterFree(IterId, LocId),
    LateBoundCls(LocId),
    LockObj(ValueId, LocId),
    MemoSet(ValueId, Box<[LocalId]>, LocId),
    MemoSetEager(ValueId, Box<[LocalId]>, LocId),
    Modulo([ValueId; 2], LocId),
    Mul([ValueId; 2], LocId),
    NewDictArray(/* capacity hint */ u32, LocId),
    NewKeysetArray(Box<[ValueId]>, LocId),
    NewObj(ValueId, LocId),
    NewObjD(ClassId, LocId),
    NewObjRD(ValueId, ClassId, LocId),
    #[has_locals(none)]
    #[has_operands(none)]
    NewObjS(SpecialClsRef, LocId),
    NewPair([ValueId; 2], LocId),
    NewStructDict(Box<[UnitStringId]>, Box<[ValueId]>, LocId),
    NewVec(Box<[ValueId]>, LocId),
    Not(ValueId, LocId),
    #[has_locals(none)]
    OODeclExists([ValueId; 2], OODeclExistsOp, LocId),
    ParentCls(LocId),
    Pow([ValueId; 2], LocId),
    Print(ValueId, LocId),
    ResolveClsMethodD(ClassId, MethodId, LocId),
    #[has_locals(none)]
    #[has_operands(none)]
    ResolveClsMethodS(SpecialClsRef, MethodId, LocId),
    ResolveFunc(FunctionId, LocId),
    ResolveMethCaller(FunctionId, LocId),
    SelfCls(LocId),
    SetG([ValueId; 2], LocId),
    SetImplicitContextByValue(ValueId, LocId),
    SetL(ValueId, LocalId, LocId),
    SetOpL(ValueId, LocalId, SetOpOp, LocId),
    SetOpS([ValueId; 3], SetOpOp, LocId),
    SetS([ValueId; 3], ReadonlyOp, LocId),
    Shl([ValueId; 2], LocId),
    Shr([ValueId; 2], LocId),
    #[has_operands(none)]
    Silence(LocalId, SilenceOp, LocId),
    Sub([ValueId; 2], LocId),
    This(LocId),
    ThrowNonExhaustiveSwitch(LocId),
    UnsetL(LocalId, LocId),
    VerifyImplicitContextState(LocId),
    VerifyOutType(ValueId, LocalId, LocId),
    VerifyParamType(LocalId, LocId),
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

#[derive(Debug, HasLoc)]
pub enum BaseOp {
    BaseC(MOpMode, LocId),
    BaseGC(MOpMode, LocId),
    BaseH(LocId),
    BaseL(MOpMode, ReadonlyOp, LocId),
    BaseSC(MOpMode, ReadonlyOp, LocId),
}

#[derive(Debug, HasLoc)]
pub enum IntermediateOp {
    Dim(MOpMode, MemberKey, ReadonlyOp, LocId),
}

#[derive(Debug, HasLoc, HasLocals, HasOperands)]
#[has_loc("base_op")]
pub struct MemberOpData {
    pub operands: Box<[ValueId]>,
    pub locals: Box<[LocalId]>,
    pub base_op: BaseOp,
    pub intermediate_ops: Box<[IntermediateOp]>,
    pub final_key: MemberKey,
    pub final_readonly: ReadonlyOp,
    pub final_loc: LocId,
}

#[derive(Debug, HasLoc, HasLocals, HasOperands)]
pub enum MemberOp {
    #[has_loc(0)]
    #[has_operands(0)]
    #[has_locals(0)]
    IncDecM(MemberOpData, IncDecOp),
    #[has_loc(0)]
    #[has_operands(0)]
    #[has_locals(0)]
    QueryM(MemberOpData, QueryMOp),
    SetM(MemberOpData),
    #[has_loc(0)]
    #[has_operands(0)]
    #[has_locals(0)]
    SetOpM(MemberOpData, SetOpOp),
    UnsetM(MemberOpData),
}

impl MemberOp {
    pub fn data(&self) -> &MemberOpData {
        match self {
            MemberOp::IncDecM(data, _)
            | MemberOp::QueryM(data, _)
            | MemberOp::SetM(data)
            | MemberOp::SetOpM(data, _)
            | MemberOp::UnsetM(data) => data,
        }
    }
}

impl CanThrow for MemberOp {
    fn can_throw(&self) -> bool {
        // TODO: Lazy... need to figure out which of these could actually throw
        // or not.
        true
    }
}

#[derive(Debug)]
pub enum CallDetail {
    FCallClsMethod {
        log: IsLogAsDynamicCallOp,
    },
    FCallClsMethodD {
        clsid: ClassId,
        method: MethodId,
    },
    FCallClsMethodM {
        method: MethodId,
        log: IsLogAsDynamicCallOp,
    },
    FCallClsMethodS {
        clsref: SpecialClsRef,
    },
    FCallClsMethodSD {
        clsref: SpecialClsRef,
        method: MethodId,
    },
    FCallCtor,
    FCallFunc,
    FCallFuncD {
        func: FunctionId,
    },
    FCallObjMethod {
        flavor: ObjMethodOp,
    },
    FCallObjMethodD {
        flavor: ObjMethodOp,
        method: MethodId,
    },
}

impl CallDetail {
    pub fn args<'a>(&self, operands: &'a [ValueId]) -> &'a [ValueId] {
        let len = operands.len();
        match self {
            CallDetail::FCallClsMethod { .. } => &operands[2..len - 2],
            CallDetail::FCallFunc
            | CallDetail::FCallObjMethod { .. }
            | CallDetail::FCallClsMethodM { .. }
            | CallDetail::FCallClsMethodS { .. } => &operands[2..len - 1],
            CallDetail::FCallClsMethodD { .. }
            | CallDetail::FCallClsMethodSD { .. }
            | CallDetail::FCallCtor
            | CallDetail::FCallFuncD { .. }
            | CallDetail::FCallObjMethodD { .. } => &operands[2..],
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
            CallDetail::FCallClsMethod { .. }
            | CallDetail::FCallClsMethodS { .. }
            | CallDetail::FCallObjMethod { .. } => operands[len - 1],
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
            | CallDetail::FCallClsMethodD { .. }
            | CallDetail::FCallClsMethodM { .. }
            | CallDetail::FCallObjMethodD { .. }
            | CallDetail::FCallObjMethod { .. } => operands[0],
            CallDetail::FCallClsMethod { .. }
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

#[derive(Debug, HasLoc, HasLocals, HasOperands)]
#[has_locals(none)]
pub struct Call {
    pub operands: Box<[ValueId]>,
    pub context: UnitStringId,
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
}

impl CanThrow for Call {
    fn can_throw(&self) -> bool {
        // TODO: Lazy... need to figure out which of these could actually throw
        // or not.
        true
    }
}

#[derive(Debug)]
pub enum IncludeKind {
    Eval,
    Include,
    IncludeOnce,
    Require,
    RequireOnce,
    RequireOnceDoc,
}

#[derive(Debug, HasLoc, HasLocals, HasOperands)]
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

#[derive(Copy, Clone, Debug)]
pub enum SurpriseCheck {
    Yes,
    No,
}

#[derive(Debug)]
pub enum MemberKey {
    EC,
    EI(i64),
    EL,
    ET(UnitStringId),
    PC,
    PL,
    PT(PropId),
    QT(PropId),
    W,
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

#[derive(Debug)]
pub enum Predicate {
    NonZero,
    Zero,
}

#[derive(Debug, HasLoc)]
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

#[derive(Copy, Clone, Debug)]
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

#[derive(Debug, HasLoc, HasLocals, HasOperands)]
pub enum Special {
    Copy(ValueId),
    Param,
    // Used during ir_to_bc - not in normal IR.
    PopC,
    // Used during ir_to_bc - not in normal IR.
    PopL(LocalId),
    // Used during ir_to_bc - not in normal IR.
    PushL(LocalId),
    // Used during ir_to_bc - not in normal IR.
    PushLiteral(ValueId),
    Select(ValueId, u32),
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

    pub fn jmp(bid: BlockId, loc: LocId) -> Instr {
        Instr::Terminator(Terminator::Jmp(bid, SurpriseCheck::Yes, loc))
    }

    pub fn jmp_no_surprise(bid: BlockId, loc: LocId) -> Instr {
        Instr::Terminator(Terminator::Jmp(bid, SurpriseCheck::No, loc))
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
}

newtype_int!(UnnamedLocalId, u32, UnnamedLocalIdMap, UnnamedLocalIdSet);

/// Note: Unlike HHBC the IR named and unnamed are in distinct namespaces.
#[derive(Debug, Copy, Clone, Hash, Eq, PartialEq)]
pub enum LocalId {
    Named(UnitStringId),
    // The UnnamedLocalIds are just for uniqueness - there are no requirements
    // for these IDs to be contiguous or in any particular order.
    Unnamed(UnnamedLocalId),
}

#[test]
fn check_sizes() {
    use static_assertions::assert_eq_size;
    assert_eq_size!(ValueId, u32);
}
