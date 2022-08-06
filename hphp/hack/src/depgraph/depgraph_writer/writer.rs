// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use std::collections::HashMap;
use std::io::Write;

pub use depgraph::dep::Dep;
use memmap::MmapMut;
use parking_lot::Mutex;
use rayon::prelude::*;

const PAGE_ALIGN: u32 = 4096;

pub const NUM_SHARDS: usize = 2048;
const SHARDS_INDEX_MASK: u64 = (NUM_SHARDS - 1) as u64;

#[inline(always)]
pub fn shard_index_for(hash: u64) -> usize {
    (hash & SHARDS_INDEX_MASK) as usize
}

/// Type-safe hash index wrapper
#[derive(Copy, Clone)]
pub struct HashIndex(u32);

/// Type-safe hash list index wrapper
#[derive(Copy, Clone, PartialEq, Eq, PartialOrd, Ord, Hash)]
pub struct HashListIndex(pub u32);

impl HashListIndex {
    #[inline(always)]
    pub fn incr(self, x: u32) -> Self {
        Self(self.0 + x)
    }
}

/// Sharded hash-to-hash-index map, for writing.
struct ShardedIndexerWriter {
    shards: Vec<Mutex<HashMap<u64, HashIndex>>>,
}

impl ShardedIndexerWriter {
    fn new() -> Self {
        Self {
            shards: std::iter::repeat_with(|| Mutex::new(HashMap::new()))
                .take(NUM_SHARDS)
                .collect(),
        }
    }

    fn insert(&self, hash: u64, hash_index: HashIndex) {
        let shard_index = shard_index_for(hash);
        self.shards
            .get(shard_index)
            .unwrap()
            .lock()
            .insert(hash, hash_index);
    }

    fn into_read_only(self) -> ShardedIndexer {
        ShardedIndexer::new(
            self.shards
                .into_iter()
                .map(|lck| lck.into_inner())
                .collect(),
        )
    }
}

/// Sharded hash-to-hash-index map, for reading.
#[derive(Clone)]
pub struct ShardedIndexer {
    shards: Vec<HashMap<u64, HashIndex>>,
}

impl ShardedIndexer {
    fn new(shards: Vec<HashMap<u64, HashIndex>>) -> Self {
        Self { shards }
    }

    fn get(&self, hash: u64) -> Option<HashIndex> {
        let shard_index = shard_index_for(hash);
        self.shards.get(shard_index).unwrap().get(&hash).copied()
    }
}

/// Sharded lookup table, used for construction
pub struct ShardedLookupTableWriter {
    shards: Vec<Mutex<HashMap<u64, HashListIndex>>>,
}

impl ShardedLookupTableWriter {
    pub fn new() -> Self {
        Self {
            shards: std::iter::repeat_with(|| Mutex::new(HashMap::new()))
                .take(NUM_SHARDS)
                .collect(),
        }
    }

    pub fn insert(&self, dependency: u64, hash_list_index: HashListIndex) {
        let shard_index = shard_index_for(dependency);
        let mut m = self.shards.get(shard_index).unwrap().lock();
        m.insert(dependency, hash_list_index);
    }

    fn into_read_only(self) -> ShardedLookupTable {
        ShardedLookupTable::new(
            self.shards
                .into_iter()
                .map(|lck| lck.into_inner())
                .collect(),
        )
    }
}

/// Sharded lookup table, used for reading
struct ShardedLookupTable {
    shards: Vec<HashMap<u64, HashListIndex>>,
}

impl ShardedLookupTable {
    fn new(shards: Vec<HashMap<u64, HashListIndex>>) -> Self {
        Self { shards }
    }

    fn get(&self, hash: u64) -> Option<HashListIndex> {
        let shard_index = shard_index_for(hash);
        self.shards.get(shard_index).unwrap().get(&hash).copied()
    }
}

const SHARDED_MMAP_SHARD_SIZE: usize = 1 * 1024 * 1024; /* 2048 x 1 MiB = 2 GiB */

/// Utility to provide safe but efficient access to the writable mmap.
pub struct ShardedMmap<'a> {
    shards: Vec<Mutex<&'a mut [u8]>>,
}

impl<'a> ShardedMmap<'a> {
    fn new(mut bytes: &'a mut [u8]) -> Self {
        let mut shards = Vec::new();

        while bytes.len() > SHARDED_MMAP_SHARD_SIZE {
            let (left, right) = bytes.split_at_mut(SHARDED_MMAP_SHARD_SIZE);
            shards.push(Mutex::new(left));
            bytes = right;
        }
        shards.push(Mutex::new(bytes));
        Self { shards }
    }

    fn write(&self, mut at: usize, xs: &[u8]) {
        let mut start: usize = 0;
        let end: usize = xs.len();
        while start < end {
            let shard_index = at / SHARDED_MMAP_SHARD_SIZE;
            let shard_offset = at % SHARDED_MMAP_SHARD_SIZE;
            let shard_end = (shard_index + 1) * SHARDED_MMAP_SHARD_SIZE;
            let to_write = std::cmp::min(end - start, shard_end - at);

            self.shards.get(shard_index).unwrap().lock()[shard_offset..shard_offset + to_write]
                .copy_from_slice(&xs[start..start + to_write]);

            at += to_write;
            start += to_write;
        }
    }
}

/// Phase 1 of writing a dependency graph.
///
/// Phase where space is allocated for all hash sets
pub struct Phase1AllocateHashSets;

/// Phase 2 of writing a dependency graph.
///
/// Phase were all hash sets are written to file
pub struct Phase2WriteHashSets {
    mmap: MmapMut,
}

/// Phase 3 of writing a dependency graph.
///
/// Link dependencies with hash sets.
pub struct Phase3RegisterLookupTable {
    mmap: MmapMut,
}

/// Data structure that can be used to construct a dependency graph.
///
/// Phase is a phantom type parameter (see `Phase*` enums).
#[must_use]
pub struct DepGraphWriter<PHASE> {
    state: DepGraphWriterState,
    phase: PHASE,
}

struct DepGraphWriterState {
    indexer_offset: u32,
    lookup_table_offset: u32,
    next_set_offset: u32,

    sorted_hashes: Vec<u64>,
    indexer: ShardedIndexer,
    lookup_table: ShardedLookupTable,
}

impl DepGraphWriter<Phase1AllocateHashSets> {
    /// Initialize a new dependency graph writer with the given
    /// set of sorted hashes.
    ///
    /// # Panics
    ///
    /// - If the list of hashes is empty.
    /// - If the given list of hashes is not sorted or deduplicated.
    pub fn new(sorted_hashes: Vec<u64>) -> std::io::Result<Self> {
        let num_hashes: u32 = sorted_hashes.len().try_into().unwrap();

        // The indexer starts right after the header
        let indexer_offset = 4 * 2;

        // Lookup table offset is on page boundary, after indexer
        let mut lookup_table_offset = indexer_offset + 8 + num_hashes * 8;
        if lookup_table_offset & (PAGE_ALIGN - 1) != 0 {
            lookup_table_offset += PAGE_ALIGN;
            lookup_table_offset &= !(PAGE_ALIGN - 1);
        }

        // Next set offset is on page boundary, after look up table
        let mut next_set_offset = lookup_table_offset + num_hashes * 4;
        if next_set_offset & (PAGE_ALIGN - 1) != 0 {
            next_set_offset += PAGE_ALIGN;
            next_set_offset &= !(PAGE_ALIGN - 1);
        }

        // Check sorted
        let mut hashes_iter = sorted_hashes.iter();
        let mut prev_hash = hashes_iter.next();
        for h in hashes_iter {
            if h <= prev_hash.unwrap() {
                panic!("hashes not sorted: {} <= {}", h, prev_hash.unwrap());
            }
            prev_hash = Some(h);
        }

        // Indexer!
        let indexer = ShardedIndexerWriter::new();
        sorted_hashes.par_iter().enumerate().for_each(|(i, h)| {
            indexer.insert(*h, HashIndex(i as u32));
        });

        Ok(DepGraphWriter {
            state: DepGraphWriterState {
                indexer_offset,
                lookup_table_offset,
                next_set_offset,

                sorted_hashes,
                indexer: indexer.into_read_only(),
                lookup_table: ShardedLookupTable::new(vec![]),
            },

            phase: Phase1AllocateHashSets,
        })
    }

    pub fn first_hash_list_offset(&self) -> u32 {
        self.state.next_set_offset
    }

    pub fn size_needed_for_hash_list<H: Copy + Into<u64>>(set: &[H]) -> u32 {
        let len: u32 = set.len().try_into().unwrap();

        4 + 4 * len
    }

    pub fn open_writer(
        mut self,
        file: &std::fs::File,
        next_set_offset: HashListIndex,
    ) -> std::io::Result<DepGraphWriter<Phase2WriteHashSets>> {
        self.state.next_set_offset = next_set_offset.0;
        file.set_len(self.state.next_set_offset as u64)?;
        // Safety: we rely on the memmap library to provide safety.
        let mmap = unsafe { MmapMut::map_mut(file) }?;
        Ok(DepGraphWriter {
            state: self.state,
            phase: Phase2WriteHashSets { mmap },
        })
    }
}

impl DepGraphWriter<Phase2WriteHashSets> {
    pub fn clone_indexer(&self) -> ShardedIndexer {
        self.state.indexer.clone()
    }

    pub fn sharded_mmap<'a>(&'a mut self) -> ShardedMmap<'a> {
        ShardedMmap::new(&mut self.phase.mmap)
    }

    pub fn write_hash_list(
        mmap: &ShardedMmap<'_>,
        indexer: &ShardedIndexer,
        hash_list_index: HashListIndex,
        set: &[u64],
    ) -> std::io::Result<()> {
        let len: u32 = set.len().try_into().unwrap();
        let size: usize = (len as usize) * 4 + 4;
        let mut buf: Vec<u8> = Vec::with_capacity(size);

        buf.write_all(&len.to_ne_bytes())?;
        for h in set {
            let i: HashIndex = indexer.get(*h).unwrap();
            let i: u32 = i.0;
            buf.write_all(&i.to_ne_bytes())?;
        }

        let offset = hash_list_index.0 as usize;
        mmap.write(offset as usize, &buf);
        Ok(())
    }

    pub fn finalize(self) -> DepGraphWriter<Phase3RegisterLookupTable> {
        DepGraphWriter {
            state: self.state,
            phase: Phase3RegisterLookupTable {
                mmap: self.phase.mmap,
            },
        }
    }
}

impl DepGraphWriter<Phase3RegisterLookupTable> {
    pub fn register_lookup_table(
        &mut self,
        lookup_table: ShardedLookupTableWriter,
    ) -> std::io::Result<()> {
        let lookup_table = lookup_table.into_read_only();

        let mmap =
            ShardedMmap::new(&mut self.phase.mmap[self.state.lookup_table_offset as usize..]);

        // We write the lookup table
        self.state
            .sorted_hashes
            .par_iter()
            .enumerate()
            .for_each(|(i, s)| {
                let hash_list: HashListIndex = lookup_table.get(*s).unwrap_or(HashListIndex(0));
                let hash_list: u32 = hash_list.0;

                let offset: usize = i * std::mem::size_of_val(&hash_list);
                mmap.write(offset, &hash_list.to_ne_bytes());
            });

        self.state.lookup_table = lookup_table;
        Ok(())
    }

    pub fn write_indexer_and_finalize(mut self) -> std::io::Result<()> {
        let mut buf: &mut [u8] = &mut self.phase.mmap;

        // We write the header
        buf.write_all(&self.state.indexer_offset.to_ne_bytes())?;
        buf.write_all(&self.state.lookup_table_offset.to_ne_bytes())?;

        // We write the indexer table
        let len: u64 = self.state.sorted_hashes.len() as u64;
        buf.write_all(&len.to_ne_bytes())?;
        for s in &self.state.sorted_hashes {
            let s: u64 = *s;
            buf.write_all(&s.to_ne_bytes())?;
        }

        self.phase.mmap.flush()
    }
}

#[cfg(test)]
mod tests {
    use std::collections::BTreeSet;
    use std::collections::HashMap;
    use std::collections::HashSet;
    use std::fs;

    use depgraph::reader::DepGraphOpener;
    use tempfile::NamedTempFile;

    use super::*;

    struct T {
        graph: HashMap<u64, BTreeSet<u64>>,
    }

    impl T {
        fn new(graph: &[(u64, &[u64])]) -> Self {
            let graph = graph
                .iter()
                .map(|(k, v)| (*k, BTreeSet::from_iter(v.iter().copied())))
                .collect();
            T { graph }
        }

        fn run(&self) {
            let path = self.write();
            let err = match std::panic::catch_unwind(|| self.read_check(&path)) {
                Ok(err) => err,
                Err(err) => Some(format!("{:?}", err)),
            };

            match err {
                None => std::fs::remove_file(&path).unwrap(),
                Some(err) => {
                    panic!("error in test with file {:?}: {}", path, err);
                }
            }
        }

        fn write(&self) -> std::path::PathBuf {
            let (_tmpfile, tmpfile_path) = NamedTempFile::new().unwrap().keep().unwrap();
            let tf = fs::OpenOptions::new()
                .read(true)
                .write(true)
                .open(&tmpfile_path)
                .unwrap();

            let mut all_hashes: HashSet<u64> = HashSet::new();
            self.graph.iter().for_each(|(dependency, dependents)| {
                all_hashes.insert(*dependency);
                for h in dependents.iter() {
                    all_hashes.insert(*h);
                }
            });
            let mut all_hashes_vec: Vec<u64> = all_hashes.iter().copied().collect();
            all_hashes_vec.sort_unstable();

            let writer = DepGraphWriter::new(all_hashes_vec).unwrap();

            let lookup_table = ShardedLookupTableWriter::new();
            let mut all_sets: HashMap<&BTreeSet<u64>, HashListIndex> = HashMap::new();
            let mut next_offset = HashListIndex(writer.first_hash_list_offset());
            for (dependency, dependents) in self.graph.iter() {
                let set_vec: Vec<u64> = dependents.iter().copied().collect();

                let hash_list_index = all_sets.entry(dependents).or_insert_with(|| {
                    let size_needed = DepGraphWriter::size_needed_for_hash_list(&set_vec);
                    let old_offset = next_offset;
                    next_offset = next_offset.incr(size_needed);
                    old_offset
                });
                lookup_table.insert(*dependency, *hash_list_index);
            }

            let mut writer = writer.open_writer(&tf, next_offset).unwrap();
            let indexer = writer.clone_indexer();
            let mmap = writer.sharded_mmap();
            for (dependents, hash_list_index) in all_sets.iter() {
                let set_vec: Vec<u64> = dependents.iter().copied().collect();
                DepGraphWriter::write_hash_list(&mmap, &indexer, *hash_list_index, &set_vec)
                    .unwrap();
            }

            let mut writer = writer.finalize();
            writer.register_lookup_table(lookup_table).unwrap();
            writer.write_indexer_and_finalize().unwrap();

            tmpfile_path
        }

        fn read_check(&self, path: &std::path::Path) -> Option<String> {
            let opener = DepGraphOpener::from_path(path).unwrap();
            let s = opener.open().unwrap();
            s.validate_hash_lists().unwrap();

            let new_graph: HashMap<u64, BTreeSet<u64>> = self
                .graph
                .keys()
                .map(|hash| match s.hash_list_for(Dep::new(*hash)) {
                    None => panic!("no hash set for {}", hash),
                    Some(set) => ((*hash), s.hash_list_hashes(set).map(|x| x.into()).collect()),
                })
                .collect();

            assert_eq!(new_graph, self.graph);

            None
        }
    }

    #[test]
    fn test_1() {
        T::new(&[(1, &[1, 2])]).run();
    }

    #[test]
    fn test_2() {
        T::new(&[(1, &[1, 2]), (3, &[1, 2]), (5, &[1, 2, 6])]).run();
    }

    #[test]
    fn test_3() {
        T::new(&[
            (1, &[1, 2]),
            (3, &[1, 2]),
            (5, &[1, 2, 6]),
            (9, &[1, 4, 6, 2]),
        ])
        .run();
    }

    #[test]
    fn test_4() {
        let graph: Vec<(u64, Vec<u64>)> = (0..10000)
            .map(|i| {
                let adj = Vec::from([i + 1, i + 2, i + 3]);
                (i, adj)
            })
            .collect();
        let graph_ref: Vec<(u64, &[u64])> = graph.iter().map(|(h, s)| (*h, s.as_slice())).collect();
        T::new(&graph_ref).run();
    }
}

#[cfg(test)]
mod sharded_mmap_tests {
    use super::*;

    #[derive(Debug)]
    struct TestScenario {
        mmap_size: usize,
        write_size: usize,
        at: usize,
    }

    fn test_write(test: TestScenario) {
        let TestScenario {
            mmap_size,
            write_size,
            at,
        } = test;

        let mut mmap_vec: Vec<u8> = vec![0; mmap_size];
        let mut control_vec: Vec<u8> = vec![0; mmap_size];
        let write_vec: Vec<u8> = vec![1; write_size];

        let sharded_mmap = ShardedMmap::new(&mut mmap_vec);
        sharded_mmap.write(at, &write_vec);
        control_vec[at..at + write_size].copy_from_slice(&write_vec);
        if control_vec != mmap_vec {
            assert!(false, "{:?} failed", test);
        }
    }

    #[test]
    fn test_sharded_mmap() {
        // Let's write something (1) half the shard size and (2) three
        // times the shard size at the following positions:
        let ats = vec![
            0,
            2 * SHARDED_MMAP_SHARD_SIZE / 3,
            SHARDED_MMAP_SHARD_SIZE - 1,
            SHARDED_MMAP_SHARD_SIZE,
            SHARDED_MMAP_SHARD_SIZE + 1,
            4 * SHARDED_MMAP_SHARD_SIZE / 3,
            5 * SHARDED_MMAP_SHARD_SIZE / 3,
        ];
        for at in ats.into_iter() {
            test_write(TestScenario {
                mmap_size: 10 * SHARDED_MMAP_SHARD_SIZE,
                write_size: SHARDED_MMAP_SHARD_SIZE / 2,
                at,
            });
            test_write(TestScenario {
                mmap_size: 10 * SHARDED_MMAP_SHARD_SIZE,
                write_size: 3 * SHARDED_MMAP_SHARD_SIZE,
                at,
            });
        }
    }
}
