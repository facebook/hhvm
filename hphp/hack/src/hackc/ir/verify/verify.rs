#![allow(unused_mut)]
// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

//! Verifier for HackIR.  This traverses the HackIR structures and makes sure
//! that the required invariants still hold true.

use core::{
    instr::{HasEdges, HasOperands, Special, Terminator},
    string_intern::StringInterner,
    Block, BlockId, BlockIdMap, FullInstrId, Func, Instr, InstrId, InstrIdSet, ValueId,
};
use itertools::Itertools;
use print::{DisplayFunc, FmtBid, FmtRawVid, FmtVid};
use std::collections::hash_map::Entry;

type Result<T = ()> = anyhow::Result<T>;

/// Flags controlling the details of verification.
pub struct Flags {
    // Currently empty.
}

impl Flags {
    pub fn new() -> Self {
        Self {}
    }
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
            return Err($self.check_failed(&format!($($why),+)));
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

struct VerifyFunc<'a, 'b> {
    // Mapping of what InstrIds dominate each Block.
    dominated_iids: BlockIdMap<InstrIdSet>,
    func: &'b Func<'a>,
    #[allow(dead_code)]
    flags: &'b Flags,
    strings: &'b StringInterner<'a>,
}

impl<'a, 'b> VerifyFunc<'a, 'b> {
    fn new(func: &'b Func<'a>, flags: &'b Flags, strings: &'b StringInterner<'a>) -> Self {
        VerifyFunc {
            dominated_iids: Default::default(),
            func,
            flags,
            strings,
        }
    }

    fn raw_check_failed(func: &Func<'_>, why: &str, strings: &StringInterner<'_>) -> anyhow::Error {
        panic!(
            "VERIFY FAILED: {}\n{}",
            why,
            DisplayFunc(func, true, strings)
        );
    }

    fn check_failed(&self, why: &str) -> anyhow::Error {
        Self::raw_check_failed(self.func, why, self.strings)
    }

    fn verify_block(&mut self, bid: BlockId, block: &Block) -> Result {
        check!(
            self,
            block.is_terminated(self.func),
            "block {} is unterminated",
            bid
        );

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
            )?;
        }

        // Push the dominated IIDs into the successors.
        for &edge in self.func.edges(bid) {
            match self.dominated_iids.entry(edge) {
                Entry::Vacant(e) => {
                    e.insert(dominated_iids.clone());
                }
                Entry::Occupied(mut e) => {
                    e.get_mut().retain(|iid| dominated_iids.contains(iid));
                }
            }
        }
        self.dominated_iids.insert(bid, dominated_iids);

        Ok(())
    }

    fn verify_func_body(&mut self) -> Result {
        self.verify_func_unique_iids()?;

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
            self.verify_block(bid, self.func.block(bid))?;
        }
        Ok(())
    }

    fn verify_func_unique_iids(&self) -> Result {
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
        Ok(())
    }

    fn verify_operand(
        &self,
        src_iid: InstrId,
        op_vid: ValueId,
        op_idx: usize,
        dominated_iids: &InstrIdSet,
    ) -> Result {
        match op_vid.full() {
            FullInstrId::Literal(cid) => {
                check!(
                    self,
                    self.func.literals.get(cid).is_some(),
                    "iid {} refers to missing literal {}",
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
                        return Err(check_failed!(
                            self,
                            "iid {} operand {} ({})is a tombstone",
                            src_iid,
                            op_idx,
                            FmtVid(self.func, op_vid, true)
                        ));
                    }
                    _ => {}
                }
            }
            FullInstrId::None => {
                return Err(check_failed!(
                    self,
                    "iid {} operand {} is none",
                    src_iid,
                    op_idx
                ));
            }
        }

        Ok(())
    }

    fn verify_block_args(&self, iid: InstrId, bid: BlockId, args: usize) -> Result {
        // Make sure that the target block is expecting the args.
        let expected_args = self.func.block(bid).params.len();
        check!(
            self,
            expected_args == args,
            "iid {} is a jump with {} args to block expecting {} args",
            iid,
            args,
            expected_args
        );

        Ok(())
    }

    fn verify_selects(&self, n: usize, block: &Block, iid: InstrId, iid_idx: usize) -> Result {
        for i in 1..=n {
            let select_iid = *block.iids.get(iid_idx + i).ok_or_else(|| {
                check_failed!(
                    self,
                    "iid {} returns {} values but block doesn't contain enough iids (only followed by {})",
                    iid,
                    n,
                    i - 1
                )
            })?;

            check!(
                self,
                matches!(
                    self.func.instr(select_iid),
                    Instr::Special(Special::Select(..))
                ),
                "iid {} returns {} values but is only followed by {} selects (or pops)",
                iid,
                n,
                i - 1
            );
        }

        Ok(())
    }

    fn verify_instr(
        &self,
        block: &Block,
        iid: InstrId,
        iid_idx: usize,
        instr: &Instr,
        dominated_iids: &mut InstrIdSet,
    ) -> Result {
        // Ensure that all operands exist.
        for (op_idx, op_vid) in instr.operands().iter().copied().enumerate() {
            self.verify_operand(iid, op_vid, op_idx, dominated_iids)?;
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
                    self.verify_selects(call.num_rets as usize, block, iid, iid_idx)?;
                }
            }
            Instr::Terminator(Terminator::CallAsync(_, targets)) => {
                self.verify_block_args(iid, targets[0], 1)?;
                self.verify_block_args(iid, targets[1], 1)?;
            }
            Instr::Terminator(Terminator::Jmp(bid, _, _)) => {
                self.verify_block_args(iid, bid, 0)?;
            }
            Instr::Terminator(Terminator::JmpArgs(bid, _, ref vids, _)) => {
                check!(
                    self,
                    !vids.is_empty(),
                    "iid {} is JmpArgs with no args!",
                    iid
                );
                self.verify_block_args(iid, bid, vids.len())?;
            }
            Instr::Terminator(Terminator::JmpOp { targets, .. }) => {
                self.verify_block_args(iid, targets[0], 0)?;
                self.verify_block_args(iid, targets[1], 0)?;
            }
            Instr::Terminator(
                Terminator::IterInit(ref args, _) | Terminator::IterNext(ref args),
            ) => {
                self.verify_block_args(iid, args.targets[0], 0)?;
                self.verify_block_args(iid, args.targets[1], 0)?;
            }
            Instr::Special(Special::Param) => {
                return Err(check_failed!(
                    self,
                    "Param iid {} shouldn't be in block instrs",
                    iid
                ));
            }
            Instr::Special(Special::PushLiteral(vid)) => {
                check!(
                    self,
                    vid.is_literal(),
                    "vid {} PushLiteral must refer to a LiteralId",
                    iid
                );
            }
            Instr::Special(Special::Select(target_vid, idx)) => {
                // The select instruction must follow either another select for the
                // same target or the target it's selecting from.
                check!(
                    self,
                    iid_idx > 0,
                    "iid {} select cannot be first instr in block",
                    iid
                );
                let prev_vid = prev_vid.unwrap();
                if prev_vid != target_vid {
                    // If the previous vid is not our target then it must be a
                    // select with the same target as us.
                    let prev_instr = self.func.instr(prev_vid.expect_instr("instr expected"));
                    match *prev_instr {
                        Instr::Special(Special::Select(prev_target, prev_idx)) => {
                            if prev_target != target_vid {
                                return Err(check_failed!(
                                    self,
                                    "iid {} select with target {} follows a select with a different target instr {}",
                                    iid,
                                    FmtRawVid(target_vid),
                                    FmtRawVid(prev_target)
                                ));
                            }
                            if prev_idx + 1 != idx {
                                return Err(check_failed!(
                                    self,
                                    "iid {} select with index {} follows a non-contiguous select index {}",
                                    iid,
                                    idx,
                                    prev_idx
                                ));
                            }
                        }
                        _ => {
                            return Err(check_failed!(
                                self,
                                "iid {} select is not contiguous with its target",
                                iid
                            ));
                        }
                    }
                } else {
                    let prev_instr = self.func.instr(prev_vid.expect_instr("instr expected"));
                    match prev_instr {
                        Instr::Call(call) => {
                            if call.num_rets < 2 {
                                return Err(check_failed!(
                                    self,
                                    "iid {} select follows Call with only {} returns",
                                    iid,
                                    call.num_rets
                                ));
                            }
                            if idx >= call.num_rets {
                                return Err(check_failed!(
                                    self,
                                    "iid {} select references index {} from call with only {} returns",
                                    iid,
                                    idx,
                                    call.num_rets
                                ));
                            }
                        }
                        _ => {
                            return Err(check_failed!(
                                self,
                                "iid {} select follows non-selectable instr {:?}",
                                iid,
                                prev_instr
                            ));
                        }
                    }
                }
            }
            Instr::Special(Special::Tombstone) => {
                // We should never see a tombstone as part of the graph.
                return Err(check_failed!(self, "iid {} is a tombstone", iid));
            }
            _ => {}
        }

        Ok(())
    }
}

pub fn verify_func(func: &Func<'_>, flags: &Flags, strings: &StringInterner<'_>) -> Result {
    let mut verify = VerifyFunc::new(func, flags, strings);
    verify.verify_func_body()
}
