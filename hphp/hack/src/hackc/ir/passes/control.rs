// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use core::{
    instr::{HasEdges, SurpriseCheck, Terminator},
    BlockId, BlockIdMap, Func, InstrId, ValueId,
};

/// Attempt to merge simple blocks together. Returns true if the Func was
/// changed.
pub fn run(func: &mut Func<'_>) -> bool {
    let mut remap = BlockIdMap::default();
    for bid in func.block_ids() {
        if let Some(target) = forward_edge(func, bid) {
            remap.insert(bid, target);
        }
    }

    if remap.is_empty() {
        return false;
    }

    if let Some(&target) = remap.get(&Func::ENTRY_BID) {
        // Uh oh - we're remapping the ENTRY_BID - this is no good. Instead
        // remap the target and swap the two blocks.
        remap.remove(&Func::ENTRY_BID);
        remap.insert(target, Func::ENTRY_BID);
        func.blocks.swap(Func::ENTRY_BID, target);
    }

    for param in &mut func.params {
        if let Some(dv) = param.default_value.as_mut() {
            dv.0 = remap.get(&dv.0).copied().unwrap_or(dv.0);
        }
    }

    let bids: Vec<BlockId> = func.block_ids().collect();
    for bid in bids {
        let terminator = func.terminator_mut(bid);
        for edge in terminator.edges_mut() {
            *edge = remap.get(edge).copied().unwrap_or(*edge);
        }
    }

    // Now go through the remaps and move our pnames over.
    for (bid, target) in remap.into_iter() {
        let src = func.block(bid).pname_hint.as_ref();
        let dst = func.block(target).pname_hint.as_ref();
        match (src, dst) {
            (_, Some(_)) | (None, None) => {}
            (Some(pname), None) => {
                let pname = pname.clone();
                func.block_mut(target).pname_hint = Some(pname);
            }
        }
    }

    crate::rpo_sort(func);
    true
}

fn params_eq(a: &[InstrId], b: &[ValueId]) -> bool {
    a.len() == b.len()
        && a.iter()
            .zip(b.iter())
            .all(|(a, b)| ValueId::from_instr(*a) == *b)
}

fn forward_edge(func: &Func<'_>, original: BlockId) -> Option<BlockId> {
    // If this block just contains a jump to another block then jump directly to
    // that one instead.

    let mut changed = false;
    let mut bid = original;

    loop {
        // Check for infinite recursion.
        if changed && bid == original {
            return None;
        }

        let block = func.block(bid);
        let terminator = func.terminator(bid);
        match terminator {
            // In general SurpriseCheck::No means that this was an artificial
            // jump but SurpriseCheck::Yes means it was in the original
            // bytecode. When we care less about matching the original bytecode
            // this can be relaxed to accept both.
            Terminator::Jmp(target, SurpriseCheck::No, _)
                if block.params.is_empty() && block.iids.len() == 1 =>
            {
                bid = *target;
                changed = true;
            }
            Terminator::JmpArgs(target, SurpriseCheck::No, args, _)
                if (block.iids.len() == 1) && params_eq(&block.params, args) =>
            {
                bid = *target;
                changed = true;
            }
            _ => {
                break;
            }
        }
    }

    changed.then(|| bid)
}
