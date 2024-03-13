// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::collections::VecDeque;
use std::sync::Arc;

use hash::HashMap;
use hhbc::BytesId;
use hhbc::Instruct;
use ir::instr;
use ir::instr::Special;
use ir::BlockId;
use ir::BlockIdMap;
use ir::FuncBuilder;
use ir::Instr;
use ir::LocId;
use ir::LocalId;
use ir::ValueId;
use ir::VarId;
use newtype::newtype_int;

use crate::convert::UnitState;
use crate::sequence::Sequence;

pub(crate) type LabelMap<T> =
    std::collections::HashMap<hhbc::Label, T, newtype::BuildIdHasher<u32>>;

// An Addr represents an index into an Instruct Vec.
newtype_int!(Addr, u32, AddrMap, AddrIndexSet);

impl Addr {
    pub(crate) const ENTRY: Addr = Addr(0);

    pub(crate) fn incr(self) -> Addr {
        Addr::from_usize(self.as_usize() + 1)
    }

    /// Can't iterator on Range<T> without implementing `Step for T` which is
    /// unstable.
    pub(crate) fn range_to_iter(range: std::ops::Range<Addr>) -> impl Iterator<Item = Addr> {
        (range.start.0..range.end.0).map(Addr)
    }
}

/// Context used during conversion of an HhasBody to an ir::Func.
pub(crate) struct Context<'b> {
    /// Conversion from hhbc::AdataId to the hhbc:TypedValue it represents.
    pub(crate) adata_lookup: &'b HashMap<hhbc::AdataId, Arc<ir::TypedValue>>,
    /// Source instructions from the bytecode
    pub(crate) instrs: &'b [Instruct],
    pub(crate) addr_to_seq: AddrMap<Sequence>,
    pub(crate) builder: FuncBuilder,
    bid_to_addr: BlockIdMap<Addr>,
    pub(crate) label_to_addr: LabelMap<Addr>,
    pub(crate) loc: ir::LocId,
    pub(crate) member_op: Option<MemberOpBuilder>,
    pub(crate) named_local_lookup: Vec<LocalId>,
    pub(crate) stack: VecDeque<ir::ValueId>,
    pub(crate) work_queue_pending: VecDeque<Addr>,
    work_queue_inserted: AddrIndexSet,
}

impl<'b> Context<'b> {
    pub(crate) fn new(func: ir::Func, instrs: &'b [Instruct], unit_state: &'b UnitState) -> Self {
        let mut builder = FuncBuilder::with_func(func);
        let (label_to_addr, bid_to_addr, addr_to_seq) = Sequence::compute(&mut builder, instrs);

        let mut ctx = Context {
            adata_lookup: &unit_state.adata_lookup,
            instrs,
            addr_to_seq,
            label_to_addr,
            bid_to_addr,
            builder,
            loc: ir::LocId::NONE,
            member_op: None,
            named_local_lookup: Default::default(),
            stack: Default::default(),
            work_queue_pending: Default::default(),
            work_queue_inserted: Default::default(),
        };

        ctx.add_work_addr(Addr::ENTRY);

        ctx
    }

    pub(crate) fn add_work_addr(&mut self, addr: Addr) {
        if self.work_queue_inserted.insert(addr) {
            self.work_queue_pending.push_back(addr);
        }
    }

    pub(crate) fn add_work_bid(&mut self, bid: BlockId) {
        let addr = self.bid_to_addr[&bid];
        self.add_work_addr(addr);
    }

    /// Allocate a new Param, register it with the current Block and push it
    /// onto the stack.
    pub(crate) fn alloc_and_push_param(&mut self) -> ir::ValueId {
        let iid = self.builder.alloc_param();
        self.push(iid);
        iid
    }

    pub(crate) fn alloc_bid(&mut self) -> BlockId {
        let bid = self.builder.alloc_bid();
        let tcid = self.builder.cur_block().tcid;
        self.builder.func.block_mut(bid).tcid = tcid;
        bid
    }

    pub(crate) fn debug_get_stack(&self) -> &VecDeque<ir::ValueId> {
        &self.stack
    }

    pub(crate) fn emit(&mut self, i: ir::Instr) -> ir::ValueId {
        self.builder.emit(i)
    }

    pub(crate) fn emit_constant(&mut self, constant: ir::Constant) -> ir::ValueId {
        self.builder.emit_constant(constant)
    }

    /// Emit an Instr and push its return onto the stack.
    pub(crate) fn emit_push(&mut self, i: ir::Instr) -> ir::ValueId {
        let vid = self.emit(i);
        self.push(vid);
        vid
    }

    /// Emit a Constant and push its return onto the stack.
    pub(crate) fn emit_push_constant(&mut self, lc: ir::Constant) -> ir::ValueId {
        let vid = self.emit_constant(lc);
        self.push(vid);
        vid
    }

    /// For an Instr that returns multiple values emit_push `count` Select
    /// instructions.
    pub(crate) fn emit_selects(&mut self, vid: ValueId, count: u32) {
        // We need to push the selects onto the stack in reverse order but
        // we want them to follow the instr in ascending order.
        let mut selects: Vec<_> = (0..count)
            .map(|i| self.emit(Instr::Special(Special::Select(vid, i))))
            .collect();
        selects.reverse();
        for instr in selects {
            self.push(instr);
        }
    }

    pub(crate) fn intern_bytes_id(&mut self, s: BytesId) -> BytesId {
        s
    }

    pub(crate) fn pop(&mut self) -> ir::ValueId {
        assert!(!self.stack.is_empty());
        self.stack.pop_back().unwrap()
    }

    pub(crate) fn pop_n(&mut self, n: u32) -> VecDeque<ir::ValueId> {
        self.stack.split_off(self.stack.len() - n as usize)
    }

    pub(crate) fn push(&mut self, vid: ir::ValueId) {
        self.stack.push_back(vid);
    }

    pub(crate) fn push_n(&mut self, values: impl Iterator<Item = ir::ValueId>) {
        self.stack.extend(values);
    }

    /// Save the current stack into VarIds.
    pub(crate) fn spill_stack(&mut self) -> usize {
        let sz = self.stack.len();
        let stack = std::mem::take(&mut self.stack);
        for (i, vid) in stack.into_iter().enumerate().rev() {
            let var = VarId::from_usize(i);
            self.emit(Instr::set_var(var, vid));
        }
        sz
    }

    pub(crate) fn stack_clear(&mut self) {
        self.stack.clear();
    }

    // Note that idx is relative to the end of the stack!
    pub(crate) fn stack_get(&mut self, idx: usize) -> ir::ValueId {
        // Need to treat missing elements like `pop`.
        assert!(idx < self.stack.len());
        self.stack[self.stack.len() - 1 - idx]
    }

    pub(crate) fn target_from_label(&mut self, label: hhbc::Label, stack_size: usize) -> BlockId {
        let addr = *self.label_to_addr.get(&label).unwrap();
        self.target_from_addr(addr, stack_size)
    }

    pub(crate) fn target_from_addr(&mut self, addr: Addr, stack_size: usize) -> BlockId {
        let seq = self.addr_to_seq.get_mut(&addr).unwrap();
        if let Some(old_size) = seq.input_stack_size.replace(stack_size) {
            assert_eq!(old_size, stack_size);
        }
        let bid = seq.bid;
        self.add_work_addr(addr);
        bid
    }

    /// Restore the current stack from VarIds.
    pub(crate) fn unspill_stack(&mut self, n: usize) {
        for i in 0..n {
            let var = VarId::from_usize(i);
            self.emit_push(Instr::get_var(var));
        }
    }
}

pub(crate) struct MemberOpBuilder {
    pub(crate) operands: Vec<ValueId>,
    pub(crate) locals: Vec<LocalId>,
    pub(crate) base_op: instr::BaseOp,
    pub(crate) intermediate_ops: Vec<instr::IntermediateOp>,
}

impl MemberOpBuilder {
    pub(crate) fn into_member_op(self, final_op: instr::FinalOp) -> instr::MemberOp {
        instr::MemberOp {
            final_op,
            operands: self.operands.into(),
            locals: self.locals.into(),
            base_op: self.base_op,
            intermediate_ops: self.intermediate_ops.into(),
        }
    }
}

pub(crate) fn add_loc(builder: &mut FuncBuilder, loc: &hhbc::SrcLoc) -> LocId {
    builder.add_loc(*loc)
}
