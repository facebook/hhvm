// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
pub use crate::dep::Dep;

use std::collections::HashMap;
use std::convert::TryInto;
use std::io::{Seek, SeekFrom, Write};

const PAGE_ALIGN: u32 = 4096;

/// Type-safe hash index wrapper
#[derive(Copy, Clone)]
struct HashIndex(u32);

/// Type-safe hash list index wrapper
#[derive(Copy, Clone, PartialEq, Eq, PartialOrd, Ord, Hash)]
pub struct HashListIndex(u32);

/// Phase 1 of writing a dependency graph.
///
/// Phase where all hash sets are written to the file.
pub enum Phase1AllocateHashSets {}

/// Phase 2 of writing a dependency graph.
///
/// Link dependencies with hash sets.
pub enum Phase2RegisterHashSets {}

/// Data structure that can be used to construct a dependency graph.
///
/// Phase is a phantom type parameter (see `Phase*` enums).
#[must_use]
pub struct DepGraphWriter<W, PHASE> {
    state: DepGraphWriterState<W>,
    _phase: std::marker::PhantomData<PHASE>,
}

struct DepGraphWriterState<W> {
    f: W,
    indexer_offset: u32,
    lookup_table_offset: u32,
    next_set_offset: u32,

    sorted_hashes: Vec<u64>,
    indexer: HashMap<u64, HashIndex>,
    lookup_table: HashMap<u64, HashListIndex>,
}

impl<W: Write + Seek> DepGraphWriter<W, Phase1AllocateHashSets> {
    /// Initialize a new dependency graph writer with the given
    /// set of sorted hashes.
    ///
    /// # Panics
    ///
    /// - If the list of hashes is empty.
    /// - If the given list of hashes is not sorted or deduplicated.
    pub fn new(mut f: W, sorted_hashes: Vec<u64>) -> std::io::Result<Self> {
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
        let mut prev_hash = hashes_iter.next().expect("exected at least one hash");
        for h in hashes_iter {
            if h <= prev_hash {
                panic!("hashes not sorted: {} <= {}", h, prev_hash);
            }
            prev_hash = h;
        }

        // Indexer!
        let indexer = sorted_hashes
            .iter()
            .enumerate()
            .map(|(i, h)| (*h, HashIndex(i as u32)))
            .collect();

        // Seek to position to start writing sets of dependents.
        f.seek(SeekFrom::Start(next_set_offset as u64))?;

        Ok(DepGraphWriter {
            state: DepGraphWriterState {
                f,
                indexer_offset,
                lookup_table_offset,
                next_set_offset,

                sorted_hashes,
                indexer,
                lookup_table: HashMap::new(),
            },

            _phase: std::marker::PhantomData,
        })
    }

    pub fn allocate_hash_list<H: Copy + Into<u64>>(
        &mut self,
        set: &[H],
    ) -> std::io::Result<HashListIndex> {
        let offset = self.state.next_set_offset;

        let len: u32 = set.len().try_into().unwrap();
        self.state.f.write_all(&len.to_ne_bytes())?;
        for h in set {
            let i: HashIndex = *self.state.indexer.get(&(*h).into()).unwrap();
            let i: u32 = i.0;
            self.state.f.write_all(&i.to_ne_bytes())?;
        }

        self.state.next_set_offset += 4 + 4 * len;
        Ok(HashListIndex(offset))
    }

    pub fn finalize(self) -> DepGraphWriter<W, Phase2RegisterHashSets> {
        DepGraphWriter {
            state: self.state,
            _phase: std::marker::PhantomData,
        }
    }
}

impl<W: Write + Seek> DepGraphWriter<W, Phase2RegisterHashSets> {
    pub fn register_lookup_table(&mut self, lookup_table: HashMap<u64, HashListIndex>) {
        self.state.lookup_table = lookup_table;
    }

    pub fn finalize(mut self) -> std::io::Result<()> {
        // We write the header
        self.state.f.seek(SeekFrom::Start(0))?;
        self.state
            .f
            .write_all(&self.state.indexer_offset.to_ne_bytes())?;
        self.state
            .f
            .write_all(&self.state.lookup_table_offset.to_ne_bytes())?;

        // We write the indexer table
        let len: u64 = self.state.sorted_hashes.len() as u64;
        self.state.f.write_all(&len.to_ne_bytes())?;
        for s in &self.state.sorted_hashes {
            let s: u64 = *s;
            self.state.f.write_all(&s.to_ne_bytes())?;
        }

        // We write the lookup table
        self.state
            .f
            .seek(SeekFrom::Start(self.state.lookup_table_offset as u64))?;
        for s in &self.state.sorted_hashes {
            let hash_list: HashListIndex = self
                .state
                .lookup_table
                .get(s)
                .copied()
                .unwrap_or(HashListIndex(0));
            let hash_list: u32 = hash_list.0;
            self.state.f.write_all(&hash_list.to_ne_bytes())?;
        }

        Ok(())
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use crate::reader::DepGraphOpener;

    use std::collections::{BTreeSet, HashMap, HashSet};
    use std::iter::FromIterator;

    use tempfile::NamedTempFile;

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
            let (tmpfile, tmpfile_path) = NamedTempFile::new().unwrap().keep().unwrap();

            let mut all_hashes: HashSet<u64> = HashSet::new();
            self.graph.iter().for_each(|(dependency, dependents)| {
                all_hashes.insert(*dependency);
                for h in dependents.iter() {
                    all_hashes.insert(*h);
                }
            });
            let mut all_hashes_vec: Vec<u64> = all_hashes.iter().copied().collect();
            all_hashes_vec.sort();

            let mut writer = DepGraphWriter::new(tmpfile, all_hashes_vec).unwrap();

            let mut lookup_table: HashMap<u64, HashListIndex> = HashMap::new();
            let mut all_sets: HashMap<&BTreeSet<u64>, HashListIndex> = HashMap::new();
            for (dependency, dependents) in self.graph.iter() {
                let hash_list_index = all_sets.entry(dependents).or_insert_with(|| {
                    let set_vec: Vec<u64> = dependents.iter().copied().collect();
                    writer.allocate_hash_list(&set_vec).unwrap()
                });
                lookup_table.insert(*dependency, *hash_list_index);
            }

            let mut writer = writer.finalize();
            writer.register_lookup_table(lookup_table);

            writer.finalize().unwrap();

            tmpfile_path
        }

        fn read_check(&self, path: &std::path::PathBuf) -> Option<String> {
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
