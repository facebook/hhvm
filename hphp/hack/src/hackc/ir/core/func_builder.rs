// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use crate::{instr::Special, Block, BlockId, Func, Instr, Literal, LocId, SrcLoc, ValueId};
use hash::HashMap;

pub struct FuncBuilder<'a> {
    pub func: Func<'a>,
    cur_bid: BlockId,
    pub loc_lookup: HashMap<SrcLoc, LocId>,
}

impl<'a> FuncBuilder<'a> {
    pub fn new() -> Self {
        let mut builder = Self::with_body(Func::new());
        builder.cur_bid = builder.alloc_bid();
        builder
    }

    pub fn with_body(func: Func<'a>) -> Self {
        FuncBuilder {
            func,
            cur_bid: BlockId::NONE,
            loc_lookup: Default::default(),
        }
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

    pub fn add_param(&mut self) -> ValueId {
        let iid = self
            .func
            .alloc_instr_in(self.cur_bid, Instr::Special(Special::Param));
        ValueId::from_instr(iid)
    }

    pub fn emit(&mut self, i: Instr) -> ValueId {
        assert!(!i.is_param());
        let iid = self.func.alloc_instr_in(self.cur_bid, i);
        ValueId::from_instr(iid)
    }

    pub fn emit_literal(&mut self, literal: Literal<'a>) -> ValueId {
        ValueId::from_literal(self.func.alloc_literal(literal))
    }

    pub fn into_func(mut self) -> Func<'a> {
        while !self.func.blocks.is_empty() {
            let bid = BlockId::from_usize(self.func.blocks.len() - 1);
            if self.func.blocks[bid].iids.is_empty() {
                self.func.blocks.pop();
            } else {
                break;
            }
        }

        self.func
    }

    pub fn start_block(&mut self, bid: BlockId) {
        self.cur_bid = bid;
    }
}
