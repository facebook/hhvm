// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use analysis::PredecessorCatchMode;
use analysis::PredecessorFlags;
use ir_core::instr::HasEdges;
use ir_core::instr::Terminator;
use ir_core::BlockId;
use ir_core::BlockIdMap;
use ir_core::Func;
use ir_core::InstrId;
use ir_core::ValueId;
use newtype::IdVec;

/// Attempt to merge simple blocks together. Returns true if the Func was
/// changed.
pub fn run(func: &mut Func<'_>) -> bool {
    let mut predecesors = analysis::compute_num_predecessors(
        func,
        PredecessorFlags {
            mark_entry_blocks: false,
            catch: PredecessorCatchMode::Throw,
        },
    );

    for bid in func.block_ids() {
        let mut instr = std::mem::replace(func.terminator_mut(bid), Terminator::Unreachable);

        let edges = instr.edges_mut();
        let successors = edges.len() as u32;
        for edge in edges {
            let target = forward_edge(func, *edge, successors, &predecesors);
            if target != *edge {
                predecesors[*edge] -= 1;
                predecesors[target] += 1;
                *edge = target;
            }
        }

        *func.terminator_mut(bid) = instr;
    }

    // Function params
    for idx in 0..func.params.len() {
        if let Some((edge, _)) = func.params[idx].default_value {
            let target = forward_edge(func, edge, 1, &predecesors);
            if target != edge {
                predecesors[edge] -= 1;
                predecesors[target] += 1;
                func.params[idx].default_value.as_mut().unwrap().0 = target;
            }
        }
    }

    // Entry Block
    let target = forward_edge(func, Func::ENTRY_BID, 1, &predecesors);
    if target != Func::ENTRY_BID {
        // Uh oh - we're remapping the ENTRY_BID - this is no good. Instead
        // remap the target and swap the two blocks.
        let mut remap = BlockIdMap::default();
        remap.insert(target, Func::ENTRY_BID);
        func.remap_bids(&remap);
        func.blocks.swap(Func::ENTRY_BID, target);
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

fn forward_edge(
    func: &Func<'_>,
    mut bid: BlockId,
    source_successors: u32,
    predecesors: &IdVec<BlockId, u32>,
) -> BlockId {
    // If the target block just contains a jump to another block then jump
    // directly to that one instead - but we need to worry about creating a
    // critical edge. If our predecesor has multiple successors and our target
    // has multiple predecesors then we can't combine them because that would be
    // a critical edge.

    if source_successors > 1 {
        return bid;
    }

    // TODO: Optimize this. Nothing will break if this is too small or too
    // big. Too small means we'll snap through fewer chains. To big means we'll
    // take longer than necessary to bail on an infinite loop.
    const LOOP_CHECK: usize = 100;

    for _ in 0..LOOP_CHECK {
        let block = func.block(bid);
        if block.iids.len() != 1 {
            // This block has multiple instrs.
            break;
        }

        let terminator = func.terminator(bid);
        let target = match *terminator {
            Terminator::Jmp(target, _) => {
                if !block.params.is_empty() {
                    // The block takes params but the jump doesn't pass them
                    // along - it's not a simple forward.
                    break;
                }

                target
            }
            Terminator::JmpArgs(target, ref args, _) => {
                if !params_eq(&block.params, args) {
                    // Our jump is passing a different set of args from what our
                    // block takes in.
                    break;
                }

                target
            }
            _ => {
                break;
            }
        };

        let target_block = func.block(target);
        if target_block.tcid != block.tcid {
            // This jump crosses an exception frame - can't forward.
            break;
        }

        // Critical edges: If this block has multiple successsors and the target
        // has multiple predecesors then we can't do this because it would
        // create a critical edge.  (At this point we know that our source only
        // has a single successor because either we checked outside the loop or
        // we're coming from a Jmp or JmpArgs)
        if predecesors[target] > 1 {
            break;
        }

        bid = target;
    }

    bid
}
