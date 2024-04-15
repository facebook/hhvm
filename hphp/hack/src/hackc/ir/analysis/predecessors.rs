// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use ir_core::BlockId;
use ir_core::BlockIdMap;
use ir_core::BlockIdSet;
use ir_core::Func;
use ir_core::IrRepr;
use newtype::IdVec;

pub type Predecessors = BlockIdMap<BlockIdSet>;

/// Compute the predecessor Blocks for a Func's Blocks.
///
/// For every Block in the Func compute the Block's predecessors.
///
/// The result is guaranteed to have an entry for every known block, even if it
/// has no predecessors.
pub fn compute_predecessor_blocks(func: &Func, flags: PredecessorFlags) -> Predecessors {
    let mut predecessors: Predecessors = Default::default();

    if flags.mark_entry_blocks {
        // Insert BlockId::NONE as a source of ENTRY_BID to indicate that it's
        // called externally.
        mark_edge(&mut predecessors, BlockId::NONE, IrRepr::ENTRY_BID);

        // Handle the default params.
        for (_, dv) in &func.repr.params {
            if let Some(dv) = dv {
                mark_edge(&mut predecessors, BlockId::NONE, dv.init);
            }
        }
    }

    for bid in func.repr.block_ids() {
        predecessors.entry(bid).or_default();
        for &target_bid in func.repr.edges(bid) {
            mark_edge(&mut predecessors, bid, target_bid);
        }

        flags.catch.mark(&mut predecessors, func, bid);
    }

    predecessors
}

#[derive(Clone, Copy, Eq, PartialEq)]
pub enum PredecessorCatchMode {
    // Don't mark catch blocks with any source.
    Ignore,

    // Mark catch blocks with a predecessor of BlockId::NONE
    FromNone,

    // Mark catch blocks with a predecessor of potential throwers.
    Throw,
}

impl Default for PredecessorCatchMode {
    fn default() -> Self {
        Self::Ignore
    }
}

impl PredecessorCatchMode {
    fn mark(self, predecessors: &mut Predecessors, func: &Func, mut src: BlockId) {
        if self == PredecessorCatchMode::Ignore {
            return;
        }

        let dst = func.repr.catch_target(src);
        if dst == BlockId::NONE {
            return;
        }

        if self == PredecessorCatchMode::FromNone {
            src = BlockId::NONE;
        }

        mark_edge(predecessors, src, dst);
    }
}

#[derive(Default)]
pub struct PredecessorFlags {
    // If mark_entry_blocks is true then blocks that can be called from outside
    // the function (such as the entry block and default value blocks) will be
    // marked as having a predecessor of BlockId::NONE.
    pub mark_entry_blocks: bool,
    pub catch: PredecessorCatchMode,
}

fn mark_edge(predecessors: &mut Predecessors, src: BlockId, dst: BlockId) {
    predecessors.entry(dst).or_default().insert(src);
}

/// Compute the number of incoming control-flow edges to each block. If there are no
/// critical edges, this is also the number of predecessor blocks.
pub fn compute_num_predecessors(func: &Func, flags: PredecessorFlags) -> IdVec<BlockId, u32> {
    let mut counts = IdVec::new_from_vec(vec![0; func.repr.blocks.len()]);

    if flags.mark_entry_blocks {
        // Entry
        counts[BlockId::NONE] += 1;

        // Default Params
        for (_, dv) in &func.repr.params {
            if let Some(dv) = dv {
                counts[dv.init] += 1;
            }
        }
    }

    let note_catch = !matches!(flags.catch, PredecessorCatchMode::Ignore);

    for bid in func.repr.block_ids() {
        for &target in func.repr.edges(bid) {
            counts[target] += 1;
        }
        if note_catch {
            let dst = func.repr.catch_target(bid);
            if dst != BlockId::NONE {
                counts[dst] += 1;
            }
        }
    }

    counts
}
