// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use ir::analysis;
use ir::instr;
use ir::instr::HasLocals;
use ir::instr::HasOperands;
use ir::instr::IrToBc;
use ir::instr::LocalId;
use ir::instr::Special;
use ir::instr::Terminator;
use ir::passes;
use ir::BlockId;
use ir::FullInstrId;
use ir::Func;
use ir::FuncBuilder;
use ir::Instr;
use ir::InstrId;
use ir::InstrIdMap;
use ir::LocId;
use ir::UnnamedLocalId;
use ir::ValueId;
use ir::ValueIdMap;
use itertools::Itertools;
use log::trace;
use smallvec::SmallVec;

use crate::push_count::PushCount;

/// Run through a Func and insert pushes and pops for instructions in
/// preparation of emitting bytecode. In general no attempt is made to optimize
/// the process - we simply push the needed values before any Instr and pop off
/// any stack values after (there is a peephole optimizer that will turn a
/// simple pop/push combo into a no-op).
///
/// JmpArgs pushes its args and then branches. Block args pop their args at the
/// start of a block before executing the block instructions.
///
/// In the future we should probably have a pass that attempts to move common
/// pushes up to common locations (so if there's a common push in both targets
/// of a branch, move the push before the branch).
pub(crate) fn run(func: Func) -> Func {
    let liveness = analysis::LiveInstrs::compute(&func);
    trace!("LIVENESS: {liveness:?}");

    let next_temp_idx = find_next_unnamed_local_idx(&func);
    trace!("First temporary local is {}", next_temp_idx);

    let mut pusher = PushInserter {
        builder: FuncBuilder::with_func(func),
        liveness,
        next_temp_idx,
        instr_ids: Default::default(),
    };

    // Run the blocks in RPO so we see definitions before use.
    for bid in pusher.builder.func.block_ids() {
        pusher.run_block(bid);
    }

    // Careful! At this point the Func is no longer a valid Func! (For example
    // we've converted JmpArgs into plain old Jmp).

    let mut func = pusher.builder.finish();
    passes::control::run(&mut func);
    func
}

#[derive(Clone, Debug, Default, Eq, PartialEq)]
struct BlockInput {
    stack: Vec<ValueId>,
}

/// Helper class to compute where we need to insert stack pushes and pops before
/// we convert to bytecode.
struct PushInserter {
    builder: FuncBuilder,
    liveness: analysis::LiveInstrs,
    next_temp_idx: usize,
    instr_ids: InstrIdMap<ir::LocalId>,
}

impl PushInserter {
    fn alloc_temp(&mut self, iid: InstrId) -> ir::LocalId {
        let temp = ir::LocalId::Unnamed(UnnamedLocalId::from_usize(self.next_temp_idx));
        self.next_temp_idx += 1;
        self.instr_ids.insert(iid, temp);
        temp
    }

    fn lookup_temp(&self, iid: InstrId) -> ir::LocalId {
        *self.instr_ids.get(&iid).unwrap()
    }

    fn consume_temp(&self, iid: InstrId) -> Option<ir::LocalId> {
        // TODO: Consider marking the temp for reuse (but then we have to think
        // about liveness across blocks).
        self.instr_ids.get(&iid).copied()
    }

    fn run_block(&mut self, bid: BlockId) {
        self.builder.start_block(bid);

        trace!("BLOCK {bid}");

        let block = self.builder.func.block_mut(bid);
        let params = std::mem::take(&mut block.params);
        let mut instrs = std::mem::take(&mut block.iids);

        let terminator = instrs.pop().unwrap();

        trace!(
            "  dead on entry: [{}]",
            self.liveness.blocks[bid]
                .dead_on_entry
                .iter()
                .map(|iid| format!("%{}", iid.as_usize()))
                .join(", ")
        );
        let dead_on_entry = self.liveness.blocks[bid].dead_on_entry.clone();
        for &iid in &dead_on_entry {
            // This iid is dead on entry to the block - we need to unset it.
            if let Some(lid) = self.consume_temp(iid) {
                trace!("  UNSET {}", ir::print::FmtLid(lid,));
                self.builder
                    .emit(Instr::Special(Special::IrToBc(IrToBc::UnsetL(lid))));
            }
        }

        for iid in params.into_iter().rev() {
            trace!("  PARAM {}", iid,);
            if dead_on_entry.contains(&iid) {
                // The result of this Param is immediately dead.
                self.builder
                    .emit(Instr::Special(Special::IrToBc(IrToBc::PopC)));
            } else {
                let lid = self.alloc_temp(iid);
                self.builder
                    .emit(Instr::Special(Special::IrToBc(IrToBc::PopL(lid))));
            }
        }

        for iid in instrs {
            self.run_instr(iid);
        }

        self.run_terminator(terminator);
    }

    fn run_instr(&mut self, iid: InstrId) {
        trace!(
            "  INSTR {}: {}",
            iid,
            ir::print::FmtInstr(&self.builder.func, iid)
        );

        let instr = self.builder.func.instr(iid);
        // Handle weird instructions
        match instr {
            Instr::Special(Special::Select(..)) => return self.run_select(iid),
            Instr::Terminator(_) => unreachable!(),
            _ => {}
        }

        // Push needed items onto the stack.
        let is_param = instr.is_param();
        let push_count = instr.push_count();
        self.push_operands(iid);

        // Push instr
        if !is_param {
            let bid = self.builder.cur_bid();
            self.builder.func.block_mut(bid).iids.push(iid);
        }

        // Pop/Transfer items off the stack.
        self.instr_cleanup(iid, push_count);
    }

    fn instr_cleanup(&mut self, iid: InstrId, push_count: usize) {
        if push_count == 0 {
            // no-op
        } else if push_count == 1 {
            if self.liveness.instr_last_use[iid].contains(&iid) {
                // The result of this Instr is immediately dead.
                self.builder
                    .emit(Instr::Special(Special::IrToBc(IrToBc::PopC)));
            } else {
                let lid = self.alloc_temp(iid);
                self.builder
                    .emit(Instr::Special(Special::IrToBc(IrToBc::PopL(lid))));
            }
        } else {
            // This is guaranteed to be followed by 'select' statements - which
            // will pop their values as necessary.
        }
    }

    fn run_select(&mut self, iid: InstrId) {
        // Select is weird - it just represents a stack slot from a previous
        // instruction.
        self.instr_cleanup(iid, 1);
    }

    fn run_terminator(&mut self, iid: InstrId) {
        // Deal with the terminator.  Although the inputs are the same as normal
        // instructions the outputs are handled differently.
        trace!(
            "  TERMINATOR {}: {}",
            iid,
            ir::print::FmtInstr(&self.builder.func, iid)
        );

        self.push_operands(iid);

        let bid = self.builder.cur_bid();

        let instr = self.builder.func.instr(iid);
        match *instr {
            // terminals
            Instr::Terminator(Terminator::JmpArgs(target, _, loc)) => {
                // The args have already been pushed as operands.
                // Convert the JmpArgs into a Jmp
                *self.builder.func.instr_mut(iid) = Instr::Terminator(Terminator::Jmp(target, loc));
            }

            Instr::Terminator(_) => {
                // No special handling.
            }

            // non-terminals
            _ => unreachable!(),
        }

        self.builder.func.block_mut(bid).iids.push(iid);
    }

    fn push_vids(&mut self, cur_iid: InstrId, vids: &[ValueId]) {
        // For each ValueId we need to know if this is the last use of the value
        // or not. If it's the last use then we need to teleport it from the
        // Local to the stack (PushL) - otherwise we copy it (CGetL).  We also
        // need to be careful about declaring last use when it's possible for a
        // ValueId to be used multiple times in the same call.

        // For each vid that is consumed by this Instr figure out the last index
        // that uses it.
        let mut vid_histogram: ValueIdMap<usize> = vids
            .iter()
            .copied()
            .filter(|vid| {
                vid.instr().map_or(false, |iid| {
                    self.liveness.instr_last_use[iid].contains(&cur_iid)
                })
            })
            .sorted_by_key(|vid| vid.raw())
            .dedup_with_count()
            .map(|(count, vid)| (vid, count))
            .collect();

        for vid in vids {
            let vid_consume = vid_histogram.get_mut(vid).map_or(false, |count| {
                *count -= 1;
                *count == 0
            });
            self.push_input(*vid, vid_consume);
        }
    }

    fn push_input(&mut self, vid: ValueId, vid_consume: bool) {
        match vid.full() {
            FullInstrId::None => {
                unreachable!();
            }
            FullInstrId::Instr(iid) => {
                if vid_consume {
                    // This instr consumes this value.
                    let lid = self.consume_temp(iid).unwrap();

                    // Peephole: If the previous instr was a PopL(lid) then
                    // we just elide the pair.
                    if let Some(iid) = self.builder.cur_block().iids.last() {
                        match self.builder.func.instr(*iid) {
                            Instr::Special(Special::IrToBc(IrToBc::PopL(popl))) if *popl == lid => {
                                self.builder.cur_block_mut().iids.pop();
                                return;
                            }
                            _ => {}
                        }
                    }

                    self.builder
                        .emit(Instr::Special(Special::IrToBc(IrToBc::PushL(lid))));
                } else {
                    // This instr does NOT consume the value.
                    let lid = self.lookup_temp(iid);
                    self.builder
                        .emit(Instr::Hhbc(instr::Hhbc::CGetL(lid, LocId::NONE)));
                }
            }
            FullInstrId::Imm(cid) => {
                self.builder
                    .emit(Instr::Special(Special::IrToBc(IrToBc::PushConstant(
                        ValueId::from_imm(cid),
                    ))));
            }
        }
    }

    fn push_operands(&mut self, iid: InstrId) {
        let instr = self.builder.func.instr(iid);
        match *instr {
            Instr::Call(..) | Instr::Terminator(Terminator::CallAsync(..)) => {
                self.push_call_operands(iid)
            }
            _ => {
                let operands = SmallVec::<[ValueId; 4]>::from_slice(instr.operands());
                self.push_vids(iid, &operands);
            }
        }
    }

    fn push_call_operands(&mut self, iid: InstrId) {
        let instr = self.builder.func.instr(iid);

        let call = match instr {
            Instr::Call(call) | Instr::Terminator(Terminator::CallAsync(call, _)) => call,
            _ => unreachable!(),
        };

        let num_inouts = call.inouts.as_ref().map_or(0, |inouts| inouts.len());

        let has_class = match &call.detail {
            ir::instr::CallDetail::FCallClsMethod { .. }
            | ir::instr::CallDetail::FCallClsMethodD { .. }
            | ir::instr::CallDetail::FCallClsMethodM { .. }
            | ir::instr::CallDetail::FCallClsMethodS { .. }
            | ir::instr::CallDetail::FCallClsMethodSD { .. }
            | ir::instr::CallDetail::FCallFuncD { .. }
            | ir::instr::CallDetail::FCallFunc => false,
            ir::instr::CallDetail::FCallCtor
            | ir::instr::CallDetail::FCallObjMethod { .. }
            | ir::instr::CallDetail::FCallObjMethodD { .. } => true,
        };

        // Need to copy the operands so we can write to the mutable
        // self.builder.
        let operands = SmallVec::<[ValueId; 4]>::from_slice(instr.operands());

        for _ in 0..num_inouts {
            self.builder
                .emit(Instr::Special(Special::IrToBc(IrToBc::PushUninit)));
        }

        if !has_class {
            self.builder
                .emit(Instr::Special(Special::IrToBc(IrToBc::PushUninit)));
        } else {
            self.push_vids(iid, &operands[0..1]);
        }
        self.builder
            .emit(Instr::Special(Special::IrToBc(IrToBc::PushUninit)));
        self.push_vids(iid, &operands[has_class as usize..]);
    }
}

fn find_next_unnamed_local_idx(func: &Func) -> usize {
    let mut next_unnamed_local_idx: usize = 0;

    for instr in func.body_instrs() {
        if let Some(idx) = instr
            .locals()
            .iter()
            .filter_map(|lid| match lid {
                LocalId::Named(_) => None,
                LocalId::Unnamed(idx) => Some(idx.as_usize()),
            })
            .max()
        {
            next_unnamed_local_idx = usize::max(next_unnamed_local_idx, idx + 1);
        }
    }

    next_unnamed_local_idx
}
