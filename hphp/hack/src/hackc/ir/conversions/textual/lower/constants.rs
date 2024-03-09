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
use ir::StringInterner;
use ir::ValueId;
use ir::ValueIdMap;
use log::trace;

/// Write the complex constants to the start of the entry block (and 'default'
/// handling blocks) and remap their uses to the emitted values.
pub(crate) fn write_constants(builder: &mut FuncBuilder) {
    // Rewrite some types of constants.
    for c in builder.func.constants.iter_mut() {
        match c {
            Constant::File => {
                // Rewrite __FILE__ as a simple string since the real filename
                // shouldn't matter for analysis.
                let id = builder.strings.intern_str("__FILE__");
                *c = Constant::String(id);
            }
            _ => {}
        }
    }

    // Write the complex constants to the entry block.
    insert_constants(builder, Func::ENTRY_BID);
}

/// Follow the blocks successors around but stopping at an 'enter'. We stop at
/// enter under the assumption that default blocks enter into the entry path -
/// so those will be handled separately.
fn follow_block_successors(func: &Func, bid: BlockId) -> Vec<BlockId> {
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

/// Compute the set of constants that are visible starting from `bid`.
fn compute_live_constants(func: &Func, bids: &Vec<BlockId>) -> ConstantIdSet {
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

fn remap_constants(func: &mut Func, bids: &Vec<BlockId>, remap: ValueIdMap<ValueId>) {
    for &bid in bids {
        func.remap_block_vids(bid, &remap);
    }
}

fn insert_constants(builder: &mut FuncBuilder, start_bid: BlockId) {
    // Allocate a new block, fill it with constants, append the old InstrIds,
    // swap the old and new blocks and then clear the new block.

    let bids = follow_block_successors(&builder.func, start_bid);
    let constants = compute_live_constants(&builder.func, &bids);

    let constants = sort_and_filter_constants(&builder.func, constants, &builder.strings);

    let mut remap = ValueIdMap::default();
    let mut fixups = Vec::default();

    let new_bid = builder.alloc_bid_based_on(start_bid);
    builder.start_block(new_bid);

    for cid in constants.into_iter() {
        let (vid, needs_fixup) = write_constant(builder, cid);
        let src = ValueId::from_constant(cid);
        remap.insert(src, vid);
        if needs_fixup {
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

/// Arrays can refer to some prior constants (like Strings) so they need to be
/// sorted before being written. Right now arrays can't refer to other arrays so
/// they don't need to be sorted relative to each other.
fn sort_and_filter_constants(
    func: &Func,
    constants: ConstantIdSet,
    string_intern: &StringInterner,
) -> Vec<ConstantId> {
    let mut result = Vec::with_capacity(constants.len());
    let mut arrays = Vec::with_capacity(constants.len());
    for cid in constants {
        let constant = func.constant(cid);
        match constant {
            Constant::Bool(..)
            | Constant::Dir
            | Constant::EnumClassLabel(..)
            | Constant::Float(..)
            | Constant::File
            | Constant::FuncCred
            | Constant::Int(..)
            | Constant::LazyClass(_)
            | Constant::Method
            | Constant::NewCol(..)
            | Constant::Null
            | Constant::Uninit => {}

            Constant::Array(_) => {
                arrays.push(cid);
            }
            Constant::Named(..) => {
                result.push(cid);
            }
            Constant::String(s) => {
                // If the string is short then just keep it inline. This makes
                // it easier to visually read the output but may be more work
                // for infer (because it's a call)...
                if string_intern.lookup_bstr(*s).len() > 40 {
                    result.push(cid);
                }
            }
        }
    }

    result.append(&mut arrays);
    result
}

fn write_constant(builder: &mut FuncBuilder, cid: ConstantId) -> (ValueId, bool) {
    let constant = builder.func.constant(cid);
    trace!("    Const {cid}: {constant:?}");
    match constant {
        Constant::Bool(..)
        | Constant::Dir
        | Constant::Float(..)
        | Constant::File
        | Constant::FuncCred
        | Constant::Int(..)
        | Constant::LazyClass(_)
        | Constant::Method
        | Constant::NewCol(..)
        | Constant::Null
        | Constant::Uninit => unreachable!(),

        // Insert a tombstone which will be turned into a 'copy' later.
        Constant::Array(_) | Constant::String(_) | Constant::EnumClassLabel(_) => {
            (builder.emit(Instr::tombstone()), true)
        }

        Constant::Named(name) => {
            let id = ir::GlobalId::new(name.as_string_id());
            let vid = builder.emit(Instr::Special(ir::instr::Special::Textual(
                ir::instr::Textual::LoadGlobal { id, is_const: true },
            )));
            (vid, false)
        }
    }
}
