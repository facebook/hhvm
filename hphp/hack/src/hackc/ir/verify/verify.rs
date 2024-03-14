// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

//! Verifier for HackIR.  This traverses the HackIR structures and makes sure
//! that the required invariants still hold true.

use std::collections::hash_map::Entry;
use std::fmt::Display;

use analysis::PredecessorFlags;
use analysis::Predecessors;
use ir_core::instr::HasEdges;
use ir_core::instr::HasOperands;
use ir_core::instr::Hhbc;
use ir_core::instr::IrToBc;
use ir_core::instr::Special;
use ir_core::instr::Terminator;
use ir_core::Block;
use ir_core::BlockId;
use ir_core::BlockIdMap;
use ir_core::FullInstrId;
use ir_core::Func;
use ir_core::Instr;
use ir_core::InstrId;
use ir_core::InstrIdSet;
use ir_core::ValueId;
use itertools::Itertools;
use print::FmtBid;
use print::FmtRawVid;
use print::FmtVid;

/// Flags controlling the details of verification.
#[derive(Default)]
pub struct Flags {
    pub allow_critical_edges: bool,
}

/// Asserts that a condition is true and reports if it's not.
///
/// ```
/// check!(self, mycond, "mycond should be true but it's {mycond}");
/// ```
macro_rules! check {
    ($self:expr, $cond:expr, $($why:expr),+ $(,)? ) => {{
        let pred = $cond;
        if (!pred) {
            $self.check_failed(&format!($($why),+));
        }
    }}
}

/// Fail a verify.
///
/// ```
/// check_failed!("mycond failed to verify with value {mycond}");
/// ```
macro_rules! check_failed {
    ($self:expr, $($why:expr),+ $(,)? ) => {{
        $self.check_failed(&format!($($why),+))
    }}
}

struct VerifyFunc<'b> {
    // Mapping of what InstrIds dominate each Block.
    dominated_iids: BlockIdMap<InstrIdSet>,
    func: &'b Func,
    #[allow(dead_code)]
    flags: &'b Flags,
    predecessors: Predecessors,
}

impl<'b> VerifyFunc<'b> {
    fn new(func: &'b Func, flags: &'b Flags) -> Self {
        // Catch unwind so we can print the function before continuing the
        // panic.
        let predecessors = std::panic::catch_unwind(|| {
            analysis::compute_predecessor_blocks(func, PredecessorFlags::default())
        });
        let predecessors = match predecessors {
            Ok(predecessors) => predecessors,
            Err(e) => {
                Self::report_func_error("compute_predecessor_blocks panic'd", func, None, None);
                std::panic::resume_unwind(e);
            }
        };
        VerifyFunc {
            dominated_iids: Default::default(),
            func,
            flags,
            predecessors,
        }
    }

    fn report_func_error(
        why: &str,
        func: &Func,
        predecessors: Option<&Predecessors>,
        dominated_iids: Option<&BlockIdMap<InstrIdSet>>,
    ) {
        eprintln!("VERIFY FAILED: {why}");
        use print::FmtSep;

        let f_pre_block = |w: &mut dyn std::fmt::Write, bid| {
            if let Some(pds) = predecessors.as_ref().and_then(|p| p.get(&bid)) {
                writeln!(
                    w,
                    "  Predecessor BlockIds: [{}]",
                    FmtSep::comma(pds.iter().sorted(), |w, bid| FmtBid(func, *bid, true)
                        .fmt(w))
                )?;
            }
            if let Some(dids) = dominated_iids.as_ref().and_then(|d| d.get(&bid)) {
                writeln!(
                    w,
                    "  Dominator InstrIds: [{}]",
                    FmtSep::comma(dids.iter().sorted(), |w, vid| FmtVid(
                        func,
                        (*vid).into(),
                        true,
                    )
                    .fmt(w))
                )?;
            }
            Ok(())
        };
        let mut df = print::DisplayFunc::new(func, true);
        df.f_pre_block = Some(&f_pre_block);
        eprintln!("{}", df);
    }

    fn check_failed(&self, why: &str) -> ! {
        Self::report_func_error(
            why,
            self.func,
            Some(&self.predecessors),
            Some(&self.dominated_iids),
        );
        panic!("VERIFY FAILED: {}", why);
    }

    fn verify_block(&mut self, bid: BlockId, block: &Block) {
        check!(
            self,
            block.is_terminated(self.func),
            "block {} is unterminated",
            bid
        );

        // This requires RPO to work properly!
        let mut dominated_iids = std::mem::take(self.dominated_iids.entry(bid).or_default());
        for &iid in &block.params {
            dominated_iids.insert(iid);
        }

        for &iid in &block.params {
            let instr = self.func.instr(iid);
            check!(
                self,
                instr.is_param(),
                "block {} param {} is not a Param",
                FmtBid(self.func, bid, true),
                iid
            );
        }

        let terminator_iid = block.terminator_iid();
        for (iid_idx, iid) in block.iids().enumerate() {
            let instr = self.func.instr(iid);

            // Only the last Instr should be terminal.
            if iid == terminator_iid {
                check!(
                    self,
                    instr.is_terminal(),
                    "terminator iid {} in block {} is non-terminal",
                    iid,
                    bid
                );
            } else {
                check!(
                    self,
                    !instr.is_terminal(),
                    "non-terminator iid {} in block {} is terminal",
                    iid,
                    bid
                );
            }

            self.verify_instr(
                block,
                iid,
                iid_idx,
                self.func.instr(iid),
                &mut dominated_iids,
            );
        }

        // Push the dominated IIDs into the successors.
        for &edge in self.func.edges(bid) {
            self.push_dominated_iids(edge, &dominated_iids);
        }
        let handler = self.func.catch_target(bid);
        if handler != BlockId::NONE {
            // Push the dominated IIDs into the catch handler.
            self.push_dominated_iids(handler, &dominated_iids);
        }
        self.dominated_iids.insert(bid, dominated_iids);

        if !self.flags.allow_critical_edges {
            // If we have multiple successors then our successors must all have
            // only a single predecessor.
            let i = self.func.terminator(bid);
            let successors = i.edges();
            if successors.len() > 1 {
                for t in successors {
                    check!(
                        self,
                        self.predecessors[t].len() == 1,
                        "Block {bid} successor {t} is a critical edge! ({bid} has {} successors, {t} has {} predecessors)",
                        successors.len(),
                        self.predecessors[t].len()
                    );
                }
            }
        }
    }

    fn push_dominated_iids(&mut self, bid: BlockId, dominated_iids: &InstrIdSet) {
        match self.dominated_iids.entry(bid) {
            Entry::Vacant(e) => {
                e.insert(dominated_iids.clone());
            }
            Entry::Occupied(mut e) => {
                e.get_mut().retain(|iid| dominated_iids.contains(iid));
            }
        }
    }

    fn verify_func_body(&mut self) {
        self.verify_func_unique_iids();

        // Check that the body blocks are in strict RPO order.
        let rpo = analysis::compute_rpo(self.func);
        check!(
            self,
            rpo == self.func.block_ids().collect_vec(),
            "Func Blocks are not in RPO order.  Expected\n[{}]\nbut got\n[{}]",
            rpo.iter()
                .map(|&bid| FmtBid(self.func, bid, true).to_string())
                .collect_vec()
                .join(", "),
            self.func
                .block_ids()
                .map(|bid| FmtBid(self.func, bid, true).to_string())
                .collect_vec()
                .join(", "),
        );

        for bid in self.func.block_ids() {
            self.verify_block(bid, self.func.block(bid));
        }
    }

    fn verify_func_unique_iids(&self) {
        // InstrIds should never be used twice (although zero times is valid for
        // dead InstrIds).
        let mut seen = InstrIdSet::default();
        for iid in self.func.body_iids() {
            check!(
                self,
                seen.insert(iid),
                "iid {} used twice in Func body",
                iid
            );
        }
    }

    fn verify_operand(
        &self,
        src_iid: InstrId,
        op_vid: ValueId,
        op_idx: usize,
        dominated_iids: &InstrIdSet,
    ) {
        match op_vid.full() {
            FullInstrId::Imm(cid) => {
                check!(
                    self,
                    self.func.imms.get(cid).is_some(),
                    "iid {} refers to missing constant {}",
                    src_iid,
                    cid
                )
            }
            FullInstrId::Instr(op_iid) => {
                let op_instr = self.func.get_instr(op_iid);
                // Technically this is a subset of the following domination
                // check - but gives a better error message.
                check!(
                    self,
                    op_instr.is_some(),
                    "iid {} refers to missing instr {}",
                    src_iid,
                    op_iid
                );
                check!(
                    self,
                    dominated_iids.contains(&op_iid),
                    "iid {} doesn't dominate use at {}",
                    FmtVid(self.func, op_vid, true),
                    FmtVid(self.func, ValueId::from_instr(src_iid), true),
                );
                match op_instr.unwrap() {
                    Instr::Special(Special::Tombstone) => {
                        // We should never see a tombstone as part of the graph.
                        check_failed!(
                            self,
                            "iid {} operand {} ({})is a tombstone",
                            src_iid,
                            op_idx,
                            FmtVid(self.func, op_vid, true)
                        );
                    }
                    _ => {}
                }
            }
            FullInstrId::None => {
                check_failed!(self, "iid {} operand {} is none", src_iid, op_idx);
            }
        }
    }

    fn verify_block_args(&self, iid: InstrId, bid: BlockId, args: usize) {
        // Make sure that the target block is expecting the args.
        let expected_args = self.func.block(bid).params.len();
        check!(
            self,
            expected_args == args,
            "iid {iid} is a jump with {args} args to block {bid} expecting {expected_args} args",
        );
    }

    fn verify_selects(&self, n: usize, block: &Block, iid: InstrId, iid_idx: usize) {
        for i in 1..=n {
            let select_iid = if let Some(iid) = block.iids.get(iid_idx + i) {
                *iid
            } else {
                check_failed!(
                    self,
                    "iid {} returns {} values but block doesn't contain enough iids (only followed by {})",
                    iid,
                    n,
                    i - 1
                )
            };

            check!(
                self,
                matches!(
                    self.func.instr(select_iid),
                    Instr::Special(Special::Select(..))
                ),
                "iid {} returns {} values but is only followed by {} selects",
                iid,
                n,
                i - 1
            );
        }
    }

    fn verify_select(
        &self,
        iid: InstrId,
        iid_idx: usize,
        prev_vid: Option<ValueId>,
        target_vid: ValueId,
        idx: u32,
    ) {
        // The select instruction must follow either another select for the
        // same target or the target it's selecting from.
        check!(
            self,
            iid_idx > 0,
            "iid {} select cannot be first instr in block",
            iid
        );
        let prev_vid = prev_vid.unwrap();
        let prev_instr = self.func.instr(prev_vid.expect_instr("instr expected"));
        if prev_vid != target_vid {
            // If the previous vid is not our target then it must be a
            // select with the same target as us.
            match *prev_instr {
                Instr::Special(Special::Select(prev_target, prev_idx)) => {
                    if prev_target != target_vid {
                        check_failed!(
                            self,
                            "iid {} select with target {} follows a select with a different target instr {}",
                            iid,
                            FmtRawVid(target_vid),
                            FmtRawVid(prev_target)
                        );
                    }
                    if prev_idx + 1 != idx {
                        check_failed!(
                            self,
                            "iid {} select with index {} follows a non-contiguous select index {}",
                            iid,
                            idx,
                            prev_idx
                        );
                    }
                }
                _ => {
                    check_failed!(self, "iid {} select is not contiguous with its target", iid);
                }
            }
        } else {
            // Check that the previous instruction expects a select. The actual
            // number of expected selects was checked when we saw the previous
            // instruction.
            let expects_select = match *prev_instr {
                Instr::Call(ref call) => call.num_rets >= 2,
                Instr::Hhbc(Hhbc::ClassGetTS(..)) => true,
                Instr::MemberOp(ref op) => op.num_values() >= 2,
                _ => false,
            };
            if !expects_select {
                check_failed!(
                    self,
                    "iid {} select follows non-selectable instr {:?}",
                    iid,
                    prev_instr
                );
            }
        }
    }

    fn verify_instr(
        &self,
        block: &Block,
        iid: InstrId,
        iid_idx: usize,
        instr: &Instr,
        dominated_iids: &mut InstrIdSet,
    ) {
        // Ensure that all operands exist.
        for (op_idx, op_vid) in instr.operands().iter().copied().enumerate() {
            self.verify_operand(iid, op_vid, op_idx, dominated_iids);
        }

        dominated_iids.insert(iid);

        // Ensure all edges are valid.
        for &bid in instr.edges() {
            check!(
                self,
                self.func.blocks.get(bid).is_some(),
                "iid {} refers to missing block {}",
                iid,
                bid
            );
        }

        let prev_vid = if iid_idx > 0 {
            block
                .iids
                .get(iid_idx - 1)
                .copied()
                .map(ValueId::from_instr)
        } else {
            None
        };

        // Special cases for individual instructions.
        match *instr {
            Instr::Call(ref call) => {
                if call.num_rets > 1 {
                    self.verify_selects(call.num_rets as usize, block, iid, iid_idx);
                }
            }
            Instr::Hhbc(Hhbc::ClassGetTS(..)) => {
                self.verify_selects(2, block, iid, iid_idx);
            }
            Instr::MemberOp(ref op) => {
                let num_rets = op.num_values();
                if num_rets > 1 {
                    self.verify_selects(num_rets, block, iid, iid_idx);
                }
            }
            Instr::Terminator(Terminator::CallAsync(_, targets)) => {
                self.verify_block_args(iid, targets[0], 1);
                self.verify_block_args(iid, targets[1], 1);
            }
            Instr::Terminator(Terminator::Enter(bid, _) | Terminator::Jmp(bid, _)) => {
                self.verify_block_args(iid, bid, 0);
            }
            Instr::Terminator(Terminator::JmpArgs(bid, ref vids, _)) => {
                check!(
                    self,
                    !vids.is_empty(),
                    "iid {} is JmpArgs with no args!",
                    iid
                );
                self.verify_block_args(iid, bid, vids.len());
            }
            Instr::Terminator(Terminator::JmpOp { targets, .. }) => {
                self.verify_block_args(iid, targets[0], 0);
                self.verify_block_args(iid, targets[1], 0);
            }
            Instr::Terminator(
                Terminator::IterInit(ref args, _) | Terminator::IterNext(ref args),
            ) => {
                self.verify_block_args(iid, args.targets[0], 0);
                self.verify_block_args(iid, args.targets[1], 0);
            }
            Instr::Special(Special::Param) => {
                check_failed!(self, "Param iid {} shouldn't be in block instrs", iid);
            }
            Instr::Special(Special::IrToBc(IrToBc::PushConstant(vid))) => {
                check!(
                    self,
                    vid.is_imm(),
                    "vid {} PushConstant must refer to a ImmId",
                    iid
                );
            }
            Instr::Special(Special::Select(target_vid, idx)) => {
                self.verify_select(iid, iid_idx, prev_vid, target_vid, idx);
            }
            Instr::Special(Special::Tombstone) => {
                // We should never see a tombstone as part of the graph.
                check_failed!(self, "iid {} is a tombstone", iid);
            }
            _ => {}
        }
    }
}

pub fn verify_func(func: &Func, flags: &Flags) {
    let mut verify = VerifyFunc::new(func, flags);
    verify.verify_func_body();
}
