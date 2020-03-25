// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
#![allow(dead_code)]

use env::{iterator::Id as IterId, local};
use hhbc_ast_rust::*;
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
    fn default() -> Self {
        Self::Empty
    }
}

impl From<(InstrSeq, InstrSeq)> for InstrSeq {
    fn from((i1, i2): (InstrSeq, InstrSeq)) -> Self {
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
pub struct InstrIter<'i> {
    instr_seq: &'i InstrSeq,
    index: usize,
    concat_cur: Option<Box<InstrIter<'i>>>,
}

impl<'i> InstrIter<'i> {
    pub fn new(instr_seq: &'i InstrSeq) -> Self {
        Self {
            instr_seq,
            index: 0,
            concat_cur: None,
        }
    }
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
            InstrSeq::Concat(ii) if self.index >= ii.len() => None,
            InstrSeq::Concat(ii) => match &mut self.concat_cur {
                Some(cur) => {
                    let r = cur.as_mut().next();
                    if r.is_some() {
                        r
                    } else {
                        self.index += 1;
                        std::mem::replace(&mut self.concat_cur, None);
                        self.next()
                    }
                }
                None => {
                    std::mem::replace(&mut self.concat_cur, Some(Box::new(ii[self.index].iter())));
                    self.next()
                }
            },
        }
    }
}

impl InstrSeq {
    pub fn gather(instrs: Vec<Self>) -> Self {
        let nonempty_instrs = instrs
            .into_iter()
            .filter(|x| match x {
                Self::Empty => false,
                _ => true,
            })
            .collect::<Vec<_>>();
        match &nonempty_instrs[..] {
            [] => Self::Empty,
            [x] => x.clone(),
            xs => Self::Concat(xs.to_vec()),
        }
    }

    pub fn of_pair((i1, i2): (Self, Self)) -> Self {
        Self::gather(vec![i1, i2])
    }

    pub fn optional(pred: bool, instrs: Vec<Self>) -> Self {
        if pred {
            Self::gather(instrs)
        } else {
            Self::Empty
        }
    }

    pub fn iter(&self) -> InstrIter {
        InstrIter::new(self)
    }

    pub fn compact_iter(&self) -> impl Iterator<Item = &Instruct> {
        CompactIter::new(self.iter())
    }

    pub fn make_empty() -> Self {
        Self::Empty
    }

    pub fn make_instr(instruction: Instruct) -> Self {
        Self::One(Box::new(instruction))
    }

    pub fn make_instrs(instructions: Vec<Instruct>) -> Self {
        Self::List(instructions)
    }

    pub fn make_lit_const(l: InstructLitConst) -> Self {
        Self::make_instr(Instruct::ILitConst(l))
    }

    /* TODO(hrust): re-enable it with arg
     pub fn make_lit_empty_varray() -> Self {
        Self::make_lit_const(InstructLitConst::TypedValue(TypedValue::VArray(vec![])))
    } */

    pub fn make_iterinit(args: IterArgs, label: Label) -> Self {
        Self::make_instr(Instruct::IIterator(InstructIterator::IterInit(args, label)))
    }

    pub fn make_iternext(args: IterArgs, label: Label) -> Self {
        Self::make_instr(Instruct::IIterator(InstructIterator::IterNext(args, label)))
    }

    pub fn make_iternextk(id: IterId, label: Label, value: local::Type, key: local::Type) -> Self {
        let args = IterArgs {
            iter_id: id,
            key_id: Some(key),
            val_id: value,
        };
        Self::make_instr(Instruct::IIterator(InstructIterator::IterNext(args, label)))
    }

    pub fn make_iterfree(id: IterId) -> Self {
        Self::make_instr(Instruct::IIterator(InstructIterator::IterFree(id)))
    }

    pub fn make_whresult() -> Self {
        Self::make_instr(Instruct::IAsync(AsyncFunctions::WHResult))
    }

    pub fn make_jmp(label: Label) -> Self {
        Self::make_instr(Instruct::IContFlow(InstructControlFlow::Jmp(label)))
    }

    pub fn make_jmpz(label: Label) -> Self {
        Self::make_instr(Instruct::IContFlow(InstructControlFlow::JmpZ(label)))
    }

    pub fn make_jmpnz(label: Label) -> Self {
        Self::make_instr(Instruct::IContFlow(InstructControlFlow::JmpNZ(label)))
    }

    pub fn make_jmpns(label: Label) -> Self {
        Self::make_instr(Instruct::IContFlow(InstructControlFlow::JmpNS(label)))
    }

    pub fn make_continue(level: isize) -> Self {
        Self::make_instr(Instruct::ISpecialFlow(InstructSpecialFlow::Continue(level)))
    }

    pub fn make_break(level: isize) -> Self {
        Self::make_instr(Instruct::ISpecialFlow(InstructSpecialFlow::Break(level)))
    }

    pub fn make_goto(label: String) -> Self {
        Self::make_instr(Instruct::ISpecialFlow(InstructSpecialFlow::Goto(label)))
    }

    pub fn make_iter_break(label: Label, itrs: Vec<IterId>) -> Self {
        let mut instrs = itrs
            .into_iter()
            .map(|id| Instruct::IIterator(InstructIterator::IterFree(id)))
            .collect::<Vec<_>>();
        instrs.push(Instruct::IContFlow(InstructControlFlow::Jmp(label)));
        Self::make_instrs(instrs)
    }

    pub fn make_false() -> Self {
        Self::make_instr(Instruct::ILitConst(InstructLitConst::False))
    }

    pub fn make_true() -> Self {
        Self::make_instr(Instruct::ILitConst(InstructLitConst::True))
    }

    pub fn make_clscnsd(const_id: ConstId, cid: ClassId) -> Self {
        Self::make_instr(Instruct::ILitConst(InstructLitConst::ClsCnsD(
            const_id, cid,
        )))
    }

    pub fn make_clscns(const_id: ConstId) -> Self {
        Self::make_instr(Instruct::ILitConst(InstructLitConst::ClsCns(const_id)))
    }

    pub fn make_eq() -> Self {
        Self::make_instr(Instruct::IOp(InstructOperator::Eq))
    }

    pub fn make_neq() -> Self {
        Self::make_instr(Instruct::IOp(InstructOperator::Neq))
    }

    pub fn make_gt() -> Self {
        Self::make_instr(Instruct::IOp(InstructOperator::Gt))
    }

    pub fn make_gte() -> Self {
        Self::make_instr(Instruct::IOp(InstructOperator::Gte))
    }

    pub fn make_lt() -> Self {
        Self::make_instr(Instruct::IOp(InstructOperator::Lt))
    }

    pub fn make_lte() -> Self {
        Self::make_instr(Instruct::IOp(InstructOperator::Lte))
    }

    pub fn make_concat() -> Self {
        Self::make_instr(Instruct::IOp(InstructOperator::Concat))
    }

    pub fn make_concatn(n: isize) -> Self {
        Self::make_instr(Instruct::IOp(InstructOperator::ConcatN(n)))
    }

    pub fn make_print() -> Self {
        Self::make_instr(Instruct::IOp(InstructOperator::Print))
    }

    pub fn make_cast_darray() -> Self {
        Self::make_instr(Instruct::IOp(InstructOperator::CastDArray))
    }

    pub fn make_cast_dict() -> Self {
        Self::make_instr(Instruct::IOp(InstructOperator::CastDict))
    }

    pub fn make_cast_string() -> Self {
        Self::make_instr(Instruct::IOp(InstructOperator::CastString))
    }

    pub fn make_retc() -> Self {
        Self::make_instr(Instruct::IContFlow(InstructControlFlow::RetC))
    }

    pub fn make_retc_suspended() -> Self {
        Self::make_instr(Instruct::IContFlow(InstructControlFlow::RetCSuspended))
    }

    pub fn make_retm(p: NumParams) -> Self {
        Self::make_instr(Instruct::IContFlow(InstructControlFlow::RetM(p)))
    }

    pub fn make_null() -> Self {
        Self::make_instr(Instruct::ILitConst(InstructLitConst::Null))
    }

    pub fn make_nulluninit() -> Self {
        Self::make_instr(Instruct::ILitConst(InstructLitConst::NullUninit))
    }

    pub fn make_chain_faults() -> Self {
        Self::make_instr(Instruct::IMisc(InstructMisc::ChainFaults))
    }

    pub fn make_dup() -> Self {
        Self::make_instr(Instruct::IBasic(InstructBasic::Dup))
    }

    pub fn make_nop() -> Self {
        Self::make_instr(Instruct::IBasic(InstructBasic::Nop))
    }

    pub fn make_instanceofd(s: ClassId) -> Self {
        Self::make_instr(Instruct::IOp(InstructOperator::InstanceOfD(s)))
    }

    pub fn make_instanceof() -> Self {
        Self::make_instr(Instruct::IOp(InstructOperator::InstanceOf))
    }

    pub fn make_islateboundcls() -> Self {
        Self::make_instr(Instruct::IOp(InstructOperator::IsLateBoundCls))
    }

    pub fn make_istypestructc(mode: TypestructResolveOp) -> Self {
        Self::make_instr(Instruct::IOp(InstructOperator::IsTypeStructC(mode)))
    }

    pub fn make_throwastypestructexception() -> Self {
        Self::make_instr(Instruct::IOp(InstructOperator::ThrowAsTypeStructException))
    }

    pub fn make_throw_non_exhaustive_switch() -> Self {
        Self::make_instr(Instruct::IMisc(InstructMisc::ThrowNonExhaustiveSwitch))
    }

    pub fn make_combine_and_resolve_type_struct(i: isize) -> Self {
        Self::make_instr(Instruct::IOp(
            InstructOperator::CombineAndResolveTypeStruct(i),
        ))
    }

    pub fn make_record_reified_generic() -> Self {
        Self::make_instr(Instruct::IMisc(InstructMisc::RecordReifiedGeneric))
    }

    pub fn make_check_reified_generic_mismatch() -> Self {
        Self::make_instr(Instruct::IMisc(InstructMisc::CheckReifiedGenericMismatch))
    }

    pub fn make_int(i: isize) -> Self {
        Self::make_instr(Instruct::ILitConst(InstructLitConst::Int(
            i.try_into().unwrap(),
        )))
    }

    pub fn make_int64(i: i64) -> Self {
        Self::make_instr(Instruct::ILitConst(InstructLitConst::Int(i)))
    }

    pub fn make_int_of_string(litstr: &str) -> Self {
        Self::make_instr(Instruct::ILitConst(InstructLitConst::Int(
            litstr.parse::<i64>().unwrap(),
        )))
    }

    pub fn make_double(litstr: &str) -> Self {
        Self::make_instr(Instruct::ILitConst(InstructLitConst::Double(
            litstr.to_string(),
        )))
    }

    pub fn make_string(litstr: impl Into<String>) -> Self {
        Self::make_instr(Instruct::ILitConst(InstructLitConst::String(litstr.into())))
    }

    pub fn make_this() -> Self {
        Self::make_instr(Instruct::IMisc(InstructMisc::This))
    }

    pub fn make_initthisloc(id: local::Type) -> Self {
        Self::make_instr(Instruct::IMisc(InstructMisc::InitThisLoc(id)))
    }

    pub fn make_istypec(op: IstypeOp) -> Self {
        Self::make_instr(Instruct::IIsset(InstructIsset::IsTypeC(op)))
    }

    pub fn make_istypel(id: local::Type, op: IstypeOp) -> Self {
        Self::make_instr(Instruct::IIsset(InstructIsset::IsTypeL(id, op)))
    }

    pub fn make_add() -> Self {
        Self::make_instr(Instruct::IOp(InstructOperator::Add))
    }

    pub fn make_addo() -> Self {
        Self::make_instr(Instruct::IOp(InstructOperator::AddO))
    }

    pub fn make_sub() -> Self {
        Self::make_instr(Instruct::IOp(InstructOperator::Sub))
    }

    pub fn make_subo() -> Self {
        Self::make_instr(Instruct::IOp(InstructOperator::SubO))
    }

    pub fn make_mul() -> Self {
        Self::make_instr(Instruct::IOp(InstructOperator::Mul))
    }

    pub fn make_mulo() -> Self {
        Self::make_instr(Instruct::IOp(InstructOperator::MulO))
    }

    pub fn make_shl() -> Self {
        Self::make_instr(Instruct::IOp(InstructOperator::Shl))
    }

    pub fn make_shr() -> Self {
        Self::make_instr(Instruct::IOp(InstructOperator::Shr))
    }

    pub fn make_cmp() -> Self {
        Self::make_instr(Instruct::IOp(InstructOperator::Cmp))
    }

    pub fn make_mod() -> Self {
        Self::make_instr(Instruct::IOp(InstructOperator::Mod))
    }

    pub fn make_div() -> Self {
        Self::make_instr(Instruct::IOp(InstructOperator::Div))
    }

    pub fn make_same() -> Self {
        Self::make_instr(Instruct::IOp(InstructOperator::Same))
    }

    pub fn make_pow() -> Self {
        Self::make_instr(Instruct::IOp(InstructOperator::Pow))
    }

    pub fn make_nsame() -> Self {
        Self::make_instr(Instruct::IOp(InstructOperator::NSame))
    }

    pub fn make_not() -> Self {
        Self::make_instr(Instruct::IOp(InstructOperator::Not))
    }

    pub fn make_xor() -> Self {
        Self::make_instr(Instruct::IOp(InstructOperator::Xor))
    }

    pub fn make_bitnot() -> Self {
        Self::make_instr(Instruct::IOp(InstructOperator::BitNot))
    }

    pub fn make_bitand() -> Self {
        Self::make_instr(Instruct::IOp(InstructOperator::BitAnd))
    }

    pub fn make_bitor() -> Self {
        Self::make_instr(Instruct::IOp(InstructOperator::BitOr))
    }

    pub fn make_bitxor() -> Self {
        Self::make_instr(Instruct::IOp(InstructOperator::BitXor))
    }

    pub fn make_sets() -> Self {
        Self::make_instr(Instruct::IMutator(InstructMutator::SetS))
    }

    pub fn make_setl(local: local::Type) -> Self {
        Self::make_instr(Instruct::IMutator(InstructMutator::SetL(local)))
    }

    pub fn make_setg() -> Self {
        Self::make_instr(Instruct::IMutator(InstructMutator::SetG))
    }

    pub fn make_unsetl(local: local::Type) -> Self {
        Self::make_instr(Instruct::IMutator(InstructMutator::UnsetL(local)))
    }

    pub fn make_unsetg() -> Self {
        Self::make_instr(Instruct::IMutator(InstructMutator::UnsetG))
    }

    pub fn make_incdecl(local: local::Type, op: IncdecOp) -> Self {
        Self::make_instr(Instruct::IMutator(InstructMutator::IncDecL(local, op)))
    }

    pub fn make_incdecg(op: IncdecOp) -> Self {
        Self::make_instr(Instruct::IMutator(InstructMutator::IncDecG(op)))
    }

    pub fn make_incdecs(op: IncdecOp) -> Self {
        Self::make_instr(Instruct::IMutator(InstructMutator::IncDecS(op)))
    }

    pub fn make_setopg(op: EqOp) -> Self {
        Self::make_instr(Instruct::IMutator(InstructMutator::SetOpG(op)))
    }

    pub fn make_setopl(local: local::Type, op: EqOp) -> Self {
        Self::make_instr(Instruct::IMutator(InstructMutator::SetOpL(local, op)))
    }

    pub fn make_setops(op: EqOp) -> Self {
        Self::make_instr(Instruct::IMutator(InstructMutator::SetOpS(op)))
    }

    pub fn make_issetl(local: local::Type) -> Self {
        Self::make_instr(Instruct::IIsset(InstructIsset::IssetL(local)))
    }

    pub fn make_issetg() -> Self {
        Self::make_instr(Instruct::IIsset(InstructIsset::IssetG))
    }

    pub fn make_issets() -> Self {
        Self::make_instr(Instruct::IIsset(InstructIsset::IssetS))
    }

    pub fn make_cgets() -> Self {
        Self::make_instr(Instruct::IGet(InstructGet::CGetS))
    }

    pub fn make_cgetg() -> Self {
        Self::make_instr(Instruct::IGet(InstructGet::CGetG))
    }

    pub fn make_cgetl(local: local::Type) -> Self {
        Self::make_instr(Instruct::IGet(InstructGet::CGetL(local)))
    }

    pub fn make_cugetl(local: local::Type) -> Self {
        Self::make_instr(Instruct::IGet(InstructGet::CUGetL(local)))
    }

    pub fn make_cgetl2(local: local::Type) -> Self {
        Self::make_instr(Instruct::IGet(InstructGet::CGetL2(local)))
    }

    pub fn make_cgetquietl(local: local::Type) -> Self {
        Self::make_instr(Instruct::IGet(InstructGet::CGetQuietL(local)))
    }

    pub fn make_classgetc() -> Self {
        Self::make_instr(Instruct::IGet(InstructGet::ClassGetC))
    }

    pub fn make_classgetts() -> Self {
        Self::make_instr(Instruct::IGet(InstructGet::ClassGetTS))
    }

    pub fn make_classname() -> Self {
        Self::make_instr(Instruct::IMisc(InstructMisc::ClassName))
    }

    pub fn make_self() -> Self {
        Self::make_instr(Instruct::IMisc(InstructMisc::Self_))
    }

    pub fn make_lateboundcls() -> Self {
        Self::make_instr(Instruct::IMisc(InstructMisc::LateBoundCls))
    }

    pub fn make_parent() -> Self {
        Self::make_instr(Instruct::IMisc(InstructMisc::Parent))
    }

    pub fn make_popu() -> Self {
        Self::make_instr(Instruct::IBasic(InstructBasic::PopU))
    }

    pub fn make_popc() -> Self {
        Self::make_instr(Instruct::IBasic(InstructBasic::PopC))
    }

    pub fn make_popl(l: local::Type) -> Self {
        Self::make_instr(Instruct::IMutator(InstructMutator::PopL(l)))
    }

    pub fn make_initprop(pid: PropId, op: InitpropOp) -> Self {
        Self::make_instr(Instruct::IMutator(InstructMutator::InitProp(pid, op)))
    }

    pub fn make_checkprop(pid: PropId) -> Self {
        Self::make_instr(Instruct::IMutator(InstructMutator::CheckProp(pid)))
    }

    pub fn make_pushl(local: local::Type) -> Self {
        Self::make_instr(Instruct::IGet(InstructGet::PushL(local)))
    }

    pub fn make_throw() -> Self {
        Self::make_instr(Instruct::IContFlow(InstructControlFlow::Throw))
    }

    pub fn make_new_vec_array(i: isize) -> Self {
        Self::make_instr(Instruct::ILitConst(InstructLitConst::NewVecArray(i)))
    }

    pub fn make_new_pair() -> Self {
        Self::make_instr(Instruct::ILitConst(InstructLitConst::NewPair))
    }

    pub fn make_add_elemc() -> Self {
        Self::make_instr(Instruct::ILitConst(InstructLitConst::AddElemC))
    }

    pub fn make_add_new_elemc() -> Self {
        Self::make_instr(Instruct::ILitConst(InstructLitConst::AddNewElemC))
    }

    pub fn make_switch(labels: Vec<Label>) -> Self {
        Self::make_instr(Instruct::IContFlow(InstructControlFlow::Switch(
            Switchkind::Unbounded,
            0,
            labels,
        )))
    }

    pub fn make_newobj() -> Self {
        Self::make_instr(Instruct::ICall(InstructCall::NewObj))
    }

    pub fn make_newobjr() -> Self {
        Self::make_instr(Instruct::ICall(InstructCall::NewObjR))
    }

    pub fn make_newobjd(id: ClassId) -> Self {
        Self::make_instr(Instruct::ICall(InstructCall::NewObjD(id)))
    }

    pub fn make_newobjrd(id: ClassId) -> Self {
        Self::make_instr(Instruct::ICall(InstructCall::NewObjRD(id)))
    }

    pub fn make_newobjs(scref: SpecialClsRef) -> Self {
        Self::make_instr(Instruct::ICall(InstructCall::NewObjS(scref)))
    }

    pub fn make_lockobj() -> Self {
        Self::make_instr(Instruct::IMisc(InstructMisc::LockObj))
    }

    pub fn make_clone() -> Self {
        Self::make_instr(Instruct::IOp(InstructOperator::Clone))
    }

    pub fn make_new_record(id: ClassId, keys: Vec<String>) -> Self {
        Self::make_instr(Instruct::ILitConst(InstructLitConst::NewRecord(id, keys)))
    }

    pub fn make_new_recordarray(id: ClassId, keys: Vec<String>) -> Self {
        Self::make_instr(Instruct::ILitConst(InstructLitConst::NewRecordArray(
            id, keys,
        )))
    }

    pub fn make_newstructarray(keys: Vec<String>) -> Self {
        Self::make_instr(Instruct::ILitConst(InstructLitConst::NewStructArray(keys)))
    }

    pub fn make_newstructdarray(keys: Vec<String>) -> Self {
        Self::make_instr(Instruct::ILitConst(InstructLitConst::NewStructDArray(keys)))
    }

    pub fn make_newstructdict(keys: Vec<String>) -> Self {
        Self::make_instr(Instruct::ILitConst(InstructLitConst::NewStructDict(keys)))
    }

    pub fn make_newcol(collection_type: CollectionType) -> Self {
        Self::make_instr(Instruct::ILitConst(InstructLitConst::NewCol(
            collection_type,
        )))
    }

    pub fn make_colfromarray(collection_type: CollectionType) -> Self {
        Self::make_instr(Instruct::ILitConst(InstructLitConst::ColFromArray(
            collection_type,
        )))
    }

    pub fn make_entrynop() -> Self {
        Self::make_instr(Instruct::IBasic(InstructBasic::EntryNop))
    }

    pub fn make_typedvalue(xs: TypedValue) -> Self {
        Self::make_instr(Instruct::ILitConst(InstructLitConst::TypedValue(xs)))
    }

    pub fn make_basel(local: local::Type, mode: MemberOpMode) -> Self {
        Self::make_instr(Instruct::IBase(InstructBase::BaseL(local, mode)))
    }

    pub fn make_basec(stack_index: StackIndex, mode: MemberOpMode) -> Self {
        Self::make_instr(Instruct::IBase(InstructBase::BaseC(stack_index, mode)))
    }

    pub fn make_basegc(stack_index: StackIndex, mode: MemberOpMode) -> Self {
        Self::make_instr(Instruct::IBase(InstructBase::BaseGC(stack_index, mode)))
    }

    pub fn make_basesc(y: StackIndex, z: StackIndex, mode: MemberOpMode) -> Self {
        Self::make_instr(Instruct::IBase(InstructBase::BaseSC(y, z, mode)))
    }

    pub fn make_baseh() -> Self {
        Self::make_instr(Instruct::IBase(InstructBase::BaseH))
    }

    pub fn make_cgetcunop() -> Self {
        Self::make_instr(Instruct::IMisc(InstructMisc::CGetCUNop))
    }

    pub fn make_ugetcunop() -> Self {
        Self::make_instr(Instruct::IMisc(InstructMisc::UGetCUNop))
    }

    pub fn make_memoget(label: Label, range: Option<(local::Type, isize)>) -> Self {
        Self::make_instr(Instruct::IMisc(InstructMisc::MemoGet(label, range)))
    }

    pub fn make_memoget_eager(
        label1: Label,
        label2: Label,
        range: Option<(local::Type, isize)>,
    ) -> Self {
        Self::make_instr(Instruct::IMisc(InstructMisc::MemoGetEager(
            label1, label2, range,
        )))
    }

    pub fn make_memoset(range: Option<(local::Type, isize)>) -> Self {
        Self::make_instr(Instruct::IMisc(InstructMisc::MemoSet(range)))
    }

    pub fn make_memoset_eager(range: Option<(local::Type, isize)>) -> Self {
        Self::make_instr(Instruct::IMisc(InstructMisc::MemoSetEager(range)))
    }

    pub fn make_getmemokeyl(local: local::Type) -> Self {
        Self::make_instr(Instruct::IMisc(InstructMisc::GetMemoKeyL(local)))
    }

    pub fn make_barethis(notice: BareThisOp) -> Self {
        Self::make_instr(Instruct::IMisc(InstructMisc::BareThis(notice)))
    }

    pub fn make_checkthis() -> Self {
        Self::make_instr(Instruct::IMisc(InstructMisc::CheckThis))
    }

    pub fn make_verify_ret_type_c() -> Self {
        Self::make_instr(Instruct::IMisc(InstructMisc::VerifyRetTypeC))
    }

    pub fn make_verify_ret_type_ts() -> Self {
        Self::make_instr(Instruct::IMisc(InstructMisc::VerifyRetTypeTS))
    }

    pub fn make_verify_out_type(i: ParamId) -> Self {
        Self::make_instr(Instruct::IMisc(InstructMisc::VerifyOutType(i)))
    }

    pub fn make_verify_param_type(i: ParamId) -> Self {
        Self::make_instr(Instruct::IMisc(InstructMisc::VerifyParamType(i)))
    }

    pub fn make_verify_param_type_ts(i: ParamId) -> Self {
        Self::make_instr(Instruct::IMisc(InstructMisc::VerifyParamTypeTS(i)))
    }

    pub fn make_dim(op: MemberOpMode, key: MemberKey) -> Self {
        Self::make_instr(Instruct::IBase(InstructBase::Dim(op, key)))
    }

    pub fn make_dim_warn_pt(key: PropId) -> Self {
        Self::make_dim(MemberOpMode::Warn, MemberKey::PT(key))
    }

    pub fn make_dim_define_pt(key: PropId) -> Self {
        Self::make_dim(MemberOpMode::Define, MemberKey::PT(key))
    }

    pub fn make_fcallclsmethod(
        is_log_as_dynamic_call: IsLogAsDynamicCallOp,
        fcall_args: FcallArgs,
    ) -> Self {
        Self::make_instr(Instruct::ICall(InstructCall::FCallClsMethod(
            fcall_args,
            is_log_as_dynamic_call,
        )))
    }

    pub fn make_fcallclsmethodd(
        fcall_args: FcallArgs,
        method_name: MethodId,
        class_name: ClassId,
    ) -> Self {
        Self::make_instr(Instruct::ICall(InstructCall::FCallClsMethodD(
            fcall_args,
            class_name,
            method_name,
        )))
    }

    pub fn make_fcallclsmethods(fcall_args: FcallArgs, scref: SpecialClsRef) -> Self {
        Self::make_instr(Instruct::ICall(InstructCall::FCallClsMethodS(
            fcall_args, scref,
        )))
    }

    pub fn make_fcallclsmethodsd(
        fcall_args: FcallArgs,
        scref: SpecialClsRef,
        method_name: MethodId,
    ) -> Self {
        Self::make_instr(Instruct::ICall(InstructCall::FCallClsMethodSD(
            fcall_args,
            scref,
            method_name,
        )))
    }

    pub fn make_fcallctor(fcall_args: FcallArgs) -> Self {
        Self::make_instr(Instruct::ICall(InstructCall::FCallCtor(fcall_args)))
    }

    pub fn make_fcallfunc(fcall_args: FcallArgs) -> Self {
        Self::make_instr(Instruct::ICall(InstructCall::FCallFunc(fcall_args)))
    }

    pub fn make_fcallfuncd(fcall_args: FcallArgs, id: FunctionId) -> Self {
        Self::make_instr(Instruct::ICall(InstructCall::FCallFuncD(fcall_args, id)))
    }

    pub fn make_fcallobjmethod(fcall_args: FcallArgs, flavor: ObjNullFlavor) -> Self {
        Self::make_instr(Instruct::ICall(InstructCall::FCallObjMethod(
            fcall_args, flavor,
        )))
    }

    pub fn make_fcallobjmethodd(
        fcall_args: FcallArgs,
        method: MethodId,
        flavor: ObjNullFlavor,
    ) -> Self {
        Self::make_instr(Instruct::ICall(InstructCall::FCallObjMethodD(
            fcall_args, flavor, method,
        )))
    }

    pub fn make_fcallobjmethodd_nullthrows(fcall_args: FcallArgs, method: MethodId) -> Self {
        Self::make_fcallobjmethodd(fcall_args, method, ObjNullFlavor::NullThrows)
    }

    pub fn make_querym(num_params: NumParams, op: QueryOp, key: MemberKey) -> Self {
        Self::make_instr(Instruct::IFinal(InstructFinal::QueryM(num_params, op, key)))
    }

    pub fn make_querym_cget_pt(num_params: NumParams, key: PropId) -> Self {
        Self::make_querym(num_params, QueryOp::CGet, MemberKey::PT(key))
    }

    pub fn make_setm(num_params: NumParams, key: MemberKey) -> Self {
        Self::make_instr(Instruct::IFinal(InstructFinal::SetM(num_params, key)))
    }

    pub fn make_unsetm(num_params: NumParams, key: MemberKey) -> Self {
        Self::make_instr(Instruct::IFinal(InstructFinal::UnsetM(num_params, key)))
    }

    pub fn make_setopm(num_params: NumParams, op: EqOp, key: MemberKey) -> Self {
        Self::make_instr(Instruct::IFinal(InstructFinal::SetOpM(num_params, op, key)))
    }

    pub fn make_incdecm(num_params: NumParams, op: IncdecOp, key: MemberKey) -> Self {
        Self::make_instr(Instruct::IFinal(InstructFinal::IncDecM(
            num_params, op, key,
        )))
    }

    pub fn make_setm_pt(num_params: NumParams, key: PropId) -> Self {
        Self::make_setm(num_params, MemberKey::PT(key))
    }

    pub fn make_resolve_func(func_id: FunctionId) -> Self {
        Self::make_instr(Instruct::IOp(InstructOperator::ResolveFunc(func_id)))
    }

    pub fn make_resolve_obj_method() -> Self {
        Self::make_instr(Instruct::IOp(InstructOperator::ResolveObjMethod))
    }

    pub fn make_resolveclsmethod(method_id: MethodId) -> Self {
        Self::make_instr(Instruct::IOp(InstructOperator::ResolveClsMethod(method_id)))
    }

    pub fn make_resolveclsmethodd(class_id: ClassId, method_id: MethodId) -> Self {
        Self::make_instr(Instruct::IOp(InstructOperator::ResolveClsMethodD(
            class_id, method_id,
        )))
    }

    pub fn make_resolveclsmethods(scref: SpecialClsRef, method_id: MethodId) -> Self {
        Self::make_instr(Instruct::IOp(InstructOperator::ResolveClsMethodS(
            scref, method_id,
        )))
    }

    pub fn make_fatal(op: FatalOp) -> Self {
        Self::make_instr(Instruct::IOp(InstructOperator::Fatal(op)))
    }

    pub fn make_await() -> Self {
        Self::make_instr(Instruct::IAsync(AsyncFunctions::Await))
    }

    pub fn make_yield() -> Self {
        Self::make_instr(Instruct::IGenerator(GenCreationExecution::Yield))
    }

    pub fn make_yieldk() -> Self {
        Self::make_instr(Instruct::IGenerator(GenCreationExecution::YieldK))
    }

    pub fn make_createcont() -> Self {
        Self::make_instr(Instruct::IGenerator(GenCreationExecution::CreateCont))
    }

    pub fn make_awaitall(range: Option<(local::Type, isize)>) -> Self {
        Self::make_instr(Instruct::IAsync(AsyncFunctions::AwaitAll(range)))
    }

    pub fn make_label(label: Label) -> Self {
        Self::make_instr(Instruct::ILabel(label))
    }

    pub fn make_awaitall_list(unnamed_locals: Vec<local::Type>) -> Self {
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
                    Self::make_awaitall(Some((
                        Unnamed(*hd_id),
                        unnamed_locals.len().try_into().unwrap(),
                    )))
                } else {
                    panic!("Expected unnamed local")
                }
            }
        }
    }

    pub fn make_exit() -> Self {
        Self::make_instr(Instruct::IOp(InstructOperator::Exit))
    }

    pub fn make_idx() -> Self {
        Self::make_instr(Instruct::IMisc(InstructMisc::Idx))
    }

    pub fn make_array_idx() -> Self {
        Self::make_instr(Instruct::IMisc(InstructMisc::ArrayIdx))
    }

    pub fn make_createcl(param_num: NumParams, cls_num: ClassNum) -> Self {
        Self::make_instr(Instruct::IMisc(InstructMisc::CreateCl(param_num, cls_num)))
    }

    pub fn make_fcallbuiltin(
        n: NumParams,
        un: NumParams,
        io: NumParams,
        s: impl Into<String>,
    ) -> Self {
        Self::make_instr(Instruct::ICall(InstructCall::FCallBuiltin(
            n,
            un,
            io,
            s.into(),
        )))
    }

    pub fn make_defcls(n: ClassNum) -> Self {
        Self::make_instr(Instruct::IIncludeEvalDefine(
            InstructIncludeEvalDefine::DefCls(n),
        ))
    }

    pub fn make_defclsnop(n: ClassNum) -> Self {
        Self::make_instr(Instruct::IIncludeEvalDefine(
            InstructIncludeEvalDefine::DefClsNop(n),
        ))
    }

    pub fn make_defrecord(n: ClassNum) -> Self {
        Self::make_instr(Instruct::IIncludeEvalDefine(
            InstructIncludeEvalDefine::DefRecord(n),
        ))
    }

    pub fn make_deftypealias(n: ClassNum) -> Self {
        Self::make_instr(Instruct::IIncludeEvalDefine(
            InstructIncludeEvalDefine::DefTypeAlias(n),
        ))
    }

    pub fn make_defcns(n: ConstNum) -> Self {
        Self::make_instr(Instruct::IIncludeEvalDefine(
            InstructIncludeEvalDefine::DefCns(n),
        ))
    }

    pub fn make_eval() -> Self {
        Self::make_instr(Instruct::IIncludeEvalDefine(
            InstructIncludeEvalDefine::Eval,
        ))
    }

    pub fn make_incl() -> Self {
        Self::make_instr(Instruct::IIncludeEvalDefine(
            InstructIncludeEvalDefine::Incl,
        ))
    }

    pub fn make_inclonce() -> Self {
        Self::make_instr(Instruct::IIncludeEvalDefine(
            InstructIncludeEvalDefine::InclOnce,
        ))
    }

    pub fn make_req() -> Self {
        Self::make_instr(Instruct::IIncludeEvalDefine(InstructIncludeEvalDefine::Req))
    }

    pub fn make_reqdoc() -> Self {
        Self::make_instr(Instruct::IIncludeEvalDefine(
            InstructIncludeEvalDefine::ReqDoc,
        ))
    }

    pub fn make_reqonce() -> Self {
        Self::make_instr(Instruct::IIncludeEvalDefine(
            InstructIncludeEvalDefine::ReqOnce,
        ))
    }

    pub fn make_silence_start(local: local::Type) -> Self {
        Self::make_instr(Instruct::IMisc(InstructMisc::Silence(
            local,
            OpSilence::Start,
        )))
    }

    pub fn make_silence_end(local: local::Type) -> Self {
        Self::make_instr(Instruct::IMisc(InstructMisc::Silence(
            local,
            OpSilence::End,
        )))
    }

    pub fn make_cont_assign_delegate(iter: IterId) -> Self {
        Self::make_instr(Instruct::IGenDelegation(GenDelegation::ContAssignDelegate(
            iter,
        )))
    }

    pub fn make_cont_enter_delegate() -> Self {
        Self::make_instr(Instruct::IGenDelegation(GenDelegation::ContEnterDelegate))
    }

    pub fn make_yield_from_delegate(iter: IterId, l: Label) -> Self {
        Self::make_instr(Instruct::IGenDelegation(GenDelegation::YieldFromDelegate(
            iter, l,
        )))
    }

    pub fn make_cont_unset_delegate_free(iter: IterId) -> Self {
        Self::make_instr(Instruct::IGenDelegation(GenDelegation::ContUnsetDelegate(
            FreeIterator::FreeIter,
            iter,
        )))
    }

    pub fn make_cont_unset_delegate_ignore(iter: IterId) -> Self {
        Self::make_instr(Instruct::IGenDelegation(GenDelegation::ContUnsetDelegate(
            FreeIterator::IgnoreIter,
            iter,
        )))
    }

    pub fn make_contcheck_check() -> Self {
        Self::make_instr(Instruct::IGenerator(GenCreationExecution::ContCheck(
            CheckStarted::CheckStarted,
        )))
    }

    pub fn make_contcheck_ignore() -> Self {
        Self::make_instr(Instruct::IGenerator(GenCreationExecution::ContCheck(
            CheckStarted::IgnoreStarted,
        )))
    }

    pub fn make_contenter() -> Self {
        Self::make_instr(Instruct::IGenerator(GenCreationExecution::ContEnter))
    }

    pub fn make_contraise() -> Self {
        Self::make_instr(Instruct::IGenerator(GenCreationExecution::ContRaise))
    }

    pub fn make_contvalid() -> Self {
        Self::make_instr(Instruct::IGenerator(GenCreationExecution::ContValid))
    }

    pub fn make_contcurrent() -> Self {
        Self::make_instr(Instruct::IGenerator(GenCreationExecution::ContCurrent))
    }

    pub fn make_contkey() -> Self {
        Self::make_instr(Instruct::IGenerator(GenCreationExecution::ContKey))
    }

    pub fn make_contgetreturn() -> Self {
        Self::make_instr(Instruct::IGenerator(GenCreationExecution::ContGetReturn))
    }

    pub fn make_trigger_sampled_error() -> Self {
        Self::make_fcallbuiltin(3, 3, 0, String::from("trigger_sampled_error"))
    }

    pub fn make_nativeimpl() -> Self {
        Self::make_instr(Instruct::IMisc(InstructMisc::NativeImpl))
    }

    pub fn make_srcloc(
        line_begin: isize,
        line_end: isize,
        col_begin: isize,
        col_end: isize,
    ) -> Self {
        Self::make_instr(Instruct::ISrcLoc(Srcloc {
            line_begin,
            line_end,
            col_begin,
            col_end,
        }))
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
        Self::gather(vec![
            Self::make_instr(Instruct::ITry(InstructTry::TryCatchBegin)),
            try_instrs,
            Self::make_jmp(done_label.clone()),
            Self::make_instr(Instruct::ITry(InstructTry::TryCatchMiddle)),
            catch_instrs,
            if skip_throw {
                Self::Empty
            } else {
                Self::make_instr(Instruct::IContFlow(InstructControlFlow::Throw))
            },
            Self::make_instr(Instruct::ITry(InstructTry::TryCatchEnd)),
            Self::make_label(done_label.clone()),
        ])
    }

    fn get_or_put_label<'a>(
        label_gen: &mut label::Gen,
        name_label_map: &'a HashMap<String, Label>,
        name: String,
    ) -> (Label, &'a HashMap<String, Label>) {
        match name_label_map.get(&name) {
            Some(label) => (label.clone(), name_label_map),
            None => (label_gen.next_regular(), name_label_map),
        }
    }

    fn rewrite_user_labels_instr<'a>(
        label_gen: &mut label::Gen,
        instruction: &Instruct,
        name_label_map: &'a HashMap<String, Label>,
    ) -> (Instruct, &'a HashMap<String, Label>) {
        use Instruct::*;
        let mut get_result = |x| Self::get_or_put_label(label_gen, name_label_map, x);
        match instruction {
            IContFlow(InstructControlFlow::Jmp(Label::Named(name))) => {
                let (label, name_label_map) = get_result(name.to_string());
                (
                    Instruct::IContFlow(InstructControlFlow::Jmp(label)),
                    name_label_map,
                )
            }
            IContFlow(InstructControlFlow::JmpNS(Label::Named(name))) => {
                let (label, name_label_map) = get_result(name.to_string());
                (
                    Instruct::IContFlow(InstructControlFlow::JmpNS(label)),
                    name_label_map,
                )
            }
            IContFlow(InstructControlFlow::JmpZ(Label::Named(name))) => {
                let (label, name_label_map) = get_result(name.to_string());
                (
                    Instruct::IContFlow(InstructControlFlow::JmpZ(label)),
                    name_label_map,
                )
            }
            IContFlow(InstructControlFlow::JmpNZ(Label::Named(name))) => {
                let (label, name_label_map) = get_result(name.to_string());
                (
                    Instruct::IContFlow(InstructControlFlow::JmpNZ(label)),
                    name_label_map,
                )
            }
            ILabel(Label::Named(name)) => {
                let (label, name_label_map) = get_result(name.to_string());
                (ILabel(label), name_label_map)
            }
            i => (i.clone(), name_label_map),
        }
    }

    pub fn rewrite_user_labels(&mut self, label_gen: &mut label::Gen) {
        *self = Self::rewrite_user_labels_aux(label_gen, self, &HashMap::new()).0
    }

    fn rewrite_user_labels_aux<'a>(
        label_gen: &mut label::Gen,
        instrseq: &Self,
        name_label_map: &'a HashMap<String, Label>,
    ) -> (Self, &'a HashMap<String, Label>) {
        match &instrseq {
            Self::Empty => (Self::Empty, name_label_map),
            Self::One(instr) => {
                let (i, name_label_map) =
                    Self::rewrite_user_labels_instr(label_gen, instr, name_label_map);
                (Self::make_instr(i), name_label_map)
            }
            Self::Concat(instrseq) => {
                let folder = |(mut acc, map): (Vec<Self>, &'a HashMap<String, Label>),
                              seq: &Self| {
                    let (l, map) = Self::rewrite_user_labels_aux(label_gen, seq, map);
                    acc.push(l);
                    (acc, map)
                };
                let (instrseq, name_label_map) =
                    instrseq.iter().fold((vec![], name_label_map), folder);
                (Self::Concat(instrseq), name_label_map)
            }
            Self::List(l) => {
                let folder = |(mut acc, map): (Vec<Instruct>, &'a HashMap<String, Label>),
                              instr: &Instruct| {
                    let (i, map) = Self::rewrite_user_labels_instr(label_gen, instr, map);
                    acc.push(i);
                    (acc, map)
                };
                let (instrlst, name_label_map) = l.iter().fold((vec![], name_label_map), folder);
                (Self::List(instrlst), name_label_map)
            }
        }
    }

    fn is_srcloc(instruction: &Instruct) -> bool {
        match instruction {
            Instruct::ISrcLoc(_) => true,
            _ => false,
        }
    }

    pub fn first(&self) -> Option<&Instruct> {
        match self {
            Self::Empty => None,
            Self::One(i) => {
                if Self::is_srcloc(i) {
                    None
                } else {
                    Some(i)
                }
            }
            Self::List(l) => match l.iter().find(|&i| !Self::is_srcloc(i)) {
                Some(i) => Some(i),
                None => None,
            },
            Self::Concat(l) => l.iter().find_map(Self::first),
        }
    }

    pub fn is_empty(&self) -> bool {
        match self {
            Self::Empty => true,
            Self::One(i) => Self::is_srcloc(i),
            Self::List(l) => l.is_empty() || l.iter().all(Self::is_srcloc),
            Self::Concat(l) => l.iter().all(Self::is_empty),
        }
    }

    pub fn flat_map<F>(&self, f: &mut F) -> Self
    where
        F: FnMut(&Instruct) -> Vec<Instruct>,
    {
        match self {
            Self::Empty => Self::Empty,
            Self::One(instr) => match &f(&instr)[..] {
                [] => Self::Empty,
                [x] => Self::make_instr(x.clone()),
                xs => Self::List(xs.to_vec()),
            },
            Self::List(instr_lst) => {
                let newlst = instr_lst.iter().flat_map(|x| f(x)).collect::<Vec<_>>();
                Self::List(newlst)
            }
            Self::Concat(instrseq_lst) => {
                let newlst = instrseq_lst
                    .iter()
                    .map(|x| x.flat_map(f))
                    .collect::<Vec<_>>();
                Self::Concat(newlst)
            }
        }
    }

    pub fn flat_map_seq<F>(&self, f: &mut F) -> Self
    where
        F: FnMut(&Instruct) -> Self,
    {
        match self {
            Self::Empty => Self::Empty,
            Self::One(instr) => f(instr),
            Self::List(instr_lst) => {
                Self::Concat(instr_lst.iter().map(|x| f(x)).collect::<Vec<_>>())
            }
            Self::Concat(instrseq_lst) => Self::Concat(
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
            Self::Empty => init,
            Self::One(x) => f(init, x),
            Self::List(instr_lst) => instr_lst.iter().fold(init, f),
            Self::Concat(instrseq_lst) => {
                instrseq_lst.iter().fold(init, |acc, x| x.fold_left(f, acc))
            }
        }
    }

    pub fn filter_map<F>(&self, f: &mut F) -> Self
    where
        F: FnMut(&Instruct) -> Option<Instruct>,
    {
        match self {
            Self::Empty => Self::Empty,
            Self::One(x) => match f(x) {
                Some(x) => Self::make_instr(x),
                None => Self::Empty,
            },
            Self::List(instr_lst) => Self::List(instr_lst.iter().filter_map(f).collect::<Vec<_>>()),
            Self::Concat(instrseq_lst) => Self::Concat(
                instrseq_lst
                    .iter()
                    .map(|x| x.filter_map(f))
                    .collect::<Vec<_>>(),
            ),
        }
    }

    pub fn filter_map_mut<F>(&mut self, f: &mut F)
    where
        F: FnMut(&mut Instruct) -> Option<Instruct>,
    {
        match self {
            Self::Empty => (),
            Self::One(x) => {
                *self = match f(x) {
                    Some(x) => Self::make_instr(x),
                    None => Self::Empty,
                }
            }
            Self::List(instr_lst) => {
                *instr_lst = instr_lst.iter_mut().filter_map(f).collect::<Vec<_>>()
            }
            Self::Concat(instrseq_lst) => instrseq_lst.iter_mut().for_each(|x| x.filter_map_mut(f)),
        }
    }

    pub fn map_mut<F>(&mut self, f: &mut F)
    where
        F: FnMut(&mut Instruct),
    {
        match self {
            Self::Empty => (),
            Self::One(x) => f(x),
            Self::List(instr_lst) => instr_lst.iter_mut().for_each(f),
            Self::Concat(instrseq_lst) => instrseq_lst.iter_mut().for_each(|x| x.map_mut(f)),
        }
    }

    pub fn map_result_mut<F>(&mut self, f: &mut F) -> Result<()>
    where
        F: FnMut(&mut Instruct) -> Result<()>,
    {
        match self {
            Self::Empty => Ok(()),
            Self::One(x) => f(x),
            Self::List(instr_lst) => instr_lst.iter_mut().map(|x| f(x)).collect::<Result<()>>(),
            Self::Concat(instrseq_lst) => instrseq_lst
                .iter_mut()
                .map(|x| x.map_result_mut(f))
                .collect::<Result<()>>(),
        }
    }

    pub fn map<F>(&self, f: &mut F) -> Self
    where
        F: FnMut(Instruct) -> Instruct,
    {
        self.filter_map(&mut |x| Some(f(x.clone())))
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use pretty_assertions::assert_eq;

    #[test]
    fn iter() {
        let mk_i = || Instruct::IComment("".into());
        let empty = || InstrSeq::Empty;
        let one = || InstrSeq::make_instr(mk_i());
        let list0 = || InstrSeq::make_instrs(vec![]);
        let list1 = || InstrSeq::make_instrs(vec![mk_i()]);
        let list2 = || InstrSeq::make_instrs(vec![mk_i(), mk_i()]);
        let concat0 = || InstrSeq::Concat(vec![]);

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
    }
}
