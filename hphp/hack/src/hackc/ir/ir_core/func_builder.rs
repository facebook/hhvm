// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use hash::HashMap;

use crate::instr::HasOperands;
use crate::instr::Special;
use crate::Block;
use crate::BlockId;
use crate::Func;
use crate::Instr;
use crate::InstrId;
use crate::InstrIdMap;
use crate::Literal;
use crate::LiteralId;
use crate::LocId;
use crate::SrcLoc;
use crate::ValueId;

pub struct FuncBuilder<'a> {
    pub func: Func<'a>,
    cur_bid: BlockId,
    pub loc_lookup: HashMap<SrcLoc, LocId>,
    pub literal_lookup: HashMap<Literal<'a>, LiteralId>,
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

impl<'a> FuncBuilder<'a> {
    pub fn build_func<F>(f: F) -> Func<'a>
    where
        F: FnOnce(&mut FuncBuilder<'a>),
    {
        let mut builder = FuncBuilder::with_func(Func::default());
        builder.cur_bid = builder.alloc_bid();
        f(&mut builder);
        builder.finish()
    }

    pub fn with_func(func: Func<'a>) -> Self {
        let literal_lookup: HashMap<Literal<'a>, LiteralId> = func
            .literals
            .iter()
            .enumerate()
            .map(|(idx, literal)| (literal.clone(), LiteralId::from_usize(idx)))
            .collect();
        FuncBuilder {
            func,
            cur_bid: BlockId::NONE,
            loc_lookup: Default::default(),
            literal_lookup,
            block_rewrite_stopped: false,
            changed: false,
            replacements: Default::default(),
        }
    }

    pub fn borrow_func<F, R>(borrowed: &mut Func<'a>, f: F) -> R
    where
        F: FnOnce(&mut FuncBuilder<'a>) -> R,
    {
        let func = std::mem::take(borrowed);
        let mut builder = FuncBuilder::with_func(func);
        let r = f(&mut builder);
        *borrowed = builder.finish();
        r
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

    pub fn emit_literal(&mut self, literal: Literal<'a>) -> ValueId {
        let lit_id = self
            .literal_lookup
            .entry(literal)
            .or_insert_with_key(|literal| self.func.alloc_literal(literal.clone()));
        ValueId::from_literal(*lit_id)
    }

    pub fn finish(self) -> Func<'a> {
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

        for iid in block_snapshot {
            self.rewrite_instr(iid, transformer);

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

    fn rewrite_instr(&mut self, iid: InstrId, transformer: &mut dyn TransformInstr) {
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

        // Apply the transformation and learn what to do with the result.
        match transformer.apply(iid, instr, self) {
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
    fn apply<'a>(&mut self, iid: InstrId, instr: Instr, builder: &mut FuncBuilder<'a>) -> Instr;
}
