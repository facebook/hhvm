use analysis::PredecessorCatchMode;
use analysis::PredecessorFlags;
use ir_core::instr::HasEdges;
use ir_core::instr::HasLoc;
use ir_core::Block;
use ir_core::BlockId;
use ir_core::Func;
use ir_core::Instr;
use ir_core::LocId;

/// Find and split critical edges. Assumes there are no unreachable blocks,
/// which is the case after rpo_sort().
///
/// A critical edge is one where the source has multiple successors and the
/// destination has multiple predecessors. By splitting them we ensure that we
/// can always add instructions to one side of an edge which only affects that
/// edge.
///
/// 1. find critical edges, build a simple work list. Create new BlockIds now
///    and update the edges to the new target BlockIds.
/// 2. Create the new empty blocks, then fill them in from the worklist. Order
///    doesn't matter because we recorded all BlockIds in step 1.
/// 3. re-run rpo_sort().
pub fn split_critical_edges(func: &mut Func, rpo_sort: bool) {
    let pred_counts = analysis::compute_num_predecessors(
        func,
        PredecessorFlags {
            mark_entry_blocks: false,
            catch: PredecessorCatchMode::Ignore,
        },
    );
    let mut work = Vec::new();
    let num_blocks = func.blocks.len();
    for bid in func.block_ids() {
        let term = func.terminator_mut(bid);
        let loc = term.loc_id();
        let edges = term.edges_mut();
        if edges.len() > 1 {
            for edge in edges {
                if pred_counts[*edge] > 1 {
                    // cannot create new blocks and instructions while iterating
                    // here, so make a simple worklist
                    let split_bid = BlockId((num_blocks + work.len()) as u32);
                    work.push(WorkItem {
                        loc,
                        split_bid,
                        old_target: edge.clone(),
                    });
                    *edge = split_bid;
                }
            }
        }
    }
    if !work.is_empty() {
        func.blocks
            .resize(num_blocks + work.len(), Block::default());
        for item in work.drain(..) {
            let _params = func.block(item.old_target).params.len();
            let instr = Instr::jmp(item.old_target, item.loc);
            func.emit(item.split_bid, instr);
        }
        if rpo_sort {
            crate::rpo_sort(func)
        }
    }
}

struct WorkItem {
    loc: LocId,
    split_bid: BlockId,
    old_target: BlockId,
}

#[cfg(test)]
mod test {
    #[test]
    fn basic_no_split() {
        let mut func = testutils::build_test_func(&[
            testutils::Block::jmp_op("a", ["b", "c"]),
            testutils::Block::jmp("b", "d"),
            testutils::Block::jmp("c", "d"),
            testutils::Block::ret("d"),
        ]);
        let expected = func.clone();

        super::split_critical_edges(&mut func, true);
        testutils::assert_func_struct_eq(&func, &expected);
    }

    #[test]
    fn basic_split() {
        let mut func = testutils::build_test_func(&[
            testutils::Block::jmp_op("a", ["b", "c"]),
            testutils::Block::jmp("b", "d"),
            testutils::Block::jmp_op("c", ["d", "e"]),
            testutils::Block::ret("d"),
            testutils::Block::ret("e"),
        ]);

        super::split_critical_edges(&mut func, true);

        let expected = testutils::build_test_func(&[
            testutils::Block::jmp_op("a", ["b", "c"]),
            testutils::Block::jmp("b", "d"),
            testutils::Block::jmp_op("c", ["f", "e"]),
            testutils::Block::jmp("f", "d").unnamed(),
            testutils::Block::ret("d"),
            testutils::Block::ret("e"),
        ]);
        testutils::assert_func_struct_eq(&func, &expected);
    }
}
