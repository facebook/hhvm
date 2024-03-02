// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use ffi::Vector;
use serde::Serialize;

use crate::opcodes::Opcode;
use crate::BytesId;
use crate::FCallArgsFlags;
use crate::PropName;
use crate::ReadonlyOp;
use crate::SrcLoc;
use crate::StringId;

/// see runtime/base/repo-auth-type.h
pub type RepoAuthType = StringId;

pub type StackIndex = u32;
pub type ClassNum = u32;

#[derive(Copy, Clone, Debug, PartialEq, Eq, Hash, Serialize)]
#[repr(C)]
pub struct Dummy(bool);

impl Dummy {
    pub const DEFAULT: Dummy = Dummy(false);
}

/// HHBC encodes bytecode offsets as i32 (HPHP::Offset) so u32
/// is plenty of range for label ids.
#[derive(Debug, Clone, Copy, PartialEq, Eq, Hash, Ord, PartialOrd, Serialize)]
#[repr(transparent)]
pub struct Label(pub u32);

impl std::convert::From<Label> for usize {
    fn from(id: Label) -> Self {
        id.0 as usize
    }
}

impl std::fmt::Display for Label {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        self.0.fmt(f)
    }
}

impl Label {
    pub const INVALID: Label = Label(u32::MAX);
    pub const ZERO: Label = Label(0);

    pub fn is_valid(&self) -> bool {
        self.0 != u32::MAX
    }

    pub fn as_usize(&self) -> usize {
        self.0 as usize
    }
}

pub type NumParams = u32;
pub type ByRefs = Vector<bool>;

// This corresponds to kActRecCells in bytecode.h and must be kept in sync.
pub const NUM_ACT_REC_CELLS: usize = 2;

#[derive(Clone, Debug, PartialEq, Eq, Hash, Serialize)]
#[repr(C)]
pub struct FCallArgs {
    pub flags: FCallArgsFlags,
    pub async_eager_target: Label,
    pub num_args: NumParams,
    pub num_rets: NumParams,
    pub inouts: ByRefs,
    pub readonly: ByRefs,
    pub context: StringId,
}

impl FCallArgs {
    pub fn new(
        mut flags: FCallArgsFlags,
        num_rets: NumParams,
        num_args: NumParams,
        inouts: Vec<bool>,
        readonly: Vec<bool>,
        async_eager_target: Option<Label>,
        context: Option<StringId>,
    ) -> Self {
        assert!(
            inouts.is_empty() || inouts.len() == num_args as usize,
            "length of inouts must be either zero or num_args"
        );
        assert!(
            readonly.is_empty() || readonly.len() == num_args as usize,
            "length of readonly must be either zero or num_args"
        );
        assert!(
            context.map_or(true, |c| !c.is_empty()),
            "unexpected empty context"
        );
        if context.is_some() {
            flags |= FCallArgsFlags::ExplicitContext;
        }
        let async_eager_target = match async_eager_target {
            Some(label) => {
                flags |= FCallArgsFlags::HasAsyncEagerOffset;
                label
            }
            None => Label::INVALID,
        };
        Self {
            flags,
            num_args,
            num_rets,
            inouts: inouts.into(),
            readonly: readonly.into(),
            async_eager_target,
            context: context.unwrap_or(StringId::EMPTY),
        }
    }

    pub fn has_async_eager_target(&self) -> bool {
        self.flags.contains(FCallArgsFlags::HasAsyncEagerOffset)
    }

    pub fn targets(&self) -> &[Label] {
        if self.has_async_eager_target() {
            std::slice::from_ref(&self.async_eager_target)
        } else {
            &[]
        }
    }

    /// num_inputs() only includes "interesting" inputs - so it doesn't include
    /// the ActRec or inout values. It also doesn't include inputs defined by
    /// the specific instruct (such as FCallFunc vs FCallFuncD).
    ///
    /// This is similar but not exactly the same as the numArgsInclUnpack()
    /// lambda in HHVM's fcallImpl().
    pub fn num_inputs(&self) -> usize {
        self.num_args as usize
            + self.flags.contains(FCallArgsFlags::HasUnpack) as usize
            + self.flags.contains(FCallArgsFlags::HasGenerics) as usize
    }

    pub fn num_inouts(&self) -> usize {
        self.inouts.iter().map(|&b| b as usize).sum()
    }
}

/// Local variable numbers are ultimately encoded as IVA, limited to u32.
/// Locals with idx < num_params + num_decl_vars are considered named,
/// higher numbered locals are considered unnamed.
#[derive(Copy, Clone, Debug, Eq, PartialEq, Ord, PartialOrd, Hash, Serialize)]
#[repr(C)]
pub struct Local {
    /// 0-based index into HHBC stack frame locals.
    pub idx: u32,
}

impl std::fmt::Display for Local {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        self.idx.fmt(f)
    }
}

impl Local {
    pub const INVALID: Self = Self { idx: u32::MAX };
    pub const ZERO: Self = Self { idx: 0 };

    pub fn new(x: usize) -> Self {
        Self { idx: x as u32 }
    }

    pub fn is_valid(self) -> bool {
        self != Self::INVALID
    }

    pub fn from_usize(idx: usize) -> Local {
        Local { idx: idx as u32 }
    }

    pub fn as_usize(&self) -> usize {
        self.idx as usize
    }
}

#[derive(
    Default, Debug, Clone, Copy, PartialEq, Eq, Hash, Ord, PartialOrd, Serialize
)]
#[repr(C)]
pub struct IterId {
    /// 0-based index into HHBC stack frame iterators
    pub idx: u32,
}

impl std::fmt::Display for IterId {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        write!(f, "{}", self.idx)
    }
}

#[derive(Clone, Debug, Hash, Eq, PartialEq, Serialize)]
#[repr(C)]
pub struct IterArgs {
    pub iter_id: IterId,
    pub key_id: Local,
    pub val_id: Local,
}

impl std::default::Default for IterArgs {
    fn default() -> Self {
        Self {
            iter_id: Default::default(),
            key_id: Local::INVALID,
            val_id: Local::INVALID,
        }
    }
}

#[derive(Clone, Copy, Debug, Hash, Eq, PartialEq, Serialize)]
#[repr(C)]
pub enum MemberKey {
    EC(StackIndex, ReadonlyOp),
    EL(Local, ReadonlyOp),
    ET(BytesId, ReadonlyOp),
    EI(i64, ReadonlyOp),
    PC(StackIndex, ReadonlyOp),
    PL(Local, ReadonlyOp),
    PT(PropName, ReadonlyOp),
    QT(PropName, ReadonlyOp),
    W,
}

#[derive(Clone, Copy, Debug, Eq, PartialEq, Serialize)]
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

#[derive(Clone, Copy, Debug, Eq, PartialEq, Serialize)]
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

impl From<Visibility> for hhvm_types_ffi::Attr {
    fn from(k: Visibility) -> Self {
        match k {
            Visibility::Private => Self::AttrPrivate,
            Visibility::Public => Self::AttrPublic,
            Visibility::Protected => Self::AttrProtected,
            // TODO(T115356820): Decide whether internal should be mutually
            // exclusive with other visibility modifiers or it should be a
            // modifier on top the others.
            // In order to unblock typechecker, let it be a modifier on top for now.
            Visibility::Internal => Self::AttrInternal | Self::AttrPublic,
        }
    }
}

/// A Contiguous range of locals. The canonical (default) empty
/// range is {0, 0}. This is normally only used for unnamed locals
/// but nothing prevents arbitrary ranges.
#[derive(Clone, Copy, Debug, Hash, Eq, PartialEq, Serialize)]
#[repr(C)]
pub struct LocalRange {
    pub start: Local,
    pub len: u32,
}

impl LocalRange {
    pub const EMPTY: LocalRange = LocalRange {
        start: Local::INVALID,
        len: 0,
    };

    pub fn from_local(local: Local) -> LocalRange {
        LocalRange {
            start: local,
            len: 1,
        }
    }

    pub fn iter(&self) -> impl Iterator<Item = Local> {
        let start = self.start.as_usize();
        let end = start + self.len as usize;
        (start..end).map(Local::from_usize)
    }
}

/// These are HHAS pseudo-instructions that are handled in the HHAS parser and
/// do not have HHBC opcodes equivalents.
#[derive(Clone, Debug, Eq, PartialEq, Hash, Serialize)]
#[repr(C)]
pub enum Pseudo {
    /// An internal representation of a break statement that is removed by the
    /// try/finally rewriter.
    Break,
    /// An internal representation of a continue statement that is removed by
    /// the try/finally rewriter.
    Continue,
    Label(Label),
    SrcLoc(SrcLoc),
    TryCatchBegin,
    TryCatchEnd,
    TryCatchMiddle,
}

pub trait Targets {
    /// Return a slice of labels for the conditional branch targets of this
    /// instruction. This excludes the Label in an ILabel instruction, which is
    /// not a conditional branch.
    fn targets(&self) -> &[Label];
}

pub trait Flow {
    /// Returns true if the bytecode does not ever pass control to the next
    /// bytecode in the sequence. Bytecodes for which is_terminal() is true
    /// may still be is_flow() if they can still branch.
    fn is_terminal(&self) -> bool;

    /// Returns true if the bytecode performs control flow. The flow need not
    /// be within the current frame (e.g. a Call bytecode is a flow bytecode),
    /// however, bytecodes which produce faults but do not otherwise branch are
    /// not considered is_flow() bytecodes.
    fn is_flow(&self) -> bool;
}

pub trait Locals {
    /// Return a Vec of locals referenced in the immediates for this bytecode
    fn locals(&self) -> Vec<Local>;
}

#[derive(Clone, Debug, Eq, PartialEq, Hash, Serialize)]
#[repr(C)]
pub enum Instruct<'arena> {
    // HHVM opcodes.
    Opcode(Opcode<'arena>),
    // HHAS pseudo-instructions.
    Pseudo(Pseudo),
}

impl Instruct<'_> {
    /// Return a slice of labels for the conditional branch targets of this instruction.
    /// This excludes the Label in an ILabel instruction, which is not a conditional branch.
    pub fn targets(&self) -> &[Label] {
        match self {
            Self::Opcode(opcode) => opcode.targets(),

            // Make sure new variants with branch target Labels are handled above
            // before adding items to this catch-all.
            Self::Pseudo(
                Pseudo::Continue
                | Pseudo::Break
                | Pseudo::Label(_)
                | Pseudo::TryCatchBegin
                | Pseudo::TryCatchMiddle
                | Pseudo::TryCatchEnd
                | Pseudo::SrcLoc(_),
            ) => &[],
        }
    }

    pub fn num_inputs(&self) -> usize {
        match self {
            Self::Opcode(opcode) => opcode.num_inputs(),

            Self::Pseudo(
                Pseudo::Continue
                | Pseudo::Break
                | Pseudo::Label(_)
                | Pseudo::TryCatchBegin
                | Pseudo::TryCatchMiddle
                | Pseudo::TryCatchEnd
                | Pseudo::SrcLoc(_),
            ) => 0,
        }
    }

    pub fn variant_name(&self) -> &'static str {
        match self {
            Self::Opcode(opcode) => opcode.variant_name(),
            Self::Pseudo(Pseudo::Continue) => "Continue",
            Self::Pseudo(Pseudo::Break) => "Break",
            Self::Pseudo(Pseudo::Label(_)) => "Label",
            Self::Pseudo(Pseudo::TryCatchBegin) => "TryCatchBegin",
            Self::Pseudo(Pseudo::TryCatchMiddle) => "TryCatchMiddle",
            Self::Pseudo(Pseudo::TryCatchEnd) => "TryCatchEnd",
            Self::Pseudo(Pseudo::SrcLoc(_)) => "SrcLoc",
        }
    }
}
