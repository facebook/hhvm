// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use ffi::{
    BumpSliceMut,
    Maybe::{self, Just, Nothing},
    Pair, Slice, Str,
};
use hhbc_ast::*;
use hhvm_hhbc_defs_ffi::ffi::{
    BareThisOp, CollectionType, ContCheckOp, FatalOp, IncDecOp, InitPropOp, IsLogAsDynamicCallOp,
    IsTypeOp, MOpMode, ObjMethodOp, QueryMOp, ReadonlyOp, SetOpOp, SilenceOp, SpecialClsRef,
    SwitchKind, TypeStructResolveOp,
};
use iterator::IterId;
use label::Label;
use local::Local;
use oxidized::ast_defs::Pos;
use runtime::TypedValue;
use thiserror::Error;

pub type Result<T> = std::result::Result<T, Error>;

#[derive(Error, Debug)]
pub enum Error {
    #[error("IncludeTimeFatalException: FatalOp={0:?}, {1}")]
    IncludeTimeFatalException(FatalOp, Pos, std::string::String),

    #[error("Unrecoverable: {0}")]
    Unrecoverable(std::string::String),
}

pub fn unrecoverable(msg: impl std::convert::Into<std::string::String>) -> Error {
    Error::Unrecoverable(msg.into())
}

/// The various from_X functions below take some kind of AST
/// (expression, statement, etc.) and produce what is logically a
/// sequence of instructions. This could be represented by a list, but
/// we wish to avoid the quadratic complexity associated with repeated
/// appending. So, we build a tree of instructions which can be
/// flattened when complete.
#[derive(Debug, Clone)]
pub enum InstrSeq<'a> {
    List(Vec<Instruct<'a>>),
    Concat(Vec<InstrSeq<'a>>),
}

// The following iterator implementations produce streams of instruction lists
// (vecs or slices) and use an internal stack to flatten InstrSeq. The
// instruction lists can be manipulated directly, doing bulk opertaions like
// extend() or retain(), or flatten()'d once more into streams of instructions.
//
// Some tricks that were done for speed:
// * keep the top-of-stack iterator in a dedicated field
// * filter out empty lists - consumers of the iterator only see nonempty lists.
//
// Some other tricks didn't seem to help and were abandoned:
// * on Concat, if the current `top` iterator is empty, can just overwrite
// with a new iterator instead of pushing it and later having to pop it.
// * Skipping empty Concat sequences.
// * Special cases for 1-entry Lists.
//
// Future ideas to try:
// * use SmallVec for the stack. Can it eliminate the need for `top`?

/// An iterator that owns an InstrSeq and produces a stream of owned lists
/// of instructions: `Vec<Instruct>`.
#[derive(Debug)]
struct IntoListIter<'a> {
    // Keeping top-of-stack in its own field for speed.
    top: std::vec::IntoIter<InstrSeq<'a>>,
    stack: Vec<std::vec::IntoIter<InstrSeq<'a>>>,
}

impl<'a> IntoListIter<'a> {
    pub fn new(iseq: InstrSeq<'a>) -> Self {
        Self {
            top: match iseq {
                InstrSeq::List(_) => vec![iseq].into_iter(),
                InstrSeq::Concat(s) => s.into_iter(),
            },
            stack: Vec::new(),
        }
    }
}

impl<'a> Iterator for IntoListIter<'a> {
    type Item = Vec<Instruct<'a>>;
    fn next(&mut self) -> Option<Self::Item> {
        loop {
            match self.top.next() {
                // Short-circuit the empty list case for speed
                Some(InstrSeq::List(s)) if s.is_empty() => {}
                Some(InstrSeq::List(s)) => break Some(s),
                Some(InstrSeq::Concat(s)) => self
                    .stack
                    .push(std::mem::replace(&mut self.top, s.into_iter())),
                None => match self.stack.pop() {
                    Some(top) => self.top = top,
                    None => break None,
                },
            }
        }
    }
}

/// An iterator that borrows an InstrSeq and produces a stream of borrowed
/// slices of instructions: `&[Instruct]`.
#[derive(Debug)]
struct ListIter<'i, 'a> {
    // Keeping top-of-stack in its own field for speed.
    top: std::slice::Iter<'i, InstrSeq<'a>>,
    stack: Vec<std::slice::Iter<'i, InstrSeq<'a>>>,
}

impl<'i, 'a> ListIter<'i, 'a> {
    fn new(iseq: &'i InstrSeq<'a>) -> Self {
        Self {
            top: match iseq {
                InstrSeq::List(_) => std::slice::from_ref(iseq).iter(),
                InstrSeq::Concat(s) => s.iter(),
            },
            stack: Vec::new(),
        }
    }
}

impl<'i, 'a> Iterator for ListIter<'i, 'a> {
    type Item = &'i [Instruct<'a>];
    fn next(&mut self) -> Option<Self::Item> {
        loop {
            match self.top.next() {
                // Short-circuit the empty list case for speed
                Some(InstrSeq::List(s)) if s.is_empty() => {}
                Some(InstrSeq::List(s)) => break Some(s),
                Some(InstrSeq::Concat(s)) => {
                    self.stack.push(std::mem::replace(&mut self.top, s.iter()))
                }
                None => match self.stack.pop() {
                    Some(top) => self.top = top,
                    None => break None,
                },
            }
        }
    }
}

/// An iterator that borrows a mutable InstrSeq and produces a stream of
/// borrowed lists of instructions: `&mut Vec<Instruct>`. This is a borrowed
/// Vec instead of a borrowed slice so sub-sequences of instructions can
/// grow or shrink independently, for example by retain().
#[derive(Debug)]
struct ListIterMut<'i, 'a> {
    top: std::slice::IterMut<'i, InstrSeq<'a>>,
    stack: Vec<std::slice::IterMut<'i, InstrSeq<'a>>>,
}

impl<'i, 'a> ListIterMut<'i, 'a> {
    fn new(iseq: &'i mut InstrSeq<'a>) -> Self {
        Self {
            top: match iseq {
                InstrSeq::List(_) => std::slice::from_mut(iseq).iter_mut(),
                InstrSeq::Concat(s) => s.iter_mut(),
            },
            stack: Vec::new(),
        }
    }
}

impl<'i, 'a> Iterator for ListIterMut<'i, 'a> {
    type Item = &'i mut Vec<Instruct<'a>>;
    fn next(&mut self) -> Option<Self::Item> {
        loop {
            match self.top.next() {
                // Short-circuit the empty list case for speed
                Some(InstrSeq::List(s)) if s.is_empty() => {}
                Some(InstrSeq::List(s)) => break Some(s),
                Some(InstrSeq::Concat(s)) => self
                    .stack
                    .push(std::mem::replace(&mut self.top, s.iter_mut())),
                None => match self.stack.pop() {
                    Some(top) => self.top = top,
                    None => break None,
                },
            }
        }
    }
}

pub mod instr {
    use crate::*;

    pub fn empty<'a>() -> InstrSeq<'a> {
        InstrSeq::new_empty()
    }

    pub fn instr<'a>(i: Instruct<'a>) -> InstrSeq<'a> {
        InstrSeq::List(vec![i])
    }

    pub(crate) fn instrs<'a>(is: Vec<Instruct<'a>>) -> InstrSeq<'a> {
        InstrSeq::List(is)
    }

    pub fn lit_const<'a>(l: InstructLitConst<'a>) -> InstrSeq<'a> {
        instr(Instruct::LitConst(l))
    }

    pub fn iterinit<'a>(args: IterArgs<'a>, label: Label) -> InstrSeq<'a> {
        instr(Instruct::Iterator(InstructIterator::IterInit(args, label)))
    }

    pub fn iternext<'a>(args: IterArgs<'a>, label: Label) -> InstrSeq<'a> {
        instr(Instruct::Iterator(InstructIterator::IterNext(args, label)))
    }

    pub fn iternextk<'a>(
        id: IterId,
        label: Label,
        value: Local<'a>,
        key: Local<'a>,
    ) -> InstrSeq<'a> {
        let args = IterArgs {
            iter_id: id,
            key_id: Just(key),
            val_id: value,
        };
        instr(Instruct::Iterator(InstructIterator::IterNext(args, label)))
    }

    pub fn iterfree<'a>(id: IterId) -> InstrSeq<'a> {
        instr(Instruct::Iterator(InstructIterator::IterFree(id)))
    }

    pub fn whresult<'a>() -> InstrSeq<'a> {
        instr(Instruct::Async(AsyncFunctions::WHResult))
    }

    pub fn jmp<'a>(label: Label) -> InstrSeq<'a> {
        instr(Instruct::ContFlow(InstructControlFlow::Jmp(label)))
    }

    pub fn jmpz<'a>(label: Label) -> InstrSeq<'a> {
        instr(Instruct::ContFlow(InstructControlFlow::JmpZ(label)))
    }

    pub fn jmpnz<'a>(label: Label) -> InstrSeq<'a> {
        instr(Instruct::ContFlow(InstructControlFlow::JmpNZ(label)))
    }

    pub fn jmpns<'a>(label: Label) -> InstrSeq<'a> {
        instr(Instruct::ContFlow(InstructControlFlow::JmpNS(label)))
    }

    pub fn continue_<'a>(level: isize) -> InstrSeq<'a> {
        instr(Instruct::SpecialFlow(InstructSpecialFlow::Continue(level)))
    }

    pub fn break_<'a>(level: isize) -> InstrSeq<'a> {
        instr(Instruct::SpecialFlow(InstructSpecialFlow::Break(level)))
    }

    pub fn iter_break<'a>(label: Label, iters: Vec<IterId>) -> InstrSeq<'a> {
        let mut vec: Vec<Instruct<'a>> = iters
            .into_iter()
            .map(|id| Instruct::Iterator(InstructIterator::IterFree(id)))
            .collect();
        vec.push(Instruct::ContFlow(InstructControlFlow::Jmp(label)));
        instrs(vec)
    }

    pub fn false_<'a>() -> InstrSeq<'a> {
        instr(Instruct::LitConst(InstructLitConst::False))
    }

    pub fn true_<'a>() -> InstrSeq<'a> {
        instr(Instruct::LitConst(InstructLitConst::True))
    }

    pub fn clscnsd<'a>(const_id: ConstId<'a>, cid: ClassId<'a>) -> InstrSeq<'a> {
        instr(Instruct::LitConst(InstructLitConst::ClsCnsD(const_id, cid)))
    }

    pub fn clscns<'a>(const_id: ConstId<'a>) -> InstrSeq<'a> {
        instr(Instruct::LitConst(InstructLitConst::ClsCns(const_id)))
    }

    pub fn clscnsl<'a>(local: Local<'a>) -> InstrSeq<'a> {
        instr(Instruct::LitConst(InstructLitConst::ClsCnsL(local)))
    }

    pub fn eq<'a>() -> InstrSeq<'a> {
        instr(Instruct::Op(InstructOperator::Eq))
    }

    pub fn neq<'a>() -> InstrSeq<'a> {
        instr(Instruct::Op(InstructOperator::Neq))
    }

    pub fn gt<'a>() -> InstrSeq<'a> {
        instr(Instruct::Op(InstructOperator::Gt))
    }

    pub fn gte<'a>() -> InstrSeq<'a> {
        instr(Instruct::Op(InstructOperator::Gte))
    }

    pub fn lt<'a>() -> InstrSeq<'a> {
        instr(Instruct::Op(InstructOperator::Lt))
    }

    pub fn lte<'a>() -> InstrSeq<'a> {
        instr(Instruct::Op(InstructOperator::Lte))
    }

    pub fn concat<'a>() -> InstrSeq<'a> {
        instr(Instruct::Op(InstructOperator::Concat))
    }

    pub fn concatn<'a>(n: isize) -> InstrSeq<'a> {
        instr(Instruct::Op(InstructOperator::ConcatN(n)))
    }

    pub fn print<'a>() -> InstrSeq<'a> {
        instr(Instruct::Op(InstructOperator::Print))
    }

    pub fn cast_dict<'a>() -> InstrSeq<'a> {
        instr(Instruct::Op(InstructOperator::CastDict))
    }

    pub fn cast_string<'a>() -> InstrSeq<'a> {
        instr(Instruct::Op(InstructOperator::CastString))
    }

    pub fn cast_int<'a>() -> InstrSeq<'a> {
        instr(Instruct::Op(InstructOperator::CastInt))
    }

    pub fn cast_bool<'a>() -> InstrSeq<'a> {
        instr(Instruct::Op(InstructOperator::CastBool))
    }

    pub fn cast_double<'a>() -> InstrSeq<'a> {
        instr(Instruct::Op(InstructOperator::CastDouble))
    }

    pub fn retc<'a>() -> InstrSeq<'a> {
        instr(Instruct::ContFlow(InstructControlFlow::RetC))
    }

    pub fn retc_suspended<'a>() -> InstrSeq<'a> {
        instr(Instruct::ContFlow(InstructControlFlow::RetCSuspended))
    }

    pub fn retm<'a>(p: NumParams) -> InstrSeq<'a> {
        instr(Instruct::ContFlow(InstructControlFlow::RetM(p)))
    }

    pub fn null<'a>() -> InstrSeq<'a> {
        instr(Instruct::LitConst(InstructLitConst::Null))
    }

    pub fn nulluninit<'a>() -> InstrSeq<'a> {
        instr(Instruct::LitConst(InstructLitConst::NullUninit))
    }

    pub fn chain_faults<'a>() -> InstrSeq<'a> {
        instr(Instruct::Misc(InstructMisc::ChainFaults))
    }

    pub fn dup<'a>() -> InstrSeq<'a> {
        instr(Instruct::Basic(InstructBasic::Dup))
    }

    pub fn nop<'a>() -> InstrSeq<'a> {
        instr(Instruct::Basic(InstructBasic::Nop))
    }

    pub fn instanceofd<'a>(s: ClassId<'a>) -> InstrSeq<'a> {
        instr(Instruct::Op(InstructOperator::InstanceOfD(s)))
    }

    pub fn instanceof<'a>() -> InstrSeq<'a> {
        instr(Instruct::Op(InstructOperator::InstanceOf))
    }

    pub fn islateboundcls<'a>() -> InstrSeq<'a> {
        instr(Instruct::Op(InstructOperator::IsLateBoundCls))
    }

    pub fn istypestructc<'a>(mode: TypeStructResolveOp) -> InstrSeq<'a> {
        instr(Instruct::Op(InstructOperator::IsTypeStructC(mode)))
    }

    pub fn throwastypestructexception<'a>() -> InstrSeq<'a> {
        instr(Instruct::Op(InstructOperator::ThrowAsTypeStructException))
    }

    pub fn throw_non_exhaustive_switch<'a>() -> InstrSeq<'a> {
        instr(Instruct::Misc(InstructMisc::ThrowNonExhaustiveSwitch))
    }

    pub fn raise_class_string_conversion_warning<'a>() -> InstrSeq<'a> {
        instr(Instruct::Misc(
            InstructMisc::RaiseClassStringConversionWarning,
        ))
    }

    pub fn combine_and_resolve_type_struct<'a>(i: isize) -> InstrSeq<'a> {
        instr(Instruct::Op(InstructOperator::CombineAndResolveTypeStruct(
            i,
        )))
    }

    pub fn record_reified_generic<'a>() -> InstrSeq<'a> {
        instr(Instruct::Misc(InstructMisc::RecordReifiedGeneric))
    }

    pub fn check_reified_generic_mismatch<'a>() -> InstrSeq<'a> {
        instr(Instruct::Misc(InstructMisc::CheckReifiedGenericMismatch))
    }

    pub fn int<'a>(i: isize) -> InstrSeq<'a> {
        instr(Instruct::LitConst(InstructLitConst::Int(
            i.try_into().unwrap(),
        )))
    }

    pub fn int64<'a>(i: i64) -> InstrSeq<'a> {
        instr(Instruct::LitConst(InstructLitConst::Int(i)))
    }

    pub fn int_of_string<'a>(litstr: &str) -> InstrSeq<'a> {
        instr(Instruct::LitConst(InstructLitConst::Int(
            litstr.parse::<i64>().unwrap(),
        )))
    }

    pub fn double<'a>(alloc: &'a bumpalo::Bump, litstr: &str) -> InstrSeq<'a> {
        instr(Instruct::LitConst(InstructLitConst::Double(Str::from(
            bumpalo::collections::String::from_str_in(litstr, alloc).into_bump_str(),
        ))))
    }

    pub fn string<'a>(alloc: &'a bumpalo::Bump, litstr: impl Into<String>) -> InstrSeq<'a> {
        instr(Instruct::LitConst(InstructLitConst::String(Str::from(
            bumpalo::collections::String::from_str_in(litstr.into().as_str(), alloc)
                .into_bump_str(),
        ))))
    }

    pub fn this<'a>() -> InstrSeq<'a> {
        instr(Instruct::Misc(InstructMisc::This))
    }

    pub fn istypec<'a>(op: IsTypeOp) -> InstrSeq<'a> {
        instr(Instruct::Isset(InstructIsset::IsTypeC(op)))
    }

    pub fn istypel<'a>(id: Local<'a>, op: IsTypeOp) -> InstrSeq<'a> {
        instr(Instruct::Isset(InstructIsset::IsTypeL(id, op)))
    }

    pub fn add<'a>() -> InstrSeq<'a> {
        instr(Instruct::Op(InstructOperator::Add))
    }

    pub fn addo<'a>() -> InstrSeq<'a> {
        instr(Instruct::Op(InstructOperator::AddO))
    }

    pub fn sub<'a>() -> InstrSeq<'a> {
        instr(Instruct::Op(InstructOperator::Sub))
    }

    pub fn subo<'a>() -> InstrSeq<'a> {
        instr(Instruct::Op(InstructOperator::SubO))
    }

    pub fn mul<'a>() -> InstrSeq<'a> {
        instr(Instruct::Op(InstructOperator::Mul))
    }

    pub fn mulo<'a>() -> InstrSeq<'a> {
        instr(Instruct::Op(InstructOperator::MulO))
    }

    pub fn shl<'a>() -> InstrSeq<'a> {
        instr(Instruct::Op(InstructOperator::Shl))
    }

    pub fn shr<'a>() -> InstrSeq<'a> {
        instr(Instruct::Op(InstructOperator::Shr))
    }

    pub fn cmp<'a>() -> InstrSeq<'a> {
        instr(Instruct::Op(InstructOperator::Cmp))
    }

    pub fn mod_<'a>() -> InstrSeq<'a> {
        instr(Instruct::Op(InstructOperator::Mod))
    }

    pub fn div<'a>() -> InstrSeq<'a> {
        instr(Instruct::Op(InstructOperator::Div))
    }

    pub fn same<'a>() -> InstrSeq<'a> {
        instr(Instruct::Op(InstructOperator::Same))
    }

    pub fn pow<'a>() -> InstrSeq<'a> {
        instr(Instruct::Op(InstructOperator::Pow))
    }

    pub fn nsame<'a>() -> InstrSeq<'a> {
        instr(Instruct::Op(InstructOperator::NSame))
    }

    pub fn not<'a>() -> InstrSeq<'a> {
        instr(Instruct::Op(InstructOperator::Not))
    }

    pub fn bitnot<'a>() -> InstrSeq<'a> {
        instr(Instruct::Op(InstructOperator::BitNot))
    }

    pub fn bitand<'a>() -> InstrSeq<'a> {
        instr(Instruct::Op(InstructOperator::BitAnd))
    }

    pub fn bitor<'a>() -> InstrSeq<'a> {
        instr(Instruct::Op(InstructOperator::BitOr))
    }

    pub fn bitxor<'a>() -> InstrSeq<'a> {
        instr(Instruct::Op(InstructOperator::BitXor))
    }

    pub fn sets<'a>(readonly_op: ReadonlyOp) -> InstrSeq<'a> {
        instr(Instruct::Mutator(InstructMutator::SetS(readonly_op)))
    }

    pub fn setl<'a>(local: Local<'a>) -> InstrSeq<'a> {
        instr(Instruct::Mutator(InstructMutator::SetL(local)))
    }

    pub fn setg<'a>() -> InstrSeq<'a> {
        instr(Instruct::Mutator(InstructMutator::SetG))
    }

    pub fn unsetl<'a>(local: Local<'a>) -> InstrSeq<'a> {
        instr(Instruct::Mutator(InstructMutator::UnsetL(local)))
    }

    pub fn unsetg<'a>() -> InstrSeq<'a> {
        instr(Instruct::Mutator(InstructMutator::UnsetG))
    }

    pub fn incdecl<'a>(local: Local<'a>, op: IncDecOp) -> InstrSeq<'a> {
        instr(Instruct::Mutator(InstructMutator::IncDecL(local, op)))
    }

    pub fn incdecg<'a>(op: IncDecOp) -> InstrSeq<'a> {
        instr(Instruct::Mutator(InstructMutator::IncDecG(op)))
    }

    pub fn incdecs<'a>(op: IncDecOp) -> InstrSeq<'a> {
        instr(Instruct::Mutator(InstructMutator::IncDecS(op)))
    }

    pub fn setopg<'a>(op: SetOpOp) -> InstrSeq<'a> {
        instr(Instruct::Mutator(InstructMutator::SetOpG(op)))
    }

    pub fn setopl<'a>(local: Local<'a>, op: SetOpOp) -> InstrSeq<'a> {
        instr(Instruct::Mutator(InstructMutator::SetOpL(local, op)))
    }

    pub fn setops<'a>(op: SetOpOp) -> InstrSeq<'a> {
        instr(Instruct::Mutator(InstructMutator::SetOpS(op)))
    }

    pub fn issetl<'a>(local: Local<'a>) -> InstrSeq<'a> {
        instr(Instruct::Isset(InstructIsset::IssetL(local)))
    }

    pub fn issetg<'a>() -> InstrSeq<'a> {
        instr(Instruct::Isset(InstructIsset::IssetG))
    }

    pub fn issets<'a>() -> InstrSeq<'a> {
        instr(Instruct::Isset(InstructIsset::IssetS))
    }

    pub fn isunsetl<'a>(local: Local<'a>) -> InstrSeq<'a> {
        instr(Instruct::Isset(InstructIsset::IsUnsetL(local)))
    }

    pub fn cgets<'a>(readonly_op: ReadonlyOp) -> InstrSeq<'a> {
        instr(Instruct::Get(InstructGet::CGetS(readonly_op)))
    }

    pub fn cgetg<'a>() -> InstrSeq<'a> {
        instr(Instruct::Get(InstructGet::CGetG))
    }

    pub fn cgetl<'a>(local: Local<'a>) -> InstrSeq<'a> {
        instr(Instruct::Get(InstructGet::CGetL(local)))
    }

    pub fn cugetl<'a>(local: Local<'a>) -> InstrSeq<'a> {
        instr(Instruct::Get(InstructGet::CUGetL(local)))
    }

    pub fn cgetl2<'a>(local: Local<'a>) -> InstrSeq<'a> {
        instr(Instruct::Get(InstructGet::CGetL2(local)))
    }

    pub fn cgetquietl<'a>(local: Local<'a>) -> InstrSeq<'a> {
        instr(Instruct::Get(InstructGet::CGetQuietL(local)))
    }

    pub fn classgetc<'a>() -> InstrSeq<'a> {
        instr(Instruct::Get(InstructGet::ClassGetC))
    }

    pub fn classgetts<'a>() -> InstrSeq<'a> {
        instr(Instruct::Get(InstructGet::ClassGetTS))
    }

    pub fn classname<'a>() -> InstrSeq<'a> {
        instr(Instruct::Misc(InstructMisc::ClassName))
    }

    pub fn lazyclassfromclass<'a>() -> InstrSeq<'a> {
        instr(Instruct::Misc(InstructMisc::LazyClassFromClass))
    }

    pub fn self_<'a>() -> InstrSeq<'a> {
        instr(Instruct::Misc(InstructMisc::Self_))
    }

    pub fn lateboundcls<'a>() -> InstrSeq<'a> {
        instr(Instruct::Misc(InstructMisc::LateBoundCls))
    }

    pub fn parent<'a>() -> InstrSeq<'a> {
        instr(Instruct::Misc(InstructMisc::Parent))
    }

    pub fn popu<'a>() -> InstrSeq<'a> {
        instr(Instruct::Basic(InstructBasic::PopU))
    }

    pub fn popc<'a>() -> InstrSeq<'a> {
        instr(Instruct::Basic(InstructBasic::PopC))
    }

    pub fn popl<'a>(l: Local<'a>) -> InstrSeq<'a> {
        instr(Instruct::Mutator(InstructMutator::PopL(l)))
    }

    pub fn initprop<'a>(pid: PropId<'a>, op: InitPropOp) -> InstrSeq<'a> {
        instr(Instruct::Mutator(InstructMutator::InitProp(pid, op)))
    }

    pub fn checkprop<'a>(pid: PropId<'a>) -> InstrSeq<'a> {
        instr(Instruct::Mutator(InstructMutator::CheckProp(pid)))
    }

    pub fn pushl<'a>(local: Local<'a>) -> InstrSeq<'a> {
        instr(Instruct::Get(InstructGet::PushL(local)))
    }

    pub fn throw<'a>() -> InstrSeq<'a> {
        instr(Instruct::ContFlow(InstructControlFlow::Throw))
    }

    pub fn new_vec_array<'a>(i: isize) -> InstrSeq<'a> {
        instr(Instruct::LitConst(InstructLitConst::NewVec(i)))
    }

    pub fn new_pair<'a>() -> InstrSeq<'a> {
        instr(Instruct::LitConst(InstructLitConst::NewPair))
    }

    pub fn add_elemc<'a>() -> InstrSeq<'a> {
        instr(Instruct::LitConst(InstructLitConst::AddElemC))
    }

    pub fn add_new_elemc<'a>() -> InstrSeq<'a> {
        instr(Instruct::LitConst(InstructLitConst::AddNewElemC))
    }

    pub fn switch<'a>(
        alloc: &'a bumpalo::Bump,
        targets: bumpalo::collections::Vec<'a, Label>,
    ) -> InstrSeq<'a> {
        instr(Instruct::ContFlow(InstructControlFlow::Switch {
            kind: SwitchKind::Unbounded,
            base: 0,
            targets: BumpSliceMut::new(alloc, targets.into_bump_slice_mut()),
        }))
    }

    pub fn newobj<'a>() -> InstrSeq<'a> {
        instr(Instruct::New(InstructNew::NewObj))
    }

    pub fn sswitch<'a>(
        alloc: &'a bumpalo::Bump,
        cases: bumpalo::collections::Vec<'a, (&'a str, Label)>,
    ) -> InstrSeq<'a> {
        let targets = BumpSliceMut::new(
            alloc,
            alloc.alloc_slice_fill_iter(cases.iter().map(|(_, target)| *target)),
        );
        let cases = BumpSliceMut::new(
            alloc,
            alloc.alloc_slice_fill_iter(cases.into_iter().map(|(s, _)| Str::from(s))),
        );
        instr(Instruct::ContFlow(InstructControlFlow::SSwitch {
            cases,
            targets,
        }))
    }

    pub fn newobjr<'a>() -> InstrSeq<'a> {
        instr(Instruct::New(InstructNew::NewObjR))
    }

    pub fn newobjd<'a>(id: ClassId<'a>) -> InstrSeq<'a> {
        instr(Instruct::New(InstructNew::NewObjD(id)))
    }

    pub fn newobjrd<'a>(id: ClassId<'a>) -> InstrSeq<'a> {
        instr(Instruct::New(InstructNew::NewObjRD(id)))
    }

    pub fn newobjs<'a>(scref: SpecialClsRef) -> InstrSeq<'a> {
        instr(Instruct::New(InstructNew::NewObjS(scref)))
    }

    pub fn lockobj<'a>() -> InstrSeq<'a> {
        instr(Instruct::Misc(InstructMisc::LockObj))
    }

    pub fn clone<'a>() -> InstrSeq<'a> {
        instr(Instruct::Op(InstructOperator::Clone))
    }

    pub fn newstructdict<'a>(alloc: &'a bumpalo::Bump, keys: &'a [&'a str]) -> InstrSeq<'a> {
        let keys = Slice::new(alloc.alloc_slice_fill_iter(keys.iter().map(|s| Str::from(*s))));
        instr(Instruct::LitConst(InstructLitConst::NewStructDict(keys)))
    }

    pub fn newcol<'a>(collection_type: CollectionType) -> InstrSeq<'a> {
        instr(Instruct::LitConst(InstructLitConst::NewCol(
            collection_type,
        )))
    }

    pub fn colfromarray<'a>(collection_type: CollectionType) -> InstrSeq<'a> {
        instr(Instruct::LitConst(InstructLitConst::ColFromArray(
            collection_type,
        )))
    }

    pub fn entrynop<'a>() -> InstrSeq<'a> {
        instr(Instruct::Basic(InstructBasic::EntryNop))
    }

    pub fn typedvalue<'a>(xs: TypedValue<'a>) -> InstrSeq<'a> {
        instr(Instruct::LitConst(InstructLitConst::TypedValue(xs)))
    }

    pub fn basel<'a>(local: Local<'a>, mode: MOpMode, readonly_op: ReadonlyOp) -> InstrSeq<'a> {
        instr(Instruct::Base(InstructBase::BaseL(
            local,
            mode,
            readonly_op,
        )))
    }

    pub fn basec<'a>(stack_index: StackIndex, mode: MOpMode) -> InstrSeq<'a> {
        instr(Instruct::Base(InstructBase::BaseC(stack_index, mode)))
    }

    pub fn basegc<'a>(stack_index: StackIndex, mode: MOpMode) -> InstrSeq<'a> {
        instr(Instruct::Base(InstructBase::BaseGC(stack_index, mode)))
    }

    pub fn basegl<'a>(local: Local<'a>, mode: MOpMode) -> InstrSeq<'a> {
        instr(Instruct::Base(InstructBase::BaseGL(local, mode)))
    }

    pub fn basesc<'a>(
        y: StackIndex,
        z: StackIndex,
        mode: MOpMode,
        readonly_op: ReadonlyOp,
    ) -> InstrSeq<'a> {
        instr(Instruct::Base(InstructBase::BaseSC(
            y,
            z,
            mode,
            readonly_op,
        )))
    }

    pub fn baseh<'a>() -> InstrSeq<'a> {
        instr(Instruct::Base(InstructBase::BaseH))
    }

    pub fn cgetcunop<'a>() -> InstrSeq<'a> {
        instr(Instruct::Misc(InstructMisc::CGetCUNop))
    }

    pub fn ugetcunop<'a>() -> InstrSeq<'a> {
        instr(Instruct::Misc(InstructMisc::UGetCUNop))
    }

    pub fn memoget<'a>(label: Label, range: Option<(Local<'a>, isize)>) -> InstrSeq<'a> {
        instr(Instruct::Misc(InstructMisc::MemoGet(
            label,
            match range {
                Some((fst, snd)) => Just(Pair(fst, snd)),
                None => Nothing,
            },
        )))
    }

    // Factored out to reduce verbosity.
    fn range_opt_to_maybe<'a>(range: Option<(Local<'a>, isize)>) -> Maybe<Pair<Local<'a>, isize>> {
        match range {
            Some((fst, snd)) => Just(Pair(fst, snd)),
            None => Nothing,
        }
    }

    pub fn memoget_eager<'a>(
        label1: Label,
        label2: Label,
        range: Option<(Local<'a>, isize)>,
    ) -> InstrSeq<'a> {
        instr(Instruct::Misc(InstructMisc::MemoGetEager(
            [label1, label2],
            range_opt_to_maybe(range),
        )))
    }

    pub fn memoset<'a>(range: Option<(Local<'a>, isize)>) -> InstrSeq<'a> {
        instr(Instruct::Misc(InstructMisc::MemoSet(range_opt_to_maybe(
            range,
        ))))
    }

    pub fn memoset_eager<'a>(range: Option<(Local<'a>, isize)>) -> InstrSeq<'a> {
        instr(Instruct::Misc(InstructMisc::MemoSetEager(
            range_opt_to_maybe(range),
        )))
    }

    pub fn getmemokeyl<'a>(local: Local<'a>) -> InstrSeq<'a> {
        instr(Instruct::Misc(InstructMisc::GetMemoKeyL(local)))
    }

    pub fn barethis<'a>(notice: BareThisOp) -> InstrSeq<'a> {
        instr(Instruct::Misc(InstructMisc::BareThis(notice)))
    }

    pub fn checkthis<'a>() -> InstrSeq<'a> {
        instr(Instruct::Misc(InstructMisc::CheckThis))
    }

    pub fn verify_ret_type_c<'a>() -> InstrSeq<'a> {
        instr(Instruct::Misc(InstructMisc::VerifyRetTypeC))
    }

    pub fn verify_ret_type_ts<'a>() -> InstrSeq<'a> {
        instr(Instruct::Misc(InstructMisc::VerifyRetTypeTS))
    }

    pub fn verify_out_type<'a>(i: ParamId<'a>) -> InstrSeq<'a> {
        instr(Instruct::Misc(InstructMisc::VerifyOutType(i)))
    }

    pub fn verify_param_type<'a>(i: ParamId<'a>) -> InstrSeq<'a> {
        instr(Instruct::Misc(InstructMisc::VerifyParamType(i)))
    }

    pub fn verify_param_type_ts<'a>(i: ParamId<'a>) -> InstrSeq<'a> {
        instr(Instruct::Misc(InstructMisc::VerifyParamTypeTS(i)))
    }

    pub fn dim<'a>(op: MOpMode, key: MemberKey<'a>) -> InstrSeq<'a> {
        instr(Instruct::Base(InstructBase::Dim(op, key)))
    }

    pub fn dim_warn_pt<'a>(key: PropId<'a>, readonly_op: ReadonlyOp) -> InstrSeq<'a> {
        dim(MOpMode::Warn, MemberKey::PT(key, readonly_op))
    }

    pub fn dim_define_pt<'a>(key: PropId<'a>, readonly_op: ReadonlyOp) -> InstrSeq<'a> {
        dim(MOpMode::Define, MemberKey::PT(key, readonly_op))
    }

    pub fn fcallclsmethod<'a>(
        log: IsLogAsDynamicCallOp,
        fcall_args: FcallArgs<'a>,
    ) -> InstrSeq<'a> {
        instr(Instruct::Call(InstructCall::FCallClsMethod {
            fcall_args,
            log,
        }))
    }

    pub fn fcallclsmethodd<'a>(
        fcall_args: FcallArgs<'a>,
        method: MethodId<'a>,
        class: ClassId<'a>,
    ) -> InstrSeq<'a> {
        instr(Instruct::Call(InstructCall::FCallClsMethodD {
            fcall_args,
            class,
            method,
        }))
    }

    pub fn fcallclsmethods<'a>(fcall_args: FcallArgs<'a>, clsref: SpecialClsRef) -> InstrSeq<'a> {
        instr(Instruct::Call(InstructCall::FCallClsMethodS {
            fcall_args,
            clsref,
        }))
    }

    pub fn fcallclsmethodsd<'a>(
        fcall_args: FcallArgs<'a>,
        clsref: SpecialClsRef,
        method: MethodId<'a>,
    ) -> InstrSeq<'a> {
        instr(Instruct::Call(InstructCall::FCallClsMethodSD {
            fcall_args,
            clsref,
            method,
        }))
    }

    pub fn fcallctor<'a>(fcall_args: FcallArgs<'a>) -> InstrSeq<'a> {
        instr(Instruct::Call(InstructCall::FCallCtor(fcall_args)))
    }

    pub fn fcallfunc<'a>(fcall_args: FcallArgs<'a>) -> InstrSeq<'a> {
        instr(Instruct::Call(InstructCall::FCallFunc(fcall_args)))
    }

    pub fn fcallfuncd<'a>(fcall_args: FcallArgs<'a>, func: FunctionId<'a>) -> InstrSeq<'a> {
        instr(Instruct::Call(InstructCall::FCallFuncD {
            fcall_args,
            func,
        }))
    }

    pub fn fcallobjmethod<'a>(fcall_args: FcallArgs<'a>, flavor: ObjMethodOp) -> InstrSeq<'a> {
        instr(Instruct::Call(InstructCall::FCallObjMethod {
            fcall_args,
            flavor,
        }))
    }

    pub fn fcallobjmethodd<'a>(
        fcall_args: FcallArgs<'a>,
        method: MethodId<'a>,
        flavor: ObjMethodOp,
    ) -> InstrSeq<'a> {
        instr(Instruct::Call(InstructCall::FCallObjMethodD {
            fcall_args,
            flavor,
            method,
        }))
    }

    pub fn fcallobjmethodd_nullthrows<'a>(
        fcall_args: FcallArgs<'a>,
        method: MethodId<'a>,
    ) -> InstrSeq<'a> {
        fcallobjmethodd(fcall_args, method, ObjMethodOp::NullThrows)
    }

    pub fn querym<'a>(num_params: NumParams, op: QueryMOp, key: MemberKey<'a>) -> InstrSeq<'a> {
        instr(Instruct::Final(InstructFinal::QueryM(num_params, op, key)))
    }

    pub fn querym_cget_pt<'a>(
        num_params: NumParams,
        key: PropId<'a>,
        readonly_op: ReadonlyOp,
    ) -> InstrSeq<'a> {
        querym(num_params, QueryMOp::CGet, MemberKey::PT(key, readonly_op))
    }

    pub fn setm<'a>(num_params: NumParams, key: MemberKey<'a>) -> InstrSeq<'a> {
        instr(Instruct::Final(InstructFinal::SetM(num_params, key)))
    }

    pub fn unsetm<'a>(num_params: NumParams, key: MemberKey<'a>) -> InstrSeq<'a> {
        instr(Instruct::Final(InstructFinal::UnsetM(num_params, key)))
    }

    pub fn setopm<'a>(num_params: NumParams, op: SetOpOp, key: MemberKey<'a>) -> InstrSeq<'a> {
        instr(Instruct::Final(InstructFinal::SetOpM(num_params, op, key)))
    }

    pub fn incdecm<'a>(num_params: NumParams, op: IncDecOp, key: MemberKey<'a>) -> InstrSeq<'a> {
        instr(Instruct::Final(InstructFinal::IncDecM(num_params, op, key)))
    }

    pub fn setm_pt<'a>(
        num_params: NumParams,
        key: PropId<'a>,
        readonly_op: ReadonlyOp,
    ) -> InstrSeq<'a> {
        setm(num_params, MemberKey::PT(key, readonly_op))
    }

    pub fn resolve_func<'a>(func_id: FunctionId<'a>) -> InstrSeq<'a> {
        instr(Instruct::Op(InstructOperator::ResolveFunc(func_id)))
    }

    pub fn resolve_rfunc<'a>(func_id: FunctionId<'a>) -> InstrSeq<'a> {
        instr(Instruct::Op(InstructOperator::ResolveRFunc(func_id)))
    }

    pub fn resolveclsmethod<'a>(method_id: MethodId<'a>) -> InstrSeq<'a> {
        instr(Instruct::Op(InstructOperator::ResolveClsMethod(method_id)))
    }

    pub fn resolveclsmethodd<'a>(class_id: ClassId<'a>, method_id: MethodId<'a>) -> InstrSeq<'a> {
        instr(Instruct::Op(InstructOperator::ResolveClsMethodD(
            class_id, method_id,
        )))
    }

    pub fn resolveclsmethods<'a>(scref: SpecialClsRef, method_id: MethodId<'a>) -> InstrSeq<'a> {
        instr(Instruct::Op(InstructOperator::ResolveClsMethodS(
            scref, method_id,
        )))
    }

    pub fn resolverclsmethod<'a>(method_id: MethodId<'a>) -> InstrSeq<'a> {
        instr(Instruct::Op(InstructOperator::ResolveRClsMethod(method_id)))
    }

    pub fn resolverclsmethodd<'a>(class_id: ClassId<'a>, method_id: MethodId<'a>) -> InstrSeq<'a> {
        instr(Instruct::Op(InstructOperator::ResolveRClsMethodD(
            class_id, method_id,
        )))
    }

    pub fn resolverclsmethods<'a>(scref: SpecialClsRef, method_id: MethodId<'a>) -> InstrSeq<'a> {
        instr(Instruct::Op(InstructOperator::ResolveRClsMethodS(
            scref, method_id,
        )))
    }

    pub fn resolve_meth_caller<'a>(fun_id: FunctionId<'a>) -> InstrSeq<'a> {
        instr(Instruct::Op(InstructOperator::ResolveMethCaller(fun_id)))
    }

    pub fn resolveclass<'a>(class_id: ClassId<'a>) -> InstrSeq<'a> {
        instr(Instruct::Op(InstructOperator::ResolveClass(class_id)))
    }

    pub fn lazyclass<'a>(class_id: ClassId<'a>) -> InstrSeq<'a> {
        instr(Instruct::LitConst(InstructLitConst::LazyClass(class_id)))
    }

    pub fn oodeclexists<'a>(class_kind: ClassishKind) -> InstrSeq<'a> {
        instr(Instruct::Misc(InstructMisc::OODeclExists(class_kind)))
    }

    pub fn fatal<'a>(op: FatalOp) -> InstrSeq<'a> {
        instr(Instruct::Op(InstructOperator::Fatal(op)))
    }

    pub fn await_<'a>() -> InstrSeq<'a> {
        instr(Instruct::Async(AsyncFunctions::Await))
    }

    pub fn yield_<'a>() -> InstrSeq<'a> {
        instr(Instruct::Generator(GenCreationExecution::Yield))
    }

    pub fn yieldk<'a>() -> InstrSeq<'a> {
        instr(Instruct::Generator(GenCreationExecution::YieldK))
    }

    pub fn createcont<'a>() -> InstrSeq<'a> {
        instr(Instruct::Generator(GenCreationExecution::CreateCont))
    }

    pub fn awaitall<'a>(range: Option<(Local<'a>, isize)>) -> InstrSeq<'a> {
        instr(Instruct::Async(AsyncFunctions::AwaitAll(
            range_opt_to_maybe(range),
        )))
    }

    pub fn label<'a>(label: Label) -> InstrSeq<'a> {
        instr(Instruct::Label(label))
    }

    pub fn awaitall_list<'a>(unnamed_locals: Vec<Local<'a>>) -> InstrSeq<'a> {
        use Local::Unnamed;
        match unnamed_locals.split_first() {
            None => panic!("Expected at least one await"),
            Some((head, tail)) => {
                if let Unnamed(head_id) = head {
                    let mut prev_id = head_id;
                    for unnamed_local in tail {
                        match unnamed_local {
                            Unnamed(id) => {
                                assert_eq!(prev_id.idx + 1, id.idx);
                                prev_id = id;
                            }
                            _ => panic!("Expected unnamed local"),
                        }
                    }
                    awaitall(Some((
                        Unnamed(*head_id),
                        unnamed_locals.len().try_into().unwrap(),
                    )))
                } else {
                    panic!("Expected unnamed local")
                }
            }
        }
    }

    pub fn exit<'a>() -> InstrSeq<'a> {
        instr(Instruct::Op(InstructOperator::Exit))
    }

    pub fn idx<'a>() -> InstrSeq<'a> {
        instr(Instruct::Misc(InstructMisc::Idx))
    }

    pub fn array_idx<'a>() -> InstrSeq<'a> {
        instr(Instruct::Misc(InstructMisc::ArrayIdx))
    }

    pub fn createcl<'a>(param_num: NumParams, cls_num: ClassNum) -> InstrSeq<'a> {
        instr(Instruct::Misc(InstructMisc::CreateCl(param_num, cls_num)))
    }

    pub fn eval<'a>() -> InstrSeq<'a> {
        instr(Instruct::IncludeEvalDefine(InstructIncludeEvalDefine::Eval))
    }

    pub fn incl<'a>() -> InstrSeq<'a> {
        instr(Instruct::IncludeEvalDefine(InstructIncludeEvalDefine::Incl))
    }

    pub fn inclonce<'a>() -> InstrSeq<'a> {
        instr(Instruct::IncludeEvalDefine(
            InstructIncludeEvalDefine::InclOnce,
        ))
    }

    pub fn req<'a>() -> InstrSeq<'a> {
        instr(Instruct::IncludeEvalDefine(InstructIncludeEvalDefine::Req))
    }

    pub fn reqdoc<'a>() -> InstrSeq<'a> {
        instr(Instruct::IncludeEvalDefine(
            InstructIncludeEvalDefine::ReqDoc,
        ))
    }

    pub fn reqonce<'a>() -> InstrSeq<'a> {
        instr(Instruct::IncludeEvalDefine(
            InstructIncludeEvalDefine::ReqOnce,
        ))
    }

    pub fn silence_start<'a>(local: Local<'a>) -> InstrSeq<'a> {
        instr(Instruct::Misc(InstructMisc::Silence(
            local,
            SilenceOp::Start,
        )))
    }

    pub fn silence_end<'a>(local: Local<'a>) -> InstrSeq<'a> {
        instr(Instruct::Misc(InstructMisc::Silence(local, SilenceOp::End)))
    }

    pub fn contcheck_check<'a>() -> InstrSeq<'a> {
        instr(Instruct::Generator(GenCreationExecution::ContCheck(
            ContCheckOp::CheckStarted,
        )))
    }

    pub fn contcheck_ignore<'a>() -> InstrSeq<'a> {
        instr(Instruct::Generator(GenCreationExecution::ContCheck(
            ContCheckOp::IgnoreStarted,
        )))
    }

    pub fn contenter<'a>() -> InstrSeq<'a> {
        instr(Instruct::Generator(GenCreationExecution::ContEnter))
    }

    pub fn contraise<'a>() -> InstrSeq<'a> {
        instr(Instruct::Generator(GenCreationExecution::ContRaise))
    }

    pub fn contvalid<'a>() -> InstrSeq<'a> {
        instr(Instruct::Generator(GenCreationExecution::ContValid))
    }

    pub fn contcurrent<'a>() -> InstrSeq<'a> {
        instr(Instruct::Generator(GenCreationExecution::ContCurrent))
    }

    pub fn contkey<'a>() -> InstrSeq<'a> {
        instr(Instruct::Generator(GenCreationExecution::ContKey))
    }

    pub fn contgetreturn<'a>() -> InstrSeq<'a> {
        instr(Instruct::Generator(GenCreationExecution::ContGetReturn))
    }

    pub fn nativeimpl<'a>() -> InstrSeq<'a> {
        instr(Instruct::Misc(InstructMisc::NativeImpl))
    }

    pub fn srcloc<'a>(
        line_begin: isize,
        line_end: isize,
        col_begin: isize,
        col_end: isize,
    ) -> InstrSeq<'a> {
        instr(Instruct::SrcLoc(SrcLoc {
            line_begin,
            line_end,
            col_begin,
            col_end,
        }))
    }

    pub fn is_type_structc_resolve<'a>() -> InstrSeq<'a> {
        instr(Instruct::Op(InstructOperator::IsTypeStructC(
            TypeStructResolveOp::Resolve,
        )))
    }

    pub fn is_type_structc_dontresolve<'a>() -> InstrSeq<'a> {
        instr(Instruct::Op(InstructOperator::IsTypeStructC(
            TypeStructResolveOp::DontResolve,
        )))
    }
}

impl<'a> InstrSeq<'a> {
    /// Produce an empty instruction sequence.
    pub fn new_empty() -> Self {
        InstrSeq::List(Vec::new())
    }

    /// Transitional version. We mean to write a `gather!` in the future.
    pub fn gather(mut iss: Vec<InstrSeq<'a>>) -> Self {
        iss.retain(|iseq| match iseq {
            InstrSeq::List(s) if s.is_empty() => false,
            _ => true,
        });
        if iss.is_empty() {
            InstrSeq::new_empty()
        } else {
            InstrSeq::Concat(iss)
        }
    }

    pub fn iter<'i>(&'i self) -> impl Iterator<Item = &'i Instruct<'a>> {
        ListIter::new(self).flatten()
    }

    pub fn iter_mut<'i>(&'i mut self) -> impl Iterator<Item = &'i mut Instruct<'a>> {
        ListIterMut::new(self).flatten()
    }

    fn full_len(&self) -> usize {
        ListIter::new(self).map(|s| s.len()).sum()
    }

    pub fn compact(self, alloc: &'a bumpalo::Bump) -> Slice<'a, Instruct<'a>> {
        let mut v = bumpalo::collections::Vec::with_capacity_in(self.full_len(), alloc);
        for list in IntoListIter::new(self) {
            let len = v.len();
            let start = if len > 0 && Self::is_srcloc(&v[len - 1]) {
                // v ends with a SrcLoc; back up so we can compact it if eligible.
                len - 1
            } else {
                len
            };
            v.extend(list);
            let mut i = start;
            let len = v.len();
            for j in start..len {
                if Self::is_srcloc(&v[j]) && j + 1 < len && Self::is_srcloc(&v[j + 1]) {
                    // skip v[j]
                } else {
                    if i < j {
                        v[i] = v[j].clone();
                    }
                    i += 1;
                }
            }
            v.truncate(i);
        }
        Slice::new(v.into_bump_slice())
    }

    pub fn create_try_catch(
        label_gen: &mut label::Gen,
        opt_done_label: Option<Label>,
        skip_throw: bool,
        try_instrs: Self,
        catch_instrs: Self,
    ) -> Self {
        let done_label = match opt_done_label {
            Some(l) => l,
            None => label_gen.next_regular(),
        };
        InstrSeq::gather(vec![
            instr::instr(Instruct::Try(InstructTry::TryCatchBegin)),
            try_instrs,
            instr::jmp(done_label),
            instr::instr(Instruct::Try(InstructTry::TryCatchMiddle)),
            catch_instrs,
            if skip_throw {
                instr::empty()
            } else {
                instr::instr(Instruct::ContFlow(InstructControlFlow::Throw))
            },
            instr::instr(Instruct::Try(InstructTry::TryCatchEnd)),
            instr::label(done_label),
        ])
    }

    /// Test whether `i` is of case `Instruct::SrcLoc`.
    fn is_srcloc(instruction: &Instruct<'a>) -> bool {
        matches!(instruction, Instruct::SrcLoc(_))
    }

    /// Return the first non-SrcLoc instruction.
    pub fn first(&self) -> Option<&Instruct<'a>> {
        self.iter().find(|&i| !Self::is_srcloc(i))
    }

    /// Test for the empty instruction sequence, ignoring SrcLocs
    pub fn is_empty(&self) -> bool {
        self.iter().all(Self::is_srcloc)
    }

    pub fn retain(&mut self, mut f: impl FnMut(&Instruct<'a>) -> bool) {
        for s in ListIterMut::new(self) {
            s.retain(&mut f)
        }
    }

    pub fn retain_mut<F>(&mut self, mut f: F)
    where
        F: FnMut(&mut Instruct<'a>) -> bool,
    {
        for s in ListIterMut::new(self) {
            *s = std::mem::take(s)
                .into_iter()
                .filter_map(|mut instr| match f(&mut instr) {
                    true => Some(instr),
                    false => None,
                })
                .collect()
        }
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use instr::{instr, instrs};
    use pretty_assertions::assert_eq;

    #[test]
    fn iter() {
        let mk_i = || Instruct::Comment(Str::from(""));
        let empty = InstrSeq::new_empty;

        let one = || instr(mk_i());
        let list0 = || instrs(vec![]);
        let list1 = || instrs(vec![mk_i()]);
        let list2 = || instrs(vec![mk_i(), mk_i()]);
        let concat0 = || InstrSeq::Concat(vec![]);
        let concat1 = || InstrSeq::Concat(vec![one()]);

        assert_eq!(empty().iter().count(), 0);
        assert_eq!(one().iter().count(), 1);
        assert_eq!(list0().iter().count(), 0);
        assert_eq!(list1().iter().count(), 1);
        assert_eq!(list2().iter().count(), 2);
        assert_eq!(concat0().iter().count(), 0);
        assert_eq!(concat1().iter().count(), 1);

        let concat = InstrSeq::Concat(vec![empty()]);
        assert_eq!(concat.iter().count(), 0);

        let concat = InstrSeq::Concat(vec![empty(), one()]);
        assert_eq!(concat.iter().count(), 1);

        let concat = InstrSeq::Concat(vec![one(), empty()]);
        assert_eq!(concat.iter().count(), 1);

        let concat = InstrSeq::Concat(vec![one(), list1()]);
        assert_eq!(concat.iter().count(), 2);

        let concat = InstrSeq::Concat(vec![list2(), list1()]);
        assert_eq!(concat.iter().count(), 3);

        let concat = InstrSeq::Concat(vec![concat0(), list2(), list1()]);
        assert_eq!(concat.iter().count(), 3);

        let concat = InstrSeq::Concat(vec![concat1(), concat1()]);
        assert_eq!(concat.iter().count(), 2);

        let concat = InstrSeq::Concat(vec![concat0(), concat1()]);
        assert_eq!(concat.iter().count(), 1);

        let concat = InstrSeq::Concat(vec![list2(), concat1()]);
        assert_eq!(concat.iter().count(), 3);

        let concat = InstrSeq::Concat(vec![list2(), concat0()]);
        assert_eq!(concat.iter().count(), 2);

        let concat = InstrSeq::Concat(vec![one(), concat0()]);
        assert_eq!(concat.iter().count(), 1);

        let concat = InstrSeq::Concat(vec![empty(), concat0()]);
        assert_eq!(concat.iter().count(), 0);
    }
}
