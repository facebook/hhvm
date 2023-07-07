// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use std::fs::File;
use std::io::BufWriter;
use std::io::Write;
use std::path::Path;

use bytemuck::Pod;
use bytemuck::Zeroable;
pub use dep::Dep;
use log::info;
use newtype::newtype_int;
use newtype::IdVec;

// A 32-bit index into a table of Deps.
newtype_int!(HashIndex, u32, HashIndexMap, HashIndexSet, Pod, Zeroable);

// Type-safe hash list index wrapper
newtype_int!(
    HashListIndex,
    u32,
    HashListIndexMap,
    HashListIndexSet,
    Pod,
    Zeroable
);

pub struct MemDepGraph {
    /// Unique hashes, in sorted order.
    pub hashes: IdVec<HashIndex, Dep>,

    /// Which edge list each member of `hashes` has (index int `edge_lists`).
    /// Same length as `hashes`.
    pub edge_list_indices: IdVec<HashIndex, HashListIndex>,

    /// Unique edge lists in some canonical order.
    pub edge_lists: IdVec<HashListIndex, Box<[HashIndex]>>,
}

impl MemDepGraph {
    /// Returns a succession of dependency -> dependents mappings, containing all of the
    /// dependents to which that dependency has an edge.
    pub fn all_edges(&self) -> impl Iterator<Item = (Dep, impl Iterator<Item = Dep> + '_)> + '_ {
        self.edge_list_indices
            .iter()
            .zip(self.hashes.iter())
            .filter_map(|(&edge_list_index, &dependency)| {
                let edges = &self.edge_lists[edge_list_index];
                if edges.is_empty() {
                    None
                } else {
                    Some((dependency, edges.iter().map(|&h| self.hashes[h])))
                }
            })
    }
}

/// Write the given `MemDepGraph` to disk.
pub fn write_dep_graph(output: &Path, g: &MemDepGraph) -> std::io::Result<()> {
    info!("Opening output file at {:?}", output);
    let f = File::create(output)?;

    let num_deps = g.hashes.len() as u64;
    let header_size = 8;
    let indexer_size = 8 + num_deps * 8;
    let lookup_size = num_deps * 4;

    info!("Calculating hash list offsets");
    let hash_list_start = header_size + indexer_size + lookup_size;
    let mut cur_offset = hash_list_start;

    // These are where the hash lists end up in memory.
    let edge_list_offsets: Vec<u32> = g
        .edge_lists
        .iter()
        .map(|edges| {
            if edges.is_empty() {
                // As a special case, the empty list pretends to have file offset 0.
                0
            } else {
                let offset = cur_offset;
                cur_offset += 4 + 4 * edges.len() as u64;
                offset as u32
            }
        })
        .collect();

    // Guarantee we didn't blow past the 4GiB file size limit.
    assert!(cur_offset <= !0u32 as u64 + 1);

    let mut out = BufWriter::new(f);

    // Write out the sections other than the hash lists.

    // Write the header and the size field at the start of the indexer.
    info!("Writing header");
    let indexer_offset = header_size;
    let lookup_offset = indexer_offset + indexer_size;
    out.write_all(&(indexer_offset as u32).to_ne_bytes())?;
    out.write_all(&(lookup_offset as u32).to_ne_bytes())?;

    // Write indexer.
    info!("Writing indexer");
    out.write_all(&num_deps.to_ne_bytes())?;
    out.write_all(bytemuck::cast_slice(&g.hashes))?;

    // Write hash list file offsets.
    info!("Writing hash list lookup table");
    for &i in g.edge_list_indices.iter() {
        out.write_all(&edge_list_offsets[i.0 as usize].to_ne_bytes())?;
    }

    // Write edge lists.
    info!("Writing edge lists");
    for h in g.edge_lists.iter() {
        if h.is_empty() {
            // As a special case, empty edge lists aren't stored in the file.
            continue;
        }

        out.write_all(&(h.len() as u32).to_ne_bytes())?;
        out.write_all(bytemuck::cast_slice(&h[..]))?;
    }

    // Flush the file and close it before logging we are done.
    drop(out.into_inner()?);

    info!(".hhdg write complete");

    Ok(())
}
