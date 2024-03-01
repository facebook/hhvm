// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use analysis::PredecessorCatchMode;
use analysis::PredecessorFlags;
use ir_core::instr::HasEdges;
use ir_core::instr::Terminator;
use ir_core::BlockId;
use ir_core::BlockIdMap;
use ir_core::Func;
use ir_core::InstrId;
use ir_core::ValueId;
use newtype::IdVec;

/// Attempt to merge simple blocks together. Returns true if the Func was
/// changed.
pub fn run(func: &mut Func) -> bool {
    let mut changed = false;

    let mut predecessors = analysis::compute_num_predecessors(
        func,
        PredecessorFlags {
            mark_entry_blocks: false,
            catch: PredecessorCatchMode::Throw,
        },
    );

    for bid in func.block_ids().rev() {
        let mut instr = std::mem::replace(func.terminator_mut(bid), Terminator::Unreachable);

        let edges = instr.edges_mut();
        let successors = edges.len() as u32;
        for edge in edges {
            if let Some(target) = forward_edge(func, *edge, successors, &predecessors) {
                changed = true;
                predecessors[*edge] -= 1;
                predecessors[target] += 1;
                *edge = target;
            }
        }

        *func.terminator_mut(bid) = instr;
    }

    // Function params
    for idx in 0..func.params.len() {
        if let Some(dv) = &func.params[idx].default_value {
            if let Some(target) = forward_edge(func, dv.init, 1, &predecessors) {
                changed = true;
                predecessors[dv.init] -= 1;
                predecessors[target] += 1;
                func.params[idx].default_value.as_mut().unwrap().init = target;
            }
        }
    }

    // Entry Block
    if let Some(target) = forward_edge(func, Func::ENTRY_BID, 1, &predecessors) {
        // Uh oh - we're remapping the ENTRY_BID - this is no good. Instead
        // remap the target and swap the two blocks.
        changed = true;
        let mut remap = BlockIdMap::default();
        remap.insert(target, Func::ENTRY_BID);
        func.remap_bids(&remap);
        func.blocks.swap(Func::ENTRY_BID, target);
    }

    if changed {
        crate::rpo_sort(func);
        true
    } else {
        false
    }
}

fn params_eq(a: &[InstrId], b: &[ValueId]) -> bool {
    a.len() == b.len()
        && a.iter()
            .zip(b.iter())
            .all(|(a, b)| ValueId::from_instr(*a) == *b)
}

fn forward_edge(
    func: &Func,
    mut bid: BlockId,
    mut predecessor_successors: u32,
    predecessors: &IdVec<BlockId, u32>,
) -> Option<BlockId> {
    // If the target block (bid) just contains a jump to another block then jump
    // directly to that one instead - but we need to worry about creating a
    // critical edge. If our predecessor has multiple successors and our target
    // has multiple predecessors then we can't combine them because that would
    // be a critical edge.

    // TODO: Optimize this. Nothing will break if this is too small or too
    // big. Too small means we'll snap through fewer chains. To big means we'll
    // take longer than necessary to bail on an infinite loop (which shouldn't
    // really happen normally).
    const LOOP_CHECK: usize = 100;

    let mut changed = false;
    for _ in 0..LOOP_CHECK {
        let block = func.block(bid);
        if block.iids.len() != 1 {
            // This block has multiple instrs.
            break;
        }

        let terminator = func.terminator(bid);
        let target = match *terminator {
            Terminator::Jmp(target, _) => {
                if !block.params.is_empty() {
                    // The block takes params but the jump doesn't pass them
                    // along - it's not a simple forward.
                    break;
                }

                target
            }
            Terminator::JmpArgs(target, ref args, _) => {
                if !params_eq(&block.params, args) {
                    // Our jump is passing a different set of args from what our
                    // block takes in.
                    break;
                }

                target
            }
            _ => {
                break;
            }
        };

        let target_block = func.block(target);
        if target_block.tcid != block.tcid {
            // This jump crosses an exception frame - can't forward.
            break;
        }

        // Critical edges: If the predecessor has multiple successors and the
        // target has multiple predecessors then we can't continue any further
        // because it would create a critical edge.
        if predecessor_successors > 1 && predecessors[target] > 1 {
            break;
        }

        bid = target;
        changed = true;
        // We know that the next time through our predecessor must have had only
        // a single successor (since we checked for Jmp and JmpArgs above).
        predecessor_successors = 1;
    }

    changed.then_some(bid)
}

#[cfg(test)]
mod test {
    use std::sync::Arc;

    use ir_core::func::DefaultValue;
    use ir_core::BlockId;
    use ir_core::Instr;
    use ir_core::InstrId;
    use ir_core::Param;
    use ir_core::StringInterner;
    use ir_core::TypeInfo;

    fn mk_param(name: &str, dv: BlockId, strings: &StringInterner) -> Param {
        Param {
            name: strings.intern_str(name),
            is_variadic: false,
            is_inout: false,
            is_readonly: false,
            user_attributes: vec![],
            ty: TypeInfo::default(),
            default_value: Some(DefaultValue {
                init: dv,
                expr: b"1".to_vec(),
            }),
        }
    }

    #[test]
    fn test1() {
        // Can't forward because 'b' isn't empty.
        let (mut func, strings) = testutils::build_test_func(&[
            testutils::Block::jmp("a", "b").with_target(),
            testutils::Block::jmp("b", "c").with_target(),
            testutils::Block::ret("c"),
        ]);
        let expected = func.clone();

        let changed = super::run(&mut func);
        assert!(!changed);

        testutils::assert_func_struct_eq(&func, &expected, &strings);
    }

    #[test]
    fn test2() {
        // 'a' forwards directly to 'c'
        let (mut func, strings) = testutils::build_test_func(&[
            testutils::Block::jmp("a", "b").with_target(),
            testutils::Block::jmp("b", "c"),
            testutils::Block::ret("c").with_target(),
        ]);

        let changed = super::run(&mut func);
        assert!(changed);

        let expected = testutils::build_test_func_with_strings(
            &[
                testutils::Block::jmp("a", "c").with_target(),
                testutils::Block::ret("c").with_target(),
            ],
            Arc::clone(&strings),
        );
        testutils::assert_func_struct_eq(&func, &expected, &strings);
    }

    #[test]
    fn test3() {
        // Can't forward because it would create a critical section.
        let (mut func, strings) = testutils::build_test_func(&[
            testutils::Block::jmp_op("a", ["b", "c"]).with_target(),
            testutils::Block::jmp("b", "d"),
            testutils::Block::jmp("c", "d"),
            testutils::Block::ret("d").with_target(),
        ]);
        let expected = func.clone();

        let changed = super::run(&mut func);
        assert!(!changed);

        testutils::assert_func_struct_eq(&func, &expected, &strings);
    }

    #[test]
    fn test4() {
        // Expect 'c' to be forwarded directly to 'e'.
        let (mut func, strings) = testutils::build_test_func(&[
            testutils::Block::jmp_op("a", ["b", "c"]).with_target(),
            testutils::Block::jmp("b", "e"),
            testutils::Block::jmp("c", "d"),
            testutils::Block::jmp("d", "e"),
            testutils::Block::ret("e").with_target(),
        ]);

        let changed = super::run(&mut func);
        assert!(changed);

        let expected = testutils::build_test_func_with_strings(
            &[
                testutils::Block::jmp_op("a", ["b", "c"]).with_target(),
                testutils::Block::jmp("b", "e"),
                testutils::Block::jmp("c", "e"),
                testutils::Block::ret("e").with_target(),
            ],
            Arc::clone(&strings),
        );
        testutils::assert_func_struct_eq(&func, &expected, &strings);
    }

    #[test]
    fn test5() {
        // Expect 'entry' to be removed.
        let (mut func, strings) = testutils::build_test_func(&[
            testutils::Block::jmp("entry", "b"),
            testutils::Block::jmp("b", "c").with_target(),
            testutils::Block::ret("c"),
        ]);

        let changed = super::run(&mut func);
        assert!(changed);

        let expected = testutils::build_test_func_with_strings(
            &[
                testutils::Block::jmp("b", "c").with_target(),
                testutils::Block::ret("c"),
            ],
            Arc::clone(&strings),
        );
        testutils::assert_func_struct_eq(&func, &expected, &strings);
    }

    #[test]
    fn test6() {
        // We can forward c -> e but still need b -> d -> e because of critedge.
        let (mut func, strings) = testutils::build_test_func(&[
            testutils::Block::jmp_op("a", ["b", "c"]),
            testutils::Block::jmp_op("b", ["d", "e"]),
            testutils::Block::jmp("c", "d"),
            testutils::Block::jmp("d", "e"),
            testutils::Block::ret("e"),
        ]);

        let changed = super::run(&mut func);
        assert!(changed);

        let expected = testutils::build_test_func_with_strings(
            &[
                testutils::Block::jmp_op("a", ["b", "c"]),
                testutils::Block::jmp_op("b", ["d", "e"]),
                testutils::Block::jmp("c", "e"),
                testutils::Block::jmp("d", "e"),
                testutils::Block::ret("e"),
            ],
            Arc::clone(&strings),
        );
        testutils::assert_func_struct_eq(&func, &expected, &strings);
    }

    #[test]
    fn test7() {
        // We expect to skip 'b' and 'c'
        let (mut func, strings) = testutils::build_test_func(&[
            testutils::Block::jmp_op("a", ["b", "c"]),
            testutils::Block::jmp("b", "d"),
            testutils::Block::jmp("c", "e"),
            testutils::Block::ret("d"),
            testutils::Block::ret("e"),
        ]);

        let changed = super::run(&mut func);
        assert!(changed);

        let expected = testutils::build_test_func_with_strings(
            &[
                testutils::Block::jmp_op("a", ["d", "e"]),
                testutils::Block::ret("d"),
                testutils::Block::ret("e"),
            ],
            Arc::clone(&strings),
        );
        testutils::assert_func_struct_eq(&func, &expected, &strings);
    }

    #[test]
    fn test8() {
        // We expect to skip the entry block.
        let (mut func, strings) = testutils::build_test_func(&[
            testutils::Block::jmp("a", "c"),
            testutils::Block::jmp("b", "c"),
            testutils::Block::jmp_op("c", ["d", "e"]),
            testutils::Block::ret("d"),
            testutils::Block::ret("e"),
        ]);
        func.params.push(mk_param("x", BlockId(1), &strings));
        *func.instr_mut(InstrId(1)) = Instr::enter(BlockId(2), ir_core::LocId::NONE);

        eprintln!("FUNC:\n{}", print::DisplayFunc::new(&func, true, &strings));

        let changed = super::run(&mut func);
        assert!(changed);

        let mut expected = testutils::build_test_func_with_strings(
            &[
                testutils::Block::jmp_op("c", ["d", "e"]),
                testutils::Block::jmp("b", "c"),
                testutils::Block::ret("d"),
                testutils::Block::ret("e"),
            ],
            Arc::clone(&strings),
        );
        expected.params.push(mk_param("x", BlockId(1), &strings));
        *expected.instr_mut(InstrId(1)) = Instr::enter(BlockId(0), ir_core::LocId::NONE);

        testutils::assert_func_struct_eq(&func, &expected, &strings);
    }
}
