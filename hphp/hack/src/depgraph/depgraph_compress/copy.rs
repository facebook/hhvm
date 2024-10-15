// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::path::Path;
use std::sync::atomic::AtomicBool;
use std::sync::atomic::Ordering;

use depgraph_reader::DepGraph;
use depgraph_writer::HashIndex;
use depgraph_writer::MemDepGraph;
use newtype::IdVec;
use rayon::prelude::*;

use crate::*;

pub(crate) fn copy_node_order(m: &mut MemDepGraph, path: &Path) -> std::io::Result<()> {
    log::info!("Copying node order from existing file {}", path.display());

    let dep_graph = DepGraph::from_path(path)?;

    let num_hashes = m.hashes.len();
    let in_use: IdVec<HashIndex, AtomicBool> =
        IdVec::new_from_vec((0..num_hashes).map(|_| AtomicBool::new(false)).collect());
    let mut remap_old_to_new: IdVec<HashIndex, HashIndex> =
        IdVec::new_from_vec(vec![HashIndex::NONE; num_hashes]);

    // Collect of the HashIndex slots where we weren't able to assign a mapping.
    let gaps: Vec<&mut HashIndex> = m
        .hashes
        .par_iter()
        .zip(remap_old_to_new.par_iter_mut())
        .filter_map(|(&dep, remap_ref)| {
            if let Some(index) = dep_graph.get_index(dep) {
                // `old_graph` may have more HashIndexes than `m`, but we want to keep
                // `m`'s numbering dense so if we see an out-of-range HashIndex just
                // pretend we didn't see it.
                let h = HashIndex(index);
                if let Some(in_use_ref) = in_use.get(h) {
                    in_use_ref.store(true, Ordering::Relaxed);
                    *remap_ref = h;
                    return None;
                }
            }

            // We failed to map, so remember that this slot needs to be filled in.
            Some(remap_ref)
        })
        .collect();

    // Find all HashIndex values not in use.
    let available: Vec<HashIndex> = in_use
        .vec
        .into_par_iter()
        .enumerate()
        .filter_map(|(index, in_use)| {
            if in_use.into_inner() {
                None
            } else {
                Some(HashIndex::from_usize(index))
            }
        })
        .collect();

    // Fill in each gap with an unused HashIndex in increasing order.
    assert_eq!(gaps.len(), available.len());
    gaps.into_par_iter()
        .zip(available.into_par_iter())
        .for_each(|(gap, h)| *gap = h);

    // Apply the new numbering we just chose to `m`.
    renumber::renumber(m, remap_old_to_new);

    Ok(())
}
