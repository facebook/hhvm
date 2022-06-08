// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use core::{Func, Instr, InstrId, ValueId, ValueIdMap};
use newtype::IdVec;

/// Attempt to clean a Func by doing an rpo_sort and then renumbering the
/// InstrIds to be contiguous based on the RPO ordering. It also removes any
/// unreferenced InstrIds from the Func.
pub fn run<'a>(func: &mut Func<'a>) {
    crate::rpo_sort(func);

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
