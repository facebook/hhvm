// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use ir::BlockId;
use ir::TryCatchId;

/// Collect the Func's Blocks into the tree represented by the exception frames.
pub(crate) fn collect_tc_sections(func: &ir::Func) -> Vec<BlockIdOrExFrame> {
    let mut root: ExFrame = ExFrame::default();

    for bid in func.block_ids() {
        let tcid = func.block(bid).tcid;
        let frame = get_frame(&mut root, func, tcid);
        let bid = BlockIdOrExFrame::Block(bid);
        frame.push(bid);
    }

    root.sort(func);
    assert!(root.catch_bids.is_empty());
    root.try_bids
}

/// A node or leaf of the exception frame Block tree.
#[derive(Debug, PartialEq, Eq)]
pub(crate) enum BlockIdOrExFrame {
    Frame(ExFrame),
    Block(BlockId),
}

impl BlockIdOrExFrame {
    pub(crate) fn bid(&self) -> BlockId {
        match *self {
            Self::Block(bid) => bid,
            Self::Frame(ref frame) => frame.bid(),
        }
    }

    fn frame_mut(&mut self) -> &mut ExFrame {
        match self {
            Self::Block(_) => panic!("Not Frame"),
            Self::Frame(frame) => frame,
        }
    }
}

#[derive(Default, Debug, Eq, PartialEq)]
pub(crate) struct ExFrame {
    exid: ir::ExFrameId,
    pub try_bids: Vec<BlockIdOrExFrame>,
    pub catch_bids: Vec<BlockIdOrExFrame>,
}

impl ExFrame {
    fn bid(&self) -> BlockId {
        self.try_bids
            .first()
            .or_else(|| self.catch_bids.first())
            .map_or(BlockId::NONE, |bof| bof.bid())
    }

    fn sort(&mut self, func: &ir::Func) {
        self.try_bids.sort_by(|a, b| Self::sort_fn(func, a, b));
        self.catch_bids.sort_by(|a, b| Self::sort_fn(func, a, b));
    }

    fn sort_fn(_func: &ir::Func, a: &BlockIdOrExFrame, b: &BlockIdOrExFrame) -> std::cmp::Ordering {
        let a_bid = a.bid();
        let b_bid = b.bid();
        if a_bid == ir::Func::ENTRY_BID || b_bid == ir::Func::ENTRY_BID {
            return a_bid.cmp(&b_bid);
        }
        a_bid.cmp(&b_bid)
    }
}

fn find_frame(parent_bids: &[BlockIdOrExFrame], exid: ir::ExFrameId) -> Option<usize> {
    parent_bids.iter().rposition(|item| {
        match item {
            BlockIdOrExFrame::Block(_) => {}
            BlockIdOrExFrame::Frame(frame) => {
                if frame.exid == exid {
                    return true;
                }
            }
        }
        false
    })
}

fn insert_frame<'c>(
    parent_bids: &'c mut Vec<BlockIdOrExFrame>,
    exid: ir::ExFrameId,
) -> &'c mut ExFrame {
    parent_bids.push(BlockIdOrExFrame::Frame(ExFrame {
        exid,
        ..Default::default()
    }));
    parent_bids.last_mut().unwrap().frame_mut()
}

fn find_or_insert_frame<'c>(
    root: &'c mut ExFrame,
    func: &ir::Func,
    exid: ir::ExFrameId,
) -> &'c mut ExFrame {
    let parent_tcid: TryCatchId = func.ex_frames[&exid].parent;
    let parent_frame = get_frame(root, func, parent_tcid);
    if let Some(frame) = find_frame(parent_frame, exid) {
        parent_frame[frame].frame_mut()
    } else {
        insert_frame(parent_frame, exid)
    }
}

fn get_frame<'c>(
    root: &'c mut ExFrame,
    func: &ir::Func,
    tcid: TryCatchId,
) -> &'c mut Vec<BlockIdOrExFrame> {
    match tcid {
        TryCatchId::None => &mut root.try_bids,
        TryCatchId::Try(exid) => {
            let frame = find_or_insert_frame(root, func, exid);
            &mut frame.try_bids
        }
        TryCatchId::Catch(exid) => {
            let frame = find_or_insert_frame(root, func, exid);
            &mut frame.catch_bids
        }
    }
}

#[cfg(test)]
mod test {
    use ir::Block;
    use ir::BlockId;
    use ir::TryCatchId;

    use super::*;

    fn make_test_func(
        blocks: Vec<(/* bid */ usize, /* tcid */ Option<&'static str>)>,
        ex_frames: Vec<(
            /* exid */ usize,
            (
                /* parent */ Option<&'static str>,
                /* catch_bid */ usize,
            ),
        )>,
    ) -> ir::Func {
        let mut func = ir::Func::default();

        fn conv_tcid(tcid: &str) -> TryCatchId {
            let exid = ir::ExFrameId::from_usize(tcid[1..].parse().unwrap());
            if tcid.starts_with('T') {
                TryCatchId::Try(exid)
            } else {
                TryCatchId::Catch(exid)
            }
        }

        for (bid, tcid) in blocks {
            let tcid = tcid.map_or(TryCatchId::None, conv_tcid);
            let block = Block {
                tcid,
                ..Block::default()
            };
            assert!(bid == func.blocks.len());
            func.blocks.push(block);
        }

        for (exid, (parent, catch_bid)) in ex_frames {
            let exid = ir::ExFrameId::from_usize(exid);
            let parent = parent.map_or(TryCatchId::None, conv_tcid);
            let catch_bid = BlockId::from_usize(catch_bid);
            func.ex_frames
                .insert(exid, ir::func::ExFrame { parent, catch_bid });
        }

        func
    }

    #[test]
    fn test_collect_tc_section() {
        let body = make_test_func(
            vec![
                (0, None),
                (1, None),
                (2, None),
                (3, None),
                (4, None),
                (5, None),
                (6, None),
                (7, Some("T3")),
                (8, Some("C3")),
                (9, Some("T2")),
                (10, Some("T2")),
                (11, Some("T2")),
                (12, Some("T2")),
                (13, Some("C2")),
                (14, None),
                (15, None),
                (16, Some("T1")),
                (17, Some("T1")),
                (18, Some("T1")),
                (19, Some("T1")),
                (20, Some("C1")),
                (21, None),
                (22, None),
                (23, None),
                (24, None),
                (25, None),
            ],
            vec![
                // ExId, (parent, catch_bid)
                (1, (None, 20)),
                (2, (None, 13)),
                (3, (Some("T2"), 8)),
            ],
        );

        let root = collect_tc_sections(&body);

        let expected = vec![
            BlockIdOrExFrame::Block(0.into()),
            BlockIdOrExFrame::Block(1.into()),
            BlockIdOrExFrame::Block(2.into()),
            BlockIdOrExFrame::Block(3.into()),
            BlockIdOrExFrame::Block(4.into()),
            BlockIdOrExFrame::Block(5.into()),
            BlockIdOrExFrame::Block(6.into()),
            BlockIdOrExFrame::Frame(ExFrame {
                exid: ir::ExFrameId(2),
                try_bids: vec![
                    BlockIdOrExFrame::Frame(ExFrame {
                        exid: ir::ExFrameId(3),
                        try_bids: vec![BlockIdOrExFrame::Block(7.into())],
                        catch_bids: vec![BlockIdOrExFrame::Block(8.into())],
                    }),
                    BlockIdOrExFrame::Block(9.into()),
                    BlockIdOrExFrame::Block(10.into()),
                    BlockIdOrExFrame::Block(11.into()),
                    BlockIdOrExFrame::Block(12.into()),
                ],
                catch_bids: vec![BlockIdOrExFrame::Block(13.into())],
            }),
            BlockIdOrExFrame::Block(14.into()),
            BlockIdOrExFrame::Block(15.into()),
            BlockIdOrExFrame::Frame(ExFrame {
                exid: ir::ExFrameId(1),
                try_bids: vec![
                    BlockIdOrExFrame::Block(16.into()),
                    BlockIdOrExFrame::Block(17.into()),
                    BlockIdOrExFrame::Block(18.into()),
                    BlockIdOrExFrame::Block(19.into()),
                ],
                catch_bids: vec![BlockIdOrExFrame::Block(20.into())],
            }),
            BlockIdOrExFrame::Block(21.into()),
            BlockIdOrExFrame::Block(22.into()),
            BlockIdOrExFrame::Block(23.into()),
            BlockIdOrExFrame::Block(24.into()),
            BlockIdOrExFrame::Block(25.into()),
        ];

        assert_eq!(root, expected);
    }
}
