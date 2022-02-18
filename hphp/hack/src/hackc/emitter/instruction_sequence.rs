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
    FatalOp, InitPropOp, IsTypeOp, MOpMode, QueryMOp, SpecialClsRef, TypeStructResolveOp,
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
#[derive(Debug)]
#[repr(C)]
pub enum InstrSeq<'a> {
    List(BumpSliceMut<'a, Instruct<'a>>),
    Concat(BumpSliceMut<'a, InstrSeq<'a>>),
}
// The slices are mutable because of `rewrite_user_labels`. This means
// we can't derive `Clone` (because you can't have multiple mutable
// references referring to the same resource). It is possible to implement
// deep copy functionality though: see `InstrSeq::clone()` below.

impl<'a> std::convert::From<(&'a bumpalo::Bump, (InstrSeq<'a>, InstrSeq<'a>))> for InstrSeq<'a> {
    fn from((alloc, (i1, i2)): (&'a bumpalo::Bump, (InstrSeq<'a>, InstrSeq<'a>))) -> InstrSeq<'a> {
        InstrSeq::gather(alloc, vec![i1, i2])
    }
}

#[derive(Debug)]
pub struct CompactIter<'i, 'a, I>
where
    I: Iterator<Item = &'i Instruct<'a>>,
{
    iter: I,
    next: Option<&'i Instruct<'a>>,
}

impl<'i, 'a, I> CompactIter<'i, 'a, I>
where
    I: Iterator<Item = &'i Instruct<'a>>,
{
    pub fn new(i: I) -> Self {
        Self {
            iter: i,
            next: None,
        }
    }
}

impl<'i, 'a, I> Iterator for CompactIter<'i, 'a, I>
where
    I: Iterator<Item = &'i Instruct<'a>>,
{
    type Item = &'i Instruct<'a>;
    fn next(&mut self) -> Option<Self::Item> {
        if self.next.is_some() {
            std::mem::replace(&mut self.next, None)
        } else {
            let mut cur = self.iter.next();
            match cur {
                Some(i) if InstrSeq::is_srcloc(i) => {
                    self.next = self.iter.next();
                    while self.next.map_or(false, InstrSeq::is_srcloc) {
                        cur = self.next;
                        self.next = self.iter.next();
                    }
                    cur
                }
                _ => cur,
            }
        }
    }
}

#[derive(Debug)]
pub struct InstrIter<'i, 'a> {
    instr_seq: &'i InstrSeq<'a>,
    index: usize,
    concat_stack: std::vec::Vec<
        itertools::Either<
            (&'i BumpSliceMut<'a, Instruct<'a>>, usize),
            (&'i BumpSliceMut<'a, InstrSeq<'a>>, usize),
        >,
    >,
}

impl<'i, 'a> InstrIter<'i, 'a> {
    pub fn new(instr_seq: &'i InstrSeq<'a>) -> Self {
        Self {
            instr_seq,
            index: 0,
            concat_stack: std::vec::Vec::new(),
        }
    }
}

impl<'i, 'a> Iterator for InstrIter<'i, 'a> {
    type Item = &'i Instruct<'a>;
    fn next(&mut self) -> Option<Self::Item> {
        //self: & mut InstrIter<'i, 'a>
        //self.instr_seq: &'i InstrSeq<'a>
        match self.instr_seq {
            InstrSeq::List(s) if s.is_empty() => None,
            InstrSeq::List(s) if s.len() == 1 && self.index > 0 => None,
            InstrSeq::List(s) if s.len() == 1 => {
                self.index += 1;
                s.get(0)
            }
            InstrSeq::List(s) if s.len() > 1 && self.index >= s.len() => None,
            InstrSeq::List(s) => {
                let r = s.get(self.index);
                self.index += 1;
                r
            }
            InstrSeq::Concat(s) => {
                if self.concat_stack.is_empty() {
                    if self.index == 0 {
                        self.index += 1;
                        self.concat_stack.push(itertools::Either::Right((s, 0)));
                    } else {
                        return None;
                    }
                }

                while !self.concat_stack.is_empty() {
                    let top: &mut itertools::Either<_, _> = self.concat_stack.last_mut().unwrap();
                    match top {
                        itertools::Either::Left((list, size)) if *size >= list.len() => {
                            self.concat_stack.pop();
                        }
                        itertools::Either::Left((list, size)) => {
                            let r: Option<&'i Instruct<'a>> = list.get(*size);
                            *size += 1;
                            return r;
                        }
                        itertools::Either::Right((concat, size)) if *size >= concat.len() => {
                            self.concat_stack.pop();
                        }
                        itertools::Either::Right((concat, size)) => {
                            let i: &'i InstrSeq<'a> = &(concat[*size]);
                            *size += 1;
                            match i {
                                InstrSeq::List(s) if s.is_empty() => {}
                                InstrSeq::List(s) if s.len() == 1 => {
                                    return s.get(0);
                                }
                                InstrSeq::List(s) => {
                                    self.concat_stack.push(itertools::Either::Left((s, 0)));
                                }
                                InstrSeq::Concat(s) => {
                                    self.concat_stack.push(itertools::Either::Right((s, 0)));
                                }
                            }
                        }
                    }
                }
                None
            }
        }
    }
}

pub mod instr {
    use crate::*;

    pub fn empty<'a>(alloc: &'a bumpalo::Bump) -> InstrSeq<'a> {
        InstrSeq::new_empty(alloc)
    }

    pub fn instr<'a>(alloc: &'a bumpalo::Bump, i: Instruct<'a>) -> InstrSeq<'a> {
        InstrSeq::new_singleton(alloc, i)
    }

    pub fn instrs<'a>(alloc: &'a bumpalo::Bump, is: &'a mut [Instruct<'a>]) -> InstrSeq<'a> {
        InstrSeq::new_list(alloc, is)
    }

    pub fn lit_const<'a>(alloc: &'a bumpalo::Bump, l: InstructLitConst<'a>) -> InstrSeq<'a> {
        instr(alloc, Instruct::LitConst(l))
    }

    pub fn iterinit<'a>(
        alloc: &'a bumpalo::Bump,
        args: IterArgs<'a>,
        label: Label,
    ) -> InstrSeq<'a> {
        instr(
            alloc,
            Instruct::Iterator(InstructIterator::IterInit(args, label)),
        )
    }

    pub fn iternext<'a>(
        alloc: &'a bumpalo::Bump,
        args: IterArgs<'a>,
        label: Label,
    ) -> InstrSeq<'a> {
        instr(
            alloc,
            Instruct::Iterator(InstructIterator::IterNext(args, label)),
        )
    }

    pub fn iternextk<'a>(
        alloc: &'a bumpalo::Bump,
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
        instr(
            alloc,
            Instruct::Iterator(InstructIterator::IterNext(args, label)),
        )
    }

    pub fn iterfree<'a>(alloc: &'a bumpalo::Bump, id: IterId) -> InstrSeq<'a> {
        instr(alloc, Instruct::Iterator(InstructIterator::IterFree(id)))
    }

    pub fn whresult<'a>(alloc: &'a bumpalo::Bump) -> InstrSeq<'a> {
        instr(alloc, Instruct::Async(AsyncFunctions::WHResult))
    }

    pub fn jmp<'a>(alloc: &'a bumpalo::Bump, label: Label) -> InstrSeq<'a> {
        instr(alloc, Instruct::ContFlow(InstructControlFlow::Jmp(label)))
    }

    pub fn jmpz<'a>(alloc: &'a bumpalo::Bump, label: Label) -> InstrSeq<'a> {
        instr(alloc, Instruct::ContFlow(InstructControlFlow::JmpZ(label)))
    }

    pub fn jmpnz<'a>(alloc: &'a bumpalo::Bump, label: Label) -> InstrSeq<'a> {
        instr(alloc, Instruct::ContFlow(InstructControlFlow::JmpNZ(label)))
    }

    pub fn jmpns<'a>(alloc: &'a bumpalo::Bump, label: Label) -> InstrSeq<'a> {
        instr(alloc, Instruct::ContFlow(InstructControlFlow::JmpNS(label)))
    }

    pub fn continue_<'a>(alloc: &'a bumpalo::Bump, level: isize) -> InstrSeq<'a> {
        instr(
            alloc,
            Instruct::SpecialFlow(InstructSpecialFlow::Continue(level)),
        )
    }

    pub fn break_<'a>(alloc: &'a bumpalo::Bump, level: isize) -> InstrSeq<'a> {
        instr(
            alloc,
            Instruct::SpecialFlow(InstructSpecialFlow::Break(level)),
        )
    }

    pub fn iter_break<'a>(
        alloc: &'a bumpalo::Bump,
        label: Label,
        itrs: std::vec::Vec<IterId>,
    ) -> InstrSeq<'a> {
        let mut vec = bumpalo::collections::Vec::from_iter_in(
            itrs.into_iter()
                .map(|id| Instruct::Iterator(InstructIterator::IterFree(id))),
            alloc,
        );
        vec.push(Instruct::ContFlow(InstructControlFlow::Jmp(label)));
        instrs(alloc, vec.into_bump_slice_mut())
    }

    pub fn false_<'a>(alloc: &'a bumpalo::Bump) -> InstrSeq<'a> {
        instr(alloc, Instruct::LitConst(InstructLitConst::False))
    }

    pub fn true_<'a>(alloc: &'a bumpalo::Bump) -> InstrSeq<'a> {
        instr(alloc, Instruct::LitConst(InstructLitConst::True))
    }

    pub fn clscnsd<'a>(
        alloc: &'a bumpalo::Bump,
        const_id: ConstId<'a>,
        cid: ClassId<'a>,
    ) -> InstrSeq<'a> {
        instr(
            alloc,
            Instruct::LitConst(InstructLitConst::ClsCnsD(const_id, cid)),
        )
    }

    pub fn clscns<'a>(alloc: &'a bumpalo::Bump, const_id: ConstId<'a>) -> InstrSeq<'a> {
        instr(
            alloc,
            Instruct::LitConst(InstructLitConst::ClsCns(const_id)),
        )
    }

    pub fn clscnsl<'a>(alloc: &'a bumpalo::Bump, local: Local<'a>) -> InstrSeq<'a> {
        instr(alloc, Instruct::LitConst(InstructLitConst::ClsCnsL(local)))
    }

    pub fn eq<'a>(alloc: &'a bumpalo::Bump) -> InstrSeq<'a> {
        instr(alloc, Instruct::Op(InstructOperator::Eq))
    }

    pub fn neq<'a>(alloc: &'a bumpalo::Bump) -> InstrSeq<'a> {
        instr(alloc, Instruct::Op(InstructOperator::Neq))
    }

    pub fn gt<'a>(alloc: &'a bumpalo::Bump) -> InstrSeq<'a> {
        instr(alloc, Instruct::Op(InstructOperator::Gt))
    }

    pub fn gte<'a>(alloc: &'a bumpalo::Bump) -> InstrSeq<'a> {
        instr(alloc, Instruct::Op(InstructOperator::Gte))
    }

    pub fn lt<'a>(alloc: &'a bumpalo::Bump) -> InstrSeq<'a> {
        instr(alloc, Instruct::Op(InstructOperator::Lt))
    }

    pub fn lte<'a>(alloc: &'a bumpalo::Bump) -> InstrSeq<'a> {
        instr(alloc, Instruct::Op(InstructOperator::Lte))
    }

    pub fn concat<'a>(alloc: &'a bumpalo::Bump) -> InstrSeq<'a> {
        instr(alloc, Instruct::Op(InstructOperator::Concat))
    }

    pub fn concatn<'a>(alloc: &'a bumpalo::Bump, n: isize) -> InstrSeq<'a> {
        instr(alloc, Instruct::Op(InstructOperator::ConcatN(n)))
    }

    pub fn print<'a>(alloc: &'a bumpalo::Bump) -> InstrSeq<'a> {
        instr(alloc, Instruct::Op(InstructOperator::Print))
    }

    pub fn cast_dict<'a>(alloc: &'a bumpalo::Bump) -> InstrSeq<'a> {
        instr(alloc, Instruct::Op(InstructOperator::CastDict))
    }

    pub fn cast_string<'a>(alloc: &'a bumpalo::Bump) -> InstrSeq<'a> {
        instr(alloc, Instruct::Op(InstructOperator::CastString))
    }

    pub fn cast_int<'a>(alloc: &'a bumpalo::Bump) -> InstrSeq<'a> {
        instr(alloc, Instruct::Op(InstructOperator::CastInt))
    }

    pub fn cast_bool<'a>(alloc: &'a bumpalo::Bump) -> InstrSeq<'a> {
        instr(alloc, Instruct::Op(InstructOperator::CastBool))
    }

    pub fn cast_double<'a>(alloc: &'a bumpalo::Bump) -> InstrSeq<'a> {
        instr(alloc, Instruct::Op(InstructOperator::CastDouble))
    }

    pub fn retc<'a>(alloc: &'a bumpalo::Bump) -> InstrSeq<'a> {
        instr(alloc, Instruct::ContFlow(InstructControlFlow::RetC))
    }

    pub fn retc_suspended<'a>(alloc: &'a bumpalo::Bump) -> InstrSeq<'a> {
        instr(
            alloc,
            Instruct::ContFlow(InstructControlFlow::RetCSuspended),
        )
    }

    pub fn retm<'a>(alloc: &'a bumpalo::Bump, p: NumParams) -> InstrSeq<'a> {
        instr(alloc, Instruct::ContFlow(InstructControlFlow::RetM(p)))
    }

    pub fn null<'a>(alloc: &'a bumpalo::Bump) -> InstrSeq<'a> {
        instr(alloc, Instruct::LitConst(InstructLitConst::Null))
    }

    pub fn nulluninit<'a>(alloc: &'a bumpalo::Bump) -> InstrSeq<'a> {
        instr(alloc, Instruct::LitConst(InstructLitConst::NullUninit))
    }

    pub fn chain_faults<'a>(alloc: &'a bumpalo::Bump) -> InstrSeq<'a> {
        instr(alloc, Instruct::Misc(InstructMisc::ChainFaults))
    }

    pub fn dup<'a>(alloc: &'a bumpalo::Bump) -> InstrSeq<'a> {
        instr(alloc, Instruct::Basic(InstructBasic::Dup))
    }

    pub fn nop<'a>(alloc: &'a bumpalo::Bump) -> InstrSeq<'a> {
        instr(alloc, Instruct::Basic(InstructBasic::Nop))
    }

    pub fn instanceofd<'a>(alloc: &'a bumpalo::Bump, s: ClassId<'a>) -> InstrSeq<'a> {
        instr(alloc, Instruct::Op(InstructOperator::InstanceOfD(s)))
    }

    pub fn instanceof<'a>(alloc: &'a bumpalo::Bump) -> InstrSeq<'a> {
        instr(alloc, Instruct::Op(InstructOperator::InstanceOf))
    }

    pub fn islateboundcls<'a>(alloc: &'a bumpalo::Bump) -> InstrSeq<'a> {
        instr(alloc, Instruct::Op(InstructOperator::IsLateBoundCls))
    }

    pub fn istypestructc<'a>(alloc: &'a bumpalo::Bump, mode: TypeStructResolveOp) -> InstrSeq<'a> {
        instr(alloc, Instruct::Op(InstructOperator::IsTypeStructC(mode)))
    }

    pub fn throwastypestructexception<'a>(alloc: &'a bumpalo::Bump) -> InstrSeq<'a> {
        instr(
            alloc,
            Instruct::Op(InstructOperator::ThrowAsTypeStructException),
        )
    }

    pub fn throw_non_exhaustive_switch<'a>(alloc: &'a bumpalo::Bump) -> InstrSeq<'a> {
        instr(
            alloc,
            Instruct::Misc(InstructMisc::ThrowNonExhaustiveSwitch),
        )
    }

    pub fn raise_class_string_conversion_warning<'a>(alloc: &'a bumpalo::Bump) -> InstrSeq<'a> {
        instr(
            alloc,
            Instruct::Misc(InstructMisc::RaiseClassStringConversionWarning),
        )
    }

    pub fn combine_and_resolve_type_struct<'a>(alloc: &'a bumpalo::Bump, i: isize) -> InstrSeq<'a> {
        instr(
            alloc,
            Instruct::Op(InstructOperator::CombineAndResolveTypeStruct(i)),
        )
    }

    pub fn record_reified_generic<'a>(alloc: &'a bumpalo::Bump) -> InstrSeq<'a> {
        instr(alloc, Instruct::Misc(InstructMisc::RecordReifiedGeneric))
    }

    pub fn check_reified_generic_mismatch<'a>(alloc: &'a bumpalo::Bump) -> InstrSeq<'a> {
        instr(
            alloc,
            Instruct::Misc(InstructMisc::CheckReifiedGenericMismatch),
        )
    }

    pub fn int<'a>(alloc: &'a bumpalo::Bump, i: isize) -> InstrSeq<'a> {
        instr(
            alloc,
            Instruct::LitConst(InstructLitConst::Int(i.try_into().unwrap())),
        )
    }

    pub fn int64<'a>(alloc: &'a bumpalo::Bump, i: i64) -> InstrSeq<'a> {
        instr(alloc, Instruct::LitConst(InstructLitConst::Int(i)))
    }

    pub fn int_of_string<'a>(alloc: &'a bumpalo::Bump, litstr: &str) -> InstrSeq<'a> {
        instr(
            alloc,
            Instruct::LitConst(InstructLitConst::Int(litstr.parse::<i64>().unwrap())),
        )
    }

    pub fn double<'a>(alloc: &'a bumpalo::Bump, litstr: &str) -> InstrSeq<'a> {
        instr(
            alloc,
            Instruct::LitConst(InstructLitConst::Double(Str::from(
                bumpalo::collections::String::from_str_in(litstr, alloc).into_bump_str(),
            ))),
        )
    }

    pub fn string<'a>(alloc: &'a bumpalo::Bump, litstr: impl Into<String>) -> InstrSeq<'a> {
        instr(
            alloc,
            Instruct::LitConst(InstructLitConst::String(Str::from(
                bumpalo::collections::String::from_str_in(litstr.into().as_str(), alloc)
                    .into_bump_str(),
            ))),
        )
    }

    pub fn this<'a>(alloc: &'a bumpalo::Bump) -> InstrSeq<'a> {
        instr(alloc, Instruct::Misc(InstructMisc::This))
    }

    pub fn istypec<'a>(alloc: &'a bumpalo::Bump, op: IsTypeOp) -> InstrSeq<'a> {
        instr(alloc, Instruct::Isset(InstructIsset::IsTypeC(op)))
    }

    pub fn istypel<'a>(alloc: &'a bumpalo::Bump, id: Local<'a>, op: IsTypeOp) -> InstrSeq<'a> {
        instr(alloc, Instruct::Isset(InstructIsset::IsTypeL(id, op)))
    }

    pub fn add<'a>(alloc: &'a bumpalo::Bump) -> InstrSeq<'a> {
        instr(alloc, Instruct::Op(InstructOperator::Add))
    }

    pub fn addo<'a>(alloc: &'a bumpalo::Bump) -> InstrSeq<'a> {
        instr(alloc, Instruct::Op(InstructOperator::AddO))
    }

    pub fn sub<'a>(alloc: &'a bumpalo::Bump) -> InstrSeq<'a> {
        instr(alloc, Instruct::Op(InstructOperator::Sub))
    }

    pub fn subo<'a>(alloc: &'a bumpalo::Bump) -> InstrSeq<'a> {
        instr(alloc, Instruct::Op(InstructOperator::SubO))
    }

    pub fn mul<'a>(alloc: &'a bumpalo::Bump) -> InstrSeq<'a> {
        instr(alloc, Instruct::Op(InstructOperator::Mul))
    }

    pub fn mulo<'a>(alloc: &'a bumpalo::Bump) -> InstrSeq<'a> {
        instr(alloc, Instruct::Op(InstructOperator::MulO))
    }

    pub fn shl<'a>(alloc: &'a bumpalo::Bump) -> InstrSeq<'a> {
        instr(alloc, Instruct::Op(InstructOperator::Shl))
    }

    pub fn shr<'a>(alloc: &'a bumpalo::Bump) -> InstrSeq<'a> {
        instr(alloc, Instruct::Op(InstructOperator::Shr))
    }

    pub fn cmp<'a>(alloc: &'a bumpalo::Bump) -> InstrSeq<'a> {
        instr(alloc, Instruct::Op(InstructOperator::Cmp))
    }

    pub fn mod_<'a>(alloc: &'a bumpalo::Bump) -> InstrSeq<'a> {
        instr(alloc, Instruct::Op(InstructOperator::Mod))
    }

    pub fn div<'a>(alloc: &'a bumpalo::Bump) -> InstrSeq<'a> {
        instr(alloc, Instruct::Op(InstructOperator::Div))
    }

    pub fn same<'a>(alloc: &'a bumpalo::Bump) -> InstrSeq<'a> {
        instr(alloc, Instruct::Op(InstructOperator::Same))
    }

    pub fn pow<'a>(alloc: &'a bumpalo::Bump) -> InstrSeq<'a> {
        instr(alloc, Instruct::Op(InstructOperator::Pow))
    }

    pub fn nsame<'a>(alloc: &'a bumpalo::Bump) -> InstrSeq<'a> {
        instr(alloc, Instruct::Op(InstructOperator::NSame))
    }

    pub fn not<'a>(alloc: &'a bumpalo::Bump) -> InstrSeq<'a> {
        instr(alloc, Instruct::Op(InstructOperator::Not))
    }

    pub fn bitnot<'a>(alloc: &'a bumpalo::Bump) -> InstrSeq<'a> {
        instr(alloc, Instruct::Op(InstructOperator::BitNot))
    }

    pub fn bitand<'a>(alloc: &'a bumpalo::Bump) -> InstrSeq<'a> {
        instr(alloc, Instruct::Op(InstructOperator::BitAnd))
    }

    pub fn bitor<'a>(alloc: &'a bumpalo::Bump) -> InstrSeq<'a> {
        instr(alloc, Instruct::Op(InstructOperator::BitOr))
    }

    pub fn bitxor<'a>(alloc: &'a bumpalo::Bump) -> InstrSeq<'a> {
        instr(alloc, Instruct::Op(InstructOperator::BitXor))
    }

    pub fn sets<'a>(alloc: &'a bumpalo::Bump, readonly_op: ReadonlyOp) -> InstrSeq<'a> {
        instr(alloc, Instruct::Mutator(InstructMutator::SetS(readonly_op)))
    }

    pub fn setl<'a>(alloc: &'a bumpalo::Bump, local: Local<'a>) -> InstrSeq<'a> {
        instr(alloc, Instruct::Mutator(InstructMutator::SetL(local)))
    }

    pub fn setg<'a>(alloc: &'a bumpalo::Bump) -> InstrSeq<'a> {
        instr(alloc, Instruct::Mutator(InstructMutator::SetG))
    }

    pub fn unsetl<'a>(alloc: &'a bumpalo::Bump, local: Local<'a>) -> InstrSeq<'a> {
        instr(alloc, Instruct::Mutator(InstructMutator::UnsetL(local)))
    }

    pub fn unsetg<'a>(alloc: &'a bumpalo::Bump) -> InstrSeq<'a> {
        instr(alloc, Instruct::Mutator(InstructMutator::UnsetG))
    }

    pub fn incdecl<'a>(alloc: &'a bumpalo::Bump, local: Local<'a>, op: IncDecOp) -> InstrSeq<'a> {
        instr(
            alloc,
            Instruct::Mutator(InstructMutator::IncDecL(local, op)),
        )
    }

    pub fn incdecg<'a>(alloc: &'a bumpalo::Bump, op: IncDecOp) -> InstrSeq<'a> {
        instr(alloc, Instruct::Mutator(InstructMutator::IncDecG(op)))
    }

    pub fn incdecs<'a>(alloc: &'a bumpalo::Bump, op: IncDecOp) -> InstrSeq<'a> {
        instr(alloc, Instruct::Mutator(InstructMutator::IncDecS(op)))
    }

    pub fn setopg<'a>(alloc: &'a bumpalo::Bump, op: EqOp) -> InstrSeq<'a> {
        instr(alloc, Instruct::Mutator(InstructMutator::SetOpG(op)))
    }

    pub fn setopl<'a>(alloc: &'a bumpalo::Bump, local: Local<'a>, op: EqOp) -> InstrSeq<'a> {
        instr(alloc, Instruct::Mutator(InstructMutator::SetOpL(local, op)))
    }

    pub fn setops<'a>(alloc: &'a bumpalo::Bump, op: EqOp) -> InstrSeq<'a> {
        instr(alloc, Instruct::Mutator(InstructMutator::SetOpS(op)))
    }

    pub fn issetl<'a>(alloc: &'a bumpalo::Bump, local: Local<'a>) -> InstrSeq<'a> {
        instr(alloc, Instruct::Isset(InstructIsset::IssetL(local)))
    }

    pub fn issetg<'a>(alloc: &'a bumpalo::Bump) -> InstrSeq<'a> {
        instr(alloc, Instruct::Isset(InstructIsset::IssetG))
    }

    pub fn issets<'a>(alloc: &'a bumpalo::Bump) -> InstrSeq<'a> {
        instr(alloc, Instruct::Isset(InstructIsset::IssetS))
    }

    pub fn isunsetl<'a>(alloc: &'a bumpalo::Bump, local: Local<'a>) -> InstrSeq<'a> {
        instr(alloc, Instruct::Isset(InstructIsset::IsUnsetL(local)))
    }

    pub fn cgets<'a>(alloc: &'a bumpalo::Bump, readonly_op: ReadonlyOp) -> InstrSeq<'a> {
        instr(alloc, Instruct::Get(InstructGet::CGetS(readonly_op)))
    }

    pub fn cgetg<'a>(alloc: &'a bumpalo::Bump) -> InstrSeq<'a> {
        instr(alloc, Instruct::Get(InstructGet::CGetG))
    }

    pub fn cgetl<'a>(alloc: &'a bumpalo::Bump, local: Local<'a>) -> InstrSeq<'a> {
        instr(alloc, Instruct::Get(InstructGet::CGetL(local)))
    }

    pub fn cugetl<'a>(alloc: &'a bumpalo::Bump, local: Local<'a>) -> InstrSeq<'a> {
        instr(alloc, Instruct::Get(InstructGet::CUGetL(local)))
    }

    pub fn cgetl2<'a>(alloc: &'a bumpalo::Bump, local: Local<'a>) -> InstrSeq<'a> {
        instr(alloc, Instruct::Get(InstructGet::CGetL2(local)))
    }

    pub fn cgetquietl<'a>(alloc: &'a bumpalo::Bump, local: Local<'a>) -> InstrSeq<'a> {
        instr(alloc, Instruct::Get(InstructGet::CGetQuietL(local)))
    }

    pub fn classgetc<'a>(alloc: &'a bumpalo::Bump) -> InstrSeq<'a> {
        instr(alloc, Instruct::Get(InstructGet::ClassGetC))
    }

    pub fn classgetts<'a>(alloc: &'a bumpalo::Bump) -> InstrSeq<'a> {
        instr(alloc, Instruct::Get(InstructGet::ClassGetTS))
    }

    pub fn classname<'a>(alloc: &'a bumpalo::Bump) -> InstrSeq<'a> {
        instr(alloc, Instruct::Misc(InstructMisc::ClassName))
    }

    pub fn lazyclassfromclass<'a>(alloc: &'a bumpalo::Bump) -> InstrSeq<'a> {
        instr(alloc, Instruct::Misc(InstructMisc::LazyClassFromClass))
    }

    pub fn self_<'a>(alloc: &'a bumpalo::Bump) -> InstrSeq<'a> {
        instr(alloc, Instruct::Misc(InstructMisc::Self_))
    }

    pub fn lateboundcls<'a>(alloc: &'a bumpalo::Bump) -> InstrSeq<'a> {
        instr(alloc, Instruct::Misc(InstructMisc::LateBoundCls))
    }

    pub fn parent<'a>(alloc: &'a bumpalo::Bump) -> InstrSeq<'a> {
        instr(alloc, Instruct::Misc(InstructMisc::Parent))
    }

    pub fn popu<'a>(alloc: &'a bumpalo::Bump) -> InstrSeq<'a> {
        instr(alloc, Instruct::Basic(InstructBasic::PopU))
    }

    pub fn popc<'a>(alloc: &'a bumpalo::Bump) -> InstrSeq<'a> {
        instr(alloc, Instruct::Basic(InstructBasic::PopC))
    }

    pub fn popl<'a>(alloc: &'a bumpalo::Bump, l: Local<'a>) -> InstrSeq<'a> {
        instr(alloc, Instruct::Mutator(InstructMutator::PopL(l)))
    }

    pub fn initprop<'a>(alloc: &'a bumpalo::Bump, pid: PropId<'a>, op: InitPropOp) -> InstrSeq<'a> {
        instr(alloc, Instruct::Mutator(InstructMutator::InitProp(pid, op)))
    }

    pub fn checkprop<'a>(alloc: &'a bumpalo::Bump, pid: PropId<'a>) -> InstrSeq<'a> {
        instr(alloc, Instruct::Mutator(InstructMutator::CheckProp(pid)))
    }

    pub fn pushl<'a>(alloc: &'a bumpalo::Bump, local: Local<'a>) -> InstrSeq<'a> {
        instr(alloc, Instruct::Get(InstructGet::PushL(local)))
    }

    pub fn throw<'a>(alloc: &'a bumpalo::Bump) -> InstrSeq<'a> {
        instr(alloc, Instruct::ContFlow(InstructControlFlow::Throw))
    }

    pub fn new_vec_array<'a>(alloc: &'a bumpalo::Bump, i: isize) -> InstrSeq<'a> {
        instr(alloc, Instruct::LitConst(InstructLitConst::NewVec(i)))
    }

    pub fn new_pair<'a>(alloc: &'a bumpalo::Bump) -> InstrSeq<'a> {
        instr(alloc, Instruct::LitConst(InstructLitConst::NewPair))
    }

    pub fn add_elemc<'a>(alloc: &'a bumpalo::Bump) -> InstrSeq<'a> {
        instr(alloc, Instruct::LitConst(InstructLitConst::AddElemC))
    }

    pub fn add_new_elemc<'a>(alloc: &'a bumpalo::Bump) -> InstrSeq<'a> {
        instr(alloc, Instruct::LitConst(InstructLitConst::AddNewElemC))
    }

    pub fn switch<'a>(
        alloc: &'a bumpalo::Bump,
        targets: bumpalo::collections::Vec<'a, Label>,
    ) -> InstrSeq<'a> {
        instr(
            alloc,
            Instruct::ContFlow(InstructControlFlow::Switch {
                kind: SwitchKind::Unbounded,
                base: 0,
                targets: BumpSliceMut::new(alloc, targets.into_bump_slice_mut()),
            }),
        )
    }

    pub fn newobj<'a>(alloc: &'a bumpalo::Bump) -> InstrSeq<'a> {
        instr(alloc, Instruct::New(InstructNew::NewObj))
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
        instr(
            alloc,
            Instruct::ContFlow(InstructControlFlow::SSwitch { cases, targets }),
        )
    }

    pub fn newobjr<'a>(alloc: &'a bumpalo::Bump) -> InstrSeq<'a> {
        instr(alloc, Instruct::New(InstructNew::NewObjR))
    }

    pub fn newobjd<'a>(alloc: &'a bumpalo::Bump, id: ClassId<'a>) -> InstrSeq<'a> {
        instr(alloc, Instruct::New(InstructNew::NewObjD(id)))
    }

    pub fn newobjrd<'a>(alloc: &'a bumpalo::Bump, id: ClassId<'a>) -> InstrSeq<'a> {
        instr(alloc, Instruct::New(InstructNew::NewObjRD(id)))
    }

    pub fn newobjs<'a>(alloc: &'a bumpalo::Bump, scref: SpecialClsRef) -> InstrSeq<'a> {
        instr(alloc, Instruct::New(InstructNew::NewObjS(scref)))
    }

    pub fn lockobj<'a>(alloc: &'a bumpalo::Bump) -> InstrSeq<'a> {
        instr(alloc, Instruct::Misc(InstructMisc::LockObj))
    }

    pub fn clone<'a>(alloc: &'a bumpalo::Bump) -> InstrSeq<'a> {
        instr(alloc, Instruct::Op(InstructOperator::Clone))
    }

    pub fn new_record<'a>(
        alloc: &'a bumpalo::Bump,
        id: ClassId<'a>,
        keys: &'a [&'a str],
    ) -> InstrSeq<'a> {
        let keys = Slice::new(alloc.alloc_slice_fill_iter(keys.iter().map(|s| Str::from(*s))));
        instr(
            alloc,
            Instruct::LitConst(InstructLitConst::NewRecord(id, keys)),
        )
    }

    pub fn newstructdict<'a>(alloc: &'a bumpalo::Bump, keys: &'a [&'a str]) -> InstrSeq<'a> {
        let keys = Slice::new(alloc.alloc_slice_fill_iter(keys.iter().map(|s| Str::from(*s))));
        instr(
            alloc,
            Instruct::LitConst(InstructLitConst::NewStructDict(keys)),
        )
    }

    pub fn newcol<'a>(alloc: &'a bumpalo::Bump, collection_type: CollectionType) -> InstrSeq<'a> {
        instr(
            alloc,
            Instruct::LitConst(InstructLitConst::NewCol(collection_type)),
        )
    }

    pub fn colfromarray<'a>(
        alloc: &'a bumpalo::Bump,
        collection_type: CollectionType,
    ) -> InstrSeq<'a> {
        instr(
            alloc,
            Instruct::LitConst(InstructLitConst::ColFromArray(collection_type)),
        )
    }

    pub fn entrynop<'a>(alloc: &'a bumpalo::Bump) -> InstrSeq<'a> {
        instr(alloc, Instruct::Basic(InstructBasic::EntryNop))
    }

    pub fn typedvalue<'a>(alloc: &'a bumpalo::Bump, xs: TypedValue<'a>) -> InstrSeq<'a> {
        instr(alloc, Instruct::LitConst(InstructLitConst::TypedValue(xs)))
    }

    pub fn basel<'a>(
        alloc: &'a bumpalo::Bump,
        local: Local<'a>,
        mode: MOpMode,
        readonly_op: ReadonlyOp,
    ) -> InstrSeq<'a> {
        instr(
            alloc,
            Instruct::Base(InstructBase::BaseL(local, mode, readonly_op)),
        )
    }

    pub fn basec<'a>(
        alloc: &'a bumpalo::Bump,
        stack_index: StackIndex,
        mode: MOpMode,
    ) -> InstrSeq<'a> {
        instr(
            alloc,
            Instruct::Base(InstructBase::BaseC(stack_index, mode)),
        )
    }

    pub fn basegc<'a>(
        alloc: &'a bumpalo::Bump,
        stack_index: StackIndex,
        mode: MOpMode,
    ) -> InstrSeq<'a> {
        instr(
            alloc,
            Instruct::Base(InstructBase::BaseGC(stack_index, mode)),
        )
    }

    pub fn basegl<'a>(alloc: &'a bumpalo::Bump, local: Local<'a>, mode: MOpMode) -> InstrSeq<'a> {
        instr(alloc, Instruct::Base(InstructBase::BaseGL(local, mode)))
    }

    pub fn basesc<'a>(
        alloc: &'a bumpalo::Bump,
        y: StackIndex,
        z: StackIndex,
        mode: MOpMode,
        readonly_op: ReadonlyOp,
    ) -> InstrSeq<'a> {
        instr(
            alloc,
            Instruct::Base(InstructBase::BaseSC(y, z, mode, readonly_op)),
        )
    }

    pub fn baseh<'a>(alloc: &'a bumpalo::Bump) -> InstrSeq<'a> {
        instr(alloc, Instruct::Base(InstructBase::BaseH))
    }

    pub fn cgetcunop<'a>(alloc: &'a bumpalo::Bump) -> InstrSeq<'a> {
        instr(alloc, Instruct::Misc(InstructMisc::CGetCUNop))
    }

    pub fn ugetcunop<'a>(alloc: &'a bumpalo::Bump) -> InstrSeq<'a> {
        instr(alloc, Instruct::Misc(InstructMisc::UGetCUNop))
    }

    pub fn memoget<'a>(
        alloc: &'a bumpalo::Bump,
        label: Label,
        range: Option<(Local<'a>, isize)>,
    ) -> InstrSeq<'a> {
        instr(
            alloc,
            Instruct::Misc(InstructMisc::MemoGet(
                label,
                match range {
                    Some((fst, snd)) => Just(Pair(fst, snd)),
                    None => Nothing,
                },
            )),
        )
    }

    // Factored out to reduce verbosity.
    fn range_opt_to_maybe<'a>(range: Option<(Local<'a>, isize)>) -> Maybe<Pair<Local<'a>, isize>> {
        match range {
            Some((fst, snd)) => Just(Pair(fst, snd)),
            None => Nothing,
        }
    }

    pub fn memoget_eager<'a>(
        alloc: &'a bumpalo::Bump,
        label1: Label,
        label2: Label,
        range: Option<(Local<'a>, isize)>,
    ) -> InstrSeq<'a> {
        instr(
            alloc,
            Instruct::Misc(InstructMisc::MemoGetEager(
                [label1, label2],
                range_opt_to_maybe(range),
            )),
        )
    }

    pub fn memoset<'a>(
        alloc: &'a bumpalo::Bump,
        range: Option<(Local<'a>, isize)>,
    ) -> InstrSeq<'a> {
        instr(
            alloc,
            Instruct::Misc(InstructMisc::MemoSet(range_opt_to_maybe(range))),
        )
    }

    pub fn memoset_eager<'a>(
        alloc: &'a bumpalo::Bump,
        range: Option<(Local<'a>, isize)>,
    ) -> InstrSeq<'a> {
        instr(
            alloc,
            Instruct::Misc(InstructMisc::MemoSetEager(range_opt_to_maybe(range))),
        )
    }

    pub fn getmemokeyl<'a>(alloc: &'a bumpalo::Bump, local: Local<'a>) -> InstrSeq<'a> {
        instr(alloc, Instruct::Misc(InstructMisc::GetMemoKeyL(local)))
    }

    pub fn barethis<'a>(alloc: &'a bumpalo::Bump, notice: BareThisOp) -> InstrSeq<'a> {
        instr(alloc, Instruct::Misc(InstructMisc::BareThis(notice)))
    }

    pub fn checkthis<'a>(alloc: &'a bumpalo::Bump) -> InstrSeq<'a> {
        instr(alloc, Instruct::Misc(InstructMisc::CheckThis))
    }

    pub fn verify_ret_type_c<'a>(alloc: &'a bumpalo::Bump) -> InstrSeq<'a> {
        instr(alloc, Instruct::Misc(InstructMisc::VerifyRetTypeC))
    }

    pub fn verify_ret_type_ts<'a>(alloc: &'a bumpalo::Bump) -> InstrSeq<'a> {
        instr(alloc, Instruct::Misc(InstructMisc::VerifyRetTypeTS))
    }

    pub fn verify_out_type<'a>(alloc: &'a bumpalo::Bump, i: ParamId<'a>) -> InstrSeq<'a> {
        instr(alloc, Instruct::Misc(InstructMisc::VerifyOutType(i)))
    }

    pub fn verify_param_type<'a>(alloc: &'a bumpalo::Bump, i: ParamId<'a>) -> InstrSeq<'a> {
        instr(alloc, Instruct::Misc(InstructMisc::VerifyParamType(i)))
    }

    pub fn verify_param_type_ts<'a>(alloc: &'a bumpalo::Bump, i: ParamId<'a>) -> InstrSeq<'a> {
        instr(alloc, Instruct::Misc(InstructMisc::VerifyParamTypeTS(i)))
    }

    pub fn dim<'a>(alloc: &'a bumpalo::Bump, op: MOpMode, key: MemberKey<'a>) -> InstrSeq<'a> {
        instr(alloc, Instruct::Base(InstructBase::Dim(op, key)))
    }

    pub fn dim_warn_pt<'a>(
        alloc: &'a bumpalo::Bump,
        key: PropId<'a>,
        readonly_op: ReadonlyOp,
    ) -> InstrSeq<'a> {
        dim(alloc, MOpMode::Warn, MemberKey::PT(key, readonly_op))
    }

    pub fn dim_define_pt<'a>(
        alloc: &'a bumpalo::Bump,
        key: PropId<'a>,
        readonly_op: ReadonlyOp,
    ) -> InstrSeq<'a> {
        dim(alloc, MOpMode::Define, MemberKey::PT(key, readonly_op))
    }

    pub fn fcallclsmethod<'a>(
        alloc: &'a bumpalo::Bump,
        log: IsLogAsDynamicCallOp,
        fcall_args: FcallArgs<'a>,
    ) -> InstrSeq<'a> {
        instr(
            alloc,
            Instruct::Call(InstructCall::FCallClsMethod { fcall_args, log }),
        )
    }

    pub fn fcallclsmethodd<'a>(
        alloc: &'a bumpalo::Bump,
        fcall_args: FcallArgs<'a>,
        method: MethodId<'a>,
        class: ClassId<'a>,
    ) -> InstrSeq<'a> {
        instr(
            alloc,
            Instruct::Call(InstructCall::FCallClsMethodD {
                fcall_args,
                class,
                method,
            }),
        )
    }

    pub fn fcallclsmethods<'a>(
        alloc: &'a bumpalo::Bump,
        fcall_args: FcallArgs<'a>,
        clsref: SpecialClsRef,
    ) -> InstrSeq<'a> {
        instr(
            alloc,
            Instruct::Call(InstructCall::FCallClsMethodS { fcall_args, clsref }),
        )
    }

    pub fn fcallclsmethodsd<'a>(
        alloc: &'a bumpalo::Bump,
        fcall_args: FcallArgs<'a>,
        clsref: SpecialClsRef,
        method: MethodId<'a>,
    ) -> InstrSeq<'a> {
        instr(
            alloc,
            Instruct::Call(InstructCall::FCallClsMethodSD {
                fcall_args,
                clsref,
                method,
            }),
        )
    }

    pub fn fcallctor<'a>(alloc: &'a bumpalo::Bump, fcall_args: FcallArgs<'a>) -> InstrSeq<'a> {
        instr(alloc, Instruct::Call(InstructCall::FCallCtor(fcall_args)))
    }

    pub fn fcallfunc<'a>(alloc: &'a bumpalo::Bump, fcall_args: FcallArgs<'a>) -> InstrSeq<'a> {
        instr(alloc, Instruct::Call(InstructCall::FCallFunc(fcall_args)))
    }

    pub fn fcallfuncd<'a>(
        alloc: &'a bumpalo::Bump,
        fcall_args: FcallArgs<'a>,
        func: FunctionId<'a>,
    ) -> InstrSeq<'a> {
        instr(
            alloc,
            Instruct::Call(InstructCall::FCallFuncD { fcall_args, func }),
        )
    }

    pub fn fcallobjmethod<'a>(
        alloc: &'a bumpalo::Bump,
        fcall_args: FcallArgs<'a>,
        flavor: ObjNullFlavor,
    ) -> InstrSeq<'a> {
        instr(
            alloc,
            Instruct::Call(InstructCall::FCallObjMethod { fcall_args, flavor }),
        )
    }

    pub fn fcallobjmethodd<'a>(
        alloc: &'a bumpalo::Bump,
        fcall_args: FcallArgs<'a>,
        method: MethodId<'a>,
        flavor: ObjNullFlavor,
    ) -> InstrSeq<'a> {
        instr(
            alloc,
            Instruct::Call(InstructCall::FCallObjMethodD {
                fcall_args,
                flavor,
                method,
            }),
        )
    }

    pub fn fcallobjmethodd_nullthrows<'a>(
        alloc: &'a bumpalo::Bump,
        fcall_args: FcallArgs<'a>,
        method: MethodId<'a>,
    ) -> InstrSeq<'a> {
        fcallobjmethodd(alloc, fcall_args, method, ObjNullFlavor::NullThrows)
    }

    pub fn querym<'a>(
        alloc: &'a bumpalo::Bump,
        num_params: NumParams,
        op: QueryMOp,
        key: MemberKey<'a>,
    ) -> InstrSeq<'a> {
        instr(
            alloc,
            Instruct::Final(InstructFinal::QueryM(num_params, op, key)),
        )
    }

    pub fn querym_cget_pt<'a>(
        alloc: &'a bumpalo::Bump,
        num_params: NumParams,
        key: PropId<'a>,
        readonly_op: ReadonlyOp,
    ) -> InstrSeq<'a> {
        querym(
            alloc,
            num_params,
            QueryMOp::CGet,
            MemberKey::PT(key, readonly_op),
        )
    }

    pub fn setm<'a>(
        alloc: &'a bumpalo::Bump,
        num_params: NumParams,
        key: MemberKey<'a>,
    ) -> InstrSeq<'a> {
        instr(alloc, Instruct::Final(InstructFinal::SetM(num_params, key)))
    }

    pub fn unsetm<'a>(
        alloc: &'a bumpalo::Bump,
        num_params: NumParams,
        key: MemberKey<'a>,
    ) -> InstrSeq<'a> {
        instr(
            alloc,
            Instruct::Final(InstructFinal::UnsetM(num_params, key)),
        )
    }

    pub fn setopm<'a>(
        alloc: &'a bumpalo::Bump,
        num_params: NumParams,
        op: EqOp,
        key: MemberKey<'a>,
    ) -> InstrSeq<'a> {
        instr(
            alloc,
            Instruct::Final(InstructFinal::SetOpM(num_params, op, key)),
        )
    }

    pub fn incdecm<'a>(
        alloc: &'a bumpalo::Bump,
        num_params: NumParams,
        op: IncDecOp,
        key: MemberKey<'a>,
    ) -> InstrSeq<'a> {
        instr(
            alloc,
            Instruct::Final(InstructFinal::IncDecM(num_params, op, key)),
        )
    }

    pub fn setm_pt<'a>(
        alloc: &'a bumpalo::Bump,
        num_params: NumParams,
        key: PropId<'a>,
        readonly_op: ReadonlyOp,
    ) -> InstrSeq<'a> {
        setm(alloc, num_params, MemberKey::PT(key, readonly_op))
    }

    pub fn resolve_func<'a>(alloc: &'a bumpalo::Bump, func_id: FunctionId<'a>) -> InstrSeq<'a> {
        instr(alloc, Instruct::Op(InstructOperator::ResolveFunc(func_id)))
    }

    pub fn resolve_rfunc<'a>(alloc: &'a bumpalo::Bump, func_id: FunctionId<'a>) -> InstrSeq<'a> {
        instr(alloc, Instruct::Op(InstructOperator::ResolveRFunc(func_id)))
    }

    pub fn resolveclsmethod<'a>(alloc: &'a bumpalo::Bump, method_id: MethodId<'a>) -> InstrSeq<'a> {
        instr(
            alloc,
            Instruct::Op(InstructOperator::ResolveClsMethod(method_id)),
        )
    }

    pub fn resolveclsmethodd<'a>(
        alloc: &'a bumpalo::Bump,
        class_id: ClassId<'a>,
        method_id: MethodId<'a>,
    ) -> InstrSeq<'a> {
        instr(
            alloc,
            Instruct::Op(InstructOperator::ResolveClsMethodD(class_id, method_id)),
        )
    }

    pub fn resolveclsmethods<'a>(
        alloc: &'a bumpalo::Bump,
        scref: SpecialClsRef,
        method_id: MethodId<'a>,
    ) -> InstrSeq<'a> {
        instr(
            alloc,
            Instruct::Op(InstructOperator::ResolveClsMethodS(scref, method_id)),
        )
    }

    pub fn resolverclsmethod<'a>(
        alloc: &'a bumpalo::Bump,
        method_id: MethodId<'a>,
    ) -> InstrSeq<'a> {
        instr(
            alloc,
            Instruct::Op(InstructOperator::ResolveRClsMethod(method_id)),
        )
    }

    pub fn resolverclsmethodd<'a>(
        alloc: &'a bumpalo::Bump,
        class_id: ClassId<'a>,
        method_id: MethodId<'a>,
    ) -> InstrSeq<'a> {
        instr(
            alloc,
            Instruct::Op(InstructOperator::ResolveRClsMethodD(class_id, method_id)),
        )
    }

    pub fn resolverclsmethods<'a>(
        alloc: &'a bumpalo::Bump,
        scref: SpecialClsRef,
        method_id: MethodId<'a>,
    ) -> InstrSeq<'a> {
        instr(
            alloc,
            Instruct::Op(InstructOperator::ResolveRClsMethodS(scref, method_id)),
        )
    }

    pub fn resolve_meth_caller<'a>(
        alloc: &'a bumpalo::Bump,
        fun_id: FunctionId<'a>,
    ) -> InstrSeq<'a> {
        instr(
            alloc,
            Instruct::Op(InstructOperator::ResolveMethCaller(fun_id)),
        )
    }

    pub fn resolveclass<'a>(alloc: &'a bumpalo::Bump, class_id: ClassId<'a>) -> InstrSeq<'a> {
        instr(
            alloc,
            Instruct::Op(InstructOperator::ResolveClass(class_id)),
        )
    }

    pub fn lazyclass<'a>(alloc: &'a bumpalo::Bump, class_id: ClassId<'a>) -> InstrSeq<'a> {
        instr(
            alloc,
            Instruct::LitConst(InstructLitConst::LazyClass(class_id)),
        )
    }

    pub fn oodeclexists<'a>(alloc: &'a bumpalo::Bump, class_kind: ClassishKind) -> InstrSeq<'a> {
        instr(
            alloc,
            Instruct::Misc(InstructMisc::OODeclExists(class_kind)),
        )
    }

    pub fn fatal<'a>(alloc: &'a bumpalo::Bump, op: FatalOp) -> InstrSeq<'a> {
        instr(alloc, Instruct::Op(InstructOperator::Fatal(op)))
    }

    pub fn await_<'a>(alloc: &'a bumpalo::Bump) -> InstrSeq<'a> {
        instr(alloc, Instruct::Async(AsyncFunctions::Await))
    }

    pub fn yield_<'a>(alloc: &'a bumpalo::Bump) -> InstrSeq<'a> {
        instr(alloc, Instruct::Generator(GenCreationExecution::Yield))
    }

    pub fn yieldk<'a>(alloc: &'a bumpalo::Bump) -> InstrSeq<'a> {
        instr(alloc, Instruct::Generator(GenCreationExecution::YieldK))
    }

    pub fn createcont<'a>(alloc: &'a bumpalo::Bump) -> InstrSeq<'a> {
        instr(alloc, Instruct::Generator(GenCreationExecution::CreateCont))
    }

    pub fn awaitall<'a>(
        alloc: &'a bumpalo::Bump,
        range: Option<(Local<'a>, isize)>,
    ) -> InstrSeq<'a> {
        instr(
            alloc,
            Instruct::Async(AsyncFunctions::AwaitAll(range_opt_to_maybe(range))),
        )
    }

    pub fn label<'a>(alloc: &'a bumpalo::Bump, label: Label) -> InstrSeq<'a> {
        instr(alloc, Instruct::Label(label))
    }

    pub fn awaitall_list<'a>(
        alloc: &'a bumpalo::Bump,
        unnamed_locals: std::vec::Vec<Local<'a>>,
    ) -> InstrSeq<'a> {
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
                    awaitall(
                        alloc,
                        Some((Unnamed(*head_id), unnamed_locals.len().try_into().unwrap())),
                    )
                } else {
                    panic!("Expected unnamed local")
                }
            }
        }
    }

    pub fn exit<'a>(alloc: &'a bumpalo::Bump) -> InstrSeq<'a> {
        instr(alloc, Instruct::Op(InstructOperator::Exit))
    }

    pub fn idx<'a>(alloc: &'a bumpalo::Bump) -> InstrSeq<'a> {
        instr(alloc, Instruct::Misc(InstructMisc::Idx))
    }

    pub fn array_idx<'a>(alloc: &'a bumpalo::Bump) -> InstrSeq<'a> {
        instr(alloc, Instruct::Misc(InstructMisc::ArrayIdx))
    }

    pub fn createcl<'a>(
        alloc: &'a bumpalo::Bump,
        param_num: NumParams,
        cls_num: ClassNum,
    ) -> InstrSeq<'a> {
        instr(
            alloc,
            Instruct::Misc(InstructMisc::CreateCl(param_num, cls_num)),
        )
    }

    pub fn eval<'a>(alloc: &'a bumpalo::Bump) -> InstrSeq<'a> {
        instr(
            alloc,
            Instruct::IncludeEvalDefine(InstructIncludeEvalDefine::Eval),
        )
    }

    pub fn incl<'a>(alloc: &'a bumpalo::Bump) -> InstrSeq<'a> {
        instr(
            alloc,
            Instruct::IncludeEvalDefine(InstructIncludeEvalDefine::Incl),
        )
    }

    pub fn inclonce<'a>(alloc: &'a bumpalo::Bump) -> InstrSeq<'a> {
        instr(
            alloc,
            Instruct::IncludeEvalDefine(InstructIncludeEvalDefine::InclOnce),
        )
    }

    pub fn req<'a>(alloc: &'a bumpalo::Bump) -> InstrSeq<'a> {
        instr(
            alloc,
            Instruct::IncludeEvalDefine(InstructIncludeEvalDefine::Req),
        )
    }

    pub fn reqdoc<'a>(alloc: &'a bumpalo::Bump) -> InstrSeq<'a> {
        instr(
            alloc,
            Instruct::IncludeEvalDefine(InstructIncludeEvalDefine::ReqDoc),
        )
    }

    pub fn reqonce<'a>(alloc: &'a bumpalo::Bump) -> InstrSeq<'a> {
        instr(
            alloc,
            Instruct::IncludeEvalDefine(InstructIncludeEvalDefine::ReqOnce),
        )
    }

    pub fn silence_start<'a>(alloc: &'a bumpalo::Bump, local: Local<'a>) -> InstrSeq<'a> {
        instr(
            alloc,
            Instruct::Misc(InstructMisc::Silence(local, OpSilence::Start)),
        )
    }

    pub fn silence_end<'a>(alloc: &'a bumpalo::Bump, local: Local<'a>) -> InstrSeq<'a> {
        instr(
            alloc,
            Instruct::Misc(InstructMisc::Silence(local, OpSilence::End)),
        )
    }

    pub fn contcheck_check<'a>(alloc: &'a bumpalo::Bump) -> InstrSeq<'a> {
        instr(
            alloc,
            Instruct::Generator(GenCreationExecution::ContCheck(CheckStarted::CheckStarted)),
        )
    }

    pub fn contcheck_ignore<'a>(alloc: &'a bumpalo::Bump) -> InstrSeq<'a> {
        instr(
            alloc,
            Instruct::Generator(GenCreationExecution::ContCheck(CheckStarted::IgnoreStarted)),
        )
    }

    pub fn contenter<'a>(alloc: &'a bumpalo::Bump) -> InstrSeq<'a> {
        instr(alloc, Instruct::Generator(GenCreationExecution::ContEnter))
    }

    pub fn contraise<'a>(alloc: &'a bumpalo::Bump) -> InstrSeq<'a> {
        instr(alloc, Instruct::Generator(GenCreationExecution::ContRaise))
    }

    pub fn contvalid<'a>(alloc: &'a bumpalo::Bump) -> InstrSeq<'a> {
        instr(alloc, Instruct::Generator(GenCreationExecution::ContValid))
    }

    pub fn contcurrent<'a>(alloc: &'a bumpalo::Bump) -> InstrSeq<'a> {
        instr(
            alloc,
            Instruct::Generator(GenCreationExecution::ContCurrent),
        )
    }

    pub fn contkey<'a>(alloc: &'a bumpalo::Bump) -> InstrSeq<'a> {
        instr(alloc, Instruct::Generator(GenCreationExecution::ContKey))
    }

    pub fn contgetreturn<'a>(alloc: &'a bumpalo::Bump) -> InstrSeq<'a> {
        instr(
            alloc,
            Instruct::Generator(GenCreationExecution::ContGetReturn),
        )
    }

    pub fn nativeimpl<'a>(alloc: &'a bumpalo::Bump) -> InstrSeq<'a> {
        instr(alloc, Instruct::Misc(InstructMisc::NativeImpl))
    }

    pub fn srcloc<'a>(
        alloc: &'a bumpalo::Bump,
        line_begin: isize,
        line_end: isize,
        col_begin: isize,
        col_end: isize,
    ) -> InstrSeq<'a> {
        instr(
            alloc,
            Instruct::SrcLoc(SrcLoc {
                line_begin,
                line_end,
                col_begin,
                col_end,
            }),
        )
    }

    pub fn is_type_structc_resolve<'a>(alloc: &'a bumpalo::Bump) -> InstrSeq<'a> {
        instr(
            alloc,
            Instruct::Op(InstructOperator::IsTypeStructC(
                TypeStructResolveOp::Resolve,
            )),
        )
    }

    pub fn is_type_structc_dontresolve<'a>(alloc: &'a bumpalo::Bump) -> InstrSeq<'a> {
        instr(
            alloc,
            Instruct::Op(InstructOperator::IsTypeStructC(
                TypeStructResolveOp::DontResolve,
            )),
        )
    }
}

impl<'a> InstrSeq<'a> {
    /// We can't implement `std::Clone`` because of the need for an
    /// allocator. Instead, use this associated function.
    pub fn clone(alloc: &'a bumpalo::Bump, s: &InstrSeq<'a>) -> InstrSeq<'a> {
        InstrSeq::from_iter_in(alloc, InstrIter::new(s).cloned())
    }

    /// We can't implement `std::Default` because of the need
    /// for an allocator. Instead, use this associated function
    /// to produce an empty instruction sequence.
    pub fn new_empty(alloc: &'a bumpalo::Bump) -> InstrSeq<'a> {
        InstrSeq::List(BumpSliceMut::new(
            alloc,
            bumpalo::vec![in alloc; ].into_bump_slice_mut(),
        ))
    }

    /// An instruction sequence of a single instruction.
    pub fn new_singleton(alloc: &'a bumpalo::Bump, i: Instruct<'a>) -> InstrSeq<'a> {
        InstrSeq::List(BumpSliceMut::new(
            alloc,
            bumpalo::vec![in alloc; i].into_bump_slice_mut(),
        ))
    }

    /// An instruction sequence of a sequence of instructions.
    pub fn new_list(alloc: &'a bumpalo::Bump, is: &'a mut [Instruct<'a>]) -> InstrSeq<'a> {
        InstrSeq::List(BumpSliceMut::new(alloc, is))
    }

    /// An instruction sequence of a concatenation of instruction sequences.
    pub fn new_concat(alloc: &'a bumpalo::Bump, iss: &'a mut [InstrSeq<'a>]) -> InstrSeq<'a> {
        InstrSeq::Concat(BumpSliceMut::new(alloc, iss))
    }

    /// Move instructions out of a container.
    pub fn from_iter_in<T: IntoIterator<Item = Instruct<'a>>>(
        alloc: &'a bumpalo::Bump,
        it: T,
    ) -> InstrSeq<'a> {
        InstrSeq::new_list(
            alloc,
            bumpalo::collections::Vec::from_iter_in(it, alloc).into_bump_slice_mut(),
        )
    }

    /// Transitional version. We mean to write a `gather!` in the future.
    pub fn gather(alloc: &'a bumpalo::Bump, iss: std::vec::Vec<InstrSeq<'a>>) -> InstrSeq<'a> {
        fn prd<'a>(is: &InstrSeq<'a>) -> bool {
            match is {
                InstrSeq::List(s) if s.is_empty() => false,
                _ => true,
            }
        }

        let non_empty = bumpalo::collections::Vec::from_iter_in(iss.into_iter().filter(prd), alloc);
        if non_empty.is_empty() {
            InstrSeq::new_empty(alloc)
        } else {
            InstrSeq::new_concat(alloc, non_empty.into_bump_slice_mut())
        }
    }

    pub fn iter<'i>(&'i self) -> InstrIter<'i, 'a> {
        InstrIter::new(self)
    }

    pub fn compact_iter<'i>(&'i self) -> impl Iterator<Item = &Instruct<'a>> {
        CompactIter::new(self.iter())
    }

    pub fn create_try_catch(
        alloc: &'a bumpalo::Bump,
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
        InstrSeq::gather(
            alloc,
            vec![
                instr::instr(alloc, Instruct::Try(InstructTry::TryCatchBegin)),
                try_instrs,
                instr::jmp(alloc, done_label),
                instr::instr(alloc, Instruct::Try(InstructTry::TryCatchMiddle)),
                catch_instrs,
                if skip_throw {
                    instr::empty(alloc)
                } else {
                    instr::instr(alloc, Instruct::ContFlow(InstructControlFlow::Throw))
                },
                instr::instr(alloc, Instruct::Try(InstructTry::TryCatchEnd)),
                instr::label(alloc, done_label),
            ],
        )
    }

    /// Test whether `i` is of case `Instruct::SrcLoc`.
    fn is_srcloc(instruction: &Instruct<'a>) -> bool {
        match instruction {
            Instruct::SrcLoc(_) => true,
            _ => false,
        }
    }

    pub fn first(&self) -> Option<&Instruct<'a>> {
        // self: &InstrSeq<'a>
        match self {
            InstrSeq::List(s) if s.is_empty() => None,
            InstrSeq::List(s) if s.len() == 1 => {
                let i = &s[0];
                if InstrSeq::is_srcloc(i) {
                    None
                } else {
                    Some(i)
                }
            }
            InstrSeq::List(s) => match s.iter().find(|&i| !InstrSeq::is_srcloc(i)) {
                Some(i) => Some(i),
                None => None,
            },
            InstrSeq::Concat(s) => s.iter().find_map(InstrSeq::first),
        }
    }

    /// Test for the empty instruction sequence.
    pub fn is_empty(&self) -> bool {
        // self:&InstrSeq<'a>
        match self {
            InstrSeq::List(s) if s.is_empty() => true,
            InstrSeq::List(s) if s.len() == 1 => InstrSeq::is_srcloc(&s[0]),
            InstrSeq::List(s) => s.is_empty() || s.iter().all(InstrSeq::is_srcloc),
            InstrSeq::Concat(s) => s.iter().all(InstrSeq::is_empty),
        }
    }

    pub fn flat_map_seq<F>(&self, alloc: &'a bumpalo::Bump, f: &mut F) -> Self
    where
        F: FnMut(&Instruct<'a>) -> Self,
    {
        // self: &InstrSeq<'a>
        match self {
            InstrSeq::List(s) if s.is_empty() => InstrSeq::new_empty(alloc),
            InstrSeq::List(s) if s.len() == 1 => f(&s[0]),
            InstrSeq::List(s) => InstrSeq::Concat(BumpSliceMut::new(
                alloc,
                bumpalo::collections::vec::Vec::from_iter_in(s.iter().map(f), alloc)
                    .into_bump_slice_mut(),
            )),
            InstrSeq::Concat(s) => InstrSeq::Concat(BumpSliceMut::new(
                alloc,
                bumpalo::collections::vec::Vec::from_iter_in(
                    s.iter().map(|x| x.flat_map_seq(alloc, f)),
                    alloc,
                )
                .into_bump_slice_mut(),
            )),
        }
    }

    pub fn filter_map<F>(&self, alloc: &'a bumpalo::Bump, f: &mut F) -> Self
    where
        F: FnMut(&Instruct<'a>) -> Option<Instruct<'a>>,
    {
        //self: &InstrSeq<'a>
        match self {
            InstrSeq::List(s) if s.is_empty() => InstrSeq::new_empty(alloc),
            InstrSeq::List(s) if s.len() == 1 => {
                let x: &Instruct<'a> = &s[0];
                match f(x) {
                    Some(x) => instr::instr(alloc, x),
                    None => InstrSeq::new_empty(alloc),
                }
            }
            InstrSeq::List(s) => InstrSeq::List(BumpSliceMut::new(
                alloc,
                bumpalo::collections::vec::Vec::from_iter_in(s.iter().filter_map(f), alloc)
                    .into_bump_slice_mut(),
            )),
            InstrSeq::Concat(s) => InstrSeq::Concat(BumpSliceMut::new(
                alloc,
                bumpalo::collections::vec::Vec::from_iter_in(
                    s.iter().map(|x| x.filter_map(alloc, f)),
                    alloc,
                )
                .into_bump_slice_mut(),
            )),
        }
    }

    pub fn filter_map_mut<F>(&mut self, alloc: &'a bumpalo::Bump, f: &mut F)
    where
        F: FnMut(&mut Instruct<'a>) -> bool,
    {
        //self: &mut InstrSeq<'a>
        match self {
            InstrSeq::List(s) if s.is_empty() => {}
            InstrSeq::List(s) if s.len() == 1 => {
                let x: &mut Instruct<'a> = &mut s[0];
                if !f(x) {
                    *self = instr::empty(alloc)
                }
            }
            InstrSeq::List(s) => {
                let mut new_lst = bumpalo::vec![in alloc;];
                for i in s.iter_mut() {
                    if f(i) {
                        new_lst.push(i.clone())
                    }
                }
                *self = instr::instrs(alloc, new_lst.into_bump_slice_mut())
            }
            InstrSeq::Concat(s) => s.iter_mut().for_each(|x| x.filter_map_mut(alloc, f)),
        }
    }

    pub fn map_mut<F>(&mut self, f: &mut F)
    where
        F: FnMut(&mut Instruct<'a>),
    {
        //self: &mut InstrSeq<'a>
        match self {
            InstrSeq::List(s) if s.is_empty() => {}
            InstrSeq::List(s) if s.len() == 1 => f(&mut s[0]),
            InstrSeq::List(s) => s.iter_mut().for_each(f),
            InstrSeq::Concat(s) => s.iter_mut().for_each(|x| x.map_mut(f)),
        }
    }

    #[allow(clippy::result_unit_err)]
    pub fn map_result_mut<F>(&mut self, f: &mut F) -> Result<()>
    where
        F: FnMut(&mut Instruct<'a>) -> Result<()>,
    {
        //self: &mut InstrSeq<'a>
        match self {
            InstrSeq::List(s) if s.is_empty() => Ok(()),
            InstrSeq::List(s) if s.len() == 1 => f(&mut s[0]),
            InstrSeq::List(s) => s.iter_mut().try_for_each(f),
            InstrSeq::Concat(s) => s.iter_mut().try_for_each(|x| x.map_result_mut(f)),
        }
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use crate::instr::{instr, instrs};
    use pretty_assertions::assert_eq;

    #[test]
    fn iter() {
        let a = bumpalo::Bump::new();
        let alloc: &bumpalo::Bump = &a;
        let mk_i = || Instruct::Comment(Str::from(""));
        let empty = || InstrSeq::new_empty(alloc);

        let one = || instr(alloc, mk_i());
        let list0 = || instrs(alloc, bumpalo::vec![in alloc;].into_bump_slice_mut());
        let list1 = || instrs(alloc, bumpalo::vec![in alloc; mk_i()].into_bump_slice_mut());
        let list2 = || {
            instrs(
                alloc,
                bumpalo::vec![in alloc; mk_i(), mk_i()].into_bump_slice_mut(),
            )
        };
        let concat0 = || {
            InstrSeq::Concat(BumpSliceMut::new(
                alloc,
                bumpalo::vec![in alloc;].into_bump_slice_mut(),
            ))
        };
        let concat1 = || {
            InstrSeq::Concat(BumpSliceMut::new(
                alloc,
                bumpalo::vec![in alloc; one()].into_bump_slice_mut(),
            ))
        };

        assert_eq!(empty().iter().count(), 0);
        assert_eq!(one().iter().count(), 1);
        assert_eq!(list0().iter().count(), 0);
        assert_eq!(list1().iter().count(), 1);
        assert_eq!(list2().iter().count(), 2);
        assert_eq!(concat0().iter().count(), 0);
        assert_eq!(concat1().iter().count(), 1);

        let concat = InstrSeq::Concat(BumpSliceMut::new(
            alloc,
            bumpalo::vec![in alloc; empty()].into_bump_slice_mut(),
        ));
        assert_eq!(concat.iter().count(), 0);

        let concat = InstrSeq::Concat(BumpSliceMut::new(
            alloc,
            bumpalo::vec![in alloc; empty(), one()].into_bump_slice_mut(),
        ));
        assert_eq!(concat.iter().count(), 1);

        let concat = InstrSeq::Concat(BumpSliceMut::new(
            alloc,
            bumpalo::vec![in alloc; one(), empty()].into_bump_slice_mut(),
        ));
        assert_eq!(concat.iter().count(), 1);

        let concat = InstrSeq::Concat(BumpSliceMut::new(
            alloc,
            bumpalo::vec![in alloc; one(), list1()].into_bump_slice_mut(),
        ));
        assert_eq!(concat.iter().count(), 2);

        let concat = InstrSeq::Concat(BumpSliceMut::new(
            alloc,
            bumpalo::vec![in alloc; list2(), list1()].into_bump_slice_mut(),
        ));
        assert_eq!(concat.iter().count(), 3);

        let concat = InstrSeq::Concat(BumpSliceMut::new(
            alloc,
            bumpalo::vec![in alloc; concat0(), list2(), list1()].into_bump_slice_mut(),
        ));
        assert_eq!(concat.iter().count(), 3);

        let concat = InstrSeq::Concat(BumpSliceMut::new(
            alloc,
            bumpalo::vec![in alloc; concat1(), concat1()].into_bump_slice_mut(),
        ));
        assert_eq!(concat.iter().count(), 2);

        let concat = InstrSeq::Concat(BumpSliceMut::new(
            alloc,
            bumpalo::vec![in alloc; concat0(), concat1()].into_bump_slice_mut(),
        ));
        assert_eq!(concat.iter().count(), 1);

        let concat = InstrSeq::Concat(BumpSliceMut::new(
            alloc,
            bumpalo::vec![in alloc; list2(), concat1()].into_bump_slice_mut(),
        ));
        assert_eq!(concat.iter().count(), 3);

        let concat = InstrSeq::Concat(BumpSliceMut::new(
            alloc,
            bumpalo::vec![in alloc; list2(), concat0()].into_bump_slice_mut(),
        ));
        assert_eq!(concat.iter().count(), 2);

        let concat = InstrSeq::Concat(BumpSliceMut::new(
            alloc,
            bumpalo::vec![in alloc; one(), concat0()].into_bump_slice_mut(),
        ));
        assert_eq!(concat.iter().count(), 1);

        let concat = InstrSeq::Concat(BumpSliceMut::new(
            alloc,
            bumpalo::vec![in alloc; empty(), concat0()].into_bump_slice_mut(),
        ));
        assert_eq!(concat.iter().count(), 0);
    }
}

#[no_mangle]
pub unsafe extern "C" fn no_call_compile_only_USED_TYPES_instruction_sequence<'arena>(
    _: InstrSeq<'arena>,
) {
    unimplemented!()
}
