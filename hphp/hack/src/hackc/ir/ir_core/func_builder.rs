// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::sync::Arc;

use hash::HashMap;

use crate::func::SrcLoc;
use crate::instr::BaseOp;
use crate::instr::FinalOp;
use crate::instr::HasLoc;
use crate::instr::HasOperands;
use crate::instr::Hhbc;
use crate::instr::IntermediateOp;
use crate::instr::MemberKey;
use crate::instr::MemberOp;
use crate::instr::Special;
use crate::Block;
use crate::BlockId;
use crate::Constant;
use crate::ConstantId;
use crate::Func;
use crate::Instr;
use crate::InstrId;
use crate::InstrIdMap;
use crate::LocId;
use crate::LocalId;
use crate::MOpMode;
use crate::PropName;
use crate::QueryMOp;
use crate::ReadonlyOp;
use crate::StringInterner;
use crate::ValueId;

pub struct FuncBuilder {
    pub func: Func,
    pub strings: Arc<StringInterner>,
    cur_bid: BlockId,
    pub loc_lookup: HashMap<SrcLoc, LocId>,
    pub constant_lookup: HashMap<Constant, ConstantId>,
    block_rewrite_stopped: bool,
    changed: bool,

    /// Used with rewrite_block(), maps old InstrId -> new InstrId, to handle
    /// replacing one instr with another.
    ///
    /// For example, if we fold "add 2,2" => "4", and a Constant for 4 already exists, we will
    /// redirect all references to the add's InstrId to use 4's ID instead. This is effectively
    /// on-the-fly copy propagation.
    ///
    /// InstrIds not in this table implicitly map to themselves, so the table is only populated
    /// at all once a remapping is needed. InstrIds in this table may or may not map to
    /// themselves.
    replacements: InstrIdMap<ValueId>,
}

impl FuncBuilder {
    pub fn build_func<F>(strings: Arc<StringInterner>, f: F) -> Func
    where
        F: FnOnce(&mut FuncBuilder),
    {
        let mut builder = FuncBuilder::with_func(Func::default(), strings);
        builder.cur_bid = builder.alloc_bid();
        f(&mut builder);
        builder.finish()
    }

    /// Create a FuncBuilder to edit a Func. If the caller can guarantee that no
    /// strings will be created (in an RPO pass which just re-orders blocks, for
    /// example) then it can create and pass in a
    /// StringInterner::read_only().
    pub fn with_func(func: Func, strings: Arc<StringInterner>) -> Self {
        let constant_lookup: HashMap<Constant, ConstantId> = func
            .constants
            .iter()
            .enumerate()
            .map(|(idx, constant)| (constant.clone(), ConstantId::from_usize(idx)))
            .collect();

        let loc_lookup: HashMap<SrcLoc, LocId> = func
            .locs
            .iter()
            .enumerate()
            .map(|(idx, loc)| (loc.clone(), LocId::from_usize(idx)))
            .collect();

        FuncBuilder {
            block_rewrite_stopped: false,
            changed: false,
            constant_lookup,
            cur_bid: BlockId::NONE,
            func,
            loc_lookup,
            replacements: Default::default(),
            strings,
        }
    }

    /// Similar to with_func() but borrows the Func instead of owning it.
    pub fn borrow_func<F, R>(borrowed: &mut Func, strings: Arc<StringInterner>, f: F) -> R
    where
        F: FnOnce(&mut FuncBuilder) -> R,
    {
        let func = std::mem::take(borrowed);
        let mut builder = FuncBuilder::with_func(func, strings);
        let r = f(&mut builder);
        *borrowed = builder.finish();
        r
    }

    /// Similar to borrow_func() but for a builder that doesn't need the
    /// StringInterner. A temporary StringInterner is used and checked when the
    /// handler returns. If any strings were interned during the call a panic is
    /// raised.
    pub fn borrow_func_no_strings<F, R>(borrowed: &mut Func, f: F) -> R
    where
        F: FnOnce(&mut FuncBuilder) -> R,
    {
        let tmp_strings = Arc::new(StringInterner);
        Self::borrow_func(borrowed, tmp_strings, f)
    }

    pub fn add_loc(&mut self, loc: SrcLoc) -> LocId {
        use std::collections::hash_map::Entry;
        match self.loc_lookup.entry(loc) {
            Entry::Occupied(e) => *e.get(),
            Entry::Vacant(e) => {
                let id = LocId::from_usize(self.func.locs.len());
                self.func.locs.push(e.key().clone());
                e.insert(id);
                id
            }
        }
    }

    pub fn alloc_bid(&mut self) -> BlockId {
        let bid = BlockId::from_usize(self.func.blocks.len());
        self.func.blocks.push(Block::default());
        bid
    }

    pub fn alloc_bid_based_on(&mut self, basis: BlockId) -> BlockId {
        let basis = &self.func.blocks[basis];
        let bid = BlockId::from_usize(self.func.blocks.len());
        self.func.blocks.push(Block {
            pname_hint: basis.pname_hint.clone(),
            tcid: basis.tcid,
            ..Block::default()
        });
        bid
    }

    pub fn alloc_bid_based_on_cur(&mut self) -> BlockId {
        self.alloc_bid_based_on(self.cur_bid)
    }

    pub fn cur_bid(&self) -> BlockId {
        self.cur_bid
    }

    pub fn cur_block(&self) -> &Block {
        &self.func.blocks[self.cur_bid]
    }

    pub fn cur_block_mut(&mut self) -> &mut Block {
        &mut self.func.blocks[self.cur_bid]
    }

    pub fn alloc_param(&mut self) -> ValueId {
        let iid = self.func.alloc_param_in(self.cur_bid);
        ValueId::from_instr(iid)
    }

    pub fn emit(&mut self, i: Instr) -> ValueId {
        assert!(!i.is_param());
        let iid = self.func.alloc_instr_in(self.cur_bid, i);
        ValueId::from_instr(iid)
    }

    pub fn emit_constant(&mut self, constant: Constant) -> ValueId {
        let lit_id = self
            .constant_lookup
            .entry(constant)
            .or_insert_with_key(|constant| self.func.alloc_constant(constant.clone()));
        ValueId::from_constant(*lit_id)
    }

    pub fn finish(self) -> Func {
        let mut func = self.func;
        while !func.blocks.is_empty() {
            let bid = BlockId::from_usize(func.blocks.len() - 1);
            if func.blocks[bid].iids.is_empty() {
                func.blocks.pop();
            } else {
                break;
            }
        }

        func
    }

    pub fn start_block(&mut self, bid: BlockId) {
        self.cur_bid = bid;
    }

    /// Walk through the block calling transformer.apply() on each Instr.
    pub fn rewrite_block(&mut self, bid: BlockId, transformer: &mut dyn TransformInstr) {
        // Steal the block contents so we can loop through them without a func
        // borrow, and clear the old block's body so we can append to it as we
        // go.
        let rb = &mut self.func.blocks[bid];
        let rb_len = rb.iids.len();
        let block_snapshot = std::mem::replace(&mut rb.iids, Vec::with_capacity(rb_len));

        // Process the block.
        self.cur_bid = bid;
        self.block_rewrite_stopped = false;

        let mut state = TransformState::new();

        for iid in block_snapshot {
            self.rewrite_instr(iid, transformer, &mut state);

            if self.block_rewrite_stopped {
                break;
            }
        }
        self.cur_bid = BlockId::NONE;
    }

    /// Cause the block rewrite loop to "break" once the current instr is
    /// finished.
    pub fn stop_block_rewrite(&mut self) {
        self.block_rewrite_stopped = true;
    }

    fn rewrite_instr(
        &mut self,
        iid: InstrId,
        transformer: &mut dyn TransformInstr,
        state: &mut TransformState,
    ) {
        // Temporarily steal from the func the instr we want to transform, so we
        // can manipulate it without borrowing the func. If we borrow it, we
        // wouldn't be able to create new instrs as part of the transformation.
        //
        // Swapping in a tombstone solves this problem and avoids needing to
        // clone().  The tombstone instr being there temporarily is OK because
        // we know nothing will look it up (although they technically could).
        let mut instr = std::mem::replace(&mut self.func.instrs[iid], Instr::tombstone());

        // Snap through any relaced instrs to the real value.
        self.replace_operands(&mut instr);

        let output = match instr {
            Instr::Special(Special::Select(src, idx)) => {
                let src = src.expect_instr("instr expected");
                self.rewrite_select(iid, instr, src, idx, state)
            }
            _ => transformer.apply(iid, instr, self, state),
        };

        // Apply the transformation and learn what to do with the result.
        match output {
            Instr::Special(Special::Copy(value)) => {
                // Do copy-propagation on the fly, emitting nothing to the block.
                self.changed = true;
                self.replace(iid, value);
            }
            Instr::Special(Special::Tombstone) => {}
            other => {
                self.func.instrs[iid] = other;
                self.func.blocks[self.cur_bid].iids.push(iid);
            }
        }
    }

    fn num_selects(instr: &Instr) -> u32 {
        let num_rets = match instr {
            Instr::Call(call) => call.num_rets,
            Instr::Hhbc(Hhbc::ClassGetTS(..)) => 2,
            Instr::MemberOp(ref op) => op.num_values() as u32,
            _ => 1,
        };
        if num_rets > 1 { num_rets } else { 0 }
    }

    fn rewrite_select(
        &mut self,
        iid: InstrId,
        instr: Instr,
        src: InstrId,
        idx: u32,
        state: &mut TransformState,
    ) -> Instr {
        if let Some(dst) = state.select_mapping.get(&iid) {
            // This acts like the Select was just replaced by Instr::copy().
            return Instr::copy(*dst);
        }

        if iid == InstrId::from_usize(src.as_usize() + 1 + idx as usize) {
            if idx < Self::num_selects(&self.func.instrs[src]) {
                // If the rewriter didn't do anything interesting with the
                // original Instr then we just want to keep the Select
                // as-is.
                return instr;
            }
        }

        panic!("Un-rewritten Select: InstrId {iid:?} ({instr:?})");
    }

    /// If iid has been replaced with another InstrId, return that, else return vid.
    pub fn find_replacement(&mut self, vid: ValueId) -> ValueId {
        let replacements = &mut self.replacements;

        if let Some(iid) = vid.instr() {
            if let Some(&replacement) = replacements.get(&iid) {
                // We found a replacement (rare), so go ahead and do the extra work of
                // following and optimizing the replacement chain.

                let mut prev = iid;
                let mut cur = replacement;

                while let Some(&next) = cur.instr().and_then(|cur| replacements.get(&cur)) {
                    // This is a multi-hop chain (very rare).
                    //
                    // At this point we have the chain: prev -> cur -> next.
                    // Shorten it as in union-find path splitting to speed
                    // future lookups by making prev point directly to next.
                    replacements.insert(prev, next);
                    prev = cur.expect_instr("not instr");
                    cur = next;
                }

                cur
            } else {
                vid
            }
        } else {
            vid
        }
    }

    /// Apply substitutions requested by replace() to each operand.
    pub fn replace_operands(&mut self, instr: &mut Instr) {
        if !self.replacements.is_empty() {
            for iid in instr.operands_mut() {
                let new = self.find_replacement(*iid);
                if *iid != new {
                    self.changed = true;
                    *iid = new;
                }
            }
        }
    }

    /// Indicate that during a rewrite all references to old_iid should be
    /// replaced with new_vid.
    ///
    /// NOTE: This does not set self.changed = true, because simply indicating
    /// that references to a dead instr should be replaced doesn't actually change
    /// the function.
    pub fn replace(&mut self, old_iid: InstrId, new_vid: ValueId) {
        if ValueId::from_instr(old_iid) != new_vid {
            self.replacements.insert(old_iid, new_vid);
        }
    }
}

/// Trait for instruction rewriters.
pub trait TransformInstr {
    /// Optionally transform the given Instr (e.g. constant-fold) and return the
    /// Instr that should be used instead. This is called by
    /// FuncBuilder::rewrite_block which visits each instr in a block and
    /// transforms it.
    ///
    /// Note that when apply() is called, the FuncBuilder has temporarily
    /// swapped an instr::tombstone() into the func.instrs slot being
    /// transformed.
    ///
    /// Some notable return values:
    ///
    /// - Returning instr::tombstone() indicates that FuncBuilder should do
    ///   nothing further, including appending no instr to the current block
    ///   being rewritten.  This is typically used when an instr should be
    ///   discarded. However it can also be used in any situation when the
    ///   callee knows what it is doing and doesn't want the FuncBuilder to do
    ///   anything further. This can include manually replacing the tombstone in
    ///   func.instrs[iid] with something else.
    ///
    /// - Returning a Copy instr will actually emit nothing and get
    ///   copy-propagated into later instrs, which is usually what you want, so
    ///   chains of simplifications can all happen in a single pass. If you
    ///   really want to emit an actual Copy instr for some reason you can
    ///   record one in func.instrs[iid], append iid to the block, then return
    ///   instr::tombstone(). Returning a Copy automatically sets rw.changed =
    ///   true.
    ///
    /// - Any other instr is recorded in func.instrs[iid] and iid is appended to
    ///   the current block being built up.
    fn apply(
        &mut self,
        iid: InstrId,
        instr: Instr,
        builder: &mut FuncBuilder,
        state: &mut TransformState,
    ) -> Instr;
}

/// Helper for TransformInstr
pub struct TransformState {
    select_mapping: InstrIdMap<ValueId>,
}

impl TransformState {
    fn new() -> Self {
        TransformState {
            select_mapping: Default::default(),
        }
    }

    /// When rewriting an Instr that returns multiple values it is required to
    /// call this function to rewrite the multiple return values.  The inputs to
    /// this function are the InstrId of the Instr::Special(Special::Select(..)).
    pub fn rewrite_select(&mut self, src: InstrId, dst: ValueId) {
        assert!(!self.select_mapping.contains_key(&src));
        self.select_mapping.insert(src, dst);
    }

    /// Like rewrite_select() this is used to note multiple return values - but
    /// uses the InstrId of the original instruction (not the Select) and a
    /// Select index.
    pub fn rewrite_select_idx(&mut self, src_iid: InstrId, src_idx: usize, dst_vid: ValueId) {
        self.rewrite_select(
            InstrId::from_usize(src_iid.as_usize() + 1 + src_idx),
            dst_vid,
        );
    }
}

/// Used like:
/// let vid = MemberOpBuilder::base_c(vid1, loc).emit_query_ec(builder, vid2);
pub struct MemberOpBuilder {
    pub operands: Vec<ValueId>,
    pub locals: Vec<LocalId>,
    pub base_op: BaseOp,
    pub intermediate_ops: Vec<IntermediateOp>,
}

impl MemberOpBuilder {
    // There are a lot of unhandled operations here - feel free to extend as
    // necessary...

    fn new(base_op: BaseOp) -> MemberOpBuilder {
        MemberOpBuilder {
            operands: Default::default(),
            locals: Default::default(),
            base_op,
            intermediate_ops: Default::default(),
        }
    }

    pub fn final_op(self, final_op: FinalOp) -> MemberOp {
        MemberOp {
            operands: self.operands.into_boxed_slice(),
            locals: self.locals.into_boxed_slice(),
            base_op: self.base_op,
            intermediate_ops: self.intermediate_ops.into_boxed_slice(),
            final_op,
        }
    }

    pub fn base_c(base: ValueId, loc: LocId) -> Self {
        let mode = MOpMode::None;
        let mut mop = Self::new(BaseOp::BaseC { mode, loc });
        mop.operands.push(base);
        mop
    }

    pub fn base_h(loc: LocId) -> Self {
        Self::new(BaseOp::BaseH { loc })
    }

    pub fn base_st(
        base: ValueId,
        prop: PropName,
        mode: MOpMode,
        readonly: ReadonlyOp,
        loc: LocId,
    ) -> Self {
        let mut mop = Self::new(BaseOp::BaseST {
            mode,
            readonly,
            loc,
            prop,
        });
        mop.operands.push(base);
        mop
    }

    pub fn isset_ec(mut self, key: ValueId) -> MemberOp {
        self.operands.push(key);
        let loc = self.base_op.loc_id();
        self.final_op(FinalOp::QueryM {
            key: MemberKey::EC,
            readonly: ReadonlyOp::Any,
            query_m_op: QueryMOp::Isset,
            loc,
        })
    }

    pub fn emit_isset_ec(self, builder: &mut FuncBuilder, key: ValueId) -> ValueId {
        let mop = self.isset_ec(key);
        builder.emit(Instr::MemberOp(mop))
    }

    pub fn query_ec(mut self, key: ValueId) -> MemberOp {
        self.operands.push(key);
        let loc = self.base_op.loc_id();
        self.final_op(FinalOp::QueryM {
            key: MemberKey::EC,
            readonly: ReadonlyOp::Any,
            query_m_op: QueryMOp::CGet,
            loc,
        })
    }

    pub fn emit_query_ec(self, builder: &mut FuncBuilder, key: ValueId) -> ValueId {
        let mop = self.query_ec(key);
        builder.emit(Instr::MemberOp(mop))
    }

    pub fn query_pt(self, key: PropName) -> MemberOp {
        let loc = self.base_op.loc_id();
        self.final_op(FinalOp::QueryM {
            key: MemberKey::PT(key),
            readonly: ReadonlyOp::Any,
            query_m_op: QueryMOp::CGet,
            loc,
        })
    }

    pub fn emit_query_pt(self, builder: &mut FuncBuilder, key: PropName) -> ValueId {
        let mop = self.query_pt(key);
        builder.emit(Instr::MemberOp(mop))
    }

    pub fn set_m_pt(mut self, key: PropName, value: ValueId) -> MemberOp {
        let loc = self.base_op.loc_id();
        let key = MemberKey::PT(key);
        let readonly = ReadonlyOp::Any;
        self.operands.push(value);
        self.final_op(FinalOp::SetM { key, readonly, loc })
    }

    pub fn emit_set_m_pt(
        self,
        builder: &mut FuncBuilder,
        key: PropName,
        value: ValueId,
    ) -> ValueId {
        let mop = self.set_m_pt(key, value);
        builder.emit(Instr::MemberOp(mop))
    }
}
