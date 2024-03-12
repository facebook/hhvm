use std::rc::Rc;

use anyhow::bail;
use anyhow::Context;
use anyhow::Result;
use hash::HashMap;
use hhbc::AdataId;
use hhbc::Instruct;
use hhbc::Label;
use hhbc::Local;
use hhbc::Opcode;
use hhbc::Pseudo;
use hhbc::SrcLoc;
use hhbc::StringId;
use hhbc::TypedValue;
use hhbc_gen::OpcodeData;
use newtype::IdVec;

use crate::code_path::CodePath;
use crate::instr_ptr::InstrPtr;
use crate::sequence::Sequence;
use crate::value::ValueBuilder;
use crate::work_queue::WorkQueue;

/// Compare two `hhbc::Body`s to figure out if they are semantically equivalent.
///
/// We don't want to simply require the two bodies to match exactly - that's
/// boring. Instead we allow their instruction sequences to drift and define
/// checkpoints where they need to match up.
///
/// We start by having a work queue that identifies pairs of related (left and
/// right side) `State`s. The most obvious starting state is at the entry IP
/// with an empty stack - but default parameters will add additional starting
/// states. We then loop until either we find a difference or the work queue is
/// empty.
///
/// At each loop step we take the pair of `State`s and use them to collect a
/// `Sequence` (see `State::collect()`) . A `Sequence` represents a series of
/// checkpoint instructions along with their abstract input values.  The
/// abstract input values are defined such that a set of inputs run through the
/// same Instruct will always produce the same value.
///
/// As we collect sequences any conditional control flow (like a JmpZ) adds new
/// pairs of state to the work queue.
///
/// Finally we compare the two sequences to make sure that they're equal or we
/// return an Err (see `Sequence::compare()`).
pub(crate) fn compare_bodies<'a>(
    path: &CodePath<'_>,
    body_a: &hhbc::Body,
    a_adata: &'a HashMap<AdataId, &'a TypedValue>,
    body_b: &hhbc::Body,
    b_adata: &'a HashMap<AdataId, &'a TypedValue>,
) -> Result<()> {
    let mut work_queue = WorkQueue::default();

    let a = Body::new(body_a);
    let b = Body::new(body_b);

    let mut value_builder = ValueBuilder::new();
    work_queue.init_from_bodies(&mut value_builder, &a, a_adata, &b, b_adata);

    // If we loop more than this number of times it's almost certainly a bug.
    let mut infinite_loop = std::cmp::max(body_a.body_instrs.len(), body_b.body_instrs.len()) * 5;

    while let Some((a, b)) = work_queue.pop() {
        let seq_a = a.collect(&mut value_builder).context("collecting a")?;
        let seq_b = b.collect(&mut value_builder).context("collecting b")?;

        Sequence::compare(path, seq_a.seq, seq_b.seq)?;

        debug_assert_eq!(seq_a.forks.len(), seq_a.forks.len());
        for (a, b) in seq_a.forks.into_iter().zip(seq_b.forks.into_iter()) {
            work_queue.add(a, b);
        }

        match (seq_a.catch_state, seq_b.catch_state) {
            (None, None) => {}
            (None, Some(_)) => {
                bail!("Mismatched catch blocks - 'a' doesn't have catch but 'b' does.",);
            }
            (Some(_), None) => {
                bail!("Mismatched catch blocks - 'a' has catch but 'b' does not.",);
            }
            (Some(mut catch_a), Some(mut catch_b)) => {
                let ex = value_builder.alloc();
                catch_a.stack.push(ex);
                catch_b.stack.push(ex);
                work_queue.add(catch_a, catch_b);
            }
        }

        infinite_loop -= 1;
        if infinite_loop == 0 {
            bail!("Looping out of control - almost certainly a sem_diff bug.");
        }
    }

    Ok(())
}

pub(crate) struct Body<'a> {
    pub(crate) hhbc_body: &'a hhbc::Body,
    pub(crate) label_to_ip: HashMap<Label, InstrPtr>,
    /// Mapping from InstrPtr to the InstrPtr of its catch block.
    try_catch: HashMap<InstrPtr, InstrPtr>,
    ip_to_loc: IdVec<InstrPtr, Rc<SrcLoc>>,
}

impl<'a> Body<'a> {
    fn new(hhbc_body: &'a hhbc::Body) -> Self {
        let (label_to_ip, ip_to_loc) = Self::compute_per_instr_info(hhbc_body);
        let try_catch = Self::compute_try_catch(hhbc_body);
        Body {
            hhbc_body,
            label_to_ip,
            ip_to_loc,
            try_catch,
        }
    }

    pub(crate) fn lookup_catch(&self, ip: InstrPtr) -> InstrPtr {
        self.try_catch.get(&ip).copied().unwrap_or(InstrPtr::None)
    }

    pub(crate) fn local_name(&self, local: Local) -> Option<StringId> {
        let mut n = local.as_usize();
        let p = self.hhbc_body.params.len();
        if n < p {
            return Some(self.hhbc_body.params[n].name);
        }
        n -= p;
        let v = self.hhbc_body.decl_vars.len();
        if n < v {
            return Some(self.hhbc_body.decl_vars[n]);
        }
        None
    }

    fn compute_per_instr_info(
        hhbc_body: &'a hhbc::Body,
    ) -> (HashMap<Label, InstrPtr>, IdVec<InstrPtr, Rc<SrcLoc>>) {
        let mut label_to_ip = HashMap::default();
        let mut ip_to_loc = Vec::with_capacity(hhbc_body.body_instrs.len());
        let mut cur_loc = Rc::new(SrcLoc::default());
        for (ip, instr) in body_instrs(hhbc_body) {
            match instr {
                Instruct::Pseudo(Pseudo::SrcLoc(src_loc)) => cur_loc = Rc::new(src_loc.clone()),
                Instruct::Pseudo(Pseudo::Label(label)) => {
                    label_to_ip.insert(*label, ip);
                }
                _ => {}
            }
            ip_to_loc.push(cur_loc.clone());
        }
        (label_to_ip, IdVec::new_from_vec(ip_to_loc))
    }

    /// Compute a mapping from InstrPtrs to their catch target.
    fn compute_try_catch(hhbc_body: &'a hhbc::Body) -> HashMap<InstrPtr, InstrPtr> {
        let mut cur: Vec<InstrPtr> = Vec::new();
        let mut mapping: HashMap<InstrPtr, InstrPtr> = HashMap::default();

        for (ip, instr) in body_instrs(hhbc_body).rev() {
            match instr {
                Instruct::Pseudo(Pseudo::TryCatchBegin) => {
                    cur.pop();
                }
                Instruct::Pseudo(Pseudo::TryCatchMiddle) => {
                    cur.push(ip);
                }
                Instruct::Pseudo(Pseudo::TryCatchEnd) => {}
                _ => {
                    if let Some(cur) = cur.last() {
                        mapping.insert(ip, *cur);
                    }
                }
            }
        }

        mapping
    }

    pub(crate) fn ip_to_loc(&self, ip: InstrPtr) -> &Rc<SrcLoc> {
        &self.ip_to_loc[ip]
    }
}

fn body_instrs<'a>(
    hhbc_body: &'a hhbc::Body,
) -> impl DoubleEndedIterator<Item = (InstrPtr, &'a Instruct)> {
    hhbc_body.body_instrs.iter().enumerate().map(|(i, instr)| {
        let ip = InstrPtr::from_usize(i);
        (ip, instr)
    })
}

pub(crate) fn lookup_data_for_opcode(opcode: &Opcode) -> &'static OpcodeData {
    hhbc_gen::opcode_data().get(opcode.variant_index()).unwrap()
}
