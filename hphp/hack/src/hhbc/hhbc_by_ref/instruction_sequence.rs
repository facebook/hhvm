// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use hhbc_by_ref_hhbc_ast::*;
use hhbc_by_ref_iterator::Id as IterId;
use hhbc_by_ref_label::Label;
use hhbc_by_ref_local as local;
use hhbc_by_ref_runtime::TypedValue;
use oxidized::ast_defs::Pos;
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
/// sequence of instructions. This could simply be represented by a
/// list, but then we would need to use an accumulator to avoid the
/// quadratic complexity associated with repeated appending to a list.
/// Instead, we simply build a tree of instructions which can easily
/// be flattened at the end.
#[derive(Debug)]
pub enum InstrSeq<'a> {
    List(&'a mut [Instruct<'a>]),
    Concat(&'a mut [InstrSeq<'a>]),
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
        itertools::Either<(&'i &'a mut [Instruct<'a>], usize), (&'i &'a mut [InstrSeq<'a>], usize)>,
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
        match &*(self.instr_seq) {
            InstrSeq::List(&mut []) => None,
            InstrSeq::List(&mut [_]) if self.index > 0 => None,
            InstrSeq::List(&mut [ref i]) => {
                self.index += 1;
                Some(i)
            }
            InstrSeq::List(is @ &mut [_, _, ..]) if self.index >= is.len() => None,
            InstrSeq::List(is @ &mut [_, _, ..]) => {
                let r = is.get(self.index);
                self.index += 1;
                r
            }

            InstrSeq::Concat(ref cc) => {
                if self.concat_stack.is_empty() {
                    if self.index == 0 {
                        self.index += 1;
                        self.concat_stack.push(itertools::Either::Right((cc, 0)));
                    } else {
                        return None;
                    }
                }

                while !self.concat_stack.is_empty() {
                    let top: &mut itertools::Either<_, _> = self.concat_stack.last_mut().unwrap();
                    match *top {
                        itertools::Either::Left((list, size)) if size >= list.len() => {
                            self.concat_stack.pop();
                        }
                        itertools::Either::Left((list, ref mut size)) => {
                            let r: &Instruct<'a> = &(list[*size]);
                            *size += 1;
                            return Some(r);
                        }
                        itertools::Either::Right((concat, size)) if size >= concat.len() => {
                            self.concat_stack.pop();
                        }
                        itertools::Either::Right((concat, ref mut size)) => {
                            let i: &InstrSeq<'a> = &(concat[*size]);
                            *size += 1;
                            match *i {
                                InstrSeq::List(&mut []) => {}
                                InstrSeq::List(&mut [ref instr]) => {
                                    return Some(instr);
                                }
                                InstrSeq::List(ref list) => {
                                    self.concat_stack.push(itertools::Either::Left((list, 0)));
                                }
                                InstrSeq::Concat(ref concat) => {
                                    self.concat_stack
                                        .push(itertools::Either::Right((concat, 0)));
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

#[allow(clippy::needless_lifetimes)]
pub mod instr {
    use crate::*;

    pub fn empty<'a>(alloc: &'a bumpalo::Bump) -> InstrSeq<'a> {
        InstrSeq::new_empty(alloc)
    }

    pub fn instr<'a>(alloc: &'a bumpalo::Bump, i: Instruct<'a>) -> InstrSeq<'a> {
        InstrSeq::new_singleton(alloc, i)
    }

    pub fn instrs<'a>(_alloc: &'a bumpalo::Bump, is: &'a mut [Instruct<'a>]) -> InstrSeq<'a> {
        InstrSeq::new_list(is)
    }

    pub fn lit_const<'a>(alloc: &'a bumpalo::Bump, l: InstructLitConst<'a>) -> InstrSeq<'a> {
        instr(alloc, Instruct::ILitConst(l))
    }

    pub fn iterinit<'a>(
        alloc: &'a bumpalo::Bump,
        args: IterArgs<'a>,
        label: Label<'a>,
    ) -> InstrSeq<'a> {
        instr(
            alloc,
            Instruct::IIterator(InstructIterator::IterInit(args, label)),
        )
    }

    pub fn iternext<'a>(
        alloc: &'a bumpalo::Bump,
        args: IterArgs<'a>,
        label: Label<'a>,
    ) -> InstrSeq<'a> {
        instr(
            alloc,
            Instruct::IIterator(InstructIterator::IterNext(args, label)),
        )
    }

    pub fn iternextk<'a>(
        alloc: &'a bumpalo::Bump,
        id: IterId,
        label: Label<'a>,
        value: local::Type<'a>,
        key: local::Type<'a>,
    ) -> InstrSeq<'a> {
        let args = IterArgs {
            iter_id: id,
            key_id: Some(key),
            val_id: value,
        };
        instr(
            alloc,
            Instruct::IIterator(InstructIterator::IterNext(args, label)),
        )
    }

    pub fn iterfree<'a>(alloc: &'a bumpalo::Bump, id: IterId) -> InstrSeq<'a> {
        instr(alloc, Instruct::IIterator(InstructIterator::IterFree(id)))
    }

    pub fn whresult<'a>(alloc: &'a bumpalo::Bump) -> InstrSeq<'a> {
        instr(alloc, Instruct::IAsync(AsyncFunctions::WHResult))
    }

    pub fn jmp<'a>(alloc: &'a bumpalo::Bump, label: Label<'a>) -> InstrSeq<'a> {
        instr(alloc, Instruct::IContFlow(InstructControlFlow::Jmp(label)))
    }

    pub fn jmpz<'a>(alloc: &'a bumpalo::Bump, label: Label<'a>) -> InstrSeq<'a> {
        instr(alloc, Instruct::IContFlow(InstructControlFlow::JmpZ(label)))
    }

    pub fn jmpnz<'a>(alloc: &'a bumpalo::Bump, label: Label<'a>) -> InstrSeq<'a> {
        instr(
            alloc,
            Instruct::IContFlow(InstructControlFlow::JmpNZ(label)),
        )
    }

    pub fn jmpns<'a>(alloc: &'a bumpalo::Bump, label: Label<'a>) -> InstrSeq<'a> {
        instr(
            alloc,
            Instruct::IContFlow(InstructControlFlow::JmpNS(label)),
        )
    }

    pub fn continue_<'a>(alloc: &'a bumpalo::Bump, level: isize) -> InstrSeq<'a> {
        instr(
            alloc,
            Instruct::ISpecialFlow(InstructSpecialFlow::Continue(level)),
        )
    }

    pub fn break_<'a>(alloc: &'a bumpalo::Bump, level: isize) -> InstrSeq<'a> {
        instr(
            alloc,
            Instruct::ISpecialFlow(InstructSpecialFlow::Break(level)),
        )
    }

    pub fn goto<'a>(alloc: &'a bumpalo::Bump, label: std::string::String) -> InstrSeq {
        instr(
            alloc,
            Instruct::ISpecialFlow(InstructSpecialFlow::Goto(
                bumpalo::collections::String::from_str_in(label.as_str(), alloc).into_bump_str(),
            )),
        )
    }

    pub fn iter_break<'a>(
        alloc: &'a bumpalo::Bump,
        label: Label<'a>,
        itrs: std::vec::Vec<IterId>,
    ) -> InstrSeq<'a> {
        let mut vec = bumpalo::collections::Vec::from_iter_in(
            itrs.into_iter()
                .map(|id| Instruct::IIterator(InstructIterator::IterFree(id))),
            alloc,
        );
        vec.push(Instruct::IContFlow(InstructControlFlow::Jmp(label)));
        instrs(alloc, vec.into_bump_slice_mut())
    }

    pub fn false_<'a>(alloc: &'a bumpalo::Bump) -> InstrSeq<'a> {
        instr(alloc, Instruct::ILitConst(InstructLitConst::False))
    }

    pub fn true_<'a>(alloc: &'a bumpalo::Bump) -> InstrSeq<'a> {
        instr(alloc, Instruct::ILitConst(InstructLitConst::True))
    }

    pub fn clscnsd<'a>(
        alloc: &'a bumpalo::Bump,
        const_id: ConstId<'a>,
        cid: ClassId<'a>,
    ) -> InstrSeq<'a> {
        instr(
            alloc,
            Instruct::ILitConst(InstructLitConst::ClsCnsD(const_id, cid)),
        )
    }

    pub fn clscns<'a>(alloc: &'a bumpalo::Bump, const_id: ConstId<'a>) -> InstrSeq<'a> {
        instr(
            alloc,
            Instruct::ILitConst(InstructLitConst::ClsCns(const_id)),
        )
    }

    pub fn clscnsl<'a>(alloc: &'a bumpalo::Bump, local: local::Type<'a>) -> InstrSeq<'a> {
        instr(alloc, Instruct::ILitConst(InstructLitConst::ClsCnsL(local)))
    }

    pub fn eq<'a>(alloc: &'a bumpalo::Bump) -> InstrSeq<'a> {
        instr(alloc, Instruct::IOp(InstructOperator::Eq))
    }

    pub fn neq<'a>(alloc: &'a bumpalo::Bump) -> InstrSeq<'a> {
        instr(alloc, Instruct::IOp(InstructOperator::Neq))
    }

    pub fn gt<'a>(alloc: &'a bumpalo::Bump) -> InstrSeq<'a> {
        instr(alloc, Instruct::IOp(InstructOperator::Gt))
    }

    pub fn gte<'a>(alloc: &'a bumpalo::Bump) -> InstrSeq<'a> {
        instr(alloc, Instruct::IOp(InstructOperator::Gte))
    }

    pub fn lt<'a>(alloc: &'a bumpalo::Bump) -> InstrSeq<'a> {
        instr(alloc, Instruct::IOp(InstructOperator::Lt))
    }

    pub fn lte<'a>(alloc: &'a bumpalo::Bump) -> InstrSeq<'a> {
        instr(alloc, Instruct::IOp(InstructOperator::Lte))
    }

    pub fn concat<'a>(alloc: &'a bumpalo::Bump) -> InstrSeq<'a> {
        instr(alloc, Instruct::IOp(InstructOperator::Concat))
    }

    pub fn concatn<'a>(alloc: &'a bumpalo::Bump, n: isize) -> InstrSeq<'a> {
        instr(alloc, Instruct::IOp(InstructOperator::ConcatN(n)))
    }

    pub fn print<'a>(alloc: &'a bumpalo::Bump) -> InstrSeq<'a> {
        instr(alloc, Instruct::IOp(InstructOperator::Print))
    }

    pub fn cast_dict<'a>(alloc: &'a bumpalo::Bump) -> InstrSeq<'a> {
        instr(alloc, Instruct::IOp(InstructOperator::CastDict))
    }

    pub fn cast_string<'a>(alloc: &'a bumpalo::Bump) -> InstrSeq<'a> {
        instr(alloc, Instruct::IOp(InstructOperator::CastString))
    }

    pub fn cast_int<'a>(alloc: &'a bumpalo::Bump) -> InstrSeq<'a> {
        instr(alloc, Instruct::IOp(InstructOperator::CastInt))
    }

    pub fn cast_bool<'a>(alloc: &'a bumpalo::Bump) -> InstrSeq<'a> {
        instr(alloc, Instruct::IOp(InstructOperator::CastBool))
    }

    pub fn cast_double<'a>(alloc: &'a bumpalo::Bump) -> InstrSeq<'a> {
        instr(alloc, Instruct::IOp(InstructOperator::CastDouble))
    }

    pub fn retc<'a>(alloc: &'a bumpalo::Bump) -> InstrSeq<'a> {
        instr(alloc, Instruct::IContFlow(InstructControlFlow::RetC))
    }

    pub fn retc_suspended<'a>(alloc: &'a bumpalo::Bump) -> InstrSeq<'a> {
        instr(
            alloc,
            Instruct::IContFlow(InstructControlFlow::RetCSuspended),
        )
    }

    pub fn retm<'a>(alloc: &'a bumpalo::Bump, p: NumParams) -> InstrSeq<'a> {
        instr(alloc, Instruct::IContFlow(InstructControlFlow::RetM(p)))
    }

    pub fn null<'a>(alloc: &'a bumpalo::Bump) -> InstrSeq<'a> {
        instr(alloc, Instruct::ILitConst(InstructLitConst::Null))
    }

    pub fn nulluninit<'a>(alloc: &'a bumpalo::Bump) -> InstrSeq<'a> {
        instr(alloc, Instruct::ILitConst(InstructLitConst::NullUninit))
    }

    pub fn chain_faults<'a>(alloc: &'a bumpalo::Bump) -> InstrSeq<'a> {
        instr(alloc, Instruct::IMisc(InstructMisc::ChainFaults))
    }

    pub fn dup<'a>(alloc: &'a bumpalo::Bump) -> InstrSeq<'a> {
        instr(alloc, Instruct::IBasic(InstructBasic::Dup))
    }

    pub fn nop<'a>(alloc: &'a bumpalo::Bump) -> InstrSeq<'a> {
        instr(alloc, Instruct::IBasic(InstructBasic::Nop))
    }

    pub fn instanceofd<'a>(alloc: &'a bumpalo::Bump, s: ClassId<'a>) -> InstrSeq<'a> {
        instr(alloc, Instruct::IOp(InstructOperator::InstanceOfD(s)))
    }

    pub fn instanceof<'a>(alloc: &'a bumpalo::Bump) -> InstrSeq<'a> {
        instr(alloc, Instruct::IOp(InstructOperator::InstanceOf))
    }

    pub fn islateboundcls<'a>(alloc: &'a bumpalo::Bump) -> InstrSeq<'a> {
        instr(alloc, Instruct::IOp(InstructOperator::IsLateBoundCls))
    }

    pub fn istypestructc<'a>(alloc: &'a bumpalo::Bump, mode: TypestructResolveOp) -> InstrSeq<'a> {
        instr(alloc, Instruct::IOp(InstructOperator::IsTypeStructC(mode)))
    }

    pub fn throwastypestructexception<'a>(alloc: &'a bumpalo::Bump) -> InstrSeq<'a> {
        instr(
            alloc,
            Instruct::IOp(InstructOperator::ThrowAsTypeStructException),
        )
    }

    pub fn throw_non_exhaustive_switch<'a>(alloc: &'a bumpalo::Bump) -> InstrSeq<'a> {
        instr(
            alloc,
            Instruct::IMisc(InstructMisc::ThrowNonExhaustiveSwitch),
        )
    }

    pub fn raise_class_string_conversion_warning<'a>(alloc: &'a bumpalo::Bump) -> InstrSeq<'a> {
        instr(
            alloc,
            Instruct::IMisc(InstructMisc::RaiseClassStringConversionWarning),
        )
    }

    pub fn combine_and_resolve_type_struct<'a>(alloc: &'a bumpalo::Bump, i: isize) -> InstrSeq<'a> {
        instr(
            alloc,
            Instruct::IOp(InstructOperator::CombineAndResolveTypeStruct(i)),
        )
    }

    pub fn record_reified_generic<'a>(alloc: &'a bumpalo::Bump) -> InstrSeq<'a> {
        instr(alloc, Instruct::IMisc(InstructMisc::RecordReifiedGeneric))
    }

    pub fn check_reified_generic_mismatch<'a>(alloc: &'a bumpalo::Bump) -> InstrSeq<'a> {
        instr(
            alloc,
            Instruct::IMisc(InstructMisc::CheckReifiedGenericMismatch),
        )
    }

    pub fn int<'a>(alloc: &'a bumpalo::Bump, i: isize) -> InstrSeq<'a> {
        use std::convert::TryInto;
        instr(
            alloc,
            Instruct::ILitConst(InstructLitConst::Int(i.try_into().unwrap())),
        )
    }

    pub fn int64<'a>(alloc: &'a bumpalo::Bump, i: i64) -> InstrSeq<'a> {
        instr(alloc, Instruct::ILitConst(InstructLitConst::Int(i)))
    }

    pub fn int_of_string<'a>(alloc: &'a bumpalo::Bump, litstr: &str) -> InstrSeq<'a> {
        instr(
            alloc,
            Instruct::ILitConst(InstructLitConst::Int(litstr.parse::<i64>().unwrap())),
        )
    }

    pub fn double<'a>(alloc: &'a bumpalo::Bump, litstr: &str) -> InstrSeq<'a> {
        instr(
            alloc,
            Instruct::ILitConst(InstructLitConst::Double(
                bumpalo::collections::String::from_str_in(litstr, alloc).into_bump_str(),
            )),
        )
    }

    pub fn string<'a>(alloc: &'a bumpalo::Bump, litstr: impl Into<String>) -> InstrSeq<'a> {
        instr(
            alloc,
            Instruct::ILitConst(InstructLitConst::String(
                bumpalo::collections::String::from_str_in(litstr.into().as_str(), alloc)
                    .into_bump_str(),
            )),
        )
    }

    pub fn this<'a>(alloc: &'a bumpalo::Bump) -> InstrSeq<'a> {
        instr(alloc, Instruct::IMisc(InstructMisc::This))
    }

    pub fn istypec<'a>(alloc: &'a bumpalo::Bump, op: IstypeOp) -> InstrSeq<'a> {
        instr(alloc, Instruct::IIsset(InstructIsset::IsTypeC(op)))
    }

    pub fn istypel<'a>(
        alloc: &'a bumpalo::Bump,
        id: local::Type<'a>,
        op: IstypeOp,
    ) -> InstrSeq<'a> {
        instr(alloc, Instruct::IIsset(InstructIsset::IsTypeL(id, op)))
    }

    pub fn add<'a>(alloc: &'a bumpalo::Bump) -> InstrSeq<'a> {
        instr(alloc, Instruct::IOp(InstructOperator::Add))
    }

    pub fn addo<'a>(alloc: &'a bumpalo::Bump) -> InstrSeq<'a> {
        instr(alloc, Instruct::IOp(InstructOperator::AddO))
    }

    pub fn sub<'a>(alloc: &'a bumpalo::Bump) -> InstrSeq<'a> {
        instr(alloc, Instruct::IOp(InstructOperator::Sub))
    }

    pub fn subo<'a>(alloc: &'a bumpalo::Bump) -> InstrSeq<'a> {
        instr(alloc, Instruct::IOp(InstructOperator::SubO))
    }

    pub fn mul<'a>(alloc: &'a bumpalo::Bump) -> InstrSeq<'a> {
        instr(alloc, Instruct::IOp(InstructOperator::Mul))
    }

    pub fn mulo<'a>(alloc: &'a bumpalo::Bump) -> InstrSeq<'a> {
        instr(alloc, Instruct::IOp(InstructOperator::MulO))
    }

    pub fn shl<'a>(alloc: &'a bumpalo::Bump) -> InstrSeq<'a> {
        instr(alloc, Instruct::IOp(InstructOperator::Shl))
    }

    pub fn shr<'a>(alloc: &'a bumpalo::Bump) -> InstrSeq<'a> {
        instr(alloc, Instruct::IOp(InstructOperator::Shr))
    }

    pub fn cmp<'a>(alloc: &'a bumpalo::Bump) -> InstrSeq<'a> {
        instr(alloc, Instruct::IOp(InstructOperator::Cmp))
    }

    pub fn mod_<'a>(alloc: &'a bumpalo::Bump) -> InstrSeq<'a> {
        instr(alloc, Instruct::IOp(InstructOperator::Mod))
    }

    pub fn div<'a>(alloc: &'a bumpalo::Bump) -> InstrSeq<'a> {
        instr(alloc, Instruct::IOp(InstructOperator::Div))
    }

    pub fn same<'a>(alloc: &'a bumpalo::Bump) -> InstrSeq<'a> {
        instr(alloc, Instruct::IOp(InstructOperator::Same))
    }

    pub fn pow<'a>(alloc: &'a bumpalo::Bump) -> InstrSeq<'a> {
        instr(alloc, Instruct::IOp(InstructOperator::Pow))
    }

    pub fn nsame<'a>(alloc: &'a bumpalo::Bump) -> InstrSeq<'a> {
        instr(alloc, Instruct::IOp(InstructOperator::NSame))
    }

    pub fn not<'a>(alloc: &'a bumpalo::Bump) -> InstrSeq<'a> {
        instr(alloc, Instruct::IOp(InstructOperator::Not))
    }

    pub fn bitnot<'a>(alloc: &'a bumpalo::Bump) -> InstrSeq<'a> {
        instr(alloc, Instruct::IOp(InstructOperator::BitNot))
    }

    pub fn bitand<'a>(alloc: &'a bumpalo::Bump) -> InstrSeq<'a> {
        instr(alloc, Instruct::IOp(InstructOperator::BitAnd))
    }

    pub fn bitor<'a>(alloc: &'a bumpalo::Bump) -> InstrSeq<'a> {
        instr(alloc, Instruct::IOp(InstructOperator::BitOr))
    }

    pub fn bitxor<'a>(alloc: &'a bumpalo::Bump) -> InstrSeq<'a> {
        instr(alloc, Instruct::IOp(InstructOperator::BitXor))
    }

    pub fn sets<'a>(alloc: &'a bumpalo::Bump, readonly_op: ReadOnlyOp) -> InstrSeq<'a> {
        instr(
            alloc,
            Instruct::IMutator(InstructMutator::SetS(readonly_op)),
        )
    }

    pub fn setl<'a>(alloc: &'a bumpalo::Bump, local: local::Type<'a>) -> InstrSeq<'a> {
        instr(alloc, Instruct::IMutator(InstructMutator::SetL(local)))
    }

    pub fn setg<'a>(alloc: &'a bumpalo::Bump) -> InstrSeq<'a> {
        instr(alloc, Instruct::IMutator(InstructMutator::SetG))
    }

    pub fn unsetl<'a>(alloc: &'a bumpalo::Bump, local: local::Type<'a>) -> InstrSeq<'a> {
        instr(alloc, Instruct::IMutator(InstructMutator::UnsetL(local)))
    }

    pub fn unsetg<'a>(alloc: &'a bumpalo::Bump) -> InstrSeq<'a> {
        instr(alloc, Instruct::IMutator(InstructMutator::UnsetG))
    }

    pub fn incdecl<'a>(
        alloc: &'a bumpalo::Bump,
        local: local::Type<'a>,
        op: IncdecOp,
    ) -> InstrSeq<'a> {
        instr(
            alloc,
            Instruct::IMutator(InstructMutator::IncDecL(local, op)),
        )
    }

    pub fn incdecg<'a>(alloc: &'a bumpalo::Bump, op: IncdecOp) -> InstrSeq<'a> {
        instr(alloc, Instruct::IMutator(InstructMutator::IncDecG(op)))
    }

    pub fn incdecs<'a>(
        alloc: &'a bumpalo::Bump,
        op: IncdecOp,
        readonly_op: ReadOnlyOp,
    ) -> InstrSeq<'a> {
        instr(
            alloc,
            Instruct::IMutator(InstructMutator::IncDecS(op, readonly_op)),
        )
    }

    pub fn setopg<'a>(alloc: &'a bumpalo::Bump, op: EqOp) -> InstrSeq<'a> {
        instr(alloc, Instruct::IMutator(InstructMutator::SetOpG(op)))
    }

    pub fn setopl<'a>(alloc: &'a bumpalo::Bump, local: local::Type<'a>, op: EqOp) -> InstrSeq<'a> {
        instr(
            alloc,
            Instruct::IMutator(InstructMutator::SetOpL(local, op)),
        )
    }

    pub fn setops<'a>(alloc: &'a bumpalo::Bump, op: EqOp, readonly_op: ReadOnlyOp) -> InstrSeq<'a> {
        instr(
            alloc,
            Instruct::IMutator(InstructMutator::SetOpS(op, readonly_op)),
        )
    }

    pub fn issetl<'a>(alloc: &'a bumpalo::Bump, local: local::Type<'a>) -> InstrSeq<'a> {
        instr(alloc, Instruct::IIsset(InstructIsset::IssetL(local)))
    }

    pub fn issetg<'a>(alloc: &'a bumpalo::Bump) -> InstrSeq<'a> {
        instr(alloc, Instruct::IIsset(InstructIsset::IssetG))
    }

    pub fn issets<'a>(alloc: &'a bumpalo::Bump) -> InstrSeq<'a> {
        instr(alloc, Instruct::IIsset(InstructIsset::IssetS))
    }

    pub fn isunsetl<'a>(alloc: &'a bumpalo::Bump, local: local::Type<'a>) -> InstrSeq<'a> {
        instr(alloc, Instruct::IIsset(InstructIsset::IsUnsetL(local)))
    }

    pub fn cgets<'a>(alloc: &'a bumpalo::Bump, readonly_op: ReadOnlyOp) -> InstrSeq<'a> {
        instr(alloc, Instruct::IGet(InstructGet::CGetS(readonly_op)))
    }

    pub fn cgetg<'a>(alloc: &'a bumpalo::Bump) -> InstrSeq<'a> {
        instr(alloc, Instruct::IGet(InstructGet::CGetG))
    }

    pub fn cgetl<'a>(alloc: &'a bumpalo::Bump, local: local::Type<'a>) -> InstrSeq<'a> {
        instr(alloc, Instruct::IGet(InstructGet::CGetL(local)))
    }

    pub fn cugetl<'a>(alloc: &'a bumpalo::Bump, local: local::Type<'a>) -> InstrSeq<'a> {
        instr(alloc, Instruct::IGet(InstructGet::CUGetL(local)))
    }

    pub fn cgetl2<'a>(alloc: &'a bumpalo::Bump, local: local::Type<'a>) -> InstrSeq<'a> {
        instr(alloc, Instruct::IGet(InstructGet::CGetL2(local)))
    }

    pub fn cgetquietl<'a>(alloc: &'a bumpalo::Bump, local: local::Type<'a>) -> InstrSeq<'a> {
        instr(alloc, Instruct::IGet(InstructGet::CGetQuietL(local)))
    }

    pub fn classgetc<'a>(alloc: &'a bumpalo::Bump) -> InstrSeq<'a> {
        instr(alloc, Instruct::IGet(InstructGet::ClassGetC))
    }

    pub fn classgetts<'a>(alloc: &'a bumpalo::Bump) -> InstrSeq<'a> {
        instr(alloc, Instruct::IGet(InstructGet::ClassGetTS))
    }

    pub fn classname<'a>(alloc: &'a bumpalo::Bump) -> InstrSeq<'a> {
        instr(alloc, Instruct::IMisc(InstructMisc::ClassName))
    }

    pub fn self_<'a>(alloc: &'a bumpalo::Bump) -> InstrSeq<'a> {
        instr(alloc, Instruct::IMisc(InstructMisc::Self_))
    }

    pub fn lateboundcls<'a>(alloc: &'a bumpalo::Bump) -> InstrSeq<'a> {
        instr(alloc, Instruct::IMisc(InstructMisc::LateBoundCls))
    }

    pub fn parent<'a>(alloc: &'a bumpalo::Bump) -> InstrSeq<'a> {
        instr(alloc, Instruct::IMisc(InstructMisc::Parent))
    }

    pub fn popu<'a>(alloc: &'a bumpalo::Bump) -> InstrSeq<'a> {
        instr(alloc, Instruct::IBasic(InstructBasic::PopU))
    }

    pub fn popc<'a>(alloc: &'a bumpalo::Bump) -> InstrSeq<'a> {
        instr(alloc, Instruct::IBasic(InstructBasic::PopC))
    }

    pub fn popl<'a>(alloc: &'a bumpalo::Bump, l: local::Type<'a>) -> InstrSeq<'a> {
        instr(alloc, Instruct::IMutator(InstructMutator::PopL(l)))
    }

    pub fn initprop<'a>(alloc: &'a bumpalo::Bump, pid: PropId<'a>, op: InitpropOp) -> InstrSeq<'a> {
        instr(
            alloc,
            Instruct::IMutator(InstructMutator::InitProp(pid, op)),
        )
    }

    pub fn checkprop<'a>(alloc: &'a bumpalo::Bump, pid: PropId<'a>) -> InstrSeq<'a> {
        instr(alloc, Instruct::IMutator(InstructMutator::CheckProp(pid)))
    }

    pub fn pushl<'a>(alloc: &'a bumpalo::Bump, local: local::Type<'a>) -> InstrSeq<'a> {
        instr(alloc, Instruct::IGet(InstructGet::PushL(local)))
    }

    pub fn throw<'a>(alloc: &'a bumpalo::Bump) -> InstrSeq<'a> {
        instr(alloc, Instruct::IContFlow(InstructControlFlow::Throw))
    }

    pub fn new_vec_array<'a>(alloc: &'a bumpalo::Bump, i: isize) -> InstrSeq<'a> {
        instr(alloc, Instruct::ILitConst(InstructLitConst::NewVec(i)))
    }

    pub fn new_pair<'a>(alloc: &'a bumpalo::Bump) -> InstrSeq<'a> {
        instr(alloc, Instruct::ILitConst(InstructLitConst::NewPair))
    }

    pub fn add_elemc<'a>(alloc: &'a bumpalo::Bump) -> InstrSeq<'a> {
        instr(alloc, Instruct::ILitConst(InstructLitConst::AddElemC))
    }

    pub fn add_new_elemc<'a>(alloc: &'a bumpalo::Bump) -> InstrSeq<'a> {
        instr(alloc, Instruct::ILitConst(InstructLitConst::AddNewElemC))
    }

    pub fn switch<'a>(
        alloc: &'a bumpalo::Bump,
        labels: bumpalo::collections::Vec<'a, Label<'a>>,
    ) -> InstrSeq<'a> {
        instr(
            alloc,
            Instruct::IContFlow(InstructControlFlow::Switch(
                Switchkind::Unbounded,
                0,
                labels,
            )),
        )
    }

    pub fn newobj<'a>(alloc: &'a bumpalo::Bump) -> InstrSeq<'a> {
        instr(alloc, Instruct::ICall(InstructCall::NewObj))
    }

    pub fn sswitch<'a>(
        alloc: &'a bumpalo::Bump,
        cases: bumpalo::collections::Vec<'a, (&'a str, Label<'a>)>,
    ) -> InstrSeq<'a> {
        instr(
            alloc,
            Instruct::IContFlow(InstructControlFlow::SSwitch(cases)),
        )
    }

    pub fn newobjr<'a>(alloc: &'a bumpalo::Bump) -> InstrSeq<'a> {
        instr(alloc, Instruct::ICall(InstructCall::NewObjR))
    }

    pub fn newobjd<'a>(alloc: &'a bumpalo::Bump, id: ClassId<'a>) -> InstrSeq<'a> {
        instr(alloc, Instruct::ICall(InstructCall::NewObjD(id)))
    }

    pub fn newobjrd<'a>(alloc: &'a bumpalo::Bump, id: ClassId<'a>) -> InstrSeq<'a> {
        instr(alloc, Instruct::ICall(InstructCall::NewObjRD(id)))
    }

    pub fn newobjs<'a>(alloc: &'a bumpalo::Bump, scref: SpecialClsRef) -> InstrSeq<'a> {
        instr(alloc, Instruct::ICall(InstructCall::NewObjS(scref)))
    }

    pub fn lockobj<'a>(alloc: &'a bumpalo::Bump) -> InstrSeq<'a> {
        instr(alloc, Instruct::IMisc(InstructMisc::LockObj))
    }

    pub fn clone<'a>(alloc: &'a bumpalo::Bump) -> InstrSeq<'a> {
        instr(alloc, Instruct::IOp(InstructOperator::Clone))
    }

    pub fn new_record<'a>(
        alloc: &'a bumpalo::Bump,
        id: ClassId<'a>,
        keys: &'a [&'a str],
    ) -> InstrSeq<'a> {
        instr(
            alloc,
            Instruct::ILitConst(InstructLitConst::NewRecord(id, keys)),
        )
    }

    pub fn newstructdict<'a>(alloc: &'a bumpalo::Bump, keys: &'a [&'a str]) -> InstrSeq<'a> {
        instr(
            alloc,
            Instruct::ILitConst(InstructLitConst::NewStructDict(keys)),
        )
    }

    pub fn newcol<'a>(alloc: &'a bumpalo::Bump, collection_type: CollectionType) -> InstrSeq<'a> {
        instr(
            alloc,
            Instruct::ILitConst(InstructLitConst::NewCol(collection_type)),
        )
    }

    pub fn colfromarray<'a>(
        alloc: &'a bumpalo::Bump,
        collection_type: CollectionType,
    ) -> InstrSeq<'a> {
        instr(
            alloc,
            Instruct::ILitConst(InstructLitConst::ColFromArray(collection_type)),
        )
    }

    pub fn entrynop<'a>(alloc: &'a bumpalo::Bump) -> InstrSeq<'a> {
        instr(alloc, Instruct::IBasic(InstructBasic::EntryNop))
    }

    pub fn typedvalue<'a>(alloc: &'a bumpalo::Bump, xs: TypedValue<'a>) -> InstrSeq<'a> {
        instr(alloc, Instruct::ILitConst(InstructLitConst::TypedValue(xs)))
    }

    pub fn basel<'a>(
        alloc: &'a bumpalo::Bump,
        local: local::Type<'a>,
        mode: MemberOpMode,
    ) -> InstrSeq<'a> {
        instr(alloc, Instruct::IBase(InstructBase::BaseL(local, mode)))
    }

    pub fn basec<'a>(
        alloc: &'a bumpalo::Bump,
        stack_index: StackIndex,
        mode: MemberOpMode,
    ) -> InstrSeq<'a> {
        instr(
            alloc,
            Instruct::IBase(InstructBase::BaseC(stack_index, mode)),
        )
    }

    pub fn basegc<'a>(
        alloc: &'a bumpalo::Bump,
        stack_index: StackIndex,
        mode: MemberOpMode,
    ) -> InstrSeq<'a> {
        instr(
            alloc,
            Instruct::IBase(InstructBase::BaseGC(stack_index, mode)),
        )
    }

    pub fn basegl<'a>(
        alloc: &'a bumpalo::Bump,
        local: local::Type<'a>,
        mode: MemberOpMode,
    ) -> InstrSeq<'a> {
        instr(alloc, Instruct::IBase(InstructBase::BaseGL(local, mode)))
    }

    pub fn basesc<'a>(
        alloc: &'a bumpalo::Bump,
        y: StackIndex,
        z: StackIndex,
        mode: MemberOpMode,
        readonly_op: ReadOnlyOp,
    ) -> InstrSeq<'a> {
        instr(
            alloc,
            Instruct::IBase(InstructBase::BaseSC(y, z, mode, readonly_op)),
        )
    }

    pub fn baseh<'a>(alloc: &'a bumpalo::Bump) -> InstrSeq<'a> {
        instr(alloc, Instruct::IBase(InstructBase::BaseH))
    }

    pub fn cgetcunop<'a>(alloc: &'a bumpalo::Bump) -> InstrSeq<'a> {
        instr(alloc, Instruct::IMisc(InstructMisc::CGetCUNop))
    }

    pub fn ugetcunop<'a>(alloc: &'a bumpalo::Bump) -> InstrSeq<'a> {
        instr(alloc, Instruct::IMisc(InstructMisc::UGetCUNop))
    }

    pub fn memoget<'a>(
        alloc: &'a bumpalo::Bump,
        label: Label<'a>,
        range: Option<(local::Type<'a>, isize)>,
    ) -> InstrSeq<'a> {
        instr(alloc, Instruct::IMisc(InstructMisc::MemoGet(label, range)))
    }

    pub fn memoget_eager<'a>(
        alloc: &'a bumpalo::Bump,
        label1: Label<'a>,
        label2: Label<'a>,
        range: Option<(local::Type<'a>, isize)>,
    ) -> InstrSeq<'a> {
        instr(
            alloc,
            Instruct::IMisc(InstructMisc::MemoGetEager(label1, label2, range)),
        )
    }

    pub fn memoset<'a>(
        alloc: &'a bumpalo::Bump,
        range: Option<(local::Type<'a>, isize)>,
    ) -> InstrSeq<'a> {
        instr(alloc, Instruct::IMisc(InstructMisc::MemoSet(range)))
    }

    pub fn memoset_eager<'a>(
        alloc: &'a bumpalo::Bump,
        range: Option<(local::Type<'a>, isize)>,
    ) -> InstrSeq<'a> {
        instr(alloc, Instruct::IMisc(InstructMisc::MemoSetEager(range)))
    }

    pub fn getmemokeyl<'a>(alloc: &'a bumpalo::Bump, local: local::Type<'a>) -> InstrSeq<'a> {
        instr(alloc, Instruct::IMisc(InstructMisc::GetMemoKeyL(local)))
    }

    pub fn barethis<'a>(alloc: &'a bumpalo::Bump, notice: BareThisOp) -> InstrSeq<'a> {
        instr(alloc, Instruct::IMisc(InstructMisc::BareThis(notice)))
    }

    pub fn checkthis<'a>(alloc: &'a bumpalo::Bump) -> InstrSeq<'a> {
        instr(alloc, Instruct::IMisc(InstructMisc::CheckThis))
    }

    pub fn verify_ret_type_c<'a>(alloc: &'a bumpalo::Bump) -> InstrSeq<'a> {
        instr(alloc, Instruct::IMisc(InstructMisc::VerifyRetTypeC))
    }

    pub fn verify_ret_type_ts<'a>(alloc: &'a bumpalo::Bump) -> InstrSeq<'a> {
        instr(alloc, Instruct::IMisc(InstructMisc::VerifyRetTypeTS))
    }

    pub fn verify_out_type<'a>(alloc: &'a bumpalo::Bump, i: ParamId<'a>) -> InstrSeq<'a> {
        instr(alloc, Instruct::IMisc(InstructMisc::VerifyOutType(i)))
    }

    pub fn verify_param_type<'a>(alloc: &'a bumpalo::Bump, i: ParamId<'a>) -> InstrSeq<'a> {
        instr(alloc, Instruct::IMisc(InstructMisc::VerifyParamType(i)))
    }

    pub fn verify_param_type_ts<'a>(alloc: &'a bumpalo::Bump, i: ParamId<'a>) -> InstrSeq<'a> {
        instr(alloc, Instruct::IMisc(InstructMisc::VerifyParamTypeTS(i)))
    }

    pub fn dim<'a>(alloc: &'a bumpalo::Bump, op: MemberOpMode, key: MemberKey<'a>) -> InstrSeq<'a> {
        instr(alloc, Instruct::IBase(InstructBase::Dim(op, key)))
    }

    pub fn dim_warn_pt<'a>(
        alloc: &'a bumpalo::Bump,
        key: PropId<'a>,
        readonly_op: ReadOnlyOp,
    ) -> InstrSeq<'a> {
        dim(alloc, MemberOpMode::Warn, MemberKey::PT(key, readonly_op))
    }

    pub fn dim_define_pt<'a>(
        alloc: &'a bumpalo::Bump,
        key: PropId<'a>,
        readonly_op: ReadOnlyOp,
    ) -> InstrSeq<'a> {
        dim(alloc, MemberOpMode::Define, MemberKey::PT(key, readonly_op))
    }

    pub fn fcallclsmethod<'a>(
        alloc: &'a bumpalo::Bump,
        is_log_as_dynamic_call: IsLogAsDynamicCallOp,
        fcall_args: FcallArgs<'a>,
    ) -> InstrSeq<'a> {
        instr(
            alloc,
            Instruct::ICall(InstructCall::FCallClsMethod(
                fcall_args,
                is_log_as_dynamic_call,
            )),
        )
    }

    pub fn fcallclsmethodd<'a>(
        alloc: &'a bumpalo::Bump,
        fcall_args: FcallArgs<'a>,
        method_name: MethodId<'a>,
        class_name: ClassId<'a>,
    ) -> InstrSeq<'a> {
        instr(
            alloc,
            Instruct::ICall(InstructCall::FCallClsMethodD(
                fcall_args,
                class_name,
                method_name,
            )),
        )
    }

    pub fn fcallclsmethods<'a>(
        alloc: &'a bumpalo::Bump,
        fcall_args: FcallArgs<'a>,
        scref: SpecialClsRef,
    ) -> InstrSeq<'a> {
        instr(
            alloc,
            Instruct::ICall(InstructCall::FCallClsMethodS(fcall_args, scref)),
        )
    }

    pub fn fcallclsmethodsd<'a>(
        alloc: &'a bumpalo::Bump,
        fcall_args: FcallArgs<'a>,
        scref: SpecialClsRef,
        method_name: MethodId<'a>,
    ) -> InstrSeq<'a> {
        instr(
            alloc,
            Instruct::ICall(InstructCall::FCallClsMethodSD(
                fcall_args,
                scref,
                method_name,
            )),
        )
    }

    pub fn fcallctor<'a>(alloc: &'a bumpalo::Bump, fcall_args: FcallArgs<'a>) -> InstrSeq<'a> {
        instr(alloc, Instruct::ICall(InstructCall::FCallCtor(fcall_args)))
    }

    pub fn fcallfunc<'a>(alloc: &'a bumpalo::Bump, fcall_args: FcallArgs<'a>) -> InstrSeq<'a> {
        instr(alloc, Instruct::ICall(InstructCall::FCallFunc(fcall_args)))
    }

    pub fn fcallfuncd<'a>(
        alloc: &'a bumpalo::Bump,
        fcall_args: FcallArgs<'a>,
        id: FunctionId<'a>,
    ) -> InstrSeq<'a> {
        instr(
            alloc,
            Instruct::ICall(InstructCall::FCallFuncD(fcall_args, id)),
        )
    }

    pub fn fcallobjmethod<'a>(
        alloc: &'a bumpalo::Bump,
        fcall_args: FcallArgs<'a>,
        flavor: ObjNullFlavor,
    ) -> InstrSeq<'a> {
        instr(
            alloc,
            Instruct::ICall(InstructCall::FCallObjMethod(fcall_args, flavor)),
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
            Instruct::ICall(InstructCall::FCallObjMethodD(fcall_args, flavor, method)),
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
        op: QueryOp,
        key: MemberKey<'a>,
    ) -> InstrSeq<'a> {
        instr(
            alloc,
            Instruct::IFinal(InstructFinal::QueryM(num_params, op, key)),
        )
    }

    pub fn querym_cget_pt<'a>(
        alloc: &'a bumpalo::Bump,
        num_params: NumParams,
        key: PropId<'a>,
        readonly_op: ReadOnlyOp,
    ) -> InstrSeq<'a> {
        querym(
            alloc,
            num_params,
            QueryOp::CGet,
            MemberKey::PT(key, readonly_op),
        )
    }

    pub fn setm<'a>(
        alloc: &'a bumpalo::Bump,
        num_params: NumParams,
        key: MemberKey<'a>,
    ) -> InstrSeq<'a> {
        instr(
            alloc,
            Instruct::IFinal(InstructFinal::SetM(num_params, key)),
        )
    }

    pub fn unsetm<'a>(
        alloc: &'a bumpalo::Bump,
        num_params: NumParams,
        key: MemberKey<'a>,
    ) -> InstrSeq<'a> {
        instr(
            alloc,
            Instruct::IFinal(InstructFinal::UnsetM(num_params, key)),
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
            Instruct::IFinal(InstructFinal::SetOpM(num_params, op, key)),
        )
    }

    pub fn incdecm<'a>(
        alloc: &'a bumpalo::Bump,
        num_params: NumParams,
        op: IncdecOp,
        key: MemberKey<'a>,
    ) -> InstrSeq<'a> {
        instr(
            alloc,
            Instruct::IFinal(InstructFinal::IncDecM(num_params, op, key)),
        )
    }

    pub fn setm_pt<'a>(
        alloc: &'a bumpalo::Bump,
        num_params: NumParams,
        key: PropId<'a>,
        readonly_op: ReadOnlyOp,
    ) -> InstrSeq<'a> {
        setm(alloc, num_params, MemberKey::PT(key, readonly_op))
    }

    pub fn resolve_func<'a>(alloc: &'a bumpalo::Bump, func_id: FunctionId<'a>) -> InstrSeq<'a> {
        instr(alloc, Instruct::IOp(InstructOperator::ResolveFunc(func_id)))
    }

    pub fn resolve_rfunc<'a>(alloc: &'a bumpalo::Bump, func_id: FunctionId<'a>) -> InstrSeq<'a> {
        instr(
            alloc,
            Instruct::IOp(InstructOperator::ResolveRFunc(func_id)),
        )
    }

    pub fn resolve_obj_method<'a>(alloc: &'a bumpalo::Bump) -> InstrSeq<'a> {
        instr(alloc, Instruct::IOp(InstructOperator::ResolveObjMethod))
    }

    pub fn resolveclsmethod<'a>(alloc: &'a bumpalo::Bump, method_id: MethodId<'a>) -> InstrSeq<'a> {
        instr(
            alloc,
            Instruct::IOp(InstructOperator::ResolveClsMethod(method_id)),
        )
    }

    pub fn resolveclsmethodd<'a>(
        alloc: &'a bumpalo::Bump,
        class_id: ClassId<'a>,
        method_id: MethodId<'a>,
    ) -> InstrSeq<'a> {
        instr(
            alloc,
            Instruct::IOp(InstructOperator::ResolveClsMethodD(class_id, method_id)),
        )
    }

    pub fn resolveclsmethods<'a>(
        alloc: &'a bumpalo::Bump,
        scref: SpecialClsRef,
        method_id: MethodId<'a>,
    ) -> InstrSeq<'a> {
        instr(
            alloc,
            Instruct::IOp(InstructOperator::ResolveClsMethodS(scref, method_id)),
        )
    }

    pub fn resolverclsmethod<'a>(
        alloc: &'a bumpalo::Bump,
        method_id: MethodId<'a>,
    ) -> InstrSeq<'a> {
        instr(
            alloc,
            Instruct::IOp(InstructOperator::ResolveRClsMethod(method_id)),
        )
    }

    pub fn resolverclsmethodd<'a>(
        alloc: &'a bumpalo::Bump,
        class_id: ClassId<'a>,
        method_id: MethodId<'a>,
    ) -> InstrSeq<'a> {
        instr(
            alloc,
            Instruct::IOp(InstructOperator::ResolveRClsMethodD(class_id, method_id)),
        )
    }

    pub fn resolverclsmethods<'a>(
        alloc: &'a bumpalo::Bump,
        scref: SpecialClsRef,
        method_id: MethodId<'a>,
    ) -> InstrSeq<'a> {
        instr(
            alloc,
            Instruct::IOp(InstructOperator::ResolveRClsMethodS(scref, method_id)),
        )
    }

    pub fn resolve_meth_caller<'a>(
        alloc: &'a bumpalo::Bump,
        fun_id: FunctionId<'a>,
    ) -> InstrSeq<'a> {
        instr(
            alloc,
            Instruct::IOp(InstructOperator::ResolveMethCaller(fun_id)),
        )
    }

    pub fn resolveclass<'a>(alloc: &'a bumpalo::Bump, class_id: ClassId<'a>) -> InstrSeq<'a> {
        instr(
            alloc,
            Instruct::IOp(InstructOperator::ResolveClass(class_id)),
        )
    }

    pub fn lazyclass<'a>(alloc: &'a bumpalo::Bump, class_id: ClassId<'a>) -> InstrSeq<'a> {
        instr(
            alloc,
            Instruct::ILitConst(InstructLitConst::LazyClass(class_id)),
        )
    }

    pub fn oodeclexists<'a>(alloc: &'a bumpalo::Bump, class_kind: ClassKind) -> InstrSeq<'a> {
        instr(
            alloc,
            Instruct::IMisc(InstructMisc::OODeclExists(class_kind)),
        )
    }

    pub fn fatal<'a>(alloc: &'a bumpalo::Bump, op: FatalOp) -> InstrSeq<'a> {
        instr(alloc, Instruct::IOp(InstructOperator::Fatal(op)))
    }

    pub fn await_<'a>(alloc: &'a bumpalo::Bump) -> InstrSeq<'a> {
        instr(alloc, Instruct::IAsync(AsyncFunctions::Await))
    }

    pub fn yield_<'a>(alloc: &'a bumpalo::Bump) -> InstrSeq<'a> {
        instr(alloc, Instruct::IGenerator(GenCreationExecution::Yield))
    }

    pub fn yieldk<'a>(alloc: &'a bumpalo::Bump) -> InstrSeq<'a> {
        instr(alloc, Instruct::IGenerator(GenCreationExecution::YieldK))
    }

    pub fn createcont<'a>(alloc: &'a bumpalo::Bump) -> InstrSeq<'a> {
        instr(
            alloc,
            Instruct::IGenerator(GenCreationExecution::CreateCont),
        )
    }

    pub fn awaitall<'a>(
        alloc: &'a bumpalo::Bump,
        range: Option<(local::Type<'a>, isize)>,
    ) -> InstrSeq<'a> {
        instr(alloc, Instruct::IAsync(AsyncFunctions::AwaitAll(range)))
    }

    pub fn label<'a>(alloc: &'a bumpalo::Bump, label: Label<'a>) -> InstrSeq<'a> {
        instr(alloc, Instruct::ILabel(label))
    }

    pub fn awaitall_list<'a>(
        alloc: &'a bumpalo::Bump,
        unnamed_locals: std::vec::Vec<local::Type<'a>>,
    ) -> InstrSeq<'a> {
        use local::Type::Unnamed;
        use std::convert::TryInto;
        match unnamed_locals.split_first() {
            None => panic!("Expected at least one await"),
            Some((hd, tl)) => {
                if let Unnamed(hd_id) = hd {
                    let mut prev_id = hd_id;
                    for unnamed_local in tl.iter() {
                        match unnamed_local {
                            Unnamed(id) => {
                                assert_eq!(*prev_id + 1, *id);
                                prev_id = id;
                            }
                            _ => panic!("Expected unnamed local"),
                        }
                    }
                    awaitall(
                        alloc,
                        Some((Unnamed(*hd_id), unnamed_locals.len().try_into().unwrap())),
                    )
                } else {
                    panic!("Expected unnamed local")
                }
            }
        }
    }

    pub fn exit<'a>(alloc: &'a bumpalo::Bump) -> InstrSeq<'a> {
        instr(alloc, Instruct::IOp(InstructOperator::Exit))
    }

    pub fn idx<'a>(alloc: &'a bumpalo::Bump) -> InstrSeq<'a> {
        instr(alloc, Instruct::IMisc(InstructMisc::Idx))
    }

    pub fn array_idx<'a>(alloc: &'a bumpalo::Bump) -> InstrSeq<'a> {
        instr(alloc, Instruct::IMisc(InstructMisc::ArrayIdx))
    }

    pub fn createcl<'a>(
        alloc: &'a bumpalo::Bump,
        param_num: NumParams,
        cls_num: ClassNum,
    ) -> InstrSeq<'a> {
        instr(
            alloc,
            Instruct::IMisc(InstructMisc::CreateCl(param_num, cls_num)),
        )
    }

    pub fn eval<'a>(alloc: &'a bumpalo::Bump) -> InstrSeq<'a> {
        instr(
            alloc,
            Instruct::IIncludeEvalDefine(InstructIncludeEvalDefine::Eval),
        )
    }

    pub fn incl<'a>(alloc: &'a bumpalo::Bump) -> InstrSeq<'a> {
        instr(
            alloc,
            Instruct::IIncludeEvalDefine(InstructIncludeEvalDefine::Incl),
        )
    }

    pub fn inclonce<'a>(alloc: &'a bumpalo::Bump) -> InstrSeq<'a> {
        instr(
            alloc,
            Instruct::IIncludeEvalDefine(InstructIncludeEvalDefine::InclOnce),
        )
    }

    pub fn req<'a>(alloc: &'a bumpalo::Bump) -> InstrSeq<'a> {
        instr(
            alloc,
            Instruct::IIncludeEvalDefine(InstructIncludeEvalDefine::Req),
        )
    }

    pub fn reqdoc<'a>(alloc: &'a bumpalo::Bump) -> InstrSeq<'a> {
        instr(
            alloc,
            Instruct::IIncludeEvalDefine(InstructIncludeEvalDefine::ReqDoc),
        )
    }

    pub fn reqonce<'a>(alloc: &'a bumpalo::Bump) -> InstrSeq<'a> {
        instr(
            alloc,
            Instruct::IIncludeEvalDefine(InstructIncludeEvalDefine::ReqOnce),
        )
    }

    pub fn silence_start<'a>(alloc: &'a bumpalo::Bump, local: local::Type<'a>) -> InstrSeq<'a> {
        instr(
            alloc,
            Instruct::IMisc(InstructMisc::Silence(local, OpSilence::Start)),
        )
    }

    pub fn silence_end<'a>(alloc: &'a bumpalo::Bump, local: local::Type<'a>) -> InstrSeq<'a> {
        instr(
            alloc,
            Instruct::IMisc(InstructMisc::Silence(local, OpSilence::End)),
        )
    }

    pub fn contcheck_check<'a>(alloc: &'a bumpalo::Bump) -> InstrSeq<'a> {
        instr(
            alloc,
            Instruct::IGenerator(GenCreationExecution::ContCheck(CheckStarted::CheckStarted)),
        )
    }

    pub fn contcheck_ignore<'a>(alloc: &'a bumpalo::Bump) -> InstrSeq<'a> {
        instr(
            alloc,
            Instruct::IGenerator(GenCreationExecution::ContCheck(CheckStarted::IgnoreStarted)),
        )
    }

    pub fn contenter<'a>(alloc: &'a bumpalo::Bump) -> InstrSeq<'a> {
        instr(alloc, Instruct::IGenerator(GenCreationExecution::ContEnter))
    }

    pub fn contraise<'a>(alloc: &'a bumpalo::Bump) -> InstrSeq<'a> {
        instr(alloc, Instruct::IGenerator(GenCreationExecution::ContRaise))
    }

    pub fn contvalid<'a>(alloc: &'a bumpalo::Bump) -> InstrSeq<'a> {
        instr(alloc, Instruct::IGenerator(GenCreationExecution::ContValid))
    }

    pub fn contcurrent<'a>(alloc: &'a bumpalo::Bump) -> InstrSeq<'a> {
        instr(
            alloc,
            Instruct::IGenerator(GenCreationExecution::ContCurrent),
        )
    }

    pub fn contkey<'a>(alloc: &'a bumpalo::Bump) -> InstrSeq<'a> {
        instr(alloc, Instruct::IGenerator(GenCreationExecution::ContKey))
    }

    pub fn contgetreturn<'a>(alloc: &'a bumpalo::Bump) -> InstrSeq<'a> {
        instr(
            alloc,
            Instruct::IGenerator(GenCreationExecution::ContGetReturn),
        )
    }

    pub fn nativeimpl<'a>(alloc: &'a bumpalo::Bump) -> InstrSeq<'a> {
        instr(alloc, Instruct::IMisc(InstructMisc::NativeImpl))
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
            Instruct::ISrcLoc(Srcloc {
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
            Instruct::IOp(InstructOperator::IsTypeStructC(
                TypestructResolveOp::Resolve,
            )),
        )
    }

    pub fn is_type_structc_dontresolve<'a>(alloc: &'a bumpalo::Bump) -> InstrSeq<'a> {
        instr(
            alloc,
            Instruct::IOp(InstructOperator::IsTypeStructC(
                TypestructResolveOp::DontResolve,
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
        InstrSeq::List(bumpalo::vec![in &alloc; ].into_bump_slice_mut())
    }

    /// An instruction sequence of a single instruction.
    pub fn new_singleton(alloc: &'a bumpalo::Bump, i: Instruct<'a>) -> InstrSeq<'a> {
        InstrSeq::List(bumpalo::vec![in &alloc; i].into_bump_slice_mut())
    }

    /// An instruction sequence of a sequence of instructions.
    pub fn new_list(is: &'a mut [Instruct<'a>]) -> InstrSeq<'a> {
        InstrSeq::List(is)
    }

    /// An instruction sequence of a concatenation of instruction sequences.
    pub fn new_concat(iss: &'a mut [InstrSeq<'a>]) -> InstrSeq<'a> {
        InstrSeq::Concat(iss)
    }

    /// Move instructions out of a container.
    pub fn from_iter_in<T: std::iter::IntoIterator<Item = Instruct<'a>>>(
        alloc: &'a bumpalo::Bump,
        it: T,
    ) -> InstrSeq<'a> {
        InstrSeq::new_list(bumpalo::collections::Vec::from_iter_in(it, alloc).into_bump_slice_mut())
    }

    /// Transitional version. We mean to write a `gather!` in the future.
    pub fn gather(alloc: &'a bumpalo::Bump, iss: std::vec::Vec<InstrSeq<'a>>) -> InstrSeq<'a> {
        let non_empty = bumpalo::collections::Vec::from_iter_in(
            iss.into_iter()
                .filter(|is| !matches!(is, InstrSeq::List(&mut []))),
            alloc,
        );
        if non_empty.is_empty() {
            InstrSeq::new_empty(alloc)
        } else {
            InstrSeq::new_concat(non_empty.into_bump_slice_mut())
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
        label_gen: &mut hhbc_by_ref_label::Gen,
        opt_done_label: Option<Label<'a>>,
        skip_throw: bool,
        try_instrs: Self,
        catch_instrs: Self,
    ) -> Self {
        let done_label = match opt_done_label {
            Some(l) => l,
            None => label_gen.next_regular(alloc),
        };
        InstrSeq::gather(
            alloc,
            vec![
                instr::instr(alloc, Instruct::ITry(InstructTry::TryCatchBegin)),
                try_instrs,
                instr::jmp(alloc, done_label.clone()),
                instr::instr(alloc, Instruct::ITry(InstructTry::TryCatchMiddle)),
                catch_instrs,
                if skip_throw {
                    instr::empty(alloc)
                } else {
                    instr::instr(alloc, Instruct::IContFlow(InstructControlFlow::Throw))
                },
                instr::instr(alloc, Instruct::ITry(InstructTry::TryCatchEnd)),
                instr::label(alloc, done_label),
            ],
        )
    }

    fn get_or_put_label<'i, 'm>(
        alloc: &'a bumpalo::Bump,
        label_gen: &mut hhbc_by_ref_label::Gen,
        name_label_map: &'m mut hash::HashMap<std::string::String, Label<'a>>,
        name: &'i str,
    ) -> Label<'a> {
        match name_label_map.get(name) {
            Some(label) => label.clone(),
            None => {
                let l = label_gen.next_regular(alloc);
                name_label_map.insert(name.to_string(), l.clone());
                l
            }
        }
    }

    fn rewrite_user_labels_instr<'i, 'm>(
        alloc: &'a bumpalo::Bump,
        label_gen: &mut hhbc_by_ref_label::Gen,
        i: &'i mut Instruct<'a>,
        name_label_map: &'m mut hash::HashMap<std::string::String, Label<'a>>,
    ) {
        use Instruct::*;
        let mut get_result = |x| InstrSeq::get_or_put_label(alloc, label_gen, name_label_map, x);
        match i {
            IContFlow(InstructControlFlow::Jmp(Label::Named(name))) => {
                let label = get_result(name);
                *i = Instruct::IContFlow(InstructControlFlow::Jmp(label));
            }
            IContFlow(InstructControlFlow::JmpNS(Label::Named(name))) => {
                let label = get_result(name);
                *i = Instruct::IContFlow(InstructControlFlow::JmpNS(label));
            }
            IContFlow(InstructControlFlow::JmpZ(Label::Named(name))) => {
                let label = get_result(name);
                *i = Instruct::IContFlow(InstructControlFlow::JmpZ(label));
            }
            IContFlow(InstructControlFlow::JmpNZ(Label::Named(name))) => {
                let label = get_result(name);
                *i = Instruct::IContFlow(InstructControlFlow::JmpNZ(label));
            }
            ILabel(Label::Named(name)) => {
                let label = get_result(name);
                *i = ILabel(label);
            }
            _ => {}
        };
    }

    pub fn rewrite_user_labels(
        &mut self,
        alloc: &'a bumpalo::Bump,
        label_gen: &mut hhbc_by_ref_label::Gen,
    ) {
        let name_label_map = &mut hash::HashMap::default();
        self.map_mut(&mut |i| {
            InstrSeq::rewrite_user_labels_instr(alloc, label_gen, i, name_label_map)
        });
    }

    /// Test whether `i` is of case `Instruct::ISrcLoc`.
    fn is_srcloc(instruction: &Instruct<'a>) -> bool {
        match instruction {
            Instruct::ISrcLoc(_) => true,
            _ => false,
        }
    }

    pub fn first(&self) -> Option<&Instruct<'a>> {
        match self {
            InstrSeq::List(&mut []) => None,
            InstrSeq::List(&mut [ref i]) => {
                if InstrSeq::is_srcloc(i) {
                    None
                } else {
                    Some(i)
                }
            }
            InstrSeq::List(l) => match l.iter().find(|&i| !InstrSeq::is_srcloc(i)) {
                Some(i) => Some(i),
                None => None,
            },
            InstrSeq::Concat(l) => l.iter().find_map(InstrSeq::first),
        }
    }

    /// Test for the empty instruction sequence.
    pub fn is_empty(&self) -> bool {
        match self {
            InstrSeq::List(&mut []) => true,
            InstrSeq::List(&mut [ref i]) => InstrSeq::is_srcloc(i),
            InstrSeq::List(l) => l.is_empty() || l.iter().all(InstrSeq::is_srcloc),
            InstrSeq::Concat(l) => l.iter().all(InstrSeq::is_empty),
        }
    }

    pub fn flat_map_seq<F>(&self, alloc: &'a bumpalo::Bump, f: &mut F) -> Self
    where
        F: FnMut(&Instruct<'a>) -> Self,
    {
        match self {
            InstrSeq::List(&mut []) => InstrSeq::new_empty(alloc),
            InstrSeq::List(&mut [ref instr]) => f(instr),
            InstrSeq::List(instr_lst) => InstrSeq::Concat(
                bumpalo::collections::vec::Vec::from_iter_in(instr_lst.iter().map(|x| f(x)), alloc)
                    .into_bump_slice_mut(),
            ),
            InstrSeq::Concat(instrseq_lst) => InstrSeq::Concat(
                bumpalo::collections::vec::Vec::from_iter_in(
                    instrseq_lst.iter().map(|x| x.flat_map_seq(alloc, f)),
                    alloc,
                )
                .into_bump_slice_mut(),
            ),
        }
    }

    pub fn fold_left<'i, F, A>(&'i self, f: &mut F, init: A) -> A
    where
        F: FnMut(A, &'i Instruct<'a>) -> A,
    {
        match self {
            InstrSeq::List(&mut []) => init,
            InstrSeq::List(&mut [ref x]) => f(init, x),
            InstrSeq::List(instr_lst) => instr_lst.iter().fold(init, f),
            InstrSeq::Concat(instrseq_lst) => {
                instrseq_lst.iter().fold(init, |acc, x| x.fold_left(f, acc))
            }
        }
    }

    pub fn filter_map<F>(&self, alloc: &'a bumpalo::Bump, f: &mut F) -> Self
    where
        F: FnMut(&Instruct<'a>) -> Option<Instruct<'a>>,
    {
        match self {
            InstrSeq::List(&mut []) => InstrSeq::new_empty(alloc),
            InstrSeq::List(&mut [ref x]) => match f(x) {
                Some(x) => instr::instr(alloc, x),
                None => InstrSeq::new_empty(alloc),
            },
            InstrSeq::List(instr_lst) => InstrSeq::List(
                bumpalo::collections::vec::Vec::from_iter_in(instr_lst.iter().filter_map(f), alloc)
                    .into_bump_slice_mut(),
            ),
            InstrSeq::Concat(instrseq_lst) => InstrSeq::Concat(
                bumpalo::collections::vec::Vec::from_iter_in(
                    instrseq_lst.iter().map(|x| x.filter_map(alloc, f)),
                    alloc,
                )
                .into_bump_slice_mut(),
            ),
        }
    }

    pub fn filter_map_mut<F>(&mut self, alloc: &'a bumpalo::Bump, f: &mut F)
    where
        F: FnMut(&mut Instruct<'a>) -> bool,
    {
        match self {
            InstrSeq::List(&mut []) => {}
            InstrSeq::List(&mut [ref mut x]) => {
                if !f(x) {
                    *self = instr::empty(alloc)
                }
            }
            InstrSeq::List(ref mut instr_lst) => {
                let mut new_lst = bumpalo::vec![in alloc;];
                for mut i in instr_lst.iter_mut() {
                    if f(&mut i) {
                        new_lst.push(i.clone())
                    }
                }
                *self = instr::instrs(alloc, new_lst.into_bump_slice_mut())
            }
            InstrSeq::Concat(instrseq_lst) => instrseq_lst
                .iter_mut()
                .for_each(|x| x.filter_map_mut(alloc, f)),
        }
    }

    pub fn map_mut<'i, F>(&'i mut self, f: &mut F)
    where
        F: FnMut(&'i mut Instruct<'a>),
    {
        match self {
            InstrSeq::List(&mut []) => {}
            InstrSeq::List(&mut [ref mut x]) => f(x),
            InstrSeq::List(instr_lst) => instr_lst.iter_mut().for_each(f),
            InstrSeq::Concat(instrseq_lst) => instrseq_lst.iter_mut().for_each(|x| x.map_mut(f)),
        }
    }

    #[allow(clippy::result_unit_err)]
    pub fn map_result_mut<F>(&mut self, f: &mut F) -> Result<()>
    where
        F: FnMut(&mut Instruct<'a>) -> Result<()>,
    {
        match self {
            InstrSeq::List(&mut []) => Ok(()),
            InstrSeq::List(&mut [ref mut x]) => f(x),
            InstrSeq::List(instr_lst) => instr_lst.iter_mut().try_for_each(|x| f(x)),
            InstrSeq::Concat(instrseq_lst) => instrseq_lst
                .iter_mut()
                .try_for_each(|x| x.map_result_mut(f)),
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
        let mk_i = || {
            Instruct::IComment(bumpalo::collections::String::from_str_in("", alloc).into_bump_str())
        };
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
        let concat0 = || InstrSeq::Concat(bumpalo::vec![in alloc;].into_bump_slice_mut());
        let concat1 = || InstrSeq::Concat(bumpalo::vec![in alloc; one()].into_bump_slice_mut());

        assert_eq!(empty().iter().count(), 0);
        assert_eq!(one().iter().count(), 1);
        assert_eq!(list0().iter().count(), 0);
        assert_eq!(list1().iter().count(), 1);
        assert_eq!(list2().iter().count(), 2);
        assert_eq!(concat0().iter().count(), 0);
        assert_eq!(concat1().iter().count(), 1);

        let concat = InstrSeq::Concat(bumpalo::vec![in alloc; empty()].into_bump_slice_mut());
        assert_eq!(concat.iter().count(), 0);

        let concat =
            InstrSeq::Concat(bumpalo::vec![in alloc; empty(), one()].into_bump_slice_mut());
        assert_eq!(concat.iter().count(), 1);

        let concat =
            InstrSeq::Concat(bumpalo::vec![in alloc; one(), empty()].into_bump_slice_mut());
        assert_eq!(concat.iter().count(), 1);

        let concat =
            InstrSeq::Concat(bumpalo::vec![in alloc; one(), list1()].into_bump_slice_mut());
        assert_eq!(concat.iter().count(), 2);

        let concat =
            InstrSeq::Concat(bumpalo::vec![in alloc; list2(), list1()].into_bump_slice_mut());
        assert_eq!(concat.iter().count(), 3);

        let concat = InstrSeq::Concat(
            bumpalo::vec![in alloc; concat0(), list2(), list1()].into_bump_slice_mut(),
        );
        assert_eq!(concat.iter().count(), 3);

        let concat =
            InstrSeq::Concat(bumpalo::vec![in alloc; concat1(), concat1()].into_bump_slice_mut());
        assert_eq!(concat.iter().count(), 2);

        let concat =
            InstrSeq::Concat(bumpalo::vec![in alloc; concat0(), concat1()].into_bump_slice_mut());
        assert_eq!(concat.iter().count(), 1);

        let concat =
            InstrSeq::Concat(bumpalo::vec![in alloc; list2(), concat1()].into_bump_slice_mut());
        assert_eq!(concat.iter().count(), 3);

        let concat =
            InstrSeq::Concat(bumpalo::vec![in alloc; list2(), concat0()].into_bump_slice_mut());
        assert_eq!(concat.iter().count(), 2);

        let concat =
            InstrSeq::Concat(bumpalo::vec![in alloc; one(), concat0()].into_bump_slice_mut());
        assert_eq!(concat.iter().count(), 1);

        let concat =
            InstrSeq::Concat(bumpalo::vec![in alloc; empty(), concat0()].into_bump_slice_mut());
        assert_eq!(concat.iter().count(), 0);
    }
}
