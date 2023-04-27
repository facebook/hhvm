// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
pub mod byteutils;
pub mod compress;

use std::fs::File;
use std::iter::FusedIterator;
use std::ops::Deref;
use std::ops::Range;

pub use dep::Dep;
use memmap2::Mmap;
use rayon::iter::Either;
use rayon::prelude::*;
use rpds::HashTrieSet;

use crate::compress::*;

/// An opaque token that identifies a hash list.
///
/// This can be used to see whether two Deps have the same HashList.
/// It can also be used to get the actual HashList.
#[derive(Clone, Copy, Hash, PartialEq, Eq)]
pub struct HashListId(u32);

pub trait DepGraphTrait {
    /// Make sure the database is not corrupt.
    ///
    /// If you got this far, the indexer and lookup table were
    /// successfully initialized. This function checks whether
    /// all hash lists can be properly read from disk.
    fn validate_hash_lists(&self) -> Result<(), String>;

    /// Query the hash list for a given hash.
    ///
    /// Returns `None` if there is no hash list related to the hash.
    ///
    /// # Panics
    ///
    /// Panics if the file is corrupt. Use `validate_hash_lists` when
    /// initializing the reader to avoid these panics.
    fn hash_list_for(&self, hash: Dep) -> Option<HashList<'_>> {
        self.hash_list_id_for_dep(hash)
            .map(|id| self.hash_list_for_id(id))
    }

    fn hash_list_id_for_dep(&self, hash: Dep) -> Option<HashListId>;

    fn hash_list_id_for_index(&self, index: u32) -> Option<HashListId>;

    fn hash_list_for_id(&self, id: HashListId) -> HashList<'_>;

    /// Query the hash list for a given hash index.
    ///
    /// Returns `None` if there is no hash list related to the hash.
    fn hash_list_for_index(&self, index: u32) -> Option<HashList<'_>> {
        self.hash_list_id_for_index(index)
            .map(|id| self.hash_list_for_id(id))
    }

    fn dependent_dependency_edge_exists(&self, dependent: Dep, dependency: Dep) -> bool;

    fn contains(&self, dep: Dep) -> bool;
}

pub enum DepGraph {
    New(NewDepGraph),
    Old(OldDepGraph),
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
                    return Ok(DepGraph::New(NewDepGraph::from_mmap(mmap)?));
                }
            }
        }
        Ok(DepGraph::Old(OldDepGraph::from_mmap(mmap)?))
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

    /// Return an iterator over all hashes in a hash list.
    pub fn hash_list_hashes<'a>(
        &'a self,
        hash_list: HashList<'a>,
    ) -> impl Iterator<Item = Dep> + 'a {
        match (self, hash_list) {
            (DepGraph::New(dg), HashList::New(h)) => Either::Left(dg.hash_list_hashes(h)),
            (DepGraph::Old(dg), HashList::Old(h)) => Either::Right(dg.hash_list_hashes(h)),
            _ => panic!("HashList type mismatch!"),
        }
    }

    /// All unique dependency hashes in the graph.
    pub fn all_hashes(&self) -> impl DoubleEndedIterator<Item = Dep> + ExactSizeIterator + '_ {
        match self {
            DepGraph::New(dg) => Either::Left(dg.all_hashes()),
            DepGraph::Old(dg) => Either::Right(dg.all_hashes()),
        }
    }

    /// All unique dependency hashes in the graph.
    pub fn par_all_hashes(&self) -> impl IndexedParallelIterator<Item = Dep> + '_ {
        match self {
            DepGraph::New(dg) => Either::Left(dg.par_all_hashes()),
            DepGraph::Old(dg) => Either::Right(dg.par_all_hashes()),
        }
    }

    /// Add the direct typing dependents for one dependency (i.e. the fanout of
    /// that one dependency).
    pub fn add_typing_deps_for_dep(&self, acc: &mut HashTrieSet<Dep>, dep: Dep) {
        if let Some(dept_hash_list) = self.hash_list_for(dep) {
            for dept in self.hash_list_hashes(dept_hash_list) {
                acc.insert_mut(dept);
            }
        }
    }

    /// Query the direct typing dependents for the given set of dependencies.
    pub fn query_typing_deps_multi(&self, deps: &HashTrieSet<Dep>) -> HashTrieSet<Dep> {
        let mut acc = deps.clone();
        for dep in deps {
            self.add_typing_deps_for_dep(&mut acc, *dep);
        }
        acc
    }
}

impl Deref for DepGraph {
    type Target = dyn DepGraphTrait + Send + Sync;

    fn deref(&self) -> &Self::Target {
        match self {
            DepGraph::New(dg) => dg,
            DepGraph::Old(dg) => dg,
        }
    }
}

/// An memory-mapped dependency graph.
#[ouroboros::self_referencing]
pub struct NewDepGraph {
    /// The file holding the storage for this graph. Boxed for ouroboros.
    storage: Box<Mmap>,

    /// All Deps in the graph. These are NOT sorted -- use `deps_order` if you need sorting.
    #[borrows(storage)]
    deps: &'this [Dep],

    /// Indices into `deps` providing sorted order, e.g. deps[deps_order[0]] is first.
    /// One entry per entry in `deps`.
    #[borrows(storage)]
    deps_order: &'this [u32],

    /// Indices in `adjacency_lists` for the serialized edge list for the corresponding `deps`
    /// entry. One entry per entry in `deps`.
    ///
    /// Each entry in this array must be left shifted by `adjacency_list_alignment_shift`
    /// before being used as an index. This is to support `edge_lists` larger than 4GB.
    #[borrows(storage)]
    unshifted_edge_list_offset: &'this [u32],

    /// Amount to left-shift unshifted_edge_list_offset to get a byte index into `adjacency_lists`.
    adjacency_list_alignment_shift: u8,

    /// Individually serialized edge lists. `NewHashList` knows how to deserialize.
    #[borrows(storage)]
    adjacency_lists: &'this [u8],
}

impl NewDepGraph {
    fn from_mmap(mmap: Mmap) -> Result<Self, String> {
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
        let adjacency_list_alignment_shift = header.adjacency_list_alignment_shift;

        let builder = NewDepGraphBuilder {
            storage: Box::new(mmap),
            deps_builder: |mmap: &Mmap| bytemuck::cast_slice(&mmap[hlen..hlen + num_deps * 8]),
            deps_order_builder: |mmap: &Mmap| {
                bytemuck::cast_slice(&mmap[hlen + num_deps * 8..hlen + num_deps * 12])
            },
            unshifted_edge_list_offset_builder: |mmap: &Mmap| {
                bytemuck::cast_slice(&mmap[hlen + num_deps * 12..hlen + num_deps * 16])
            },
            adjacency_list_alignment_shift,
            adjacency_lists_builder: |mmap: &Mmap| &mmap[hlen + num_deps * 16..],
        };

        Ok(builder.build())
    }

    /// Return `true` iff the given hash list contains the index for the given hash.
    fn hash_list_contains(&self, hash_list: HashList<'_>, dep: Dep) -> bool {
        if let Some(index) = self.get_index(dep) {
            hash_list.has_index(index)
        } else {
            false
        }
    }

    /// Implementation helper for `DepGraph::hash_list_hashes`.
    fn hash_list_hashes<'a>(
        &'a self,
        hash_list: NewHashList<'a>,
    ) -> impl Iterator<Item = Dep> + 'a {
        hash_list
            .hash_indices()
            .map(|index| self.borrow_deps()[index as usize])
    }

    /// Returns the internal, physical order for a Dep, or None if not found.
    pub fn get_index(&self, dep: Dep) -> Option<u32> {
        let deps = self.borrow_deps();
        let deps_order = self.borrow_deps_order();
        deps_order
            .binary_search_by_key(&dep, move |&i| deps[i as usize])
            .map_or(None, move |x| Some(deps_order[x]))
    }

    /// All unique dependency hashes in the graph, in sorted order.
    pub fn all_hashes(
        &self,
    ) -> impl DoubleEndedIterator<Item = Dep> + ExactSizeIterator + FusedIterator + '_ {
        let deps = self.borrow_deps();
        self.borrow_deps_order()
            .iter()
            .map(move |&i| deps[i as usize])
    }

    /// All unique dependency hashes in the graph, in sorted order, in parallel.
    pub fn par_all_hashes(&self) -> impl IndexedParallelIterator<Item = Dep> + '_ {
        let deps = self.borrow_deps();
        self.borrow_deps_order()
            .par_iter()
            .map(move |&i| deps[i as usize])
    }

    /// Returns all hashes in internal node order. More efficient than `par_all_hashes`.
    pub fn par_all_hashes_in_physical_order(
        &self,
    ) -> impl IndexedParallelIterator<Item = Dep> + '_ {
        self.borrow_deps().par_iter().copied()
    }
}

impl DepGraphTrait for NewDepGraph {
    fn validate_hash_lists(&self) -> Result<(), String> {
        // TODO: What to check here?
        Ok(())
    }

    fn hash_list_id_for_dep(&self, dep: Dep) -> Option<HashListId> {
        self.hash_list_id_for_index(self.get_index(dep)?)
    }

    fn hash_list_id_for_index(&self, index: u32) -> Option<HashListId> {
        // This function cannot fail, because we assume an index is always valid.
        // It would be crazy to be asking about some random unknown index.
        // Once OldDepGraph is gone, make this infallible.

        let id = HashListId(self.borrow_unshifted_edge_list_offset()[index as usize]);
        Some(id)
    }

    fn hash_list_for_id(&self, id: HashListId) -> HashList<'_> {
        let start = (id.0 as usize) << self.borrow_adjacency_list_alignment_shift();
        let bytes = &self.borrow_adjacency_lists()[start..];
        HashList::New(NewHashList::new(bytes))
    }

    /// Return whether the given dependent-to-dependency edge is in the graph.
    fn dependent_dependency_edge_exists(&self, dependent: Dep, dependency: Dep) -> bool {
        match self.hash_list_for(dependency) {
            Some(hash_list) => self.hash_list_contains(hash_list, dependent),
            None => false,
        }
    }

    fn contains(&self, dep: Dep) -> bool {
        self.get_index(dep).is_some()
    }
}

/// An memory-mapped dependency graph.
#[ouroboros::self_referencing]
pub struct OldDepGraph {
    /// The file holding the storage for this graph. Boxed for ouroboros.
    storage: Box<Mmap>,

    #[borrows(storage)]
    data: &'this [u8],

    #[borrows(storage)]
    #[covariant]
    indexer: Indexer<'this>,

    #[borrows(storage)]
    #[covariant]
    lookup_table: LookupTable<'this>,
}

impl OldDepGraph {
    /// Initialize a dependency graph using the byte array
    /// from a memory map.
    fn from_mmap(mmap: Mmap) -> Result<Self, String> {
        let data = mmap.deref();

        if data.len() < 4 * 2 {
            return Err("not enough bytes to read header".to_string());
        }

        // Parse the header of the structure.
        //
        // Contains the offset to the indexer and the lookup table.
        //
        // Memory layout:
        //
        // ```txt
        //           32 bits
        //  +-----------------------+
        //  |    indexer offset     |
        //  +-----------------------+
        //  |  lookup table offset  |
        //  +-----------------------+
        // ```
        let indexer_offset = byteutils::read_u32_ne(data);
        let lookup_table_offset = byteutils::read_u32_ne(&data[4..]);

        let indexer_offset: usize = indexer_offset.try_into().unwrap();
        let lookup_table_offset: usize = lookup_table_offset.try_into().unwrap();

        let indexer_bytes = byteutils::subslice(data, indexer_offset.., "indexer_bytes")?;
        let num_hashes = Indexer::new(indexer_bytes)?.len();

        let builder = OldDepGraphBuilder {
            storage: Box::new(mmap),
            data_builder: |mmap: &Mmap| mmap.as_ref(),
            indexer_builder: |mmap: &Mmap| Indexer::new(&mmap.as_ref()[indexer_offset..]).unwrap(),
            lookup_table_builder: |mmap: &Mmap| {
                LookupTable::new(&mmap.as_ref()[lookup_table_offset..], num_hashes).unwrap()
            },
        };

        Ok(builder.build())
    }

    /// Return `true` iff the given hash list contains the index for the given hash.
    fn hash_list_contains(&self, hash_list: HashList<'_>, hash: Dep) -> bool {
        if let Some(index) = self.borrow_indexer().find(hash.into()) {
            hash_list.has_index(index)
        } else {
            false
        }
    }

    /// Implementation helper for `DepGraph::hash_list_hashes`.
    fn hash_list_hashes<'a>(
        &'a self,
        hash_list: OldHashList<'a>,
    ) -> impl Iterator<Item = Dep> + ExactSizeIterator + FusedIterator + 'a {
        let indexer = self.borrow_indexer();
        hash_list
            .indices
            .iter()
            .map(move |&index| Dep::new(indexer.hashes[index as usize]))
    }

    /// All unique dependency hashes in the graph.
    fn all_hashes(&self) -> impl DoubleEndedIterator<Item = Dep> + ExactSizeIterator + '_ {
        self.borrow_indexer().hashes.iter().copied().map(Dep::new)
    }

    /// All unique dependency hashes in the graph, in parallel.
    fn par_all_hashes(&self) -> impl IndexedParallelIterator<Item = Dep> + '_ {
        self.borrow_indexer()
            .hashes
            .par_iter()
            .copied()
            .map(Dep::new)
    }
}

impl DepGraphTrait for OldDepGraph {
    fn validate_hash_lists(&self) -> Result<(), String> {
        let len: usize = self.borrow_indexer().len();
        let lookup_table = self.borrow_lookup_table();
        for index in 0..len {
            match lookup_table.get(index as u32) {
                Some(list_offset) => {
                    let data = byteutils::subslice(
                        self.borrow_data(),
                        list_offset as usize..,
                        "hash list data during validation",
                    )?;
                    let _ = OldHashList::new(data)?;
                }
                None => {}
            }
        }
        Ok(())
    }

    /// Query the hash list for a given hash.
    ///
    /// Returns `None` if there is no hash list related to the hash.
    ///
    /// # Panics
    ///
    /// Panics if the file is corrupt. Use `validate_hash_lists` when
    /// initializing the reader to avoid these panics.
    fn hash_list_for(&self, hash: Dep) -> Option<HashList<'_>> {
        self.hash_list_id_for_dep(hash)
            .map(|id| self.hash_list_for_id(id))
    }

    /// Map a `Dep` to the `HashListId` that uniquely identifies its `HashList`.
    ///
    /// Unless you are interested in `HashList` identity, you want to call
    /// `hash_list_for` instead.
    fn hash_list_id_for_dep(&self, hash: Dep) -> Option<HashListId> {
        let index = self.borrow_indexer().find(hash.into())?;
        self.hash_list_id_for_index(index)
    }

    fn hash_list_id_for_index(&self, index: u32) -> Option<HashListId> {
        Some(HashListId(self.borrow_lookup_table().get(index)?))
    }

    /// Maps a `HashListId` to its `HashList`.
    fn hash_list_for_id(&self, id: HashListId) -> HashList<'_> {
        let list_offset = id.0;
        HashList::Old(OldHashList::new(&self.borrow_data()[list_offset as usize..]).unwrap())
    }

    /// Return whether the given dependent-to-dependency edge is in the graph.
    fn dependent_dependency_edge_exists(&self, dependent: Dep, dependency: Dep) -> bool {
        match self.hash_list_for(dependency) {
            Some(hash_list) => self.hash_list_contains(hash_list, dependent),
            None => false,
        }
    }

    fn contains(&self, dep: Dep) -> bool {
        self.borrow_indexer().find(dep.into()).is_some()
    }
}

/// The indexer table.
///
/// The indexer table maps a hash to an index.
///
/// Memory layout:
///
/// ```txt
///     64 bits
///  +===========+
///  |  length   |
///  +===========+
///  |   hash1   |
///  +-----------+
///  |   hash2   |
///  +-----------+
///  |    ...    |
///  +===========+
/// ```
#[derive(Clone, Copy)]
struct Indexer<'bytes> {
    /// All hashes.
    hashes: &'bytes [u64],
}

impl<'bytes> Indexer<'bytes> {
    fn new(data: &'bytes [u8]) -> Result<Self, String> {
        if data.len() < 8 {
            return Err("not enough bytes to read indexer".to_string());
        }

        // Read in length
        let length = byteutils::read_u64_ne(data);
        if length > (1 << 32) - 1 {
            return Err("indexer: length is too big".to_string());
        }

        // Read in u64 array
        let length: usize = length as usize;
        let indexer_data = byteutils::subslice(data, 8.., "indexer_data")?;
        let indexer_data = byteutils::as_u64_slice(indexer_data)
            .ok_or_else(|| "indexer: data is not properly aligned".to_string())?;
        if indexer_data.len() < length {
            return Err("indexer: not enough hashes".to_string());
        }

        let hashes = &indexer_data[..length];
        Ok(Indexer { hashes })
    }

    /// The number if hashes in the indexer
    #[inline]
    fn len(self) -> usize {
        self.hashes.len()
    }

    /// Binary search the indexer to find the index of a hash.
    #[inline]
    fn find(self, hash: u64) -> Option<u32> {
        if let Ok(index) = self.hashes.binary_search(&hash) {
            Some(index as u32)
        } else {
            None
        }
    }
}

/// The actual lookup table.
///
/// Currently this is a list of pointers to dependent-lists.
///
/// To index in a lookup table with a hash, you should first find
/// the hashes index using the indexer.
///
/// Memory layout:
///
/// ```txt
///           32 bits
///  +========================+
///  |   pointer for hash 1   |
///  +------------------------+
///  |   pointer for hash 2   |
///  +------------------------+
///  |           ...          |
///  +========================+
/// ```
#[derive(Clone, Copy)]
struct LookupTable<'bytes> {
    /// All hash list offsets in the table.
    hash_list_offsets: &'bytes [u32],
}

impl<'bytes> LookupTable<'bytes> {
    fn new(data: &'bytes [u8], len: usize) -> Result<Self, String> {
        // Read in u32 array
        let hash_list_offsets = byteutils::as_u32_slice(data)
            .ok_or_else(|| "lookup table: data is not properly aligned".to_string())?;
        if hash_list_offsets.len() < len {
            return Err("lookup table: not enough pointers".to_string());
        }

        let hash_list_offsets = &hash_list_offsets[..len];
        Ok(LookupTable { hash_list_offsets })
    }

    #[inline]
    fn get(self, index: u32) -> Option<u32> {
        let offset = self.hash_list_offsets.get(index as usize).copied()?;
        if offset == 0 { None } else { Some(offset) }
    }
}

/// A pointer to a list of hashes.
///
/// This data structure is read lazily.
///
/// Memory layout:
///
///
/// ```txt
///           32 bits
///  +========================+
///  |        length          |
///  +========================+
///  |     index of hash 1    |
///  +------------------------+
///  |     index of hash 2    |
///  +------------------------+
///  |           ...          |
///  +========================+
/// ```
#[derive(Clone, Copy)]
pub struct OldHashList<'bytes> {
    indices: &'bytes [u32],
}

impl<'bytes> OldHashList<'bytes> {
    fn new(data: &'bytes [u8]) -> Result<Self, String> {
        let data = byteutils::as_u32_slice(data)
            .ok_or_else(|| "hash list: not properly aligned".to_string())?;

        let len: u32 = *data
            .first()
            .ok_or_else(|| "hash list: couldn't read length".to_string())?;
        let len: usize = len as usize;

        let indices = byteutils::subslice(data, 1.., "hash list.data")?;
        if indices.len() < len {
            return Err("hash list: not enough indices".to_string());
        }
        let indices = &indices[..len];
        Ok(OldHashList { indices })
    }

    #[inline]
    pub fn len(&self) -> u32 {
        self.indices.len() as u32
    }

    pub fn is_empty(&self) -> bool {
        // Return true if there are any RLE blocks, no need to count them in O(n).
        self.len() == 0
    }

    #[inline]
    fn has_index(&self, index: u32) -> bool {
        self.indices.binary_search(&index).is_ok()
    }

    /// Return all raw hash indices in this list.
    pub fn hash_indices(&self) -> impl Iterator<Item = u32> + FusedIterator + '_ {
        self.indices.iter().copied()
    }
}

pub struct NewHashList<'bytes> {
    blocks: &'bytes [RleBlock],

    // The total number of indices that walking `hash_indices()` will yield.
    //
    // This is identical to the sum of all the `blocks` lengths, but it's
    // precomputed here to keep the `len()` method O(1).
    num_indices: u32,
}

impl<'bytes> NewHashList<'bytes> {
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

pub enum HashList<'bytes> {
    Old(OldHashList<'bytes>),
    New(NewHashList<'bytes>),
}

impl<'bytes> HashList<'bytes> {
    // FIXME: Can we delete this? It's O(n) for NewHashList.
    pub fn len(&self) -> u32 {
        match self {
            HashList::Old(x) => x.len(),
            HashList::New(x) => x.len(),
        }
    }

    pub fn is_empty(&self) -> bool {
        match self {
            HashList::Old(x) => x.is_empty(),
            HashList::New(x) => x.is_empty(),
        }
    }

    fn has_index(&self, index: u32) -> bool {
        match self {
            HashList::Old(x) => x.has_index(index),
            HashList::New(x) => x.has_index(index),
        }
    }

    /// Return all raw hash indices in this list.
    pub fn hash_indices(&self) -> impl Iterator<Item = u32> + '_ {
        match self {
            HashList::Old(x) => Either::Left(x.hash_indices()),
            HashList::New(x) => Either::Right(x.hash_indices()),
        }
    }
}
