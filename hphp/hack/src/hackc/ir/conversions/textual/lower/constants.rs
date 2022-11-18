// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use ir::instr;
use ir::instr::HasOperands;
use ir::instr::Terminator;
use ir::newtype::ConstantIdSet;
use ir::BlockId;
use ir::BlockIdSet;
use ir::Constant;
use ir::ConstantId;
use ir::Func;
use ir::FuncBuilder;
use ir::HasEdges;
use ir::Instr;
use ir::ValueId;
use ir::ValueIdMap;
use log::trace;

/// Write the complex constants to the start of the entry block and remap their
/// uses to the emitted values.
pub(crate) fn write_constants(builder: &mut FuncBuilder<'_>) {
    insert_constants(builder, Func::ENTRY_BID);
}

/// Follow the blocks successors around but stopping at an 'enter'.
fn follow_block_successors(func: &Func<'_>, bid: BlockId) -> Vec<BlockId> {
    let mut result = Vec::new();
    let mut processed = BlockIdSet::default();
    let mut pending = vec![bid];
    while let Some(bid) = pending.pop() {
        processed.insert(bid);
        result.push(bid);

        // Add unprocessed successors to pending.
        let terminator = func.terminator(bid);
        match terminator {
            Terminator::Enter(..) => {
                // We don't want to trace through Enter.
            }
            terminator => {
                for edge in terminator.edges() {
                    if !processed.contains(edge) {
                        pending.push(*edge);
                    }
                }
            }
        }

        // And catch handlers
        let handler = func.catch_target(bid);
        if handler != BlockId::NONE && !processed.contains(&handler) {
            pending.push(handler);
        }
    }

    result
}

/// Compute the set of constants that are visible starting from `bid` up to
/// either a `ret` or an `enter`.
fn compute_live_constants(func: &Func<'_>, bids: &Vec<BlockId>) -> ConstantIdSet {
    let mut visible = ConstantIdSet::default();
    for &bid in bids {
        let block = func.block(bid);
        visible.extend(
            block
                .iids()
                .map(|iid| func.instr(iid))
                .flat_map(|instr| instr.operands().iter().filter_map(|op| op.constant())),
        );
    }

    visible
}

fn remap_constants(func: &mut Func<'_>, bids: &Vec<BlockId>, remap: ValueIdMap<ValueId>) {
    for &bid in bids {
        func.remap_block_vids(bid, &remap);
    }
}

fn insert_constants(builder: &mut FuncBuilder<'_>, start_bid: BlockId) {
    // Allocate a new block, fill it with constants, append the old InstrIds,
    // swap the old and new blocks and then clear the new block.

    let bids = follow_block_successors(&builder.func, start_bid);
    let constants = compute_live_constants(&builder.func, &bids);

    let mut remap = ValueIdMap::default();
    let mut fixups = Vec::default();

    let new_bid = builder.alloc_bid();
    builder.start_block(new_bid);

    for cid in constants.into_iter() {
        if let Some(vid) = write_constant(builder, cid) {
            let src = ValueId::from_constant(cid);
            remap.insert(src, vid);
            fixups.push((vid, src));
        }
    }

    let mut old_iids = std::mem::take(&mut builder.func.block_mut(start_bid).iids);
    builder.cur_block_mut().iids.append(&mut old_iids);

    builder.func.blocks.swap(start_bid, new_bid);

    remap_constants(&mut builder.func, &bids, remap);

    for (vid, src) in fixups {
        let iid = vid.instr().unwrap();
        builder.func.instrs[iid] = Instr::Special(instr::Special::Copy(src));
    }
}

fn write_constant(builder: &mut FuncBuilder<'_>, cid: ConstantId) -> Option<ValueId> {
    let constant = builder.func.constant(cid);
    trace!("    Const {cid}: {constant:?}");
    match constant {
        Constant::Bool(..)
        | Constant::Dir
        | Constant::Float(..)
        | Constant::File
        | Constant::FuncCred
        | Constant::Int(..)
        | Constant::Method
        | Constant::Named(..)
        | Constant::NewCol(..)
        | Constant::Null
        | Constant::Uninit => None,

        // Insert a tombstone which will be turned into a 'copy' later.
        Constant::Array(_) | Constant::String(_) => Some(builder.emit(Instr::tombstone())),
    }
}
