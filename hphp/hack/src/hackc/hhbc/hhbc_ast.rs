// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use crate::{opcodes::Opcode, typed_value::TypedValue, FCallArgsFlags, PropName, ReadonlyOp};
use ffi::{Slice, Str};

/// see runtime/base/repo-auth-type.h
pub type RepoAuthType<'arena> = Str<'arena>;

/// Export these publicly so consumers of hhbc_ast don't have to know the
/// internal details about the ffi.

#[derive(Clone, Debug, Eq, PartialEq, Hash)]
#[repr(C)]
pub enum ParamName<'arena> {
    ParamUnnamed(isize),
    ParamNamed(Str<'arena>),
}

pub type StackIndex = u32;
pub type ClassNum = u32;

#[derive(Clone, Debug, PartialEq, Eq, Hash)]
#[repr(C)]
pub struct Dummy;

/// HHBC encodes bytecode offsets as i32 (HPHP::Offset) so u32
/// is plenty of range for label ids.
#[derive(Debug, Default, Clone, Copy, PartialEq, Eq, Hash, Ord, PartialOrd)]
#[repr(transparent)]
pub struct Label(pub u32);

impl std::fmt::Display for Label {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        self.0.fmt(f)
    }
}

impl Label {
    pub const INVALID: Label = Label(u32::MAX);
}

pub type NumParams = u32;
pub type ByRefs<'arena> = Slice<'arena, bool>;

// This corresponds to kActRecCells in bytecode.h and must be kept in sync.
pub const NUM_ACT_REC_CELLS: usize = 2;

#[derive(Clone, Debug, PartialEq, Eq, Hash)]
#[repr(C)]
pub struct FCallArgs<'arena> {
    pub flags: FCallArgsFlags,
    pub async_eager_target: Label,
    pub num_args: NumParams,
    pub num_rets: NumParams,
    pub inouts: ByRefs<'arena>,
    pub readonly: ByRefs<'arena>,
    pub context: Str<'arena>,
}

impl<'arena> FCallArgs<'arena> {
    pub fn new(
        mut flags: FCallArgsFlags,
        num_rets: NumParams,
        num_args: NumParams,
        inouts: Slice<'arena, bool>,
        readonly: Slice<'arena, bool>,
        async_eager_target: Option<Label>,
        context: Option<&'arena str>,
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
            inouts,
            readonly,
            async_eager_target,
            context: Str::new(context.unwrap_or("").as_bytes()),
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

    pub fn targets_mut(&mut self) -> &mut [Label] {
        if self.has_async_eager_target() {
            std::slice::from_mut(&mut self.async_eager_target)
        } else {
            &mut []
        }
    }

    pub fn num_inputs(&self) -> usize {
        self.num_args as usize
            + self.flags.contains(FCallArgsFlags::HasUnpack) as usize
            + self.flags.contains(FCallArgsFlags::HasGenerics) as usize
    }
}

/// Local variable numbers are ultimately encoded as IVA, limited to u32.
/// Locals with idx < num_params + num_decl_vars are considered named,
/// higher numbered locals are considered unnamed.
#[derive(Copy, Clone, Debug, Default, Eq, PartialEq, Ord, PartialOrd, Hash)]
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

#[derive(Default, Debug, Clone, Copy, PartialEq, Eq, Hash)]
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

#[derive(Clone, Debug, Hash, Eq, PartialEq)]
#[repr(C)]
pub struct IterArgs {
    pub iter_id: IterId,
    pub key_id: Local,
    pub val_id: Local,
}

/// Conventionally this is "A_" followed by an integer
pub type AdataId<'arena> = Str<'arena>;

#[derive(Clone, Copy, Debug, Hash, Eq, PartialEq)]
#[repr(C)]
pub enum MemberKey<'arena> {
    EC(StackIndex, ReadonlyOp),
    EL(Local, ReadonlyOp),
    ET(Str<'arena>, ReadonlyOp),
    EI(i64, ReadonlyOp),
    PC(StackIndex, ReadonlyOp),
    PL(Local, ReadonlyOp),
    PT(PropName<'arena>, ReadonlyOp),
    QT(PropName<'arena>, ReadonlyOp),
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
            Visibility::Internal => (Self::AttrInternal | Self::AttrPublic).into(),
        }
    }
}

/// A Contiguous range of locals. The canonical (default) empty
/// range is {0, 0}. This is normally only used for unnamed locals
/// but nothing prevents arbitrary ranges.
#[derive(Clone, Copy, Debug, Default, Hash, Eq, PartialEq)]
#[repr(C)]
pub struct LocalRange {
    pub start: Local,
    pub len: u32,
}

#[derive(Clone, Debug, Hash, Eq, PartialEq, Default)]
#[repr(C)]
pub struct SrcLoc {
    pub line_begin: isize,
    pub col_begin: isize,
    pub line_end: isize,
    pub col_end: isize,
}

/// These are HHAS pseudo-instructions that are handled in the HHAS parser and
/// do not have HHBC opcodes equivalents.
#[derive(Clone, Debug, Eq, PartialEq, Hash)]
#[repr(C)]
pub enum Pseudo<'arena> {
    Break(isize),
    Comment(Str<'arena>),
    Continue(isize),
    Label(Label),
    SrcLoc(SrcLoc),
    TryCatchBegin,
    TryCatchEnd,
    TryCatchMiddle,
    /// Pseudo instruction that will get translated into appropraite literal
    /// bytecode, with possible reference to .adata *)
    TypedValue(TypedValue<'arena>),
}

pub trait Targets {
    /// Return a slice of labels for the conditional branch targets of this
    /// instruction. This excludes the Label in an ILabel instruction, which is
    /// not a conditional branch.
    fn targets(&self) -> &[Label];

    /// Return a mutable slice of labels for the conditional branch targets of
    /// this instruction. This excludes the Label in an ILabel instruction,
    /// which is not a conditional branch.
    fn targets_mut(&mut self) -> &mut [Label];
}

#[derive(Clone, Debug, Eq, PartialEq, Hash)]
#[repr(C)]
pub enum Instruct<'arena> {
    // HHVM opcodes.
    Opcode(Opcode<'arena>),
    // HHAS pseudo-instructions.
    Pseudo(Pseudo<'arena>),
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
                Pseudo::TypedValue(_)
                | Pseudo::Continue(_)
                | Pseudo::Break(_)
                | Pseudo::Label(_)
                | Pseudo::TryCatchBegin
                | Pseudo::TryCatchMiddle
                | Pseudo::TryCatchEnd
                | Pseudo::Comment(_)
                | Pseudo::SrcLoc(_),
            ) => &[],
        }
    }

    /// Return a mutable slice of labels for the conditional branch targets of this instruction.
    /// This excludes the Label in an ILabel instruction, which is not a conditional branch.
    pub fn targets_mut(&mut self) -> &mut [Label] {
        match self {
            Self::Opcode(opcode) => opcode.targets_mut(),

            // Make sure new variants with branch target Labels are handled
            // above before adding items to this catch-all.
            Self::Pseudo(
                Pseudo::TypedValue(_)
                | Pseudo::Continue(_)
                | Pseudo::Break(_)
                | Pseudo::Label(_)
                | Pseudo::TryCatchBegin
                | Pseudo::TryCatchMiddle
                | Pseudo::TryCatchEnd
                | Pseudo::Comment(_)
                | Pseudo::SrcLoc(_),
            ) => &mut [],
        }
    }

    pub fn num_inputs(&self) -> usize {
        match self {
            Self::Opcode(opcode) => opcode.num_inputs(),

            Self::Pseudo(
                Pseudo::TypedValue(_)
                | Pseudo::Continue(_)
                | Pseudo::Break(_)
                | Pseudo::Label(_)
                | Pseudo::TryCatchBegin
                | Pseudo::TryCatchMiddle
                | Pseudo::TryCatchEnd
                | Pseudo::Comment(_)
                | Pseudo::SrcLoc(_),
            ) => 0,
        }
    }

    pub fn variant_name(&self) -> &'static str {
        match self {
            Self::Opcode(opcode) => opcode.variant_name(),
            Self::Pseudo(Pseudo::TypedValue(_)) => "TypedValue",
            Self::Pseudo(Pseudo::Continue(_)) => "Continue",
            Self::Pseudo(Pseudo::Break(_)) => "Break",
            Self::Pseudo(Pseudo::Label(_)) => "Label",
            Self::Pseudo(Pseudo::TryCatchBegin) => "TryCatchBegin",
            Self::Pseudo(Pseudo::TryCatchMiddle) => "TryCatchMiddle",
            Self::Pseudo(Pseudo::TryCatchEnd) => "TryCatchEnd",
            Self::Pseudo(Pseudo::Comment(_)) => "Comment",
            Self::Pseudo(Pseudo::SrcLoc(_)) => "SrcLoc",
        }
    }
}
