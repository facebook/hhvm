// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use ir_core::BlockId;
use ir_core::Func;
use newtype::IdVec;

/// Compute the Block post-order for a Func. In the returned Vec a BlockId will
/// appear after all of its successors.
pub fn compute_po(func: &Func) -> Vec<BlockId> {
    compute_general_po(func, |edges| edges.split_last())
}

/// Compute the Block reverse-post-order for a Func. In the retuned Vec a
/// BlockId will appear before any of its successors.
pub fn compute_rpo(func: &Func) -> Vec<BlockId> {
    let mut po = compute_general_po(func, |edges| edges.split_last());
    po.reverse();
    po
}

/// Compute reversed rpo, which is rpo of a dfs that walks children in
/// the opposite order that ordinary rpo does.  This is useful for
/// varying rpo traversal order when doing a fixed point computation
/// such as dominators to avoid quadratic corner cases.
pub fn compute_rrpo(func: &Func) -> Vec<BlockId> {
    // Note that we're spliting by first whereas compute_rpo() splits by last...
    let mut po = compute_general_po(func, |edges| edges.split_first());
    po.reverse();
    po
}

fn compute_general_po(
    func: &Func,
    splitter: impl Fn(&[BlockId]) -> Option<(&BlockId, &[BlockId])>,
) -> Vec<BlockId> {
    let blocks = &func.blocks;
    let mut result = Vec::with_capacity(blocks.len());
    if blocks.is_empty() {
        return result;
    }

    // Use an explicit recursion stack seeded with the entry block to avoid
    // blowing the program stack recursing through a large Func.
    let mut stack = Vec::new();
    let mut already_pushed = IdVec::new_from_vec(vec![false; blocks.len()]);

    mark_block(&mut stack, &mut already_pushed, func, Func::ENTRY_BID);

    for param in &func.params {
        if let Some(dv) = param.default_value.as_ref() {
            mark_block(&mut stack, &mut already_pushed, func, dv.init);
        }
    }

    fn mark_block<'a>(
        stack: &mut Vec<(BlockId, &'a [BlockId], BlockId)>,
        already_pushed: &mut [bool],
        func: &'a Func,
        bid: BlockId,
    ) {
        let edges = func.edges(bid);
        let catch_bid = func.catch_target(bid);
        stack.push((bid, edges, catch_bid));
        already_pushed[bid.as_usize()] = true;
    }

    fn process_child<'a>(
        func: &'a Func,
        stack: &mut Vec<(BlockId, &'a [BlockId], BlockId)>,
        already_pushed: &mut IdVec<BlockId, bool>,
        bid: BlockId,
        parent: (BlockId, &'a [BlockId], BlockId),
    ) -> bool {
        let was_pushed = &mut already_pushed[bid];
        if *was_pushed {
            return false;
        }

        *was_pushed = true;

        // Remember to revisit the parent once this child is done.
        stack.push(parent);

        let child_edges = func.edges(bid);
        let child_catch_bid = func.catch_target(bid);
        stack.push((bid, child_edges, child_catch_bid));
        true
    }

    'outer: while let Some((bid, mut edges, catch_bid)) = stack.pop() {
        while !edges.is_empty() {
            let (&child_bid, next_edges) = splitter(edges).unwrap();
            edges = next_edges;

            let parent = (bid, edges, catch_bid);
            if process_child(func, &mut stack, &mut already_pushed, child_bid, parent) {
                continue 'outer;
            }
        }

        // If this is the 'try' for a try/catch block then add a fake edge to
        // the catch block.
        if catch_bid != BlockId::NONE {
            let parent = (bid, &[] as &[BlockId], BlockId::NONE);
            if process_child(func, &mut stack, &mut already_pushed, catch_bid, parent) {
                continue;
            }
        }

        // All successors were visited, so record this block (i.e. "postorder").
        result.push(bid);
    }
    result
}
