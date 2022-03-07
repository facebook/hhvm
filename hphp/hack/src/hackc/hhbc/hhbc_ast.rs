// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

mod opcodes;

use ffi::{
    BumpSliceMut,
    Maybe::{self, *},
    Slice, Str,
};
use iterator::IterId;
use label::Label;
use local::{Local, LocalId};

pub use opcodes::Opcodes;

/// see runtime/base/repo-auth-type.h
pub type RepoAuthType<'arena> = Str<'arena>;

/// Export these publicly so consumers of hhbc_ast don't have to know the
/// internal details about the ffi.
pub use hhvm_hhbc_defs_ffi::ffi::{
    BareThisOp, CollectionType, ContCheckOp, FCallArgsFlags, FatalOp, IncDecOp, InitPropOp,
    IsLogAsDynamicCallOp, IsTypeOp, MOpMode, OODeclExistsOp, ObjMethodOp, QueryMOp, ReadonlyOp,
    SetOpOp, SetRangeOp, SilenceOp, SpecialClsRef, SwitchKind, TypeStructResolveOp,
};

#[derive(Clone, Debug)]
#[repr(C)]
pub enum ParamId<'arena> {
    ParamUnnamed(isize),
    ParamNamed(Str<'arena>),
}

pub type StackIndex = u32;
pub type ClassNum = u32;

pub type ClassId<'arena> = hhbc_id::class::ClassType<'arena>;
pub type FunctionId<'arena> = hhbc_id::function::FunctionType<'arena>;
pub type MethodId<'arena> = hhbc_id::method::MethodType<'arena>;
pub type ConstId<'arena> = hhbc_id::constant::ConstType<'arena>;
pub type PropId<'arena> = hhbc_id::prop::PropType<'arena>;

pub type NumParams = u32;
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
        num_rets: NumParams,
        num_args: NumParams,
        inouts: Slice<'arena, bool>,
        readonly: Slice<'arena, bool>,
        async_eager_target: Option<Label>,
        context: Option<&'arena str>,
    ) -> FcallArgs<'arena> {
        assert!(
            (inouts.is_empty() || inouts.len() == num_args as usize)
                && (readonly.is_empty() || readonly.len() == num_args as usize),
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

/// A Contiguous range of unnamed locals. The canonical (default) empty
/// range is {0, 0}. Ranges of named locals cannot be represented.
#[derive(Clone, Copy, Debug, Default)]
#[repr(C)]
pub struct LocalRange {
    pub start: LocalId,
    pub len: u32,
}

#[derive(Clone, Debug)]
#[repr(C)]
pub struct SrcLoc {
    pub line_begin: isize,
    pub col_begin: isize,
    pub line_end: isize,
    pub col_end: isize,
}

/// These are HHAS pseudo-instructions that are handled in the HHAS parser and
/// do not have HHBC opcodes equivalents.
#[derive(Clone, Debug)]
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
    TypedValue(runtime::TypedValue<'arena>),
}

#[derive(Clone, Debug)]
#[repr(C)]
pub enum Instruct<'arena> {
    // HHVM opcodes.
    Opcode(Opcodes<'arena>),
    // HHAS pseudo-instructions.
    Pseudo(Pseudo<'arena>),
}

impl Instruct<'_> {
    /// Return a slice of labels for the conditional branch targets of this instruction.
    /// This excludes the Label in an ILabel instruction, which is not a conditional branch.
    pub fn targets(&self) -> &[Label] {
        match self {
            Self::Opcode(Opcodes::FCallClsMethod(fcall_args, ..))
            | Self::Opcode(Opcodes::FCallClsMethodD(fcall_args, ..))
            | Self::Opcode(Opcodes::FCallClsMethodS(fcall_args, ..))
            | Self::Opcode(Opcodes::FCallClsMethodSD(fcall_args, ..))
            | Self::Opcode(Opcodes::FCallCtor(fcall_args, ..))
            | Self::Opcode(Opcodes::FCallFunc(fcall_args))
            | Self::Opcode(Opcodes::FCallFuncD { fcall_args, .. })
            | Self::Opcode(Opcodes::FCallObjMethod { fcall_args, .. })
            | Self::Opcode(Opcodes::FCallObjMethodD { fcall_args, .. }) => fcall_args.targets(),
            Self::Opcode(Opcodes::Jmp(x))
            | Self::Opcode(Opcodes::JmpNS(x))
            | Self::Opcode(Opcodes::JmpZ(x))
            | Self::Opcode(Opcodes::JmpNZ(x)) => std::slice::from_ref(x),
            Self::Opcode(Opcodes::Switch { targets, .. })
            | Self::Opcode(Opcodes::SSwitch { targets, .. }) => targets.as_ref(),
            Self::Opcode(Opcodes::IterInit(_, target))
            | Self::Opcode(Opcodes::IterNext(_, target)) => std::slice::from_ref(target),
            Self::Opcode(Opcodes::MemoGet(target, _)) => std::slice::from_ref(target),
            Self::Opcode(Opcodes::MemoGetEager(targets, _)) => targets,

            // Make sure new variants with branch target Labels are handled above
            // before adding items to this catch-all.
            Self::Opcode(Opcodes::RetC)
            | Self::Opcode(Opcodes::This)
            | Self::Opcode(Opcodes::BareThis(_))
            | Self::Opcode(Opcodes::CheckThis)
            | Self::Opcode(Opcodes::ChainFaults)
            | Self::Opcode(Opcodes::OODeclExists(_))
            | Self::Opcode(Opcodes::VerifyParamType(_))
            | Self::Opcode(Opcodes::VerifyParamTypeTS(_))
            | Self::Opcode(Opcodes::VerifyOutType(_))
            | Self::Opcode(Opcodes::VerifyRetTypeC)
            | Self::Opcode(Opcodes::VerifyRetTypeTS)
            | Self::Opcode(Opcodes::SelfCls)
            | Self::Opcode(Opcodes::ParentCls)
            | Self::Opcode(Opcodes::LateBoundCls)
            | Self::Opcode(Opcodes::ClassName)
            | Self::Opcode(Opcodes::LazyClassFromClass)
            | Self::Opcode(Opcodes::RecordReifiedGeneric)
            | Self::Opcode(Opcodes::CheckReifiedGenericMismatch)
            | Self::Opcode(Opcodes::NativeImpl)
            | Self::Opcode(Opcodes::AKExists)
            | Self::Opcode(Opcodes::CreateCl(_, _))
            | Self::Opcode(Opcodes::Idx)
            | Self::Opcode(Opcodes::ArrayIdx)
            | Self::Opcode(Opcodes::ArrayMarkLegacy)
            | Self::Opcode(Opcodes::ArrayUnmarkLegacy)
            | Self::Opcode(Opcodes::AssertRATL(_, _))
            | Self::Opcode(Opcodes::AssertRATStk(_, _))
            | Self::Opcode(Opcodes::BreakTraceHint)
            | Self::Opcode(Opcodes::Silence(_, _))
            | Self::Opcode(Opcodes::GetMemoKeyL(_))
            | Self::Opcode(Opcodes::CGetCUNop)
            | Self::Opcode(Opcodes::UGetCUNop)
            | Self::Opcode(Opcodes::MemoSet(_))
            | Self::Opcode(Opcodes::MemoSetEager(_))
            | Self::Opcode(Opcodes::LockObj)
            | Self::Opcode(Opcodes::ThrowNonExhaustiveSwitch)
            | Self::Opcode(Opcodes::RaiseClassStringConversionWarning)
            | Self::Opcode(Opcodes::SetImplicitContextByValue)
            | Self::Opcode(Opcodes::RetCSuspended)
            | Self::Opcode(Opcodes::RetM(_))
            | Self::Opcode(Opcodes::Throw)
            | Self::Opcode(Opcodes::Nop)
            | Self::Opcode(Opcodes::EntryNop)
            | Self::Opcode(Opcodes::PopC)
            | Self::Opcode(Opcodes::PopU)
            | Self::Opcode(Opcodes::Dup)
            | Self::Opcode(Opcodes::IterFree(_))
            | Self::Opcode(Opcodes::Null)
            | Self::Opcode(Opcodes::True)
            | Self::Opcode(Opcodes::False)
            | Self::Opcode(Opcodes::NullUninit)
            | Self::Opcode(Opcodes::Int(_))
            | Self::Opcode(Opcodes::Double(_))
            | Self::Opcode(Opcodes::String(_))
            | Self::Opcode(Opcodes::LazyClass(_))
            | Self::Pseudo(Pseudo::TypedValue(_))
            | Self::Opcode(Opcodes::Vec(_))
            | Self::Opcode(Opcodes::Dict(_))
            | Self::Opcode(Opcodes::Keyset(_))
            | Self::Opcode(Opcodes::NewDictArray(_))
            | Self::Opcode(Opcodes::NewStructDict(_))
            | Self::Opcode(Opcodes::NewVec(_))
            | Self::Opcode(Opcodes::NewKeysetArray(_))
            | Self::Opcode(Opcodes::NewPair)
            | Self::Opcode(Opcodes::AddElemC)
            | Self::Opcode(Opcodes::AddNewElemC)
            | Self::Opcode(Opcodes::NewCol(_))
            | Self::Opcode(Opcodes::ColFromArray(_))
            | Self::Opcode(Opcodes::CnsE(_))
            | Self::Opcode(Opcodes::ClsCns(_))
            | Self::Opcode(Opcodes::ClsCnsD(_, _))
            | Self::Opcode(Opcodes::ClsCnsL(_))
            | Self::Opcode(Opcodes::File)
            | Self::Opcode(Opcodes::Dir)
            | Self::Opcode(Opcodes::Method)
            | Self::Opcode(Opcodes::FuncCred)
            | Self::Opcode(Opcodes::Concat)
            | Self::Opcode(Opcodes::ConcatN(_))
            | Self::Opcode(Opcodes::Add)
            | Self::Opcode(Opcodes::Sub)
            | Self::Opcode(Opcodes::Mul)
            | Self::Opcode(Opcodes::AddO)
            | Self::Opcode(Opcodes::SubO)
            | Self::Opcode(Opcodes::MulO)
            | Self::Opcode(Opcodes::Div)
            | Self::Opcode(Opcodes::Mod)
            | Self::Opcode(Opcodes::Pow)
            | Self::Opcode(Opcodes::Not)
            | Self::Opcode(Opcodes::Same)
            | Self::Opcode(Opcodes::NSame)
            | Self::Opcode(Opcodes::Eq)
            | Self::Opcode(Opcodes::Neq)
            | Self::Opcode(Opcodes::Lt)
            | Self::Opcode(Opcodes::Lte)
            | Self::Opcode(Opcodes::Gt)
            | Self::Opcode(Opcodes::Gte)
            | Self::Opcode(Opcodes::Cmp)
            | Self::Opcode(Opcodes::BitAnd)
            | Self::Opcode(Opcodes::BitOr)
            | Self::Opcode(Opcodes::BitXor)
            | Self::Opcode(Opcodes::BitNot)
            | Self::Opcode(Opcodes::Shl)
            | Self::Opcode(Opcodes::Shr)
            | Self::Opcode(Opcodes::CastBool)
            | Self::Opcode(Opcodes::CastInt)
            | Self::Opcode(Opcodes::CastDouble)
            | Self::Opcode(Opcodes::CastString)
            | Self::Opcode(Opcodes::CastVec)
            | Self::Opcode(Opcodes::CastDict)
            | Self::Opcode(Opcodes::CastKeyset)
            | Self::Opcode(Opcodes::InstanceOf)
            | Self::Opcode(Opcodes::InstanceOfD(_))
            | Self::Opcode(Opcodes::IsLateBoundCls)
            | Self::Opcode(Opcodes::IsTypeStructC(_))
            | Self::Opcode(Opcodes::ThrowAsTypeStructException)
            | Self::Opcode(Opcodes::CombineAndResolveTypeStruct(_))
            | Self::Opcode(Opcodes::Print)
            | Self::Opcode(Opcodes::Clone)
            | Self::Opcode(Opcodes::Exit)
            | Self::Opcode(Opcodes::Fatal(_))
            | Self::Opcode(Opcodes::ResolveFunc(_))
            | Self::Opcode(Opcodes::ResolveRFunc(_))
            | Self::Opcode(Opcodes::ResolveMethCaller(_))
            | Self::Opcode(Opcodes::ResolveClsMethod(_))
            | Self::Opcode(Opcodes::ResolveClsMethodD(_, _))
            | Self::Opcode(Opcodes::ResolveClsMethodS(_, _))
            | Self::Opcode(Opcodes::ResolveRClsMethod(_))
            | Self::Opcode(Opcodes::ResolveRClsMethodD(_, _))
            | Self::Opcode(Opcodes::ResolveRClsMethodS(_, _))
            | Self::Opcode(Opcodes::ResolveClass(_))
            | Self::Pseudo(Pseudo::Continue(_))
            | Self::Pseudo(Pseudo::Break(_))
            | Self::Opcode(Opcodes::NewObj)
            | Self::Opcode(Opcodes::NewObjR)
            | Self::Opcode(Opcodes::NewObjD(_))
            | Self::Opcode(Opcodes::NewObjRD(_))
            | Self::Opcode(Opcodes::NewObjS(_))
            | Self::Opcode(Opcodes::CGetL(_))
            | Self::Opcode(Opcodes::CGetQuietL(_))
            | Self::Opcode(Opcodes::CGetL2(_))
            | Self::Opcode(Opcodes::CUGetL(_))
            | Self::Opcode(Opcodes::PushL(_))
            | Self::Opcode(Opcodes::CGetG)
            | Self::Opcode(Opcodes::CGetS(_))
            | Self::Opcode(Opcodes::ClassGetC)
            | Self::Opcode(Opcodes::ClassGetTS)
            | Self::Opcode(Opcodes::SetL(_))
            | Self::Opcode(Opcodes::PopL(_))
            | Self::Opcode(Opcodes::SetG)
            | Self::Opcode(Opcodes::SetS(_))
            | Self::Opcode(Opcodes::SetOpL(_, _))
            | Self::Opcode(Opcodes::SetOpG(_))
            | Self::Opcode(Opcodes::SetOpS(_))
            | Self::Opcode(Opcodes::IncDecL(_, _))
            | Self::Opcode(Opcodes::IncDecG(_))
            | Self::Opcode(Opcodes::IncDecS(_))
            | Self::Opcode(Opcodes::UnsetL(_))
            | Self::Opcode(Opcodes::UnsetG)
            | Self::Opcode(Opcodes::CheckProp(_))
            | Self::Opcode(Opcodes::InitProp(_, _))
            | Self::Opcode(Opcodes::IssetL(_))
            | Self::Opcode(Opcodes::IssetG)
            | Self::Opcode(Opcodes::IssetS)
            | Self::Opcode(Opcodes::IsUnsetL(_))
            | Self::Opcode(Opcodes::IsTypeC(_))
            | Self::Opcode(Opcodes::IsTypeL(_, _))
            | Self::Opcode(Opcodes::BaseGC(_, _))
            | Self::Opcode(Opcodes::BaseGL(_, _))
            | Self::Opcode(Opcodes::BaseSC(_, _, _, _))
            | Self::Opcode(Opcodes::BaseL(_, _, _))
            | Self::Opcode(Opcodes::BaseC(_, _))
            | Self::Opcode(Opcodes::BaseH)
            | Self::Opcode(Opcodes::Dim(_, _))
            | Self::Opcode(Opcodes::QueryM(_, _, _))
            | Self::Opcode(Opcodes::SetM(_, _))
            | Self::Opcode(Opcodes::IncDecM(_, _, _))
            | Self::Opcode(Opcodes::SetOpM(_, _, _))
            | Self::Opcode(Opcodes::UnsetM(_, _))
            | Self::Opcode(Opcodes::SetRangeM(_, _, _))
            | Self::Pseudo(Pseudo::Label(_))
            | Self::Pseudo(Pseudo::TryCatchBegin)
            | Self::Pseudo(Pseudo::TryCatchMiddle)
            | Self::Pseudo(Pseudo::TryCatchEnd)
            | Self::Pseudo(Pseudo::Comment(_))
            | Self::Pseudo(Pseudo::SrcLoc(_))
            | Self::Opcode(Opcodes::WHResult)
            | Self::Opcode(Opcodes::Await)
            | Self::Opcode(Opcodes::AwaitAll(_))
            | Self::Opcode(Opcodes::CreateCont)
            | Self::Opcode(Opcodes::ContEnter)
            | Self::Opcode(Opcodes::ContRaise)
            | Self::Opcode(Opcodes::Yield)
            | Self::Opcode(Opcodes::YieldK)
            | Self::Opcode(Opcodes::ContCheck(_))
            | Self::Opcode(Opcodes::ContValid)
            | Self::Opcode(Opcodes::ContKey)
            | Self::Opcode(Opcodes::ContGetReturn)
            | Self::Opcode(Opcodes::ContCurrent)
            | Self::Opcode(Opcodes::Incl)
            | Self::Opcode(Opcodes::InclOnce)
            | Self::Opcode(Opcodes::Req)
            | Self::Opcode(Opcodes::ReqOnce)
            | Self::Opcode(Opcodes::ReqDoc)
            | Self::Opcode(Opcodes::Eval) => &[],
        }
    }

    /// Return a mutable slice of labels for the conditional branch targets of this instruction.
    /// This excludes the Label in an ILabel instruction, which is not a conditional branch.
    pub fn targets_mut(&mut self) -> &mut [Label] {
        match self {
            Self::Opcode(Opcodes::FCallClsMethod(fcall_args, ..))
            | Self::Opcode(Opcodes::FCallClsMethodD(fcall_args, ..))
            | Self::Opcode(Opcodes::FCallClsMethodS(fcall_args, ..))
            | Self::Opcode(Opcodes::FCallClsMethodSD(fcall_args, ..))
            | Self::Opcode(Opcodes::FCallCtor(fcall_args, ..))
            | Self::Opcode(Opcodes::FCallFunc(fcall_args))
            | Self::Opcode(Opcodes::FCallFuncD { fcall_args, .. })
            | Self::Opcode(Opcodes::FCallObjMethod { fcall_args, .. })
            | Self::Opcode(Opcodes::FCallObjMethodD { fcall_args, .. }) => fcall_args.targets_mut(),
            Self::Opcode(Opcodes::Jmp(x))
            | Self::Opcode(Opcodes::JmpNS(x))
            | Self::Opcode(Opcodes::JmpZ(x))
            | Self::Opcode(Opcodes::JmpNZ(x)) => std::slice::from_mut(x),
            Self::Opcode(Opcodes::Switch { targets, .. })
            | Self::Opcode(Opcodes::SSwitch { targets, .. }) => targets.as_mut(),
            Self::Opcode(Opcodes::IterInit(_, target))
            | Self::Opcode(Opcodes::IterNext(_, target)) => std::slice::from_mut(target),
            Self::Opcode(Opcodes::MemoGet(target, _)) => std::slice::from_mut(target),
            Self::Opcode(Opcodes::MemoGetEager(targets, _)) => targets,

            // Make sure new variants with branch target Labels are handled above
            // before adding items to this catch-all.
            Self::Opcode(Opcodes::RetC)
            | Self::Opcode(Opcodes::This)
            | Self::Opcode(Opcodes::BareThis(_))
            | Self::Opcode(Opcodes::CheckThis)
            | Self::Opcode(Opcodes::ChainFaults)
            | Self::Opcode(Opcodes::OODeclExists(_))
            | Self::Opcode(Opcodes::VerifyParamType(_))
            | Self::Opcode(Opcodes::VerifyParamTypeTS(_))
            | Self::Opcode(Opcodes::VerifyOutType(_))
            | Self::Opcode(Opcodes::VerifyRetTypeC)
            | Self::Opcode(Opcodes::VerifyRetTypeTS)
            | Self::Opcode(Opcodes::SelfCls)
            | Self::Opcode(Opcodes::ParentCls)
            | Self::Opcode(Opcodes::LateBoundCls)
            | Self::Opcode(Opcodes::ClassName)
            | Self::Opcode(Opcodes::LazyClassFromClass)
            | Self::Opcode(Opcodes::RecordReifiedGeneric)
            | Self::Opcode(Opcodes::CheckReifiedGenericMismatch)
            | Self::Opcode(Opcodes::NativeImpl)
            | Self::Opcode(Opcodes::AKExists)
            | Self::Opcode(Opcodes::CreateCl(_, _))
            | Self::Opcode(Opcodes::Idx)
            | Self::Opcode(Opcodes::ArrayIdx)
            | Self::Opcode(Opcodes::ArrayMarkLegacy)
            | Self::Opcode(Opcodes::ArrayUnmarkLegacy)
            | Self::Opcode(Opcodes::AssertRATL(_, _))
            | Self::Opcode(Opcodes::AssertRATStk(_, _))
            | Self::Opcode(Opcodes::BreakTraceHint)
            | Self::Opcode(Opcodes::Silence(_, _))
            | Self::Opcode(Opcodes::GetMemoKeyL(_))
            | Self::Opcode(Opcodes::CGetCUNop)
            | Self::Opcode(Opcodes::UGetCUNop)
            | Self::Opcode(Opcodes::MemoSet(_))
            | Self::Opcode(Opcodes::MemoSetEager(_))
            | Self::Opcode(Opcodes::LockObj)
            | Self::Opcode(Opcodes::ThrowNonExhaustiveSwitch)
            | Self::Opcode(Opcodes::RaiseClassStringConversionWarning)
            | Self::Opcode(Opcodes::SetImplicitContextByValue)
            | Self::Opcode(Opcodes::RetCSuspended)
            | Self::Opcode(Opcodes::RetM(_))
            | Self::Opcode(Opcodes::Throw)
            | Self::Opcode(Opcodes::Nop)
            | Self::Opcode(Opcodes::EntryNop)
            | Self::Opcode(Opcodes::PopC)
            | Self::Opcode(Opcodes::PopU)
            | Self::Opcode(Opcodes::Dup)
            | Self::Opcode(Opcodes::IterFree(_))
            | Self::Opcode(Opcodes::Null)
            | Self::Opcode(Opcodes::True)
            | Self::Opcode(Opcodes::False)
            | Self::Opcode(Opcodes::NullUninit)
            | Self::Opcode(Opcodes::Int(_))
            | Self::Opcode(Opcodes::Double(_))
            | Self::Opcode(Opcodes::String(_))
            | Self::Opcode(Opcodes::LazyClass(_))
            | Self::Pseudo(Pseudo::TypedValue(_))
            | Self::Opcode(Opcodes::Vec(_))
            | Self::Opcode(Opcodes::Dict(_))
            | Self::Opcode(Opcodes::Keyset(_))
            | Self::Opcode(Opcodes::NewDictArray(_))
            | Self::Opcode(Opcodes::NewStructDict(_))
            | Self::Opcode(Opcodes::NewVec(_))
            | Self::Opcode(Opcodes::NewKeysetArray(_))
            | Self::Opcode(Opcodes::NewPair)
            | Self::Opcode(Opcodes::AddElemC)
            | Self::Opcode(Opcodes::AddNewElemC)
            | Self::Opcode(Opcodes::NewCol(_))
            | Self::Opcode(Opcodes::ColFromArray(_))
            | Self::Opcode(Opcodes::CnsE(_))
            | Self::Opcode(Opcodes::ClsCns(_))
            | Self::Opcode(Opcodes::ClsCnsD(_, _))
            | Self::Opcode(Opcodes::ClsCnsL(_))
            | Self::Opcode(Opcodes::File)
            | Self::Opcode(Opcodes::Dir)
            | Self::Opcode(Opcodes::Method)
            | Self::Opcode(Opcodes::FuncCred)
            | Self::Opcode(Opcodes::Concat)
            | Self::Opcode(Opcodes::ConcatN(_))
            | Self::Opcode(Opcodes::Add)
            | Self::Opcode(Opcodes::Sub)
            | Self::Opcode(Opcodes::Mul)
            | Self::Opcode(Opcodes::AddO)
            | Self::Opcode(Opcodes::SubO)
            | Self::Opcode(Opcodes::MulO)
            | Self::Opcode(Opcodes::Div)
            | Self::Opcode(Opcodes::Mod)
            | Self::Opcode(Opcodes::Pow)
            | Self::Opcode(Opcodes::Not)
            | Self::Opcode(Opcodes::Same)
            | Self::Opcode(Opcodes::NSame)
            | Self::Opcode(Opcodes::Eq)
            | Self::Opcode(Opcodes::Neq)
            | Self::Opcode(Opcodes::Lt)
            | Self::Opcode(Opcodes::Lte)
            | Self::Opcode(Opcodes::Gt)
            | Self::Opcode(Opcodes::Gte)
            | Self::Opcode(Opcodes::Cmp)
            | Self::Opcode(Opcodes::BitAnd)
            | Self::Opcode(Opcodes::BitOr)
            | Self::Opcode(Opcodes::BitXor)
            | Self::Opcode(Opcodes::BitNot)
            | Self::Opcode(Opcodes::Shl)
            | Self::Opcode(Opcodes::Shr)
            | Self::Opcode(Opcodes::CastBool)
            | Self::Opcode(Opcodes::CastInt)
            | Self::Opcode(Opcodes::CastDouble)
            | Self::Opcode(Opcodes::CastString)
            | Self::Opcode(Opcodes::CastVec)
            | Self::Opcode(Opcodes::CastDict)
            | Self::Opcode(Opcodes::CastKeyset)
            | Self::Opcode(Opcodes::InstanceOf)
            | Self::Opcode(Opcodes::InstanceOfD(_))
            | Self::Opcode(Opcodes::IsLateBoundCls)
            | Self::Opcode(Opcodes::IsTypeStructC(_))
            | Self::Opcode(Opcodes::ThrowAsTypeStructException)
            | Self::Opcode(Opcodes::CombineAndResolveTypeStruct(_))
            | Self::Opcode(Opcodes::Print)
            | Self::Opcode(Opcodes::Clone)
            | Self::Opcode(Opcodes::Exit)
            | Self::Opcode(Opcodes::Fatal(_))
            | Self::Opcode(Opcodes::ResolveFunc(_))
            | Self::Opcode(Opcodes::ResolveRFunc(_))
            | Self::Opcode(Opcodes::ResolveMethCaller(_))
            | Self::Opcode(Opcodes::ResolveClsMethod(_))
            | Self::Opcode(Opcodes::ResolveClsMethodD(_, _))
            | Self::Opcode(Opcodes::ResolveClsMethodS(_, _))
            | Self::Opcode(Opcodes::ResolveRClsMethod(_))
            | Self::Opcode(Opcodes::ResolveRClsMethodD(_, _))
            | Self::Opcode(Opcodes::ResolveRClsMethodS(_, _))
            | Self::Opcode(Opcodes::ResolveClass(_))
            | Self::Pseudo(Pseudo::Continue(_))
            | Self::Pseudo(Pseudo::Break(_))
            | Self::Opcode(Opcodes::NewObj)
            | Self::Opcode(Opcodes::NewObjR)
            | Self::Opcode(Opcodes::NewObjD(_))
            | Self::Opcode(Opcodes::NewObjRD(_))
            | Self::Opcode(Opcodes::NewObjS(_))
            | Self::Opcode(Opcodes::CGetL(_))
            | Self::Opcode(Opcodes::CGetQuietL(_))
            | Self::Opcode(Opcodes::CGetL2(_))
            | Self::Opcode(Opcodes::CUGetL(_))
            | Self::Opcode(Opcodes::PushL(_))
            | Self::Opcode(Opcodes::CGetG)
            | Self::Opcode(Opcodes::CGetS(_))
            | Self::Opcode(Opcodes::ClassGetC)
            | Self::Opcode(Opcodes::ClassGetTS)
            | Self::Opcode(Opcodes::SetL(_))
            | Self::Opcode(Opcodes::PopL(_))
            | Self::Opcode(Opcodes::SetG)
            | Self::Opcode(Opcodes::SetS(_))
            | Self::Opcode(Opcodes::SetOpL(_, _))
            | Self::Opcode(Opcodes::SetOpG(_))
            | Self::Opcode(Opcodes::SetOpS(_))
            | Self::Opcode(Opcodes::IncDecL(_, _))
            | Self::Opcode(Opcodes::IncDecG(_))
            | Self::Opcode(Opcodes::IncDecS(_))
            | Self::Opcode(Opcodes::UnsetL(_))
            | Self::Opcode(Opcodes::UnsetG)
            | Self::Opcode(Opcodes::CheckProp(_))
            | Self::Opcode(Opcodes::InitProp(_, _))
            | Self::Opcode(Opcodes::IssetL(_))
            | Self::Opcode(Opcodes::IssetG)
            | Self::Opcode(Opcodes::IssetS)
            | Self::Opcode(Opcodes::IsUnsetL(_))
            | Self::Opcode(Opcodes::IsTypeC(_))
            | Self::Opcode(Opcodes::IsTypeL(_, _))
            | Self::Opcode(Opcodes::BaseGC(_, _))
            | Self::Opcode(Opcodes::BaseGL(_, _))
            | Self::Opcode(Opcodes::BaseSC(_, _, _, _))
            | Self::Opcode(Opcodes::BaseL(_, _, _))
            | Self::Opcode(Opcodes::BaseC(_, _))
            | Self::Opcode(Opcodes::BaseH)
            | Self::Opcode(Opcodes::Dim(_, _))
            | Self::Opcode(Opcodes::QueryM(_, _, _))
            | Self::Opcode(Opcodes::SetM(_, _))
            | Self::Opcode(Opcodes::IncDecM(_, _, _))
            | Self::Opcode(Opcodes::SetOpM(_, _, _))
            | Self::Opcode(Opcodes::UnsetM(_, _))
            | Self::Opcode(Opcodes::SetRangeM(_, _, _))
            | Self::Pseudo(Pseudo::Label(_))
            | Self::Pseudo(Pseudo::TryCatchBegin)
            | Self::Pseudo(Pseudo::TryCatchMiddle)
            | Self::Pseudo(Pseudo::TryCatchEnd)
            | Self::Pseudo(Pseudo::Comment(_))
            | Self::Pseudo(Pseudo::SrcLoc(_))
            | Self::Opcode(Opcodes::WHResult)
            | Self::Opcode(Opcodes::Await)
            | Self::Opcode(Opcodes::AwaitAll(_))
            | Self::Opcode(Opcodes::CreateCont)
            | Self::Opcode(Opcodes::ContEnter)
            | Self::Opcode(Opcodes::ContRaise)
            | Self::Opcode(Opcodes::Yield)
            | Self::Opcode(Opcodes::YieldK)
            | Self::Opcode(Opcodes::ContCheck(_))
            | Self::Opcode(Opcodes::ContValid)
            | Self::Opcode(Opcodes::ContKey)
            | Self::Opcode(Opcodes::ContGetReturn)
            | Self::Opcode(Opcodes::ContCurrent)
            | Self::Opcode(Opcodes::Incl)
            | Self::Opcode(Opcodes::InclOnce)
            | Self::Opcode(Opcodes::Req)
            | Self::Opcode(Opcodes::ReqOnce)
            | Self::Opcode(Opcodes::ReqDoc)
            | Self::Opcode(Opcodes::Eval) => &mut [],
        }
    }
}
