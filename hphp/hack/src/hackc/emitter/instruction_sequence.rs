// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use hhbc::BytesId;
use hhbc::Instruct;
use hhbc::Pseudo;

/// The various from_X functions below take some kind of AST
/// (expression, statement, etc.) and produce what is logically a
/// sequence of instructions. This could be represented by a list, but
/// we wish to avoid the quadratic complexity associated with repeated
/// appending. So, we build a tree of instructions which can be
/// flattened when complete.
#[derive(Debug, Clone)]
pub enum InstrSeq {
    List(Vec<Instruct>),
    Concat(Vec<InstrSeq>),
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
struct IntoListIter {
    // Keeping top-of-stack in its own field for speed.
    top: std::vec::IntoIter<InstrSeq>,
    stack: Vec<std::vec::IntoIter<InstrSeq>>,
}

impl IntoListIter {
    pub fn new(iseq: InstrSeq) -> Self {
        Self {
            top: match iseq {
                InstrSeq::List(_) => vec![iseq].into_iter(),
                InstrSeq::Concat(s) => s.into_iter(),
            },
            stack: Vec::new(),
        }
    }
}

impl Iterator for IntoListIter {
    type Item = Vec<Instruct>;
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
struct ListIter<'i> {
    // Keeping top-of-stack in its own field for speed.
    top: std::slice::Iter<'i, InstrSeq>,
    stack: Vec<std::slice::Iter<'i, InstrSeq>>,
}

impl<'i> ListIter<'i> {
    fn new(iseq: &'i InstrSeq) -> Self {
        Self {
            top: match iseq {
                InstrSeq::List(_) => std::slice::from_ref(iseq).iter(),
                InstrSeq::Concat(s) => s.iter(),
            },
            stack: Vec::new(),
        }
    }
}

impl<'i> Iterator for ListIter<'i> {
    type Item = &'i [Instruct];
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
struct ListIterMut<'i> {
    top: std::slice::IterMut<'i, InstrSeq>,
    stack: Vec<std::slice::IterMut<'i, InstrSeq>>,
}

impl<'i> ListIterMut<'i> {
    fn new(iseq: &'i mut InstrSeq) -> Self {
        Self {
            top: match iseq {
                InstrSeq::List(_) => std::slice::from_mut(iseq).iter_mut(),
                InstrSeq::Concat(s) => s.iter_mut(),
            },
            stack: Vec::new(),
        }
    }
}

impl<'i> Iterator for ListIterMut<'i> {
    type Item = &'i mut Vec<Instruct>;
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
    use hhbc::AdataId;
    use hhbc::AsTypeStructExceptionKind;
    use hhbc::BareThisOp;
    use hhbc::BytesId;
    use hhbc::ClassGetCMode;
    use hhbc::ClassName;
    use hhbc::CollectionType;
    use hhbc::ConstName;
    use hhbc::ContCheckOp;
    use hhbc::Dummy;
    use hhbc::FCallArgs;
    use hhbc::FatalOp;
    use hhbc::FloatBits;
    use hhbc::FunctionName;
    use hhbc::IncDecOp;
    use hhbc::InitPropOp;
    use hhbc::Instruct;
    use hhbc::IsLogAsDynamicCallOp;
    use hhbc::IsTypeOp;
    use hhbc::IterArgs;
    use hhbc::IterId;
    use hhbc::Label;
    use hhbc::Local;
    use hhbc::LocalRange;
    use hhbc::MOpMode;
    use hhbc::MemberKey;
    use hhbc::MethodName;
    use hhbc::NumParams;
    use hhbc::OODeclExistsOp;
    use hhbc::ObjMethodOp;
    use hhbc::Opcode;
    use hhbc::PropName;
    use hhbc::Pseudo;
    use hhbc::QueryMOp;
    use hhbc::ReadonlyOp;
    use hhbc::RepoAuthType;
    use hhbc::SetOpOp;
    use hhbc::SetRangeOp;
    use hhbc::SilenceOp;
    use hhbc::SpecialClsRef;
    use hhbc::SrcLoc;
    use hhbc::StackIndex;
    use hhbc::SwitchKind;
    use hhbc::TypeStructEnforceKind;
    use hhbc::TypeStructResolveOp;

    use crate::InstrSeq;

    // This macro builds helper functions for each of the given opcodes.  See
    // the definition of define_instr_seq_helpers for details.
    emit_opcodes_macro::define_instr_seq_helpers! {
        // These get custom implementations below.
        FCallClsMethod | FCallClsMethodM | FCallClsMethodD | FCallClsMethodS | FCallClsMethodSD |
        FCallCtor | FCallObjMethod | FCallObjMethodD | MemoGetEager |
        NewStructDict | SSwitch | String | Switch => {}

        // These are "custom" names that don't match the simple snake-case of
        // their Opcodes.
        Await => await_,
        False => false_,
        Mod => mod_,
        True => true_,
        Yield => yield_,
    }

    pub fn empty() -> InstrSeq {
        InstrSeq::new_empty()
    }

    pub fn instr(i: Instruct) -> InstrSeq {
        InstrSeq::List(vec![i])
    }

    pub(crate) fn instrs(is: Vec<Instruct>) -> InstrSeq {
        InstrSeq::List(is)
    }

    // Special constructors for Opcode

    pub fn cont_check_check() -> InstrSeq {
        cont_check(ContCheckOp::CheckStarted)
    }

    pub fn cont_check_ignore() -> InstrSeq {
        cont_check(ContCheckOp::IgnoreStarted)
    }

    pub fn dim_warn_pt(key: PropName, readonly_op: ReadonlyOp) -> InstrSeq {
        dim(MOpMode::Warn, MemberKey::PT(key, readonly_op))
    }

    pub fn f_call_cls_method(log: IsLogAsDynamicCallOp, fcall_args: FCallArgs) -> InstrSeq {
        instr(Instruct::Opcode(Opcode::FCallClsMethod(
            fcall_args,
            BytesId::EMPTY,
            log,
        )))
    }

    pub fn f_call_cls_method_m(
        log: IsLogAsDynamicCallOp,
        fcall_args: FCallArgs,
        method: MethodName,
    ) -> InstrSeq {
        instr(Instruct::Opcode(Opcode::FCallClsMethodM(
            fcall_args,
            BytesId::EMPTY,
            log,
            method,
        )))
    }

    pub fn f_call_cls_method_d(
        fcall_args: FCallArgs,
        method: MethodName,
        class: ClassName,
    ) -> InstrSeq {
        instr(Instruct::Opcode(Opcode::FCallClsMethodD(
            fcall_args, class, method,
        )))
    }

    pub fn f_call_cls_method_s(fcall_args: FCallArgs, clsref: SpecialClsRef) -> InstrSeq {
        instr(Instruct::Opcode(Opcode::FCallClsMethodS(
            fcall_args,
            BytesId::EMPTY,
            clsref,
        )))
    }

    pub fn f_call_cls_method_sd(
        fcall_args: FCallArgs,
        clsref: SpecialClsRef,
        method: MethodName,
    ) -> InstrSeq {
        instr(Instruct::Opcode(Opcode::FCallClsMethodSD(
            fcall_args,
            BytesId::EMPTY,
            clsref,
            method,
        )))
    }

    pub fn f_call_ctor(fcall_args: FCallArgs) -> InstrSeq {
        instr(Instruct::Opcode(Opcode::FCallCtor(
            fcall_args,
            BytesId::EMPTY,
        )))
    }

    pub fn f_call_obj_method(fcall_args: FCallArgs, flavor: ObjMethodOp) -> InstrSeq {
        instr(Instruct::Opcode(Opcode::FCallObjMethod(
            fcall_args,
            BytesId::EMPTY,
            flavor,
        )))
    }

    pub fn f_call_obj_method_d_(
        fcall_args: FCallArgs,
        method: MethodName,
        flavor: ObjMethodOp,
    ) -> InstrSeq {
        instr(Instruct::Opcode(Opcode::FCallObjMethodD(
            fcall_args,
            BytesId::EMPTY,
            flavor,
            method,
        )))
    }

    pub fn f_call_obj_method_d(fcall_args: FCallArgs, method: MethodName) -> InstrSeq {
        f_call_obj_method_d_(fcall_args, method, ObjMethodOp::NullThrows)
    }

    pub fn iter_break(label: Label, iters: Vec<IterId>) -> InstrSeq {
        let mut vec: Vec<Instruct> = iters
            .into_iter()
            .map(|i| Instruct::Opcode(Opcode::IterFree(i)))
            .collect();
        vec.push(Instruct::Opcode(Opcode::Jmp(label)));
        instrs(vec)
    }

    pub fn memo_get_eager(label1: Label, label2: Label, range: LocalRange) -> InstrSeq {
        instr(Instruct::Opcode(Opcode::MemoGetEager(
            [label1, label2],
            // Need dummy immediate here to satisfy opcodes translator expectation of immediate
            // with name _0.
            Dummy::DEFAULT,
            range,
        )))
    }

    pub fn new_struct_dict(keys: &[&[u8]]) -> InstrSeq {
        let keys: Vec<hhbc::BytesId> = keys.iter().map(|&s| hhbc::intern_bytes(s)).collect();
        instr(Instruct::Opcode(Opcode::NewStructDict(keys.into())))
    }

    pub fn set_m_pt(num_params: NumParams, key: PropName, readonly_op: ReadonlyOp) -> InstrSeq {
        set_m(num_params, MemberKey::PT(key, readonly_op))
    }

    pub fn silence_end(local: Local) -> InstrSeq {
        silence(local, SilenceOp::End)
    }

    pub fn silence_start(local: Local) -> InstrSeq {
        silence(local, SilenceOp::Start)
    }

    pub fn s_switch(cases: Vec<(BytesId, Label)>) -> InstrSeq {
        let targets = Vec::from_iter(cases.iter().map(|(_, target)| *target)).into();
        let cases = Vec::from_iter(cases.into_iter().map(|(s, _)| s)).into();
        instr(Instruct::Opcode(Opcode::SSwitch(cases, targets)))
    }

    /// Make a String instruction when the litstr is already interned
    pub fn string_lit(litstr: BytesId) -> InstrSeq {
        instr(Instruct::Opcode(Opcode::String(litstr)))
    }

    /// Intern the given litstr, and make a String instruction
    pub fn string(litstr: impl AsRef<[u8]>) -> InstrSeq {
        string_lit(hhbc::intern_bytes(litstr.as_ref()))
    }

    pub fn switch(targets: Vec<Label>) -> InstrSeq {
        instr(Instruct::Opcode(Opcode::Switch(
            SwitchKind::Unbounded,
            0,
            targets.into(),
        )))
    }

    pub fn await_all_list(unnamed_locals: Vec<Local>) -> InstrSeq {
        match unnamed_locals.split_first() {
            None => panic!("Expected at least one await"),
            Some((head, tail)) => {
                // Assert that the Locals are sequentially numbered.
                let mut prev_id = head;
                for id in tail {
                    assert_eq!(prev_id.idx + 1, id.idx);
                    prev_id = id;
                }
                await_all(LocalRange {
                    start: *head,
                    len: unnamed_locals.len().try_into().unwrap(),
                })
            }
        }
    }

    // Special constructors for Pseudo
    pub fn break_() -> InstrSeq {
        instr(Instruct::Pseudo(Pseudo::Break))
    }

    pub fn continue_() -> InstrSeq {
        instr(Instruct::Pseudo(Pseudo::Continue))
    }

    pub fn label(label: Label) -> InstrSeq {
        instr(Instruct::Pseudo(Pseudo::Label(label)))
    }

    pub fn srcloc(
        line_begin: isize,
        line_end: isize,
        col_begin: isize,
        col_end: isize,
    ) -> InstrSeq {
        instr(Instruct::Pseudo(Pseudo::SrcLoc(SrcLoc {
            line_begin: line_begin as i32,
            line_end: line_end as i32,
            col_begin: col_begin as i32,
            col_end: col_end as i32,
        })))
    }
}

impl InstrSeq {
    /// Produce an empty instruction sequence.
    pub fn new_empty() -> Self {
        InstrSeq::List(Vec::new())
    }

    /// Transitional version. We mean to write a `gather!` in the future.
    pub fn gather(mut iss: Vec<InstrSeq>) -> Self {
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

    pub fn iter<'i>(&'i self) -> impl Iterator<Item = &'i Instruct> {
        ListIter::new(self).flatten()
    }

    pub fn iter_mut<'i>(&'i mut self) -> impl Iterator<Item = &'i mut Instruct> {
        ListIterMut::new(self).flatten()
    }

    fn full_len(&self) -> usize {
        ListIter::new(self).map(|s| s.len()).sum()
    }

    fn compact_tail(v: &mut [Instruct], mut start: usize) -> usize {
        if start > 0 && Self::is_srcloc(&v[start - 1]) {
            // previous compacted range ends with a SrcLoc;
            // back up so we can compact it if eligible.
            start -= 1;
        };
        let mut i = start;
        for j in start..v.len() {
            if Self::is_srcloc(&v[j]) && j + 1 < v.len() && Self::is_srcloc(&v[j + 1]) {
                // skip v[j]
            } else {
                if i < j {
                    // move v[j] -> v[i], leaving an arbitrary placeholder
                    // that will be overwritten or truncated.
                    v[i] = std::mem::replace(&mut v[j], Instruct::Pseudo(Pseudo::Break));
                }
                i += 1;
            }
        }
        i
    }

    /// Flatten self into a contiguous Vec of instructions, with SrcLocs compacted
    /// to only preserve the final SrcLoc in any consecutive sequence.
    pub fn to_vec(self) -> Vec<Instruct> {
        let mut v = Vec::with_capacity(self.full_len());
        for list in IntoListIter::new(self) {
            let start = v.len();
            v.extend(list);
            let end = Self::compact_tail(&mut v[..], start);
            v.truncate(end);
        }
        v
    }

    /// Test whether `instruction` is of case `Pseudo::SrcLoc`.
    fn is_srcloc(instruction: &Instruct) -> bool {
        matches!(instruction, Instruct::Pseudo(Pseudo::SrcLoc(_)))
    }

    /// Return the first non-SrcLoc instruction.
    pub fn first(&self) -> Option<&Instruct> {
        self.iter().find(|&i| !Self::is_srcloc(i))
    }

    /// Test for the empty instruction sequence, ignoring SrcLocs
    pub fn is_empty(&self) -> bool {
        self.iter().all(Self::is_srcloc)
    }

    pub fn retain(&mut self, mut f: impl FnMut(&Instruct) -> bool) {
        for s in ListIterMut::new(self) {
            s.retain(&mut f)
        }
    }

    pub fn retain_mut<F>(&mut self, mut f: F)
    where
        F: FnMut(&mut Instruct) -> bool,
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
    use instr::instr;
    use instr::instrs;
    use pretty_assertions::assert_eq;

    use super::*;

    #[test]
    fn iter() {
        let mk_i = || Instruct::Opcode(hhbc::Opcode::Nop);
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
