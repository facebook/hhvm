// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use analysis::{PredecessorCatchMode, PredecessorFlags, Predecessors};
use core::{
    instr::{HasOperands, Terminator},
    BlockId, Func, Instr, InstrId, ValueId, ValueIdMap,
};
use itertools::Itertools;
use log::trace;
use newtype::IdVec;
use print::FmtRawVid;

/// Attempt to clean a Func by doing several clean-up steps:
/// - rpo_sort
/// - remove common JmpArgs parameters
/// - renumber Instrs
/// - remove unreferenced Instrs.
pub fn run<'a>(func: &mut Func<'a>) {
    crate::rpo_sort(func);
    remove_common_args(func);
    renumber(func);
}

/// Go through a Func and for any BlockParam that is passed the same value from
/// all callers snap the value through to the passed value and remove the
/// BlockParam.
fn remove_common_args(func: &mut Func<'_>) {
    // TODO: This algorithm could be a lot smarter about when it needs to
    // reprocess a block.

    let predecessors = analysis::compute_predecessor_blocks(
        func,
        PredecessorFlags {
            mark_entry_blocks: false,
            catch: PredecessorCatchMode::FromNone,
        },
    );

    let mut remap: IdVec<InstrId, ValueId> = IdVec::new_from_vec(
        (0..func.instrs.len())
            .map(|iid| ValueId::from_instr(InstrId::from_usize(iid)))
            .collect(),
    );

    let mut changed = true;
    while changed {
        changed = false;

        for bid in func.block_ids() {
            if func.block(bid).params.is_empty() {
                continue;
            }

            trace!("Block: {}", bid);

            let common_params = compute_common_args(func, &mut remap, bid, &predecessors);
            trace!("  result = {:?}", common_params);
            match common_params {
                CommonArgs::Ignore => {
                    // Ignore this block.
                }
                CommonArgs::CatchArgs => {
                    // There should be exactly one arg.
                }
                CommonArgs::NoCommon => {
                    // Nothing to do - everything is an arg.
                }
                CommonArgs::AllCommon(common_params) => {
                    // Everything is common - turn the predecessors into a
                    // simple Jmp and zap the params. Don't forget to remap the
                    // params to the common args.

                    if let Some(preds) = predecessors.get(&bid) {
                        for &pred in preds.iter() {
                            let terminator = func.terminator_mut(pred);
                            match *terminator {
                                Terminator::JmpArgs(target, sc, _, loc) => {
                                    *terminator = Terminator::Jmp(target, sc, loc);
                                }
                                _ => unreachable!(),
                            }
                        }
                    }

                    let block = func.block_mut(bid);
                    for (param, common) in block.params.drain(..).zip(common_params.into_iter()) {
                        trace!("    remap: {} => {}", param, FmtRawVid(common));
                        remap[param] = common;
                    }
                }
                CommonArgs::SomeCommon(common_params) => {
                    changed = true;

                    trace!(
                        "    common: [{}]",
                        common_params
                            .iter()
                            .map(|&vid| FmtRawVid(vid).to_string())
                            .join(", ")
                    );

                    let mut remove = Vec::with_capacity(common_params.len());
                    for (i, (&param, common)) in func
                        .block(bid)
                        .params
                        .iter()
                        .zip(common_params.into_iter())
                        .enumerate()
                        .filter(|(_, (_, common))| !common.is_none())
                    {
                        trace!("    remap: {} => {}", param, FmtRawVid(common));
                        remap[param] = common;
                        remove.push(i);
                    }

                    // Remove the unused args from the predecessors.
                    if let Some(preds) = predecessors.get(&bid) {
                        for &pred in preds.iter() {
                            let terminator = func.terminator_mut(pred);
                            match terminator {
                                Terminator::JmpArgs(_, _, old_values, _) => {
                                    let values = remove_multi(
                                        old_values.iter().copied(),
                                        remove.iter().copied(),
                                    )
                                    .collect_vec();
                                    *old_values = values.into_boxed_slice();
                                }
                                _ => unreachable!(),
                            }
                        }
                    }

                    // Remove the unused args from the block params.
                    let params = &mut func.block_mut(bid).params;
                    *params =
                        remove_multi(params.iter().copied(), remove.iter().copied()).collect();
                }
            }
        }
    }

    let remap_vids: ValueIdMap<ValueId> = remap
        .into_iter()
        .enumerate()
        .filter_map(|(src_idx, dst)| {
            let src = ValueId::from_instr(InstrId::from_usize(src_idx));
            if src == dst { None } else { Some((src, dst)) }
        })
        .collect();
    func.remap_vids(&remap_vids);
}

/// Produce an iterator with the indices of `remove` removed.  `remove` must be
/// sorted.
///
/// For example:
///   remove_multi([1, 2, 3, 4, 5, 6].iter(), [1, 3].iter())
/// would return `[1, 3, 5, 6].iter()`
///
#[allow(unreachable_code, unused_variables)]
fn remove_multi<T, I, R>(input: I, remove: R) -> impl Iterator<Item = T>
where
    I: Iterator<Item = T>,
    R: Iterator<Item = usize>,
{
    let mut last_remove: Option<usize> = None;
    let mut remove = remove.peekable();
    input.enumerate().filter_map(move |(i, value)| {
        if Some(&i) == remove.peek() {
            // Ensure that 'remove' was actually sorted...
            debug_assert!(last_remove.map_or(true, |last| last <= i));
            last_remove = Some(i);
            remove.next();
            None
        } else {
            Some(value)
        }
    })
}

#[derive(Debug)]
enum CommonArgs {
    Ignore,
    CatchArgs,
    NoCommon,
    AllCommon(Vec<ValueId>),
    // Common args are marked as the common value. Mixed args are marked with
    // ValueId::none().
    SomeCommon(Vec<ValueId>),
}

fn compute_common_args(
    func: &Func<'_>,
    remap: &mut IdVec<InstrId, ValueId>,
    bid: BlockId,
    predecessors: &Predecessors,
) -> CommonArgs {
    if let Some(preds) = predecessors.get(&bid) {
        trace!(
            "  Predecessors: [{}]",
            preds.iter().map(|bid| bid.to_string()).join(", ")
        );

        let mut pred_iter = preds.iter().copied();
        if let Some(first_pred) = pred_iter.next() {
            if first_pred == BlockId::NONE {
                // This is a catch block.
                return CommonArgs::CatchArgs;
            }

            let pred_terminator = func.terminator(first_pred);
            if !matches!(pred_terminator, Terminator::JmpArgs(..)) {
                // The predecessor is not a JmpArgs - we can't do anything here.
                return CommonArgs::Ignore;
            }
            let mut common_params = pred_terminator
                .operands()
                .iter()
                .map(|&iid| lookup(remap, iid))
                .collect_vec();
            let mut all_common = true;
            let mut some_common = false;
            trace!(
                "    pred {}: args [{}]",
                first_pred,
                common_params
                    .iter()
                    .map(|&vid| FmtRawVid(vid).to_string())
                    .join(", ")
            );
            for next_pred in pred_iter {
                let next_terminator = func.terminator(next_pred);
                if !matches!(next_terminator, Terminator::JmpArgs(..)) {
                    // The predecessor is not a JmpArgs - we can't do anything here.
                    return CommonArgs::Ignore;
                }
                let next_params = next_terminator.operands();
                trace!(
                    "    pred {}: args [{}]",
                    next_pred,
                    next_params
                        .iter()
                        .map(|&vid| FmtRawVid(lookup(remap, vid)).to_string())
                        .join(", ")
                );
                for (idx, &iid) in next_params.iter().enumerate() {
                    if common_params[idx] != lookup(remap, iid) {
                        common_params[idx] = ValueId::none();
                        all_common = false;
                    } else {
                        some_common = true;
                    }
                }
            }

            match (all_common, some_common) {
                (false, false) => CommonArgs::NoCommon,
                (false, true) => CommonArgs::SomeCommon(common_params),
                (true, _) => CommonArgs::AllCommon(common_params),
            }
        } else {
            // No predecessors - ignore it
            CommonArgs::Ignore
        }
    } else {
        // No predecessors - ignore it
        CommonArgs::Ignore
    }
}

fn lookup(remap: &mut IdVec<InstrId, ValueId>, vid: ValueId) -> ValueId {
    if let Some(iid) = vid.instr() {
        let repl = remap[iid];
        if repl != ValueId::from_instr(iid) {
            // We found a replacement (rare), so go ahead and do the extra
            // work of following and optimizing the replacement chain.

            let mut prev = iid;
            let mut cur = repl;

            while let Some(cur_instr) = cur.instr() {
                let next = remap[cur_instr];
                if next != cur {
                    // This is a multi-hop chain (very rare).
                    //
                    // At this point we have the chain: prev -> cur -> next.
                    // Shorten it as in union-find path splitting to speed
                    // future lookups by making prev point directly to next.
                    remap[prev] = next;
                    prev = cur_instr;
                    cur = next;
                } else {
                    break;
                }
            }

            cur
        } else {
            repl
        }
    } else {
        vid
    }
}

fn renumber(func: &mut Func<'_>) {
    let mut remap: ValueIdMap<ValueId> = ValueIdMap::default();
    let mut src_instrs = std::mem::take(&mut func.instrs);
    let mut dst_instrs: IdVec<InstrId, Instr> = IdVec::with_capacity(func.instrs_len());

    for bid in func.block_ids() {
        let block = func.block_mut(bid);

        block.params = remapper(&mut remap, &mut src_instrs, &block.params, &mut dst_instrs);
        block.iids = remapper(&mut remap, &mut src_instrs, &block.iids, &mut dst_instrs);
    }

    func.instrs = dst_instrs;
    func.remap_vids(&remap);
}

fn remapper(
    remap: &mut ValueIdMap<ValueId>,
    src_instrs: &mut IdVec<InstrId, Instr>,
    src_iids: &[InstrId],
    dst_instrs: &mut IdVec<InstrId, Instr>,
) -> Vec<InstrId> {
    let mut expected_iid = InstrId::from_usize(dst_instrs.len());
    let mut dst_iids = Vec::new();
    for &iid in src_iids {
        if iid != expected_iid {
            remap.insert(ValueId::from_instr(iid), ValueId::from_instr(expected_iid));
        }
        let instr = std::mem::replace(&mut src_instrs[iid], Instr::tombstone());
        dst_instrs.push(instr);
        dst_iids.push(expected_iid);
        expected_iid.0 += 1;
    }

    dst_iids
}

#[cfg(test)]
mod test {
    use super::*;

    #[test]
    fn test_lookup() {
        let mut remap: IdVec<InstrId, ValueId> = IdVec::new_from_vec(
            (0..10)
                .map(|iid| ValueId::from_instr(InstrId::from_usize(iid)))
                .collect(),
        );

        fn iid(n: usize) -> InstrId {
            InstrId::from_usize(n)
        }
        fn vid(n: usize) -> ValueId {
            ValueId::from_instr(InstrId::from_usize(n))
        }

        assert_eq!(lookup(&mut remap, vid(0)), vid(0));
        assert_eq!(lookup(&mut remap, vid(1)), vid(1));

        remap[iid(2)] = vid(5);
        remap[iid(5)] = vid(6);
        assert_eq!(lookup(&mut remap, vid(2)), vid(6));
        assert_eq!(lookup(&mut remap, vid(2)), vid(6));

        remap[iid(6)] = vid(7);
        assert_eq!(lookup(&mut remap, vid(2)), vid(7));
    }
}
