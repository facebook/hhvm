// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use core::{BlockId, BlockIdMap, BlockIdSet, Func};
use newtype::IdVec;

/// Compute the predecessor Blocks for a Func's Blocks.
///
/// For every Block in the Func compute the Block's predecessors.
///
/// The result is guaranteed to have an entry for every known block, even if it
/// has no predecessors.
pub fn compute_predecessor_blocks(
    func: &Func<'_>,
    flags: PredecessorFlags,
) -> BlockIdMap<BlockIdSet> {
    let mut predecessors: BlockIdMap<BlockIdSet> = Default::default();

    if flags.mark_externals {
        // Insert BlockId::NONE as a source of ENTRY_BID to indicate that it's
        // called externally.
        mark_edge(&mut predecessors, BlockId::NONE, Func::ENTRY_BID);

        // Handle the default params.
        for param in &func.params {
            if let Some((bid, _)) = param.default_value {
                mark_edge(&mut predecessors, BlockId::NONE, bid);
            }
        }
    }

    for bid in func.block_ids() {
        predecessors.entry(bid).or_default();
        for &target_bid in func.edges(bid) {
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
    fn mark(self, predecessors: &mut BlockIdMap<BlockIdSet>, func: &Func<'_>, mut src: BlockId) {
        if self == PredecessorCatchMode::Ignore {
            return;
        }

        let dst = func.catch_target(src);
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
    // If mark_externals is true then blocks that can be called from outside the
    // function (such as the entry block and default value blocks) will be
    // marked as having a predecessor of BlockId::NONE.
    pub mark_externals: bool,
    pub catch: PredecessorCatchMode,
}

fn mark_edge(predecessors: &mut BlockIdMap<BlockIdSet>, src: BlockId, dst: BlockId) {
    predecessors.entry(dst).or_default().insert(src);
}

/// Compute the number of incoming control-flow edges to each block. If there are no
/// critical edges, this is also the number of predecessor blocks.
pub fn compute_num_predecessors(func: &Func<'_>) -> IdVec<BlockId, u32> {
    let mut counts = IdVec::new_from_vec(vec![0; func.blocks.len()]);
    for bid in func.block_ids() {
        if let Some(edges) = func.get_edges(bid) {
            for &target in edges {
                counts[target] += 1;
            }
        }
    }
    counts
}
