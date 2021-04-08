// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use hhbc_ast_rust::*;
use iterator::Id as IterId;
use itertools::Either;
use label_rust as label;
use label_rust::Label;
use oxidized::pos::Pos;
use runtime::TypedValue;
use thiserror::Error;

use std::{collections::HashMap, convert::TryInto};

pub type Result<T = InstrSeq> = std::result::Result<T, Error>;

#[derive(Error, Debug)]
pub enum Error {
    #[error("IncludeTimeFatalException: FatalOp={0:?}, {1}")]
    IncludeTimeFatalException(FatalOp, Pos, String),

    #[error("Unrecoverable: {0}")]
    Unrecoverable(String),
}

pub fn unrecoverable(msg: impl Into<String>) -> Error {
    Error::Unrecoverable(msg.into())
}

/// The various from_X functions below take some kind of AST (expression,
/// statement, etc.) and produce what is logically a sequence of instructions.
/// This could simply be represented by a list, but then we would need to
/// use an accumulator to avoid the quadratic complexity associated with
/// repeated appending to a list. Instead, we simply build a tree of
/// instructions which can easily be flattened at the end.
#[derive(Clone, Debug)]
pub enum InstrSeq {
    Empty,
    One(Box<Instruct>),
    List(Vec<Instruct>),
    Concat(Vec<InstrSeq>),
}

impl Default for InstrSeq {
    fn default() -> InstrSeq {
        InstrSeq::Empty
    }
}

impl From<(InstrSeq, InstrSeq)> for InstrSeq {
    fn from((i1, i2): (InstrSeq, InstrSeq)) -> InstrSeq {
        InstrSeq::gather(vec![i1, i2])
    }
}

#[derive(Debug)]
pub struct CompactIter<'i, I>
where
    I: Iterator<Item = &'i Instruct>,
{
    iter: I,
    next: Option<&'i Instruct>,
}

impl<'i, I> CompactIter<'i, I>
where
    I: Iterator<Item = &'i Instruct>,
{
    pub fn new(i: I) -> Self {
        Self {
            iter: i,
            next: None,
        }
    }
}

impl<'i, I> Iterator for CompactIter<'i, I>
where
    I: Iterator<Item = &'i Instruct>,
{
    type Item = &'i Instruct;
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
pub struct InstrIterByValue {
    stack: Vec<(InstrSeq, usize)>,
}

impl IntoIterator for InstrSeq {
    type Item = Instruct;
    type IntoIter = InstrIterByValue;

    fn into_iter(self) -> Self::IntoIter {
        InstrIterByValue {
            stack: vec![(self, 0)],
        }
    }
}

impl Iterator for InstrIterByValue {
    type Item = Instruct;

    /// Iterate an `InstrSeq` as a flat iterable of `Instruct` values.
    ///
    /// Uses constant stack space by avoiding recursion. Avoids
    /// copying and minimises pushing/popping, because this function
    /// is hot in large Hack functions.
    fn next(&mut self) -> Option<Self::Item> {
        while let Some((ref mut instr_seq, ref mut idx)) = self.stack.last_mut() {
            match instr_seq {
                InstrSeq::Empty => {
                    self.stack.pop();
                }
                InstrSeq::One(_) => {
                    if let Some((InstrSeq::One(i), _)) = self.stack.pop() {
                        return Some(*i);
                    }
                }
                InstrSeq::List(ref mut instrs) => {
                    if *idx < instrs.len() {
                        let instr = std::mem::replace(
                            &mut instrs[*idx],
                            Instruct::IBasic(InstructBasic::Nop),
                        );
                        *idx += 1;
                        return Some(instr);
                    } else {
                        self.stack.pop();
                    }
                }
                InstrSeq::Concat(ref mut instr_seqs) => {
                    if *idx < instr_seqs.len() {
                        let next_instr_seq =
                            std::mem::replace(&mut instr_seqs[*idx], InstrSeq::Empty);
                        *idx += 1;

                        self.stack.push((next_instr_seq, 0));
                    } else {
                        self.stack.pop();
                    }
                }
            }
        }
        None
    }
}

#[derive(Debug)]
pub struct InstrIter<'i> {
    instr_seq: &'i InstrSeq,
    index: usize,
    concat_stack: Vec<Either<(&'i Vec<Instruct>, usize), (&'i Vec<InstrSeq>, usize)>>,
}

impl<'i> InstrIter<'i> {
    pub fn new(instr_seq: &'i InstrSeq) -> Self {
        Self {
            instr_seq,
            index: 0,
            concat_stack: vec![],
        }
    }
}

pub fn flatten(instrs: InstrSeq) -> InstrSeq {
    InstrSeq::List(instrs.into_iter().collect())
}

impl<'i> Iterator for InstrIter<'i> {
    type Item = &'i Instruct;
    fn next(&mut self) -> Option<Self::Item> {
        match self.instr_seq {
            InstrSeq::Empty => None,
            InstrSeq::One(_) if self.index > 0 => None,
            InstrSeq::One(i) => {
                self.index += 1;
                Some(i)
            }
            InstrSeq::List(ii) if self.index >= ii.len() => None,
            InstrSeq::List(ii) => {
                let r = ii.get(self.index);
                self.index += 1;
                r
            }
            InstrSeq::Concat(cc) => {
                if self.concat_stack.is_empty() {
                    if self.index == 0 {
                        self.index += 1;
                        self.concat_stack.push(Either::Right((cc, 0)));
                    } else {
                        return None;
                    }
                }

                while !self.concat_stack.is_empty() {
                    let top = self.concat_stack.last_mut().unwrap();
                    match top {
                        Either::Left((list, size)) if *size >= list.len() => {
                            self.concat_stack.pop();
                        }
                        Either::Left((list, size)) => {
                            let r: &Instruct = &(list[*size]);
                            *size += 1;
                            return Some(&r);
                        }
                        Either::Right((concat, size)) if *size >= concat.len() => {
                            self.concat_stack.pop();
                        }
                        Either::Right((concat, size)) => {
                            let i: &InstrSeq = &(concat[*size]);
                            *size += 1;
                            match i {
                                InstrSeq::One(ref instr) => {
                                    return Some(instr);
                                }
                                InstrSeq::List(ref list) => {
                                    self.concat_stack.push(Either::Left((list, 0)));
                                }
                                InstrSeq::Concat(ref concat) => {
                                    self.concat_stack.push(Either::Right((concat, 0)));
                                }
                                InstrSeq::Empty => {}
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

    pub fn empty() -> InstrSeq {
        InstrSeq::Empty
    }

    pub fn instr(instruction: Instruct) -> InstrSeq {
        InstrSeq::One(Box::new(instruction))
    }

    pub fn instrs(instructions: Vec<Instruct>) -> InstrSeq {
        InstrSeq::List(instructions)
    }

    pub fn lit_const(l: InstructLitConst) -> InstrSeq {
        instr(Instruct::ILitConst(l))
    }

    pub fn iterinit(args: IterArgs, label: Label) -> InstrSeq {
        instr(Instruct::IIterator(InstructIterator::IterInit(args, label)))
    }

    pub fn iternext(args: IterArgs, label: Label) -> InstrSeq {
        instr(Instruct::IIterator(InstructIterator::IterNext(args, label)))
    }

    pub fn iternextk(id: IterId, label: Label, value: local::Type, key: local::Type) -> InstrSeq {
        let args = IterArgs {
            iter_id: id,
            key_id: Some(key),
            val_id: value,
        };
        instr(Instruct::IIterator(InstructIterator::IterNext(args, label)))
    }

    pub fn iterfree(id: IterId) -> InstrSeq {
        instr(Instruct::IIterator(InstructIterator::IterFree(id)))
    }

    pub fn whresult() -> InstrSeq {
        instr(Instruct::IAsync(AsyncFunctions::WHResult))
    }

    pub fn jmp(label: Label) -> InstrSeq {
        instr(Instruct::IContFlow(InstructControlFlow::Jmp(label)))
    }

    pub fn jmpz(label: Label) -> InstrSeq {
        instr(Instruct::IContFlow(InstructControlFlow::JmpZ(label)))
    }

    pub fn jmpnz(label: Label) -> InstrSeq {
        instr(Instruct::IContFlow(InstructControlFlow::JmpNZ(label)))
    }

    pub fn jmpns(label: Label) -> InstrSeq {
        instr(Instruct::IContFlow(InstructControlFlow::JmpNS(label)))
    }

    pub fn continue_(level: isize) -> InstrSeq {
        instr(Instruct::ISpecialFlow(InstructSpecialFlow::Continue(level)))
    }

    pub fn break_(level: isize) -> InstrSeq {
        instr(Instruct::ISpecialFlow(InstructSpecialFlow::Break(level)))
    }

    pub fn goto(label: String) -> InstrSeq {
        instr(Instruct::ISpecialFlow(InstructSpecialFlow::Goto(label)))
    }

    pub fn iter_break(label: Label, itrs: Vec<IterId>) -> InstrSeq {
        let mut itrs = itrs
            .into_iter()
            .map(|id| Instruct::IIterator(InstructIterator::IterFree(id)))
            .collect::<Vec<_>>();
        itrs.push(Instruct::IContFlow(InstructControlFlow::Jmp(label)));
        instrs(itrs)
    }

    pub fn false_() -> InstrSeq {
        instr(Instruct::ILitConst(InstructLitConst::False))
    }

    pub fn true_() -> InstrSeq {
        instr(Instruct::ILitConst(InstructLitConst::True))
    }

    pub fn clscnsd(const_id: ConstId, cid: ClassId) -> InstrSeq {
        instr(Instruct::ILitConst(InstructLitConst::ClsCnsD(
            const_id, cid,
        )))
    }

    pub fn clscns(const_id: ConstId) -> InstrSeq {
        instr(Instruct::ILitConst(InstructLitConst::ClsCns(const_id)))
    }

    pub fn clscnsl(local: local::Type) -> InstrSeq {
        instr(Instruct::ILitConst(InstructLitConst::ClsCnsL(local)))
    }

    pub fn eq() -> InstrSeq {
        instr(Instruct::IOp(InstructOperator::Eq))
    }

    pub fn neq() -> InstrSeq {
        instr(Instruct::IOp(InstructOperator::Neq))
    }

    pub fn gt() -> InstrSeq {
        instr(Instruct::IOp(InstructOperator::Gt))
    }

    pub fn gte() -> InstrSeq {
        instr(Instruct::IOp(InstructOperator::Gte))
    }

    pub fn lt() -> InstrSeq {
        instr(Instruct::IOp(InstructOperator::Lt))
    }

    pub fn lte() -> InstrSeq {
        instr(Instruct::IOp(InstructOperator::Lte))
    }

    pub fn concat() -> InstrSeq {
        instr(Instruct::IOp(InstructOperator::Concat))
    }

    pub fn concatn(n: isize) -> InstrSeq {
        instr(Instruct::IOp(InstructOperator::ConcatN(n)))
    }

    pub fn print() -> InstrSeq {
        instr(Instruct::IOp(InstructOperator::Print))
    }

    pub fn cast_dict() -> InstrSeq {
        instr(Instruct::IOp(InstructOperator::CastDict))
    }

    pub fn cast_string() -> InstrSeq {
        instr(Instruct::IOp(InstructOperator::CastString))
    }

    pub fn cast_int() -> InstrSeq {
        instr(Instruct::IOp(InstructOperator::CastInt))
    }

    pub fn cast_bool() -> InstrSeq {
        instr(Instruct::IOp(InstructOperator::CastBool))
    }

    pub fn cast_double() -> InstrSeq {
        instr(Instruct::IOp(InstructOperator::CastDouble))
    }

    pub fn retc() -> InstrSeq {
        instr(Instruct::IContFlow(InstructControlFlow::RetC))
    }

    pub fn retc_suspended() -> InstrSeq {
        instr(Instruct::IContFlow(InstructControlFlow::RetCSuspended))
    }

    pub fn retm(p: NumParams) -> InstrSeq {
        instr(Instruct::IContFlow(InstructControlFlow::RetM(p)))
    }

    pub fn null() -> InstrSeq {
        instr(Instruct::ILitConst(InstructLitConst::Null))
    }

    pub fn nulluninit() -> InstrSeq {
        instr(Instruct::ILitConst(InstructLitConst::NullUninit))
    }

    pub fn chain_faults() -> InstrSeq {
        instr(Instruct::IMisc(InstructMisc::ChainFaults))
    }

    pub fn dup() -> InstrSeq {
        instr(Instruct::IBasic(InstructBasic::Dup))
    }

    pub fn nop() -> InstrSeq {
        instr(Instruct::IBasic(InstructBasic::Nop))
    }

    pub fn instanceofd(s: ClassId) -> InstrSeq {
        instr(Instruct::IOp(InstructOperator::InstanceOfD(s)))
    }

    pub fn instanceof() -> InstrSeq {
        instr(Instruct::IOp(InstructOperator::InstanceOf))
    }

    pub fn islateboundcls() -> InstrSeq {
        instr(Instruct::IOp(InstructOperator::IsLateBoundCls))
    }

    pub fn istypestructc(mode: TypestructResolveOp) -> InstrSeq {
        instr(Instruct::IOp(InstructOperator::IsTypeStructC(mode)))
    }

    pub fn throwastypestructexception() -> InstrSeq {
        instr(Instruct::IOp(InstructOperator::ThrowAsTypeStructException))
    }

    pub fn throw_non_exhaustive_switch() -> InstrSeq {
        instr(Instruct::IMisc(InstructMisc::ThrowNonExhaustiveSwitch))
    }

    pub fn raise_class_string_conversion_warning() -> InstrSeq {
        instr(Instruct::IMisc(
            InstructMisc::RaiseClassStringConversionWarning,
        ))
    }

    pub fn combine_and_resolve_type_struct(i: isize) -> InstrSeq {
        instr(Instruct::IOp(
            InstructOperator::CombineAndResolveTypeStruct(i),
        ))
    }

    pub fn record_reified_generic() -> InstrSeq {
        instr(Instruct::IMisc(InstructMisc::RecordReifiedGeneric))
    }

    pub fn check_reified_generic_mismatch() -> InstrSeq {
        instr(Instruct::IMisc(InstructMisc::CheckReifiedGenericMismatch))
    }

    pub fn int(i: isize) -> InstrSeq {
        instr(Instruct::ILitConst(InstructLitConst::Int(
            i.try_into().unwrap(),
        )))
    }

    pub fn int64(i: i64) -> InstrSeq {
        instr(Instruct::ILitConst(InstructLitConst::Int(i)))
    }

    pub fn int_of_string(litstr: &str) -> InstrSeq {
        instr(Instruct::ILitConst(InstructLitConst::Int(
            litstr.parse::<i64>().unwrap(),
        )))
    }

    pub fn double(litstr: &str) -> InstrSeq {
        instr(Instruct::ILitConst(InstructLitConst::Double(
            litstr.to_string(),
        )))
    }

    pub fn string(litstr: impl Into<String>) -> InstrSeq {
        instr(Instruct::ILitConst(InstructLitConst::String(litstr.into())))
    }

    pub fn this() -> InstrSeq {
        instr(Instruct::IMisc(InstructMisc::This))
    }

    pub fn istypec(op: IstypeOp) -> InstrSeq {
        instr(Instruct::IIsset(InstructIsset::IsTypeC(op)))
    }

    pub fn istypel(id: local::Type, op: IstypeOp) -> InstrSeq {
        instr(Instruct::IIsset(InstructIsset::IsTypeL(id, op)))
    }

    pub fn add() -> InstrSeq {
        instr(Instruct::IOp(InstructOperator::Add))
    }

    pub fn addo() -> InstrSeq {
        instr(Instruct::IOp(InstructOperator::AddO))
    }

    pub fn sub() -> InstrSeq {
        instr(Instruct::IOp(InstructOperator::Sub))
    }

    pub fn subo() -> InstrSeq {
        instr(Instruct::IOp(InstructOperator::SubO))
    }

    pub fn mul() -> InstrSeq {
        instr(Instruct::IOp(InstructOperator::Mul))
    }

    pub fn mulo() -> InstrSeq {
        instr(Instruct::IOp(InstructOperator::MulO))
    }

    pub fn shl() -> InstrSeq {
        instr(Instruct::IOp(InstructOperator::Shl))
    }

    pub fn shr() -> InstrSeq {
        instr(Instruct::IOp(InstructOperator::Shr))
    }

    pub fn cmp() -> InstrSeq {
        instr(Instruct::IOp(InstructOperator::Cmp))
    }

    pub fn mod_() -> InstrSeq {
        instr(Instruct::IOp(InstructOperator::Mod))
    }

    pub fn div() -> InstrSeq {
        instr(Instruct::IOp(InstructOperator::Div))
    }

    pub fn same() -> InstrSeq {
        instr(Instruct::IOp(InstructOperator::Same))
    }

    pub fn pow() -> InstrSeq {
        instr(Instruct::IOp(InstructOperator::Pow))
    }

    pub fn nsame() -> InstrSeq {
        instr(Instruct::IOp(InstructOperator::NSame))
    }

    pub fn not() -> InstrSeq {
        instr(Instruct::IOp(InstructOperator::Not))
    }

    pub fn bitnot() -> InstrSeq {
        instr(Instruct::IOp(InstructOperator::BitNot))
    }

    pub fn bitand() -> InstrSeq {
        instr(Instruct::IOp(InstructOperator::BitAnd))
    }

    pub fn bitor() -> InstrSeq {
        instr(Instruct::IOp(InstructOperator::BitOr))
    }

    pub fn bitxor() -> InstrSeq {
        instr(Instruct::IOp(InstructOperator::BitXor))
    }

    pub fn sets(op: ReadOnlyOp) -> InstrSeq {
        instr(Instruct::IMutator(InstructMutator::SetS(op)))
    }

    pub fn setl(local: local::Type) -> InstrSeq {
        instr(Instruct::IMutator(InstructMutator::SetL(local)))
    }

    pub fn setg() -> InstrSeq {
        instr(Instruct::IMutator(InstructMutator::SetG))
    }

    pub fn unsetl(local: local::Type) -> InstrSeq {
        instr(Instruct::IMutator(InstructMutator::UnsetL(local)))
    }

    pub fn unsetg() -> InstrSeq {
        instr(Instruct::IMutator(InstructMutator::UnsetG))
    }

    pub fn incdecl(local: local::Type, op: IncdecOp) -> InstrSeq {
        instr(Instruct::IMutator(InstructMutator::IncDecL(local, op)))
    }

    pub fn incdecg(op: IncdecOp) -> InstrSeq {
        instr(Instruct::IMutator(InstructMutator::IncDecG(op)))
    }

    pub fn incdecs(op: IncdecOp, rop: ReadOnlyOp) -> InstrSeq {
        instr(Instruct::IMutator(InstructMutator::IncDecS(op, rop)))
    }

    pub fn setopg(op: EqOp) -> InstrSeq {
        instr(Instruct::IMutator(InstructMutator::SetOpG(op)))
    }

    pub fn setopl(local: local::Type, op: EqOp) -> InstrSeq {
        instr(Instruct::IMutator(InstructMutator::SetOpL(local, op)))
    }

    pub fn setops(op: EqOp, rop: ReadOnlyOp) -> InstrSeq {
        instr(Instruct::IMutator(InstructMutator::SetOpS(op, rop)))
    }

    pub fn issetl(local: local::Type) -> InstrSeq {
        instr(Instruct::IIsset(InstructIsset::IssetL(local)))
    }

    pub fn issetg() -> InstrSeq {
        instr(Instruct::IIsset(InstructIsset::IssetG))
    }

    pub fn issets() -> InstrSeq {
        instr(Instruct::IIsset(InstructIsset::IssetS))
    }

    pub fn isunsetl(local: local::Type) -> InstrSeq {
        instr(Instruct::IIsset(InstructIsset::IsUnsetL(local)))
    }

    pub fn cgets(op: ReadOnlyOp) -> InstrSeq {
        instr(Instruct::IGet(InstructGet::CGetS(op)))
    }

    pub fn cgetg() -> InstrSeq {
        instr(Instruct::IGet(InstructGet::CGetG))
    }

    pub fn cgetl(local: local::Type) -> InstrSeq {
        instr(Instruct::IGet(InstructGet::CGetL(local)))
    }

    pub fn cugetl(local: local::Type) -> InstrSeq {
        instr(Instruct::IGet(InstructGet::CUGetL(local)))
    }

    pub fn cgetl2(local: local::Type) -> InstrSeq {
        instr(Instruct::IGet(InstructGet::CGetL2(local)))
    }

    pub fn cgetquietl(local: local::Type) -> InstrSeq {
        instr(Instruct::IGet(InstructGet::CGetQuietL(local)))
    }

    pub fn classgetc() -> InstrSeq {
        instr(Instruct::IGet(InstructGet::ClassGetC))
    }

    pub fn classgetts() -> InstrSeq {
        instr(Instruct::IGet(InstructGet::ClassGetTS))
    }

    pub fn classname() -> InstrSeq {
        instr(Instruct::IMisc(InstructMisc::ClassName))
    }

    pub fn self_() -> InstrSeq {
        instr(Instruct::IMisc(InstructMisc::Self_))
    }

    pub fn lateboundcls() -> InstrSeq {
        instr(Instruct::IMisc(InstructMisc::LateBoundCls))
    }

    pub fn parent() -> InstrSeq {
        instr(Instruct::IMisc(InstructMisc::Parent))
    }

    pub fn popu() -> InstrSeq {
        instr(Instruct::IBasic(InstructBasic::PopU))
    }

    pub fn popc() -> InstrSeq {
        instr(Instruct::IBasic(InstructBasic::PopC))
    }

    pub fn popl(l: local::Type) -> InstrSeq {
        instr(Instruct::IMutator(InstructMutator::PopL(l)))
    }

    pub fn initprop(pid: PropId, op: InitpropOp) -> InstrSeq {
        instr(Instruct::IMutator(InstructMutator::InitProp(pid, op)))
    }

    pub fn checkprop(pid: PropId) -> InstrSeq {
        instr(Instruct::IMutator(InstructMutator::CheckProp(pid)))
    }

    pub fn pushl(local: local::Type) -> InstrSeq {
        instr(Instruct::IGet(InstructGet::PushL(local)))
    }

    pub fn throw() -> InstrSeq {
        instr(Instruct::IContFlow(InstructControlFlow::Throw))
    }

    pub fn new_vec_array(i: isize) -> InstrSeq {
        instr(Instruct::ILitConst(InstructLitConst::NewVec(i)))
    }

    pub fn new_pair() -> InstrSeq {
        instr(Instruct::ILitConst(InstructLitConst::NewPair))
    }

    pub fn add_elemc() -> InstrSeq {
        instr(Instruct::ILitConst(InstructLitConst::AddElemC))
    }

    pub fn add_new_elemc() -> InstrSeq {
        instr(Instruct::ILitConst(InstructLitConst::AddNewElemC))
    }

    pub fn switch(labels: Vec<Label>) -> InstrSeq {
        instr(Instruct::IContFlow(InstructControlFlow::Switch(
            Switchkind::Unbounded,
            0,
            labels,
        )))
    }

    pub fn newobj() -> InstrSeq {
        instr(Instruct::ICall(InstructCall::NewObj))
    }

    pub fn sswitch(cases: Vec<(String, label::Label)>) -> InstrSeq {
        instr(Instruct::IContFlow(InstructControlFlow::SSwitch(cases)))
    }

    pub fn newobjr() -> InstrSeq {
        instr(Instruct::ICall(InstructCall::NewObjR))
    }

    pub fn newobjd(id: ClassId) -> InstrSeq {
        instr(Instruct::ICall(InstructCall::NewObjD(id)))
    }

    pub fn newobjrd(id: ClassId) -> InstrSeq {
        instr(Instruct::ICall(InstructCall::NewObjRD(id)))
    }

    pub fn newobjs(scref: SpecialClsRef) -> InstrSeq {
        instr(Instruct::ICall(InstructCall::NewObjS(scref)))
    }

    pub fn lockobj() -> InstrSeq {
        instr(Instruct::IMisc(InstructMisc::LockObj))
    }

    pub fn clone() -> InstrSeq {
        instr(Instruct::IOp(InstructOperator::Clone))
    }

    pub fn new_record(id: ClassId, keys: Vec<String>) -> InstrSeq {
        instr(Instruct::ILitConst(InstructLitConst::NewRecord(id, keys)))
    }

    pub fn newstructdict(keys: Vec<String>) -> InstrSeq {
        instr(Instruct::ILitConst(InstructLitConst::NewStructDict(keys)))
    }

    pub fn newcol(collection_type: CollectionType) -> InstrSeq {
        instr(Instruct::ILitConst(InstructLitConst::NewCol(
            collection_type,
        )))
    }

    pub fn colfromarray(collection_type: CollectionType) -> InstrSeq {
        instr(Instruct::ILitConst(InstructLitConst::ColFromArray(
            collection_type,
        )))
    }

    pub fn entrynop() -> InstrSeq {
        instr(Instruct::IBasic(InstructBasic::EntryNop))
    }

    pub fn typedvalue(xs: TypedValue) -> InstrSeq {
        instr(Instruct::ILitConst(InstructLitConst::TypedValue(xs)))
    }

    pub fn basel(local: local::Type, mode: MemberOpMode) -> InstrSeq {
        instr(Instruct::IBase(InstructBase::BaseL(local, mode)))
    }

    pub fn basec(stack_index: StackIndex, mode: MemberOpMode) -> InstrSeq {
        instr(Instruct::IBase(InstructBase::BaseC(stack_index, mode)))
    }

    pub fn basegc(stack_index: StackIndex, mode: MemberOpMode) -> InstrSeq {
        instr(Instruct::IBase(InstructBase::BaseGC(stack_index, mode)))
    }

    pub fn basegl(local: local::Type, mode: MemberOpMode) -> InstrSeq {
        instr(Instruct::IBase(InstructBase::BaseGL(local, mode)))
    }

    pub fn basesc(y: StackIndex, z: StackIndex, mode: MemberOpMode, op: ReadOnlyOp) -> InstrSeq {
        instr(Instruct::IBase(InstructBase::BaseSC(y, z, mode, op)))
    }

    pub fn baseh() -> InstrSeq {
        instr(Instruct::IBase(InstructBase::BaseH))
    }

    pub fn cgetcunop() -> InstrSeq {
        instr(Instruct::IMisc(InstructMisc::CGetCUNop))
    }

    pub fn ugetcunop() -> InstrSeq {
        instr(Instruct::IMisc(InstructMisc::UGetCUNop))
    }

    pub fn memoget(label: Label, range: Option<(local::Type, isize)>) -> InstrSeq {
        instr(Instruct::IMisc(InstructMisc::MemoGet(label, range)))
    }

    pub fn memoget_eager(
        label1: Label,
        label2: Label,
        range: Option<(local::Type, isize)>,
    ) -> InstrSeq {
        instr(Instruct::IMisc(InstructMisc::MemoGetEager(
            label1, label2, range,
        )))
    }

    pub fn memoset(range: Option<(local::Type, isize)>) -> InstrSeq {
        instr(Instruct::IMisc(InstructMisc::MemoSet(range)))
    }

    pub fn memoset_eager(range: Option<(local::Type, isize)>) -> InstrSeq {
        instr(Instruct::IMisc(InstructMisc::MemoSetEager(range)))
    }

    pub fn getmemokeyl(local: local::Type) -> InstrSeq {
        instr(Instruct::IMisc(InstructMisc::GetMemoKeyL(local)))
    }

    pub fn barethis(notice: BareThisOp) -> InstrSeq {
        instr(Instruct::IMisc(InstructMisc::BareThis(notice)))
    }

    pub fn checkthis() -> InstrSeq {
        instr(Instruct::IMisc(InstructMisc::CheckThis))
    }

    pub fn verify_ret_type_c() -> InstrSeq {
        instr(Instruct::IMisc(InstructMisc::VerifyRetTypeC))
    }

    pub fn verify_ret_type_ts() -> InstrSeq {
        instr(Instruct::IMisc(InstructMisc::VerifyRetTypeTS))
    }

    pub fn verify_out_type(i: ParamId) -> InstrSeq {
        instr(Instruct::IMisc(InstructMisc::VerifyOutType(i)))
    }

    pub fn verify_param_type(i: ParamId) -> InstrSeq {
        instr(Instruct::IMisc(InstructMisc::VerifyParamType(i)))
    }

    pub fn verify_param_type_ts(i: ParamId) -> InstrSeq {
        instr(Instruct::IMisc(InstructMisc::VerifyParamTypeTS(i)))
    }

    pub fn dim(op: MemberOpMode, key: MemberKey) -> InstrSeq {
        instr(Instruct::IBase(InstructBase::Dim(op, key)))
    }

    pub fn dim_warn_pt(key: PropId, op: ReadOnlyOp) -> InstrSeq {
        dim(MemberOpMode::Warn, MemberKey::PT(key, op))
    }

    pub fn dim_define_pt(key: PropId, op: ReadOnlyOp) -> InstrSeq {
        dim(MemberOpMode::Define, MemberKey::PT(key, op))
    }

    pub fn fcallclsmethod(
        is_log_as_dynamic_call: IsLogAsDynamicCallOp,
        fcall_args: FcallArgs,
    ) -> InstrSeq {
        instr(Instruct::ICall(InstructCall::FCallClsMethod(
            fcall_args,
            is_log_as_dynamic_call,
        )))
    }

    pub fn fcallclsmethodd(
        fcall_args: FcallArgs,
        method_name: MethodId,
        class_name: ClassId,
    ) -> InstrSeq {
        instr(Instruct::ICall(InstructCall::FCallClsMethodD(
            fcall_args,
            class_name,
            method_name,
        )))
    }

    pub fn fcallclsmethods(fcall_args: FcallArgs, scref: SpecialClsRef) -> InstrSeq {
        instr(Instruct::ICall(InstructCall::FCallClsMethodS(
            fcall_args, scref,
        )))
    }

    pub fn fcallclsmethodsd(
        fcall_args: FcallArgs,
        scref: SpecialClsRef,
        method_name: MethodId,
    ) -> InstrSeq {
        instr(Instruct::ICall(InstructCall::FCallClsMethodSD(
            fcall_args,
            scref,
            method_name,
        )))
    }

    pub fn fcallctor(fcall_args: FcallArgs) -> InstrSeq {
        instr(Instruct::ICall(InstructCall::FCallCtor(fcall_args)))
    }

    pub fn fcallfunc(fcall_args: FcallArgs) -> InstrSeq {
        instr(Instruct::ICall(InstructCall::FCallFunc(fcall_args)))
    }

    pub fn fcallfuncd(fcall_args: FcallArgs, id: FunctionId) -> InstrSeq {
        instr(Instruct::ICall(InstructCall::FCallFuncD(fcall_args, id)))
    }

    pub fn fcallobjmethod(fcall_args: FcallArgs, flavor: ObjNullFlavor) -> InstrSeq {
        instr(Instruct::ICall(InstructCall::FCallObjMethod(
            fcall_args, flavor,
        )))
    }

    pub fn fcallobjmethodd(
        fcall_args: FcallArgs,
        method: MethodId,
        flavor: ObjNullFlavor,
    ) -> InstrSeq {
        instr(Instruct::ICall(InstructCall::FCallObjMethodD(
            fcall_args, flavor, method,
        )))
    }

    pub fn fcallobjmethodd_nullthrows(fcall_args: FcallArgs, method: MethodId) -> InstrSeq {
        fcallobjmethodd(fcall_args, method, ObjNullFlavor::NullThrows)
    }

    pub fn querym(num_params: NumParams, op: QueryOp, key: MemberKey) -> InstrSeq {
        instr(Instruct::IFinal(InstructFinal::QueryM(num_params, op, key)))
    }

    pub fn querym_cget_pt(num_params: NumParams, key: PropId, op: ReadOnlyOp) -> InstrSeq {
        querym(num_params, QueryOp::CGet, MemberKey::PT(key, op))
    }

    pub fn setm(num_params: NumParams, key: MemberKey) -> InstrSeq {
        instr(Instruct::IFinal(InstructFinal::SetM(num_params, key)))
    }

    pub fn unsetm(num_params: NumParams, key: MemberKey) -> InstrSeq {
        instr(Instruct::IFinal(InstructFinal::UnsetM(num_params, key)))
    }

    pub fn setopm(num_params: NumParams, op: EqOp, key: MemberKey) -> InstrSeq {
        instr(Instruct::IFinal(InstructFinal::SetOpM(num_params, op, key)))
    }

    pub fn incdecm(num_params: NumParams, op: IncdecOp, key: MemberKey) -> InstrSeq {
        instr(Instruct::IFinal(InstructFinal::IncDecM(
            num_params, op, key,
        )))
    }

    pub fn setm_pt(num_params: NumParams, key: PropId, op: ReadOnlyOp) -> InstrSeq {
        setm(num_params, MemberKey::PT(key, op))
    }

    pub fn resolve_func(func_id: FunctionId) -> InstrSeq {
        instr(Instruct::IOp(InstructOperator::ResolveFunc(func_id)))
    }

    pub fn resolve_rfunc(func_id: FunctionId) -> InstrSeq {
        instr(Instruct::IOp(InstructOperator::ResolveRFunc(func_id)))
    }

    pub fn resolve_obj_method() -> InstrSeq {
        instr(Instruct::IOp(InstructOperator::ResolveObjMethod))
    }

    pub fn resolveclsmethod(method_id: MethodId) -> InstrSeq {
        instr(Instruct::IOp(InstructOperator::ResolveClsMethod(method_id)))
    }

    pub fn resolveclsmethodd(class_id: ClassId, method_id: MethodId) -> InstrSeq {
        instr(Instruct::IOp(InstructOperator::ResolveClsMethodD(
            class_id, method_id,
        )))
    }

    pub fn resolveclsmethods(scref: SpecialClsRef, method_id: MethodId) -> InstrSeq {
        instr(Instruct::IOp(InstructOperator::ResolveClsMethodS(
            scref, method_id,
        )))
    }

    pub fn resolverclsmethod(method_id: MethodId) -> InstrSeq {
        instr(Instruct::IOp(InstructOperator::ResolveRClsMethod(
            method_id,
        )))
    }

    pub fn resolverclsmethodd(class_id: ClassId, method_id: MethodId) -> InstrSeq {
        instr(Instruct::IOp(InstructOperator::ResolveRClsMethodD(
            class_id, method_id,
        )))
    }

    pub fn resolverclsmethods(scref: SpecialClsRef, method_id: MethodId) -> InstrSeq {
        instr(Instruct::IOp(InstructOperator::ResolveRClsMethodS(
            scref, method_id,
        )))
    }

    pub fn resolve_meth_caller(fun_id: FunctionId) -> InstrSeq {
        instr(Instruct::IOp(InstructOperator::ResolveMethCaller(fun_id)))
    }

    pub fn resolveclass(class_id: ClassId) -> InstrSeq {
        instr(Instruct::IOp(InstructOperator::ResolveClass(class_id)))
    }

    pub fn lazyclass(class_id: ClassId) -> InstrSeq {
        instr(Instruct::ILitConst(InstructLitConst::LazyClass(class_id)))
    }

    pub fn oodeclexists(class_kind: ClassKind) -> InstrSeq {
        instr(Instruct::IMisc(InstructMisc::OODeclExists(class_kind)))
    }

    pub fn fatal(op: FatalOp) -> InstrSeq {
        instr(Instruct::IOp(InstructOperator::Fatal(op)))
    }

    pub fn await_() -> InstrSeq {
        instr(Instruct::IAsync(AsyncFunctions::Await))
    }

    pub fn yield_() -> InstrSeq {
        instr(Instruct::IGenerator(GenCreationExecution::Yield))
    }

    pub fn yieldk() -> InstrSeq {
        instr(Instruct::IGenerator(GenCreationExecution::YieldK))
    }

    pub fn createcont() -> InstrSeq {
        instr(Instruct::IGenerator(GenCreationExecution::CreateCont))
    }

    pub fn awaitall(range: Option<(local::Type, isize)>) -> InstrSeq {
        instr(Instruct::IAsync(AsyncFunctions::AwaitAll(range)))
    }

    pub fn label(label: Label) -> InstrSeq {
        instr(Instruct::ILabel(label))
    }

    pub fn awaitall_list(unnamed_locals: Vec<local::Type>) -> InstrSeq {
        use local::Type::Unnamed;
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
                    awaitall(Some((
                        Unnamed(*hd_id),
                        unnamed_locals.len().try_into().unwrap(),
                    )))
                } else {
                    panic!("Expected unnamed local")
                }
            }
        }
    }

    pub fn exit() -> InstrSeq {
        instr(Instruct::IOp(InstructOperator::Exit))
    }

    pub fn idx() -> InstrSeq {
        instr(Instruct::IMisc(InstructMisc::Idx))
    }

    pub fn array_idx() -> InstrSeq {
        instr(Instruct::IMisc(InstructMisc::ArrayIdx))
    }

    pub fn createcl(param_num: NumParams, cls_num: ClassNum) -> InstrSeq {
        instr(Instruct::IMisc(InstructMisc::CreateCl(param_num, cls_num)))
    }

    pub fn eval() -> InstrSeq {
        instr(Instruct::IIncludeEvalDefine(
            InstructIncludeEvalDefine::Eval,
        ))
    }

    pub fn incl() -> InstrSeq {
        instr(Instruct::IIncludeEvalDefine(
            InstructIncludeEvalDefine::Incl,
        ))
    }

    pub fn inclonce() -> InstrSeq {
        instr(Instruct::IIncludeEvalDefine(
            InstructIncludeEvalDefine::InclOnce,
        ))
    }

    pub fn req() -> InstrSeq {
        instr(Instruct::IIncludeEvalDefine(InstructIncludeEvalDefine::Req))
    }

    pub fn reqdoc() -> InstrSeq {
        instr(Instruct::IIncludeEvalDefine(
            InstructIncludeEvalDefine::ReqDoc,
        ))
    }

    pub fn reqonce() -> InstrSeq {
        instr(Instruct::IIncludeEvalDefine(
            InstructIncludeEvalDefine::ReqOnce,
        ))
    }

    pub fn silence_start(local: local::Type) -> InstrSeq {
        instr(Instruct::IMisc(InstructMisc::Silence(
            local,
            OpSilence::Start,
        )))
    }

    pub fn silence_end(local: local::Type) -> InstrSeq {
        instr(Instruct::IMisc(InstructMisc::Silence(
            local,
            OpSilence::End,
        )))
    }

    pub fn contcheck_check() -> InstrSeq {
        instr(Instruct::IGenerator(GenCreationExecution::ContCheck(
            CheckStarted::CheckStarted,
        )))
    }

    pub fn contcheck_ignore() -> InstrSeq {
        instr(Instruct::IGenerator(GenCreationExecution::ContCheck(
            CheckStarted::IgnoreStarted,
        )))
    }

    pub fn contenter() -> InstrSeq {
        instr(Instruct::IGenerator(GenCreationExecution::ContEnter))
    }

    pub fn contraise() -> InstrSeq {
        instr(Instruct::IGenerator(GenCreationExecution::ContRaise))
    }

    pub fn contvalid() -> InstrSeq {
        instr(Instruct::IGenerator(GenCreationExecution::ContValid))
    }

    pub fn contcurrent() -> InstrSeq {
        instr(Instruct::IGenerator(GenCreationExecution::ContCurrent))
    }

    pub fn contkey() -> InstrSeq {
        instr(Instruct::IGenerator(GenCreationExecution::ContKey))
    }

    pub fn contgetreturn() -> InstrSeq {
        instr(Instruct::IGenerator(GenCreationExecution::ContGetReturn))
    }

    pub fn nativeimpl() -> InstrSeq {
        instr(Instruct::IMisc(InstructMisc::NativeImpl))
    }

    pub fn srcloc(
        line_begin: isize,
        line_end: isize,
        col_begin: isize,
        col_end: isize,
    ) -> InstrSeq {
        instr(Instruct::ISrcLoc(Srcloc {
            line_begin,
            line_end,
            col_begin,
            col_end,
        }))
    }

    pub fn is_type_structc_resolve() -> InstrSeq {
        instr(Instruct::IOp(InstructOperator::IsTypeStructC(
            TypestructResolveOp::Resolve,
        )))
    }

    pub fn is_type_structc_dontresolve() -> InstrSeq {
        instr(Instruct::IOp(InstructOperator::IsTypeStructC(
            TypestructResolveOp::DontResolve,
        )))
    }
}

impl InstrSeq {
    pub fn gather(instrs: Vec<InstrSeq>) -> InstrSeq {
        let mut nonempty_instrs = instrs
            .into_iter()
            .filter(|x| !matches!(x, InstrSeq::Empty))
            .collect::<Vec<_>>();
        if nonempty_instrs.is_empty() {
            InstrSeq::Empty
        } else if nonempty_instrs.len() == 1 {
            nonempty_instrs.pop().unwrap()
        } else {
            InstrSeq::Concat(nonempty_instrs)
        }
    }

    pub fn optional(pred: bool, instrs: Vec<Self>) -> InstrSeq {
        if pred {
            InstrSeq::gather(instrs)
        } else {
            InstrSeq::Empty
        }
    }

    pub fn iter(&self) -> InstrIter {
        InstrIter::new(self)
    }

    pub fn compact_iter(&self) -> impl Iterator<Item = &Instruct> {
        CompactIter::new(self.iter())
    }

    pub fn create_try_catch(
        label_gen: &mut label::Gen,
        opt_done_label: Option<Label>,
        skip_throw: bool,
        try_instrs: Self,
        catch_instrs: Self,
    ) -> InstrSeq {
        let done_label = match opt_done_label {
            Some(l) => l,
            None => label_gen.next_regular(),
        };
        InstrSeq::gather(vec![
            instr::instr(Instruct::ITry(InstructTry::TryCatchBegin)),
            try_instrs,
            instr::jmp(done_label.clone()),
            instr::instr(Instruct::ITry(InstructTry::TryCatchMiddle)),
            catch_instrs,
            if skip_throw {
                instr::empty()
            } else {
                instr::instr(Instruct::IContFlow(InstructControlFlow::Throw))
            },
            instr::instr(Instruct::ITry(InstructTry::TryCatchEnd)),
            instr::label(done_label),
        ])
    }

    fn get_or_put_label<'i, 'm>(
        label_gen: &mut label::Gen,
        name_label_map: &'m mut HashMap<String, Label>,
        name: &'i String,
    ) -> Label {
        match name_label_map.get(name.as_str()) {
            Some(label) => label.clone(),
            None => {
                let l = label_gen.next_regular();
                name_label_map.insert(name.to_string(), l.clone());
                l
            }
        }
    }

    fn rewrite_user_labels_instr<'i, 'm>(
        label_gen: &mut label::Gen,
        i: &'i mut Instruct,
        name_label_map: &'m mut HashMap<String, Label>,
    ) {
        use Instruct::*;
        let mut get_result = |x| InstrSeq::get_or_put_label(label_gen, name_label_map, x);
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

    pub fn rewrite_user_labels(&mut self, label_gen: &mut label::Gen) {
        let name_label_map = &mut HashMap::new();
        self.map_mut(&mut |i| InstrSeq::rewrite_user_labels_instr(label_gen, i, name_label_map));
    }

    fn is_srcloc(instruction: &Instruct) -> bool {
        match instruction {
            Instruct::ISrcLoc(_) => true,
            _ => false,
        }
    }

    pub fn first(&self) -> Option<&Instruct> {
        match self {
            InstrSeq::Empty => None,
            InstrSeq::One(i) => {
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

    pub fn is_empty(&self) -> bool {
        match self {
            InstrSeq::Empty => true,
            InstrSeq::One(i) => InstrSeq::is_srcloc(i),
            InstrSeq::List(l) => l.is_empty() || l.iter().all(InstrSeq::is_srcloc),
            InstrSeq::Concat(l) => l.iter().all(InstrSeq::is_empty),
        }
    }

    pub fn flat_map_seq<F>(&self, f: &mut F) -> Self
    where
        F: FnMut(&Instruct) -> Self,
    {
        match self {
            InstrSeq::Empty => InstrSeq::Empty,
            InstrSeq::One(instr) => f(instr),
            InstrSeq::List(instr_lst) => {
                InstrSeq::Concat(instr_lst.iter().map(|x| f(x)).collect::<Vec<_>>())
            }
            InstrSeq::Concat(instrseq_lst) => InstrSeq::Concat(
                instrseq_lst
                    .iter()
                    .map(|x| x.flat_map_seq(f))
                    .collect::<Vec<_>>(),
            ),
        }
    }

    pub fn fold_left<'a, F, A>(&'a self, f: &mut F, init: A) -> A
    where
        F: FnMut(A, &'a Instruct) -> A,
    {
        match self {
            InstrSeq::Empty => init,
            InstrSeq::One(x) => f(init, x),
            InstrSeq::List(instr_lst) => instr_lst.iter().fold(init, f),
            InstrSeq::Concat(instrseq_lst) => {
                instrseq_lst.iter().fold(init, |acc, x| x.fold_left(f, acc))
            }
        }
    }

    pub fn filter_map<F>(&self, f: &mut F) -> Self
    where
        F: FnMut(&Instruct) -> Option<Instruct>,
    {
        match self {
            InstrSeq::Empty => InstrSeq::Empty,
            InstrSeq::One(x) => match f(x) {
                Some(x) => instr::instr(x),
                None => InstrSeq::Empty,
            },
            InstrSeq::List(instr_lst) => {
                InstrSeq::List(instr_lst.iter().filter_map(f).collect::<Vec<_>>())
            }
            InstrSeq::Concat(instrseq_lst) => InstrSeq::Concat(
                instrseq_lst
                    .iter()
                    .map(|x| x.filter_map(f))
                    .collect::<Vec<_>>(),
            ),
        }
    }

    pub fn filter_map_mut<F>(&mut self, f: &mut F)
    where
        F: FnMut(&mut Instruct) -> bool,
    {
        match self {
            InstrSeq::Empty => {}
            InstrSeq::One(x) => {
                if !f(x) {
                    *self = InstrSeq::Empty
                }
            }
            InstrSeq::List(ref mut instr_lst) => {
                let mut new_list = Vec::with_capacity(instr_lst.len());
                for mut i in instr_lst.drain(..).into_iter() {
                    if f(&mut i) {
                        new_list.push(i)
                    }
                }
                *instr_lst = new_list;
            }
            InstrSeq::Concat(instrseq_lst) => {
                instrseq_lst.iter_mut().for_each(|x| x.filter_map_mut(f))
            }
        }
    }

    pub fn map_mut<'i, F>(&'i mut self, f: &mut F)
    where
        F: FnMut(&'i mut Instruct),
    {
        match self {
            InstrSeq::Empty => {}
            InstrSeq::One(x) => f(x),
            InstrSeq::List(instr_lst) => instr_lst.iter_mut().for_each(f),
            InstrSeq::Concat(instrseq_lst) => instrseq_lst.iter_mut().for_each(|x| x.map_mut(f)),
        }
    }

    pub fn map_result_mut<F>(&mut self, f: &mut F) -> Result<()>
    where
        F: FnMut(&mut Instruct) -> Result<()>,
    {
        match self {
            InstrSeq::Empty => Ok(()),
            InstrSeq::One(x) => f(x),
            InstrSeq::List(instr_lst) => instr_lst.iter_mut().map(|x| f(x)).collect::<Result<()>>(),
            InstrSeq::Concat(instrseq_lst) => instrseq_lst
                .iter_mut()
                .map(|x| x.map_result_mut(f))
                .collect::<Result<()>>(),
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
        let mk_i = || Instruct::IComment("".into());
        let empty = || InstrSeq::Empty;
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
