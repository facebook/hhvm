// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::cmp::Reverse;
use std::sync::atomic::AtomicU32;
use std::sync::atomic::Ordering;

use balanced_partition::Doc;
use balanced_partition::ExternalId;
use depgraph_writer::HashIndex;
use depgraph_writer::MemDepGraph;
use hash::DashMap;
use newtype::IdVec;
use rayon::prelude::*;
use smallvec::SmallVec;

pub(crate) struct TransposedMemDepGraph {
    pub(crate) docs: Vec<Doc>,

    /// Concatenated slices of duplicated doc data.
    dups: Vec<HashIndex>,
}

impl TransposedMemDepGraph {
    pub(crate) fn get_hash_indexes<'a>(&'a self, doc: &'a Doc) -> &'a [HashIndex] {
        let weight = doc.weight;
        if weight == 1 {
            let r: &HashIndex = bytemuck::cast_ref(&doc.id.0);
            std::slice::from_ref(r)
        } else {
            let start = doc.id.0 as usize;
            &self.dups[start..start + weight as usize]
        }
    }
}

pub(crate) fn transpose(g: &MemDepGraph) -> TransposedMemDepGraph {
    log::info!("Starting to transpose graph");

    // Count how many in-edges each doc has.
    let mut num_in_edges: Vec<AtomicU32> = (0..g.hashes.len()).map(|_| AtomicU32::new(0)).collect();
    g.edge_lists.par_iter().flat_map(|b| &b[..]).for_each(|&n| {
        num_in_edges[n.as_usize()].fetch_add(1, Ordering::Relaxed);
    });

    // Figure out the starting index for each doc when all concatenated into
    // a single array. For example if the first doc has 4 in edges, and the second
    // has 3, then the slice for the third doc will start at array index 7.
    let mut num_edges = 0;
    let next_index: IdVec<HashIndex, AtomicU32> = num_in_edges
        .iter_mut()
        .map(|p| {
            let r = num_edges;
            num_edges += *p.get_mut();
            AtomicU32::new(r)
        })
        .collect();

    // Allocate a single block of storage to hold all the in-edges.
    let in_edges: Vec<AtomicU32> = (0..num_edges).map(|_| AtomicU32::new(u32::MAX)).collect();

    // Insert all of the edges (in nondeterministic order due to threading).
    // So the in edges for each doc will be in a known slice, but the order of
    // the values within that slice will be arbitrarily permuted.
    g.edge_lists
        .par_iter()
        .enumerate()
        .for_each(|(i, edge_list)| {
            for &to_doc in edge_list.iter() {
                let slot = next_index[to_doc].fetch_add(1, Ordering::Relaxed);
                in_edges[slot as usize].store(i as u32, Ordering::Relaxed);
            }
        });

    // TODO: Once the API has stabilized, use `in_edges.get_mut_slice()` since we don't need
    // atomic operations on the `in_edges` values any more.

    // Bucket identical docs together.
    let canonical_doc: DashMap<Box<[u32]>, SmallVec<[HashIndex; 4]>> = DashMap::default();

    next_index
        .vec
        .into_par_iter()
        .zip(num_in_edges.into_par_iter())
        .enumerate()
        .for_each(|(i, (end, size))| {
            let size = size.into_inner() as usize;
            if size == 0 {
                // Ignore docs with no in-edges.
                return;
            }

            let end = end.into_inner() as usize;
            let mut v: Box<[u32]> = in_edges[end - size..end]
                .iter()
                .map(|n| n.load(Ordering::Relaxed))
                .collect();
            v.sort_unstable();

            canonical_doc
                .entry(v)
                .or_default()
                .push(HashIndex::from_usize(i));
        });

    // `dups` is a single block of storage to hold slices for all of our duplicate doc slices.
    //
    // This capacity guess isn't exactly right, but doesn't need to be.
    let mut dups = Vec::with_capacity(g.hashes.len() - canonical_doc.len());

    let mut docs: Vec<Doc> = canonical_doc
        .into_iter()
        .map(|(edge_list, mut dups_with_edge_list)| {
            let weight = dups_with_edge_list.len() as u32;

            let id = if weight == 1 {
                // Don't allocate a slice in `dups` if we have just one (common case).
                dups_with_edge_list[0].0
            } else {
                // Canonicalize away the nondeterministic insertion order.
                dups_with_edge_list.sort_unstable();

                let start = dups.len() as u32;
                dups.extend(dups_with_edge_list);
                start
            };

            Doc {
                edge_list: edge_list.into(),
                weight,
                id: ExternalId(id),
            }
        })
        .collect();
    dups.shrink_to_fit();

    assert!(dups.len() <= u32::MAX as usize);

    // Sort to put longest edge lists first.
    docs.par_sort_unstable_by_key(|n| {
        let r = Reverse(n.edge_list.len());
        let id = n.id.0;

        // For determinism, break ties using this doc's smallest hash_index.
        let smallest_hash_index = if n.weight == 1 {
            HashIndex(id)
        } else {
            dups[id as usize]
        };
        (r, smallest_hash_index)
    });

    log::info!("Graph transpose done");

    TransposedMemDepGraph { docs, dups }
}
