// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

use newtype::IdVec;

use crate::instr::HasOperands;
use crate::BlockId;
use crate::Func;
use crate::Instr;
use crate::InstrId;
use crate::TryCatchId;
use crate::ValueId;
use crate::ValueIdMap;

/// A Block represents a basic-block in a CFG. A well-formed Block contains zero
/// or more non-terminal InstrIds followed by a terminal InstrId. The
/// InstrIds refer to Instrs in the Block's Func's `Func::instrs` table.
#[derive(Clone, Debug, Default, PartialEq, Eq, Hash)]
pub struct Block {
    pub params: Vec<InstrId>,
    pub iids: Vec<InstrId>,
    /// A name for the Block. This is used for printing and to a minor extent
    /// when converting from IR to bytecode.
    pub pname_hint: Option<String>,
    /// TryCatchId for the Block. This defines which Try or Catch frame the Block
    /// is in.
    pub tcid: TryCatchId,
}

impl Block {
    /// Return an Iterator to the InstrIds that make up this Block.
    pub fn iids(&self) -> impl DoubleEndedIterator<Item = InstrId> + '_ {
        self.iids.iter().copied()
    }

    pub fn is_empty(&self) -> bool {
        self.iids.is_empty()
    }

    /// A well-formed Block is always terminated. This is only useful when
    /// building a Block.
    pub fn is_terminated(&self, func: &Func) -> bool {
        if let Some(&iid) = self.iids.last() {
            func.instr(iid).is_terminal()
        } else {
            false
        }
    }

    /// Visits all the instructions in the block and uses the `renumber` mapping
    /// on them. Does not renumber the instructions of the block itself.
    pub(crate) fn remap_vids(
        &mut self,
        instrs: &mut IdVec<InstrId, Instr>,
        remap: &ValueIdMap<ValueId>,
    ) {
        for iid in self.iids() {
            for iid2 in instrs[iid].operands_mut() {
                *iid2 = *remap.get(iid2).unwrap_or(iid2);
            }
        }
    }

    /// Returns the InstrId of the terminator for this Block. If the Block is
    /// malformed then the behavior is undefined.
    pub fn terminator_iid(&self) -> InstrId {
        self.iids
            .last()
            .copied()
            .unwrap_or_else(|| panic!("malformed block - empty block"))
    }

    /// Assign the name to the Block and return the Block.
    pub fn with_pname<T: ToString>(mut self, name: T) -> Block {
        self.pname_hint = Some(name.to_string());
        self
    }
}

#[derive(Debug)]
pub struct BlockIdIterator {
    pub(crate) current: BlockId,
    pub(crate) limit: BlockId,
}

impl Iterator for BlockIdIterator {
    type Item = BlockId;

    fn next(&mut self) -> Option<BlockId> {
        let current = self.current;
        if current == self.limit {
            None
        } else {
            self.current.0 += 1;
            Some(current)
        }
    }

    fn size_hint(&self) -> (usize, Option<usize>) {
        let n = self.limit.0 - self.current.0;
        (n as usize, Some(n as usize))
    }

    fn nth(&mut self, n: usize) -> Option<BlockId> {
        let max_skip = (self.limit.0 - self.current.0) as usize;
        let skip = std::cmp::min(max_skip, n) as u32;
        self.current.0 += skip;
        self.next()
    }
}

impl DoubleEndedIterator for BlockIdIterator {
    fn next_back(&mut self) -> Option<BlockId> {
        let limit = self.limit;
        if limit == self.current {
            None
        } else {
            self.limit.0 -= 1;
            Some(BlockId(limit.0 - 1))
        }
    }

    fn nth_back(&mut self, n: usize) -> Option<BlockId> {
        let max_skip = (self.limit.0 - self.current.0) as usize;
        let skip = std::cmp::min(max_skip, n) as u32;
        self.limit.0 -= skip;
        self.next_back()
    }
}

impl std::iter::ExactSizeIterator for BlockIdIterator {
    fn len(&self) -> usize {
        (self.limit.0 - self.current.0) as usize
    }
}

impl std::iter::FusedIterator for BlockIdIterator {}
