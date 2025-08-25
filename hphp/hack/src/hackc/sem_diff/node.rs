use std::fmt;
use std::rc::Rc;

use hhbc::IncDecOp;
use hhbc::Label;
use hhbc::MOpMode;
use hhbc::MemberKey;
use hhbc::Opcode;
use hhbc::QueryMOp;
use hhbc::ReadonlyOp;
use hhbc::SetOpOp;
use hhbc::SetRangeOp;
use hhbc::SrcLoc;
use hhbc::Targets;
use hhbc::TypedValue;

#[derive(Debug, Eq, PartialEq, Clone, Hash)]
pub(crate) enum Input<'a> {
    Class(String),
    Constant(u32),
    ConstantArray(&'a TypedValue),
    // A value that appears exactly once in the stack or locals.
    Owned(u32),
    // A value that is guaranteed to be used in a read-only context.
    Read(u32),
    // A value that appears more than once in the stack or locals.
    Shared(u32),
    Undefined,
    // A value that doesn't appear in the stack or locals.
    Unowned(u32),
}

impl<'a> Input<'a> {
    pub(crate) fn to_read_only(&self) -> Input<'a> {
        match *self {
            Input::Owned(idx) | Input::Read(idx) | Input::Shared(idx) | Input::Unowned(idx) => {
                Input::Read(idx)
            }
            Input::Class(_) | Input::Constant(_) | Input::ConstantArray(_) | Input::Undefined => {
                self.clone()
            }
        }
    }
}

impl fmt::Display for Input<'_> {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        match self {
            Input::Class(s) => write!(f, "class({s})"),
            Input::Constant(v) => write!(f, "constant(#{v})"),
            Input::ConstantArray(tv) => write!(f, "constant({tv:?})"),
            Input::Owned(v) => write!(f, "owned(@{v})"),
            Input::Read(v) => write!(f, "read(@{v})"),
            Input::Shared(v) => write!(f, "shared(@{v})"),
            Input::Undefined => write!(f, "undefined"),
            Input::Unowned(v) => write!(f, "unowned(@{v})"),
        }
    }
}

#[derive(Debug, Eq, PartialEq, Hash)]
pub(crate) enum BaseOp {
    Base(MOpMode, ReadonlyOp, Rc<SrcLoc>),
    BaseH(Rc<SrcLoc>),
    BaseSC(MOpMode, ReadonlyOp, Rc<SrcLoc>),
}

#[derive(Debug, Eq, PartialEq, Hash)]
pub(crate) struct IntermediateOp {
    pub key: MemberKey,
    pub mode: MOpMode,
    pub src_loc: Rc<SrcLoc>,
}

#[derive(Debug, Eq, PartialEq, Hash)]
pub(crate) enum FinalOp {
    IncDecM(MemberKey, IncDecOp, Rc<SrcLoc>),
    QueryM(MemberKey, QueryMOp, Rc<SrcLoc>),
    SetM(MemberKey, Rc<SrcLoc>),
    SetRangeM(u32, SetRangeOp, Rc<SrcLoc>),
    SetOpM(MemberKey, SetOpOp, Rc<SrcLoc>),
    UnsetM(MemberKey, Rc<SrcLoc>),
}

impl FinalOp {
    pub(crate) fn is_write(&self) -> bool {
        match self {
            FinalOp::QueryM { .. } => false,
            FinalOp::SetRangeM { .. }
            | FinalOp::UnsetM { .. }
            | FinalOp::IncDecM { .. }
            | FinalOp::SetM { .. }
            | FinalOp::SetOpM { .. } => true,
        }
    }

    pub(crate) fn pushes_value(&self) -> bool {
        match self {
            FinalOp::IncDecM(..)
            | FinalOp::QueryM(..)
            | FinalOp::SetM(..)
            | FinalOp::SetOpM(..) => true,
            FinalOp::SetRangeM(..) | FinalOp::UnsetM(..) => false,
        }
    }
}

#[derive(Debug, Eq, PartialEq, Hash)]
pub(crate) struct MemberOp {
    pub(crate) base_op: BaseOp,
    pub(crate) intermediate_ops: Vec<IntermediateOp>,
    pub(crate) final_op: FinalOp,
}

#[derive(Debug, Eq, PartialEq, Hash)]
pub(crate) enum NodeInstr {
    Opcode(Opcode),
    MemberOp(MemberOp),
}

impl Targets for NodeInstr {
    fn targets(&self) -> &[Label] {
        match self {
            NodeInstr::MemberOp(_) => &[],
            NodeInstr::Opcode(o) => o.targets(),
        }
    }
}

#[derive(Debug)]
pub(crate) struct Node<'a> {
    pub(crate) instr: NodeInstr,
    pub(crate) inputs: Box<[Input<'a>]>,
    pub(crate) src_loc: Rc<SrcLoc>,
}
