// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::borrow::Cow;
use std::ops::Range;

use hhbc::Instruct;
use ir::BlockId;
use ir::BlockIdMap;
use ir::ExFrameId;
use ir::FuncBuilder;
use ir::LocId;
use ir::TryCatchId;

use crate::context::Addr;
use crate::context::AddrMap;
use crate::context::LabelMap;

/// A Sequence is a linear sequence of HHBC Instructs that doesn't cross either
/// a Label or a Try/Catch boundary.
///
///     (section 1)
///     try {
///       (section 2)
///     label1:
///       (section 3)
///     label2:
///       (section 4)
///     } catch {
///       (section 5)
///     }
///     (section 6)
#[derive(Clone)]
pub(crate) struct Sequence {
    pub kind: SequenceKind,
    pub bid: BlockId,
    pub tcid: TryCatchId,
    pub input_stack_size: Option<usize>,
    pub loc_id: LocId,
    pub next: Option<Addr>,
    pub range: Range<Addr>,
}

impl Sequence {
    pub(crate) fn compute(
        builder: &mut FuncBuilder,
        filename: ir::Filename,
        body_instrs: &[Instruct],
    ) -> (LabelMap<Addr>, BlockIdMap<Addr>, AddrMap<Sequence>) {
        let seq_builder = SeqBuilder {
            sequences: Default::default(),
            builder,
            filename,
            seq_addr: Addr::ENTRY,
            next_exid: ExFrameId::from_usize(1),
            tc_stack: Default::default(),
        };
        seq_builder.compute(body_instrs)
    }
}

#[derive(Copy, Clone, Debug, Eq, PartialEq)]
pub(crate) enum SequenceKind {
    Normal,
    Catch,
}

struct SeqBuilder<'b> {
    sequences: Vec<(Addr, Sequence)>,
    builder: &'b mut FuncBuilder,
    filename: ir::Filename,
    seq_addr: Addr,
    next_exid: ExFrameId,
    tc_stack: Vec<TryCatchId>,
}

impl SeqBuilder<'_> {
    fn add(
        &mut self,
        kind: SequenceKind,
        range: Range<Addr>,
        src_loc: &hhbc::SrcLoc,
        next: Option<Addr>,
    ) -> BlockId {
        let bid = self.builder.alloc_bid();
        let loc_id = crate::context::add_loc(self.builder, self.filename, src_loc);
        let tcid = self.tc_stack.last().copied().unwrap_or_default();
        self.builder.func.block_mut(bid).tcid = tcid;
        let sequence = Sequence {
            kind,
            bid,
            tcid,
            input_stack_size: None,
            loc_id,
            next,
            range,
        };
        self.sequences.push((self.seq_addr, sequence));
        bid
    }

    fn alloc_exid(&mut self) -> ExFrameId {
        let cur = self.next_exid;
        self.next_exid = ir::ExFrameId::from_usize(cur.as_usize() + 1);
        cur
    }

    fn compute(
        mut self,
        body_instrs: &[Instruct],
    ) -> (LabelMap<Addr>, BlockIdMap<Addr>, AddrMap<Sequence>) {
        let mut prev_addr: Addr = Addr::ENTRY;
        let mut label_to_addr: LabelMap<Addr> = Default::default();
        let mut bid_to_addr: BlockIdMap<Addr> = Default::default();
        let mut prev_kind = SequenceKind::Normal;
        let mut cur_src_loc: Cow<'_, hhbc::SrcLoc> = Cow::Owned(hhbc::SrcLoc::default());
        let mut prev_src_loc = cur_src_loc.clone();

        use hhbc::Pseudo;

        for (idx, instr) in body_instrs.iter().enumerate() {
            let idx = Addr::from_usize(idx);
            match *instr {
                Instruct::Pseudo(Pseudo::Label(label)) if idx == Addr::ENTRY => {
                    // Special case - if the first Instr is a Label then we need
                    // to include it in the first block.
                    prev_addr = idx.incr();
                    label_to_addr.insert(label, idx);
                }
                Instruct::Pseudo(
                    Pseudo::Label(_)
                    | Pseudo::TryCatchBegin
                    | Pseudo::TryCatchMiddle
                    | Pseudo::TryCatchEnd,
                ) => {
                    let bid = self.add(prev_kind, prev_addr..idx, &prev_src_loc, Some(idx));
                    bid_to_addr.insert(bid, self.seq_addr);

                    if prev_kind == SequenceKind::Catch {
                        // Now that we've created the catch target update the
                        // ex_frame for it.
                        let exid = self.tc_stack.last().unwrap().exid();
                        let parent = if self.tc_stack.len() > 1 {
                            *self.tc_stack.get(self.tc_stack.len() - 2).unwrap()
                        } else {
                            TryCatchId::None
                        };
                        self.builder.func.ex_frames.insert(
                            exid,
                            ir::func::ExFrame {
                                parent,
                                catch_bid: bid,
                            },
                        );
                    }

                    self.seq_addr = idx;
                    prev_addr = idx.incr();
                    prev_kind = SequenceKind::Normal;
                    prev_src_loc = cur_src_loc.clone();

                    match *instr {
                        Instruct::Pseudo(Pseudo::Label(label)) => {
                            label_to_addr.insert(label, idx);
                        }
                        Instruct::Pseudo(Pseudo::TryCatchBegin) => {
                            let exid = self.alloc_exid();
                            self.tc_stack.push(TryCatchId::Try(exid));
                        }
                        Instruct::Pseudo(Pseudo::TryCatchMiddle) => {
                            let exid = self.tc_stack.pop().unwrap().exid();
                            self.tc_stack.push(TryCatchId::Catch(exid));
                            prev_kind = SequenceKind::Catch;
                        }
                        Instruct::Pseudo(Pseudo::TryCatchEnd) => {
                            self.tc_stack.pop();
                        }
                        _ => unreachable!(),
                    }
                }
                Instruct::Pseudo(Pseudo::SrcLoc(ref src_loc)) => {
                    cur_src_loc = Cow::Borrowed(src_loc);
                }
                _ => {}
            }
        }

        let range = prev_addr..Addr::from_usize(body_instrs.len());
        self.add(SequenceKind::Normal, range, &prev_src_loc, None);

        let sequences: AddrMap<Sequence> = self.sequences.into_iter().collect();
        (label_to_addr, bid_to_addr, sequences)
    }
}
