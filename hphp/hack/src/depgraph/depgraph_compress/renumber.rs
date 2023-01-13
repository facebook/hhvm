// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::sync::atomic::AtomicU32;
use std::sync::atomic::Ordering;

use depgraph_writer::HashIndex;
use depgraph_writer::HashListIndex;
use depgraph_writer::MemDepGraph;
use newtype::IdVec;
use rayon::prelude::*;
use transpose::TransposedMemDepGraph;

use crate::*;

/// Apply the proposed node renumbering.
pub(crate) fn renumber(g: &mut MemDepGraph, remap_old_to_new: IdVec<HashIndex, HashIndex>) {
    assert_eq!(g.hashes.len(), remap_old_to_new.len());

    log::info!("Updating edge lists to use the new numbering system");

    // Update edge lists to use the new numbering system.
    g.edge_lists.par_iter_mut().for_each(|edge_list| {
        for h in edge_list.iter_mut() {
            *h = remap_old_to_new[*h];
        }
        edge_list.sort_unstable()
    });

    log::info!("Permuting hashes and edge_list_indices tables");

    // Create the inverse mapping so we can permute hashes and edge_list_indices.
    let remap_new_to_old: IdVec<HashIndex, AtomicU32> = IdVec::new_from_vec(
        (0..remap_old_to_new.len())
            .map(|_| AtomicU32::new(!0))
            .collect(),
    );
    remap_old_to_new
        .vec
        .into_par_iter()
        .enumerate()
        .for_each(|(old_index, new_index)| {
            remap_new_to_old[new_index].store(old_index as u32, Ordering::Relaxed)
        });

    // Permute `hashes` and `edge_list_indices` to use the new numbering system.
    //
    // I initially used an in-place permute, which doesn't need these temp arrays,
    // or even `remap_new_to_old`, but that algorithm was single-threaded and thus
    // much slower overall.
    let old_hashes = g.hashes.clone();
    let old_edge_list_indices = g.edge_list_indices.clone();
    remap_new_to_old
        .vec
        .into_par_iter()
        .zip(g.hashes.par_iter_mut())
        .zip(g.edge_list_indices.par_iter_mut())
        .for_each(move |((old_index, hash), edge_list_index)| {
            let old_index = HashIndex(old_index.into_inner());
            *hash = old_hashes[old_index];
            *edge_list_index = old_edge_list_indices[old_index]
        });
}

/// Take the node ordering from `tg` and apply it to `g`.
///
/// `tg` only cares about nodes with in-edges, so we need to also determine numbers
/// for the rest.
pub(crate) fn apply_node_renumbering(g: &mut MemDepGraph, tg: TransposedMemDepGraph) {
    log::info!("Applying node numbering");

    log::info!("Assigning node numbers for nodes with in-edges");

    let num_hashes = g.hashes.len();
    let mut remap_old_to_new: IdVec<HashIndex, HashIndex> =
        IdVec::new_from_vec(vec![HashIndex(!0u32); num_hashes]);

    // Number all of the nodes with any in-edges consecutively. We want those to have
    // the smallest numbers because they get named using variable-length integers.
    let mut out = 0;
    for n in tg.docs.iter() {
        for &e in tg.get_hash_indexes(n) {
            remap_old_to_new[e] = HashIndex(out);
            out += 1;
        }
    }

    log::info!("Assigning node numbers for nodes with out-edges but no in-edges");

    // Count how many not-yet-numbered nodes share each edge list.
    let num_edge_list_users: IdVec<HashListIndex, AtomicU32> =
        IdVec::new_from_vec((0..g.edge_lists.len()).map(|_| AtomicU32::new(0)).collect());
    g.edge_list_indices
        .par_iter()
        .zip(remap_old_to_new.par_iter())
        .for_each(|(&hash_list_index, &remap)| {
            if remap == HashIndex(!0u32) {
                num_edge_list_users[hash_list_index].fetch_add(1, Ordering::Relaxed);
            }
        });

    // Figure out the range for each edge list, so we can group nodes that use the same
    // edge list together with consecutive values.
    let mut edge_list_users_start = num_edge_list_users;
    edge_list_users_start
        .iter_mut()
        .for_each(|n| out += std::mem::replace(n.get_mut(), out));

    // Next, number everything else not already numbered. We do it in such a way that nodes
    // that share the same edge list tend to be numbered consecutively, which makes the way
    // we later write out `MemDepGraph::edge_list_indices` more compact.
    for (&hash_list_index, remap) in g.edge_list_indices.iter().zip(remap_old_to_new.iter_mut()) {
        if *remap == HashIndex(!0u32) {
            let next_index = edge_list_users_start[hash_list_index].get_mut();
            *remap = HashIndex(*next_index);
            *next_index += 1;
        }
    }
    drop(edge_list_users_start);

    renumber::renumber(g, remap_old_to_new);

    log::info!("Applying node numbering done");
}
