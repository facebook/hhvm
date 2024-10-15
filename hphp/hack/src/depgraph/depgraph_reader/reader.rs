// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

pub mod byteutils;
pub mod compress;

use std::fs::File;
use std::iter::FusedIterator;
use std::ops::Range;

pub use dep::Dep;
use memmap2::Mmap;
use rayon::iter::Either;
use rayon::prelude::*;

use crate::compress::*;

/// An opaque token that identifies a hash list.
///
/// This can be used to see whether two Deps have the same HashList.
/// It can also be used to get the actual HashList.
#[derive(Clone, Copy, Hash, PartialEq, Eq)]
pub struct HashListId(u32);

pub trait BaseDepgraphTrait {
    fn iter_dependents(&self, dep: Dep) -> Box<dyn Iterator<Item = Dep> + '_>;
    fn dependent_dependency_edge_exists(&self, dependent: Dep, dependency: Dep) -> bool;
}

impl BaseDepgraphTrait for DepGraph {
    fn iter_dependents(&self, dep: Dep) -> Box<dyn Iterator<Item = Dep> + '_> {
        Box::new(match self.hash_list_for(dep) {
            None => Either::Left(std::iter::empty()),
            Some(hashes) => Either::Right(self.hash_list_hashes(hashes)),
        })
    }
    fn dependent_dependency_edge_exists(&self, dependent: Dep, dependency: Dep) -> bool {
        self.dependent_dependency_edge_exists(dependent, dependency)
    }
}

/// An memory-mapped dependency graph.
pub struct DepGraph {
    /// The file holding the storage for this graph.
    storage: Mmap,

    /// All Deps in the graph. These are NOT sorted -- use `deps_order` if you need sorting.
    ///
    /// This holds the byte range in the mmap file for this data -- use `deps()` to access.
    deps_range: Range<usize>,

    /// Indices into `deps` providing sorted order, e.g. deps[deps_order[0]] is first.
    /// One entry per entry in `deps`.
    ///
    /// This holds the byte range in the mmap file for this data -- use `deps_order()` to access.
    deps_order_range: Range<usize>,

    /// Indices in `adjacency_lists` for the serialized edge list for the corresponding `deps`
    /// entry. One entry per entry in `deps`.
    ///
    /// Each entry in this array must be left shifted by `adjacency_list_alignment_shift`
    /// before being used as an index. This is to support `edge_lists` larger than 4GB.
    ///
    /// This holds the byte range in the mmap file for this data -- use `unshifted_edge_list_offset_range()` to access.
    unshifted_edge_list_offset_range: Range<usize>,

    /// Amount to left-shift unshifted_edge_list_offset to get a byte index into `adjacency_lists`.
    adjacency_list_alignment_shift: u8,

    /// Individually serialized edge lists. `HashList` knows how to deserialize.
    /// This holds the byte range in the mmap file for this data -- use `unshifted_lists_range()` to access.
    adjacency_lists_range: Range<usize>,
}

impl DepGraph {
    /// Open the dependency graph, or return an error description.
    pub fn from_mmap(mmap: Mmap) -> Result<DepGraph, String> {
        let in_bytes: &[u8] = mmap.as_ref();
        if in_bytes.len() >= std::mem::size_of::<UncompressedHeader>() {
            if let Ok(maybe_header) = bytemuck::try_from_bytes::<UncompressedHeader>(
                &in_bytes[..std::mem::size_of::<UncompressedHeader>()],
            ) {
                // It's technically possible for the first four bytes of an old file
                // to randomly be "HHDG", but the version won't look like a small number.
                // By the time we get to a large version number we'll have deleted OldDepGraph.
                if maybe_header.magic == UncompressedHeader::MAGIC && maybe_header.version < 100 {
                    return DepGraph::from_mmap_impl(mmap);
                }
            }
        }
        Err("Unable to open depgraph from mmap, possibly unexpected old depgraph format".into())
    }

    /// Create a dependency graph opener given an open file handle.
    ///
    /// The file handle can be safely closed afterwards.
    pub fn from_file(file: &std::fs::File) -> std::io::Result<Self> {
        // Safety: we rely on the memmap library to provide safety.
        let mmap = unsafe { Mmap::map(file) }?;
        Self::from_mmap(mmap).map_err(|e| std::io::Error::new(std::io::ErrorKind::InvalidData, e))
    }

    /// Create a dependency graph opener given a file path.
    pub fn from_path<P: AsRef<std::path::Path>>(path: P) -> std::io::Result<Self> {
        let f = File::open(path)?;
        Self::from_file(&f)
    }

    fn from_mmap_impl(mmap: Mmap) -> Result<Self, String> {
        let data: &[u8] = mmap.as_ref();
        let hlen = std::mem::size_of::<UncompressedHeader>();
        if data.len() < hlen {
            return Err("Missing header".to_string());
        }

        let header_bytes = &data[..hlen];
        let header: &UncompressedHeader =
            bytemuck::try_from_bytes(header_bytes).map_err(|e| format!("{}", e))?;
        if header.magic != UncompressedHeader::MAGIC {
            return Err("Incorrect header magic number".to_string());
        }

        let expected_version = UncompressedHeader::LATEST_VERSION;
        if header.version != expected_version {
            return Err(format!(
                "Incorrect header version; expected {}, got {}",
                expected_version, header.version
            ));
        }

        let num_deps = header.num_deps as usize;

        let g = DepGraph {
            deps_range: hlen..hlen + num_deps * 8,
            deps_order_range: hlen + num_deps * 8..hlen + num_deps * 12,
            unshifted_edge_list_offset_range: hlen + num_deps * 12..hlen + num_deps * 16,
            adjacency_list_alignment_shift: header.adjacency_list_alignment_shift,
            adjacency_lists_range: hlen + num_deps * 16..mmap.len(),
            storage: mmap,
        };

        Ok(g)
    }

    /// Return `true` iff the given hash list contains the index for the given hash.
    pub fn hash_list_contains(&self, hash_list: HashList<'_>, dep: Dep) -> bool {
        if let Some(index) = self.get_index(dep) {
            hash_list.has_index(index)
        } else {
            false
        }
    }

    fn deps(&self) -> &[Dep] {
        bytemuck::cast_slice(&self.storage[self.deps_range.clone()])
    }

    fn deps_order(&self) -> &[u32] {
        bytemuck::cast_slice(&self.storage[self.deps_order_range.clone()])
    }

    fn unshifted_edge_list_offset(&self) -> &[u32] {
        bytemuck::cast_slice(&self.storage[self.unshifted_edge_list_offset_range.clone()])
    }

    fn adjacency_lists(&self) -> &[u8] {
        &self.storage[self.adjacency_lists_range.clone()]
    }

    /// Implementation helper for `DepGraph::hash_list_hashes`.
    pub fn hash_list_hashes<'a>(
        &'a self,
        hash_list: HashList<'a>,
    ) -> impl Iterator<Item = Dep> + 'a {
        let deps = self.deps();
        hash_list.hash_indices().map(move |i| deps[i as usize])
    }

    /// Returns the internal, physical order for a Dep, or None if not found.
    pub fn get_index(&self, dep: Dep) -> Option<u32> {
        let deps = self.deps();
        let deps_order = self.deps_order();
        deps_order
            .binary_search_by_key(&dep, move |&i| deps[i as usize])
            .map_or(None, move |x| Some(deps_order[x]))
    }

    /// All unique dependency hashes in the graph, in sorted order.
    pub fn all_hashes(
        &self,
    ) -> impl DoubleEndedIterator<Item = Dep> + ExactSizeIterator + FusedIterator + '_ {
        let deps = self.deps();
        self.deps_order().iter().map(move |&i| deps[i as usize])
    }

    /// All unique dependency hashes in the graph, in sorted order, in parallel.
    pub fn par_all_hashes(&self) -> impl IndexedParallelIterator<Item = Dep> + '_ {
        let deps = self.deps();
        self.deps_order().par_iter().map(move |&i| deps[i as usize])
    }

    /// Returns all hashes in internal node order. More efficient than `par_all_hashes`.
    pub fn par_all_hashes_in_physical_order(
        &self,
    ) -> impl IndexedParallelIterator<Item = Dep> + '_ {
        self.deps().par_iter().copied()
    }

    pub fn hash_list_for(&self, hash: Dep) -> Option<HashList<'_>> {
        self.hash_list_id_for_dep(hash)
            .map(|id| self.hash_list_for_id(id))
    }

    /// Query the hash list for a given hash index.
    ///
    /// Returns `None` if there is no hash list related to the hash.
    pub fn hash_list_for_index(&self, index: u32) -> Option<HashList<'_>> {
        self.hash_list_id_for_index(index)
            .map(|id| self.hash_list_for_id(id))
    }

    /// Make sure the database is not corrupt.
    ///
    /// If you got this far, the indexer and lookup table were
    /// successfully initialized. This function checks whether
    /// all hash lists can be properly read from disk.
    pub fn validate_hash_lists(&self) -> Result<(), String> {
        // TODO: What to check here?
        Ok(())
    }

    pub fn hash_list_id_for_dep(&self, dep: Dep) -> Option<HashListId> {
        self.hash_list_id_for_index(self.get_index(dep)?)
    }

    fn hash_list_id_for_index(&self, index: u32) -> Option<HashListId> {
        // This function cannot fail, because we assume an index is always valid.
        // It would be crazy to be asking about some random unknown index.
        // Once OldDepGraph is gone, make this infallible.

        let id = HashListId(self.unshifted_edge_list_offset()[index as usize]);
        Some(id)
    }

    pub fn hash_list_for_id(&self, id: HashListId) -> HashList<'_> {
        let start = (id.0 as usize) << self.adjacency_list_alignment_shift;
        let bytes = &self.adjacency_lists()[start..];
        HashList::new(bytes)
    }

    /// Query the hash list for a given hash.
    ///
    /// Returns `None` if there is no hash list related to the hash.
    pub fn dependent_dependency_edge_exists(&self, dependent: Dep, dependency: Dep) -> bool {
        match self.hash_list_for(dependency) {
            Some(hash_list) => self.hash_list_contains(hash_list, dependent),
            None => false,
        }
    }

    pub fn contains(&self, dep: Dep) -> bool {
        self.get_index(dep).is_some()
    }
}

pub struct HashList<'bytes> {
    blocks: &'bytes [RleBlock],

    // The total number of indices that walking `hash_indices()` will yield.
    //
    // This is identical to the sum of all the `blocks` lengths, but it's
    // precomputed here to keep the `len()` method O(1).
    num_indices: u32,
}

impl<'bytes> HashList<'bytes> {
    fn new(mut b: &'bytes [u8]) -> Self {
        // The raw memory representation looks like two lengths followed by an array:
        //     num_blocks: vint64
        //     num_indices: vint64
        //     [RleBlock; num_blocks]
        let num_blocks = vint64::decode(&mut b).unwrap() as usize;

        // If the list is long enough, we'll have precomputed the number of indices.
        // This way self.len() is O(1) not O(N).
        let num_indices = vint64::decode(&mut b).unwrap() as u32;

        Self {
            blocks: bytemuck::cast_slice(&b[..num_blocks * std::mem::size_of::<RleBlock>()]),
            num_indices,
        }
    }

    pub fn is_empty(&self) -> bool {
        self.num_indices == 0
    }

    /// Returns the number of indices that `hash_indices()` will visit. This may be far more than
    /// the length of `self.blocks`, due to run-length encoding.
    pub fn len(&self) -> u32 {
        self.num_indices
    }

    fn has_index(&self, index: u32) -> bool {
        match self.blocks.binary_search_by_key(&index, |b| b.start) {
            Ok(_) => true,
            Err(slot) => {
                // Not an exact match for a block start, but maybe it's contained in a block.
                // `slot` is the insertion point, so we actually want the previous block.
                slot.checked_sub(1).map_or(false, |i| {
                    let b = &self.blocks[i];
                    // We already know b.start < index since binary_search got here,
                    // and we have no zero-length blocks so there's no ambiguity.
                    index - b.start < b.len()
                })
            }
        }
    }

    /// Return raw hash indices in this list.
    pub fn hash_indices(
        &self,
    ) -> impl DoubleEndedIterator<Item = u32> + std::iter::FusedIterator + 'bytes {
        // If we even care, we could create an ExactSizeIterator type using `self.num_indices`.
        self.blocks.iter().flat_map(|b| Range {
            start: b.start,
            end: b.start + b.len(),
        })
    }
}
