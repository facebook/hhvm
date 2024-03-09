// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use newtype::newtype_int;
use newtype::IdVec;

use crate::block::BlockIdIterator;
use crate::instr::Terminator;
use crate::Attr;
use crate::Attribute;
use crate::Block;
use crate::BlockId;
use crate::BlockIdMap;
use crate::ClassName;
use crate::ClassNameMap;
use crate::Coeffects;
use crate::Constant;
use crate::ConstantId;
use crate::FunctionFlags;
use crate::FunctionName;
use crate::HasEdges;
use crate::Instr;
use crate::InstrId;
use crate::LocId;
use crate::LocalId;
use crate::MethodFlags;
use crate::MethodName;
use crate::StringId;
use crate::TypeInfo;
use crate::UnitBytesId;
use crate::ValueId;
use crate::ValueIdMap;
use crate::Visibility;

#[derive(Copy, Clone, Debug, Hash, Eq, PartialEq)]
pub struct Filename(pub UnitBytesId);

impl Default for Filename {
    fn default() -> Self {
        Self(UnitBytesId::EMPTY)
    }
}

impl Filename {
    pub const NONE: Filename = Filename(UnitBytesId::EMPTY);
}

#[derive(Clone, Debug, Default, Hash, Eq, PartialEq)]
pub struct SrcLoc {
    pub filename: Filename,
    pub line_begin: i32,
    pub col_begin: i32,
    pub line_end: i32,
    pub col_end: i32,
}

impl SrcLoc {
    pub fn to_hhbc(&self) -> hhbc::SrcLoc {
        hhbc::SrcLoc {
            line_begin: self.line_begin,
            col_begin: self.col_begin,
            line_end: self.line_end,
            col_end: self.col_end,
        }
    }

    pub fn to_span(&self) -> hhbc::Span {
        hhbc::Span {
            line_begin: self.line_begin,
            line_end: self.line_end,
        }
    }

    pub fn from_hhbc(filename: Filename, src_loc: &hhbc::SrcLoc) -> Self {
        Self {
            filename,
            line_begin: src_loc.line_begin,
            col_begin: src_loc.col_begin,
            line_end: src_loc.line_end,
            col_end: src_loc.col_end,
        }
    }

    pub fn from_span(filename: Filename, span: &hhbc::Span) -> Self {
        Self {
            filename,
            line_begin: span.line_begin,
            col_begin: 0,
            line_end: span.line_end,
            col_end: 0,
        }
    }
}

/// Func parameters.
#[derive(Clone, Debug)]
pub struct Param {
    pub name: StringId,
    pub is_variadic: bool,
    pub is_inout: bool,
    pub is_readonly: bool,
    pub user_attributes: Vec<Attribute>,
    pub ty: TypeInfo,
    /// This is the BlockId which is the entrypoint for where initialization of
    /// this param begins.  The string is the code string which was used to
    /// compile the initialization value (for reflection).
    ///
    /// For example, for the function:
    ///   function myfn(int $a = 2 + 3)
    ///
    /// The block will initialize `$a` to 5 and the string will be "2 + 3".
    pub default_value: Option<DefaultValue>,
}

#[derive(Clone, Debug)]
pub struct DefaultValue {
    /// What block to jump to to initialize this default value
    pub init: BlockId,

    /// Source text for the default value initializer expression (for reflection).
    pub expr: Vec<u8>,
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
#[derive(Clone, Debug)]
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
#[derive(Default)]
pub enum TryCatchId {
    /// This block doesn't catch exceptions.
    #[default]
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

/// A Func represents a body of code. It's used for both Function and Method.
///
/// Code is organized into basic-Blocks which contain a series of 0 or more
/// non-terminal instructions ending with a terminal instruction.
///
/// A Block can take parameters which are used to pass data when control is
/// transferred to the Block. Block parameters correspond to SSA phi nodes.
///
///   - A Block can end with the ControlFlow::JmpArgs instruction to pass data
///     to a successor block. These arguments correspond to SSA phi node inputs.
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
#[derive(Clone, Debug, Default)]
pub struct Func {
    pub attributes: Vec<Attribute>,
    pub attrs: Attr,
    pub blocks: IdVec<BlockId, Block>,
    pub coeffects: Coeffects,
    pub doc_comment: Option<Vec<u8>>,
    pub ex_frames: ExFrameIdMap<ExFrame>,
    pub instrs: IdVec<InstrId, Instr>,
    pub is_memoize_wrapper: bool,
    pub is_memoize_wrapper_lsb: bool,
    pub constants: IdVec<ConstantId, Constant>,
    pub locs: IdVec<LocId, SrcLoc>,
    pub num_iters: usize,
    pub params: Vec<Param>,
    pub return_type: TypeInfo,
    /// shadowed_tparams are the set of tparams on a method which shadow a
    /// tparam on the containing class.
    pub shadowed_tparams: Vec<ClassName>,
    pub loc_id: LocId,
    pub tparams: ClassNameMap<TParamBounds>,
}

impl Func {
    // By definition the entry block is block zero.
    pub const ENTRY_BID: BlockId = BlockId(0);

    pub fn alloc_constant(&mut self, constant: Constant) -> ConstantId {
        let cid = ConstantId::from_usize(self.constants.len());
        self.constants.push(constant);
        cid
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
        fn get_catch_frame(func: &Func, tcid: TryCatchId) -> Option<&ExFrame> {
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

    pub fn constant(&self, cid: ConstantId) -> &Constant {
        self.get_constant(cid).unwrap()
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

    pub fn get_constant(&self, cid: ConstantId) -> Option<&Constant> {
        self.constants.get(cid)
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

    pub fn get_param_by_lid(&self, lid: LocalId) -> Option<&Param> {
        match lid {
            LocalId::Named(name) => self.params.iter().find(|p| p.name == name),
            LocalId::Unnamed(_) => None,
        }
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

    pub fn is_reified(&self) -> bool {
        self.attributes
            .iter()
            .any(|attr| attr.name.as_str() == "__Reified")
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
            if let Some(dv) = param.default_value.as_mut() {
                dv.init = remap.get(&dv.init).copied().unwrap_or(dv.init);
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
pub struct Function {
    pub flags: FunctionFlags,
    pub name: FunctionName,
    pub func: Func,
}

/// A Hack method contained within a Class.
#[derive(Debug)]
pub struct Method {
    pub flags: MethodFlags,
    pub func: Func,
    pub name: MethodName,
    pub visibility: Visibility,
}

#[derive(Clone, Debug, Default)]
pub struct TParamBounds {
    pub bounds: Vec<TypeInfo>,
}
