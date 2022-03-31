// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use ffi::{BumpSliceMut, Slice, Str};
use hhbc_ast::*;
use iterator::IterId;
use label::Label;
use local::Local;
use oxidized::ast_defs::Pos;
use thiserror::Error;

pub type Result<T, E = Error> = std::result::Result<T, E>;

#[derive(Error, Debug)]
#[error(transparent)]
pub struct Error(Box<ErrorKind>);

impl Error {
    pub fn unrecoverable(msg: impl Into<String>) -> Self {
        Self(Box::new(ErrorKind::Unrecoverable(msg.into())))
    }

    pub fn fatal_runtime(pos: &Pos, msg: impl Into<String>) -> Self {
        Self(Box::new(ErrorKind::IncludeTimeFatalException(
            FatalOp::Runtime,
            pos.clone(),
            msg.into(),
        )))
    }

    pub fn fatal_parse(pos: &Pos, msg: impl Into<String>) -> Self {
        Self(Box::new(ErrorKind::IncludeTimeFatalException(
            FatalOp::Parse,
            pos.clone(),
            msg.into(),
        )))
    }

    pub fn kind(&self) -> &ErrorKind {
        &self.0
    }

    pub fn into_kind(self) -> ErrorKind {
        *self.0
    }
}

#[derive(Error, Debug)]
pub enum ErrorKind {
    #[error("IncludeTimeFatalException: FatalOp={0:?}, {1}")]
    IncludeTimeFatalException(FatalOp, Pos, std::string::String),

    #[error("Unrecoverable: {0}")]
    Unrecoverable(std::string::String),
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

    pub fn iterinit<'a>(args: IterArgs, label: Label) -> InstrSeq<'a> {
        instr(Instruct::Opcode(Opcode::IterInit(args, label)))
    }

    pub fn iternext<'a>(args: IterArgs, label: Label) -> InstrSeq<'a> {
        instr(Instruct::Opcode(Opcode::IterNext(args, label)))
    }

    pub fn iterfree<'a>(id: IterId) -> InstrSeq<'a> {
        instr(Instruct::Opcode(Opcode::IterFree(id)))
    }

    pub fn whresult<'a>() -> InstrSeq<'a> {
        instr(Instruct::Opcode(Opcode::WHResult))
    }

    pub fn jmp<'a>(label: Label) -> InstrSeq<'a> {
        instr(Instruct::Opcode(Opcode::Jmp(label)))
    }

    pub fn jmpz<'a>(label: Label) -> InstrSeq<'a> {
        instr(Instruct::Opcode(Opcode::JmpZ(label)))
    }

    pub fn jmpnz<'a>(label: Label) -> InstrSeq<'a> {
        instr(Instruct::Opcode(Opcode::JmpNZ(label)))
    }

    pub fn jmpns<'a>(label: Label) -> InstrSeq<'a> {
        instr(Instruct::Opcode(Opcode::JmpNS(label)))
    }

    pub fn continue_<'a>(level: isize) -> InstrSeq<'a> {
        instr(Instruct::Pseudo(Pseudo::Continue(level)))
    }

    pub fn break_<'a>(level: isize) -> InstrSeq<'a> {
        instr(Instruct::Pseudo(Pseudo::Break(level)))
    }

    pub fn iter_break<'a>(label: Label, iters: Vec<IterId>) -> InstrSeq<'a> {
        let mut vec: Vec<Instruct<'a>> = iters
            .into_iter()
            .map(|i| Instruct::Opcode(Opcode::IterFree(i)))
            .collect();
        vec.push(Instruct::Opcode(Opcode::Jmp(label)));
        instrs(vec)
    }

    pub fn false_<'a>() -> InstrSeq<'a> {
        instr(Instruct::Opcode(Opcode::False))
    }

    pub fn true_<'a>() -> InstrSeq<'a> {
        instr(Instruct::Opcode(Opcode::True))
    }

    pub fn clscnsd<'a>(const_id: ConstId<'a>, cid: ClassId<'a>) -> InstrSeq<'a> {
        instr(Instruct::Opcode(Opcode::ClsCnsD(const_id, cid)))
    }

    pub fn clscns<'a>(const_id: ConstId<'a>) -> InstrSeq<'a> {
        instr(Instruct::Opcode(Opcode::ClsCns(const_id)))
    }

    pub fn clscnsl<'a>(local: Local) -> InstrSeq<'a> {
        instr(Instruct::Opcode(Opcode::ClsCnsL(local)))
    }

    pub fn eq<'a>() -> InstrSeq<'a> {
        instr(Instruct::Opcode(Opcode::Eq))
    }

    pub fn neq<'a>() -> InstrSeq<'a> {
        instr(Instruct::Opcode(Opcode::Neq))
    }

    pub fn gt<'a>() -> InstrSeq<'a> {
        instr(Instruct::Opcode(Opcode::Gt))
    }

    pub fn gte<'a>() -> InstrSeq<'a> {
        instr(Instruct::Opcode(Opcode::Gte))
    }

    pub fn lt<'a>() -> InstrSeq<'a> {
        instr(Instruct::Opcode(Opcode::Lt))
    }

    pub fn lte<'a>() -> InstrSeq<'a> {
        instr(Instruct::Opcode(Opcode::Lte))
    }

    pub fn concat<'a>() -> InstrSeq<'a> {
        instr(Instruct::Opcode(Opcode::Concat))
    }

    pub fn concatn<'a>(n: u32) -> InstrSeq<'a> {
        instr(Instruct::Opcode(Opcode::ConcatN(n)))
    }

    pub fn print<'a>() -> InstrSeq<'a> {
        instr(Instruct::Opcode(Opcode::Print))
    }

    pub fn cast_dict<'a>() -> InstrSeq<'a> {
        instr(Instruct::Opcode(Opcode::CastDict))
    }

    pub fn cast_string<'a>() -> InstrSeq<'a> {
        instr(Instruct::Opcode(Opcode::CastString))
    }

    pub fn cast_int<'a>() -> InstrSeq<'a> {
        instr(Instruct::Opcode(Opcode::CastInt))
    }

    pub fn cast_bool<'a>() -> InstrSeq<'a> {
        instr(Instruct::Opcode(Opcode::CastBool))
    }

    pub fn cast_double<'a>() -> InstrSeq<'a> {
        instr(Instruct::Opcode(Opcode::CastDouble))
    }

    pub fn retc<'a>() -> InstrSeq<'a> {
        instr(Instruct::Opcode(Opcode::RetC))
    }

    pub fn retc_suspended<'a>() -> InstrSeq<'a> {
        instr(Instruct::Opcode(Opcode::RetCSuspended))
    }

    pub fn retm<'a>(p: NumParams) -> InstrSeq<'a> {
        instr(Instruct::Opcode(Opcode::RetM(p)))
    }

    pub fn null<'a>() -> InstrSeq<'a> {
        instr(Instruct::Opcode(Opcode::Null))
    }

    pub fn nulluninit<'a>() -> InstrSeq<'a> {
        instr(Instruct::Opcode(Opcode::NullUninit))
    }

    pub fn chain_faults<'a>() -> InstrSeq<'a> {
        instr(Instruct::Opcode(Opcode::ChainFaults))
    }

    pub fn dup<'a>() -> InstrSeq<'a> {
        instr(Instruct::Opcode(Opcode::Dup))
    }

    pub fn nop<'a>() -> InstrSeq<'a> {
        instr(Instruct::Opcode(Opcode::Nop))
    }

    pub fn instanceofd<'a>(s: ClassId<'a>) -> InstrSeq<'a> {
        instr(Instruct::Opcode(Opcode::InstanceOfD(s)))
    }

    pub fn instanceof<'a>() -> InstrSeq<'a> {
        instr(Instruct::Opcode(Opcode::InstanceOf))
    }

    pub fn islateboundcls<'a>() -> InstrSeq<'a> {
        instr(Instruct::Opcode(Opcode::IsLateBoundCls))
    }

    pub fn istypestructc<'a>(mode: TypeStructResolveOp) -> InstrSeq<'a> {
        instr(Instruct::Opcode(Opcode::IsTypeStructC(mode)))
    }

    pub fn throwastypestructexception<'a>() -> InstrSeq<'a> {
        instr(Instruct::Opcode(Opcode::ThrowAsTypeStructException))
    }

    pub fn throw_non_exhaustive_switch<'a>() -> InstrSeq<'a> {
        instr(Instruct::Opcode(Opcode::ThrowNonExhaustiveSwitch))
    }

    pub fn raise_class_string_conversion_warning<'a>() -> InstrSeq<'a> {
        instr(Instruct::Opcode(Opcode::RaiseClassStringConversionWarning))
    }

    pub fn combine_and_resolve_type_struct<'a>(i: u32) -> InstrSeq<'a> {
        instr(Instruct::Opcode(Opcode::CombineAndResolveTypeStruct(i)))
    }

    pub fn record_reified_generic<'a>() -> InstrSeq<'a> {
        instr(Instruct::Opcode(Opcode::RecordReifiedGeneric))
    }

    pub fn check_reified_generic_mismatch<'a>() -> InstrSeq<'a> {
        instr(Instruct::Opcode(Opcode::CheckReifiedGenericMismatch))
    }

    pub fn int<'a>(i: i64) -> InstrSeq<'a> {
        instr(Instruct::Opcode(Opcode::Int(i)))
    }

    pub fn double<'a>(f: f64) -> InstrSeq<'a> {
        instr(Instruct::Opcode(Opcode::Double(f)))
    }

    pub fn string<'a>(alloc: &'a bumpalo::Bump, litstr: impl Into<String>) -> InstrSeq<'a> {
        instr(Instruct::Opcode(Opcode::String(Str::from(
            bumpalo::collections::String::from_str_in(litstr.into().as_str(), alloc)
                .into_bump_str(),
        ))))
    }

    pub fn this<'a>() -> InstrSeq<'a> {
        instr(Instruct::Opcode(Opcode::This))
    }

    pub fn istypec<'a>(op: IsTypeOp) -> InstrSeq<'a> {
        instr(Instruct::Opcode(Opcode::IsTypeC(op)))
    }

    pub fn istypel<'a>(id: Local, op: IsTypeOp) -> InstrSeq<'a> {
        instr(Instruct::Opcode(Opcode::IsTypeL(id, op)))
    }

    pub fn add<'a>() -> InstrSeq<'a> {
        instr(Instruct::Opcode(Opcode::Add))
    }

    pub fn addo<'a>() -> InstrSeq<'a> {
        instr(Instruct::Opcode(Opcode::AddO))
    }

    pub fn sub<'a>() -> InstrSeq<'a> {
        instr(Instruct::Opcode(Opcode::Sub))
    }

    pub fn subo<'a>() -> InstrSeq<'a> {
        instr(Instruct::Opcode(Opcode::SubO))
    }

    pub fn mul<'a>() -> InstrSeq<'a> {
        instr(Instruct::Opcode(Opcode::Mul))
    }

    pub fn mulo<'a>() -> InstrSeq<'a> {
        instr(Instruct::Opcode(Opcode::MulO))
    }

    pub fn shl<'a>() -> InstrSeq<'a> {
        instr(Instruct::Opcode(Opcode::Shl))
    }

    pub fn shr<'a>() -> InstrSeq<'a> {
        instr(Instruct::Opcode(Opcode::Shr))
    }

    pub fn cmp<'a>() -> InstrSeq<'a> {
        instr(Instruct::Opcode(Opcode::Cmp))
    }

    pub fn mod_<'a>() -> InstrSeq<'a> {
        instr(Instruct::Opcode(Opcode::Mod))
    }

    pub fn div<'a>() -> InstrSeq<'a> {
        instr(Instruct::Opcode(Opcode::Div))
    }

    pub fn same<'a>() -> InstrSeq<'a> {
        instr(Instruct::Opcode(Opcode::Same))
    }

    pub fn pow<'a>() -> InstrSeq<'a> {
        instr(Instruct::Opcode(Opcode::Pow))
    }

    pub fn nsame<'a>() -> InstrSeq<'a> {
        instr(Instruct::Opcode(Opcode::NSame))
    }

    pub fn not<'a>() -> InstrSeq<'a> {
        instr(Instruct::Opcode(Opcode::Not))
    }

    pub fn bitnot<'a>() -> InstrSeq<'a> {
        instr(Instruct::Opcode(Opcode::BitNot))
    }

    pub fn bitand<'a>() -> InstrSeq<'a> {
        instr(Instruct::Opcode(Opcode::BitAnd))
    }

    pub fn bitor<'a>() -> InstrSeq<'a> {
        instr(Instruct::Opcode(Opcode::BitOr))
    }

    pub fn bitxor<'a>() -> InstrSeq<'a> {
        instr(Instruct::Opcode(Opcode::BitXor))
    }

    pub fn sets<'a>(readonly_op: ReadonlyOp) -> InstrSeq<'a> {
        instr(Instruct::Opcode(Opcode::SetS(readonly_op)))
    }

    pub fn setl<'a>(local: Local) -> InstrSeq<'a> {
        instr(Instruct::Opcode(Opcode::SetL(local)))
    }

    pub fn setg<'a>() -> InstrSeq<'a> {
        instr(Instruct::Opcode(Opcode::SetG))
    }

    pub fn unsetl<'a>(local: Local) -> InstrSeq<'a> {
        instr(Instruct::Opcode(Opcode::UnsetL(local)))
    }

    pub fn unsetg<'a>() -> InstrSeq<'a> {
        instr(Instruct::Opcode(Opcode::UnsetG))
    }

    pub fn incdecl<'a>(local: Local, op: IncDecOp) -> InstrSeq<'a> {
        instr(Instruct::Opcode(Opcode::IncDecL(local, op)))
    }

    pub fn incdecg<'a>(op: IncDecOp) -> InstrSeq<'a> {
        instr(Instruct::Opcode(Opcode::IncDecG(op)))
    }

    pub fn incdecs<'a>(op: IncDecOp) -> InstrSeq<'a> {
        instr(Instruct::Opcode(Opcode::IncDecS(op)))
    }

    pub fn setopg<'a>(op: SetOpOp) -> InstrSeq<'a> {
        instr(Instruct::Opcode(Opcode::SetOpG(op)))
    }

    pub fn setopl<'a>(local: Local, op: SetOpOp) -> InstrSeq<'a> {
        instr(Instruct::Opcode(Opcode::SetOpL(local, op)))
    }

    pub fn setops<'a>(op: SetOpOp) -> InstrSeq<'a> {
        instr(Instruct::Opcode(Opcode::SetOpS(op)))
    }

    pub fn issetl<'a>(local: Local) -> InstrSeq<'a> {
        instr(Instruct::Opcode(Opcode::IssetL(local)))
    }

    pub fn issetg<'a>() -> InstrSeq<'a> {
        instr(Instruct::Opcode(Opcode::IssetG))
    }

    pub fn issets<'a>() -> InstrSeq<'a> {
        instr(Instruct::Opcode(Opcode::IssetS))
    }

    pub fn isunsetl<'a>(local: Local) -> InstrSeq<'a> {
        instr(Instruct::Opcode(Opcode::IsUnsetL(local)))
    }

    pub fn cgets<'a>(readonly_op: ReadonlyOp) -> InstrSeq<'a> {
        instr(Instruct::Opcode(Opcode::CGetS(readonly_op)))
    }

    pub fn cgetg<'a>() -> InstrSeq<'a> {
        instr(Instruct::Opcode(Opcode::CGetG))
    }

    pub fn cgetl<'a>(local: Local) -> InstrSeq<'a> {
        instr(Instruct::Opcode(Opcode::CGetL(local)))
    }

    pub fn cugetl<'a>(local: Local) -> InstrSeq<'a> {
        instr(Instruct::Opcode(Opcode::CUGetL(local)))
    }

    pub fn cgetl2<'a>(local: Local) -> InstrSeq<'a> {
        instr(Instruct::Opcode(Opcode::CGetL2(local)))
    }

    pub fn cgetquietl<'a>(local: Local) -> InstrSeq<'a> {
        instr(Instruct::Opcode(Opcode::CGetQuietL(local)))
    }

    pub fn classgetc<'a>() -> InstrSeq<'a> {
        instr(Instruct::Opcode(Opcode::ClassGetC))
    }

    pub fn classgetts<'a>() -> InstrSeq<'a> {
        instr(Instruct::Opcode(Opcode::ClassGetTS))
    }

    pub fn classname<'a>() -> InstrSeq<'a> {
        instr(Instruct::Opcode(Opcode::ClassName))
    }

    pub fn lazyclassfromclass<'a>() -> InstrSeq<'a> {
        instr(Instruct::Opcode(Opcode::LazyClassFromClass))
    }

    pub fn selfcls<'a>() -> InstrSeq<'a> {
        instr(Instruct::Opcode(Opcode::SelfCls))
    }

    pub fn lateboundcls<'a>() -> InstrSeq<'a> {
        instr(Instruct::Opcode(Opcode::LateBoundCls))
    }

    pub fn parentcls<'a>() -> InstrSeq<'a> {
        instr(Instruct::Opcode(Opcode::ParentCls))
    }

    pub fn popu<'a>() -> InstrSeq<'a> {
        instr(Instruct::Opcode(Opcode::PopU))
    }

    pub fn popc<'a>() -> InstrSeq<'a> {
        instr(Instruct::Opcode(Opcode::PopC))
    }

    pub fn popl<'a>(l: Local) -> InstrSeq<'a> {
        instr(Instruct::Opcode(Opcode::PopL(l)))
    }

    pub fn initprop<'a>(pid: PropId<'a>, op: InitPropOp) -> InstrSeq<'a> {
        instr(Instruct::Opcode(Opcode::InitProp(pid, op)))
    }

    pub fn checkprop<'a>(pid: PropId<'a>) -> InstrSeq<'a> {
        instr(Instruct::Opcode(Opcode::CheckProp(pid)))
    }

    pub fn pushl<'a>(local: Local) -> InstrSeq<'a> {
        instr(Instruct::Opcode(Opcode::PushL(local)))
    }

    pub fn throw<'a>() -> InstrSeq<'a> {
        instr(Instruct::Opcode(Opcode::Throw))
    }

    pub fn new_vec_array<'a>(i: u32) -> InstrSeq<'a> {
        instr(Instruct::Opcode(Opcode::NewVec(i)))
    }

    pub fn new_pair<'a>() -> InstrSeq<'a> {
        instr(Instruct::Opcode(Opcode::NewPair))
    }

    pub fn add_elemc<'a>() -> InstrSeq<'a> {
        instr(Instruct::Opcode(Opcode::AddElemC))
    }

    pub fn add_new_elemc<'a>() -> InstrSeq<'a> {
        instr(Instruct::Opcode(Opcode::AddNewElemC))
    }

    pub fn switch<'a>(
        alloc: &'a bumpalo::Bump,
        targets: bumpalo::collections::Vec<'a, Label>,
    ) -> InstrSeq<'a> {
        instr(Instruct::Opcode(Opcode::Switch(
            SwitchKind::Unbounded,
            0,
            BumpSliceMut::new(alloc, targets.into_bump_slice_mut()),
        )))
    }

    pub fn newobj<'a>() -> InstrSeq<'a> {
        instr(Instruct::Opcode(Opcode::NewObj))
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
        instr(Instruct::Opcode(Opcode::SSwitch { cases, targets }))
    }

    pub fn newobjr<'a>() -> InstrSeq<'a> {
        instr(Instruct::Opcode(Opcode::NewObjR))
    }

    pub fn newobjd<'a>(id: ClassId<'a>) -> InstrSeq<'a> {
        instr(Instruct::Opcode(Opcode::NewObjD(id)))
    }

    pub fn vec<'a>(id: AdataId<'a>) -> InstrSeq<'a> {
        instr(Instruct::Opcode(Opcode::Vec(id)))
    }

    pub fn dict<'a>(id: AdataId<'a>) -> InstrSeq<'a> {
        instr(Instruct::Opcode(Opcode::Dict(id)))
    }

    pub fn keyset<'a>(id: AdataId<'a>) -> InstrSeq<'a> {
        instr(Instruct::Opcode(Opcode::Keyset(id)))
    }

    pub fn newobjrd<'a>(id: ClassId<'a>) -> InstrSeq<'a> {
        instr(Instruct::Opcode(Opcode::NewObjRD(id)))
    }

    pub fn newobjs<'a>(scref: SpecialClsRef) -> InstrSeq<'a> {
        instr(Instruct::Opcode(Opcode::NewObjS(scref)))
    }

    pub fn lockobj<'a>() -> InstrSeq<'a> {
        instr(Instruct::Opcode(Opcode::LockObj))
    }

    pub fn clone<'a>() -> InstrSeq<'a> {
        instr(Instruct::Opcode(Opcode::Clone))
    }

    pub fn newstructdict<'a>(alloc: &'a bumpalo::Bump, keys: &'a [&'a str]) -> InstrSeq<'a> {
        let keys = Slice::new(alloc.alloc_slice_fill_iter(keys.iter().map(|s| Str::from(*s))));
        instr(Instruct::Opcode(Opcode::NewStructDict(keys)))
    }

    pub fn newcol<'a>(collection_type: CollectionType) -> InstrSeq<'a> {
        instr(Instruct::Opcode(Opcode::NewCol(collection_type)))
    }

    pub fn colfromarray<'a>(collection_type: CollectionType) -> InstrSeq<'a> {
        instr(Instruct::Opcode(Opcode::ColFromArray(collection_type)))
    }

    pub fn entrynop<'a>() -> InstrSeq<'a> {
        instr(Instruct::Opcode(Opcode::EntryNop))
    }

    pub fn basel<'a>(local: Local, mode: MOpMode, readonly_op: ReadonlyOp) -> InstrSeq<'a> {
        instr(Instruct::Opcode(Opcode::BaseL(local, mode, readonly_op)))
    }

    pub fn basec<'a>(stack_index: StackIndex, mode: MOpMode) -> InstrSeq<'a> {
        instr(Instruct::Opcode(Opcode::BaseC(stack_index, mode)))
    }

    pub fn basegc<'a>(stack_index: StackIndex, mode: MOpMode) -> InstrSeq<'a> {
        instr(Instruct::Opcode(Opcode::BaseGC(stack_index, mode)))
    }

    pub fn basegl<'a>(local: Local, mode: MOpMode) -> InstrSeq<'a> {
        instr(Instruct::Opcode(Opcode::BaseGL(local, mode)))
    }

    pub fn basesc<'a>(
        y: StackIndex,
        z: StackIndex,
        mode: MOpMode,
        readonly_op: ReadonlyOp,
    ) -> InstrSeq<'a> {
        instr(Instruct::Opcode(Opcode::BaseSC(y, z, mode, readonly_op)))
    }

    pub fn baseh<'a>() -> InstrSeq<'a> {
        instr(Instruct::Opcode(Opcode::BaseH))
    }

    pub fn cgetcunop<'a>() -> InstrSeq<'a> {
        instr(Instruct::Opcode(Opcode::CGetCUNop))
    }

    pub fn ugetcunop<'a>() -> InstrSeq<'a> {
        instr(Instruct::Opcode(Opcode::UGetCUNop))
    }

    pub fn memoget<'a>(label: Label, range: LocalRange) -> InstrSeq<'a> {
        instr(Instruct::Opcode(Opcode::MemoGet(label, range)))
    }

    pub fn memoget_eager<'a>(label1: Label, label2: Label, range: LocalRange) -> InstrSeq<'a> {
        instr(Instruct::Opcode(Opcode::MemoGetEager(
            [label1, label2],
            range,
        )))
    }

    pub fn memoset<'a>(range: LocalRange) -> InstrSeq<'a> {
        instr(Instruct::Opcode(Opcode::MemoSet(range)))
    }

    pub fn memoset_eager<'a>(range: LocalRange) -> InstrSeq<'a> {
        instr(Instruct::Opcode(Opcode::MemoSetEager(range)))
    }

    pub fn getmemokeyl<'a>(local: Local) -> InstrSeq<'a> {
        instr(Instruct::Opcode(Opcode::GetMemoKeyL(local)))
    }

    pub fn barethis<'a>(notice: BareThisOp) -> InstrSeq<'a> {
        instr(Instruct::Opcode(Opcode::BareThis(notice)))
    }

    pub fn checkthis<'a>() -> InstrSeq<'a> {
        instr(Instruct::Opcode(Opcode::CheckThis))
    }

    pub fn verify_ret_type_c<'a>() -> InstrSeq<'a> {
        instr(Instruct::Opcode(Opcode::VerifyRetTypeC))
    }

    pub fn verify_ret_type_ts<'a>() -> InstrSeq<'a> {
        instr(Instruct::Opcode(Opcode::VerifyRetTypeTS))
    }

    pub fn verify_out_type<'a>(i: ParamId<'a>) -> InstrSeq<'a> {
        instr(Instruct::Opcode(Opcode::VerifyOutType(i)))
    }

    pub fn verify_param_type<'a>(i: ParamId<'a>) -> InstrSeq<'a> {
        instr(Instruct::Opcode(Opcode::VerifyParamType(i)))
    }

    pub fn verify_param_type_ts<'a>(i: ParamId<'a>) -> InstrSeq<'a> {
        instr(Instruct::Opcode(Opcode::VerifyParamTypeTS(i)))
    }

    pub fn dim<'a>(op: MOpMode, key: MemberKey<'a>) -> InstrSeq<'a> {
        instr(Instruct::Opcode(Opcode::Dim(op, key)))
    }

    pub fn dim_warn_pt<'a>(key: PropId<'a>, readonly_op: ReadonlyOp) -> InstrSeq<'a> {
        dim(MOpMode::Warn, MemberKey::PT(key, readonly_op))
    }

    pub fn dim_define_pt<'a>(key: PropId<'a>, readonly_op: ReadonlyOp) -> InstrSeq<'a> {
        dim(MOpMode::Define, MemberKey::PT(key, readonly_op))
    }

    pub fn fcallclsmethod<'a>(
        log: IsLogAsDynamicCallOp,
        fcall_args: FCallArgs<'a>,
    ) -> InstrSeq<'a> {
        instr(Instruct::Opcode(Opcode::FCallClsMethod(
            fcall_args,
            Default::default(),
            log,
        )))
    }

    pub fn fcallclsmethodd<'a>(
        fcall_args: FCallArgs<'a>,
        method: MethodId<'a>,
        class: ClassId<'a>,
    ) -> InstrSeq<'a> {
        instr(Instruct::Opcode(Opcode::FCallClsMethodD(
            fcall_args,
            Default::default(),
            class,
            method,
        )))
    }

    pub fn fcallclsmethods<'a>(fcall_args: FCallArgs<'a>, clsref: SpecialClsRef) -> InstrSeq<'a> {
        instr(Instruct::Opcode(Opcode::FCallClsMethodS(
            fcall_args,
            Default::default(),
            clsref,
        )))
    }

    pub fn fcallclsmethodsd<'a>(
        fcall_args: FCallArgs<'a>,
        clsref: SpecialClsRef,
        method: MethodId<'a>,
    ) -> InstrSeq<'a> {
        instr(Instruct::Opcode(Opcode::FCallClsMethodSD(
            fcall_args,
            Default::default(),
            clsref,
            method,
        )))
    }

    pub fn fcallctor<'a>(fcall_args: FCallArgs<'a>) -> InstrSeq<'a> {
        instr(Instruct::Opcode(Opcode::FCallCtor(
            fcall_args,
            Default::default(),
        )))
    }

    pub fn fcallfunc<'a>(fcall_args: FCallArgs<'a>) -> InstrSeq<'a> {
        instr(Instruct::Opcode(Opcode::FCallFunc(fcall_args)))
    }

    pub fn fcallfuncd<'a>(fcall_args: FCallArgs<'a>, func: FunctionId<'a>) -> InstrSeq<'a> {
        instr(Instruct::Opcode(Opcode::FCallFuncD(fcall_args, func)))
    }

    pub fn fcallobjmethod<'a>(fcall_args: FCallArgs<'a>, flavor: ObjMethodOp) -> InstrSeq<'a> {
        instr(Instruct::Opcode(Opcode::FCallObjMethod(
            fcall_args,
            Default::default(),
            flavor,
        )))
    }

    pub fn fcallobjmethodd<'a>(
        fcall_args: FCallArgs<'a>,
        method: MethodId<'a>,
        flavor: ObjMethodOp,
    ) -> InstrSeq<'a> {
        instr(Instruct::Opcode(Opcode::FCallObjMethodD(
            fcall_args,
            Default::default(),
            flavor,
            method,
        )))
    }

    pub fn fcallobjmethodd_nullthrows<'a>(
        fcall_args: FCallArgs<'a>,
        method: MethodId<'a>,
    ) -> InstrSeq<'a> {
        fcallobjmethodd(fcall_args, method, ObjMethodOp::NullThrows)
    }

    pub fn querym<'a>(num_params: NumParams, op: QueryMOp, key: MemberKey<'a>) -> InstrSeq<'a> {
        instr(Instruct::Opcode(Opcode::QueryM(num_params, op, key)))
    }

    pub fn querym_cget_pt<'a>(
        num_params: NumParams,
        key: PropId<'a>,
        readonly_op: ReadonlyOp,
    ) -> InstrSeq<'a> {
        querym(num_params, QueryMOp::CGet, MemberKey::PT(key, readonly_op))
    }

    pub fn setm<'a>(num_params: NumParams, key: MemberKey<'a>) -> InstrSeq<'a> {
        instr(Instruct::Opcode(Opcode::SetM(num_params, key)))
    }

    pub fn unsetm<'a>(num_params: NumParams, key: MemberKey<'a>) -> InstrSeq<'a> {
        instr(Instruct::Opcode(Opcode::UnsetM(num_params, key)))
    }

    pub fn setopm<'a>(num_params: NumParams, op: SetOpOp, key: MemberKey<'a>) -> InstrSeq<'a> {
        instr(Instruct::Opcode(Opcode::SetOpM(num_params, op, key)))
    }

    pub fn incdecm<'a>(num_params: NumParams, op: IncDecOp, key: MemberKey<'a>) -> InstrSeq<'a> {
        instr(Instruct::Opcode(Opcode::IncDecM(num_params, op, key)))
    }

    pub fn setm_pt<'a>(
        num_params: NumParams,
        key: PropId<'a>,
        readonly_op: ReadonlyOp,
    ) -> InstrSeq<'a> {
        setm(num_params, MemberKey::PT(key, readonly_op))
    }

    pub fn resolve_func<'a>(func_id: FunctionId<'a>) -> InstrSeq<'a> {
        instr(Instruct::Opcode(Opcode::ResolveFunc(func_id)))
    }

    pub fn resolve_rfunc<'a>(func_id: FunctionId<'a>) -> InstrSeq<'a> {
        instr(Instruct::Opcode(Opcode::ResolveRFunc(func_id)))
    }

    pub fn resolveclsmethod<'a>(method_id: MethodId<'a>) -> InstrSeq<'a> {
        instr(Instruct::Opcode(Opcode::ResolveClsMethod(method_id)))
    }

    pub fn resolveclsmethodd<'a>(class_id: ClassId<'a>, method_id: MethodId<'a>) -> InstrSeq<'a> {
        instr(Instruct::Opcode(Opcode::ResolveClsMethodD(
            class_id, method_id,
        )))
    }

    pub fn resolveclsmethods<'a>(scref: SpecialClsRef, method_id: MethodId<'a>) -> InstrSeq<'a> {
        instr(Instruct::Opcode(Opcode::ResolveClsMethodS(
            scref, method_id,
        )))
    }

    pub fn resolverclsmethod<'a>(method_id: MethodId<'a>) -> InstrSeq<'a> {
        instr(Instruct::Opcode(Opcode::ResolveRClsMethod(method_id)))
    }

    pub fn resolverclsmethodd<'a>(class_id: ClassId<'a>, method_id: MethodId<'a>) -> InstrSeq<'a> {
        instr(Instruct::Opcode(Opcode::ResolveRClsMethodD(
            class_id, method_id,
        )))
    }

    pub fn resolverclsmethods<'a>(scref: SpecialClsRef, method_id: MethodId<'a>) -> InstrSeq<'a> {
        instr(Instruct::Opcode(Opcode::ResolveRClsMethodS(
            scref, method_id,
        )))
    }

    pub fn resolve_meth_caller<'a>(fun_id: FunctionId<'a>) -> InstrSeq<'a> {
        instr(Instruct::Opcode(Opcode::ResolveMethCaller(fun_id)))
    }

    pub fn resolveclass<'a>(class_id: ClassId<'a>) -> InstrSeq<'a> {
        instr(Instruct::Opcode(Opcode::ResolveClass(class_id)))
    }

    pub fn lazyclass<'a>(class_id: ClassId<'a>) -> InstrSeq<'a> {
        instr(Instruct::Opcode(Opcode::LazyClass(class_id)))
    }

    pub fn oodeclexists<'a>(class_kind: OODeclExistsOp) -> InstrSeq<'a> {
        instr(Instruct::Opcode(Opcode::OODeclExists(class_kind)))
    }

    pub fn fatal<'a>(op: FatalOp) -> InstrSeq<'a> {
        instr(Instruct::Opcode(Opcode::Fatal(op)))
    }

    pub fn await_<'a>() -> InstrSeq<'a> {
        instr(Instruct::Opcode(Opcode::Await))
    }

    pub fn yield_<'a>() -> InstrSeq<'a> {
        instr(Instruct::Opcode(Opcode::Yield))
    }

    pub fn yieldk<'a>() -> InstrSeq<'a> {
        instr(Instruct::Opcode(Opcode::YieldK))
    }

    pub fn createcont<'a>() -> InstrSeq<'a> {
        instr(Instruct::Opcode(Opcode::CreateCont))
    }

    pub fn awaitall<'a>(range: LocalRange) -> InstrSeq<'a> {
        instr(Instruct::Opcode(Opcode::AwaitAll(range)))
    }

    pub fn label<'a>(label: Label) -> InstrSeq<'a> {
        instr(Instruct::Pseudo(Pseudo::Label(label)))
    }

    pub fn awaitall_list<'a>(unnamed_locals: Vec<Local>) -> InstrSeq<'a> {
        match unnamed_locals.split_first() {
            None => panic!("Expected at least one await"),
            Some((head, tail)) => {
                // Assert that the Locals are sequentially numbered.
                let mut prev_id = head;
                for id in tail {
                    assert_eq!(prev_id.idx + 1, id.idx);
                    prev_id = id;
                }
                awaitall(LocalRange {
                    start: *head,
                    len: unnamed_locals.len().try_into().unwrap(),
                })
            }
        }
    }

    pub fn exit<'a>() -> InstrSeq<'a> {
        instr(Instruct::Opcode(Opcode::Exit))
    }

    pub fn idx<'a>() -> InstrSeq<'a> {
        instr(Instruct::Opcode(Opcode::Idx))
    }

    pub fn array_idx<'a>() -> InstrSeq<'a> {
        instr(Instruct::Opcode(Opcode::ArrayIdx))
    }

    pub fn createcl<'a>(param_num: NumParams, cls_num: ClassNum) -> InstrSeq<'a> {
        instr(Instruct::Opcode(Opcode::CreateCl(param_num, cls_num)))
    }

    pub fn eval<'a>() -> InstrSeq<'a> {
        instr(Instruct::Opcode(Opcode::Eval))
    }

    pub fn incl<'a>() -> InstrSeq<'a> {
        instr(Instruct::Opcode(Opcode::Incl))
    }

    pub fn inclonce<'a>() -> InstrSeq<'a> {
        instr(Instruct::Opcode(Opcode::InclOnce))
    }

    pub fn req<'a>() -> InstrSeq<'a> {
        instr(Instruct::Opcode(Opcode::Req))
    }

    pub fn reqdoc<'a>() -> InstrSeq<'a> {
        instr(Instruct::Opcode(Opcode::ReqDoc))
    }

    pub fn reqonce<'a>() -> InstrSeq<'a> {
        instr(Instruct::Opcode(Opcode::ReqOnce))
    }

    pub fn silence_start<'a>(local: Local) -> InstrSeq<'a> {
        instr(Instruct::Opcode(Opcode::Silence(local, SilenceOp::Start)))
    }

    pub fn silence_end<'a>(local: Local) -> InstrSeq<'a> {
        instr(Instruct::Opcode(Opcode::Silence(local, SilenceOp::End)))
    }

    pub fn contcheck_check<'a>() -> InstrSeq<'a> {
        instr(Instruct::Opcode(Opcode::ContCheck(
            ContCheckOp::CheckStarted,
        )))
    }

    pub fn contcheck_ignore<'a>() -> InstrSeq<'a> {
        instr(Instruct::Opcode(Opcode::ContCheck(
            ContCheckOp::IgnoreStarted,
        )))
    }

    pub fn contenter<'a>() -> InstrSeq<'a> {
        instr(Instruct::Opcode(Opcode::ContEnter))
    }

    pub fn contraise<'a>() -> InstrSeq<'a> {
        instr(Instruct::Opcode(Opcode::ContRaise))
    }

    pub fn contvalid<'a>() -> InstrSeq<'a> {
        instr(Instruct::Opcode(Opcode::ContValid))
    }

    pub fn contcurrent<'a>() -> InstrSeq<'a> {
        instr(Instruct::Opcode(Opcode::ContCurrent))
    }

    pub fn contkey<'a>() -> InstrSeq<'a> {
        instr(Instruct::Opcode(Opcode::ContKey))
    }

    pub fn contgetreturn<'a>() -> InstrSeq<'a> {
        instr(Instruct::Opcode(Opcode::ContGetReturn))
    }

    pub fn nativeimpl<'a>() -> InstrSeq<'a> {
        instr(Instruct::Opcode(Opcode::NativeImpl))
    }

    pub fn srcloc<'a>(
        line_begin: isize,
        line_end: isize,
        col_begin: isize,
        col_end: isize,
    ) -> InstrSeq<'a> {
        instr(Instruct::Pseudo(Pseudo::SrcLoc(SrcLoc {
            line_begin,
            line_end,
            col_begin,
            col_end,
        })))
    }

    pub fn is_type_structc_resolve<'a>() -> InstrSeq<'a> {
        instr(Instruct::Opcode(Opcode::IsTypeStructC(
            TypeStructResolveOp::Resolve,
        )))
    }

    pub fn is_type_structc_dontresolve<'a>() -> InstrSeq<'a> {
        instr(Instruct::Opcode(Opcode::IsTypeStructC(
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
            instr::instr(Instruct::Pseudo(Pseudo::TryCatchBegin)),
            try_instrs,
            instr::jmp(done_label),
            instr::instr(Instruct::Pseudo(Pseudo::TryCatchMiddle)),
            catch_instrs,
            if skip_throw {
                instr::empty()
            } else {
                instr::instr(Instruct::Opcode(Opcode::Throw))
            },
            instr::instr(Instruct::Pseudo(Pseudo::TryCatchEnd)),
            instr::label(done_label),
        ])
    }

    /// Test whether `i` is of case `Pseudo::SrcLoc`.
    fn is_srcloc(instruction: &Instruct<'a>) -> bool {
        matches!(instruction, Instruct::Pseudo(Pseudo::SrcLoc(_)))
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
        let mk_i = || Instruct::Pseudo(Pseudo::Comment(Str::from("")));
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
