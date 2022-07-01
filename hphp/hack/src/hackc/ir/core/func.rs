// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use crate::{
    block::BlockIdIterator, instr::Terminator, Attr, Attribute, Block, BlockId, BlockIdMap,
    Coeffects, HasEdges, Instr, InstrId, Literal, LiteralId, LocId, SrcLoc, Type, UnitStringId,
    ValueId, ValueIdMap,
};

use ffi::Str;
use newtype::{newtype_int, IdVec};

pub use hhbc::{
    hhas_function::HhasFunctionFlags as FunctionFlags, hhas_method::HhasMethodFlags as MethodFlags,
    hhas_pos::HhasSpan, FunctionName, MethodName, Visibility,
};

/// Func parameters.
#[derive(Debug)]
pub struct Param<'a> {
    pub name: UnitStringId,
    pub is_variadic: bool,
    pub is_inout: bool,
    pub is_readonly: bool,
    pub user_attributes: Vec<Attribute<'a>>,
    pub ty: Type<'a>,
    /// This is the BlockId which is the entrypoint for where initialization of
    /// this param begins.  The string is the code string which was used to
    /// compile the initialization value (for reflection).
    ///
    /// For example, for the function:
    ///   function myfn(int $a = 2 + 3)
    ///
    /// The block will initialize `$a` to 5 and the string will be "2 + 3".
    pub default_value: Option<(BlockId, Str<'a>)>,
}

newtype_int!(ExFrameId, u32, ExFrameIdMap, ExFrameIdSet);

/// Exception Frame State
///
/// Every Block contains a TryCatchId which indicates if exceptions thrown from
/// that block should be caught or not. If caught then they refer to an
/// ExFrameId which tells the system where to jump on an exception.
///
/// Every try/catch block is made of two sections: the 'try' section and the
/// 'catch' section. Again - this could probably be computed but is easier to
/// store for now.
///
/// Q: Why like this and not like LLVM?
/// A: LLVM does exception handling with an `invoke` instruction instead of a
///    call for calls that can throw. The problem is that most HHVM instructions
///    can throw whereas most LLVM instructions cannot - so everythng would need
///    to be an `invoke` in that model.  In addition reconstructing the HHVM
///    TryBegin/TryMiddle/TryEnd model directly from the LLVM style state can be
///    tricky.
#[derive(Debug)]
pub struct ExFrame {
    /// The parent of this ExFrame. The parent is probably not strictly
    /// necessary and could be recomputed from the try/catch structure - but
    /// makes generating the bytecode a little simpler for now.
    pub parent: TryCatchId,

    /// Where exceptions thrown from this block should be handled. The
    /// `catch_bid` should take a single Block parameter which receives the
    /// thrown exception.
    pub catch_bid: BlockId,
}

#[derive(Copy, Clone, Debug, PartialEq, Eq, Hash)]
pub enum TryCatchId {
    /// This block doesn't catch exceptions.
    None,

    /// This block is part of the 'try' portion of this exception frame.
    Try(ExFrameId),

    /// This block is part of the 'catch' portion of this exception frame.
    Catch(ExFrameId),
}

impl TryCatchId {
    pub fn is_none(&self) -> bool {
        matches!(self, Self::None)
    }

    pub fn exid(&self) -> ExFrameId {
        match self {
            Self::None => ExFrameId::NONE,
            Self::Try(id) | Self::Catch(id) => *id,
        }
    }
}

impl Default for TryCatchId {
    fn default() -> Self {
        TryCatchId::None
    }
}

/// A Func represents a body of code. It's used for both Function and Method.
///
/// Code is organized into basic-Blocks which contain a series of 0 or more
/// non-terminal instructions ending with a terminal instruction.
///
/// A Block can take parameters which are used to pass data when control is
/// transferred to the Block.
///
///   - A Block can end with the ControlFlow::JmpArgs instruction to pass data
///     to a successor block (these correspond to phi-nodes in the SSA graph).
///
///   - A Block which is the target of exception handling will be passed the
///     exception object as a Block parameter.
///
///   - Async calls (Instr::CallAsync) are terminators which transfer control to
///     one of two successor Blocks depending on whether the async completed
///     (eager) or had to await. The eager successor is passed the call's return
///     value as a Block parameter whereas the await successor is passed the
///     Await object as a Block parameter.
///
/// Funcs also contain some amount of storage for data associated with the code
/// - such as constant values or locations - so they can referred to with
/// indices instead of having to deal with refereces and ownerhip (and to make
/// them smaller).
///
/// Exception frames are tracked as a separate tree structure made up of Blocks.
/// Each exception frame says where to jump in the case of a thrown exception.
///
#[derive(Debug, Default)]
pub struct Func<'a> {
    pub blocks: IdVec<BlockId, Block>,
    pub doc_comment: Option<Str<'a>>,
    pub ex_frames: ExFrameIdMap<ExFrame>,
    pub instrs: IdVec<InstrId, Instr>,
    pub is_memoize_wrapper: bool,
    pub is_memoize_wrapper_lsb: bool,
    pub literals: IdVec<LiteralId, Literal<'a>>,
    pub locs: IdVec<LocId, SrcLoc>,
    pub num_iters: usize,
    pub params: Vec<Param<'a>>,
    pub return_type: Type<'a>,
}

impl<'a> Func<'a> {
    // By definition the entry block is block zero.
    pub const ENTRY_BID: BlockId = BlockId(0);

    pub fn new() -> Self {
        Self::default()
    }

    pub fn alloc_literal(&mut self, literal: Literal<'a>) -> LiteralId {
        let lid = LiteralId::from_usize(self.literals.len());
        self.literals.push(literal);
        lid
    }

    pub fn alloc_instr(&mut self, i: Instr) -> InstrId {
        let iid = InstrId::from_usize(self.instrs.len());
        self.instrs.push(i);
        iid
    }

    pub fn alloc_instr_in(&mut self, bid: BlockId, i: Instr) -> InstrId {
        let iid = self.alloc_instr(i);
        self.blocks[bid].iids.push(iid);
        iid
    }

    pub fn alloc_param_in(&mut self, bid: BlockId) -> InstrId {
        let iid = self.alloc_instr(Instr::param());
        self.blocks[bid].params.push(iid);
        iid
    }

    pub fn alloc_bid(&mut self, block: Block) -> BlockId {
        let bid = BlockId::from_usize(self.blocks.len());
        self.blocks.push(block);
        bid
    }

    pub fn block(&self, bid: BlockId) -> &Block {
        self.get_block(bid)
            .unwrap_or_else(|| panic!("{:?} not found", bid))
    }

    pub fn block_ids(&self) -> BlockIdIterator {
        BlockIdIterator {
            current: Self::ENTRY_BID,
            limit: BlockId(self.blocks.len() as u32),
        }
    }

    pub fn block_mut(&mut self, bid: BlockId) -> &mut Block {
        self.blocks.get_mut(bid).unwrap()
    }

    /// Yields normal instructions in bodies (not constants or params).
    pub fn body_iids(&self) -> impl DoubleEndedIterator<Item = InstrId> + '_ {
        self.block_ids().flat_map(|bid| self.blocks[bid].iids())
    }

    pub fn body_instrs(&self) -> impl DoubleEndedIterator<Item = &Instr> + '_ {
        self.body_iids().map(|iid| &self.instrs[iid])
    }

    pub fn catch_target(&self, bid: BlockId) -> BlockId {
        fn get_catch_frame<'b>(func: &'b Func<'_>, tcid: TryCatchId) -> Option<&'b ExFrame> {
            match tcid {
                TryCatchId::None => None,
                TryCatchId::Try(exid) => Some(&func.ex_frames[&exid]),
                TryCatchId::Catch(exid) => {
                    let parent = func.ex_frames[&exid].parent;
                    get_catch_frame(func, parent)
                }
            }
        }

        let tcid = self.block(bid).tcid;
        if let Some(frame) = get_catch_frame(self, tcid) {
            frame.catch_bid
        } else {
            BlockId::NONE
        }
    }

    pub fn literal(&self, lid: LiteralId) -> &Literal<'a> {
        self.get_literal(lid).unwrap()
    }

    pub fn edges(&self, bid: BlockId) -> &[BlockId] {
        self.terminator(bid).edges()
    }

    pub fn edges_mut(&mut self, bid: BlockId) -> &mut [BlockId] {
        self.terminator_mut(bid).edges_mut()
    }

    pub fn emit(&mut self, bid: BlockId, instr: Instr) -> ValueId {
        let iid = self.alloc_instr(instr);
        self.block_mut(bid).iids.push(iid);
        ValueId::from_instr(iid)
    }

    pub fn get_block(&self, bid: BlockId) -> Option<&Block> {
        self.blocks.get(bid)
    }

    pub fn get_literal(&self, lid: LiteralId) -> Option<&Literal<'a>> {
        self.literals.get(lid)
    }

    pub fn get_edges(&self, bid: BlockId) -> Option<&[BlockId]> {
        self.get_terminator(bid).map(|instr| instr.edges())
    }

    pub fn get_instr(&self, iid: InstrId) -> Option<&Instr> {
        self.instrs.get(iid)
    }

    pub fn get_instr_mut(&mut self, iid: InstrId) -> Option<&mut Instr> {
        self.instrs.get_mut(iid)
    }

    pub fn get_loc(&self, loc: LocId) -> Option<&SrcLoc> {
        self.locs.get(loc)
    }

    pub fn get_terminator(&self, bid: BlockId) -> Option<&Terminator> {
        let block = self.block(bid);
        let iid = *block.iids.last()?;
        let instr = self.get_instr(iid)?;
        match instr {
            Instr::Terminator(terminator) => Some(terminator),
            _ => None,
        }
    }

    pub fn instr(&self, iid: InstrId) -> &Instr {
        self.get_instr(iid).unwrap()
    }

    pub fn instrs_len(&self) -> usize {
        self.instrs.len()
    }

    pub fn instr_mut(&mut self, iid: InstrId) -> &mut Instr {
        self.get_instr_mut(iid).unwrap()
    }

    pub fn is_empty(&self, bid: BlockId) -> bool {
        self.block(bid).is_empty()
    }

    pub fn is_terminated(&self, bid: BlockId) -> bool {
        self.block(bid).is_terminated(self)
    }

    pub fn is_terminal(&self, vid: ValueId) -> bool {
        vid.instr()
            .map_or(false, |iid| self.instr(iid).is_terminal())
    }

    pub fn loc(&self, loc: LocId) -> &SrcLoc {
        assert!(loc != LocId::NONE);
        self.get_loc(loc).unwrap()
    }

    // Visits all the instructions in the block and uses the `remap` mapping on
    // them. Does not renumber the instructions of the block itself.
    pub fn remap_block_vids(&mut self, bid: BlockId, remap: &ValueIdMap<ValueId>) {
        let block = self.blocks.get_mut(bid).unwrap();
        block.remap_vids(&mut self.instrs, remap)
    }

    // Rewrite instructions using the remap mapping. Won't renumber the
    // instructions themselves (so a remapping with %1 -> %2 won't remap
    // instruction %1 to %2, just references to %1 will be remapped to %2).
    pub fn remap_vids(&mut self, remap: &ValueIdMap<ValueId>) {
        for bid in self.block_ids() {
            self.remap_block_vids(bid, remap);
        }
    }

    /// Rewrite BlockIds using the remap remapping. Won't renumber the blocks
    /// themselves (so a remapping with b1 -> b2 won't remap block b1 to b2,
    /// just references to b1 will be remapped to b2).
    pub fn remap_bids(&mut self, remap: &BlockIdMap<BlockId>) {
        for param in &mut self.params {
            if let Some((bid, _)) = param.default_value.as_mut() {
                *bid = remap.get(bid).copied().unwrap_or(*bid);
            }
        }

        for instr in self.instrs.iter_mut() {
            for bid in instr.edges_mut() {
                *bid = remap.get(bid).copied().unwrap_or(*bid);
            }
        }
    }

    pub fn terminator(&self, bid: BlockId) -> &Terminator {
        let iid = self.block(bid).terminator_iid();
        match self.instr(iid) {
            Instr::Terminator(terminator) => terminator,
            _ => panic!("Non-Terminator found in terminator location {}", iid),
        }
    }

    pub fn terminator_mut(&mut self, bid: BlockId) -> &mut Terminator {
        let iid = self.block(bid).terminator_iid();
        match self.instr_mut(iid) {
            Instr::Terminator(terminator) => terminator,
            _ => panic!("Non-Terminator found in terminator location {}", iid),
        }
    }
}

/// A top-level Hack function.
#[derive(Debug)]
pub struct Function<'a> {
    pub attributes: Vec<Attribute<'a>>,
    pub attrs: Attr,
    pub coeffects: Coeffects<'a>,
    pub flags: FunctionFlags,
    pub name: FunctionName<'a>,
    pub func: Func<'a>,
    pub span: HhasSpan,
}

/// A Hack method contained within a Class.
#[derive(Debug)]
pub struct Method<'a> {
    pub attributes: Vec<Attribute<'a>>,
    pub attrs: Attr,
    pub coeffects: Coeffects<'a>,
    pub flags: MethodFlags,
    pub func: Func<'a>,
    pub name: MethodName<'a>,
    pub span: HhasSpan,
    pub visibility: Visibility,
}
