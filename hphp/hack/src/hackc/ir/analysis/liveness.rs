// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

// Everything you want to know about what InstrIds are alive when.

use std::collections::VecDeque;

use ir_core::instr::HasOperands;
use ir_core::BlockId;
use ir_core::BlockIdMap;
use ir_core::BlockIdSet;
use ir_core::Func;
use ir_core::InstrId;
use ir_core::InstrIdSet;
use ir_core::ValueId;
use itertools::Itertools;
use newtype::IdVec;

use crate::PredecessorCatchMode;
use crate::PredecessorFlags;

/// Used to compute the set of live InstrIds across a Func.
pub struct LiveInstrs {
    /// This is a list of where a given InstrId dies.  It's a set because an
    /// InstrId could end its lifetime in multiple blocks.
    pub instr_last_use: IdVec<InstrId, InstrIdSet>,

    /// This is the inverse of instr_last_use - for a given InstrId what
    /// InstrIds have their last use there.
    pub instrs_dead_at: IdVec<InstrId, InstrIdSet>,

    /// Per-block liveness information.
    pub blocks: IdVec<BlockId, LiveBlockInfo>,
}

impl std::fmt::Debug for LiveInstrs {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        use std::fmt::Display;

        use print::FmtRawVid;
        use print::FmtSep;
        writeln!(f, "instr_last_use: {{")?;
        for (vid, v) in self.instr_last_use.iter().enumerate() {
            writeln!(
                f,
                "  {vid} => ({}),",
                FmtSep::comma(v.iter(), |f, i| { Display::fmt(&i, f) })
            )?;
        }
        writeln!(f, "}},")?;
        writeln!(f, "instrs_dead_at: {{")?;
        for (vid, v) in self.instrs_dead_at.iter().enumerate() {
            writeln!(
                f,
                "  {vid} => [{}],",
                FmtSep::comma(v.iter(), |f, i| { Display::fmt(&i, f) })
            )?;
        }
        writeln!(f, "}},")?;
        writeln!(f, "blocks: {{")?;
        for (bid, bi) in self.blocks.iter().enumerate() {
            writeln!(
                f,
                "  {bid} => {{ entry: [{entry}], dead_on_entry: [{doe}], exit: [{exit}] }}",
                entry = FmtSep::comma(bi.entry.iter(), |f, i| {
                    FmtRawVid(ValueId::from_instr(*i)).fmt(f)
                }),
                doe = FmtSep::comma(bi.dead_on_entry.iter(), |f, i| {
                    FmtRawVid(ValueId::from_instr(*i)).fmt(f)
                }),
                exit = FmtSep::comma(bi.exit.iter(), |f, i| {
                    FmtRawVid(ValueId::from_instr(*i)).fmt(f)
                }),
            )?;
        }
        writeln!(f, "}}")
    }
}

impl LiveInstrs {
    pub fn compute(func: &Func) -> Self {
        let mut live = LiveInstrs {
            instr_last_use: IdVec::new_from_vec(vec![Default::default(); func.instrs_len()]),
            instrs_dead_at: IdVec::new_from_vec(vec![Default::default(); func.instrs_len()]),
            blocks: IdVec::new_from_vec(vec![Default::default(); func.blocks.len()]),
        };

        // Start by computing what InstrIds are introduced and referenced in
        // each block.
        for bid in func.block_ids() {
            live.compute_referenced(func, bid);
        }

        let predecessor_blocks = crate::compute_predecessor_blocks(
            func,
            PredecessorFlags {
                mark_entry_blocks: false,
                catch: PredecessorCatchMode::Throw,
            },
        );

        // Next compute the entry and exit InstrIds for each block.
        //
        // Because we'll add stuff back to the work queue as necessary this
        // doesn't actually have to start in post-order - it's just more
        // efficient.
        let mut work: VecDeque<BlockId> = func.block_ids().rev().collect_vec().into();
        while let Some(bid) = work.pop_front() {
            live.compute_exit(&mut work, func, &predecessor_blocks, bid);
        }

        // At this point LiveBlockInfo::exit is correct but LiveBlockInfo::entry
        // may be wrong (although it will be 'close')! This is because
        // LiveBlockInfo::entry needs to be recomputed from all the
        // LiveBlockInfo::exit values - in case one successor expanded the exit
        // of a predecessor (see the example at the end of
        // compute_instr_use_by_block()).
        for bid in func.block_ids() {
            live.expand_entry(func, bid);
        }

        // Finally compute where each instruction is last used.
        for bid in func.block_ids() {
            live.compute_instr_use_by_block(func, bid);
        }

        live
    }

    fn compute_referenced(&mut self, func: &Func, bid: BlockId) {
        // Nothing in self is valid here.
        let block_info = &mut self.blocks[bid];
        block_info.referenced = func
            .block(bid)
            .iids()
            .flat_map(|iid| {
                func.instr(iid)
                    .operands()
                    .iter()
                    .copied()
                    .filter_map(ValueId::instr)
            })
            .collect();

        // At this point LiveBlockInfo::referenced is correct for this
        // block.
    }

    // Go through `bid` and figure out which InstrIds are referenced and live on
    // exit but not created in this block - those must be live on entry. If this
    // changes our callers' exit then we need to recompute our callers.
    fn compute_exit(
        &mut self,
        work: &mut VecDeque<BlockId>,
        func: &Func,
        predecessor_blocks: &BlockIdMap<BlockIdSet>,
        bid: BlockId,
    ) {
        // At this point:
        // - LiveBlockInfo::referenced is correct for all blocks.
        // - The LiveBlockInfo::exit is a subset of the "correct" info (we loop
        //   until we hit a steady state).
        let block_info = &self.blocks[bid];
        let created: InstrIdSet = func
            .block(bid)
            .iids()
            .chain(func.block(bid).params.iter().copied())
            .collect();
        let entry = &(&block_info.exit | &block_info.referenced) - &created;

        for &src in &predecessor_blocks[&bid] {
            let exit = &mut self.blocks[src].exit;
            let old = exit.len();
            exit.extend(entry.iter().copied());
            if exit.len() != old {
                work.push_back(src);
            }
        }

        self.blocks[bid].entry = entry;
        // At this point:
        // - LiveBlockInfo::entry is an accurate reflection of LiveBlockInfo::exit.
        // - If we changed LiveBlockInfo::exit for our callers we've added them
        //   back onto the work list.
    }

    fn expand_entry(&mut self, func: &Func, bid: BlockId) {
        let exit = self.blocks[bid].exit.clone();
        for &edge in func.edges(bid) {
            self.blocks[edge].entry.extend(exit.iter().copied());
        }

        let catch_bid = func.catch_target(bid);
        if catch_bid != BlockId::NONE {
            self.blocks[catch_bid].entry.extend(exit.iter().copied());
        }
    }

    fn compute_instr_use_by_block(&mut self, func: &Func, bid: BlockId) {
        // At this point:
        // - LiveBlockInfo is correct for all blocks.

        // We start with the InstrIds that are live on block exit and walk
        // backward through the block.

        let block = func.block(bid);
        let mut live = self.blocks[bid].exit.clone();

        for iid in block.iids().rev() {
            if !live.remove(&iid) {
                // This InstrId was both created and last live at this point.
                self.instr_last_use[iid].insert(iid);
                self.instrs_dead_at[iid].insert(iid);
            }

            for &op in func.instr(iid).operands() {
                if let Some(op) = op.instr() {
                    if live.insert(op) {
                        // If we're just seeing this ValueId for the first time then
                        // this must be where the ValueId is last used.
                        self.instr_last_use[op].insert(iid);
                        self.instrs_dead_at[iid].insert(op);
                    }
                }
            }
        }

        let mostly_dead = {
            let mut s: InstrIdSet = self.blocks[bid].entry.clone();
            s.extend(func.block(bid).params.iter());
            s.retain(|item| !live.contains(item));
            s
        };
        if !mostly_dead.is_empty() {
            // Anything in entry or params that isn't in live needs to be marked
            // dead at our start (note that this includes block params)!  This
            // can happen like this:
            //
            //     b0:
            //       %1 = call...
            //       %2 = jmp if zero %1 b1 else b2
            //     b1:
            //       %3 = call(%1)
            //       %4 = jmp b3
            //     b2:
            //       %5 = jmp b3
            //     b3:
            //       %6 = ret
            //
            // Where would %1 die? It can't be %2 because it's used after that
            // in b1.  It can't be %6 because it dies long before that. It needs
            // to be at %3 and %5 - but b2 never actually sees a use of %1 so it
            // doesn't know that it dies there.

            // Mark it dead at the first InstrId of the block (an argument
            // could be made for doing it at the terminal as well).

            let dead_at = *block.iids.first().unwrap();
            for &iid in &mostly_dead {
                self.instr_last_use[iid].insert(dead_at);
            }
            self.instrs_dead_at[dead_at].extend(&mostly_dead);

            self.blocks[bid].dead_on_entry = mostly_dead;
        }
    }
}

/// Information about what InstrIds are live at different points in a block.
/// The relationship is:
///
///     entry - dead_on_entry - referenced = exit
///
/// dead_on_entry and referenced are mutually exclusive - no InstrId should
/// appear in both sets.
#[derive(Debug, Default, Clone)]
pub struct LiveBlockInfo {
    /// What InstrIds are live on entry to this block.
    pub entry: InstrIdSet,
    /// What InstrIds are live on exit from this block.
    pub exit: InstrIdSet,
    /// What InstrIds are referenced by this block.
    pub referenced: InstrIdSet,
    /// What InstrIds are dead immediately on block entry.
    pub dead_on_entry: InstrIdSet,
}
