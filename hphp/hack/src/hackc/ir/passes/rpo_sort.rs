// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use analysis::rpo::compute_rpo;
use ir_core::BlockId;
use ir_core::Func;
use newtype::HasNone;
use newtype::IdVec;

/// Sort func's blocks using reverse postorder DFS, renumbering successor edges as needed.
///
/// RPO has the nice property that an Instr's operands will always be "earlier" in the ordering.
///
/// Note that blocks unreachable from the entry block will be discarded.
pub fn rpo_sort(func: &mut Func) {
    let rpo = compute_rpo(func);

    if block_order_matches(func, &rpo) {
        // Already in RPO, nothing to do.
        return;
    }

    // Create the mapping from old BlockId to new BlockId.
    // Dead blocks will have a BlockId::NONE entry.
    let remap = order_to_remap(&rpo, func.blocks.len());

    // Update successor edges for surviving blocks to use the new BlockIds.
    // (Note that this double loop will make it so that the closer to RPO we
    // start the less we have to do.)
    for (old, &new) in remap.iter().enumerate() {
        if new != BlockId::NONE {
            let bid = BlockId(old as u32);
            for edge in func.edges_mut(bid) {
                let target = remap[*edge];
                debug_assert_ne!(target, BlockId::NONE);
                *edge = target;
            }
        }
    }

    for param in &mut func.params {
        if let Some(dv) = param.default_value.as_mut() {
            if let Some(target) = remap.get(dv.init) {
                dv.init = *target;
            }
        }
    }

    // Update catch_bids for TryCatch frames.
    for ex_frame in &mut func.ex_frames.values_mut() {
        let bid = &mut ex_frame.catch_bid;
        if *bid != BlockId::NONE {
            let target = remap[*bid];
            // TODO: If target is BlockId::NONE then this ex_frame is no longer
            // needed and should be reaped.
            // debug_assert_ne!(target, BlockId::NONE);
            *bid = target;
        }
    }

    // Drop dead blocks and put the others in the RPO order specified by remap.
    permute_and_truncate(&mut func.blocks, remap);

    assert_eq!(func.blocks.len(), rpo.len());
}

/// Invert the block order in order to map each old BlockId to its new BlockId.
/// Dead blocks (those not listed in order) map to BlockId::NONE.
fn order_to_remap(order: &[BlockId], len: usize) -> IdVec<BlockId, BlockId> {
    let mut remap = IdVec::new_from_vec(vec![BlockId::NONE; len]);
    for (i, &bid) in order.iter().enumerate() {
        // Check for duplicates in 'order'.
        debug_assert_eq!(remap[bid], BlockId::NONE);
        remap[bid] = BlockId(i as u32);
    }
    remap
}

/// Replace 'value' with a Vec where each entry is moved to the slot specified in remap.
///
/// Entries that map to Id::NONE are discarded, in which case the Vec will shrink.
///
/// Entries other than Id::NONE must be unique, so if there are no Id::NONE then this
/// computes a permutation.
pub fn permute_and_truncate<Id, T>(values: &mut IdVec<Id, T>, mut remap: IdVec<Id, Id>)
where
    Id: HasNone + Into<usize> + From<u32> + Ord + PartialEq + Eq + Copy,
{
    // This algorithm walks left-to-right in the values array, only moving on when
    // the current slot has the correct value in it.
    //
    // It works by looking at what's in the slot, and if it's not what is supposed to
    // be there it swaps it into where it's supposed to be and tries again with the
    // value it just swapped in from the later slot. So at each step we correct at least
    // one value, but perhaps not the one at the current slot.
    //
    // When hit a value we need to discard, rather than swapping we pop the last entry,
    // drop that into the current slot, and retry. This way the array shrinks.
    let mut slot = Id::from(0);
    while slot < Id::from(values.len() as u32) {
        // Where we want values[slot] to go.
        let mut desired_slot = remap[slot];

        while slot != desired_slot {
            if desired_slot != Id::NONE {
                // Swap the current slot to where it belongs, thus making progress, and retry.
                values.swap(slot, desired_slot);
                std::mem::swap(&mut remap[desired_slot], &mut desired_slot);
            } else {
                // Discard this value.
                values.swap_remove(slot.into());
                if slot.into() == values.len() {
                    return;
                }
                desired_slot = remap[Id::from(values.len() as u32)];
            }
        }

        slot = Id::from((slot.into() + 1) as u32);
    }
}

/// Are the Blocks in func already in the order specified by rpo?
fn block_order_matches(func: &Func, order: &[BlockId]) -> bool {
    // See if the ordering is the no-op ordering [0, 1, 2, 3, blocks.len()).
    order.len() == func.blocks.len()
        && order.iter().enumerate().all(|(i, &b)| {
            let bid_as_usize: usize = b.into();
            i == bid_as_usize
        })
}

#[cfg(test)]
mod tests {
    use hash::HashMap;
    use hash::HashSet;
    use ir_core::Block;
    use ir_core::BlockId;
    use rand::seq::SliceRandom;
    use rand::thread_rng;
    use rand::Rng;

    use super::*;

    fn make_dummy_blocks(names: &[&str]) -> IdVec<BlockId, Block> {
        IdVec::new_from_vec(
            names
                .iter()
                .map(|name| Block::default().with_pname(*name))
                .collect(),
        )
    }

    #[test]
    fn permute() {
        let mut blocks = make_dummy_blocks(&["s", "a", "c", "r", "e", "b", "l", "m"]);

        let order = IdVec::new_from_vec(vec![
            BlockId(0),
            BlockId(3),
            BlockId(1),
            BlockId(2),
            BlockId(7),
            BlockId(5),
            BlockId(6),
            BlockId(4),
        ]);
        permute_and_truncate(&mut blocks, order);

        let q: Vec<_> = blocks
            .iter()
            .map(|b| b.pname_hint.as_ref().map_or("", String::as_str))
            .collect();
        assert_eq!(q.join(""), "scramble");
    }

    #[test]
    fn permute_and_drop() {
        let mut blocks = make_dummy_blocks(&["s", "a", "c", "r", "e", "b", "l", "m"]);

        let order = IdVec::new_from_vec(vec![
            BlockId::NONE,
            BlockId(2),
            BlockId(0),
            BlockId(1),
            BlockId::NONE,
            BlockId::NONE,
            BlockId::NONE,
            BlockId(3),
        ]);
        permute_and_truncate(&mut blocks, order);

        let q: Vec<_> = blocks
            .iter()
            .map(|b| b.pname_hint.as_ref().map_or("", String::as_str))
            .collect();
        assert_eq!(q.join(""), "cram");
    }

    #[test]
    fn permute_random() {
        let names = ["a", "b", "c", "d", "e", "f"];
        let blocks = make_dummy_blocks(&names);
        let mut rng = thread_rng();

        for _ in 0..100 {
            // Create a random selection, including usually dropping entries.
            let mut order = (0..names.len())
                .map(|n| BlockId(n as u32))
                .collect::<Vec<_>>();
            order.shuffle(&mut rng);
            order.truncate(rng.gen_range(0..order.len()));

            let mut b1 = blocks.clone();
            permute_and_truncate(&mut b1, order_to_remap(&order, blocks.len()));

            let result: Vec<_> = b1
                .iter()
                .map(|b| b.pname_hint.as_ref().map_or("", String::as_str))
                .collect();
            assert_eq!(result.len(), order.len());

            for (&r, o) in result.iter().zip(order) {
                let i: usize = o.into();
                assert_eq!(r, names[i]);
            }
        }
    }

    // Helper function for postorder DFS.
    fn postorder_aux<'a>(
        cfg: &HashMap<&'a str, Vec<&'a str>>,
        block: &'a str,
        result: &mut Vec<String>,
        seen: &mut HashSet<&'a str>,
    ) {
        if !seen.contains(block) {
            seen.insert(block);
            for s in cfg[block].iter().rev() {
                postorder_aux(cfg, s, result, seen)
            }
            result.push(format!("{}({})", block, cfg[block].join(",")));
        }
    }

    // Recursive reference implementation.
    fn rpo_reference(testcase: &[testutils::Block]) -> String {
        let mut cfg: HashMap<&str, Vec<&str>> = HashMap::default();
        for block in testcase {
            cfg.insert(
                &block.name,
                block.successors().map(String::as_str).collect(),
            );
        }

        let mut result = Vec::with_capacity(cfg.len());
        let mut seen: HashSet<&str> =
            HashSet::with_capacity_and_hasher(cfg.len(), Default::default());
        postorder_aux(&cfg, &testcase[0].name, &mut result, &mut seen);
        result.reverse();

        result.join(",")
    }

    #[test]
    fn rpo() {
        // Define a little CFG with a some branches and loops.
        let blocks: Vec<testutils::Block> = vec![
            testutils::Block::jmp("a", "b"),
            testutils::Block::jmp_op("b", ["c", "d"]),
            testutils::Block::jmp("c", "e"),
            testutils::Block::jmp_op("d", ["e", "f"]),
            testutils::Block::jmp_op("e", ["f", "f"]),
            testutils::Block::jmp_op("f", ["b", "g"]),
            testutils::Block::ret("g"),
            testutils::Block::jmp_op("dead", ["b", "dead2"]),
            testutils::Block::jmp("dead2", "dead"),
        ];

        let mut rng = thread_rng();

        for _ in 0..50 {
            // Shuffle block order, but leave the entry block first. We should always
            // end up with the same RPO, since that depends only on the CFG.
            let mut c = blocks.clone();
            c[1..].shuffle(&mut rng);

            let (mut func, _) = testutils::build_test_func(&c);
            rpo_sort(&mut func);

            let order = func
                .blocks
                .iter()
                .map(|b| b.pname_hint.as_ref().map_or("", String::as_str))
                .collect::<Vec<_>>()
                .join(",");

            assert_eq!(order, "a,b,c,d,e,f,g");
        }
    }

    #[test]
    fn random_rpo() {
        let names = ["a", "b", "c", "d", "e", "f", "g", "h", "i", "j"];
        let mut rng = thread_rng();

        for _ in 0..500 {
            // Build up a random function.
            let num_blocks = rng.gen_range(2..names.len());
            let mut blocks = Vec::new();
            let mut rblocks = Vec::new();
            for i in 0..num_blocks {
                // Choose 0, 1 or 2 successor, with only a small chance of 0.
                let num_successors = (rng.gen_range(0..7) + 2) / 3;
                let mut succ = || {
                    // NOTE: You can't branch to the entry block.
                    names[rng.gen_range(1..num_blocks)]
                };
                let (term, rterm) = match num_successors {
                    0 => (testutils::Terminator::Ret, testutils::Terminator::Ret),
                    1 => {
                        let a = succ();
                        (
                            testutils::Terminator::Jmp(a.to_string()),
                            testutils::Terminator::Jmp(a.to_string()),
                        )
                    }
                    2 => {
                        let a = succ();
                        let b = succ();
                        (
                            testutils::Terminator::JmpOp(a.to_string(), b.to_string()),
                            testutils::Terminator::JmpOp(b.to_string(), a.to_string()),
                        )
                    }
                    _ => unreachable!(),
                };
                blocks.push(testutils::Block::ret(names[i]).with_terminator(term));
                rblocks.push(testutils::Block::ret(names[i]).with_terminator(rterm));
            }

            let (mut func, _) = testutils::build_test_func(&blocks);
            rpo_sort(&mut func);

            let (mut rfunc, _) = testutils::build_test_func(&rblocks);
            rpo_sort(&mut rfunc);

            fn bname<'a>(func: &'a Func, bid: BlockId) -> &'a str {
                func.blocks[bid]
                    .pname_hint
                    .as_ref()
                    .map_or("", String::as_str)
            }
            let actual = func
                .blocks
                .iter()
                .enumerate()
                .map(|(i, b)| {
                    let succ = func
                        .edges(BlockId(i as u32))
                        .iter()
                        .map(|e| bname(&func, *e))
                        .collect::<Vec<_>>()
                        .join(",");
                    format!(
                        "{}({})",
                        b.pname_hint.as_ref().map_or("", String::as_str),
                        succ
                    )
                })
                .collect::<Vec<_>>()
                .join(",");
            let ractual = analysis::compute_rrpo(&rfunc)
                .iter()
                .map(|b| {
                    let succ = rfunc
                        .edges(*b)
                        .iter()
                        .rev()
                        .map(|e| bname(&rfunc, *e))
                        .collect::<Vec<_>>()
                        .join(",");
                    format!("{}({})", bname(&rfunc, *b), succ)
                })
                .collect::<Vec<_>>()
                .join(",");

            // Compare its RPO to a simple reference implementation's.
            let expected = rpo_reference(&blocks);

            assert_eq!(actual, expected);
            assert_eq!(ractual, expected);
        }
    }
}
